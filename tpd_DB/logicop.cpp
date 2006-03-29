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
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Tue Sep 28 2004
//         Author: s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: polygon logic operations
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <assert.h>
#include <sstream>
#include "logicop.h"
#include "tedat.h"
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"

//-----------------------------------------------------------------------------
// class logic
//-----------------------------------------------------------------------------
/*!This is the place where Bentley-Ottmann algorithm is prepared and invoked.
As a result of it, all crossing points between the two initial polygons are
produced. As a second step, two raw data structures are produced, that replicate
the input polygons, but contain also the crossing points with the required links
between them. These data structures will be used in all subsequently called
methods, implementing the actual logic operations*/
logicop::logic::logic(const pointlist& poly1, const pointlist& poly2) :
                                                _poly1(poly1), _poly2(poly2) {
   polycross::segmentlist _segl1(poly1,1);
   polycross::segmentlist _segl2(poly2,2);
   polycross::XQ* _eq = new polycross::XQ(_segl1, _segl2); // create the event queue
   _eq->sweep();

   unsigned crossp1 = _segl1.normalize(poly1);
   unsigned crossp2 = _segl2.normalize(poly2);
   assert(crossp1 == crossp2);
   _crossp = crossp1;
   delete _eq;
   _shape1 = _segl1.dump_points();
   _shape2 = _segl2.dump_points();
}

/*!If more than one logical operatoin has to be executed over the input shapes
the raw data #_shape1 and #_shape2 can be reused, but has to be recycled beforehand
This method is traversing both fields and invokes VPoint::reset_visited() in 
order to reinitialize the CPoint::_visited fields*/
void logicop::logic::reset_visited() {
   polycross::VPoint* centinel = _shape1;
   polycross::VPoint* looper = centinel;
   do {
      looper->reset_visited();
      looper = looper->next();
   }  while (centinel != looper);
   centinel = _shape2;
   looper = centinel;
   do {
      looper->reset_visited();
      looper = looper->next();
   }  while (centinel != looper);
}
/*!The method uses #_shape1 and #_shape2 structure as an input data and produces
one or more polygons representing the result of the logical AND between the 
input polygons. Method returns false if no output shapes are generated, and
true otherwise*/
bool logicop::logic::AND(pcollection& plycol) {
   bool result = false;
   polycross::VPoint* centinel = NULL;
   if (0 == _crossp) {
      // If there are no crossing points found, this still does not mean
      // that the operation will fail. Polygons might be fully overlapping...
      // Check that a random point from poly1 is inside poly2 ...
      if       (_shape1->inside(_poly2)) centinel = _shape1;
      // ... if not, check that a random point from poly2 is inside poly1 ...
      else if (_shape2->inside(_poly1)) centinel = _shape2;
      // ... if not - polygons does not have any common area
      else return false;
      // If we've got here means that one of the polygons is completely 
      // overlapped by the other one. So we need to return the inner one
      pointlist *shgen = new pointlist();
      polycross::VPoint* vpnt = centinel;
      do {
         shgen->push_back(TP(vpnt->cp()->x(), vpnt->cp()->y()));
         vpnt = vpnt->next();
      }while (centinel != vpnt);
      plycol.push_back(shgen);
      return true;
   }
   bool direction = true; /*next*/
   //if crossing points exists, get first external and non crossing  point
   centinel = getFirstOutside(_poly2, _shape1);
   if (NULL == centinel) centinel = getFirstOutside(_poly1, _shape2);
   assert(centinel);   
   polycross::VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = new pointlist();
         polycross::VPoint* pickup = collector;
         do {
            pickup = pickup->follower(direction);
            shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
         } while (pickup != collector);
         plycol.push_back(shgen);
         result = true;
      }
      collector = collector->prev();
   } while (collector != centinel);
   return result;
}

