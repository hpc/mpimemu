/*
 * Copyright (c) 2017-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#pragma once

#include <inttypes.h>
#include <string.h>

#include <iostream>
#include <map>

class memnesia_smaps_sampler {
public:
    //
    enum entry_id {
        SIZE = 0,
        RSS,
        PSS,
        SHARED_CLEAN,
        SHARED_DIRTY,
        PRIVATE_CLEAN,
        PRIVATE_DIRTY,
        REFERENCED,
        ANONYMOUS,
        ANONHUGEPAGES,
        SHMEMPMDMAPPED,
        SHARED_HUGETLB,
        PRIVATE_HUGETLB,
        SWAP,
        SWAPPSS,
        KERNELPAGESIZE,
        MMUPAGESIZE,
        LOCKED,
        LAST
    };
    //
    static std::map<std::string, entry_id> tokid_tab;
    //
    struct sample {
        //
        int64_t data_in_kb[entry_id::LAST];
        //
        sample(void)
        {
            (void)memset(data_in_kb, 0, sizeof(data_in_kb));
        }
        //
        static void
        delta(
            const sample &happened_before,
            const sample &happened_after,
            sample &delta
        ) {
            auto &b = happened_before;
            auto &a = happened_after;
            for (int i = 0; i < entry_id::LAST; ++i) {
                delta.data_in_kb[i] = a.data_in_kb[i] - b.data_in_kb[i];
            }
        }
        //
        static void
        emit(
            const sample &s
        ) {
            using namespace std;

            cout << "# smaps -----------------------------------------" << endl;
            for (auto &i : memnesia_smaps_sampler::tokid_tab) {
                const auto idx = i.second;
                if (idx == entry_id::LAST) break;
                cout << i.first << ": " << s.data_in_kb[idx] << " kB" << endl;
            }
            cout << "# -----------------------------------------------" << endl;
        }
    };

private:
    //
    static sample
    get_sample_impl(void);

public:
    //
    static sample
    get_sample(void)
    {
        return get_sample_impl();
    }
};
