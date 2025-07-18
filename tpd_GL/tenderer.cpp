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
//    Description: OpenGL renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tenderer.h"
#include "viewprop.h"
#include "trend.h"

extern trend::TrendCenter*        TRENDC;

//=============================================================================
//
// class TenderTV
//
trend::TenderTV::TenderTV(TrxCellRef* const refCell, bool filled, bool reusable,
                   unsigned parray_offset, unsigned iarray_offset) :
   TrendTV(refCell, filled, reusable, parray_offset, iarray_offset),
   _point_array_offset  ( parray_offset   ),
   _index_array_offset  ( iarray_offset   )
{
   for (int i = ITtria; i < IDX_TYPES; i++)
   {
      _sizesix[i] = NULL;
      _firstix[i] = NULL;
   }
   for (int i = OTcntr; i < OBJ_TYPES; i++)
   {
      _sizesvx[i] = NULL;
      _firstvx[i] = NULL;
   }
}

void trend::TenderTV::collectIndexs(unsigned int* index_array, const TessellChain* tdata, unsigned* size_index,
                             unsigned* index_offset, unsigned cpoint_index)
{
   for (TessellChain::const_iterator TCH = tdata->begin(); TCH != tdata->end(); TCH++)
   {
      switch (TCH->type())
      {
         case GL_TRIANGLES      :
         {
            assert(_sizesix[ITtria]);
            _firstix[ITtria][size_index[ITtria]  ] = sizeof(unsigned) * index_offset[ITtria];
            _sizesix[ITtria][size_index[ITtria]++] = TCH->size();
            for (unsigned i = 0; i < TCH->size(); i++)
               index_array[index_offset[ITtria]++] = TCH->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_STRIP :
         {
            assert(_sizesix[ITtstr]);
            _firstix[ITtstr][size_index[ITtstr]  ] = sizeof(unsigned) * index_offset[ITtstr];
            _sizesix[ITtstr][size_index[ITtstr]++] = TCH->size();
            for (unsigned i = 0; i < TCH->size(); i++)
               index_array[index_offset[ITtstr]++] = TCH->index_seq()[i] + cpoint_index;
            break;
         }
         default: assert(0);break;
      }
   }
}

void trend::TenderTV::collect(TNDR_GLDATAT* point_array, unsigned int* index_array)
{
   unsigned line_arr_size = 2 * _vrtxnum[OTline]; // x2 to accomodate X & Y coordinate
   unsigned fqus_arr_size = 2 * _vrtxnum[OTcnvx]; // x2 to accomodate X & Y coordinate
   unsigned cont_arr_size = 2 * _vrtxnum[OTcntr]; // x2 to accomodate X & Y coordinate
   unsigned poly_arr_size = 2 * _vrtxnum[OTncvx]; // x2 to accomodate X & Y coordinate
   // initialise the indexing
   unsigned pntindx = 0;

   if  (_vobjnum[OTline] > 0)
   {// collect all central lines of the wires
      unsigned  szindx  = 0;
      _firstvx[OTline] = DEBUG_NEW int[_vobjnum[OTline]];
      _sizesvx[OTline] = DEBUG_NEW int[_vobjnum[OTline]];
      for (SliceWires::const_iterator CSH = _line_data.begin(); CSH != _line_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[OTline][szindx  ] = pntindx/2;
         _sizesvx[OTline][szindx++] = (*CSH)->lDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      assert(pntindx == line_arr_size);
      assert(szindx  == _vobjnum[OTline]);
   }

   if  (_vobjnum[OTcnvx] > 0)
   {// collect all convex polygons
      unsigned  szindx  = 0;
      _firstvx[OTcnvx] = DEBUG_NEW int[_vobjnum[OTcnvx]];
      _sizesvx[OTcnvx] = DEBUG_NEW int[_vobjnum[OTcnvx]];
      for (SliceObjects::const_iterator CSH = _cnvx_data.begin(); CSH != _cnvx_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[OTcnvx][szindx  ] = pntindx/2;
         _sizesvx[OTcnvx][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      assert(pntindx == line_arr_size + fqus_arr_size);
      assert(szindx  == _vobjnum[OTcnvx]);
   }

   if  (_vobjnum[OTncvx] > 0)
   {// collect all non-convex polygons
      unsigned  szindx  = 0;
      _firstvx[OTncvx] = DEBUG_NEW int[_vobjnum[OTncvx]];
      _sizesvx[OTncvx] = DEBUG_NEW int[_vobjnum[OTncvx]];
      if (NULL != index_array)
      {
         assert(_iobjnum[ITtria] + _iobjnum[ITtstr]);
         if (0 < _iobjnum[ITtria])
         {
            _sizesix[ITtria] = DEBUG_NEW GLsizei[_iobjnum[ITtria]];
            _firstix[ITtria] = DEBUG_NEW GLuint[_iobjnum[ITtria]];
         }
         if (0 < _iobjnum[ITtstr])
         {
            _sizesix[ITtstr] = DEBUG_NEW GLsizei[_iobjnum[ITtstr]];
            _firstix[ITtstr] = DEBUG_NEW GLuint[_iobjnum[ITtstr]];
         }
      }
      unsigned size_index[4];
      unsigned index_offset[4];
      size_index[ITtria] = size_index[ITtstr] = 0u;
      index_offset[ITtria] = _index_array_offset;
      index_offset[ITtstr] = index_offset[ITtria] + _indxnum[ITtria];
      for (SlicePolygons::const_iterator CSH = _ncvx_data.begin(); CSH != _ncvx_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)

         if (NULL != (*CSH)->tdata())
            collectIndexs( index_array     ,
                          (*CSH)->tdata()  ,
                           size_index      ,
                           index_offset    ,
                           pntindx/2
                         );
         _firstvx[OTncvx][szindx  ] = pntindx/2;
         _sizesvx[OTncvx][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);

      }
      assert(size_index[ITtria] == _iobjnum[ITtria]);
      assert(size_index[ITtstr] == _iobjnum[ITtstr]);
      assert(index_offset[ITtria] == (_index_array_offset + _indxnum[ITtria]));
      assert(index_offset[ITtstr] == (_index_array_offset + _indxnum[ITtria] + _indxnum[ITtstr] ));
      assert(pntindx == line_arr_size + fqus_arr_size + poly_arr_size);
      assert(szindx  == _vobjnum[OTncvx]);
   }

   if  (_vobjnum[OTcntr] > 0)
   {// collect all contours (only non-filled objects here)
      unsigned  szindx  = 0;
      _firstvx[OTcntr] = DEBUG_NEW int[_vobjnum[OTcntr]];
      _sizesvx[OTcntr] = DEBUG_NEW int[_vobjnum[OTcntr]];
      for (SliceObjects::const_iterator CSH = _cont_data.begin(); CSH != _cont_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[OTcntr][szindx  ] = pntindx/2;
         _sizesvx[OTcntr][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      //... and text overlapping boxes
      for (RefTxtList::const_iterator CSH = _txto_data.begin(); CSH != _txto_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[OTcntr][szindx  ] = pntindx/2;
         _sizesvx[OTcntr][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      assert(pntindx == line_arr_size + fqus_arr_size + cont_arr_size + poly_arr_size);
      assert(szindx  == _vobjnum[OTcntr] );
   }
}

void trend::TenderTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   setAlpha(drawprop);
   // Switch the vertex buffers ON in the openGL engine ...
   glEnableClientState(GL_VERTEX_ARRAY);
   // Set-up the offset in the binded Vertex buffer
   glVertexPointer(2, TNDR_GLENUMT, 0, (GLvoid*)(sizeof(TNDR_GLDATAT) * _point_array_offset));
   // ... and here we go ...
   if  (_vobjnum[OTline] > 0)
   {// Draw the wire center lines
      assert(_firstvx[OTline]);
      assert(_sizesvx[OTline]);
      glMultiDrawArrays(GL_LINE_STRIP, _firstvx[OTline], _sizesvx[OTline], _vobjnum[OTline]);
   }
   if  (_vobjnum[OTcnvx] > 0)
   {// Draw convex polygons
      assert(_firstvx[OTcnvx]);
      assert(_sizesvx[OTcnvx]);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx[OTcnvx], _sizesvx[OTcnvx], _vobjnum[OTcnvx]);
      glMultiDrawArrays(GL_QUADS, _firstvx[OTcnvx], _sizesvx[OTcnvx], _vobjnum[OTcnvx]);
   }
   if  (_vobjnum[OTncvx] > 0)
   {// Draw non-convex polygons
      glEnableClientState(GL_INDEX_ARRAY);
      assert(_firstvx[OTncvx]);
      assert(_sizesvx[OTncvx]);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx[OTncvx], _sizesvx[OTncvx], _vobjnum[OTncvx]);
      if (_iobjnum[ITtria] > 0)
      {
         assert(_sizesix[ITtria]);
         assert(_firstix[ITtria]);
         //glMultiDrawElements(GL_TRIANGLES     , _sizesix[ftrs], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftrs], _alobjix[ftrs]);
         for (unsigned i= 0; i < _iobjnum[ITtria]; i++)
            DBGL_CALL(tpd_glDrawElements, GL_TRIANGLES, _sizesix[ITtria][i], GL_UNSIGNED_INT, _firstix[ITtria][i])
      }
      if (_iobjnum[ITtstr] > 0)
      {
         assert(_sizesix[ITtstr]);
         assert(_firstix[ITtstr]);
         //glMultiDrawElements(GL_TRIANGLE_STRIP, _sizesix[ftss], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftss], _alobjix[ftss]);
         for (unsigned i= 0; i < _iobjnum[ITtstr]; i++)
            DBGL_CALL(tpd_glDrawElements, GL_TRIANGLE_STRIP, _sizesix[ITtstr][i], GL_UNSIGNED_INT, _firstix[ITtstr][i])
      }
      glDisableClientState(GL_INDEX_ARRAY);
   }
   if (_vobjnum[OTcntr] > 0)
   {// Draw the remaining non-filled shapes of any kind
      assert(_firstvx[OTcntr]);
      assert(_sizesvx[OTcntr]);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx[OTcntr], _sizesvx[OTcntr], _vobjnum[OTcntr]);
   }
   // Switch the vertex buffers OFF in the openGL engine ...
   glDisableClientState(GL_VERTEX_ARRAY);
   // ... and finally restore the openGL translation matrix
   glPopMatrix();
}

void trend::TenderTV::drawTexts(layprop::DrawProperties* drawprop)
{
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   setAlpha(drawprop);
   for (TrendStrings::const_iterator TSTR = _text_data.begin(); TSTR != _text_data.end(); TSTR++)
   {
      real ftm[16];
      (*TSTR)->ctm().oglForm(ftm);
      glPushMatrix();
      glMultMatrixd(ftm);
      glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
      (*TSTR)->draw(_filled, drawprop);
      glPopMatrix();
   }
   glPopMatrix();
}

trend::TenderTV::~TenderTV()
{
   if (NULL != _sizesvx[OTcntr]) delete [] _sizesvx[OTcntr];
   if (NULL != _sizesvx[OTline]) delete [] _sizesvx[OTline];
   if (NULL != _sizesvx[OTcnvx]) delete [] _sizesvx[OTcnvx];
   if (NULL != _sizesvx[OTncvx]) delete [] _sizesvx[OTncvx];

   if (NULL != _sizesix[ITtria]) delete [] _sizesix[ITtria];
   if (NULL != _sizesix[ITtstr]) delete [] _sizesix[ITtstr];

   if (NULL != _firstvx[OTcntr]) delete [] _firstvx[OTcntr];
   if (NULL != _firstvx[OTline]) delete [] _firstvx[OTline];
   if (NULL != _firstvx[OTcnvx]) delete [] _firstvx[OTcnvx];
   if (NULL != _firstvx[OTncvx]) delete [] _firstvx[OTncvx];

   if (NULL != _firstix[ITtria]) delete [] _firstix[ITtria];
   if (NULL != _firstix[ITtstr]) delete [] _firstix[ITtstr];
}

//=============================================================================
//
// class TenderReTV
//
void trend::TenderReTV::draw(layprop::DrawProperties* drawprop)
{
   TrxCellRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->draw(drawprop);
   _chunk->swapRefCells(sref_cell);
}

void trend::TenderReTV::drawTexts(layprop::DrawProperties* drawprop)
{
   TrxCellRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->drawTexts(drawprop);
   _chunk->swapRefCells(sref_cell);
}

//=============================================================================
//
// class TenderLay
//
trend::TenderLay::TenderLay(bool rend3D):
   TrendLay              ( rend3D      ),
   _pbuffer              (          0u ),
   _ibuffer              (          0u ),
   _stv_array_offset     (          0u ),
   _slctd_array_offset   (          0u )
{
   for (int i = STlstr; i < SLCT_TYPES; i++)
   {
      _sizslix[i] = NULL;
      _fstslix[i] = NULL;
   }
}

/**
 * Create a new object of TenderTV type which will be referred to by _cslice
 * @param ctrans Current translation matrix of the new object
 * @param fill Whether to fill the drawing objects
 */
void trend::TenderLay::newSlice(TrxCellRef* const ctrans, bool fill, bool reusable, unsigned slctd_array_offset)
{
   assert( 0 == total_slctdx());
   _slctd_array_offset = slctd_array_offset;
   _stv_array_offset = 2 * _num_total_points;
   newSlice(ctrans, fill, reusable);
}

void trend::TenderLay::newSlice(TrxCellRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW TenderTV(ctrans, fill, reusable, 2 * _num_total_points, _num_total_indexs);
}

bool trend::TenderLay::chunkExists(TrxCellRef* const ctrans, bool filled)
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
   _reLayData.push_back(DEBUG_NEW TenderReTV(achunk->second, ctrans));
   return true;
}

void trend::TenderLay::collect(bool /*fill*/, GLuint pbuf, GLuint ibuf)
{
   TNDR_GLDATAT* cpoint_array = NULL;
   unsigned int* cindex_array = NULL;
   _pbuffer = pbuf;
   _ibuffer = ibuf;
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _pbuffer)
   DBGL_CALL(glBufferData,GL_ARRAY_BUFFER                   ,
                2 * _num_total_points * sizeof(TNDR_GLDATAT),
                nullptr                                     ,
                GL_DYNAMIC_DRAW                              )
   cpoint_array = (TNDR_GLDATAT*)DBGL_CALL(glMapBuffer,GL_ARRAY_BUFFER, GL_WRITE_ONLY);
   if (0 != _ibuffer)
   {
      DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, _ibuffer)
      DBGL_CALL(glBufferData,GL_ELEMENT_ARRAY_BUFFER    ,
                   _num_total_indexs * sizeof(unsigned) ,
                   nullptr                              ,
                   GL_DYNAMIC_DRAW                    )
      cindex_array = (unsigned int*)DBGL_CALL(glMapBuffer,GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
   }
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      (*TLAY)->collect(cpoint_array, cindex_array);
   // Unmap the buffers
//   trend::dumpOGLArrayFloat(cpoint_array, _num_total_points );
   DBGL_CALL(glUnmapBuffer,GL_ARRAY_BUFFER)
   if (0 != _ibuffer)
   {
//      trend::dumpOGLArrayUint(cindex_array, _num_total_indexs);
      DBGL_CALL(glUnmapBuffer,GL_ELEMENT_ARRAY_BUFFER)
//      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }
}

void trend::TenderLay::collectSelected(unsigned int* slctd_array)
{
   unsigned      slct_arr_size = _asindxs[STlstr] + _asindxs[STllps] + _asindxs[STlnes];
   if (0 == slct_arr_size) return;

   // initialise the indexing arrays of selected objects
   if (0 < _asobjix[STlstr])
   {
      _sizslix[STlstr] = DEBUG_NEW GLsizei[_asobjix[STlstr]];
      _fstslix[STlstr] = DEBUG_NEW GLuint[_asobjix[STlstr]];
   }
   if (0 < _asobjix[STllps])
   {
      _sizslix[STllps] = DEBUG_NEW GLsizei[_asobjix[STllps]];
      _fstslix[STllps] = DEBUG_NEW GLuint[_asobjix[STllps]];
   }
   if (0 < _asobjix[STlnes])
   {
      _sizslix[STlnes] = DEBUG_NEW GLsizei[_asobjix[STlnes]];
      _fstslix[STlnes] = DEBUG_NEW GLuint[_asobjix[STlnes]];
   }
   unsigned size_sindex[3];
   unsigned index_soffset[3];
   size_sindex[STlstr] = size_sindex[STllps] = size_sindex[STlnes] = 0u;
   index_soffset[STlstr] = _slctd_array_offset;
   index_soffset[STllps] = index_soffset[STlstr] + _asindxs[STlstr];
   index_soffset[STlnes] = index_soffset[STllps] + _asindxs[STllps];


   for (SliceSelected::const_iterator SSL = _slct_data.begin(); SSL != _slct_data.end(); SSL++)
   {
      TrxSelected* cchunk = *SSL;
      switch (cchunk->type())
      {
         case STlstr : // LINES
         {
            assert(_sizslix[STlstr]);
            _fstslix[STlstr][size_sindex[STlstr]  ] = sizeof(unsigned) * index_soffset[STlstr];
            _sizslix[STlstr][size_sindex[STlstr]++] = cchunk->sDataCopy(slctd_array, index_soffset[STlstr]);
            break;
         }
         case STllps      : // LINE_LOOP
         {
            assert(_sizslix[STllps]);
            _fstslix[STllps][size_sindex[STllps]  ] = sizeof(unsigned) * index_soffset[STllps];
            _sizslix[STllps][size_sindex[STllps]++] = cchunk->sDataCopy(slctd_array, index_soffset[STllps]);
            break;
         }
         case STlnes   : // LINE_STRIP
         {
            assert(_sizslix[STlnes]);
            _fstslix[STlnes][size_sindex[STlnes]  ] = sizeof(unsigned) * index_soffset[STlnes];
            _sizslix[STlnes][size_sindex[STlnes]++] = cchunk->sDataCopy(slctd_array, index_soffset[STlnes]);
            break;
         }
         default: assert(false);break;
      }
   }
}

void trend::TenderLay::draw(layprop::DrawProperties* drawprop)
{
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(TNDR_GLDATAT)));
   if (0 != _ibuffer)
   {
      DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _ibuffer)
      DBGL_CALL(glGetBufferParameteriv, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
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

   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   if (0 != _ibuffer)
      DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
}

void trend::TenderLay::drawSelected()
{
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(TNDR_GLDATAT)));

   DBGL_CALL(glEnableClientState, GL_VERTEX_ARRAY)
   DBGL_CALL(glEnableClientState, GL_INDEX_ARRAY)
   DBGL_CALL(glVertexPointer, 2, TNDR_GLENUMT, 0, (GLvoid*)(sizeof(TNDR_GLDATAT) * _stv_array_offset))

   if (_asobjix[STlstr] > 0)
   {
      assert(_sizslix[STlstr]);
      assert(_fstslix[STlstr]);
      //glMultiDrawElements(GL_LINE_STRIP, _sizslix[lstr], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lstr], _asobjix[lstr]);
      for (unsigned i= 0; i < _asobjix[STlstr]; i++)
         DBGL_CALL(tpd_glDrawElements,GL_LINE_STRIP, _sizslix[STlstr][i], GL_UNSIGNED_INT, _fstslix[STlstr][i])
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
         DBGL_CALL(tpd_glDrawElements,GL_LINES, _sizslix[STlnes][i], GL_UNSIGNED_INT, _fstslix[STlnes][i])
   }
   DBGL_CALL(glDisableClientState, GL_INDEX_ARRAY)
   DBGL_CALL(glDisableClientState, GL_VERTEX_ARRAY)


   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
}

