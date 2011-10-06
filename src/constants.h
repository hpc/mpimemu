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

#ifndef CONSTANTS_INCLUDED
#define CONSTANTS_INCLUDED

/* return codes used for internal purposes */
enum {
    /* general success return code */
    MMU_SUCCESS = 0,
    /* general failure return code */
    MMU_FAILURE,
    /* general mpi failure return code */
    MMU_FAILURE_MPI,
    /* out of resources failure return code */
    MMU_FAILURE_OOR
} mpimemu_ret;

/* memory query types */
typedef enum {
    MEM_TYPE_NODE = 0,
    MEM_TYPE_PROC
} mem_info_type_t;

/* local reduction operations */
typedef enum {
    LOCAL_MIN = 0,
    LOCAL_MAX,
    LOCAL_SUM
} local_reduction_ops;

#endif /* ifndef CONSTANTS_INCLUDED */
