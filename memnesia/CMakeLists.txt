#
# Copyright (c) 2017-2019 Triad National Security, LLC
#                         All rights reserved.
#
# This file is part of the mpimemu project. See the LICENSE file at the
# top-level directory of this distribution.
#

project(memnesia)

cmake_minimum_required(VERSION 3.1)

# Enable C++11 support.
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wextra -g \
    -D_GNU_SOURCE -D__STDC_FORMAT_MACROS \
    -Wno-deprecated-declarations"
)

set(
    CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wall -Wextra -g \
    -D_GNU_SOURCE -D__STDC_FORMAT_MACROS \
    -Wno-deprecated-declarations"
)

# rpath everything.
option(
    CMAKE_INSTALL_RPATH_USE_LINK_PATH
    "Add rpath for all dependencies."
    TRUE
)

function(emit_package_status name)
  if (${name}_FOUND)
    message(STATUS "Using ${name} found in ${${name}_DIR}.")
  endif()
endfunction()

add_subdirectory(trace)
add_subdirectory(test)
