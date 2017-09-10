/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcu-rt.h"
#include "mpimcu-timer.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <unistd.h>
#include <string.h>

/**
 *
 */
mmcu_rt *
mmcu_rt::the_mmcu_rt(void)
{
    static mmcu_rt singleton;
    return &singleton;
}

/**
 *
 */
void
mmcu_rt::set_init_begin_time_now(void)
{
    init_begin_time = mmcu_time();
}

/**
 *
 */
void
mmcu_rt::set_init_end_time_now(void)
{
    init_end_time = mmcu_time();
}

/**
 *
 */
void
mmcu_rt::set_hostname(void)
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
mmcu_rt::get_hostname(void)
{
    return std::string(hostname, sizeof(hostname));
}

/**
 *
 */
void
mmcu_rt::set_target_cmdline(void)
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
mmcu_rt::get_app_name(void)
{
    return std::string(app_comm, sizeof(app_comm));
}

/**
 *
 */
void
mmcu_rt::gather_target_metadata(void)
{
    set_hostname();
    set_target_cmdline();
}

/**
 *
 */
double
mmcu_rt::get_init_begin_time(void)
{
    return init_begin_time;
}

/**
 *
 */
double
mmcu_rt::get_init_end_time(void)
{
    return init_end_time;
}

/**
 *
 */
std::string
mmcu_rt::get_date_time_str_now(void)
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
mmcu_rt::emit_header(void)
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
mmcu_rt::sample(
    const std::string &what,
    mmcu_sample &res
) {
    res = mmcu_sample(what);
}

/**
 *
 */
void
mmcu_rt::sample_delta(
    const mmcu_sample &happened_before,
    const mmcu_sample &happened_after,
    mmcu_sample &delta
) {
    mmcu_sample::delta(happened_before, happened_after, delta);
}

/**
 *
 */
void
mmcu_rt::sample_emit(
    const mmcu_sample &s
) {
    mmcu_sample::emit(s);
}

/**
 *
 */
void
mmcu_rt::store_sample(
    const mmcu_sample &happened_before,
    const mmcu_sample &happened_after
) {
    mmcu_sample delta;

    mmcu_rt::sample_delta(happened_before, happened_after, delta);

    dataset.push_back(mmcu_dataset::APP, happened_before);
    dataset.push_back(mmcu_dataset::APP, happened_after);

    dataset.push_back(mmcu_dataset::MPI, delta);
}

/**
 *
 */
void
mmcu_rt::report(void)
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
    char *output_dir = getenv("MMCU_REPORT_OUTPUT_PATH");
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
        output_dir, rank, "mmcu"
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
        "# Number of Operation Captures Performed: %" PRIu64 "\n",
        0uL
    );

    fprintf(
        reportf,
        "# Number of Memory Operations Recorded: %" PRIu64 "\n",
        0uL
    );

    fprintf(
        reportf,
        "# Number of Allocation-Related Operations Recorded: %" PRIu64 "\n",
        0uL
    );

    fprintf(
        reportf,
        "# Number of Deallocation-Related Operations Recorded: %" PRIu64 "\n",
        0uL
    );

    fprintf(
        reportf,
        "# Number of MPI Library PSS Samples Collected: %" PRIu64 "\n",
        0uL
    );

    fprintf(
        reportf,
        "# Number of Application PSS Samples Collected: %" PRIu64 "\n",
        0uL
    );

    fprintf(
        reportf,
        "# High Memory Usage Watermark (MPI) (MB): %lf\n",
        0.0
    );

    fprintf(
        reportf,
        "# High Memory Usage Watermark (Application + MPI) (MB): %lf\n",
        0.0
    );

    fprintf(reportf, "# [Run Info End]\n");
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    const double init_time = get_init_begin_time();

    fprintf(
        reportf,
        "# MPI Library Memory Usage (B) Over Time "
        "(Since MPI_Init):\n"
    );
    dataset.report(reportf, mmcu_dataset::MPI, init_time);

    fprintf(
        reportf,
        "# Application Memory Usage (B) Over Time "
        "(Since MPI_Init):\n"
    );
    dataset.report(reportf, mmcu_dataset::APP, init_time);

    fclose(reportf);

    if (rank == 0) {
        printf("# Report written to %s\n", output_dir);
    }
}
