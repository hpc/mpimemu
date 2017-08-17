#pragma once

#include "mpimcb-mem-hook-state.h"

#ifdef __cplusplus
class mmcb_rt {
private:
    mmcb_mem_hook_mgr_t mhmgr;
    //
    mmcb_rt(void);
    //
    ~mmcb_rt(void);

public:
    int rank = 0;
    int numpe = 0;
    //
    static mmcb_rt *
    the_mmcb_rt(void);
    //
    mmcb_rt(const mmcb_rt &that) = delete;
    //
    mmcb_rt &
    operator=(const mmcb_rt &) = delete;
    //
    mmcb_mem_hook_mgr_t *
    get_mem_hook_mgr(void);
    //
    void
    activate_all_mem_hooks(void);
    //
    void
    deactivate_all_mem_hooks(void);
};
#endif

#ifdef __cplusplus
extern "C" {
#endif
mmcb_mem_hook_mgr_t *
mmcb_rt_get_mem_hook_mgr(void);
#ifdef __cplusplus
}
#endif
