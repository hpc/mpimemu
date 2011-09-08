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

/**
 * @author Samuel K. Gutierrez - samuelREMOVEME@lanl.gov
 * found a bug? have an idea? please let me know.
 */

/* /////////////////////////////////////////////////////////////////////////////
o ASSUMPTIONS:
    processes are placed in rank order.

o BUILD
    mpicc mpi_mem_usage.c -o mpi_mem_usage
    cc mpi_mem_usage.c -o mpi_mem_usage

o EXAMPLE USAGE
    mpirun -np 16 ./mpi_mem_usage
    srun -n16 ./mpi_mem_usage
    aprun -n 16 ./mpi_mem_usage

o TODO
    detect rank placement
///////////////////////////////////////////////////////////////////////////// */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include "mpi.h"

/* application name */
#define MPIMEMU_NAME "mpi_mem_usage"
/* current version */
#define MPIMEMU_VER  "0.1.5rc1"
/* TODO - this shoud be a run-time parameter */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* target PPN - USER CHANGE BELOW TO MATCH TARGET MACHINE                     */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#define MPIMEMU_PPN 16

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* do_send_recv_ring on by default                                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef MPIMEMU_DO_SEND_RECV
    #define MPIMEMU_DO_SEND_RECV 1
#elif defined MPIMEMU_DO_SEND_RECV && (MPIMEMU_DO_SEND_RECV) <= 0
    #undef MPIMEMU_DO_SEND_RECVV
    #define MPIMEMU_DO_SEND_RECV 0
#endif

/* ////////////////////////////////////////////////////////////////////////// */
/* convenience macros                                                         */
/* ////////////////////////////////////////////////////////////////////////// */
/* "master" rank */
#define MPIMEMU_MASTER_RANK  0

#define MPIMEMU_STRINGIFY(x) #x
#define MPIMEMU_TOSTRING(x)  MPIMEMU_STRINGIFY(x)

#define MPIMEMU_ERR_AT       __FILE__ " ("MPIMEMU_TOSTRING(__LINE__)")"
#define MPIMEMU_ERR_PREFIX   "-[MPIMEMU ERROR: "MPIMEMU_ERR_AT"]- "

/* error message */
#define MPIMEMU_ERR_MSG(pfargs...)                                             \
do {                                                                           \
    fprintf(stderr, MPIMEMU_ERR_PREFIX);                                       \
    fprintf(stderr, pfargs);                                                   \
} while (0)

