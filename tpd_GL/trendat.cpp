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
//        Created: Fri Dec 28 GMT 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Toped Rendering verteX data
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "trendat.h"
#include "trend.h"


GLUtriangulatorObj*  TessellPoly_old::tenderTesel = NULL;
GLUtriangulatorObj*  TessellPoly::tenderTesel = NULL;
extern trend::TrendCenter*            TRENDC;

GLubyte ref_mark_bmp[30] = {
   0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x18, 0x18,
   0x24, 0x48, 0x42, 0x84, 0x81, 0x02, 0x42, 0x84, 0x24, 0x48,
   0x18, 0x18, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00
};

GLubyte text_mark_bmp[30] = {
   0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x09, 0x20, 0x11, 0x10,
   0x21, 0x08, 0x41, 0x04, 0x8F, 0xE2, 0x40, 0x04, 0x20, 0x08,
   0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00
};

GLubyte aref_mark_bmp[30]= {
   0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10,
   0x20, 0x08, 0x50, 0x0A, 0x8F, 0xE2, 0x44, 0x44, 0x22, 0x88,
   0x11, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00
};



//=============================================================================
// TessEarClip

//ECVertex::ECVertex(word iP, word iLast, const PointVector& pv)
//{
//   _P = pv[iP];
//   switch(iP)
//   {
//      case     0: _pP = pv[iLast]  ; _nP = pv[iP+1]; break;
//      case iLast: _pP = pv[iLast-1]; _nP = pv[0]   ; break;
//      default   : _pP = pv[iP-1]   ; _nP = pv[iP+1]; break;
//   }
//   _angle = laydata::xangle(_pP,_P) - laydata::xangle(_P,_nP);
//}

ECVertex::ECVertex(word  pidx, word idx, word nidx, const PointVector& pv) :
  _idx  (idx)
, _pidx (pidx)
, _nidx (nidx)
, _pv   (pv)
, _angle(0)
, _ecGood(false)
{
   WordSet dummy;// the list of already clipped vertices is emty at this stage
   update(dummy);
}

void ECVertex::update(const WordSet& clipped)
{
   _angle = xangle(_pv[_idx],_pv[_nidx]) - xangle(_pv[_pidx],_pv[_idx]);
   if      (_angle >  180) _angle -= 360;
   else if (_angle < -180) _angle += 360;
   // add the vertexes of this angle to the list of already clipped vertexes
   WordSet tbExcluded = clipped;
//   WordSet tbExcluded = {_pidx, _idx, _nidx};
//   tbExcluded.merge(clipped);
   tbExcluded.insert(_pidx);
   tbExcluded.insert(_idx);
   tbExcluded.insert(_nidx);
   
   checkClipable(tbExcluded);
}

void ECVertex::checkClipable(const WordSet& excluded)
{
   if (0 > _angle) return; // i.e. angle is > 180 degrees, this vertex can NOT be clipped!
   bool overlappedVertex = false;
   // check whether each point of _pv is eventually overlapped
   for (word i = 0; i < _pv.size(); i++)
   {
      if (0==excluded.count(i))// i.e. current vertex index (i) is not to be excluded from the check from the check
//      if (abs(_idx - i) > 1) // don't check vetices belonging to this object
         overlappedVertex |= internalPoint(i);
//      else continue;
   }
   _ecGood = !overlappedVertex;
}

