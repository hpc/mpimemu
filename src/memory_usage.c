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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "constants.h"
#include "util.h"
#include "conv_macros.h"
#include "memory_usage.h"

/* ////////////////////////////////////////////////////////////////////////// */
int
node_mem_usage_construct(mmu_node_mem_usage_container_t *containerp)
{
    int i;

    if (NULL == containerp) {
        return MMU_FAILURE_INVALID_ARG;
    }
    if (NULL == (containerp->mem_vals = lucalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (containerp->min_sample_values = lucalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (containerp->max_sample_values = lucalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (containerp->sample_aves = lfcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (containerp->min_sample_aves = lfcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (containerp->max_sample_aves = lfcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (containerp->samples = lupcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    for (i = 0; i < MMU_MEM_INFO_LEN; ++i) {
        if (NULL == (containerp->samples[i] = lucalloc(MMU_NUM_SAMPLES))) {
            MMU_OOR_COMPLAIN();
            return MMU_FAILURE_OOR;
        }
    }
    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
node_mem_usage_destruct(mmu_node_mem_usage_container_t *containerp)
{
    int i;

    if (NULL == containerp) {
        return MMU_FAILURE_INVALID_ARG;
    }
    if (NULL != containerp->mem_vals) {
        free(containerp->mem_vals);
    }
    if (NULL != containerp->min_sample_values) {
        free(containerp->min_sample_values);
    }
    if (NULL != containerp->max_sample_values) {
        free(containerp->max_sample_values);
    }
    if (NULL != containerp->sample_aves) {
        free(containerp->sample_aves);
    }
    if (NULL != containerp->min_sample_aves) {
        free(containerp->min_sample_aves);
    }
    if (NULL != containerp->max_sample_aves) {
        free(containerp->max_sample_aves);
    }
    if (NULL != containerp->samples) {
        for (i = 0; i < MMU_MEM_INFO_LEN; ++i) {
            if (NULL != containerp->samples[i]) {
                free(containerp->samples[i]);
            }
        }
        free(containerp->samples);
    }
    return MMU_SUCCESS;
}

#if 0
/* ////////////////////////////////////////////////////////////////////////// */
static int
fini(void)
{
    int i;

    /* proc */
    free(proc_mem_vals);
    free(proc_min_sample_values);
    free(proc_max_sample_values);
    free(proc_sample_aves);
    free(proc_min_sample_aves);
    free(proc_max_sample_aves);
    for (i = 0; i < MMU_NUM_STATUS_VARS; ++i) {
        free(proc_samples[i]);
        free(pre_mpi_init_proc_samples[i]);
    }
    free(proc_samples);
    free(pre_mpi_init_proc_samples);
    return MMU_SUCCESS;
}
#endif
