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
#include "tenderer.h"
#include "viewprop.h"
#include "tedat.h"
#include <sstream>

GLUtriangulatorObj   *TeselPoly::tenderTesel = NULL;

//=============================================================================
//
TeselChunk::TeselChunk(const TeselVertices& data, GLenum type, unsigned offset)
{
   _size = data.size();
   _index_seq = DEBUG_NEW unsigned[_size];
   word li = 0;
   for(TeselVertices::const_iterator CVX = data.begin(); CVX != data.end(); CVX++)
      _index_seq[li++] = *CVX + offset;
   _type = type;
}

TeselChunk::TeselChunk(const TeselChunk* data, unsigned offset)
{
   _size = data->size();
   _type = data->type();
   _index_seq = DEBUG_NEW unsigned[_size];
   const unsigned* copy_seq = data->index_seq();
   for(unsigned i = 0; i < _size; i++)
      _index_seq[i] = copy_seq[i] + offset;
}

TeselChunk::TeselChunk(const int* data, unsigned size, unsigned offset)
{ // used for wire tesselation explicitly
   _size = size;
   _type = GL_QUAD_STRIP;
   assert(0 ==(size % 2));
   _index_seq = DEBUG_NEW unsigned[_size];
   word findex = 0;     // forward  index
   word bindex = _size; // backward index
   for (word i = 0; i < _size / 2; i++)
   {
      _index_seq[2*i  ] = (findex++) + offset;
      _index_seq[2*i+1] = (--bindex) + offset;
   }
}

TeselChunk::~TeselChunk()
{
   delete [] _index_seq;
}
//=============================================================================
//

TeselTempData::TeselTempData(unsigned offset) :_the_chain(NULL), _cindexes(),
           _all_ftrs(0), _all_ftfs(0), _all_ftss(0), _offset(offset)
{}

TeselTempData::TeselTempData(TeselChain* tc) : _the_chain(tc), _cindexes(),
           _all_ftrs(0), _all_ftfs(0), _all_ftss(0), _offset(0)
{}

void TeselTempData::storeChunk()
{
   TeselChunk* achunk = DEBUG_NEW TeselChunk(_cindexes, _ctype, _offset);
   _the_chain->push_back(achunk);
   switch (_ctype)
   {
      case GL_TRIANGLE_FAN   : _all_ftfs++; break;
      case GL_TRIANGLE_STRIP : _all_ftss++; break;
      case GL_TRIANGLES      : _all_ftrs++; break;
      default: assert(0);
   }
}

//=============================================================================
// TeselPoly

TeselPoly::TeselPoly(const int4b* pdata, unsigned psize)
{
   TeselTempData ttdata( &_tdata );
   // Start tessellation
   gluTessBeginPolygon(tenderTesel, &ttdata);
   GLdouble pv[3];
   pv[2] = 0;
   word* index_arr = DEBUG_NEW word[psize];
   for (unsigned i = 0; i < psize; i++ )
   {
      pv[0] = pdata[2*i]; pv[1] = pdata[2*i+1];
      index_arr[i] = i;
      gluTessVertex(tenderTesel,pv, &(index_arr[i]));
   }
   gluTessEndPolygon(tenderTesel);
   delete [] index_arr;
   _all_ftrs = ttdata.num_ftrs();
   _all_ftfs = ttdata.num_ftfs();
   _all_ftss = ttdata.num_ftss();
}


GLvoid TeselPoly::teselBegin(GLenum type, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newChunk(type);
}

GLvoid TeselPoly::teselVertex(GLvoid *pindex, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newIndex(*(static_cast<word*>(pindex)));
}

GLvoid TeselPoly::teselEnd(GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->storeChunk();
}

void TeselPoly::num_indexs(unsigned& iftrs, unsigned& iftfs, unsigned& iftss)
{
   for (TeselChain::const_iterator CCH = _tdata.begin(); CCH != _tdata.end(); CCH++)
   {
      switch ((*CCH)->type())
      {
         case GL_TRIANGLE_FAN   : iftfs += (*CCH)->size(); break;
         case GL_TRIANGLE_STRIP : iftss += (*CCH)->size(); break;
         case GL_TRIANGLES      : iftrs += (*CCH)->size(); break;
         default: assert(0);
      }
   }
}


TeselPoly::~TeselPoly()
{
   for (TeselChain::const_iterator CTC = _tdata.begin(); CTC != _tdata.end(); CTC++)
      delete (*CTC);
}

//=============================================================================
//
// TenderCnvx
//
unsigned TenderCnvx::cDataCopy(int* array, unsigned& pindex)
{
   assert(_csize);
   memcpy(&(array[pindex]), _cdata, 2 * sizeof(int4b) * _csize);
   pindex += 2 * _csize;
   return _csize;
}

//=============================================================================
//
// TenderWire
//
TenderWire::TenderWire(int4b* pdata, unsigned psize, const word width, bool clo)
   : TenderNcvx(NULL, 0), _ldata(pdata), _lsize(psize), _celno(clo), _tdata(NULL)
{
   if (!_celno)
      precalc(width);
}

unsigned TenderWire::lDataCopy(int* array, unsigned& pindex)
{
   assert(_lsize);
   memcpy(&(array[pindex]), _ldata, 2 * sizeof(int4b) * _lsize);
   pindex += 2 * _lsize;
   return _lsize;
}

void TenderWire::precalc(word width)
{
   _csize = 2 * _lsize;
   _cdata = DEBUG_NEW int[2 * _csize];
   DBbox* ln1 = endPnts(width, 0,1, true);
   word index = 0;
   word rindex = 2 * _csize - 1;
   assert (ln1);
   _cdata[ index++] = ln1->p1().x();
   _cdata[ index++] = ln1->p1().y();
   _cdata[rindex--] = ln1->p2().y();
   _cdata[rindex--] = ln1->p2().x();
   delete ln1;
   for (unsigned i = 1; i < _lsize - 1; i++)
   {
      ln1 = mdlPnts(width, i-1,i,i+1);
      assert(ln1);
      _cdata[ index++] = ln1->p1().x();
      _cdata[ index++] = ln1->p1().y();
      _cdata[rindex--] = ln1->p2().y();
      _cdata[rindex--] = ln1->p2().x();
      delete ln1;
   }
   ln1 = endPnts(width, _lsize -2, _lsize - 1,false);
   assert(ln1);
   _cdata[ index++] = ln1->p1().x();
   _cdata[ index++] = ln1->p1().y();
   _cdata[rindex--] = ln1->p2().y();
   _cdata[rindex--] = ln1->p2().x();
   delete ln1;
   assert(index == _csize);
   assert((rindex + 1u) == _csize);
}

