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

#include <math.h>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include "logicop.h"
#include "tedat.h"
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"

#define BO_DEBUG
#define BO_print(a) printf("Event %s :polygon %i, segment %i, lP (%i,%i), rP (%i,%i)  \n" , \
#a, _seg->polyNo, _seg->edge , _seg->lP->x(), _seg->lP->y(), _seg->rP->x(),_seg->rP->y());

#define BO_above_below \
if (NULL != _seg->above)\
   printf("---above: %i(%i)\n", _seg->above->edge, _seg->above->polyNo);\
else\
   printf("---above: NULL\n");\
if (NULL != _seg->below)\
   printf("---below: %i(%i)\n", _seg->below->edge, _seg->below->polyNo);\
else\
   printf("---below: NULL\n");
//-----------------------------------------------------------------------------
// The declaratoin of the avl related functions. They are declared originally
// in avl.h and redeclared here in C++ manner with extern "C" clause
extern "C" {
   avl_table* avl_create (avl_comparison_func *, const void *, libavl_allocator *);
   void       avl_destroy (struct avl_table *, avl_item_func *);
   void*      avl_t_next (avl_traverser *);
   void*      avl_t_prev (avl_traverser *);
   void*      avl_delete (avl_table *, const void *);
   void*      avl_t_find (avl_traverser *, avl_table *, void *);
   void*      avl_t_insert (avl_traverser *, avl_table *, void *);
   void*      avl_t_first (avl_traverser *, avl_table *);
   void*      avl_t_replace (avl_traverser *, void *);
}   
   const logicop::Event*  logicop::SweepLine::_curE = NULL;

