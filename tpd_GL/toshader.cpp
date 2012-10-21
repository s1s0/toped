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
//        Created: Sun Oct 14 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL shader renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "toshader.h"
#include "viewprop.h"
#include "trend.h"

#define TSHDR_LOC_VERTEX 0
#define TSHDR_LOC_CTM    1
extern trend::FontLibrary*  fontLib;
       trend::GlslUniVarLoc glslUniVarLoc;
//=============================================================================
//
// class ToshaderTV
//
trend::ToshaderTV::ToshaderTV(TrendRef* const refCell, bool filled, bool reusable,
                                    unsigned parray_offset, unsigned iarray_offset) :
   TenderTV(refCell, filled, reusable, parray_offset, iarray_offset)
{}

void trend::ToshaderTV::setCtm(layprop::DrawProperties* drawprop)
{
   drawprop->pushCtm(_refCell->ctm() * drawprop->topCtm());
   real mtrxOrtho [16];
   drawprop->topCtm().oglForm(mtrxOrtho);
   glUniformMatrix4dv(glslUniVarLoc[glslu_in_CTM], 1, GL_FALSE, mtrxOrtho);
}

void trend::ToshaderTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   setCtm(drawprop);
   //TODO - colors! 
   //drawprop->adjustAlpha(_refCell->alphaDepth() - 1);
   // Activate the vertex buffers in the vartex shader ...
   glEnableVertexAttribArray(TSHDR_LOC_VERTEX);  // glEnableClientState(GL_VERTEX_ARRAY)
   // Set-up the offset in the binded Vertex buffer
   glVertexAttribPointer(TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, (GLvoid*)(sizeof(TNDR_GLDATAT) * _point_array_offset));
   // ... and here we go ...
   if  (_alobjvx[line] > 0)
   {// Draw the wire center lines
      assert(_firstvx[line]);
      assert(_sizesvx[line]);
      glMultiDrawArrays(GL_LINE_STRIP, _firstvx[line], _sizesvx[line], _alobjvx[line]);
   }
   if  (_alobjvx[cnvx] > 0)
   {// Draw convex polygons (TODO replace GL_QUADS to GL_POLY)
      assert(_firstvx[cnvx]);
      assert(_sizesvx[cnvx]);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx[cnvx], _sizesvx[cnvx], _alobjvx[cnvx]);
      glMultiDrawArrays(GL_QUADS, _firstvx[cnvx], _sizesvx[cnvx], _alobjvx[cnvx]);
   }
   if  (_alobjvx[ncvx] > 0)
   {// Draw non-convex polygons
      glEnableClientState(GL_INDEX_ARRAY);
      assert(_firstvx[ncvx]);
      assert(_sizesvx[ncvx]);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx[ncvx], _sizesvx[ncvx], _alobjvx[ncvx]);
      if (_alobjix[fqss] > 0)
      {
         assert(_sizesix[fqss]);
         assert(_firstix[fqss]);
         // The line below works on Windows, but doesn't work (hangs) on Linux with nVidia driver.
         // The suspect is (const GLvoid**)_firstix[fqss] but it's quite possible that it is a driver bug
         // Besides - everybody is saying that there is no speed benefit from this operation
         //glMultiDrawElements(GL_QUAD_STRIP    , _sizesix[fqss], GL_UNSIGNED_INT, (const GLvoid**)_firstix[fqss], _alobjix[fqss]);
         for (unsigned i= 0; i < _alobjix[fqss]; i++)
            glDrawElements(GL_QUAD_STRIP, _sizesix[fqss][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_firstix[fqss][i]));
      }
      if (_alobjix[ftrs] > 0)
      {
         assert(_sizesix[ftrs]);
         assert(_firstix[ftrs]);
         //glMultiDrawElements(GL_TRIANGLES     , _sizesix[ftrs], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftrs], _alobjix[ftrs]);
         for (unsigned i= 0; i < _alobjix[ftrs]; i++)
            glDrawElements(GL_TRIANGLES, _sizesix[ftrs][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_firstix[ftrs][i]));
      }
      if (_alobjix[ftfs] > 0)
      {
         assert(_sizesix[ftfs]);
         assert(_firstix[ftfs]);
         //glMultiDrawElements(GL_TRIANGLE_FAN  , _sizesix[ftfs], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftfs], _alobjix[ftfs]);
         for (unsigned i= 0; i < _alobjix[ftfs]; i++)
            glDrawElements(GL_TRIANGLE_FAN, _sizesix[ftfs][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_firstix[ftfs][i]));
      }
      if (_alobjix[ftss] > 0)
      {
         assert(_sizesix[ftss]);
         assert(_firstix[ftss]);
         //glMultiDrawElements(GL_TRIANGLE_STRIP, _sizesix[ftss], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftss], _alobjix[ftss]);
         for (unsigned i= 0; i < _alobjix[ftss]; i++)
            glDrawElements(GL_TRIANGLE_STRIP, _sizesix[ftss][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_firstix[ftss][i]));
      }
      glDisableClientState(GL_INDEX_ARRAY);
   }
   if (_alobjvx[cont] > 0)
   {// Draw the remaining non-filled shapes of any kind
      assert(_firstvx[cont]);
      assert(_sizesvx[cont]);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx[cont], _sizesvx[cont], _alobjvx[cont]);
   }
   // Switch the vertex buffers OFF in the openGL engine ...
   glDisableVertexAttribArray(TSHDR_LOC_VERTEX); //glDisableClientState(GL_VERTEX_ARRAY);
   // ... and finally restore the openGL translation matrix
   drawprop->popCtm();
}

