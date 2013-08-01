SET(lib_LTLIBRARIES_GL tpd_GL)
SET(libtpd_GL_common_la_HEADERS  tpdph.h viewprop.h trend.h tolder.h tenderer.h toshader.h drawprop.h trendat.h basetrend.h)
SET(libtpd_GL_common_la_SOURCES tpdph.cpp drawprop.cpp viewprop.cpp trend.cpp trendat.cpp basetrend.cpp tolder.cpp tenderer.cpp toshader.cpp)

#OpenGL Directories
include_directories(${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ../tpd_common)
INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES_GL} STATIC ${libtpd_GL_common_la_SOURCES} ${libtpd_GL_common_la_HEADERS})