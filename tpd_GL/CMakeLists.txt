find_package(glm REQUIRED)

file( GLOB tpd_GL_headers     "*.h")
file( GLOB tpd_GL_sources     "*.cpp")

add_library(tpd_GL
                   ${tpd_GL_headers}
                   ${tpd_GL_sources}
)

target_include_directories(tpd_GL PUBLIC
#                         ${glew_SOURCE_DIR}/include
                         ${GLEW_INCLUDE_DIRS}
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         ${CMAKE_SOURCE_DIR}/tpd_common

)
target_link_libraries(tpd_GL PUBLIC
                         toped_compiler_flags
                         glm::glm
)

#SET(lib_LTLIBRARIES_GL tpd_GL)
#SET(libtpd_GL_common_la_HEADERS  tpdph.h viewprop.h trend.h tolder.h tenderer.h toshader.h drawprop.h trendat.h basetrend.h)
#SET(libtpd_GL_common_la_SOURCES tpdph.cpp drawprop.cpp viewprop.cpp trend.cpp trendat.cpp basetrend.cpp tolder.cpp tenderer.cpp toshader.cpp)
#
##OpenGL Directories
#include_directories(${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ../tpd_common)
#INCLUDE(${wxWidgets_USE_FILE})
#add_library(${lib_LTLIBRARIES_GL} STATIC ${libtpd_GL_common_la_SOURCES} ${libtpd_GL_common_la_HEADERS})
