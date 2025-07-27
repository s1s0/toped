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
//        Created: Thu Jul 24 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL 3D renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "t3der.h"


//===========================================================================
trend::T3Der::T3Der(layprop::DrawProperties *drawprop, real UU) :
   TrendBase             (drawprop, UU)
  ,_num_ogl_buffers      (       0u   )
//  ,_num_ogl_grc_buffers  (       0u   )
  ,_ogl_buffers          (       NULL )
//  ,_ogl_grc_buffers      (       NULL )
//  ,_sbuffer              (       0u   )
{
//   _refLayer = DEBUG_NEW ToshaderRefLay();
}

void trend::T3Der::pushCell(std::string cname, const CTM& trans, const DBbox& overlap, bool active, bool /*selected*/)
{
   TrxCellRef* cRefBox = DEBUG_NEW TrxCellRef(cname,
                                          trans * _cellStack.top()->ctm(),
                                          overlap,
                                          _cellStack.size()
                                         );
//   if (selected || (!_drawprop->cellBoxHidden()))
//      _refLayer->addCellOBox(cRefBox, _cellStack.size(), selected);
//   else
//      // This list is to keep track of the hidden cRefBox - so we can clean
//      // them up. Don't get confused - we need cRefBox during the collecting
//      // and drawing phase so we can't really delete them here or after they're
//      // poped-up from _cellStack. The confusion is coming from the "duality"
//      // of the TrxCellRef - once as a cell reference with CTM, view depth etc.
//      // and then as a placeholder of the overlapping reference box
//      _hiddenRefBoxes.push_back(cRefBox);

   _cellStack.push(cRefBox);
   if (active)
   {
      assert(NULL == _activeCS);
      _activeCS = cRefBox;
   }
//   else if (!_drawprop->cellMarksHidden())
//   {
//      _marks->addRefMark(overlap.p1(), _cellStack.top()->ctm());
//   }
}

bool trend::T3Der::collect()
{
   // First filter-out the layers that doesn't have any objects on them,
   // post process the last slices in the layers and also gather the number
   // of required virtual buffers
   //
   DataLay::Iterator CCLAY = _data.begin();
//   unsigned num_total_slctdx = 0; // Initialize the number of total selected indexes
//   unsigned num_total_strings = 0;
   while (CCLAY != _data.end())
   {
      CCLAY->ppSlice();
//      num_total_strings += CCLAY->total_strings();
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
//         num_total_slctdx += CCLAY->total_slctdx();
         _num_ogl_buffers++;
         if (0 < CCLAY->total_indexs())
            _num_ogl_buffers++;
         ++CCLAY;
      }
      else
         ++CCLAY;
   }
   _clayer = NULL;
//   if (0 < _refLayer->total_points())  _num_ogl_buffers ++; // reference boxes
//   if (0 < _marks->total_points()   )  _num_ogl_buffers ++; // reference marks
//   if (0 < num_total_slctdx      )     _num_ogl_buffers ++;  // selected
   // Check whether we have to continue after traversing
   if (0 == _num_ogl_buffers)
      return false;

   //--------------------------------------------------------------------------
   //
   // generate all VBOs
   //

   checkOGLError("collect");

   _ogl_buffers = DEBUG_NEW GLuint [_num_ogl_buffers];
   DBGL_CALL(glGenBuffers,_num_ogl_buffers,_ogl_buffers)
   unsigned current_buffer = 0;
   //
   // collect the point & index arrays across all visible cells for every layer
   // separately. This is effectively all vertexes of all visible TDT shapes
   // TDT cell structure at this stage (i.e. after traversing) is transparent
   for (auto layer : _data)
   {//... layer by layer
      if (0 == layer->total_points())
      {
         continue;
      }
      assert(current_buffer < _num_ogl_buffers);
      GLuint pbuf = _ogl_buffers[current_buffer++];
      assert( (0 == layer->total_indexs()) || (current_buffer < _num_ogl_buffers) );
      GLuint ibuf = (0 == layer->total_indexs()) ? 0u : _ogl_buffers[current_buffer++];
      layer->collect(pbuf, ibuf);
   }

   checkOGLError("collect");

