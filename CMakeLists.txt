#===========================================================================
#                                                                          =
#   This program is free software; you can redistribute it and/or modify   =
#   it under the terms of the GNU General Public License as published by   =
#   the Free Software Foundation; either version 2 of the License, or      =
#   (at your option) any later version.                                    =
# ------------------------------------------------------------------------ =
#                  TTTTT    OOO    PPPP    EEEE    DDDD                    =
#                  T T T   O   O   P   P   E       D   D                   =
#                    T    O     O  PPPP    EEE     D    D                  =
#                    T     O   O   P       E       D   D                   =
#                    T      OOO    P       EEEEE   DDDD                    =
#                                                                          =
#   This file is a part of Toped project (C) 2001-2012 Toped developers    =
# ------------------------------------------------------------------------ =
#           $URL$
#        Created: Sun Feb 19 10:08:17 2012
#     Originator: yannick.heinrich@gmail.com
#    Description: Top configuration file of the cmake utility
#---------------------------------------------------------------------------
#  Revision info Rewritten from scratch (after a long break)
#---------------------------------------------------------------------------
#      $Revision$
#          $Date$
#        $Author$
#===========================================================================

cmake_minimum_required(VERSION 3.15)
set(PRJ_NAME toped)
set(TOPED_VERSION 0.98)

project(${PRJ_NAME} VERSION ${TOPED_VERSION})

#---------------------------------------------------------------------------
# manage compiler features and flags common for all modules
add_library(toped_compiler_flags INTERFACE)
target_compile_features(toped_compiler_flags INTERFACE cxx_std_11)

# add compiler warning flags just when building this project via
# the BUILD_INTERFACE genex
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(toped_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

#---------------------------------------------------------------------------
# external libraries, prerequisites for the project
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
#find_package(wxWidgets COMPONENTS core base aui adv REQUIRED)
find_package(wxWidgets COMPONENTS core base aui gl REQUIRED)
include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS})
#---------------------------------------------------------------------------
# add project modules as internal libraries
add_subdirectory(tpd_common)
add_subdirectory(tpd_GL)
add_subdirectory(tpd_DB)
add_subdirectory(tpd_ifaces)
add_subdirectory(tpd_parser)
add_subdirectory(tpd_bidfunc)

#---------------------------------------------------------------------------
# Packging & install
file(GLOB TPD_ICONS      "icons/*.png")
file(GLOB TPD_FONTS      "fonts/*.glf")
file(GLOB TPD_TLL        "tll/*.tll")
file(GLOB TPD_RESOURCES  "ui/*")
file(GLOB TPD_UTILS      "user_functions/general/*.tll")

set_source_files_properties( "toped.icns"  PROPERTIES
                             MACOSX_PACKAGE_LOCATION Resources
)
set_source_files_properties( ${TPD_ICONS} PROPERTIES
                             MACOSX_PACKAGE_LOCATION MacOS/icons
)
set_source_files_properties( ${TPD_FONTS} PROPERTIES
                             MACOSX_PACKAGE_LOCATION MacOS/fonts
)
set_source_files_properties( ${TPD_TLL} PROPERTIES
                             MACOSX_PACKAGE_LOCATION MacOS/tll
)
set_source_files_properties( ${TPD_RESOURCES} PROPERTIES
                             MACOSX_PACKAGE_LOCATION MacOS/ui
)
set_source_files_properties( ${TPD_UTILS} PROPERTIES
                             MACOSX_PACKAGE_LOCATION MacOS/tll/utils
)
set(MACOSX_BUNDLE_ICON_FILE "toped.icns")
set(WITH_MAC_BUNDLE_NAME "toped")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/wxcocoa_config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/wxcocoa_config.h)

#---------------------------------------------------------------------------
add_executable(${PRJ_NAME} MACOSX_BUNDLE
                      src/tpdph.cpp
                      src/browsers.cpp
                      src/layoutcanvas.cpp
                      src/resourcecenter.cpp
                      src/tui.cpp
                      src/techeditor.cpp
                      src/toped.cpp
                      src/main.cpp
                      ${TPD_FONTS} ${TPD_ICONS} ${TPD_TLL} ${TPD_UTILS} ${TPD_RESOURCES}
)

target_link_libraries(${PRJ_NAME} PUBLIC
#                      ${OPENGL_LIBRARIES}
#                      ${GLEW_LIBRARIES}
#                      ${wxWidgets_LIBRARIES}
#                      wx_osx_cocoau_gl-2.9
                      GLEW::GLEW
                      z
                      toped_compiler_flags
                      tpd_common
                      tpd_GL
                      tpd_DB
                      tpd_parser
                      tpd_bidfunc
                      tpd_ifaces
)

target_include_directories(${PRJ_NAME} PUBLIC
                      tpd_common
                      tpd_GL
                      tpd_DB
                      tpd_parser
                      tpd_bidfunc
                      tpd_ifaces
)



#install(TARGETS ${PRJ_NAME}
#        RUNTIME
#        DESTINATION bin
#        COMPONENT applications
#)
#install(FILES ${TPD_ICONS} DESTINATION icons)
#install(FILES ${TPD_FONTS} DESTINATION fonts)
#install(FILES ${TPD_TLL}   DESTINATION tll)
#install(FILES ${TPD_UTILS} DESTINATION utils)

#install(TARGETS mylib
#  ARCHIVE
#  DESTINATION lib
#  COMPONENT libraries)
#install(FILES mylib.h
#  DESTINATION include
#  COMPONENT headers)


#set(CPACK_BUNDLE_NAME "Toped")
#set(CPACK_BUNDLE_ICON "toped.icns")
##set(CPACK_BUNDLE_PLIST $PRJ_NAME)
#
#set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IC layout editor")
#set(CPACK_PACKAGE_VENDOR "${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "99")
#
#include(CPACK)

