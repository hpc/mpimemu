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

#ifndef MMU_MEMORY_SAMPLE_INCLUDED
#define MMU_MEMORY_SAMPLE_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmu_list.h"
#include "mmu_memory_common.h"

/* holds values for a single sample type */
typedef struct mmu_memory_sample_t {
    /* list length - note: list_len does not necessarily equal max_index */
    size_t list_len;
    /* max valid index */
    size_t max_index;
    /* memory flags */
    mmu_memory_flags_t flags;
    /* sample elements - in kilobytes */
    unsigned long long int elements_in_kb[MMU_MEMORY_SAMP_LIST_LEN];
} mmu_memory_sample_t;

int
mmu_memory_sample_construct(mmu_memory_sample_t **samp,
                            mmu_memory_flags_t flags);

int
mmu_memory_sample_destruct(mmu_memory_sample_t **m);

int
mmu_memory_sample_mma_construct(size_t lists_length,
                                mmu_memory_sample_mma_t **m);

int
mmu_memory_sample_mma_destruct(mmu_memory_sample_mma_t **m);

int
mmu_memory_sample_get_mma(mmu_list_t *target_list,
                          mmu_memory_sample_mma_t **resultp);

#endif /* ifndef MMU_MEMORY_SAMPLE_INCLUDED */
