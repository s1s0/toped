noinst_HEADERS = gds_io.h cif_io.h tpdph.h
lib_LTLIBRARIES = libtpd_ifaces.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_ifaces_la_SOURCES = gds_io.cpp cif_io.cpp
libtpd_ifaces_la_LIBADD = ../tpd_parser/libtpd_parser.la \
                          ../tpd_common/libtpd_common.la
INCLUDES = -I$(top_srcdir)/tpd_common
