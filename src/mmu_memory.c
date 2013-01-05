/**
 * Copyright (c) 2010-2013 Los Alamos National Security, LLC.
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

#include "mmu_constants.h"
#include "mmu_conv_macros.h"
#include "mmu_util.h"
#include "mmu_list.h"
#include "mmu_memory.h"
#include "mmu_memory_sample.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/* max key length */
#define MMU_MEMORY_KEY_LEN_MAX 64

enum {
    VMPEAK = 0,
    VMSIZE,
    VMLCK,
    VMHWM,
    VMRSS,
    VMDATA,
    VMSTK,
    VMEXE,
    VMLIB,
    VMPTE
} mmu_memory_proc_self_index_tab;

static char * mmu_memory_proc_self_tab[MMU_MEMORY_PROC_SELF_TAB_LEN + 1] = {
    "VmPeak",
    "VmSize",
    "VmLck",
    "VmHWM",
    "VmRSS",
    "VmData",
    "VmStk",
    "VmExe",
    "VmLib",
    "VmPTE",
    NULL
};

typedef enum {
    MEM_TOTAL = 0,
    MEM_FREE,
    MEM_USED,
    BUFFERS,
    CACHED,
    SWAP_CACHED,
    ACTIVE,
    INACTIVE,
    SWAP_TOTAL,
    SWAP_FREE,
    DIRTY,
    COMMIT_LIMIT,
    COMMITTED_AS
} mmu_memory_proc_node_index_tab;

static char * mmu_memory_proc_node_tab[MMU_MEMORY_PROC_NODE_TAB_LEN + 1] = {
    "MemTotal",
    "MemFree",
    "MemUsed",
    "Buffers",
    "Cached",
    "SwapCached",
    "Active",
    "Inactive",
    "SwapTotal",
    "SwapFree",
    "Dirty",
    "CommitLimit",
    "Committed_AS",
    NULL
};

/* ////////////////////////////////////////////////////////////////////////// */
static inline bool
is_valid_key(const char *key,
             mmu_memory_flags_t flags,
             int *tab_index_if_valid)
{
    int i;
    char **target_tab_ptr = NULL;

    *tab_index_if_valid = 0;

    if (MMU_MEMORY_SAMPLE_SELF(flags)) {
        target_tab_ptr = mmu_memory_proc_self_tab;
    }
    else if (MMU_MEMORY_SAMPLE_NODE(flags)) {
        target_tab_ptr = mmu_memory_proc_node_tab;
    }
    else {
        fprintf(stderr,
                MMU_ERR_PREFIX
                "invalid mmu memory sample type. flags: 0x%04x.\n", flags);
        return false;
    }
    for (i = 0; NULL != target_tab_ptr[i]; ++i) {
        if (0 == strcmp(key, target_tab_ptr[i])) {
            *tab_index_if_valid = i;
            return true;
        }
    }
    /* didn't find it */
    return false;
}

/* ////////////////////////////////////////////////////////////////////////// */
useconds_t
mmu_memory_get_usleep_time_per_iteration(const mmu_memory_t *m)
{
    int sample_rate = m->sample_rate;
    double tmp_utime = (double)1e6/(double)sample_rate;
    return (useconds_t)tmp_utime;
}

