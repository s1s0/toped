lib_LTLIBRARIES = libtpd_ifaces.la

noinst_HEADERS =                                                              \
                 tpdph.h                                                      \
                 calbr_reader.h                                               \
                 cif_io.h                                                     \
                 gds_io.h                                                     \
                 oasis_io.h                                                   \
                 ps_out.h 

libtpd_ifaces_la_SOURCES =                                                    \
                 tpdph.cpp                                                    \
                 cif_io.cpp                                                   \
                 cif_lex.ll                                                   \
                 cif_yacc.yy                                                  \
                 gds_io.cpp                                                   \
                 oasis_io.cpp                                                 \
                 calbr_reader.cpp                                             \
                 ps_out.cpp 

METASOURCES     = AUTO
BUILT_SOURCES   = cif_yacc.h
AM_YFLAGS       = -p cif -d -y -v
AM_LFLAGS       = -Pcif
LEX_OUTPUT_ROOT = lex.cif

###############################################################################
INCLUDES =                                                                    \
           -I$(top_srcdir)/tpd_common                                         \
           -I$(top_srcdir)/tpd_GL                                             \
           -I$(top_srcdir)/tpd_DB

libtpd_ifaces_la_LIBADD =                                                     \
                          $(top_builddir)/tpd_common/libtpd_common.la         \
                          $(top_builddir)/tpd_GL/libtpd_GL.la                 \
                          $(top_builddir)/tpd_DB/libtpd_DB.la

#libtpd_ifaces_ladir=$(includedir)/tpd

libtpd_ifaces_la_LDFLAGS = -no-undefined
