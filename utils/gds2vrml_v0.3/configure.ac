#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT(gds2vrml, 0.3)
AC_CONFIG_HEADERS([config.h])
#AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_INIT_AUTOMAKE([-Wall -Werror])
#AC_CONFIG_SRCDIR([src/main.cpp])

# Checks for programs.
AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_SUBST(TOP_GDS2VRML_SRCDIR, ['$(top_srcdir)'])
AC_SUBST(TOP_GDS2VRML_BUILDDIR, ['$(top_builddir)'])

AC_CONFIG_FILES([
   Makefile 
   libgdsto3d/Makefile
   src/Makefile
])

AC_OUTPUT