#if 0
/* ////////////////////////////////////////////////////////////////////////// */
static inline void
memory_self_dump(const mmu_memory_sample_t *sample)
{
    fprintf(stderr, "memory_self_dump:\n"
                    "\tVmPeak: %llu\n"
                    "\tVmSize: %llu\n"
                    "\tVmLck:  %llu\n"
                    "\tVmHWM:  %llu\n"
                    "\tVmRSS:  %llu\n"
                    "\tVmData: %llu\n"
                    "\tVmStk:  %llu\n"
                    "\tVmExe:  %llu\n"
                    "\tVmLib:  %llu\n"
                    "\tVmPTE:  %llu\n",
                    sample->elements_in_kb[VMPEAK],
                    sample->elements_in_kb[VMSIZE],
                    sample->elements_in_kb[VMLCK],
                    sample->elements_in_kb[VMHWM],
                    sample->elements_in_kb[VMRSS],
                    sample->elements_in_kb[VMDATA],
                    sample->elements_in_kb[VMSTK],
                    sample->elements_in_kb[VMEXE],
                    sample->elements_in_kb[VMLIB],
                    sample->elements_in_kb[VMPTE]);
    fflush(stderr);
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline void
memory_node_dump(const mmu_memory_sample_t *sample)
{
    fprintf(stderr, "memory_node_dump:\n"
                    "\tMemTotal:     %llu\n"
                    "\tMemFree:      %llu\n"
                    "\tMemUsed:      %llu\n"
                    "\tBuffers:      %llu\n"
                    "\tCached:       %llu\n"
                    "\tSwapCached:   %llu\n"
                    "\tActive:       %llu\n"
                    "\tInactive:     %llu\n"
                    "\tSwapTotal:    %llu\n"
                    "\tSwapFree:     %llu\n"
                    "\tDirty:        %llu\n"
                    "\tCommitLimit:  %llu\n"
                    "\tCommitted_AS: %llu\n",
                    sample->elements_in_kb[MEM_TOTAL],
                    sample->elements_in_kb[MEM_FREE],
                    sample->elements_in_kb[MEM_USED],
                    sample->elements_in_kb[BUFFERS],
                    sample->elements_in_kb[CACHED],
                    sample->elements_in_kb[SWAP_CACHED],
                    sample->elements_in_kb[ACTIVE],
                    sample->elements_in_kb[INACTIVE],
                    sample->elements_in_kb[SWAP_TOTAL],
                    sample->elements_in_kb[SWAP_FREE],
                    sample->elements_in_kb[DIRTY],
                    sample->elements_in_kb[COMMIT_LIMIT],
                    sample->elements_in_kb[COMMITTED_AS]);
    fflush(stderr);
}
#endif

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
update_memory_data(mmu_memory_sample_t *sample,
                   int target_index,
                   unsigned long long int value)
{
    if (NULL == sample) return MMU_FAILURE_INVALID_ARG;

    if (target_index > sample->max_index) {
        fprintf(stderr, MMU_ERR_PREFIX"invalid target index.\n");
        return MMU_FAILURE_INVALID_ARG;
    }
    sample->elements_in_kb[target_index] = value;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
post_process_sample(mmu_memory_sample_t *sample)
{
    if (NULL == sample) return MMU_FAILURE_INVALID_ARG;
    /* calculate memory used */
    if (MMU_MEMORY_SAMPLE_NODE(sample->flags)) {
        sample->elements_in_kb[MEM_USED] =
            sample->elements_in_kb[MEM_TOTAL] - sample->elements_in_kb[MEM_FREE];
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
/*
expected /proc/self/status output (snippet)

VmPeak:    14792 kB
VmSize:    14792 kB
VmLck:         0 kB
VmHWM:      3024 kB
VmRSS:      3024 kB
VmData:     2308 kB
VmStk:        84 kB
VmExe:       672 kB
VmLib:      1940 kB
VmPTE:        40 kB

expected /proc/meminfo output (snippet)

MemTotal:     32830436 kB
MemFree:       5418976 kB
Buffers:        255740 kB
Cached:       24029444 kB
SwapCached:         12 kB
Active:        9515740 kB
Inactive:     16483212 kB
HighTotal:           0 kB
HighFree:            0 kB
LowTotal:     32830436 kB
LowFree:       5418976 kB
SwapTotal:     409mit:  20511748 kB
Committed_AS:  3989268 kB

*/
static inline int
get_sample(mmu_memory_t *mem,
           mmu_memory_flags_t flags)
{
    int rc = MMU_SUCCESS;
    char *info_path = NULL;
    char current_line_buf[256];
    char current_key_buf[MMU_MEMORY_KEY_LEN_MAX];
    int key_index = 0;
    FILE *fp = NULL;
    mmu_list_t *target_list_ptr = NULL;
    mmu_memory_sample_t *sample = NULL;

    if (NULL == mem) return MMU_FAILURE_INVALID_ARG;

    (void)memset(current_line_buf, '\0', sizeof(current_line_buf));

    /* construct sample */
    if (MMU_SUCCESS != (rc = mmu_memory_sample_construct(&sample, flags))) {
        fprintf(stderr, MMU_ERR_PREFIX"mmu_memory_sample_construct failure.\n");
        goto out;
    }
    /* figure out which file to grab the info from */
    if (MMU_MEMORY_SAMPLE_SELF(flags)) {
        info_path = "/proc/self/status";
        if (MMU_MEMORY_SAMPLE_PRE_MPI_INIT(flags)) {
            target_list_ptr = mem->self_pre_mpi_init_samples;
        }
        else {
            target_list_ptr = mem->self_post_mpi_init_samples;
        }
    }
    else if (MMU_MEMORY_SAMPLE_NODE(flags) ||
             MMU_MEMORY_SAMPLE_NODE_BOGUS(flags)) {
        info_path = "/proc/meminfo";
        if (MMU_MEMORY_SAMPLE_PRE_MPI_INIT(flags)) {
            target_list_ptr = mem->node_pre_mpi_init_samples;
        }
        else {
            target_list_ptr = mem->node_post_mpi_init_samples;
        }
    }
    else {
        fprintf(stderr, MMU_ERR_PREFIX"unknown option. cannot continue.\n");
        rc = MMU_FAILURE_INVALID_ARG;
        goto out;
    }

    /* if we are just a bogus request, just add the item */
    if (MMU_MEMORY_SAMPLE_NODE_BOGUS(flags)) {
        goto add_item;
    }

    /* open the target file */
    if (NULL == (fp = fopen(info_path, "r"))) {
        int err = errno;
        fprintf(stderr, MMU_ERR_PREFIX"cannot open %s: %d (%s)\n", info_path,
                err, strerror(err));
        rc = MMU_FAILURE_IO;
        goto out;
    }
    /* iterate over it one line at a time */
    while (NULL != fgets(current_line_buf, sizeof(current_line_buf) - 1, fp)) {
        /* start at one to reserve space for '\0' */
        size_t key_len = 1;
        char *hptr = current_line_buf;
        char *tptr = NULL;

        (void)memset(current_key_buf, '\0', sizeof(current_key_buf));
        /* eat leading whitespace - may not be needed, but doesn't hurt */
        while ('\0' != *hptr && isspace(*hptr)) {
            ++hptr;
        }
        tptr = hptr;
        /* move the head pointer passed the key */
        while ('\0' != *hptr && ':' != *hptr) {
            ++hptr;
            ++key_len;
        }
        /* sanity */
        if (':' != *hptr) {
            fprintf(stderr,
                    MMU_ERR_PREFIX"cannot parse %s\nbuffer dump:\n%s\n",
                    info_path, current_line_buf);
            rc = MMU_FAILURE_IO;
            goto out;
        }
        /* make sure that we have enough buffer space for the key */
        if (key_len > sizeof(current_key_buf)) {
            fprintf(stderr,
                    MMU_ERR_PREFIX"key buffer too small.  "
                    "cannot parse %s\nbuffer dump:\n%s\n",
                    info_path, current_line_buf);
            rc = MMU_FAILURE_IO;
            goto out;
        }
        /* fill in the buffer with the key value */
        snprintf(current_key_buf, key_len, "%s", tptr);
        /* is this a key that we care about? */
        if (is_valid_key(current_key_buf, flags, &key_index)) {
            unsigned long long int current_value = 0;
            /* eat whitespace and move passed the ':' */
            while (('\0' != *hptr && isspace(*hptr)) || ':' == *hptr) {
                ++hptr;
            }
            tptr = hptr;
            /* sanity */
            if (!isdigit(*hptr)) {
                fprintf(stderr,
                        MMU_ERR_PREFIX"cannot parse %s\nbuffer dump:\n%s\n",
                        info_path, current_line_buf);
                rc = MMU_FAILURE_IO;
                goto out;
            }
            /* cap before we process because
             * mmu_util_str_to_ull only wants digits */
            while ('\0' != hptr && isdigit(*hptr)) {
                ++hptr;
            }
            *hptr = '\0';
            /* extract the value */
            if (MMU_SUCCESS != (rc = mmu_util_str_to_ull(tptr,
                                                         &current_value))) {
                fprintf(stderr, MMU_ERR_PREFIX"mmu_util_str_to_ull failure.\n");
                goto out;
            }
            /* update sample with current_value */
            if (MMU_SUCCESS != (rc = update_memory_data(sample, key_index,
                                                        current_value))) {
                fprintf(stderr,
                        MMU_ERR_PREFIX"update_memory_data failure.\n");
                goto out;
            }
        }
        /* not a key that we care about, so just continue */
        else {
            continue;
        }
    }
    /* complete any post processing that the sample may need */
    if (MMU_SUCCESS != (rc = post_process_sample(sample))) {
        fprintf(stderr, MMU_ERR_PREFIX"post_process_sample failure.\n");
        goto out;
    }
    /* done! our sample is now fully populated, so add what we have to the list
     */
add_item:
    if (MMU_SUCCESS != (rc = mmu_list_append(target_list_ptr,
                                             sample, sizeof(*sample)))) {
        fprintf(stderr, MMU_ERR_PREFIX"mmu_list_append failure.\n");
        goto out;
    }

out:
    /* the payload has been copied and added to the list at this point, so just
     * free the sample */
    (void)mmu_memory_sample_destruct(&sample);
    if (NULL != fp) fclose(fp);

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
get_self_sample(mmu_memory_t *m,
                mmu_memory_flags_t flags)
{
    mmu_memory_flags_t mod_flags = flags;

    /* turn off any non-self flags before calling get_sample */
    mod_flags &= MMU_MEMORY_TYPE_SELF_MASK;
    /* resore any stage flags that may have been masked away */
    mod_flags |= (MMU_MEMORY_STAGE_MASK & flags);

    return get_sample(m, mod_flags);
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
get_node_sample(mmu_memory_t *m,
                mmu_memory_flags_t flags)
{
    mmu_memory_flags_t mod_flags = flags;

    /* turn off any non-node flags before calling get_sample */
    mod_flags &= MMU_MEMORY_TYPE_NODE_MASK;
    /* resore any stage flags that may have been masked away */
    mod_flags |= (MMU_MEMORY_STAGE_MASK & flags);

    return get_sample(m, mod_flags);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_mma_ptr_list_construct(size_t list_len,
                                  mmu_memory_sample_mma_t ***new_ptr_list)
{
    mmu_memory_sample_mma_t **new = NULL;

    if (NULL == new_ptr_list) return MMU_FAILURE_INVALID_ARG;

    *new_ptr_list = NULL;
    /* +1 for NULL cap -- so we know when to stop traversing the pointer list */
    if (NULL == (new = calloc(list_len + 1, sizeof(*new)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }

    *new_ptr_list = new;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_mma_ptr_list_destruct(mmu_memory_sample_mma_t ***ptr_list)
{
    mmu_memory_sample_mma_t **tmp = NULL;

    if (NULL == ptr_list) return MMU_FAILURE_INVALID_ARG;

    tmp = *ptr_list;
    if (NULL != tmp) {
        int i;
        for (i = 0; NULL != tmp[i]; ++i) {
            free(tmp[i]);
            tmp[i] = NULL;
        }
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_get_local_mmas(mmu_memory_t *m,
                          mmu_memory_sample_mma_t ***new_mma_ptr_list)
{
    size_t samp_type;
    int rc;
    mmu_memory_sample_mma_t **loc_mma_ptr_list = NULL;

    if (NULL == m || NULL == new_mma_ptr_list) return MMU_FAILURE_INVALID_ARG;

    if (MMU_SUCCESS != (rc =
        mmu_memory_mma_ptr_list_construct(m->num_sample_types,
                                          &loc_mma_ptr_list))) {
        fprintf(stderr, MMU_ERR_PREFIX
                "mmu_memory_mma_ptr_list_construct failure\n");
        return rc;
    }

    /* iterate over all sample lists and calculate the respective mmas. this
     * process also constructs each respective mmu_memory_sample_mma_t and
     * points it to loc_mma_ptr_list[samp_type] */
    for (samp_type = 0; samp_type < m->num_sample_types; ++samp_type) {
        if (MMU_SUCCESS != (rc =
            mmu_memory_sample_get_mma(m->sample_list[samp_type],
                                      &(loc_mma_ptr_list[samp_type])))) {
            fprintf(stderr, MMU_ERR_PREFIX
                    "mmu_memory_sample_get_mma failure\n");
            goto out;
        }
    }
    /* caller is responsible for freeing returned resources */
    *new_mma_ptr_list = loc_mma_ptr_list;

out:
    if (MMU_SUCCESS != rc) {
        (void)mmu_memory_mma_ptr_list_destruct(&loc_mma_ptr_list);
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_construct(mmu_memory_t **m)
{
    int rc = MMU_FAILURE;
    size_t num_samp_types = 0;
    mmu_memory_t *tmp = NULL;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp = calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (NULL == (tmp->sample_list =
        calloc(MMU_MEMORY_LAST, sizeof(*(tmp->sample_list))))) {
        MMU_OOR_COMPLAIN();
        goto out;
    }
    /* now go through and construct all the sample types */
    if (MMU_SUCCESS != (rc =
        mmu_list_construct(&tmp->self_pre_mpi_init_samples))) {
        /* rc already set */
        goto out;
    }
    else {
        tmp->sample_list[MMU_MEMORY_SELF_PRE_INIT] =
            tmp->self_pre_mpi_init_samples;
        ++num_samp_types;
    }
    if (MMU_SUCCESS != (rc =
        mmu_list_construct(&tmp->node_pre_mpi_init_samples))) {
        /* rc already set */
        goto out;
    }
    else {
        tmp->sample_list[MMU_MEMORY_NODE_PRE_INIT] =
            tmp->node_pre_mpi_init_samples;
        ++num_samp_types;
    }
    if (MMU_SUCCESS != (rc =
        mmu_list_construct(&tmp->self_post_mpi_init_samples))) {
        /* rc already set */
        goto out;
    }
    else {
        tmp->sample_list[MMU_MEMORY_SELF_POST_INIT] =
            tmp->self_post_mpi_init_samples;
        ++num_samp_types;
    }
    if (MMU_SUCCESS != (rc =
        mmu_list_construct(&tmp->node_post_mpi_init_samples))) {
        /* rc already set */
        goto out;
    }
    else {
        tmp->sample_list[MMU_MEMORY_NODE_POST_INIT] =
            tmp->node_post_mpi_init_samples;
        ++num_samp_types;
    }

    /* this is the number of sample types that we have. for example, self pre
     * mpi init, self post mpi init, and node post mpi init */
    tmp->num_sample_types = num_samp_types;

    /* sanity */
    if (MMU_MEMORY_LAST != num_samp_types) {
        rc = MMU_FAILURE_INVALID_ARG;
        goto out;
    }

    *m = tmp;

out:
    if (MMU_SUCCESS != rc) mmu_memory_destruct(&tmp);

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_destruct(mmu_memory_t **m)
{
    mmu_memory_t *tmp = NULL;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    tmp = *m;

    if (NULL != tmp) {
        mmu_list_destruct(&tmp->self_pre_mpi_init_samples);
        mmu_list_destruct(&tmp->node_pre_mpi_init_samples);
        mmu_list_destruct(&tmp->self_post_mpi_init_samples);
        mmu_list_destruct(&tmp->node_post_mpi_init_samples);
        if (NULL != tmp->sample_list) free(tmp->sample_list);
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_set_sample_rate(mmu_memory_t *m,
                           int samp_rate)
{
    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    m->sample_rate = samp_rate;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_set_sample_time(mmu_memory_t *m,
                           int samp_time)
{
    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    m->sample_time = samp_time;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
reserve_memory_for_samples(mmu_memory_t *m,
                           size_t reserve_size)
{
    int rc = MMU_FAILURE;
    size_t type_index;

    /* iterate over all sample type as reserve enough space for all the samples
     * that will be needed for the entire run. */
    for (type_index = 0; type_index < m->num_sample_types; ++type_index) {
        if (MMU_SUCCESS != (rc =
            mmu_list_reserve(m->sample_list[type_index], reserve_size))) {
            return rc;
        }
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_get_total_iters(const mmu_memory_t *m,
                           size_t *total_iters)
{
    uint64_t total_iters_test;

    if (NULL == m || NULL == total_iters) {
        return MMU_FAILURE_INVALID_ARG;
    }

    /* make sure that the user isn't requesting something insane */
    total_iters_test = m->sample_rate * m->sample_time;
    if (total_iters_test > (uint64_t)SIZE_MAX) {
        /* overflow */
        fprintf(stderr,
                MMU_ERR_PREFIX"cannot continue with given sample rate and "
                "time:  result is too large!\n");
        return MMU_FAILURE_INVALID_USER_INPUT;
    }
    /* this is easy, but probably not what the user intended */
    else if (0 == total_iters_test) {
        fprintf(stderr,
                MMU_ERR_PREFIX"cannot continue with given sample rate and "
                "time:  results in zero total samples. this is probably not "
                "what you intended.\n");
        return MMU_FAILURE_INVALID_USER_INPUT;
    }

    *total_iters = total_iters_test;
    return MMU_SUCCESS;

}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_reserve_memory(mmu_memory_t *m)
{
    int rc = MMU_FAILURE;
    size_t total_iters = 0;

    if (MMU_SUCCESS != (rc = mmu_memory_get_total_iters(m, &total_iters))) {
        return rc;
    }
    /* reserve space for all the samples. the idea here is that we want to
     * allocate all the memory required for the collected samples before we
     * start measuring the memory usage. otherwise, we will be reporting some of
     * our growing memory usage. do this only once. */
    return reserve_memory_for_samples(m, total_iters);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_memory_sample_memory_usage(mmu_memory_t *m,
                               mmu_memory_flags_t flags)
{
    int rc = MMU_FAILURE;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    /* /// self /// */
    if (MMU_MEMORY_SAMPLE_SELF(flags) ||
        /* we don't really use MMU_MEMORY_SAMPLE_SELF_BOGUS, but it is here
         * for completeness */
        MMU_MEMORY_SAMPLE_SELF_BOGUS(flags)) {
        if (MMU_SUCCESS != (rc = get_self_sample(m, flags))) {
            fprintf(stderr,
                    MMU_ERR_PREFIX" error encountered while collecting "
                    "sample data: %s.\n", mmu_util_rc2str(rc));
            return rc;
        }
    }
    /* /// node /// */
    if (MMU_MEMORY_SAMPLE_NODE(flags) || MMU_MEMORY_SAMPLE_NODE_BOGUS(flags)) {
        if (MMU_SUCCESS != (rc = get_node_sample(m, flags))) {
            fprintf(stderr,
                    MMU_ERR_PREFIX" error encountered while collecting "
                    "sample data: %s.\n", mmu_util_rc2str(rc));
            return rc;
        }
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
char *
mmu_memory_get_item_label(mmu_memory_list_type_t type,
                          int type_index)
{

    if (type >= MMU_MEMORY_LAST) return NULL;

    switch (type) {
        case (MMU_MEMORY_SELF_PRE_INIT):
        case (MMU_MEMORY_SELF_POST_INIT):
            return mmu_memory_proc_self_tab[type_index];

        case (MMU_MEMORY_NODE_PRE_INIT):
        case (MMU_MEMORY_NODE_POST_INIT):
            return mmu_memory_proc_node_tab[type_index];
        default:
            return NULL;
    }
}
