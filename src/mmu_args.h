/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifndef MMU_ARGS_INCLUDED
#define MMU_ARGS_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

typedef struct mmu_args_t {
    int sample_rate;
    int sample_time;
    bool enable_workload;
} mmu_args_t;

int
mmu_args_construct(mmu_args_t **a);

int
mmu_args_destruct(mmu_args_t **a);

char **
mmu_args_dup_argv(int argc, char **argv);

void
mmu_args_free_dup_argv(char **dup);

int
mmu_args_process_user_input(mmu_args_t *a, int argc, char **argv);

#endif /* ifndef MMU_ARGS_INCLUDED */
