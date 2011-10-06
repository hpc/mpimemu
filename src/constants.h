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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

/* TODO - this shoud be a run-time parameter */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* target PPN - USER CHANGE BELOW TO MATCH TARGET MACHINE                     */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#define MMU_PPN 2

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* TODO rename to "simulated workload" or something like that */
/* do_send_recv_ring on by default                                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef MMU_DO_SEND_RECV
    #define MMU_DO_SEND_RECV 1
#elif defined MMU_DO_SEND_RECV && (MMU_DO_SEND_RECV) <= 0
    #undef MMU_DO_SEND_RECVV
    #define MMU_DO_SEND_RECV 0
#endif

/* "master" rank */
#define MMU_MASTER_RANK  0
/* pre mpi init prefix */
#define MMU_PMPI_PREFIX    "Pre_MPI_Init_"
/* file containing node memory usage information */
#define MMU_MEMINFO_FILE   "/proc/meminfo"
/* template for proc memory usage information */
#define MMU_PMEMINFO_TMPLT "/proc/%d/status"
/* line buffer max */
#define MMU_LINE_MAX        128
/* key value max length */
#define MMU_KEY_LEN_MAX     32
/* start time string max length */
#define MMU_TIME_STR_MAX    16
/* at most 10 samples/second (in microseconds) */
#define MMU_SLEEPY_TIME     100000
#define MMU_FAILURE         0
/* success return code */
#define MMU_SUCCESS         1
/* number of items we are recording for node mem info */
#define MMU_MEM_INFO_LEN    13
/* number of items we are recording for proc mem info */
#define MMU_NUM_STATUS_VARS 10
/* number of samples */
#define MMU_NUM_SAMPLES     100
/* send recv buff size */
#define MMU_SR_BUFF_SZ      1024
/* invalid key index value */
#define MMU_INVLD_KEY_INDX  -1
/* path buffer max */
#define MMU_PATH_MAX        1024
/* number of memory types */
#define MMU_NUM_MEM_TYPES   2

#endif /* ifndef CONSTANTS_INCLUDED */
