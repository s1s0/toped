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
//        Created: Wed Sep 12 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Base class for all openGL renderers
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "basetrend.h"


GLUtriangulatorObj*          TessellPoly::tenderTesel = NULL;

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