DBbox* TenderWire::endPnts(const word width, word i1, word i2, bool first)
{
   double     w = width/2;
   i1 *= 2; i2 *= 2;
   double denom = first ? (_ldata[i2  ] - _ldata[i1  ]) : (_ldata[i1  ] - _ldata[i2  ]);
   double   nom = first ? (_ldata[i2+1] - _ldata[i1+1]) : (_ldata[i1+1] - _ldata[i2+1]);
   double xcorr, ycorr; // the corrections
   assert((0 != nom) || (0 != denom));
   double signX = (  nom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   double signY = (denom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   if      (0 == denom) // vertical
   {
      xcorr =signX * w ; ycorr = 0        ;
   }
   else if (0 == nom  )// horizontal |----|
   {
      xcorr = 0        ; ycorr = signY * w;
   }
   else
   {
      double sl   = nom / denom;
      double sqsl = signY*sqrt( sl*sl + 1);
      xcorr = rint(w * (sl / sqsl));
      ycorr = rint(w * ( 1 / sqsl));
   }
   word it = first ? i1 : i2;
   return DEBUG_NEW DBbox((int4b) rint(_ldata[it  ] - xcorr),
                          (int4b) rint(_ldata[it+1] + ycorr),
                          (int4b) rint(_ldata[it  ] + xcorr),
                          (int4b) rint(_ldata[it+1] - ycorr) );
}

DBbox* TenderWire::mdlPnts(const word width, word i1, word i2, word i3)
{
   double    w = width/2;
   i1 *= 2; i2 *= 2; i3 *= 2;
   double  x32 = _ldata[i3  ] - _ldata[i2  ];
   double  x21 = _ldata[i2  ] - _ldata[i1  ];
   double  y32 = _ldata[i3+1] - _ldata[i2+1];
   double  y21 = _ldata[i2+1] - _ldata[i1+1];
   double   L1 = sqrt(x21*x21 + y21*y21); //the length of segment 1
   double   L2 = sqrt(x32*x32 + y32*y32); //the length of segment 2
   double denom = x32 * y21 - x21 * y32;
   assert (denom);
   assert (L2);
   // the corrections
   double xcorr = w * ((x32 * L1 - x21 * L2) / denom);
   double ycorr = w * ((y21 * L2 - y32 * L1) / denom);
   return DEBUG_NEW DBbox((int4b) rint(_ldata[i2  ] - xcorr),
                          (int4b) rint(_ldata[i2+1] + ycorr),
                          (int4b) rint(_ldata[i2  ] + xcorr),
                          (int4b) rint(_ldata[i2+1] - ycorr) );
}

/** For wire tessellation we can use the common polygon tessellation procedure.
    This could be a huge overhead though. The thing is that we've
    already been trough the precalc procedure and we know that wire object is
    very specific non-convex polygon. Using this knowledge the tessallation
    is getting really trivial. All we have to do is to list the contour points
    indexes in pairs - one from the front, and the other from the back of the
    array. Then this can be drawn as GL_QUAD_STRIP
*/
void TenderWire::Tesselate()
{
   _tdata = DEBUG_NEW TeselChain();
   _tdata->push_back( DEBUG_NEW TeselChunk(_cdata, _csize, 0));
}

TenderWire::~TenderWire()
{
   if (NULL != _cdata) delete [] _cdata;
   if (NULL != _tdata)
   {
      for (TeselChain::const_iterator CCH = _tdata->begin(); CCH != _tdata->end(); CCH++)
         delete (*CCH);
      delete _tdata;
   }
}

//=============================================================================
//
// TenderSCnvx
//
unsigned TenderSCnvx::ssize()
{
   if (NULL == _slist) return _csize;
   // get the number of selected segments first - don't forget that here
   // we're using GL_LINE_STRIP which means that we're counting selected
   // line segments and for each segment we're going to store two indexes
   unsigned ssegs = 0;
   word  ssize = _slist->size();
   for (word i = 0; i < _csize; i++)
      if (_slist->check(i) && _slist->check((i+1)% ssize )) ssegs +=2;
   return ssegs;
}

unsigned  TenderSCnvx::cDataCopy(int* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderCnvx::cDataCopy(array, pindex);
}

unsigned  TenderSCnvx::sDataCopy(unsigned* array, unsigned& pindex)
{
   if (NULL != _slist)
   { // shape is partially selected
      // copy the indexes of the selected segment points
      for (unsigned i = 0; i < _csize; i++)
      {
         if (_slist->check(i) && _slist->check((i+1)%_csize))
         {
            array[pindex++] = _offset + i;
            array[pindex++] = _offset + ((i+1)%_csize);
         }
      }
   }
   else
   {
      for (unsigned i = 0; i < _csize; i++)
         array[pindex++] = _offset + i;
   }
   return ssize();
}

//=============================================================================
//
// TenderSNcvx
//
unsigned TenderSNcvx::ssize()
{
   if (NULL == _slist) return _csize;
   // get the number of selected segments first - don't forget that here
   // we're using GL_LINE_STRIP which means that we're counting selected
   // line segments and for each segment we're going to store two indexes
   unsigned ssegs = 0;
   word  ssize = _slist->size();
   for (word i = 0; i < _csize; i++)
      if (_slist->check(i) && _slist->check((i+1)% ssize )) ssegs +=2;
   return ssegs;
}

unsigned  TenderSNcvx::cDataCopy(int* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderCnvx::cDataCopy(array, pindex);
}

unsigned  TenderSNcvx::sDataCopy(unsigned* array, unsigned& pindex)
{
   if (NULL != _slist)
   { // shape is partially selected
      // copy the indexes of the selected segment points
      for (unsigned i = 0; i < _csize; i++)
      {
         if (_slist->check(i) && _slist->check((i+1)%_csize))
         {
            array[pindex++] = _offset + i;
            array[pindex++] = _offset + ((i+1)%_csize);
         }
      }
   }
   else
   {
      for (unsigned i = 0; i < _csize; i++)
         array[pindex++] = _offset + i;
   }
   return ssize();
}

//=============================================================================
//
// TenderSWire
//
unsigned TenderSWire::cDataCopy(int* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderCnvx::cDataCopy(array, pindex);
}

unsigned TenderSWire::lDataCopy(int* array, unsigned& pindex)
{
   _loffset = pindex/2;
   return TenderWire::lDataCopy(array, pindex);
}

unsigned TenderSWire::ssize()
{
   if (NULL == _slist) return _lsize;
   // get the number of selected segments first - don't forget that here
   // we're using GL_LINE_STRIP which means that we're counting selected
   // line segments and for each segment we're going to store two indexes
   unsigned ssegs = 0;
   word  ssize = _slist->size();
   for (word i = 0; i < _lsize - 1; i++)
      if (_slist->check(i) && _slist->check((i+1)% ssize )) ssegs +=2;
   // Don't forget the edge points. This tiny little special case was dictating
   // the rules when the selected objects were fitted into the whole Tenderer
   // class hierarchy.
   if (_slist->check(0)            ) ssegs +=2;
   if (_slist->check(_lsize-1)  ) ssegs +=2;
   return ssegs;
}

unsigned  TenderSWire::sDataCopy(unsigned* array, unsigned& pindex)
{
   if (NULL != _slist)
   { // shape is partially selected
      // copy the indexes of the selected segment points
      for (unsigned i = 0; i < _lsize; i++)
      {
         if (_slist->check(i) && _slist->check((i+1)%_csize))
         {
            array[pindex++] = _loffset + i;
            array[pindex++] = _loffset + ((i+1)%_lsize);
         }
      }
      // And the edge points!
      if (_slist->check(0)       ) // if first point is selected
      {
         array[pindex++] = _offset;
         array[pindex++] = _offset + _csize -1;
      }
      if (_slist->check(_lsize-1))// if last point is selected
      {
         array[pindex++] = _offset + (_csize/2) -1;
         array[pindex++] = _offset + (_csize/2);
      }
   }
   else
   {
      for (unsigned i = 0; i < _lsize; i++)
         array[pindex++] = _loffset + i;
   }
   return ssize();
}

//=============================================================================
//
// class TenderOBox
//
TenderOBox::TenderOBox(const DBbox& obox, const CTM& ctm)
{
   TP tp = TP(obox.p1().x(), obox.p1().y()) * ctm;
   _obox[0] = tp.x();_obox[1] = tp.y();
   tp = TP(obox.p2().x(), obox.p1().y()) * ctm;
   _obox[2] = tp.x();_obox[3] = tp.y();
   tp = TP(obox.p2().x(), obox.p2().y()) * ctm;
   _obox[4] = tp.x();_obox[5] = tp.y();
   tp = TP(obox.p1().x(), obox.p2().y()) * ctm;
   _obox[6] = tp.x();_obox[7] = tp.y();
}

unsigned TenderOBox::cDataCopy(int* array, unsigned& pindex)
{
   memcpy(&(array[pindex]), _obox, sizeof(int4b) * 8);
   pindex += 8;
   return 4;
}

//=============================================================================
//
// class TenderRef
//
TenderRef::TenderRef(std::string name, const CTM& ctm, const DBbox& obox,
                   word alphaDepth)
   : _name(name), _ctm(ctm), _alphaDepth(alphaDepth)
{
   _ctm.oglForm(_translation);
   TP tp = TP(obox.p1().x(), obox.p1().y()) * _ctm;
   _obox[0] = tp.x();_obox[1] = tp.y();
   tp = TP(obox.p2().x(), obox.p1().y()) * _ctm;
   _obox[2] = tp.x();_obox[3] = tp.y();
   tp = TP(obox.p2().x(), obox.p2().y()) * _ctm;
   _obox[4] = tp.x();_obox[5] = tp.y();
   tp = TP(obox.p1().x(), obox.p2().y()) * _ctm;
   _obox[6] = tp.x();_obox[7] = tp.y();
}


TenderRef::TenderRef() : _name(""), _ctm(CTM()), _alphaDepth(0)
{
   _ctm.oglForm(_translation);
   for (word i = 0; i < 8; _obox[i++] = 0);
}

unsigned TenderRef::cDataCopy(int* array, unsigned& pindex)
{
   memcpy(&(array[pindex]), _obox, sizeof(int4b) * 8);
   pindex += 8;
   return 4;
}

//=============================================================================
//
// class TenderTV
//
TenderTV::TenderTV(TenderRef* const refCell, bool filled, bool reusable,
                   unsigned parray_offset, unsigned iarray_offset) :
   _refCell             ( refCell         ),
   _point_array_offset  ( parray_offset   ),
   _index_array_offset  ( iarray_offset   ),
   _filled              ( filled          ),
   _reusable            ( reusable        )
{
   for (int i = fqss; i <= ftss; i++)
   {
      _sizesix[i] = NULL;
      _firstix[i] = NULL;
      _alobjix[i] = 0u;
      _alindxs[i] = 0u;
   }
   for (int i = cont; i <= ncvx; i++)
   {
      _sizesvx[i] = NULL;
      _firstvx[i] = NULL;
      _alobjvx[i] = 0u;
      _alvrtxs[i] = 0u;
   }
}

void TenderTV::registerBox (TenderCnvx* cobj)
{
   unsigned allpoints = cobj->csize();
   if (_filled)
   {
      _cnvx_data.push_back(cobj);
      _alvrtxs[cnvx] += allpoints;
      _alobjvx[cnvx]++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _alvrtxs[cont] += allpoints;
      _alobjvx[cont]++;
   }
}

void TenderTV::registerPoly (TenderNcvx* cobj, TeselPoly* tchain)
{
   unsigned allpoints = cobj->csize();
   if (_filled)
   {
      cobj->setTeselData(tchain);
      _ncvx_data.push_back(cobj);
      _alvrtxs[ncvx] += allpoints;
      _alobjix[ftrs] += tchain->num_ftrs();
      _alobjix[ftfs] += tchain->num_ftfs();
      _alobjix[ftss] += tchain->num_ftss();
      tchain->num_indexs(_alindxs[ftrs], _alindxs[ftfs], _alindxs[ftss]);
      _alobjvx[ncvx]++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _alvrtxs[cont] += allpoints;
      _alobjvx[cont]++;
   }
}

void TenderTV::registerWire (TenderWire* cobj)
{
   unsigned allpoints = cobj->csize();
   _line_data.push_back(cobj);
   _alvrtxs[line] += cobj->lsize();
   _alobjvx[line]++;
   if ( !cobj->center_line_only() )
   {
       if (_filled)
       {
         cobj->Tesselate();
         _ncvx_data.push_back(cobj);
         _alvrtxs[ncvx] += allpoints;
         _alindxs[fqss] += allpoints;
         _alobjvx[ncvx]++;
         _alobjix[fqss]++;
       }
       else
       {
          _cont_data.push_back(cobj);
          _alobjvx[cont] += 1;
          _alvrtxs[cont] += allpoints;
       }
   }
}

unsigned TenderTV::num_total_points()
{
   return ( _alvrtxs[cont] +
            _alvrtxs[line] +
            _alvrtxs[cnvx] +
            _alvrtxs[ncvx]
          );
}

unsigned TenderTV::num_total_indexs()
{
   return ( _alindxs[fqss] +
            _alindxs[ftrs] +
            _alindxs[ftfs] +
            _alindxs[ftss]
          );
}

void TenderTV::collectIndexs(unsigned int* index_array, TeselChain* tdata, unsigned* size_index,
                             unsigned* index_offset, unsigned cpoint_index)
{
   for (TeselChain::const_iterator TCH = tdata->begin(); TCH != tdata->end(); TCH++)
   {
      TeselChunk* cchunk = *TCH;
      switch (cchunk->type())
      {
         case GL_QUAD_STRIP     :
         {
            assert(_sizesix[fqss]);
            _firstix[fqss][size_index[fqss]  ] = sizeof(unsigned) * index_offset[fqss];
            _sizesix[fqss][size_index[fqss]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[fqss]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLES      :
         {
            assert(_sizesix[ftrs]);
            _firstix[ftrs][size_index[ftrs]  ] = sizeof(unsigned) * index_offset[ftrs];
            _sizesix[ftrs][size_index[ftrs]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[ftrs]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_FAN   :
         {
            assert(_sizesix[ftfs]);
            _firstix[ftfs][size_index[ftfs]  ] = sizeof(unsigned) * index_offset[ftfs];
            _sizesix[ftfs][size_index[ftfs]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[ftfs]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_STRIP :
         {
            assert(_sizesix[ftss]);
            _firstix[ftss][size_index[ftss]  ] = sizeof(unsigned) * index_offset[ftss];
            _sizesix[ftss][size_index[ftss]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[ftss]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         default: assert(0);
      }
   }
}

void TenderTV::collect(int* point_array, unsigned int* index_array, unsigned int*)
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
            collectIndexs(index_array    ,
                        (*CSH)->tdata()  ,
                        size_index       ,
                        index_offset     ,
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
      assert(pntindx == line_arr_size + fqus_arr_size + cont_arr_size + poly_arr_size);
      assert(szindx  == _alobjvx[cont] );
   }
}

void TenderTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   drawprop->adjustAlpha(_refCell->alphaDepth() - 1);
   // Set-up the offset in the binded Vertex buffer
   glVertexPointer(2, GL_INT, 0, (GLvoid*)(sizeof(int4b) * _point_array_offset));
   // Switch the vertex buffers ON in the openGL engine ...
   glEnableClientState(GL_VERTEX_ARRAY);
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
            glDrawElements(GL_QUAD_STRIP, _sizesix[fqss][i], GL_UNSIGNED_INT, (const GLvoid*)_firstix[fqss][i]);
      }
      if (_alobjix[ftrs] > 0)
      {
         assert(_sizesix[ftrs]);
         assert(_firstix[ftrs]);
         //glMultiDrawElements(GL_TRIANGLES     , _sizesix[ftrs], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftrs], _alobjix[ftrs]);
         for (unsigned i= 0; i < _alobjix[ftrs]; i++)
            glDrawElements(GL_TRIANGLES, _sizesix[ftrs][i], GL_UNSIGNED_INT, (const GLvoid*)_firstix[ftrs][i]);
      }
      if (_alobjix[ftfs] > 0)
      {
         assert(_sizesix[ftfs]);
         assert(_firstix[ftfs]);
         //glMultiDrawElements(GL_TRIANGLE_FAN  , _sizesix[ftfs], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftfs], _alobjix[ftfs]);
         for (unsigned i= 0; i < _alobjix[ftfs]; i++)
            glDrawElements(GL_TRIANGLE_FAN, _sizesix[ftfs][i], GL_UNSIGNED_INT, (const GLvoid*)_firstix[ftfs][i]);
      }
      if (_alobjix[ftss] > 0)
      {
         assert(_sizesix[ftss]);
         assert(_firstix[ftss]);
         //glMultiDrawElements(GL_TRIANGLE_STRIP, _sizesix[ftss], GL_UNSIGNED_INT, (const GLvoid**)_firstix[ftss], _alobjix[ftss]);
         for (unsigned i= 0; i < _alobjix[ftss]; i++)
            glDrawElements(GL_TRIANGLE_STRIP, _sizesix[ftss][i], GL_UNSIGNED_INT, (const GLvoid*)_firstix[ftss][i]);
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


TenderRef* TenderTV::swapRefCells(TenderRef* newRefCell)
{
   TenderRef* the_swap = _refCell;
   _refCell = newRefCell;
   return the_swap;
}

TenderTV::~TenderTV()
{
   for (SliceWires::const_iterator CSO = _line_data.begin(); CSO != _line_data.end(); CSO++)
      if ((*CSO)->center_line_only()) delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cnvx_data.begin(); CSO != _cnvx_data.end(); CSO++)
      delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cont_data.begin(); CSO != _cont_data.end(); CSO++)
      delete (*CSO);
   for (SlicePolygons::const_iterator CSO = _ncvx_data.begin(); CSO != _ncvx_data.end(); CSO++)
      delete (*CSO);
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
   // Don't delete  _tmatrix. It's only a reference to it here
}

void TenderReTV::draw(layprop::DrawProperties* drawprop)
{
   TenderRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->draw(drawprop);
   _chunk->swapRefCells(sref_cell);
}
//=============================================================================
//
// class TenderLay
//
TenderLay::TenderLay(): _cslice(NULL),
   _num_total_points(0u), _num_total_indexs(0u),
   _has_selected(false), _stv_array_offset(0u), _slctd_array_offset(0u)
{
   for (int i = lstr; i <= lnes; i++)
   {
      _sizslix[i] = NULL;
      _fstslix[i] = NULL;
      _asindxs[i] = 0u;
      _asobjix[i] = 0u;
   }
}

/**
 * Create a new object of TenderTV type which will be reffered to by _cslice
 * @param ctrans Current translation matrix of the new object
 * @param fill Whether to fill the drawing objects
 */
void TenderLay::newSlice(TenderRef* const ctrans, bool fill, bool reusable, bool has_selected, unsigned slctd_array_offset)
{
   if ((_has_selected = has_selected)) // <-- that's not a mistake!
   {
      assert( 0 == total_slctdx());
      _slctd_array_offset = slctd_array_offset;
      _stv_array_offset = 2 * _num_total_points;
   }
   _cslice = DEBUG_NEW TenderTV(ctrans, fill, reusable, 2 * _num_total_points, _num_total_indexs);
}

bool TenderLay::chunkExists(TenderRef* const ctrans)
{
   ReusableTTVMap::iterator achunk;
   if (_reusableData.end() == ( achunk =_reusableData.find(ctrans->name()) ) )
      return false;
   _reLayData.push_back(DEBUG_NEW TenderReTV(achunk->second, ctrans));
   return true;
}

/** Add the current slice object (_cslice) to the list of slices _layData but
only if it's not empty. Also track the total number of vertexes in the layer
*/
void TenderLay::ppSlice()
{
   if (NULL != _cslice)
   {
      unsigned num_points = _cslice->num_total_points();
      if (num_points > 0)
      {
         _layData.push_back(_cslice);
         _num_total_points += num_points;
         _num_total_indexs += _cslice->num_total_indexs();
         if (_cslice->reusable())
         {
            assert(_reusableData.end() == _reusableData.find(_cslice->cellName()));
            _reusableData[_cslice->cellName()] = _cslice;
         }
      }
      else
         delete _cslice;
      _cslice = NULL;
   }
}

void TenderLay::box  (int4b* pdata, bool sel = false, const SGBitSet* ss= NULL)
{
   // Make sure that selected shapes don't come unexpected
   assert(_has_selected ? true : !sel);
   TenderCnvx* cobj = NULL;
   if (sel)
   {
      TenderSCnvx* sobj = DEBUG_NEW TenderSCnvx(pdata, 4, ss);
      registerSBox(sobj);
      cobj = sobj;
   }
   else
   {
      cobj = DEBUG_NEW TenderCnvx(pdata, 4);
   }
   _cslice->registerBox(cobj);
}

void TenderLay::poly (int4b* pdata, unsigned psize, TeselPoly* tpoly, bool sel = false, const SGBitSet* ss= NULL)
{
   // Make sure that selected shapes don't come unexpected
   assert(_has_selected ? true : !sel);
   TenderNcvx* cobj = NULL;
   if (sel)
   {
      TenderSNcvx* sobj = DEBUG_NEW TenderSNcvx(pdata, psize, ss);
      registerSPoly(sobj);
      cobj = sobj;
   }
   else
   {
      cobj = DEBUG_NEW TenderNcvx(pdata, psize);
   }
   _cslice->registerPoly(cobj, tpoly);
}

void TenderLay::wire (int4b* pdata, unsigned psize, word width, bool center_only, bool sel = false, const SGBitSet* ss= NULL)
{
   assert(_has_selected ? true : !sel);
   TenderWire* cobj;
   if (sel)
   {
      TenderSWire* sobj = DEBUG_NEW TenderSWire(pdata, psize, width, center_only, ss);
      registerSWire(sobj);
      cobj = sobj;
   }
   else
   {
      cobj = DEBUG_NEW TenderWire(pdata, psize, width, center_only);
   }
   _cslice->registerWire(cobj);
}

void TenderLay::text (const std::string* txt, const CTM& cmtrx)
{
//   assert(_has_selected ? true : !sel);
   //@TODO Font rendering!
}

void TenderLay::registerSBox (TenderSCnvx* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[lnes] += sobj->ssize();
      _asobjix[lnes]++;
   }
   else
   {
      _asindxs[llps] += sobj->csize();
      _asobjix[llps]++;
   }
}

void TenderLay::registerSPoly (TenderSNcvx* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[lnes] += sobj->ssize();
      _asobjix[lnes]++;
   }
   else
   {
      _asindxs[llps] += sobj->csize();
      _asobjix[llps]++;
   }
}

void TenderLay::registerSWire (TenderSWire* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[lnes] += sobj->ssize();
      _asobjix[lnes]++;
   }
   else
   {
      _asindxs[lstr] += sobj->lsize();
      _asobjix[lstr]++;
   }
}

unsigned TenderLay::total_slctdx()
{
   return ( _asindxs[lstr] +
            _asindxs[llps] +
            _asindxs[lnes]
          );
}

void TenderLay::collect(bool fill, GLuint pbuf, GLuint ibuf)
{
   int* cpoint_array = NULL;
   unsigned int* cindex_array = NULL;
   _pbuffer = pbuf;
   _ibuffer = ibuf;
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   glBufferData(GL_ARRAY_BUFFER                       ,
                2 * _num_total_points * sizeof(int4b) ,
                NULL                                  ,
                GL_DYNAMIC_DRAW                       );
   cpoint_array = (int*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
   if (0 != _ibuffer)
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER           ,
                   _num_total_indexs * sizeof(unsigned) ,
                   NULL                              ,
                   GL_DYNAMIC_DRAW                    );
      cindex_array = (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
   }
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      (*TLAY)->collect(cpoint_array, cindex_array, NULL);
   // Unmap the buffers
   glUnmapBuffer(GL_ARRAY_BUFFER);
//   glBindBuffer(GL_ARRAY_BUFFER, 0);
   if (0 != _ibuffer)
   {
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
//      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }
}

void TenderLay::collectSelected(unsigned int* slctd_array)
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
      TenderSelected* cchunk = *SSL;
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
         default: assert(false);
      }
   }
}

void TenderLay::draw(layprop::DrawProperties* drawprop)
{
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   // Check the state of the buffer
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(int4b)));
   if (0 != _ibuffer)
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
      glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
      assert(bufferSize == (GLint)(_num_total_indexs * sizeof(unsigned)));
   }
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      (*TLAY)->draw(drawprop);
   }
   for (TenderReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
   {
      (*TLAY)->draw(drawprop);
   }

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   if (0 != _ibuffer)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TenderLay::drawSelected()
{
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   // Check the state of the buffer
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   assert(bufferSize == (GLint)(2 * _num_total_points * sizeof(int4b)));

   glVertexPointer(2, GL_INT, 0, (GLvoid*)(sizeof(int4b) * _stv_array_offset));
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_INDEX_ARRAY);

   if (_asobjix[lstr] > 0)
   {
      assert(_sizslix[lstr]);
      assert(_fstslix[lstr]);
      //glMultiDrawElements(GL_LINE_STRIP, _sizslix[lstr], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lstr], _asobjix[lstr]);
      for (unsigned i= 0; i < _asobjix[lstr]; i++)
         glDrawElements(GL_LINE_STRIP, _sizslix[lstr][i], GL_UNSIGNED_INT, (const GLvoid*)_fstslix[lstr][i]);
   }
   if (_asobjix[llps] > 0)
   {
      assert(_sizslix[llps]);
      assert(_fstslix[llps]);
         //glMultiDrawElements(GL_LINE_LOOP     , _sizslix[llps], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[llps], _alobjix[llps]);
      for (unsigned i= 0; i < _asobjix[llps]; i++)
         glDrawElements(GL_LINE_LOOP, _sizslix[llps][i], GL_UNSIGNED_INT, (const GLvoid*)_fstslix[llps][i]);
   }
   if (_asobjix[lnes] > 0)
   {
      assert(_sizslix[lnes]);
      assert(_fstslix[lnes]);
         //glMultiDrawElements(GL_LINES  , _sizslix[lnes], GL_UNSIGNED_INT, (const GLvoid**)_fstslix[lnes], _alobjix[lnes]);
      for (unsigned i= 0; i < _asobjix[lnes]; i++)
         glDrawElements(GL_LINES, _sizslix[lnes][i], GL_UNSIGNED_INT, (const GLvoid*)_fstslix[lnes][i]);
   }
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);


   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

