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

extern trend::TrendCenter*         TRENDC;

       
void trend::setShaderCtm(layprop::DrawProperties* drawprop, const TrxCellRef* refCell)
{
   drawprop->pushCtm(refCell->ctm() * drawprop->topCtm());
   float mtrxOrtho [16];
   drawprop->topCtm().oglForm(mtrxOrtho);
   TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
}
       
//=============================================================================
//
// class ToshaderTV
//
trend::ToshaderTV::ToshaderTV(TrxCellRef* const refCell, bool filled, bool reusable,
                              unsigned parray_offset, unsigned iarray_offset) :
   TenderTV(refCell, filled, reusable, parray_offset, iarray_offset)
{}

void trend::ToshaderTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   setShaderCtm(drawprop, _refCell);
   setAlpha(drawprop);

   // Activate the vertex buffers in the vertex shader ...
   DBGL_CALL(glEnableVertexAttribArray,TSHDR_LOC_VERTEX)
   // Set-up the offset in the binded Vertex buffer
#warning introduce a common function to calculate properly vertex array offsets for glVertexAttribPointer
   size_t koko = 2*sizeof(/*TPX*/TNDR_GLDATAT) * _point_array_offset;
//   assert(0==koko);
   /*printf("Offset in the vertex buffer: %d\n", koko)*/;
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, (GLvoid*)(koko))
   // ... and here we go ...
   drawTriQuads();
   TRENDC->setUniVarui(glslu_in_StippleEn, 0);
   drawLines();
   TRENDC->setUniVarui(glslu_in_StippleEn, 1);
   // Switch the vertex buffers OFF in the openGL engine ...
   DBGL_CALL(glDisableVertexAttribArray,TSHDR_LOC_VERTEX)
   // ... and finally restore the openGL translation matrix
   drawprop->popCtm();
}

void trend::ToshaderTV::drawTexts(layprop::DrawProperties* drawprop)
{
   setShaderCtm(drawprop, _refCell);
   setAlpha(drawprop);
   for (TrendStrings::const_iterator TSTR = _text_data.begin(); TSTR != _text_data.end(); TSTR++)
   {
      //  Things to remember...
      // Font has to be translated using its own matrix in which
      // tx/ty are forced to zero. 
      // The text binding point (i.e. tx/ty) is multiplied ALONE with the current
      // translation matrix, but NEVER with the font matrix.
      CTM ftm((*TSTR)->ctm());
      CTM ctm( ftm.a(), ftm.b(), ftm.c(), ftm.d(), 0,0);
      ctm.Scale((real)OPENGL_FONT_UNIT, (real)OPENGL_FONT_UNIT);
      ctm.Translate(TP(ftm.tx(), ftm.ty()));
      drawprop->pushCtm(ctm * drawprop->topCtm());
      float mtrxOrtho[16];
      drawprop->topCtm().oglForm(mtrxOrtho);
      TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
      (*TSTR)->draw(_filled, drawprop);
      drawprop->popCtm();
   }
   drawprop->popCtm();
}

void trend::ToshaderTV::drawTriQuads()
{
   if  (_vobjnum[OTcnvx] > 0)
   {// Draw convex polygons
      assert(_firstvx[OTcnvx]);
      assert(_sizesvx[OTcnvx]);
      DBGL_CALL(glMultiDrawArrays, GL_TRIANGLE_FAN, _firstvx[OTcnvx], _sizesvx[OTcnvx], _vobjnum[OTcnvx])
   }
   if  (_vobjnum[OTncvx] > 0)
   {// Draw non-convex polygons
      if (_iobjnum[ITtria] > 0)
      {
         assert(_sizesix[ITtria]);
         assert(_firstix[ITtria]);
         //glMultiDrawElements(GL_TRIANGLES     , _sizesix[ftrs], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftrs], _alobjix[ftrs]);
         for (unsigned i= 0; i < _iobjnum[ITtria]; i++)
         {
            DBGL_CALL(tpd_glDrawElements,GL_TRIANGLES, _sizesix[ITtria][i], GL_UNSIGNED_INT, _firstix[ITtria][i])
//            printf("DRAW TRIA: Offset: %d; Size: %d \n", _firstix[ITtria][i], _sizesix[ITtria][i]);
         }
      }
      if (_iobjnum[ITtstr] > 0)
      {
         assert(_sizesix[ITtstr]);
         assert(_firstix[ITtstr]);
         //glMultiDrawElements(GL_TRIANGLE_STRIP, _sizesix[ftss], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftss], _alobjix[ftss]);
         for (unsigned i= 0; i < _iobjnum[ITtstr]; i++)
         {
            DBGL_CALL(tpd_glDrawElements, GL_TRIANGLE_STRIP, _sizesix[ITtstr][i], GL_UNSIGNED_INT, _firstix[ITtstr][i])
//            printf("DRAW STRP: Offset: %d; Size: %d \n", _firstix[ITtstr][i], _sizesix[ITtstr][i]);
         }

      }
   }
}

