/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-timer.h"

#include <cstdlib>
#include <sys/time.h>
#include <sys/resource.h>

/**
 *
 */
double
mmcb_time(void)
{
    struct timeval tp;
    static long starts = 0, startu;
    if (!starts) {
        gettimeofday(&tp, NULL);
        starts = tp.tv_sec;
        startu = tp.tv_usec;
        return 0.0;
    }
    gettimeofday(&tp, NULL);
    return (double(tp.tv_sec  - starts)) +
           (double(tp.tv_usec - startu)) / 1000000.0;
}
