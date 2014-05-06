#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ts_mruby], [0.0.1], [syucream1031@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ts_mruby.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX

# mruby root
TS_PREFIX_ROOT=/usr/local/
AC_ARG_WITH(ts_prefix_root, AC_HELP_STRING([--with-ts-prefix-root=DIR],
    [pathname to ts_prefix_root [[ts_prefix_root]]]),
    [TS_PREFIX_ROOT="$with_ts_prefix_root"])
AC_SUBST(TS_PREFIX_ROOT)

# Checks for libraries.
# AC_HAVE_LIBRARY([atscppapi])
# AC_HAVE_LIBRARY([mruby])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
