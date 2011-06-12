# these are the headers for the project
noinst_HEADERS = browsers.h layoutcanvas.h resourcecenter.h toped.h tpdph.h \
	tui.h techeditor.h

#BUILT_SOURCES = tell_yacc.h
#AM_YFLAGS = -p tell -d -y -v
#AM_LFLAGS = -Ptell

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = toped

# the application source, library search path, and link libraries
toped_SOURCES = browsers.cpp layoutcanvas.cpp main.cpp resourcecenter.cpp \
	toped.cpp tpdph.cpp tui.cpp techeditor.cpp

#########################################################################
# EXTRAS
#########################################################################
#LEX_OUTPUT_ROOT = lex.tell
INCLUDES = -I$(top_srcdir)/tpd_common    \
           -I$(top_srcdir)/tpd_DB        \
           -I$(top_srcdir)/tpd_ifaces	  \
           -I$(top_srcdir)/tpd_parser    \
           -I$(top_srcdir)/tpd_bidfunc
toped_LDADD = $(top_builddir)/tpd_common/libtpd_common.la     \
              $(top_builddir)/tpd_DB/libtpd_DB.la             \
              $(top_builddir)/tpd_ifaces/libtpd_ifaces.la     \
              $(top_builddir)/tpd_parser/libtpd_parser.la     \
              $(top_builddir)/tpd_bidfunc/libtpd_bidfunc.la
#toped_LDFLAGS = -static

