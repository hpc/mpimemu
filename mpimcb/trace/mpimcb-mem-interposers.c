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

/**
 *
 */
void *
realloc(
    void *ptr,
    size_t size
) {
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_REALLOC)) {
        return mmcb_mem_hooks_realloc_hook(ptr, size);
    }
    return __libc_realloc(ptr, size);
}

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
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_POSIX_MEMALIGN)) {
        return mmcb_mem_hooks_posix_memalign_hook(memptr, alignment, size);
    }
    if (!fun) {
        fun = (op_fn_t)dlsym(RTLD_NEXT, "posix_memalign");
    }
    return fun(memptr, alignment, size);
}

/**
 *
 */
void *
mmap(
    void *addr,
    size_t length,
    int prot,
    int flags,
    int fd,
    off_t offset
) {
    typedef void *(*op_fn_t)(void *, size_t, int, int, int, off_t);
    static op_fn_t fun = NULL;
    //
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_MMAP)) {
        return mmcb_mem_hooks_mmap_hook(
                   addr, length, prot, flags, fd, offset
               );
    }
    if (!fun) {
        fun = (op_fn_t)dlsym(RTLD_NEXT, "mmap");
    }
    return fun(addr, length, prot, flags, fd, offset);
}

/**
 *
 */
int
munmap(
    void *addr,
    size_t length
) {
    typedef int (*op_fn_t)(void *, size_t);
    static op_fn_t fun = NULL;
    //
    mmcb_mem_hook_mgr_t *mgr = mmcb_rt_get_mem_hook_mgr();
    if (mmcb_mem_hook_mgr_hook_active(mgr, MMCB_HOOK_MUNMAP)) {
        return mmcb_mem_hooks_munmap_hook(addr, length);
    }
    if (!fun) {
        fun = (op_fn_t)dlsym(RTLD_NEXT, "munmap");
    }
    return fun(addr, length);
}
