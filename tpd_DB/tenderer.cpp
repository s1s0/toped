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
#include "tedat.h"


GLUtriangulatorObj*          TessellPoly::tenderTesel = NULL;
extern layprop::FontLibrary* fontLib;

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

TeselChunk::TeselChunk(const TeselChunk& tcobj)
{
   _size = tcobj._size;
   _type = tcobj._type;
   _index_seq = DEBUG_NEW unsigned[_size];
   memcpy(_index_seq, tcobj._index_seq, sizeof(unsigned) * _size);
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
   _the_chain->push_back(TeselChunk(_cindexes, _ctype, _offset));
   switch (_ctype)
   {
      case GL_TRIANGLE_FAN   : _all_ftfs++; break;
      case GL_TRIANGLE_STRIP : _all_ftss++; break;
      case GL_TRIANGLES      : _all_ftrs++; break;
      default: assert(0);break;
   }
}

//=============================================================================
// TessellPoly

TessellPoly::TessellPoly() : _tdata(), _all_ftrs(0), _all_ftfs(0), _all_ftss(0)
{
}

void TessellPoly::tessellate(const int4b* pdata, unsigned psize)
{
   _tdata.clear();
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


GLvoid TessellPoly::teselBegin(GLenum type, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newChunk(type);
}

GLvoid TessellPoly::teselVertex(GLvoid *pindex, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newIndex(*(static_cast<word*>(pindex)));
}

GLvoid TessellPoly::teselEnd(GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->storeChunk();
}

void TessellPoly::num_indexs(unsigned& iftrs, unsigned& iftfs, unsigned& iftss) const
{
   for (TeselChain::const_iterator CCH = _tdata.begin(); CCH != _tdata.end(); CCH++)
   {
      switch (CCH->type())
      {
         case GL_TRIANGLE_FAN   : iftfs += CCH->size(); break;
         case GL_TRIANGLE_STRIP : iftss += CCH->size(); break;
         case GL_TRIANGLES      : iftrs += CCH->size(); break;
         default: assert(0);break;
      }
   }
}

//=============================================================================
//
// TenderCnvx
//
unsigned tenderer::TenderCnvx::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   assert(_csize);
#ifdef TENDERER_USE_FLOATS
   for (unsigned i = 0; i < 2 * _csize; i++)
      array[pindex+i] = (TNDR_GLDATAT) _cdata[i];
#else
   memcpy(&(array[pindex]), _cdata, 2 * sizeof(TNDR_GLDATAT) * _csize);
#endif
   pindex += 2 * _csize;
   return _csize;
}

//=============================================================================
//
// TenderBox
//
unsigned  tenderer::TenderBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   assert(_csize);
   array[pindex++] = (TNDR_GLDATAT)_cdata[0];array[pindex++] = (TNDR_GLDATAT)_cdata[1];
   array[pindex++] = (TNDR_GLDATAT)_cdata[2];array[pindex++] = (TNDR_GLDATAT)_cdata[1];
   array[pindex++] = (TNDR_GLDATAT)_cdata[2];array[pindex++] = (TNDR_GLDATAT)_cdata[3];
   array[pindex++] = (TNDR_GLDATAT)_cdata[0];array[pindex++] = (TNDR_GLDATAT)_cdata[3];
   return _csize;
}

//=============================================================================
//
// TenderWire
//
tenderer::TenderWire::TenderWire(int4b* pdata, unsigned psize, const WireWidth width, bool clo)
   : TenderNcvx(NULL, 0), _ldata(pdata), _lsize(psize), _celno(clo), _tdata(NULL)
{
   if (!_celno)
   {
      laydata::WireContour wcontour(_ldata, _lsize, width);
      _csize = wcontour.csize();
      // this intermediate variable is just to deal with the const-ness of _cdata;
      int4b* contData = DEBUG_NEW int4b[ 2 * _csize];
      wcontour.getArrayData(contData);
      _cdata = contData;
   }
}

unsigned tenderer::TenderWire::lDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   assert(_lsize);
#ifdef TENDERER_USE_FLOATS
   for (unsigned i = 0; i < 2 * _lsize; i++)
      array[pindex+i] = (TNDR_GLDATAT) _ldata[i];
