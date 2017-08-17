#!/bin/bash

# Prints out MPI types and functions used in source.

dirs_to_search=( "$@" )

for dir in ${dirs_to_search[*]}; do
    echo "This is what I found in: $dir"
    egrep -Rho 'MPI_[A-Z][a-z]+[_]?[a-z]+' "$dir/"* | grep MPI_ | sort | uniq
    echo
done
