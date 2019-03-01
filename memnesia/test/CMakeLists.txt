#
# Copyright (c) 2017-2019 Triad National Security, LLC
#                         All rights reserved.
#
# This file is part of the mpimemu project. See the LICENSE file at the
# top-level directory of this distribution.
#

add_executable(
    mpi-init
    mpi-init.c
)

add_executable(
    mpi-sendrecv
    mpi-sendrecv.c
)

add_executable(
    mpi-alltoall
    mpi-alltoall.c
)

add_executable(
    smaps-bench
    smaps-bench.cc
)
