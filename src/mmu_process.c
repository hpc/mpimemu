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

/**
 * @author Samuel K. Gutierrez - samuel@lanl.gov
 * found a bug? have an idea? please let me know.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmu_constants.h"
#include "mmu_util.h"
#include "mmu_conv_macros.h"
#include "mmu_mpi.h"
#include "mmu_memory.h"
#include "mmu_memory_sample.h"
#include "mmu_process.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STRING_H
#include <stdbool.h>
#endif

#define MMU_TIME_STR_LEN_MAX 32

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_construct(mmu_process_t **p)
{
    struct tm *loc_time = NULL;
    mmu_process_t *tmp = NULL;
    int rc;
    time_t tloc;

    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp = calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (tmp->start_time_buf =
        calloc(MMU_TIME_STR_LEN_MAX, sizeof(char)))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (NULL == (tmp->hostname_buf = calloc(MMU_MPI_MAX_PROCESSOR_NAME,
                                            sizeof(char)))) {
        MMU_OOR_COMPLAIN();
        rc = MMU_FAILURE_OOR;
        goto error;
    }
    if (MMU_SUCCESS != (rc = mmu_mpi_construct(&tmp->mpi))) {
        fprintf(stderr, MMU_ERR_PREFIX"mmu_mpi_construct failure\n");
        goto error;
    }
    if (MMU_SUCCESS != (rc = mmu_memory_construct(&tmp->memory))) {
        fprintf(stderr, MMU_ERR_PREFIX"mmu_memory_construct failure\n");
        goto error;
    }
    /* set my pid */
    tmp->pid = getpid();
    /* set my hostname */
    if (0 != gethostname(tmp->hostname_buf, MMU_MPI_MAX_PROCESSOR_NAME - 1)) {
        int err = errno;
        fprintf(stderr, MMU_ERR_PREFIX"gethostname failure: %d (%s)\n", err,
                strerror(err));
        rc = MMU_FAILURE;
        goto error;
    }
    tmp->hostname_buf[MMU_MPI_MAX_PROCESSOR_NAME - 1] = '\0';
    /* set my start time */
    time(&tloc);
    loc_time = localtime(&tloc);
    strftime(tmp->start_time_buf, MMU_TIME_STR_LEN_MAX - 1, "%Y%m%d-%H%M%S",
             loc_time);
    tmp->start_time_buf[MMU_TIME_STR_LEN_MAX - 1] = '\0';

    *p = tmp;

    return MMU_SUCCESS;