//==============================================================================
/*! Determines the lexicographical order of two points comparing X first. 
   Returns:
   - +1 -> p1  > p2
   - -1 -> p1  < p2
   -  0 -> p1 == p2
*/
int logicop::xyorder(const TP* p1, const TP* p2) {
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

inline float logicop::isLeft(const TP* p0, const TP* p1, const TP* p2) {
   return ((float)p1->x() - (float)p0->x()) * ((float)p2->y() - (float)p0->y()) - 
          ((float)p2->x() - (float)p0->x()) * ((float)p1->y() - (float)p0->y());
}

//-----------------------------------------------------------------------------
// class VPoint
//-----------------------------------------------------------------------------
/*! This is the VPoint own constuctor. It links the new points in a VPoint
list at the moment of construction*/
logicop::VPoint::VPoint(const TP* point, VPoint* prev)  : 
                                                   _cp(point), _prev(prev) {
   if (_prev) _prev->_next = this;
}
      
/*! This method implements crossing number method to determine whether #_cp
is inside plist. The essence of the method is:\n
Create a ray in any direction starting from the #_cp point. In this 
implementation the ray is parallel to the X axis and directed in the same way.
Count the number of times that ray crosses the polygon boundary edges. If this
number is odd or 1 - means that the point is inside, if it is even or 0 - point
is outside.
 */
bool logicop::VPoint::inside(const pointlist& plist)
{
   TP p0, p1;
   byte cc = 0;
   unsigned size = plist.size();
   for (unsigned i = 0; i < size ; i++)
   {
      p0 = plist[i]; p1 = plist[(i+1) % size];
      if (((p0.y() <= _cp->y()) && (p1.y() >  _cp->y())) 
        ||((p0.y() >  _cp->y()) && (p1.y() <= _cp->y())) )
      {
         float tngns = (float) (_cp->y() - p0.y())/(p1.y() - p0.y());
         if (_cp->x() <= p0.x() + tngns * (p1.x() - p0.x()))
            cc++;
      }
   }
   return (cc & 0x01) ? true : false;
}

/*! Returns simply #_next or #_prev VPoint depending of the direction. Second
parameter is not used here, but just in the CPoint::follower()*/
logicop::VPoint* logicop::VPoint::follower(bool& direction, bool) {
   return direction ? _next : _prev;
}   

//-----------------------------------------------------------------------------
// class CPoint
//-----------------------------------------------------------------------------
/*! The actual #CPoint objects are created during the Bentley-Ottmann algorithm
when a crossing point is digitized (in plysegment::insertcrosspoint() method)
At that time this object can not be linked properly in a #VPoint list at least
because plysegments are not normalized yet. That is why they must be re-linked
afterwards during plysegment::dump_points(). This method is doing the job*/
void logicop::CPoint::linkage(VPoint*& prev) {
   _prev = prev;
   if (prev) prev->set_next(this);
   prev = this;
}

/*! When a new polygons are generated by the logic methods a reliable way to
switch between the two input #VPoint lists is required in order to utilize the
traversal. This method return the following point of the currently generated 
polygon. That point is always from the "other" polygon, but never the 
corresponding crossing point there. Instead its successor or predcessor, 
depending on the direction variable. Thus switching between the two source 
#VPoint lists is executed here and is hidden from the actual traversers. The 
method also increments the #_visited field of this object as well as of the linked
object. It changes the direction variable if modify parameter is true. This is 
required by some of the logic operations*/
logicop::VPoint* logicop::CPoint::follower(bool& direction, bool modify) {
   if (modify) direction = !direction;
   logicop::VPoint* flw = direction ? _link->_next : _link->_prev;
   _visited++; _link->_visited++;
   return flw;
}

logicop::VPoint* logicop::BPoint::follower(bool& direction, bool modify) {
   if (modify) {
      direction = !direction;
      return direction ? _next : _prev;
   }   
   else return _link;
}

//-----------------------------------------------------------------------------
// class SortLine
//-----------------------------------------------------------------------------
/*! Returns true if the xyorder of the input points coincides with _direction
field and false otherwise*/
bool logicop::SortLine::operator() (CPoint* cp1, CPoint* cp2) {
   if (direction == xyorder(cp1->cp(), cp2->cp()))  return true;
   else                                             return false;
}

//-----------------------------------------------------------------------------
// class plysegment 
//-----------------------------------------------------------------------------
/*!Creates a new polygon segment. Sorts p1 and p2 using xyorder() function and
assigns the corresponding pointers to #lP and #rP. Initialises the #above/#below
pointers to NULL and stores the polygon ID and the edge number*/
logicop::plysegment::plysegment(const TP* p1, const TP* p2, int num, char plyn) {
   if (xyorder(p1, p2) < 0) { lP = p1; rP = p2; }
   else                     { lP = p2; rP = p1; }
   above = below = NULL;
   edge = num;
   polyNo = plyn;
}
/*! The method creates a new #CPoint object from the input pnt and adds it to the 
crosspoints array. The input point is assumed to be unique. Method is called by
Event::checkNupdate() only, and returns the new #CPoint that will be linked to its
counterpart by the caller*/
logicop::CPoint* logicop::plysegment::insertcrosspoint(const TP* pnt) {
   CPoint* cp = new CPoint(pnt);
   crosspoints.push_back(cp);
   return cp;
}

logicop::BPoint* logicop::plysegment::insertbindpoint(const TP* pnt) {
   BPoint* cp = new BPoint(pnt);
   crosspoints.push_back(cp);
   return cp;
}

/*! This method sorts the array of the crossing points using the functor class 
#SortLine. It is called by segmentlist::normalize(). See the description there.
The method also reinitializes #lP=p1 and #rP=p2. In result the segment is 
normalized i.e. contains the proper vertex data in a proper order for 
traversing*/
unsigned logicop::plysegment::normalize(const TP* p1, const TP* p2) {
   lP = p1; rP = p2;
   unsigned numcross = crosspoints.size();
   if (crosspoints.size() > 1) {
      SortLine functor(p1,p2);
	  std::sort(crosspoints.begin(), crosspoints.end(), functor);
   }
   return numcross;
}

/*!Updates the list pointed by vlist variable with all points that this segment
posses. It should be clear that #rP is not included in this list because it
will be #lP of the next segment. All crossing points are dumped however. Method
is called by segmentlist::dump_points() but only after the segment has been 
normalized*/
void logicop::plysegment::dump_points(VPoint*& vlist) {
   vlist = new VPoint(lP, vlist);
   for (unsigned i = 0; i < crosspoints.size(); i++) 
      crosspoints[i]->linkage(vlist);
}

//-----------------------------------------------------------------------------
// class segmentlist 
//-----------------------------------------------------------------------------
/*!Creates a sequence of segments from the plist input and adds them to the 
#_segs collection.*/
logicop::segmentlist::segmentlist(const pointlist& plst, byte plyn) {
   unsigned plysize = plst.size();
   _segs.reserve(plysize);
   for (unsigned i = 0; i < plysize; i++)
      _segs.push_back(new plysegment(&(plst[i]),&(plst[(i+1)%plysize]),i, plyn));
}

logicop::BPoint* logicop::segmentlist::insertbindpoint(unsigned segno, const TP* point) {
   return _segs[segno]->insertbindpoint(point);
}

/*!Traveses the entire #_segs collection and deletes all segments. Finally the
#_segs vector is cleared-up as well*/
logicop::segmentlist::~segmentlist() {
   for (unsigned i = 0; i < _segs.size(); i++)
      delete _segs[i];
   _segs.clear();
}

/*!Traveses the entire #_segs collection in order to sort the crossing points
for every segment. One pretty important point here:-\n This method is called 
after Bentley-Ottmann algorithm is concluded and all crossing points are 
collected and inserted in their respective segments. The thing however is that
these points are inserted in the order that they are discovered and like this
they are not of much use for the consequent polygon generation routines. The 
order of the crossing points for every segment must be normalized. What does
this mean:- Every polygon entering this alghorithm is normalised - means that
the vertex sequence is winded counterclockwise. The new vertices (crossing 
points) must fit in this sequence smoothly, i.e. without any "tango" style
steps (forward-backward). The problem however is that we can not rely on the 
segment endpoints lP and rP to do the job properly because: alghorithm 
replaces the pointer of the original lP vertex with the pointer to the crossing
point, and second, lP and rP are actually sorted and they does not represent
the original direction of the segment in the polygon. So to resolve this problem
we need the original polygon vertex list (plst input) and using the edge 
correspondance, to call plysegment::normalize() with the original polygon 
vertices in the original order*/
unsigned logicop::segmentlist::normalize(const pointlist& plst) {
   unsigned numcross = 0;
   unsigned plysize = plst.size();
   for (unsigned i = 0; i < plysize; i++)
      numcross += _segs[i]->normalize(&(plst[i]),&(plst[(i+1)%plysize]));
   return numcross;   
}

/*! This method returns properly sorted dual linked list of all vertices 
(including crossing ones) of this segment collection. The method should be 
called after normalize(). The list created here is used as a source data when
the new polygons are generated. All logic operations are using this data. This
is effectively the input polygon vertices and the crossing points lined-up
conterclockwise*/
logicop::VPoint* logicop::segmentlist::dump_points() {
   logicop::VPoint* vlist = NULL;
   for (unsigned i = 0; i < _segs.size(); i++)
      _segs[i]->dump_points(vlist);
   logicop::VPoint* lastV = vlist;
   VPoint* centinel = NULL;
   while (vlist->prev())  {
      if (-1 == vlist->visited()) centinel = vlist;
      vlist = vlist->prev();
   }   
   lastV->set_next(vlist);
   vlist->set_prev(lastV);
   if (NULL != centinel) {
      VPoint* vwork = centinel;
      do {
         if (-1 == vwork->visited()) {
            //here visited == 0 means only that the object is Cpoint.
            VPoint* tbdel = NULL;
            if ((*vwork->cp()) == (*vwork->prev()->cp())) {
               tbdel = vwork->prev();
               vwork->set_prev(vwork->prev()->prev());
               vwork->prev()->set_next(vwork);
            }
            else if ((*vwork->cp()) == (*vwork->next()->cp())) {
               tbdel = vwork->next();
               vwork->set_next(vwork->next()->next());
               vwork->next()->set_prev(vwork);
            }
            vwork = vwork->next();
            if (tbdel) delete tbdel;   
         }
         else vwork = vwork->next();
      } while (centinel != vwork);
   }   
   return vlist;
}
      
//-----------------------------------------------------------------------------
// class Event
//-----------------------------------------------------------------------------
/*! This is the only non-virtual method of this class. After a crossing point has 
been discovered and a CEvent created this method checks whether this event is not
already in the event queue. See the description of EventQueue::E_compare(). If the 
event is not already there it is added, otherwise, it is deleted.\n
The other important thing is that if the event is unique, the cross point is added 
to each of the input segments after what they are linked to each other as well. 
This data will be used later, outside the Bentley-Ottmann algorithm to form the 
new polygons.\n
I am stil not sure that this class is the best place for this method - just see
the number of the input parameters*/
void logicop::Event::checkNupdate(avl_table* EQ, plysegment* above, 
                                  plysegment* below, CEvent* evt, bool check)
{
   avl_traverser trav;
   bool insert;
   if (check)
   {
      void* retitem =  avl_t_find(&trav, EQ, evt);
      insert = (NULL == retitem);
   }
   else insert = true;
   if (insert)
   {
      void* retitem = avl_t_insert(&trav,EQ,evt);
      assert(retitem == evt);
      if  (!evt->swaponly())
      {
         CPoint* cpsegA = above->insertcrosspoint(evt->evertex());
         CPoint* cpsegB = below->insertcrosspoint(evt->evertex());
         cpsegA->linkto(cpsegB);
         cpsegB->linkto(cpsegA);
      }
   }
   else delete evt;
}

//-----------------------------------------------------------------------------
// class LEvent
//-----------------------------------------------------------------------------
/*! Implements Bentley-Ottmann operations for left point event. A new segment
is added to the sweep line using SweepLine::add() after what the segment is 
checked for crossing points with its neighbours*/
void logicop::LEvent::swipe4cross(SweepLine& SL, avl_table* EQ) {
   SL.add(_seg);
   CEvent* evt;
#ifdef BO_DEBUG
BO_print(ENTER)
BO_above_below
#endif
   if ((evt = SL.intersect(_seg->above, _seg))) 
      checkNupdate(EQ, _seg->above, _seg, evt,false);
   if ((evt = SL.intersect(_seg, _seg->below)))
      checkNupdate(EQ, _seg, _seg->below, evt,false);
}

void logicop::LEvent::swipe4bind(SweepLine& SL, BindCollection& BC) {
   SL.add(_seg);
   // action is taken only for points in the second polygon
   if (0 == _seg->polyNo) return;
   // if left point is above and the neighbour above is from polygon 0
   if ((_seg->lP->y() >= _seg->rP->y()) && (NULL != _seg->above) && (0 == _seg->above->polyNo)) {
      //get the binding point
      BC.update_BL(_seg->above, this);
   }   
   // if the left point is below and the neighbour below is from polygon 0   
   if ((_seg->lP->y() <= _seg->rP->y()) && (NULL != _seg->below) && (0 == _seg->below->polyNo)) {
      //get the binding point
      BC.update_BL(_seg->below, this);
   }

}

//-----------------------------------------------------------------------------
// class REvent
//-----------------------------------------------------------------------------
/*! Implements Bentley-Ottmann operations for right point event. Removes the 
segment from the SweepLine, after checking its above/below neightbours for 
crossing. */
void logicop::REvent::swipe4cross(SweepLine& SL, avl_table* EQ) {
   CEvent* evt;
#ifdef BO_DEBUG
BO_print(EXIT)
BO_above_below
#endif
   if ((evt = SL.intersect(_seg->above, _seg->below))) 
      checkNupdate(EQ, _seg->above, _seg->below, evt);
   SL.remove(_seg);
}

void logicop::REvent::swipe4bind(SweepLine& SL, BindCollection& BC) {
   // action is taken only for points in the second polygon
   if (1 == _seg->polyNo) {
      // if right point is above and the neighbour above is from polygon 0
      if ((_seg->lP->y() <= _seg->rP->y()) && (NULL != _seg->above) && (0 == _seg->above->polyNo)) {
         //get the binding point
         BC.update_BL(_seg->above, this);
      }   
      // if the right point is below and the neighbour below is from polygon 0   
      if ((_seg->lP->y() >= _seg->rP->y()) && (NULL != _seg->below) && (0 == _seg->below->polyNo)) {
         //get the binding point
         BC.update_BL(_seg->below, this);
      }
   }   
   SL.remove(_seg);
}

//-----------------------------------------------------------------------------
// class CEvent
//-----------------------------------------------------------------------------
/*! Here a crossing point #_cp is calculated from the input segments. The 
constructor assumes that the input two segments definitely cross each other.
It is called only from SweepLine::intersect() only if the check for crossing
segments is positive.\n
Crossing point is calculated from general line equations Ax+By+C=0. A care
is taken for division by 0 case when A or B coefficients are 0. It 
is important to note as well that the calculations must be done in floting
point expressions, otherwise the hell might brake loose.
*/
logicop::CEvent::CEvent(plysegment* above, plysegment* below) :
      _above(above), _below(below), _swaponly(false)
{
   real A1 = _above->rP->y() - _above->lP->y();
   real A2 = _below->rP->y() - _below->lP->y();
   real B1 = _above->lP->x() - _above->rP->x();
   real B2 = _below->lP->x() - _below->rP->x();
   real C1 = -(A1*_above->lP->x() + B1*_above->lP->y());
   real C2 = -(A2*_below->lP->x() + B2*_below->lP->y());
   assert((A1 != 0) || (A2 != 0));
   assert((B1 != 0) || (B2 != 0));
   //SGREM
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
   else assert(0);   
   _cp = new TP((int)rint(X),(int)rint(Y));
}

logicop::CEvent::CEvent(plysegment* above, plysegment* below, const TP* cp, bool swo) :
      _above(above), _below(below), _swaponly(swo), _cp(new TP(*cp))
{
}


/*! Implements Bentley-Ottmann operations for cross point event. Swaps the 
above/below segments first calling SweepLine::swap() and then checks the 
segment pair for new crossing points with their new neightbours*/
void logicop::CEvent::swipe4cross(SweepLine& SL, avl_table* EQ) {
   // swap the place of the segments, unless the cross points
   // is an edge of a certain segment
#ifdef BO_DEBUG
printf("Event CROSS: Segment above %i(%i); Segment below %i(%i), point (%i,%i)\n",
       _above->edge, _above->polyNo, _below->edge, _below->polyNo, _cp->x(), _cp->y());
#endif
   SL.swap(_above, _below);
   CEvent* evt;
#ifdef BO_DEBUG
printf(" Segments above/below SWAPPED\n");
#endif
   // check for crossing points between the new neighbours after the swap
   if ((evt = SL.intersect(_above->above, _above)))
      checkNupdate(EQ, _above->above, _above, evt);
   if ((evt = SL.intersect(_below, _below->below)))
      checkNupdate(EQ, _below, _below->below, evt);
}

//-----------------------------------------------------------------------------
// class EventQueue
//-----------------------------------------------------------------------------
/*! Create the initial event queue from the input segment lists of the polygons
For each segment the constructor creates two events - LEvent for the leftmost
point of the segment and REvent for the rightmost one. The placeholder of the
events is the AVL tree #_equeue*/
logicop::EventQueue::EventQueue( const segmentlist& seg1, const segmentlist& seg2 ) {
   _equeue = avl_create(E_compare, NULL, NULL);
   // Initialize event queue with edge segment endpoints
   avl_traverser trav;
	unsigned cnt;
   for ( cnt=0; cnt < seg1.size(); cnt++) {
      Event* evt = new LEvent(seg1[cnt]);
      void* retitem =  avl_t_insert(&trav,_equeue,evt);
      assert(evt == retitem);
      evt = new REvent(seg1[cnt]);
      retitem =  avl_t_insert(&trav,_equeue,evt);
      assert(evt == retitem);
   }
   for ( cnt=0; cnt < seg2.size(); cnt++) {
      Event* evt = new LEvent(seg2[cnt]);
      void* retitem =  avl_t_insert(&trav,_equeue,evt);
      assert(evt == retitem);
      evt = new REvent(seg2[cnt]);
      retitem =  avl_t_insert(&trav,_equeue,evt);
      assert(evt == retitem);
   }
}

/*!This method is actually executing Bentley-Ottman algorithm. It traverses 
the event queue and calls swipe4cross methods of every Event. It should be noted
that the _equeue is dynamically updated with the crossing events by the same
swipe4cross methods. \n The method is sensible to the updates in the tree in a wierd
way. Program will bomb out here if a new event has not been inserted properly.
Some kind of try-catch mechanism should be implemented.*/
void logicop::EventQueue::swipe4cross(SweepLine& SL) {
// see the comment around find parameter in the E_compare
   Event* evt;
   avl_traverser trav;
   avl_t_first(&trav,_equeue);
   while (trav.avl_node != NULL) {
      evt = (Event*)trav.avl_node->avl_data;
      SL.set_curE(evt);
      evt->swipe4cross(SL, _equeue);
      avl_t_next(&trav);
   }
}

void logicop::EventQueue::swipe4bind(SweepLine& SL, BindCollection& BC) {
   Event* evt;
   avl_traverser trav;
   avl_t_first(&trav,_equeue);
   while (trav.avl_node != NULL) {
      evt = (Event*)trav.avl_node->avl_data;
      SL.set_curE(evt);
      evt->swipe4bind(SL, BC);
      avl_t_next(&trav);
   }   
}

/*! Some important notes here about using avl_tree \n
AVL library that is used will not insert an item that is the same as an item 
already in the tree. Instead it will replace the latter one. In our case however
we have a plenty of events that share the same point for example every polygon 
vertex is the endpoint of two neighboring segments. That is why if xyorder 
function reports equal points, we have to apply additional criterias:
   - first compare the points using xyorder() function
   - if they are equal, sort them by the polygon index
   - if they belong to the same polygon, sort them by edge index
   - if they are still the same return 0 - i.e. equal points

Two things should be noted here.
The only type of points that are supposed to be reported equal are crossing points
The reason is that the algorihm leaks here in a way that a crossing point of two 
segments can be calculated twice. This is the reason why, when a crossing point 
is identified (by theory in the exit event of the segment) we need to check and 
ensure that such a crossing point event does not exists already in the event 
queue. Here this check is done for every crossing point because of simple 
program structural reasons, although once again it should be enough to check 
only in REvent::swipe4cross method\n
Second note is that a simple pointer comparison (v1 == v2) should tell us whether
or not these are the same points, thus we can avoid calling xyorder for this kind 
of cases. The reason of not doing this is that there is no guarantee that two 
different polygons will not share the same point - for whatever reason. Like this
the alghorithm will work bit slower though because the number of comparisons is 
bigger \n
And the last note about having an unique crossing points. It seems obvious, that
the polygons we are processing (non-selfcrossing, simple) can't have two pair
of segments crossing at the same point. It seems so obvious to me, because if 
four segments are crossing the same point then every two of them will cross each
other - i.e. both input polygons will be self-crossing */
int logicop::EventQueue::E_compare( const void* v1, const void* v2, void*)
{
   Event*  pe1 = (Event*)v1;
   Event*  pe2 = (Event*)v2;
   int result =  logicop::xyorder(pe1->evertex(),pe2->evertex());
   if (0 != result) return result;
   // if the event vertexes are equal, sort them by event priority
   // eporiority 0 is bigger than epriority 1
   if (pe1->epriority() != pe2->epriority())
      return (pe1->epriority() > pe2->epriority()) ? -1 : 1;
   // appears that we have same events with the same event vertexes,
   // so we'll try to sort them by their opposite vertices
   // but just in case that's not cross-event (0 priority)
   if (2 == pe1->epriority()) return 0; // the only case to return 0!
   result = logicop::xyorder(pe2->overtex(),pe1->overtex());
   if (0 != result) return result;
   // if still here -> these should be coinciding edges of different
   // polygons, so let's compare the polygon numbers
   if (pe1->polyNo() != pe2->polyNo())
      return (pe1->polyNo() > pe2->polyNo()) ? 1 : -1;
   else
      return 0;
   //
/*   // first compare the points
   int result =  logicop::xyorder(pe1->evertex(),pe2->evertex());
   if (result != 0) return result;
   // if they are equal, sort them by the polygon index
   if (pe1->polyNo() != pe2->polyNo())
      return (pe1->polyNo() > pe2->polyNo()) ? -1 : 1;
   // if they belong to the same polygon, sort them by edge number
   if (pe1->edgeNo() != pe2->edgeNo())
      return (pe1->edgeNo() > pe2->edgeNo()) ? -1 : 1;
   return 0;*/
}
 
/*! avl_destroy call this function for every item in the tree.*/
void logicop::EventQueue::destroy_event(void* item, void*) {
   Event* evt = (Event*) item;
   delete evt;
}   

/*! Destroys the event queue, calling avl_destroy */
logicop::EventQueue::~EventQueue() {
   avl_destroy(_equeue, destroy_event);
}

//-----------------------------------------------------------------------------
// class SweepLine
//-----------------------------------------------------------------------------
/*! The only valuable thing done during construction is the initialisation of the 
AVL tree */ 
logicop::SweepLine::SweepLine(const pointlist& plist0, const pointlist& plist1) :
      _plist0(plist0), _plist1(plist1)
{
    _tree = avl_create(compare_seg, &_curE, NULL);
}

/*!The AVL tree is detroyed here. The segment members are not destroyed here, they 
will be used later to gather the cross points*/
logicop::SweepLine::~SweepLine() {
    avl_destroy(_tree, NULL);
}    

/*! Checks and asserts that the segment is unique within the AVL tree. Updates 
above/below pointers of the input seg as well as its new neighbours*/
void logicop::SweepLine::add(plysegment* seg) {
   avl_traverser trav;
   void* retitem =  avl_t_insert(&trav,_tree,seg);
   assert(retitem == seg);
   avl_traverser save_trav = trav;
   plysegment* nx = (plysegment*)avl_t_next(&save_trav);
   plysegment* np = (plysegment*)avl_t_prev(&trav);
   if (NULL != nx) { seg->above = nx; nx->below = seg; }
   if (NULL != np) { seg->below = np; np->above = seg; }
}
/*! Before the actual removing, the method fixes the above/below pointers
of the seg neighbours, so that they point to each other instead of
the seg. \n It has to be noted also that neither this method nor the AVL tree delete
the actual segment seg. It will be deleted later, when EventQueue is destroyed*/
void logicop::SweepLine::remove(plysegment* seg) {
   avl_traverser trav;
   void* retitem = avl_t_find(&trav,_tree,seg);
   assert(retitem);
   //if (NULL == retitem) return;
   avl_traverser save_trav = trav;
   plysegment* nx = (plysegment*)avl_t_next(&save_trav);
   if (NULL != nx) nx->below = seg->below;
   plysegment* np = (plysegment*)avl_t_prev(&trav);
   if (NULL != np) np->above = seg->above;
   avl_delete(_tree,seg);
}
/*!
The method is using isLeft() function to do the job. Segments that belong to 
the same polygon are not checked and method returns NULL. The same result is 
obtained if one of the input segmens is NULL. \n
In case segments intersect, the method creates and returns a new CEvent.*/
logicop::CEvent* logicop::SweepLine::intersect(plysegment* above, plysegment* below) {
   // check both segments exists
   if ((NULL == above) || (NULL == below)) return NULL;
   // check for polygon edges belonging to the same polygon
   if (above->polyNo == below->polyNo)  return NULL;
   // Now test for intersection point exsistence
   float lsign, rsign, rlmul;
   lsign = isLeft(above->lP, above->rP, below->lP);
   rsign = isLeft(above->lP, above->rP, below->rP);
   //check that both s2 endpoints are on the same side of s1
   rlmul = lsign*rsign;
   if      (0  < rlmul)  return NULL;
   else if (0 == rlmul)
   {
      CEvent* gev = coinsideCheck(above, below, lsign, rsign);
      if (NULL != gev) return gev;
   }
   lsign = isLeft(below->lP, below->rP, above->lP);
   rsign = isLeft(below->lP, below->rP, above->rP);
   //check that both s1 endpoints are on the same side of s2
   rlmul = lsign*rsign;
   if      (0  < rlmul) return NULL;
   else if (0 == rlmul) return coinsideCheck(below, above, lsign, rsign);
   // at this point - the only possibility is that they intersect
   // so - create a cross event 
   return new CEvent(above,below);
}

logicop::CEvent* logicop::SweepLine::coinsideCheck(plysegment* line,
      plysegment* cross, float lps, float rps)
{
   // first we are going to check that the points laying on the line are not
   // outside the segment
   // i.e. if neither of the entry points are lying on the segment line
   // then there is nothing to check further here, get out
   // getLambda return  : < 0  the point is outside the segment
   //                     = 0  the point coinsides with one of the
   //                          segment endpoints
   //                     > 0  the point is inside the segment
   float lambdaL = (0 == lps) ? getLambda(line->lP, line->rP, cross->lP) : 0;
   float lambdaR = (0 == rps) ? getLambda(line->lP, line->rP, cross->rP) : 0;
   // filter-out all cases when the segments have no common points
   if (((0 != lps) || (lambdaL < 0)) && ((0 != rps) || (lambdaR < 0))) return NULL;
   // filter-out the cases when both segments have exactly one common point
   // and it is an edge point of both segmets
   // Seems that they will bring us only troubles - so lets filter them out as well
   bool Ljoint = (0==lps) && (0 == lambdaL);
   bool Rjoint = (0==rps) && (0 == lambdaR);
   if ((!Ljoint && Rjoint) || (!Rjoint && Ljoint)) return NULL;
   // deal with the cases when the line have exactly one common point with cross,
   // i.e one of the edge points of cross lies on line
   if      ((0==lps) && (0!=rps))
      return new CEvent(line, cross,cross->lP, false);
   else if ((0!=lps) && (0==rps))
      return new CEvent(cross, line,cross->rP, false);

   // What remains here is the case when cross coincides fully or partially with
   // line.
   assert((0 == lps) && (0 == rps));
   // first of all cases when neither of the lines is enclosing the other one
   // here we are generating "hidden" cross point, right in the moddle of their
   // coinsiding segment
   if       ((lambdaL < 0) && (lambdaR > 0))
      return new CEvent(line, cross, getMiddle(line->lP, cross->rP), false);
   else if ((lambdaL > 0) && (lambdaR < 0))
      return new CEvent(line, cross, getMiddle(cross->lP, line->rP), false);
   else
   {
      // now the cases when one of the lines encloses fully the other one
      //get the _plist index of the points in segment cross
      const pointlist& _plist = (0 == cross->polyNo) ? _plist0 : _plist1;
      int numv = _plist.size();
      unsigned indxLP = (*(cross->lP) == _plist[cross->edge]) ? cross->edge : cross->edge + 1;
      unsigned indxRP = (*(cross->rP) == _plist[cross->edge]) ? cross->edge : cross->edge + 1;
      // make sure they are not the same
      assert(indxLP != indxRP);
      
      // we'll pickup the neighbour of the point(s) laying on the line and will
      // recalculate the lps/rps for them. 
      bool indxpos = indxLP > indxRP;
      do
      {
         // the code below just increments/decrements the indexes in the point sequence
         // they look so weird, to keep the indexes within [0:_numv-1] boundaries
         if (0 == lps)
            if (indxpos) indxLP = ++indxLP % numv;
            else (0==indxLP) ? indxLP = numv-1 : indxLP--;
         if (0 == rps)
            if (!indxpos) indxRP = ++indxRP % numv;
            else (0==indxRP) ? indxRP = numv-1 : indxRP--;
         // calculate lps/rps with the new points   
         lps = isLeft(line->lP, line->rP, &_plist[indxLP]);
         rps = isLeft(line->lP, line->rP, &_plist[indxRP]);
      } while (0 == (lps * rps));
      if (lps * rps > 0)
      {
         // no "hidden" cross point here, because the shapes touch each outher
         // outside/outside or inside/outside. Nevertheless we need to mark
         // this event because of the AVL segment tree in the Sweepline
         if      ((lambdaL > 0) && (lambdaR > 0))
            // in the middle of cross segment
            return new CEvent(cross, line,getMiddle(cross->lP, cross->rP), true);
         else if ((lambdaL < 0) && (lambdaR < 0))
            // in the middle of line segment
            return new CEvent(line, cross,getMiddle(line->lP, line->rP), true);
         assert(false);
      }
      else
      {
         // a "hidden" cross point has to be introduced ....
         if      ((lambdaL > 0) && (lambdaR > 0))
            // in the middle of cross segment
            return new CEvent(line, cross,getMiddle(cross->lP, cross->rP), false);
         else if ((lambdaL < 0) && (lambdaR < 0))
            // in the middle of line segment
            return new CEvent(cross, line,getMiddle(line->lP, line->rP), false);
         assert(false);
      }
   }
}

TP* logicop::SweepLine::getMiddle(const TP* p1, const TP* p2)
{
   int4b x = (p2->x() + p1->x()) / 2;
   int4b y = (p2->y() + p1->y()) / 2;
   return new TP(x,y);
}

float logicop::SweepLine::getLambda( const TP* p1, const TP* p2, const TP* p)
{
   float denomX = p2->x() - p->x();
   float denomY = p2->y() - p->y();
   float lambda;
   if      (0 != denomX) lambda = (p->x() - p1->x()) / denomX;
   else if (0 != denomY) lambda = (p->y() - p1->y()) / denomY;
   // point coincides with the lp vertex of the segment
   else lambda = 0;
   return lambda;
}

/*! It is clear, that segements has to be sorted vertically, but
it is far from obvious (to me) what does it means.  Function checks whether 
the input two segments belong to the same polygon and if so, it is calling 
yxorder() function for left points first. If they coincide, yxorder is called
 for the right points.\n
If segments belong to different polygons, then isLeft() function is called 
again for left points first and if they coincide - for the right points 
of the segements. \n
Function returns 1 if o1 is above o2 and -1 otherwise. Function must be 
declared as static, otherwise can not be used as a callback*/
int logicop::SweepLine::compare_seg(const void* o1, const void* o2, void* param) {
   Event** curE = static_cast<Event**>(param);
   const TP* curP = (*curE)->evertex();
   const plysegment* seg0 = (plysegment*)o1;
   const plysegment* seg1 = (plysegment*)o2;
   // first check that we are comparing one and the same segments,
   // and return equal immediately
   if ((seg0->polyNo == seg1->polyNo) && (seg0->edge == seg1->edge))
      return 0;
   // Then get the current Y of both segments using their line equations
   real dX0 = seg0->rP->x() - seg0->lP->x();
   real dX1 = seg1->rP->x() - seg1->lP->x();
   real dY0 = seg0->rP->y() - seg0->lP->y();
   real dY1 = seg1->rP->y() - seg1->lP->y();
   real Y0, Y1;
   if (dX0 != 0) 
      Y0 = (dY0/dX0)*(curP->x() - seg0->lP->x()) + seg0->lP->y();
   else Y0 = curP->y();
   if (dX1 != 0) 
      Y1 = (dY1/dX1)*(curP->x() - seg1->lP->x()) + seg1->lP->y();
   else Y1 = curP->y();
   // if we've got different Y's - great!    
   if (Y0 != Y1)  return (Y0 > Y1) ? 1 : -1;
   // Calculated Y coordinaes are the same, so lets the fun begin
   typedef enum{leftP, crossP, rightP} entry_PT;
   entry_PT seg0_PT, seg1_PT;
   // first of all get a clear idea what the crossP is to both 
   // segments - it is lP, rP, or somewhere in the middle
   if       ((*curP) == (*seg0->lP)) seg0_PT = leftP;
   else if ((*curP) == (*seg0->rP)) seg0_PT = rightP;
   else                             seg0_PT = crossP;
   if       ((*curP) == (*seg1->lP)) seg1_PT = leftP;
   else if ((*curP) == (*seg1->rP)) seg1_PT = rightP;
   else                             seg1_PT = crossP;
   // Having done this, check all the possibilities...
   int4b compare;
   if       ((leftP == seg0_PT) && (leftP == seg1_PT)) 
      // lP/lP -> compare rP/rP
      compare = yxorder(seg0->rP, seg1->rP);
   else if ((rightP == seg0_PT) && (rightP == seg1_PT))
      // rP/rP -> compare lP/lP
      compare = yxorder(seg0->lP, seg1->lP);
   else if ((leftP == seg0_PT) && (rightP == seg1_PT))
      // lP/rP -> compare rP/rP
      compare = yxorder(seg0->rP, seg1->rP);
   else if ((rightP == seg0_PT) && (leftP == seg1_PT))
      // rP/lP -> compare rP/rP
      compare = yxorder(seg0->rP, seg1->rP);
   else if ((crossP == seg0_PT) && (crossP == seg1_PT))
   {
      // two crossing points - here the fun is even bigger
      if       ((0 == dX0) && (0 != dX1))
         // if seg0 is vertical -> compare its lP with current point
         compare = yxorder(seg0->lP, curP);
      else if ((0 != dX0) && (0 == dX1))
         // if seg1 is vertical -> compare its lP with current point
         compare = yxorder(curP, seg1->lP);
      else
         // if none of them is vertical -> compare lP/lP
         compare = yxorder(seg0->lP, seg1->lP);
   }
   else if (((crossP == seg0_PT) && (leftP == seg1_PT)) ||
            ((leftP == seg0_PT) && (crossP == seg1_PT)))
      compare = yxorder(seg0->lP, seg1->lP);
   else if ((crossP == seg0_PT) && (rightP == seg1_PT))
   {
      if (0 == (*curE)->epriority())
         compare = yxorder(seg0->rP, curP);
      else if (2 == (*curE)->epriority())
      {
         CEvent* boza = static_cast<CEvent*>(*curE);
         if (((seg0 == boza->above()) || (seg1 == boza->above())) &&
               ((seg0 == boza->below()) || (seg1 == boza->below())))
            compare = yxorder(seg0->lP, curP);
         else
            compare = yxorder(seg0->rP, curP);
      }
      else assert(false);
/*      if (0 == (*curE)->epriority()) // REvent
         compare = yxorder(seg0->rP, curP);
      else if (2 == (*curE)->epriority()) //CEvent
         compare = yxorder(seg0->lP, curP);
      else
         assert(false);*/
   }
   else if ((rightP == seg0_PT) && (crossP == seg1_PT))
   {
      if (0 == (*curE)->epriority())
         compare = yxorder(curP, seg1->rP);
      else if (2 == (*curE)->epriority())
      {
         CEvent* boza = static_cast<CEvent*>(*curE);
         if (((seg0 == boza->above()) || (seg1 == boza->above())) &&
               ((seg0 == boza->below()) || (seg1 == boza->below())))
            compare = yxorder(curP, seg1->lP);
         else
            compare = yxorder(curP, seg1->rP);
      }
      else assert(false);
/*      
      if (0 == (*curE)->epriority())
         compare = yxorder(curP, seg1->rP);// REvent
      else if (2 == (*curE)->epriority())
         compare = yxorder(curP, seg1->lP);// CEvent
      else
         assert(false);*/
   }
   else assert(false);
   if (0 == compare) {
      // at this point it appears that the two segments are identical,
      // so sort them by polygon or segment number 
      assert((*(seg0->lP) == *(seg1->lP)) && (*(seg0->rP) == *(seg1->rP)));
      if (seg0->polyNo != seg1->polyNo) 
         compare = (seg0->polyNo > seg1->polyNo) ? 1 : -1;
      else {
         assert(seg0->edge != seg1->edge);
         compare = (seg0->edge > seg1->edge) ? 1 : -1;
      }
   }   
   return compare;      
}

/*! Used in the compare_seg() method when both input segments belong to the 
same polygon. Returns:
   - +1 when p1  > p2
   - -1 when p1  < p2
   -  0 when p1 == p2
*/
int logicop::SweepLine::yxorder(const TP* p1, const TP* p2) {
   // test y coord first
   if (p1->y() > p2->y()) return  1;
   if (p1->y() < p2->y()) return -1;
//   // and then x
   if (p1->x() > p2->x()) return  1;
   if (p1->x() < p2->x()) return -1;
   return 0;
}

/*!When an intersection point is rendered, the segments that it belongs to 
must be swapped according to the Bentley-Ottmann algorithm. This method performs
the swapping in three steps:
 - first reshaffle the above/below pointers of the input segments and their 
   neighbours. 
 - then find the input segements in the AVL tree and swap them
 - finally replace the left points of the segments with the crossing point, 
   otherwise the #_tree will be stuffed because of the swapped segments
   
I've tried to have some fun with the above/below pointers, but it almost turn
into a nightmare. So if you are not sure what happens, don't mess around*/
void logicop::SweepLine::swap(plysegment*& above, plysegment*& below) {
   // first  fix the pointers of the neighbor segments
   if (below->below) below->below->above = above;
   if (above->above) above->above->below = below;
   // now swap above pointers ...
   below->above = above->above; above->above = below;
   // ... and below pointers ...
   above->below = below->below; below->below = above;
   // .. and then swap the segments themselfs in the tree ...
   avl_traverser trava, travb;
   void* retitem;
   retitem = avl_t_find(&trava,_tree,above);
   assert(NULL != retitem);
   retitem = avl_t_find(&travb,_tree,below);
   assert(NULL != retitem);
   below = (plysegment*)avl_t_replace(&trava,below);
   above = (plysegment*)avl_t_replace(&travb,above);
}

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
   segmentlist _segl1(poly1,0);
   segmentlist _segl2(poly2,1);
   EventQueue* _eq = new EventQueue(_segl1, _segl2); // create the event queue
   SweepLine   _sl(poly1, poly2);
   _eq->swipe4cross(_sl);
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
   VPoint* centinel = _shape1;
   VPoint* looper = centinel;
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
   VPoint* centinel = NULL;
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
      VPoint* vpnt = centinel;
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
   VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = new pointlist();
         VPoint* pickup = collector;
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
   VPoint* centinel = NULL;
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
   VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = new pointlist();
         VPoint* pickup = collector;
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
   VPoint* centinel = NULL;
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
      VPoint* vpnt = centinel;
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
   VPoint* collector = centinel;
   bool direction = true; /*next*/
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = new pointlist();
         VPoint* pickup = collector;
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

      
logicop::VPoint* logicop::logic::getFirstOutside(const pointlist& plist, VPoint* init) {
   unsigned plysize = plist.size();
   unsigned count = 0;
   while (init->inside(plist)) {
      init = init->next();
      if (count++ > plysize) return NULL;
   }
   return init;
}   


pointlist* logicop::logic::hole2simple(const pointlist& outside, const pointlist& inside) {
   segmentlist _segl0(outside,0);
   segmentlist _segl1(inside,1);
   EventQueue* _eq = new EventQueue(_segl0, _segl1); // create the event queue
   SweepLine   _sl(outside,inside);
   BindCollection BC;
   _eq->swipe4bind(_sl, BC);
   BindSegment* sbc = BC.get_highest();
   //insert 2 crossing points and link them
   BPoint* cpsegA = _segl0.insertbindpoint(sbc->poly0seg(), sbc->poly0pnt());
   BPoint* cpsegB = _segl1.insertbindpoint(sbc->poly1seg(), sbc->poly1pnt());
   cpsegA->linkto(cpsegB);
   cpsegB->linkto(cpsegA);

   // normalize the segment lists
   _segl0.normalize(outside);
   _segl1.normalize(inside);
   // dump the new polygons in VList terms
   VPoint* outshape = _segl0.dump_points();
                      _segl1.dump_points();
   
   // traverse and form the resulting shape
   VPoint* centinel = outshape;
   pointlist *shgen = new pointlist();
   bool direction = true; /*next*/
   VPoint* pickup = centinel;
   VPoint* prev = centinel->prev();
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

void logicop::BindCollection::update_BL(plysegment* outseg, Event* evt) {
   unsigned poly0seg = outseg->edge;
   unsigned poly1seg = evt->edgeNo();
   const TP* poly1pnt = evt->evertex();
   real A = outseg->rP->y() - outseg->lP->y();
   real B = outseg->lP->x() - outseg->rP->x();
   real C = -(A*outseg->lP->x() + B*outseg->lP->y());
   assert((A != 0) || (B != 0));
   real line1 = A*poly1pnt->x() + B*poly1pnt->y() + C;
   real denom = A*A + B*B;
   real X = poly1pnt->x() - (A / denom) * line1;
   real Y = poly1pnt->y() - (B / denom) * line1;
   TP* poly0pnt = new TP((int)rint(X),(int)rint(Y));
   if (isLeft(outseg->lP, outseg->rP, poly0pnt) == 0) {
      real distance = fabs(line1 / sqrt(denom));
      if (is_shorter(poly0seg, distance))
         _blist.push_back(new BindSegment(poly0seg, poly1seg, poly0pnt, poly1pnt, distance));
      else delete poly0pnt;
   }
   else delete poly0pnt;
}

bool logicop::BindCollection::is_shorter(unsigned segno, real dist) {
   for (BindList::iterator BI = _blist.begin(); BI != _blist.end(); BI++)
      if ((*BI)->poly0seg() == segno)
         if ((*BI)->distance() > dist) {
            delete (*BI);
            _blist.erase(BI);
            return true;
         }
         else return false;
   return true;      
}

logicop::BindSegment* logicop::BindCollection::get_highest() {
   BindList::iterator BI = _blist.begin();
   logicop::BindSegment* shseg = *BI;
   while (++BI != _blist.end())
      if ((*BI)->poly1pnt()->y() > shseg->poly1pnt()->y()) shseg = *BI;
   return shseg;
}
