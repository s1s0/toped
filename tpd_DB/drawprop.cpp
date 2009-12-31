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

layprop::FontLibrary* fontLib = NULL;
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
layprop::TGlfFont::TGlfFont(std::string filename) : _pitch(0.1f), _spaceWidth(0.5f)
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
      fread(_fname, 96, 1, ffile);
      _fname[96] = 0x0;
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

void layprop::TGlfFont::collect(GLuint pbuf, GLuint ibuf)
{
   _pbuffer = pbuf;
   _ibuffer = ibuf;
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

   //... and collect them deleting meanwhile the themporary objects
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
   _status = 0;
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
}

//=============================================================================
//
//
//
layprop::FontLibrary::FontLibrary(std::string fontfile, bool fti) : _fti(fti)
{
   _ogl_buffers = NULL;
   _num_ogl_buffers = 0;
   if (_fti)
   {
      //@TODO - list of fonts here!
      // Parse the font library
      _font = DEBUG_NEW TGlfFont(fontfile);
      _num_ogl_buffers += 2;
      // Create the VBO
      _ogl_buffers = DEBUG_NEW GLuint [_num_ogl_buffers];
      glGenBuffers(_num_ogl_buffers, _ogl_buffers);
      _font->collect(_ogl_buffers[0], _ogl_buffers[1]);
   }
   else
   {
      glfInit();
      if ( -1 != glfLoadFont(fontfile.c_str()) )
      {
         std::ostringstream ost1;
         ost1<<"Error loading font file \"" << fontfile << "\". All text objects will not be properly processed";
         tell_log(console::MT_ERROR,ost1.str());
      }
   }
}

void layprop::FontLibrary::getStringBounds(const std::string* text, DBbox* overlap)
{
   if (_fti)
   {
      assert(NULL != _font); // make sure that fonts are initialised
      _font->getStringBounds(text, overlap);
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
     _font->drawString(text, fill);
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
      _font->drawString(&text, false);
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
      _font->drawString(&text, true);
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
   if (NULL != _font)
      return _font->bindBuffers();
   else
      return false;
}

void  layprop::FontLibrary::unbindFont()
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

layprop::FontLibrary::~FontLibrary()
{
   if (_fti)
   {
      delete (_font);
      glDeleteBuffers(_num_ogl_buffers, _ogl_buffers);
      delete [] _ogl_buffers;
      _ogl_buffers = NULL;
   }
   else
      glfClose();
}

//=============================================================================
layprop::DrawProperties::DrawProperties() : _clipRegion(0,0)
{
   _blockFill = false;
   _currentOp = console::op_none;
   _drawingLayer = 0;
   _cellMarksHidden = true;
   _textMarksHidden = true;
   _cellBoxHidden = true;
   _textBoxHidden = true;
   _refStack = NULL;
   _renderType = false;
   _propertyState = DB;
   _adjustTextOrientation = false;
}

void layprop::DrawProperties::loadLayoutFonts(std::string fontfile, bool vbo)
{
   _renderType = vbo;
   fontLib = DEBUG_NEW FontLibrary(fontfile, vbo);
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

void layprop::DrawProperties::setCurrentColor(unsigned layno)
{
   _drawingLayer = layno;
   const layprop::tellRGB& theColor = getColor(_drawingLayer);
   glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
}

void layprop::DrawProperties::adjustAlpha(word factor)
{
   return;
   //@TODO! - A tell option(function or variable) to adjust the constant (30 below)
   // user must know what's going on, otherwise - the rendering result might be confusing
   // Having done that, the method can be enabled
/*   if (_layset.end() != _layset.find(_drawingLayer))
   {
      if (_layColors.end() != _layColors.find(_layset[_drawingLayer]->color()))
      {
         tellRGB* gcol = _layColors[_layset[_drawingLayer]->color()];
         if (gcol)
         {
            byte alpha = gcol->alpha();
            if (0 < factor)
               alpha = ((factor * 50) > gcol->alpha()) ? 0 : gcol->alpha() - (factor * 50);
            glColor4ub(gcol->red(), gcol->green(), gcol->blue(), alpha);
         }
      }
   }*/
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

bool layprop::DrawProperties::isFilled(unsigned layno) const
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

void layprop::DrawProperties::drawTextBoundary(const pointlist& ptlist)
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

void layprop::DrawProperties::drawCellBoundary(const pointlist& ptlist)
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

void  layprop::DrawProperties::blockFill(laydata::CellRefStack* refstack)
{
   _blockFill = true;
   _refStack = refstack;
}

void  layprop::DrawProperties::unblockFill()
{
   _blockFill = false;
   _refStack = NULL;
}

void  layprop::DrawProperties::pushRef(const laydata::TdtCellRef* cref)
{
   assert(cref);
   if (_refStack)
   {
      if (_refStack->empty()) _blockFill = true;
      _refStack->push_front(cref);
   }
}

byte layprop::DrawProperties::popRef(const laydata::TdtCellRef* cref)
{
   assert(cref);
   if (_refStack && !_refStack->empty())
   {
      if (_refStack->front() == cref)
      {
         _refStack->pop_front();
         if (_refStack->empty())
         {
            _blockFill = false;
            return 2;
         }
         else return 1;
      }
   }
   return 0;
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
//   if (NULL != _refStack) d elete _refStack; -> deleted in EditObject
   delete fontLib;
}



