SET(lib_LTLIBRARIES tpd_GL)
SET(libtpd_common_la_HEADERS  tpdph.h viewprop.h trend.h tolder.h tenderer.h toshader.h drawprop.h trendat.h basetrend.h)
SET(libtpd_common_la_SOURCES tpdph.cpp drawprop.cpp viewprop.cpp trend.cpp trendat.cpp basetrend.cpp tolder.cpp tenderer.cpp toshader.cpp)

#OpenGL Directories
include_directories(${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES} STATIC ${libtpd_common_la_SOURCES} ${libtpd_common_la_HEADERS})