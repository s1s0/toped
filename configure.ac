#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(toped, 0.9.5)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cpp])

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
CPPFLAGS="$CPPFLAGS -Wall -g -fexceptions"
CFLAGS="$CFLAGS -Wall -g -fexceptions"
LDFLAGS="$LDFLAGS -Wl,--as-needed"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([pow rint sqrt strtoul])

# Checks for size of the types
#AC_CHECK_SIZEOF(int)
#AC_CHECK_SIZEOF(time_t)
#AC_CHECK_SIZEOF(char)
#AC_CHECK_SIZEOF(short)
#AC_CHECK_SIZEOF(long)

# Checks for libraries.

#Check openGL
AX_CHECK_GL
   if test "$no_gl" == "yes"; then
      AC_MSG_ERROR([(GL) openGL not found])
   else
      LDFLAGS="$LDFLAGS $GL_LIBS"
       CFLAGS="$CFLAGS $GL_CFLAGS"
   fi
AX_CHECK_GLU
   if test "$no_glu" == "yes"; then
      AC_MSG_ERROR([(GLU) openGL utility not found])
   else
      LDFLAGS="$LDFLAGS $GLU_LIBS"
       CFLAGS="$CFLAGS $GLU_CFLAGS"
   fi
AC_SEARCH_LIBS(glewInit, [GLEW glew32], , AC_MSG_ERROR([Glew library not found. Make sure it's installed]))
AC_CHECK_LIB(z, gzsetparams,
             [AC_CHECK_HEADER(zlib.h,,[AC_MSG_ERROR(zlib headers not found)])],
             [AC_MSG_ERROR(zlib library not found)])

#Check wxWidgets
 AM_OPTIONS_WXCONFIG
 AM_PATH_WXCONFIG(2.8.9, wxWin=1,,std gl)
 if test "$wxWin" != 1 || test "$wx_ver_ok" != "yes"; then
    AC_MSG_ERROR([
       wxWindows ver.2.8.9 or later must be installed on your system
       Please install or update your wxWindows
    ])
 else
    CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
    CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
    LDFLAGS="$LDFLAGS $WX_LIBS"
 fi

#Check for wx gl library and for wx threads
#

dnl----------------------------------------------------------------------------
dnl
dnl Toped Compilation options
dnl
dnl----------------------------------------------------------------------------
AC_ARG_WITH(mem-track,
            [  --with-mem-track        Compile with build-in heap tracker],
            [ tpd_mem_track="yes"])

if test "$tpd_mem_track" == "yes"; then
    CPPFLAGS="$CPPFLAGS -DDB_MEMORY_TRACE"
fi

AC_CONFIG_FILES([
   Makefile
   ui/Makefile
   tpd_common/Makefile
   tpd_DB/Makefile
   tpd_ifaces/Makefile
   tpd_parser/Makefile
   tpd_bidfunc/Makefile
   src/Makefile
   fonts/Makefile
   icons/Makefile
   tll/Makefile
   utils/cadence/Makefile
])
AC_OUTPUT
