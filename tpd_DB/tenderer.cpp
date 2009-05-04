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

// Themporary - to clarify the possible proper usage of VBO's
//#define USE_VBOS

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
// TenderObj


//=============================================================================

/*void TenderPoly::TeselData(TeselChain* tdata, unsigned offset)
{
   assert(tdata);
   for (TeselChain::const_iterator CTC = tdata->begin(); CTC != tdata->end(); CTC++)
   {
      TeselChunk* achunk = DEBUG_NEW TeselChunk(*CTC, offset);
      _tdata.push_back(achunk);
   }
}
*/
// TenderPoly::~TenderPoly()
// {
//    for (TeselChain::const_iterator CTC = _tdata.begin(); CTC != _tdata.end(); CTC++)
//       delete (*CTC);
// }

//=============================================================================
TenderWire::TenderWire(int4b* pdata, unsigned psize, const word width,
                       bool center_line_only) : TenderPoly(NULL, 0), 
                       _ldata(pdata), _lsize(psize), _center_line_only(center_line_only), _tdata(NULL)
{
   if (!_center_line_only)
      precalc(width);
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
   assert((rindex + 1) == _csize);
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
// class TenderLine
TenderLine::TenderLine(TenderObj* obj, const SGBitSet* psel)
{
   _partial = (NULL != psel);
   if (_partial)
   { // shape is partially selected
      // get the number of selected segments first
      _lsize = 0;
      word allpoints = obj->csize();
      for (unsigned i = 0; i < allpoints; i++)
         if (psel->check(i) && psel->check((i+1)%allpoints)) _lsize +=2;
      // now copy the segment points
      _ldata = DEBUG_NEW int4b [2*_lsize];
      word curpoint = 0;
      for (unsigned i = 0; i < allpoints; i++)
         if (psel->check(i) && psel->check((i+1)%allpoints))
      {
         _ldata[2*curpoint  ] = obj->cdata()[2*i  ];
         _ldata[2*curpoint+1] = obj->cdata()[2*i+1];
         curpoint++;
         _ldata[2*curpoint  ] = obj->cdata()[2*((i+1)%allpoints)  ];
         _ldata[2*curpoint+1] = obj->cdata()[2*((i+1)%allpoints)+1];
         curpoint++;
      }
      assert(curpoint == _lsize);
   }
   else
   {
      _lsize = obj->csize();
      _ldata = obj->cdata();
   }
}

TenderLine::TenderLine(TenderPoly* obj, const SGBitSet* psel)
{
   _partial = (NULL != psel);
   if (_partial)
   { // shape is partially selected
      // get the number of selected segments first
      _lsize = 0;
      word allpoints = obj->csize();
      for (unsigned i = 0; i < allpoints; i++)
         if (psel->check(i) && psel->check((i+1)%allpoints)) _lsize +=2;
      // now copy the segment points
      _ldata = DEBUG_NEW int4b [2*_lsize];
      word curpoint = 0;
      for (unsigned i = 0; i < allpoints; i++)
         if (psel->check(i) && psel->check((i+1)%allpoints))
         {
            _ldata[2*curpoint  ] = obj->cdata()[2*i  ];
            _ldata[2*curpoint+1] = obj->cdata()[2*i+1];
            curpoint++;
            _ldata[2*curpoint  ] = obj->cdata()[2*((i+1)%allpoints)  ];
            _ldata[2*curpoint+1] = obj->cdata()[2*((i+1)%allpoints)+1];
            curpoint++;
         }
      assert(curpoint == _lsize);
   }
   else
   {
      _lsize = obj->csize();
      _ldata = obj->cdata();
   }
}

TenderLine::TenderLine(TenderWire* obj, const SGBitSet* psel)
{
   _partial = (NULL != psel);
   if (_partial)
   { // shape is partially selected
      // get the number of selected segments first
      _lsize = 0;
      word allpoints = obj->lsize();
      for (unsigned i = 0; i < allpoints - 1; i++)
         if (psel->check(i) && psel->check(i+1)) _lsize +=2;
      if (psel->check(0)            ) _lsize +=2;
      if (psel->check(allpoints-1)  ) _lsize +=2;
      // now copy the segment points
      _ldata = DEBUG_NEW int4b [2*_lsize];
      word curpoint = 0;
      for (unsigned i = 0; i < allpoints - 1; i++)
         if (psel->check(i) && psel->check((i+1)%allpoints))
         {
            _ldata[2*curpoint  ] = obj->ldata()[2*i  ];
            _ldata[2*curpoint+1] = obj->ldata()[2*i+1];
            curpoint++;
            _ldata[2*curpoint  ] = obj->ldata()[2*(i+1)  ];
            _ldata[2*curpoint+1] = obj->ldata()[2*(i+1)+1];
            curpoint++;
         }
      if (psel->check(0)            )
      {
         _ldata[2*curpoint  ] = obj->cdata()[0];
         _ldata[2*curpoint+1] = obj->cdata()[1];
         curpoint++;
         _ldata[2*curpoint  ] = obj->cdata()[4*allpoints - 2];
         _ldata[2*curpoint+1] = obj->cdata()[4*allpoints - 1];
         curpoint++;
      }
      if (psel->check(allpoints-1)  )
      {
         _ldata[2*curpoint  ] = obj->cdata()[2*allpoints - 2];
         _ldata[2*curpoint+1] = obj->cdata()[2*allpoints - 1];
         curpoint++;
         _ldata[2*curpoint  ] = obj->cdata()[2*allpoints    ];
         _ldata[2*curpoint+1] = obj->cdata()[2*allpoints + 1];
         curpoint++;
      }
      assert(curpoint == _lsize);
   }
   else
   {
      _lsize = obj->lsize();
      _ldata = obj->ldata();
   }
}

TenderLine::~TenderLine()
{
   if (_partial)
      delete []_ldata;
}

//=============================================================================
// class TenderRB
TenderRB::TenderRB(const CTM& tmatrix, const DBbox& obox) : _tmatrix (tmatrix),
                   _obox(obox)
{}

void TenderRB::draw()
{
   glPushMatrix();
   real openGLmatrix[16];
   _tmatrix.oglForm(openGLmatrix);
   glMultMatrixd(openGLmatrix);
   //
   glRecti(_obox.p1().x(), _obox.p1().y(), _obox.p2().x(), _obox.p2().y());
   //
   glPopMatrix();
}

//=============================================================================
// class TenderTVB
TenderTVB::TenderTVB() :
   _num_ln_points(0u), _num_ll_points(0u),_num_ls_points(0u),
   _num_ln(0), _num_ll(0), _num_ls(0)
{}

void TenderTVB::add(TenderObj* tobj, const SGBitSet* psel)
{
   TenderLine* cline = DEBUG_NEW TenderLine(tobj, psel);
   if (NULL == psel)
   {
      _ll_data.push_back(cline);
      _num_ll_points += cline->lsize();
      _num_ll++;
   }
   else
   {
      _ls_data.push_back(cline);
      _num_ls_points += cline->lsize();
      _num_ls++;
   }
}

void TenderTVB::add(TenderPoly* tobj, const SGBitSet* psel)
{
   TenderLine* cline = DEBUG_NEW TenderLine(tobj, psel);
   if (NULL == psel)
   {
      _ll_data.push_back(cline);
      _num_ll_points += cline->lsize();
      _num_ll++;
   }
   else
   {
      _ls_data.push_back(cline);
      _num_ls_points += cline->lsize();
      _num_ls++;
   }
}

void TenderTVB::add(TenderWire* tobj, const SGBitSet* psel)
{
   TenderLine* cline = DEBUG_NEW TenderLine(tobj, psel);
   if (NULL == psel)
   {
      _ln_data.push_back(cline);
      _num_ln_points += cline->lsize();
      _num_ln++;
   }
   else
   {
      _ls_data.push_back(cline);
      _num_ls_points += cline->lsize();
      _num_ls++;
   }
}

void TenderTVB::draw_lloops()
{
   if  (0 == _num_ll) return;
   unsigned long arr_size = 2 * _num_ll_points;
   int* point_array = DEBUG_NEW int[arr_size];
   GLsizei* size_array = DEBUG_NEW int[_num_ll];
   GLsizei* first_array = DEBUG_NEW int[_num_ll];
   unsigned long pntindx = 0;
   unsigned      szindx  = 0;

   for (SliceLines::const_iterator CSH = _ll_data.begin(); CSH != _ll_data.end(); CSH++)
   {
      unsigned clsize = (*CSH)->lsize();
      assert(clsize);
      first_array[szindx] = pntindx/2;
      size_array[szindx++] = clsize;
      memcpy(&(point_array[pntindx]), (*CSH)->ldata(), 2 * sizeof(int4b) * clsize);
      pntindx += 2 * clsize;
   }
   assert(pntindx == arr_size);
   assert(szindx == _num_ll);
   glVertexPointer(2, GL_INT, 0, point_array);
   glMultiDrawArrays(GL_LINE_LOOP, first_array, size_array, szindx);

   delete [] point_array;
   delete [] size_array;
   delete [] first_array;
}

void TenderTVB::draw_lines()
{
   if  (0 == _num_ln) return;
   unsigned long arr_size = 2 * _num_ln_points;
   int* point_array = DEBUG_NEW int[arr_size];
   GLsizei* size_array = DEBUG_NEW int[_num_ln];
   GLsizei* first_array = DEBUG_NEW int[_num_ln];
   unsigned long pntindx = 0;
   unsigned      szindx  = 0;

   for (SliceLines::const_iterator CSH = _ln_data.begin(); CSH != _ln_data.end(); CSH++)
   {
      unsigned clsize = (*CSH)->lsize();
      assert(clsize);
      first_array[szindx] = pntindx/2;
      size_array[szindx++] = clsize;
      memcpy(&(point_array[pntindx]), (*CSH)->ldata(), 2 * sizeof(int4b) * clsize);
      pntindx += 2 * clsize;
   }
   assert(pntindx == arr_size);
   assert(szindx == _num_ln);
   glVertexPointer(2, GL_INT, 0, point_array);
   glMultiDrawArrays(GL_LINE_STRIP, first_array, size_array, szindx);

   delete [] point_array;
   delete [] size_array;
   delete [] first_array;
}

void TenderTVB::draw_lsegments()
{
   if  (0 == _num_ls) return;
   unsigned long arr_size = 2 * _num_ls_points;
   int* point_array = DEBUG_NEW int[arr_size];
   GLsizei* size_array = DEBUG_NEW int[_num_ls];
   GLsizei* first_array = DEBUG_NEW int[_num_ls];
   unsigned long pntindx = 0;
   unsigned      szindx  = 0;

   for (SliceLines::const_iterator CSH = _ls_data.begin(); CSH != _ls_data.end(); CSH++)
   {
      unsigned clsize = (*CSH)->lsize();
      assert(clsize);
      first_array[szindx] = pntindx/2;
      size_array[szindx++] = clsize;
      memcpy(&(point_array[pntindx]), (*CSH)->ldata(), 2 * sizeof(int4b) * clsize);
      pntindx += 2 * clsize;
   }
   assert(pntindx == arr_size);
   assert(szindx == _num_ls);
   glVertexPointer(2, GL_INT, 0, point_array);
   glMultiDrawArrays(GL_LINES, first_array, size_array, szindx);

   delete [] point_array;
   delete [] size_array;
   delete [] first_array;
}

//=============================================================================
// class TenderTV
TenderTV::TenderTV(CTM& translation, bool filled, unsigned parray_offset, unsigned iarray_offset) :
   _tmatrix(translation),
   _num_cont_points (0u), _num_line_points(0u), _num_ncvx_points(0u),
   _num_fqss_indexs(0u) , _num_ftrs_indexs(0u), _num_ftfs_indexs(0u),  _num_ftss_indexs(0u),
   _all_conts(0u)  , _all_lines(0u)  , _all_cnvx(0u)   , _all_ncvx(0u)  ,
   _all_fqss(0u)   , _all_ftrs(0u)   , _all_ftfs(0u)   , _all_ftss(0u)  ,
   _point_array_offset(parray_offset), _index_array_offset(iarray_offset),
   _sza_cont(NULL), _sza_line(NULL), _sza_cnvx(NULL), _sza_ncvx(NULL),
   _sza_fqss(NULL), _sza_ftrs(NULL), _sza_ftfs(NULL), _sza_ftss(NULL),
   _fst_cont(NULL), _fst_line(NULL), _fst_cnvx(NULL), _fst_ncvx(NULL),
   _fst_fqss(NULL), _fst_ftrs(NULL), _fst_ftfs(NULL), _fst_ftss(NULL),
   _filled(filled)
{}

TenderObj* TenderTV::box (int4b* pdata)
{
   TenderObj* cobj = DEBUG_NEW TenderObj(pdata, 4);
   if (_filled)
   {
      _cnvx_data.push_back(cobj);
      _all_cnvx++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _num_cont_points += 4;
      _all_conts++;
   }
   return cobj;
}

TenderPoly* TenderTV::poly (int4b* pdata, unsigned psize, TeselPoly* tchain)
{
   TenderPoly* cobj = DEBUG_NEW TenderPoly(pdata, psize);
   if (_filled)
   {
      cobj->setTeselData(tchain);
      _ncvx_data.push_back(cobj);
      _num_ncvx_points += cobj->csize();
      _all_ftrs += tchain->num_ftrs();
      _all_ftfs += tchain->num_ftfs();
      _all_ftss += tchain->num_ftss();
      tchain->num_indexs(_num_ftrs_indexs, _num_ftfs_indexs, _num_ftss_indexs);
      _all_ncvx++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _num_cont_points += cobj->csize();
      _all_conts++;
   }
   return cobj;
}

TenderWire* TenderTV::wire (int4b* pdata, unsigned psize, word width, bool center_line_only)
{
   TenderWire* cobj = DEBUG_NEW TenderWire(pdata, psize, width, center_line_only);
   _line_data.push_back(cobj);
   _num_line_points += cobj->lsize();
   _all_lines++;
   if (!center_line_only)
   {
       if (_filled)
       {
         cobj->Tesselate();
         _ncvx_data.push_back(cobj);
         _num_ncvx_points += cobj->csize();
         _num_fqss_indexs += cobj->csize();
         _all_ncvx++;
         _all_fqss++;
       }
       else
       {
          _cont_data.push_back(cobj);
          _all_conts += 1;
          _num_cont_points += cobj->csize();
       }
   }
   return cobj;
}

unsigned TenderTV::num_total_points()
{
   return ( _num_cont_points +
            _num_line_points +
           (_all_cnvx * 4)   +
            _num_ncvx_points
          );
}

unsigned TenderTV::num_total_indexs()
{
   return ( _num_fqss_indexs +
            _num_ftrs_indexs +
            _num_ftfs_indexs +
            _num_ftss_indexs
          );
}
// void TenderTV::collectNdraw_contours()
// {
//    if  (0 == _all_conts) return;
//    unsigned long arr_size = 2 * _num_cont_points;
// #ifdef USE_VBOS
//    // Organise the VBO ...
//    GLuint ogl_buffer;
//    glGenBuffers(1, &ogl_buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, ogl_buffer);
//    glBufferData(GL_ARRAY_BUFFER, arr_size * sizeof(int4b), NULL, GL_DYNAMIC_DRAW);
//    int* point_array = (int*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
// #else
//    int* point_array = DEBUG_NEW int[arr_size];
// #endif
//    // ... and the additional arrays
//    GLsizei* size_array = DEBUG_NEW int[_all_conts];
//    GLsizei* first_array = DEBUG_NEW int[_all_conts];
//    // initialise the indexing
//    unsigned long pntindx = 0;
//    unsigned      szindx  = 0;
//    // copy all the data in the VBO and in the same time update the
//    // contour size array and the array containg first indexes
//    for (SliceObjects::const_iterator CSH = _cont_data.begin(); CSH != _cont_data.end(); CSH++)
//    { // shapes in the current translation (layer within the cell)
//       unsigned clsize = (*CSH)->csize();
//       assert(clsize);
//       first_array[szindx] = pntindx/2;
//       size_array[szindx++] = clsize;
// //#ifdef USE_VBOS
// //      glBufferSubData(GL_ARRAY_BUFFER, pntindx * sizeof(int4b), 2 * sizeof(int4b) * clsize, (*CSH)->cdata());
// //#else
//       memcpy(&(point_array[pntindx]), (*CSH)->cdata(), 2 * sizeof(int4b) * clsize);
// //#endif
//       pntindx += 2 * clsize;
//    }
//    assert(pntindx == arr_size);
//    assert(szindx == _all_conts);
// #ifdef USE_VBOS
//    // Draw the VBO
//    glUnmapBuffer(GL_ARRAY_BUFFER);
//    glVertexPointer(2, GL_INT, 0, 0);
// #else
//    glVertexPointer(2, GL_INT, 0, point_array);
// #endif
//    glMultiDrawArrays(GL_LINE_LOOP, first_array, size_array, szindx);
//    // Release the VBO memory in the GPU
// #ifdef USE_VBOS
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glDeleteBuffers(1, &ogl_buffer);
// #else
//    delete [] point_array;
// #endif
//    // Clean-up the CPU memory
//    delete [] size_array;
//    delete [] first_array;
// }

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
            assert(_sza_fqss);
            _fst_fqss[size_index[fqss]  ] = sizeof(unsigned) * index_offset[fqss];
            _sza_fqss[size_index[fqss]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[fqss]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLES      :
         {
            assert(_sza_ftrs);
            _fst_ftrs[size_index[ftrs]  ] = sizeof(unsigned) * index_offset[ftrs];
            _sza_ftrs[size_index[ftrs]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[ftrs]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_FAN   :
         {
            assert(_sza_ftfs);
            _fst_ftfs[size_index[ftfs]  ] = sizeof(unsigned) * index_offset[ftfs];
            _sza_ftfs[size_index[ftfs]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[ftfs]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         case GL_TRIANGLE_STRIP :
         {
            assert(_sza_ftss);
            _fst_ftss[size_index[ftss]  ] = sizeof(unsigned) * index_offset[ftss];
            _sza_ftss[size_index[ftss]++] = cchunk->size();
            for (unsigned i = 0; i < cchunk->size(); i++)
               index_array[index_offset[ftss]++] = cchunk->index_seq()[i] + cpoint_index;
            break;
         }
         default: assert(0);
      }
   }
}

void TenderTV::collect(int* point_array, unsigned int* index_array)
{
   unsigned long line_arr_size = 2 * _num_line_points;
   unsigned long fqus_arr_size = 8 * _all_cnvx; //2 * 4
   unsigned long cont_arr_size = 2 * _num_cont_points;
   unsigned long poly_arr_size = 2 * _num_ncvx_points;
   // initialise the indexing
   unsigned long pntindx = 0;
   if  (_all_lines > 0)
   {
      unsigned  szindx  = 0;
      _fst_line = DEBUG_NEW int[_all_lines];
      _sza_line = DEBUG_NEW int[_all_lines];
      for (SliceWires::const_iterator CSH = _line_data.begin(); CSH != _line_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         unsigned clsize = (*CSH)->lsize();
         assert(clsize);
         _fst_line[szindx] = pntindx/2;
         _sza_line[szindx++] = clsize;
         memcpy(&(point_array[_point_array_offset + pntindx]), (*CSH)->ldata(), 2 * sizeof(int4b) * clsize);
         pntindx += 2 * clsize;
      }
      assert(pntindx == line_arr_size);
      assert(szindx  == _all_lines);
   }

   if  (_all_cnvx > 0)
   {
      unsigned  szindx  = 0;
      _fst_cnvx = DEBUG_NEW int[_all_cnvx];
      _sza_cnvx = DEBUG_NEW int[_all_cnvx];
      for (SliceObjects::const_iterator CSH = _cnvx_data.begin(); CSH != _cnvx_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         unsigned clsize = (*CSH)->csize();
         assert(clsize);
         _fst_cnvx[szindx] = pntindx/2;
         _sza_cnvx[szindx++] = clsize;
         memcpy(&(point_array[_point_array_offset + pntindx]), (*CSH)->cdata(), 2 * sizeof(int4b) * clsize);
         pntindx += 2 * clsize;
      }
      assert(pntindx == line_arr_size + fqus_arr_size);
      assert(szindx  == _all_cnvx);
   }

   if  (_all_ncvx > 0)
   {
      unsigned  szindx  = 0;
      _fst_ncvx = DEBUG_NEW int[_all_ncvx];
      _sza_ncvx = DEBUG_NEW int[_all_ncvx];
      if (NULL != index_array)
      {
         assert(_all_fqss + _all_ftrs + _all_ftfs + _all_ftss);
         if (0 < _all_fqss)
         {
            _sza_fqss = DEBUG_NEW GLsizei[_all_fqss];
            _fst_fqss = DEBUG_NEW GLuint[_all_fqss];
         }
         if (0 < _all_ftrs)
         {
            _sza_ftrs = DEBUG_NEW GLsizei[_all_ftrs];
            _fst_ftrs = DEBUG_NEW GLuint[_all_ftrs];
         }
         if (0 < _all_ftfs)
         {
            _sza_ftfs = DEBUG_NEW GLsizei[_all_ftfs];
            _fst_ftfs = DEBUG_NEW GLuint[_all_ftfs];
         }
         if (0 < _all_ftss)
         {
            _sza_ftss = DEBUG_NEW GLsizei[_all_ftss];
            _fst_ftss = DEBUG_NEW GLuint[_all_ftss];
         }
      }
      unsigned size_index[4];
      unsigned index_offset[4];
      size_index[fqss] = size_index[ftrs] = size_index[ftfs] = size_index[ftss] = 0u;
      index_offset[fqss] = _index_array_offset;
      index_offset[ftrs] = index_offset[fqss] + _num_fqss_indexs;
      index_offset[ftfs] = index_offset[ftrs] + _num_ftrs_indexs;
      index_offset[ftss] = index_offset[ftfs] + _num_ftfs_indexs;
      for (SlicePolygons::const_iterator CSH = _ncvx_data.begin(); CSH != _ncvx_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         unsigned clsize = (*CSH)->csize();
         assert(clsize);

         if (NULL != (*CSH)->tdata())
            collectIndexs(index_array    ,
                        (*CSH)->tdata()  ,
                        size_index       ,
                        index_offset     ,
                        pntindx/2
                        );

         _fst_ncvx[szindx] = pntindx/2;
         _sza_ncvx[szindx++] = clsize;
         memcpy(&(point_array[_point_array_offset + pntindx]), (*CSH)->cdata(), 2 * sizeof(int4b) * clsize);
         pntindx += 2 * clsize;
      }
      assert(size_index[fqss] == _all_fqss);
      assert(size_index[ftrs] == _all_ftrs);
      assert(size_index[ftfs] == _all_ftfs);
      assert(size_index[ftss] == _all_ftss);
      assert(index_offset[fqss] == (_index_array_offset + _num_fqss_indexs));
      assert(index_offset[ftrs] == (_index_array_offset + _num_fqss_indexs + _num_ftrs_indexs));
      assert(index_offset[ftfs] == (_index_array_offset + _num_fqss_indexs + _num_ftrs_indexs + _num_ftfs_indexs ));
      assert(index_offset[ftss] == (_index_array_offset + _num_fqss_indexs + _num_ftrs_indexs + _num_ftfs_indexs + _num_ftss_indexs ));
      assert(pntindx == line_arr_size + fqus_arr_size + poly_arr_size);
      assert(szindx  == _all_ncvx);
   }

   if  (_all_conts > 0)
   {
      unsigned  szindx  = 0;
      _fst_cont = DEBUG_NEW int[_all_conts];
      _sza_cont = DEBUG_NEW int[_all_conts];
      for (SliceObjects::const_iterator CSH = _cont_data.begin(); CSH != _cont_data.end(); CSH++)
      { // shapes in the current translation (layer within the cell)
         unsigned clsize = (*CSH)->csize();
         assert(clsize);
         _fst_cont[szindx] = pntindx/2;
         _sza_cont[szindx++] = clsize;
         memcpy(&(point_array[_point_array_offset + pntindx]), (*CSH)->cdata(), 2 * sizeof(int4b) * clsize);
         pntindx += 2 * clsize;
      }
      assert(pntindx == line_arr_size + fqus_arr_size + cont_arr_size + poly_arr_size);
      assert(szindx  == _all_conts );
   }
}

void TenderTV::draw()
{
   glVertexPointer(2, GL_INT, 0, (GLvoid*)(sizeof(int4b) * _point_array_offset));
   if  (_all_lines > 0)
   {
      assert(_fst_line);
      assert(_sza_line);
      glMultiDrawArrays(GL_LINE_STRIP, _fst_line, _sza_line, _all_lines);
   }
   if  (_all_cnvx > 0)
   {
      assert(_fst_cnvx);
      assert(_sza_cnvx);
      glMultiDrawArrays(GL_LINE_LOOP, _fst_cnvx, _sza_cnvx, _all_cnvx);
      glMultiDrawArrays(GL_QUADS, _fst_cnvx, _sza_cnvx, _all_cnvx);
   }
   if  (_all_ncvx > 0)
   {
      assert(_fst_ncvx);
      assert(_sza_ncvx);
      glMultiDrawArrays(GL_LINE_LOOP, _fst_ncvx, _sza_ncvx, _all_ncvx);

      if (_all_fqss > 0)
      {
         glMultiDrawElements(GL_QUAD_STRIP    , _sza_fqss, GL_UNSIGNED_INT, (const GLvoid**)_fst_fqss, _all_fqss);
      }
      if (_all_ftrs > 0)
      {
         glMultiDrawElements(GL_TRIANGLES     , _sza_ftrs, GL_UNSIGNED_INT, (const GLvoid**)_fst_ftrs, _all_ftrs);
      }
      if (_all_ftfs > 0)
      {
         glMultiDrawElements(GL_TRIANGLE_FAN  , _sza_ftfs, GL_UNSIGNED_INT, (const GLvoid**)_fst_ftfs, _all_ftfs);
      }
      if (_all_ftss > 0)
      {
         glMultiDrawElements(GL_TRIANGLE_STRIP, _sza_ftss, GL_UNSIGNED_INT, (const GLvoid**)_fst_ftss, _all_ftss);
      }

   }
   if (_all_conts > 0)
   {
      assert(_fst_cont);
      assert(_sza_cont);
      glMultiDrawArrays(GL_LINE_LOOP, _fst_cont, _sza_cont, _all_conts);
   }
}

TenderTV::~TenderTV()
{
   for (SliceWires::const_iterator CSO = _line_data.begin(); CSO != _line_data.end(); CSO++)
      if ((*CSO)->center_line_only()) delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cnvx_data.begin(); CSO != _cnvx_data.end(); CSO++)
      delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cont_data.begin(); CSO != _cont_data.end(); CSO++)
      delete (*CSO);
   if (NULL != _sza_cont)
   {
      delete [] _sza_cont; _sza_cont = NULL;
   }
   if (NULL != _sza_line)
   {
      delete [] _sza_line; _sza_line = NULL;
   }
   if (NULL != _sza_cnvx)
   {
      delete [] _sza_cnvx; _sza_cnvx = NULL;
   }
   if (NULL != _sza_ncvx)
   {
      delete [] _sza_ncvx; _sza_cnvx = NULL;
   }
   if (NULL != _fst_cont)
   {
      delete [] _fst_cont; _fst_cont = NULL;
   }
   if (NULL != _fst_line)
   {
      delete [] _fst_line; _fst_line = NULL;
   }
   if (NULL != _fst_cnvx)
   {
      delete [] _fst_cnvx; _fst_cnvx = NULL;
   }
   if (NULL != _fst_ncvx)
   {
      delete [] _fst_ncvx; _fst_cnvx = NULL;
   }
}

//=============================================================================
/**
 * Create a new object of TenderTV type which will be reffered to by _cslice
 * @param ctrans Current translation matrix of the new object
 * @param fill Whether to fill the drawing objects
 */
void TenderLay::newSlice(CTM& ctrans, bool fill)
{
   _cslice = DEBUG_NEW TenderTV(ctrans, fill, 2 * _num_total_points, _num_total_indexs);
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
      }
      else
         delete _cslice;
      _cslice = NULL;
   }
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
                   _num_total_indexs * sizeof(int4b) ,
                   NULL                              ,
                   GL_DYNAMIC_DRAW                    );
      cindex_array = (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
   }
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      (*TLAY)->collect(cpoint_array, cindex_array);
   glUnmapBuffer(GL_ARRAY_BUFFER);
   if (0 != _ibuffer)
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

}

