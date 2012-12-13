/**
 * Copyright (c) 2010-2012 Los Alamos National Security, LLC.
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

#include "mmu_constants.h"
#include "mmu_conv_macros.h"
#include "mmu_util.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_util_str_to_ull(const char *str,
                    unsigned long long int *val_if_valid)
{
    char *end_ptr = NULL, *tmp_ptr = NULL;
    int err = 0;
    unsigned long long int val;

    if (NULL == str || NULL == val_if_valid) return MMU_FAILURE_INVALID_ARG;

    *val_if_valid = 0;
    tmp_ptr = (char *)str; 
    
    /* make certain str contrains only digits */
    while ('\0' != *tmp_ptr) {
        if (!isdigit(*tmp_ptr)) {
            fprintf(stderr,
                    MMU_ERR_PREFIX"mmu_util_str_to_ull error: non-digit "
                    "found in input %s.\n", str);
            return MMU_FAILURE_INVALID_ARG;
        }
        ++tmp_ptr;
    }
    errno = 0;
    val = strtoull(str, &end_ptr, 10);
    err = errno;
    /* did we get any digits? */
    if (str == end_ptr) {
        fprintf(stderr,
                MMU_ERR_PREFIX"mmu_util_str_to_ull error: no digits.\n");
        return MMU_FAILURE_INVALID_ARG;
    }
    if (0 != err) {
        fprintf(stderr, MMU_ERR_PREFIX"strtoull error: %s (errno: %d).\n",
                strerror(err), err);
        return MMU_FAILURE;
    }

    *val_if_valid = val;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
const char *
mmu_util_rc2str(int rc)
{
    static char str_buf[64];
    char *tmp = NULL;

    switch (rc) {
        case MMU_SUCCESS:
            tmp = "success";
            break;
        case MMU_FAILURE:
            tmp = "failure";
            break;
        case MMU_FAILURE_MPI:
            tmp = "mpi failure";
            break;
        case MMU_FAILURE_IO:
            tmp = "io failure";
            break;
        case MMU_FAILURE_OOR:
            tmp = "out of resources";
            break;
        case MMU_FAILURE_INVALID_ARG:
            tmp = "invalid argument";
            break;
        case MMU_FAILURE_OVERFLOW:
            tmp = "overflow";
            break;
        case MMU_FAILURE_INVALID_USER_INPUT:
            tmp = "invalid user input";
            break;
        case MMU_FAILURE_LIST_POP:
            tmp = "pop attempted on empty list";
            break;
        case MMU_FAILURE_PPN_DIFFERS:
            tmp = "number of mpi processes per node differs";
            break;
        default:
            tmp = "???";
    }
    (void)snprintf(str_buf, sizeof(str_buf), "%s", tmp);

    return str_buf;
}
