SET(lib_LTLIBRARIES tpd_ifaces)
SET(libtpd_ifaces_la_SOURCES cif_io.cpp 
   gds_io.cpp tpdph.cpp oasis_io.cpp calbr_reader.cpp)

#Flex Bison

FIND_PACKAGE(BISON)
FIND_PACKAGE(FLEX)

SET(AM_YFLAGS "-p cif -d -y -v")
SET(AM_LFLAGS "-Pcif")


FLEX_TARGET(Scanner cif_lex.ll ${CMAKE_CURRENT_SOURCE_DIR}/cif_lex.cpp COMPILE_FLAGS ${AM_LFLAGS})
BISON_TARGET(Parser cif_yacc.yy ${CMAKE_CURRENT_SOURCE_DIR}/cif_yacc.cpp COMPILE_FLAGS ${AM_YFLAGS})
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)


include_directories(../tpd_common ../tpd_DB ${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES}  STATIC ${libtpd_ifaces_la_SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS}) 