void TenderLay::draw(bool fill)
{
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   if (0 != _ibuffer)
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
   }
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      TenderTV* ctv = (*TLAY);
      glPushMatrix();
      real openGLmatrix[16];
      ctv->tmatrix()->oglForm(openGLmatrix);
      glMultMatrixd(openGLmatrix);
      ctv->draw();
      glPopMatrix();
   }
}

TenderLay::~TenderLay()
{
   for (TenderTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      delete (*TLAY);
}

//=============================================================================
Tenderer::Tenderer( layprop::DrawProperties* drawprop, real UU ) :
      _drawprop(drawprop), _UU(UU), _clayer(NULL),
      _num_ogl_buffers(0u), _ogl_buffers(NULL)
{}

void Tenderer::setLayer(word layer)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with layer 0 by accident
   assert(layer);
   if (NULL != _clayer)
      _clayer->ppSlice();
   if (_data.end() != _data.find(layer))
   {
      _clayer = _data[layer];
   }
   else
   {
      _clayer = DEBUG_NEW TenderLay();
      _data[layer] = _clayer;
   }
   _clayer->newSlice(_ctrans, _drawprop->isFilled(layer));
   // @TODO! current fill on/off should be determined here!
}

void Tenderer::setSdataContainer(word layer)
{
   _sslice = DEBUG_NEW TenderTVB();
   _sdata[layer] = _sslice;
}

