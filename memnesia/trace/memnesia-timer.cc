/*
 * Copyright (c) 2017-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#include "memnesia-timer.h"

#include <chrono>

/**
 *
 */
double
memnesia_time(void)
{
    using namespace std::chrono;

    const auto n = steady_clock::now();
    auto tse_ms = time_point_cast<microseconds>(n).time_since_epoch().count();
    return (double(tse_ms) / 1e6);
}
