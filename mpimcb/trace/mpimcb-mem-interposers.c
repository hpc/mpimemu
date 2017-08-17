/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hooks.h"
#include "mpimcb-rt.h"

#include <stdlib.h>
#include <dlfcn.h>

extern void *__libc_malloc(size_t size);
extern void *__libc_calloc(size_t nmemb, size_t size);
extern void *__libc_realloc(void *ptr, size_t size);
extern void __libc_free(void *ptr);

/**
 *
 */
void *
malloc(size_t size)
{
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_MALLOC)) {
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
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_CALLOC)) {
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
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_FREE)) {
        mmcb_mem_hooks_free_hook(ptr);
    }
    else {
        __libc_free(ptr);
    }
}
#if 0
/**
 *
 */
int
posix_memalign(
    void **memptr,
    size_t alignment,
    size_t size
) {
    typedef int (*op_fn_t)(void **, size_t, size_t);
    static op_fn_t fun = NULL;
    //
    if (mmcb_mem_hook_mgr_hook_active(
            &mmcb_mem_hook_mgr,
            MMCB_HOOK_POSIX_MEMALIGN
    )) {
        return mmcb_mem_hooks_posix_memalign_hook(memptr, alignment, size);
    }
    if (!fun) {
        fun = (op_fn_t)dlsym(RTLD_NEXT, "posix_memalign");
    }
    return fun(memptr, alignment, size);
}
#endif
