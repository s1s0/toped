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
//    Description: Toped shape operations
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <math.h>
#include <algorithm>
#include "tedop.h"

//-----------------------------------------------------------------------------
// The declaratoin of the avl related functions. They are declared originally
// in avl.h and redeclared here in C++ manner with extern "C" clause
extern "C" {
   avl_table *avl_create (avl_comparison_func *, void *, libavl_allocator *);
   void       avl_destroy (struct avl_table *, avl_item_func *);
   void      *avl_t_next (avl_traverser *);
   void      *avl_t_prev (avl_traverser *);
   void      *avl_delete (avl_table *, const void *);
   void      *avl_t_find (avl_traverser *, avl_table *, void *);
   void      *avl_t_insert (avl_traverser *, avl_table *, void *);
}   

//==============================================================================
/*! Determines the lexicographical order of two points comparing X first. 
   Returns:
   +1 -> p1  > p2
   -1 -> p1  < p2
    0 -> p1 == p2
*/
int tedop::xyorder(const TP* p1, const TP* p2) {
   // test X coord first
   if (p1->x() > p2->x()) return  1;
   if (p1->x() < p2->x()) return -1;
   // and then y
   if (p1->y() > p2->y()) return  1;
   if (p1->y() < p2->y()) return -1;
   return 0;
}

/*! Determines the lexicographical order of two points comparing Y first. 
   Returns:
   +1 -> p1  > p2
   -1 -> p1  < p2
    0 -> p1 == p2
*/
int tedop::yxorder(const TP* p1, const TP* p2) {
   // test y coord first
   if (p1->y() > p2->y()) return  1;
   if (p1->y() < p2->y()) return -1;
   // and then x
   if (p1->x() > p2->x()) return  1;
   if (p1->x() < p2->x()) return -1;
   return 0;
}

inline float tedop::isLeft(const TP* p0, const TP* p1, const TP* p2) {
   return ((float)p1->x() - (float)p0->x()) * ((float)p2->y() - (float)p0->y()) - 
          ((float)p2->x() - (float)p0->x()) * ((float)p1->y() - (float)p0->y());
}

//-----------------------------------------------------------------------------
// class plysegment 
//-----------------------------------------------------------------------------
tedop::plysegment::plysegment(const TP* p1, const TP* p2, unsigned num) {
   if (xyorder(p1, p2) < 0) { lP = p1; rP = p2; }
   else                     { lP = p2; rP = p1; }
   above = below = NULL;
   edge = num;
}

//-----------------------------------------------------------------------------
// class segmentlist 
//-----------------------------------------------------------------------------
tedop::segmentlist::segmentlist(const pointlist& plst, bool wire) {
   byte adjustment = wire ? 1 : 0;
   unsigned plysize = plst.size();
   segs.reserve(plysize - adjustment);
   for (unsigned i = 0; i < plysize - adjustment; i++)
      segs.push_back(new plysegment(&(plst[i]),&(plst[(i+1)%plysize]),i));
}

tedop::segmentlist::~segmentlist() {
   for (unsigned i = 0; i < segs.size(); i++)
      delete segs[i];
   segs.clear();   
}

//-----------------------------------------------------------------------------
// class EventQueue
//-----------------------------------------------------------------------------
tedop::EventQueue::EventQueue( const segmentlist& segments ) {
   equeue.reserve(2*segments.size());
   // Initialize event queue with edge segment endpoints
   for (unsigned i=0; i < segments.size(); i++) {
      equeue.push_back(new LEvent(segments[i]));
      equeue.push_back(new REvent(segments[i]));
   }
   std::sort(equeue.begin(), equeue.end(), E_compare);
}

bool tedop::EventQueue::check_valid( SweepLine& SL ) {
   for (unsigned i = 0; i < equeue.size(); i++)
      if (!equeue[i]->check_valid(SL)) return false;
   return true;
}
      
