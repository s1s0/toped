SET(lib_LTLIBRARIES tpd_parser)

SET(libtpd_parser_la_HEADERS tellyzer.h tldat.h)
SET(libtpd_parser_la_SOURCES ted_prompt.cpp  tellyzer.cpp tldat.cpp tpdph.cpp)
#Flex Bison

FIND_PACKAGE(BISON)
FIND_PACKAGE(FLEX)

SET(AM_YFLAGS "-p tell -d -y -v")
SET(AM_LFLAGS "-Ptell")


FLEX_TARGET(Scanner tell_lex.ll ${CMAKE_CURRENT_SOURCE_DIR}/tell_lex.cpp COMPILE_FLAGS ${AM_LFLAGS})
BISON_TARGET(Parser tell_yacc.yy ${CMAKE_CURRENT_SOURCE_DIR}/tell_yacc.cpp COMPILE_FLAGS ${AM_YFLAGS})
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)


include_directories(../tpd_common ../tpd_DB ${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})

INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES}  STATIC ${libtpd_parser_la_SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS}) 