void trend::ToshaderTV::drawLines()
{
   if  (_vobjnum[OTline] > 0)
   {// Draw the wire center lines
      assert(_firstvx[OTline]);
      assert(_sizesvx[OTline]);
      DBGL_CALL(glMultiDrawArrays, GL_LINE_STRIP, _firstvx[OTline], _sizesvx[OTline], _vobjnum[OTline])
   }
   if  (_vobjnum[OTcnvx] > 0)
   {// Draw convex polygons
      assert(_firstvx[OTcnvx]);
      assert(_sizesvx[OTcnvx]);
      DBGL_CALL(glMultiDrawArrays, GL_LINE_LOOP, _firstvx[OTcnvx], _sizesvx[OTcnvx], _vobjnum[OTcnvx])
   }
   if  (_vobjnum[OTncvx] > 0)
   {// Draw non-convex polygons
      assert(_firstvx[OTncvx]);
      assert(_sizesvx[OTncvx]);
      DBGL_CALL(glMultiDrawArrays, GL_LINE_LOOP, _firstvx[OTncvx], _sizesvx[OTncvx], _vobjnum[OTncvx])
   }
   if (_vobjnum[OTcntr] > 0)
   {// Draw the remaining non-filled shapes of any kind
      assert(_firstvx[OTcntr]);
      assert(_sizesvx[OTcntr]);
      DBGL_CALL(glMultiDrawArrays, GL_LINE_LOOP, _firstvx[OTcntr], _sizesvx[OTcntr], _vobjnum[OTcntr])
   }
}

void trend::ToshaderTV::setAlpha(layprop::DrawProperties* drawprop)
{
   layprop::tellRGB tellColor;
   if (drawprop->getAlpha(_refCell->alphaDepth() - 1, tellColor))
   {
      float* color = tellColor.getOGLcolor();
      TRENDC->setUniColor(color);
      delete[] color;
   }
}

//=============================================================================
//
// class ToshaderReTV
//
//void trend::ToshaderReTV::draw(layprop::DrawProperties* drawprop)
//{
//   TrxCellRef* sref_cell = _chunk->swapRefCells(_refCell);
//   _chunk->draw(drawprop);
//   _chunk->swapRefCells(sref_cell);
//}
//
//void trend::ToshaderReTV::drawTexts(layprop::DrawProperties* drawprop)
//{
//   TrxCellRef* sref_cell = _chunk->swapRefCells(_refCell);
//   _chunk->drawTexts(drawprop);
//   _chunk->swapRefCells(sref_cell);
//}


//=============================================================================
//
// class ToshaderLay
//
trend::ToshaderLay::ToshaderLay(bool rend3D):
   TenderLay           ( rend3D      )
{
}

/**
 * Create a new object of TenderTV type which will be referred to by _cslice
 * @param ctrans Current translation matrix of the new object
 * @param fill Whether to fill the drawing objects
 */
void trend::ToshaderLay::newSlice(TrxCellRef* const ctrans, bool fill, bool reusable, unsigned slctd_array_offset)
{
   assert( 0 == total_slctdx());
   _slctd_array_offset = slctd_array_offset;
   _stv_array_offset = 2 * _num_total_points;
   newSlice(ctrans, fill, reusable);
}

void trend::ToshaderLay::newSlice(TrxCellRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW ToshaderTV(ctrans, fill, reusable, _num_total_points, _num_total_indexs);
}

bool trend::ToshaderLay::chunkExists(TrxCellRef* const ctrans, bool filled)
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

