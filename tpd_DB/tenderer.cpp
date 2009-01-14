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
#include <GL/gl.h>
#include <GL/glu.h>
#include "tenderer.h"
#include "viewprop.h"

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

}
