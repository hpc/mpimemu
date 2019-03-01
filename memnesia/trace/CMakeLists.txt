#
# Copyright (c) 2017-2019 Triad National Security, LLC
#                         All rights reserved.
#
# This file is part of the mpimemu project. See the LICENSE file at the
# top-level directory of this distribution.
#

################################################################################
add_library(
    memnesia-rt STATIC
    memnesia.h
    memnesia-sample.h
    memnesia-sampler.h memnesia-sampler.cc
    memnesia-timer.h memnesia-timer.cc
    memnesia-rt.h memnesia-rt.cc
)

set_property(
    TARGET
    memnesia-rt
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
add_library(
    memnesia-trace SHARED
    memnesia-pmpi.cc
)
set_property(
    TARGET
    memnesia-trace
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
target_link_libraries(
    memnesia-trace
    memnesia-rt
)

# Remove the 'lib' prefix.
set_target_properties(memnesia-trace PROPERTIES PREFIX "")
