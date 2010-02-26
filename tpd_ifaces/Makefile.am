noinst_HEADERS = tpdph.h calbr_reader.h
BUILT_SOURCES = cif_yacc.h
AM_YFLAGS = -p cif -d -y -v
AM_LFLAGS = -Pcif
lib_LTLIBRARIES = libtpd_ifaces.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_ifaces_ladir=$(includedir)/tpd
libtpd_ifaces_la_HEADERS = cif_io.h gds_io.h oasis_io.h 
libtpd_ifaces_la_SOURCES = cif_io.cpp cif_lex.ll cif_yacc.yy \
   gds_io.cpp tpdph.cpp oasis_io.cpp calbr_reader.cpp
LEX_OUTPUT_ROOT = lex.cif
INCLUDES = -I$(top_srcdir)/tpd_common       \
           -I$(top_srcdir)/tpd_DB
libtpd_ifaces_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la    \
                          $(top_builddir)/tpd_DB/libtpd_DB.la

METASOURCES = AUTO