//   //
//   // collect the indexes of the selected objects
//   if (0 < num_total_slctdx)
//   {// selected objects buffer
//      _sbuffer = _ogl_buffers[current_buffer++];
//      DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _sbuffer)
//      DBGL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER  ,
//                   num_total_slctdx * sizeof(unsigned) ,
//                   nullptr                             ,
//                   GL_STATIC_DRAW                    )
//      unsigned int* sindex_array = (unsigned int*)DBGL_CALL(glMapBuffer, GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY)
//      for (auto layer : _data)
//      {
//         if (0 == layer->total_slctdx()) continue;
//         layer->collectSelected(sindex_array);
////         trend::dumpOGLArrayUint(sindex_array, num_total_slctdx );
//      }
////      trend::dumpOGLArrayUint(sindex_array, num_total_slctdx );
//      DBGL_CALL(glUnmapBuffer, GL_ELEMENT_ARRAY_BUFFER)
//   }
//
////   checkOGLError("collect");
//
//   //
//   // collect the reference boxes
//   if (0 < _refLayer->total_points())
//   {
//      GLuint pbuf = _ogl_buffers[current_buffer++];
//      _refLayer->collect(pbuf);
//   }
//   // collect reference marks
//   if (0 < _marks->total_points())
//   {
//      GLuint pbuf = _ogl_buffers[current_buffer++];
//      _marks->collect(pbuf);
//   }

   //
   // that's about it...
   checkOGLError("collect");
   return true;
}

bool trend::T3Der::chunkExists(const LayerDef& laydef, bool /*has_selected*/)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with REF_LAY by accident
   assert(REF_LAY_DEF != laydef);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
//      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(laydef))
   {
      _clayer = _data[laydef];
      if (_clayer->chunkExists(_cellStack.top(), _drawprop->layerFilled(laydef) ) ) return true;
   }
   else
   {
      _clayer = DEBUG_NEW T3DLay();
      _data.add(laydef, _clayer);
   }
//   if (has_selected)
//      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true, _cslctd_array_offset);
//   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true);
   return false;

}

void trend::T3Der::setLayer(const LayerDef& laydef, bool /*has_selected*/)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with REF_LAY by accident
   assert(REF_LAY_DEF != laydef);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
//      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(laydef))
   {
      _clayer = _data[laydef];
   }
   else
   {
      _clayer = DEBUG_NEW T3DLay();
      _data.add(laydef, _clayer);
   }
//   if (has_selected)
//      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false, _cslctd_array_offset);
//   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false);
}

void trend::T3Der::text(const std::string *, const CTM &, const DBbox &, const TP &, bool) {
   // In 3D rendering - texts are not visualized - at least for the time being
//   return;
}

trend::T3Der::~T3Der() /*noexcept*/
{
}

//===========================================================================
trend::T3DLay::T3DLay() :
   TrendLay()
{
   
}

void trend::T3DLay::newSlice(TrxCellRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW T3DTV(ctrans, fill, reusable, _num_total_points, _num_total_indexs);
}

trend::T3DLay::~T3DLay()
{
   
}


void trend::T3DLay::box  (const int4b* pdata)
{
   static_cast<T3DTV*>(_cslice)->register3DBox(DEBUG_NEW Trx3DBox(pdata));
}

void trend::T3DLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly)
{
   static_cast<T3DTV*>(_cslice)->register3DPoly(DEBUG_NEW Trx3DPoly(pdata, psize), tpoly);
}

void trend::T3DLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool /*center_only*/)
{
   static_cast<T3DTV*>(_cslice)->register3DWire(DEBUG_NEW Trx3DWire(pdata, psize, width));
}

void trend::T3DLay::collect(GLuint pbuf, GLuint ibuf)
{
//   TNDR_GLDATAT* cpoint_array = NULL;
   unsigned int* cindex_array = NULL;
   _pbuffer = pbuf;
   _ibuffer = ibuf;
   assert(0 != _ibuffer);
//   if (0 != _ibuffer)
//   {
      //---------------------------------------------------------
      // bind the index buffer & get the reference to the OGL index arrays
      DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, _ibuffer)
      DBGL_CALL(glBufferData,GL_ELEMENT_ARRAY_BUFFER    ,
                   _num_total_indexs * sizeof(unsigned) ,
                   nullptr                              ,
                   GL_STATIC_DRAW                    )
      cindex_array = (unsigned int*)DBGL_CALL(glMapBuffer,GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
//   }

   //---------------------------------------------------------
   // For the vertex buffer use std::vector structure i.e. TPVX
   TPVX cpoint_array(_num_total_points);
   // Fill-up the buffers with data and indexes for drawing
   for( auto layChunk : _layData)
      layChunk->collect(cpoint_array, cindex_array);

   // get the vertex data transferred to OGL
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _pbuffer)
   DBGL_CALL(glBufferData,GL_ARRAY_BUFFER                ,
             byteSize(cpoint_array)                      ,
             &(cpoint_array[0])                          ,
             GL_STATIC_DRAW                              )
   
   // Unmap the index buffer. This should transfer the data
   // to OGL
