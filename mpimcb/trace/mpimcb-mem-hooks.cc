/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"

#include "mpimcb-rt.h"
#include "mpimcb-mem-stat-mgr.h"

#include <cstdlib>

namespace {

mmcb_rt *rt = nullptr; 

}

/**
 *
 */
void *
mmcb_mem_hooks_malloc_hook(
    size_t size
) {
    rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
    // Do op.
    void *res = malloc(size);
    // Do logging.
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(MMCB_HOOK_MALLOC, uintptr_t(res), size)
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();

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
    rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
    // Do op.
    void *res = calloc(nmemb, size);
    // Do logging.
    const size_t real_size = nmemb * size;
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(MMCB_HOOK_CALLOC, uintptr_t(res), real_size)
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();

    return res;
}

#if 0
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
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(
            MMCB_HOOK_REALLOC,
            uintptr_t(res),
            size,
            uintptr_t(ptr))
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();
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
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(
            MMCB_HOOK_POSIX_MEMALIGN,
            uintptr_t(*memptr),
            size
        )
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();
    //
    return rc;
}

#endif
/**
 *
 */
void
mmcb_mem_hooks_free_hook(
    void *ptr
) {
    rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
    // Do op.
    free(ptr);
    // Do logging.
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(MMCB_HOOK_FREE, uintptr_t(ptr))
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();
}