TenderLay::~TenderLay()
{
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      delete (*TLAY);

   for (TenderReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
      delete (*TLAY);

   if (NULL != _sizslix[lstr]) delete [] _sizslix[lstr];
   if (NULL != _sizslix[llps]) delete [] _sizslix[llps];
   if (NULL != _sizslix[lnes]) delete [] _sizslix[lnes];

   if (NULL != _fstslix[lstr]) delete [] _fstslix[lstr];
   if (NULL != _fstslix[llps]) delete [] _fstslix[llps];
   if (NULL != _fstslix[lnes]) delete [] _fstslix[lnes];
}
//=============================================================================
//
// class Tender0Lay
//
Tender0Lay::Tender0Lay()
{
   _alvrtxs[0] = _alvrtxs[1] = 0u;
   _alobjvx[0] = _alobjvx[1] = 0u;
   _asindxs[0] = _asindxs[1] = 0u;
   _asobjix[0] = _asobjix[1] = 0u;
   _sizesvx[0] = _sizesvx[1] = NULL;
   _firstvx[0] = _firstvx[1] = NULL;
   _sizslix[0] = _sizslix[1] = NULL;
   _fstslix[0] = _fstslix[1] = NULL;
}

TenderRef* Tender0Lay::addCellRef(std::string cname, const CTM& trans,
                                 const DBbox& overlap, bool selected, word alphaDepth)
{
   TenderRef* cRefBox = DEBUG_NEW TenderRef(cname, trans, overlap, alphaDepth);
   if (selected)
   {
      _cellSRefBoxes.push_back(cRefBox);
      assert(2 == alphaDepth);
      _asindxs[0] += 4;
      _asobjix[0]++;
   }
   else
   {
      _cellRefBoxes.push_back(cRefBox);
      if (1 < alphaDepth)
      {
         _alvrtxs[0] += 4;
         _alobjvx[0]++;
      }
   }
   return cRefBox;
}

void Tender0Lay::addTextOBox(const DBbox& overlap, const CTM& trans, bool selected)
{
   TenderOBox* tRefBox = DEBUG_NEW TenderOBox(overlap, trans);
   if (selected)
   {
      _textSRefBoxes.push_back(tRefBox);
      _asindxs[1] += 4;
      _asobjix[1]++;
   }
   else
   {
      _textRefBoxes.push_back(tRefBox);
      _alvrtxs[1] += 4;
      _alobjvx[1]++;
   }
}

unsigned Tender0Lay::total_points()
{
   return (_alvrtxs[0] + _asindxs[0] + _alvrtxs[1] + _asindxs[1]);
}

unsigned Tender0Lay::total_indexes()
{
   return (_alobjvx[0] + _asobjix[0] + _alobjvx[1] + _asobjix[1]);
}

void Tender0Lay::collect(GLuint pbuf)
{
   int* cpoint_array = NULL;
   _pbuffer = pbuf;
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   glBufferData(GL_ARRAY_BUFFER              ,
                2 * total_points() * sizeof(int4b) ,
                NULL                         ,
                GL_DYNAMIC_DRAW               );
   cpoint_array = (int*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

   // initialise the indexing
   unsigned pntindx = 0;
   unsigned  szindx  = 0;
   for (byte i = 0; i < 2; i++)
   {
      if (0 < (_alvrtxs[i] + _asindxs[i]))
      {
         _firstvx[i] = DEBUG_NEW GLsizei[_alobjvx[i] + _asobjix[i]];
         _sizesvx[i] = DEBUG_NEW GLsizei[_alobjvx[i] + _asobjix[i]];
         if (0 < _asobjix[i])
         {
            _fstslix[i] = DEBUG_NEW GLsizei[_asobjix[i]];
            _sizslix[i] = DEBUG_NEW GLsizei[_asobjix[i]];
         }
      }
   }
   // first the cells
   for (RefBoxList::const_iterator CSH = _cellRefBoxes.begin(); CSH != _cellRefBoxes.end(); CSH++)
   {
      if (1 < (*CSH)->alphaDepth())
      {
         _firstvx[0][szindx  ] = pntindx/2;
         _sizesvx[0][szindx++] = (*CSH)->cDataCopy(cpoint_array, pntindx);
      }
   }
   for (RefBoxList::const_iterator CSH = _cellSRefBoxes.begin(); CSH != _cellSRefBoxes.end(); CSH++)
   {
      _fstslix[0][szindx-_alobjvx[0]] = _firstvx[0][szindx] = pntindx/2;
      _sizslix[0][szindx-_alobjvx[0]] = _sizesvx[0][szindx] = (*CSH)->cDataCopy(cpoint_array, pntindx);
      szindx++;
   }
   // now the texts
   for (RefTxtList::const_iterator CSH = _textRefBoxes.begin(); CSH != _textRefBoxes.end(); CSH++)
   {
      _firstvx[1][szindx  ] = pntindx/2;
      _sizesvx[1][szindx++] = (*CSH)->cDataCopy(cpoint_array, pntindx);
   }
   for (RefTxtList::const_iterator CSH = _textSRefBoxes.begin(); CSH != _textSRefBoxes.end(); CSH++)
   {
      _fstslix[1][szindx-_alobjvx[1]] = _firstvx[1][szindx] = pntindx/2;
      _sizslix[1][szindx-_alobjvx[1]] = _sizesvx[1][szindx] = (*CSH)->cDataCopy(cpoint_array, pntindx);
      szindx++;
   }

   assert(pntindx == 2 * total_points());
   assert(szindx  == total_indexes());

   // Unmap the buffers
   glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Tender0Lay::draw(layprop::DrawProperties* drawprop)
{
   drawprop->setCurrentColor(0);
   // Bind the buffer
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   // Check the state of the buffer
   GLint bufferSize;
   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
   assert(bufferSize == (GLint)(2 * total_points() * sizeof(int4b)));

   glVertexPointer(2, GL_INT, 0, 0);
   glEnableClientState(GL_VERTEX_ARRAY);
   for (byte i = 0; i < 2; i++)
   {
      if (0 < (_alvrtxs[i] + _asindxs[i]))
      {
         assert(_firstvx[i]); assert(_sizesvx[i]);
         glMultiDrawArrays(GL_LINE_LOOP, _firstvx[i], _sizesvx[i], _alobjvx[i] + _asobjix[i]);
         if (0 < _asindxs[i])
         {
            assert(_fstslix[i]); assert(_sizslix[i]);
            drawprop->setLineProps(true);
            glMultiDrawArrays(GL_LINE_LOOP, _fstslix[i], _sizslix[i], _asobjix[i]);
            drawprop->setLineProps(false);
         }
      }
   }
   glDisableClientState(GL_VERTEX_ARRAY);
}

Tender0Lay::~Tender0Lay()
{
   for (byte i = 0; i < 2; i++)
   {
      if (NULL != _sizesvx[i]) delete [] (_sizesvx[i]);
      if (NULL != _firstvx[i]) delete [] (_firstvx[i]);
      if (NULL != _sizslix[i]) delete [] (_sizslix[i]);
      if (NULL != _fstslix[i]) delete [] (_fstslix[i]);
   }
   for (RefBoxList::const_iterator CSH = _cellRefBoxes.begin(); CSH != _cellRefBoxes.end(); CSH++)
      delete (*CSH);
   for (RefBoxList::const_iterator CSH = _cellSRefBoxes.begin(); CSH != _cellSRefBoxes.end(); CSH++)
      delete (*CSH);
   for (RefTxtList::const_iterator CSH = _textRefBoxes.begin(); CSH != _textRefBoxes.end(); CSH++)
      delete (*CSH);
   for (RefTxtList::const_iterator CSH = _textSRefBoxes.begin(); CSH != _textSRefBoxes.end(); CSH++)
      delete (*CSH);
}

//=============================================================================
//
// class Tenderer
//
Tenderer::Tenderer( layprop::DrawProperties* drawprop, real UU ) :
      _drawprop(drawprop), _UU(UU), _clayer(NULL),
      _cslctd_array_offset(0u), _num_ogl_buffers(0u), _ogl_buffers(NULL),
      _activeCS(NULL)
{
   // Initialise the cell (CTM) stack
   _dummyCS = DEBUG_NEW TenderRef();
   _cellStack.push(_dummyCS);
}

bool Tenderer::chunkExists(word layno, bool has_selected)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with layer 0 by accident
   assert(layno);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(layno))
   {
      _clayer = _data[layno];
      if (_clayer->chunkExists(_cellStack.top()) ) return true;
   }
   else
   {
      _clayer = DEBUG_NEW TenderLay();
      _data[layno] = _clayer;
   }
   _clayer->newSlice(_cellStack.top(), _drawprop->isFilled(layno), true, has_selected, _cslctd_array_offset);
   return false;
}

void Tenderer::setLayer(word layno, bool has_selected)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with layer 0 by accident
   assert(layno);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(layno))
   {
      _clayer = _data[layno];
   }
   else
   {
      _clayer = DEBUG_NEW TenderLay();
      _data[layno] = _clayer;
   }
   _clayer->newSlice(_cellStack.top(), _drawprop->isFilled(layno), false, has_selected, _cslctd_array_offset);
}