bool tedop::EventQueue::E_compare( const Event* v1, const Event* v2 ) {
   // sort can send the same component for comparison, so ...
   if (v1 == v2) return true;
   // ... then compare the points
   int result =  tedop::xyorder(v1->evertex(),v2->evertex());
   if (result != 0) return (result > 0) ? false : true;
   // if they are the same, sort them by edge number
   if (v1->edgeNo() != v2->edgeNo())
      return (v1->edgeNo() > v2->edgeNo()) ? false : true;
   assert(false);   
}
 
tedop::EventQueue::~EventQueue() {
   for (unsigned i = 0; i < equeue.size(); i++)
      delete equeue[i];
   equeue.clear();
}

//-----------------------------------------------------------------------------
// class LEvent
//-----------------------------------------------------------------------------
bool tedop::LEvent::check_valid(SweepLine& SL) const {
   SL.add(_seg);
   if ((SL.intersect(_seg, _seg->above)) || (SL.intersect(_seg, _seg->below)))
      return false;
   return true;
}

//-----------------------------------------------------------------------------
// class REvent
//-----------------------------------------------------------------------------
bool tedop::REvent::check_valid(SweepLine& SL) const {
   if (SL.intersect(_seg->above, _seg->below))
      return false;
   SL.remove(_seg);
   return true;
}

//-----------------------------------------------------------------------------
// class SweepLine
//-----------------------------------------------------------------------------
tedop::SweepLine::SweepLine(const pointlist plist) : _plist(plist) {
   _numv = _plist.size();
   _tree = avl_create(compare_seg, NULL, NULL);
}    

tedop::SweepLine::~SweepLine() {
    avl_destroy(_tree, NULL);
}    

void tedop::SweepLine::add(plysegment* seg) {
   // add a node to the AVL tree
   avl_traverser trav;
   void* retitem =  avl_t_insert(&trav,_tree,seg);
   assert(retitem == seg);
   avl_traverser save_trav = trav;
   plysegment* nx = (plysegment*)avl_t_next(&save_trav);
   plysegment* np = (plysegment*)avl_t_prev(&trav);
   if (NULL != nx) {
      seg->above = nx; nx->below = seg;
   }
   if (NULL != np) {
      seg->below = np; np->above = seg;
   }
}

void tedop::SweepLine::remove(plysegment* seg) {
   avl_traverser trav;
   void* retitem = avl_t_find(&trav,_tree,seg);
   if (NULL == retitem) return;
   avl_traverser save_trav = trav;
   plysegment* nx = (plysegment*)avl_t_next(&save_trav);
   if (NULL != nx) nx->below = seg->below;
   plysegment* np = (plysegment*)avl_t_prev(&trav);
   if (NULL != np) np->above = seg->above;
   avl_delete(_tree,seg);
}

int  tedop::SweepLine::intersect(plysegment* seg1, plysegment* seg2) {
   // check both segments exists
   if ((NULL == seg1) || (NULL == seg2)) return false;
   // check for consecuitive polygon edges
   unsigned edge1 = seg1->edge;
   unsigned edge2 = seg2->edge;
   if ((((edge1+1) % _numv) == edge2) || (((edge2+1) % _numv) == edge1))
      return false;
   // Now test for intersection point exsistence
   float lsign, rsign;
   lsign = isLeft(seg1->lP, seg1->rP, seg2->lP);
   rsign = isLeft(seg1->lP, seg1->rP, seg2->rP);
   //check that both s2 endpoints are on the same side of s1
   if (lsign*rsign > 0)  return false;
   // Check the exception with coinciding segments
   if ((0 == lsign*rsign) && coincideOK(seg1, seg2, lsign, rsign))
      return false;
   lsign = isLeft(seg2->lP, seg2->rP, seg1->lP);
   rsign = isLeft(seg2->lP, seg2->rP, seg1->rP);
   //check that both s1 endpoints are on the same side of s2
   if (lsign*rsign > 0)  return false;
   // Check the exception with coinciding segments
   if ((0 == lsign*rsign) && coincideOK(seg2, seg1, lsign, rsign))
      return false;
   // at this point - the only possibility is that they intersect
   return true;
}