//=============================================================================
//
// class ToshaderReTV
//
void trend::ToshaderReTV::draw(layprop::DrawProperties* drawprop)
{
   TrendRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->draw(drawprop);
   _chunk->swapRefCells(sref_cell);
}

void trend::ToshaderReTV::drawTexts(layprop::DrawProperties*)
{
   //TODO
}


//=============================================================================
//
// class ToshaderLay
//
trend::ToshaderLay::ToshaderLay():
   TenderLay           (             )
{
}

/**
 * Create a new object of TenderTV type which will be referred to by _cslice
 * @param ctrans Current translation matrix of the new object
 * @param fill Whether to fill the drawing objects
 */
void trend::ToshaderLay::newSlice(TrendRef* const ctrans, bool fill, bool reusable, unsigned slctd_array_offset)
{
   assert( 0 == total_slctdx());
   _slctd_array_offset = slctd_array_offset;
   _stv_array_offset = 2 * _num_total_points;
   newSlice(ctrans, fill, reusable);
}

void trend::ToshaderLay::newSlice(TrendRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW ToshaderTV(ctrans, fill, reusable, 2 * _num_total_points, _num_total_indexs);
}

bool trend::ToshaderLay::chunkExists(TrendRef* const ctrans, bool filled)
{
   ReusableTTVMap::iterator achunk;
   if (filled)
   {
      if (_reusableFData.end() == ( achunk =_reusableFData.find(ctrans->name()) ) )
         return false;
   }
   else
   {
      if (_reusableCData.end() == ( achunk =_reusableCData.find(ctrans->name()) ) )
         return false;
   }
   _reLayData.push_back(DEBUG_NEW ToshaderReTV(achunk->second, ctrans));
   return true;
}

void trend::ToshaderLay::draw(layprop::DrawProperties* drawprop)
{
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   // Check the state of the buffer
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(TNDR_GLDATAT)));
   if (0 != _ibuffer)
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
      glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
      assert(bufferSize == (GLint)(_num_total_indexs * sizeof(unsigned)));
   }
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      (*TLAY)->draw(drawprop);
   }
   for (TrendReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
   {
      (*TLAY)->draw(drawprop);
   }

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   if (0 != _ibuffer)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void trend::ToshaderLay::drawSelected()
{
   //TODO
}

void trend::ToshaderLay::drawTexts(layprop::DrawProperties* drawprop)
{
   //TODO
}

trend::ToshaderLay::~ToshaderLay()
{
}

//=============================================================================
//
// class TenderRefLay
//
trend::ToshaderRefLay::ToshaderRefLay() :
   TenderRefLay    (      )
{
}


void trend::ToshaderRefLay::draw(layprop::DrawProperties* drawprop)
{
   //TODO
}

trend::ToshaderRefLay::~ToshaderRefLay()
{
}



//=============================================================================
//
// class Toshader
//
trend::Toshader::Toshader( layprop::DrawProperties* drawprop, real UU) :
    Tenderer             (drawprop, UU )
{
   _refLayer = DEBUG_NEW ToshaderRefLay();// FIXME! this object is creted twice - here and in the parent constructor
}

bool trend::Toshader::chunkExists(const LayerDef& laydef, bool has_selected)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with REF_LAY by accident
   assert(REF_LAY_DEF != laydef);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(laydef))
   {
      _clayer = _data[laydef];
      if (_clayer->chunkExists(_cellStack.top(), _drawprop->layerFilled(laydef) ) ) return true;
   }
   else
   {
      _clayer = DEBUG_NEW ToshaderLay();
      _data.add(laydef, _clayer);
   }
   if (has_selected)
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true, _cslctd_array_offset);
   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true);
   return false;
}

