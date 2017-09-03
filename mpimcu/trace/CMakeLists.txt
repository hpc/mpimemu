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
    mpimcu-mem-hook-state.h
    mpimcu-mem-hook-state.c
    mpimcu-mem-hooks.h
    mpimcu-mem-hooks.cc
    mpimcu-rt.h
    mpimcu-rt.cc
)

set_property(
    TARGET
    mpimcu-rt
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
add_library(
    mpimcu-mem-stat-mgr STATIC
    mpimcu-mem-stat-mgr.h
    mpimcu-mem-stat-mgr.cc
)

set_property(
    TARGET
    mpimcu-mem-stat-mgr
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
add_library(
    mpimcu-timer STATIC
    mpimcu-timer.h
    mpimcu-timer.cc
)
set_property(
    TARGET
    mpimcu-timer
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

################################################################################
add_library(
    mpimcu-trace SHARED
    mpimcu-mem-interposers.c
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
    mpimcu-mem-stat-mgr
    mpimcu-timer
    ${CMAKE_DL_LIBS}
    pthread
)

# Remove the 'lib' prefix.
set_target_properties(mpimcu-trace PROPERTIES PREFIX "")