#else
   memcpy(&(array[pindex]), _ldata, 2 * sizeof(TNDR_GLDATAT) * _lsize);
#endif
   pindex += 2 * _lsize;
   return _lsize;
}

/** For wire tessellation we can use the common polygon tessellation procedure.
    This could be a huge overhead though. The thing is that we've
    already been trough the precalc procedure and we know that wire object is
    very specific non-convex polygon. Using this knowledge the tessallation
    is getting really trivial. All we have to do is to list the contour points
    indexes in pairs - one from the front, and the other from the back of the
    array. Then this can be drawn as GL_QUAD_STRIP
*/
void tenderer::TenderWire::Tesselate()
{
   _tdata = DEBUG_NEW TeselChain();
   _tdata->push_back( TeselChunk(_cdata, _csize, 0));
}

tenderer::TenderWire::~TenderWire()
{
   if (NULL != _cdata) delete [] _cdata;
   if (NULL != _tdata) delete _tdata;
}

//=============================================================================
//
// TextSOvlBox
//
unsigned tenderer::TextSOvlBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TextOvlBox::cDataCopy(array, pindex);
}

unsigned tenderer::TextSOvlBox::sDataCopy(unsigned* array, unsigned& pindex)
{
   assert (NULL == _slist);
   for (unsigned i = 0; i < 4; i++)
      array[pindex++] = _offset + i;
   return ssize();
}

//=============================================================================
//
// TenderSCnvx
//
unsigned tenderer::TenderSCnvx::ssize()
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

unsigned tenderer::TenderSCnvx::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderCnvx::cDataCopy(array, pindex);
}

unsigned tenderer::TenderSCnvx::sDataCopy(unsigned* array, unsigned& pindex)
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
// TenderSBox
//
unsigned tenderer::TenderSBox::ssize()
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

unsigned tenderer::TenderSBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderBox::cDataCopy(array, pindex);
}

unsigned tenderer::TenderSBox::sDataCopy(unsigned* array, unsigned& pindex)
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
unsigned tenderer::TenderSNcvx::ssize()
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

unsigned tenderer::TenderSNcvx::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderCnvx::cDataCopy(array, pindex);
}

unsigned tenderer::TenderSNcvx::sDataCopy(unsigned* array, unsigned& pindex)
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
unsigned tenderer::TenderSWire::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TenderCnvx::cDataCopy(array, pindex);
}

unsigned tenderer::TenderSWire::lDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _loffset = pindex/2;
   return TenderWire::lDataCopy(array, pindex);
}

unsigned tenderer::TenderSWire::ssize()
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
   // the rules when the selected objects were fitted into the whole TopRend
   // class hierarchy.
   if (!_celno)
   {
      if (_slist->check(0)         ) ssegs +=2;
      if (_slist->check(_lsize-1)  ) ssegs +=2;
   }
   return ssegs;
}

