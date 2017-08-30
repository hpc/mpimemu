#include "mpimcb-rt.h"
#include "mpimcb-timer.h"

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <string.h>

/**
 *
 */
mmcb_rt::mmcb_rt(void)
{
    deactivate_all_mem_hooks();
}

/**
 *
 */
mmcb_rt::~mmcb_rt(void) = default;

/**
 *
 */
mmcb_rt *
mmcb_rt::the_mmcb_rt(void)
{
    static mmcb_rt singleton;
    return &singleton;
}

/**
 *
 */
mmcb_mem_hook_mgr_t *
mmcb_rt::get_mem_hook_mgr(void)
{
    return &mhmgr;
}

/**
 *
 */
void
mmcb_rt::activate_all_mem_hooks(void)
{
    mmcb_mem_hook_mgr_activate_all(&mhmgr);
}

/**
 *
 */
void
mmcb_rt::deactivate_all_mem_hooks(void)
{
    mmcb_mem_hook_mgr_deactivate_all(&mhmgr);
}

/**
 *
 */
void
mmcb_rt::set_init_time_now(void)
{
    init_time = mmcb_time();
}

/**
 *
 */
mmcb_mem_hook_mgr_t *
mmcb_rt_get_mem_hook_mgr(void)
{
    return mmcb_rt::the_mmcb_rt()->get_mem_hook_mgr();
}

/**
 *
 */
void
mmcb_rt::set_hostname(void)
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
mmcb_rt::get_hostname(void)
{
    return std::string(hostname, sizeof(hostname));
}

/**
 *
 */
void
mmcb_rt::set_target_cmdline(void)
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
mmcb_rt::gather_target_meta(void)
{
    set_hostname();
    set_target_cmdline();
}

/**
 *
 */
std::string
mmcb_rt::get_app_name(void)
{
    return std::string(app_comm, sizeof(app_comm));
}
