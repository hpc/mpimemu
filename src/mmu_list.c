/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmu_constants.h"
#include "mmu_conv_macros.h"
#include "mmu_util.h"
#include "mmu_list.h"

#include <stdlib.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDBOOL_H
#include "stdbool.h"
#endif

/* debug harness */
#define MMU_LIST_TEST_HARNESS 0

/* list item type */
struct mmu_list_item_t {
    /* pointer to payload */
    void *datap;
    /* size of payload */
    size_t size;
};

struct mmu_list_t {
    /* number of actual items in the list */
    size_t size;
    /* size of the list.  size does not necessarily equal capacity */
    size_t capacity;
    /* points to array of list item pointers */
    mmu_list_item_t **item_ptrs;
};

static inline int
list_item_construct(mmu_list_item_t **item,
                    const void *payload_base,
                    size_t payload_extent);
static inline int
list_item_destruct(mmu_list_item_t *item,
                   bool preserve_payload);

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
list_item_construct(mmu_list_item_t **item,
                    const void *payload_base,
                    size_t payload_extent)
{
    int rc = MMU_FAILURE;
    mmu_list_item_t *tmp;
    void *payload;

    if (NULL == item || NULL == payload_base || 0 == payload_extent) {
        return MMU_FAILURE_INVALID_ARG;
    }
    if (NULL == (tmp = calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    /* allocate enough space for the payload */
    if (NULL == (payload = calloc(1, payload_extent))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto err;
    }
    /* copy the payload */
    (void)memmove(payload, payload_base, payload_extent);
    tmp->datap = payload;
    tmp->size = payload_extent;
    *item = tmp;

    return MMU_SUCCESS;

err:
    list_item_destruct(tmp, false);
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
list_item_destruct(mmu_list_item_t *item,
                   bool preserve_payload)
{
    if (NULL == item) return MMU_FAILURE_INVALID_ARG;

    if (NULL != item->datap && !preserve_payload) free(item->datap);
    free(item);

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
list_grow(mmu_list_t *list,
          size_t new_capacity)
{
    if (NULL == list || new_capacity <= 0) return MMU_FAILURE_INVALID_ARG;
    /* this is easy. */
    if (list->capacity >= new_capacity) return MMU_SUCCESS;
    /* if we are here, it is safe to realloc to the given size */
    if (NULL == (list->item_ptrs =
                 realloc(list->item_ptrs,
                         new_capacity * sizeof(mmu_list_item_t *)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    /* update list capacity to the new capacity */
    list->capacity = new_capacity;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline bool
list_full(const mmu_list_t *list) {
    return (list->capacity == list->size);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_list_reserve(mmu_list_t *list,
                 size_t reserve_size)
{
    return list_grow(list, reserve_size);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_list_construct(mmu_list_t **list)
{
    mmu_list_t *tmp;

    if (NULL == list) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp = calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    /* start off with nothing */
    tmp->item_ptrs = NULL;
    tmp->size = 0;
    tmp->capacity = 0;
    *list = tmp;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_list_destruct(mmu_list_t **list)
{
    mmu_list_t *tmp;

    if (NULL == list) return MMU_FAILURE_INVALID_ARG;

    tmp = *list;
    if (NULL != tmp) {
        size_t item;
        if (NULL != tmp->item_ptrs) {
            for (item = 0; item < tmp->size; ++item) {
                (void)list_item_destruct(tmp->item_ptrs[item], false);
            }
            free(tmp->item_ptrs);
        }
        tmp->capacity = 0;
        tmp->size = 0;
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* adds an element to the end of the list */
int
mmu_list_append(mmu_list_t *list,
                const void *base,
                size_t extent)
{
    mmu_list_item_t *new_item;
    int rc = MMU_FAILURE;

    if (NULL == list || NULL == base || 0 == extent) {
        return MMU_FAILURE_INVALID_ARG;
    }
    /* construct the new list element */
    if (MMU_SUCCESS != (rc = list_item_construct(&new_item, base, extent))) {
        goto out;
    }
    /* list_grow takes care of all the magic dealing with whether or not the
     * list will actually grow. so, always call this with size + 1 before we add
     * the new item to the list. */
    if (MMU_SUCCESS != (rc = list_grow(list, list->size + 1))) {
        goto out;
    }
    /* add the item to the list */
    list->item_ptrs[list->size] = new_item;
    list->size++;

out:
    if (MMU_SUCCESS != rc) list_item_destruct(new_item, false);

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* returns last item on the list */
/* Note: user is responsible for freeing popped item */
int
mmu_list_pop(mmu_list_t *list,
             void **itemp)
{
    if (NULL == list || NULL == itemp) return MMU_FAILURE_INVALID_ARG;

    /* no items in the list */
    if (0 == list->size) {
        return MMU_FAILURE_LIST_POP;
    }
    *itemp = list->item_ptrs[--list->size]->datap;
    /* now we can safely destruct all the infrastructure around the data */
    return list_item_destruct(list->item_ptrs[list->size], true);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_list_dump(const char *prefix,
              mmu_list_t *list,
              void (*print_fn)(const char *oprefix,
                               const void *item))
{
    size_t item = 0;

    if (NULL == list) return MMU_FAILURE_INVALID_ARG;

    if (NULL == print_fn) return MMU_SUCCESS;
    /* print from head to tail */
    for (item = 0; item < list->size; ++item) {
        print_fn(prefix, list->item_ptrs[item]->datap);
    }

    return MMU_SUCCESS;
}

#if (MMU_LIST_TEST_HARNESS == 1)

void
print_doubles(const char *prefix, const void *item)
{
    printf("%sitem: %lf\n", prefix, *(double *)item);
}

#include <assert.h>
int
main(int argc, const char **argv)
{
    mmu_list_t *list_a = NULL;

    double d1 = 1.14, *p1 = NULL;
    double d2 = 2.14, *p2 = NULL;
    double d3 = 3.14, *p3 = NULL;

    printf("starting %s%s test\n", __FILE__, "\b\b");

    assert(MMU_SUCCESS == mmu_list_construct(&list_a));
    assert(MMU_SUCCESS == mmu_list_reserve(list_a, 2));
    assert(MMU_SUCCESS == mmu_list_append(list_a, &d1, sizeof(double)));
    assert(MMU_SUCCESS == mmu_list_append(list_a, &d2, sizeof(double)));
    assert(MMU_SUCCESS == mmu_list_append(list_a, &d3, sizeof(double)));

    assert(MMU_SUCCESS == mmu_list_dump(list_a, print_doubles));

    assert(MMU_SUCCESS == mmu_list_pop(list_a, (void **)&p1));
    assert(MMU_SUCCESS == mmu_list_pop(list_a, (void **)&p2));
    assert(MMU_SUCCESS == mmu_list_pop(list_a, (void **)&p3));

    printf("p1: %lf\n", *(double *)p1);
    printf("p2: %lf\n", *(double *)p2);
    printf("p3: %lf\n", *(double *)p3);

    free(p1); free(p2); free(p3);

    assert(MMU_SUCCESS == mmu_list_destruct(&list_a));

    return EXIT_SUCCESS;
}
#endif
