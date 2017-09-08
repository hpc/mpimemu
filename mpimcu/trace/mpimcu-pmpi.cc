/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcu-rt.h"

#include <iostream>

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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    // Set init time.
    rt->set_init_begin_time_now();
    //
    mmcu_sample sbefore, safter;
    mmcu_rt_sample(rt, sbefore);
    int rc = PMPI_Init(argc, argv);
    mmcu_rt_sample(rt, safter);
    // Set init end time.
    rt->set_init_end_time_now();
    // Store the results.
    rt->store_sample(sbefore, safter);
    // Reset any signal handlers that may have been set in MPI_Init.
    (void)signal(SIGSEGV, SIG_DFL);
    //
    rt->gather_target_meta();
    PMPI_Comm_rank(MPI_COMM_WORLD, &rt->rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &rt->numpe);
    //
    const int nsyncs = 2;
    for (int i = 0; i < nsyncs; ++i) {
        PMPI_Barrier(MPI_COMM_WORLD);
    }
    // Emit obnoxious header that lets the user know something is happening.
    if (rt->rank == 0) {
        rt->emit_header();
    }
    //
    return rc;
}

#if 0
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
MPI_Recv(
    void *buf,
    int count,
    MPI_Datatype datatype,
    int source,
    int tag,
    MPI_Comm comm,
    MPI_Status *status
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Recv(
        buf,
        count,
        datatype,
        source,
        tag,
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
MPI_Isend(
    const void *buf,
    int count,
    MPI_Datatype datatype,
    int dest,
    int tag,
    MPI_Comm comm,
    MPI_Request *request
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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

/**
 *
 */
int
MPI_Iprobe(
    int source,
    int tag,
    MPI_Comm comm,
    int *flag,
    MPI_Status *status
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Iprobe(
        source,
        tag,
        comm,
        flag,
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
MPI_Issend(
    const void *buf,
    int count,
    MPI_Datatype datatype,
    int dest,
    int tag,
    MPI_Comm comm,
    MPI_Request *request
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Issend(
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
MPI_Ssend(
    const void *buf,
    int count,
    MPI_Datatype datatype,
    int dest,
    int tag,
    MPI_Comm comm
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Ssend(
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Other
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
double
MPI_Wtime(void)
{
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    double res = PMPI_Wtime();
    rt->deactivate_all_mem_hooks();
    //
    return res;
}

/**
 *
 */
int
MPI_Address(
    void *location,
    MPI_Aint *address
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Address(
        location,
        address
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Comm_split(
    MPI_Comm comm,
    int color,
    int key,
    MPI_Comm *newcomm
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Comm_split(
        comm,
        color,
        key,
        newcomm
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Comm_free(
    MPI_Comm *comm
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Comm_free(
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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
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

/**
 *
 */
int
MPI_Type_commit(
    MPI_Datatype *type
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Type_commit(
        type
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Type_free(
    MPI_Datatype *type
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Type_free(
        type
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Type_contiguous(
    int count,
    MPI_Datatype oldtype,
    MPI_Datatype *newtype
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Type_contiguous(
        count,
        oldtype,
        newtype
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Type_struct(
    int count,
    int array_of_blocklengths[],
    MPI_Aint array_of_displacements[],
    MPI_Datatype array_of_types[],
    MPI_Datatype *newtype
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Type_struct(
        count,
        array_of_blocklengths,
        array_of_displacements,
        array_of_types,
        newtype
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}

/**
 *
 */
int
MPI_Type_vector(
    int count,
    int blocklength,
    int stride,
    MPI_Datatype oldtype,
    MPI_Datatype *newtype
) {
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    //
    rt->activate_all_mem_hooks();
    int rc = PMPI_Type_vector(
        count,
        blocklength,
        stride,
        oldtype,
        newtype
    );
    rt->deactivate_all_mem_hooks();
    //
    return rc;
}
#endif

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
    static mmcu_rt *rt = mmcu_rt::the_mmcu_rt();
    // Sync.
    PMPI_Barrier(MPI_COMM_WORLD);
    // Report.
    rt->report();
    //
    return PMPI_Finalize();
}
