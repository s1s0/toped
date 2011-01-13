
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = gds2vrml

# the application source, library search path, and link libraries
gds2vrml_SOURCES = gds2vrml.cpp  gdsobject_vrml.cpp gdsparse_vrml.cpp

#########################################################################
# EXTRAS
#########################################################################
## AM_CPPFLAGS is the replacement for INCLUDES
AM_CPPFLAGS = -I$(top_srcdir)/libgdsto3d

gds2vrml_LDADD = $(top_builddir)/libgdsto3d/libgdsto3d.la
