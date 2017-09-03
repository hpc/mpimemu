/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
void *
mmcu_mem_hooks_malloc_hook(
    size_t size
);

/**
 *
 */
void *
mmcu_mem_hooks_calloc_hook(
    size_t nmemb,
    size_t size
);

/**
 *
 */
void *
mmcu_mem_hooks_realloc_hook(
    void *ptr,
    size_t size
);

/**
 *
 */
int
mmcu_mem_hooks_posix_memalign_hook(
    void **memptr,
    size_t alignment,
    size_t size
);

/**
 *
 */
void *
mmcu_mem_hooks_mmap_hook(
    void *addr,
    size_t length,
    int prot,
    int flags,
    int fd,
    off_t offset
);

/**
 *
 */
void
mmcu_mem_hooks_free_hook(
    void *ptr
);

/**
 *
 */
int
mmcu_mem_hooks_munmap_hook(
    void *addr,
    size_t length
);

#ifdef __cplusplus
}
#endif