void Tenderer::pushCell(const CTM& trans, const DBbox& overlap, bool active, bool selected)
{
   _ctrans = trans * _drawprop->topCTM();
   _oboxes.push_back(DEBUG_NEW TenderRB(_ctrans, overlap));
   if (selected)
      _osboxes.push_back(DEBUG_NEW TenderRB(_ctrans, overlap));
   _drawprop->pushCTM(_ctrans);
   if (active)
      _atrans = trans;
}

void Tenderer::box  (int4b* pdata, const SGBitSet* psel)
{
   assert(_sslice);
   TenderObj* dobj = _clayer->box(pdata);
   _sslice->add(dobj, psel);
}

void Tenderer::poly (int4b* pdata, unsigned psize, TeselPoly* tpoly, const SGBitSet* psel)
{
   assert(_sslice);
   TenderPoly* dpoly = _clayer->poly(pdata, psize, tpoly);
   _sslice->add(dpoly, psel);
}

void Tenderer::wire (int4b* pdata, unsigned psize, word width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM());
   _clayer->wire(pdata, psize, width, center_line_only);
}

void Tenderer::wire (int4b* pdata, unsigned psize, word width, const SGBitSet* psel)
{
   assert(_sslice);
   TenderWire* dwire = _clayer->wire(pdata, psize, width, false);
   _sslice->add(dwire, psel);
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
   while (CCLAY != _data.end())
   {
      CCLAY->second->ppSlice();
      if (0 == CCLAY->second->total_points())
      {
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
         _num_ogl_buffers++;
         if (0 < CCLAY->second->total_indexs())
            _num_ogl_buffers++;
         CCLAY++;
      }
   }

   // Organise the VBOs ...
   _ogl_buffers = DEBUG_NEW GLuint [_num_ogl_buffers];
   glGenBuffers(_num_ogl_buffers, _ogl_buffers);
   unsigned current_buffer = 0;
   for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      assert (0 != CLAY->second->total_points());
      GLuint pbuf = _ogl_buffers[current_buffer++];
      GLuint ibuf = (0 == CLAY->second->total_indexs()) ? 0u : _ogl_buffers[current_buffer++];
      CLAY->second->collect(_drawprop->isFilled(CLAY->first), pbuf, ibuf);
   }
}

