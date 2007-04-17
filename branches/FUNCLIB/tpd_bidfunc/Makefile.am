noinst_HEADERS = tellibin.h resourcecenter.h
lib_LTLIBRARIES = libtpd_bidfunc.la
libtpd_bidfunc_la_LDFLAGS = -no-undefined
libtpd_bidfunc_la_SOURCES = tellibin.cpp resourcecenter.cpp
INCLUDES = -I$(top_srcdir)/tpd_DB -I$(top_srcdir)/tpd_common\
	-I$(top_srcdir)/tpd_parser
libtpd_bidfunc_la_LIBADD = ../tpd_parser/libtpd_parser.la ../tpd_common/libtpd_common.la \
              ../tpd_DB/libtpd_DB.la
