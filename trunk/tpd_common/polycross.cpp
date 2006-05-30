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
//        Created: Tue Mar 21 2006
//         Author: s_krustev@yahoo.com
//      Copyright: (C) 2001-2006 by Svilen Krustev
//    Description: Modified Bentley-Ottman algorithm
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <math.h>
#include <algorithm>
#include "polycross.h"
#include "outbox.h"

//#define BO2_DEBUG
#define BO_printseg(SEGM) printf("thread %i : polygon %i, segment %i, \
lP (%i,%i), rP (%i,%i)  \n" , SEGM->threadID(), SEGM->polyNo() , SEGM->edge(), \
SEGM->lP()->x(), SEGM->lP()->y(), SEGM->rP()->x(),SEGM->rP()->y());

//-----------------------------------------------------------------------------
// The declaratoin of the avl related functions. They are declared originally
// in avl.h and redeclared here in C++ manner with extern "C" clause
extern "C" {
   avl_table* avl_create (avl_comparison_func *, const void *, libavl_allocator *);
   void avl_destroy (struct avl_table *, avl_item_func *);
   void **avl_probe (struct avl_table *, void *);
   void*      avl_delete (avl_table *, const void *);
   void*      avl_t_first (avl_traverser *, avl_table *);
}

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
   // test X coord first
   if (p1->x() > p2->x()) return  1;
   if (p1->x() < p2->x()) return -1;
   // and then y
   if (p1->y() > p2->y()) return  1;
   if (p1->y() < p2->y()) return -1;
   return 0;
}

int polycross::orientation(const TP* p1, const TP* p2, const TP* p3)
{
   // twice the "orientated" area of the enclosed triangle
   real area = (real(p1->x()) - real(p3->x())) * (real(p2->y()) - real(p3->y())) -
         (real(p2->x()) - real(p3->x())) * (real(p1->y()) - real(p3->y()));
   if (0 == area) return 0;
   else
      return (area > 0) ? 1 : -1;
}

/**
 * Cheeck whether point p lies inside the segment defined by p1 and p2. The
function assumes that p lies on the line defined by p1 and p2
 * @param p1 first point defining the input segment
 * @param p2 second point defining the input segment
 * @param p the point to be checked
 * @return < 0  the point is outside the segment
           = 0  the point coinsides with one of the segment endpoints
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

bool polycross::coinsidingSegm(const TP* pa, const TP* pb, const TP* pc)
{
   if (0 != orientation(pa, pb, pc)) return false;
   if ((getLambda(pa, pb, pc) >= 0) || (getLambda(pa, pc, pb) >= 0))
      return true;
   else return false;
}
//==============================================================================
// VPoint
polycross::VPoint::VPoint(const TP* point, VPoint* prev) : _cp(point), _prev(prev)
{
   if (_prev) _prev->_next = this;
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
neightboring points crosses the ray. If it doesn't increment by 2.

If the point lies on any of the polygon segments, we abort the check, and depending
on @touching input parameter - report the result immediately.

Finally, divide the score by two and if the result is odd - the point is inside.
 * @param plist input polygon
 * @param touching when the point lies on a polygon edge, then depending on this
parameter, procedure reports the point inside (if true) or outside the polygon
 * @return true if the point is inside the polygon
 */
