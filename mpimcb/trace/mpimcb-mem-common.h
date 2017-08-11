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
mmcb_malloc_hook(size_t size);

#ifdef __cplusplus
}
#endif

void
dump(void);
