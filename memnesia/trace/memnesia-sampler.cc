/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "memnesia-sampler.h"

#include <limits.h>

#include <cstdlib>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

namespace {
class smaps_parser {
    //
    static constexpr size_t lbuff_size = 2 * PATH_MAX;
    //
    static constexpr size_t gets_size = lbuff_size - 1;
    //
    static constexpr size_t max_toks = 32;
    //
    static constexpr bool eop = true;
    //
    static int64_t
    to_int64(const string &str)
    {
        return int64_t(std::stoll(str, 0, 10));
    }
    //
    static void
    tok_it(
        char *buff,
        const string &delim,
        vector<string> &toks
    ) {
        char *tokp = nullptr, *strp = buff;
        while ((tokp = strtok(strp, delim.c_str()))) {
            toks.push_back(string(tokp));
            strp = nullptr;
        }
    }
    //
    static bool
    has_suffix(
        const std::string &str,
        const std::string &suffix
    ) {
        return str.size() >= suffix.size() &&
               str.compare(
                    str.size() - suffix.size(),
                    suffix.size(), suffix
                ) == 0;
    }
    //
    static bool
    skip_entry(char *header)
    {
        // Format
        // address           perms offset   dev   inode   pathname
        // 08048000-08056000 r-xp  00000000 03:0c 64593   /usr/sbin/gpm
        vector<string> toks(max_toks);
        toks.reserve(max_toks);
        tok_it(header, " ", toks);
        string pathname = toks.back();
        // Remove '\n'
        pathname = pathname.substr(0, pathname.length() - 1);
        // Skip all entries that end with memnesia-trace.so
        static const string trace_lib("memnesia-trace.so");
        return has_suffix(pathname, trace_lib);
    }
    //
    static bool
    parse_header(
        FILE *smapsf,
        bool &add_entry_to_tally
    ) {
        char lbuff[lbuff_size];

        char *gets = fgets(lbuff, gets_size, smapsf);

        if (!gets) return eop;

        add_entry_to_tally = !skip_entry(gets);

        return !eop;
    }
    //
    static bool
    parse_body(
        FILE *smapsf,
        bool add_entry_to_tally,
        memnesia_smaps_sampler::sample &sample
    ) {
        char lbuff[lbuff_size];
        bool hit_last = false;
        static const uint8_t kidx = 0, vidx = 1, uidx = 2;
        // Format:
        // 0                     1    2
        // Key:                  Size Units
        while (fgets(lbuff, gets_size, smapsf)) {
            vector<string> toks;
            toks.reserve(max_toks);
            tok_it(lbuff, " ", toks);
            // Remove ':'
            string key   = toks[kidx].substr(0, toks[kidx].length() - 1);
            // Nothing to remove.
            string value = toks[vidx];
            // Remove '\n'
            string units = toks[uidx].substr(0, toks[uidx].length() - 1);
            auto got = memnesia_smaps_sampler::tokid_tab.find(key);
            // We found a key that we care about.
            if (got != memnesia_smaps_sampler::tokid_tab.end()) {
                const memnesia_smaps_sampler::entry_id idx = got->second;
                // Done!
                if (memnesia_smaps_sampler::LAST == idx) {
                    hit_last = true;
                    break;
                }
                // Else, continue processing.
                if (add_entry_to_tally) {
                    sample.data_in_kb[idx] += to_int64(value);
                }
#if 1 // Debug memnesia memory usage using the PSS metric.
                else {
                    static uint64_t memnesia_pss = 0;
                    if (memnesia_smaps_sampler::PSS == idx) {
                        memnesia_pss += to_int64(value);
                        std::cout << "MEMNESIA-PSS(kB)=" << memnesia_pss << std::endl;
                    }
                }
#endif
                // Sanity
                static const string exp_units("kB");
                if (exp_units != units) {
                    fprintf(
                        stderr,
                        "PARSE ERROR: Unexpected units: "
                        "expected \'%s\', but got \'%s\'.\n",
                        exp_units.c_str(),
                        units.c_str()
                    );
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(
                    stderr,
                    "PARSE ERROR: Unexpected format: key \'%s\' not found.\n",
                    key.c_str()
                );
                exit(EXIT_FAILURE);
            }
        }
        return (hit_last ? !eop : eop);
    }

public:

    //
    static memnesia_smaps_sampler::sample
    parse(void)
    {
        static const char *f_name = "/proc/self/smaps";

        FILE *smapsf = fopen(f_name, "r");
        if (!smapsf) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        memnesia_smaps_sampler::sample result;
        bool add_entry_to_tally = false;
        while (true) {
            if (eop == parse_header(smapsf, add_entry_to_tally)) break;
            if (eop == parse_body(smapsf, add_entry_to_tally, result)) break;
        }

        fclose(smapsf);

        return result;
    }
};

} // namespace

map<string, memnesia_smaps_sampler::entry_id>
memnesia_smaps_sampler::tokid_tab = {
    make_pair("Size",            memnesia_smaps_sampler::SIZE),
    make_pair("Rss",             memnesia_smaps_sampler::RSS),
    make_pair("Pss",             memnesia_smaps_sampler::PSS),
    make_pair("Shared_Clean",    memnesia_smaps_sampler::SHARED_CLEAN),
    make_pair("Shared_Dirty",    memnesia_smaps_sampler::SHARED_DIRTY),
    make_pair("Private_Clean",   memnesia_smaps_sampler::PRIVATE_CLEAN),
    make_pair("Private_Dirty",   memnesia_smaps_sampler::PRIVATE_DIRTY),
    make_pair("Referenced",      memnesia_smaps_sampler::REFERENCED),
    make_pair("Anonymous",       memnesia_smaps_sampler::ANONYMOUS),
    make_pair("AnonHugePages",   memnesia_smaps_sampler::ANONHUGEPAGES),
    make_pair("ShmemPmdMapped",  memnesia_smaps_sampler::SHMEMPMDMAPPED),
    make_pair("Shared_Hugetlb",  memnesia_smaps_sampler::SHARED_HUGETLB),
    make_pair("Private_Hugetlb", memnesia_smaps_sampler::PRIVATE_HUGETLB),
    make_pair("Swap",            memnesia_smaps_sampler::SWAP),
    make_pair("SwapPss",         memnesia_smaps_sampler::SWAPPSS),
    make_pair("KernelPageSize",  memnesia_smaps_sampler::KERNELPAGESIZE),
    make_pair("MMUPageSize",     memnesia_smaps_sampler::MMUPAGESIZE),
    make_pair("Locked",          memnesia_smaps_sampler::LOCKED),
    // We rely on this being the last element for a given entry.
    make_pair("VmFlags",         memnesia_smaps_sampler::LAST)
};

/**
 *
 */
memnesia_smaps_sampler::sample
memnesia_smaps_sampler::get_sample_impl(void)
{
    return smaps_parser::parse();
}