// checks whether the point p is overlapped by the triangle constituted
// by the points of this object - i.e. with vertex indexes _pP, _P, _nP
bool ECVertex::internalPoint(word p)
{
   //reference:https://math.stackexchange.com/questions/51326/determining-if-an-arbitrary-point-lies-inside-a-triangle-defined-by-three-points/4624564#4624564

   //   const AP = { x: point.x - a.x, y: point.y - a.y };
   //   const AB = { x: (b.x - a.x), y: (b.y - a.y) };
   //   const thirdTermABxAPisPositive = AB.x * AP.y - AB.y * AP.x > 0;
   const TP AP(  _pv[p].x()-_pv[_idx].x() ,   _pv[p].y()-_pv[_idx].y());
   const TP AB(_pv[_nidx].x()-_pv[_idx].x() , _pv[_nidx].y()-_pv[_idx].y());
   const bool thirdTermABxAPisPositive = (AB.x() * AP.y() - AB.y() * AP.x()) > 0;
   //   const BC = { x: (c.x - b.x), y: (c.y - b.y) };
   //   const BP = { x: (point.x - b.x), y: (point.y - b.y) };
   //   const thirdTermBCxBPisPositive = BC.x * BP.y - BC.y * BP.x > 0;
   const TP BC(_pv[_pidx].x()-_pv[_nidx].x() , _pv[_pidx].y()-_pv[_nidx].y());
   const TP BP(  _pv[p].x()-_pv[_nidx].x() ,   _pv[p].y()-_pv[_nidx].y());
   const bool thirdTermBCxBPisPositive = (BC.x() * BP.y() - BC.y() * BP.x()) > 0;

   if (thirdTermBCxBPisPositive != thirdTermABxAPisPositive)
       return false;

   //   const CA = { x: (a.x - c.x), y: (a.y - c.y) };
   //   const CP = { x: (point.x - c.x), y: (point.y - c.y) };
   //   const thirdTermCAxCPisPositive = CA.x * CP.y - CA.y * CP.x > 0;
   const TP CA(_pv[_idx].x()-_pv[_pidx].x() , _pv[_idx].y()-_pv[_pidx].y());
   const TP CP( _pv[p].x()-_pv[_pidx].x() ,  _pv[p].y()-_pv[_pidx].y());
   const bool thirdTermCAxCPisPositive = (CA.x() * CP.y() - CA.y() * CP.x()) > 0;

   return (thirdTermCAxCPisPositive == thirdTermABxAPisPositive);
}
bool ECVertex::tryClipVertex(ECV_iter prev, ECV_iter next, const WordSet& clipped)
{
   if (_ecGood) {
      prev->_nidx =_nidx;
      prev->update(clipped);
      next->_pidx =_pidx;
      next->update(clipped);
      return true;
   }
   return false;
}

bool ECVertex::pushIndex(TeselVertices& vrtx)
{
   if (vrtx.empty())
   {
      vrtx.push_back(_idx);
      vrtx.push_back(_nidx);
      vrtx.push_back(_pidx);
      return true;
   }
   else {
      TeselVertices::reverse_iterator koko0 = vrtx.rbegin();
      TeselVertices::reverse_iterator koko1 = koko0;koko1++;
      word idx0 = *koko0;
      word idx1 = *koko1;
      if (((idx0 == _pidx) || (idx0 == _idx)) &&
          ((idx1 == _pidx) || (idx1 == _idx))   )
      {
         vrtx.push_back(_nidx);
         return true;
      }
      return false;
   }
}

TessellPoly::TessellPoly():
     _tdata    ()
    ,_all_ftrs ()
    ,_all_ftfs ()
    ,_all_ftss ()
{}

GLvoid TessellPoly::teselBegin(GLenum, GLvoid*)
{
   // DELETE_ME
}

GLvoid TessellPoly::teselVertex(GLvoid*, GLvoid*)
{
   // DELETE_ME
}

GLvoid TessellPoly::teselEnd(GLvoid*)
{
   // DELETE_ME
}



void TessellPoly::tessellate(const int4b* pdata, unsigned psize)
{
   assert(false); //TODO
}