void Tenderer::pushCell(std::string cname, const CTM& trans, const DBbox& overlap, bool active, bool selected)
{
   TenderRef* ccellref = _0layer.addCellRef(cname,
                                           trans * _cellStack.top()->ctm(),
                                           overlap,
                                           selected,
                                           _cellStack.size()
                                          );
   _cellStack.push(ccellref );
   if (active)
   {
      assert(NULL == _activeCS);
      _activeCS = ccellref;
   }
}

void Tenderer::wire (int4b* pdata, unsigned psize, word width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM());
   _clayer->wire(pdata, psize, width, center_line_only, false, NULL);
}

void Tenderer::wire (int4b* pdata, unsigned psize, word width, const SGBitSet* psel)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM());
   _clayer->wire(pdata, psize, width, center_line_only, true, psel);
}

void Tenderer::text (const std::string* txt, const CTM& cmtrx, const DBbox& ovl, bool sel)
{
   _0layer.addTextOBox(ovl, cmtrx, sel);
   _clayer->text(txt, cmtrx);
}

void Tenderer::Grid(const real step, const std::string color)
{
   int gridstep = (int)rint(step / _UU);
   if ( abs((int)(_drawprop->ScrCTM().a() * gridstep)) > GRID_LIMIT)
   {
      _drawprop->setGridColor(color);
      // set first grid step to be multiply on the step
      TP bl = TP(_drawprop->clipRegion().p1().x(),_drawprop->clipRegion().p2().y());
      TP tr = TP(_drawprop->clipRegion().p2().x(),_drawprop->clipRegion().p1().y());
      int signX = (bl.x() > 0) ? 1 : -1;
      int X_is = (int)((rint(abs(bl.x()) / gridstep)) * gridstep * signX);
      int signY = (tr.y() > 0) ? 1 : -1;
      int Y_is = (int)((rint(abs(tr.y()) / gridstep)) * gridstep * signY);

      glEnableClientState(GL_VERTEX_ARRAY);
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
      glVertexPointer(2, GL_INT, 0, point_array);
      glDrawArrays(GL_POINTS, 0, arr_size);
      delete [] point_array;
      glDisableClientState(GL_VERTEX_ARRAY);
   }
}

