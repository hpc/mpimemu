#pragma once

#include "memnesia.h"
#include "memnesia-sample.h"

#include <limits.h>

#include <string>
#include <vector>
#include <map>

class memnesia_rt {
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
    memnesia_dataset dataset;
    //
    memnesia_rt(void) {
        (void)memset(hostname, '\0', sizeof(hostname));
        (void)memset(app_comm, '\0', sizeof(app_comm));
    }
    //
    ~memnesia_rt(void) = default;
    //
    memnesia_rt(const memnesia_rt &that) = delete;
    //
    memnesia_rt &
    operator=(const memnesia_rt &) = delete;
    //
    void
    set_hostname(void);
    //
    void
    set_target_cmdline(void);
    //
    static void
    sample_delta(
        const memnesia_sample &happened_before,
        const memnesia_sample &happened_after,
        memnesia_sample &delta
    );

public:
    int rank = 0;
    //
    int numpe = 0;
    //
    static memnesia_rt *
    the_memnesia_rt(void);
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
        memnesia_sample &res
    );
    //
    static void
    sample_emit(
        const memnesia_sample &s
    );
    //
    void
    add_samples_to_dataset(
        const memnesia_sample &happened_before,
        const memnesia_sample &happened_after
    );
    //
    void
    report(void);
};

class memnesia_scoped_data_collector {
    //
    memnesia_rt *rt = nullptr;
    //
    std::string callers_name;
    //
    memnesia_sample before, after, delta;
    //
    memnesia_scoped_data_collector(void) = default;

public:
    //
    memnesia_scoped_data_collector(
        const std::string &callers_name
    ) : rt(memnesia_rt::the_memnesia_rt())
      , callers_name(callers_name)
    {
        rt->sample(callers_name, before);
    }
    //
    ~memnesia_scoped_data_collector(void)
    {
        rt->sample(callers_name, after);
        rt->add_samples_to_dataset(before, after);
    }
};
