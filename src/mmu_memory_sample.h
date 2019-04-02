/*
 * Copyright (c) 2012-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
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
