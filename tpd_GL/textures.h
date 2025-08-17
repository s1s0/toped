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
#include <map>
#include <GL/glew.h>

namespace trend {
   
   
   class Texture
   {
   public:
               Texture(GLenum texTarget, const wxString& fName, GLenum texUnit);
      bool     Load();
      void     Bind() const;
      void     unBind() const;
      void     GetImageSize(int& imageWidth, int& imageHeight) const;
      unsigned scaleFactor() const {return _scaleFactor;}
      GLuint   GetOglTID() const { return _oglTID; }
      
   private:
      std::string         _fileName;
      GLenum              _tType = 0;
      GLuint              _oglTID   = 0;
      GLenum              _tUnit = GL_TEXTURE0;
      int                 _imageWidth  = 0;
      int                 _imageHeight = 0;
      unsigned            _scaleFactor = 1;
   };
   

   class TextureVault {
      public:
         typedef  std::map<std::string, trend::Texture*>       TextureMap;
         static TextureVault*    getInstance();
         void                    addTexture(const wxString fname, const std::string tname);
         bool                    texDefined(const std::string texture) const {return (_textures.end() != _textures.find(texture));}
         const Texture*          getTexture(const std::string) const;
      private:
                                 TextureVault() { _curTexUnit = 0;}
         static TextureVault*    _singleton;
         TextureMap              _textures;
         GLushort                _curTexUnit;

   };



}

#endif //TEXTURES
