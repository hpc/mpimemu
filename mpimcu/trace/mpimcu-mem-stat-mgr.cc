/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcu-mem-stat-mgr.h"

mmcu_mem_stat_mgr *
mmcu_mem_stat_mgr::the_mmcu_mem_stat_mgr(void)
{
    static mmcu_mem_stat_mgr singleton;
    return &singleton;
}
