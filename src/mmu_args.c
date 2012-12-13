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

/**
 * @author Samuel K. Gutierrez - samuel@lanl.gov
 * found a bug? have an idea? please let me know.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmu_constants.h"
#include "mmu_conv_macros.h"
#include "mmu_args.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

const char *opt_string = "s:t:whv";

static struct option long_opts[] = {
    {"samples-per-sec", required_argument, NULL, 's'},
    {"sample-time",     required_argument, NULL, 't'},
    {"enable-workload", no_argument,       NULL, 'w'},
    {"help",            no_argument,       NULL, 'h'},
    {"version",         no_argument,       NULL, 'v'},
    {NULL,              0,                 NULL,  0 }
};

/* ////////////////////////////////////////////////////////////////////////// */
static inline void
set_arg_defaults(mmu_args_t *a)
{
    a->sample_rate = MMU_DEFAULT_SAMPLE_RATE;
    a->sample_time = MMU_DEFAULT_SAMPLE_TIME;
    a->enable_workload = false;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_args_construct(mmu_args_t **a)
{
    mmu_args_t *tmp = NULL;

    if (NULL == a) return MMU_FAILURE_INVALID_ARG;

    tmp = (mmu_args_t *)calloc(1, sizeof(*tmp));
    if (NULL == tmp) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    set_arg_defaults(tmp);
    *a = tmp;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_args_destruct(mmu_args_t **a)
{
    mmu_args_t *tmp = NULL;

    if (NULL == a) return MMU_FAILURE_INVALID_ARG;

    tmp = *a;
    if (NULL != tmp) {
        free(tmp);
        tmp = NULL;
    }
    *a = NULL;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
mmu_args_free_dup_argv(char **dup)
{
    char **str = NULL;

    if (NULL == dup) return;

    for (str = dup; NULL != *str; ++str) {
        free(*str);
        *str = NULL;
    }
    free(dup);
}

/* ////////////////////////////////////////////////////////////////////////// */
char **
mmu_args_dup_argv(int argc, char **argv)
{
    int i, str_len;
    char **dup = NULL;

    /* allocate an extra spot for cap */
    dup = (char **)calloc(argc + 1, sizeof(char *));
    if (NULL == dup) {
        MMU_OOR_COMPLAIN();
        return NULL;
    }
    for (i = 0; i < argc; ++i) {
        str_len = strlen(argv[i]) + 1;
        dup[i] = (char *)calloc(str_len, sizeof(char *));
        if (NULL == dup[i]) {
            MMU_OOR_COMPLAIN();
            mmu_args_free_dup_argv(dup);
            return NULL;
        }
        (void)memmove(dup[i], argv[i], str_len);
    }
    dup[i] = NULL;
    return dup;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
mmu_args_usage(void)
{
    char *usage =
    "\nUsage:\n"
    "[mpirun|aprun|srun|etc] "PACKAGE" [OPTIONS]\n"
    "Options:\n"
    "[-s|--samples-per-sec X] "
    "Specifies sampling rate (default: 10)\n"
    "[-t|--sample-time X]     "
    "Specifies sampling duration in seconds (default: 10)\n"
    "[-w|--enable-workload]   "
    "Enable synthetic MPI communication workload (default: disabled)\n"
    "[-h|--help]              "
    "Show this message and exit\n"
    "[-v|--version]           "
    "Show version and exit\n";

    fprintf(stdout, "%s", usage);
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
showver(void)
{
    fprintf(stdout, "%s\n", PACKAGE_STRING);
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
get_long_index(int val)
{
    int i;
    for (i = 0; NULL != long_opts[i].name; ++i) {
        if (val == long_opts[i].val) {
            return i;
        }
    }
    /* didn't find val */
    return -1;
}

/* ////////////////////////////////////////////////////////////////////////// */
static inline int
validate_int(int val,
             const char *s,
             int base,
             int *val_if_valid,
             char **err_str)
{
    bool want_err_str = true;
    int err = 0, rc = MMU_SUCCESS, tmp_val = 0;
    char *end_ptr = NULL;
    char *tmp_err = NULL;
    static char err_buf[128];

    *val_if_valid = 0;

    /* the caller doesn't want the error string */
    if (NULL == err_str) {
        want_err_str = false;
    }

    errno = 0;
    tmp_val = (int)strtol(s, &end_ptr, base);
    err = errno;
    /* no digits found. note that err will be 0 here */
    if (end_ptr == s) {
        rc = MMU_FAILURE_INVALID_USER_INPUT;
        tmp_err = "integer value required, but not found";
        goto out;
    }
    /* some other type of error occurred */
    if (0 != err) {
        rc = MMU_FAILURE;
        tmp_err = strerror(err);
        goto out;
    }
    /* all is well */
    *val_if_valid = tmp_val;

out:
    /* an error occurred and the caller wants the error string */
    if (want_err_str && NULL != tmp_err) {
        int target_index = get_long_index(val);
        /* if we are here, then this should never happen */
        if (-1 == target_index) {
            /* how did this happen? */
            tmp_err = "this is embarrassing: target_index not found";
        }
        snprintf(err_buf, sizeof(err_buf) - 1,
                 "user input error: option --%s (-%c): %s.",
                 long_opts[target_index].name, long_opts[target_index].val,
                 tmp_err);
        *err_str = err_buf;
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* sample rate */
static inline int
handle_s(mmu_args_t *a,
         int val,
         const char *opt_arg)
{
    int tmp, rc;
    char *err_str = NULL;

    if (MMU_SUCCESS != (rc = validate_int(val, opt_arg, 10, &tmp, &err_str))) {
        fprintf(stderr, "%s\n", err_str);
        return rc;
    }

    a->sample_rate = tmp;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* sampling time */
static inline int
handle_t(mmu_args_t *a,
         int val,
         const char *opt_arg)
{
    int tmp, rc;
    char *err_str = NULL;

    if (MMU_SUCCESS != (rc = validate_int(val, opt_arg, 10, &tmp, &err_str))) {
        fprintf(stderr, "%s\n", err_str);
        return rc;
    }
    a->sample_time = tmp;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* enable synthetic mpi workload */
static inline int
handle_w(mmu_args_t *a)
{
    a->enable_workload = true;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_args_process_user_input(mmu_args_t *a,
                            int argc,
                            char **argv)
{
    int c;
    int rc = MMU_FAILURE;
    char **tmp_argv = NULL;

    if (NULL == (tmp_argv = mmu_args_dup_argv(argc, argv))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    while (-1 != (c = getopt_long_only(argc, tmp_argv, opt_string, long_opts,
                                       NULL))) {
        switch (c) {
            case 's': /* samples-per-sec */
                if (MMU_SUCCESS != (rc = handle_s(a, c, optarg))) {
                    goto show_usage;
                }
                break;

            case 't': /* sample-time */
                if (MMU_SUCCESS != (rc = handle_t(a, c, optarg))) {
                    goto show_usage;
                }
                break;
            case 'w': /* enable workload */
                if (MMU_SUCCESS != (rc = handle_w(a))) {
                    goto show_usage;
                }
                break;

            case 'h': /* help! */
                mmu_args_usage();
                rc = MMU_SUCCESS_EXIT_SUCCESS;
                goto cleanup;
                break;

            case 'v': /* version, please */
                showver();
                rc = MMU_SUCCESS_EXIT_SUCCESS;
                goto cleanup;
                break;

            default:
                goto show_usage;
        }
    }
    if (optind < argc) {
        fprintf(stderr, MMU_ERR_PREFIX"unrecognized input: \"%s\"\n",
                tmp_argv[optind]);
        goto show_usage;
    }

    /* all is well */
    rc = MMU_SUCCESS;
    goto cleanup;

show_usage:
    mmu_args_usage();
    rc = MMU_FAILURE_INVALID_USER_INPUT;

cleanup:
    if (NULL != tmp_argv) {
        mmu_args_free_dup_argv(tmp_argv);
    }
    return rc;
}