void trend::ToshaderLay::drawSelected()
{
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(TNDR_GLDATAT)));

   // Activate the vertex buffers in the vertex shader ...
   DBGL_CALL(glEnableVertexAttribArray, TSHDR_LOC_VERTEX)
   // Set-up the offset in the binded Vertex buffer
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, (GLvoid*)(sizeof(TNDR_GLDATAT) * _stv_array_offset))

   if (_asobjix[STlstr] > 0)
   {
      assert(_sizslix[STlstr]);
      assert(_fstslix[STlstr]);
      //glMultiDrawElements(GL_LINE_STRIP, _sizslix[lstr], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lstr], _asobjix[lstr]);
      for (unsigned i= 0; i < _asobjix[STlstr]; i++)
         DBGL_CALL(tpd_glDrawElements, GL_LINE_STRIP, _sizslix[STlstr][i], GL_UNSIGNED_INT, _fstslix[STlstr][i])
   }
   if (_asobjix[STllps] > 0)
   {
      assert(_sizslix[STllps]);
      assert(_fstslix[STllps]);
         //glMultiDrawElements(GL_LINE_LOOP     , _sizslix[llps], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[llps], _alobjix[llps]);
      for (unsigned i= 0; i < _asobjix[STllps]; i++)
         DBGL_CALL(tpd_glDrawElements, GL_LINE_LOOP, _sizslix[STllps][i], GL_UNSIGNED_INT, _fstslix[STllps][i])
   }
   if (_asobjix[STlnes] > 0)
   {
      assert(_sizslix[STlnes]);
      assert(_fstslix[STlnes]);
         //glMultiDrawElements(GL_LINES  , _sizslix[lnes], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lnes], _alobjix[lnes]);
      for (unsigned i= 0; i < _asobjix[STlnes]; i++)
         DBGL_CALL(tpd_glDrawElements, GL_LINES, _sizslix[STlnes][i], GL_UNSIGNED_INT, _fstslix[STlnes][i])
   }
   DBGL_CALL(glDisableVertexAttribArray, TSHDR_LOC_VERTEX)
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0)
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
   // Bind the buffer
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * total_points() * sizeof(TNDR_GLDATAT)));

   DBGL_CALL(glEnableVertexAttribArray, TSHDR_LOC_VERTEX)
   // Set-up the offset in the binded Vertex buffer
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, nullptr)
   // ... and here we go ...
   if (0 < (_alvrtxs + _asindxs))
   {
      assert(_firstvx); assert(_sizesvx);
      DBGL_CALL(glMultiDrawArrays, GL_LINE_LOOP, _firstvx, _sizesvx, _alobjvx + _asobjix)
      if (0 < _asindxs)
      {
         assert(_fstslix); assert(_sizslix);
         setLine(drawprop, true);
         DBGL_CALL(glMultiDrawArrays, GL_LINE_LOOP, _fstslix, _sizslix, _asobjix)
         setLine(drawprop, false);
      }
   }
   DBGL_CALL(glDisableVertexAttribArray, TSHDR_LOC_VERTEX)
}

void trend::ToshaderRefLay::setLine(layprop::DrawProperties* drawprop, bool selected)
{
   layprop::LineSettings curLine;
   drawprop->getCurrentLine(curLine, selected);
   GLfloat clw = selected ? static_cast<GLfloat>(curLine.width()) : 1.0;
   TRENDC->setUniVarf(glslu_in_LWidth, clw);
   if (0xffff == curLine.pattern())
      TRENDC->setUniVarui(glslu_in_LStippleEn, 0);
   else
   {
      GLuint lPattern = curLine.pattern();
      TRENDC->setUniVarui(glslu_in_LStipple, lPattern);
      TRENDC->setUniVarui(glslu_in_PatScale, curLine.patscale());
      TRENDC->setUniVarui(glslu_in_LStippleEn, 1);
   }
}

trend::ToshaderRefLay::~ToshaderRefLay()
{
}

//=============================================================================
//
// class ToshaderMarks
//
void trend::ToshaderMarks::draw(layprop::DrawProperties* drawprop)
{
   // Bind the buffer
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * total_points() * sizeof(TNDR_GLDATAT)));

   DBGL_CALL(glEnableVertexAttribArray, TSHDR_LOC_VERTEX)
   // Set-up the offset in the binded Vertex buffer
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, nullptr)
   unsigned start = 0;
   unsigned size = static_cast<unsigned>(_refMarks.size());
   if (0 < size)
   {
      setStipple(drawprop->ref_mark_bmp());
      DBGL_CALL(glDrawArrays, GL_POINTS, start, size)
      start += size;
   }
   if (0 < (size = static_cast<unsigned>(_textMarks.size())) )
   {
      setStipple(drawprop->text_mark_bmp());
      DBGL_CALL(glDrawArrays, GL_POINTS, start, size)
      start += size;
   }
   if (0 < (size = static_cast<unsigned>(_arefMarks.size())) )
   {
      setStipple(drawprop->aref_mark_bmp());
      DBGL_CALL(glDrawArrays, GL_POINTS, start, size)
   }
   DBGL_CALL(glDisableVertexAttribArray, TSHDR_LOC_VERTEX)

}

