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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Sep  9 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Render dispatcher
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "trend.h"
#include <GL/glew.h>
#include "glf.h"
#include "tolder.h"
#include "tenderer.h"
#include "viewprop.h"


trend::FontLibrary*              fontLib = NULL;
trend::TrendCenter*              TRENDC  = NULL;
extern layprop::PropertyCenter*  PROPC;

//=============================================================================
//
//
//
trend::TGlfSymbol::TGlfSymbol(FILE* ffile)
{
   fread(&_alvrtxs, 1, 1, ffile);
   _vdata = DEBUG_NEW float[2 * _alvrtxs];
   fread(&_alchnks, 1, 1, ffile);
   _idata = DEBUG_NEW byte[3 * _alchnks];
   fread(&_alcntrs, 1, 1, ffile);
   _cdata = DEBUG_NEW byte[_alcntrs];
   // init the symbol boundaries
   _minX = _minY =  10;
   _maxX = _maxY = -10;
   // get the vertexes
   for (byte i = 0; i < _alvrtxs; i++)
   {
      float vX, vY;
      fread(&vX, 4, 1, ffile);
      fread(&vY, 4, 1, ffile);
      // update the symbol boundaries
      if      (vX < _minX) _minX = vX;
      else if (vX > _maxX) _maxX = vX;
      if      (vY < _minY) _minY = vY;
      else if (vY > _maxY) _maxY = vY;
      _vdata[2*i  ] = vX;
      _vdata[2*i+1] = vY;
   }
   // get index chunks
   for (byte i = 0; i < _alchnks; i++)
      fread(&(_idata[i*3]), 3, 1, ffile);
   // get contour data
   for (byte i = 0; i < _alcntrs; i++)
      fread(&(_cdata[i  ]), 1, 1, ffile);
}

void trend::TGlfSymbol::dataCopy(GLfloat* vxarray, GLuint* ixarray, word ioffset)
{
   memcpy(vxarray, _vdata, _alvrtxs * sizeof(float) * 2);
   for (word i = 0; i < _alchnks * 3; i++)
      ixarray[i] = _idata[i] + ioffset;
}

trend::TGlfSymbol::~TGlfSymbol()
{
   delete [] _vdata;
   delete [] _idata;
   delete [] _cdata;
}

//=============================================================================
//
//
//
trend::TGlfRSymbol::TGlfRSymbol(TGlfSymbol* tsym, word voffset, word ioffset)
{
   _alcntrs = tsym->_alcntrs;
   _alchnks = tsym->_alchnks;
   //
   _csize = DEBUG_NEW GLsizei[_alcntrs];
   _firstvx = DEBUG_NEW GLint[_alcntrs];
   for (unsigned i = 0; i < _alcntrs; i++)
   {
      _csize[i] = tsym->_cdata[i] + 1;
      _firstvx[i] = voffset;
      if (0 != i)
      {
         _firstvx[i] += (tsym->_cdata[i-1] + 1);
         _csize[i]   -= (tsym->_cdata[i-1] + 1);
      }
   }
   _firstix = ioffset  * sizeof(unsigned);
   //
   _minX = tsym->_minX;
   _maxX = tsym->_maxX;
   _minY = tsym->_minY;
   _maxY = tsym->_maxY;
}

void trend::TGlfRSymbol::draw(bool fill)
{
   glMultiDrawArrays(GL_LINE_LOOP, _firstvx, _csize, _alcntrs);
   if (!fill) return;
   glDrawElements(GL_TRIANGLES, _alchnks * 3, GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_firstix));
}

trend::TGlfRSymbol::~TGlfRSymbol()
{
   delete [] _firstvx;
   delete [] _csize;
}

