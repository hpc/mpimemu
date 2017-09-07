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
    while (fgets(lb, sizeof(lb) - 1, commf)) {
        break;
    }
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
mmcu_rt::gather_target_meta(void)
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
        "\n"
        "_|      _|  _|_|_|    _|_|_|  _|      _|    _|_|_|  _|    _|\n"
        "_|_|  _|_|  _|    _|    _|    _|_|  _|_|  _|        _|    _|\n"
        "_|  _|  _|  _|_|_|      _|    _|  _|  _|  _|        _|    _|\n"
        "_|      _|  _|          _|    _|      _|  _|        _|    _|\n"
        "_|      _|  _|        _|_|_|  _|      _|    _|_|_|    _|_|  \n"
        "\n"
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
