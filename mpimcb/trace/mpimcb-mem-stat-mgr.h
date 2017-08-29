/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#pragma once

#include "mpimcb-mem-hook-state.h"
#include "mpimcb-timer.h"

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <deque>
#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

class mmcb_memory_op_entry {
public:
    // Memory opteration ID.
    uint8_t opid;
    // Address associated with memory operation.
    uintptr_t addr;
    // If applicable, size associated with memory operation. Singed size_t
    // because some update operations will be negative.
    ssize_t size;
    // If applicable, 'old' address associated with memory operation. Mostly for
    // things like realloc.
    uintptr_t old_addr;

    /**
     *
     */
    mmcb_memory_op_entry(
        uint8_t opid,
        uintptr_t addr,
        ssize_t size = 0,
        uintptr_t old_addr = 0
    ) : opid(opid)
      , addr(addr)
      , size(size)
      , old_addr(old_addr) { }
};

class mmcb_proc_maps_entry {
public:
    // Address start.
    uintptr_t addr_start;
    // Address end.
    uintptr_t addr_end;
    // Max observed value of PSS (proportional set size).
    size_t pss_in_b;
    // Whether or not the region permissions say it is shared.
    bool reg_shared;
    // Path to backing store, if backed by a file.
    char path[PATH_MAX];

    /**
     *
     */
    mmcb_proc_maps_entry(void) {
        addr_start = 0;
        addr_end = 0;
        pss_in_b = 0;
        reg_shared = false;
        memset(path, '\0', sizeof(path));
    }

    void
    set_pss(size_t cur_pss_in_kb) {
        pss_in_b = cur_pss_in_kb * 1024;
    }
};

class mmcb_proc_smaps_parser {
    //
    enum {
        MMCB_PROC_MAPS_ADDR = 0,
        MMCB_PROC_MAPS_PERMS,
        MMCB_PROC_MAPS_OFFSET,
        MMCB_PROC_MAPS_DEV,
        MMCB_PROC_MAPS_INODE,
        MMCB_PROC_MAPS_PATH_NAME,
    };
    //
    static constexpr uint8_t n_tok = 6;
    //
    static constexpr int32_t max_entry_len = PATH_MAX;

public:
    /**
     *
     */
    static void
    get_proc_self_smaps_entry(
        uintptr_t target_addr,
        mmcb_proc_maps_entry &res_entry,
        bool &found_entry
    ) {
        found_entry = false;
        // First line format.
        // address           perms offset  dev   inode   pathname
        // 08048000-08056000 r-xp 00000000 03:0c 64593   /usr/sbin/gpm
        FILE *mapsf = fopen("/proc/self/smaps", "r");
        if (!mapsf) {
            perror("fopen /proc/self/smaps");
            exit(EXIT_FAILURE);
        }
        char cline_buff[2 * PATH_MAX];
        // Iterate over it one line at a time.
        while (fgets(cline_buff, sizeof(cline_buff) - 1, mapsf)) {
            char *tokp = nullptr, *strp = cline_buff;
            char toks[n_tok][PATH_MAX];
            // Tokenize.
            for (uint8_t i = 0;
                 i < n_tok && (NULL != (tokp = strtok(strp, " ")));
                 ++i
            ) {
                strp = nullptr;
                strncpy(toks[i], tokp, sizeof(toks[i]) - 1);
            }
            // Look for target address.
            uintptr_t addr_start = 0, addr_end = 0;
            get_addr_range(toks[MMCB_PROC_MAPS_ADDR], addr_start, addr_end);
            // If we found it, then process and return to caller.
            if (target_addr == addr_start) {
                found_entry = true;
                //
                res_entry.addr_start = addr_start;
                res_entry.addr_end = addr_end;
                res_entry.reg_shared = entry_has_shared_perms(
                    toks[MMCB_PROC_MAPS_PERMS]
                );
                // Stash path to file backing store only if shared.
                if (res_entry.reg_shared) {
                    strncpy(
                        res_entry.path,
                        toks[MMCB_PROC_MAPS_PATH_NAME],
                        sizeof(res_entry.path) - 1
                    );
                }
                // Found it, so bail.
                break;
            }
        }
        //
        if (!found_entry) {
#if 0
            // TODO add better error message.
            fprintf(
                stderr,
                "(pid: %d) WARNING: "
                "missing /proc/self/smaps entry!\n",
                (int)getpid()
            );
#endif
        }
        else {
            size_t pss_val = 0;
            get_pss(mapsf, pss_val);
            // In kilobytes.
            res_entry.set_pss(pss_val);
        }
        //
        fclose(mapsf);
    }

