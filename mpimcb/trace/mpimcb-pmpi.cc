/**
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-common.h"
#include "mpimcb-mem-hook-state.h"

#include "mpi.h"

#include <iostream>

namespace {

struct mmcb_mpi_context {
    int rank;
    int numpe;
};

} // namespace

mmcb_mpi_context mmcb_mpictx;

mmcb_mem_hook_mgr_t mmcb_mem_hook_mgr;

/**
 *
 */
int
MPI_Init(
    int *argc,
    char ***argv
) {
    int rc = PMPI_Init(argc, argv);

    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);

    PMPI_Comm_rank(MPI_COMM_WORLD, &mmcb_mpictx.rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &mmcb_mpictx.numpe);

    return rc;
}

/**
 *
 */
int
MPI_Finalize(void)
{
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    if (mmcb_mpictx.rank == 0) {
        dump();
    }
    return PMPI_Finalize();
}
