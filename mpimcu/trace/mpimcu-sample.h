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
    mmcu_sample(void) = default;
    //
    mmcu_sample(
        std::string func_name
    ) : target_func_name(func_name)
      , capture_time(mmcu_time())
      , smaps(mmcu_smaps_sampler::get_sample()) { }
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
#if 0
"MPI_MEM_USAGE",
"ALL_MEM_USAGE",
#endif
    // FIXME
    static void
    report(
        FILE *tof,
        const std::vector<mmcu_sample> &samples,
        double since
    ) {
    }
};

class mmcu_dataset {
    //
    enum type {
        MPI,
        APP
    };
    //
    std::map< sample_type, std::vector<mmcu_sample> > samples;
};
