lib_LTLIBRARIES = libtpd_GL.la

noinst_HEADERS =                                                              \
                 tpdph.h                                                      \
                 viewprop.h                                                   \
                 trend.h                                                      \
                 tolder.h                                                     \
                 tenderer.h                                                   \
                 glf.h 

libtpd_GL_la_HEADERS =                                                        \
                 drawprop.h                                                   \
                 basetrend.h

libtpd_GL_la_SOURCES =                                                        \
                 tpdph.cpp                                                    \
                 drawprop.cpp                                                 \
                 viewprop.cpp                                                 \
                 trend.cpp                                                    \
                 basetrend.cpp                                                \
                 tolder.cpp                                                   \
                 tenderer.cpp                                                 \
                 glf.cpp 

###############################################################################
INCLUDES =                                                                    \
                 -I$(top_srcdir)/tpd_common

libtpd_GL_la_LIBADD =                                                         \
                 $(top_builddir)/tpd_common/libtpd_common.la

libtpd_GL_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd

libtpd_GL_la_LDFLAGS = -no-undefined