void Tenderer::collect()
{
   // First filter-out the layers that doesn't have any objects on them,
   // post process the last slices in the layers and also gather the number
   // of required virtual buffers
   //
   DataLay::iterator CCLAY = _data.begin();
   unsigned num_total_slctdx = 0; // initialise the number of total selected indexes
   while (CCLAY != _data.end())
   {
      CCLAY->second->ppSlice();
      if (0 == CCLAY->second->total_points())
      {
         delete (CCLAY->second);
         // Note! Carefull here with the map iteration and erasing! Erase method
         // of map<> template doesn't return an iterator (unlike the list<>).
         // Despite the temptation to assume that the iterator will be valid after
         // the erase, it must be clear that erasing will invalidate the iterator.
         // If this is implemented more trivially using "for" cycle the code shall
         // crash, although it seems to work on certain platforms. Only seems -
         // it doesn't always crash, but it iterates in a weird way.
         // The implementation below seems to be the cleanest way to do this,
         // although it relies on my understanding of the way "++" operator should
         // be implemented
         _data.erase(CCLAY++);
      }
      else
      {
         num_total_slctdx += CCLAY->second->total_slctdx();
         _num_ogl_buffers++;
         if (0 < CCLAY->second->total_indexs())
            _num_ogl_buffers++;
         CCLAY++;
      }
   }
   if (0 < _0layer.total_points())  _num_ogl_buffers ++; // reference boxes
   if (0 < num_total_slctdx      )  _num_ogl_buffers++;  // selected
   //--------------------------------------------------------------------------
   //
   // generate all VBOs
   _ogl_buffers = DEBUG_NEW GLuint [_num_ogl_buffers];
   glGenBuffers(_num_ogl_buffers, _ogl_buffers);
   unsigned current_buffer = 0;
   //
   // collect the point arrays
   for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      assert (0 != CLAY->second->total_points());
      GLuint pbuf = _ogl_buffers[current_buffer++];
      GLuint ibuf = (0 == CLAY->second->total_indexs()) ? 0u : _ogl_buffers[current_buffer++];
      CLAY->second->collect(_drawprop->isFilled(CLAY->first), pbuf, ibuf);
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
      for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
      {
         if (0 == CLAY->second->total_slctdx()) 
            continue;
         CLAY->second->collectSelected(sindex_array);
      }
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
   }
   //
   // collect the reference boxes
   if (0 < _0layer.total_points())
   {
      GLuint pbuf = _ogl_buffers[current_buffer++];
      _0layer.collect(pbuf);
   }
   //
   // that's about it...
   checkOGLError("collect");
}