void TessellPoly::tessellate(const PointVector& polyVertex)
{
   // create and initialize the vertex structure we'll need to run the EarClip algo
   // for tesselation - in this case polygon triangulation
   ECVertexList ecVertexList;
   unsigned int lvi = (unsigned int)polyVertex.size() - 1;// last vertex index
   for (word i = 0; i <= lvi; i++)
   {
      if      (  0 == i) ecVertexList.push_back(ECVertex(lvi  ,0  ,1  ,polyVertex));
      else if (lvi == i) ecVertexList.push_back(ECVertex(lvi-1,lvi,0  ,polyVertex));
      else               ecVertexList.push_back(ECVertex(i-1  ,i  ,i+1,polyVertex));
   }
   // init the iterators
   ECV_iter iecv       = ecVertexList.begin()          ;// iterator to the first vertex
   ECV_iter iecvprev   = ecVertexList.end(); iecvprev--;// iterator to the vlast vertex
   ECV_iter iecvnext   = iecv; iecvnext++              ;// iterator to the second vertex
   // start clipping the ears ...
   WordSet clipped; // init the list of already clipped vertexes
   // .. and gather indexes from tessellation
   TeselVertices indexSequence;
   while (2 < ecVertexList.size()) // ... until a signle triangle has left in the list
   {
      if (iecv->tryClipVertex(iecvprev, iecvnext, clipped))
      { // vertex can be clipped!
         if (iecv->angle())
         {
            clipped.insert(iecv->idx());   // add the vertex index to the list of clipped list
            if (!iecv->pushIndex(indexSequence))// add the appropriate indexes to the index sequence
            {
               _tdata.push_back(TeselChunk(indexSequence, ((3==indexSequence.size()) ? GL_TRIANGLES:GL_TRIANGLE_STRIP), 0));
               indexSequence.clear();
               iecv->pushIndex(indexSequence);
            }
         }
         iecv = ecVertexList.erase(iecv);// erase the ECVertex from the list
         iecvnext++;                     // and update the iterators
         if (ecVertexList.end() == iecvnext)
            iecvnext = ecVertexList.begin();
      }
      else
      { // vertex can't be clipped
         if (!indexSequence.empty())
         {
            _tdata.push_back(TeselChunk(indexSequence, ((3==indexSequence.size()) ? GL_TRIANGLES:GL_TRIANGLE_STRIP), 0));
            indexSequence.clear();
         }
         iecvprev = iecv;                // updated the iterators
         iecv = iecvnext;
         iecvnext++;
         if (ecVertexList.end() == iecvnext)
            iecvnext = ecVertexList.begin();
      }
   }
   if (!indexSequence.empty())
   {
      _tdata.push_back(TeselChunk(indexSequence, ((3==indexSequence.size()) ? GL_TRIANGLES:GL_TRIANGLE_STRIP), 0));
      indexSequence.clear();
   }
   for (TeselChain::const_iterator i = _tdata.begin(); i != _tdata.end();i++)
      switch (i->type())
      {
         case GL_TRIANGLE_FAN   : _all_ftfs++ ; break;
         case GL_TRIANGLE_STRIP : _all_ftss++ ; break;
         case GL_TRIANGLES      : _all_ftrs++ ; break;
      }
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

TeselChunk::TeselChunk(const int* /*data*/, unsigned size, unsigned offset)
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

TeselTempData::TeselTempData(unsigned offset) :
   _the_chain   (    NULL),
   _ctype       (      0 ),
   _cindexes    (        ),
   _all_ftrs    (      0 ),
   _all_ftfs    (      0 ),
   _all_ftss    (      0 ),
   _offset      ( offset )
{}

TeselTempData::TeselTempData(TeselChain* tc) :
   _the_chain   ( tc     ),
   _ctype       (      0 ),
   _cindexes    (        ),
   _all_ftrs    (      0 ),
   _all_ftfs    (      0 ),
   _all_ftss    (      0 ),
   _offset      (      0 )
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

TessellPoly_old::TessellPoly_old() : _tdata(), _all_ftrs(0), _all_ftfs(0), _all_ftss(0)
{
}

void TessellPoly_old::tessellate(const int4b* pdata, unsigned psize)
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


GLvoid TessellPoly_old::teselBegin(GLenum type, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newChunk(type);
}

GLvoid TessellPoly_old::teselVertex(GLvoid *pindex, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newIndex(*(static_cast<word*>(pindex)));
}

GLvoid TessellPoly_old::teselEnd(GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->storeChunk();
}

void TessellPoly_old::num_indexs(unsigned& iftrs, unsigned& iftfs, unsigned& iftss) const
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
// TrxCnvx
//
unsigned trend::TrxCnvx::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
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

void trend::TrxCnvx::drctDrawContour()
{
   DBGL_CALL(glBegin,GL_LINE_LOOP)
   for (unsigned i = 0; i < _csize; i++)
      DBGL_CALL(glVertex2i,_cdata[2*i], _cdata[2*i+1])
   DBGL_CALL0(glEnd)
}

void trend::TrxCnvx::drctDrawFill()
{
   assert(false);//How did we get here?
}

//=============================================================================
//
// TrxBox
//
unsigned  trend::TrxBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   assert(_csize);
   array[pindex++] = (TNDR_GLDATAT)_cdata[0];array[pindex++] = (TNDR_GLDATAT)_cdata[1];
   array[pindex++] = (TNDR_GLDATAT)_cdata[2];array[pindex++] = (TNDR_GLDATAT)_cdata[1];
   array[pindex++] = (TNDR_GLDATAT)_cdata[2];array[pindex++] = (TNDR_GLDATAT)_cdata[3];
   array[pindex++] = (TNDR_GLDATAT)_cdata[0];array[pindex++] = (TNDR_GLDATAT)_cdata[3];
   return _csize;
}

void trend::TrxBox::drctDrawContour()
{
   DBGL_CALL(glBegin,GL_LINE_LOOP)
      DBGL_CALL(glVertex2i,_cdata[0], _cdata[1])
      DBGL_CALL(glVertex2i,_cdata[2], _cdata[1])
      DBGL_CALL(glVertex2i,_cdata[2], _cdata[3])
      DBGL_CALL(glVertex2i,_cdata[0], _cdata[3])
   DBGL_CALL0(glEnd)
}

