#include "mpimcb-mem-common.h"
#include "mpimcb-mem-hook-state.h"

#include "CallpathRuntime.h"
#include "Translator.h"

#include <iostream>
#include <vector>

#include <stdlib.h>
#include <stdio.h>

extern mmcb_mem_hook_mgr_t mmcb_mem_hook_mgr;

CallpathRuntime cprt;
Translator trans;

std::vector<FrameInfo> finfos;

/**
 *
 */
void *
mmcb_malloc_hook(size_t size)
{
    // Deactivate hooks for logging.
    mmcb_mem_hook_mgr_deactivate_all(&mmcb_mem_hook_mgr);

    // Do logging.
    Callpath path = cprt.doStackwalk();
    int ps = path.size();
    for (int i = 0; i < ps; i++) {
        FrameId frame = path[i];
        FrameInfo info = trans.translate(frame);
        finfos.push_back(info);
    }
    FrameInfo e;
    finfos.push_back(e);

    void *result = malloc(size);

    // Reactivate hooks.
    mmcb_mem_hook_mgr_activate_all(&mmcb_mem_hook_mgr);

    return result;
}

void
dump(void) {
    for (auto &f : finfos) {
        std::cout << f << std::endl;
    }
}
