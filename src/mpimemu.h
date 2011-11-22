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

#ifndef MPIMEMU_INCLUDED
#define MPIMEMU_INCLUDED

#include "constants.h"
#include "memory_usage.h"
#include "mpi.h"

/* mem info stuff */
typedef struct mem_info_t {
    const char **index_name_ptr;
    int num_elements;
} mem_info_t;

/* container for mpi-related information */
typedef struct mpi_info_t {
    /* my rank */
    int rank;
    /* number of ranks in MPI_COMM_WORLD */
    int num_ranks;
    /* 0 (collects node mem info) or 1 (performs dummy collectives) */
    int worker_id;
    /* number of processes that are doing work */
    int num_workers;
    /* worker communicator */
    MPI_Comm worker_comm;
} mpi_info_t;

typedef struct process_info_t {
    /* start time buffer */
    char *start_time_buf;
    /* holds host's name */
    char *hostname_buf;
    /* my pid */
    pid_t pid;
    mpi_info_t mpi;
} process_info_t;

/* mem info array
 * items should following the ordering specified by mem_info_type_t
 */
static mem_info_t mem_info[MMU_NUM_MEM_TYPES] = {
    /* node */
    {meminfo_name_list,  MMU_MEM_INFO_LEN},
    /* proc */
    {status_name_list, MMU_NUM_STATUS_VARS}
};

static int
init(process_info_t **proc_infop,
     mmu_mem_usage_container_t **node_mem_usagep,
     mmu_mem_usage_container_t **proc_mem_usagep);

static int
init_mpi(process_info_t *proc_infop,
         int argc,
         char **argv);

static int
fini(process_info_t **proc_infop,
     mmu_mem_usage_container_t **node_mem_usagep,
     mmu_mem_usage_container_t **proc_mem_usagep);

static int
fini_mpi(process_info_t *proc_infop);

static int
set_mem_info(int mem_info_type,
             unsigned long int *mem_vals,
             const char *mem_info_str);

static int
update_mem_info(process_info_t *proc_infop,
                int mem_info_type,
                unsigned long int *mem_vals);

static int
reduce_local(const unsigned long int *in_vec,
             unsigned long int *out,
             int in_vec_len,
             int op);

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

#if MMU_DO_SEND_RECV == 1
static int
do_send_recv_ring(process_info_t *proc_infop);
#endif

#endif /* ifndef MPIMEMU_INCLUDED */