unsigned tenderer::TenderSWire::sDataCopy(unsigned* array, unsigned& pindex)
{
   if (NULL != _slist)
   { // shape is partially selected
      // copy the indexes of the selected segment points
      for (unsigned i = 0; i < _lsize - 1; i++)
      {
         if (_slist->check(i) && _slist->check(i+1))
         {
            array[pindex++] = _loffset + i;
            array[pindex++] = _loffset + i + 1;
         }
      }
      if (!_celno)
      {
         // And the edge points!
         if (_slist->check(0)       ) // if first point is selected
         {
            array[pindex++] = _offset + (_csize/2) -1;
            array[pindex++] = _offset + (_csize/2);
         }
         if (_slist->check(_lsize-1))// if last point is selected
         {
            array[pindex++] = _offset;
            array[pindex++] = _offset + _csize -1;
         }
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
// class TextOvlBox
//
tenderer::TextOvlBox::TextOvlBox(const DBbox& obox, const CTM& ctm)
{
   // Don't get confused here! It's not a stupid code. Think about
   // boxes rotated to 45 deg for example and you'll see why
   // obox * ctm is not possible
   TP tp = TP(obox.p1().x(), obox.p1().y()) * ctm;
   _obox[0] = tp.x();_obox[1] = tp.y();
   tp = TP(obox.p2().x(), obox.p1().y()) * ctm;
   _obox[2] = tp.x();_obox[3] = tp.y();
   tp = TP(obox.p2().x(), obox.p2().y()) * ctm;
   _obox[4] = tp.x();_obox[5] = tp.y();
   tp = TP(obox.p1().x(), obox.p2().y()) * ctm;
   _obox[6] = tp.x();_obox[7] = tp.y();
}

unsigned tenderer::TextOvlBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
#ifdef TENDERER_USE_FLOATS
   for (unsigned i = 0; i <  8; i++)
      array[pindex+i] = (TNDR_GLDATAT) _obox[i];
#else
   memcpy(&(array[pindex]), _obox, sizeof(TNDR_GLDATAT) * 8);
#endif
   pindex += 8;
   return 4;
}

//=============================================================================
//
// class TenderRef
//
tenderer::TenderRef::TenderRef(std::string name, const CTM& ctm, const DBbox& obox,
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


tenderer::TenderRef::TenderRef() : _name(""), _ctm(CTM()), _alphaDepth(0)
{
   _ctm.oglForm(_translation);
   for (word i = 0; i < 8; _obox[i++] = 0);
}

unsigned tenderer::TenderRef::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
#ifdef TENDERER_USE_FLOATS
   for (unsigned i = 0; i <  8; i++)
      array[pindex+i] = (TNDR_GLDATAT) _obox[i];
#else
   memcpy(&(array[pindex]), _obox, sizeof(TNDR_GLDATAT) * 8);
#endif
   pindex += 8;
   return 4;
}

//=============================================================================
//
// class TenderText
//
tenderer::TenderText::TenderText(const std::string* text, const CTM& ctm) : _text(text)
{
   ctm.oglForm(_ftm);
}

void tenderer::TenderText::draw(bool fill)
{
   glPushMatrix();
   glMultMatrixd(_ftm);
   glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
   fontLib->drawString(_text, fill);
   glPopMatrix();
}

//=============================================================================
//
// class TenderTV
//
tenderer::TenderTV::TenderTV(TenderRef* const refCell, bool filled, bool reusable,
                   unsigned parray_offset, unsigned iarray_offset) :
   _refCell             ( refCell         ),
   _point_array_offset  ( parray_offset   ),
   _index_array_offset  ( iarray_offset   ),
   _num_total_strings   ( 0u              ),
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

void tenderer::TenderTV::registerBox (TenderCnvx* cobj)
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

void tenderer::TenderTV::registerPoly (TenderNcvx* cobj, const TessellPoly* tchain)
{
   unsigned allpoints = cobj->csize();
   if (_filled && tchain && tchain->valid())
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

void tenderer::TenderTV::registerWire (TenderWire* cobj)
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
          _alobjvx[cont] ++;
          _alvrtxs[cont] += allpoints;
       }
   }
}

void tenderer::TenderTV::registerText (TenderText* cobj, TextOvlBox* oobj)
{
   _text_data.push_back(cobj);
   _num_total_strings++;
   if (NULL != oobj)
   {
      _txto_data.push_back(oobj);
      _alvrtxs[cont] += 4;
      _alobjvx[cont]++;
   }
}

unsigned tenderer::TenderTV::num_total_points()
{
   return ( _alvrtxs[cont] +
            _alvrtxs[line] +
            _alvrtxs[cnvx] +
            _alvrtxs[ncvx]
          );
}

unsigned tenderer::TenderTV::num_total_indexs()
{
   return ( _alindxs[fqss] +
            _alindxs[ftrs] +
            _alindxs[ftfs] +
            _alindxs[ftss]
          );
}

