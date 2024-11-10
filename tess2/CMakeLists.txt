file( GLOB tess2_headers     "*.h")
file( GLOB tess2_sources     "*.c")

add_library(tess2
                   ${tess2_sources}
                   ${tess2_headers}
)

target_include_directories(tess2 PUBLIC
                         ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(tess2 PUBLIC
                         toped_compiler_flags
)



##libtpd_DB
#SET(lib_LTLIBRARIES_DB tpd_DB)
#SET(libtpd_DB_la_HEADERS  quadtree.h tedat.h tedcell.h tedesign.h tedstd.h)
#SET(libtpd_DB_la_SOURCES logicop.cpp quadtree.cpp
#	tedat.cpp tedcell.cpp tedesign.cpp tedstd.cpp tpdph.cpp
#	tedat_ext.cpp qtree_tmpl.cpp auxdat.cpp)
#
##OpenGL Directories
#include_directories(${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR} ../tpd_common ../tpd_GL)
#INCLUDE(${wxWidgets_USE_FILE})
#add_library(${lib_LTLIBRARIES_DB} STATIC ${libtpd_DB_la_SOURCES} ${libtpd_DB_la_HEADERS})
