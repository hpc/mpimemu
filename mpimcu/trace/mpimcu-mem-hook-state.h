/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MMCB_HOOK_MALLOC = 0,
    MMCB_HOOK_CALLOC,
    MMCB_HOOK_REALLOC,
    MMCB_HOOK_FREE,
    MMCB_HOOK_POSIX_MEMALIGN,
    MMCB_HOOK_MMAP,
    MMCB_HOOK_MMAP_PSS_UPDATE, /* For internal use only. */
    MMCB_HOOK_MUNMAP,
    MMCB_HOOK_NOOP,            /* For internal use only. */
    MMCB_HOOK_LAST
};

typedef struct mmcu_mem_hook_state_t {
    uint8_t active;
} mmcu_mem_hook_state_t;

typedef struct mmcu_mem_hook_mgr_t {
    mmcu_mem_hook_state_t mmcu_mem_hook_state_tab[MMCB_HOOK_LAST];
} mmcu_mem_hook_mgr_t;

/**
 *
 */
void
mmcu_mem_hook_mgr_activate_all(
    mmcu_mem_hook_mgr_t *mgr
);

/**
 *
 */
void
mmcu_mem_hook_mgr_deactivate_all(
    mmcu_mem_hook_mgr_t *mgr
);

/**
 *
 */
uint8_t
mmcu_mem_hook_mgr_hook_active(
    mmcu_mem_hook_mgr_t *mgr,
    uint8_t hook_id
);

#ifdef __cplusplus
}
#endif
