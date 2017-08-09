/**
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include <iostream>
#include "mpi.h"

namespace {

struct MPIContext {
    int rank;
    int numpe;
};

MPIContext mpictx;

} // namespace

/**
 *
 */
int
MPI_Init(
    int *argc,
    char ***argv
) {
    int rc = PMPI_Init(argc, argv);

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
    return PMPI_Finalize();
}