void trend::TenderLay::drawTexts(layprop::DrawProperties* drawprop)
{
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      (*TLAY)->drawTexts(drawprop);
   }
   for (TrendReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
   {
      (*TLAY)->drawTexts(drawprop);
   }
}

trend::TenderLay::~TenderLay()
{
   if (NULL != _sizslix[STlstr]) delete [] _sizslix[STlstr];
   if (NULL != _sizslix[STllps]) delete [] _sizslix[STllps];
   if (NULL != _sizslix[STlnes]) delete [] _sizslix[STlnes];

   if (NULL != _fstslix[STlstr]) delete [] _fstslix[STlstr];
   if (NULL != _fstslix[STllps]) delete [] _fstslix[STllps];
   if (NULL != _fstslix[STlnes]) delete [] _fstslix[STlnes];
}


//=============================================================================
//
// class TenderRefLay
//
trend::TenderRefLay::TenderRefLay() :
   TrendRefLay    (      ),
   _pbuffer       (   0u ),
   _sizesvx       ( NULL ),
   _firstvx       ( NULL ),
   _sizslix       ( NULL ),
   _fstslix       ( NULL )
{
}

void trend::TenderRefLay::collect(GLuint pbuf)
{
   TNDR_GLDATAT* cpoint_array = NULL;
   _pbuffer = pbuf;
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _pbuffer)
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER              ,
                2 * total_points() * sizeof(TNDR_GLDATAT) ,
                nullptr                         ,
                GL_DYNAMIC_DRAW               )
   cpoint_array = (TNDR_GLDATAT*)DBGL_CALL(glMapBuffer,GL_ARRAY_BUFFER, GL_WRITE_ONLY)

   // initialise the indexing
   unsigned pntindx = 0;
   unsigned  szindx  = 0;
   if (0 < (_alvrtxs + _asindxs))
   {
      _firstvx = DEBUG_NEW GLsizei[_alobjvx + _asobjix];
      _sizesvx = DEBUG_NEW GLsizei[_alobjvx + _asobjix];
      if (0 < _asobjix)
      {
         _fstslix = DEBUG_NEW GLsizei[_asobjix];
         _sizslix = DEBUG_NEW GLsizei[_asobjix];
      }
   }
   // collect the cell overlapping boxes
   for (RefBoxList::const_iterator CSH = _cellRefBoxes.begin(); CSH != _cellRefBoxes.end(); CSH++)
   {
      if (1 < (*CSH)->alphaDepth())
      {
         _firstvx[szindx  ] = pntindx/2;
         _sizesvx[szindx++] = (*CSH)->cDataCopy(cpoint_array, pntindx);
      }
   }
   for (RefBoxList::const_iterator CSH = _cellSRefBoxes.begin(); CSH != _cellSRefBoxes.end(); CSH++)
   {
      _fstslix[szindx-_alobjvx] = _firstvx[szindx] = pntindx/2;
      _sizslix[szindx-_alobjvx] = _sizesvx[szindx] = (*CSH)->cDataCopy(cpoint_array, pntindx);
      szindx++;
   }
   assert(pntindx == 2 * (_alvrtxs + _asindxs));
   assert(szindx  ==     (_alobjvx + _asobjix));

   // Unmap the buffers
   glUnmapBuffer(GL_ARRAY_BUFFER);
}

