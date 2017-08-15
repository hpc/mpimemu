/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
void *
mmcb_mem_hooks_malloc_hook(
    size_t size
);

/**
 *
 */
void *
mmcb_mem_hooks_calloc_hook(
    size_t nmemb,
    size_t size
);

/**
 *
 */
void *
mmcb_mem_hooks_realloc_hook(
    void *ptr,
    size_t size
);

/**
 *
 */
void
mmcb_mem_hooks_free_hook(
    void *ptr
);

#ifdef __cplusplus
}
#endif
