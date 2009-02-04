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
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>
#include "tenderer.h"
#include "viewprop.h"
#include "tedat.h"

GLUtriangulatorObj   *TenderPoly::tenderTesel = NULL;

//=============================================================================
//
TeselChunk::TeselChunk(const TeselVertices& data, GLenum type, unsigned offset)
{
   _size = data.size();
   _index_seq = new unsigned[_size];
   word li = 0;
   for(TeselVertices::const_iterator CVX = data.begin(); CVX != data.end(); CVX++)
      _index_seq[li++] = *CVX + offset;
   _type = type;
}

TeselChunk::~TeselChunk()
{
   delete [] _index_seq;
}
//=============================================================================
//

TeselTempData::TeselTempData(unsigned offset) :_the_chain(NULL), _cindexes(),
           _num_ftrs(0)      , _num_ftfs(0)      , _num_ftss(0), _offset(offset)
{}

void TeselTempData::storeChunk()
{
   TeselChunk* achunk = DEBUG_NEW TeselChunk(_cindexes, _ctype, _offset);
   _the_chain->push_back(achunk);
   switch (_ctype)
   {
      case GL_TRIANGLE_FAN   : _num_ftfs++; break;
      case GL_TRIANGLE_STRIP : _num_ftss++; break;
      case GL_TRIANGLES      : _num_ftrs++; break;
      default: assert(0);
   }
}

//=============================================================================
//
TenderObj::TenderObj(const TP* p1, const TP* p2)
{
   _csize = 4;
   _cdata = new int[8];
   word index = 0;
   _cdata[index++] = p1->x();_cdata[index++] = p1->y();
   _cdata[index++] = p2->x();_cdata[index++] = p1->y();
   _cdata[index++] = p2->x();_cdata[index++] = p2->y();
   _cdata[index++] = p1->x();_cdata[index++] = p2->y();
}

TenderObj::TenderObj(const pointlist& plst)
{
   _csize = plst.size();
   if (0 == _csize)
      _cdata = NULL;
   else
   {
      _cdata = new int[_csize*2];
      word index = 0;
      for (unsigned i = 0; i < _csize; i++)
      {
         _cdata[index++] = plst[i].x();
         _cdata[index++] = plst[i].y();
      }
   }
}

TenderObj::~TenderObj()
{
   if (_cdata) delete [] _cdata;
}
//=============================================================================
TenderPoly::TenderPoly(const pointlist& plst) : TenderObj(plst)/*,
                       _fdata(NULL), _fsize(0)*/
{
}

void TenderPoly::Tessel(TeselTempData* ptdata)
{
   ptdata->setChainP(&_tdata);
   // Start tessellation
   gluTessBeginPolygon(tenderTesel, ptdata);
   GLdouble pv[3];
   pv[2] = 0;
   word* index_arr = DEBUG_NEW word[_csize];
   for (unsigned i = 0; i < _csize; i++ )
   {
      pv[0] = _cdata[2*i]; pv[1] = _cdata[2*i+1];
      index_arr[i] = i;
      gluTessVertex(tenderTesel,pv, &(index_arr[i]));
   }
   gluTessEndPolygon(tenderTesel);
}

GLvoid TenderPoly::teselBegin(GLenum type, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newChunk(type);
}

GLvoid TenderPoly::teselVertex(GLvoid *pindex, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->newIndex(*(static_cast<word*>(pindex)));
}

GLvoid TenderPoly::teselEnd(GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   ptmp->storeChunk();
}

TenderPoly::~TenderPoly()
{
   for (TeselChain::const_iterator CTC = _tdata.begin(); CTC != _tdata.end(); CTC++)
      delete (*CTC);
}