void trend::TenderRefLay::draw(layprop::DrawProperties* drawprop)
{
   // Bind the buffer
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * total_points() * sizeof(TNDR_GLDATAT)));

   DBGL_CALL(glEnableClientState, GL_VERTEX_ARRAY)
   DBGL_CALL(glVertexPointer, 2, TNDR_GLENUMT, 0, nullptr)
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
   DBGL_CALL(glDisableClientState, GL_VERTEX_ARRAY)
}

void trend::TenderRefLay::setLine(layprop::DrawProperties* drawprop, bool selected)
{
   layprop::LineSettings curLine;
   drawprop->getCurrentLine(curLine, selected);
   glLineWidth(curLine.width());
   if (0xffff == curLine.pattern())
   {
      glDisable(GL_LINE_STIPPLE);
   }
   else
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(curLine.patscale(),curLine.pattern());
   }

}

trend::TenderRefLay::~TenderRefLay()
{
   if (NULL != _sizesvx) delete [] (_sizesvx);
   if (NULL != _firstvx) delete [] (_firstvx);
   if (NULL != _sizslix) delete [] (_sizslix);
   if (NULL != _fstslix) delete [] (_fstslix);
}

//=============================================================================
//
// class TenderMarks
//

trend::TenderMarks::TenderMarks() :
    TrendMarks    (      ),
   _pbuffer       (   0u )
{
}

