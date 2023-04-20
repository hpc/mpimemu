#
# Copyright (c) 2011-2023 Triad National Security, LLC
#                         All rights reserved.
#
# This file is part of the mpimemu project. See the LICENSE file at the
# top-level directory of this distribution.
#

AC_PREREQ([2.60])

AC_INIT([mpimemu], [1.5-alpha], [samuel@lanl.gov])

# Check If AM_SILENT_RULES Rule Are Supported
# If So, Build Using AM_SILENT_RULES
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src])

# note: -Wall -Werror are automake options.  they are NOT compiler flags
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# checks for programs.
# note that we are checking for mpicc first, the next check will verify CC
AC_PROG_CC([mpicc cc])

# check for adequate mpi support
AC_MSG_CHECKING([if $CC can compile MPI applications])
AC_LINK_IFELSE([
    AC_LANG_PROGRAM(
        [[#include <mpi.h>]],
        [[MPI_Finalize();]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([$CC cannot compile MPI applications. cannot continue.])]
)

AM_PROG_CC_C_O

# checks for libraries.
# none at this point

# checks for header files.
AC_CHECK_HEADERS([\
inttypes.h limits.h stdint.h stdlib.h string.h unistd.h stdbool.h time.h \
getopt.h ctype.h netdb.h sys/socket.h netinet/in.h arpa/inet.h
])

# checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strerror strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT

# let the user know the configuration

cat << EOF

mpimemu configuration
+--------------------
|
| CC      : $CC
| CFLAGS  : $CFLAGS
| LDFLAGS : $LDFLAGS
| LIBS    : $LIBS
| CPPFLAGS: $CPPFLAGS
| CPP     : $CPP

EOF
