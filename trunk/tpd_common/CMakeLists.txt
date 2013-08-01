#libtpd_common.la
SET(lib_LTLIBRARIES tpd_common)
SET(libtpd_common_la_HEADERS  avl_def.h avl.h polycross.h tpdph.h tuidefs.h)
SET(libtpd_common_la_SOURCES avl.cpp outbox.cpp polycross.cpp tpdph.cpp ttt.cpp MemTrack.cpp tedbac.cpp)

#OpenGL Directories
include_directories(${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES} STATIC ${libtpd_common_la_SOURCES} ${libtpd_common_la_HEADERS})