void trend::TenderMarks::collect(GLuint pbuf)
{
   TNDR_GLDATAT* cpoint_array = NULL;
   _pbuffer = pbuf;
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _pbuffer);
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER              ,
                2 * total_points() * sizeof(TNDR_GLDATAT) ,
                nullptr                         ,
                GL_DYNAMIC_DRAW               )
   cpoint_array = (TNDR_GLDATAT*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

   unsigned pntindx = 0;
   for (PointList::const_iterator CP = _refMarks.begin(); CP != _refMarks.end(); CP++)
   {
      cpoint_array[pntindx++] = (TNDR_GLDATAT) CP->x();
      cpoint_array[pntindx++] = (TNDR_GLDATAT) CP->y();
   }
   for (PointList::const_iterator CP = _textMarks.begin(); CP != _textMarks.end(); CP++)
   {
      cpoint_array[pntindx++] = (TNDR_GLDATAT) CP->x();
      cpoint_array[pntindx++] = (TNDR_GLDATAT) CP->y();
   }
   for (PointList::const_iterator CP = _arefMarks.begin(); CP != _arefMarks.end(); CP++)
   {
      cpoint_array[pntindx++] = (TNDR_GLDATAT) CP->x();
      cpoint_array[pntindx++] = (TNDR_GLDATAT) CP->y();
   }
   assert(pntindx == 2 * total_points());
   // Unmap the buffers
   DBGL_CALL(glUnmapBuffer, GL_ARRAY_BUFFER)
}

