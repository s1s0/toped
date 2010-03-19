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
//   This file is a part of Toped project (C) 2001-2009 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Jan 11 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Canvas drawing properties
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <GL/glew.h>
#include <sstream>
#include <string>
#include "drawprop.h"
#include "viewprop.h"
#include "ps_out.h"
#include "glf.h"
#include "tuidefs.h"


GLubyte cell_mark_bmp[30] = {
   0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x18, 0x18,
   0x24, 0x48, 0x42, 0x84, 0x81, 0x02, 0x42, 0x84, 0x24, 0x48,
   0x18, 0x18, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00
};

GLubyte text_mark_bmp[30] = {
   0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x09, 0x20, 0x11, 0x10,
   0x21, 0x08, 0x41, 0x04, 0x8F, 0xE2, 0x40, 0x04, 0x20, 0x08,
   0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00
};

GLubyte array_mark_bmp[30]= {
   0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10,
   0x20, 0x08, 0x50, 0x0A, 0x8F, 0xE2, 0x44, 0x44, 0x22, 0x88,
   0x11, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00
};

const layprop::tellRGB        layprop::DrawProperties::_defaultColor(127,127,127,127);
const layprop::LineSettings   layprop::DrawProperties::_defaultSeline("", 0xffff, 1, 3);
const byte                    layprop::DrawProperties::_defaultFill [128] = {
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00
};

//=============================================================================
//
//
//
layprop::TGlfSymbol::TGlfSymbol(FILE* ffile)
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

void layprop::TGlfSymbol::dataCopy(GLfloat* vxarray, GLuint* ixarray, word ioffset)
{
   memcpy(vxarray, _vdata, _alvrtxs * sizeof(float) * 2);
   for (word i = 0; i < _alchnks * 3; i++)
      ixarray[i] = _idata[i] + ioffset;
}

layprop::TGlfSymbol::~TGlfSymbol()
{
   delete [] _vdata;
   delete [] _idata;
   delete [] _cdata;
}

//=============================================================================
//
//
//
layprop::TGlfRSymbol::TGlfRSymbol(TGlfSymbol* tsym, word voffset, word ioffset)
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

void layprop::TGlfRSymbol::draw(bool fill)
{
   glMultiDrawArrays(GL_LINE_LOOP, _firstvx, _csize, _alcntrs);
   if (!fill) return;
   glDrawElements(GL_TRIANGLES, _alchnks * 3, GL_UNSIGNED_INT, (const GLvoid*)(_firstix));
}

layprop::TGlfRSymbol::~TGlfRSymbol()
{
   delete [] _firstvx;
   delete [] _csize;
}

//=============================================================================
//
//
//
layprop::TGlfFont::TGlfFont(std::string filename, std::string& fontname) :
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
      // Get the number of symbols
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

void layprop::TGlfFont::collect()
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

bool layprop::TGlfFont::bindBuffers()
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

void layprop::TGlfFont::getStringBounds(const std::string* text, DBbox* overlap)
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

