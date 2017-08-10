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

project(mpimcb)

cmake_minimum_required(VERSION 3.1)

# Enable C++11 support.
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# rpath everything.
option(CMAKE_INSTALL_RPATH_USE_LINK_PATH "Add rpath for all dependencies." TRUE)

function(emit_package_status name)
  if (${name}_FOUND)
    message(STATUS "Using ${name} found in ${${name}_DIR}.")
  endif()
endfunction()

find_package(adept_utils REQUIRED)
emit_package_status(adept_utils)

find_package(callpath REQUIRED)
emit_package_status(callpath)

add_subdirectory(trace)
