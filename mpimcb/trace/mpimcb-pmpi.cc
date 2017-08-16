/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"
#include "mpimcb-mem-hook-state.h"
#include "mpimcb-memory.h"

#include "mpi.h"

namespace {

struct mmcb_mpi_context {
    int rank;
    int numpe;
};

} // namespace

mmcb_mpi_context mmcb_mpictx;

mmcb_mem_hook_mgr_t mmcb_mem_hook_mgr;

mmcb_memory mmcb_mem;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
int
MPI_Init(
    int *argc,
    char ***argv
) {
    //
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Init(argc, argv);
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    // For tool purposes, so don't track.
    PMPI_Comm_rank(MPI_COMM_WORLD, &mmcb_mpictx.rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &mmcb_mpictx.numpe);
    // Sync.
    PMPI_Barrier(MPI_COMM_WORLD);
    //
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Point to Point
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
int
MPI_Irecv(
    void *buf,
    int count,
    MPI_Datatype datatype,
    int source,
    int tag,
    MPI_Comm comm,
    MPI_Request *request
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Irecv(
        buf,
        count,
        datatype,
        source,
        tag,
        comm,
        request
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Send(
    const void *buf,
    int count,
    MPI_Datatype datatype,
    int dest,
    int tag,
    MPI_Comm comm
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Send(
        buf,
        count,
        datatype,
        dest,
        tag,
        comm
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Wait(
    MPI_Request *request,
    MPI_Status *status
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Wait(
        request,
        status
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Collective
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
int
MPI_Comm_size(
    MPI_Comm comm,
    int *size
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Comm_size(
        comm,
        size
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Comm_rank(
    MPI_Comm comm,
    int *rank
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Comm_rank(
        comm,
        rank
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Barrier(
    MPI_Comm comm
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Barrier(
        comm
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Allreduce(
    const void *sendbuf,
    void *recvbuf,
    int count,
    MPI_Datatype datatype,
    MPI_Op op,
    MPI_Comm comm
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Allreduce(
        sendbuf,
        recvbuf,
        count,
        datatype,
        op,
        comm
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Bcast(
    void *buffer,
    int count,
    MPI_Datatype datatype,
    int root,
    MPI_Comm comm
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Bcast(
        buffer,
        count,
        datatype,
        root,
        comm
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

/**
 *
 */
int
MPI_Abort(
    MPI_Comm comm,
    int errorcode
) {
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    int rc = PMPI_Abort(
        comm,
        errorcode
    );
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Finalize
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 *
 */
int
MPI_Finalize(void)
{
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    //
    PMPI_Barrier(MPI_COMM_WORLD);
    //
    if (mmcb_mpictx.rank == 0) {
        mmcb_mem.report();
    }
    //
    return PMPI_Finalize();
}
