#pragma once

#include "mpimcu.h"
#include "mpimcu-sample.h"

#include <limits.h>

#include <string>
#include <vector>
#include <map>

class mmcu_rt {
private:
    //
    double init_begin_time = 0.0;
    //
    double init_end_time = 0.0;
    //
    char hostname[256];
    //
    char app_comm[PATH_MAX];
    //
    mmcu_dataset dataset;
    //
    mmcu_rt(void) {
        (void)memset(hostname, '\0', sizeof(hostname));
        (void)memset(app_comm, '\0', sizeof(app_comm));
    }
    //
    ~mmcu_rt(void) = default;
    //
    mmcu_rt(const mmcu_rt &that) = delete;
    //
    mmcu_rt &
    operator=(const mmcu_rt &) = delete;
    //
    void
    set_hostname(void);
    //
    void
    set_target_cmdline(void);

public:
    int rank = 0;
    //
    int numpe = 0;
    //
    static mmcu_rt *
    the_mmcu_rt(void);
    //
    void
    emit_header(void);
    //
    void
    set_init_begin_time_now(void);
    //
    void
    set_init_end_time_now(void);
    //
    std::string
    get_date_time_str_now(void);
    //
    std::string
    get_hostname(void);
    //
    std::string
    get_app_name(void);
    //
    void
    gather_target_metadata(void);
    //
    double
    get_init_begin_time(void);
    //
    double
    get_init_end_time(void);
    //
    void
    sample(
        const std::string &what,
        mmcu_sample &res
    );
    //
    static void
    sample_delta(
        const mmcu_sample &happened_before,
        const mmcu_sample &happened_after,
        mmcu_sample &delta
    );
    //
    static void
    sample_emit(
        const mmcu_sample &s
    );
    //
    void
    store_sample(
        const mmcu_sample &happened_before,
        const mmcu_sample &happened_after
    );
    //
    void
    report(void);
};

#define mmcu_rt_sample(mmcu_rtp, mmcu_samp_res)                                \
do {                                                                           \
    mmcu_rtp->sample(MMCU_FUNC, mmcu_samp_res);                                \
} while (0)