void tenderer::TenderTV::collectIndexs(unsigned int* index_array, const TeselChain* tdata, unsigned* size_index,
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

void tenderer::TenderTV::collect(TNDR_GLDATAT* point_array, unsigned int* index_array, unsigned int*)
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

void tenderer::TenderTV::draw(layprop::DrawProperties* drawprop)
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

void tenderer::TenderTV::drawTexts(layprop::DrawProperties* drawprop)
{
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   drawprop->adjustAlpha(_refCell->alphaDepth() - 1);

   for (TenderStrings::const_iterator TSTR = _text_data.begin(); TSTR != _text_data.end(); TSTR++)
      (*TSTR)->draw(_filled);

   glPopMatrix();
}

tenderer::TenderRef* tenderer::TenderTV::swapRefCells(TenderRef* newRefCell)
{
   TenderRef* the_swap = _refCell;
   _refCell = newRefCell;
   return the_swap;
}

tenderer::TenderTV::~TenderTV()
{
   for (SliceWires::const_iterator CSO = _line_data.begin(); CSO != _line_data.end(); CSO++)
      if ((*CSO)->center_line_only()) delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cnvx_data.begin(); CSO != _cnvx_data.end(); CSO++)
      delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cont_data.begin(); CSO != _cont_data.end(); CSO++)
      delete (*CSO);
   for (SlicePolygons::const_iterator CSO = _ncvx_data.begin(); CSO != _ncvx_data.end(); CSO++)
      delete (*CSO);
   for (TenderStrings::const_iterator CSO = _text_data.begin(); CSO != _text_data.end(); CSO++)
      delete (*CSO);
   for (RefTxtList::const_iterator CSO = _txto_data.begin(); CSO != _txto_data.end(); CSO++)
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

//=============================================================================
//
// class TenderReTV
//
void tenderer::TenderReTV::draw(layprop::DrawProperties* drawprop)
{
   TenderRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->draw(drawprop);
   _chunk->swapRefCells(sref_cell);
}

void tenderer::TenderReTV::drawTexts(layprop::DrawProperties* drawprop)
{
   TenderRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->drawTexts(drawprop);
   _chunk->swapRefCells(sref_cell);
}

//=============================================================================
//
// class TenderLay
//
tenderer::TenderLay::TenderLay(): _cslice(NULL),
   _num_total_points(0u), _num_total_indexs(0u), _num_total_strings(0u),
   _stv_array_offset(0u), _slctd_array_offset(0u)
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
 * Create a new object of TenderTV type which will be referred to by _cslice
 * @param ctrans Current translation matrix of the new object
 * @param fill Whether to fill the drawing objects
 */
void tenderer::TenderLay::newSlice(TenderRef* const ctrans, bool fill, bool reusable, unsigned slctd_array_offset)
{
   assert( 0 == total_slctdx());
   _slctd_array_offset = slctd_array_offset;
   _stv_array_offset = 2 * _num_total_points;
   newSlice(ctrans, fill, reusable);
}

void tenderer::TenderLay::newSlice(TenderRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW TenderTV(ctrans, fill, reusable, 2 * _num_total_points, _num_total_indexs);
}

bool tenderer::TenderLay::chunkExists(TenderRef* const ctrans, bool filled)
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

/** Add the current slice object (_cslice) to the list of slices _layData but
only if it's not empty. Also track the total number of vertexes in the layer
*/
void tenderer::TenderLay::ppSlice()
{
   if (NULL != _cslice)
   {
      unsigned num_points  = _cslice->num_total_points();
      unsigned num_strings = _cslice->num_total_strings();
      if ((num_points > 0) || (num_strings > 0))
      {
         _layData.push_back(_cslice);
         _num_total_points  += num_points;
         _num_total_strings += num_strings;
         _num_total_indexs  += _cslice->num_total_indexs();
         if (_cslice->reusable())
         {
            if (_cslice->filled())
            {
               assert(_reusableFData.end() == _reusableFData.find(_cslice->cellName()));
               _reusableFData[_cslice->cellName()] = _cslice;
            }
            else
            {
               assert(_reusableCData.end() == _reusableCData.find(_cslice->cellName()));
               _reusableCData[_cslice->cellName()] = _cslice;
            }
         }
      }
      else
         delete _cslice;
      _cslice = NULL;
   }
}

void tenderer::TenderLay::box  (const int4b* pdata)
{
   _cslice->registerBox(DEBUG_NEW TenderBox(pdata));
}

void tenderer::TenderLay::box (const int4b* pdata, const SGBitSet* ss)
{
   TenderSBox* sobj = DEBUG_NEW TenderSBox(pdata, ss);
   registerSBox(sobj);
   _cslice->registerBox(sobj);
}

void tenderer::TenderLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly)
{
   _cslice->registerPoly(DEBUG_NEW TenderNcvx(pdata, psize), tpoly);
}

