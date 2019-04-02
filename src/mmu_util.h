/*
 * Copyright (c) 2010-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#ifndef MMU_UTIL_INCLUDED
#define MMU_UTIL_INCLUDED

#include "mmu_constants.h"

const char *
mmu_util_rc2str(int rc);

int
mmu_util_str_to_ull(const char *str,
                    unsigned long long int *val_if_valid);

#endif /* ifndef MMU_UTIL_INCLUDED */
