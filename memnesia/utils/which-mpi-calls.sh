#
# Copyright (c) 2017-2019 Triad National Security, LLC
#                         All rights reserved.
#
# This file is part of the mpimemu project. See the LICENSE file at the
# top-level directory of this distribution.
#

#!/bin/bash

# Prints out MPI types and functions used in source.

dirs_to_search=( "$@" )

for dir in ${dirs_to_search[*]}; do
    echo "This is what I found in: $dir"
    egrep -Rho 'MPI_[A-Z][a-z]+[_]?[a-z]+' "$dir/"* | grep MPI_ | sort | uniq
    echo
done
