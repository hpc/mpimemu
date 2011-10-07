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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <errno.h>

#include "constants.h"
#include "util.h"

/* ////////////////////////////////////////////////////////////////////////// */
double *
lfcalloc(size_t count)
{
    return (double *)calloc(count, sizeof(double));
}

/* ////////////////////////////////////////////////////////////////////////// */
unsigned long int *
lucalloc(size_t count)
{
    return (unsigned long int *)calloc(count, sizeof(unsigned long int));
}

/* ////////////////////////////////////////////////////////////////////////// */
unsigned long int **
lupcalloc(size_t count)
{
    return (unsigned long int **)calloc(count, sizeof(unsigned long int *));
}

/* ////////////////////////////////////////////////////////////////////////// */
/* strtoul with error checks - yeah, i said it */
unsigned long int
strtoul_wec(const char *nptr,
            char **endptr,
            int base,
            int *ret_code)
{
    char *end_ptr = NULL;
    unsigned long int value;

    /* assume all is well */
    *ret_code = MMU_SUCCESS;

    /* check for strtoul errors */
    errno = 0;
    value = strtoul(nptr, endptr, 10);

    if ((ERANGE == errno && (ULONG_MAX == value || 0 == value)) ||
        (0 != errno && 0 == value)) {
        *ret_code = MMU_FAILURE;
    }
    if (nptr == end_ptr) {
        *ret_code = MMU_FAILURE;
    }

    /* caller must always check the return code */
    return value;
}
