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
#include "trend.h"

extern trend::TrendCenter*         TRENDC;

unsigned trend::Trx3D::cDataCopy(TPVX3& array, unsigned& pindex, const unsigned offset)
{
   for (unsigned z = 0; z < 2; z++) // front and back plane
   {
      TNDR_GLDATAT zCoord = (TNDR_GLDATAT) (0==z ? _zDepth.bottom : _zDepth.top);
      for ( unsigned i = 0; i < 2*_csize; i+=2)
         array[offset+pindex++] = TPX3((TNDR_GLDATAT)_cdata[i],(TNDR_GLDATAT)_cdata[i+1], zCoord);
   }
   return _csize * 2;
}

unsigned trend::Trx3DBox::cDataCopy(TPVX3& array, unsigned& pindex, const unsigned offset)
{
   unsigned axs[4][2] = {{0,1},{2,1},{0,3},{2,3}};
   for (unsigned z = 0; z < 2; z++)
   {
      TNDR_GLDATAT zCoord = (TNDR_GLDATAT) (0==z ? _zDepth.bottom : _zDepth.top);
      for (unsigned i = 0; i < _csize; i++)
         array[offset+pindex++] = TPX3((TNDR_GLDATAT)_cdata[axs[i][0]], (TNDR_GLDATAT)_cdata[axs[i][1]], zCoord);
   }
   return _csize * 2;
}

trend::Trx3DWire::Trx3DWire(const int4b* ldata, unsigned lsize, WireWidth width, const ZDepth& zDepth) :
   Trx3D (NULL, 0, zDepth)
{
   _tdata = DEBUG_NEW TessellPoly;
   laydata::WireContour wcontour(ldata, lsize, width);
   _csize = wcontour.csize();
   // this intermediate variable is just to deal with the const-ness of _cdata;
   int4b* contData = DEBUG_NEW int4b[ 2 * _csize];
   wcontour.getArrayData(contData);
   _cdata = contData;
}


void trend::Trx3DWire::Tesselate()
{
   _tdata->pushBackTChunk(TessellChunk(_cdata, _csize, 0));
}

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
//   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
//   {//... layer by layer
//      if (0 == CLAY->total_points())
//      {
//         continue;
//      }
//      assert(current_buffer < _num_ogl_buffers);
//      GLuint pbuf = _ogl_buffers[current_buffer++];
//      assert( (0 == CLAY->total_indexs()) || (current_buffer < _num_ogl_buffers) );
//      GLuint ibuf = (0 == CLAY->total_indexs()) ? 0u : _ogl_buffers[current_buffer++];
//      CLAY->collect(pbuf, ibuf);
//   }
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

   //
   // that's about it...
   checkOGLError("collect");
   return true;
}

void trend::T3Der::draw()
{
   _drawprop->initCtmStack();
   TRENDC->setGlslProg(glslp_3D);
   setShaderMVP();
   _drawprop->resetCurrentColor();
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      setLayColor(CLAY());
//      setLine(false);
//      setStipple();
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
//      // draw texts
//      if (0 != CLAY->total_strings())
//      {
//         TRENDC->bindFont();
//         CLAY->drawTexts(_drawprop);
//      }
   }
   checkOGLError("draw");
   _drawprop->clearCtmStack();
}

void trend::T3Der::setShaderMVP()
{
   // manage the Model/View/Projection matrixes-------------------------------------------

   // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
   glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
   // Camera matrix
   glm::mat4 View       = glm::lookAt(
                          glm::vec3(0,0,6), // Camera is at (4,3,3), in World Space
                          glm::vec3(0,0,0), // and looks at the origin
                          glm::vec3(0,-1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                     );
//
//   glm::mat4 View       = glm::lookAt(
//                          glm::vec3(1,1,1), // Camera is at (4,3,3), in World Space
//                          glm::vec3(0,0,0), // and looks at the origin
//                          glm::vec3(0,0,1)  // Head is up (set to 0,-1,0 to look upside-down)
//                     );
   // Model matrix : an identity matrix (model will be at the origin)
   glm::mat4 Model      = glm::mat4(1.0f);
   // Our ModelViewProjection : multiplication of our 3 matrices
   glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

   TRENDC->setUniMtrx4fv(glslu_in_MVP, &MVP[0][0]);
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
      _clayer = DEBUG_NEW T3DLay(_drawprop->getLayDepth(laydef));
      _data.add(laydef, _clayer);
   }
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
      _clayer = DEBUG_NEW T3DLay(_drawprop->getLayDepth(laydef));
      _data.add(laydef, _clayer);
   }
   _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false);
}

void trend::T3Der::setLayColor(const LayerDef& layer)
{
   layprop::tellRGB tellColor;
   if (_drawprop->setCurrentColor(layer, tellColor))
   {
      float* oglColor = tellColor.getOGLcolor();
      TRENDC->setUniColor(oglColor);
      delete[] oglColor;
   }
}


void trend::T3Der::text(const std::string *, const CTM &, const DBbox &, const TP &, bool) {
   // In 3D rendering - texts are not visualized - at least for the time being
//   return;
}

