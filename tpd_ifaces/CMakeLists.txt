#-------------------------------------------------------------------------------
# setup Flex&Bison
FIND_PACKAGE(FLEX REQUIRED)
SET(AM_LFLAGS "-Pcif")
FLEX_TARGET(Scanner cif_lex.ll ${CMAKE_CURRENT_SOURCE_DIR}/cif_lex.cpp
                      COMPILE_FLAGS ${AM_LFLAGS}
                      DEFINES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cif_lex.hh"
)

FIND_PACKAGE(BISON REQUIRED)
SET(AM_YFLAGS "-p cif -d -y -v")
BISON_TARGET(Parser cif_yacc.yy ${CMAKE_CURRENT_SOURCE_DIR}/cif_yacc.cpp
                      COMPILE_FLAGS ${AM_YFLAGS}
                      DEFINES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cif_yacc.hh"
)

ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

add_library (tpd_ifaces
                      ${BISON_Parser_OUTPUTS}
                      ${FLEX_Scanner_OUTPUTS}
                      calbr_reader.cpp
                      cif_io.cpp
                      gds_io.cpp
                      oasis_io.cpp
                      ps_out.cpp
                      tpdph.cpp
)

target_include_directories(tpd_ifaces PUBLIC
                         ${GLEW_INCLUDE_DIRS}
#                         ${glew_SOURCE_DIR}/include
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         ${CMAKE_SOURCE_DIR}/tpd_common
                         ${CMAKE_SOURCE_DIR}/tpd_DB
                         ${CMAKE_SOURCE_DIR}/tpd_GL
)

target_link_libraries(tpd_ifaces PUBLIC
                         toped_compiler_flags
)


#SET(lib_LTLIBRARIES tpd_ifaces)
#SET(libtpd_ifaces_la_SOURCES cif_io.cpp
#   gds_io.cpp tpdph.cpp oasis_io.cpp calbr_reader.cpp ps_out.cpp)
#include_directories(../tpd_common ../tpd_DB ../tpd_GL ${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
#INCLUDE(${wxWidgets_USE_FILE})
#add_library(${lib_LTLIBRARIES}  STATIC ${libtpd_ifaces_la_SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