void trend::ToshaderMarks::setStipple(const byte* markStipple)
{
   // the matrix above must be converted to something suitable by the shader
   // the size of the array below should be 32 of course. The AMD platform however
   // behaves in a weird way, so the simplest workaround is to expand the
   // array with one and to bind the first one to 0. The bug was reported here:
   // http://devgurus.amd.com/thread/160053
   // Note that the fragment shader is updated accordingly.
   GLuint shdrStipple [33];
   shdrStipple[0] = 0;
   for (unsigned i = 0; i < 16; i++)
      shdrStipple[i+1] = ((GLuint)(markStipple[2*i + 0]) << 8*1)
                       | ((GLuint)(markStipple[2*i + 1]) << 8*0);
   for (unsigned i = 16; i < 32; i++)
      shdrStipple[i+1] = 0;
   TRENDC->setUniStipple(shdrStipple);
}

//=============================================================================
//
// class Toshader
//
trend::Toshader::Toshader( layprop::DrawProperties* drawprop, real UU) :
    Tenderer             (drawprop, UU, false )
{
   _refLayer = DEBUG_NEW ToshaderRefLay();
   _marks    = DEBUG_NEW ToshaderMarks();
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
      _clayer = DEBUG_NEW ToshaderLay(_rend3D);
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
      _clayer = DEBUG_NEW ToshaderLay(_rend3D);
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
      _clayer = DEBUG_NEW ToshaderLay(false);
      _data.add(laydef, _clayer);
      _clayer->newSlice(_cellStack.top(), false, false, 0 /*_cslctd_array_offset*/);
   }
}

void trend::Toshader::grdDraw()
{
   TRENDC->setGlslProg(glslp_VF);
   TRENDC->setUniVarui(glslu_in_StippleEn, 0);
   _drawprop->initCtmStack();
   float mtrxOrtho [16];
   _drawprop->topCtm().oglForm(mtrxOrtho);
   TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);

   DBGL_CALL(glEnableVertexAttribArray,TSHDR_LOC_VERTEX)

   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _ogl_grd_buffer[0]);
   unsigned startP = 0;
   for (VGrids::const_iterator CG = _grid_props.begin(); CG != _grid_props.end(); CG++)
   {
      // color
      layprop::tellRGB theColor(_drawprop->getColor((*CG)->color()));
      float* oglColor = theColor.getOGLcolor();
      TRENDC->setUniColor(oglColor);
      delete[] oglColor;
      //draw
      DBGL_CALL(glVertexAttribPointer,TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, nullptr)
      DBGL_CALL(glDrawArrays,GL_POINTS, startP, (*CG)->asize())
      startP += (*CG)->asize();
   }
   DBGL_CALL(glDisableVertexAttribArray,TSHDR_LOC_VERTEX)
   // clean-up the buffers
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
}

void trend::Toshader::setLayColor(const LayerDef& layer)
{
   layprop::tellRGB tellColor;
   if (_drawprop->setCurrentColor(layer, tellColor))
   {
      float* oglColor = tellColor.getOGLcolor();
      TRENDC->setUniColor(oglColor);
      delete[] oglColor;
   }
}

void trend::Toshader::setStipple()
{
   const byte* tellStipple = _drawprop->getCurrentFill();
   if (NULL == tellStipple)
      TRENDC->setUniVarui(glslu_in_StippleEn, 0);
   else
   {
      // the matrix above must be converted to something suitable by the shader
      // the size of the array below should be 32 of course. The AMD platform however
      // behaves in a weird way, so the simplest workaround is to to expand the
      // array with one and to bind the first one to 0. The bug was reported here:
      // http://devgurus.amd.com/thread/160053
      // Note that the fragment shader is updated accordingly.
      GLuint shdrStipple [33];
      shdrStipple[0] = 0;
      for (unsigned i = 0; i < 32; i++)
         shdrStipple[i+1] = ((GLuint)(tellStipple[4*i + 0]) << 8*3)
                          | ((GLuint)(tellStipple[4*i + 1]) << 8*2)
                          | ((GLuint)(tellStipple[4*i + 2]) << 8*1)
                          | ((GLuint)(tellStipple[4*i + 3]) << 8*0);
      TRENDC->setUniStipple(shdrStipple);
      TRENDC->setUniVarui(glslu_in_StippleEn, 1);
   }
}