    /**
     *
     */
    static void
    get_pss(
        FILE *mapsf,
        size_t &pss_val
    ) {
        /**
         * Format
         * Size:               4100 kB
         * Rss:                 256 kB
         * Pss:                  81 kB
         */
        // Number of lines to ignore before we get to the PSS entry.
        static const uint8_t ni = 2;
        static const uint8_t n_tok = 3;
        static const char *errmsg = "ERROR: Invalid /proc/self/smaps format";
        char lb[2 * PATH_MAX];
        // Skip lines.
        for (uint8_t i = 0; i < ni && (fgets(lb, sizeof(lb) - 1, mapsf)); ++i) { }
        // Grab PSS entry.
        char *gets = fgets(lb, sizeof(lb) - 1, mapsf);
        if (!gets) {
            fprintf(stderr, "%s (%s)\n", errmsg, "PSS entry not found");
            exit(EXIT_FAILURE);
        }
        char *tokp = nullptr, *strp = lb;
        // Tokenize.
        char pss_tokens[n_tok][PATH_MAX];
        for (uint8_t i = 0;
             i < n_tok && (NULL != (tokp = strtok(strp, " ")));
             ++i
        ) {
            strp = nullptr;
            strncpy(pss_tokens[i], tokp, sizeof(pss_tokens[i]) - 1);
        }
        // Sanity (expecting kB).
        static const char *units = "kB";
        if (strncmp(units, pss_tokens[2], strlen(units)) != 0) {
            fprintf(stderr, "%s (%s)\n", errmsg, "PSS unit mismatch");
            exit(EXIT_FAILURE);
        }
        // Get the value.
        errno = 0;
        pss_val = (size_t)strtoull(pss_tokens[1], NULL, 10);
        int err = errno;
        if (err != 0) {
            perror("strtoull");
            exit(EXIT_FAILURE);
        }
    }

    /**
     *
     */
    static void
    get_addr_range(
        char addr_str_buff[max_entry_len],
        uintptr_t &start,
        uintptr_t &end
    ) {
        static const uint8_t n_addrs = 2;
        char addr_strs[n_addrs][128];
        char *strp = addr_str_buff;
        char *tokp = nullptr;
        // Tokenize.
        for (uint8_t i = 0;
             i < n_addrs && (NULL != (tokp = strtok(strp, "-")));
             ++i
        ) {
            strp = nullptr;
            strncpy(addr_strs[i], tokp, sizeof(addr_strs[i]) - 1);
        }
        // Convert hex strings to numerical values.
        for (uint8_t i = 0; i < n_addrs; ++i) {
            errno = 0;
            const uintptr_t caddr = (uintptr_t)strtoull(addr_strs[i], NULL, 16);
            int err = errno;
            if (err != 0) {
                perror("strtoull");
                exit(EXIT_FAILURE);
            }
            //
            if (i == 0) start = caddr;
            else end = caddr;
        }
    }

    /**
     *
     */
    static bool
    entry_has_shared_perms(
        char perms_buff[max_entry_len]
    ) {
        return 's' == perms_buff[3];
    }
};

class mmcb_mem_stat_mgr {
private:
    //
    static constexpr uint64_t mem_allocd_sample_freq = 1;
    //
    uint64_t num_captures = 0;
    //
    uint64_t n_mem_ops_recorded = 0;
    //
    uint64_t n_pss_updates_requested = 0;
    //
    uint64_t n_pss_updates = 0;
    //
    uint64_t n_mem_alloc_ops = 0;
    //
    uint64_t n_mem_free_ops = 0;
    //
    ssize_t current_mem_allocd = 0;
    //
    ssize_t high_mem_usage_mark = 0;
    // Mapping between address and memory operation entries.
    std::unordered_map<uintptr_t, mmcb_memory_op_entry *> addr2entry;
    // Mapping between address and mmap/munmap operation entries.
    std::unordered_map<uintptr_t, mmcb_memory_op_entry *> addr2mmap_entry;
    // Array of collected memory allocated samples.
    std::deque< std::pair<double, ssize_t> > mem_allocd_samples;
    //
    mmcb_mem_stat_mgr(void) = default;
    //
    ~mmcb_mem_stat_mgr(void)
    {
        // Leaky, but at least we won't crash at exit.
    }
    //
    mmcb_mem_stat_mgr(const mmcb_mem_stat_mgr &that) = delete;
    //
    mmcb_mem_stat_mgr &
    operator=(const mmcb_mem_stat_mgr &) = delete;

