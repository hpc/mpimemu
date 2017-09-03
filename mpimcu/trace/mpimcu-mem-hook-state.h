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
    MMCU_HOOK_MALLOC = 0,
    MMCU_HOOK_CALLOC,
    MMCU_HOOK_REALLOC,
    MMCU_HOOK_FREE,
    MMCU_HOOK_POSIX_MEMALIGN,
    MMCU_HOOK_MMAP,
    MMCU_HOOK_MMAP_PSS_UPDATE, /* For internal use only. */
    MMCU_HOOK_MUNMAP,
    MMCU_HOOK_NOOP,            /* For internal use only. */
    MMCU_HOOK_LAST
};

typedef struct mmcu_mem_hook_state_t {
    uint8_t active;
} mmcu_mem_hook_state_t;

typedef struct mmcu_mem_hook_mgr_t {
    mmcu_mem_hook_state_t mmcu_mem_hook_state_tab[MMCU_HOOK_LAST];
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
