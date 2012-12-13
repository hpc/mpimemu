/**
 * Copyright (c) 2010-2011 Los Alamos National Security, LLC.
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