void trend::TenderMarks::draw(layprop::DrawProperties* drawprop)
{
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   for (PointList::const_iterator CM = _refMarks.begin(); CM != _refMarks.end(); CM++)
   {
      glRasterPos2i(CM->x(),CM->y());
      glBitmap(16,16,7,7,0,0, drawprop->ref_mark_bmp());
   }

   for (PointList::const_iterator CM = _textMarks.begin(); CM != _textMarks.end(); CM++)
   {
      glRasterPos2i(CM->x(),CM->y());
      glBitmap(16,16,7,7,0,0, drawprop->text_mark_bmp());
   }

   for (PointList::const_iterator CM = _arefMarks.begin(); CM != _arefMarks.end(); CM++)
   {
      glRasterPos2i(CM->x(),CM->y());
      glBitmap(16,16,7,7,0,0, drawprop->aref_mark_bmp());
   }
}

//=============================================================================
//
// class Tenderer
//
trend::Tenderer::Tenderer( layprop::DrawProperties* drawprop, real UU, bool createRefLay ) :
    TrendBase            (drawprop, UU),
   _num_ogl_buffers      (       0u   ),
   _num_ogl_grc_buffers  (       0u   ),
//   _num_grid_points      (       0u   ),
   _num_ruler_ticks      (       0u   ),
   _ogl_buffers          (       NULL ),
   _ogl_grc_buffers      (       NULL ),
   _ogl_rlr_buffer       (       NULL ),
   _ogl_grd_buffer       (       NULL ),
   _sbuffer              (       0u   )
{
   if (createRefLay)
   {
      _refLayer = DEBUG_NEW TenderRefLay();
      _marks    = DEBUG_NEW TenderMarks();
   }
}

bool trend::Tenderer::chunkExists(const LayerDef& laydef, bool has_selected)
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
      _clayer = DEBUG_NEW TenderLay();
      _data.add(laydef, _clayer);
   }
   if (has_selected)
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true, _cslctd_array_offset);
   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true);
   return false;
}

void trend::Tenderer::setLayer(const LayerDef& laydef, bool has_selected)
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
      _clayer = DEBUG_NEW TenderLay();
      _data.add(laydef, _clayer);
   }
   if (has_selected)
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false, _cslctd_array_offset);
   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false);
}

