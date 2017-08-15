/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"
#include "mpimcb-mem-hook-state.h"

#include <stdlib.h>

extern void *__libc_malloc(size_t size);
extern void *__libc_calloc(size_t nmemb, size_t size);
extern void *__libc_realloc(void *ptr, size_t size);
extern void __libc_free(void *ptr);

extern mmcb_mem_hook_mgr_t mmcb_mem_hook_mgr;

/**
 *
 */
void *
malloc(size_t size)
{
    if (mmcb_mem_hook_mgr_hook_active(&mmcb_mem_hook_mgr, MMCB_HOOK_MALLOC)) {
        return mmcb_mem_hooks_malloc_hook(size);
    }
    return __libc_malloc(size);
}

/**
 *
 */
void *
calloc(size_t nmemb, size_t size)
{
    if (mmcb_mem_hook_mgr_hook_active(&mmcb_mem_hook_mgr, MMCB_HOOK_CALLOC)) {
        return mmcb_mem_hooks_calloc_hook(nmemb, size);
    }
    return __libc_calloc(nmemb, size);
}

#if 0
/**
 *
 */
void *
realloc(
    void *ptr,
    size_t size
) {
    if (mmcb_mem_hook_mgr_hook_active(&mmcb_mem_hook_mgr, MMCB_HOOK_REALLOC)) {
        return mmcb_mem_hooks_realloc_hook(ptr, size);
    }
    return __libc_realloc(ptr, size);
}
#endif
/**
 *
 */
void
free(void *ptr)
{
    if (mmcb_mem_hook_mgr_hook_active(&mmcb_mem_hook_mgr, MMCB_HOOK_FREE)) {
        return mmcb_mem_hooks_free_hook(ptr);
    }
    return __libc_free(ptr);
}
