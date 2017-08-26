/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-timer.h"

#include "mpi.h"

/**
 *
 */
double
mmcb_time(void)
{
    return PMPI_Wtime();
}