/*!The method uses #_shape1 and #_shape2 structure as an input data and produces
one or more polygons representing the result of the logical ANDNOT between the 
input polygons. Method returns false if no output shapes are generated, and
true otherwise*/
bool logicop::logic::ANDNOT(pcollection& plycol) {
   bool result = false;
   polycross::VPoint* centinel = NULL;
   if (0 == _crossp) {
      // If there are no crossing points found, this still does not mean
      // that the operation will fail. Polygons might be overlapping...
      // if poly1 is inside poly2, or both are non overlapping -> 
      //      resulting shape is null
      // if poly2 is inside poly1, then we have to generate a polygon
      // combining both shapes
      if (_shape2->inside(_poly1)) {
         plycol.push_back(hole2simple(_poly1, _poly2));
         return true;
      }
      else return false;
   }
   //if crossing points exists, get first external and non crossing  point
   bool direction;
   centinel = getFirstOutside(_poly1, _shape2);
   if (NULL == centinel) {
      centinel = getFirstOutside(_poly2, _shape1);
      direction = false; /*prev*/
   }
   else direction = true; /*next*/
   assert(centinel);   
   //   
   polycross::VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = new pointlist();
         polycross::VPoint* pickup = collector;
         do {
            pickup = pickup->follower(direction, true);
            shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
         } while (pickup != collector);
         plycol.push_back(shgen);
         result = true;
      }
      collector = collector->prev();
   } while (collector != centinel);
   return result;
}

/*!The method uses #_shape1 and #_shape2 structure as an input data and produces
one or more polygons representing the result of the logical OR between the 
input polygons. Method returns false if no output shapes are generated, and
true otherwise*/
bool logicop::logic::OR(pcollection& plycol) {
   bool result = false;
   polycross::VPoint* centinel = NULL;
   if (0 == _crossp) {
      // If there are no crossing points found, this still does not mean
      // that the operation will fail. Polygons might be fully overlapping...
      // Check that a random point from poly1 is inside poly2 ...
      if       (_shape1->inside(_poly2)) centinel = _shape2;
      // ... if not, check that a random point from poly2 is inside poly1 ...
      else if (_shape2->inside(_poly1)) centinel = _shape1;
      // ... if not - polygons does not have any common area
      else return false;
      // If we've got here means that one of the polygons is completely 
      // overlapped by the other one. So we need to return the outer one
      pointlist *shgen = new pointlist();
      polycross::VPoint* vpnt = centinel;
      do {
         shgen->push_back(TP(vpnt->cp()->x(), vpnt->cp()->y()));
         vpnt = vpnt->next();
      }while (centinel != vpnt);
      plycol.push_back(shgen);
      return true;
   }
   pcollection lclcol; // local collection of the resulting shapes
   // get first external and non crossing  point
   centinel = getFirstOutside(_poly2, _shape1);
   if (NULL == centinel) centinel = getFirstOutside(_poly1, _shape2);
   assert(centinel);   
   polycross::VPoint* collector = centinel;
   bool direction = true; /*next*/
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = new pointlist();
         polycross::VPoint* pickup = collector;
         direction = (0 == lclcol.size());
         do {
            pickup = pickup->follower(direction);
            shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
         } while (pickup != collector);
         direction = true;
         lclcol.push_back(shgen);
         result = true;
      }
      collector = collector->next();
   } while (collector != centinel);
   if (!result) return result;
   // Convert all collected shapes to a single normalized polygon
   pointlist* respoly = lclcol.front();lclcol.pop_front();
   while (0 < lclcol.size()) {
      respoly = hole2simple(*respoly, *(lclcol.front()));
      lclcol.pop_front();
   }
   plycol.push_back(respoly);
   return result;
}

      
polycross::VPoint* logicop::logic::getFirstOutside(const pointlist& plist, polycross::VPoint* init) {
   unsigned plysize = plist.size();
   unsigned count = 0;
   while (init->inside(plist)) {
      init = init->next();
      if (count++ > plysize) break;
   }
   if (count > plysize) return NULL;
   else return init;
}   


