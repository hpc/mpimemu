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
#include "mmu_util.h"
#include "mmu_process.h"
#include "mmu_args.h"

#include <stdlib.h>
#include <stdio.h>

/* ////////////////////////////////////////////////////////////////////////// */
/* update a mmu_process_t with the settings contained within mmu_args_t */
static int
mpimemu_update_proc(mmu_process_t *p,
                    mmu_args_t *a)
{
    int rc;
    char *bad_func = NULL;
    mmu_args_t real_settings;

    /* /// get the real settings. do validation, etc. /// */
    if (a->sample_rate < 0) {
        fprintf(stderr, MMU_WARN_PREFIX"negative sample rate requested: "
                        "ignoring and continuing with default rate...\n");
        real_settings.sample_rate = MMU_DEFAULT_SAMPLE_RATE;
    }
    else {
        real_settings.sample_rate = a->sample_rate;
    }

    if (a->sample_time < 0) {
        fprintf(stderr, MMU_WARN_PREFIX"negative sample time requested: "
                        "ignoring and continuing with default time...\n");
        real_settings.sample_time = MMU_DEFAULT_SAMPLE_TIME;
    }
    else {
        real_settings.sample_time = a->sample_time;
    }

    real_settings.enable_workload = a->enable_workload;

    /* /// update the process /// */
    if (MMU_SUCCESS != (rc =
        mmu_process_set_sample_rate(p, real_settings.sample_rate))) {
        bad_func = "mmu_process_set_sample_rate";
        goto out;
    }
    if (MMU_SUCCESS != (rc =
        mmu_process_set_sample_time(p, real_settings.sample_time))) {
        bad_func = "mmu_process_set_sample_time";
        goto out;
    }
    if (MMU_SUCCESS != (rc =
        mmu_process_enable_workload(p, real_settings.enable_workload))) {
        bad_func = "mmu_process_enable_workload";
        goto out;
    }

out:
    if (NULL != bad_func) {
        fprintf(stderr,
                MMU_ERR_PREFIX"%s failure: %d (%s).\n", bad_func, rc,
                mmu_util_rc2str(rc));
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
print_settings(const mmu_process_t *p,
               const mmu_args_t *settings)
{
    if (mmu_process_is_delegate(p)) {
        printf("###\n");
        printf("### %s: %s\n", PACKAGE, PACKAGE_VERSION);
        printf("### date-time: %s\n", p->start_time_buf);
        printf("### mpi version: %s\n", mmu_process_get_mpi_version_str(p));
        printf("### numpe: %d\n", mmu_process_get_world_size(p));
        printf("### num nodes: %d\n", mmu_process_get_num_hoods(p));
        printf("### ppn: %d\n", mmu_process_get_hood_size(p));
        printf("### communication workload enabled: %s\n",
                mmu_process_with_workload(p) ? "yes" : "no");
        printf("### samples/second: %d\n", settings->sample_rate);
        printf("### sampling duration (seconds): %d\n", settings->sample_time);
        printf("###\n");
        fflush(stdout);
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
/* main                                                                       */
/* ////////////////////////////////////////////////////////////////////////// */
int
main(int argc,
     char **argv)
{
    int rc, ec = EXIT_SUCCESS;
    mmu_process_t *process = NULL;
    mmu_args_t *app_settings = NULL;
    char *bad_func = NULL;

    /* construct needed resources */
    if (MMU_SUCCESS != (rc = mmu_process_construct(&process))) {
        bad_func = "mmu_process_construct";
        goto out;
    }
    if (MMU_SUCCESS != (rc = mmu_args_construct(&app_settings))) {
        bad_func = "mmu_args_construct";
        goto out;
    }
    /* process user arguments and set application parameters.  this isn't ideal
     * because it would be best to have only one process do this and send the
     * configuration to the rest of the processes.  the problem is, however,
     * that we need to process users inputs BEFORE mpi_init. this translates
     * into all the processes in the job spewing error messages if the user's
     * input isn't valid (or some other exception occurred during
     * mmu_args_process_user_input). sad face... */
    if (MMU_SUCCESS != (rc = mmu_args_process_user_input(app_settings, argc,
                                                         argv))) {
        /* do not set bad_func here because mmu_args_process_user_input will
         * supply all the output.  we don't want to complain about this type of
         * error. */
        goto out;
    }
    /* update process with run settings */
    if (MMU_SUCCESS != (rc = mpimemu_update_proc(process, app_settings))) {
        bad_func = "mpimemu_update_proc";
        goto out;
    }

    /* now that we know what the run setup is going to look like, allocate most,
     * if not all of the memory required for our run before we start the
     * collection */
    if (MMU_SUCCESS != (rc = mmu_process_reserve_memory(process))) {
        bad_func = "mmu_process_reserve_memory";
        goto out;
    }

    /* /// collect pre-mpi_init samples /// */
    if (MMU_SUCCESS != (rc = mmu_process_sample_memory_usage_self(process))) {
        bad_func = "mmu_process_get_memory_usage_self";
        goto out;
    }

    /* /// initialize mpi /// */
    rc = mmu_process_init_mpi(process, argc, argv);
    /* this failure is special.  we want only one process to display the error
     * message.  otherwise, the user will be flooded by messages */
    if (MMU_FAILURE_PPN_DIFFERS == rc) {
        if (mmu_process_is_delegate(process)) {
            fprintf(stderr,
                    "\n\nnumber of processes per node differs. "
                    "in order to obtain an accurate\nmeasurement, it is "
                    "imperative that the number of tasks per node is equal "
                    "across\nyour entire allocation.  cannot continue.\n\n");
        }
        /* do not set bad_func here, but do set ec */
        ec = EXIT_FAILURE;
        goto out;
    }
    else if (MMU_SUCCESS != rc) {
        bad_func = "mmu_process_init_mpi";
        goto out;
    }

    /* /// display settings /// */
    print_settings(process, app_settings);

    /* /// collect post-mpi_init samples - both self and node /// */
    if (MMU_SUCCESS != (rc = mmu_process_sample_memory_usage_all(process))) {
        bad_func = "mmu_process_sample_memory_usage_all";
        goto out;
    }

#if 0 /* debug only */
    if (MMU_SUCCESS != (rc = mmu_process_sample_dump(process))) {
        return EXIT_FAILURE;
    }
    goto out;
#endif

    /* after this point all the data have been collected, so we don't have to be
     * as careful as we were prior to to this point regarding our memory usage
     * and how/when things are allocated. */

    /* /// process the samples that we have collected /// */
    if (MMU_SUCCESS != (rc = mmu_process_process_usage(process))) {
        bad_func = "mmu_process_process_usage";
        goto out;
    }

out:
    if (NULL != bad_func) {
        fprintf(stderr,
                MMU_ERR_PREFIX"%s failure: %d (%s).\n", bad_func, rc,
                mmu_util_rc2str(rc));
        /* update the exit code */
        ec = EXIT_FAILURE;
    }
    /* finalize mpi */
    if (mmu_process_mpi_initialized()) {
        (void)mmu_process_finalize_mpi(process);
    }
    (void)mmu_args_destruct(&app_settings);
    (void)mmu_process_destruct(&process);

    return ec;
}
