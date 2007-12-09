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
//        Created: Tue Sep 28 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Self crossing polygons algo
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <math.h>
#include <algorithm>
#include "tedop.h"

// //-----------------------------------------------------------------------------
// // class plysegment 
// //-----------------------------------------------------------------------------
// tedop::plysegment::plysegment(const TP* p1, const TP* p2, unsigned num) {
//    if (xyorder(p1, p2) < 0) { lP = p1; rP = p2; }
//    else                     { lP = p2; rP = p1; }
//    above = below = NULL;
//    edge = num;
// }
// 
// /*! Determines the lexicographical order of two points comparing X first.
//    Returns:
//    +1 -> p1  > p2
//    -1 -> p1  < p2
//     0 -> p1 == p2
//  */
// int tedop::plysegment::xyorder(const TP* p1, const TP* p2) {
//    // test X coord first
//    if (p1->x() > p2->x()) return  1;
//    if (p1->x() < p2->x()) return -1;
//    // and then y
//    if (p1->y() > p2->y()) return  1;
//    if (p1->y() < p2->y()) return -1;
//    return 0;
// }
// 
// //-----------------------------------------------------------------------------
// // class segmentlist 
// //-----------------------------------------------------------------------------
// tedop::segmentlist::segmentlist(const pointlist& plst, bool wire) {
//    byte adjustment = wire ? 1 : 0;
//    unsigned plysize = plst.size();
//    segs.reserve(plysize - adjustment);
//    for (unsigned i = 0; i < plysize - adjustment; i++)
//       segs.push_back(DEBUG_NEW plysegment(&(plst[i]),&(plst[(i+1)%plysize]),i));
// }
// 
// tedop::segmentlist::~segmentlist() {
//    for (unsigned i = 0; i < segs.size(); i++)
//       delete segs[i];
//    segs.clear();   
// }