//   if (0 != _ibuffer)
      DBGL_CALL(glUnmapBuffer,GL_ELEMENT_ARRAY_BUFFER)

}

//===========================================================================
trend::T3DTV::T3DTV(TrxCellRef* const refCell, bool filled, bool reusable,
                   unsigned parray_offset, unsigned iarray_offset) :
   TrendTV(refCell, filled, reusable, parray_offset, iarray_offset)
  ,_point_array_offset  ( parray_offset   )
  ,_index_array_offset  ( iarray_offset   )
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

trend::T3DTV::~T3DTV()
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


void trend::T3DTV::register3DBox(Trx3DBox* cobj)
{
   // a generic tesselation object for all boxes
   TessellPoly* tdata = DEBUG_NEW TessellPoly();
   tdata->tessellate3DBox();
   cobj->setTeselData(tdata);

   _ncvx_data.push_back(cobj);
   _vrtxnum[OTncvx] += 2 * cobj->csize();
   _iobjnum[ITtria] += tdata->num_tria();
   _iobjnum[ITtstr] += tdata->num_tstr();
   tdata->num_indexs(_indxnum[ITtria], _indxnum[ITtstr]);
   _vobjnum[OTncvx]++;
}

void trend::T3DTV::register3DPoly(Trx3DPoly* cobj, const TessellPoly* tchain)
{
   TessellPoly* tdata = DEBUG_NEW TessellPoly(tchain);
   tdata->tessellate3DPoly(cobj->csize());
   cobj->setTeselData(tdata);

   _ncvx_data.push_back(cobj);
   _vrtxnum[OTncvx] += 2 * cobj->csize();
   _iobjnum[ITtria] += tdata->num_tria();
//   _alobjix[ftfs] += tdata->num_ftfs();
   _iobjnum[ITtstr] += tdata->num_tstr();
   tdata->num_indexs(_indxnum[ITtria], /*_alindxs[ftfs],*/ _indxnum[ITtstr]);
   _vobjnum[OTncvx]++;
}

void trend::T3DTV::register3DWire(Trx3DWire* cobj)
{
   cobj->Tesselate();
//   unsigned allpoints = cobj->csize();
   _ncvx_data.push_back(cobj);
   
   _vrtxnum[OTncvx] += 2 * cobj->csize();
   _iobjnum[ITtria] += cobj->tpdata()->num_tria();
   _iobjnum[ITtstr] += cobj->tpdata()->num_tstr();
   cobj->tpdata()->num_indexs(_indxnum[ITtria], _indxnum[ITtstr]);
   _vobjnum[OTncvx]++;
}

void trend::T3DTV::collectIndexs(unsigned int* index_array, const TessellChain* tdata, unsigned* size_index,
                             unsigned* index_offset, const unsigned cpoint_index)
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

void trend::T3DTV::collect(TPVX& point_array, unsigned int* index_array)
{
   // initialise the indexing
   unsigned    pntindx     = 0;//_point_array_offset;
   unsigned    szindx      = 0;
   unsigned    controlSize = 0; // used only in asserts
   ObjectTypes objType        ;

   //====================================================================
   // deal with non-convex polygons
   if  (_vobjnum[OTncvx] > 0)
   {// collect all non-convex polygons
      szindx  = 0;
      objType = OTncvx;
      controlSize += _vrtxnum[objType];
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
      unsigned size_index[IDX_TYPES];
      unsigned index_offset[IDX_TYPES];
      size_index[ITtria] = size_index[ITtstr] = 0u;
      index_offset[ITtria] = _index_array_offset;
      index_offset[ITtstr] = index_offset[ITtria] + _indxnum[ITtria];
      for (auto CSH : _ncvx_data)
      {
         if (NULL != CSH->tdata())
            collectIndexs( index_array     ,
                           CSH->tdata()    ,
                           size_index      ,
                           index_offset    ,
                           pntindx
                         );
         _firstvx[OTncvx][szindx  ] = pntindx;
         _sizesvx[OTncvx][szindx++] = CSH->cDataCopy(point_array, pntindx, _point_array_offset);
      }
      assert(size_index[ITtria] == _iobjnum[ITtria]);
      assert(size_index[ITtstr] == _iobjnum[ITtstr]);
      assert(index_offset[ITtria] == (_index_array_offset + _indxnum[ITtria]));
      assert(index_offset[ITtstr] == (_index_array_offset + _indxnum[ITtria] + _indxnum[ITtstr] ));

      assert(pntindx == controlSize);
      assert(szindx  == _vobjnum[OTncvx]);
   }

}