void tenderer::TenderLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss)
{
   TenderSNcvx* sobj = DEBUG_NEW TenderSNcvx(pdata, psize, ss);
   registerSPoly(sobj);
   _cslice->registerPoly(sobj, tpoly);
}

void tenderer::TenderLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool center_only)
{
   _cslice->registerWire(DEBUG_NEW TenderWire(pdata, psize, width, center_only));
}

void tenderer::TenderLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool center_only, const SGBitSet* ss)
{
   TenderSWire* sobj = DEBUG_NEW TenderSWire(pdata, psize, width, center_only, ss);
   registerSWire(sobj);
   _cslice->registerWire(sobj);
}

void tenderer::TenderLay::text (const std::string* txt, const CTM& ftmtrx, const DBbox* ovl, const TP& cor, bool sel)
{
   // Make sure that selected shapes don't come unexpected
   TextOvlBox* cobj = NULL;
   if (sel)
   {
      assert(ovl);
      TextSOvlBox* sobj = DEBUG_NEW TextSOvlBox((*ovl) , ftmtrx);
      registerSOBox(sobj);
      cobj = sobj;
   }
   else if (ovl)
   {
      cobj = DEBUG_NEW TextOvlBox((*ovl) , ftmtrx);
   }

   CTM ftm(ftmtrx.a(), ftmtrx.b(), ftmtrx.c(), ftmtrx.d(), 0, 0);
   ftm.Translate(cor * ftmtrx);
   _cslice->registerText(DEBUG_NEW TenderText(txt, ftm), cobj);
}


void tenderer::TenderLay::registerSBox (TenderSBox* sobj)
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

void tenderer::TenderLay::registerSOBox (TextSOvlBox* sobj)
{
   _slct_data.push_back(sobj);
   _asindxs[llps] += 4;
   _asobjix[llps]++;
}


void tenderer::TenderLay::registerSPoly (TenderSNcvx* sobj)
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

void tenderer::TenderLay::registerSWire (TenderSWire* sobj)
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

unsigned tenderer::TenderLay::total_slctdx()
{
   return ( _asindxs[lstr] +
            _asindxs[llps] +
            _asindxs[lnes]
          );
}

void tenderer::TenderLay::collect(bool fill, GLuint pbuf, GLuint ibuf)
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

void tenderer::TenderLay::collectSelected(unsigned int* slctd_array)
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
         default: assert(false);break;
      }
   }
}

void tenderer::TenderLay::draw(layprop::DrawProperties* drawprop)
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

void tenderer::TenderLay::drawSelected()
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

void tenderer::TenderLay::drawTexts(layprop::DrawProperties* drawprop)
{
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      (*TLAY)->drawTexts(drawprop);
   }
   for (TenderReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
   {
      (*TLAY)->drawTexts(drawprop);
   }
}

tenderer::TenderLay::~TenderLay()
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
// class TenderRefLay
//
tenderer::TenderRefLay::TenderRefLay()
{
   _alvrtxs = 0u;
   _alobjvx = 0u;
   _asindxs = 0u;
   _asobjix = 0u;
   _sizesvx = NULL;
   _firstvx = NULL;
   _sizslix = NULL;
   _fstslix = NULL;
}

void tenderer::TenderRefLay::addCellOBox(TenderRef* cRefBox, word alphaDepth, bool selected)
{
   if (selected)
   {
      _cellSRefBoxes.push_back(cRefBox);
//      assert(2 == alphaDepth); <-- @TODO (Alpha depth dependent rendering). Why this is hit during edit in place?
      _asindxs += 4;
      _asobjix++;
   }
   else
   {
      _cellRefBoxes.push_back(cRefBox);
      if (1 < alphaDepth)
      {
         // The meaning of this condition is to prevent rendering of the overlapping box of the
         // top visible cell. Only the top visible cell has alphaDepth parameter equals to 1
         _alvrtxs += 4;
         _alobjvx++;
      }
   }
}

unsigned tenderer::TenderRefLay::total_points()
{
   return (_alvrtxs + _asindxs);
}

unsigned tenderer::TenderRefLay::total_indexes()
{
   return (_alobjvx + _asobjix);
}

void tenderer::TenderRefLay::collect(GLuint pbuf)
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

