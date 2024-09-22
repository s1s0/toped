file( GLOB tpd_common_headers     "*.h")
file( GLOB tpd_common_sources     "*.cpp")

add_library (tpd_common
                         ${tpd_common_headers}
                         ${tpd_common_sources}
)

target_include_directories(tpd_common INTERFACE
                         ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tpd_common PUBLIC
                         toped_compiler_flags
)

#SET(tpd_common_LIBRARY tpd_common)
#
##OpenGL Directories
#include_directories(${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
#INCLUDE(${wxWidgets_USE_FILE})
#
#add_library(${tpd_common_LIBRARY} #[<TYPE>] default is STATIC or SHARED based on the value of the BUILD_SHARED_LIBS variable.
#   ${tpd_common_SOURCES}
#   ${tpd_common_HEADERS}
#)
