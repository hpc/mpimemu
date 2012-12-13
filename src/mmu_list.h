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