error:
    (void)mmu_process_destruct(&tmp);
    *p = NULL;
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_destruct(mmu_process_t **p)
{
    mmu_process_t *tmp = NULL;

    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    tmp = *p;

    if (NULL != tmp) {
        if (NULL != tmp->hostname_buf) {
            free(tmp->hostname_buf);
            tmp->hostname_buf = NULL;
        }
        if (NULL != tmp->start_time_buf) {
            free(tmp->start_time_buf);
            tmp->start_time_buf = NULL;
        }
        (void)mmu_mpi_destruct(&tmp->mpi);
        (void)mmu_memory_destruct(&tmp->memory);
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_reserve_memory(mmu_process_t *p)
{
    /* this is easy, but add any other memory reservation code here as needed */
    return mmu_memory_reserve_memory(p->memory);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_set_sample_rate(mmu_process_t *p,
                            int samp_rate)
{
    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    return mmu_memory_set_sample_rate(p->memory, samp_rate);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_set_sample_time(mmu_process_t *p,
                            int samp_time)
{
    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    return mmu_memory_set_sample_time(p->memory, samp_time);
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
sample(mmu_process_t *p, mmu_memory_flags_t flags)
{
    int rc = MMU_FAILURE, i = 0;
    bool dowork = (mmu_mpi_with_workload(p->mpi) && mmu_mpi_initialized());
    size_t total_iters = 0;
    useconds_t sleep_utime;

    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    /* get the total number of iterations that we are required to complete */
    if (MMU_SUCCESS != (rc = mmu_memory_get_total_iters(p->memory,
                                                        &total_iters))) {
        fprintf(stderr, MMU_ERR_PREFIX"mmu_memory_get_total_iters failure!\n");
        return rc;
    }

    /* how long do we have to sleep per iteration? */
    sleep_utime = mmu_memory_get_usleep_time_per_iteration(p->memory);

    /* /// start sampling /// */
    for (i = 0; i < total_iters; ++i) {
        /* only add an mpi workload if requested and mpi has been initialized */
        if (dowork) {
            if (MMU_SUCCESS != (rc = mmu_mpi_work(p->mpi))) {
                fprintf(stderr, MMU_ERR_PREFIX"mmu_mpi_work failure!\n");
                return rc;
            }
        }
        if (MMU_SUCCESS != (rc = mmu_memory_sample_memory_usage(p->memory,
                                                                flags))) {
            fprintf(stderr,
                    MMU_ERR_PREFIX" error encountered while collecting "
                    "sample data: %s.\n", mmu_util_rc2str(rc));
            break;
        }
        usleep(sleep_utime);
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_sample_memory_usage_self(mmu_process_t *p)
{
    mmu_memory_flags_t flags;

    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    /* self will never collect node usage, so just set self flag */
    flags = MMU_MEMORY_FLAGS_SAMPLE_SELF;

    /* is this pre or post mpi init? */
    if (!mmu_process_mpi_initialized()) {
        /* if so, set that bit */
        flags |= MMU_MEMORY_FLAGS_SAMPLE_PRE_MPI_INIT;
    }

    return sample(p, flags);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_sample_memory_usage_all(mmu_process_t *p)
{
    mmu_memory_flags_t flags;

    if (NULL == p) return MMU_FAILURE_INVALID_ARG;

    flags = MMU_MEMORY_FLAGS_SAMPLE_SELF;

    /* is this pre or post mpi init? */
    if (!mmu_process_mpi_initialized()) {
        flags |= MMU_MEMORY_FLAGS_SAMPLE_PRE_MPI_INIT;
    }
    /* only the neighborhood delegate will collect node usage */
    if (mmu_process_is_hood_delegate(p)) {
        flags |= MMU_MEMORY_FLAGS_SAMPLE_NODE;
    }
    /* others will accumulate bogus data.  the general idea is to avoid a
     * memory allocation imbalance across delegates and non-delegates. */
    else {
        flags |= MMU_MEMORY_FLAGS_SAMPLE_NODE_BOGUS;
    }

    return sample(p, flags);
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
dump_sample_fn(const char *prefix, const void *item)
{
    size_t elem = 0;

    mmu_memory_sample_t *samp = (mmu_memory_sample_t *)item;

    fprintf(stderr, "%s  | flags: %04x max index: %lu list length: %lu\n",
            prefix, samp->flags, (unsigned long)samp->max_index,
            (unsigned long)samp->list_len);
    for (elem = 0; elem < samp->max_index; ++elem) {
        fprintf(stderr, "%s  %p @ %2d---- %llu\n", prefix, (void *)samp,
                (int)elem,
                samp->elements_in_kb[elem]);
        fflush(stderr);
    }
    fflush(stderr);

}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_sample_dump(const mmu_process_t *p)
{
    size_t samp_type_i = 0;
    char prefix[128];

    (void)memset(prefix, '\0', sizeof(prefix));
    snprintf(prefix, sizeof(prefix) - 1, "%d-%d: ", (int)p->pid, p->mpi->rank);

    fprintf(stderr, "%s%s%lu\n", prefix,
            "# sample types: ", (unsigned long)p->memory->num_sample_types);
    fflush(stderr);

    for (samp_type_i = 0;
         samp_type_i < p->memory->num_sample_types;
         ++samp_type_i) {
        (void)mmu_list_dump(prefix, p->memory->sample_list[samp_type_i],
                            dump_sample_fn);
    }


    fflush(stderr);

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
const char *
mmu_process_get_mpi_version_str(const mmu_process_t *p)
{
    if (NULL == p) return NULL;

    return mmu_mpi_get_version_str(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_init_mpi(mmu_process_t *p,
                     int argc,
                     char **argv)
{
    if (NULL == p || NULL == argv) return MMU_FAILURE_INVALID_ARG;

    return mmu_mpi_init(p->mpi, argc, argv);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_finalize_mpi(mmu_process_t *p)
{
    return mmu_mpi_finalize(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_process_mpi_initialized(void)
{
    return mmu_mpi_initialized();
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_process_is_delegate(const mmu_process_t *p)
{
    return  mmu_mpi_world_rank_zero(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_process_is_hood_delegate(const mmu_process_t *p)
{
    return mmu_mpi_smp_rank_zero(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_get_num_hoods(const mmu_process_t *p)
{
    return mmu_mpi_get_num_nodes(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_get_hood_size(const mmu_process_t *p)
{
    return mmu_mpi_get_num_smp_ranks(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_get_world_size(const mmu_process_t *p)
{
    return mmu_mpi_get_comm_world_size(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_process_with_workload(const mmu_process_t *p)
{
    return mmu_mpi_with_workload(p->mpi);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_process_enable_workload(mmu_process_t *p,
                            bool enable)
{
    return mmu_mpi_enable_workload(p->mpi, enable);
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
get_global_mmas(const mmu_process_t *p,
                MPI_Comm communicator,
                mmu_memory_sample_mma_t **in_mma_ptr_list,
                mmu_memory_sample_mma_t ***new_global_mma_ptr_list)
{
    int rc = MMU_SUCCESS, comm_size = 0;
    size_t i = 0, j = 0;
    mmu_memory_sample_mma_t **global_mma_ptr_list = NULL;

    if (NULL == p || NULL == in_mma_ptr_list||
        NULL == new_global_mma_ptr_list) {
        return MMU_FAILURE_INVALID_ARG;
    }

    if (MMU_SUCCESS != (rc =
        mmu_memory_mma_ptr_list_construct(p->memory->num_sample_types,
                                          &global_mma_ptr_list))) {
        fprintf(stderr, MMU_ERR_PREFIX
                "mmu_memory_mma_ptr_list_construct failure\n");
        return rc;
    }
    /* allocate memory for each sample type */
    for (i = 0; i < p->memory->num_sample_types; ++i) {
        if (MMU_SUCCESS != (rc =
            mmu_memory_sample_mma_construct(in_mma_ptr_list[i]->lists_length,
                                            &(global_mma_ptr_list[i])))) {
            fprintf(stderr, MMU_ERR_PREFIX
                    "mmu_memory_sample_mma_construct failure\n");
            goto out;
        }
    }

    if (MMU_SUCCESS != (rc = mmu_mpi_comm_size(communicator, &comm_size))) {
        return rc;
    }

    /* allreduce instead of reduce here, again, to maintain some sort
     * of memory usage balance across all processes as best we can.*/
    for (i = 0; i < p->memory->num_sample_types; ++i) {
        /* --- global mins --- */
        if (MMU_SUCCESS != (rc =
            mmu_mpi_allreduce(in_mma_ptr_list[i]->mins,
                              global_mma_ptr_list[i]->mins,
                              in_mma_ptr_list[i]->lists_length,
                              MPI_UNSIGNED_LONG_LONG,
                              MPI_MIN, communicator))) {
            goto out;
        }
        /* --- global maxes --- */
        if (MMU_SUCCESS != (rc =
            mmu_mpi_allreduce(in_mma_ptr_list[i]->maxes,
                              global_mma_ptr_list[i]->maxes,
                              in_mma_ptr_list[i]->lists_length,
                              MPI_UNSIGNED_LONG_LONG,
                              MPI_MAX, communicator))) {
            goto out;
        }
        /* --- global average total --- */
        /* note that global_mmas[i]->totals will NOT be populated here because
         * we already have the node averages.  so, just sum up the averages
         * here. */
        if (MMU_SUCCESS != (rc =
            mmu_mpi_allreduce(in_mma_ptr_list[i]->aves,
                              global_mma_ptr_list[i]->aves,
                              in_mma_ptr_list[i]->lists_length,
                              MPI_DOUBLE, MPI_SUM,
                              communicator))) {
            goto out;
        }
        /* --- global averages --- */
        /* XXX add proper float equality check here */
        for (j = 0; j < in_mma_ptr_list[i]->lists_length; ++j) {
            global_mma_ptr_list[i]->aves[j] =
            (0.0 == global_mma_ptr_list[i]->aves[j]) ? 0.0 :
            (double)global_mma_ptr_list[i]->aves[j] / (double)comm_size;
        }
    }
    /* caller is responsible for freeing returned resources */
    *new_global_mma_ptr_list = global_mma_ptr_list;

out:
    if (MMU_SUCCESS != rc) {
        (void)mmu_memory_mma_ptr_list_destruct(&global_mma_ptr_list);
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* we have to be careful with this one because some of the data that we are
 * working on is bogus. specifically, exactly one node has collected real node
 * data within the smp communicator, so we can't safely do an MPI_MIN allreduce,
 * for example, and expect to get the correct answer. */
static int
get_node_mmas(const mmu_process_t *p,
                MPI_Comm smp_comm,
                mmu_memory_sample_mma_t **in_mma_ptr_list,
                mmu_memory_sample_mma_t ***new_node_mma_ptr_list)
{
    int rc = MMU_SUCCESS, comm_size = 0;
    size_t i = 0, j = 0;
    mmu_memory_sample_mma_t **node_mma_ptr_list = NULL;

    if (NULL == p || NULL == in_mma_ptr_list ||
        NULL == new_node_mma_ptr_list) {
        return MMU_FAILURE_INVALID_ARG;
    }

    if (MMU_SUCCESS != (rc =
        mmu_memory_mma_ptr_list_construct(p->memory->num_sample_types,
                                          &node_mma_ptr_list))) {
        fprintf(stderr, MMU_ERR_PREFIX
                "mmu_memory_mma_ptr_list_construct failure\n");
        return rc;
    }

    /* allocate memory for each sample type */
    for (i = 0; i < p->memory->num_sample_types; ++i) {
        if (MMU_SUCCESS != (rc =
            mmu_memory_sample_mma_construct(in_mma_ptr_list[i]->lists_length,
                                            &(node_mma_ptr_list[i])))) {
            fprintf(stderr, MMU_ERR_PREFIX
                    "mmu_memory_sample_mma_construct failure\n");
            goto out;
        }
    }

    /* get the number of processes on the node */
    if (MMU_SUCCESS != (rc = mmu_mpi_comm_size(smp_comm, &comm_size))) {
        return rc;
    }

    /* allreduce instead of reduce here, again, to maintain some sort
     * of memory usage balance across all processes as best we can.*/
    for (i = 0; i < p->memory->num_sample_types; ++i) {
        /* stash the comm_size, because it may be changed at some point */
        int tmp_comm_size = comm_size;
        /* opset[0] for global mins
         * opset[1] for global maxes */
        MPI_Op opset[2] = {MPI_MIN, MPI_MAX};

        /* if we are dealing with post node init samples, then update the opset
         * to request the max of the set. the idea here is that all nodes except
         * the one responsible for collecting node memory data will have values
         * that all all >= 0. so, requesting the max is the same as getting only
         * those vaules. if the values we collect ever have negative values,
         * then this approach MUST be updated. */
        if (MMU_MEMORY_NODE_POST_INIT == i) {
            opset[0] = MPI_MAX;
        }
        /* --- global mins --- */
        if (MMU_SUCCESS != (rc =
            mmu_mpi_allreduce(in_mma_ptr_list[i]->mins,
                              node_mma_ptr_list[i]->mins,
                              in_mma_ptr_list[i]->lists_length,
                              MPI_UNSIGNED_LONG_LONG,
                              opset[0], smp_comm))) {
            goto out;
        }
        /* --- global maxes --- */
        if (MMU_SUCCESS != (rc =
            mmu_mpi_allreduce(in_mma_ptr_list[i]->maxes,
                              node_mma_ptr_list[i]->maxes,
                              in_mma_ptr_list[i]->lists_length,
                              MPI_UNSIGNED_LONG_LONG,
                              opset[1], smp_comm))) {
            goto out;
        }
        /* --- global average total --- */
        /* note that node_mmas[i]->totals will NOT be populated here because
         * we already have the local averages.  so, just sum up the averages
         * here. */
        if (MMU_SUCCESS != (rc =
            mmu_mpi_allreduce(in_mma_ptr_list[i]->aves,
                              node_mma_ptr_list[i]->aves,
                              in_mma_ptr_list[i]->lists_length,
                              MPI_DOUBLE, MPI_SUM,
                              smp_comm))) {
            goto out;
        }
        /* --- global averages --- */
        /* do we need to update the divisor? the general idea here is that some
         * processes accumulated bogus data during the run and we don't want to
         * take their info into account. there will always be exactly one
         * collector for this type of data. */
        if (MMU_MEMORY_NODE_POST_INIT == i) {
            tmp_comm_size = 1;
        }
        /* XXX add proper float equality check here */
        for (j = 0; j < in_mma_ptr_list[i]->lists_length; ++j) {
            node_mma_ptr_list[i]->aves[j] =
            (0.0 == node_mma_ptr_list[i]->aves[j]) ? 0.0 :
            (double)node_mma_ptr_list[i]->aves[j] / (double)tmp_comm_size;
        }
    }
    /* caller is responsible for freeing returned resources */
    *new_node_mma_ptr_list = node_mma_ptr_list;

out:
    if (MMU_SUCCESS != rc) {
        (void)mmu_memory_mma_ptr_list_destruct(&node_mma_ptr_list);
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static char *
get_type_prefix_str(mmu_memory_list_type_t type)
{
    static char *nada_str = "";
    static char *pre_init_str = "Pre-Init ";
    static char *post_init_str = "Post-Init ";

    switch (type) {
        case (MMU_MEMORY_SELF_PRE_INIT):
            return pre_init_str;
        case (MMU_MEMORY_SELF_POST_INIT):
        case (MMU_MEMORY_NODE_POST_INIT):
            return post_init_str;
        default:
            return nada_str;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
/* too_too_too_many processes */
int
mmu_process_process_usage(const mmu_process_t *p)
{
    int rc = MMU_SUCCESS;
    mmu_memory_sample_mma_t **local_mmas = NULL;
    mmu_memory_sample_mma_t **node_mmas = NULL;
    mmu_memory_sample_mma_t **global_mmas = NULL;

    /* calculate my local min, max, and average values for all collection
     * types. */
    if (MMU_SUCCESS != (rc =
        mmu_memory_get_local_mmas(p->memory, &local_mmas))) {
        fprintf(stderr, MMU_ERR_PREFIX"mmu_memory_get_local_mmas failure\n");
        goto out;
    }

    /* at this point everyone has their own min, max, totals, and averages
     * calculated. let's first calculate node averages, then true global
     * averages. */

    /* next calculate NODE min, max, and averages for all collection types */
    if (MMU_SUCCESS != (rc =
        get_node_mmas(p, p->mpi->smp_comm, local_mmas, &node_mmas))) {
        fprintf(stderr, MMU_ERR_PREFIX"get_global_mmas failure\n");
        goto out;
    }

    /* calculate global min, max, and average values for all collection types.
     */
    if (MMU_SUCCESS != (rc =
        get_global_mmas(p, MPI_COMM_WORLD, node_mmas, &global_mmas))) {
        fprintf(stderr, MMU_ERR_PREFIX"get_global_mmas failure\n");
        goto out;
    }

    /* at this point we have all the data required to output our result. */

    if (mmu_mpi_world_rank_zero(p->mpi)) {
        fprintf(stdout, "### item name, min (kB), max (kB), average (kB)\n");
        int samp_type_index = 0;
        int samp_item_index = 0;
        for (samp_type_index = 0;
             samp_type_index < p->memory->num_sample_types;
             ++samp_type_index) {
            for (samp_item_index = 0;
                 samp_item_index < global_mmas[samp_type_index]->lists_length;
                 ++samp_item_index) {

                printf("%s%s, %llu, %llu, %0.2lf\n",
                       get_type_prefix_str(samp_type_index),
                       mmu_memory_get_item_label(samp_type_index,
                                                 samp_item_index),
                       global_mmas[samp_type_index]->mins[samp_item_index],
                       global_mmas[samp_type_index]->maxes[samp_item_index],
                       global_mmas[samp_type_index]->aves[samp_item_index]);
                fflush(stdout);
            }
            fflush(stdout);
        }
    }

out:
    (void)mmu_memory_mma_ptr_list_destruct(&local_mmas);
    (void)mmu_memory_mma_ptr_list_destruct(&node_mmas);
    (void)mmu_memory_mma_ptr_list_destruct(&global_mmas);
    return rc;
}
