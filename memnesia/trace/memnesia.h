#pragma once

#include <cstdlib>

#define MEMNESIA_FUNC __func__

#define MEMNESIA_ENV_REPORT_OUTPUT_PATH "MEMNESIA_REPORT_OUTPUT_PATH"

template<typename T>
static inline double
memnesia_util_kb2mb(T kb) {
    return double(kb) / 1024.0;
}

static inline void
memnesia_exit_failure(void)
{
    exit(EXIT_FAILURE);
}