void trend::TrxBox::drctDrawFill()
{
   DBGL_CALL(glBegin,GL_POLYGON);
      DBGL_CALL(glVertex2i,_cdata[0], _cdata[1])
      DBGL_CALL(glVertex2i,_cdata[2], _cdata[1])
      DBGL_CALL(glVertex2i,_cdata[2], _cdata[3])
      DBGL_CALL(glVertex2i,_cdata[0], _cdata[3])
   DBGL_CALL0(glEnd)
}

//=============================================================================
//
// TrxNcvx
//
void trend::TrxNcvx::drctDrawFill()
{
   for ( TeselChain::const_iterator CCH = _tdata->tdata()->begin(); CCH != _tdata->tdata()->end(); CCH++ )
   {
      DBGL_CALL(glBegin,CCH->type())
      for(unsigned cindx = 0 ; cindx < CCH->size(); cindx++)
      {
         unsigned vindex = CCH->index_seq()[cindx];
         DBGL_CALL(glVertex2i,_cdata[2*vindex], _cdata[2*vindex+1])
      }
      DBGL_CALL0(glEnd)
   }
}

//=============================================================================
//
// TrxWire
//
trend::TrxWire::TrxWire(int4b* pdata, unsigned psize, WireWidth width, bool clo) :
   TrxNcvx (NULL, 0),
   _ldata    (pdata  ),
   _lsize    (psize  ),
   _celno    (clo    ),
   _tdata    (NULL   )
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

trend::TrxWire::TrxWire(unsigned psize, const WireWidth /*width*/, bool clo) :
   TrxNcvx (NULL, 0),
   _ldata    (NULL   ),
   _lsize    (psize  ),
   _celno    (clo    ),
   _tdata    (NULL   )
{
}

unsigned trend::TrxWire::lDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
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

void trend::TrxWire::drctDrawCLine()
{
   DBGL_CALL(glBegin,GL_LINE_STRIP)
   for (unsigned i = 0; i < _lsize; i++)
      DBGL_CALL(glVertex2i,_ldata[2*i], _ldata[2*i+1])
   DBGL_CALL0(glEnd)
}

void trend::TrxWire::drctDrawFill()
{
   for ( TeselChain::const_iterator TCH = _tdata->begin(); TCH != _tdata->end(); TCH++ )
   {
      DBGL_CALL(glBegin,TCH->type())
      for(unsigned cindx = 0 ; cindx < TCH->size(); cindx++)
      {
         unsigned vindex = TCH->index_seq()[cindx];
         DBGL_CALL(glVertex2i,_cdata[2*vindex], _cdata[2*vindex+1])
      }
      DBGL_CALL0(glEnd)
   }
}

/** For wire tessellation we can use the common polygon tessellation procedure.
    This could be a huge overhead though. The thing is that we've
    already been trough the precalc procedure and we know that wire object is
    very specific non-convex polygon. Using this knowledge the tessallation
    is getting really trivial. All we have to do is to list the contour points
    indexes in pairs - one from the front, and the other from the back of the
    array. Then this can be drawn as GL_QUAD_STRIP
*/
void trend::TrxWire::Tesselate()
{
   _tdata = DEBUG_NEW TeselChain();
   _tdata->push_back( TeselChunk(_cdata, _csize, 0));
}

trend::TrxWire::~TrxWire()
{
   if (NULL != _cdata) delete [] _cdata;
   if (NULL != _tdata) delete _tdata;
}

//=============================================================================
//
// class TrxText
//
trend::TrxText::TrxText(const std::string* text, const CTM& ctm) :
   _text ( text   ),
   _ctm  ( ctm    )
{
}

void trend::TrxText::draw(bool fill, layprop::DrawProperties* drawprop) const
{
   TRENDC->drawString(*_text, fill, drawprop);
}


//=============================================================================
//
// class TrxTextOvlBox
//
trend::TrxTextOvlBox::TrxTextOvlBox(const DBbox& obox, const CTM& ctm)
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

unsigned trend::TrxTextOvlBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
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

void trend::TrxTextOvlBox::drctDrawContour()
{
   DBGL_CALL(glBegin,GL_LINE_LOOP)
   for (unsigned i = 0; i < 4; i++)
      DBGL_CALL(glVertex2i,_obox[2*i], _obox[2*i+1])
   DBGL_CALL0(glEnd)
}


//=============================================================================
//
// TrxSCnvx
//
unsigned trend::TrxSCnvx::ssize()
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

unsigned trend::TrxSCnvx::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TrxCnvx::cDataCopy(array, pindex);
}

unsigned trend::TrxSCnvx::sDataCopy(unsigned* array, unsigned& pindex)
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

