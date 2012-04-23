noinst_HEADERS = config_cfg.h gdselements.h gds_globals.h gdsobject.h \
gdsobjects.h gdsparse.h gdspath.h gdspolygon.h gdstext.h gdsto3d.h \
gds_types.h process_cfg.h
 
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
lib_LTLIBRARIES = libgdsto3d.la

# the application source, library search path, and link libraries
libgdsto3d_la_SOURCES = config_cfg.cpp gdsobject.cpp  gdsparse.cpp \
   gdspolygon.cpp gds_types.cpp gds_globals.cpp  gdsobjects.cpp \
   gdspath.cpp   gdstext.cpp process_cfg.cpp

#########################################################################
# EXTRAS
#########################################################################
## AM_CPPFLAGS is the replacement for INCLUDES

