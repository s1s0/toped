noinst_HEADERS = browsers.h datacenter.h drawprop.h logicop.h ps_out.h \
	quadtree.h tedat.h tedcell.h tedesign.h tedstd.h tenderer.h tpdph.h \
	viewprop.h
lib_LTLIBRARIES = libtpd_DB.la
#libtpd_DB_la_LDFLAGS = -no-undefined
libtpd_DB_la_SOURCES = browsers.cpp datacenter.cpp drawprop.cpp logicop.cpp \
	ps_out.cpp quadtree.cpp tedat.cpp tedcell.cpp tedesign.cpp tedstd.cpp	 \
	tenderer.cpp viewprop.cpp
libtpd_DB_la_LIBADD = ../tpd_parser/libtpd_parser.la \
                      ../tpd_ifaces/libtpd_ifaces.la \
                      ../tpd_common/libtpd_common.la
INCLUDES = -I$(top_srcdir)/tpd_common -I$(top_srcdir)/tpd_ifaces
