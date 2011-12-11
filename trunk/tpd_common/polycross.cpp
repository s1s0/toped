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
//        Created: Tue Mar 21 2006
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Modified Bentley-Ottman algorithm
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
#include "polycross.h"
#include "outbox.h"
#include "avl.h"

//#define BO2_DEBUG
#define BO_printseg(SEGM) printf("thread %i : polygon %i, segment %i, \
lP (%i,%i), rP (%i,%i)  \n" , SEGM->threadID(), SEGM->polyNo() , SEGM->edge(), \
SEGM->lP()->x(), SEGM->lP()->y(), SEGM->rP()->x(),SEGM->rP()->y());

//==============================================================================
/**
 * Determines the lexicographical order of two points comparing X first.
 * @param p1 - first point for comparison
 * @param p2 - second point for comparison
 * @return +1 -> p1  > p2
           -1 -> p1  < p2
            0 -> p1 == p2
 */
int polycross::xyorder(const TP* p1, const TP* p2) {
   // first check that the pointers are the same
   if (p1 == p2) return 0;
   // test X coordinate first
   if (p1->x() > p2->x()) return  1;
   if (p1->x() < p2->x()) return -1;
   // and then y
   if (p1->y() > p2->y()) return  1;
   if (p1->y() < p2->y()) return -1;
   return 0;
}

int polycross::orientation(const TP* p1, const TP* p2, const TP* p3)
{
   // twice the "oriented" area of the enclosed triangle
   real area = (real(p1->x()) - real(p3->x())) * (real(p2->y()) - real(p3->y())) -
         (real(p2->x()) - real(p3->x())) * (real(p1->y()) - real(p3->y()));
   if (0 == area) return 0;
   else
      return (area > 0) ? 1 : -1;
}

/**
 * Check whether point p lies inside the segment defined by p1 and p2. The
function assumes that p lies on the line defined by p1 and p2
 * @param p1 first point defining the input segment
 * @param p2 second point defining the input segment
 * @param p the point to be checked
 * @return < 0  the point is outside the segment
           = 0  the point coincides with one of the segment endpoints
           > 0  the point is inside the segment

 */
float polycross::getLambda( const TP* p1, const TP* p2, const TP* p)
{
   float denomX = p2->x() - p->x();
   float denomY = p2->y() - p->y();
   float lambda;
   if      (0 != denomX) lambda = real(p->x() - p1->x()) / denomX;
   else if (0 != denomY) lambda = real(p->y() - p1->y()) / denomY;
   // point coincides with the lp vertex of the segment
   else lambda = 0;
   return lambda;
}

/*!
 * Checks whether two joining segments coincide
 * @param p common point
 * @param pa the end point of the first segment
 * @param pb the end point of the second segment
 * @return 1 if p, pb, pa are in one line this order
 *         0 if the input points are not in one line or p is a central point
 *        -1 if p, pa, pb are lined-up
 */
char polycross::coincidingSegm(const TP* p, const TP* pa, const TP* pb)
{
   if (0 != orientation(p, pa, pb)) return  0;
   if (getLambda(p, pa, pb) >= 0)   return  1;
   if (getLambda(p, pb, pa) >= 0)   return -1;
   else return 0;
}

/**
 * Checks whether the point lies inside a polygon represented by plist. It
implements the odd/even crossings algo, but modified to deal with exceptions.
Here is the idea. every segment of the input polygon is checked for intersection
with a ray starting from this point, parallel to the X axis and with the same
direction. Three types of intersections are registered:
- if ray crosses a segment -> score is incremented by two
- if ray touches a segment -> score is incremented by one
- if ray coincides with the segment - check whether the line, generated by the
neighboring points crosses the ray. If it doesn't increment by 2.

If the point lies on any of the polygon segments, we abort the check, and depending
on @touching input parameter - report the result immediately.

Finally, divide the score by two and if the result is odd - the point is inside.
 * @param plist input polygon
 * @param touching when the point lies on a polygon edge, then depending on this
parameter, procedure reports the point inside (if true) or outside the polygon
 * @return true if the point is inside the polygon
 */
bool polycross::pointInside(const TP* cp, const PointVector& plist, bool touching)
{
   TP p0, p1;
   byte cc = 0;
   unsigned size = plist.size();
   for (unsigned i = 0; i < size ; i++)
   {
      p0 = plist[i]; p1 = plist[(i+1) % size];
      if (((p0.y() <= cp->y()) && (p1.y() >=  cp->y()))
            ||((p0.y() >=  cp->y()) && (p1.y() <= cp->y())) )
      {
         int ori = orientation(&p0, &p1, cp);
         if ((0==ori) && (getLambda(&p0, &p1, cp) >= 0))
         {
            //cp lies on the edge p0-p1
            if (touching) return true;
            else return false;
         }
         else
         {
            if ((p1.y() == p0.y()) && (cp->x() < p1.x()))
            {
               // segment parallel to the ray and right of cp
               // check neighboring points
               unsigned indx0 = (0==i) ? size-1 : i-1;
               unsigned indx1 = (i+2) % size;
               p0 = plist[indx0]; p1 = plist[indx1];
               if (!(((p0.y() <= cp->y()) && (p1.y() >=  cp->y()))
                     ||((p0.y() >=  cp->y()) && (p1.y() <= cp->y())) ))
                  cc+=2;
            }
            else if (p1.y() != p0.y())
            {
               float tngns = (float) (cp->y() - p0.y())/ (float)(p1.y() - p0.y());
               float calcx = (float) p0.x() + tngns * (float)(p1.x() - p0.x());
               if ((float)cp->x() <= calcx)
               {
                  // if ray touches the segment
                  if ((cp->y() == p0.y()) || (cp->y() == p1.y())) cc++;
                  // ray crosses the segment
                  else cc+=2;
               }
            }
         }
      }
   }
   assert(0 == (cc % 2));
   cc /= 2;
   return (cc & 0x01) ? true : false;
}

//==============================================================================
// VPoint
polycross::VPoint::VPoint(const TP* point, VPoint* prev) : _cp(point), _prev(prev)
{
   if (_prev) _prev->_next = this;
}

bool polycross::VPoint::inside(const PointVector& plist, bool touching)
{
   return pointInside(_cp, plist, touching);
//   TP p0, p1;
//   byte cc = 0;
//   unsigned size = plist.size();
//   for (unsigned i = 0; i < size ; i++)
//   {
//      p0 = plist[i]; p1 = plist[(i+1) % size];
//      if (((p0.y() <= _cp->y()) && (p1.y() >=  _cp->y()))
//            ||((p0.y() >=  _cp->y()) && (p1.y() <= _cp->y())) )
//      {
//         int ori = orientation(&p0, &p1, _cp);
//         if ((0==ori) && (getLambda(&p0, &p1, _cp) >= 0))
//         {
//            //_cp lies on the edge p0-p1
//            if (touching) return true;
//            else return false;
//         }
//         else
//         {
//            if ((p1.y() == p0.y()) && (_cp->x() < p1.x()))
//            {
//               // segment parallel to the ray and right of _cp
//               // check neighboring points
//               unsigned indx0 = (0==i) ? size-1 : i-1;
//               unsigned indx1 = (i+2) % size;
//               p0 = plist[indx0]; p1 = plist[indx1];
//               if (!(((p0.y() <= _cp->y()) && (p1.y() >=  _cp->y()))
//                     ||((p0.y() >=  _cp->y()) && (p1.y() <= _cp->y())) ))
//                  cc+=2;
//            }
//            else if (p1.y() != p0.y())
//            {
//               float tngns = (float) (_cp->y() - p0.y())/ (float)(p1.y() - p0.y());
//               float calcx = (float) p0.x() + tngns * (float)(p1.x() - p0.x());
//               if ((float)_cp->x() <= calcx)
//               {
//                  // if ray touches the segment
//                  if ((_cp->y() == p0.y()) || (_cp->y() == p1.y())) cc++;
//                  // ray crosses the segment
//                  else cc+=2;
//               }
//            }
//         }
//      }
//   }
//   assert(0 == (cc % 2));
//   cc /= 2;
//   return (cc & 0x01) ? true : false;
}

polycross::VPoint* polycross::VPoint::follower(bool& direction, bool) {
   return direction ? _next : _prev;
}


//==============================================================================
// class CPoint

void polycross::CPoint::linkage(VPoint*& prev) {
   _prev = prev;
   if (prev) prev->set_next(this);
   prev = this;
}

polycross::VPoint* polycross::CPoint::follower(bool& direction, bool modify) {
   if (modify) direction = !direction;
   polycross::VPoint* flw = direction ? _link->_next : _link->_prev;
   _visited++; _link->_visited++;
   return flw;
}

