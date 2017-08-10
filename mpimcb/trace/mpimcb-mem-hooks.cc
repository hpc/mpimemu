#include "mpimcb-mem-common.h"

#include "CallpathRuntime.h"
#include "Translator.h"

#include <iostream>
#include <vector>

#include <stdlib.h>
#include <stdio.h>

int mmcb_malloc_hook_active = 0;

CallpathRuntime cprt;
Translator trans;

std::vector<FrameInfo> finfos;

/**
 *
 */
extern "C" {
void *
mmcb_malloc_hook(size_t size)
{
    // Deactivate hooks for logging.
    mmcb_malloc_hook_active = 0;

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

    // reactivate hooks
    mmcb_malloc_hook_active = 1;

    return result;
}
}

void
dump(void) {
    for (auto &f : finfos) {
        std::cout << f << std::endl;
    }
}