void tenderer::TenderRefLay::draw(layprop::DrawProperties* drawprop)
{
   drawprop->setCurrentColor(REF_LAY_DEF);
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

tenderer::TenderRefLay::~TenderRefLay()
{
   if (NULL != _sizesvx) delete [] (_sizesvx);
   if (NULL != _firstvx) delete [] (_firstvx);
   if (NULL != _sizslix) delete [] (_sizslix);
   if (NULL != _fstslix) delete [] (_fstslix);
   for (RefBoxList::const_iterator CSH = _cellRefBoxes.begin(); CSH != _cellRefBoxes.end(); CSH++)
      delete (*CSH);
   for (RefBoxList::const_iterator CSH = _cellSRefBoxes.begin(); CSH != _cellSRefBoxes.end(); CSH++)
      delete (*CSH);
}

//=============================================================================
//
// class TopRend
//
tenderer::TopRend::TopRend( layprop::DrawProperties* drawprop, real UU ) :
      _drawprop(drawprop), _UU(UU), _clayer(NULL), _grcLayer(NULL),
      _cslctd_array_offset(0u), _num_ogl_buffers(0u), _num_ogl_grc_buffers(0u),
      _ogl_buffers(NULL), _ogl_grc_buffers(NULL),
      _activeCS(NULL), _dovCorrection(0)
{
   // Initialize the cell (CTM) stack
   _cellStack.push(DEBUG_NEW TenderRef());
}

bool tenderer::TopRend::chunkExists(const LayerDef& laydef, bool has_selected)
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

void tenderer::TopRend::setLayer(const LayerDef& laydef, bool has_selected)
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

void tenderer::TopRend::pushCell(std::string cname, const CTM& trans, const DBbox& overlap, bool active, bool selected)
{
   TenderRef* cRefBox = DEBUG_NEW TenderRef(cname,
                                            trans * _cellStack.top()->ctm(),
                                            overlap,
                                            _cellStack.size()
                                           );
   if (selected || (!_drawprop->isCellBoxHidden()))
      _refLayer.addCellOBox(cRefBox, _cellStack.size(), selected);
   else
      // This list is to keep track of the hidden cRefBox - so we can clean
      // them up. Don't get confused - we need cRefBox during the collecting
      // and drawing phase so we can't really delete them here or after they're
      // poped-up from _cellStack. The confusion is coming from the "duality"
      // of the TenderRef - once as a cell reference with CTM, view depth etc.
      // and then as a placeholder of the overlapping reference box
      _hiddenRefBoxes.push_back(cRefBox);

   _cellStack.push(cRefBox);
   if (active)
   {
      assert(NULL == _activeCS);
      _activeCS = cRefBox;
   }
}

void tenderer::TopRend::grcpoly(int4b* pdata, unsigned psize)
{
   assert(_grcLayer);
   _grcLayer->poly(pdata, psize, NULL);
}

void tenderer::TopRend::wire (int4b* pdata, unsigned psize, WireWidth width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM(), visualLimit());
   _clayer->wire(pdata, psize, width, center_line_only);
}

void tenderer::TopRend::wire (int4b* pdata, unsigned psize, WireWidth width, const SGBitSet* psel)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM(), visualLimit());
   _clayer->wire(pdata, psize, width, center_line_only,psel);
}

void tenderer::TopRend::grcwire (int4b* pdata, unsigned psize, WireWidth width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM(), visualLimit());
   _grcLayer->wire(pdata, psize, width, center_line_only);
}

void tenderer::TopRend::arefOBox(std::string cname, const CTM& trans, const DBbox& overlap, bool selected)
{
   if (selected || (!_drawprop->isCellBoxHidden()))
   {
      TenderRef* cRefBox = DEBUG_NEW TenderRef(cname,
                                               trans * _cellStack.top()->ctm(),
                                               overlap,
                                               _cellStack.size()
                                              );
      _refLayer.addCellOBox(cRefBox, _cellStack.size(), selected);
   }
}

void tenderer::TopRend::text (const std::string* txt, const CTM& ftmtrx, const DBbox& ovl, const TP& cor, bool sel)
{
   if (sel)
      _clayer->text(txt, ftmtrx, &ovl, cor, true);
   else if (_drawprop->isTextBoxHidden())
      _clayer->text(txt, ftmtrx, NULL, cor, false);
   else
      _clayer->text(txt, ftmtrx, &ovl, cor, false);
}