bool tedop::SweepLine::coincideOK( plysegment* line, plysegment* cross, float lps, float rps) {
   // first we going to check that the points laying on the line are not 
   // outside the segment
   // i.e. if neither of the entry points are lying on the segment line
   // then there is nothing to check further here, get out
   // In fact the result returned here doesn't really matter, because this case should
   // be in place only after the first pair of isLeft() checks in intersect()
   // if false is returned, the second pair of isLeft() should reject this case as
   // non-crossing.
   float lambdaL = (0 == lps) ? getLambda(line->lP, line->rP, cross->lP) : 0;
   float lambdaR = (0 == rps) ? getLambda(line->lP, line->rP, cross->rP) : 0;
   // filter-out all cases when the segments have no common points
   if (((0 != lps) || (lambdaL < 0)) && ((0 != rps) || (lambdaR < 0))) return true;
   // filter-out the cases when both lines have exactly one common point
   // and it is an edge point of both segmets
   bool Ljoint = (0==lps) && (0 == lambdaL);
   bool Rjoint = (0==rps) && (0 == lambdaR);
   if ((!Ljoint && Rjoint) || (!Rjoint && Ljoint)) return true;
   // now start checking the coincidence case
   //get the _plist index of the points in segment cross
   unsigned indxLP = (*(cross->lP) == _plist[cross->edge]) ? cross->edge : cross->edge + 1;
   unsigned indxRP = (*(cross->rP) == _plist[cross->edge]) ? cross->edge : cross->edge + 1;
   // make sure they are not the same
   assert(indxLP != indxRP);
   // we'll pickup the neighbour of the point(s) laying on the line and will
   // recalculate the lps/rps for them. 
   bool indxpos = (indxLP == ((indxRP + 1) % _numv));
   
   do {
      // the code below just increments/decrements the indexes in the point sequence
      // they look so weird, to keep the indexes within [0:_numv-1] boundaries
      if (0 == lps)
         if (indxpos) indxLP = (indxLP+1) % _numv;
         else (0==indxLP) ? indxLP = _numv-1 : indxLP--;
      if (0 == rps)
         if (!indxpos) indxRP = (indxRP+1) % _numv;
         else (0==indxRP) ? indxRP = _numv-1 : indxRP--;
      // calculate lps/rps with the new points   
      lps = isLeft(line->lP, line->rP, &_plist[indxLP]);
      rps = isLeft(line->lP, line->rP, &_plist[indxRP]);
   } while (0 == (lps * rps));
   return (lps*rps > 0) ? true : false;
}

float tedop::SweepLine::getLambda( const TP* p1, const TP* p2, const TP* p) {
   float denomX = p2->x() - p->x();
   float denomY = p2->y() - p->y();
   float lambda;
   if      (0 != denomX) lambda = (p->x() - p1->x()) / denomX;
   else if (0 != denomY) lambda = (p->y() - p1->y()) / denomY;
   // point coincides with the lp vertex of the segment
   else lambda = 0;
   return lambda;
}

int tedop::SweepLine::compare_seg(const void* o1, const void* o2, void*) {
   const plysegment* seg0 = (plysegment*)o1;
   const plysegment* seg1 = (plysegment*)o2;
   // first check that we are comparing one and the same segments,
   // and return equal immediately
   if (seg0->edge == seg1->edge) return 0;
   
   int order;
   // compare left points first
   order = tedop::yxorder(seg0->lP, seg1->lP);
   // if they coincide - compare right points
   if (0 == order) order = yxorder(seg0->rP, seg1->rP);
   if (0 == order) {
      // at this point it appears that the two segments are identical,
      // so sort them by polygon or segment number 
//      assert((*(seg0->lP) == *(seg1->lP)) && (*(seg0->rP) == *(seg1->rP)));
//      assert(seg0->edge != seg1->edge);
      order = (seg0->edge > seg1->edge) ? 1 : -1;
   }
   return order;
}
