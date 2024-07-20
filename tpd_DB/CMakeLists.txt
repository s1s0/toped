add_library(tpd_DB
                   auxdat.cpp
                   logicop.cpp
                   qtree_tmpl.cpp
                   quadtree.cpp
                   tedat.cpp
                   tedat_ext.cpp
                   tedcell.cpp
                   tedesign.cpp
                   tedstd.cpp
                   tpdph.cpp
)

target_include_directories(tpd_DB PUBLIC
                         ${GLEW_INCLUDE_DIRS}
#                         ${glew_SOURCE_DIR}/include
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         ${CMAKE_SOURCE_DIR}/tpd_common
                         ${CMAKE_SOURCE_DIR}/tpd_GL
)
target_link_libraries(tpd_DB PUBLIC
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
