/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"
#include "mpimcb-mem-hook-state.h"
#include "mpimcb-memory.h"

#include "CallpathRuntime.h"
#include "Translator.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <deque>

extern mmcb_mem_hook_mgr_t mmcb_mem_hook_mgr;

extern mmcb_memory mmcb_mem;

/**
 *
 */
void *
mmcb_mem_hooks_malloc_hook(
    size_t size
) {
    // Deactivate hooks for logging.
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    // Do op.
    void *res = malloc(size);
    // Do logging.
    mmcb_mem.capture(
        new mmcb_memory_op_entry(MMCB_HOOK_MALLOC, uintptr_t(res), size)
    );
    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);

    return res;
}

/**
 *
 */
void
mmcb_mem_hooks_free_hook(
    void *ptr
) {
    // Deactivate hooks for logging.
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    // Do op.
    free(ptr);
    // Do logging.
    mmcb_mem.capture(
        new mmcb_memory_op_entry(MMCB_HOOK_FREE, uintptr_t(ptr), 0)
    );
    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
}
