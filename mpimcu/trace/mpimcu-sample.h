/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#pragma once

#include "mpimcu-timer.h"
#include "mpimcu-sampler.h"

#include <string>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

class mmcu_sample {
    //
    std::string target_func_name;
    //
    double capture_time = 0.0;
    // Only valid for deltas.
    double duration = 0.0;
    //
    mmcu_smaps_sampler::sample smaps;

public:
    //
    enum get_id {
        MEM_USAGE = 0
    };
    //
    mmcu_sample(void) = default;
    //
    mmcu_sample(
        std::string func_name
    ) : target_func_name(func_name)
      , capture_time(mmcu_time())
      , smaps(mmcu_smaps_sampler::get_sample()) { }
    //
    double
    get_capture_time(void) const {
        return capture_time;
    }
    //
    double
    get_duration(void) const {
        return duration;
    }
    //
    int64_t
    get_mem_usage_in_kb(void) const {
        return smaps.data_in_kb[mmcu_smaps_sampler::PSS];
    }
    //
    static void
    delta(
        const mmcu_sample &happened_before,
        const mmcu_sample &happened_after,
        mmcu_sample &delta
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
        mmcu_smaps_sampler::sample::delta(
            before.smaps,
            after.smaps,
            delta.smaps
        );
    }
    //
    static void
    emit(
        const mmcu_sample &s
    ) {
        using namespace std;

        cout << "# Sample ########################################" << endl;
        cout << "# Function Name: " << s.target_func_name << endl;
        cout << "# Capture Time : " << s.capture_time << endl;
        cout << "# Duration     : " << s.duration << endl;
        mmcu_smaps_sampler::sample::emit(s.smaps);
        cout << "# ###############################################" << endl;
    }
};

class mmcu_dataset {
public:
    //
    enum type_id {
        MPI = 0,
        APP,
        LAST
    };

private:
    //
    std::map< type_id, std::vector<mmcu_sample> > data;
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
        const mmcu_sample &sample
    ) {
        data[tid].push_back(sample);
    }
    //
    void
    report(
        FILE *tof,
        type_id tid,
        double since
    ) {
        for (const auto &d : data[tid]) {
            fprintf(
                tof,
                "%s %lf %" PRId64 "\n",
                tid_name_tab[tid].c_str(),
                d.get_capture_time() - since,
                d.get_mem_usage_in_kb() * 1024
            );
        }
    }
};
