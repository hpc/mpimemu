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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "constants.h"
#include "util.h"
#include "conv_macros.h"
#include "memory_usage.h"
#include "mpimemu.h"

#include "mpi.h"

/* ////////////////////////////////////////////////////////////////////////// */
static int
init(process_info_t *proc_infop)
{
    int i;
    int rc;

    /* get start date and time */
    time(&raw_time);
    bd_time_ptr = localtime(&raw_time);
    strftime(start_time_buff,
             MMU_TIME_STR_MAX,
             "%Y%m%d-%H%M%S",
             bd_time_ptr);

    /* record my pid */
    proc_infop->pid = getpid();

    /* --- EVERYONE allocate some memory --- */

    /* ////////////////////////////////////////////////////////////////////// */
    /* node memory usage */
    /* ////////////////////////////////////////////////////////////////////// */
    if (MMU_SUCCESS != (rc = mem_usage_construct(&node_mem_usage))) {
        /* no check in error path */
        mem_usage_destruct(&node_mem_usage);
        return rc;
    }
    /* ////////////////////////////////////////////////////////////////////// */
    /* proc memory usage vars */
    /* ////////////////////////////////////////////////////////////////////// */
    if (MMU_SUCCESS != (rc = mem_usage_construct(&proc_mem_usage))) {
        /* no check in error path */
        mem_usage_destruct(&proc_mem_usage);
        return rc;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static char *
mmu_mpi_rc2estr(int rc)
{
    static char errstr[MPI_MAX_ERROR_STRING];
    int elen;
    MPI_Error_string(rc, errstr, &elen);
    return errstr;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
init_mpi(int argc,
         char **argv)
{
    int i;
    int rc;
    char *bad_func = NULL;

    /* init MPI */
    if (MPI_SUCCESS != (rc = MPI_Init(&argc, &argv))) {
        bad_func = "MPI_Init";
        goto out;
    }
    /* get comm size */
    if (MPI_SUCCESS != (rc = MPI_Comm_size(MPI_COMM_WORLD, &num_ranks))) {
        bad_func = "MPI_Comm_size";
        goto out;
    }
    /* get my rank */
    if (MPI_SUCCESS != (rc = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank))) {
        bad_func = "MPI_Comm_rank";
        goto out;
    }
    /* get my host's name */
    if (MPI_SUCCESS != (rc = MPI_Get_processor_name(hostname_buff, &i))) {
        bad_func = "MPI_Get_processor_name";
        goto out;
    }
    /* split into two groups - 0: no work; 1: all work and no play */
    my_color = (0 == my_rank % MMU_PPN);

    if (MPI_SUCCESS != (rc = MPI_Comm_split(MPI_COMM_WORLD, my_color, my_rank,
                                            &worker_comm))) {
        bad_func = "MPI_Comm_split";
        goto out;
    }
    /* how many workers do we have? */
    if (MPI_SUCCESS != (rc = MPI_Allreduce(&my_color, &num_workers, 1,
                                           MPI_INT, MPI_SUM, MPI_COMM_WORLD))) {
        bad_func = "MPI_Allreduce";
        goto out;
    }

out:
    /* an error occurred */
    if (NULL != bad_func) {
        int initialized;
        MMU_ERR_MSG("%s failure detected [mpi rc: %d (%s)]\n", bad_func, rc,
                    mmu_mpi_rc2estr(rc));
        /* no error checks in error path */
        MPI_Initialized(&initialized);
        if (initialized) {
            MPI_Finalize();
        }
        return MMU_FAILURE_MPI;
    }
    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
fini_mpi(void)
{
    char *bad_func = NULL;
    int rc;

    if (MPI_SUCCESS != (rc = MPI_Comm_free(&worker_comm))) {
        bad_func = "MPI_Comm_free";
        goto out;
    }
    if (MPI_SUCCESS != (rc = MPI_Finalize())) {
        bad_func = "MPI_Finalize";
        goto out;
    }

out:
    if (NULL != bad_func) {
        MMU_ERR_MSG("%s failure detected [mpi rc: %d (%s)]\n", bad_func, rc,
                    mmu_mpi_rc2estr(rc));
        return MMU_FAILURE_MPI;
    }
    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
fini(void)
{
    int rc;
    if (MMU_SUCCESS != (rc = mem_usage_destruct(&node_mem_usage))) {
        /* TODO add error message */
        return rc;
    }
    if (MMU_SUCCESS != (rc = mem_usage_destruct(&proc_mem_usage))) {
        /* TODO add error message */
        return rc;
    }
    return MMU_SUCCESS;
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
        if (MMU_SUCCESS != reduce_local(in_mat[i], &minv[i],
                                            MMU_NUM_SAMPLES, LOCAL_MIN)) {
            goto err;
        }
        /* local max */
        if (MMU_SUCCESS != reduce_local(in_mat[i], &maxv[i],
                                            MMU_NUM_SAMPLES, LOCAL_MAX)) {
            goto err;
        }
        /* local ave */
        if (MMU_SUCCESS != reduce_local(in_mat[i], &tmp_sum,
                                            MMU_NUM_SAMPLES, LOCAL_SUM)) {
            goto err;
        }
        avev[i] = (0 == tmp_sum) ? 0.0 :
        (double)tmp_sum/(double)MMU_NUM_SAMPLES;
    }

    return MMU_SUCCESS;
err:
    return MMU_FAILURE;
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
        MMU_MPICHK(mpi_ret_code, error);

        o_min_samp_vp[i] = (0 == io_min_vp[i]) ? 0.0 :
                           (double)io_min_vp[i]/(double)num_members;

        tmp_send_buf = io_max_vp[i];
        mpi_ret_code = MPI_Allreduce(&tmp_send_buf, &io_max_vp[i], 1,
                                     MPI_UNSIGNED_LONG, MPI_SUM, comm);
        MMU_MPICHK(mpi_ret_code, error);

        o_max_samp_vp[i] = (0 == io_max_vp[i]) ? 0.0 :
                           (double)io_max_vp[i]/(double)num_members;

        tmp_double_buf = io_ave_vp[i];
        mpi_ret_code = MPI_Allreduce(&tmp_double_buf, &io_ave_vp[i], 1,
                                     MPI_DOUBLE, MPI_SUM, comm);
        MMU_MPICHK(mpi_ret_code, error);

        io_ave_vp[i] = (0.0 == io_ave_vp[i]) ? 0.0 :
                       io_ave_vp[i]/(double)num_members;
    }

    return MMU_SUCCESS;
error:
    return MMU_FAILURE;
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
                    MMU_ERR_MSG("OVERFLOW DETECTED\n");
                    goto err;
                }
            }
            break;

        default:
            MMU_ERR_MSG("%d : unknown option", op);
            goto err;
    }

    *out = val;

    return MMU_SUCCESS;
