#pragma once

#include <stdlib.h>

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
