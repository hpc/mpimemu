/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"
#include "mpimcb-mem-hook-state.h"
#include "mpimcb-memory.h"

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
void *
mmcb_mem_hooks_calloc_hook(
    size_t nmemb,
    size_t size
) {
    // Deactivate hooks for logging.
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    // Do op.
    void *res = calloc(nmemb, size);
    // Do logging.
    const size_t real_size = nmemb * size;
    mmcb_mem.capture(
        new mmcb_memory_op_entry(MMCB_HOOK_CALLOC, uintptr_t(res), real_size)
    );
    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);

    return res;
}

/**
 *
 */
void *
mmcb_mem_hooks_realloc_hook(
    void *ptr,
    size_t size
) {
    // Deactivate hooks for logging.
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    // Do op.
    void *res = realloc(ptr, size);
    // Do logging.
    mmcb_mem.capture(
        new mmcb_memory_op_entry(
            MMCB_HOOK_REALLOC,
            uintptr_t(res),
            size,
            uintptr_t(ptr))
    );
    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    //
    return res;
}

/**
 *
 */
int
mmcb_mem_hooks_posix_memalign_hook(
    void **memptr,
    size_t alignment,
    size_t size
) {
    // Deactivate hooks for logging.
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);
    // Do op.
    int rc = posix_memalign(memptr, alignment, size);
    // Do logging.
    mmcb_mem.capture(
        new mmcb_memory_op_entry(
            MMCB_HOOK_POSIX_MEMALIGN,
            uintptr_t(*memptr),
            size
        )
    );
    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
    //
    return rc;
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
        new mmcb_memory_op_entry(MMCB_HOOK_FREE, uintptr_t(ptr))
    );
    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);
}