/*! One of the Bentley-Ottman post process functions. This one is dealing
with the cases when a vertex is lying on a segment it doesn't belongs to,
i.e. two neighboring segments are touching the third segment. The algorithm
is generating a crossing point exactly at the vertex of each segment. This
results in a sequence of CPoint-VPoint-CPoint which refer to the same place.
First of all we need to make sure that the corresponding cross couples are
sorted properly, because they will coincide and there is no way for the
sorting function to find out which is the proper order. Then we have to deal
with two general cases. Both segments are approaching from the
same side - the K case. Then the crossing points should be removed. The
second case is when the segments are piercing the third segment Then one
of the crossing points and the vertex point are redundant and should be
removed.
*/
polycross::VPoint* polycross::VPoint::checkNreorder(VPoint*& pairedShape, bool single)
{
   CPoint* nextCross = static_cast<CPoint*>(_next);
   CPoint* prevCross = static_cast<CPoint*>(_prev);
   assert(*(prevCross->cp()) == *(nextCross->cp()));
   CPoint* nextCrossCouple = nextCross->link();
   CPoint* prevCrossCouple = prevCross->link();
   // Check that crossing points are not cross coupled i.e. that nextCrossCouple
   // is after the prevCrossCouple. If they are - swap their links to fix them
   // Do not forget that there might be other cross points between them as well as
   // a vertex points
   VPoint* snV = prevCrossCouple->next();
   while ( (snV != nextCrossCouple) && (*snV->cp() == *cp()) )
      snV = snV->next();
   VPoint *spV = nextCrossCouple->prev();
   while ( (spV != prevCrossCouple) && (*spV->cp() == *cp()) )
      spV = spV->prev();
   if (snV == nextCrossCouple)
   {
      assert(spV == prevCrossCouple);
   }
   else
   {
      // swap the links
      prevCross->linkto(nextCrossCouple); nextCrossCouple->linkto(prevCross);
      nextCross->linkto(prevCrossCouple); prevCrossCouple->linkto(nextCross);
      // ... and don't forget to update the points themselves
      nextCrossCouple = nextCross->link();
      prevCrossCouple = prevCross->link();
   }
   // now check for piercing edge cross points
   // First find the neighboring points which does not coincide with this vertex
   spV = prevCrossCouple->prev();
   while (*spV->cp() == *cp()) spV = spV->prev();
   snV = nextCrossCouple->next();
   while ((*snV->cp() == *cp()) || (*snV->cp() == *spV->cp())) snV = snV->next();

   VPoint* pV = prevCross;
   int oriP, oriN;
   do pV = pV->prev();
   while  (0 == (oriP = orientation(spV->cp(), snV->cp(), pV->cp())));

   VPoint *nV = nextCross;
   do nV = nV->next();
   while  (0 == (oriN = orientation(spV->cp(), snV->cp(), nV->cp())));
   if (oriP != oriN)
   {
      // we have a piercing edge cross - so let's remove the redundant points
      prevCross->prev()->set_next(nextCross);
      nextCross->set_prev(prevCross->prev());
      if ((prevCrossCouple->next() == nextCrossCouple->prev()) && (1 == prevCrossCouple->next()->visited()))
      {
         VPoint* deadVertex = prevCrossCouple->next();
         // there is a single non-crossing point between the cross coupled points
         // which must be removed
         if (pairedShape == deadVertex)
            pairedShape = nextCrossCouple;
         prevCrossCouple->set_next(nextCrossCouple);
         nextCrossCouple->set_prev(prevCrossCouple);
         delete (deadVertex);
      }
      prevCrossCouple->prev()->set_next(prevCrossCouple->next());
      prevCrossCouple->next()->set_prev(prevCrossCouple->prev());
      if (prevCrossCouple == pairedShape)
         pairedShape = prevCrossCouple->prev();
      // delete here the removed crossing points
      delete prevCrossCouple;
      delete prevCross;
      delete this;
      return nextCross;
   }
   else
   {
      if (single)
      {
         //we have a touching edges (K case) and we're post-processing
         //a single polygon - so, let's remove both crossing points
         prevCross->prev()->set_next(this); set_prev(prevCross->prev());
         nextCross->next()->set_prev(this); set_next(nextCross->next());
         delete prevCross;
         delete nextCross;
         // Cross coupled points are non necessarily neighbors in the second
         // polygon. We can have a plenty of them in one vertex - so take them
         // out one by one. First prevCrossCouple...
         prevCrossCouple->prev()->set_next(prevCrossCouple->next());
         prevCrossCouple->next()->set_prev(prevCrossCouple->prev());
         delete prevCrossCouple;
         // ...then nextCrossCouple
         nextCrossCouple->prev()->set_next(nextCrossCouple->next());
         nextCrossCouple->next()->set_prev(nextCrossCouple->prev());
         delete nextCrossCouple;
         return _next;
      }
      else
      {
         //we have a touching edges (K case) and we're post-processing
         //a two polygon case - so, let's remove both crossing points
         // keeping the second polygon intact
         prevCross->prev()->set_next(this); set_prev(prevCross->prev());
         nextCross->next()->set_prev(this); set_next(nextCross->next());
         delete prevCross;
         delete nextCross;
         // Cross coupled points are non necessarily neighbors in the second
         // polygon. We can have a plenty of them in one vertex - so take them
         // out one by one. First prevCrossCouple...
         prevCrossCouple->prev()->set_next(prevCrossCouple->next());
         prevCrossCouple->next()->set_prev(prevCrossCouple->prev());
         if (prevCrossCouple == pairedShape)
            pairedShape = prevCrossCouple->prev();
         delete prevCrossCouple;
         // ...then nextCrossCouple
         nextCrossCouple->prev()->set_next(nextCrossCouple->next());
         nextCrossCouple->next()->set_prev(nextCrossCouple->prev());
         if (nextCrossCouple == pairedShape)
            pairedShape = nextCrossCouple->next();
         delete nextCrossCouple;
         return _next;
      }
   }
}

polycross::VPoint* polycross::VPoint::checkRedundantCross()
{
   // Basically we are expecting the case with coinciding or partially
   // coinciding segments here. This case should result in a sequence CV-C
   // or C-VC i.e. CrossPoint/VertexPoint with the same coordinates
   // followed/preceded by another CorssPoint with different coordinates.
   // The idea is that the first crossing point is redundant and must be
   // removed
   polycross::CPoint* target;
   polycross::VPoint* retval;

   if (*prev()->cp() == *cp())
   {
      // remove previous point
      assert( 0 == prev()->visited() );
      target = static_cast<CPoint*>(prev());
      retval = next();
   }
   else
   {
      // remove next point
      assert(*next()->cp() == *cp());
      assert(0 == next()->visited());
      target = static_cast<CPoint*>(next());
      retval = next()->next();
   }
   target->prev()->set_next(target->next());
   target->next()->set_prev(target->prev());
   target->link()->prev()->set_next(target->link()->next());
   target->link()->next()->set_prev(target->link()->prev());
   delete target->link();
   delete target;
   return retval;
}

//==============================================================================
// class BPoint
polycross::VPoint* polycross::BPoint::follower(bool& direction, bool modify) {
   if (modify) {
      direction = !direction;
      return direction ? _next : _prev;
   }
   else return _link;
}

//==============================================================================
// SortLine

bool polycross::SortLine::operator() (CPoint* cp1, CPoint* cp2) {
   assert(direction != 0);
   int ord = xyorder(cp1->cp(), cp2->cp());
   if (direction > 0)
      return (ord > 0);
   else
      return (ord < 0);
}

//==============================================================================
// polysegment

polycross::polysegment::polysegment(const TP* p1, const TP* p2, int num, char plyn) {
   if (xyorder(p1, p2) < 0) { _lP = p1; _rP = p2; }
   else                     { _lP = p2; _rP = p1; }
   _edge = num; _polyNo = plyn;_threadID = 0;
}

/**
 * The method creates a new #CPoint object from the input pnt and adds it to the
_crossPoints array. The input point is assumed to be unique. Method is called by
Event::insertCrossPoint() only.
 * @param pnt the new cross point - assumed to be unique
 * @return the #CPoint that will be linked to its counterpart by the caller
 */
polycross::CPoint* polycross::polysegment::insertCrossPoint(const TP* pnt) {
   CPoint* cp = DEBUG_NEW CPoint(pnt, _edge);
   _crossPoints.push_back(cp);
   return cp;
}

unsigned polycross::polysegment::normalize(const TP* p1, const TP* p2)
{
   _lP = p1; _rP = p2;
   unsigned numcross = _crossPoints.size();
   if (_crossPoints.size() > 1)
   {
      SortLine functor(p1,p2);
      _crossPoints.sort(functor);
   }
   return numcross;
}

/*! Dump the valid segment points and link them. A valid segment points are the
segment left points and all crossing points except the coinciding ones. Two
segments cross each other in a single point. This is axiomatic of course, but
here it includes the coinciding or partially coinciding segments. It is rather
a sanity check here for double crossing points, but BO algo implementation is
producing such cases exactly when segments partially coincides.
*/
void polycross::polysegment::dump_points(polycross::VPoint*& vlist)
{
   // for all left points - create a new VPoint
   vlist = DEBUG_NEW VPoint(_lP, vlist);
   crossCList::iterator CCPA = _crossPoints.begin();
   while(CCPA != _crossPoints.end())
   {
      // Check whether a cross point which links to the same segment has not
      // been already dumped
      crossCList::iterator CCPB = _crossPoints.begin();
      while ((CCPB != _crossPoints.end()) && (CCPB != CCPA))
      {
         if ((*CCPA)->link()->edge() == (*CCPB)->link()->edge())
            break;
         else
            CCPB++;
      }
      if (CCPA == CCPB)
      {
         // for unique CPoints - use existing objects, don't create a new VPoint,
         // just link it.
         (*CCPA)->linkage(vlist);
#ifdef BO2_DEBUG
         printf("( %i , %i )\n", (*CCPA)->cp()->x(), (*CCPA)->cp()->y());
#endif
         CCPA++;
      }
      else
      {
#ifdef BO2_DEBUG
         printf("(<><><><><> Double cross points on segmets %i and %i)\n", _edge, (*CCPA)->link()->edge() );
#endif
         // double point found - not going to be used and shall be removed
         // to avoid leakages
		 //FIXME -> this is a leakage, but it must be cleaned-up alltogether with its
		 // counterpart. If you delete it here alnone, the tool will crash randomly
		 // further down thw line 
//         delete (*CCPA);  
         CCPA = _crossPoints.erase(CCPA);
      }
   }
}

bool polycross::polysegment::operator == (const polysegment& comp) const
{
   return ((_polyNo == comp._polyNo) &&
           (_edge   == comp._edge  )     );
}

polycross::BPoint* polycross::polysegment::insertBindPoint(const TP* pnt)
{
   BPoint* cp = DEBUG_NEW BPoint(pnt, _edge);
   _crossPoints.push_back(cp);
   return cp;
}


polycross::polysegment::~polysegment()
{
//   for (unsigned i = 0; i < _crossPoints.size(); i++)
//      delete (_crossPoints[i]);
}

//==============================================================================
// class segmentlist

polycross::segmentlist::segmentlist(const PointVector& plst, byte plyn, bool looped) {
   _originalPL = &plst;
   unsigned plysize = plst.size();
   if (!looped)
   {
      plysize--;
      _segs.reserve(plysize);
      for (unsigned i = 0; i < plysize; i++)
         _segs.push_back(DEBUG_NEW polysegment(&(plst[i]),&(plst[i+1]),i, plyn));
   }
   else
   {
      _segs.reserve(plysize);
      for (unsigned i = 0; i < plysize; i++)
         _segs.push_back(DEBUG_NEW polysegment(&(plst[i]),&(plst[(i+1)%plysize]),i, plyn));
   }
}

polycross::BPoint* polycross::segmentlist::insertBindPoint(unsigned segno, const TP* point) {
    return _segs[segno]->insertBindPoint(point);
}

polycross::segmentlist::~segmentlist() {
   for (unsigned i = 0; i < _segs.size(); i++)
      delete _segs[i];
   _segs.clear();
}

unsigned polycross::segmentlist::normalize(const PointVector& plst, bool looped) {
   unsigned numcross = 0;
   unsigned plysize = plst.size();
   if (looped)
      for (unsigned i = 0; i < plysize; i++)
         numcross += _segs[i]->normalize(&(plst[i]),&(plst[(i+1)%plysize]));
   else
      for (unsigned i = 0; i < plysize - 1; i++)
         numcross += _segs[i]->normalize(&(plst[i]),&(plst[i+1]));
   return numcross;
}

polycross::VPoint* polycross::segmentlist::dump_points(bool looped)
{
   VPoint* vlist = NULL;
#ifdef BO2_DEBUG
      printf("---------------Crossing points found-----------\n");
#endif
   for (unsigned i = 0; i < _segs.size(); i++)
      _segs[i]->dump_points(vlist);
   if (!looped)
      vlist = DEBUG_NEW VPoint(_segs[_segs.size()-1]->rP(), vlist);
   polycross::VPoint* lastV = vlist;
   while (vlist->prev())
      vlist = vlist->prev();
   lastV->set_next(vlist);
   vlist->set_prev(lastV);
   return vlist;
}

//==============================================================================
// TEvent

