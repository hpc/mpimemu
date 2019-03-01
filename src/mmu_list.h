/*
 * Copyright (c) 2012-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifndef MMU_LIST_INCLUDED
#define MMU_LIST_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

struct mmu_list_item_t;
typedef struct mmu_list_item_t mmu_list_item_t;

struct mmu_list_t;
typedef struct mmu_list_t mmu_list_t;

int
mmu_list_construct(mmu_list_t **list);

int
mmu_list_destruct(mmu_list_t **list);

int
mmu_list_reserve(mmu_list_t *list,
                 size_t reserve_size);

int
mmu_list_append(mmu_list_t *list,
                const void *base,
                size_t extent);

int
mmu_list_pop(mmu_list_t *list,
             void **itemp);

int
mmu_list_dump(const char *prefix,
              mmu_list_t *list,
              void (*print_fn)(const char *oprefix,
                               const void *item));

#endif /* ifndef MMU_LIST_INCLUDED */
