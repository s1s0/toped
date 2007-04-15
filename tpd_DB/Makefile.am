noinst_HEADERS = quadtree.h tedat.h tedcell.h tedesign.h datacenter.h \
                 tedstd.h viewprop.h logicop.h gds_io.h ps_out.h
lib_LTLIBRARIES = libtpd_DB.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_DB_la_SOURCES = quadtree.cpp tedat.cpp tedcell.cpp \
	               tedesign.cpp tedstd.cpp viewprop.cpp \
                       logicop.cpp gds_io.cpp ps_out.cpp \
		       datacenter.cpp
libtpd_DB_la_LIBADD = ../tpd_parser/libtpd_parser.la \
                     ../tpd_common/libtpd_common.la
INCLUDES = -I$(top_srcdir)/tpd_common