/**
 * The method is using orientation() function to do the job. Segments that belong to
the same polygon are not checked and method returns NULL. \n
In case segments intersect, the method calls insertCrossPoint() method.
 * @param above
 * @param below
 * @param eventQ Event queue that will take the new crossing event.
 * @param iff represent the conditional point - if the crossing point found has
the same coordinates as iff point, then a new event will be added to the event
queue. This is used for touching points, i.e. iff can be only a vertex of the
input segments
 * @return the crossing point between segments if it exists. Otherwise - NULL
 */
void polycross::TEvent::checkIntersect(polysegment* above, polysegment* below,
                                       XQ& eventQ, bool single, const TP* iff)
{
   TP* CrossPoint = NULL;
   // check that the cross candidates are not a sentinel segments
   // check for polygon edges belonging to the same polygon when two polygons are processed
   // check for neighboring edges when a single polygon is processed
   if ( (0 == below->polyNo()) ||
        (0 == above->polyNo()) ||
        (!single && (above->polyNo() == below->polyNo())) ||
        ( single && (1 == abs(above->edge() - below->edge())))
      ) return;
   // Now test for intersection point existence
   float lsign, rsign, rlmul;

   //check that both below endpoints are on the same side of above segment
   lsign = orientation(above->lP(), above->rP(), below->lP());
   rsign = orientation(above->lP(), above->rP(), below->rP());
   // do the case when both segments lie on the same line
   if ((lsign == 0) && (rsign == 0))
   {
      if ((iff == NULL) &&
           ((CrossPoint = oneLineSegments(above, below, eventQ.sweepline()))))
      {
         insertCrossPoint(CrossPoint, above, below, eventQ, true);
         delete CrossPoint;
      }
      return;
   }
   rlmul = lsign*rsign;
   if      (0  < rlmul)  return;// not crossing
   if (0 == rlmul)
   {
      // possibly touching segments
      CrossPoint = joiningSegments(above, below, lsign, rsign);
      if (NULL != CrossPoint)
      {
         if ((NULL == iff) || (*CrossPoint == *iff))
            insertCrossPoint(CrossPoint, above, below, eventQ);
         delete CrossPoint;
      }
      return;
   }

   //now check that both above endpoints are on the same side of below segment
   lsign = orientation(below->lP(), below->rP(), above->lP());
   rsign = orientation(below->lP(), below->rP(), above->rP());
   // lsign == rsign == 0 case here should not be possible
   if ((lsign == 0) && (rsign == 0))
      throw EXPTNpolyCross("Segments shouldn't coincide at this point");
   rlmul = lsign*rsign;
   if      (0  < rlmul) return;
   if (0 == rlmul)
   {
      // possibly touching segments
      CrossPoint = joiningSegments(below, above, lsign, rsign);
      if (NULL != CrossPoint)
      {
         if ((NULL == iff) || (*CrossPoint == *iff))
            insertCrossPoint(CrossPoint, above, below, eventQ);
         delete CrossPoint;
      }
      return;
   }

   // at this point - the only possibility is that they intersect
   // so - create a cross event
   if (NULL == iff)
   {
      CrossPoint = getCross(above, below);
      insertCrossPoint(CrossPoint, above, below, eventQ);
      delete CrossPoint;
   }
}

/**
 * Method should be called with one of lps/rps parameters equal to 0. Crossing
point is returned only if it is not an end point of both segments.

 * @param above
 * @param below
 * @param lps
 * @param rps
 * @return
 */
TP* polycross::TEvent::joiningSegments(polysegment* above, polysegment* below, float lps, float rps)
{
   if (0 == lps)
   {
      if (0 >= getLambda(above->lP(), above->rP(), below->lP())) return NULL;
      else return DEBUG_NEW TP(*(below->lP()));
   }
   else if(0 == rps)
   {
      if (0 >= getLambda(above->lP(), above->rP(), below->rP())) return NULL;
      else return DEBUG_NEW TP(*(below->rP()));
   }
   assert(false);
}

TP* polycross::TEvent::oneLineSegments(polysegment* above, polysegment* below, YQ* sweepL)
{
   float lps = 0.0;
   float rps = 0.0;
   float lambdaL = getLambda(above->lP(), above->rP(), below->lP());
   float lambdaR = getLambda(above->lP(), above->rP(), below->rP());
   // partially coinciding or joining segments -> don't generate anything.
   // The post-process of begin/end/modify events in a single vertex should
   // take proper care about those cases. Notable exception are fully 
   // coinciding segments which shall be handled here
   if ((lambdaL != lambdaR) && (0 == (lambdaL * lambdaR))) return NULL;
   bool swaped = false;
   // first of all cases when neither of the lines is enclosing the other one
   // The idea of this check is :
   // 1. Find-out the common part of the two input segments
   // 2. For each of the input segments, pick-up the point which is neighboring
   //    to the one lying inside the opposite segment and does not belong to this
   //    segment. Example:
   //    - the above segment is defined by the points A & B
   //    - the below segment is defined by the points C & D
   //    if A lies inside CD, find the point N from segment above which is a
   //    neighbor to A, but it is not B.
   // 3. Having found those two points in p.2 - check their location against the
   //    segment defined in p.1
   //    - if both points lie on the SAME side of the common segment - generate
   //      a "hidden" cross point, right in the middle of the common segment
   //    - if both points are on the opposite sides of the segment - do nothing.
   //
   // NOTE! this is kind of opposite to the case further down in this method
   //       where one segment lies entirely inside the other. There, we generate
   //       a "hidden" cross point when of the control points are on the OPPOSITE
   //       side of the test segment.
   if ((lambdaL < 0) && (lambdaR > 0))
   {
      // below->lP is outside above; below->rP is inside
      // the points below constitute the piece of segment common for both input segments
      // (above->lP(), below->rP()
      unsigned indxRP, indxLP, dummy;
      bool direction = true;
      int numv;
      //
      const PointVector* aboveP = sweepL->locateOriginals(above , dummy, indxLP, direction);
      numv = aboveP->size();
      if (!direction) indxLP = (indxLP+1) % numv;
      else (0==indxLP) ? indxLP = numv-1 : indxLP--;

      const PointVector* belowP = sweepL->locateOriginals(below , indxRP, dummy, direction);
      numv = belowP->size();
      if (direction) indxRP = (indxRP+1) % numv;
      else (0==indxRP) ? indxRP = numv-1 : indxRP--;

      lps = orientation(above->lP(), below->rP(), &((*aboveP)[indxLP]));
      rps = orientation(above->lP(), below->rP(), &((*belowP)[indxRP]));

      if ((lps * rps) > 0) return getMiddle(above->lP(), below->rP());
      else                 return NULL;
   }
   if ((lambdaL > 0) && (lambdaR < 0))
   {
      // below->lP is inside above; below->rP is outside
      // the points below constitute the piece of segment common for both input segments
      // (below->lP(), above->rP()
      unsigned indxRP, indxLP, dummy;
      bool direction = true;
      int numv;

      const PointVector* belowP = sweepL->locateOriginals(below , dummy, indxLP, direction);
      numv = belowP->size();
      if (!direction) indxLP = (indxLP+1) % numv;
      else (0==indxLP) ? indxLP = numv-1 : indxLP--;

      const PointVector* aboveP = sweepL->locateOriginals(above , indxRP, dummy, direction);
      numv = aboveP->size();
      if (direction) indxRP = (indxRP+1) % numv;
      else (0==indxRP) ? indxRP = numv-1 : indxRP--;

      lps = orientation(below->lP(), above->rP(), &((*aboveP)[indxLP]));
      rps = orientation(below->lP(), above->rP(), &((*belowP)[indxRP]));

      if ((lps * rps) > 0) return getMiddle(below->lP(), above->rP());
      else                 return NULL;

   }

   if ((lambdaL < 0) && (lambdaR < 0))
   {
      // below edge points are outside the above segment, this however
      // could mean that below fully encloses above
      lambdaL = getLambda(below->lP(), below->rP(), above->lP());
      lambdaR = getLambda(below->lP(), below->rP(), above->rP());
      if ((lambdaL < 0) && (lambdaR < 0)) return NULL;
      swaped = true;
   }
   // now the cases when one of the lines encloses fully the other one
   polysegment* outside = swaped ? below : above;
   polysegment* inside  = swaped ? above : below;
   unsigned indxRP, indxLP;
   bool direction = true;

   const PointVector* insideP = sweepL->locateOriginals(inside  , indxRP, indxLP, direction);
   int numv = insideP->size();

   do
   {
      // make sure indexes are not the same
      if (indxLP == indxRP)
         throw EXPTNpolyCross("Can't locate properly the inside segment");

      // the code below just increments/decrements the indexes in the point sequence
      // they look so weird, to keep the indexes within [0:_numv-1] boundaries
      if (0 == lps)
      {
         if (direction) indxLP = (indxLP+1) % numv;
         else (0==indxLP) ? indxLP = numv-1 : indxLP--;
      }
      if (0 == rps)
      {
         if (!direction) indxRP = (indxRP+1) % numv;
         else (0==indxRP) ? indxRP = numv-1 : indxRP--;
      }
      // calculate lps/rps with the new points
      lps = orientation(outside->lP(), outside->rP(), &((*insideP)[indxLP]));
      rps = orientation(outside->lP(), outside->rP(), &((*insideP)[indxRP]));
   } while (0 == (lps * rps));
   // When segments are crossing (because of the direction of their neighbors),
   // then a crossing point is introduced in the middle of the internal segment
   if   ((lps*rps) > 0) return NULL;
   else return getMiddle(inside->lP(), inside->rP());
}

/**
 * Here a crossing point #_cp is calculated from the input segments. The
function assumes that the input two segments definitely cross each other.
It is called only from TEvent::checkIntersect() in case the check for crossing
segments is positive.\n
Crossing point is calculated from general line equations Ax+By+C=0. A care
is taken for division by 0 case when A or B coefficients are 0. It
is important to note as well that the calculations must be done in floating
point expressions, otherwise the hell might brake loose.
 * @param above first input segment
 * @param below second input segment
 * @return the crossing point between the input segments
 */
TP* polycross::TEvent::getCross(polysegment* above, polysegment* below)
{
   real A1 = above->rP()->y() - above->lP()->y();
   real A2 = below->rP()->y() - below->lP()->y();
   real B1 = above->lP()->x() - above->rP()->x();
   real B2 = below->lP()->x() - below->rP()->x();
   real C1 = -(A1*above->lP()->x() + B1*above->lP()->y());
   real C2 = -(A2*below->lP()->x() + B2*below->lP()->y());
   assert((A1 != 0) || (A2 != 0));
   assert((B1 != 0) || (B2 != 0));
   // segments will coincide if    A1/A2 == B1/B2 == C1/C2
   // segments will be parallel if A1/A2 == B1/B2 != C1/C2
   real X,Y;
   if ((0 != A1) && (0 != B2))
   {
      X = - ((C1 - (B1/B2) * C2) / (A1 - (B1/B2) * A2));
      Y = - ((C2 - (A2/A1) * C1) / (B2 - (A2/A1) * B1));
   }
   else if ((0 != B1) && (0 != A2))
   {
      X = - (C2 - (B2/B1) * C1) / (A2 - (B2/B1) * A1);
      Y = - (C1 - (A1/A2) * C2) / (B1 - (A1/A2) * B2);
   }
   else
      throw EXPTNpolyCross("Input segments don't have a crossing point");
   return DEBUG_NEW TP((int)rint(X),(int)rint(Y));
}


