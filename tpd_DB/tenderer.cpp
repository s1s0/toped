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

//=============================================================================
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

void Tenderer::add_data(const laydata::atticList* cell4Drawing, const SLMap* numPoints)
{
   glEnableClientState(GL_VERTEX_ARRAY);
   glPushMatrix();
   real openGLmatrix[16];
   _drawprop->topCTM().oglForm(openGLmatrix);
   glMultMatrixd(openGLmatrix);

   for (laydata::atticList::const_iterator CLAY = cell4Drawing->begin(); CLAY != cell4Drawing->end(); CLAY++)
   { // layers in the cell
      word curlayno = CLAY->first;
      assert(numPoints->end() != numPoints->find(curlayno));
      unsigned long arr_size = 2 * numPoints->find(curlayno)->second;
      int* point_array = new int[arr_size];
      GLsizei* size_array = new int[CLAY->second->size()];
      GLsizei* first_array = new int[CLAY->second->size()];
      unsigned long pntindx = 0;
      unsigned      szindx  = 0;
      _drawprop->setCurrentColor(curlayno);
      for (laydata::shapeList::const_iterator CSH = CLAY->second->begin(); CSH != CLAY->second->end(); CSH++)
      { // shapes in a layer
         pointlist shape_points;
         (*CSH)->tender_gen(shape_points);
         first_array[szindx] = pntindx/2;
         size_array[szindx++] = shape_points.size();
         for (pointlist::const_iterator CP = shape_points.begin(); CP != shape_points.end(); CP++)
         { // points in the shape
            point_array[pntindx++] = CP->x();
            point_array[pntindx++] = CP->y();
         }
      }
      assert(pntindx == arr_size);
      assert(szindx == CLAY->second->size());
      glVertexPointer(2, GL_INT, 0, point_array);
      glMultiDrawArrays(GL_LINE_LOOP, first_array, size_array, szindx);

   }
   glPopMatrix();
   glDisableClientState(GL_VERTEX_ARRAY);
}