void tenderer::TopRend::Grid(const real step, const std::string color)
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

bool tenderer::TopRend::collect()
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
   if (0 < _refLayer.total_points())  _num_ogl_buffers ++; // reference boxes
   if (0 < num_total_slctdx      )  _num_ogl_buffers++;  // selected
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
   if (0 < _refLayer.total_points())
   {
      GLuint pbuf = _ogl_buffers[current_buffer++];
      _refLayer.collect(pbuf);
   }
   //
   // that's about it...
   checkOGLError("collect");
   return true;
}

bool tenderer::TopRend::grcCollect()
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

void tenderer::TopRend::draw()
{
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      _drawprop->setCurrentColor(CLAY());
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
   if (0 < _refLayer.total_points())   _refLayer.draw(_drawprop);
   checkOGLError("draw");
}

void tenderer::TopRend::grcDraw()
{
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {// for every layer
      _drawprop->setCurrentColor(CLAY());
      _drawprop->setCurrentFill(true); // force fill (ignore block_fill state)
      _drawprop->setLineProps(false);
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
   }
   checkOGLError("grcDraw");
}

void tenderer::TopRend::cleanUp()
{
   // Clean-up the buffers
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void tenderer::TopRend::grcCleanUp()
{
   // Clean-up the buffers
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void tenderer::TopRend::setGrcLayer(bool setEData, const LayerDef& laydef)
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

LayerDef tenderer::TopRend::getTenderLay(const LayerDef& laydef)
{
   return _drawprop->getTenderLay(laydef);
}

bool tenderer::TopRend::preCheckCRS(const laydata::TdtCellRef* ref, layprop::CellRefChainType& crchain)
{
   crchain = _drawprop->preCheckCRS(ref);
   byte dovLimit = _drawprop->cellDepthView();
   if (0 == dovLimit) return true;
   switch (crchain)
   {
      case layprop::crc_VIEW:
         return (_cellStack.size() <= _drawprop->cellDepthView());
      case layprop::crc_POSTACTIVE:
         return ((_cellStack.size() - _dovCorrection) < _drawprop->cellDepthView());
      case layprop::crc_ACTIVE:
         _dovCorrection = _cellStack.size(); return true;
      default: return true;
   }
   return true;// Dummy statement - to prevent compiler warnings
}

tenderer::TopRend::~TopRend()
{
//   char debug_message[256];
//   unsigned long all_points_drawn = 0;
//   unsigned      allLayers = 0;
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
//      all_points_drawn += CLAY->second->total_points();
//      allLayers++;
//      sprintf (debug_message, "Layer %i:  %i points", CLAY->first, CLAY->second->total_points());
//      tell_log(console::MT_INFO,debug_message);
      delete (*CLAY);
   }
   //
   assert(1 == _cellStack.size());
   delete (_cellStack.top()); _cellStack.pop();
   for (RefBoxList::const_iterator CSH = _hiddenRefBoxes.begin(); CSH != _hiddenRefBoxes.end(); CSH++)
      delete (*CSH);

   //
//   sprintf (debug_message, "Rendering summary: %lu vertexes in %i buffers", all_points_drawn, allLayers);
//   tell_log(console::MT_WARNING,debug_message);
   //
   if (NULL != _ogl_buffers)
   {
      glDeleteBuffers(_num_ogl_buffers, _ogl_buffers);
      delete [] _ogl_buffers;
      _ogl_buffers = NULL;
   }
   // GRC clean-up
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
      delete (*CLAY);
   if (NULL != _ogl_grc_buffers)
   {
      glDeleteBuffers(_num_ogl_grc_buffers, _ogl_grc_buffers);
      delete [] _ogl_grc_buffers;
      _ogl_grc_buffers = NULL;
   }
}

void tenderer::checkOGLError(std::string loc)
{
   std::ostringstream ost;
   GLenum ogle;
   while ((ogle=glGetError()) != GL_NO_ERROR)
   {
      ost << "OpenGL Error: \"" << gluErrorString(ogle)
          << "\" during " << loc;
      tell_log(console::MT_ERROR,ost.str());
   }
}
