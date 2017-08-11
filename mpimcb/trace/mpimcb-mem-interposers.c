#include "mpimcb-mem-common.h"
#include "mpimcb-mem-hook-state.h"

#include <stdlib.h>

extern void *__libc_malloc(size_t size);
extern void __libc_free(void *ptr);

extern mmcb_mem_hook_mgr_t mmcb_mem_hook_mgr;

/**
 *
 */
void *
malloc(size_t size)
{
    if (mmcb_mem_hook_mgr_hook_active(&mmcb_mem_hook_mgr, MMCB_HOOK_MALLOC)) {
        return mmcb_malloc_hook(size);
    }
    return __libc_malloc(size);
}
