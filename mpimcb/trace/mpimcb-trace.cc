/**
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-common.h"

#include "mpi.h"

#include <iostream>

namespace {

struct mmcb_mpi_context {
    int rank;
    int numpe;
};

} // namespace

mmcb_mpi_context mpictx;

extern int mmcb_malloc_hook_active;

/**
 *
 */
int
MPI_Init(
    int *argc,
    char ***argv
) {
    int rc = PMPI_Init(argc, argv);

    mmcb_malloc_hook_active = 1;

    PMPI_Comm_rank(MPI_COMM_WORLD, &mpictx.rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &mpictx.numpe);

    return rc;
}

/**
 *
 */
int
MPI_Finalize(void)
{
    mmcb_malloc_hook_active = 0;
    if (mpictx.rank == 0) {
        dump();
    }
    return PMPI_Finalize();
}
