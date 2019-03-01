/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifndef MMU_MEMORY_INCLUDED
#define MMU_MEMORY_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mmu_list.h"
#include "mmu_memory_common.h"

typedef struct mmu_memory_t {
    /* sample rate in samples/s */
    int sample_rate;
    /* sampling time in s */
    int sample_time;
    /* number of sample types */
    size_t num_sample_types;
    /* array of list pointers - one for each sample type */
    mmu_list_t **sample_list;
    /* /// sample types /// */
    /* list of pre-mpi_init samples (self) */
    mmu_list_t *self_pre_mpi_init_samples;
    /* list of pre-mpi_init samples (node) */
    mmu_list_t *node_pre_mpi_init_samples;
    /* list of post-mpi_init samples (self) */
    mmu_list_t *self_post_mpi_init_samples;
    /* list of post-mpi_init samples (node/bogus) */
    mmu_list_t *node_post_mpi_init_samples;
} mmu_memory_t;

int
mmu_memory_construct(mmu_memory_t **m);

int
mmu_memory_destruct(mmu_memory_t **m);

int
mmu_memory_reserve_memory(mmu_memory_t *m);

int
mmu_memory_mma_ptr_list_construct(size_t list_len,
                                  mmu_memory_sample_mma_t ***new_ptr_list);
int
mmu_memory_mma_ptr_list_destruct(mmu_memory_sample_mma_t ***ptr_list);

int
mmu_memory_set_sample_rate(mmu_memory_t *m,
                           int samp_rate);

int
mmu_memory_set_sample_time(mmu_memory_t *m,
                           int samp_time);

int
mmu_memory_sample_memory_usage(mmu_memory_t *m,
                               mmu_memory_flags_t flags);

int
mmu_memory_get_local_mmas(mmu_memory_t *m,
                          mmu_memory_sample_mma_t ***new_mma_ptr_list);

char *
mmu_memory_get_item_label(mmu_memory_list_type_t type,
                          int type_index);

int
mmu_memory_get_total_iters(const mmu_memory_t *m,
                           size_t *total_iters);

useconds_t
mmu_memory_get_usleep_time_per_iteration(const mmu_memory_t *m);

#endif /* ifndef MMU_MEMORY_INCLUDED */