void trend::TrxSCnvx::drctDrawSlctd()
{// same as for non-convex polygon
   if (NULL == _slist)
   {
      DBGL_CALL(glBegin,GL_LINE_LOOP)
      for (unsigned i = 0; i < _csize; i++)
         DBGL_CALL(glVertex2i,_cdata[2*i], _cdata[2*i+1])
      DBGL_CALL0(glEnd)
   }
   else
   {// shape is partially selected
      DBGL_CALL(glBegin,GL_LINES)
      for (unsigned i = 0; i < _csize; i++)
      {
         if (_slist->check(i) && _slist->check((i+1)%_csize))
         {
            DBGL_CALL(glVertex2i,_cdata[2*i], _cdata[2*i+1])
            DBGL_CALL(glVertex2i,_cdata[2*((i+1)%_csize)], _cdata[2*((i+1)%_csize)+1])
         }
      }
      DBGL_CALL0(glEnd);
   }
}

//=============================================================================
//
// TrxSBox
//
unsigned trend::TrxSBox::ssize()
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

unsigned trend::TrxSBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TrxBox::cDataCopy(array, pindex);
}

unsigned trend::TrxSBox::sDataCopy(unsigned* array, unsigned& pindex)
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

void trend::TrxSBox::drctDrawSlctd()
{
   if (NULL == _slist)
   {// shape is fully selected
      DBGL_CALL(glBegin,GL_LINE_LOOP)
         DBGL_CALL(glVertex2i,_cdata[0], _cdata[1])
         DBGL_CALL(glVertex2i,_cdata[2], _cdata[1])
         DBGL_CALL(glVertex2i,_cdata[2], _cdata[3])
         DBGL_CALL(glVertex2i,_cdata[0], _cdata[3])
      DBGL_CALL0(glEnd)
   }
   else
   {// shape is partially selected
      DBGL_CALL(glBegin,GL_LINES)
      for (unsigned i = 0; i < _csize; i++)
      {
         if (_slist->check(i) && _slist->check((i+1)%_csize))
         {
            switch(i)
            {
               case 0: DBGL_CALL(glVertex2i,_cdata[0], _cdata[1]);glVertex2i(_cdata[2], _cdata[1]); break;
               case 1: DBGL_CALL(glVertex2i,_cdata[2], _cdata[1]);glVertex2i(_cdata[2], _cdata[3]); break;
               case 2: DBGL_CALL(glVertex2i,_cdata[2], _cdata[3]);glVertex2i(_cdata[0], _cdata[3]); break;
               case 3: DBGL_CALL(glVertex2i,_cdata[0], _cdata[3]);glVertex2i(_cdata[0], _cdata[1]); break;
               default: assert(false); break;
            }
         }
      }
      DBGL_CALL0(glEnd);
   }
}

//=============================================================================
//
// TrxSNcvx
//
unsigned trend::TrxSNcvx::ssize()
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

unsigned trend::TrxSNcvx::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TrxCnvx::cDataCopy(array, pindex);
}

unsigned trend::TrxSNcvx::sDataCopy(unsigned* array, unsigned& pindex)
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

void trend::TrxSNcvx::drctDrawSlctd()
{// same as for convex polygons
   if (NULL == _slist)
   {
      DBGL_CALL(glBegin,GL_LINE_LOOP)
      for (unsigned i = 0; i < _csize; i++)
         DBGL_CALL(glVertex2i,_cdata[2*i], _cdata[2*i+1])
      DBGL_CALL0(glEnd)
   }
   else
   {// shape is partially selected
      DBGL_CALL(glBegin,GL_LINES)
      for (unsigned i = 0; i < _csize; i++)
      {
         if (_slist->check(i) && _slist->check((i+1)%_csize))
         {
            DBGL_CALL(glVertex2i,_cdata[2*i], _cdata[2*i+1])
            DBGL_CALL(glVertex2i,_cdata[2*((i+1)%_csize)], _cdata[2*((i+1)%_csize)+1])
         }
      }
      DBGL_CALL0(glEnd);
   }
}

//=============================================================================
//
// TrxSWire
//
unsigned trend::TrxSWire::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TrxCnvx::cDataCopy(array, pindex);
}

unsigned trend::TrxSWire::lDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _loffset = pindex/2;
   return TrxWire::lDataCopy(array, pindex);
}

unsigned trend::TrxSWire::ssize()
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
   // the rules when the selected objects were fitted into the whole TrendBase
   // class hierarchy.
   if (!_celno)
   {
      if (_slist->check(0)         ) ssegs +=2;
      if (_slist->check(_lsize-1)  ) ssegs +=2;
   }
   return ssegs;
}

