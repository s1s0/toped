# these are the headers for your project
noinst_HEADERS = browsers.h gds_io.h layoutcanvas.h toped.h tui.h \
	datacenter.h tellibin.h

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
toped_SOURCES = browsers.cpp gds_io.cpp layoutcanvas.cpp main.cpp \
	tellibin.cpp datacenter.cpp toped.cpp tui.cpp

#########################################################################
# EXTRAS
#########################################################################
#LEX_OUTPUT_ROOT = lex.tell
INCLUDES = -I$(top_srcdir)/tpd_parser -I$(top_srcdir)/tpd_DB \
	-I$(top_srcdir)/tpd_common
toped_LDADD = ../tpd_parser/libtpd_parser.la ../tpd_common/libtpd_common.la \
              ../tpd_DB/libtpd_DB.la
#toped_LDFLAGS = -static              
