lib_LTLIBRARIES = libtpd_GL.la
#libtpd_GL_la_LDFLAGS = -no-undefined
libtpd_GL_la_SOURCES = drawprop.cpp tenderer.cpp tpdph.cpp viewprop.cpp glf.cpp 
libtpd_GL_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd
noinst_HEADERS = tpdph.h viewprop.h glf.h 
libtpd_GL_la_HEADERS = drawprop.h tenderer.h
INCLUDES = -I$(top_srcdir)/tpd_common
libtpd_GL_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la
