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

#ifndef CONV_MACROS_INCLUDED
#define CONV_MACROS_INCLUDED 

/* ////////////////////////////////////////////////////////////////////////// */
/* convenience macros                                                         */
/* ////////////////////////////////////////////////////////////////////////// */

#define MMU_STRINGIFY(x) #x
#define MMU_TOSTRING(x)  MMU_STRINGIFY(x)

#define MMU_ERR_AT       __FILE__ " ("MMU_TOSTRING(__LINE__)")"
#define MMU_ERR_PREFIX   "-[MPIMEMU ERROR: "MMU_ERR_AT"]- "

/* error message */
#define MMU_ERR_MSG(pfargs...)                                                 \
do {                                                                           \
    fprintf(stderr, MMU_ERR_PREFIX);                                           \
    fprintf(stderr, pfargs);                                                   \
} while (0)

/* mpi check */
#define MMU_MPICHK(_ret_,_gt_)                                                 \
do {                                                                           \
    if (MPI_SUCCESS != (_ret_)) {                                              \
        char err_str[MPI_MAX_ERROR_STRING];                                    \
        int err_str_len;                                                       \
        MPI_Error_string((_ret_),                                              \
                         err_str,                                              \
                         &err_str_len);                                        \
        MMU_ERR_MSG("mpi success not returned... %s (errno: %d)\n",            \
                        err_str,                                               \
                        (_ret_));                                              \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

#define MMU_OOR_COMPLAIN()                                                     \
do {                                                                           \
    fprintf(stderr, MMU_ERR_PREFIX "out of resources\n");                      \
    fflush(stderr);                                                            \
} while (0)

/* memory alloc check */
#define MMU_MEMCHK(_ptr_,_gt_)                                                 \
do {                                                                           \
    if (NULL == (_ptr_)) {                                                     \
        MMU_ERR_MSG("memory allocation error on %s\n", hostname_buff);         \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

/* printf with flush */
#define MMU_PF(pfargs...)                                                      \
do {                                                                           \
    fprintf(stdout, pfargs);                                                   \
    fflush(stdout);                                                            \
} while (0)

/* master rank printf */
#define MMU_MPF(pfargs...)                                                     \
do {                                                                           \
    if (my_rank == (MMU_MASTER_RANK)) {                                        \
        fprintf(stdout, pfargs);                                               \
        fflush(stdout);                                                        \
    }                                                                          \
} while (0)

#endif /* ifndef CONV_MACROS_INCLUDED */
