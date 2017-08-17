/*
 * Copyright (c)      2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#include "mpimcb-mem-stat-mgr.h"

mmcb_mem_stat_mgr *
mmcb_mem_stat_mgr::the_mmcb_mem_stat_mgr(void)
{
    static mmcb_mem_stat_mgr singleton;
    return &singleton;
}