TP* polycross::TEvent::getMiddle(const TP* p1, const TP* p2)
{
   int4b x = (p2->x() + p1->x()) / 2;
   int4b y = (p2->y() + p1->y()) / 2;
   return DEBUG_NEW TP(x,y);
}

/**
 * After a crossing point has been discovered, this method inserts it in each
of the input segments after what they are linked to each other as well. This
data will be used later, outside the Bentley-Ottmann algorithm to form the
new polygons. The method also creates a cross event (#TcEvent), but only if
dontswap is false.
* @param CP the crossing point
 * @param above first segment
 * @param below second segment
 * @param eventQ the event queue that will assept the possible cross event
 * @param dontswap if *true* a cross event is not created
 */
void polycross::TEvent::insertCrossPoint(const TP* CP, polysegment* above,
                                 polysegment* below, XQ& eventQ, bool dontswap)
{
   assert(NULL != CP);
   CPoint* cpsegA = above->insertCrossPoint(CP);
   CPoint* cpsegB = below->insertCrossPoint(CP);
   cpsegA->linkto(cpsegB);
   cpsegB->linkto(cpsegA);
#ifdef BO2_DEBUG
   if (dontswap)
      printf("**Cross point (%i,%i) NON-SWAPING inserted between segments below :\n", CP->x(), CP->y());
   else
      printf("**Cross point (%i,%i) inserted between segments below :\n", CP->x(), CP->y());
   BO_printseg(above)
   BO_printseg(below)
#endif
   if (!dontswap)
      eventQ.addCrossEvent(CP, above, below);
}

//==============================================================================
// TbEvent

polycross::TbEvent::TbEvent (polysegment* seg1, polysegment* seg2):TEvent()
{
   if (seg1->lP() != seg2->lP())
      throw EXPTNpolyCross("Invalid input segments in thread begin");
   int ori = orientation(seg2->lP(), seg2->rP(), seg1->rP());
   if (0 == ori)
      throw EXPTNpolyCross("Invalid input segments in thread begin");
   if (ori > 0)
   {
      _aseg = seg1; _bseg = seg2;
   }
   else
   {
      _aseg = seg2; _bseg = seg1;
   }
   _evertex = seg1->lP();
}

void polycross::TbEvent::sweep(XQ& eventQ, YQ& sweepline, ThreadList& threadl, bool single)
{
   // create the threads
   SegmentThread* athr = sweepline.beginThread(_aseg);
   SegmentThread* bthr = sweepline.beginThread(_bseg);
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread begin");
   threadl.insert(_aseg->threadID());
   threadl.insert(_bseg->threadID());
#ifdef BO2_DEBUG
   printf("Begin 2 threads :\n");
   BO_printseg(_aseg)
   BO_printseg(_bseg)
   sweepline.report();
#endif
   // in all cases check:
   checkIntersect(athr->threadAbove()->cseg(), _aseg, eventQ, single);
   checkIntersect(_bseg, bthr->threadBelow()->cseg(), eventQ, single);
   // check that the new threads are neighbors
   if (!((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr)))
   {
      // not neighbours - check the rest
      checkIntersect(bthr->threadAbove()->cseg(), _bseg, eventQ, single);
      checkIntersect(_aseg, athr->threadBelow()->cseg(), eventQ, single);
   }
   // check the case when left points are also crossing points
   // this is also the case when one of the input segments coincides with
   // an existing segment
   checkIntersect(athr->threadAbove()->cseg(), _bseg, eventQ, single, _bseg->lP());
   checkIntersect(_aseg, bthr->threadBelow()->cseg(), eventQ, single, _aseg->lP());
}


void polycross::TbEvent::sweep2bind(YQ& sweepline, BindCollection& bindColl)
{
   // create the threads
   SegmentThread* athr = sweepline.beginThread(_aseg);
   SegmentThread* bthr = sweepline.beginThread(_bseg);
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread begin - bind");
   // action is taken only for points in the second polygon
   if ((1 == _aseg->polyNo()) && (_aseg->polyNo() == _bseg->polyNo())) return;
   // assuming that the polygons are not crossing ...
   if (!((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr)))
      throw EXPTNpolyCross("Crossing input polygons in bind algo - begin");
   // if left point is above and the neighbor above is from polygon 1
   if ((_aseg->lP()->y() >= _aseg->rP()->y()) &&
        (1 == athr->threadAbove()->cseg()->polyNo()))
      bindColl.update_BL(athr->threadAbove()->cseg(), _aseg->edge(), _aseg->lP());
   // if the left point is below and the neighbor below is from polygon 1
   if ((_bseg->lP()->y() <= _bseg->rP()->y()) &&
        (1 == bthr->threadBelow()->cseg()->polyNo()))
      bindColl.update_BL(bthr->threadBelow()->cseg(), _bseg->edge(), _bseg->lP());
}

//==============================================================================
// TeEvent

polycross::TeEvent::TeEvent (polysegment* seg1, polysegment* seg2): TEvent()
{
   if (seg1->rP() != seg2->rP())
      throw EXPTNpolyCross("Invalid input segments in thread end");
   int ori = orientation(seg2->lP(), seg2->rP(), seg1->lP());
   if (0 == ori)
      throw EXPTNpolyCross("Invalid input segments in thread end");
   if (ori > 0)
   {
      _aseg = seg1; _bseg = seg2;
   }
   else
   {
      _aseg = seg2; _bseg = seg1;
   }
   _evertex = seg1->rP();

}

void polycross::TeEvent::sweep (XQ& eventQ, YQ& sweepline, ThreadList& threadl, bool single)
{
   threadl.insert(_aseg->threadID());
   threadl.insert(_bseg->threadID());
   SegmentThread* athr = sweepline.getThread(_aseg->threadID());
   SegmentThread* bthr = sweepline.getThread(_bseg->threadID());
#ifdef BO2_DEBUG
   printf("End 2 threads\n");
#endif
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread end");
   if ((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr))
   {
      // if the segments that are about to be removed are neighbors -
      // check only their external neighbors
      checkIntersect(athr->threadAbove()->cseg(), bthr->threadBelow()->cseg(), eventQ, single);
   }
   else
   {
      // not neighbors - check both neighbors
      checkIntersect(athr->threadAbove()->cseg(), athr->threadBelow()->cseg(), eventQ, single);
      checkIntersect(bthr->threadAbove()->cseg(), bthr->threadBelow()->cseg(), eventQ, single);
   }
   // in all cases - check the case when right points are also crossing points
   checkIntersect(athr->threadAbove()->cseg(), _bseg, eventQ, single, _bseg->rP());
   checkIntersect(_aseg, bthr->threadBelow()->cseg(), eventQ, single, _aseg->rP());
   // remove segment threads from the sweep line
   sweepline.endThread(_aseg->threadID());
   sweepline.endThread(_bseg->threadID());
#ifdef BO2_DEBUG
   BO_printseg(_aseg)
   BO_printseg(_bseg)
   sweepline.report();
#endif
}

void polycross::TeEvent::sweep2bind(YQ& sweepline, BindCollection& bindColl)
{
   SegmentThread* athr = sweepline.getThread(_aseg->threadID());
   SegmentThread* bthr = sweepline.getThread(_bseg->threadID());
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread end - bind");
   // action is taken only for points in the second polygon
   if ((2 == _aseg->polyNo()) && (_aseg->polyNo() == _bseg->polyNo()))
   {
      // assuming that the polygons are not crossing ...
      if (!((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr)))
         throw EXPTNpolyCross("Crossing input polygons in bind algo - end");
      // if right point is above and the neighbor above is from polygon 1
      if ((_aseg->lP()->y() <= _aseg->rP()->y()) &&
            (1 == athr->threadAbove()->cseg()->polyNo()))
         bindColl.update_BL(athr->threadAbove()->cseg(), _aseg->edge(), _aseg->rP());
      // if the right point is below and the neighbor below is from polygon 1
      if ((_bseg->lP()->y() >= _bseg->rP()->y()) &&
            (1 == bthr->threadBelow()->cseg()->polyNo()))
         bindColl.update_BL(bthr->threadBelow()->cseg(), _bseg->edge(), _bseg->rP());
   }
   // delete the threads
   sweepline.endThread(_aseg->threadID());
   sweepline.endThread(_bseg->threadID());
}

//==============================================================================
// TmEvent

polycross::TmEvent::TmEvent (polysegment* seg1, polysegment* seg2):TEvent( )
{
   if       (seg1->rP() == seg2->lP())
   {
      _aseg = seg1; _bseg = seg2;
      _evertex = seg1->rP();
   }
   else if (seg2->rP() == seg1->lP())
   {
      _aseg = seg2; _bseg = seg1;
      _evertex = seg2->rP();
   }
   else
      throw EXPTNpolyCross("Invalid input segments in thread modify");

}

void polycross::TmEvent::sweep (XQ& eventQ, YQ& sweepline, ThreadList& threadl, bool single)
{
#ifdef BO2_DEBUG
   printf("Modify thread\n");
   BO_printseg(_aseg)
   BO_printseg(_bseg)
#endif
   if (0 == _aseg->threadID())
      throw EXPTNpolyCross("Sorted segment expected here");
   bool smooth; // i.e. the thread sequence is the same
   SegmentThread* thr = sweepline.modifyThread(_aseg->threadID(), _bseg, smooth);
   if (smooth)
      threadl.insert(_bseg->threadID());

   // check for intersections of the neighbors with the new segment
   checkIntersect(thr->threadAbove()->cseg(), thr->cseg(), eventQ, single);
   checkIntersect(thr->cseg(), thr->threadBelow()->cseg(), eventQ, single);
}

