/**
 * Copyright (c) 2012-2013 Los Alamos National Security, LLC.
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

#ifndef MMU_MEMORY_COMMON_INCLUDED
#define MMU_MEMORY_COMMON_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

typedef uint16_t mmu_memory_flags_t;

/* number of elements in proc_self_tab - not including terminator */
#define MMU_MEMORY_PROC_SELF_TAB_LEN 10
/* number of elements in proc_node_tab - not including terminator */
#define MMU_MEMORY_PROC_NODE_TAB_LEN 13
/* must be max(MMU_MEMORY_PROC_SELF_TAB_LEN, MMU_MEMORY_PROC_NODE_TAB_LEN) */ 
#define MMU_MEMORY_SAMP_LIST_LEN MMU_MEMORY_PROC_NODE_TAB_LEN

/* /// sample types /// */
/* self */
#define MMU_MEMORY_FLAGS_SAMPLE_SELF              0x0001
#define MMU_MEMORY_FLAGS_SAMPLE_SELF_BOGUS        0x0002
/* node */
#define MMU_MEMORY_FLAGS_SAMPLE_NODE              0x0010
#define MMU_MEMORY_FLAGS_SAMPLE_NODE_BOGUS        0x0020
/* /// stage types /// */
#define MMU_MEMORY_FLAGS_SAMPLE_PRE_MPI_INIT      0x0100

#define MMU_MEMORY_SAMPLE_SELF(flags)                                          \
    ( (flags) & MMU_MEMORY_FLAGS_SAMPLE_SELF )

#define MMU_MEMORY_SAMPLE_SELF_BOGUS(flags)                                    \
    ( (flags) & MMU_MEMORY_FLAGS_SAMPLE_SELF_BOGUS )

#define MMU_MEMORY_SAMPLE_NODE(flags)                                          \
    ( (flags) & MMU_MEMORY_FLAGS_SAMPLE_NODE )

#define MMU_MEMORY_SAMPLE_NODE_BOGUS(flags)                                    \
    ( (flags) & MMU_MEMORY_FLAGS_SAMPLE_NODE_BOGUS )

#define MMU_MEMORY_SAMPLE_PRE_MPI_INIT(flags)                                  \
( (flags) & MMU_MEMORY_FLAGS_SAMPLE_PRE_MPI_INIT )

/* masks */
#define MMU_MEMORY_TYPE_SELF_MASK 0x000F
#define MMU_MEMORY_TYPE_NODE_MASK 0x00F0
#define MMU_MEMORY_TYPE_MASK      0x00FF
#define MMU_MEMORY_STAGE_MASK     0xFF00

typedef enum {
    MMU_MEMORY_SELF_PRE_INIT = 0,
    MMU_MEMORY_SELF_POST_INIT,
    MMU_MEMORY_NODE_PRE_INIT,
    MMU_MEMORY_NODE_POST_INIT,
    MMU_MEMORY_LAST
} mmu_memory_list_type_t;

/* holds min, max, and average sample values */
typedef struct mmu_memory_sample_mma_t {
    /* the length of all the lists - note that
     * all of the lists MUST be the same length! */
    size_t lists_length;
    /* points to list of minimum values */
    unsigned long long int *mins;
    /* points to list of maximum values */
    unsigned long long int *maxes;
    /* points to totals used to calculate averages */
    unsigned long long int *totals;
    /* points to list of averages */
    double *aves;
} mmu_memory_sample_mma_t;

#endif /* ifndef MMU_MEMORY_COMMON_INCLUDED */
