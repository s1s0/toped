AC_DEFUN([AX_CHECK_GLEW], [
  AC_CHECK_HEADER([GL/glew.h])
  AC_CHECK_LIB(GLEW, glewContextInit, , no_glew="yes")

  if test -z "$no_glew"; then
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <GL/glew.h>
#if !defined(GL_SGIS_texture_lod) || !defined(GL_ARB_vertex_buffer_object) || !defined(GL_ARB_pixel_buffer_object)
# error
#endif
                                    ]],
                                    [[glewContextInit()]])],
                   [LIBS="-lGLEW $LIBS"])
  fi
])
