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

#ifndef MMU_CONV_MACROS_INCLUDED
#define MMU_CONV_MACROS_INCLUDED 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

/* ////////////////////////////////////////////////////////////////////////// */
/* convenience macros                                                         */
/* ////////////////////////////////////////////////////////////////////////// */

#define MMU_STRINGIFY(x) #x
#define MMU_TOSTRING(x)  MMU_STRINGIFY(x)

#define MMU_ERR_AT       __FILE__ ": "MMU_TOSTRING(__LINE__)""
#define MMU_ERR_PREFIX   "-["PACKAGE" ERROR: "MMU_ERR_AT"]- "
#define MMU_WARN_PREFIX  "-["PACKAGE" WARNING]- "

#define MMU_OOR_COMPLAIN()                                                     \
do {                                                                           \
    fprintf(stderr, MMU_ERR_PREFIX "out of resources\n");                      \
    fflush(stderr);                                                            \
} while (0)

#endif /* ifndef MMU_CONV_MACROS_INCLUDED */
