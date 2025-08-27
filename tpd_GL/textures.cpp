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

#include "tpdph.h"

#include "wx/image.h"
#include "wx/wx.h"

#include "textures.h"
#include "basetrend.h"

trend::TextureVault* trend::TextureVault::_singleton = NULL;

//=====================================================================================
trend::Texture::Texture(GLenum texTarget, const wxString& fName, GLenum texUnit) :
   _fileName      ( fName        )
 , _tType         ( texTarget    )
 , _tUnit         ( texUnit      )
 , _tLoaded       ( false        )
{
}

bool trend::Texture::Load()
{
   assert(wxFileExists(_fileName));

   wxImage* img = DEBUG_NEW wxImage( _fileName );
   
   DBGL_CALL(glGenTextures,      1, &_oglTID);
   DBGL_CALL(glBindTexture, _tType,  _oglTID);
//   printf("Texture buffer %2d generated\n",_oglTID);
   
   _imageWidth    = img->GetWidth();
   _imageHeight   = img->GetHeight();
   
   // note: must make a local copy before passing the data to OpenGL, as GetData() returns RGB
   // and we want the Alpha channel if it's present. Additionally OpenGL seems to interpret the
   // data upside-down so we need to compensate for that.
   GLubyte* bitmapData     = img->GetData();
   GLubyte* alphaData      = img->GetAlpha();
   int      bytesPerPixel  = img->HasAlpha() ?  4 : 3;
   int      imageSize      = _imageWidth * _imageHeight * bytesPerPixel;
   GLubyte* imageData      = DEBUG_NEW GLubyte[imageSize];

   int rev_val=_imageHeight-1;
   for(int y = 0; y < _imageHeight; y++)
      for(int x = 0; x < _imageWidth; x++)
      {
         int didx = (x+         y *_imageWidth)*bytesPerPixel;
         int sidx = (x+(rev_val-y)*_imageWidth)*3;
         imageData[didx  ] = bitmapData[sidx    ];
         imageData[didx+1] = bitmapData[sidx + 1];
         imageData[didx+2] = bitmapData[sidx + 2];
         if(img->HasAlpha())
            imageData[didx+3]= alphaData[ x+(rev_val-y)*_imageWidth ];
      }

   DBGL_CALL(  glTexImage2D
             , _tType
             , 0                                   // level of detail
             , img->HasAlpha() ?  GL_RGBA : GL_RGB //bytesPerPixel
             , _imageWidth
             , _imageHeight
             , 0
             , img->HasAlpha() ?  GL_RGBA : GL_RGB
             , GL_UNSIGNED_BYTE
             , imageData
             );

   DBGL_CALL(glTexParameteri, _tType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   DBGL_CALL(glTexParameteri, _tType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   DBGL_CALL(glTexParameteri, _tType, GL_TEXTURE_BASE_LEVEL, 0);
   DBGL_CALL(glTexParameteri, _tType, GL_TEXTURE_WRAP_S, GL_REPEAT);
   DBGL_CALL(glTexParameteri, _tType, GL_TEXTURE_WRAP_T, GL_REPEAT);
   //glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);

   DBGL_CALL(glGenerateMipmap, _tType);
   DBGL_CALL(glBindTexture, _tType, 0);

   delete [] imageData;
   delete img;
   _tLoaded = true;
   return true;
}

void trend::Texture::GetImageSize(int& ImageWidth, int& ImageHeight) const
{
     ImageWidth = _imageWidth;
     ImageHeight = _imageHeight;
}

void trend::Texture::Bind() const
{
   assert(_tLoaded);
   DBGL_CALL(glActiveTexture, _tUnit);
   DBGL_CALL(glBindTexture, _tType, _oglTID);
}

void trend::Texture::unBind() const
{
//   DBGL_CALL(glActiveTexture, _tUnit);
   assert(_tLoaded);
   DBGL_CALL(glBindTexture, _tType, 0);
}

//=====================================================================================
trend::TextureVault*  trend::TextureVault::getInstance()
{
   if(NULL == _singleton)
   {
      _singleton = new TextureVault();
//      printf("TextureVault created\n");
   }
//   else {
//      assert(false); //This class is supposed to have a single instance!
//   }
   return _singleton;
}


void trend::TextureVault::registerTexture(const wxString fname, const std::string tname)
{
   trend::Texture* texture = DEBUG_NEW trend::Texture(GL_TEXTURE_2D, fname.mb_str(), GL_TEXTURE1+_curTexUnit);
   _textures.insert(std::pair<std::string, trend::Texture*>(tname, texture));
//   printf("Texture %d registered\n", _curTexUnit);
   _curTexUnit++;
}

void trend::TextureVault::loadAllTextures()
{
   for(auto texture:_textures)
      if (!texture.second->loaded())
         texture.second->Load();
   
}

const trend::Texture* trend::TextureVault::getTexture(const std::string tname) const
{
//   trend::Texture* result = _textures.find(tname);
   
   if (_textures.end() == _textures.find(tname))
      return nullptr;
   else
   {
      trend::Texture* result = _textures.find(tname)->second;
      assert(result->loaded());
      return _textures.find(tname)->second;
   }
}