void polycross::TmEvent::sweep2bind(YQ& sweepline, BindCollection& bindColl)
{
   if (0 == _aseg->threadID())
      throw EXPTNpolyCross("Sorted segment expected here - bind");
   // TODO - check whether we need the result of the smooth segments here
   bool todovar;
   SegmentThread* thr = sweepline.modifyThread(_aseg->threadID(), _bseg, todovar);

   // action is taken only for points in the second polygon
   if ( (1 == _aseg->polyNo()) && (_aseg->polyNo() == _bseg->polyNo())) return;

   // first for _aseg
   // if right point is above and the neighbor above is from polygon 1
   if ((_aseg->lP()->y() <= _aseg->rP()->y()) &&
        (1 == thr->threadAbove()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadAbove()->cseg(), _aseg->edge(), _aseg->rP());
   // if the right point is below and the neighbor below is from polygon 1
   if ((_aseg->lP()->y() >= _aseg->rP()->y()) &&
        (1 == thr->threadBelow()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadBelow()->cseg(), _aseg->edge(), _aseg->rP());

   // then for _bseg
   // if left point is above and the neighbor above is from polygon 1
   if ((_bseg->lP()->y() >= _bseg->rP()->y()) &&
        (1 == thr->threadAbove()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadAbove()->cseg(), _bseg->edge(), _bseg->lP());
   // if the left point is below and the neighbor below is from polygon 1
   if ((_bseg->lP()->y() <= _bseg->rP()->y()) &&
        (1 == thr->threadBelow()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadBelow()->cseg(), _bseg->edge(), _bseg->lP());
}

//==============================================================================
// TbsEvent

polycross::TbsEvent::TbsEvent (polysegment* seg1) : TEvent()
{
   _aseg = seg1; _bseg = NULL;
   _evertex = seg1->lP();
}

void polycross::TbsEvent::sweep(XQ& eventQ, YQ& sweepline, ThreadList& threadl, bool single)
{
   // create the threads
   SegmentThread* athr = sweepline.beginThread(_aseg);
   threadl.insert(_aseg->threadID());
#ifdef BO2_DEBUG
   printf("Begin 1 threads :\n");
   BO_printseg(_aseg)
   sweepline.report();
#endif
   checkIntersect(athr->threadAbove()->cseg(), _aseg, eventQ, single);
   checkIntersect(_aseg, athr->threadBelow()->cseg(), eventQ, single);
   // check the case when left points are also crossing points
   // this is also the case when one of the input segments coincides with
   // an existing segment
   checkIntersect(athr->threadAbove()->cseg(), _aseg, eventQ, single, _aseg->lP());
   checkIntersect(_aseg, athr->threadBelow()->cseg(), eventQ, single, _aseg->lP());
}

//==============================================================================
// TesEvent

polycross::TesEvent::TesEvent (polysegment* seg1) : TEvent()
{
   _aseg = seg1; _bseg = NULL;
   _evertex = seg1->rP();
}

void polycross::TesEvent::sweep (XQ& eventQ, YQ& sweepline, ThreadList& threadl, bool single)
{
   threadl.insert(_aseg->threadID());
   SegmentThread* athr = sweepline.getThread(_aseg->threadID());
   checkIntersect(athr->threadAbove()->cseg(), athr->threadBelow()->cseg(), eventQ, single);
   // in all cases - check the case when right points are also crossing points
   checkIntersect(athr->threadAbove()->cseg(), _aseg, eventQ, single, _aseg->rP());
   checkIntersect(_aseg, athr->threadBelow()->cseg(), eventQ, single, _aseg->rP());
   // remove segment threads from the sweep line
   sweepline.endThread(_aseg->threadID());
#ifdef BO2_DEBUG
   printf("End 1 threads\n");
   BO_printseg(_aseg)
   sweepline.report();
#endif
}

//==============================================================================
// TcEvent

void polycross::TcEvent::sweep(XQ& eventQ, YQ& sweepline, ThreadList& threadl, bool single)
{
   if ((threadl.end() != std::find(threadl.begin(), threadl.end(),_threadAbove)) ||
       (threadl.end() != std::find(threadl.begin(), threadl.end(),_threadBelow)))
   {
#ifdef BO2_DEBUG
      printf("Swiping a crossing point\n");
      printf("SKIP swapping threads %i and % i \n", _threadAbove, _threadBelow);
#endif
      return;
   }
#ifdef BO2_DEBUG
      printf("Swiping a crossing point\n");
      printf("SWAPPING threads % i and %i\n", _threadAbove, _threadBelow);
#endif
   SegmentThread* below = sweepline.swapThreads(_threadAbove, _threadBelow);
   SegmentThread* above = below->threadAbove();
#ifdef BO2_DEBUG
   sweepline.report();
#endif
   // check for intersections with the new neighbours
   checkIntersect(above->threadAbove()->cseg(), above->cseg(), eventQ, single);
   checkIntersect(below->cseg(), below->threadBelow()->cseg(), eventQ, single);
}

bool polycross::TcEvent::operator == (const TcEvent& event) const
{
   return ((_threadAbove == event._threadAbove) &&
         (_threadBelow == event._threadBelow)   );
}

//==============================================================================
// EventVertex

void polycross::EventVertex::addEvent(TEvent* tevent, EventTypes etype)
{
   assert(NULL != tevent);
   Events& simevents = _events[etype];
   if (_crossE == etype)
   {
      // Don't double the cross events -> do we really need this after all?
      for (Events::const_iterator CE=simevents.begin();
            CE != simevents.end(); CE++)
         if (*(static_cast<TcEvent*>(*CE)) == *(static_cast<TcEvent*>(tevent)))
         {
            delete tevent;
            return;
         }
   }
#ifdef BO2_DEBUG
      printf("++New event added in vertex ( %i , %i ) on top of the pending %u +++++\n",
             _evertex->x(), _evertex->y(), unsigned(simevents.size()));
#endif
   simevents.push_back(tevent);
}


void polycross::EventVertex::sweep(YQ& sweepline, XQ& eventq, bool single, bool looped)
{
#ifdef BO2_DEBUG
   printf("______________ POINT = ( %i , %i ) ___________\n", _evertex->x(), _evertex->y());
#endif
   Events nonCrossE;
   for( int cetype = _endE; cetype <= _crossE; cetype++)
   {
      if (_events.end() != _events.find(cetype))
      {
         Events& simEvents = _events[cetype];
         for( Events::iterator CE = simEvents.begin(); CE != simEvents.end() ; CE++){
            (*CE)->sweep(eventq, sweepline, _threadsSweeped, single);
            if (_crossE != cetype)
               nonCrossE.push_back(*CE);
         }
      }
   }
   if (looped)
      // now - the post-check. All segments belonging to non cross events should be
      // checked for joining points between each other
      for (Events::iterator CE1 = nonCrossE.begin(); CE1 != nonCrossE.end(); CE1++)
         for (Events::iterator CE2 = CE1; CE2 != nonCrossE.end(); CE2++)
            CheckBEM(eventq, **CE1, **CE2, single);
}

void polycross::EventVertex::CheckBEM(XQ& eventq, TEvent& thr1, TEvent& thr2, bool single)
{
   if ((!single) && (thr1.aseg()->polyNo() == thr2.aseg()->polyNo())) return;
#ifdef BO2_DEBUG
   printf("Checking BEM ------------------------\n");
#endif
   char sa1sa2 = coincidingSegm(thr1.evertex(), thr1.avertex(), thr2.avertex());
#ifdef BO2_DEBUG
   printf("Points: (%d, %d) ; (%d, %d) ; (%d, %d) ;",thr1.evertex()->x(), thr1.evertex()->y(),
                                                     thr1.avertex()->x(), thr1.avertex()->y(),
                                                     thr2.avertex()->x(), thr2.avertex()->y());
   if (sa1sa2)
      printf("COINCIDING\n");
   else
      printf("\n");
#endif
   char sa1sb2 = coincidingSegm(thr1.evertex(), thr1.avertex(), thr2.bvertex());
#ifdef BO2_DEBUG
   printf("Points: (%d, %d) ; (%d, %d) ; (%d, %d) ;",thr1.evertex()->x(), thr1.evertex()->y(),
                                                     thr1.avertex()->x(), thr1.avertex()->y(),
                                                     thr2.bvertex()->x(), thr2.bvertex()->y());
   if (sa1sb2)
      printf("COINCIDING\n");
   else
      printf("\n");
#endif
   char sb1sa2 = coincidingSegm(thr1.evertex(), thr1.bvertex(), thr2.avertex());
#ifdef BO2_DEBUG
   printf("Points: (%d, %d) ; (%d, %d) ; (%d, %d) ;",thr1.evertex()->x(), thr1.evertex()->y(),
                                                     thr1.bvertex()->x(), thr1.bvertex()->y(),
                                                     thr2.avertex()->x(), thr2.avertex()->y());
   if (sb1sa2)
      printf("COINCIDING\n");
   else
      printf("\n");
#endif
   char sb1sb2 = coincidingSegm(thr1.evertex(), thr1.bvertex(), thr2.bvertex());
#ifdef BO2_DEBUG
   printf("Points: (%d, %d) ; (%d, %d) ; (%d, %d) ;",thr1.evertex()->x(), thr1.evertex()->y(),
                                                     thr1.bvertex()->x(), thr1.bvertex()->y(),
                                                     thr2.bvertex()->x(), thr2.bvertex()->y());
   if (sb1sb2)
      printf("COINCIDING\n");
   else
      printf("\n");
#endif
#ifdef BO2_DEBUG
   printf("-------------------------------------\n");
#endif
   if    ((0 == sa1sa2) && (0 == sa1sb2) && (0 == sb1sa2) && (0 == sb1sb2))
   {
      //check for threads, crossing in the event vertex point
      float lsignA = orientation(thr1.evertex(), thr1.avertex(), thr2.avertex());
      float rsignA = orientation(thr1.evertex(), thr1.avertex(), thr2.bvertex());
      float lsignB = orientation(thr1.evertex(), thr1.bvertex(), thr2.avertex());
      float rsignB = orientation(thr1.evertex(), thr1.bvertex(), thr2.bvertex());
      // it is still possible to get one of the signs eq 0. This is the case
      // when the points are lined-up, but thr1.evertex() is in the middle
      float signA = lsignA * rsignA;
      float signB = lsignB * rsignB;
      //the BIG question here is WHERE to place the cross points
      if ((signA < 0) || (signB < 0))
      {
         thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.bseg(), eventq, true );
         thr1.insertCrossPoint(thr1.evertex(), thr1.aseg(), thr2.aseg(), eventq, true );
      }
   }
   else if ((sa1sa2  != 0) && (0 == sa1sb2) && (0 == sb1sa2) && (0 == sb1sb2))
      thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.bseg(), eventq, true );
   else if ((sa1sb2  != 0) && (0 == sa1sa2) && (0 == sb1sa2) && (0 == sb1sb2))
      thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.aseg(), eventq, true );
   else if ((sb1sa2  != 0) && (0 == sa1sa2) && (0 == sa1sb2) && (0 == sb1sb2))
      thr1.insertCrossPoint(thr1.evertex(), thr1.aseg(), thr2.bseg(), eventq, true );
   else if ((sb1sb2  != 0) && (0 == sa1sa2) && (0 == sa1sb2) && (0 == sb1sa2))
      thr1.insertCrossPoint(thr1.evertex(), thr1.aseg(), thr2.aseg(), eventq, true );
   else if ((0 == sa1sb2) && (0 == sb1sa2))
      if (sa1sa2 * sb1sb2 < 0)
         thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.bseg(), eventq, true );
      else
      {
         // segment threads overlapping each other - additional cross point is not required
      }
   else if ((0 == sa1sa2) && (0 == sb1sb2))
      if (sa1sb2 * sb1sa2 < 0)
         thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.aseg(), eventq, true );
      else
      {
         // segment threads overlapping each other - additional cross point is not required
      }
   else
      throw EXPTNpolyCross("Unexpected combination of joining segments");
}