void trend::Tenderer::setHvrLayer(const LayerDef& laydef)
{
   if (REF_LAY_DEF != laydef)
   {
      _clayer = DEBUG_NEW TenderLay();
      _data.add(laydef, _clayer);
      _clayer->newSlice(_cellStack.top(), false, false, 0 /*_cslctd_array_offset*/);
   }
}

void trend::Tenderer::grdDraw()
{
   DBGL_CALL(glEnableClientState, GL_VERTEX_ARRAY)
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _ogl_grd_buffer[0])
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * _num_grid_points * sizeof(TNDR_GLDATAT)));
   // Set-up the offset in the binded Vertex buffer
   DBGL_CALL(glVertexPointer, 2, TNDR_GLENUMT, 0, nullptr)
   unsigned startP = 0;
   for (VGrids::const_iterator CG = _grid_props.begin(); CG != _grid_props.end(); CG++)
   {
      layprop::tellRGB theColor(_drawprop->getColor((*CG)->color()));
      DBGL_CALL(glColor4ub, theColor.red(), theColor.green(), theColor.blue(), theColor.alpha())
      // draw
      DBGL_CALL(glDrawArrays, GL_POINTS, startP, (*CG)->asize())
      startP += (*CG)->asize();
   }
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0)
}

bool trend::Tenderer::collect()
{
   // First filter-out the layers that doesn't have any objects on them,
   // post process the last slices in the layers and also gather the number
   // of required virtual buffers
   //
   DataLay::Iterator CCLAY = _data.begin();
   unsigned num_total_slctdx = 0; // Initialize the number of total selected indexes
   unsigned num_total_strings = 0;
   while (CCLAY != _data.end())
   {
      CCLAY->ppSlice();
      num_total_strings += CCLAY->total_strings();
      if ((0 == CCLAY->total_points()) && (0 == CCLAY->total_strings()))
      {
         delete (*CCLAY);
         // Note! Careful here with the map iteration and erasing! Erase method
         // of map<> template doesn't return an iterator (unlike the list<>).
         // Despite the temptation to assume that the iterator will be valid after
         // the erase, it must be clear that erasing will invalidate the iterator.
         // If this is implemented more trivially using "for" cycle the code shall
         // crash, although it seems to work on certain platforms. Only seems -
         // it doesn't always crash, but it iterates in a weird way.
         // The implementation below seems to be the cleanest way to do this,
         // although it relies on my understanding of the way "++" operator should
         // be implemented
         _data.erase(CCLAY++());
      }
      else if (0 != CCLAY->total_points())
      {
         num_total_slctdx += CCLAY->total_slctdx();
         _num_ogl_buffers++;
         if (0 < CCLAY->total_indexs())
            _num_ogl_buffers++;
         ++CCLAY;
      }
      else
         ++CCLAY;
   }
   _clayer = NULL;
   if (0 < _refLayer->total_points())  _num_ogl_buffers ++; // reference boxes
   if (0 < _marks->total_points()   )  _num_ogl_buffers ++; // reference marks
   if (0 < num_total_slctdx      )     _num_ogl_buffers ++;  // selected
   // Check whether we have to continue after traversing
   if (0 == _num_ogl_buffers)
   {
      if (0 == num_total_strings)  return false;
      else                         return true;
   }
   //--------------------------------------------------------------------------
   //
   // generate all VBOs
   //

   checkOGLError("collect");

   _ogl_buffers = DEBUG_NEW GLuint [_num_ogl_buffers];
   DBGL_CALL(glGenBuffers,_num_ogl_buffers,_ogl_buffers)
   unsigned current_buffer = 0;
   //
   // collect the point arrays
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      if (0 == CLAY->total_points())
      {
         assert(0 != CLAY->total_strings());
         continue;
      }
      assert(current_buffer < _num_ogl_buffers);
      GLuint pbuf = _ogl_buffers[current_buffer++];
      assert( (0 == CLAY->total_indexs()) || (current_buffer < _num_ogl_buffers) );
      GLuint ibuf = (0 == CLAY->total_indexs()) ? 0u : _ogl_buffers[current_buffer++];
      CLAY->collect(_drawprop->layerFilled(CLAY()), pbuf, ibuf);
   }

   checkOGLError("collect");

   //
   // collect the indexes of the selected objects
   if (0 < num_total_slctdx)
   {// selected objects buffer
      _sbuffer = _ogl_buffers[current_buffer++];
      DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _sbuffer)
      DBGL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER           ,
                   num_total_slctdx * sizeof(unsigned) ,
                   nullptr                             ,
                   GL_DYNAMIC_DRAW                    )
      unsigned int* sindex_array = (unsigned int*)DBGL_CALL(glMapBuffer, GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY)
      for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
      {
         if (0 == CLAY->total_slctdx())
            continue;
         CLAY->collectSelected(sindex_array);
//         trend::dumpOGLArrayUint(sindex_array, num_total_slctdx );
      }
//      trend::dumpOGLArrayUint(sindex_array, num_total_slctdx );
      DBGL_CALL(glUnmapBuffer, GL_ELEMENT_ARRAY_BUFFER)
   }

//   checkOGLError("collect");

   //
   // collect the reference boxes
   if (0 < _refLayer->total_points())
   {
      GLuint pbuf = _ogl_buffers[current_buffer++];
      _refLayer->collect(pbuf);
   }
   // collect reference marks
   if (0 < _marks->total_points())
   {
      GLuint pbuf = _ogl_buffers[current_buffer++];
      _marks->collect(pbuf);
   }

   //
   // that's about it...
   checkOGLError("collect");
   return true;
}

