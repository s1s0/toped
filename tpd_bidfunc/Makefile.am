lib_LTLIBRARIES = libtpd_bidfunc.la

noinst_HEADERS =                                                              \
                 tpdph.h                                                      \
                 tpdf_props.h                                                 \
                 tellibin.h                                                   \
                 tllf_list.h                                                  \
                 tpdf_add.h                                                   \
                 tpdf_cells.h                                                 \
                 tpdf_db.h                                                    \
                 tpdf_edit.h                                                  \
                 tpdf_get.h                                                   \
                 tpdf_select.h                                                \
                 drc_tenderer.h

libtpd_bidfunc_la_HEADERS =                                                   \
                 tpdf_common.h                                                \
                 datacenter.h 

libtpd_bidfunc_la_SOURCES =                                                   \
                 tpdph.cpp                                                    \
                 tpdf_db.cpp                                                  \
                 tpdf_select.cpp                                              \
                 datacenter.cpp                                               \
                 tellibin.cpp                                                 \
                 tllf_list.cpp                                                \
                 tpdf_add.cpp                                                 \
                 tpdf_cells.cpp                                               \
                 tpdf_common.cpp                                              \
                 tpdf_edit.cpp                                                \
                 tpdf_get.cpp                                                 \
                 tpdf_props.cpp                                               \
                 drc_tenderer.cpp 

###############################################################################
AM_CPPFLAGS =                                                                 \
                 -I$(top_srcdir)/tpd_common                                   \
                 -I$(top_srcdir)/tpd_GL                                       \
                 -I$(top_srcdir)/tpd_DB                                       \
                 -I$(top_srcdir)/tpd_ifaces                                   \
                 -I$(top_srcdir)/tpd_parser

libtpd_bidfunc_la_LIBADD =                                                    \
                 $(top_builddir)/tpd_common/libtpd_common.la                  \
                 $(top_builddir)/tpd_GL/libtpd_GL.la                          \
                 $(top_builddir)/tpd_DB/libtpd_DB.la                          \
                 $(top_builddir)/tpd_ifaces/libtpd_ifaces.la                  \
                 $(top_builddir)/tpd_parser/libtpd_parser.la

libtpd_bidfunc_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd

libtpd_bidfunc_la_LDFLAGS = -no-undefined

