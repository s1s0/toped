#-------------------------------------------------------------------------------
# setup Flex&Bison
FIND_PACKAGE(FLEX REQUIRED)
SET(AM_LFLAGS "-Ptell")
FLEX_TARGET(Scanner tell_lex.ll  ${CMAKE_CURRENT_SOURCE_DIR}/tell_lex.cpp
            COMPILE_FLAGS ${AM_LFLAGS}
            DEFINES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tell_lex.hh"
)

FIND_PACKAGE(BISON REQUIRED)
SET(AM_YFLAGS "-p tell -d -y -v")
BISON_TARGET(Parser tell_yacc.yy ${CMAKE_CURRENT_SOURCE_DIR}/tell_yacc.cpp
            COMPILE_FLAGS ${AM_YFLAGS}
            DEFINES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tell_yacc.hh"
)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

find_package(glm REQUIRED)

file( GLOB tpd_parser_headers     "*.h")
file( GLOB tpd_parser_sources     "*.cpp")
#-------------------------------------------------------------------------------
add_library(tpd_parser
                      ${BISON_Parser_OUTPUTS}
                      ${FLEX_Scanner_OUTPUTS}
                      ${tpd_parser_headers}
                      ${tpd_parser_sources}
)

target_include_directories(tpd_parser PUBLIC
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         ${CMAKE_SOURCE_DIR}/tpd_common
)
target_link_libraries(tpd_parser PUBLIC
                         toped_compiler_flags
                         glm::glm
)



#include_directories(../tpd_common ../tpd_DB ${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
#
#INCLUDE(${wxWidgets_USE_FILE})
#add_library(${lib_LTLIBRARIES}  STATIC ${libtpd_parser_la_SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})

#SET(lib_LTLIBRARIES tpd_parser)
#
#SET(libtpd_parser_la_HEADERS tellyzer.h tldat.h)
#SET(libtpd_parser_la_SOURCES ted_prompt.cpp  tellyzer.cpp tldat.cpp tpdph.cpp)