bool trend::Tenderer::grcCollect()
{
   // First filter-out the layers that doesn't have any objects on them,
   // post process the last slices in the layers and also gather the number
   // of required virtual buffers
   //
   DataLay::Iterator CCLAY = _grcData.begin();
   while (CCLAY != _grcData.end())
   {
      CCLAY->ppSlice();
      if (0 == CCLAY->total_points())
      {
         delete (*CCLAY);
         _grcData.erase(CCLAY++());
      }
      else if (0 != CCLAY->total_points())
      {
         _num_ogl_grc_buffers++;
         if (0 < CCLAY->total_indexs())
            _num_ogl_grc_buffers++;
         ++CCLAY;
      }
      else
         ++CCLAY;
   }
   // Check whether we have to continue after traversing
   if (0 == _num_ogl_grc_buffers) return false;
   //--------------------------------------------------------------------------
   //
   // generate all VBOs
   //
   _ogl_grc_buffers = DEBUG_NEW GLuint [_num_ogl_grc_buffers];
   glGenBuffers(_num_ogl_grc_buffers, _ogl_grc_buffers);
   unsigned current_buffer = 0;
   //
   // collect the point arrays
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {
      if (0 == CLAY->total_points())
      {
         assert(0 != CLAY->total_strings());
         continue;
      }
      GLuint pbuf = _ogl_grc_buffers[current_buffer++];
      GLuint ibuf = (0 == CLAY->total_indexs()) ? 0u : _ogl_grc_buffers[current_buffer++];
      CLAY->collect(_drawprop->layerFilled(CLAY()), pbuf, ibuf);
   }
   //
   // collect the indexes of the selected objects
   // that's about it...
   checkOGLError("grcCollect");
   return true;
}

bool trend::Tenderer::grdCollect(const layprop::LayoutGrid** allGrids)
{
//   unsigned allPoints = 0;
   for (byte gridNo = 0; gridNo < 3; gridNo++)
   {
      const layprop::LayoutGrid* cgrid = allGrids[gridNo];
      if ((NULL !=  cgrid) && cgrid->visual())
      {
         int gridstep = (int)rint(cgrid->step() / _UU);
         bool gridOn = ( abs((int)(_drawprop->scrCtm().a() * gridstep)) > GRID_LIMIT);
         if (!gridOn) continue;
         // set first grid step to be multiply on the step
         TP bl = TP(_drawprop->clipRegion().p1().x(),_drawprop->clipRegion().p2().y());
         TP tr = TP(_drawprop->clipRegion().p2().x(),_drawprop->clipRegion().p1().y());

         TrendGridC* cvgrid = DEBUG_NEW TrendGridC(bl,tr,gridstep, cgrid->color());
         _num_grid_points += cvgrid->asize();
         _grid_props.push_back(cvgrid);
      }
   }
   if (0 == _num_grid_points) return false;
   _ogl_grd_buffer = DEBUG_NEW GLuint [1];
   DBGL_CALL(glGenBuffers, 1, _ogl_grd_buffer)

   TNDR_GLDATAT* cpoint_array = NULL;
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _ogl_grd_buffer[0])
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER                   ,
                2 * _num_grid_points * sizeof(TNDR_GLDATAT)  ,
                nullptr                                  ,
                GL_DYNAMIC_DRAW                       )
   cpoint_array = (TNDR_GLDATAT*)DBGL_CALL(glMapBuffer, GL_ARRAY_BUFFER, GL_WRITE_ONLY)
   unsigned pnt = 0;
   for (VGrids::const_iterator VG = _grid_props.begin(); VG != _grid_props.end(); VG++)
   {
      pnt = (*VG)->dump(cpoint_array, 2*pnt);
   }
   assert(pnt <= (_num_grid_points));
   // Unmap the buffers
   DBGL_CALL(glUnmapBuffer, GL_ARRAY_BUFFER)
   return true;
}

bool trend::Tenderer::rlrCollect(const layprop::RulerList& rulers, int4b step, const DBlineList& zcross)
{
   if (rulers.empty() && zcross.empty()) return false;
   DBline long_mark, short_mark, text_bp;
   double scaledpix;
   genRulerMarks(scrCTM().Reversed(), long_mark, short_mark, text_bp, scaledpix);
   DBlineList        noniList;        //!All ruler lines including Vernier ticks.
   for(layprop::RulerList::const_iterator RA = rulers.begin(); RA != rulers.end(); RA++)
   {
      RA->nonius(short_mark, long_mark, step, noniList);
      RA->addBaseLine(noniList);
      _rulerTexts.push_back(DEBUG_NEW trend::TrxText(RA->value(), RA->getFtmtrx(text_bp, scaledpix)));
   }
   for(DBlineList::const_iterator RA = zcross.begin(); RA != zcross.end(); RA++)
   {
      noniList.push_back(*RA);
   }

   _num_ruler_ticks = static_cast<unsigned>(2 * noniList.size());
   _ogl_rlr_buffer = DEBUG_NEW GLuint [1];
   DBGL_CALL(glGenBuffers, 1, _ogl_rlr_buffer)

   TNDR_GLDATAT* cpoint_array = NULL;
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _ogl_rlr_buffer[0]);
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER                       ,
                2 * _num_ruler_ticks * sizeof(TNDR_GLDATAT),
                nullptr                                    ,
                GL_DYNAMIC_DRAW                       )
   cpoint_array = (TNDR_GLDATAT*)DBGL_CALL(glMapBuffer, GL_ARRAY_BUFFER, GL_WRITE_ONLY)
   unsigned pnt = 0;
   for (DBlineList::const_iterator CL = noniList.begin(); CL != noniList.end(); CL++)
   {
      cpoint_array[pnt++]= CL->p1().x();
      cpoint_array[pnt++]= CL->p1().y();
      cpoint_array[pnt++]= CL->p2().x();
      cpoint_array[pnt++]= CL->p2().y();
   }
   assert(pnt == (2 * _num_ruler_ticks));
   // Unmap the buffers
   DBGL_CALL(glUnmapBuffer, GL_ARRAY_BUFFER)
