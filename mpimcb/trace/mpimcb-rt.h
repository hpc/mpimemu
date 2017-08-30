#pragma once

#include "mpimcb-mem-hook-state.h"

#ifdef __cplusplus
#include <string>
#include <limits.h>

class mmcb_rt {
private:
    mmcb_mem_hook_mgr_t mhmgr;
    //
    mmcb_rt(void);
    //
    ~mmcb_rt(void);
    //
    mmcb_rt(const mmcb_rt &that) = delete;
    //
    mmcb_rt &
    operator=(const mmcb_rt &) = delete;
    //
    double init_time = 0.0;
    //
    char hostname[256];
    //
    char app_comm[PATH_MAX];
    //
    void
    set_hostname(void);
    //
    void
    set_target_cmdline(void);

public:
    int rank = 0;
    int numpe = 0;
    //
    static mmcb_rt *
    the_mmcb_rt(void);
    //
    mmcb_mem_hook_mgr_t *
    get_mem_hook_mgr(void);
    //
    void
    activate_all_mem_hooks(void);
    //
    void
    deactivate_all_mem_hooks(void);
    //
    void
    set_init_time_now(void);
    //
    std::string
    get_hostname(void);
    //
    std::string
    get_app_name(void);
    //
    void
    gather_target_meta(void);
};

#endif // #ifdef __cplusplus

#ifdef __cplusplus
extern "C" {
#endif
mmcb_mem_hook_mgr_t *
mmcb_rt_get_mem_hook_mgr(void);
#ifdef __cplusplus
}
#endif
