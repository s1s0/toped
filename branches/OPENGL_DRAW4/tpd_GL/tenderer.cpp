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

extern trend::FontLibrary* fontLib;

//=============================================================================
//
// class TenderTV
//
trend::TenderTV::TenderTV(TrendRef* const refCell, bool filled, bool reusable,
                   unsigned parray_offset, unsigned iarray_offset) :
   TrendTV(refCell, filled, reusable, parray_offset, iarray_offset),
   _point_array_offset  ( parray_offset   ),
   _index_array_offset  ( iarray_offset   )
{
   for (int i = fqss; i <= ftss; i++)
   {
      _sizesix[i] = NULL;
      _firstix[i] = NULL;
   }
   for (int i = cont; i <= ncvx; i++)
   {
      _sizesvx[i] = NULL;
      _firstvx[i] = NULL;
   }
}

void trend::TenderTV::collectIndexs(unsigned int* index_array, const TeselChain* tdata, unsigned* size_index,
                             unsigned* index_offset, unsigned cpoint_index)
{
   for (TeselChain::const_iterator TCH = tdata->begin(); TCH != tdata->end(); TCH++)
   {
      switch (TCH->type())
      {
         case GL_QUAD_STRIP     :
         {
            assert(_sizesix[fqss]);
            _firstix[fqss][size_index[fqss]  ] = sizeof(unsigned) * index_offset[fqss];
            _sizesix[fqss][size_index[fqss]++] = TCH->size();
            for (unsigned i = 0; i < TCH->size(); i++)
               index_array[index_offset[fqss]++] = TCH->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLES      :
         {
            assert(_sizesix[ftrs]);
            _firstix[ftrs][size_index[ftrs]  ] = sizeof(unsigned) * index_offset[ftrs];
            _sizesix[ftrs][size_index[ftrs]++] = TCH->size();
            for (unsigned i = 0; i < TCH->size(); i++)
               index_array[index_offset[ftrs]++] = TCH->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_FAN   :
         {
            assert(_sizesix[ftfs]);
            _firstix[ftfs][size_index[ftfs]  ] = sizeof(unsigned) * index_offset[ftfs];
            _sizesix[ftfs][size_index[ftfs]++] = TCH->size();
            for (unsigned i = 0; i < TCH->size(); i++)
               index_array[index_offset[ftfs]++] = TCH->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_STRIP :
         {
            assert(_sizesix[ftss]);
            _firstix[ftss][size_index[ftss]  ] = sizeof(unsigned) * index_offset[ftss];
            _sizesix[ftss][size_index[ftss]++] = TCH->size();
            for (unsigned i = 0; i < TCH->size(); i++)
               index_array[index_offset[ftss]++] = TCH->index_seq()[i] + cpoint_index;
            break;
         }
         default: assert(0);break;
      }
   }
}

void trend::TenderTV::collect(TNDR_GLDATAT* point_array, unsigned int* index_array)
{
   unsigned line_arr_size = 2 * _alvrtxs[line];
   unsigned fqus_arr_size = 2 * _alvrtxs[cnvx];
   unsigned cont_arr_size = 2 * _alvrtxs[cont];
   unsigned poly_arr_size = 2 * _alvrtxs[ncvx];
   // initialise the indexing
   unsigned pntindx = 0;

   if  (_alobjvx[line] > 0)
   {// collect all central lines of the wires
      unsigned  szindx  = 0;
      _firstvx[line] = DEBUG_NEW int[_alobjvx[line]];
      _sizesvx[line] = DEBUG_NEW int[_alobjvx[line]];
      for (SliceWires::const_iterator CSH = _line_data.begin(); CSH != _line_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[line][szindx  ] = pntindx/2;
         _sizesvx[line][szindx++] = (*CSH)->lDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      assert(pntindx == line_arr_size);
      assert(szindx  == _alobjvx[line]);
   }

   if  (_alobjvx[cnvx] > 0)
   {// collect all convex polygons
      unsigned  szindx  = 0;
      _firstvx[cnvx] = DEBUG_NEW int[_alobjvx[cnvx]];
      _sizesvx[cnvx] = DEBUG_NEW int[_alobjvx[cnvx]];
      for (SliceObjects::const_iterator CSH = _cnvx_data.begin(); CSH != _cnvx_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[cnvx][szindx  ] = pntindx/2;
         _sizesvx[cnvx][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      assert(pntindx == line_arr_size + fqus_arr_size);
      assert(szindx  == _alobjvx[cnvx]);
   }

   if  (_alobjvx[ncvx] > 0)
   {// collect all non-convex polygons
      unsigned  szindx  = 0;
      _firstvx[ncvx] = DEBUG_NEW int[_alobjvx[ncvx]];
      _sizesvx[ncvx] = DEBUG_NEW int[_alobjvx[ncvx]];
      if (NULL != index_array)
      {
         assert(_alobjix[fqss] + _alobjix[ftrs] + _alobjix[ftfs] + _alobjix[ftss]);
         if (0 < _alobjix[fqss])
         {
            _sizesix[fqss] = DEBUG_NEW GLsizei[_alobjix[fqss]];
            _firstix[fqss] = DEBUG_NEW GLuint[_alobjix[fqss]];
         }
         if (0 < _alobjix[ftrs])
         {
            _sizesix[ftrs] = DEBUG_NEW GLsizei[_alobjix[ftrs]];
            _firstix[ftrs] = DEBUG_NEW GLuint[_alobjix[ftrs]];
         }
         if (0 < _alobjix[ftfs])
         {
            _sizesix[ftfs] = DEBUG_NEW GLsizei[_alobjix[ftfs]];
            _firstix[ftfs] = DEBUG_NEW GLuint[_alobjix[ftfs]];
         }
         if (0 < _alobjix[ftss])
         {
            _sizesix[ftss] = DEBUG_NEW GLsizei[_alobjix[ftss]];
            _firstix[ftss] = DEBUG_NEW GLuint[_alobjix[ftss]];
         }
      }
      unsigned size_index[4];
      unsigned index_offset[4];
      size_index[fqss] = size_index[ftrs] = size_index[ftfs] = size_index[ftss] = 0u;
      index_offset[fqss] = _index_array_offset;
      index_offset[ftrs] = index_offset[fqss] + _alindxs[fqss];
      index_offset[ftfs] = index_offset[ftrs] + _alindxs[ftrs];
      index_offset[ftss] = index_offset[ftfs] + _alindxs[ftfs];
      for (SlicePolygons::const_iterator CSH = _ncvx_data.begin(); CSH != _ncvx_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)

         if (NULL != (*CSH)->tdata())
            collectIndexs( index_array     ,
                          (*CSH)->tdata()  ,
                           size_index      ,
                           index_offset    ,
                           pntindx/2
                         );
         _firstvx[ncvx][szindx  ] = pntindx/2;
         _sizesvx[ncvx][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);

      }
      assert(size_index[fqss] == _alobjix[fqss]);
      assert(size_index[ftrs] == _alobjix[ftrs]);
      assert(size_index[ftfs] == _alobjix[ftfs]);
      assert(size_index[ftss] == _alobjix[ftss]);
      assert(index_offset[fqss] == (_index_array_offset + _alindxs[fqss]));
      assert(index_offset[ftrs] == (_index_array_offset + _alindxs[fqss] + _alindxs[ftrs]));
      assert(index_offset[ftfs] == (_index_array_offset + _alindxs[fqss] + _alindxs[ftrs] + _alindxs[ftfs] ));
      assert(index_offset[ftss] == (_index_array_offset + _alindxs[fqss] + _alindxs[ftrs] + _alindxs[ftfs] + _alindxs[ftss] ));
      assert(pntindx == line_arr_size + fqus_arr_size + poly_arr_size);
      assert(szindx  == _alobjvx[ncvx]);
   }

   if  (_alobjvx[cont] > 0)
   {// collect all contours (only non-filled objects here)
      unsigned  szindx  = 0;
      _firstvx[cont] = DEBUG_NEW int[_alobjvx[cont]];
      _sizesvx[cont] = DEBUG_NEW int[_alobjvx[cont]];
      for (SliceObjects::const_iterator CSH = _cont_data.begin(); CSH != _cont_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[cont][szindx  ] = pntindx/2;
         _sizesvx[cont][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      //... and text overlapping boxes
      for (RefTxtList::const_iterator CSH = _txto_data.begin(); CSH != _txto_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         _firstvx[cont][szindx  ] = pntindx/2;
         _sizesvx[cont][szindx++] = (*CSH)->cDataCopy(&(point_array[_point_array_offset]), pntindx);
      }
      assert(pntindx == line_arr_size + fqus_arr_size + cont_arr_size + poly_arr_size);
      assert(szindx  == _alobjvx[cont] );
   }
}

void trend::TenderTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   drawprop->adjustAlpha(_refCell->alphaDepth() - 1);
   // Switch the vertex buffers ON in the openGL engine ...
   glEnableClientState(GL_VERTEX_ARRAY);
   // Set-up the offset in the binded Vertex buffer
   glVertexPointer(2, TNDR_GLENUMT, 0, (GLvoid*)(sizeof(TNDR_GLDATAT) * _point_array_offset));
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
   glDisableClientState(GL_VERTEX_ARRAY);
   // ... and finally restore the openGL translation matrix
   glPopMatrix();
}

void trend::TenderTV::drawTexts(layprop::DrawProperties* drawprop)
{
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   drawprop->adjustAlpha(_refCell->alphaDepth() - 1);

   for (TrendStrings::const_iterator TSTR = _text_data.begin(); TSTR != _text_data.end(); TSTR++)
      (*TSTR)->draw(_filled);

   glPopMatrix();
}

trend::TenderTV::~TenderTV()
{
   if (NULL != _sizesvx[cont]) delete [] _sizesvx[cont];
   if (NULL != _sizesvx[line]) delete [] _sizesvx[line];
   if (NULL != _sizesvx[cnvx]) delete [] _sizesvx[cnvx];
   if (NULL != _sizesvx[ncvx]) delete [] _sizesvx[ncvx];

   if (NULL != _sizesix[fqss]) delete [] _sizesix[fqss];
   if (NULL != _sizesix[ftrs]) delete [] _sizesix[ftrs];
   if (NULL != _sizesix[ftfs]) delete [] _sizesix[ftfs];
   if (NULL != _sizesix[ftss]) delete [] _sizesix[ftss];

   if (NULL != _firstvx[cont]) delete [] _firstvx[cont];
   if (NULL != _firstvx[line]) delete [] _firstvx[line];
   if (NULL != _firstvx[cnvx]) delete [] _firstvx[cnvx];
   if (NULL != _firstvx[ncvx]) delete [] _firstvx[ncvx];

   if (NULL != _firstix[fqss]) delete [] _firstix[fqss];
   if (NULL != _firstix[ftrs]) delete [] _firstix[ftrs];
   if (NULL != _firstix[ftfs]) delete [] _firstix[ftfs];
   if (NULL != _firstix[ftss]) delete [] _firstix[ftss];
}

//=============================================================================
//
// class TenderReTV
//
void trend::TenderReTV::draw(layprop::DrawProperties* drawprop)
{
   TrendRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->draw(drawprop);
   _chunk->swapRefCells(sref_cell);
}

void trend::TenderReTV::drawTexts(layprop::DrawProperties* drawprop)
{
   TrendRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->drawTexts(drawprop);
   _chunk->swapRefCells(sref_cell);
}

//=============================================================================
//
// class TenderLay
//
trend::TenderLay::TenderLay():
   TrendLay              (             ),
   _stv_array_offset     (          0u ),
   _slctd_array_offset   (          0u )
{
   for (int i = lstr; i <= lnes; i++)
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
void trend::TenderLay::newSlice(TrendRef* const ctrans, bool fill, bool reusable, unsigned slctd_array_offset)
{
   assert( 0 == total_slctdx());
   _slctd_array_offset = slctd_array_offset;
   _stv_array_offset = 2 * _num_total_points;
   newSlice(ctrans, fill, reusable);
}

void trend::TenderLay::newSlice(TrendRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW TenderTV(ctrans, fill, reusable, 2 * _num_total_points, _num_total_indexs);
}

bool trend::TenderLay::chunkExists(TrendRef* const ctrans, bool filled)
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

void trend::TenderLay::collect(bool fill, GLuint pbuf, GLuint ibuf)
{
   TNDR_GLDATAT* cpoint_array = NULL;
   unsigned int* cindex_array = NULL;
   _pbuffer = pbuf;
   _ibuffer = ibuf;
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   glBufferData(GL_ARRAY_BUFFER                       ,
                2 * _num_total_points * sizeof(TNDR_GLDATAT),
                NULL                                  ,
                GL_DYNAMIC_DRAW                       );
   cpoint_array = (TNDR_GLDATAT*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
   if (0 != _ibuffer)
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER           ,
                   _num_total_indexs * sizeof(unsigned) ,
                   NULL                              ,
                   GL_DYNAMIC_DRAW                    );
      cindex_array = (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
   }
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      (*TLAY)->collect(cpoint_array, cindex_array);
   // Unmap the buffers
   glUnmapBuffer(GL_ARRAY_BUFFER);
//   glBindBuffer(GL_ARRAY_BUFFER, 0);
   if (0 != _ibuffer)
   {
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
//      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }
}

void trend::TenderLay::collectSelected(unsigned int* slctd_array)
{
   unsigned      slct_arr_size = _asindxs[lstr] + _asindxs[llps] + _asindxs[lnes];
   if (0 == slct_arr_size) return;

   // initialise the indexing arrays of selected objects
   if (0 < _asobjix[lstr])
   {
      _sizslix[lstr] = DEBUG_NEW GLsizei[_asobjix[lstr]];
      _fstslix[lstr] = DEBUG_NEW GLuint[_asobjix[lstr]];
   }
   if (0 < _asobjix[llps])
   {
      _sizslix[llps] = DEBUG_NEW GLsizei[_asobjix[llps]];
      _fstslix[llps] = DEBUG_NEW GLuint[_asobjix[llps]];
   }
   if (0 < _asobjix[lnes])
   {
      _sizslix[lnes] = DEBUG_NEW GLsizei[_asobjix[lnes]];
      _fstslix[lnes] = DEBUG_NEW GLuint[_asobjix[lnes]];
   }
   unsigned size_sindex[3];
   unsigned index_soffset[3];
   size_sindex[lstr] = size_sindex[llps] = size_sindex[lnes] = 0u;
   index_soffset[lstr] = _slctd_array_offset;
   index_soffset[llps] = index_soffset[lstr] + _asindxs[lstr];
   index_soffset[lnes] = index_soffset[llps] + _asindxs[llps];


   for (SliceSelected::const_iterator SSL = _slct_data.begin(); SSL != _slct_data.end(); SSL++)
   {
      TrendSelected* cchunk = *SSL;
      switch (cchunk->type())
      {
         case lstr : // LINES
         {
            assert(_sizslix[lstr]);
            _fstslix[lstr][size_sindex[lstr]  ] = sizeof(unsigned) * index_soffset[lstr];
            _sizslix[lstr][size_sindex[lstr]++] = cchunk->sDataCopy(slctd_array, index_soffset[lstr]);
            break;
         }
         case llps      : // LINE_LOOP
         {
            assert(_sizslix[llps]);
            _fstslix[llps][size_sindex[llps]  ] = sizeof(unsigned) * index_soffset[llps];
            _sizslix[llps][size_sindex[llps]++] = cchunk->sDataCopy(slctd_array, index_soffset[llps]);
            break;
         }
         case lnes   : // LINE_STRIP
         {
            assert(_sizslix[lnes]);
            _fstslix[lnes][size_sindex[lnes]  ] = sizeof(unsigned) * index_soffset[lnes];
            _sizslix[lnes][size_sindex[lnes]++] = cchunk->sDataCopy(slctd_array, index_soffset[lnes]);
            break;
         }
         default: assert(false);break;
      }
   }
}

void trend::TenderLay::draw(layprop::DrawProperties* drawprop)
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

void trend::TenderLay::drawSelected()
{
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   // Check the state of the buffer
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(TNDR_GLDATAT)));

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_INDEX_ARRAY);
   glVertexPointer(2, TNDR_GLENUMT, 0, (GLvoid*)(sizeof(TNDR_GLDATAT) * _stv_array_offset));

   if (_asobjix[lstr] > 0)
   {
      assert(_sizslix[lstr]);
      assert(_fstslix[lstr]);
      //glMultiDrawElements(GL_LINE_STRIP, _sizslix[lstr], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lstr], _asobjix[lstr]);
      for (unsigned i= 0; i < _asobjix[lstr]; i++)
         glDrawElements(GL_LINE_STRIP, _sizslix[lstr][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_fstslix[lstr][i]));
   }
   if (_asobjix[llps] > 0)
   {
      assert(_sizslix[llps]);
      assert(_fstslix[llps]);
         //glMultiDrawElements(GL_LINE_LOOP     , _sizslix[llps], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[llps], _alobjix[llps]);
      for (unsigned i= 0; i < _asobjix[llps]; i++)
         glDrawElements(GL_LINE_LOOP, _sizslix[llps][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_fstslix[llps][i]));
   }
   if (_asobjix[lnes] > 0)
   {
      assert(_sizslix[lnes]);
      assert(_fstslix[lnes]);
         //glMultiDrawElements(GL_LINES  , _sizslix[lnes], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lnes], _alobjix[lnes]);
      for (unsigned i= 0; i < _asobjix[lnes]; i++)
         glDrawElements(GL_LINES, _sizslix[lnes][i], GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_fstslix[lnes][i]));
   }
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);


   glBindBuffer(GL_ARRAY_BUFFER, 0);
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
   if (NULL != _sizslix[lstr]) delete [] _sizslix[lstr];
   if (NULL != _sizslix[llps]) delete [] _sizslix[llps];
   if (NULL != _sizslix[lnes]) delete [] _sizslix[lnes];

   if (NULL != _fstslix[lstr]) delete [] _fstslix[lstr];
   if (NULL != _fstslix[llps]) delete [] _fstslix[llps];
   if (NULL != _fstslix[lnes]) delete [] _fstslix[lnes];
}


//=============================================================================
//
// class TenderRefLay
//
trend::TenderRefLay::TenderRefLay() :
   TrendRefLay    (      ),
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
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   glBufferData(GL_ARRAY_BUFFER              ,
                2 * total_points() * sizeof(TNDR_GLDATAT) ,
                NULL                         ,
                GL_DYNAMIC_DRAW               );
   cpoint_array = (TNDR_GLDATAT*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

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
   layprop::tellRGB theColor;
   if (drawprop->setCurrentColor(REF_LAY_DEF, theColor))
      glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
//   drawprop->setCurrentColor(REF_LAY_DEF);
   drawprop->setLineProps(false);
   // Bind the buffer
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   // Check the state of the buffer
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   assert(bufferSize == (GLint)(2 * total_points() * sizeof(TNDR_GLDATAT)));

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, TNDR_GLENUMT, 0, 0);
   if (0 < (_alvrtxs + _asindxs))
   {
      assert(_firstvx); assert(_sizesvx);
      glMultiDrawArrays(GL_LINE_LOOP, _firstvx, _sizesvx, _alobjvx + _asobjix);
      if (0 < _asindxs)
      {
         assert(_fstslix); assert(_sizslix);
         drawprop->setLineProps(true);
         glMultiDrawArrays(GL_LINE_LOOP, _fstslix, _sizslix, _asobjix);
         drawprop->setLineProps(false);
      }
   }
   glDisableClientState(GL_VERTEX_ARRAY);
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
// class Tenderer
//
trend::Tenderer::Tenderer( layprop::DrawProperties* drawprop, real UU ) :
    TrendBase            (drawprop, UU),
   _num_ogl_buffers      (       0u   ),
   _num_ogl_grc_buffers  (       0u   ),
   _ogl_buffers          (       NULL ),
   _ogl_grc_buffers      (       NULL )
{
   _refLayer = DEBUG_NEW TenderRefLay();
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

void trend::Tenderer::grid(const real step, const std::string color)
{
   int gridstep = (int)rint(step / _UU);
   if ( abs((int)(_drawprop->scrCtm().a() * gridstep)) > GRID_LIMIT)
   {
      _drawprop->setGridColor(color);
      // set first grid step to be multiply on the step
      TP bl = TP(_drawprop->clipRegion().p1().x(),_drawprop->clipRegion().p2().y());
      TP tr = TP(_drawprop->clipRegion().p2().x(),_drawprop->clipRegion().p1().y());
      int signX = (bl.x() > 0) ? 1 : -1;
      int X_is = (int)((rint(abs(bl.x()) / gridstep)) * gridstep * signX);
      int signY = (tr.y() > 0) ? 1 : -1;
      int Y_is = (int)((rint(abs(tr.y()) / gridstep)) * gridstep * signY);

      word arr_size = ( (((tr.x() - X_is + 1) / gridstep) + 1) * (((bl.y() - Y_is + 1) / gridstep) + 1) );
      int* point_array = DEBUG_NEW int[arr_size * 2];
      int index = 0;
      for (int i = X_is; i < tr.x()+1; i += gridstep)
      {
         for (int j = Y_is; j < bl.y()+1; j += gridstep)
         {
            point_array[index++] = i;
            point_array[index++] = j;
         }
      }
      assert(index <= (arr_size*2));
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(2, GL_INT, 0, point_array);
      glDrawArrays(GL_POINTS, 0, arr_size);
      glDisableClientState(GL_VERTEX_ARRAY);
      delete [] point_array;
   }
}

void trend::Tenderer::zeroCross()
{
   //TODO
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
   if (0 < num_total_slctdx      )     _num_ogl_buffers++;  // selected
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
   _ogl_buffers = DEBUG_NEW GLuint [_num_ogl_buffers];
   glGenBuffers(_num_ogl_buffers, _ogl_buffers);
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
   //
   // collect the indexes of the selected objects
   if (0 < num_total_slctdx)
   {// selected objects buffer
      _sbuffer = _ogl_buffers[current_buffer++];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _sbuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER           ,
                   num_total_slctdx * sizeof(unsigned) ,
                                              NULL                              ,
                                              GL_DYNAMIC_DRAW                    );
      unsigned int* sindex_array = (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
      for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
      {
         if (0 == CLAY->total_slctdx())
            continue;
         CLAY->collectSelected(sindex_array);
      }
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
   }
   //
   // collect the reference boxes
   if (0 < _refLayer->total_points())
   {
      GLuint pbuf = _ogl_buffers[current_buffer++];
      _refLayer->collect(pbuf);
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

void trend::Tenderer::draw()
{
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      layprop::tellRGB theColor;
      if (_drawprop->setCurrentColor(CLAY(), theColor))
         glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
//      _drawprop->setCurrentColor(CLAY());
      _drawprop->setCurrentFill(true); // force fill (ignore block_fill state)
      _drawprop->setLineProps(false);
      if (0 != CLAY->total_slctdx())
      {// redraw selected contours only
         _drawprop->setLineProps(true);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _sbuffer);
         glPushMatrix();
         glMultMatrixd(_activeCS->translation());
         CLAY->drawSelected();
         glPopMatrix();
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
         _drawprop->setLineProps(false);
      }
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
      // draw texts
      if (0 != CLAY->total_strings())
      {
         fontLib->bindFont();
         CLAY->drawTexts(_drawprop);
      }
   }
   // draw reference boxes
   if (0 < _refLayer->total_points())   _refLayer->draw(_drawprop);
   checkOGLError("draw");
}

void trend::Tenderer::grcDraw()
{
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {// for every layer
      layprop::tellRGB theColor;
      if (_drawprop->setCurrentColor(CLAY(), theColor))
         glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
//      _drawprop->setCurrentColor(CLAY());
      _drawprop->setCurrentFill(true); // force fill (ignore block_fill state)
      _drawprop->setLineProps(false);
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
   }
   checkOGLError("grcDraw");
}

void trend::Tenderer::cleanUp()
{
   // Clean-up the buffers
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   if (NULL != _ogl_buffers)
   {
      glDeleteBuffers(_num_ogl_buffers, _ogl_buffers);
      delete [] _ogl_buffers;
      _ogl_buffers = NULL;
   }
   TrendBase::cleanUp();
}

void trend::Tenderer::grcCleanUp()
{
   // Clean-up the buffers
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   if (NULL != _ogl_grc_buffers)
   {
      glDeleteBuffers(_num_ogl_grc_buffers, _ogl_grc_buffers);
      delete [] _ogl_grc_buffers;
      _ogl_grc_buffers = NULL;
   }
   TrendBase::grcCleanUp();
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
   delete _refLayer;
}

