# Copyright (c)      2017 Los Alamos National Security, LLC.
#                         All rights reserved.
#
# This program was prepared by Los Alamos National Security, LLC at Los Alamos
# National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with the U.S.
# Department of Energy (DOE). All rights in the program are reserved by the DOE
# and Los Alamos National Security, LLC. Permission is granted to the public to
# copy and use this software without charge, provided that this Notice and any
# statement of authorship are reproduced on all copies. Neither the U.S.
# Government nor LANS makes any warranty, express or implied, or assumes any
# liability or responsibility for the use of this software.

################################################################################
add_library(
    mpimcu-rt STATIC
    mpimcu.h
    mpimcu-sample.h
    mpimcu-sampler.h mpimcu-sampler.cc
    mpimcu-timer.h mpimcu-timer.cc
    mpimcu-rt.h mpimcu-rt.cc
)

set_property(
    TARGET
    mpimcu-rt
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
add_library(
    mpimcu-trace SHARED
    mpimcu-pmpi.cc
)
set_property(
    TARGET
    mpimcu-trace
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
target_link_libraries(
    mpimcu-trace
    mpimcu-rt
)

# Remove the 'lib' prefix.
set_target_properties(mpimcu-trace PROPERTIES PREFIX "")
