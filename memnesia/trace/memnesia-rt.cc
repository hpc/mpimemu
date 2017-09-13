/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "memnesia-rt.h"
#include "memnesia-timer.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <signal.h>
#include <unistd.h>
#include <string.h>

/**
 *
 */
memnesia_rt *
memnesia_rt::the_memnesia_rt(void)
{
    static memnesia_rt singleton;
    return &singleton;
}

/**
 *
 */
void
memnesia_rt::set_init_begin_time_now(void)
{
    init_begin_time = memnesia_time();
}

/**
 *
 */
void
memnesia_rt::set_init_end_time_now(void)
{
    init_end_time = memnesia_time();
}

/**
 *
 */
void
memnesia_rt::set_hostname(void)
{
    if (0 != gethostname(hostname, sizeof(hostname))) {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }
}

/**
 *
 */
std::string
memnesia_rt::get_hostname(void)
{
    return std::string(hostname, sizeof(hostname));
}

/**
 *
 */
void
memnesia_rt::set_target_cmdline(void)
{
    FILE *commf = fopen("/proc/self/comm", "r");
    if (!commf) {
        perror("fopen /proc/self/comm");
        exit(EXIT_FAILURE);
    }

    char lb[PATH_MAX];
    char *gets = fgets(lb, sizeof(lb) - 1, commf);
    if (!gets) return;

    char *nl = nullptr;
    if ((nl = strchr(lb, '\n')) != NULL) {
        *nl = '\0';
    }
    snprintf(app_comm, sizeof(app_comm) - 1, "%s", lb);

    fclose(commf);
}

/**
 *
 */
std::string
memnesia_rt::get_app_name(void)
{
    return std::string(app_comm, sizeof(app_comm));
}

/**
 *
 */
void
memnesia_rt::gather_target_metadata(void)
{
    set_hostname();
    set_target_cmdline();
}

/**
 *
 */
double
memnesia_rt::get_init_begin_time(void)
{
    return init_begin_time;
}

/**
 *
 */
double
memnesia_rt::get_init_end_time(void)
{
    return init_end_time;
}

//
void
memnesia_rt::pinit(void)
{
    // Reset any signal handlers that may have been set in MPI_Init.
    (void)signal(SIGSEGV, SIG_DFL);
    // Synchronize.
    const int nsyncs = 2;
    for (int i = 0; i < nsyncs; ++i) {
        PMPI_Barrier(MPI_COMM_WORLD);
    }
    // Gather some information for tool use.
    gather_target_metadata();
    // Create communicators for processes co-loated on a compute node.
    if (MPI_SUCCESS != PMPI_Comm_split_type(
                           MPI_COMM_WORLD,
                           MPI_COMM_TYPE_SHARED,
                           0,
                           MPI_INFO_NULL,
                           &node_comm
                        )) {
        perror("PMPI_Comm_split_type");
        exit(EXIT_FAILURE);
    }
    if (MPI_SUCCESS != PMPI_Comm_rank(MPI_COMM_WORLD, &rank)) {
        perror("PMPI_Comm_rank");
        exit(EXIT_FAILURE);
    }
    if (MPI_SUCCESS != PMPI_Comm_size(MPI_COMM_WORLD, &numpe)) {
        perror("PMPI_Comm_size");
        exit(EXIT_FAILURE);
    }
    if (MPI_SUCCESS != PMPI_Comm_rank(node_comm, &node_rank)) {
        perror("PMPI_Comm_rank");
        exit(EXIT_FAILURE);
    }
    if (MPI_SUCCESS != PMPI_Comm_size(node_comm, &node_numpe)) {
        perror("PMPI_Comm_size");
        exit(EXIT_FAILURE);
    }
    // Emit obnoxious header that lets the user know something is happening.
    if (rank == 0) {
        emit_header();
    }
}

/**
 *
 */
void
memnesia_rt::pfini(void)
{
    int rc = MPI_ERR_UNKNOWN;
    if (MPI_SUCCESS != (rc = PMPI_Comm_free(&node_comm))) {
        // We are already at the end, so it would be a shame to abort the run
        // for this, so just let the user know something went south towards the
        // end.
        fprintf(
            stderr,
            "WARNING: %s returns %d (not MPI_SUCCESS)\n",
            "PMPI_Comm_free",
            rc
        );
    }
}

/**
 *
 */
std::string
memnesia_rt::get_date_time_str_now(void)
{
    char tsb[64];
    struct tm *bd_time_ptr = NULL;

    time_t raw_time;
    time(&raw_time);
    bd_time_ptr = localtime(&raw_time);

    strftime(tsb, sizeof(tsb) - 1, "%Y%m%d-%H%M%S", bd_time_ptr);
    //
    return std::string(tsb, sizeof(tsb));
}

