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

set terminal postscript enhanced color
set output '| ps2pdf - proc_memory_usage.pdf'

set title "Average Process Memory Usage"
set xlabel "Number of PEs"
set ylabel "Memory Usage (kB)"

#plot \
#'./proc-mem-usage.csv' using 1:2  with lines title 'P_VmPeak', \
#'./proc-mem-usage.csv' using 1:3  with lines title 'P_VmSize', \
#'./proc-mem-usage.csv' using 1:4  with lines title 'P_VmLck', \
#'./proc-mem-usage.csv' using 1:5  with lines title 'P_VmHWM', \
#'./proc-mem-usage.csv' using 1:6  with lines title 'P_VmRSS', \
#'./proc-mem-usage.csv' using 1:7  with lines title 'P_VmData', \
#'./proc-mem-usage.csv' using 1:8  with lines title 'P_VmStk', \
#'./proc-mem-usage.csv' using 1:9  with lines title 'P_VmExe', \
#'./proc-mem-usage.csv' using 1:10 with lines title 'P_VmLib', \
#'./proc-mem-usage.csv' using 1:11 with lines title 'P_VmPTE', \
#'./proc-mem-usage.csv' using 1:12 with lines title 'VmPeak', \
#'./proc-mem-usage.csv' using 1:13 with lines title 'VmSize', \
#'./proc-mem-usage.csv' using 1:14 with lines title 'VmLck', \
#'./proc-mem-usage.csv' using 1:15 with lines title 'VmHWM', \
#'./proc-mem-usage.csv' using 1:16 with lines title 'VmRSS', \
#'./proc-mem-usage.csv' using 1:17 with lines title 'VmData', \
#'./proc-mem-usage.csv' using 1:18 with lines title 'VmStk', \
#'./proc-mem-usage.csv' using 1:19 with lines title 'VmExe', \
#'./proc-mem-usage.csv' using 1:20 with lines title 'VmLib', \
#'./proc-mem-usage.csv' using 1:21 with lines title 'VmPTE'

plot \
'./proc-mem-usage.csv' using 1:6  with lines title 'Pre\_MPI\_Init\_VmRSS', \
'./proc-mem-usage.csv' using 1:16 with lines title 'VmRSS'