pointlist* logicop::logic::hole2simple(const pointlist& outside, const pointlist& inside) {
   polycross::segmentlist _seg1(outside,1);
   polycross::segmentlist _seg2(inside  ,2);
   polycross::XQ* _eq = new polycross::XQ(_seg1, _seg2); // create the event queue
   polycross::BindCollection BC;
   
   _eq->sweep2bind(BC);
   polycross::BindSegment* sbc = BC.get_highest();
   //insert 2 crossing points and link them
   polycross::BPoint* cpsegA = _seg1.insertbindpoint(sbc->poly0seg(), sbc->poly0pnt());
   polycross::BPoint* cpsegB = _seg2.insertbindpoint(sbc->poly1seg(), sbc->poly1pnt());
   cpsegA->linkto(cpsegB);
   cpsegB->linkto(cpsegA);

   // normalize the segment lists
   _seg1.normalize(outside);
   _seg2.normalize(inside);
   // dump the new polygons in VList terms
   polycross::VPoint* outshape = _seg1.dump_points();
   _seg2.dump_points();
   
   // traverse and form the resulting shape
   polycross::VPoint* centinel = outshape;
   pointlist *shgen = new pointlist();
   bool direction = true; /*next*/
   polycross::VPoint* pickup = centinel;
   polycross::VPoint* prev = centinel->prev();
   bool modify = false;
   do {
      shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
      modify = (-1 == prev->visited());
      prev = pickup;
      pickup = pickup->follower(direction, modify);
   } while (pickup != centinel);

   // Validate the resulting polygon
   laydata::valid_poly check(*shgen);
//   delete shgen;
   if (!check.valid()) {
      std::ostringstream ost;
      ost << ": Resulting shape is invalid - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str().c_str());
   }
   else {
      if (laydata::shp_OK != check.status())
         *shgen = check.get_validated();
   }
   
   return shgen;
}

// pointlist* logicop::logic::hole2simple(const pointlist& outside, const pointlist& inside) {
//    segmentlist _segl0(outside,0);
//    segmentlist _segl1(inside,1);
//    EventQueue* _eq = new EventQueue(_segl0, _segl1); // create the event queue
//    SweepLine   _sl;
//    BindCollection BC;
//    _eq->swipe4bind(_sl, BC);
//    BindSegment* sbc = BC.get_highest();
//    //insert 2 crossing points and link them
//    BPoint* cpsegA = _segl0.insertbindpoint(sbc->poly0seg(), sbc->poly0pnt());
//    BPoint* cpsegB = _segl1.insertbindpoint(sbc->poly1seg(), sbc->poly1pnt());
//    cpsegA->linkto(cpsegB);
//    cpsegB->linkto(cpsegA);
// 
//    // normalize the segment lists
//    _segl0.normalize(outside);
//    _segl1.normalize(inside);
//    // dump the new polygons in VList terms
//    VPoint* outshape = _segl0.dump_points();
//                       _segl1.dump_points();
//    
//    // traverse and form the resulting shape
//    VPoint* centinel = outshape;
//    pointlist *shgen = new pointlist();
//    bool direction = true; /*next*/
//    VPoint* pickup = centinel;
//    VPoint* prev = centinel->prev();
//    bool modify = false;
//    do {
//       shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
//       modify = (-1 == prev->visited());
//       prev = pickup;
//       pickup = pickup->follower(direction, modify);
//    } while (pickup != centinel);
// 
//    // Validate the resulting polygon
//    laydata::valid_poly check(*shgen);
// //   delete shgen;
//    if (!check.valid()) {
//       std::ostringstream ost;
//       ost << ": Resulting shape is invalid - " << check.failtype();
//       tell_log(console::MT_ERROR, ost.str().c_str());
//    }   
//    else {
//       if (laydata::shp_OK != check.status())
//          *shgen = check.get_validated();
//    }         
//    
//    return shgen;
// }