unsigned trend::TrxSWire::sDataCopy(unsigned* array, unsigned& pindex)
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

void trend::TrxSWire::drctDrawSlctd()
{
   if (NULL == _slist)
   {
      DBGL_CALL(glBegin,GL_LINE_STRIP)
      for (unsigned i = 0; i < _lsize; i++)
         DBGL_CALL(glVertex2i,_ldata[2*i], _ldata[2*i+1])
      DBGL_CALL0(glEnd)
   }
   else
   {// shape is partially selected
      DBGL_CALL(glBegin,GL_LINES)
      for (unsigned i = 0; i < _lsize - 1; i++)
      {
         if (_slist->check(i) && _slist->check(i+1))
         {
            DBGL_CALL(glVertex2i,_ldata[2*i], _ldata[2*i+1])
            DBGL_CALL(glVertex2i,_ldata[2*(i+1)], _ldata[2*(i+1)+1])
         }
      }
      if (!_celno)
      {
         // And the edge points!
         if (_slist->check(0)       ) // if first point is selected
         {
            DBGL_CALL(glVertex2i,_cdata[_csize-2], _cdata[_csize-1])
            DBGL_CALL(glVertex2i,_cdata[_csize], _cdata[_csize+1])
         }
         if (_slist->check(_lsize-1))// if last point is selected
         {
            DBGL_CALL(glVertex2i,_cdata[0], _cdata[1])
            DBGL_CALL(glVertex2i,_cdata[(2*_csize)-2], _cdata[(2*_csize)-1])
         }
      }
      DBGL_CALL0(glEnd)
   }
}

//=============================================================================
//
// TrxTextSOvlBox
//
unsigned trend::TrxTextSOvlBox::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
   _offset = pindex/2;
   return TrxTextOvlBox::cDataCopy(array, pindex);
}

unsigned trend::TrxTextSOvlBox::sDataCopy(unsigned* array, unsigned& pindex)
{
   assert (NULL == _slist);
   for (unsigned i = 0; i < 4; i++)
      array[pindex++] = _offset + i;
   return ssize();
}

void trend::TrxTextSOvlBox::drctDrawSlctd()
{
   DBGL_CALL(glBegin,GL_LINE_LOOP)
   for (unsigned i = 0; i < 4; i++)
      DBGL_CALL(glVertex2i,_obox[2*i], _obox[2*i+1])
   DBGL_CALL0(glEnd)
}

//=============================================================================
//
// class TrxSMBox
//

trend::TrxSMBox::TrxSMBox(const int4b* pdata, const SGBitSet* slist, const CTM& rmm) :
   TrxSBox(pdata, slist)
{
   //   PointVector* nshape = movePointsSelected(*_slist, _rmm->Reversed(), strans);
   //   pt1 = (*nshape)[0]*(*_rmm); pt2 = (*nshape)[2]*(*_rmm);
   // The lines above - just a reminder of what the line below is actually doing
   PointVector* nshape = movePointsSelected(*_slist, CTM(), rmm);
   int4b* mdata = DEBUG_NEW int4b[4];
   mdata[0] = (*nshape)[0].x();
   mdata[1] = (*nshape)[0].y();
   mdata[2] = (*nshape)[2].x();
   mdata[3] = (*nshape)[2].y();
   _cdata = mdata;
   nshape->clear(); delete nshape;
}

trend::TrxSMBox::~TrxSMBox()
{
   delete [] _cdata;
}

PointVector* trend::TrxSMBox::movePointsSelected(const SGBitSet& pset,
                                    const CTM&  movedM, const CTM& stableM) const {
  // convert box to polygon
   PointVector* mlist = DEBUG_NEW PointVector();
   mlist->push_back(TP(_cdata[p1x], _cdata[p1y]));
   mlist->push_back(TP(_cdata[p2x], _cdata[p1y]));
   mlist->push_back(TP(_cdata[p2x], _cdata[p2y]));
   mlist->push_back(TP(_cdata[p1x], _cdata[p2y]));

   word size = mlist->size();
   PSegment seg1,seg0;
   // Things to remember in this algo...
   // Each of the points in the initial mlist is recalculated in the seg1.crossP
   // method. This actually means that on pass 0 (i == 0), no points are
   // recalculated because seg0 at that moment is empty. On pass 1 (i == 1),
   // point mlist[1] is recalculated etc. The catch comes on the last pass
   // (i == size) when constructing the seg1, we need mlist[0] and mlist[1], but
   // mlist[1] has been already recalculated and multiplying it with CTM
   // matrix again has pretty funny effect.
   // That's why another condition is introduced -> if (i == size)
   for (unsigned i = 0; i <= size; i++) {
      if (i == size)
         if (pset.check(i%size) && pset.check((i+1) % size))
            seg1 = PSegment((*mlist)[(i  ) % size] * movedM,
                            (*mlist)[(i+1) % size]         );
         else
            seg1 = PSegment((*mlist)[(i  ) % size] * stableM,
                            (*mlist)[(i+1) % size]          );
      else
         if (pset.check(i%size) && pset.check((i+1) % size))
            seg1 = PSegment((*mlist)[(i  ) % size] * movedM,
                            (*mlist)[(i+1) % size] * movedM);
         else
            seg1 = PSegment((*mlist)[(i  ) % size] * stableM,
                            (*mlist)[(i+1) % size] * stableM);
      if (!seg0.empty()) {
         seg1.crossP(seg0,(*mlist)[i%size]);
      }
      seg0 = seg1;
   }
   return mlist;
}