void Tenderer::draw()
{
   for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      _drawprop->setCurrentColor(CLAY->first);
      _drawprop->setCurrentFill();
      // draw everything
      CLAY->second->draw(_drawprop);

      if (0 != CLAY->second->total_slctdx())
      {// redraw selected contours only
         _drawprop->setLineProps(true);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _sbuffer);
         glPushMatrix();
         glMultMatrixd(_activeCS->translation());
         CLAY->second->drawSelected(  );
         glPopMatrix();
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
         _drawprop->setLineProps(false);
      }
   }
   // draw reference boxes
   if (0 < _0layer.total_points())   _0layer.draw(_drawprop);
   // Clean-up the buffers
   //glBindBuffer(GL_ARRAY_BUFFER, 0);
   //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDeleteBuffers(_num_ogl_buffers, _ogl_buffers);
   delete [] _ogl_buffers;
   _ogl_buffers = NULL;
   checkOGLError("draw");
}

Tenderer::~Tenderer()
{
   char debug_message[256];
   unsigned long all_points_drawn = 0;
   unsigned      all_layers = 0;
   for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      all_points_drawn += CLAY->second->total_points();
      all_layers++;
//      sprintf (debug_message, "Layer %i:  %i points", CLAY->first, CLAY->second->total_points());
//      tell_log(console::MT_INFO,debug_message);
      delete (CLAY->second);
   }
   sprintf (debug_message, "Rendering summary: %lu vertexes in %i buffers", all_points_drawn, all_layers);
   tell_log(console::MT_WARNING,debug_message);
   // Don't clear the _cellStack contents. All references there are references in
   // _0layer and will be cleared there. The only exception is the first dummy cell.
   delete _dummyCS;
}

