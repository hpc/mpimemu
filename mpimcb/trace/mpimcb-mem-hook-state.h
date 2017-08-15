/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MMCB_HOOK_MALLOC = 0,
    MMCB_HOOK_CALLOC,
    MMCB_HOOK_REALLOC,
    MMCB_HOOK_FREE,
    MMCB_HOOK_NOOP,
    MMCB_HOOK_LAST
};

typedef struct mmcb_mem_hook_state_t {
    bool active;
} mmcb_mem_hook_state_t;

typedef struct mmcb_mem_hook_mgr_t {
    mmcb_mem_hook_state_t mmcb_mem_hook_state_tab[MMCB_HOOK_LAST];
} mmcb_mem_hook_mgr_t;

/**
 *
 */
void
mmcb_mem_hook_mgr_activate_all(
    mmcb_mem_hook_mgr_t *mgr
);

/**
 *
 */
void
mmcb_mem_hook_mgr_deactivate_all(
    mmcb_mem_hook_mgr_t *mgr
);

/**
 *
 */
bool
mmcb_mem_hook_mgr_hook_active(
    mmcb_mem_hook_mgr_t *mgr,
    int hook_id
);

#ifdef __cplusplus
}
#endif
