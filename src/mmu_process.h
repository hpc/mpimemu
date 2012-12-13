/**
 * Copyright (c) 2010-2012 Los Alamos National Security, LLC.
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

#ifndef MMU_PROCESS_INCLUDED
#define MMU_PROCESS_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#include "mmu_memory.h"
#include "mmu_mpi.h"

typedef struct mmu_process_t {
    /* my pid */
    pid_t pid;
    /* start time buffer pointer */
    char *start_time_buf;
    /* host name buffer pointer */
    char *hostname_buf;
    /* points to mpi-related information */
    mmu_mpi_t *mpi;
    /* points to memory-related stuf information */
    mmu_memory_t *memory;
} mmu_process_t;

int
mmu_process_construct(mmu_process_t **p);

int
mmu_process_destruct(mmu_process_t **p);

int
mmu_process_reserve_memory(mmu_process_t *p);

int
mmu_process_set_sample_rate(mmu_process_t *p,
                            int samp_rate);

int
mmu_process_set_sample_time(mmu_process_t *p,
                            int samp_time);

int
mmu_process_sample_memory_usage_self(mmu_process_t *p);

int
mmu_process_sample_memory_usage_all(mmu_process_t *p);

int
mmu_process_sample_dump(const mmu_process_t *p);

const char *
mmu_process_get_mpi_version_str(const mmu_process_t *p);

int
mmu_process_init_mpi(mmu_process_t *p,
                     int argc,
                     char **argv);

int
mmu_process_finalize_mpi(mmu_process_t *p);

bool
mmu_process_mpi_initialized(void);

bool
mmu_process_is_delegate(const mmu_process_t *p);

bool
mmu_process_is_hood_delegate(const mmu_process_t *p);

int
mmu_process_get_world_size(const mmu_process_t *p);

int
mmu_process_get_num_hoods(const mmu_process_t *p);

int
mmu_process_get_hood_size(const mmu_process_t *p);

bool
mmu_process_with_workload(const mmu_process_t *p);

int
mmu_process_process_usage(const mmu_process_t *p);

int
mmu_process_enable_workload(mmu_process_t *p,
                            bool enable);

#endif /* ifndef MMU_PROCESS_INCLUDED */
