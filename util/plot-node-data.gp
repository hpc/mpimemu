###############################################################################
# Copyright (c) 2010-2013 Los Alamos National Security, LLC.
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

# Author: Samuel K. Gutierrez

# plots mpi_mem_usage node memory usage .csv files

title_v=\
"TITLE"

set terminal postscript enhanced color
set output '| ps2pdf - NAME.pdf'

set title title_v
set xlabel "Number of MPI Tasks"
set ylabel "Memory Usage (kB)"

plot \
'./node-mem-usage.csv' using 1:2 with linespoints title 'pre ave memused', \
'./node-mem-usage.csv' using 1:3 with linespoints title 'pre ave cached', \
'./node-mem-usage.csv' using 1:4 with linespoints title 'pre ave active', \
'./node-mem-usage.csv' using 1:5 with linespoints title 'pre ave inactive', \
'./node-mem-usage.csv' using 1:6 with linespoints title 'ave memused', \
'./node-mem-usage.csv' using 1:7 with linespoints title 'ave cached', \
'./node-mem-usage.csv' using 1:8 with linespoints title 'ave active', \
'./node-mem-usage.csv' using 1:9 with linespoints title 'ave inactive', \
'./node-mem-usage.csv' using 1:(column(2) - column(3)) with \
linespoints title 'ave mem used (pre ave memused - pre ave cached)', \
'./node-mem-usage.csv' using 1:(column(6) - column(7)) with \
linespoints title 'ave mem used (ave memused - ave cached)'

# NOTES
# memory usage from the user's standpoint is best represented by:
# memused - cached