bool polycross::VPoint::inside(const pointlist& plist, bool touching)
{
   TP p0, p1;
   byte cc = 0;
   unsigned size = plist.size();
   for (unsigned i = 0; i < size ; i++)
   {
      p0 = plist[i]; p1 = plist[(i+1) % size];
      if (((p0.y() <= _cp->y()) && (p1.y() >=  _cp->y()))
            ||((p0.y() >=  _cp->y()) && (p1.y() <= _cp->y())) )
      {
         int ori = orientation(&p0, &p1, _cp);
         if ((0==ori) && (getLambda(&p0, &p1, _cp) >= 0))
         {
            //_cp lies on the edge p0-p1
            if (touching) return true;
            else return false;
         }
         else
         {
            if ((p1.y() == p0.y()) && (_cp->x() < p1.x()))
            {
               // segment parallel to the ray and right of _cp
               // check neighbouring points
               unsigned indx0 = (0==i) ? size-1 : i-1;
               unsigned indx1 = (i+2) % size;
               p0 = plist[indx0]; p1 = plist[indx1];
               if (!(((p0.y() <= _cp->y()) && (p1.y() >=  _cp->y()))
                     ||((p0.y() >=  _cp->y()) && (p1.y() <= _cp->y())) ))
                  cc+=2;
            }
            else
            {
               float tngns = (float) (_cp->y() - p0.y())/(p1.y() - p0.y());
               float calcx = p0.x() + tngns * (p1.x() - p0.x());
               if (_cp->x() <= calcx)
                  // if ray touches the segment
                  if ((_cp->y() == p0.y()) || (_cp->y() == p1.y())) cc++;
                  // ray crosses the segment
                  else cc+=2;
            }
         }
      }
   }
   assert(0 == (cc % 2));
   cc /= 2;
   return (cc & 0x01) ? true : false;
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

polycross::VPoint* polycross::VPoint::checkNreorder(VPoint*& pairedShape)
{
   CPoint* nextCross = static_cast<CPoint*>(_next);
   CPoint* prevCross = static_cast<CPoint*>(_prev);
   assert(*(prevCross->cp()) == *(nextCross->cp()));
   CPoint* nextCrossCouple = nextCross->link();
   CPoint* prevCrossCouple = prevCross->link();
   if (!(*(prevCrossCouple->next()->cp()) == *(nextCrossCouple->cp())))
   {
      // swap the links
      prevCross->linkto(nextCrossCouple); nextCrossCouple->linkto(prevCross);
      nextCross->linkto(prevCrossCouple); prevCrossCouple->linkto(nextCross);
      nextCrossCouple = nextCross->link();
      prevCrossCouple = prevCross->link();
   }
   // now check for piercing edge cross points
   VPoint* pV = prevCross->prev();
   VPoint *nV = nextCross->next();
   VPoint* spV = prevCrossCouple->prev();
   VPoint *snV = nextCrossCouple->next();
   int oriP = orientation(spV->cp(), snV->cp(), pV->cp());
   int oriN = orientation(spV->cp(), snV->cp(), nV->cp());
   assert(0 != oriP);
   assert(0 != oriN);
   if (oriP != oriN)
   {
      // we have a piercing edge cross - so let's remove the redundant points
      prevCross->prev()->set_next(nextCross);
      nextCross->set_prev(prevCross->prev());
      if (prevCrossCouple->next() != nextCrossCouple)
      {
         if (pairedShape == prevCrossCouple->next())
            pairedShape = nextCrossCouple;
         delete (prevCrossCouple->next());
      }
      prevCrossCouple->prev()->set_next(nextCrossCouple);
      nextCrossCouple->set_prev(prevCrossCouple->prev());
      // delete here the removed crossing points
      delete prevCrossCouple;
      delete prevCross;
      delete this;
   }
   return nextCross;
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
      return (ord >= 0);
   else
      return (ord <= 0);
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
crosspoints array. The input point is assumed to be unique. Method is called by
Event::insertCrossPoint() only.
 * @param pnt the new cross point - assumed to be unique
 * @return the #CPoint that will be linked to its counterpart by the caller
 */
polycross::CPoint* polycross::polysegment::insertCrossPoint(const TP* pnt) {
   CPoint* cp = new CPoint(pnt);
   crosspoints.push_back(cp);
   return cp;
}

unsigned polycross::polysegment::normalize(const TP* p1, const TP* p2) {
   _lP = p1; _rP = p2;
   unsigned numcross = crosspoints.size();
   if (crosspoints.size() > 1) {
      SortLine functor(p1,p2);
      std::sort(crosspoints.begin(), crosspoints.end(), functor);
   }
   return numcross;
}

void polycross::polysegment::dump_points(polycross::VPoint*& vlist) {
   vlist = new VPoint(_lP, vlist);
   for (unsigned i = 0; i < crosspoints.size(); i++)
   {
      crosspoints[i]->linkage(vlist);
#ifdef BO2_DEBUG
      printf("( %i , %i )\n", crosspoints[i]->cp()->x(), crosspoints[i]->cp()->y());
#endif
   }
}

polycross::BPoint* polycross::polysegment::insertBindPoint(const TP* pnt)
{
   BPoint* cp = new BPoint(pnt);
   crosspoints.push_back(cp);
   return cp;
}


polycross::polysegment::~polysegment()
{
//   for (unsigned i = 0; i < crosspoints.size(); i++)
//      delete (crosspoints[i]);
}

//==============================================================================
// class segmentlist

polycross::segmentlist::segmentlist(const pointlist& plst, byte plyn) {
   _originalPL = &plst;
   unsigned plysize = plst.size();
   _segs.reserve(plysize);
   for (unsigned i = 0; i < plysize; i++)
      _segs.push_back(new polysegment(&(plst[i]),&(plst[(i+1)%plysize]),i, plyn));
}

polycross::BPoint* polycross::segmentlist::insertBindPoint(unsigned segno, const TP* point) {
    return _segs[segno]->insertBindPoint(point);
}

polycross::segmentlist::~segmentlist() {
   for (unsigned i = 0; i < _segs.size(); i++)
      delete _segs[i];
   _segs.clear();
}

unsigned polycross::segmentlist::normalize(const pointlist& plst) {
   unsigned numcross = 0;
   unsigned plysize = plst.size();
   for (unsigned i = 0; i < plysize; i++)
      numcross += _segs[i]->normalize(&(plst[i]),&(plst[(i+1)%plysize]));
   return numcross;
}

polycross::VPoint* polycross::segmentlist::dump_points()
{
   VPoint* vlist = NULL;
#ifdef BO2_DEBUG
      printf("---------------Crossing points found-----------\n");
#endif
   for (unsigned i = 0; i < _segs.size(); i++)
      _segs[i]->dump_points(vlist);
   polycross::VPoint* lastV = vlist;
   while (vlist->prev())
      vlist = vlist->prev();
   lastV->set_next(vlist);
   vlist->set_prev(lastV);
   return vlist;
}

//==============================================================================
// TEvent
void polycross::TEvent::checkIntersect(polysegment* above, polysegment* below,
                                    XQ& eventQ, const TP* iff)
{
   TP* rep = getIntersect(above, below, eventQ, iff);
   if (rep != NULL)
      delete rep;
}

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
 * @return the corssing point between segments if it exists. Otherwise - NULL
 */
TP* polycross::TEvent::getIntersect(polysegment* above, polysegment* below,
                                       XQ& eventQ, const TP* iff)
{
   TP* CrossPoint = NULL;
   // check that the cross candidates are not a sentinel segment
   // check for polygon edges belonging to the same polygon
   if ((0 == below->polyNo()) || (0 == above->polyNo()) ||
        (above->polyNo() == below->polyNo())) return NULL;
   // Now test for intersection point exsistence
   float lsign, rsign, rlmul;
   
   //check that both below endpoints are on the same side of above segment
   lsign = orientation(above->lP(), above->rP(), below->lP());
   rsign = orientation(above->lP(), above->rP(), below->rP());
   // do the case when both segments lie on the same line
   if ((lsign == 0) && (rsign == 0))
   {
      if ((iff == NULL) &&
           ((CrossPoint = oneLineSegments(above, below, eventQ.sweepline()))))
         insertCrossPoint(CrossPoint, above, below, eventQ, true);
      return CrossPoint;
   }
   rlmul = lsign*rsign;
   if      (0  < rlmul)  return NULL;// not crossing
   if (0 == rlmul)
   {
      // possibly touching segments
      CrossPoint = joiningSegments(above, below, lsign, rsign);
      if (NULL != CrossPoint)
      {
         if ((NULL == iff) || (*CrossPoint == *iff))
            insertCrossPoint(CrossPoint, above, below, eventQ);
         else
         {
            delete CrossPoint;CrossPoint = NULL;
         }
      }
      return CrossPoint;
   }

   //now check that both above endpoints are on the same side of below segment
   lsign = orientation(below->lP(), below->rP(), above->lP());
   rsign = orientation(below->lP(), below->rP(), above->rP());
   // lsign == rsign == 0 case here should not be possible
   if ((lsign == 0) && (rsign == 0))
      throw EXPTNpolyCross("Segments shouldn't coincide at this point");
   rlmul = lsign*rsign;
   if      (0  < rlmul) return NULL;
   if (0 == rlmul)
   {
      // possibly touching segments
      CrossPoint = joiningSegments(below, above, lsign, rsign);
      if (NULL != CrossPoint)
      {
         if ((NULL == iff) || (*CrossPoint == *iff))
            insertCrossPoint(CrossPoint, above, below, eventQ);
         else
         {
            delete CrossPoint;CrossPoint = NULL;
         }
      }
      return CrossPoint;
   }
   
   // at this point - the only possibility is that they intersect
   // so - create a cross event
   if (NULL == iff)
   {
      CrossPoint = getCross(above, below);
      insertCrossPoint(CrossPoint, above, below, eventQ);
   }
   return CrossPoint;
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
      else return new TP(*(below->lP()));
   }
   else if(0 == rps)
   {
      if (0 >= getLambda(above->lP(), above->rP(), below->rP())) return NULL;
      else return new TP(*(below->rP()));
   }
   assert(false);
}

TP* polycross::TEvent::oneLineSegments(polysegment* above, polysegment* below, YQ* sweepL)
{
   float lambdaL = getLambda(above->lP(), above->rP(), below->lP());
   float lambdaR = getLambda(above->lP(), above->rP(), below->rP());
   // coinciding or touching segments -> don't generate anything.
   // The post-process of begin/end/modify events in a single vertex should
   // take proper care about those cases
   if (0 == (lambdaL * lambdaR)) return NULL;
   bool swaped = false;
   // first of all cases when neither of the lines is enclosing the other one
   // here we are generating "hidden" cross point, right in the middle of their
   // coinsiding segment
   if ((lambdaL < 0) && (lambdaR > 0))
      return getMiddle(above->lP(), below->rP()); // return NULL;//
   if ((lambdaL > 0) && (lambdaR < 0))
      return getMiddle(below->lP(), above->rP()); // return NULL;
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
   // get the original polygon to which enclosed segment belongs to
   const pointlist* insideP = (1 == (swaped ?  above->polyNo() : below->polyNo())) ?
         sweepL->opl1() : sweepL->opl2();
   // ... and get the location of the inside segment in that sequence
   int numv = insideP->size();
   unsigned indxLP = (*(inside->lP()) == (*insideP)[inside->edge()]) ?
         inside->edge() : (inside->edge() + 1) % numv;
   unsigned indxRP = (*(inside->rP()) == (*insideP)[inside->edge()]) ?
         inside->edge() : (inside->edge() + 1) % numv;
   // we'll pickup the neighbouring point(s) of the inside segment and will
   // recalculate the lps/rps for them.
   bool indxpos = (indxLP == ((indxRP + 1) % numv));
//   bool indxpos = indxLP > indxRP;
   float lps = 0.0;
   float rps = 0.0;
   do
   {
      // make sure indexes are not the same
      if (indxLP == indxRP)
         throw EXPTNpolyCross("Can't locate properly the inside segment");

      // the code below just increments/decrements the indexes in the point sequence
      // they look so weird, to keep the indexes within [0:_numv-1] boundaries
      if (0 == lps)
      {
         if (indxpos) indxLP = (indxLP+1) % numv;
         else (0==indxLP) ? indxLP = numv-1 : indxLP--;
      }
      if (0 == rps)
      {
         if (!indxpos) indxRP = (indxRP+1) % numv;
         else (0==indxRP) ? indxRP = numv-1 : indxRP--;
      }
      // calculate lps/rps with the new points
      lps = orientation(outside->lP(), outside->rP(), &((*insideP)[indxLP]));
      rps = orientation(outside->lP(), outside->rP(), &((*insideP)[indxRP]));
   } while (0 == (lps * rps));
   // When segments are crossing (because of the direction of their neightbours),
   // then a crossing point is introduced in the midle of the internal segment
   if   (lps*rps > 0) return NULL;
   else return getMiddle(inside->lP(), inside->rP());
}

/**
 * Here a crossing point #_cp is calculated from the input segments. The
function assumes that the input two segments definitely cross each other.
It is called only from TEvent::checkIntersect() in case the check for crossing
segments is positive.\n
Crossing point is calculated from general line equations Ax+By+C=0. A care
is taken for division by 0 case when A or B coefficients are 0. It 
is important to note as well that the calculations must be done in floting
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
   // segments will coinside if    A1/A2 == B1/B2 == C1/C2
   // segments will be paraller if A1/A2 == B1/B2 != C1/C2
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
   return new TP((int)rint(X),(int)rint(Y));
}


TP* polycross::TEvent::getMiddle(const TP* p1, const TP* p2)
{
   int4b x = (p2->x() + p1->x()) / 2;
   int4b y = (p2->y() + p1->y()) / 2;
   return new TP(x,y);
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
   printf("**Cross point (%i,%i) inserted between segments below :\n", CP->x(), CP->y());
   BO_printseg(above)
   BO_printseg(below)
#endif
   if (!dontswap)
      eventQ.addCrossEvent(CP, above, below);
}

//==============================================================================
// TbEvent

polycross::TbEvent::TbEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
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

void polycross::TbEvent::sweep(XQ& eventQ, YQ& sweepline, ThreadList& threadl)
{
   // create the threads
   SegmentThread* athr = sweepline.beginThread(_aseg);
   SegmentThread* bthr = sweepline.beginThread(_bseg);
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread begin");
   threadl.push_back(_aseg->threadID());
   threadl.push_back(_bseg->threadID());
#ifdef BO2_DEBUG
   printf("Begin 2 threads :\n");
   BO_printseg(_aseg)
   BO_printseg(_bseg)
   sweepline.report();
#endif
   // in all cases check:
   checkIntersect(athr->threadAbove()->cseg(), _aseg, eventQ);
   checkIntersect(_bseg, bthr->threadBelow()->cseg(), eventQ);
   // check that the new threads are neighbours
   if (!((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr)))
   {
      // not neighbours - check the rest
      checkIntersect(bthr->threadAbove()->cseg(), _bseg, eventQ);
      checkIntersect(_aseg, athr->threadBelow()->cseg(), eventQ);
   }
   // check the case when left points are also crossing points
   // this is also the case when one of the input segments coincides with
   // an existing segment
   checkIntersect(athr->threadAbove()->cseg(), _bseg, eventQ, _bseg->lP());
   checkIntersect(_aseg, bthr->threadBelow()->cseg(), eventQ, _aseg->lP());
}


void polycross::TbEvent::sweep2bind(YQ& sweepline, BindCollection& bindColl)
{
   // create the threads
   SegmentThread* athr = sweepline.beginThread(_aseg);
   SegmentThread* bthr = sweepline.beginThread(_bseg);
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread begin - bind");
   // action is taken only for points in the second polygon
   if (1 == _aseg->polyNo() == _bseg->polyNo()) return;
   // assuming that the polygons are not crossing ...
   if (!((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr)))
      throw EXPTNpolyCross("Crossing input polygons in bind algo - begin");
   // if left point is above and the neighbour above is from polygon 1
   if ((_aseg->lP()->y() >= _aseg->rP()->y()) &&
        (1 == athr->threadAbove()->cseg()->polyNo()))
      bindColl.update_BL(athr->threadAbove()->cseg(), _aseg->edge(), _aseg->lP());
   // if the left point is below and the neighbour below is from polygon 1
   if ((_bseg->lP()->y() <= _bseg->rP()->y()) &&
        (1 == bthr->threadBelow()->cseg()->polyNo()))
      bindColl.update_BL(bthr->threadBelow()->cseg(), _bseg->edge(), _bseg->lP());
}

//==============================================================================
// TeEvent

polycross::TeEvent::TeEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
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

void polycross::TeEvent::sweep (XQ& eventQ, YQ& sweepline, ThreadList& threadl)
{
   threadl.push_back(_aseg->threadID());
   threadl.push_back(_bseg->threadID());
   SegmentThread* athr = sweepline.getThread(_aseg->threadID());
   SegmentThread* bthr = sweepline.getThread(_bseg->threadID());
   if ((athr->threadAbove() == bthr) || (bthr->threadBelow() == athr))
      throw EXPTNpolyCross("Invalid segment sort in thread end");
   if ((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr))
   {
      // if the segments that are about to be removed are neighbours -
      // check only their external neighbours
      checkIntersect(athr->threadAbove()->cseg(), bthr->threadBelow()->cseg(), eventQ);
   }
   else
   {
      // not neighbours - check both neighbours
      checkIntersect(athr->threadAbove()->cseg(), athr->threadBelow()->cseg(), eventQ);
      checkIntersect(bthr->threadAbove()->cseg(), bthr->threadBelow()->cseg(), eventQ);
   }
   // in all cases - check the case when right points are also crossing points
   checkIntersect(athr->threadAbove()->cseg(), _bseg, eventQ, _bseg->rP());
   checkIntersect(_aseg, bthr->threadBelow()->cseg(), eventQ, _aseg->rP());
   // remove segment threads from the sweep line
   sweepline.endThread(_aseg->threadID());
   sweepline.endThread(_bseg->threadID());
#ifdef BO2_DEBUG
   printf("End 2 threads\n");
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
   if (2 == _aseg->polyNo() == _bseg->polyNo())
   {
      // assuming that the polygons are not crossing ...
      if (!((athr->threadBelow() == bthr) && (bthr->threadAbove() == athr)))
         throw EXPTNpolyCross("Crossing input polygons in bind algo - end");
      // if right point is above and the neighbour above is from polygon 1
      if ((_aseg->lP()->y() <= _aseg->rP()->y()) &&
            (1 == athr->threadAbove()->cseg()->polyNo()))
         bindColl.update_BL(athr->threadAbove()->cseg(), _aseg->edge(), _aseg->rP());
      // if the right point is below and the neighbour below is from polygon 1
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

polycross::TmEvent::TmEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
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

void polycross::TmEvent::sweep (XQ& eventQ, YQ& sweepline, ThreadList& threadl)
{
#ifdef BO2_DEBUG
   printf("Modify thread\n");
   BO_printseg(_aseg)
   BO_printseg(_bseg)
#endif
   if (0 == _aseg->threadID())
      EXPTNpolyCross("Sorted segment expected here");
   SegmentThread* thr = sweepline.modifyThread(_aseg->threadID(), _bseg);

   // check for intersections of the neighbours with the new segment
   // and whether or not threads should be swapped
   TP* CP;
   if((CP = getIntersect(thr->threadAbove()->cseg(), thr->cseg(), eventQ)))
   {
      if ((*CP) == *(_bseg->lP()))
      {
         polysegment* aseg = thr->threadAbove()->cseg();
         int ori1 = orientation(aseg->lP(), aseg->rP(), _aseg->lP());
         int ori2 = orientation(aseg->lP(), aseg->rP(), _bseg->rP());
         if ((ori1 == ori2) || (0 == ori1 * ori2))
            threadl.push_back(_bseg->threadID());
      }
      delete CP;
   }

   if ((CP = getIntersect(thr->cseg(), thr->threadBelow()->cseg(), eventQ)))
   {
      if ((*CP) == *(_bseg->lP()))
      {
         polysegment* bseg = thr->threadBelow()->cseg();
         int ori1 = orientation(bseg->lP(), bseg->rP(), _aseg->lP());
         int ori2 = orientation(bseg->lP(), bseg->rP(), _bseg->rP());
         if ((ori1 == ori2) || (0 == ori1 * ori2))
            threadl.push_back(_bseg->threadID());
      }
      delete CP;
   }
}

void polycross::TmEvent::sweep2bind(YQ& sweepline, BindCollection& bindColl)
{
   if (0 == _aseg->threadID())
      EXPTNpolyCross("Sorted segment expected here - bind");
   SegmentThread* thr = sweepline.modifyThread(_aseg->threadID(), _bseg);
   
   // action is taken only for points in the second polygon
   if (1 == _aseg->polyNo() == _bseg->polyNo()) return;
   
   // first for _aseg 
   // if right point is above and the neighbour above is from polygon 1
   if ((_aseg->lP()->y() <= _aseg->rP()->y()) &&
        (1 == thr->threadAbove()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadAbove()->cseg(), _aseg->edge(), _aseg->rP());
   // if the right point is below and the neighbour below is from polygon 1
   if ((_aseg->lP()->y() >= _aseg->rP()->y()) &&
        (1 == thr->threadBelow()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadBelow()->cseg(), _aseg->edge(), _aseg->rP());
   
   // then for _bseg
   // if left point is above and the neighbour above is from polygon 1
   if ((_bseg->lP()->y() >= _bseg->rP()->y()) &&
        (1 == thr->threadAbove()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadAbove()->cseg(), _bseg->edge(), _bseg->lP());
   // if the left point is below and the neighbour below is from polygon 1
   if ((_bseg->lP()->y() <= _bseg->rP()->y()) &&
        (1 == thr->threadBelow()->cseg()->polyNo()))
      bindColl.update_BL(thr->threadBelow()->cseg(), _bseg->edge(), _bseg->lP());
}

//==============================================================================
// TcEvent

void polycross::TcEvent::sweep(XQ& eventQ, YQ& sweepline, ThreadList& threadl)
{
   if ((threadl.end() != std::find(threadl.begin(), threadl.end(),_threadAbove)) ||
        (threadl.end() != std::find(threadl.begin(), threadl.end(),_threadBelow)))
   {
#ifdef BO2_DEBUG
      printf("SKIP swapping threads %i and % i \n", _threadAbove, _threadBelow);
#endif
      return;
   }
#ifdef BO2_DEBUG
      printf("SWAPPING threads % i and %i\n", _threadAbove, _threadBelow);
#endif
   SegmentThread* below = sweepline.swapThreads(_threadAbove, _threadBelow);
   SegmentThread* above = below->threadAbove();
#ifdef BO2_DEBUG
   sweepline.report();
#endif
   // check for intersections with the new neighbours
   checkIntersect(above->threadAbove()->cseg(), above->cseg(), eventQ);
   checkIntersect(below->cseg(), below->threadBelow()->cseg(), eventQ);
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


void polycross::EventVertex::sweep(YQ& sweepline, XQ& eventq)
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
            (*CE)->sweep(eventq, sweepline, _threadsSweeped);
            if (_crossE != cetype)
               nonCrossE.push_back(*CE);
         }
      }
   }
   // now - the post-check. All segments belonging to non cross events should be
   // checked for joining points beween each other
   for (Events::iterator CE1 = nonCrossE.begin(); CE1 != nonCrossE.end(); CE1++)
      for (Events::iterator CE2 = CE1; CE2 != nonCrossE.end(); CE2++)
         CheckBEM(eventq, **CE1, **CE2);
}

void polycross::EventVertex::CheckBEM(XQ& eventq, TEvent& thr1, TEvent& thr2)
{
   if (thr1.aseg()->polyNo() == thr2.aseg()->polyNo()) return;
   bool sa1sa2 = coinsidingSegm(thr1.evertex(), thr1.avertex(), thr2.avertex());
   bool sa1sb2 = coinsidingSegm(thr1.evertex(), thr1.avertex(), thr2.bvertex());
   bool sb1sa2 = coinsidingSegm(thr1.evertex(), thr1.bvertex(), thr2.avertex());
   bool sb1sb2 = coinsidingSegm(thr1.evertex(), thr1.bvertex(), thr2.bvertex());
   if ((sa1sa2) && (!(sa1sb2 || sb1sa2 ||sb1sb2)))
      thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.bseg(), eventq, true );
   else if ((sa1sb2) && (!(sa1sa2 || sb1sa2 ||sb1sb2)))
      thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.aseg(), eventq, true );
   else if ((sb1sa2) && (!(sa1sa2 || sa1sb2 || sb1sb2)))
      thr1.insertCrossPoint(thr1.evertex(), thr1.aseg(), thr2.bseg(), eventq, true );
   else if ((sb1sb2) && (!(sa1sa2 || sa1sb2 || sb1sa2)))
      thr1.insertCrossPoint(thr1.evertex(), thr1.aseg(), thr2.aseg(), eventq, true );
   else if (!(sa1sa2 || sa1sb2 || sb1sa2 || sb1sb2))
   {
      float lsign, rsign, rlmul;
      //check for threads, crossing in the event vertex point
      lsign = orientation(thr1.avertex(), thr1.bvertex(), thr2.avertex());
      rsign = orientation(thr1.avertex(), thr1.bvertex(), thr2.bvertex());
      rlmul = lsign * rsign;
      if      (0  < rlmul)  return;// not crossing
      assert(rlmul); // could not be touching or coinciding
      lsign = orientation(thr2.avertex(), thr2.bvertex(), thr1.avertex());
      rsign = orientation(thr2.avertex(), thr2.bvertex(), thr1.bvertex());
      rlmul = lsign * rsign;
      if      (0  < rlmul)  return;// not crossing
      assert(rlmul); // could not be touching or coinciding
      thr1.insertCrossPoint(thr1.evertex(), thr1.aseg(), thr2.aseg(), eventq, true );
      thr1.insertCrossPoint(thr1.evertex(), thr1.bseg(), thr2.bseg(), eventq, true );
   }
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

polycross::EventVertex::~EventVertex()
{
   for( int cetype = _endE; cetype <= _crossE; cetype++)
   {
      Events& simEvents = _events[cetype];
      while (!simEvents.empty())
      {
         TEvent* cevent = simEvents.front(); simEvents.pop_front();
         delete cevent;
      }
   }
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

//==============================================================================
// YQ
polycross:: YQ::YQ(DBbox& overlap, const segmentlist* seg1, const segmentlist* seg2)
{
   _osl1 = seg1;
   _osl2 = seg2;
   _blSent = new TP(overlap.p1().x()-1, overlap.p1().y()-1);
   _brSent = new TP(overlap.p2().x()+1, overlap.p1().y()-1);
   _tlSent = new TP(overlap.p1().x()-1, overlap.p2().y()+1);
   _trSent = new TP(overlap.p2().x()+1, overlap.p2().y()+1);
   _bottomSentinel = new BottomSentinel(new polysegment(_blSent, _brSent, -1, 0));
   _cthreads[-2] = _bottomSentinel;
   _topSentinel = new TopSentinel(new polysegment(_tlSent, _trSent, -1, 0));
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
      EXPTNpolyCross("Unsorted segment expected here");
   SegmentThread* above = _bottomSentinel;
   while (sCompare(startseg, above->cseg()) > 0)
      above = above->threadAbove();
   
   SegmentThread* below = above->threadBelow();
   SegmentThread* newthread = new SegmentThread(startseg, below, above);
   above->set_threadBelow(newthread);
   below->set_threadAbove(newthread);
   _cthreads[++_lastThreadID] = newthread;
   startseg->set_threadID(_lastThreadID);
   return newthread;
}

/**
 * Removes a segment thread from YQ and relinks its neighbours accordingly
 * @param threadID The ID of the thread to be removed
 * @return the thread that use to be below the one removed
 */
polycross::SegmentThread* polycross::YQ::endThread(unsigned threadID)
{
   // get the thread from the _cthreads
   Threads::iterator threadP = _cthreads.find(threadID);
   if (_cthreads.end() == threadP)
      EXPTNpolyCross("Segment thread not found in YQ - end");
   SegmentThread* thread = threadP->second;
   // relink above/below threads
   SegmentThread* nextT = thread->threadAbove();
   if (NULL == nextT)
      EXPTNpolyCross("Unable to remove the segment thread properly");
   nextT->set_threadBelow(thread->threadBelow());
   SegmentThread* prevT = thread->threadBelow();
   if (NULL == prevT)
      EXPTNpolyCross("Unable to remove the segment thread properly");
   prevT->set_threadAbove(thread->threadAbove());
   // erase it
   _cthreads.erase(threadP);
   delete(threadP->second);
   return prevT;
}

/**
 * Replaces the current segment in a segment thread
 * @param threadID the target segment thread
 * @param newsegment the new segment - to replace the existing one
 * @return the segment thread
 */
polycross::SegmentThread* polycross::YQ::modifyThread(unsigned threadID, polysegment* newsegment)
{
   // get the thread from the _cthreads
   Threads::iterator threadP = _cthreads.find(threadID);
   if (_cthreads.end() == threadP)
      EXPTNpolyCross("Segment thread not found in YQ - modify");
   SegmentThread* thread = threadP->second;
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
      EXPTNpolyCross("Segment thread not found in YQ - swap");
   if (_cthreads.end() == tiBelow)
      EXPTNpolyCross("Segment thread not found in YQ - swap");
   SegmentThread* tAbove = tiAbove->second;
   SegmentThread* tBelow = tiBelow->second;
   // relink above/below threads
   if (tAbove != tBelow->threadAbove())
      EXPTNpolyCross("Unable to swap the segment threads properly");
   if (tBelow != tAbove->threadBelow())
      EXPTNpolyCross("Unable to swap the segment threads properly");
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
      EXPTNpolyCross("Segment thread not found in YQ - get");
   return ti->second;
}

int polycross::YQ::sCompare(const polysegment* seg0, const polysegment* seg1)
{
   // Current point is always lp of seg0
   if (seg0 == seg1)
      EXPTNpolyCross("Different segments expected here");
   //
   int ori;
   ori = orientation(seg1->lP(), seg1->rP(), seg0->lP());
   if (ori != 0) return ori;
   // if it is a crossing point -> compare the slope
   ori = orientation(seg1->lP(), seg1->rP(), seg0->rP());
   if (ori != 0) return ori;
   // if it is still the same => we have coinciding segments
   
   // get the original pointlists for the comparing segments
   const pointlist* plist0 = (1 == seg0->polyNo()) ? opl1() : opl2();
   
   // ... and get the location of the inside segment in that sequence
   int numv = plist0->size();
   unsigned indxLP = (*(seg0->lP()) == (*plist0)[seg0->edge()]) ?
         seg0->edge() : (seg0->edge() + 1) % numv;
   unsigned indxRP = (*(seg0->rP()) == (*plist0)[seg0->edge()]) ?
         seg0->edge() : (seg0->edge() + 1) % numv;

   bool indxpos = (indxRP == ((indxLP + 1) % numv));
   // we are going to get the next point in the sequence and use it to 
   // determine the position that we need
   if (indxpos) indxRP = (indxRP+1) % numv;
   else (0==indxRP) ? indxRP = numv-1 : indxRP--;
   
   ori = orientation(seg1->lP(), seg1->rP(), &((*plist0)[indxRP]));
   assert(ori != 0);
   return ori;
   
/*   int order;
   // or like that - both ways it should work
   if       (*(seg0->lP()) != *(seg1->lP()))
      order = xyorder(seg0->lP(), seg1->lP());
   else if  (*(seg0->rP()) != *(seg1->rP()))
      order = xyorder(seg0->rP(), seg1->rP());
   else
      order = (seg0->edge() > seg1->edge()) ? 1 : -1;
   if (0 == order)
      EXPTNpolyCross("Segments undistinguishable");
   return order;*/
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
//   _osl1 = ;
//   _osl2 = ;
   _xqueue = avl_create(E_compare, NULL, NULL);
   createEvents(seg1,1);
   createEvents(seg2,2);
   _sweepline = new YQ(_overlap, &seg1, &seg2);
}

void polycross::XQ::createEvents(const segmentlist& seg, byte shapeID)
{
   unsigned s1, s2;
   TEvent*      evt;
   EventTypes   etype;
   for(s1 = 0, s2 = 1 ; s1 < seg.size(); s1++, ++s2 %= seg.size())
   {
      // determine the type of event from the neighboring segments
      // and create the thread event
      if (seg[s1]->lP() == seg[s2]->lP())
      {
         evt = new TbEvent(seg[s1], seg[s2], shapeID);
         etype = _beginE;
      }
      else if (seg[s1]->rP() == seg[s2]->rP())
      {
         evt = new TeEvent(seg[s1], seg[s2], shapeID);
         etype = _endE;
      }
      else
      {
         evt = new TmEvent(seg[s1], seg[s2], shapeID);
         etype = _modifyE;
      }
      // update overlapping box
      _overlap.overlap(*(seg[s1]->lP()));
      _overlap.overlap(*(seg[s1]->rP()));
      // now create the vertex with the event inside
      EventVertex* vrtx = new EventVertex(evt->evertex());
      // and try to stick it in the AVL tree
      void** retitem =  avl_probe(_xqueue,vrtx);
      if ((*retitem) != vrtx)
         // coinsiding vertexes from different polygons
         delete(vrtx);
      // finally add the event itself
      static_cast<EventVertex*>(*retitem)->addEvent(evt,etype);
   }
}

void polycross::XQ::addCrossEvent(const TP* CP, polysegment* aseg, polysegment* bseg)
{
   TcEvent* evt = new TcEvent(CP, aseg, bseg);
   // now create the vertex with the event inside
   EventVertex* vrtx = new EventVertex(evt->evertex());
   // and try to stick it in the AVL tree
   void** retitem =  avl_probe(_xqueue,vrtx);
   if ((*retitem) != vrtx)
   {
      // coinsiding vertexes from different polygons
      delete(vrtx);
   }
   static_cast<EventVertex*>(*retitem)->addEvent(evt, _crossE);
}

int polycross::XQ::E_compare( const void* v1, const void* v2, void*)
{
   //const TP*  p1 = (*(EventVertex*)v1)();
   //const TP*  p2 = (*(EventVertex*)v2)();
   const TP* p1 = (*static_cast<const EventVertex*>(v1))();
   const TP* p2 = (*static_cast<const EventVertex*>(v2))();
   return xyorder(p1,p2);
}

void polycross::XQ::sweep()
{
// see the comment around find parameter in the E_compare
#ifdef BO2_DEBUG
      printf("***************** START SWIPING ******************\n");
#endif
   EventVertex* evtlist;
   avl_traverser trav;
   while (NULL != avl_t_first(&trav,_xqueue))
   {
      evtlist = (EventVertex*)trav.avl_node->avl_data;
      evtlist->sweep(*_sweepline, *this);
      avl_delete(_xqueue,evtlist);
      delete evtlist;
      
   }
}

void polycross::XQ::sweep2bind(BindCollection& bindColl) {
#ifdef BO2_DEBUG
      printf("***************** START BIND SWIPING ******************\n");
#endif
   EventVertex* evtlist;
   avl_traverser trav;
   while (NULL != avl_t_first(&trav,_xqueue))
   {
      evtlist = (EventVertex*)trav.avl_node->avl_data;
      evtlist->sweep2bind(*_sweepline, bindColl);
      avl_delete(_xqueue,evtlist);
      delete evtlist;
   }
}

polycross::XQ::~XQ()
{
   avl_destroy(_xqueue, NULL);
   delete _sweepline;
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
   TP* poly0pnt = new TP((int)rint(X),(int)rint(Y));
   // if the point lies inside the segment
   if (getLambda(outseg->lP(), outseg->rP(), poly0pnt) >= 0)
   {
      // get the distance
      real distance = fabs(line1 / sqrt(denom));
      // and if it's shorter than already stored one, for this segment
      // or of no distance calculated fro this segment
      if (is_shorter(poly0seg, distance))
         // store the data
         _blist.push_back(new BindSegment(poly0seg, poly1seg, poly0pnt, poly1pnt, distance));
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

polycross::BindSegment* polycross::BindCollection::get_highest()
{
   BindList::iterator BI = _blist.begin();
   BindSegment* shseg = *BI;
   while (++BI != _blist.end())
      if ((*BI)->poly1pnt()->y() > shseg->poly1pnt()->y()) shseg = *BI;
   return shseg;
}

polycross::BindCollection::~BindCollection()
{
   for (BindList::iterator BL = _blist.begin(); BL != _blist.end(); BL++)
      delete (*BL);
   _blist.clear();
}
