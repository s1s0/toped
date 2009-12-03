noinst_HEADERS = tpdf_props.h datacenter.h tellibin.h tllf_list.h tpdf_add.h \
	tpdf_cells.h tpdf_common.h tpdf_db.h tpdf_edit.h tpdf_get.h tpdf_select.h tpdph.h \
	drc_tenderer.h
lib_LTLIBRARIES = libtpd_bidfunc.la
libtpd_bidfunc_la_LDFLAGS = -no-undefined
libtpd_bidfunc_la_SOURCES = tpdf_db.cpp tpdf_select.cpp datacenter.cpp \
	tellibin.cpp tllf_list.cpp tpdf_add.cpp tpdf_cells.cpp tpdf_common.cpp \
	tpdf_edit.cpp tpdf_get.cpp tpdf_props.cpp tpdph.cpp drc_tenderer.cpp 
INCLUDES = -I$(top_srcdir)/tpd_common     \
           -I$(top_srcdir)/tpd_DB         \
           -I$(top_srcdir)/tpd_ifaces     \
           -I$(top_srcdir)/tpd_parser
libtpd_bidfunc_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la   \
                           $(top_builddir)/tpd_DB/libtpd_DB.la           \
                           $(top_builddir)/tpd_ifaces/libtpd_ifaces.la   \
                           $(top_builddir)/tpd_parser/libtpd_parser.la
