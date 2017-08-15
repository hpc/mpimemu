/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-hook-state.h"

#include "CallpathRuntime.h"
#include "Translator.h"

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <cassert>

struct mmcb_memory_op_entry {
    // Memory opteration ID.
    uint8_t opid;
    // Address associated with memory operation.
    uintptr_t addr;
    // If applicable, size associated with memory operation.
    size_t size;
    // If applicable, 'old' address associated with memory operation. 
    uintptr_t old_addr;

    /**
     *
     */
    mmcb_memory_op_entry(
        uint8_t opid,
        uintptr_t addr,
        size_t size = 0,
        uintptr_t old_addr = 0
    ) : opid(opid)
      , addr(addr)
      , size(size)
      , old_addr(old_addr) { }
};

struct mmcb_memory {
private:
    //
    size_t current_mem_allocd = 0;
    //
    size_t high_mem_usage_mark = 0;
    //
    std::unordered_map<uintptr_t, mmcb_memory_op_entry *> addr2entry;

public:
    /**
     *
     */
    ~mmcb_memory(void) {
        // TODO
    }

    /**
     *
     */
    void
    capture(
        mmcb_memory_op_entry *const ope
    ) {
        bool rm_ope = false;
        const uintptr_t addr = ope->addr;
        const uint8_t opid = ope->opid;
        //
        if (opid == MMCB_HOOK_REALLOC) {
            break_down_realloc(ope);
            return;
        }
        auto got = addr2entry.find(addr);
        // New entry.
        if (got == addr2entry.end()) {
            addr2entry.insert(std::make_pair(addr, ope));
        }
        // Existing entry.
        else {
            if (MMCB_HOOK_FREE == opid) {
                ope->size = got->second->size;
                rm_ope = true;
            }
        }
        update_current_mem_allocd(ope);
        //
        if (rm_ope) {
            addr2entry.erase(got);
        }
    }

    /**
     *
     */
    void
    break_down_realloc(
        mmcb_memory_op_entry *const ope
    ) {
        const uintptr_t addr = ope->addr;
        const uintptr_t old_addr = ope->old_addr;
        const size_t size = ope->size;
        // Returned NULL, so old_addr was unchanged.
        if (!addr) {
            // Nothing to do.
            ope->opid = MMCB_HOOK_NOOP;
        }
        // Acts like free.
        else if (size == 0 && old_addr) {
            auto got = addr2entry.find(old_addr);
            if (got != addr2entry.end()) {
                ope->opid = MMCB_HOOK_FREE;
                // Will be looked up in terms of addr, so update.
                ope->addr = old_addr;
            }
            // Probably an application bug, so do nothing.
            else {
                ope->opid = MMCB_HOOK_NOOP;
            }
        }
        // Acts like malloc.
        else if (!old_addr) {
            ope->opid = MMCB_HOOK_MALLOC;
        }
        // Area pointed to was moved.
        else if (old_addr != addr) {
            // New region was first created.
            ope->opid = MMCB_HOOK_MALLOC; 
            capture(ope);
            // Old region was freed.
            ope->opid = MMCB_HOOK_FREE;
            // Will be looked up in terms of addr, so update.
            ope->addr = old_addr;
            // The final capture will be done below.
        }
        // Area pointed to was not moved, but perhaps some other shuffling was
        // done.
        else {
            // I'm not sure if this is the best way to capture this... Ideas..?
            // First remove old entry. old_addr and addr should be equal.
            // This first bit should decrement memory usage by the old size.
            ope->opid = MMCB_HOOK_FREE;
            capture(ope);
            // Now increment memory usage by the new size.
            ope->opid = MMCB_HOOK_MALLOC;
            ope->size = size;
        }
        capture(ope);
    }

    /**
     *
     */
    void
    report(void) {
        using namespace std;

        cout << "High Memory Usage Watermark: "
             << tomb(high_mem_usage_mark)
             << " MB" << endl;
    }

private:

    /**
     *
     */
    double
    tomb(size_t inb) {
        return (double(inb) / 1024.0 / 1024.0);
    }

    /**
     *
     */
    void
    update_current_mem_allocd(
        mmcb_memory_op_entry *const ope
    ) {
        const uint8_t opid = ope->opid;
        const size_t size = ope->size;

        switch (opid) {
            case (MMCB_HOOK_MALLOC):
            case (MMCB_HOOK_CALLOC):
            case (MMCB_HOOK_POSIX_MEMALIGN):
                current_mem_allocd += size;
                break;
            case (MMCB_HOOK_FREE):
                current_mem_allocd -= size;
                break;
            case (MMCB_HOOK_NOOP):
                // Nothing to do.
                break;
            default:
                // Note: MMCB_HOOK_REALLOC is always broken down in terms of
                // other operations, so it will never reach this code path.
                assert(false && "Invalid opid");
        }
        update_high_mem_usage_mark();
    }

    /**
     *
     */
    void
    update_high_mem_usage_mark(void) {
        if (current_mem_allocd > high_mem_usage_mark) {
            high_mem_usage_mark = current_mem_allocd;
        }
    }
};