//=============================================================================
//
// class TrxSMNcvx
//
trend::TrxSMNcvx::TrxSMNcvx(const int4b* pdata, unsigned psize, const SGBitSet* slist, const CTM& rmm) :
   TrxSNcvx(pdata, psize, slist)
{
   PointVector* nshape = movePointsSelected(*_slist, CTM(), rmm);
   int4b* mdata = DEBUG_NEW int4b[2*_csize];

   for (unsigned i = 0; i < _csize; i++)
   {
      mdata[2*i  ] = (*nshape)[i].x();
      mdata[2*i+1] = (*nshape)[i].y();
   }
   _cdata = mdata;
   nshape->clear(); delete nshape;
}

trend::TrxSMNcvx::~TrxSMNcvx()
{
   delete [] _cdata;
}

PointVector* trend::TrxSMNcvx::movePointsSelected(const SGBitSet& pset,
                                    const CTM&  movedM, const CTM& stableM) const {
   PointVector* mlist = DEBUG_NEW PointVector();
   mlist->reserve(_csize);
   for (unsigned i = 0 ; i < _csize; i++ )
      mlist->push_back(TP(_cdata[2*i], _cdata[2*i+1]));

   PSegment seg1,seg0;
   // See the note about this algo in TdtBox::movePointsSelected above
   for (unsigned i = 0; i <= _csize; i++) {
      if (i == _csize)
         if (pset.check(i % _csize) && pset.check((i+1) % _csize))
            seg1 = PSegment((*mlist)[(i  ) % _csize] * movedM,
                            (*mlist)[(i+1) % _csize]         );
         else
            seg1 = PSegment((*mlist)[(i  ) % _csize] * stableM,
                            (*mlist)[(i+1) % _csize]         );
      else
         if (pset.check(i % _csize) && pset.check((i+1) % _csize))
            seg1 = PSegment((*mlist)[(i  ) % _csize] * movedM,
                            (*mlist)[(i+1) % _csize] * movedM);
         else
            seg1 = PSegment((*mlist)[(i  ) % _csize] * stableM,
                            (*mlist)[(i+1) % _csize] * stableM);
      if (!seg0.empty()) {
         seg1.crossP(seg0,(*mlist)[ i % _csize]);
      }
      seg0 = seg1;
   }
   return mlist;
}

//=============================================================================
//
// class TrxSMWire
//
trend::TrxSMWire::TrxSMWire(int4b* pdata, unsigned psize, const WireWidth width, bool clo, const SGBitSet* slist, const CTM& rmm) :
   TrxSWire(psize, width, clo, slist)
{
   _ldata = pdata;
   PointVector* nshape = movePointsSelected(*_slist, CTM(), rmm);
   int4b* mdata = DEBUG_NEW int4b[2*_lsize];

   for (unsigned i = 0; i < _lsize; i++)
   {
      mdata[2*i  ] = (*nshape)[i].x();
      mdata[2*i+1] = (*nshape)[i].y();
   }
   _ldata = mdata;
   nshape->clear(); delete nshape;

   if (!_celno)
   {
      laydata::WireContour wcontour(_ldata, _lsize, width);
      _csize = wcontour.csize();
      int4b* contData = DEBUG_NEW int4b[ 2 * _csize];
      wcontour.getArrayData(contData);
      _cdata = contData;
   }
}

trend::TrxSMWire::~TrxSMWire()
{
   delete [] _ldata;
}

