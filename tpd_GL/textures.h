//===========================================================================
//                                                                          =
//   This program is free software; you can redistribute it and/or modify   =
//   it under the terms of the GNU General Public License as published by   =
//   the Free Software Foundation; either version 2 of the License, or      =
//   (at your option) any later version.                                    =
// ------------------------------------------------------------------------ =
//                  TTTTT    OOO    PPPP    EEEE    DDDD                    =
//                  T T T   O   O   P   P   E       D   D                   =
//                    T    O     O  PPPP    EEE     D    D                  =
//                    T     O   O   P       E       D   D                   =
//                    T      OOO    P       EEEEE   DDDD                    =
//                                                                          =
//   This file is a part of Toped project (C) 2001-2025 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Tue Aug 12 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Handling textures for OGL rendering
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TEXTURES_H
#define TEXTURES_H

#include <string>
#include <GL/glew.h>

namespace trend {
   class Texture
   {
   public:
               Texture(GLenum texTarget, const std::string& fName, GLenum texUnit);
      bool     Load();
      void     Bind() const;
      void     GetImageSize(int& imageWidth, int& imageHeight) const;
      GLuint   GetTexture() const { return _tID; }
      
   private:
      std::string         _fileName;
      GLenum              _tType = 0;
      GLuint              _tID   = 0;
      GLenum              _tUnit = GL_TEXTURE0;
      int                 _imageWidth  = 0;
      int                 _imageHeight = 0;
   };
}

#endif //TEXTURES