    /**
     *
     */
    void
    increment_num_captures(void) { ++num_captures; }

public:

    /**
     *
     */
    static mmcb_mem_stat_mgr *
    the_mmcb_mem_stat_mgr(void);

    /**
     *
     */
    void
    capture(
        mmcb_memory_op_entry *const ope
    ) {
        increment_num_captures();
        //
        bool rm_ope = false;
        const uintptr_t addr = ope->addr;
        const uint8_t opid = ope->opid;
        // Deal with any special cases first.
        switch (opid) {
            // realloc is never directly handled.
            case (MMCB_HOOK_REALLOC):
                break_down_realloc(ope);
                return;
            // mmap requires some extra work to extract its 'real' usage. Note
            // that this usage will change during the life of the application,
            // so we need to periodically update mmap entries. We'll do that
            // later for all captured mmaps. munmap also requires some special
            // care because mmap captures are stored in a different container.
            case (MMCB_HOOK_MMAP):
            case (MMCB_HOOK_MUNMAP):
                capture_mmap_ops(ope);
                return;
        }
        // Now deal with the entry.
        auto got = addr2entry.find(addr);
        // New entry.
        if (got == addr2entry.end()) {
            addr2entry.insert(std::make_pair(addr, ope));
        }
        // Existing entry and free.
        else if (opid == MMCB_HOOK_FREE) {
            ope->size = got->second->size;
            rm_ope = true;
        }
        else {
            fprintf(
                stderr,
                "(pid: %d) WARNING: "
                "existing entry (%p) not a free (OP: %d was OP: %d "
                "size: %lld\n",
                (int)getpid(),
                (void *)addr,
                (int)opid,
                (int)got->second->opid,
                (long long int)got->second->size
            );
            return;
        }
        update_current_mem_allocd(ope);
        update_all_pss_entries();
        //
        if (rm_ope) {
            addr2entry.erase(got);
        }
    }

    /**
     *
     */
    void
    report(
        int id,
        bool emit_report
    ) {
        using namespace std;
        //
        setbuf(stdout, NULL);
        //
        if (id == 0) {
            printf("\n#########################################################"
                   "\n# MPI Memory Consumption Analysis Complete ##############"
                   "\n#########################################################"
                   "\n");
        }
        //
        if (!emit_report) return;
        char *output_dir = getenv("MMCB_REPORT_OUTPUT_PATH");
        // Not set, so output to pwd.
        if (!output_dir) {
            output_dir = getenv("PWD");
        }
        if (!output_dir) {
            fprintf(stderr, "Error saving report.\n");
            return;
        }

        char report_name[PATH_MAX];
        snprintf(
            report_name, sizeof(report_name) - 1, "%s/%d.%s",
            output_dir, id, "mmcb"
        );

        FILE *reportf = fopen(report_name, "w+");
        if (!reportf) {
            fprintf(stderr, "Error saving report to %s.\n", report_name);
            return;
        }

        fprintf(reportf, "# Begin Report\n");

        fprintf(
            reportf,
            "# Number of Operation Captures Performed: %llu\n",
            (unsigned long long)num_captures
        );

        fprintf(reportf, "# Number of Memory Operations Recorded: %llu\n",
                (unsigned long long)n_mem_ops_recorded);

        fprintf(
            reportf,
            "# Number of Allocation-Related Operations Recorded: %llu\n",
            (unsigned long long)n_mem_alloc_ops
        );

        fprintf(
            reportf,
            "# Number of Deallocation-Related Operations Recorded: %llu\n",
            (unsigned long long)n_mem_free_ops
        );

        fprintf(
            reportf,
            "# Number of PSS Updates Performed: %llu\n",
            (unsigned long long)n_pss_updates
        );

        fprintf(
            reportf,
            "# High Memory Usage Watermark (MB): %lf\n",
            tomb(high_mem_usage_mark)
        );

        fprintf(reportf, "# Memory Usage (B) Over Time (Logical):\n");
        for (auto &i : mem_allocd_samples) {
            fprintf(
                reportf, "%lf %llu\n",
                i.first,
                (unsigned long long)i.second);
        }

        fprintf(reportf, "# End Report\n");

        fclose(reportf);

        if (id == 0) {
            printf("# Report written to %s\n", output_dir);
        }
    }

private:

