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

add_library(
    mpimcb-rt STATIC
    mpimcb-mem-hook-state.h
    mpimcb-mem-hook-state.c
    mpimcb-mem-hooks.h
    mpimcb-mem-hooks.cc
    mpimcb-rt.h
    mpimcb-rt.cc
)

set_property(
    TARGET
    mpimcb-rt 
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

add_library(
    mpimcb-mem-stat-mgr STATIC
    mpimcb-mem-stat-mgr.h
    mpimcb-mem-stat-mgr.cc
)

set_property(
    TARGET
    mpimcb-mem-stat-mgr
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

add_library(
    mpimcb-trace SHARED
    mpimcb-mem-interposers.c
    mpimcb-pmpi.cc
)

target_link_libraries(
    mpimcb-trace
    mpimcb-rt
    mpimcb-mem-stat-mgr
    ${CMAKE_DL_LIBS}
    pthread
)

# Remove the 'lib' prefix.
set_target_properties(mpimcb-trace PROPERTIES PREFIX "")
