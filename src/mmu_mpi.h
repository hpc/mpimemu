/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifndef MMU_MPI_INCLUDED
#define MMU_MPI_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mpi.h"

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#define MMU_MPI_MAX_PROCESSOR_NAME MPI_MAX_PROCESSOR_NAME

typedef struct mmu_mpi_version_t {
    int version;
    int subversion;
} mmu_mpi_version_t;

/* container for mpi-related information */
typedef struct mmu_mpi_t {
    /* mpi library version */
    mmu_mpi_version_t version;
    /* my rank */
    int rank;
    /* number of ranks in MPI_COMM_WORLD */
    int num_ranks;
    /* number of nodes */
    int num_nodes;
    /* my SMP rank */
    int smp_rank;
    /* number of ranks in SMP communicator */
    int num_smp_ranks;
    /* my worker communicator rank */
    int worker_rank;
    /* number of processes that are doing work */
    int num_workers;
    /* flag indicating whether or not the smp comm needs to be freed */
    bool free_smp_comm;
    /* flag indicating whether or not a synthetic communication workload will
     * be used during memory usage collection */
    bool with_comm_workload;
    /* flag indicating whether or not the worker comm needs to be freed */
    bool free_worker_comm;
    /* my host's name */
    char hostname[MMU_MPI_MAX_PROCESSOR_NAME];
    /* smp communicator */
    MPI_Comm smp_comm;
    /* worker communicator */
    MPI_Comm worker_comm;
} mmu_mpi_t;

int
mmu_mpi_construct(mmu_mpi_t **m);

int
mmu_mpi_destruct(mmu_mpi_t **m);

const char *
mmu_mpi_get_version_str(const mmu_mpi_t *m);

int
mmu_mpi_init(mmu_mpi_t *p,
             int argc,
             char **argv);

int
mmu_mpi_finalize(mmu_mpi_t *m);

bool
mmu_mpi_initialized(void);

int
mmu_mpi_allreduce(void *sendbuf,
                  void *recvbuf,
                  int count,
                  MPI_Datatype datatype,
                  MPI_Op op,
                  MPI_Comm comm);

int
mmu_mpi_comm_size(MPI_Comm comm,
                  int *size);
bool
mmu_mpi_world_rank_zero(const mmu_mpi_t *m);

bool
mmu_mpi_smp_rank_zero(const mmu_mpi_t *m);

int
mmu_mpi_get_num_smp_ranks(const mmu_mpi_t *m);

int
mmu_mpi_get_comm_world_size(const mmu_mpi_t *m);

int
mmu_mpi_get_num_nodes(const mmu_mpi_t *m);

bool
mmu_mpi_with_workload(const mmu_mpi_t *m);

int
mmu_mpi_work(const mmu_mpi_t *m);

int
mmu_mpi_enable_workload(mmu_mpi_t *m, bool enable);

#endif /* ifndef MMU_MPI_INCLUDED */
