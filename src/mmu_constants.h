/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifndef CONSTANTS_INCLUDED
#define CONSTANTS_INCLUDED

/* default sample rate in samples per second */
#define MMU_DEFAULT_SAMPLE_RATE 10
/* default sampling duration in seconds */
#define MMU_DEFAULT_SAMPLE_TIME 10

/* return codes used for internal purposes */
enum {
    /* general success return code */
    MMU_SUCCESS = 0,
    /* for argument processing: like help and version */
    MMU_SUCCESS_EXIT_SUCCESS,
    /* general failure return code */
    MMU_FAILURE,
    /* general mpi failure return code */
    MMU_FAILURE_MPI,
    /* general io operation failure */
    MMU_FAILURE_IO,
    /* out of resources failure return code */
    MMU_FAILURE_OOR,
    /* invalid argument failure return code */
    MMU_FAILURE_INVALID_ARG,
    /* overflow return code */
    MMU_FAILURE_OVERFLOW,
    /* invalid user input return code */
    MMU_FAILURE_INVALID_USER_INPUT,
    /* pop attempt on empty list */
    MMU_FAILURE_LIST_POP,
    /* differing processes per node */
    MMU_FAILURE_PPN_DIFFERS
} mpimemu_ret;

#endif /* ifndef CONSTANTS_INCLUDED */
