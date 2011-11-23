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

#include <stdlib.h>

#include "constants.h"
#include "mmu_util.h"
#include "conv_macros.h"
#include "memory_usage.h"

/* ////////////////////////////////////////////////////////////////////////// */
/* TODO pass lengths */
int
mem_usage_construct(mmu_mem_usage_container_t **containerp)
{
    int i, rc;
    mmu_mem_usage_container_t *tmp = NULL;

    if (NULL == containerp) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp = calloc(1, sizeof(mmu_mem_usage_container_t)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    /* now let's allocate some memory for the struct members */
    if (NULL == (tmp->mem_vals = lucalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->min_sample_values = lucalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->max_sample_values = lucalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->sample_aves = lfcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->min_sample_aves = lfcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->max_sample_aves = lfcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->samples = lupcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->pre_mpi_init_samples = lupcalloc(MMU_MEM_INFO_LEN))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    for (i = 0; i < MMU_MEM_INFO_LEN; ++i) {
        if (NULL == (tmp->samples[i] = lucalloc(MMU_NUM_SAMPLES))) {
            MMU_OOR_COMPLAIN();
            rc = MMU_FAILURE_OOR;
            goto error;
        }
        if (NULL == (tmp->pre_mpi_init_samples[i] =
                         lucalloc(MMU_NUM_SAMPLES))) {
            MMU_OOR_COMPLAIN();
            rc = MMU_FAILURE_OOR;
            goto error;
        }
    }
    *containerp = tmp;
    return MMU_SUCCESS;

error:
    mem_usage_destruct(&tmp);
    *containerp = NULL;
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mem_usage_destruct(mmu_mem_usage_container_t **containerp)
{
    int i;
    mmu_mem_usage_container_t *tmp = NULL;

    if (NULL == containerp) return MMU_FAILURE_INVALID_ARG;

    tmp = *containerp;

    if (NULL != tmp->mem_vals) {
        free(tmp->mem_vals);
        tmp->mem_vals = NULL;
    }
    if (NULL != tmp->min_sample_values) {
        free(tmp->min_sample_values);
        tmp->min_sample_values = NULL;
    }
    if (NULL != tmp->max_sample_values) {
        free(tmp->max_sample_values);
        tmp->max_sample_values = NULL;
    }
    if (NULL != tmp->sample_aves) {
        free(tmp->sample_aves);
        tmp->sample_aves = NULL;
    }
    if (NULL != tmp->min_sample_aves) {
        free(tmp->min_sample_aves);
        tmp->min_sample_aves = NULL;
    }
    if (NULL != tmp->max_sample_aves) {
        free(tmp->max_sample_aves);
        tmp->max_sample_aves = NULL;
    }
    if (NULL != tmp->samples) {
        /* TODO we should be able to figure out the length */
        for (i = 0; i < MMU_MEM_INFO_LEN; ++i) {
            if (NULL != tmp->samples[i]) {
                free(tmp->samples[i]);
                tmp->samples[i] = NULL;
            }
            if (NULL != tmp->pre_mpi_init_samples[i]) {
                free(tmp->pre_mpi_init_samples[i]);
                tmp->pre_mpi_init_samples[i] = NULL;
            }
        }
        free(tmp->samples);
        tmp->samples = NULL;
    }
    free(tmp);
    *containerp = NULL;
    return MMU_SUCCESS;
}