void Tenderer::draw()
{
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      word curlayno = CLAY->first;
      _drawprop->setCurrentColor(curlayno);
      CLAY->second->draw( _drawprop->getCurrentFill() );
   }
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDeleteBuffers(_num_ogl_buffers, _ogl_buffers);
   delete [] _ogl_buffers;
   _ogl_buffers = NULL;
}

// void Tenderer::collectNdraw()
// {
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
//    {
//       word curlayno = CLAY->first;
//       _drawprop->setCurrentColor(curlayno);
//       CLAY->second->collectNdraw(_drawprop->getCurrentFill());
//    }
//    // now the overlapping boxes of the cell references
//    _drawprop->setCurrentColor(0);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
// //   glDisable(GL_POLYGON_STIPPLE);   //- for solid fill
//    for (TenderRBL::const_iterator CBOX = _oboxes.begin(); CBOX != _oboxes.end(); CBOX++)
//    {
//       (*CBOX)->draw();
//    }
//    _drawprop->setLineProps(true);
//    for (TenderRBL::const_iterator CBOX = _osboxes.begin(); CBOX != _osboxes.end(); CBOX++)
//    {
//       (*CBOX)->draw();
//    }
//    _drawprop->setLineProps(false);
//    // and finally -  the selected objects
//    glPushMatrix();
//    real openGLmatrix[16];
//    _atrans.oglForm(openGLmatrix);
//    glMultMatrixd(openGLmatrix);
//    for (DataSel::const_iterator CLAY = _sdata.begin(); CLAY != _sdata.end(); CLAY++)
//    {
//       _drawprop->setCurrentColor(CLAY->first);
//       _drawprop->setLineProps(true);
//       TenderTVB* ctv = CLAY->second;
//       //
//       ctv->draw_lloops();
//       ctv->draw_lines();
//       ctv->draw_lsegments();
//       //
//       _drawprop->setLineProps(false);
//    }
//    glPopMatrix();
//    glDisableClientState(GL_VERTEX_ARRAY);
// }

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

   for (TenderRBL::const_iterator CRBL = _oboxes.begin(); CRBL != _oboxes.end(); CRBL++)
      delete (*CRBL);

   sprintf (debug_message, "Rendering summary: %i vertexes in %i buffers", all_points_drawn, all_layers);
   tell_log(console::MT_WARNING,debug_message);
}

//=============================================================================
//
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
   sprintf (time_message, "%s:   %i sec. %06i msec.",message.c_str(), result.tv_sec, result.tv_usec);
#endif
   tell_log(console::MT_INFO,time_message);
}
