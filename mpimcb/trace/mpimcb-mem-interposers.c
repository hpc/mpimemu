#include "mpimcb-mem-common.h"

#include <stdlib.h>

extern void *__libc_malloc(size_t size);

extern int mmcb_malloc_hook_active;

void *
malloc(size_t size)
{
    if (mmcb_malloc_hook_active) {
        return mmcb_malloc_hook(size);
    }
    return __libc_malloc(size);
}
