/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hook-state.h"

#include <assert.h>

static inline void
mem_hook_set_all(
    mmcb_mem_hook_mgr_t *mgr,
    uint8_t state
) {
    assert(mgr);
    for (uint8_t i = 0; i < MMCB_HOOK_LAST; ++i) {
        mgr->mmcb_mem_hook_state_tab[i].active = state;
    }
}

void
mmcb_mem_hook_mgr_activate_all(
    mmcb_mem_hook_mgr_t *mgr
) {
    assert(mgr);
    mem_hook_set_all(mgr, 1);
}

void
mmcb_mem_hook_mgr_deactivate_all(
    mmcb_mem_hook_mgr_t *mgr
) {
    assert(mgr);
    mem_hook_set_all(mgr, 0);
}

uint8_t
mmcb_mem_hook_mgr_hook_active(
    mmcb_mem_hook_mgr_t *mgr,
    uint8_t hook_id
) {
    assert(mgr);
    assert(hook_id < MMCB_HOOK_LAST);
    return mgr->mmcb_mem_hook_state_tab[hook_id].active;
}