void polycross::EventVertex::sweep2bind(YQ& sweepline, BindCollection& bindColl)
{
#ifdef BO2_DEBUG
   printf("______________ POINT = ( %i , %i ) ___________\n", _evertex->x(), _evertex->y());
#endif
   for( int cetype = _endE; cetype <= _crossE; cetype++)
   {
      if (_events.end() != _events.find(cetype))
      {
         Events& simEvents = _events[cetype];
         for( Events::iterator CE = simEvents.begin(); CE != simEvents.end() ; CE++)
            (*CE)->sweep2bind(sweepline, bindColl);
      }
   }
}

void polycross::EventVertex::clearAllEvents()
{
   for (polycross::EventVertex::AllEvents::iterator CE = _events.begin(); CE != _events.end(); CE++)
   {
      Events& simEvents = CE->second;
      while (!simEvents.empty())
      {
         TEvent* cevent = simEvents.front(); simEvents.pop_front();
         delete cevent;
      }
   }
}

polycross::EventVertex::~EventVertex()
{
   clearAllEvents();
   delete _evertex;
}

//===========================================================================
// Segment Thread
polycross::polysegment* polycross::SegmentThread::set_cseg(polysegment* cs)
{
   polysegment* oldseg = _cseg;
   _cseg = cs;
   return oldseg;
}

bool polycross::SegmentThread::operator == (const SegmentThread& cmp) const
{
   return ((*_cseg) == *(cmp._cseg));
}

//==============================================================================
// YQ
polycross:: YQ::YQ(DBbox& overlap, const segmentlist* seg1, const segmentlist* seg2)
{
   _osl1 = seg1;
   _osl2 = seg2;
   initialize(overlap);
}

polycross:: YQ::YQ(DBbox& overlap, const segmentlist* seg)
{
   _osl1 = seg;
   _osl2 = NULL;
   initialize(overlap);
}

void polycross:: YQ::initialize(DBbox& overlap)
{
   _blSent = DEBUG_NEW TP(overlap.p1().x()-1, overlap.p1().y()-1);
   _brSent = DEBUG_NEW TP(overlap.p2().x()+1, overlap.p1().y()-1);
   _tlSent = DEBUG_NEW TP(overlap.p1().x()-1, overlap.p2().y()+1);
   _trSent = DEBUG_NEW TP(overlap.p2().x()+1, overlap.p2().y()+1);
   _bottomSentinel = DEBUG_NEW BottomSentinel(DEBUG_NEW polysegment(_blSent, _brSent, -1, 0));
   _cthreads[-2] = _bottomSentinel;
   _topSentinel = DEBUG_NEW TopSentinel(DEBUG_NEW polysegment(_tlSent, _trSent, -1, 255));
   _cthreads[-1] = _topSentinel;
   _bottomSentinel->set_threadAbove(_topSentinel);
   _topSentinel->set_threadBelow(_bottomSentinel);
   _lastThreadID = 0;
}

/**
 * Insert a new segment thread in the YQ
 * @param startseg the first segment of the thread that became current
 * @return the created thread
 */
polycross::SegmentThread* polycross::YQ::beginThread(polycross::polysegment* startseg)
{
   if (0 != startseg->threadID())
      throw EXPTNpolyCross("Unsorted segment expected here");
   SegmentThread* above = _bottomSentinel;
   while (sCompare(startseg, above->cseg()) > 0)
      above = above->threadAbove();

   SegmentThread* below = above->threadBelow();
   SegmentThread* newthread = DEBUG_NEW SegmentThread(startseg, below, above);
   above->set_threadBelow(newthread);
   below->set_threadAbove(newthread);
   _cthreads[++_lastThreadID] = newthread;
   startseg->set_threadID(_lastThreadID);
   return newthread;
}

/**
 * Removes a segment thread from YQ and re-links its neighbors accordingly
 * @param threadID The ID of the thread to be removed
 * @return the thread that use to be below the one removed
 */
polycross::SegmentThread* polycross::YQ::endThread(unsigned threadID)
{
   // get the thread from the _cthreads
   Threads::iterator threadP = _cthreads.find(threadID);
   if (_cthreads.end() == threadP)
      throw EXPTNpolyCross("Segment thread not found in YQ - end");
   SegmentThread* thread = threadP->second;
   // relink above/below threads
   SegmentThread* nextT = thread->threadAbove();
   if (NULL == nextT)
      throw EXPTNpolyCross("Unable to remove the segment thread properly");
   nextT->set_threadBelow(thread->threadBelow());
   SegmentThread* prevT = thread->threadBelow();
   if (NULL == prevT)
      throw EXPTNpolyCross("Unable to remove the segment thread properly");
   prevT->set_threadAbove(thread->threadAbove());
   // erase it
   delete(threadP->second);
   _cthreads.erase(threadP);

   return prevT;
}

/**
 * Replaces the current segment in a segment thread
 * @param threadID the target segment thread
 * @param newsegment the new segment - to replace the existing one
 * @return the segment thread
 */
polycross::SegmentThread* polycross::YQ::modifyThread(unsigned threadID, polysegment* newsegment, bool& smooth)
{
   // get the thread from the _cthreads
   Threads::iterator threadP = _cthreads.find(threadID);
   if (_cthreads.end() == threadP)
      throw EXPTNpolyCross("Segment thread not found in YQ - modify");
   SegmentThread* thread = threadP->second;

   // the check below is to help to the eventual cross event in this
   // point (where the thread modify is executed) to decide whether 
   // to skip the thread swaping (i.e. joining/touching etc. corner cases)
   // find the position of the new incoming segment in the thread queue
   SegmentThread* above = _bottomSentinel;
   while (sCompare(newsegment, above->cseg()) > 0)
      above = above->threadAbove();
   SegmentThread* below = above->threadBelow();
   // check whether the position is the same as for the old segment
   // Note! the thread is already in the queue - hence the fancy check below
   if (*above == *thread)
      smooth = ((*below == *(thread->threadBelow()))    );
   else if (*below == *thread)
      smooth = ((*above == *(thread->threadAbove()))    );
   else
      smooth = false;

   newsegment->set_threadID(threadID);
   polysegment* oldsegment = thread->set_cseg(newsegment);
   oldsegment->set_threadID(0);
   return thread;
}

/**
 * Swap the threads with the corresponding IDs
 * @param tAID The ID of the thread above before the swap
 * @param tBID The ID of the thread below before the swap
 * @return pointer to the thread below after the swap
 */
polycross::SegmentThread* polycross::YQ::swapThreads(unsigned tAID, unsigned tBID)
{
   // get the threads from the _cthreads
   Threads::iterator tiAbove = _cthreads.find(tAID);
   Threads::iterator tiBelow = _cthreads.find(tBID);
   if (_cthreads.end() == tiAbove)
      throw EXPTNpolyCross("Segment thread not found in YQ - swap");
   if (_cthreads.end() == tiBelow)
      throw EXPTNpolyCross("Segment thread not found in YQ - swap");
   SegmentThread* tAbove = tiAbove->second;
   SegmentThread* tBelow = tiBelow->second;
   // relink above/below threads
   if (tAbove != tBelow->threadAbove())
      throw EXPTNpolyCross("Unable to swap the segment threads properly");
   if (tBelow != tAbove->threadBelow())
      throw EXPTNpolyCross("Unable to swap the segment threads properly");
   // first  fix the pointers of the neighboring threads
   tBelow->threadBelow()->set_threadAbove(tAbove);
   tAbove->threadAbove()->set_threadBelow(tBelow);
   // now swap above pointers ...
   tBelow->set_threadAbove(tAbove->threadAbove());
   tAbove->set_threadAbove(tBelow); // <===!
   // ... and below pointers ...
   tAbove->set_threadBelow(tBelow->threadBelow());
   tBelow->set_threadBelow(tAbove); // <===!

   return tAbove;
}

polycross::SegmentThread* polycross::YQ::getThread(unsigned tID)
{
   Threads::iterator ti = _cthreads.find(tID);
   if (_cthreads.end() == ti)
      throw EXPTNpolyCross("Segment thread not found in YQ - get");
   return ti->second;
}

int polycross::YQ::sCompare(const polysegment* seg0, const polysegment* seg1)
{
   // Current point is always lp of seg0
   if (seg0 == seg1)
      throw EXPTNpolyCross("Different segments expected here");
   //
   int ori;
   ori = orientation(seg1->lP(), seg1->rP(), seg0->lP());
   if (ori != 0) return ori;
   // if it is a crossing point -> compare the slope
   ori = orientation(seg1->lP(), seg1->rP(), seg0->rP());
   if (ori != 0) return ori;

   // if it is still the same => we have coinciding segments
   // Here the fun begins.
   // 1. Retrieve the original Point vectors of both segments, get the indexes
   //    of the right points and the relative direction of point chains
   unsigned indx0RP, indx1RP;
   unsigned dummy;
   bool dir0 = true;
   bool dir1 = true;
   const PointVector* plist0 = locateOriginals(seg0, indx0RP, dummy, dir0);
   const PointVector* plist1 = locateOriginals(seg1, indx1RP, dummy, dir1);
   unsigned indx0RNP, indx1RNP;
   int numv0 = plist0->size();
   int numv1 = plist1->size();
   int loopsentinel = (numv0 > numv1) ? numv0 : numv1;
   // 2. Now loop effectively the same (this) function for every next pair of
   //    segments until they do not coincide
   do
   {
      // calculate the index of the next point in the proper direction
      if (dir0)  indx0RNP = (indx0RP+1) % numv0;
      else       indx0RNP = (0 == indx0RP) ? numv0 -1 : indx0RP - 1;
      //... for both segments
      if (dir1)  indx1RNP = (indx1RP+1) % numv1;
      else       indx1RNP = (0 == indx1RP) ? numv1 -1 : indx1RP - 1;

      ori = orientation(&((*plist1)[indx1RP]), &((*plist1)[indx1RNP]), &((*plist0)[indx0RP]));
      if (0 ==ori)
         ori = orientation(&((*plist1)[indx1RP]), &((*plist1)[indx1RNP]), &((*plist0)[indx0RNP]));
      if (0==ori)
      {//prepare for the next loop
         indx0RP = indx0RNP;
         indx1RP = indx1RNP;
         if (0 ==(--loopsentinel))
         {
            // appears that the polygons coincide?
            ori = 1;
            // The expression below is valid, exception here are collinear wires
            // if (seg0->polyNo() != seg1->polyNo())
            //    ori = 1;
            // else
            //    throw EXPTNpolyCross("Too many iterations in segment compare");
         }
      }
   } while (0 == ori);
   return ori;
}

