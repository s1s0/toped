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
AM_PROG_LEX
AC_PROG_YACC
if test "$YACC" == "yacc"; then
   AC_MSG_ERROR([Bison not found])
fi
#CPPFLAGS="$CPPFLAGS -Wall -g -fexceptions"
#CFLAGS="$CFLAGS -Wall -g -fexceptions"
#LDFLAGS="$LDFLAGS -Wl,--as-needed"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T


AC_CONFIG_FILES([
   Makefile 
   libgdsto3d/Makefile
   src/Makefile
])

AC_OUTPUT