PointVector* trend::TrxSMWire::movePointsSelected(const SGBitSet& pset,
                                    const CTM& movedM, const CTM& stableM) const
{
   PointVector* mlist = DEBUG_NEW PointVector();
   mlist->reserve(_lsize);
   for (unsigned i = 0 ; i < _lsize; i++ )
      mlist->push_back(TP(_ldata[2*i], _ldata[2*i+1]));

   PSegment* seg1 = NULL;
   PSegment* seg0 = NULL;
   for (unsigned i = 0; i < _lsize; i++)
   {
      if ((_lsize-1) == i)
      {
         if (pset.check(_lsize-1))
            seg1 = seg1->ortho((*mlist)[_lsize-1] * movedM);
         else
            seg1 = seg1->ortho((*mlist)[_lsize-1] * stableM);
      }
      else
      {
         const CTM& transM = ((pset.check(i) && pset.check(i+1))) ?
                                                               movedM : stableM;
         seg1 = DEBUG_NEW PSegment((*mlist)[(i  )] * transM, (*mlist)[(i+1)] * transM);
         if (0 == i)
         {
            if (pset.check(0))
               seg0 = seg1->ortho((*mlist)[i] * movedM);
            else
               seg0 = seg1->ortho((*mlist)[i] * stableM);
         }
      }
      if (!seg0->empty()) seg1->crossP(*seg0,(*mlist)[i]);
      if (NULL != seg0) delete seg0;
      seg0 = seg1;
   }
   if (NULL != seg0) delete seg0;
   return mlist;
}

//=============================================================================
//
// TrxTBox
//
trend::TrxTBox::TrxTBox(const TP& p1, const CTM& rmm) :
   TrxBox(NULL)
{
   TP p2 = p1 * rmm;
   int4b* contData = DEBUG_NEW int4b[4];
   contData[0] = p1.x();
   contData[1] = p1.y();
   contData[2] = p2.x();
   contData[3] = p2.y();
   _cdata = contData;
}

trend::TrxTBox::~TrxTBox()
{
   assert(_cdata);
   delete [] (_cdata);
}

//=============================================================================
//
// TrxTNcvx
//
trend::TrxTNcvx::TrxTNcvx(const PointVector& plist, const CTM& rmm) :
   TrxNcvx( NULL, static_cast<unsigned>(plist.size())+1)
{
   dword numpnts = static_cast<dword>(plist.size());
   int4b* contData = DEBUG_NEW int4b[2*(numpnts+1)];
   for (word i = 0; i < numpnts; i++)
   {
      contData[2*i  ] = plist[i].x();
      contData[2*i+1] = plist[i].y();
   }
   TP newp(plist[numpnts-1] * rmm);
   contData[2*numpnts  ] = newp.x();
   contData[2*numpnts+1] = newp.y();
   _cdata = contData;
}

trend::TrxTNcvx::~TrxTNcvx()
{
   assert(_cdata);
   delete [] (_cdata);
}

//=============================================================================
//
// TrxTWire
//
trend::TrxTWire::TrxTWire(const PointVector& plist, WireWidth width, bool clo, const CTM& rmm) :
   TrxWire(static_cast<unsigned>(plist.size())+1, width, clo)
{
   dword num_points = static_cast<dword>(plist.size());
   assert(0 < num_points);
   laydata::WireContourAux wcontour(plist, width, TP(plist[num_points-1] * rmm));

   int4b* centData = DEBUG_NEW int4b[ 2 * _lsize];
   wcontour.getArrayLData(centData);
   _ldata = centData;

   _csize = wcontour.csize();
   int4b* contData = DEBUG_NEW int4b[ 2 * _csize];
   wcontour.getArrayCData(contData);
   _cdata = contData;
}

trend::TrxTWire::~TrxTWire()
{
   delete [] _ldata;
}


//=============================================================================
//
// class TrxCellRef
//
trend::TrxCellRef::TrxCellRef(std::string name, const CTM& ctm, const DBbox& obox,
                   word alphaDepth) :
   _name          ( name         ),
   _ctm           ( ctm          ),
   _alphaDepth    ( alphaDepth   )
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


trend::TrxCellRef::TrxCellRef() :
   _name       ( ""   ),
   _ctm        (      ),
   _alphaDepth ( 0    )
{
   _ctm.oglForm(_translation);
   for (word i = 0; i < 8; _obox[i++] = 0);
}

unsigned trend::TrxCellRef::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
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

void trend::TrxCellRef::drctDrawContour()
{
   DBGL_CALL(glBegin,GL_LINE_LOOP)
   for (unsigned i = 0; i < 4; i++)
      DBGL_CALL(glVertex2i,_obox[2*i], _obox[2*i+1])
   DBGL_CALL0(glEnd)
}

