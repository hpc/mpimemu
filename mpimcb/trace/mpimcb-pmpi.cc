/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-rt.h"
#include "mpimcb-mem-stat-mgr.h"

#include <signal.h>

#include "mpi.h"

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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Init(argc, argv);
    rt->deactivate_all_mem_hooks();
    // Reset any signal handlers that may have been set in MPI_Init.
    (void)signal(SIGSEGV, SIG_DFL);
    // For tool purposes, so don't track.
    PMPI_Comm_rank(MPI_COMM_WORLD, &rt->rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &rt->numpe);
    //
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Irecv(
        buf,
        count,
        datatype,
        source,
        tag,
        comm,
        request
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Send(
        buf,
        count,
        datatype,
        dest,
        tag,
        comm
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Isend(
    const void *buf,
    int count,
    MPI_Datatype datatype,
    int dest,
    int tag,
    MPI_Comm comm,
    MPI_Request *request
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Isend(
        buf,
        count,
        datatype,
        dest,
        tag,
        comm,
        request
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Sendrecv(
    const void *sendbuf,
    int sendcount,
    MPI_Datatype sendtype,
    int dest,
    int sendtag, 
    void *recvbuf, 
    int recvcount,
    MPI_Datatype recvtype,
    int source,
    int recvtag,
    MPI_Comm comm,
    MPI_Status *status
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Sendrecv(
        sendbuf,
        sendcount,
        sendtype,
        dest,
        sendtag, 
        recvbuf, 
        recvcount,
        recvtype,
        source,
        recvtag,
        comm,
        status
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Wait(
        request,
        status
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Waitall(
    int count,
    MPI_Request array_of_requests[],
    MPI_Status *array_of_statuses
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Waitall(
        count,
        array_of_requests,
        array_of_statuses
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Comm_size(
        comm,
        size
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Comm_rank(
        comm,
        rank
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Barrier(
        comm
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Allreduce(
        sendbuf,
        recvbuf,
        count,
        datatype,
        op,
        comm
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Bcast(
        buffer,
        count,
        datatype,
        root,
        comm
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Reduce(
    const void *sendbuf,
    void *recvbuf,
    int count,
    MPI_Datatype datatype,
    MPI_Op op,
    int root,
    MPI_Comm comm
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Reduce(
        sendbuf,
        recvbuf,
        count,
        datatype,
        op,
        root,
        comm
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Abort(
        comm,
        errorcode
    );
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    rt->deactivate_all_mem_hooks();
    //
    PMPI_Barrier(MPI_COMM_WORLD);
    //
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->report(rt->rank, true);
    //
    return PMPI_Finalize();
}
