bin_PROGRAMS = toped

noinst_HEADERS =                                                              \
                 browsers.h                                                   \
                 layoutcanvas.h                                               \
                 resourcecenter.h                                             \
                 tpdph.h                                                      \
                 tui.h                                                        \
                 techeditor.h                                                 \
                 toped.h

toped_SOURCES =                                                               \
                 tpdph.cpp                                                    \
                 browsers.cpp                                                 \
                 layoutcanvas.cpp                                             \
                 resourcecenter.cpp                                           \
                 tui.cpp                                                      \
                 techeditor.cpp                                               \
                 toped.cpp                                                    \
                 main.cpp

###############################################################################
INCLUDES =                                                                    \
                 -I$(top_srcdir)/tpd_common                                   \
                 -I$(top_srcdir)/tpd_GL                                       \
                 -I$(top_srcdir)/tpd_DB                                       \
                 -I$(top_srcdir)/tpd_ifaces                                   \
                 -I$(top_srcdir)/tpd_parser                                   \
                 -I$(top_srcdir)/tpd_bidfunc
                 
toped_LDADD =                                                                 \
                 $(top_builddir)/tpd_common/libtpd_common.la                  \
                 $(top_builddir)/tpd_GL/libtpd_GL.la                          \
                 $(top_builddir)/tpd_DB/libtpd_DB.la                          \
                 $(top_builddir)/tpd_ifaces/libtpd_ifaces.la                  \
                 $(top_builddir)/tpd_parser/libtpd_parser.la                  \
                 $(top_builddir)/tpd_bidfunc/libtpd_bidfunc.la

toped_LDFLAGS = -no-undefined
#toped_LDFLAGS = -static

