noinst_HEADERS = tellibin.h tpdf_common.h tpdf_db.h \
		tpdf_props.h tpdf_cells.h tpdf_edit.h tpdf_add.h tpdf_select.h
lib_LTLIBRARIES = libtpd_bidfunc.la
libtpd_bidfunc_la_LDFLAGS = -no-undefined
libtpd_bidfunc_la_SOURCES = tellibin.cpp tpdf_common.cpp \
		tpdf_db.cpp tpdf_props.cpp tpdf_cells.cpp tpdf_edit.cpp tpdf_add.cpp \
	tpdf_select.cpp
INCLUDES = -I$(top_srcdir)/tpd_DB -I$(top_srcdir)/tpd_common\
	-I$(top_srcdir)/tpd_parser
libtpd_bidfunc_la_LIBADD = ../tpd_parser/libtpd_parser.la ../tpd_common/libtpd_common.la \
              ../tpd_DB/libtpd_DB.la
