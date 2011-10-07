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

#ifndef MEMORY_USAGE_INCLUDED
#define MEMORY_USAGE_INCLUDED 

typedef struct mmu_mem_usage_container_t {
    /* holds values for each recorded value */
    unsigned long int *mem_vals;
    /* holds meminfo sample values */
    unsigned long int **samples;
    /* holds pre-mpi init meminfo sample values */
    unsigned long int **pre_mpi_init_samples;
    /* holds min sample values */
    unsigned long int *min_sample_values;
    /* holds max sample values */
    unsigned long int *max_sample_values;
    /* holds sample averages */
    double *sample_aves;
    /* holds sample averages */
    double *min_sample_aves;
    /* holds sample averages */
    double *max_sample_aves;
} mmu_mem_usage_container_t;

int
mem_usage_construct(mmu_mem_usage_container_t *containerp);

int
mem_usage_destruct(mmu_mem_usage_container_t *containerp);

#endif /* ifndef MEMORY_USAGE_INCLUDED */
