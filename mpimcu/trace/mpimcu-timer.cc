/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcu-timer.h"

#include <chrono>

/**
 *
 */
double
mmcu_time(void)
{
    using namespace std::chrono;

    const auto n = steady_clock::now();
    auto tse_ms = time_point_cast<microseconds>(n).time_since_epoch().count();
    return (double(tse_ms) / 1e6);
}