const PointVector* polycross::YQ::locateOriginals(const polysegment* seg,
                           unsigned& indxNP, unsigned& indxPP, bool& direction)
{
   // get the original PointVector for the comparing segments
   const PointVector* plist = (1 == seg->polyNo()) ? opl1() : opl2();

   // ... and get the location of the inside segment in that sequence
   int numv = plist->size();
   indxPP = (*(seg->lP()) == (*plist)[seg->edge()]) ? seg->edge() : (seg->edge() + 1) % numv;
   indxNP = (*(seg->rP()) == (*plist)[seg->edge()]) ? seg->edge() : (seg->edge() + 1) % numv;

   direction = (indxNP == ((indxPP + 1) % numv));
   return plist;
}


void polycross::YQ::report()
{
   printf("^^^^^^^Threads currently in the YQ - from top to bottom^^^^^^^^^\n");
   SegmentThread* current = _topSentinel->threadBelow();
   while (current != _bottomSentinel)
   {
      BO_printseg(current->cseg());
      current = current->threadBelow();
   }
}

polycross::YQ::~YQ()
{
   delete _topSentinel;
   delete _bottomSentinel;
   delete _blSent;
   delete _brSent;
   delete _tlSent;
   delete _trSent;
}

//==============================================================================
// XQ
polycross::XQ::XQ( const segmentlist& seg1, const segmentlist& seg2 ) :
      _overlap(*(seg1[0]->lP()))
{
   _xQueue = avl_create(E_compare, NULL, NULL);
   _xOldQueue = avl_create(E_compare, NULL, NULL);
   createEvents(seg1);
   createEvents(seg2);
   _sweepLine = DEBUG_NEW YQ(_overlap, &seg1, &seg2);
}

polycross::XQ::XQ( const segmentlist& seg, bool loopsegs ) :
                     _overlap(*(seg[0]->lP())), _loopSegs(loopsegs)
{
   _xQueue = avl_create(E_compare, NULL, NULL);
   _xOldQueue = avl_create(E_compare, NULL, NULL);
   if (_loopSegs)
      createEvents(seg);
   else
      createSEvents(seg);
   _sweepLine = DEBUG_NEW YQ(_overlap, &seg);
}

void polycross::XQ::createEvents(const segmentlist& seg)
{
   unsigned s1, s2;
   for(s1 = 0, s2 = 1 ; s1 < seg.size(); s1++, ++s2 %= seg.size())
   {
      // determine the type of event from the neighboring segments
      // and create the thread event
      if (seg[s1]->lP() == seg[s2]->lP())
         addEvent(seg[s1],DEBUG_NEW TbEvent(seg[s1], seg[s2]),_beginE);
      else if (seg[s1]->rP() == seg[s2]->rP())
         addEvent(seg[s1],DEBUG_NEW TeEvent(seg[s1], seg[s2]),_endE);
      else // normal middle point for polygons
         addEvent(seg[s1],DEBUG_NEW TmEvent(seg[s1], seg[s2]),_modifyE);
   }
}

/*! create single events - used for open shapes - wires
First and last points are treated differently. They infer a special single threaded events
TbsEvent and TesEvent
*/
void polycross::XQ::createSEvents(const segmentlist& seg)
{
   unsigned s1, s2;
   // first point
   s1 = 0;
   if       ( (seg[s1]->rP() == seg[s1+1]->lP()) || (seg[s1]->rP() == seg[s1+1]->rP()) )
      addEvent(seg[s1],DEBUG_NEW TbsEvent(seg[s1]),_beginE);//lP is a begin
   else
      addEvent(seg[s1],DEBUG_NEW TesEvent(seg[s1]),_endE  );// rp is an end
   // last point
   s1 = seg.size() - 1;
   if       ( (seg[s1]->rP() == seg[s1-1]->lP()) || (seg[s1]->rP() == seg[s1-1]->rP()) )
      addEvent(seg[s1],DEBUG_NEW TbsEvent(seg[s1]),_beginE);//lP is a begin
   else
      addEvent(seg[s1],DEBUG_NEW TesEvent(seg[s1]),_endE  );// rp is an end

   for(s1 = 0, s2 = 1 ; s2 < seg.size(); s1++, s2++ )
   {
      // determine the type of event from the neighboring segments
      // and create the thread event
      if (seg[s1]->lP() == seg[s2]->lP())
      {
         int ori = orientation(seg[s2]->lP(), seg[s2]->rP(), seg[s1]->rP());
         if (0 == ori)
         { // collinear segments
            addEvent(seg[s1],DEBUG_NEW TbsEvent(seg[s1]),_beginE);
            addEvent(seg[s2],DEBUG_NEW TbsEvent(seg[s2]),_beginE);
         }
         else
            addEvent(seg[s1],DEBUG_NEW TbEvent(seg[s1], seg[s2]),_beginE);
      }
      else if (seg[s1]->rP() == seg[s2]->rP())
      {
         int ori = orientation(seg[s2]->lP(), seg[s2]->rP(), seg[s1]->lP());
         if (0 == ori)
         { // collinear segments
            addEvent(seg[s1],DEBUG_NEW TesEvent(seg[s1]),_endE );
            addEvent(seg[s2],DEBUG_NEW TesEvent(seg[s2]),_endE );
         }
         else
            addEvent(seg[s1],DEBUG_NEW TeEvent(seg[s1], seg[s2]),_endE);
      }
      else
         addEvent(seg[s1],DEBUG_NEW TmEvent(seg[s1], seg[s2]),_modifyE);
   }
}

void polycross::XQ::addEvent(polysegment* cseg, TEvent* evt, EventTypes etype)
{
   // update overlapping box
   _overlap.overlap(*(cseg->lP()));
   _overlap.overlap(*(cseg->rP()));
   // now create the vertex with the event inside
   EventVertex* vrtx = DEBUG_NEW EventVertex(evt->evertex());
   // and try to stick it in the AVL tree
   void** retitem =  avl_probe(_xQueue,vrtx);
   if ((*retitem) != vrtx)
      // coinsiding vertexes from different polygons
      delete(vrtx);
   // finally add the event itself
   static_cast<EventVertex*>(*retitem)->addEvent(evt,etype);
}

void polycross::XQ::addCrossEvent(const TP* CP, polysegment* aseg, polysegment* bseg)
{
   TcEvent* evt = DEBUG_NEW TcEvent(CP, aseg, bseg);
   // now create the vertex with the event inside
   EventVertex* vrtx = DEBUG_NEW EventVertex(evt->evertex());
   // check that such a vertex has been already swiped
   void* oldVertex = avl_delete(_xOldQueue, vrtx); // i.e. find, remove and return
   if (NULL != oldVertex)
   {  // yes, we had been already here, so
      // delete the EventVertex we've just created ...
      delete vrtx;
      // ... and replace it with already existing EventVertex
      vrtx = static_cast<EventVertex*>(oldVertex);
      // it's important to clear all events which had been already taken into account
      vrtx->clearAllEvents();
   }
   // and try to stick it in the AVL tree
   void** retitem =  avl_probe(_xQueue,vrtx);
   if ((*retitem) != vrtx)
   {
      // Coinciding vertexes from different polygons
      delete(vrtx);
   }
   static_cast<EventVertex*>(*retitem)->addEvent(evt, _crossE);
}

int polycross::XQ::E_compare( const void* v1, const void* v2, void*)
{
   const TP* p1 = (*static_cast<const EventVertex*>(v1))();
   const TP* p2 = (*static_cast<const EventVertex*>(v2))();
   return xyorder(p1,p2);
}

void polycross::XQ::sweep(bool single, bool looped)
{
// see the comment around find parameter in the E_compare
#ifdef BO2_DEBUG
      printf("***************** START SWIPING ******************\n");
#endif
   EventVertex* evtlist;
   avl_traverser trav;
   while (NULL != avl_t_first(&trav,_xQueue))
   {
      evtlist = (EventVertex*)trav.avl_node->avl_data;
      evtlist->sweep(*_sweepLine, *this, single, looped);
      avl_delete(_xQueue,evtlist);
      avl_insert(_xOldQueue, evtlist);
   }
   // at this point we can clear all the EventVertexes from the _xOldQueue
   evtlist = static_cast<EventVertex*>(avl_t_first(&trav,_xOldQueue));
   while (NULL != evtlist)
   {
      delete evtlist;
      evtlist = static_cast<EventVertex*>(avl_t_next(&trav));
   }
}

void polycross::XQ::sweep2bind(BindCollection& bindColl) {
#ifdef BO2_DEBUG
      printf("***************** START BIND SWIPING ******************\n");
#endif
   EventVertex* evtlist;
   avl_traverser trav;
   while (NULL != avl_t_first(&trav,_xQueue))
   {
      evtlist = (EventVertex*)trav.avl_node->avl_data;
      evtlist->sweep2bind(*_sweepLine, bindColl);
      avl_delete(_xQueue,evtlist);
      delete evtlist;
   }
}

polycross::XQ::~XQ()
{
   avl_destroy(_xQueue, NULL);
   avl_destroy(_xOldQueue, NULL);
   delete _sweepLine;
}

//==============================================================================
// BindCollection
void polycross::BindCollection::update_BL(polysegment* outseg, unsigned poly1seg,
                                          const TP* poly1pnt)
{
   unsigned poly0seg = outseg->edge();
   // calculate the distance between the point poly1pnt and the segment poly0seg
   // first calculate the coefficients of the poly0seg line equation
   real A = outseg->rP()->y() - outseg->lP()->y();
   real B = outseg->lP()->x() - outseg->rP()->x();
   real C = -(A*outseg->lP()->x() + B*outseg->lP()->y());
   assert((A != 0) || (B != 0));
   // find the point poly0pnt where the perpendicular from poly1pnt to
   // poly0seg crosses the latter
   real line1 = A*poly1pnt->x() + B*poly1pnt->y() + C;
   real denom = A*A + B*B;
   real X = poly1pnt->x() - (A / denom) * line1;
   real Y = poly1pnt->y() - (B / denom) * line1;
   TP* poly0pnt = DEBUG_NEW TP((int)rint(X),(int)rint(Y));
   // if the point lies inside the segment
   if (getLambda(outseg->lP(), outseg->rP(), poly0pnt) >= 0)
   {
      // get the distance
      real distance = fabs(line1 / sqrt(denom));
      // and if it's shorter than already stored one for this segment
      // or of no distance calculated from this segment
      if (is_shorter(poly0seg, distance))
         // store the data
         _blist.push_back(DEBUG_NEW BindSegment(poly0seg, poly1seg, poly0pnt, poly1pnt, distance));
      else delete poly0pnt;
   }
   else delete poly0pnt;
}

bool polycross::BindCollection::is_shorter(unsigned segno, real dist)
{
   for (BindList::iterator BI = _blist.begin(); BI != _blist.end(); BI++)
   {
      if ((*BI)->poly0seg() == segno)
      {
         if ((*BI)->distance() > dist)
         {
            delete (*BI);
            _blist.erase(BI);
            return true;
         }
         else return false;
      }
   }
   return true;
}

