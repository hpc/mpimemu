#pragma once

#include "mpimcu-mem-hook-state.h"

#ifdef __cplusplus
#include <string>
#include <limits.h>

class mmcu_rt {
private:
    mmcu_mem_hook_mgr_t mhmgr;
    //
    mmcu_rt(void);
    //
    ~mmcu_rt(void);
    //
    mmcu_rt(const mmcu_rt &that) = delete;
    //
    mmcu_rt &
    operator=(const mmcu_rt &) = delete;
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
    //
    int numpe = 0;
    //
    static mmcu_rt *
    the_mmcu_rt(void);
    //
    mmcu_mem_hook_mgr_t *
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
    get_date_time_str_now(void);
    //
    std::string
    get_hostname(void);
    //
    std::string
    get_app_name(void);
    //
    void
    gather_target_meta(void);
    //
    double
    get_init_time(void) {
        return init_time;
    }
};

#endif // #ifdef __cplusplus

#ifdef __cplusplus
extern "C" {
#endif
mmcu_mem_hook_mgr_t *
mmcu_rt_get_mem_hook_mgr(void);
#ifdef __cplusplus
}
#endif
