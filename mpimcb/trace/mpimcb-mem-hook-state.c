/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hook-state.h"
#include <assert.h>


static inline void
mem_hook_set_all(
    mmcb_mem_hook_mgr_t *mgr,
    bool state
) {
    assert(mgr);
    for (int i = 0; i < MMCB_HOOK_LAST; ++i) {
        mgr->mmcb_mem_hook_state_tab[i].active = state;
    }
}

void
mmcb_mem_hook_mgr_activate_all(
    mmcb_mem_hook_mgr_t *mgr
) {
    assert(mgr);
    mem_hook_set_all(mgr, true);
}

void
mmcb_mem_hook_mgr_deactivate_all(
    mmcb_mem_hook_mgr_t *mgr
) {
    assert(mgr);
    mem_hook_set_all(mgr, false);
}

bool
mmcb_mem_hook_mgr_hook_active(
    mmcb_mem_hook_mgr_t *mgr,
    int hook_id
) {
    assert(mgr);
    assert(hook_id < MMCB_HOOK_LAST);
    return mgr->mmcb_mem_hook_state_tab[hook_id].active;
}
