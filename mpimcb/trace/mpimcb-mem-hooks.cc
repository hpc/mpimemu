/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"

#include "mpimcb-rt.h"
#include "mpimcb-mem-stat-mgr.h"

#include <cstdlib>

#include <sys/mman.h>

/**
 *
 */
void *
mmcb_mem_hooks_malloc_hook(
    size_t size
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
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

/**
 *
 */
void *
mmcb_mem_hooks_realloc_hook(
    void *ptr,
    size_t size
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
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
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
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

/**
 *
 */
void *
mmcb_mem_hooks_mmap_hook(
    void *addr,
    size_t length,
    int prot,
    int flags,
    int fd,
    off_t offset
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
    // Do op.
    void *res = mmap(addr, length, prot, flags, fd, offset);
    // Do logging.
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(
            MMCB_HOOK_MMAP,
            uintptr_t(res),
            length
        )
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();
    //
    return res;
}

/**
 *
 */
void
mmcb_mem_hooks_free_hook(
    void *ptr
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
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

/**
 *
 */
int
mmcb_mem_hooks_munmap_hook(
    void *addr,
    size_t length
) {
    static mmcb_rt *rt = mmcb_rt::the_mmcb_rt();
    // Deactivate hooks for logging.
    rt->deactivate_all_mem_hooks();
    // Do op.
    int res = munmap(addr, length);
    // Do logging.
    mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr()->capture(
        new mmcb_memory_op_entry(
            MMCB_HOOK_MUNMAP,
            uintptr_t(addr),
            length
        )
    );
    // Reactivate hooks.
    rt->activate_all_mem_hooks();
    //
    return res;
}