void layprop::TGlfFont::drawString(const std::string* text, bool fill)
{
   glVertexPointer(2, GL_FLOAT, 0, NULL);
   glEnableClientState(GL_VERTEX_ARRAY);
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

layprop::TGlfFont::~TGlfFont()
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
layprop::FontLibrary::FontLibrary(bool fti) :
   _fti(fti), _activeFontName("")
{
   if (!_fti) glfInit();
}

bool layprop::FontLibrary::LoadLayoutFont(std::string fontfile)
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

bool layprop::FontLibrary::selectFont(std::string fname)
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

void layprop::FontLibrary::getStringBounds(const std::string* text, DBbox* overlap)
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

void layprop::FontLibrary::drawString(const std::string* text, bool fill)
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

void layprop::FontLibrary::drawWiredString(std::string text)
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

void layprop::FontLibrary::drawSolidString(std::string text)
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

bool  layprop::FontLibrary::bindFont()
{
   assert(_fti);
   if (NULL != _oglFont[_activeFontName])
      return _oglFont[_activeFontName]->bindBuffers();
   else
      return false;
}

void  layprop::FontLibrary::unbindFont()
{
   assert(_fti);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void layprop::FontLibrary::allFontNames(nameList& allFontNames)
{
   if (_fti)
   {
      for(OglFontCollectionMap::const_iterator CF = _oglFont.begin(); CF != _oglFont.end(); CF++)
         allFontNames.push_back(CF->first);
   }
   else
   {
      for(RamFontCollectionMap::const_iterator CF = _ramFont.begin(); CF != _ramFont.end(); CF++)
         allFontNames.push_back(CF->first);
   }
}

word layprop::FontLibrary::numFonts()
{
   if (_fti)
      return _oglFont.size();
   else
      return _ramFont.size();
}

layprop::FontLibrary::~FontLibrary()
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
layprop::DrawProperties::DrawProperties() : _clipRegion(0,0)
{
   _blockFill             = false;
   _drawingLayer          = 0;
   _cellMarksHidden       = true;
   _textMarksHidden       = true;
   _cellBoxHidden         = true;
   _textBoxHidden         = true;
   _refStack              = NULL;
   _propertyState         = DB;
   _adjustTextOrientation = false;
   _currentOp             = console::op_none;
   _curlay                = 1;
   _visualLimit           = 40;
   _cellDepthAlphaEbb     = 0;
   _cellDepthView         = 0;
}

bool layprop::DrawProperties::addLayer( unsigned layno )
{
   std::ostringstream lname;
   switch (_propertyState)
   {
      case DB : if (_laySetDb.end() != _laySetDb.find(layno)) return false;
                lname << "_UNDEF" << layno;
                _laySetDb[layno] = DEBUG_NEW LayerSettings(lname.str(),"","","");
                return true;
      case DRC: if (_laySetDrc.end() != _laySetDrc.find(layno)) return false;
                lname << "_DRC" << layno;
                _laySetDrc[layno] = DEBUG_NEW LayerSettings(lname.str(),"","","");
                return true;
      default: assert(false);
   }
   return true; // dummy statement to prevent compilation warnings
}

bool layprop::DrawProperties::addLayer(std::string name, unsigned layno, std::string col,
                                       std::string fill, std::string sline)
{
   if ((col != "") && (_layColors.end() == _layColors.find(col)))
   {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if ((fill != "") && (_layFill.end() == _layFill.find(fill)))
   {
      std::ostringstream ost;
      ost << "Warning! Fill \""<<fill<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   if ((sline != "") && (_lineSet.end() == _lineSet.find(sline)))
   {
      std::ostringstream ost;
      ost << "Warning! Line \""<<sline<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   bool new_layer = true;
   switch(_propertyState)
   {
      case DB:
         if (_laySetDb.end() != _laySetDb.find(layno))
         {
            new_layer = false;
            delete _laySetDb[layno];
            std::ostringstream ost;
            ost << "Warning! Layer "<<layno<<" redefined";
            tell_log(console::MT_WARNING, ost.str());
         }
         _laySetDb[layno] = DEBUG_NEW LayerSettings(name,col,fill,sline);
         return new_layer;
      case DRC: //User can't call DRC database directly
      default: assert(false);
   }
}

bool layprop::DrawProperties::addLayer(std::string name, unsigned layno)
{
   switch(_propertyState)
   {
      case DB:
         if (_laySetDb.end() != _laySetDb.find(layno)) return false;
         _laySetDb[layno] = DEBUG_NEW LayerSettings(name,"","","");
         return true;
      case DRC:
         if (_laySetDrc.end() != _laySetDrc.find(layno)) return false;
         _laySetDrc[layno] = DEBUG_NEW LayerSettings(name,"","","");
         return true;
      default: assert(false);
   }
   return false; // dummy statement to prevent compilation warnings
}

unsigned layprop::DrawProperties::addLayer(std::string name)
{
   unsigned layno = 1;
   LaySetList::const_reverse_iterator lastLayNo = getCurSetList().rbegin();
   if (getCurSetList().rend() != lastLayNo)
      layno = lastLayNo->first;
   while (!addLayer(name, layno)) {layno++;}
   return layno;
}

void layprop::DrawProperties::addLine(std::string name, std::string col, word pattern,
                                       byte patscale, byte width) {
   if ((col != "") && (_layColors.end() == _layColors.find(col))) {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if (_lineSet.end() != _lineSet.find(name)) {
      delete _lineSet[name];
      std::ostringstream ost;
      ost << "Warning! Line "<< name <<" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   _lineSet[name] = DEBUG_NEW LineSettings(col,pattern,patscale,width);
}

void layprop::DrawProperties::addColor(std::string name, byte R, byte G, byte B, byte A) {
   if (_layColors.end() != _layColors.find(name)) {
      delete _layColors[name];
      std::ostringstream ost;
      ost << "Warning! Color \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   tellRGB* col = DEBUG_NEW tellRGB(R,G,B,A);
   _layColors[name] = col;
}

void layprop::DrawProperties::addFill(std::string name, byte* ptrn) {
   if (_layFill.end() != _layFill.find(name)) {
      delete [] _layFill[name];
      std::ostringstream ost;
      ost << "Warning! Fill \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   _layFill[name] = ptrn;
}


void layprop::DrawProperties::setCurrentColor(unsigned layno)
{
   _drawingLayer = layno;
   const layprop::tellRGB& theColor = getColor(_drawingLayer);
   glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
}

void layprop::DrawProperties::setGridColor(std::string colname) const
{
   if (_layColors.end() == _layColors.find(colname))
   // put a default gray color if color is not found
      glColor4ub(_defaultColor.red(), _defaultColor.green(), _defaultColor.blue(), _defaultColor.alpha());
   else
   {
      tellRGB* gcol = _layColors.find(colname)->second;
      assert(NULL != gcol);
      glColor4ub(gcol->red(), gcol->green(), gcol->blue(), gcol->alpha());
   }
}

bool layprop::DrawProperties::setCurrentFill(bool force_fill) const
{
   if (REF_LAY == _drawingLayer) return true;
   // The lines below are doing effectively
   // byte* ifill = _layFill[_layset[_drawingLayer]->getfill]
   const LayerSettings* ilayset = findLayerSettings(_drawingLayer);
   if ( (NULL != ilayset) && (!_blockFill || force_fill ) )
   {
      if(ilayset->filled())
      { // layer is filled
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         FillMap::const_iterator ifillset = _layFill.find(ilayset->fill());
         if (_layFill.end() == ifillset)
         { // no stipple defined - will use solid fill
//            glDisable(GL_POLYGON_STIPPLE);
            glEnable(GL_POLYGON_STIPPLE);
            glPolygonStipple(_defaultFill);
         }
         else
         { // no stipple is defined
            glEnable(GL_POLYGON_STIPPLE);
            glPolygonStipple(ifillset->second);
         }
         return true;
      }
      else
      {
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         return false;
      }
   }
   else return false;
}

bool layprop::DrawProperties::layerFilled(unsigned layno) const
{
   assert(REF_LAY != layno);
   const LayerSettings* ilayset = findLayerSettings(layno);
   if ((NULL != ilayset) && !_blockFill)
   {
      if(ilayset->filled()) return true;
      else return false;
   }
   else return false;
}

void layprop::DrawProperties::adjustAlpha(word factor)
{
   //@TODO! - A tell option(function or variable) to adjust the constant (30 below)
   // user must know what's going on, otherwise - the rendering result might be confusing
   // Having done that, the method can be enabled
   const layprop::tellRGB& theColor = getColor(_drawingLayer);
   byte alpha = theColor.alpha();
   word resultingEbb = factor * _cellDepthAlphaEbb;
   if (0 < factor)
   {
      alpha = (resultingEbb > (word)alpha) ? 0 : alpha - resultingEbb;
      glColor4ub(theColor.red(), theColor.green(), theColor.blue(), alpha);
   }
}

bool  layprop::DrawProperties::layerHidden(unsigned layno) const
{
   if (REF_LAY == layno) return false;
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL != ilayset) return ilayset->hidden();
   else return true;
}

bool  layprop::DrawProperties::layerLocked(unsigned layno) const
{
   if (REF_LAY == layno) return false;
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL != ilayset) return ilayset->locked();
   else return true;
}

bool layprop::DrawProperties::selectable(unsigned layno) const
{
   return (!layerHidden(layno) && !layerLocked(layno));
}

void layprop::DrawProperties::drawTextBoundary(const pointlist& ptlist) const
{
   if (_textBoxHidden) return;
   else
   {
//      glColor4f(1.0, 1.0, 1.0, 0.5);
      glLineStipple(1,0x3030);
      glEnable(GL_LINE_STIPPLE);
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
      glDisable(GL_LINE_STIPPLE);
   }
}

void layprop::DrawProperties::drawCellBoundary(const pointlist& ptlist) const
{
   if (_cellBoxHidden) return;
   else
   {
      glColor4f(1.0, 1.0, 1.0, 0.5);
      glLineStipple(1,0xf18f);
      glEnable(GL_LINE_STIPPLE);
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
      glDisable(GL_LINE_STIPPLE);
   }
}

void layprop::DrawProperties::setLineProps(bool selected) const
{
   if (REF_LAY == _drawingLayer)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1,0xf18f);
      if (selected)
         glLineWidth(3);
      else
         glLineWidth(1);
   }
   else
   {
      const layprop::LineSettings* theLine = getLine(_drawingLayer);
      if (selected)
      {
         glLineWidth(theLine->width());
         glEnable(GL_LINE_STIPPLE);
         /*glEnable(GL_LINE_SMOOTH);*/
         glLineStipple(theLine->patscale(),theLine->pattern());
      }
      else
      {
         glLineWidth(1);
         glDisable(GL_LINE_SMOOTH);
         glDisable(GL_LINE_STIPPLE);
      }
   }
}

void layprop::DrawProperties::initDrawRefStack(laydata::CellRefStack* refStack)
{
   _refStack = refStack;
   _blockFill = (NULL != _refStack);
}

void layprop::DrawProperties::clearDrawRefStack()
{
   _refStack = NULL;
   _blockFill = false;
}

void  layprop::DrawProperties::postCheckCRS(const laydata::TdtCellRef* cref)
{
   assert(cref);
   if (_refStack)
   {
      if (_refStack->empty()) _blockFill = true;
      _refStack->push_front(cref);
   }
}

layprop::CellRefChainType layprop::DrawProperties::preCheckCRS(const laydata::TdtCellRef* cref)
{
   assert(cref);
   if (_refStack)
      if (!_refStack->empty())
         if (_refStack->front() == cref )
         {
            _refStack->pop_front();
            if (_refStack->empty())
            {
               _blockFill = false;
               return crc_ACTIVE;
            }
            else return crc_PREACTIVE;
         }
         else return crc_VIEW;
      else return crc_POSTACTIVE;
   else return crc_VIEW;
}

void layprop::DrawProperties::drawReferenceMarks(const TP& p0, const binding_marks mark_type) const
{
   GLubyte* the_mark;
   switch (mark_type)
   {
      case  cell_mark:if (_cellMarksHidden) return;
      else
      {
         glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
         the_mark = cell_mark_bmp;
         break;
      }
      case array_mark:if (_cellMarksHidden) return;
      else
      {
         glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
         the_mark = array_mark_bmp;
         break;
      }
      case  text_mark:if (_textMarksHidden) return;
      else the_mark = text_mark_bmp;break;
      default: assert(false);
   }
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glRasterPos2i(p0.x(),p0.y());
   glBitmap(16,16,7,7,0,0, the_mark);
}

unsigned layprop::DrawProperties::getLayerNo(std::string name) const
{
   for (LaySetList::const_iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
   {
      if (name == CL->second->name()) return CL->first;
   }
   return ERR_LAY;
}

WordList layprop::DrawProperties::getAllLayers() const
{
   WordList listLayers;
   for( LaySetList::const_iterator it = getCurSetList().begin(); it != getCurSetList().end(); it++)
      listLayers.push_back(it->first);
   return listLayers;
}

void layprop::DrawProperties::allUnselectable(DWordSet& layset)
{
   for( LaySetList::const_iterator it = getCurSetList().begin(); it != getCurSetList().end(); it++)
   {
      if (it->second->hidden() || it->second->locked())
         layset.insert(it->first);
   }
}

void layprop::DrawProperties::allInvisible(DWordSet& layset)
{
   for( LaySetList::const_iterator it = getCurSetList().begin(); it != getCurSetList().end(); it++)
   {
      if (it->second->hidden())
         layset.insert(it->first);
   }
}
//WordList layprop::PropertyCenter::getLockedLayers() const
//{
//   WordList lockedLayers;
//   for( LaySetList::const_iterator = getCurSetList().begin(); it != getCurSetList().end(); it++)
//      if(it->second->locked()) lockedLayers.push_back(it->first);
//   return lockedLayers;
//}

std::string layprop::DrawProperties::getLayerName(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL != ilayset) return ilayset->name();
   else return "";
}

std::string layprop::DrawProperties::getColorName(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL != ilayset)
   {
      return ilayset->color();
   }
   else return "";
}

std::string layprop::DrawProperties::getFillName(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL != ilayset)
   {
      return ilayset->fill();
   }
   else return "";
}

std::string layprop::DrawProperties::getLineName(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL != ilayset)
   {
      return ilayset->sline();
   }
   else return "";
}

unsigned layprop::DrawProperties::getTenderLay(unsigned layno) const
{
   switch (_propertyState)
   {
      case DB: return layno;
      case DRC: return DRC_LAY;
      default: assert(false);
   }
}

void layprop::DrawProperties::allLayers(nameList& alllays) const
{
   for (LaySetList::const_iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
      if (REF_LAY != CL->first) alllays.push_back(CL->second->name());
}

void layprop::DrawProperties::allColors(nameList& colist) const
{
   for( ColorMap::const_iterator CI = _layColors.begin(); CI != _layColors.end(); CI++)
      colist.push_back(CI->first);
}

void layprop::DrawProperties::allFills(nameList& filist) const
{
   for( FillMap::const_iterator CI = _layFill.begin(); CI != _layFill.end(); CI++)
      filist.push_back(CI->first);
}

void layprop::DrawProperties::allLines(nameList& linelist) const
{
   for( LineMap::const_iterator CI = _lineSet.begin(); CI != _lineSet.end(); CI++)
      linelist.push_back(CI->first);
}

const layprop::LineSettings* layprop::DrawProperties::getLine(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL == ilayset) return &_defaultSeline;
   LineMap::const_iterator line = _lineSet.find(ilayset->sline());
   if (_lineSet.end() == line) return &_defaultSeline;
   return line->second;
// All the stuff above is equivalent to
//   return _layFill[_layset[layno]->sline()];
// but is safer and preserves constness
}

const layprop::LineSettings* layprop::DrawProperties::getLine(std::string line_name) const
{
   LineMap::const_iterator line = _lineSet.find(line_name);
   if (_lineSet.end() == line) return &_defaultSeline;
   return line->second;
// All the stuff above is equivalent to
//   return _layFill[_layset[layno]->sline()];
// but is safer and preserves constness
}

const byte* layprop::DrawProperties::getFill(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL == ilayset) return &_defaultFill[0];
   FillMap::const_iterator fill_set = _layFill.find(ilayset->fill());
   if (_layFill.end() == fill_set) return &_defaultFill[0];
   return fill_set->second;
// All the stuff above is equivalent to
//   return _layFill[_layset[layno]->fill()];
// but is safer and preserves constness
}

const byte* layprop::DrawProperties::getFill(std::string fill_name) const
{
   FillMap::const_iterator fill_set = _layFill.find(fill_name);
   if (_layFill.end() == fill_set) return &_defaultFill[0];
   return fill_set->second;
// All the stuff above is equivalent to
//   return _layFill[fill_name];
// but is safer and preserves constness
}

const layprop::tellRGB& layprop::DrawProperties::getColor(unsigned layno) const
{
   const LayerSettings* ilayset = findLayerSettings(layno);
   if (NULL == ilayset) return _defaultColor;
   ColorMap::const_iterator col_set = _layColors.find(ilayset->color());
   if (_layColors.end() == col_set) return _defaultColor;
   return *(col_set->second);
// All the stuff above is equivalent to
//   return _layColors[_layset[layno]->color()];
// but is safer and preserves constness
}

const layprop::tellRGB& layprop::DrawProperties::getColor(std::string color_name) const
{
   ColorMap::const_iterator col_set = _layColors.find(color_name);
   if (_layColors.end() == col_set) return _defaultColor;
   return *(col_set->second);
// All the stuff above is equivalent to
//   return _layColors[color_name];
// but is safer and preserves constness
}

void layprop::DrawProperties::savePatterns(FILE* prop_file) const
{
   FillMap::const_iterator CI;
   fprintf(prop_file, "void  fillSetup() {\n");
   for( CI = _layFill.begin(); CI != _layFill.end(); CI++)
   {
      fprintf(prop_file, "   int list _%s = {\n", CI->first.c_str());
      byte* patdef = CI->second;
      for (byte i = 0; i < 16; i++)
      {
         fprintf(prop_file, "      ");
         for (byte j = 0; j < 8; j++)
         {
            if (127 == i*8+j)
               fprintf(prop_file, "0x%02x  "  , patdef[127]);
            else
               fprintf(prop_file, "0x%02x ,", patdef[i*8+j]);
         }
         fprintf(prop_file, "\n");
      }
      fprintf(prop_file, "   };\n\n");
   }
   for( CI = _layFill.begin(); CI != _layFill.end(); CI++)
   {
      fprintf(prop_file, "   definefill(\"%s\", _%s );\n", CI->first.c_str(), CI->first.c_str());
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveColors(FILE* prop_file) const
{
   ColorMap::const_iterator CI;
   fprintf(prop_file, "void  colorSetup() {\n");
   for( CI = _layColors.begin(); CI != _layColors.end(); CI++)
   {
      tellRGB* the_color = CI->second;
      fprintf(prop_file, "   definecolor(\"%s\", %3d, %3d, %3d, %3d);\n",
              CI->first.c_str() ,
              the_color->red()  ,
              the_color->green(),
              the_color->blue() ,
              the_color->alpha()
             );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLayers(FILE* prop_file) const
{
   LaySetList::const_iterator CI;
   fprintf(prop_file, "void  layerSetup() {\n");
   fprintf(prop_file, "   colorSetup(); fillSetup(); lineSetup();\n");
   for( CI = getCurSetList().begin(); CI != getCurSetList().end(); CI++)
   {
      if (0 == CI->first) continue;
      LayerSettings* the_layer = CI->second;
      fprintf(prop_file, "   layprop(\"%s\", %d , \"%s\", \"%s\", \"%s\");\n",
              the_layer->name().c_str()  ,
              CI->first                  ,
              the_layer->color().c_str() ,
              the_layer->fill().c_str()  ,
              the_layer->sline().c_str()  );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLines(FILE* prop_file) const
{
   LineMap::const_iterator CI;
   fprintf(prop_file, "void  lineSetup() {\n");
   for( CI = _lineSet.begin(); CI != _lineSet.end(); CI++)
   {
      LineSettings* the_line = CI->second;
      fprintf(prop_file, "   defineline(\"%s\", \"%s\", 0x%04x , %d, %d);\n",
              CI->first.c_str()         ,
              the_line->color().c_str() ,
              the_line->pattern()       ,
              the_line->patscale()      ,
              the_line->width()            );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLayState(FILE* prop_file) const
{
   LayStateMap::const_iterator CS;
   fprintf(prop_file, "void  layerState() {\n");
   for (CS = _layStateMap.begin(); CS != _layStateMap.end(); CS++)
   {
      LayStateList the_state = CS->second;
      //TODO In order to save a layer status we need something like:
      // locklayers({<all>}, false);
      // hidelayers({<all>}, false);
      // filllayers({<all>}, false);
      // locklayers({<listed>}, true);
      // hidelayers({<listed>}, true);
      // filllayers({<listed>}, true);
      // usinglayer(<layno>);
      // savelaystatus(<name>);
      // where <all>     - All defined layers
      //       <listed>  - The LayStateList members which have the corresponding property
      //                   set to true
      // This method is not called at the moment!
      fprintf(prop_file, "   savelaystatus(\"%s\");\n",
               CS->first.c_str());
   }
   fprintf(prop_file, "}\n\n");

}

const layprop::LayerSettings*  layprop::DrawProperties::findLayerSettings(unsigned layno) const
{
   LaySetList::const_iterator ilayset;
   switch (_propertyState)
   {
      case DB : ilayset = _laySetDb.find(layno) ; if (_laySetDb.end()  == ilayset) return NULL; break;
      case DRC: ilayset = _laySetDrc.find(layno); if (_laySetDrc.end() == ilayset) return NULL; break;
      default: assert(false);
   }
   return ilayset->second;
}

const layprop::LaySetList& layprop::DrawProperties::getCurSetList() const
{
   switch (_propertyState)
   {
      case DB : return _laySetDb;
      case DRC: return _laySetDrc;
      default: assert(false);
   }
}

void layprop::DrawProperties::psWrite(PSFile& psf) const
{
   for(ColorMap::const_iterator CI = _layColors.begin(); CI != _layColors.end(); CI++)
   {
      tellRGB* the_color = CI->second;
      psf.defineColor( CI->first.c_str() , the_color->red(),
                       the_color->green(), the_color->blue() );
   }

   for(FillMap::const_iterator CI = _layFill.begin(); CI != _layFill.end(); CI++)
      psf.defineFill( CI->first.c_str() , CI->second);
}

void  layprop::DrawProperties::hideLayer(unsigned layno, bool hide)
{
   // No error messages here, because of possible range use
   LayerSettings* ilayset = const_cast<LayerSettings*>(findLayerSettings(layno));
   if (NULL != ilayset)
      ilayset->_hidden = hide;
}

void  layprop::DrawProperties::lockLayer(unsigned layno, bool lock)
{
   // No error messages here, because of possible range use
   LayerSettings* ilayset = const_cast<LayerSettings*>(findLayerSettings(layno));
   if (NULL != ilayset)
      ilayset->_locked = lock;
}

void  layprop::DrawProperties::fillLayer(unsigned layno, bool fill)
{
   // No error messages here, because of possible range use
   LayerSettings* ilayset = const_cast<LayerSettings*>(findLayerSettings(layno));
   if (NULL != ilayset)
      ilayset->fillLayer(fill);
}

layprop::DrawProperties::~DrawProperties() {
	//clear all databases
	setState(layprop::DRC);
   for (LaySetList::const_iterator LSI = getCurSetList().begin(); LSI != getCurSetList().end(); LSI++)
      delete LSI->second;
	setState(layprop::DB);
	for (LaySetList::const_iterator LSI = getCurSetList().begin(); LSI != getCurSetList().end(); LSI++)
      delete LSI->second;

   for (ColorMap::iterator CMI = _layColors.begin(); CMI != _layColors.end(); CMI++)
      delete CMI->second;
   for (FillMap::iterator FMI = _layFill.begin(); FMI != _layFill.end(); FMI++)
      delete [] FMI->second;
   for (LineMap::iterator LMI = _lineSet.begin(); LMI != _lineSet.end(); LMI++)
      delete LMI->second;
//   if (NULL != _refStack) delete _refStack; -> deleted in EditObject
}

/*! Shall be called by the execute method of loadlaystatus TELL function.
 * Stores the current state of the defined layers in a _layStateHistory
 * WARNING! This function is only for undo purposes. Should not be used
 * to store/change/delete the layer state
 */
void layprop::DrawProperties::pushLayerStatus()
{
   _layStateHistory.push_front(LayStateList());
   LayStateList& clist = _layStateHistory.front();
   for (LaySetList::const_iterator CL = _laySetDb.begin(); CL != _laySetDb.end(); CL++)
   {
      clist.second.push_back(LayerState(CL->first, *(CL->second)));
   }
   clist.first = _curlay;
}

/*! Shall be called by the undo method of loadlaystatus TELL function.
 * Restores the loch/hide/fill state of the defined layers in a _laySetDb
 * WARNING! This function is only for undo purposes. Should not be used
 * to store/change/delete the layer state
 */
void layprop::DrawProperties::popLayerStatus()
{
   LayStateList& clist = _layStateHistory.front();
   for (std::list<LayerState>::const_iterator CL = clist.second.begin(); CL != clist.second.end(); CL++)
   {
      LaySetList::iterator clay;
      if (_laySetDb.end() != (clay = _laySetDb.find(CL->number())))
      {
         clay->second->_filled = CL->filled();
         TpdPost::layer_status(tui::BT_LAYER_FILL, CL->number(), CL->filled());
         clay->second->_hidden = CL->hidden();
         TpdPost::layer_status(tui::BT_LAYER_HIDE, CL->number(), CL->hidden());
         clay->second->_locked = CL->locked();
         TpdPost::layer_status(tui::BT_LAYER_LOCK, CL->number(), CL->locked());
      }
   }
   TpdPost::layer_default(clist.first, _curlay);
   _curlay = clist.first;
   _layStateHistory.pop_front();
}

/*!
 * Removes the oldest saved state in the _layStateHistory. Should be called
 * by undo_cleanup methods of the related tell functions.
 * WARNING! This function is only for undo purposes. Should not be used
 * to store/change/delete the layer state
 */
void layprop::DrawProperties::popBackLayerStatus()
{
   _layStateHistory.pop_back();
}

bool layprop::DrawProperties::saveLaysetStatus(const std::string& sname)
{
   LayStateList clist;
   bool status = true;
   for (LaySetList::const_iterator CL = _laySetDb.begin(); CL != _laySetDb.end(); CL++)
   {
      clist.second.push_back(LayerState(CL->first, *(CL->second)));
   }
   clist.first = _curlay;
   if (_layStateMap.end() != _layStateMap.find(sname)) status = false;
   _layStateMap[sname] = clist;
   return status;
}

bool layprop::DrawProperties::saveLaysetStatus(const std::string& sname, const WordSet& hidel,
      const WordSet& lockl, const WordSet& filll, unsigned alay)
{
   LayStateList clist;
   bool status = true;
   for (LaySetList::const_iterator CL = _laySetDb.begin(); CL != _laySetDb.end(); CL++)
   {
      bool hiden  = (hidel.end() != hidel.find(CL->first));
      bool locked = (lockl.end() != lockl.find(CL->first));
      bool filled = (filll.end() != filll.find(CL->first));
      clist.second.push_back(LayerState(CL->first, hiden, locked, filled));
   }
   clist.first = alay;
   if (_layStateMap.end() == _layStateMap.find(sname)) status = false;
   _layStateMap[sname] = clist;
   return status;
}

bool layprop::DrawProperties::loadLaysetStatus(const std::string& sname)
{
   if (_layStateMap.end() == _layStateMap.find(sname)) return false;
   LayStateList clist = _layStateMap[sname];
   for (std::list<LayerState>::const_iterator CL = clist.second.begin(); CL != clist.second.end(); CL++)
   {
      LaySetList::iterator clay;
      if (_laySetDb.end() != (clay = _laySetDb.find(CL->number())))
      {
         clay->second->_filled = CL->filled();
         TpdPost::layer_status(tui::BT_LAYER_FILL, CL->number(), CL->filled());
         clay->second->_hidden = CL->hidden();
         TpdPost::layer_status(tui::BT_LAYER_HIDE, CL->number(), CL->hidden());
         clay->second->_locked = CL->locked();
         TpdPost::layer_status(tui::BT_LAYER_LOCK, CL->number(), CL->locked());
      }
   }
   TpdPost::layer_default(clist.first, _curlay);
   _curlay = clist.first;
   return true;
}

bool layprop::DrawProperties::deleteLaysetStatus(const std::string& sname)
{
   if (_layStateMap.end() == _layStateMap.find(sname)) return false;
   _layStateMap.erase(sname);
   return true;
}

bool layprop::DrawProperties::getLaysetStatus(const std::string& sname, WordSet& hidel,
      WordSet& lockl, WordSet& filll, unsigned activel)
{
   if (_layStateMap.end() == _layStateMap.find(sname)) return false;
   LayStateList clist = _layStateMap[sname];
   for (std::list<LayerState>::const_iterator CL = clist.second.begin(); CL != clist.second.end(); CL++)
   {
      if (CL->hidden()) hidel.insert(hidel.begin(),CL->number());
      if (CL->locked()) lockl.insert(lockl.begin(),CL->number());
      if (CL->filled()) filll.insert(filll.begin(),CL->number());
   }
   activel = clist.first;
   return true;
}