//=============================================================================
//
//
//
trend::TGlfFont::TGlfFont(std::string filename, std::string& fontname) :
   _status(0), _pitch(0.1f), _spaceWidth(0.5f)
{
   FILE* ffile = fopen(filename.c_str(), "rb");
   _pbuffer = 0;
   _ibuffer = 0;
   if (NULL == ffile)
   {
      _status = 1;
      //@ TODO tellerror(...)
      return;
   }
   char fileh[4];
   fread(fileh, 3, 1, ffile);
   fileh[3] = 0x0;
   if (strcmp(fileh, "GLF"))
   {
      _status = 2;
      //@TODO tellerror(..."filename doesn't appear to be a font file");
   }
   else
   {
      // Get the font name
      char fname [97];
      fread(fname, 96, 1, ffile);
      fname[96] = 0x0;
      fontname = fname;
      // Get the layDef of symbols
      fread(&_numSymbols, 1, 1, ffile);
      // Read the rest  of bytes to 128 (unused)
      byte unused [28];
      fread(unused, 28, 1, ffile);
      // Finally - start parsing the symbol data
      _all_vertexes = 0;
      _all_indexes = 0;
      for (byte i = 0; i < _numSymbols; i++)
      {
         byte asciiCode;
         fread(&asciiCode, 1, 1, ffile);
         TGlfSymbol* csymbol = DEBUG_NEW TGlfSymbol(ffile);
         _tsymbols[asciiCode] = csymbol;
         _all_vertexes += csymbol->alvrtxs();
         _all_indexes  += 3 * csymbol->alchnks(); // only triangles!
      }
   }
   fclose(ffile);
}

void trend::TGlfFont::collect()
{
   // Create the VBO
   GLuint ogl_buffers[2];
   glGenBuffers(2, ogl_buffers);
   _pbuffer = ogl_buffers[0];
   _ibuffer = ogl_buffers[1];
   // Bind the buffers
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   glBufferData(GL_ARRAY_BUFFER                   ,
                2 * _all_vertexes * sizeof(float) ,
                NULL                              ,
                GL_STATIC_DRAW                     );
   float* cpoint_array = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER         ,
                _all_indexes * sizeof(int)      ,
                NULL                            ,
                GL_STATIC_DRAW                   );
   GLuint* cindex_array = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

   //... and collect them deleting meanwhile the temporary objects
   word vrtx_indx = 0;
   word indx_indx = 0;
   for (TFontMap::const_iterator CRS = _tsymbols.begin(); CRS != _tsymbols.end(); CRS++)
   {
      TGlfRSymbol* csymbol = DEBUG_NEW TGlfRSymbol(CRS->second, vrtx_indx, indx_indx);
      CRS->second->dataCopy(&(cpoint_array[2 * vrtx_indx]), &(cindex_array[indx_indx]), vrtx_indx);
      vrtx_indx += CRS->second->alvrtxs();
      indx_indx += 3 * CRS->second->alchnks();
      _symbols[CRS->first] = csymbol;
      delete CRS->second;
   }
   _tsymbols.clear();
   assert(_all_vertexes == vrtx_indx);
   assert(_all_indexes  == indx_indx);
   glUnmapBuffer(GL_ARRAY_BUFFER);
   glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool trend::TGlfFont::bindBuffers()
{
   if ((0 ==_pbuffer) || (0 == _ibuffer)) return false;
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   bufferSize++;

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
   glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   bufferSize++;

   return true;
}

void trend::TGlfFont::getStringBounds(const std::string* text, DBbox* overlap)
{
   // initialise the boundaries
   float top, bottom, left, right;
   if ((0x20 == (*text)[0]) ||  (_symbols.end() == _symbols.find((*text)[0])))
   {
      left =   0.0f; right  = _spaceWidth;
      top  = -_spaceWidth; bottom = _spaceWidth;
   }
   else
   {
      left   = _symbols[(*text)[0]]->minX();
      right  = _symbols[(*text)[0]]->maxX();
      bottom = _symbols[(*text)[0]]->minY();
      top    = _symbols[(*text)[0]]->maxY();
   }
   // traverse the rest of the string
   for (unsigned i = 1; i < text->length() ; i++)
   {
      FontMap::const_iterator CSI = _symbols.find((*text)[i]);
      if ((0x20 == (*text)[i]) || (_symbols.end() == CSI))
         right += _spaceWidth;
      else
      {
         right += CSI->second->maxX() - CSI->second->minX() + _pitch;

         /* Update top/bottom bounds */
         if (CSI->second->minY() < bottom) bottom = CSI->second->minY();
         if (CSI->second->maxY() > top   ) top    = CSI->second->maxY();
      }
   }
   (*overlap) = DBbox(TP(left, bottom, OPENGL_FONT_UNIT), TP(right, top, OPENGL_FONT_UNIT));
}

