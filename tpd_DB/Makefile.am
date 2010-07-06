lib_LTLIBRARIES = libtpd_DB.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_DB_la_SOURCES = drawprop.cpp logicop.cpp ps_out.cpp quadtree.cpp \
	tedat.cpp tedcell.cpp tedesign.cpp tedstd.cpp	 tenderer.cpp tpdph.cpp \
	viewprop.cpp
noinst_HEADERS = tpdph.h logicop.h ps_out.h viewprop.h drawprop.h quadtree.h tedat.h tedcell.h \
   tedesign.h tedstd.h tenderer.h
INCLUDES = -I$(top_srcdir)/tpd_common
libtpd_DB_la_LIBADD = $(top_builddir)/tpd_common/libtpd_common.la