void trend::Toshader::setLayer(const LayerDef& laydef, bool has_selected)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with REF_LAY by accident
   assert(REF_LAY_DEF != laydef);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(laydef))
   {
      _clayer = _data[laydef];
   }
   else
   {
      _clayer = DEBUG_NEW ToshaderLay();
      _data.add(laydef, _clayer);
   }
   if (has_selected)
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false, _cslctd_array_offset);
   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false);
}

void trend::Toshader::setHvrLayer(const LayerDef& laydef)
{
   if (REF_LAY_DEF != laydef)
   {
      _clayer = DEBUG_NEW ToshaderLay();
      _data.add(laydef, _clayer);
      _clayer->newSlice(_cellStack.top(), false, false, 0 /*_cslctd_array_offset*/);
   }
}

void trend::Toshader::grid(const real step, const std::string color)
{
// TODO
}

void trend::Toshader::zeroCross()
{
   //TODO
}

void trend::Toshader::setColor(const LayerDef& layer)
{
   layprop::tellRGB tellColor;
   if (_drawprop->setCurrentColor(layer, tellColor))
   {
      float oglColor[4];
      oglColor[0] = (float)tellColor.red()   / 255.0f ;
      oglColor[1] = (float)tellColor.green() / 255.0f ;
      oglColor[2] = (float)tellColor.blue()  / 255.0f ;
      oglColor[3] = (float)tellColor.alpha() / 255.0f ;
      glUniform3fv(glslUniVarLoc[glslu_in_Color], 1, oglColor);
      glUniform1f(glslUniVarLoc[glslu_in_Alpha], oglColor[3]);
   }
}

void trend::Toshader::draw()
{
   _drawprop->initCtmStack();
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      setColor(CLAY());
      //TODO - all the commented code below should be valid

      //_drawprop->setCurrentFill(true); // force fill (ignore block_fill state)
      //_drawprop->setLineProps(false);
      //if (0 != CLAY->total_slctdx())
      //{// redraw selected contours only
      //   _drawprop->setLineProps(true);
      //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _sbuffer);
      //   glPushMatrix();
      //   glMultMatrixd(_activeCS->translation());
      //   CLAY->drawSelected();
      //   glPopMatrix();
      //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      //   _drawprop->setLineProps(false);
      //}
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
      // draw texts
      //TODO - all the commented code below should be valid
      //if (0 != CLAY->total_strings())
      //{
      //   fontLib->bindFont();
      //   CLAY->drawTexts(_drawprop);
      //}
   }
   // draw reference boxes
   //TODO - all the commented code below should be valid
   //if (0 < _refLayer->total_points())   _refLayer->draw(_drawprop);
   checkOGLError("draw");
   _drawprop->clearCtmStack();
}

void trend::Toshader::grcDraw()
{
   //TODO
}

void trend::Toshader::setGrcLayer(bool setEData, const LayerDef& laydef)
{
   if (setEData)
   {
      assert(_grcLayer == NULL);
      if (_grcData.end() != _grcData.find(laydef))
      {
         _grcLayer = _grcData[laydef];
      }
      else
      {
         _grcLayer = DEBUG_NEW ToshaderLay();
         _grcData.add(laydef, _grcLayer);
      }
      _grcLayer->newSlice(_cellStack.top(), false, false);
   }
   else
   {
      assert(_grcLayer != NULL);
      // post process the current layer
      _grcLayer->ppSlice();
      _grcLayer = NULL;
//      _cslctd_array_offset += _elayer->total_slctdx();
   }
}

trend::Toshader::~Toshader()
{
//   delete _refLayer;// FIXME! this object is creted twice - here and in the parent constructor
}

