#libtpd_DB
SET(lib_LTLIBRARIES_DB tpd_DB)
SET(libtpd_DB_la_HEADERS  drawprop.h quadtree.h tedat.h tedcell.h tedesign.h tedstd.h tenderer.h)
SET(libtpd_DB_la_SOURCES drawprop.cpp logicop.cpp ps_out.cpp quadtree.cpp 
	tedat.cpp tedcell.cpp tedesign.cpp tedstd.cpp tenderer.cpp tpdph.cpp 
	viewprop.cpp tedat_ext.cpp qtree_tmpl.cpp auxdat.cpp)

#OpenGL Directories
include_directories(${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR} ../tpd_common)
INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES_DB} STATIC ${libtpd_DB_la_SOURCES} ${libtpd_DB_la_HEADERS})