/**
 *
 */
void
memnesia_rt::emit_header(void)
{
    printf(
        "\n\n"
        "                                               _____             \n"
        " _______ _______________ _________________________(_)_____ _     \n"
        " __  __ `__ \\  _ \\_  __ `__ \\_  __ \\  _ \\_  ___/_  /_  __ `/\n"
        " _  / / / / /  __/  / / / / /  / / /  __/(__  )_  / / /_/ /      \n"
        " /_/ /_/ /_/\\___//_/ /_/ /_//_/ /_/\\___//____/ /_/  \\__,_/    \n"
        "\n\n"
    );
}

/**
 *
 */
void
memnesia_rt::sample(
    const std::string &what,
    memnesia_sample &res
) {
    res = memnesia_sample(what);
}

/**
 *
 */
void
memnesia_rt::sample_delta(
    const memnesia_sample &happened_before,
    const memnesia_sample &happened_after,
    memnesia_sample &delta
) {
    memnesia_sample::delta(happened_before, happened_after, delta);
}

/**
 *
 */
void
memnesia_rt::sample_emit(
    const memnesia_sample &s
) {
    memnesia_sample::emit(s);
}

/**
 *
 */
void
memnesia_rt::add_samples_to_dataset(
    const memnesia_sample &happened_before,
    const memnesia_sample &happened_after
) {
    memnesia_sample delta;

    memnesia_rt::sample_delta(happened_before, happened_after, delta);

    dataset.push_back(memnesia_dataset::APP, happened_before);
    dataset.push_back(memnesia_dataset::APP, happened_after);

    dataset.push_back(memnesia_dataset::MPI, delta);
}

/**
 *
 */
int64_t
memnesia_rt::get_num_smaps_captures(void)
{
    return dataset.length(memnesia_dataset::APP);
}

/**
 *
 */
void
memnesia_rt::report(void)
{
    using namespace std;
    //
    setbuf(stdout, NULL);
    //
    if (rank == 0) {
        printf(
            "\n"
            "\n#"
            "\n# memnesia memory consumption analysis complete..."
            "\n#"
            "\n"
        );
    }
    //
    char *output_dir = getenv(MEMNESIA_ENV_REPORT_OUTPUT_PATH);
    // Not set, so output to pwd.
    if (!output_dir) {
        output_dir = getenv("PWD");
    }
    if (!output_dir) {
        fprintf(stderr, "Error saving report.\n");
        return;
    }

    char report_name[PATH_MAX];
    snprintf(
        report_name, sizeof(report_name) - 1, "%s/%d.%s",
        output_dir, rank, "memnesia"
    );

    FILE *reportf = fopen(report_name, "w+");
    if (!reportf) {
        fprintf(stderr, "Error saving report to %s.\n", report_name);
        return;
    }

    fprintf(reportf, "# [Run Info Begin]\n");

    fprintf(
        reportf,
        "# Report Date Time: %s\n",
        get_date_time_str_now().c_str()
    );

    fprintf(reportf, "# Application Name: %s\n", get_app_name().c_str());

    fprintf(reportf, "# Hostname: %s\n", get_hostname().c_str());

    fprintf(reportf, "# MPI_COMM_WORLD Rank: %d\n", rank);

    fprintf(reportf, "# MPI_COMM_WORLD Size: %d\n", numpe);

    fprintf(
        reportf,
        "# Number of smaps Captures Performed: %" PRIu64 "\n",
        get_num_smaps_captures()
    );

    fprintf(
        reportf,
        "# High Memory Usage Watermark (MPI) (MB): %lf\n",
        memnesia_util_kb2mb(
            dataset.get_high_mem_usage_watermark_in_kb(memnesia_dataset::MPI)
        )
    );

    fprintf(
        reportf,
        "# High Memory Usage Watermark (Application + MPI) (MB): %lf\n",
        memnesia_util_kb2mb(
            dataset.get_high_mem_usage_watermark_in_kb(memnesia_dataset::APP)
        )
    );

    fprintf(reportf, "# [Run Info End]\n");
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    const double init_time = get_init_begin_time();

    fprintf(
        reportf,
        "# MPI Library Memory Usage (MB) Over Time "
        "(Since MPI_Init):\n"
        "# Format:\n"
        "# KEY Function Time Usage\n"
    );
    dataset.report(reportf, memnesia_dataset::MPI, init_time);

    fprintf(
        reportf,
        "# Application Memory Usage (MB) Over Time "
        "(Since MPI_Init):\n"
        "# Format:\n"
        "# KEY Function Time Usage\n"
    );
    dataset.report(reportf, memnesia_dataset::APP, init_time);

    fclose(reportf);

    if (rank == 0) {
        printf("# Report written to %s\n", output_dir);
    }
}
