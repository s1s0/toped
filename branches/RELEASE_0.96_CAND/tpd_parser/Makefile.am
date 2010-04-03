#INCLUDES = 
#METASOURCES = AUTO
BUILT_SOURCES = tell_yacc.h
AM_YFLAGS = -p tell -d -y -v
AM_LFLAGS = -Ptell

lib_LTLIBRARIES = libtpd_parser.la
#libtpd_parser_la_LDFLAGS = -no-undefined
libtpd_parser_ladir=$(includedir)/tpd
libtpd_parser_la_HEADERS = ted_prompt.h tellyzer.h tldat.h tpdph.h
libtpd_parser_la_SOURCES = ted_prompt.cpp tell_lex.ll tell_yacc.yy tellyzer.cpp \
	tldat.cpp tpdph.cpp
#########################################################################
# EXTRAS
#########################################################################
LEX_OUTPUT_ROOT = lex.tell

INCLUDES = -I$(top_srcdir)/tpd_common   \
           -I$(top_srcdir)/tpd_DB
libtpd_parser_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la  \
                          $(top_builddir)/tpd_DB/libtpd_DB.la

METASOURCES = AUTO
