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
#ifndef TEDOP_H_INCLUDED
#define TEDOP_H_INCLUDED

#include <vector>
#include <list>
#include "ttt.h"

namespace tedop {

/*   class plysegment {
   public:
                        plysegment(const TP*, const TP*, unsigned);
      unsigned          edge; // to keep track of adjacent edges
      const TP*         lP;
      const TP*         rP;
      plysegment*       above;
      plysegment*       below;
   private:
      int xyorder(const TP*, const TP*);
   };
   
   //===========================================================================
   class segmentlist {
   public:
                        segmentlist(const pointlist&, bool);
      plysegment*       operator [](unsigned int& i) const {return segs[i];};
      unsigned          size() const {return segs.size();};
                       ~segmentlist();
   private:
      std::vector<plysegment*> segs;
   };*/
}
#endif
