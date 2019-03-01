/*
 * Copyright (c) 2017-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#pragma once

#include "memnesia.h"
#include "memnesia-timer.h"
#include "memnesia-sampler.h"

#include <sstream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

class memnesia_sample {
    //
    std::string target_func_name;
    //
    double capture_time = 0.0;
    // Only valid for deltas.
    double duration = 0.0;
    //
    memnesia_smaps_sampler::sample smaps;

public:
    //
    memnesia_sample(void) = default;
    //
    memnesia_sample(
        const std::string &func_name
    ) : target_func_name(func_name)
      , capture_time(memnesia_time())
      , smaps(memnesia_smaps_sampler::get_sample()
    ) { }
    //
    std::string
    get_target_func_name(void) const
    {
        return target_func_name;
    }
    //
    double
    get_capture_time(void) const
    {
        return capture_time;
    }
    //
    double
    get_duration(void) const
    {
        return duration;
    }
    //
    int64_t
    get_mem_usage_in_kb(
        int64_t *running_total = nullptr
    ) const {
        const int64_t samp_usage = smaps.data_in_kb[memnesia_smaps_sampler::PSS];
        if (running_total) {
            *running_total += samp_usage;
            return *running_total;
        }
        return samp_usage;
    }
    //
    static void
    delta(
        const memnesia_sample &happened_before,
        const memnesia_sample &happened_after,
        memnesia_sample &delta
    ) {
        using namespace std;

        auto &before = happened_before;
        auto &after = happened_after;

        string func_name;
        // The same, so pick one.
        if (before.target_func_name == after.target_func_name) {
            func_name = after.target_func_name;
        }
        // Different, so make it a name range. That is, to convey 'MPI_Init to
        // MPI_Finalize' use: 'MPI_Init-MPI_Finalize'.
        else {
            func_name = before.target_func_name + "-" + after.target_func_name;
        }

        delta.target_func_name = func_name;
        delta.capture_time = after.capture_time;
        delta.duration = after.capture_time - before.capture_time;
        //
        memnesia_smaps_sampler::sample::delta(
            before.smaps,
            after.smaps,
            delta.smaps
        );
    }
    //
    static void
    emit(
        const memnesia_sample &s
    ) {
        using namespace std;

        cout << "# Sample ########################################" << endl;
        cout << "# Function Name: " << s.target_func_name << endl;
        cout << "# Capture Time : " << s.capture_time << endl;
        cout << "# Duration     : " << s.duration << endl;
        memnesia_smaps_sampler::sample::emit(s.smaps);
        cout << "# ###############################################" << endl;
    }
};

class memnesia_dataset {
public:
    //
    enum type_id {
        MPI = 0,
        APP,
        LAST
    };

private:
    //
    std::map< type_id, std::vector<memnesia_sample> > data;
    //
    std::vector<std::string> tid_name_tab {
        "MPI_MEM_USAGE",
        "ALL_MEM_USAGE"
    };

public:
    //
    void
    push_back(
        type_id tid,
        const memnesia_sample &sample
    ) {
        data[tid].push_back(sample);
    }
    //
    int64_t
    length(
        type_id tid
    ) {
        return int64_t(data[tid].size());
    }
    //
    void
    report(
        std::stringstream &ss,
        type_id tid,
        double since
    ) {
        int64_t mem_total = 0;
        // To keep track of MPI usage, we have to sum the deltas. APP usage is
        // calculated by just using the sample values at any given point.
        int64_t *mtbp = (MPI == tid ? &mem_total : nullptr);

        for (const auto &d : data[tid]) {
            ss << tid_name_tab[tid] << " "
               << d.get_target_func_name() << " "
               << d.get_capture_time() - since << " "
               <<  memnesia_util_kb2mb(d.get_mem_usage_in_kb(mtbp))
               << std::endl;
        }
    }
    //
    int64_t
    get_high_mem_usage_watermark_in_kb(
        type_id tid
    ) {
        int64_t maxv = 0, mem_total = 0;
        int64_t *mtbp = (MPI == tid ? &mem_total : nullptr);

        for (const auto &d : data[tid]) {
            const auto cval = d.get_mem_usage_in_kb(mtbp);
            maxv = cval > maxv ? cval : maxv;
        }

        return maxv;
    }
};
