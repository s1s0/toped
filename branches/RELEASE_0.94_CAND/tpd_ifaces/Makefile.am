noinst_HEADERS = gds_io.h cif_io.h tpdph.h
BUILT_SOURCES = cif_yacc.h
AM_YFLAGS = -p cif -d -y -v
AM_LFLAGS = -Pcif
lib_LTLIBRARIES = libtpd_ifaces.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_ifaces_la_SOURCES = cif_io.cpp cif_lex.ll cif_yacc.yy gds_io.cpp
LEX_OUTPUT_ROOT = lex.cif
libtpd_ifaces_la_LIBADD = ../tpd_parser/libtpd_parser.la \
                          ../tpd_common/libtpd_common.la
INCLUDES = -I$(top_srcdir)/tpd_common
METASOURCES = AUTO