polycross::BindSegment* polycross::BindCollection::getBindSegment(const pcollection& obstructions)
{
   _blist.sort(compareSegments);
   // the bind segment candidate will be checked towards all obstruction objects
   for (BindList::const_iterator BI = _blist.begin(); BI != _blist.end(); BI++)
   {
      bool valid = true;
      for (pcollection::const_iterator OI = obstructions.begin(); OI != obstructions.end(); OI++)
         if (obstructed(**BI, **OI))
         {
            valid = false; break;
         }
      if (valid) return *BI;
   }
   //Can't find a clear bind segment which is not crossing the objects in obstructions list
   return NULL;
}

bool polycross::BindCollection::compareSegments(polycross::BindSegment* seg1, polycross::BindSegment* seg2)
{
   return (seg1->distance() < seg2->distance());
}

/**
 * This function actually checks whether a segment is crossing a polygon. The swipe line
 * can't be used because it requires effectively closed polygons (one or two)
 */
bool polycross::BindCollection::obstructed(const BindSegment& segment, const PointVector& obs)
{
   unsigned plysize = obs.size();
   int ori1, ori2;
   for (unsigned i = 0; i < plysize; i++)
   {
      ori1 = orientation(&(obs[i]), &(obs[(i+1)%plysize]), segment.poly0pnt());
      ori2 = orientation(&(obs[i]), &(obs[(i+1)%plysize]), segment.poly1pnt());
      if ( 0 < ori1*ori2 ) continue;
      ori1 = orientation(segment.poly0pnt(), segment.poly1pnt(), &(obs[  i          ]));
      ori2 = orientation(segment.poly0pnt(), segment.poly1pnt(), &(obs[(i+1)%plysize]));
      if ( 0 < ori1*ori2 ) continue;
      if ( 0 > ori1*ori2 ) return true;
      //deal with segments on one line
      if ( 0 == ori1 )
      {
         real lambda = getLambda(segment.poly0pnt(), segment.poly1pnt(), &(obs[  i          ]));
         if (lambda >= 0) return true;
      }
      if ( 0 == ori2 )
      {
         real lambda = getLambda(segment.poly0pnt(), segment.poly1pnt(), &(obs[(i+1)%plysize]));
         if (lambda >= 0) return true;
      }
   }
   return false;
}

polycross::BindCollection::~BindCollection()
{
   for (BindList::iterator BL = _blist.begin(); BL != _blist.end(); BL++)
      delete (*BL);
   _blist.clear();
}


/** Some code sniplets (all of them "almost" working) attempting to preserve
 * the CVC K case points in checkNreorder() above. It is all about resolving
 * cut_test3A case. All in all - the idea of preserving those points proved
 * to be a horrible idea because it is virtually impossible to sort them in
 * an order suitable for the post-processing.
 */
/*
// we have a touching edges (K case) and we're post-processing two
// polygon case. The hitch is to remove both crossing points exactly as
// in the single polygon case above. That would be fine only in 99.9%
// of the cases though. The exception is the case when we have polygon A
// entirely inside the polygon B, but with some touching points. It
// turns out that if we clean-up the cross points in that case there is
// effectively no way to process that case properly. So we have to deal
// with it here!
// Here is the summary of what has to be done:
// - find-out whether the K case is touching from inside or from outside
// - if the touching point is from outside - clean-up the crossing points
// - if the touching points are from inside - we have to keep them BUT!
//   -- make sure that they are sorted properly i.e. they are not cross
//      coupled
// So - let's dance!
// 1. Get points neighboring to this vertex on this object ...
// First find the neighboring points which does not coincide with this vertex
spV = prevCrossCouple->prev();
while (*spV->cp() == *cp()) spV = spV->prev();
snV = nextCrossCouple->next();
while (*snV->cp() == *cp()) snV = snV->next();

pV = prevCross;
do pV = pV->prev();
while  (0 == (oriP = orientation(spV->cp(), snV->cp(), pV->cp())));

nV = prevCrossCouple;
do nV = nV->prev();
while  (0 == (oriN = orientation(spV->cp(), snV->cp(), nV->cp())));

//         spV = prevCross->prev();
//         while (*spV->cp() == *cp())     spV = spV->prev();
//         snV = nextCross->next();
//         while (*snV->cp() == *cp())     snV = snV->next();
//         // ... and calculate the orientation of the triangle formed by them and
//         // this vertex. Make sure they are not lined-up
//         while  (0 == (oriP = orientation(spV->cp(), snV->cp(), cp() )))
//            spV = spV->prev();
//         // 2. Get points neighboring to this vertex on pairedShape object ...
//         spV = prevCrossCouple->prev();
//         while (*spV->cp() == *cp())     spV = spV->prev();
//         snV = nextCrossCouple->next();
//         while (*snV->cp() == *cp())     snV = snV->next();
//         // ... and calculate the orientation of the triangle formed by them and
//         // this vertex. Make sure they are not lined-up
//         while  (0 == (oriN = orientation(spV->cp(), snV->cp(), cp() )))
//            spV = spV->prev();
// 3. Check whether the triangles above are equally oriented
if (oriN == oriP)
{ //4. The orientation is the same - means that the vertex is touching
  //   inside. We have to keep the cross points

//         Code below is doing additional checks and sorting which appears to be
//         redundant (but one can never be sure with this algo!) essentially it
//         exploits the cases when there are more than one touching point in a
//         vertex. The reason it looks redundant is that after the implementation
//         of the inside/outside K case this should not be possible - i.e. there
//         can be only one touching point from the inside of the polygon
//         that there should not be two
//            if (prevCrossCouple->next() == nextCrossCouple)
//               // nothing to do here - cross-coupled points are neighbors
//               assert(nextCrossCouple->prev() == prevCrossCouple);
//            else if (prevCrossCouple->prev() == nextCrossCouple)
//               assert(false);
//            else if (prevCrossCouple->next() == nextCrossCouple->prev())
//            {
//               // we have one cross-coupled point between ...
//               CPoint* inter = static_cast<CPoint*>(prevCrossCouple->next());
//               // ... make sure it is on the same vertex ...
//               assert(*prevCrossCouple->cp() == *inter->cp());
//               // ... make sure it is a cross point ...
//               assert(0 == inter->visited());
//               //... OK, swap the intermediate point with the nextCrossCouple
//               inter->set_next(nextCrossCouple->next());
//               nextCrossCouple->next()->set_prev(inter);
//               inter->set_prev(nextCrossCouple);
//               nextCrossCouple->set_next(inter);
//               prevCrossCouple->set_next(nextCrossCouple);
//               nextCrossCouple->set_prev(prevCrossCouple);
//            }
//            else
//            {
//               // we have 2 or more cross-coupled point between ...
//               CPoint* interP = static_cast<CPoint*>(prevCrossCouple->next());
//               CPoint* interN = static_cast<CPoint*>(nextCrossCouple->prev());
//               // ... make sure they are on the same vertex ...
//               assert(*prevCrossCouple->cp() == *interP->cp());
//               assert(*nextCrossCouple->cp() == *interN->cp());
//               // ... make sure they are crossing points ...
//               assert(0 == interP->visited());
//               assert(0 == interN->visited());
//               interP->set_next(nextCrossCouple->next());
//               nextCrossCouple->next()->set_prev(interP);
//               interN->set_prev(nextCrossCouple);
//               nextCrossCouple->set_next(interN);
//               prevCrossCouple->set_next(nextCrossCouple);
//               nextCrossCouple->set_prev(prevCrossCouple);
//            }
}
else
{ //4. The orientation is different - means that the vertex is touching
  //   outside. Remove the crossing points
   prevCross->prev()->set_next(this); set_prev(prevCross->prev());
   nextCross->next()->set_prev(this); set_next(nextCross->next());
   delete prevCross;
   delete nextCross;
   // Cross coupled points are non necessarily neighbors in the second
   // polygon. We can have a plenty of them in one vertex - so take them
   // out one by one. First prevCrossCouple...
   prevCrossCouple->prev()->set_next(prevCrossCouple->next());
   prevCrossCouple->next()->set_prev(prevCrossCouple->prev());
   if (prevCrossCouple == pairedShape)
      pairedShape = prevCrossCouple->prev();
   delete prevCrossCouple;
   // ...then nextCrossCouple
   nextCrossCouple->prev()->set_next(nextCrossCouple->next());
   nextCrossCouple->next()->set_prev(nextCrossCouple->prev());
   if (nextCrossCouple == pairedShape)
      pairedShape = nextCrossCouple->next();
   delete nextCrossCouple;
}
*/

/*
         // test orphographical orientation to eventually swap the coupling
         spV = prevCross->prev();
         while (*spV->cp() == *cp())     spV = spV->prev();
         snV = nextCross->next();
         while (*snV->cp() == *cp())     snV = snV->next();
         int orderA = xyorder(spV->cp(), snV->cp());
         assert(0 != orderA);
         spV = prevCrossCouple->prev();
         while (*spV->cp() == *cp())     spV = spV->prev();
         snV = nextCrossCouple->next();
         while (*snV->cp() == *cp())     snV = snV->next();
         int orderB = xyorder(spV->cp(), snV->cp());
         assert(0 != orderB);

         if (orderA != orderB)
         {
            // swap the links
            if (prevCrossCouple->next() == nextCrossCouple)
            {
               assert(nextCrossCouple->prev() == prevCrossCouple);
               spV = prevCrossCouple->prev();
               snV = nextCrossCouple->next();
               prevCrossCouple->set_next(snV); snV->set_prev(prevCrossCouple);
               nextCrossCouple->set_prev(spV); spV->set_next(nextCrossCouple);
            }
            else if (prevCrossCouple->prev() == nextCrossCouple)
               assert(false);
            else if (prevCrossCouple->next() == nextCrossCouple->prev())
            {
               // we have one cross-coupled point between ...
               CPoint* inter = static_cast<CPoint*>(prevCrossCouple->next());
               // ... make sure it is on the same vertex ...
               assert(*prevCrossCouple->cp() == *inter->cp());
               // ... make sure it is a cross point ...
               assert(0 == inter->visited());
               //... OK, swap the intermediate point with the nextCrossCouple
               inter->set_next(nextCrossCouple->next());
               nextCrossCouple->next()->set_prev(inter);
               inter->set_prev(nextCrossCouple);
               nextCrossCouple->set_next(inter);
               prevCrossCouple->set_next(nextCrossCouple);
               nextCrossCouple->set_prev(prevCrossCouple);
            }
            else
            {
               // we have 2 or more cross-coupled point between ...
               CPoint* interP = static_cast<CPoint*>(prevCrossCouple->next());
               CPoint* interN = static_cast<CPoint*>(nextCrossCouple->prev());
               // ... make sure they are on the same vertex ...
               assert(*prevCrossCouple->cp() == *interP->cp());
               assert(*nextCrossCouple->cp() == *interN->cp());
               // ... make sure they are crossing points ...
               assert(0 == interP->visited());
               assert(0 == interN->visited());
               interP->set_next(nextCrossCouple->next());
               nextCrossCouple->next()->set_prev(interP);
               interN->set_prev(nextCrossCouple);
               nextCrossCouple->set_next(interN);
               prevCrossCouple->set_next(nextCrossCouple);
               nextCrossCouple->set_prev(prevCrossCouple);
            }
         }
*/
