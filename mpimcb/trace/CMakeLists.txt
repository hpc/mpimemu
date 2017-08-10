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
    mpimcb-trace SHARED
    mpimcb-mem-common.h
    mpimcb-mem-interposers.c
    mpimcb-mem-hooks.cc
    mpimcb-trace.cc
)

target_link_libraries(mpimcb-trace adept_utils callpath)

include_directories(
    ${callpath_INCLUDE_DIR}
    ${adept_utils_INCLUDE_DIR}
)
# Remove the 'lib' prefix.
set_target_properties(mpimcb-trace PROPERTIES PREFIX "")