void trend::Toshader::setLine(bool selected)
{
   layprop::LineSettings curLine;
   _drawprop->getCurrentLine(curLine, selected);
   GLfloat clnw = static_cast<GLfloat>(curLine.width());
   if (selected)
      TRENDC->setUniVarf(glslu_in_LWidth, clnw);
   if (0xffff == curLine.pattern())
      TRENDC->setUniVarf(glslu_in_LStippleEn, 0);
   else
   {
      GLuint lPattern = curLine.pattern();
      TRENDC->setUniVarui(glslu_in_LStipple, lPattern);
      TRENDC->setUniVarui(glslu_in_PatScale, curLine.patscale());
      TRENDC->setUniVarui(glslu_in_LStippleEn, 1);
   }
}

void trend::Toshader::draw()
{
   _drawprop->initCtmStack();
   TRENDC->setGlslProg(glslp_VF);
   _drawprop->resetCurrentColor();
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      setLayColor(CLAY());
      setLine(false);
      setStipple();
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
      // draw texts
      if (0 != CLAY->total_strings())
      {
         TRENDC->bindFont();
         CLAY->drawTexts(_drawprop);
      }
   }
   // Lines with stipples
   TRENDC->setGlslProg(glslp_VG);
   _drawprop->resetCurrentColor();
   TRENDC->setUniVarui(glslu_in_StippleEn, 0);
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      if (0 != CLAY->total_slctdx())
      {// redraw selected contours only
         setLayColor(CLAY());
         setLine(true);
         DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _sbuffer)
         setShaderCtm(_drawprop, _activeCS);
         CLAY->drawSelected();
         _drawprop->popCtm();
         DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0)
      }
   }
   // draw reference boxes
   if (0 < _refLayer->total_points())
   {
      TRENDC->setGlslProg(glslp_VG);
      _drawprop->resetCurrentColor();
      setLayColor(REF_LAY_DEF);
      setLine(false);
      float mtrxOrtho [16];
      _drawprop->topCtm().oglForm(mtrxOrtho);
      TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
      _refLayer->draw(_drawprop);
      TRENDC->setUniVarui(glslu_in_LStippleEn, 0);
   }
   // draw reference marks
   if (0 < _marks->total_points())
   {
      TRENDC->setGlslProg(glslp_PS);
      _drawprop->resetCurrentColor(); // required after changing the renderer
      setLayColor(REF_LAY_DEF);
      TRENDC->setUniVarui(glslu_in_StippleEn , 0);
      TRENDC->setUniVarui(glslu_in_LStippleEn, 0);
      TRENDC->setUniVarui(glslu_in_MStippleEn, 1);
      float mtrxOrtho [16];
      _drawprop->topCtm().oglForm(mtrxOrtho);
      TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
      _marks->draw(_drawprop);
   }

   checkOGLError("draw");
   _drawprop->clearCtmStack();
}

void trend::Toshader::grcDraw()
{
   _drawprop->initCtmStack();
   TRENDC->setGlslProg(glslp_VF);
   _drawprop->resetCurrentColor();
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {// for every layer
      setLayColor(CLAY());
      setLine(false);
      setStipple();
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
   }
}

void trend::Toshader::rlrDraw()
{
   TRENDC->setGlslProg(glslp_VF);
   TRENDC->setUniVarui(glslu_in_StippleEn, 0);

   _drawprop->initCtmStack();
   float mtrxOrtho [16];
   _drawprop->topCtm().oglForm(mtrxOrtho);
   TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);

   DBGL_CALL(glEnableVertexAttribArray, TSHDR_LOC_VERTEX)

   // color
   float oglColor[4];
   oglColor[0] = 1.0f ;
   oglColor[1] = 1.0f ;
   oglColor[2] = 1.0f ;
   oglColor[3] = 0.7f ;
   TRENDC->setUniColor(oglColor);

   //draw
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _ogl_rlr_buffer[0])
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 2, TNDR_GLENUMT, GL_FALSE, 0, nullptr)
   DBGL_CALL(glDrawArrays, GL_LINES, 0, _num_ruler_ticks)

   DBGL_CALL(glDisableVertexAttribArray, TSHDR_LOC_VERTEX)
   // clean-up the buffers
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0)

   // Now draw texts
//   setShaderCtm(drawprop, _refCell);
   TRENDC->bindFont();
   for (TrendStrings::const_iterator TSTR = _rulerTexts.begin(); TSTR != _rulerTexts.end(); TSTR++)
   {
      _drawprop->pushCtm((*TSTR)->ctm() * _drawprop->topCtm());
      _drawprop->topCtm().oglForm(mtrxOrtho);
      TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
      (*TSTR)->draw(false, _drawprop);
      _drawprop->popCtm();
   }
//   drawprop->popCtm();

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
         _grcLayer = DEBUG_NEW ToshaderLay(false);
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
   //   delete _refLayer; //>>> deleted in the parent constructor
}

