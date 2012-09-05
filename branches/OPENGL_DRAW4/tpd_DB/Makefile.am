lib_LTLIBRARIES = libtpd_DB.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_DB_la_SOURCES = logicop.cpp quadtree.cpp \
	tedat.cpp tedcell.cpp tedesign.cpp tedstd.cpp tpdph.cpp \
	tedat_ext.cpp qtree_tmpl.cpp auxdat.cpp
libtpd_DB_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd
noinst_HEADERS = logicop.h tedat_ext.h auxdat.h
libtpd_DB_la_HEADERS = quadtree.h tedat.h tedcell.h \
   tedesign.h tedstd.h qtree_tmpl.h
INCLUDES = -I$(top_srcdir)/tpd_common \
           -I$(top_srcdir)/tpd_GL
libtpd_DB_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la \
                      $(top_builddir)/tpd_GL/libtpd_GL.la
