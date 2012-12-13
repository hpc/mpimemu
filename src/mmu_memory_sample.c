/**
 * Copyright (c) 2012      Los Alamos National Security, LLC.
 *                         All rights reserved.
 *
 * This program was prepared by Los Alamos National Security, LLC at Los Alamos
 * National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with the U.S.
 * Department of Energy (DOE). All rights in the program are reserved by the DOE
 * and Los Alamos National Security, LLC. Permission is granted to the public to
 * copy and use this software without charge, provided that this Notice and any
 * statement of authorship are reproduced on all copies. Neither the U.S.
 * Government nor LANS makes any warranty, express or implied, or assumes any
 * liability or responsibility for the use of this software.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmu_constants.h"
#include "mmu_conv_macros.h"
#include "mmu_util.h"
#include "mmu_list.h"
#include "mmu_memory_sample.h"

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_sample_construct(mmu_memory_sample_t **samp,
                            mmu_memory_flags_t flags)
{
    int rc = MMU_SUCCESS;
    mmu_memory_sample_t *tmp = NULL;

    if (NULL == samp) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp = calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    tmp->flags = flags;
    tmp->list_len = MMU_MEMORY_SAMP_LIST_LEN;
    if (MMU_MEMORY_SAMPLE_SELF(flags) ||
        MMU_MEMORY_SAMPLE_SELF_BOGUS(flags)) {
        tmp->max_index = MMU_MEMORY_PROC_SELF_TAB_LEN - 1;
    }
    else if (MMU_MEMORY_SAMPLE_NODE(flags) ||
             MMU_MEMORY_SAMPLE_NODE_BOGUS(flags)) {
        tmp->max_index = MMU_MEMORY_PROC_NODE_TAB_LEN - 1;
    }
    else {
        fprintf(stderr, MMU_ERR_PREFIX"invalid mmu memory sample type.\n");
        rc = MMU_FAILURE_INVALID_ARG;
        goto out;
    }
    *samp = tmp;
out:
    if (rc != MMU_SUCCESS) {
        (void)mmu_memory_sample_destruct(&tmp);
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_sample_destruct(mmu_memory_sample_t **m)
{
    mmu_memory_sample_t *tmp = NULL;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    tmp = *m;
    if (NULL != tmp) {
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_sample_mma_destruct(mmu_memory_sample_mma_t **m)
{
    mmu_memory_sample_mma_t *tmp;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    tmp = *m;
    if (NULL != tmp) {
        if (NULL != tmp->mins) free(tmp->mins);
        if (NULL != tmp->maxes) free(tmp->maxes);
        if (NULL != tmp->aves) free(tmp->aves);
        if (NULL != tmp->totals) free(tmp->totals);
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_sample_mma_construct(size_t lists_length,
                                mmu_memory_sample_mma_t **m)
{
    mmu_memory_sample_mma_t *tmp;
    int rc = MMU_SUCCESS;

    if (NULL == (tmp = calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }

    tmp->lists_length = lists_length;

    if (NULL == (tmp->mins = calloc(lists_length, sizeof(*(tmp->mins))))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto out;
    }
    if (NULL == (tmp->maxes = calloc(lists_length, sizeof(*(tmp->maxes))))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto out;
    }
    if (NULL == (tmp->totals = calloc(lists_length, sizeof(*(tmp->totals))))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto out;
    }
    if (NULL == (tmp->aves = calloc(lists_length, sizeof(*(tmp->aves))))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto out;
    }
    *m = tmp;

out:
    if (MMU_SUCCESS != rc) {
        (void)mmu_memory_sample_mma_destruct(&tmp);
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_sample_get_mma(mmu_list_t *target_list,
                          mmu_memory_sample_mma_t **resultp)
{
    int rc = MMU_FAILURE;
    size_t i = 0;
    mmu_memory_sample_mma_t *mma = NULL;
    mmu_memory_sample_t *samp = NULL;
    bool first_time = true;
    size_t num_samples = 0;
    size_t list_len = 0;

    if (NULL == target_list || NULL == resultp) return MMU_FAILURE_INVALID_ARG;

    /* iterate over all samples within the target list */
    while (MMU_FAILURE_LIST_POP != (rc =
           mmu_list_pop(target_list, (void **)&samp))) {
        /* make sure that another type of error didn't occur */
        if (MMU_SUCCESS != rc) {
            fprintf(stderr,
                    MMU_ERR_PREFIX"mmu_list_pop exception: %d (%s)\n",
                    rc, mmu_util_rc2str(rc));
            goto out;
        }
        if (first_time) {
            list_len = samp->max_index + 1;
            if (MMU_SUCCESS != (rc =
                mmu_memory_sample_mma_construct(list_len, &mma))) {
                (void)mmu_memory_sample_mma_destruct(&mma);
                return rc;
            }
        }
        for (i = 0; i < list_len; ++i) {
            if (first_time) {
                mma->mins[i] = mma->maxes[i] = samp->elements_in_kb[i];
                /* mma->totals[i] calloc'd so no init to 0 required */
            }
            else {
                /* update min */
                if (samp->elements_in_kb[i] < mma->mins[i]) {
                    mma->mins[i] = samp->elements_in_kb[i];
                }
                /* update max */
                if (samp->elements_in_kb[i] > mma->maxes[i]) {
                    mma->maxes[i] = samp->elements_in_kb[i];
                }
            }
            /* update running total for average */
            mma->totals[i] += samp->elements_in_kb[i];
            /* rudimentary overflow protection */
            if (mma->totals[i] < samp->elements_in_kb[i]) {
                fprintf(stderr, MMU_ERR_PREFIX
                        "overflow exception. cannot continue.\n");
                rc = MMU_FAILURE_OVERFLOW;
                goto out;
            }
        }
        /* up to us to free the returned samp */
        free(samp);
        ++num_samples;
        first_time = false;
    }
    /* now that the mma items have been populated, calculate averages */
    for (i = 0; i < list_len; ++i) {
        mma->aves[i] = (0 == mma->totals[i] || 0 == num_samples) ?
            0.0 : (double)mma->totals[i] / (double)num_samples;
    }

    /* caller is responsible for freeing returned resources */
    *resultp = mma;

out:
    return MMU_SUCCESS;
}