    /**
     *
     */
    void
    update_all_pss_entries(void)
    {
        static const uint64_t update_freq = 10;

        if (n_pss_updates_requested++ % update_freq != 0) return;

        n_pss_updates++;

        for (auto &me : addr2mmap_entry) {
            mmcb_memory_op_entry *e = me.second;
            switch (e->opid) {
                case(MMCB_HOOK_MMAP_PSS_UPDATE): {
                    const ssize_t old_size = e->size;
                    // Next capture the new PSS value.
                    mmcb_proc_maps_entry maps_entry;
                    get_proc_self_smaps_entry(e->addr, maps_entry);
                    const ssize_t new_size = maps_entry.pss_in_b;
                    if (new_size >= old_size) {
                        e->size = new_size - old_size;
                    }
                    else {
                        // Free up old size.
                        e->size = -old_size;
                        update_current_mem_allocd(e);
                        // Now include new size.
                        e->size = new_size;
                    }
                    break;
                }
                case(MMCB_HOOK_MUNMAP):
                    // Nothing to do.
                    break;
                default:
                    fprintf(
                        stderr,
                        "(pid: %d) WARNING: unexpected opid (%d)\n",
                        (int)getpid(),
                        (int)e->opid
                    );
                    // Bail.
                    return;
            }
            //
            update_current_mem_allocd(e);
        }
    }

    /**
     *
     */
    void
    capture_mmap_ops(
        mmcb_memory_op_entry *const ope
    ) {
        bool rm_ope = false;
        const uintptr_t addr = ope->addr;
        const uint8_t opid = ope->opid;
        //
        auto got = addr2mmap_entry.find(addr);
        // New entry.
        if (got == addr2mmap_entry.end()) {
            assert(opid == MMCB_HOOK_MMAP);
            // Grab PSS stats.
            mmcb_proc_maps_entry maps_entry;
            get_proc_self_smaps_entry(addr, maps_entry);
            // Update opid.
            ope->opid = MMCB_HOOK_MMAP_PSS_UPDATE;
            // Update size.
            // The mmap length is initially captured, so update to PSS.
            ope->size = maps_entry.pss_in_b;
            // Add updated entry to map.
            addr2mmap_entry.insert(std::make_pair(addr, ope));
            // A new alloc operation not accounted for in capture because mmap
            // isn't recognized as a first-class operation.
            n_mem_alloc_ops++;
        }
        // Existing entry and munmap.
        else if (opid == MMCB_HOOK_MUNMAP) {
            // munmap already has the size, unlike free.
            rm_ope = true;
        }
        // Something went wrong.
        else {
            fprintf(
                stderr,
                "(pid: %d) WARNING: "
                "existing entry (%p) not a munmap (OP: %d)\n",
                (int)getpid(),
                (void *)addr,
                (int)opid
            );
            return;
        }
        //
        update_current_mem_allocd(ope);
        //
        if (rm_ope) {
            addr2mmap_entry.erase(got);
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
                n_mem_alloc_ops++;
                current_mem_allocd += size;
                break;
            case (MMCB_HOOK_FREE):
            case (MMCB_HOOK_MUNMAP):
                n_mem_free_ops++;
                current_mem_allocd -= size;
                break;
            case (MMCB_HOOK_MMAP_PSS_UPDATE):
                // Here size may be positive or negative.
                current_mem_allocd += size;
                break;
            case (MMCB_HOOK_NOOP):
                // Nothing to do.
                break;
            default:
                // Note: MMCB_HOOK_REALLOC and MMCB_HOOK_MMAP are always broken
                // down in terms of other operations, so they will never reach
                // this code path.
                assert(false && "Invalid opid");
        }
        update_mem_stats();
    }

    /**
     *
     */
    void
    update_mem_stats(void) {
        if (current_mem_allocd > high_mem_usage_mark) {
            high_mem_usage_mark = current_mem_allocd;
        }
        //
        if (n_mem_ops_recorded++ % mem_allocd_sample_freq == 0) {
            mem_allocd_samples.push_back(
                std::make_pair(mmcb_time(), current_mem_allocd)
            );
        }
    }

    /**
     *
     */
    void
    get_proc_self_smaps_entry(
        uintptr_t target_addr,
        mmcb_proc_maps_entry &res_entry
    ) {
        bool found_entry = false;
        static const int n_tries = 5;
        for (int i = 0; i < n_tries && !found_entry; ++i) {
            mmcb_proc_smaps_parser::get_proc_self_smaps_entry(
                target_addr, res_entry, found_entry
            );
        }
        if (!found_entry) {
            fprintf(
                stderr,
                "(pid: %d) WARNING: "
                "missing /proc/self/smaps entry!\n",
                (int)getpid()
            );
        }
    }
};
