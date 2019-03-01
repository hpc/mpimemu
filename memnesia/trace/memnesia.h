/*
 * Copyright (c) 2017-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#pragma once

#include <cstdlib>

#define MEMNESIA_FUNC __func__

#define MEMNESIA_ENV_REPORT_OUTPUT_PATH "MEMNESIA_REPORT_OUTPUT_PATH"
#define MEMNESIA_ENV_REPORT_NAME        "MEMNESIA_REPORT_NAME"

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
