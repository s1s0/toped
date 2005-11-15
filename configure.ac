#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(toped, 0.7-30)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cpp])

# Checks for programs.
AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX

CPPFLAGS="$CPPFLAGS -Wall -g -fexceptions"
CFLAGS="$CFLAGS -Wall -g -fexceptions"

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
AX_CHECK_GLUT
   if test "$no_glut" == "yes"; then
      AC_MSG_ERROR([(GLUT) openGL utility toolkit not found])
   else
      LDFLAGS="$LDFLAGS $GLUT_LIBS"
       CFLAGS="$CFLAGS $GLUT_CFLAGS"
   fi     

#Check wxWidgets
 AM_OPTIONS_WXCONFIG
 AM_PATH_WXCONFIG(2.6.2, wxWin=1,,std gl)
 if test "$wxWin" != 1 || test "$wx_ver_ok" != "yes"; then
    AC_MSG_ERROR([
	    wxWindows ver.2.6.2 or later must be installed on your system
	    Please install or update your wxWindows
    ])
 else
    CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
    CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
    LDFLAGS="$LDFLAGS $WX_LIBS"
 fi
#Check for wx gl library and for wx threads
#...


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
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(time_t)
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(long)

AC_CONFIG_FILES([Makefile tpd_common/Makefile tpd_parser/Makefile tpd_DB/Makefile src/Makefile ])
AC_OUTPUT


#configure.ac: warning: missing AC_CHECK_FUNCS([pow]) wanted by: src/gds_io.cpp:95
#configure.ac: warning: missing AC_CHECK_FUNCS([rint]) wanted by: src/ttt.cpp:35
#configure.ac: warning: missing AC_CHECK_FUNCS([sqrt]) wanted by: src/gds_io.cpp:636
#configure.ac: warning: missing AC_CHECK_FUNCS([strtoul]) wanted by: src/tell.l:168
#configure.ac: warning: missing AC_CHECK_HEADERS([stddef.h]) wanted by: src/avl.h:29
#configure.ac: warning: missing AC_CHECK_HEADERS([stdlib.h]) wanted by: src/avl.c:28
#configure.ac: warning: missing AC_CHECK_HEADERS([string.h]) wanted by: src/avl.c:29
#configure.ac: warning: missing AC_CHECK_HEADERS([unistd.h]) wanted by: src/tell_lex.cpp:31
#configure.ac: warning: missing AC_C_CONST wanted by: src/outbox.h:62
#configure.ac: warning: missing AC_C_INLINE wanted by: src/tedop.cpp:185
#configure.ac: warning: missing AC_FUNC_ERROR_AT_LINE wanted by: src/tldat.cpp:37
#configure.ac: warning: missing AC_FUNC_MALLOC wanted by: src/avl.c:852
#configure.ac: warning: missing AC_FUNC_REALLOC wanted by: src/tell_lex.cpp:1923
#configure.ac: warning: missing AC_HEADER_STDBOOL wanted by: src/tellibin.h:178
#configure.ac: warning: missing AC_HEADER_STDC wanted by: src/avl.h:29
#configure.ac: warning: missing AC_PREREQ wanted by: autoscan
#configure.ac: warning: missing AC_PROG_CC wanted by: src/outbox.h
#configure.ac: warning: missing AC_PROG_CXX wanted by: src/tedesign.cpp
#configure.ac: warning: missing AC_PROG_LEX wanted by: src/Makefile:407
#configure.ac: warning: missing AC_PROG_YACC wanted by: src/Makefile:408
#configure.ac: warning: missing AC_TYPE_SIZE_T wanted by: src/avl.c:849
##Checks for Xlibraries. 
#AC_CHECK_LIB([X11], [XOpenDisplay], [LIBS="$LIBS -lX11"], [echo "Can't find X11 library" ; exit 1], [-lX11])
## FIXME: Replace `main' with a function in `-lXext':
#AC_CHECK_LIB([Xext], [main])
## FIXME: Replace `main' with a function in `-lXmu':
#AC_CHECK_LIB([Xmu], [main])
## FIXME: Replace `main' with a function in `-lglut':
#AC_CHECK_LIB([glut], [main])
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
## FIXME: Replace `main' with a function in `-lpthread':
#AC_CHECK_LIB([pthread], [main])