err:
    return MMU_FAILURE;
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
    int key_index = MMU_INVLD_KEY_INDX;
    int ret_code;
    unsigned long int value;
    char key[MMU_KEY_LEN_MAX];

    memset(key, '\0', MMU_KEY_LEN_MAX);

    /* get the key length */
    while (':' != mem_info_str[i] && '\0' != mem_info_str[i]) {
        ++i;
    }

    /* get key substring */
    strncpy(key, mem_info_str, (MMU_KEY_LEN_MAX - 1) > i ?
            i : (MMU_KEY_LEN_MAX - 1));

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

    if (MMU_SUCCESS != ret_code) {
        MMU_ERR_MSG("%s\n", "strtoul error");
        goto err;
    }

    /* update values based on key value */
    mem_vals[key_index] = value;

out:
    return MMU_SUCCESS;
err:
    return MMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
update_mem_info(process_info_t *proc_infop,
                int mem_info_type,
                unsigned long int *mem_vals)
{
    char line_buffer[MMU_LINE_MAX];
    char file_name_buff[MMU_PATH_MAX];
    FILE *file_ptr  = NULL;

    switch (mem_info_type) {
        case MEM_TYPE_NODE:
            snprintf(file_name_buff, MMU_PATH_MAX - 1, "%s",
                     MMU_MEMINFO_FILE);
            break;

        case MEM_TYPE_PROC:
            snprintf(file_name_buff, MMU_PATH_MAX - 1,
                     MMU_PMEMINFO_TMPLT, (int)proc_infop->pid);
            break;
        default:
            MMU_ERR_MSG("unknown mem info type - sad all day\n");
            goto err;
    }

    if (NULL == (file_ptr = fopen(file_name_buff, "r"))) {
        int err = errno;
        MMU_ERR_MSG("fopen failure: errno: %d (%s)\n",
                        err,
                        strerror(err));
        goto err;
    }

    /* iterate over the file one line at a time */
    while (NULL != fgets(line_buffer, MMU_LINE_MAX, file_ptr)) {
        if (MMU_SUCCESS != set_mem_info(mem_info_type, mem_vals,
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
        MMU_ERR_MSG("fclose failure: errno: %d (%s)\n", err, strerror(err));
        goto err;
    }

    return MMU_SUCCESS;
err:
    return MMU_FAILURE;
}

/* ////////////////////////////////////////////////////////////////////////// */
#if MMU_DO_SEND_RECV == 1
static int
do_send_recv_ring(void)
{
    int i            = 0;
    int j            = 0;
    int num_iters    = MMU_PPN;
    int send_tag     = 0;
    int recv_tag     = 0;
    int r_neighbor   = 0;
    int l_neighbor   = 0;
    int mpi_ret_code = MPI_SUCCESS;
    char send_char_buff[MMU_SR_BUFF_SZ];
    char recv_char_buff[MMU_SR_BUFF_SZ];
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

        mpi_ret_code = MPI_Sendrecv(send_char_buff, MMU_SR_BUFF_SZ,
                                    MPI_CHAR, r_neighbor, i, recv_char_buff,
                                    MMU_SR_BUFF_SZ, MPI_CHAR, l_neighbor, i,
                                    MPI_COMM_WORLD, &status);
        MMU_MPICHK(mpi_ret_code, error);

        mpi_ret_code = MPI_Sendrecv(send_char_buff,
                                    MMU_SR_BUFF_SZ,
                                    MPI_CHAR,
                                    l_neighbor,
                                    send_tag,
                                    recv_char_buff,
                                    MMU_SR_BUFF_SZ,
                                    MPI_CHAR,
                                    r_neighbor,
                                    recv_tag,
                                    MPI_COMM_WORLD,
                                    &status);
        MMU_MPICHK(mpi_ret_code, error);
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
    process_info_t process_info;

    /* init some buffs, etc. */
    if (MMU_SUCCESS != init(&process_info)) {
        MMU_ERR_MSG("init error\n");
        goto error;
    }

    /* ////////////////////////////////////////////////////////////////////// */
    /* pre mpi init sampling loop */
    /* ////////////////////////////////////////////////////////////////////// */
    for (i = 0; i < MMU_NUM_SAMPLES; ++i) {
        /* all processes participate here ... update process mem usage */
        if (MMU_SUCCESS != update_mem_info(&process_info,
                                           MEM_TYPE_PROC,
                                           proc_mem_usage.mem_vals)) {
            MMU_ERR_MSG("unable to update proc memory usage info\n");
            goto error;
        }

        for (j = 0; j < MMU_NUM_STATUS_VARS; ++j) {
            /* record pre mpi process sample values */
            proc_mem_usage.pre_mpi_init_samples[j][i] =
                proc_mem_usage.mem_vals[j];
        }

        usleep((unsigned long)MMU_SLEEPY_TIME);
    }

    /* init mpi, etc. */
    if (MMU_SUCCESS != init_mpi(argc, argv)) {
        MMU_ERR_MSG("mpi init error\n");
        goto error;
    }

    /* why not set the dummy send buff to my rank */
    send_buff = my_rank;

    /**
     * make sure numpe a multiple of MMU_PPN.
     * idea: one rank process per node will calculate node memory usage.
     * ASSUMING: processes are placed in rank order.
     */
    if (0 != (num_ranks % MMU_PPN)) {
        MMU_MPF("numpe must be a multiple of %d\n", (int)MMU_PPN);
        goto finil;
    }

    /* let the "master process" print out some header stuff */
    MMU_MPF("# %s %s\n", PACKAGE, PACKAGE_VERSION);
    MMU_MPF("# host %s\n", hostname_buff);
    MMU_MPF("# date_time %s\n", start_time_buff);
    MMU_MPF("# ppn %d \n", (int)MMU_PPN);
    MMU_MPF("# numpe %d\n", num_ranks);
    MMU_MPF("# with_send_recv %d\n", MMU_DO_SEND_RECV);
    MMU_MPF("# num_samples %d \n", (int)MMU_NUM_SAMPLES);
    MMU_MPF("# samples/s %.1lf \n", 1e6/(double)MMU_SLEEPY_TIME);
    MMU_MPF("# item_name, ave_min (kB), ave_max (kB), ave (kB)\n");

    /* ////////////////////////////////////////////////////////////////////// */
    /* main sampling loop */
    /* ////////////////////////////////////////////////////////////////////// */
    for (i = 0; i < MMU_NUM_SAMPLES; ++i) {
        /* make sure everyone is participating in a global collective */
        mpi_ret_code = MPI_Allreduce(&send_buff, &recv_buff, 1, MPI_INT,
                                     MPI_MAX, MPI_COMM_WORLD);
        MMU_MPICHK(mpi_ret_code, error);

        /**
         * when enabled, also do a send recv ring.
         * idea: allocate more buffs to more closely emulate a real app.
         */
#if MMU_DO_SEND_RECV == 1
        do_send_recv_ring();
#endif
        /* do i need to do some real work? */
        if (1 == my_color) {
            /* if so, update node memory usage */
            if (MMU_SUCCESS != update_mem_info(&process_info,
                                               MEM_TYPE_NODE,
                                               node_mem_usage.mem_vals)) {
                MMU_ERR_MSG("unable to update node memory usage info\n");
                goto error;
            }
            for (j = 0; j < MMU_MEM_INFO_LEN; ++j) {
                /* record local node sample values */
                node_mem_usage.samples[j][i] = node_mem_usage.mem_vals[j];
            }
        }

        /* all processes participate here ... update process mem usage */
        if (MMU_SUCCESS != update_mem_info(&process_info,
                                           MEM_TYPE_PROC,
                                           proc_mem_usage.mem_vals)) {
            MMU_ERR_MSG("unable to update proc memory usage info\n");
            goto error;
        }

        for (j = 0; j < MMU_NUM_STATUS_VARS; ++j) {
            /* record process sample values */
            proc_mem_usage.samples[j][i] = proc_mem_usage.mem_vals[j];
        }

        usleep((unsigned long)MMU_SLEEPY_TIME);
    }
    /* ////////////////////////////////////////////////////////////////////// */
    /* end of main sampling loop */
    /* ////////////////////////////////////////////////////////////////////// */

    if (1 == my_color) {
        /* calculate local values (node min, node max, node ave) */
        if (MMU_SUCCESS != get_local_mma(node_mem_usage.samples,
                                         MMU_MEM_INFO_LEN,
                                         &node_mem_usage.min_sample_values,
                                         &node_mem_usage.max_sample_values,
                                         &node_mem_usage.sample_aves)) {
            MMU_ERR_MSG("get_local_mma error\n");
            goto error;
        }

        /* calculate global values (node min, node max, node ave) */
        if (MMU_SUCCESS != get_global_mma(&node_mem_usage.min_sample_values,
                                              &node_mem_usage.max_sample_values,
                                              &node_mem_usage.sample_aves,
                                              MMU_MEM_INFO_LEN,
                                              &node_mem_usage.min_sample_aves,
                                              &node_mem_usage.max_sample_aves,
                                              worker_comm,
                                              num_workers)) {
            MMU_ERR_MSG("get_global_mma error\n");
            goto error;
        }
    }

    /* calculate pre mpi init local values (proc min, proc max, proc ave) */
    if (MMU_SUCCESS != get_local_mma(proc_mem_usage.pre_mpi_init_samples,
                                         MMU_NUM_STATUS_VARS,
                                         &proc_mem_usage.min_sample_values,
                                         &proc_mem_usage.max_sample_values,
                                         &proc_mem_usage.sample_aves)) {
        MMU_ERR_MSG("get_local_mma error\n");
        goto error;
    }
    /* calculate pre mpi init global values (proc min, proc max, proc ave) */
    if (MMU_SUCCESS != get_global_mma(&proc_mem_usage.min_sample_values,
                                          &proc_mem_usage.max_sample_values,
                                          &proc_mem_usage.sample_aves,
                                          MMU_NUM_STATUS_VARS,
                                          &proc_mem_usage.min_sample_aves,
                                          &proc_mem_usage.max_sample_aves,
                                          MPI_COMM_WORLD,
                                          num_ranks)) {
        MMU_ERR_MSG("get_global_mma error\n");
        goto error;
    }

    /* print pre mpi init results */
    if (MMU_MASTER_RANK == my_rank) {
        for (i = 0; i < MMU_NUM_STATUS_VARS; ++i) {
            MMU_MPF(MMU_PMPI_PREFIX"%s, %.2lf, %.2lf, %.2lf\n",
                        status_name_list[i],
                        proc_mem_usage.min_sample_aves[i],
                        proc_mem_usage.max_sample_aves[i],
                        proc_mem_usage.sample_aves[i]);
        }
    }
    /* ////////////////////////////////////////////////////////////////////// */
    /* post mpi init stuff */
    /* ////////////////////////////////////////////////////////////////////// */
    if (MMU_SUCCESS != get_local_mma(proc_mem_usage.samples,
                                         MMU_NUM_STATUS_VARS,
                                         &proc_mem_usage.min_sample_values,
                                         &proc_mem_usage.max_sample_values,
                                         &proc_mem_usage.sample_aves)) {
        MMU_ERR_MSG("get_local_mma error\n");
        goto error;
    }

    /* calculate global values (proc min, proc max, proc ave) */
    if (MMU_SUCCESS != get_global_mma(&proc_mem_usage.min_sample_values,
                                      &proc_mem_usage.max_sample_values,
                                      &proc_mem_usage.sample_aves,
                                      MMU_NUM_STATUS_VARS,
                                      &proc_mem_usage.min_sample_aves,
                                      &proc_mem_usage.max_sample_aves,
                                      MPI_COMM_WORLD,
                                      num_ranks)) {
        MMU_ERR_MSG("get_global_mma error\n");
        goto error;
    }

    /* print the results */
    if (MMU_MASTER_RANK == my_rank) {
        for (i = 0; i < MMU_NUM_STATUS_VARS; ++i) {
            MMU_MPF("%s, %.2lf, %.2lf, %.2lf\n",
                        status_name_list[i],
                        proc_mem_usage.min_sample_aves[i],
                        proc_mem_usage.max_sample_aves[i],
                        proc_mem_usage.sample_aves[i]);
        }
        for (i = 0; i < MMU_MEM_INFO_LEN; ++i) {
            MMU_MPF("%s, %.2lf, %.2lf, %.2lf\n",
                        meminfo_name_list[i],
                        node_mem_usage.min_sample_aves[i],
                        node_mem_usage.max_sample_aves[i],
                        node_mem_usage.sample_aves[i]);
        }
    }

    /* done! */
    if (MMU_SUCCESS != fini_mpi()) {
        MMU_ERR_MSG("mpi finalization error\n");
        goto error;
    }

    if (MMU_SUCCESS != fini()) {
        MMU_ERR_MSG("finalization error\n");
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
