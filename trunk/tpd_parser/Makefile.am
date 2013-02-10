lib_LTLIBRARIES = libtpd_parser.la

noinst_HEADERS =                                                              \
                 tpdph.h                                                      \
                 ted_prompt.h

libtpd_parser_la_HEADERS =                                                    \
                 tellyzer.h                                                   \
                 tldat.h

libtpd_parser_la_SOURCES =                                                    \
                 tpdph.cpp                                                    \
                 ted_prompt.cpp                                               \
                 tell_lex.ll                                                  \
                 tell_yacc.yy                                                 \
                 tellyzer.cpp                                                 \
                 tldat.cpp

METASOURCES     = AUTO
BUILT_SOURCES   = tell_yacc.h
AM_YFLAGS       = -p tell -d -y -v
AM_LFLAGS       = -Ptell
LEX_OUTPUT_ROOT = lex.tell
CLEANFILES      = tell_yacc.output tell_yacc.h tell_yacc.cc tell_lex.cc

###############################################################################
INCLUDES =                                                                    \
                 -I$(top_srcdir)/tpd_common

libtpd_parser_la_LIBADD =                                                     \
                 $(top_builddir)/tpd_common/libtpd_common.la

libtpd_parser_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd

libtpd_parser_la_LDFLAGS = -no-undefined
