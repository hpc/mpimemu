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

using namespace std;

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
    return std::string(hostname, strlen(hostname) + 1);
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
    return std::string(app_comm, strlen(app_comm) + 1);
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
    //
    setbuf(stdout, NULL);
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
memnesia_rt::fill_report_buffer(
    std::stringstream &ss
) {
    ss << "# [Run Info Begin]"      << endl;

    ss << "# Report Date Time: "    << get_date_time_str_now() << endl;

    ss << "# Application Name: "    << get_app_name() << endl;

    ss << "# Hostname: "            << get_hostname() << endl;

    ss << "# MPI_COMM_WORLD Rank: " << rank << endl;

    ss << "# MPI_COMM_WORLD Size: " << numpe << endl;

    ss << "# Number of smaps Captures Performed: "
       << get_num_smaps_captures() << endl;

    ss << "# High Memory Usage Watermark (MPI) (MB): "
       <<  memnesia_util_kb2mb(
               dataset.get_high_mem_usage_watermark_in_kb(memnesia_dataset::MPI)
           )
       << endl;

    ss << "# High Memory Usage Watermark (Application + MPI) (MB): "
       <<  memnesia_util_kb2mb(
               dataset.get_high_mem_usage_watermark_in_kb(memnesia_dataset::APP)
           )
       << endl;

    ss << "# [Run Info End]" << endl;
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    const double init_time = get_init_begin_time();

    ss << "# MPI Library Memory Usage (MB) Over Time (Since MPI_Init):"
       << endl
       << "# Format:"
       << endl
       << "# KEY Function Time Usage"
       << endl;
    dataset.report(ss, memnesia_dataset::MPI, init_time);

    ss << "# Application Memory Usage (MB) Over Time (Since MPI_Init):"
       << endl
       << "# Format:"
       << endl
       << "# KEY Function Time Usage"
       << endl;
    dataset.report(ss, memnesia_dataset::APP, init_time);
}

/**
 *
 */
void
memnesia_rt::aggregate_local_data(void)
{
    stringstream ss;

    fill_report_buffer(ss);

    cout << ss.str();
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
    return std::string(tsb, strlen(tsb) + 1);
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
std::string
memnesia_rt::get_output_path(void)
{
    //
    char *output_dir = getenv(MEMNESIA_ENV_REPORT_OUTPUT_PATH);
    // Not set, so output to pwd.
    if (!output_dir) {
        output_dir = getenv("PWD");
    }
    if (!output_dir) {
        fprintf(stderr, "Error saving report.\n");
        exit(EXIT_FAILURE);
    }

    return std::string(output_dir);
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
    aggregate_local_data();
    // Only one MPI process will write the report.
    if (node_rank != 0) return;

    std::string output_dir = get_output_path();
    //
    char report_name[PATH_MAX];
    // TODO change report prefix to hostname.
    snprintf(
        report_name, sizeof(report_name) - 1, "%s/%d.%s",
        output_dir.c_str(), rank, "memnesia"
    );

    FILE *reportf = fopen(report_name, "w+");
    if (!reportf) {
        fprintf(stderr, "Error saving report to %s.\n", report_name);
        return;
    }


    fclose(reportf);

    if (rank == 0) {
        printf("# Report written to %s\n", output_dir.c_str());
    }
}