//=============================================================================
TenderWire::TenderWire(const pointlist& plst, const word width, bool center_line_only) : TenderPoly()
{
   _lsize = plst.size();
   assert(0 != _lsize);
   _ldata = new int[2 * _lsize];
   word index = 0;
   for (unsigned i = 0; i < _lsize; i++)
   {
      _ldata[index++] = plst[i].x();
      _ldata[index++] = plst[i].y();
   }
   assert(index == 2*_lsize);
   if (!center_line_only)
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

TenderWire::~TenderWire()
{
   if (_ldata) delete [] _ldata;
}
//=============================================================================
// class TenderTV
TenderTV::TenderTV(CTM& translation) : _tmatrix(translation),
    _num_contour_points (0u), _num_line_points(0u), _num_polygon_points(0u),
    _num_contours(0),         _num_lines(0),        _num_fqus(0),
    _num_fqss(0),             _num_ftrs(0),         _num_ftfs(0),
    _num_ftss(0)
{}

void TenderTV::box (const TP* p1, const TP* p2)
{
   TenderObj* cobj = DEBUG_NEW TenderObj(p1,p2);
   _contour_data.push_back(cobj);
   _num_contour_points += 4;
   _num_contours++;
   _fqu_data.push_back(cobj);
   _num_fqus++;
}

void TenderTV::poly (const pointlist& plst)
{
   TenderPoly* cobj = DEBUG_NEW TenderPoly(plst);
   _contour_data.push_back(cobj);
   _num_contour_points += cobj->csize();
   _num_contours += 1;
//   if (_fill) //@TODO!
//   {
      TeselTempData tdata(_num_polygon_points );
      cobj->Tessel(&tdata);
      _fpolygon_data.push_back(cobj);
      _num_polygon_points += cobj->csize();
      _num_ftrs += tdata.num_ftrs();
      _num_ftfs += tdata.num_ftfs();
      _num_ftss += tdata.num_ftss();
//   }
}

void TenderTV::wire (const pointlist& plst, word width, bool center_line_only)
{
   TenderObj* cobj = DEBUG_NEW TenderWire(plst, width, center_line_only);
   _line_data.push_back(cobj);
   _num_line_points += cobj->lsize();
   _num_lines += 1;
   if (!center_line_only)
   {
      _contour_data.push_back(cobj);
      _num_contour_points += cobj->csize();
      _num_contours += 1;
   }
}

void TenderTV::draw_contours()
{
   if  (0 == _num_contours) return;
   unsigned long arr_size = 2 * _num_contour_points;
   int* point_array = DEBUG_NEW int[arr_size];
   GLsizei* size_array = DEBUG_NEW int[_num_contours];
   GLsizei* first_array = DEBUG_NEW int[_num_contours];
   unsigned long pntindx = 0;
   unsigned      szindx  = 0;

   for (SliceObjects::const_iterator CSH = _contour_data.begin(); CSH != _contour_data.end(); CSH++)
   { // shapes in the current translation (layer within the cell)
      assert((*CSH)->csize());
      first_array[szindx] = pntindx/2;
      size_array[szindx++] = (*CSH)->csize();
      for (word ipnt = 0; ipnt < 2 * (*CSH)->csize() ; ipnt++)
      { // points in the shape
         point_array[pntindx++] = (*CSH)->cdata()[ipnt];
      }
   }
   assert(pntindx == arr_size);
   assert(szindx == _num_contours);
   glVertexPointer(2, GL_INT, 0, point_array);
   glMultiDrawArrays(GL_LINE_LOOP, first_array, size_array, szindx);

   delete [] point_array;
   delete [] size_array;
   delete [] first_array;
}

void TenderTV::draw_lines()
{
   if  (0 == _num_lines) return;
   unsigned long arr_size = 2 * _num_line_points;
   int* point_array = DEBUG_NEW int[arr_size];
   GLsizei* size_array = DEBUG_NEW int[_num_lines];
   GLsizei* first_array = DEBUG_NEW int[_num_lines];
   unsigned long pntindx = 0;
   unsigned      szindx  = 0;

   for (SliceObjects::const_iterator CSH = _line_data.begin(); CSH != _line_data.end(); CSH++)
   { // shapes in the current translation (layer within the cell)
      assert((*CSH)->lsize());
      first_array[szindx] = pntindx/2;
      size_array[szindx++] = (*CSH)->lsize();
      for (word ipnt = 0; ipnt < 2 * (*CSH)->lsize() ; ipnt++)
      { // points in the shape
         point_array[pntindx++] = (*CSH)->ldata()[ipnt];
      }
   }
   assert(pntindx == arr_size);
   assert(szindx == _num_lines);
   glVertexPointer(2, GL_INT, 0, point_array);
   glMultiDrawArrays(GL_LINE_STRIP, first_array, size_array, szindx);

   delete [] point_array;
   delete [] size_array;
   delete [] first_array;
}

void TenderTV::draw_fqus()
{
   if  (0 == _num_fqus) return;
   unsigned long arr_size = 8 * _num_fqus; //2 * 4
   int* point_array = DEBUG_NEW int[arr_size];
   GLsizei* size_array = DEBUG_NEW int[_num_fqus];
   GLsizei* first_array = DEBUG_NEW int[_num_fqus];
   unsigned long pntindx = 0;
   unsigned      szindx  = 0;

   for (SliceObjects::const_iterator CSH = _fqu_data.begin(); CSH != _fqu_data.end(); CSH++)
   { // shapes in the current translation (layer within the cell)
      assert((*CSH)->csize());
      first_array[szindx] = pntindx/2;
      size_array[szindx++] = (*CSH)->csize();
      for (word ipnt = 0; ipnt < 2 * (*CSH)->csize() ; ipnt++)
      { // points in the shape
         point_array[pntindx++] = (*CSH)->cdata()[ipnt];
      }
   }
   assert(pntindx == arr_size);
   assert(szindx == _num_fqus);
   glVertexPointer(2, GL_INT, 0, point_array);
   glMultiDrawArrays(GL_QUADS, first_array, size_array, szindx);

   delete [] point_array;
   delete [] size_array;
   delete [] first_array;
}

void TenderTV::draw_fpolygons()
{
   unsigned long arr_size = 2 * _num_polygon_points;
   if  (0 == arr_size) return;
   unsigned          pntindx       = 0;
   unsigned          sz_ftrs_indx  = 0;
   unsigned          sz_ftfs_indx  = 0;
   unsigned          sz_ftss_indx  = 0;
   GLsizei*          sz_ftrs_array = NULL;
   GLsizei*          sz_ftfs_array = NULL;
   GLsizei*          sz_ftss_array = NULL;
   const GLvoid**    ix_ftrs_array = NULL;
   const GLvoid**    ix_ftfs_array = NULL;
   const GLvoid**    ix_ftss_array = NULL;

   int* point_array = DEBUG_NEW int[arr_size];
   if (_num_ftrs)
   {
      sz_ftrs_array = DEBUG_NEW       GLsizei[_num_ftrs];
      ix_ftrs_array = DEBUG_NEW const GLvoid*[_num_ftrs];
   }
   if (_num_ftfs)
   {
      sz_ftfs_array = DEBUG_NEW       GLsizei[_num_ftfs];
      ix_ftfs_array = DEBUG_NEW const GLvoid*[_num_ftfs];
   }
   if (_num_ftss)
   {
      sz_ftss_array = DEBUG_NEW       GLsizei[_num_ftss];
      ix_ftss_array = DEBUG_NEW const GLvoid*[_num_ftss];
   }

   for (SlicePolygons::const_iterator CSH = _fpolygon_data.begin(); CSH != _fpolygon_data.end(); CSH++)
   { // shapes in the current translation (layer within the cell)
      TeselChain* tdata = (*CSH)->tdata();
      for (TeselChain::const_iterator TCH = tdata->begin(); TCH != tdata->end(); TCH++)
      {
         TeselChunk* cchunk = *TCH;
         switch (cchunk->type())
         {
            case GL_TRIANGLE_FAN   :
            {
               assert(sz_ftfs_array); assert(ix_ftfs_array);
               sz_ftfs_array[sz_ftfs_indx  ] = cchunk->size();
               ix_ftfs_array[sz_ftfs_indx++] = cchunk->index_seq();
               break;
            }
            case GL_TRIANGLE_STRIP :
            {
               assert(sz_ftss_array); assert(ix_ftss_array);
               sz_ftss_array[sz_ftss_indx  ] = cchunk->size();
               ix_ftss_array[sz_ftss_indx++] = cchunk->index_seq();
               break;
            }
            case GL_TRIANGLES      :
            {
               assert(sz_ftrs_array); assert(ix_ftrs_array);
               sz_ftrs_array[sz_ftrs_indx  ] = cchunk->size();
               ix_ftrs_array[sz_ftrs_indx++] = cchunk->index_seq();
               break;
            }
            default: assert(0);
         }
      }
      for (word ipnt = 0; ipnt < 2 * (*CSH)->csize() ; ipnt++)
      { // points in the shape
         point_array[pntindx++] = (*CSH)->cdata()[ipnt];
      }
   }
   assert(pntindx == arr_size);
   assert(sz_ftrs_indx == _num_ftrs);
   assert(sz_ftfs_indx == _num_ftfs);
   assert(sz_ftss_indx == _num_ftss);

   glVertexPointer(2, GL_INT, 0, point_array);
   if (sz_ftrs_indx > 0)
   {
      glMultiDrawElements(GL_TRIANGLES, sz_ftrs_array, GL_UNSIGNED_INT,
                          ix_ftrs_array, sz_ftrs_indx);
      delete[] ix_ftrs_array;
      delete[] sz_ftrs_array;
   }
   if (sz_ftfs_indx > 0)
   {
      glMultiDrawElements(GL_TRIANGLE_FAN, sz_ftfs_array, GL_UNSIGNED_INT,
                          ix_ftfs_array, sz_ftfs_indx);
      delete[] ix_ftfs_array;
      delete[] sz_ftfs_array;
   }
   if (sz_ftss_indx > 0)
   {
      glMultiDrawElements(GL_TRIANGLE_STRIP, sz_ftss_array, GL_UNSIGNED_INT,
                          ix_ftss_array, sz_ftss_indx);
      delete[] ix_ftss_array;
      delete[] sz_ftss_array;
   }
   delete [] point_array;
}

//=============================================================================
Tenderer::Tenderer( layprop::DrawProperties* drawprop, real UU ) :
      _drawprop(drawprop), _UU(UU), _cslice(NULL)
{
   TenderPoly::tenderTesel = gluNewTess();
#ifndef WIN32
   gluTessCallback(TenderPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                   (GLvoid(*)())&TenderPoly::teselBegin);
   gluTessCallback(TenderPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                   (GLvoid(*)())&TenderPoly::teselVertex);
   gluTessCallback(TenderPoly::tenderTesel, GLU_TESS_END_DATA,
                   (GLvoid(*)())&TenderPoly::teselEnd);
#else
   gluTessCallback(TenderPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                   (GLvoid(__stdcall *)())&TenderPoly::teselBegin);
   gluTessCallback(TenderPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                   (GLvoid(__stdcall *)())&TenderPoly::teselVertex);
   gluTessCallback(TenderPoly::tenderTesel, GLU_TESS_END_DATA,
                   (GLvoid(__stdcall *)())&TenderPoly::teselEnd);
#endif
}

void Tenderer::setLayer(word layer)
{
   if (0==layer) return; // @FIXME!, temporaray, until cell overlap is fixed
   TenderLay* laydata = NULL;
   if (_data.end() != _data.find(layer))
   {
      laydata = _data[layer];
   }
   else
   {
      laydata = DEBUG_NEW TenderLay();
      _data[layer] = laydata;
   }
   _cslice = DEBUG_NEW TenderTV(_ctrans);
   laydata->push_back(_cslice);
   // @TODO! current fill on/off should be determined here!
}

void Tenderer::wire (const pointlist& plst, word width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * ScrCTM());
   _cslice->wire(plst, width, center_line_only);
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
      int* point_array = new int[arr_size * 2];
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

void Tenderer::draw()
{
   glEnableClientState(GL_VERTEX_ARRAY);
   for (DataLay::const_iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      word curlayno = CLAY->first;
      _drawprop->setCurrentColor(curlayno);
      bool fill = _drawprop->getCurrentFill();
      for (TenderLay::const_iterator TLAY = CLAY->second->begin(); TLAY != CLAY->second->end(); TLAY++)
      {
         TenderTV* ctv = (*TLAY);
         glPushMatrix();
         real openGLmatrix[16];
         ctv->tmatrix()->oglForm(openGLmatrix);
         glMultMatrixd(openGLmatrix);
         //
         (*TLAY)->draw_contours();
         (*TLAY)->draw_lines();
         if (fill)
         {
            (*TLAY)->draw_fqus();
            (*TLAY)->draw_fpolygons();
         }
         //
         glPopMatrix();
      }
   }
   glDisableClientState(GL_VERTEX_ARRAY);

}
