/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcu-sampler.h"

#include <limits.h>

#include <cstdlib>
#include <utility>
#include <deque>
#include <iostream>

using namespace std;

namespace {
class smaps_parser {
    //
    static constexpr size_t lbuff_size = 2 * PATH_MAX;
    //
    static constexpr size_t gets_size = lbuff_size - 1;
    //
    static constexpr bool eop = true;
    //
    static int64_t
    to_int64(const string &str)
    {
        return int64_t(std::stoll(str, 0, 10));
    }
    //
    static bool
    parse_header(FILE *smapsf) {
        // TODO skip our instrumentation stuff and let parse_rest know.
        char lbuff[lbuff_size];

        char *gets = fgets(lbuff, gets_size, smapsf);

        if (!gets) return eop;

        return !eop;
    }
    //
    static void
    tok_it(
        char *buff,
        const string &delim,
        deque<string> &toks
    ) {
        char *tokp = nullptr, *strp = buff;
        while ((tokp = strtok(strp, delim.c_str()))) {
            toks.push_back(string(tokp));
            strp = nullptr;
        }
    }
    //
    static bool
    parse_rest(
        FILE *smapsf,
        mmcu_smaps_sampler::sample &sample
    ) {
        char lbuff[lbuff_size];
        bool hit_last = false;
        static const uint8_t kidx = 0, vidx = 1, uidx = 2;
        // Format:
        // 0                     1    2
        // Key:                  Size Units
        while (fgets(lbuff, gets_size, smapsf)) {
            deque<string> toks;
            tok_it(lbuff, " ", toks);
            // Remove ':'
            string key   = toks[kidx].substr(0, toks[kidx].length() - 1);
            // Nothing to remove.
            string value = toks[vidx];
            // Remove '\n'
            string units = toks[uidx].substr(0, toks[uidx].length() - 1);
            auto got = mmcu_smaps_sampler::tokid_tab.find(key);
            // We found a key that we care about.
            if (got != mmcu_smaps_sampler::tokid_tab.end()) {
                const mmcu_smaps_sampler::entry_id idx = got->second;
                // Done!
                if (mmcu_smaps_sampler::LAST == idx) {
                    hit_last = true;
                    break;
                }
                // Else, continue processing.
                sample.data_in_kb[idx] += to_int64(value);
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

    /**
     *
     */
    static mmcu_smaps_sampler::sample
    parse(void)
    {
        static const char *f_name = "/proc/self/smaps";

        FILE *smapsf = fopen(f_name, "r");
        if (!smapsf) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        mmcu_smaps_sampler::sample result;
        while (true) {
            if (eop == parse_header(smapsf)) break;
            if (eop == parse_rest(smapsf, result)) break;
        }

        fclose(smapsf);

        return result;
    }
};

} // namespace

map<string, mmcu_smaps_sampler::entry_id> mmcu_smaps_sampler::tokid_tab = {
    make_pair("Size",            mmcu_smaps_sampler::SIZE),
    make_pair("Rss",             mmcu_smaps_sampler::RSS),
    make_pair("Pss",             mmcu_smaps_sampler::PSS),
    make_pair("Shared_Clean",    mmcu_smaps_sampler::SHARED_CLEAN),
    make_pair("Shared_Dirty",    mmcu_smaps_sampler::SHARED_DIRTY),
    make_pair("Private_Clean",   mmcu_smaps_sampler::PRIVATE_CLEAN),
    make_pair("Private_Dirty",   mmcu_smaps_sampler::PRIVATE_DIRTY),
    make_pair("Referenced",      mmcu_smaps_sampler::REFERENCED),
    make_pair("Anonymous",       mmcu_smaps_sampler::ANONYMOUS),
    make_pair("AnonHugePages",   mmcu_smaps_sampler::ANONHUGEPAGES),
    make_pair("ShmemPmdMapped",  mmcu_smaps_sampler::SHMEMPMDMAPPED),
    make_pair("Shared_Hugetlb",  mmcu_smaps_sampler::SHARED_HUGETLB),
    make_pair("Private_Hugetlb", mmcu_smaps_sampler::PRIVATE_HUGETLB),
    make_pair("Swap",            mmcu_smaps_sampler::SWAP),
    make_pair("SwapPss",         mmcu_smaps_sampler::SWAPPSS),
    make_pair("KernelPageSize",  mmcu_smaps_sampler::KERNELPAGESIZE),
    make_pair("MMUPageSize",     mmcu_smaps_sampler::MMUPAGESIZE),
    make_pair("Locked",          mmcu_smaps_sampler::LOCKED),
    // We rely on this being the last element for a given entry.
    make_pair("VmFlags",         mmcu_smaps_sampler::LAST)
};

/**
 *
 */
mmcu_smaps_sampler::sample
mmcu_smaps_sampler::get_sample_impl(void)
{
    return smaps_parser::parse();
}