void checkOGLError(std::string loc)
{
   std::ostringstream boza;
   GLenum ogle;
   while ((ogle=glGetError()) != GL_NO_ERROR)
   {
      boza  << "Error " << ogle << "at " << loc << std::endl;
      tell_log(console::MT_ERROR,boza.str());
   }
}

//=============================================================================
//
// class HiResTimer (Profiling timer for debugging purposes)
//
HiResTimer::HiResTimer()
{
#ifdef WIN32
   // Get system frequency (number of ticks per second) of timer
   if (!QueryPerformanceFrequency(&_freq) || !QueryPerformanceCounter(&_inittime))
   {
      tell_log(console::MT_INFO,"Problem with timer");
   }
#else
   gettimeofday(&_start_time, NULL);
#endif
}

void HiResTimer::report(std::string message)
{
   char time_message[256];
#ifdef WIN32
   LARGE_INTEGER curtime;
   if (!QueryPerformanceCounter(&curtime))
      return ;
  // Convert number of ticks to milliseconds
   int millisec = (curtime.QuadPart-_inittime.QuadPart) / (_freq.QuadPart / 1000);
   int sec = millisec / 1000;
   millisec = millisec - sec * 1000;
   sprintf (time_message, "%s:   %i sec. %06i msec.",message.c_str(), sec, millisec);

#else

   gettimeofday(&_end_time, NULL);
   timeval result;
   result.tv_sec = _end_time.tv_sec - _start_time.tv_sec;
   result.tv_usec = _end_time.tv_usec - _start_time.tv_usec;
   if (result.tv_usec < 0)
   {
      result.tv_sec -= 1;
      result.tv_usec += 1000000;
   }
   sprintf (time_message, "%s:   %li sec. %06li msec.",message.c_str(), result.tv_sec, result.tv_usec);
#endif
   tell_log(console::MT_INFO,time_message);
}
