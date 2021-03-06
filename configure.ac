#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)
AC_INIT([trollfs], m4_esyscmd([ tr -d '\n' < version ]))

CXXFLAGS=""
LDFLAGS=""

AC_CONFIG_SRCDIR([src/trollfs_main.cpp])

AC_CONFIG_HEADERS([include/config.h:include/config.in])

LT_INIT
AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE([disable])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile
  src/Makefile])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Libs list autmatically generated from dependecy script
AC_CHECK_LIB([fuse],[fuse_main],[],[AC_MSG_FAILURE([could not find lib FUSE])])
AC_CHECK_LIB([magic],[magic_file],[],[AC_MSG_FAILURE([could not find lib magic])])

AC_CANONICAL_HOST

AS_CASE(["$host_cpu"],
	[ia64 | x86_64 | hppa64],
               AC_DEFINE(_FILE_OFFSET_BITS, 64, [FUSE WORD LEN]),
               AC_DEFINE(_FILE_OFFSET_BITS, 32, [FUSE WORD LEN]) )

AS_CASE(["$host_os"],
	[linux*], 
        [     OPTIMIZATION="-O2 "
              AS_CASE(["$CC"],
	            [gcc], 
                     [ CXXFLAGS=" -std=c++11 -g -Wall -Wextra \
                             -D_FORTIFY_SOURCE=2 ${OPTIMIZATION} -fstack-protector \
                             --param=ssp-buffer-size=4 -Wformat -Werror=format-security "
                        LDFLAGS="$LDFLAGS ${LIB_SSL_CUSTOM}-Wl,-z,relro"
		    ],
		    [
                         CXXFLAGS=" -std=c++11 -O2 -g -Wall "
                         LDFLAGS="$LDFLAGS "
		    ])
        ],
	[
            AC_MSG_ERROR([Your platform is not currently supported.])
        ])

AC_OUTPUT
