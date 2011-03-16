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
//   This file is a part of Toped project (C) 2001-2007 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Mar 07 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: QuadTree - supplementary classes
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "quadtree.h"
#include "qtree_tmpl.h"

laydata::QuadProps::QuadProps(): _numObjects(0), _invalid(false), _quadMap(0)
{}

byte laydata::QuadProps::numSubQuads() const
{
   assert(_quadMap < 16);
   switch (_quadMap)
   {
      case  0: return 0;
      case  1:
      case  2:
      case  4:
      case  8: return 1;
      case  7:
      case 11:
      case 13:
      case 14: return 3;
      case 15: return 4;
      default: return 2;
   }
}

char laydata::QuadProps::getPosition(QuadIdentificators quad)
{
   switch(quad)
   {
      case qidNW: return getNWQuad();
      case qidNE: return getNEQuad();
      case qidSE: return getSEQuad();
      case qidSW: return getSWQuad();
      default: assert(false);
   }
   // dummy statement to prevent compiler warnings
   return -1;
}

void laydata::QuadProps::addQuad(QuadIdentificators quad)
{
   _quadMap |= 0x01 << quad;
}

void laydata::QuadProps::removeQuad(QuadIdentificators quad)
{
   _quadMap &= ~(0x01 << quad);
}

char laydata::QuadProps::getSWQuad() const
{
   assert(_quadMap < 16);
   switch (_quadMap)
   {
      case  8: return  0;
      case  9:
      case 10: return  1;
      case 12: return  1;
      case 11:
      case 13:
      case 14: return  2;
      case 15: return  3;
      default: return -1;
   }
}

char laydata::QuadProps::getSEQuad() const
{
   assert(_quadMap < 16);
   switch (_quadMap % 8)
   {
      case  4: return  0;
      case  5:
      case  6: return  1;
      case  7: return  2;
      default: return -1;
   }
}

char laydata::QuadProps::getNEQuad() const
{
   assert(_quadMap < 16);
   switch (_quadMap % 4)
   {
      case 2: return 0;
      case 3: return 1;
      default: return -1;
   }

}

char laydata::QuadProps::getNWQuad() const
{
   assert(_quadMap < 16);
   if (_quadMap % 2) return 0;
   return -1;
}

//=============================================================================

void laydata::QTreeTmp::put(laydata::TdtData* shape)
{
   _trunk->updateOverlap(shape->overlap());
   _data.push_back(shape);
}

/*!Create new TdtBox. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
void laydata::QTreeTmp::putBox(const TP& p1, const TP& p2)
{
   laydata::TdtBox *shape = DEBUG_NEW TdtBox(p1,p2);
   put(shape);
}
/*!Create new TdtPoly. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
void laydata::QTreeTmp::putPoly(PointVector& pl)
{
   laydata::TdtPoly *shape = DEBUG_NEW TdtPoly(pl);
   put(shape);
}

void laydata::QTreeTmp::putPoly(int4b* pl, unsigned psize)
{
   laydata::TdtPoly *shape = DEBUG_NEW TdtPoly(pl, psize);
   put(shape);
}

/*!Create new TdtWire. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
void laydata::QTreeTmp::putWire(PointVector& pl,word w)
{
   laydata::TdtWire *shape = DEBUG_NEW TdtWire(pl,w);
   put(shape);
}
/*!Create new TdtText. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
void laydata::QTreeTmp::putText(std::string text,CTM trans)
{
   laydata::TdtText *shape = DEBUG_NEW TdtText(text,trans);
   put(shape);
}

void laydata::QTreeTmp::commit()
{
   _trunk->resort(_data);
}
