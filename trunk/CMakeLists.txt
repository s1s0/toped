## GPL : yannick.heinrich@gmail.com ##
##
##  WARNING : To compile change BISON generated header extension from .hpp to .h
##
cmake_minimum_required (VERSION 2.6)

SET(EXEC_NAME Toped)
project(${EXEC_NAME})
SET(CMAKE_BUILD_TYPE Debug)
##GLEW
## Set to your glew library installation directory
#SET (glew_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glew/include)
#SET (glew_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glew/lib)
#MESSAGE(${glew_LIB_DIR})

#WXWIDGETS
##Windows
#SET (wxWidgets_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/wxwidgets)
#SET (wxWidgets_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/wxwidgets/lib)
#Check dependance
SET(TPD_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpd_common)
SET(TPD_DB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpd_db)
SET(TPD_IFACES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpd_ifaces)
SET(TPD_PARSER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpd_parser)
SET(TPD_BIDFUNC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpd_bidfunc)
SET(TPD_GL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpd_GL)


FIND_PACKAGE(Glew REQUIRED)
include_directories(${GLEW_INCLUDE_DIR}) 
IF(NOT DEFINED GLEW_FOUND)
  MESSAGE(FATAL_ERROR "Glew is not found on your system")
ENDIF(NOT DEFINED GLEW_FOUND)

FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR}) 
IF(NOT DEFINED OPENGL_FOUND)
	MESSAGE(FATAL_ERROR "Opengl is not found on your system")
ENDIF(NOT DEFINED OPENGL_FOUND)


FIND_PACKAGE(ZLIB)
IF(NOT ZLIB_FOUND)
	MESSAGE(FATAL_ERROR "ZLIB is not found on your system")
ENDIF(NOT ZLIB_FOUND)


SET(wxWidgets_USE_DEBUG ON)
FIND_PACKAGE(wxWidgets)
IF(NOT wxWidgets_FOUND)
	MESSAGE(FATAL_ERROR "WxWidgets is not found on your system")
ENDIF(NOT wxWidgets_FOUND)

#Compile library
add_subdirectory(tpd_common)
add_subdirectory(tpd_DB)
add_subdirectory(tpd_ifaces)
add_subdirectory(tpd_parser)
add_subdirectory(tpd_bidfunc)
add_subdirectory(tpd_gl)

#Extrafiles
#icons
file(GLOB icons "icons/*.png")

#fonts
file(GLOB fonts "fonts/*.glf")

#Technology files
##tll
file(GLOB tll "tll/*.tll")
##user_functions
file(GLOB user_functions "user_functions/general/*.tll")

#Executable
SET(toped_SOURCES src/browsers.cpp src/layoutcanvas.cpp src/main.cpp 
src/resourcecenter.cpp src/toped.cpp src/tpdph.cpp src/tui.cpp src/techeditor.cpp)
include_directories(tpd_common tpd_DB tpd_DB tpd_ifaces tpd_parser tpd_bidfunc tpd_GL ${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})
INCLUDE(${wxWidgets_USE_FILE})

if(APPLE)
SET(MACOSX_BUNDLE_ICON_FILE "toped.icns")
SET(WITH_MAC_BUNDLE_NAME "toped")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/wxcocoa_config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/wxcocoa_config.h)
add_executable(${EXEC_NAME} MACOSX_BUNDLE ${toped_SOURCES} ${fonts} ${icons} ${tll} ${user_functions} "toped.icns")

SET_SOURCE_FILES_PROPERTIES(	
  ${icons}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION MacOS/icons
)
SET_SOURCE_FILES_PROPERTIES(	
  ${fonts}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION MacOS/fonts
)
SET_SOURCE_FILES_PROPERTIES(	
  "toped.icns"
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
)
SET_SOURCE_FILES_PROPERTIES(	
  ${tll}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION MacOS/tll
)
SET_SOURCE_FILES_PROPERTIES(	
  ${user_functions}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION MacOS/tll/user_functions/general
)
else(APPLE)
add_executable(${EXEC_NAME} ${toped_SOURCES} ${fonts} ${icons} ${tll} ${user_functions})
endif(APPLE)
link_directories(${TPD_COMMON_DIR} ${TPD_DB_DIR} ${TPD_IFACES_DIR} ${TPD_PARSER_DIR} ${TPD_BIDFUNC_DIR} ${TPD_GL_DIR} ${glew_LIB_DIR})
IF(APPLE)

target_link_libraries(${EXEC_NAME} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${wxWidgets_LIBRARIES} tpd_bidfunc tpd_parser tpd_ifaces tpd_DB tpd_common wx_osx_cocoau_gl-2.9 z) 
else(APPLE)
target_link_libraries(${EXEC_NAME} ${wxWidgets_LIBRARIES} tpd_bidfunc tpd_parser tpd_ifaces tpd_DB tpd_common GLEW z) 
endif(APPLE)
