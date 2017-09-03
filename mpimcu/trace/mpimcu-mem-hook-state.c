/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcu-mem-hook-state.h"

#include <assert.h>

static inline void
mem_hook_set_all(
    mmcu_mem_hook_mgr_t *mgr,
    uint8_t state
) {
    assert(mgr);
    for (uint8_t i = 0; i < MMCU_HOOK_LAST; ++i) {
        mgr->mmcu_mem_hook_state_tab[i].active = state;
    }
}

void
mmcu_mem_hook_mgr_activate_all(
    mmcu_mem_hook_mgr_t *mgr
) {
    assert(mgr);
    mem_hook_set_all(mgr, 1);
}

void
mmcu_mem_hook_mgr_deactivate_all(
    mmcu_mem_hook_mgr_t *mgr
) {
    assert(mgr);
    mem_hook_set_all(mgr, 0);
}

uint8_t
mmcu_mem_hook_mgr_hook_active(
    mmcu_mem_hook_mgr_t *mgr,
    uint8_t hook_id
) {
    assert(mgr);
    assert(hook_id < MMCU_HOOK_LAST);
    return mgr->mmcu_mem_hook_state_tab[hook_id].active;
}
