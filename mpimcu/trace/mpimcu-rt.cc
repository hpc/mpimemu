#include "mpimcu-rt.h"
#include "mpimcu-timer.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <unistd.h>
#include <string.h>

/**
 *
 */
mmcu_rt::mmcu_rt(void)
{
    deactivate_all_mem_hooks();
}

/**
 *
 */
mmcu_rt::~mmcu_rt(void) = default;

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
mmcu_mem_hook_mgr_t *
mmcu_rt::get_mem_hook_mgr(void)
{
    return &mhmgr;
}

/**
 *
 */
void
mmcu_rt::activate_all_mem_hooks(void)
{
    mmcu_mem_hook_mgr_activate_all(&mhmgr);
}

/**
 *
 */
void
mmcu_rt::deactivate_all_mem_hooks(void)
{
    mmcu_mem_hook_mgr_deactivate_all(&mhmgr);
}

/**
 *
 */
void
mmcu_rt::set_init_time_now(void)
{
    init_time = mmcu_time();
}

/**
 *
 */
mmcu_mem_hook_mgr_t *
mmcu_rt_get_mem_hook_mgr(void)
{
    return mmcu_rt::the_mmcu_rt()->get_mem_hook_mgr();
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
void
mmcu_rt::gather_target_meta(void)
{
    set_hostname();
    set_target_cmdline();
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
