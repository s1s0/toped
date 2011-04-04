lib_LTLIBRARIES = libtpd_DB.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_DB_la_SOURCES = drawprop.cpp logicop.cpp ps_out.cpp quadtree.cpp \
	tedat.cpp tedcell.cpp tedesign.cpp tedstd.cpp tenderer.cpp tpdph.cpp \
	viewprop.cpp tedat_ext.cpp qtree_tmpl.cpp grccell.cpp
libtpd_DB_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd
noinst_HEADERS = logicop.h ps_out.h viewprop.h tedat_ext.h grccell.h
libtpd_DB_la_HEADERS = drawprop.h quadtree.h tedat.h tedcell.h \
   tedesign.h tedstd.h tenderer.h qtree_tmpl.h
INCLUDES = -I$(top_srcdir)/tpd_common
libtpd_DB_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la