void trend::TGlfFont::drawString(const std::string* text, bool fill)
{
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_FLOAT, 0, NULL);
   if (fill)
      glEnableClientState(GL_INDEX_ARRAY);
   float right_of = 0.0f, left_of = 0.0f;
   for (unsigned i = 0; i < text->length() ; i++)
   {
      FontMap::const_iterator CSI = _symbols.find((*text)[i]);
      if (i != 0)
      {
         // move one _pitch right
         if ((0x20 == (*text)[i]) || (_symbols.end() == CSI))
            left_of = 0.0f;
         else
            left_of = -CSI->second->minX()+_pitch;
         glTranslatef(left_of+right_of, 0, 0);
      }
      if ((0x20 == (*text)[i]) || (_symbols.end() == CSI))
      {
         glTranslatef(_spaceWidth, 0, 0);
         right_of = 0.0f;
      }
      else
      {
         CSI->second->draw(fill);
         right_of = CSI->second->maxX();
      }
   }
   if (fill)
      glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
}

trend::TGlfFont::~TGlfFont()
{
   for (FontMap::const_iterator CS = _symbols.begin(); CS != _symbols.end(); CS++)
      delete (CS->second);
   GLuint ogl_buffers[2] = {_pbuffer, _ibuffer};
   glDeleteBuffers(2, ogl_buffers);
}

//=============================================================================
//
//
//
trend::FontLibrary::FontLibrary(bool fti) :
   _fti(fti), _activeFontName("")
{
   if (!_fti) glfInit();
}

bool trend::FontLibrary::loadLayoutFont(std::string fontfile)
{
   if (_fti)
   {
      // Parse the font library
      TGlfFont* curFont = DEBUG_NEW TGlfFont(fontfile, _activeFontName);
      if (!curFont->status())
      {
         // fit it in a VBO
         curFont->collect();
         _oglFont[_activeFontName] = curFont;
         return true;
      }
      return false;
   }
   else
   {
      char* chFontName = NULL;
      int fontDescriptor = glfLoadFont(fontfile.c_str(), chFontName);
      if ( -1 == fontDescriptor )
      {
         std::ostringstream ost1;
         ost1<<"Error loading font file \"" << fontfile << "\". All text objects will not be properly processed";
         tell_log(console::MT_ERROR,ost1.str());
         return false;
      }
      else
      {
         assert(chFontName);
         _activeFontName = std::string(chFontName);
         _ramFont[_activeFontName] = fontDescriptor;
         return true;
      }
   }
}

bool trend::FontLibrary::selectFont(std::string fname)
{
   if (_fti)
   {
      if (_oglFont.end() != _oglFont.find(fname))
      {
         _activeFontName = fname;
         return true;
      }
      return false;
   }
   else
   {
      if (_ramFont.end() != _ramFont.find(fname))
      {
         if (0 == glfSelectFont(_ramFont[fname]))
         {
            _activeFontName = fname;
            return true;
         }
         else return false;
      }
      return false;
   }
}

void trend::FontLibrary::getStringBounds(const std::string* text, DBbox* overlap)
{
   if (_fti)
   {
      assert(NULL != _oglFont[_activeFontName]); // make sure that fonts are initialized
      _oglFont[_activeFontName]->getStringBounds(text, overlap);
   }
   else
   {
      float minx, miny, maxx, maxy;
      glfGetStringBounds(text->c_str(),&minx, &miny, &maxx, &maxy);
      (*overlap) = DBbox(TP(minx,miny,OPENGL_FONT_UNIT), TP(maxx,maxy,OPENGL_FONT_UNIT));
   }
}

void trend::FontLibrary::drawString(const std::string* text, bool fill)
{
   if (_fti)
   {
     _oglFont[_activeFontName]->drawString(text, fill);
   }
   else
   {
      glfDrawTopedString(text->c_str(), fill);
   }
}

void trend::FontLibrary::drawWiredString(std::string text)
{
   if (_fti)
   {
      bindFont();
      _oglFont[_activeFontName]->drawString(&text, false);
      unbindFont();
   }
   else
   {
      glfDrawWiredString(text.c_str());
   }
}

void trend::FontLibrary::drawSolidString(std::string text)
{
   if (_fti)
   {
      bindFont();
      _oglFont[_activeFontName]->drawString(&text, true);
      unbindFont();
   }
   else
   {
      glfDrawSolidString(text.c_str());
   }
}