//   glBindBuffer(GL_ARRAY_BUFFER, 0);
   return true;
}

void trend::Tenderer::setLayColor(const LayerDef& layer)
{
   layprop::tellRGB theColor;
   if (_drawprop->setCurrentColor(layer, theColor))
      glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());

}

void trend::Tenderer::setStipple()
{
   const byte* tellStipple = _drawprop->getCurrentFill();
   if (NULL == tellStipple)
   {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }
   else
   {
      byte FlipStillple [128];
      for (word i = 0; i < 32; i++)
         for (word j = 0; j < 4; j++)
            FlipStillple[(31-i)*4 + j] = tellStipple[i*4 + j];
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_POLYGON_STIPPLE);
      glPolygonStipple(FlipStillple);
   }
}

void trend::Tenderer::setLine(bool selected)
{
   layprop::LineSettings curLine;
   _drawprop->getCurrentLine(curLine, selected);
   glLineWidth(curLine.width());
   if (0xffff == curLine.pattern())
   {
      glDisable(GL_LINE_STIPPLE);
      /*glDisable(GL_LINE_SMOOTH);*/
   }
   else
   {
      glEnable(GL_LINE_STIPPLE);
      /*glEnable(GL_LINE_SMOOTH);*/
      glLineStipple(curLine.patscale(),curLine.pattern());
   }
}

void trend::Tenderer::draw()
{
   _drawprop->resetCurrentColor();
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      setLayColor(CLAY());
      setStipple();
      if (0 != CLAY->total_slctdx())
      {// redraw selected contours only
         setLine(true);
         DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _sbuffer)
         DBGL_CALL0(glPushMatrix);
         DBGL_CALL(glMultMatrixd, _activeCS->translation())
         CLAY->drawSelected();
         DBGL_CALL0(glPopMatrix)
         DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0)
      }
      setLine(false);
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
   setLayColor(REF_LAY_DEF);
   // draw reference boxes
   if (0 < _refLayer->total_points())
   {
      setLine(false);
      _refLayer->draw(_drawprop);
      DBGL_CALL(glDisable, GL_LINE_STIPPLE)
   }
   // draw marks
   if (0 < _marks->total_points()   )   _marks->draw(_drawprop);

   checkOGLError("draw");
}

void trend::Tenderer::grcDraw()
{
   _drawprop->resetCurrentColor();
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {// for every layer
      setLayColor(CLAY());
      setStipple();
      setLine(false);
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
   }
   checkOGLError("grcDraw");
}

void trend::Tenderer::cleanUp()
{
   // Clean-up the buffers
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
   if (NULL != _ogl_buffers)
   {
      DBGL_CALL(glDeleteBuffers,_num_ogl_buffers, _ogl_buffers)
      delete [] _ogl_buffers;
      _ogl_buffers = NULL;
   }
   TrendBase::cleanUp();
}

void trend::Tenderer::grcCleanUp()
{
   // Clean-up the buffers
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
   if (NULL != _ogl_grc_buffers)
   {
      DBGL_CALL(glDeleteBuffers,_num_ogl_grc_buffers, _ogl_grc_buffers)
      delete [] _ogl_grc_buffers;
      _ogl_grc_buffers = NULL;
   }
   TrendBase::grcCleanUp();
}

void trend::Tenderer::grdCleanUp()
{
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
   if (NULL != _ogl_grd_buffer)
   {
      DBGL_CALL(glDeleteBuffers,1, _ogl_grd_buffer)
      delete [] _ogl_grd_buffer;
      _ogl_grd_buffer = NULL;
   }
   TrendBase::grdCleanUp();
}

void trend::Tenderer::rlrCleanUp()
{
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
   if (NULL != _ogl_rlr_buffer)
   {
      DBGL_CALL(glDeleteBuffers,1, _ogl_rlr_buffer)
      delete [] _ogl_rlr_buffer;
      _ogl_rlr_buffer = NULL;
   }
   TrendBase::rlrCleanUp();
}

void trend::Tenderer::rlrDraw()
{
//   glColor4ub
   DBGL_CALL(glColor4f,(GLfloat)1, (GLfloat)1, (GLfloat)1, (GLfloat)0.7) // gray
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _ogl_rlr_buffer[0])
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(2 * _num_ruler_ticks * sizeof(TNDR_GLDATAT)));

   DBGL_CALL(glEnableClientState, GL_VERTEX_ARRAY)
   // Set-up the offset in the binded Vertex buffer
   DBGL_CALL(glVertexPointer, 2, TNDR_GLENUMT, 0, nullptr)
   // draw
   DBGL_CALL(glDrawArrays, GL_LINES, 0, _num_ruler_ticks)

   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0)

   // draw the ruler value
   TRENDC->bindFont();
   for (TrendStrings::const_iterator TS = _rulerTexts.begin(); TS != _rulerTexts.end(); TS++)
   {
      DBGL_CALL0(glPushMatrix)
      real ftm[16];
      (*TS)->ctm().oglForm(ftm);
      DBGL_CALL(glMultMatrixd, ftm);
      (*TS)->draw(false, _drawprop);
      DBGL_CALL0(glPopMatrix)
   }
}

void trend::Tenderer::setGrcLayer(bool setEData, const LayerDef& laydef)
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
         _grcLayer = DEBUG_NEW TenderLay();
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

trend::Tenderer::~Tenderer()
{
   cleanUp();
   grcCleanUp();
   grdCleanUp();
   rlrCleanUp();
//   delete _refLayer; //>> deleted by the parent constructor
}