trend::T3Der::~T3Der() /*noexcept*/
{
}

//===========================================================================
trend::T3DLay::T3DLay(const ZDepth& zDepth) :
   TrendLay (        )
 ,_zDepth   ( zDepth )
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
   static_cast<T3DTV*>(_cslice)->register3DBox(DEBUG_NEW Trx3DBox(pdata,_zDepth));
}

void trend::T3DLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly)
{
   static_cast<T3DTV*>(_cslice)->register3DPoly(DEBUG_NEW Trx3DPoly(pdata, psize, _zDepth), tpoly);
}

void trend::T3DLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool /*center_only*/)
{
   static_cast<T3DTV*>(_cslice)->register3DWire(DEBUG_NEW Trx3DWire(pdata, psize, width,_zDepth));
}

bool trend::T3DLay::chunkExists(TrxCellRef* const ctrans, bool filled)
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
   TPVX3 cpoint_array(_num_total_points);
   // Fill-up the buffers with data and indexes for drawing
   for( auto layChunk : _layData)
      static_cast<T3DTV*>(layChunk)->collect(cpoint_array, cindex_array);

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


void trend::T3DLay::draw(layprop::DrawProperties* drawprop)
{
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _pbuffer)
   // Check the state of the buffer
   GLint bufferSize;
   DBGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
   assert(bufferSize == (GLint)(3 * _num_total_points * sizeof(TNDR_GLDATAT)));
   if (0 != _ibuffer)
   {
      DBGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _ibuffer)
      DBGL_CALL(glGetBufferParameteriv, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize)
      assert(bufferSize == (GLint)(_num_total_indexs * sizeof(unsigned)));
   }
   for (auto TLAY : _layData)
      TLAY->draw(drawprop);
   for (auto TLAY : _reLayData)
      TLAY->draw(drawprop);

   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   if (0 != _ibuffer)
      DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
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

void trend::T3DTV::collect(TPVX3& point_array, unsigned int* index_array)
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

//      DEBUGprintOGL3data(_point_array_offset, _firstix, _sizesix, index_array, point_array, size_index);
   }

}

void trend::T3DTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   setShaderCTM(drawprop, _refCell);
//   setAlpha(drawprop);
//   DBGL_CALL(glEnable, GL_DEPTH_TEST);
//   DBGL_CALL(glDepthFunc, GL_LESS); // Accept fragment if it is closer to the camera than the former one

   // Activate the vertex buffers in the vertex shader ...
   DBGL_CALL(glEnableVertexAttribArray,TSHDR_LOC_VERTEX)
   // Set-up the offset in the binded Vertex buffer
   size_t koko = sizeof(TPX) * _point_array_offset;
//   assert(0==koko);
   /*printf("Offset in the vertex buffer: %d\n", koko)*/;
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 3, TNDR_GLENUMT, GL_FALSE, 0, (GLvoid*)(koko))
   // ... and here we go ...
   drawTriQuads();
//   TRENDC->setUniVarui(glslu_in_StippleEn, 0);
//   drawLines();
//   TRENDC->setUniVarui(glslu_in_StippleEn, 1);
   // Switch the vertex buffers OFF in the openGL engine ...
   DBGL_CALL(glDisableVertexAttribArray,TSHDR_LOC_VERTEX)
   // ... and finally restore the openGL translation matrix
   drawprop->popCtm();
}


void trend::T3DTV::drawTriQuads()
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

void trend::T3DTV::DEBUGprintOGL3data(const unsigned start, GLuint **firstix, GLsizei **sizesix, unsigned int *index_array, TPVX3 &point_array, unsigned int *size_index)
{
   unsigned i = start;
   for (auto boza : point_array)
   {
      printf("%3i ->X: %7.2f; Y: %7.2f; Z: %7.2f\n", i++, boza.x, boza.y, boza.z);
   }
   
   for (i = 0; i < size_index[ITtria]; i++)
   {
      unsigned findex = firstix[ITtria][i]/sizeof(unsigned);
      printf("Triangle  index %d -> Offset: %d ; Size: %d\n", i, findex, sizesix[ITtria][i]);
      printf("       Indexes:");
      for (GLsizei j = 0; j < sizesix[ITtria][i]; j++)
         printf(" %d", index_array[findex+j]);
      printf("\n");
   }
   
   for (i = 0; i < size_index[ITtstr]; i++)
   {
      unsigned findex = firstix[ITtstr][i]/sizeof(unsigned);
      printf("TriStrips index %d -> Offset: %d ; Size: %d\n", i, findex, sizesix[ITtstr][i]);
      printf("       Indexes:");
      for (GLsizei j = 0; j < sizesix[ITtstr][i]; j++)
         printf(" %d", index_array[findex+j]);
      printf("\n");
   }
}

void trend::T3DTV::setShaderCTM(layprop::DrawProperties* drawprop, const TrxCellRef* refCell)
{
   drawprop->pushCtm(refCell->ctm() * drawprop->topCtm());
   float mtrxOrtho [16];
   drawprop->topCtm().oglForm(mtrxOrtho);
   TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
}