bool  trend::FontLibrary::bindFont()
{
   assert(_fti);
   if (NULL != _oglFont[_activeFontName])
      return _oglFont[_activeFontName]->bindBuffers();
   else
      return false;
}

void  trend::FontLibrary::unbindFont()
{
   assert(_fti);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//void trend::FontLibrary::allFontNames(NameList& allFontNames)
//{
//   if (_fti)
//   {
//      for(OglFontCollectionMap::const_iterator CF = _oglFont.begin(); CF != _oglFont.end(); CF++)
//         allFontNames.push_back(CF->first);
//   }
//   else
//   {
//      for(RamFontCollectionMap::const_iterator CF = _ramFont.begin(); CF != _ramFont.end(); CF++)
//         allFontNames.push_back(CF->first);
//   }
//}

word trend::FontLibrary::numFonts()
{
   if (_fti)
      return _oglFont.size();
   else
      return _ramFont.size();
}

trend::FontLibrary::~FontLibrary()
{
   if (_fti)
   {
      for (OglFontCollectionMap::const_iterator CF = _oglFont.begin(); CF != _oglFont.end(); CF++)
         delete (CF->second);
   }
   else
      glfClose();
}


//=============================================================================
trend::TrendCenter::TrendCenter(bool gui, bool forceBasic, bool sprtVbo, bool sprtShaders) :
   _cRenderer     (              NULL),
   _hRenderer     (              NULL)
{
   if      (!gui)             _renderType = trend::tocom;
   else if ( forceBasic )     _renderType = trend::tolder;
   else if ( sprtShaders)     _renderType = trend::toshader;
   else if ( sprtVbo    )     _renderType = trend::tenderer;
   else                       _renderType = trend::tolder;
   fontLib = DEBUG_NEW trend::FontLibrary(_renderType > trend::tolder);

}

trend::TrendBase* trend::TrendCenter::getCRenderer()
{
   // Don't block the drawing if the databases are
   // locked. This will block all redraw activities including UI
   // which have nothing to do with the DB. Drop a message in the log
   // and keep going!
   assert(NULL == _cRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp))
   {
      switch (_renderType)
      {
         case trend::tocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::tolder   :
            _cRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() );break;
         case trend::tenderer :
            _cRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::toshader : assert(false);          break;// TODO
         default: assert(false); break;
      }
   }
   else
   {
      // If property DB is locked - we can't do much drawing even if the
      // DB is not locked. In the same time there should not be an operation
      // which holds the property DB lock for a long time. So it should be
      // rather an exception
      tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
   }
   return _cRenderer;
}

trend::TrendBase* trend::TrendCenter::getHRenderer()
{
   assert(NULL == _hRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp))
   {
      switch (_renderType)
      {
         case trend::tocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::tolder   :
            _hRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() ); break;
         case trend::tenderer :
            _hRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::toshader : assert(false);          break;// TODO
         default: assert(false); break;
      }
   }
   return _hRenderer;
}

void trend::TrendCenter::releaseCRenderer()
{
   assert(NULL != _cRenderer);
   PROPC->unlockDrawProp(_cRenderer->drawprop(), false);
   delete (_cRenderer);
   _cRenderer = NULL;
}

void trend::TrendCenter::releaseHRenderer()
{
   assert(NULL != _hRenderer);
   PROPC->unlockDrawProp(_hRenderer->drawprop(), false);
   delete (_hRenderer);
   _hRenderer = NULL;
}

void trend::TrendCenter::drawFOnly()
{
//   if (NULL != _cRenderer) _cRenderer->grcDraw(); // TODO
}

void trend::TrendCenter::drawGrid()
{
   // render the grid
   for (byte gridNo = 0; gridNo < 3; gridNo++)
   {
      const layprop::LayoutGrid* cgrid = PROPC->grid(gridNo);
      if ((NULL !=  cgrid) && cgrid->visual())
         _cRenderer->grid(cgrid->step(), cgrid->color());
   }
}

void trend::TrendCenter::drawZeroCross()
{
   if (PROPC->zeroCross())
      _cRenderer->zeroCross();
}

trend::TrendCenter::~TrendCenter()
{
   delete fontLib;
   if (NULL != _cRenderer) delete _cRenderer;
}
