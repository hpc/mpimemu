/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
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