/* mpi check */
#define MPIMEMU_MPICHK(_ret_,_gt_)                                             \
do {                                                                           \
    if (MPI_SUCCESS != (_ret_)) {                                              \
        MPI_Error_string((_ret_),                                              \
                         err_str,                                              \
                         &err_str_len);                                        \
        MPIMEMU_ERR_MSG("mpi success not returned... %s (errno: %d)\n",        \
                        err_str,                                               \
                        (_ret_));                                              \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

/* memory alloc check */
#define MPIMEMU_MEMCHK(_ptr_,_gt_)                                             \
do {                                                                           \
    if (NULL == (_ptr_)) {                                                     \
        MPIMEMU_ERR_MSG("memory allocation error on %s\n", hostname_buff);     \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

/* printf with flush */
#define MPIMEMU_PF(pfargs...)                                                  \
do {                                                                           \
    fprintf(stdout, pfargs);                                                   \
    fflush(stdout);                                                            \
} while (0)

/* master rank printf */
#define MPIMEMU_MPF(pfargs...)                                                 \
do {                                                                           \
    if (my_rank == (MPIMEMU_MASTER_RANK)) {                                    \
        fprintf(stdout, pfargs);                                               \
        fflush(stdout);                                                        \
    }                                                                          \
} while (0)

/* pre mpi init prefix */
#define MPIMEMU_PMPI_PREFIX    "Pre_MPI_Init_"
/* file containing node memory usage information */
#define MPIMEMU_MEMINFO_FILE   "/proc/meminfo"
/* template for proc memory usage information */
#define MPIMEMU_PMEMINFO_TMPLT "/proc/%d/status"
/* line buffer max */
#define MPIMEMU_LINE_MAX        128
/* key value max length */
#define MPIMEMU_KEY_LEN_MAX     32
/* start time string max length */
#define MPIMEMU_TIME_STR_MAX    16
/* at most 10 samples/second (in microseconds) */
#define MPIMEMU_SLEEPY_TIME     100000
/* failure return code */
#define MPIMEMU_FAILURE         0
/* success return code */
#define MPIMEMU_SUCCESS         1
/* number of items we are recording for node mem info */
#define MPIMEMU_MEM_INFO_LEN    13
/* number of items we are recording for proc mem info */
#define MPIMEMU_NUM_STATUS_VARS 10
/* number of samples */
#define MPIMEMU_NUM_SAMPLES     100
/* send recv buff size */
#define MPIMEMU_SR_BUFF_SZ      1024
/* invalid key index value */
#define MPIMEMU_INVLD_KEY_INDX  -1
/* path buffer max */
#define MPIMEMU_PATH_MAX        1024
/* number of memory types */
#define MPIMEMU_NUM_MEM_TYPES   2

/* memory query types */
typedef enum {
    MEM_TYPE_NODE = 0,
    MEM_TYPE_PROC

} mem_info_type_t;

typedef enum {
    STATUS_VMPEAK = 0,
    STATUS_VMSIZE,
    STATUS_VMLCK,
    STATUS_VMHWM,
    STATUS_VMRSS,
    STATUS_VMDATA,
    STATUS_VMSTK,
    STATUS_VMEXE,
    STATUS_VMLIB,
    STATUS_VMPTE
} status_type_index_t;

/* "valid" status key values */
static const char *status_name_list[MPIMEMU_KEY_LEN_MAX] = {
    "VmPeak",
    "VmSize",
    "VmLck",
    "VmHWM",
    "VmRSS",
    "VmData",
    "VmStk",
    "VmExe",
    "VmLib",
    "VmPTE"
};

typedef enum {
    MEM_TOTAL_INDEX = 0,
    MEM_FREE_INDEX,
    MEM_USED_INDEX,
    BUFFERS_INDEX,
    CACHED_INDEX,
    SWAP_CACHED_INDEX,
    ACTIVE_INDEX,
    INACTIVE_INDEX,
    SWAP_TOTAL_INDEX,
    SWAP_FREE_INDEX,
    DIRTY_INDEX,
    COMMIT_LIMIT_INDEX,
    COMMITTED_AS_INDEX
} mem_info_type_index;

/* "valid" node key values */
static const char *meminfo_name_list[MPIMEMU_KEY_LEN_MAX] = {
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
    "Committed_AS"
};

/* mem info stuff */
typedef struct mem_info_t {
    const char **index_name_ptr;
    int num_elements;
} mem_info_t;

/**
 * mem info array
 * items should following the ordering specified by mem_info_type_t
 */
static mem_info_t mem_info[MPIMEMU_NUM_MEM_TYPES] = {
    /* node */
    {meminfo_name_list,  MPIMEMU_MEM_INFO_LEN},
    /* proc */
    {status_name_list, MPIMEMU_NUM_STATUS_VARS}
};

/* local reduction operations */
typedef enum {
    LOCAL_MIN = 0,
    LOCAL_MAX,
    LOCAL_SUM
} local_reduction_ops;

/* my rank */
static int my_rank;
/**
 * my color
 * either 0 (collects node mem info) or 1 (performs dummy collectives).
 */
static int my_color;
/* holds mpi return codes */
static int mpi_ret_code;
/* my pid */
static pid_t my_pid;
/* size of mpi_comm_world */
static int num_ranks;
/* worker communicator (see: my_color) */
static MPI_Comm worker_comm;
/* error string length */
static int err_str_len;
/* time junk */
static struct tm *bd_time_ptr;
static time_t raw_time;
/* error string buffer */
static char err_str[MPI_MAX_ERROR_STRING];
/* hostname buffer */
static char hostname_buff[MPI_MAX_PROCESSOR_NAME];
/* holds values for each recorded value */
static unsigned long int *node_mem_vals = NULL, *proc_mem_vals = NULL;
/* holds meminfo sample values */
static unsigned long int **node_samples = NULL,
                         **pre_mpi_init_proc_samples = NULL,
                         **proc_samples = NULL;
/* holds min sample values */
static unsigned long int *node_min_sample_values = NULL,
                         *proc_min_sample_values = NULL;
/* holds max sample values */
static unsigned long int *node_max_sample_values = NULL,
                         *proc_max_sample_values = NULL;
/* holds sample averages */
static double *node_sample_aves = NULL, *proc_sample_aves = NULL;
/* holds sample averages */
static double *node_min_sample_aves = NULL, *proc_min_sample_aves = NULL;
/* holds sample averages */
static double *node_max_sample_aves = NULL, *proc_max_sample_aves = NULL;
/* number of processes that are doing work */
static int num_workers;
/* start time buffer */
static char start_time_buff[MPIMEMU_TIME_STR_MAX];

/* ////////////////////////////////////////////////////////////////////////// */
/* static forward declarations                                                */
/* ////////////////////////////////////////////////////////////////////////// */
static int
init(void);

static int
init_mpi(int argc,
         char **argv);

static int
fini(void);

static int
fini_mpi(void);

static int
set_mem_info(int mem_info_type,
             unsigned long int *mem_vals,
             const char *mem_info_str);

static int
update_mem_info(int mem_info_type,
                unsigned long int *mem_vals);

static int
reduce_local(const unsigned long int *in_vec,
             unsigned long int *out,
             int in_vec_len,
             int op);

static unsigned long int
strtoul_wec(const char *nptr,
            char **endptr,
            int base,
            int *ret_code);

static int
is_valid_key(int mem_info_type,
             const char *key,
             int *index_if_valid);

static int
get_local_mma(unsigned long int **in_mat,
              int vec_len,
              unsigned long int **out_min_vec_ptr,
              unsigned long int **out_max_vec_ptr,
              double **out_ave_vec_ptr);

static int
get_global_mma(unsigned long int **in_out_min_vec_ptr,
               unsigned long int **in_out_max_vec_ptr,
               double **in_out_ave_vec_ptr,
               int vec_len,
               double **out_min_samp_vec_ptr,
               double **out_max_samp_vec_ptr,
               MPI_Comm comm,
               int num_members);

static double *
lfmalloc(size_t mult);

static unsigned long int *
lumalloc(size_t mult);

static unsigned long int **
lupmalloc(size_t mult);

#if MPIMEMU_DO_SEND_RECV == 1
static int
do_send_recv_ring(void);
#endif

/* ////////////////////////////////////////////////////////////////////////// */
/* helper functions                                                           */
/* ////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */
static double *
lfmalloc(size_t mult)
{
    return (double *)malloc(sizeof(double) * mult);
}

/* ////////////////////////////////////////////////////////////////////////// */
static unsigned long int *
lumalloc(size_t mult)
{
    return (unsigned long int *)malloc(sizeof(unsigned long int) * mult);
}

/* ////////////////////////////////////////////////////////////////////////// */
static unsigned long int **
lupmalloc(size_t mult)
{
    return (unsigned long int **)malloc(sizeof(unsigned long int *) * mult);
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
init(void)
{
    int i;

    /* get start date and time */
    time(&raw_time);
    bd_time_ptr = localtime(&raw_time);
    strftime(start_time_buff,
             MPIMEMU_TIME_STR_MAX,
             "%Y%m%d-%H%M%S",
             bd_time_ptr);

    /* record my pid */
    my_pid = getpid();

    /* EVERYONE allocate some memory */
    /* ////////////////////////////////////////////////////////////////////// */
    /* node memory usage */
    /* ////////////////////////////////////////////////////////////////////// */
    node_mem_vals = lumalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_mem_vals, error);

    node_min_sample_values = lumalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_min_sample_values, error);

    node_max_sample_values = lumalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_max_sample_values, error);

    node_sample_aves = lfmalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_sample_aves, error);

    node_min_sample_aves = lfmalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_min_sample_aves, error);

    node_max_sample_aves = lfmalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_max_sample_aves, error);

    node_samples = lupmalloc(MPIMEMU_MEM_INFO_LEN);
    MPIMEMU_MEMCHK(node_samples, error);

    for (i = 0; i < MPIMEMU_MEM_INFO_LEN; ++i) {
        node_samples[i] = lumalloc(MPIMEMU_NUM_SAMPLES);
        MPIMEMU_MEMCHK(node_samples[i], error);
    }

    /* ////////////////////////////////////////////////////////////////////// */
    /* proc memory usage vars */
    /* ////////////////////////////////////////////////////////////////////// */
    proc_mem_vals = lumalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_mem_vals, error);

    proc_min_sample_values = lumalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_min_sample_values, error);

    proc_max_sample_values = lumalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_max_sample_values, error);

    proc_sample_aves = lfmalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_sample_aves, error);

    proc_min_sample_aves = lfmalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_min_sample_aves, error);

    proc_max_sample_aves = lfmalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_max_sample_aves, error);

    proc_samples = lupmalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(proc_samples, error);

    pre_mpi_init_proc_samples = lupmalloc(MPIMEMU_NUM_STATUS_VARS);
    MPIMEMU_MEMCHK(pre_mpi_init_proc_samples, error);

    for (i = 0; i < MPIMEMU_NUM_STATUS_VARS; ++i) {
        proc_samples[i] = lumalloc(MPIMEMU_NUM_SAMPLES);
        MPIMEMU_MEMCHK(proc_samples[i], error);
        pre_mpi_init_proc_samples[i] = lumalloc(MPIMEMU_NUM_SAMPLES);
        MPIMEMU_MEMCHK(pre_mpi_init_proc_samples[i], error);
    }

    return MPIMEMU_SUCCESS;
error:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
init_mpi(int argc,
         char **argv)
{
    int i;
    /* init MPI */
    mpi_ret_code = MPI_Init(&argc, &argv);
    MPIMEMU_MPICHK(mpi_ret_code, error);
    /* get comm size */
    mpi_ret_code = MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    MPIMEMU_MPICHK(mpi_ret_code, error);
    /* get my rank */
    mpi_ret_code = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPIMEMU_MPICHK(mpi_ret_code, error);
    /* get my host's name */
    mpi_ret_code = MPI_Get_processor_name(hostname_buff, &i);
    MPIMEMU_MPICHK(mpi_ret_code, error);

    /* split into two groups - 0: no work; 1: all work and no play */
    my_color = (0 == my_rank % MPIMEMU_PPN);

    mpi_ret_code = MPI_Comm_split(MPI_COMM_WORLD, my_color, my_rank,
                                  &worker_comm);
    MPIMEMU_MPICHK(mpi_ret_code, error);

    /* how many workers do we have? */
    mpi_ret_code = MPI_Allreduce(&my_color, &num_workers, 1, MPI_INT, MPI_SUM,
                                 MPI_COMM_WORLD);
    MPIMEMU_MPICHK(mpi_ret_code, error);

    return MPIMEMU_SUCCESS;
error:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
fini_mpi(void)
{
    mpi_ret_code = MPI_Comm_free(&worker_comm);
    MPIMEMU_MPICHK(mpi_ret_code, error);

    mpi_ret_code = MPI_Finalize();
    MPIMEMU_MPICHK(mpi_ret_code, error);

    return MPIMEMU_SUCCESS;
error:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
fini(void)
{
    int i;
    /* node */
    free(node_mem_vals);
    free(node_min_sample_values);
    free(node_max_sample_values);
    free(node_sample_aves);
    free(node_min_sample_aves);
    free(node_max_sample_aves);
    for (i = 0; i < MPIMEMU_MEM_INFO_LEN; ++i) {
        free(node_samples[i]);
    }
    free(node_samples);

    /* proc */
    free(proc_mem_vals);
    free(proc_min_sample_values);
    free(proc_max_sample_values);
    free(proc_sample_aves);
    free(proc_min_sample_aves);
    free(proc_max_sample_aves);
    for (i = 0; i < MPIMEMU_NUM_STATUS_VARS; ++i) {
        free(proc_samples[i]);
        free(pre_mpi_init_proc_samples[i]);
    }
    free(proc_samples);
    free(pre_mpi_init_proc_samples);
    return MPIMEMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* get local min, max, ave */
static int
get_local_mma(unsigned long int **in_mat,
              int vec_len,
              unsigned long int **out_min_vec_ptr,
              unsigned long int **out_max_vec_ptr,
              double **out_ave_vec_ptr)
{
    int i;
    unsigned long int tmp_sum = 0;
    unsigned long int *minv   = *out_min_vec_ptr;
    unsigned long int *maxv   = *out_max_vec_ptr;
    double *avev              = *out_ave_vec_ptr;

    for (i = 0; i < vec_len; ++i) {
        /* local min */
        if (MPIMEMU_SUCCESS != reduce_local(in_mat[i], &minv[i],
                                            MPIMEMU_NUM_SAMPLES, LOCAL_MIN)) {
            goto err;
        }
        /* local max */
        if (MPIMEMU_SUCCESS != reduce_local(in_mat[i], &maxv[i],
                                            MPIMEMU_NUM_SAMPLES, LOCAL_MAX)) {
            goto err;
        }
        /* local ave */
        if (MPIMEMU_SUCCESS != reduce_local(in_mat[i], &tmp_sum,
                                            MPIMEMU_NUM_SAMPLES, LOCAL_SUM)) {
            goto err;
        }
        avev[i] = (0 == tmp_sum) ? 0.0 :
        (double)tmp_sum/(double)MPIMEMU_NUM_SAMPLES;
    }

    return MPIMEMU_SUCCESS;
err:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* get global min, max, ave */
static int
get_global_mma(unsigned long int **in_out_min_vec_ptr,
               unsigned long int **in_out_max_vec_ptr,
               double **in_out_ave_vec_ptr,
               int vec_len,
               double **out_min_samp_vec_ptr,
               double **out_max_samp_vec_ptr,
               MPI_Comm comm,
               int num_members)
{
    int i;
    unsigned long int *io_min_vp   = *in_out_min_vec_ptr;
    unsigned long int *io_max_vp   = *in_out_max_vec_ptr;
    double *io_ave_vp              = *in_out_ave_vec_ptr;
    double *o_min_samp_vp          = *out_min_samp_vec_ptr;
    double *o_max_samp_vp          = *out_max_samp_vec_ptr;
    unsigned long int tmp_send_buf = 0;
    double tmp_double_buf          = 0.0;

    for (i = 0; i < vec_len; ++i) {
        tmp_send_buf = io_min_vp[i];
        mpi_ret_code = MPI_Allreduce(&tmp_send_buf, &io_min_vp[i], 1,
                                     MPI_UNSIGNED_LONG, MPI_SUM, comm);
        MPIMEMU_MPICHK(mpi_ret_code, error);

        o_min_samp_vp[i] = (0 == io_min_vp[i]) ? 0.0 :
                           (double)io_min_vp[i]/(double)num_members;

        tmp_send_buf = io_max_vp[i];
        mpi_ret_code = MPI_Allreduce(&tmp_send_buf, &io_max_vp[i], 1,
                                     MPI_UNSIGNED_LONG, MPI_SUM, comm);
        MPIMEMU_MPICHK(mpi_ret_code, error);

        o_max_samp_vp[i] = (0 == io_max_vp[i]) ? 0.0 :
                           (double)io_max_vp[i]/(double)num_members;

        tmp_double_buf = io_ave_vp[i];
        mpi_ret_code = MPI_Allreduce(&tmp_double_buf, &io_ave_vp[i], 1,
                                     MPI_DOUBLE, MPI_SUM, comm);
        MPIMEMU_MPICHK(mpi_ret_code, error);

        io_ave_vp[i] = (0.0 == io_ave_vp[i]) ? 0.0 :
                       io_ave_vp[i]/(double)num_members;
    }

    return MPIMEMU_SUCCESS;
error:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
reduce_local(const unsigned long int *in_vec,
             unsigned long int *out,
             int in_vec_len,
             int op)
{
    int i, set = 0;
    unsigned long int val = 0;

    switch (op) {
        case LOCAL_MIN:
            for (i = 0; i < in_vec_len; ++i) {
                if (set) {
                    if (val > in_vec[i]) {
                        val = in_vec[i];
                    }
                }
                else {
                    val = in_vec[i];
                    set = 1;
                }
            }
            break;

        case LOCAL_MAX:
            for (i = 0; i < in_vec_len; ++i) {
                if (set) {
                    if (val < in_vec[i]) {
                        val = in_vec[i];
                    }
                }
                else {
                    val = in_vec[i];
                    set = 1;
                }
            }
            break;

        case LOCAL_SUM:
            for (i = 0; i < in_vec_len; ++i) {
                val += in_vec[i];
                /* rudimentary overflow detection */
                if (val < in_vec[i]) {
                    MPIMEMU_ERR_MSG("OVERFLOW DETECTED\n");
                    goto err;
                }
            }
            break;

        default:
            MPIMEMU_ERR_MSG("%d : unknown option", op);
            goto err;
    }

    *out = val;

    return MPIMEMU_SUCCESS;
err:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* strtoul with error checks - yeah, i said it */
static unsigned long int
strtoul_wec(const char *nptr,
            char **endptr,
            int base,
            int *ret_code)
{
    char *end_ptr = NULL;
    unsigned long int value;

    /* assume all is well */
    *ret_code = MPIMEMU_SUCCESS;

    /* check for strtoul errors */
    errno = 0;
    value = strtoul(nptr, endptr, 10);

    if ((ERANGE == errno && (ULONG_MAX == value || 0 == value)) ||
        (0 != errno && 0 == value)) {
        *ret_code = MPIMEMU_FAILURE;
    }
    if (nptr == end_ptr) {
        *ret_code = MPIMEMU_FAILURE;
    }

    /* caller must always check the return code */
    return value;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
is_valid_key(int mem_info_type,
             const char *key,
             int *index_if_valid)
{
    int i;
    for (i = 0; i < mem_info[mem_info_type].num_elements; ++i) {
        if (0 == strcmp(mem_info[mem_info_type].index_name_ptr[i], key)) {
            *index_if_valid = i;
            return 1;
        }
    }
    /* not a key that we care about */
    return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
set_mem_info(int mem_info_type,
             unsigned long int *mem_vals,
             const char *mem_info_str)
{
    int i         = 0;
    char *end_ptr = NULL;
    int key_index = MPIMEMU_INVLD_KEY_INDX;
    int ret_code;
    unsigned long int value;
    char key[MPIMEMU_KEY_LEN_MAX];

    memset(key, '\0', MPIMEMU_KEY_LEN_MAX);

    /* get the key length */
    while (':' != mem_info_str[i] && '\0' != mem_info_str[i]) {
        ++i;
    }

    /* get key substring */
    strncpy(key, mem_info_str, (MPIMEMU_KEY_LEN_MAX - 1) > i ?
            i : (MPIMEMU_KEY_LEN_MAX - 1));

    /* do we care about this particular key? */
    if (!is_valid_key(mem_info_type, key, &key_index)) {
        goto out;
    }

    /* if we are here, we are dealing with a valid key */

    /* eat whitespace */
    while (' ' != mem_info_str[i] && '\0' != mem_info_str[i]) {
        ++i;
    }

    /* update value and check for strtoul errors */
    value = strtoul_wec(mem_info_str + i, &end_ptr, 10, &ret_code);

    if (MPIMEMU_SUCCESS != ret_code) {
        MPIMEMU_ERR_MSG("%s\n", "strtoul error");
        goto err;
    }

    /* update values based on key value */
    mem_vals[key_index] = value;

out:
    return MPIMEMU_SUCCESS;
err:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
update_mem_info(int mem_info_type,
                unsigned long int *mem_vals)
{
    char line_buffer[MPIMEMU_LINE_MAX];
    char file_name_buff[MPIMEMU_PATH_MAX];
    FILE *file_ptr  = NULL;

    switch (mem_info_type) {
        case MEM_TYPE_NODE:
            snprintf(file_name_buff, MPIMEMU_PATH_MAX - 1, "%s",
                     MPIMEMU_MEMINFO_FILE);
            break;

        case MEM_TYPE_PROC:
            snprintf(file_name_buff, MPIMEMU_PATH_MAX - 1,
                     MPIMEMU_PMEMINFO_TMPLT, (int)my_pid);
            break;
        default:
            MPIMEMU_ERR_MSG("unknown mem info type - sad all day\n");
            goto err;
    }

    if (NULL == (file_ptr = fopen(file_name_buff, "r"))) {
        int err = errno;
        MPIMEMU_ERR_MSG("fopen failure: errno: %d (%s)\n",
                        err,
                        strerror(err));
        goto err;
    }

    /* iterate over the file one line at a time */
    while (NULL != fgets(line_buffer, MPIMEMU_LINE_MAX, file_ptr)) {
        if (MPIMEMU_SUCCESS != set_mem_info(mem_info_type, mem_vals,
                                            line_buffer)) {
            goto err;
        }
    }

    /* now we can safely calculate used memory on the node */
    if (MEM_TYPE_NODE == mem_info_type) {
        mem_vals[MEM_USED_INDEX] = mem_vals[MEM_TOTAL_INDEX] -
                                   mem_vals[MEM_FREE_INDEX];
    }

    /* close the file */
    if (0 != fclose(file_ptr)) {
        int err = errno;
        MPIMEMU_ERR_MSG("fclose failure: errno: %d (%s)\n", err, strerror(err));
        goto err;
    }

    return MPIMEMU_SUCCESS;
err:
    return MPIMEMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
#if MPIMEMU_DO_SEND_RECV == 1
static int
do_send_recv_ring(void)
{
    int i            = 0;
    int j            = 0;
    int num_iters    = MPIMEMU_PPN;
    int send_tag     = 0;
    int recv_tag     = 0;
    int r_neighbor   = 0;
    int l_neighbor   = 0;
    int mpi_ret_code = MPI_SUCCESS;
    char send_char_buff[MPIMEMU_SR_BUFF_SZ];
    char recv_char_buff[MPIMEMU_SR_BUFF_SZ];
    MPI_Status status;

    for (i = 1; i <= num_iters; ++i) {
        r_neighbor = (my_rank + i) % num_ranks;
        l_neighbor = my_rank;

        for (j = 0; j < i; ++j) {
            --l_neighbor;
            if (l_neighbor < 0) {
                l_neighbor = num_ranks - 1;
            }
        }

        mpi_ret_code = MPI_Sendrecv(send_char_buff, MPIMEMU_SR_BUFF_SZ,
                                    MPI_CHAR, r_neighbor, i, recv_char_buff,
                                    MPIMEMU_SR_BUFF_SZ, MPI_CHAR, l_neighbor, i,
                                    MPI_COMM_WORLD, &status);
        MPIMEMU_MPICHK(mpi_ret_code, error);

        mpi_ret_code = MPI_Sendrecv(send_char_buff,
                                    MPIMEMU_SR_BUFF_SZ,
                                    MPI_CHAR,
                                    l_neighbor,
                                    send_tag,
                                    recv_char_buff,
                                    MPIMEMU_SR_BUFF_SZ,
                                    MPI_CHAR,
                                    r_neighbor,
                                    recv_tag,
                                    MPI_COMM_WORLD,
                                    &status);
        MPIMEMU_MPICHK(mpi_ret_code, error);
    }

    return 1;
error:
    return 0;
}
#endif

/* ////////////////////////////////////////////////////////////////////////// */
/* main                                                                       */
/* ////////////////////////////////////////////////////////////////////////// */
int
main(int argc,
     char **argv)
{
    /* tmp vars */
    int i, j;
    /* buffers for "dummy" work */
    int send_buff, recv_buff;

    /* init some buffs, etc. */
    if (MPIMEMU_SUCCESS != init()) {
        MPIMEMU_ERR_MSG("init error\n");
        goto error;
    }

    /* ////////////////////////////////////////////////////////////////////// */
    /* pre mpi init sampling loop */
    /* ////////////////////////////////////////////////////////////////////// */
    for (i = 0; i < MPIMEMU_NUM_SAMPLES; ++i) {
        /* all processes participate here ... update process mem usage */
        if (MPIMEMU_SUCCESS != update_mem_info(MEM_TYPE_PROC, proc_mem_vals)) {
            MPIMEMU_ERR_MSG("unable to update proc memory usage info\n");
            goto error;
        }

        for (j = 0; j < MPIMEMU_NUM_STATUS_VARS; ++j) {
            /* record pre mpi process sample values */
            pre_mpi_init_proc_samples[j][i] = proc_mem_vals[j];
        }

        usleep((unsigned long)MPIMEMU_SLEEPY_TIME);
    }

    /* init mpi, etc. */
    if (MPIMEMU_SUCCESS != init_mpi(argc, argv)) {
        MPIMEMU_ERR_MSG("mpi init error\n");
        goto error;
    }

    /* why not set the dummy send buff to my rank */
    send_buff = my_rank;

    /**
     * make sure numpe a multiple of MPIMEMU_PPN.
     * idea: one rank process per node will calculate node memory usage.
     * ASSUMING: processes are placed in rank order.
     */
    if (0 != (num_ranks % MPIMEMU_PPN)) {
        MPIMEMU_MPF("numpe must be a multiple of %d\n", (int)MPIMEMU_PPN);
        goto finil;
    }

    /* let the "master process" print out some header stuff */
    MPIMEMU_MPF("# %s %s\n", MPIMEMU_NAME, MPIMEMU_VER);
    MPIMEMU_MPF("# host %s\n", hostname_buff);
    MPIMEMU_MPF("# date_time %s\n", start_time_buff);
    MPIMEMU_MPF("# ppn %d \n", (int)MPIMEMU_PPN);
    MPIMEMU_MPF("# numpe %d\n", num_ranks);
    MPIMEMU_MPF("# with_send_recv %d\n", MPIMEMU_DO_SEND_RECV);
    MPIMEMU_MPF("# num_samples %d \n", (int)MPIMEMU_NUM_SAMPLES);
    MPIMEMU_MPF("# samples/s %.1lf \n", 1e6/(double)MPIMEMU_SLEEPY_TIME);
    MPIMEMU_MPF("# item_name, ave_min (kB), ave_max (kB), ave (kB)\n");

    /* ////////////////////////////////////////////////////////////////////// */
    /* main sampling loop */
    /* ////////////////////////////////////////////////////////////////////// */
    for (i = 0; i < MPIMEMU_NUM_SAMPLES; ++i) {
        /* make sure everyone is participating in a global collective */
        mpi_ret_code = MPI_Allreduce(&send_buff, &recv_buff, 1, MPI_INT,
                                     MPI_MAX, MPI_COMM_WORLD);
        MPIMEMU_MPICHK(mpi_ret_code, error);

        /**
         * when enabled, also do a send recv ring.
         * idea: allocate more buffs to more closely emulate a real app.
         */
#if MPIMEMU_DO_SEND_RECV == 1
        do_send_recv_ring();
#endif
        /* do i need to do some real work? */
        if (1 == my_color) {
            /* if so, update node memory usage */
            if (MPIMEMU_SUCCESS != update_mem_info(MEM_TYPE_NODE,
                                                   node_mem_vals)) {
                MPIMEMU_ERR_MSG("unable to update node memory usage info\n");
                goto error;
            }
            for (j = 0; j < MPIMEMU_MEM_INFO_LEN; ++j) {
                /* record local node sample values */
                node_samples[j][i] = node_mem_vals[j];
            }
        }

        /* all processes participate here ... update process mem usage */
        if (MPIMEMU_SUCCESS != update_mem_info(MEM_TYPE_PROC, proc_mem_vals)) {
            MPIMEMU_ERR_MSG("unable to update proc memory usage info\n");
            goto error;
        }

        for (j = 0; j < MPIMEMU_NUM_STATUS_VARS; ++j) {
            /* record process sample values */
            proc_samples[j][i] = proc_mem_vals[j];
        }

        usleep((unsigned long)MPIMEMU_SLEEPY_TIME);
    }
    /* ////////////////////////////////////////////////////////////////////// */
    /* end of main sampling loop */
    /* ////////////////////////////////////////////////////////////////////// */

    if (1 == my_color) {
        /* calculate local values (node min, node max, node ave) */
        if (MPIMEMU_SUCCESS != get_local_mma(node_samples,
                                             MPIMEMU_MEM_INFO_LEN,
                                             &node_min_sample_values,
                                             &node_max_sample_values,
                                             &node_sample_aves)) {
            MPIMEMU_ERR_MSG("get_local_mma error\n");
            goto error;
        }

        /* calculate global values (node min, node max, node ave) */
        if (MPIMEMU_SUCCESS != get_global_mma(&node_min_sample_values,
                                              &node_max_sample_values,
                                              &node_sample_aves,
                                              MPIMEMU_MEM_INFO_LEN,
                                              &node_min_sample_aves,
                                              &node_max_sample_aves,
                                              worker_comm,
                                              num_workers)) {
            MPIMEMU_ERR_MSG("get_global_mma error\n");
            goto error;
        }
    }

    /* calculate pre mpi init local values (proc min, proc max, proc ave) */
    if (MPIMEMU_SUCCESS != get_local_mma(pre_mpi_init_proc_samples,
                                         MPIMEMU_NUM_STATUS_VARS,
                                         &proc_min_sample_values,
                                         &proc_max_sample_values,
                                         &proc_sample_aves)) {
        MPIMEMU_ERR_MSG("get_local_mma error\n");
        goto error;
    }
    /* calculate pre mpi init global values (proc min, proc max, proc ave) */
    if (MPIMEMU_SUCCESS != get_global_mma(&proc_min_sample_values,
                                          &proc_max_sample_values,
                                          &proc_sample_aves,
                                          MPIMEMU_NUM_STATUS_VARS,
                                          &proc_min_sample_aves,
                                          &proc_max_sample_aves,
                                          MPI_COMM_WORLD,
                                          num_ranks)) {
        MPIMEMU_ERR_MSG("get_global_mma error\n");
        goto error;
    }

    /* print pre mpi init results */
    if (MPIMEMU_MASTER_RANK == my_rank) {
        for (i = 0; i < MPIMEMU_NUM_STATUS_VARS; ++i) {
            MPIMEMU_MPF(MPIMEMU_PMPI_PREFIX"%s, %.2lf, %.2lf, %.2lf\n",
                        status_name_list[i],
                        proc_min_sample_aves[i],
                        proc_max_sample_aves[i],
                        proc_sample_aves[i]);
        }
    }
    /* ////////////////////////////////////////////////////////////////////// */
    /* post mpi init stuff */
    /* ////////////////////////////////////////////////////////////////////// */
    if (MPIMEMU_SUCCESS != get_local_mma(proc_samples,
                                         MPIMEMU_NUM_STATUS_VARS,
                                         &proc_min_sample_values,
                                         &proc_max_sample_values,
                                         &proc_sample_aves)) {
        MPIMEMU_ERR_MSG("get_local_mma error\n");
        goto error;
    }

    /* calculate global values (proc min, proc max, proc ave) */
    if (MPIMEMU_SUCCESS != get_global_mma(&proc_min_sample_values,
                                          &proc_max_sample_values,
                                          &proc_sample_aves,
                                          MPIMEMU_NUM_STATUS_VARS,
                                          &proc_min_sample_aves,
                                          &proc_max_sample_aves,
                                          MPI_COMM_WORLD,
                                          num_ranks)) {
        MPIMEMU_ERR_MSG("get_global_mma error\n");
        goto error;
    }

    /* print the results */
    if (MPIMEMU_MASTER_RANK == my_rank) {
        for (i = 0; i < MPIMEMU_NUM_STATUS_VARS; ++i) {
            MPIMEMU_MPF("%s, %.2lf, %.2lf, %.2lf\n",
                        status_name_list[i],
                        proc_min_sample_aves[i],
                        proc_max_sample_aves[i],
                        proc_sample_aves[i]);
        }
        for (i = 0; i < MPIMEMU_MEM_INFO_LEN; ++i) {
            MPIMEMU_MPF("%s, %.2lf, %.2lf, %.2lf\n",
                        meminfo_name_list[i],
                        node_min_sample_aves[i],
                        node_max_sample_aves[i],
                        node_sample_aves[i]);
        }
    }

    /* done! */
    if (MPIMEMU_SUCCESS != fini_mpi()) {
        MPIMEMU_ERR_MSG("mpi finalization error\n");
        goto error;
    }

    if (MPIMEMU_SUCCESS != fini()) {
        MPIMEMU_ERR_MSG("finalization error\n");
        goto error;
    }

    return EXIT_SUCCESS;
error:
    MPI_Abort(MPI_COMM_WORLD, mpi_ret_code);
    return EXIT_FAILURE;
finil:
    MPI_Finalize();
    return EXIT_FAILURE;
}

/* /////////////////////////////////////////////////////////////////////////////
o CHANGE LOG

2010-08-23 Samuel K. Gutierrez samuelREMOVE_ME[at]lanl.gov
    * minor style changes

2010-08-23 Samuel K. Gutierrez samuel(remove_me)[at]lanl.gov
    * initial implementation.

2010-11-30 Samuel K. Gutierrez samuel(remove_me)[at]lanl.gov
    * per proc memory usage information.
    * pre mpi_init memory utilization.
    * cleanup and bug fixes.

2010-12-01 Samuel K. Gutierrez samuel(remove_me)[at]lanl.gov
    * v0.1.0 ready for cielo testing

2011-03-10 Samuel K. Gutierrez samuel(remove_me)[at]lanl.gov
    * added static to a few local vars and funcs.
    * on to 0.1.1
///////////////////////////////////////////////////////////////////////////// */
