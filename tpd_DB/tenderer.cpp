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
// class TenderTV 
void TenderTV::box (const TP* p1, const TP* p2)
{
   TenderObj* cobj = DEBUG_NEW TenderObj(p1,p2);
   _data.push_front(cobj);
   _num_contour_points += 4;
   _num_objects += 1;
}

void TenderTV::poly (const pointlist& plst)
{
   TenderObj* cobj = DEBUG_NEW TenderPoly(plst);
   _data.push_front(cobj);
   _num_contour_points += cobj->csize();
   _num_objects += 1;
}

void TenderTV::wire (const pointlist& plst, word width, bool center_line_only)
{
   TenderObj* cobj = DEBUG_NEW TenderWire(plst, width, center_line_only);
   _data.push_front(cobj);
   if (cobj->csize() > 0)
   {
      _num_contour_points += cobj->csize();
      _num_objects += 1;
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

//=============================================================================
TenderPoly::TenderPoly(const pointlist& plst) : TenderObj(plst),
                       _fdata(NULL), _fsize(0)
{
}

void TenderPoly::Tessel()
{
   TeselTempData pfdata(&_fdata, &_fsize);
   // Start tessellation
   gluTessBeginPolygon(tenderTesel, &pfdata);
   GLdouble pv[3];
   pv[2] = 0;
   word* index_arr = DEBUG_NEW word[_csize/2];
   for (unsigned i = 0; i < _csize/2; i++ )
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
   ptmp->_teseldata = DEBUG_NEW TeselVertices();
}

GLvoid TenderPoly::teselVertex(GLvoid *pindex, GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   word *pidx = static_cast<word*>(pindex);
   ptmp->_teseldata->push_back(*pidx);
}

GLvoid TenderPoly::teselEnd(GLvoid* ttmp)
{
   TeselTempData* ptmp = static_cast<TeselTempData*>(ttmp);
   *(ptmp->_fsize) = ptmp->_teseldata->size();
   *(ptmp->_pfdata) = DEBUG_NEW word[*(ptmp->_fsize)];
   word index = 0;
   for (TeselVertices::const_iterator CV = ptmp->_teseldata->begin(); CV != ptmp->_teseldata->end(); CV++)
      (*ptmp->_pfdata)[index++] = *CV;
   delete ptmp->_teseldata;ptmp->_teseldata = NULL;
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
   _cdata = DEBUG_NEW int(2 * _csize);
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
}

DBbox* TenderWire::endPnts(const word width, word i1, word i2, bool first)
{
   double     w = width/2;
   i1 *= 2; i2 *= 2;
   double denom = first ? (_ldata[i2  ] - _ldata[i1  ]) : (_ldata[i1  ] - _ldata[i2  ]);
   double   nom = first ? (_ldata[i2+1] - _ldata[i1+1]) : (_ldata[i1+1] - _ldata[i2+1]);
   double xcorr, ycorr; // the corrections
   if ((0 == nom) && (0 == denom)) return NULL;
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
// @FIXME THINK about next two lines!!!    They are wrong !!!
   if ((0 == denom) || (0 == L1)) return endPnts(width, i2*2, i3*2, false);
   if (0 == L2) return NULL;
   // the corrections
   double xcorr = w * ((x32 * L1 - x21 * L2) / denom);
   double ycorr = w * ((y21 * L2 - y32 * L1) / denom);
   return DEBUG_NEW DBbox((int4b) rint(_ldata[i2  ] - xcorr),
                          (int4b) rint(_ldata[i2+1] + ycorr),
                          (int4b) rint(_ldata[i2  ] + xcorr),
                          (int4b) rint(_ldata[i2+1] - ycorr) );
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
   laydata->push_front(_cslice);
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
      for (TenderLay::const_iterator TLAY = CLAY->second->begin(); TLAY != CLAY->second->end(); TLAY++)
      {
         TenderTV* ctv = (*TLAY);
         glPushMatrix();
         real openGLmatrix[16];
         ctv->tmatrix()->oglForm(openGLmatrix);
         glMultMatrixd(openGLmatrix);
         //@TODO! Drawing here!
         unsigned long arr_size = 2 * (*TLAY)->num_contour_points();
         unsigned objts_size = (*TLAY)->num_objects();
         int* point_array = DEBUG_NEW int[arr_size];
         GLsizei* size_array = DEBUG_NEW int[objts_size];
         GLsizei* first_array = DEBUG_NEW int[objts_size];
         unsigned long pntindx = 0;
         unsigned      szindx  = 0;

         for (SliceObjects::const_iterator CSH = (*TLAY)->data()->begin(); CSH != (*TLAY)->data()->end(); CSH++)
         { // shapes in the current translation (layer within the cell)
            first_array[szindx] = pntindx/2;
            size_array[szindx++] = (*CSH)->csize();
            for (word ipnt = 0; ipnt < 2 * (*CSH)->csize() ; ipnt++)
            { // points in the shape
               point_array[pntindx++] = (*CSH)->cdata()[ipnt];
            }
         }
         assert(pntindx == arr_size);
         assert(szindx == objts_size);
         glVertexPointer(2, GL_INT, 0, point_array);
         glMultiDrawArrays(GL_LINE_LOOP, first_array, size_array, szindx);
         //
         glPopMatrix();
         delete [] point_array;
         delete [] size_array;
         delete [] first_array;
      }
   }
   glDisableClientState(GL_VERTEX_ARRAY);
}

// void Tenderer::add_data(const laydata::atticList* cell4Drawing, const SLMap* numPoints)
// {
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glPushMatrix();
//    real openGLmatrix[16];
//    _drawprop->topCTM().oglForm(openGLmatrix);
// //    printf("==========================================================\n");
// //    for (int bozai = 0; bozai < 4; bozai++)
// //    {
// //       for (int bozaj = 0; bozaj < 4; bozaj++)
// //          printf(" %1.4f",openGLmatrix[bozai * 4 + bozaj]);
// //       printf("\n");
// //    }
//    glMultMatrixd(openGLmatrix);
// 
//    for (laydata::atticList::const_iterator CLAY = cell4Drawing->begin(); CLAY != cell4Drawing->end(); CLAY++)
//    { // layers in the cell
//       word curlayno = CLAY->first;
//       assert(numPoints->end() != numPoints->find(curlayno));
//       unsigned long arr_size = 2 * numPoints->find(curlayno)->second;
//       int* point_array = DEBUG_NEW int[arr_size];
//       GLsizei* size_array = DEBUG_NEW int[CLAY->second->size()];
//       GLsizei* first_array = DEBUG_NEW int[CLAY->second->size()];
//       unsigned long pntindx = 0;
//       unsigned      szindx  = 0;
//       _drawprop->setCurrentColor(curlayno);
//       for (laydata::shapeList::const_iterator CSH = CLAY->second->begin(); CSH != CLAY->second->end(); CSH++)
//       { // shapes in a layer
//          pointlist shape_points;
//          (*CSH)->tender_gen(shape_points);
//          first_array[szindx] = pntindx/2;
//          size_array[szindx++] = shape_points.size();
//          for (pointlist::const_iterator CP = shape_points.begin(); CP != shape_points.end(); CP++)
//          { // points in the shape
//             point_array[pntindx++] = CP->x();
//             point_array[pntindx++] = CP->y();
//          }
//       }
//       assert(pntindx == arr_size);
//       assert(szindx == CLAY->second->size());
//       glVertexPointer(2, GL_INT, 0, point_array);
//       glMultiDrawArrays(GL_LINE_LOOP, first_array, size_array, szindx);
// 
//    }
//    glPopMatrix();
//    glDisableClientState(GL_VERTEX_ARRAY);
// }
