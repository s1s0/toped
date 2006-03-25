#include <math.h>
#include <algorithm>
#include "polycross.h"

#define BO2_DEBUG
#define BO_printseg(SEGM) printf("thread %i : segment %i, polygon %i, \
lP (%i,%i), rP (%i,%i)  \n" , SEGM->threadID(), SEGM->edge(), SEGM->polyNo() , \
SEGM->lP()->x(), SEGM->lP()->y(), SEGM->rP()->x(),SEGM->rP()->y());

//-----------------------------------------------------------------------------
// The declaratoin of the avl related functions. They are declared originally
// in avl.h and redeclared here in C++ manner with extern "C" clause
extern "C" {
   avl_table* avl_create (avl_comparison_func *, const void *, libavl_allocator *);
   void **avl_probe (struct avl_table *, void *);
//   void       avl_destroy (struct avl_table *, avl_item_func *);
//   void*      avl_t_next (avl_traverser *);
//   void*      avl_t_prev (avl_traverser *);
   void*      avl_delete (avl_table *, const void *);
//   void*      avl_t_find (avl_traverser *, avl_table *, void *);
//   void*      avl_t_insert (avl_traverser *, avl_table *, void *);
   void*      avl_t_first (avl_traverser *, avl_table *);
//   void*      avl_t_replace (avl_traverser *, void *);
}

//==============================================================================
/**
 * Determines the lexicographical order of two points comparing X first. 
 * @param p1 - first point for comparison
 * @param p2 - second point for comparison
 * @return
   - +1 -> p1  > p2
   - -1 -> p1  < p2
   -  0 -> p1 == p2
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
   float area = (p1->x() - p3->x()) * (p2->y() - p3->y()) -
         (p2->x() - p3->x()) * (p1->y() - p3->y());
   if (0 == area) return 0;
   else
      return (area > 0) ? 1 : -1;
}

float polycross::getLambda( const TP* p1, const TP* p2, const TP* p)
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

//==============================================================================
// VPoint
polycross::VPoint::VPoint(const TP* point, VPoint* prev) : _cp(point), _prev(prev)
{
   if (_prev) _prev->_next = this;
}

bool polycross::VPoint::inside(const pointlist& plist)
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
         float tngns = (float) (_cp->y() - p0.y())/(p1.y() - p0.y());
         if (_cp->x() <= p0.x() + tngns * (p1.x() - p0.x()))
            cc++;
      }
   }
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

//==============================================================================
// SortLine

bool polycross::SortLine::operator() (CPoint* cp1, CPoint* cp2) {
   if (direction == xyorder(cp1->cp(), cp2->cp()))  return true;
   else                                             return false;
}

//==============================================================================
// polysegment

polycross::polysegment::polysegment(const TP* p1, const TP* p2, int num, char plyn) {
   if (xyorder(p1, p2) < 0) { _lP = p1; _rP = p2; }
   else                     { _lP = p2; _rP = p1; }
   _edge = num; _polyNo = plyn;_threadID = 0;
}

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

// void polycross::polysegment::dump_points(VPoint*& vlist) {
//    vlist = new VPoint(lP, vlist);
//    for (unsigned i = 0; i < crosspoints.size(); i++)
//       crosspoints[i]->linkage(vlist);
// }

TP* polycross::polysegment::checkIntersect(polysegment* ccand)
{
   // check that the cross candidate is not a sentinel segment
   // check for polygon edges belonging to the same polygon
   if ((0 == ccand->polyNo()) || (polyNo() == ccand->polyNo())) return NULL;
   // Now test for intersection point exsistence
   float lsign, rsign, rlmul;
   //check that both below endpoints are on the same side of above segment
   lsign = orientation(lP(), rP(), ccand->lP());
   rsign = orientation(lP(), rP(), ccand->rP());
   // do the case when both segments lie on the same line
   if ((lsign == 0) && (rsign == 0)) assert(false);//return oneLineSegments(ccand);
   rlmul = lsign*rsign;
   if      (0  < rlmul)  return NULL;// not crossing
   if (0 == rlmul) return joiningSegments(ccand, lsign, rsign);
   
   //now check that both above endpoints are on the same side of below segment
   lsign = orientation(ccand->lP(), ccand->rP(), lP());
   rsign = orientation(ccand->lP(), ccand->rP(), rP());
   // lsign == rsign == 0 case here should not be possible
   assert(!((lsign == 0) && (rsign == 0)));
   rlmul = lsign*rsign;
   if      (0  < rlmul) return NULL;
   if (0 == rlmul) return ccand->joiningSegments(this, lsign, rsign);
   // at this point - the only possibility is that they intersect
   // so - create a cross event 
   return getCross(ccand);
   
}

TP* polycross::polysegment::joiningSegments(polysegment* cross, float lps, float rps)
{
   // getLambda returns : < 0  the point is outside the segment
   //                     = 0  the point coinsides with one of the
   //                          segment endpoints
   //                     > 0  the point is inside the segment
   if (0 == lps)
   {
      if (0 >= getLambda(lP(), rP(), cross->lP())) return NULL;
      else return new TP(*(cross->lP()));
   }
   else if(0 == rps)
   {
      if (0 >= getLambda(lP(), rP(), cross->rP())) return NULL;
      else return new TP(*(cross->rP()));
   }
   assert(false);
}

TP* polycross::polysegment::getCross(polysegment* cseg)
{
   real A1 = rP()->y() - lP()->y();
   real A2 = cseg->rP()->y() - cseg->lP()->y();
   real B1 = lP()->x() - rP()->x();
   real B2 = cseg->lP()->x() - cseg->rP()->x();
   real C1 = -(A1*lP()->x() + B1*lP()->y());
   real C2 = -(A2*cseg->lP()->x() + B2*cseg->lP()->y());
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
   return new TP((int)rint(X),(int)rint(Y));
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

// polycross::BPoint* logicop::segmentlist::insertbindpoint(unsigned segno, const TP* point) {
//    return _segs[segno]->insertbindpoint(point);
// }

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

// polycross::VPoint* polycross::segmentlist::dump_points() {
//    VPoint* vlist = NULL;
//    for (unsigned i = 0; i < _segs.size(); i++)
//       _segs[i]->dump_points(vlist);
//    logicop::VPoint* lastV = vlist;
//    VPoint* centinel = NULL;
//    while (vlist->prev())  {
//       if (-1 == vlist->visited()) centinel = vlist;
//       vlist = vlist->prev();
//    }
//    lastV->set_next(vlist);
//    vlist->set_prev(lastV);
//    if (NULL != centinel) {
//       VPoint* vwork = centinel;
//       do {
//          if (-1 == vwork->visited()) {
//             //here visited == 0 means only that the object is Cpoint.
//             VPoint* tbdel = NULL;
//             if ((*vwork->cp()) == (*vwork->prev()->cp())) {
//                tbdel = vwork->prev();
//                vwork->set_prev(vwork->prev()->prev());
//                vwork->prev()->set_next(vwork);
//             }
//             else if ((*vwork->cp()) == (*vwork->next()->cp())) {
//                tbdel = vwork->next();
//                vwork->set_next(vwork->next()->next());
//                vwork->next()->set_prev(vwork);
//             }
//             vwork = vwork->next();
//             if (tbdel) delete tbdel;
//          }
//          else vwork = vwork->next();
//       } while (centinel != vwork);
//    }
//    return vlist;
// }

//==============================================================================
// TEvent

void polycross::TEvent::insertCrossPoint(TP* CP, polysegment* above,
                                         polysegment* below, XQ& eventQ)
{
   CPoint* cpsegA = above->insertCrossPoint(CP);
   CPoint* cpsegB = below->insertCrossPoint(CP);
   cpsegA->linkto(cpsegB);
   cpsegB->linkto(cpsegA);
   eventQ.addCrossEvent(CP, above->threadID(), below->threadID());
}

//==============================================================================
// TbEvent

polycross::TbEvent::TbEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   assert(seg1->lP() == seg2->lP());
   _tseg1 = seg1; _tseg2 = seg2;
   _evertex = seg1->lP();
}

void polycross::TbEvent::sweep(XQ& eventQ, YQ& sweepline)
{
#ifdef BO2_DEBUG
      printf("Begin 2 threads :\n");
#endif
   // first segment - create thread...
   SegmentThread* thr1 = sweepline.beginThread(_tseg1);
   // ...check for intersection with the thread above ...
   TP* CP1A = _tseg1->checkIntersect(thr1->threadAbove()->cseg());
   if (NULL != CP1A)
      insertCrossPoint(CP1A, thr1->threadAbove()->cseg(), _tseg1, eventQ);
   // ... and the thread below ...
   TP* CP1B = _tseg1->checkIntersect(thr1->threadBelow()->cseg());
   if (NULL != CP1B)
      insertCrossPoint(CP1B, _tseg1, thr1->threadBelow()->cseg(), eventQ);
   

   // now - second segment - create thread...
   SegmentThread* thr2 = sweepline.beginThread(_tseg2);
   // ...check for intersection with the thread above ...
   TP* CP2A = _tseg2->checkIntersect(thr2->threadAbove()->cseg());
   if (NULL != CP2A)
      insertCrossPoint(CP2A, thr2->threadAbove()->cseg(), _tseg2, eventQ);
   // ... and the thread below ...
   TP* CP2B = _tseg1->checkIntersect(thr2->threadBelow()->cseg());
   if (NULL != CP2B)
      insertCrossPoint(CP2B, _tseg2, thr2->threadBelow()->cseg(), eventQ);
#ifdef BO2_DEBUG
   BO_printseg(_tseg1)
   BO_printseg(_tseg2)
#endif
}

//==============================================================================
// TeEvent

polycross::TeEvent::TeEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   assert(seg1->rP() == seg2->rP());
   _tseg1 = seg1; _tseg2 = seg2;
   _evertex = seg1->rP();
}

void polycross::TeEvent::sweep (XQ& eventQ, YQ& sweepline)
{
#ifdef BO2_DEBUG
      printf("End 2 threads\n");
#endif
   // ...check for intersection of the threads above and below ...
   SegmentThread* tA = sweepline.getThread(_tseg1->threadID());
   TP* CPA = tA->threadBelow()->cseg()->checkIntersect(tA->threadAbove()->cseg());
   if (NULL != CPA)
      insertCrossPoint(CPA, tA->threadAbove()->cseg(), tA->threadBelow()->cseg(), eventQ);
   // remove the thread
//   SegmentThread* thr1 = sweepline.endThread(_tseg1->threadID());
   sweepline.endThread(_tseg1->threadID());
   
   SegmentThread* tB = sweepline.getThread(_tseg2->threadID());
   TP* CPB = tB->threadBelow()->cseg()->checkIntersect(tB->threadAbove()->cseg());
   if (NULL != CPB)
      insertCrossPoint(CPB, tB->threadAbove()->cseg(), tB->threadBelow()->cseg(), eventQ);
   // remove the thread
//   SegmentThread* thr2 = sweepline.endThread(_tseg2->threadID());
   sweepline.endThread(_tseg2->threadID());
#ifdef BO2_DEBUG
   BO_printseg(_tseg1)
   BO_printseg(_tseg2)
#endif
}

//==============================================================================
// TmEvent

polycross::TmEvent::TmEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   if       (seg1->rP() == seg2->lP())
   {
      _tseg1 = seg1; _tseg2 = seg2;
      _evertex = seg1->rP();
   }
   else if (seg2->rP() == seg1->lP())
   {
      _tseg1 = seg2; _tseg2 = seg1;
      _evertex = seg2->rP();
   }
   else assert(false);
}

void polycross::TmEvent::sweep (XQ& eventQ, YQ& sweepline)
{
#ifdef BO2_DEBUG
      printf("Modify thread\n");
#endif
   assert(0 != _tseg1->threadID());
   SegmentThread* thr = sweepline.modifyThread(_tseg1->threadID(), _tseg2);

   // check for intersections of the neighbours with the new segment
   TP* CPA = thr->cseg()->checkIntersect(thr->threadAbove()->cseg());
   if (NULL != CPA)
      insertCrossPoint(CPA, thr->threadAbove()->cseg(), thr->cseg(), eventQ);

   TP* CPB = thr->cseg()->checkIntersect(thr->threadBelow()->cseg());
   if (NULL != CPB)
      insertCrossPoint(CPB, thr->cseg(), thr->threadBelow()->cseg(), eventQ);
#ifdef BO2_DEBUG
   BO_printseg(_tseg1)
   BO_printseg(_tseg2)
#endif
}

//==============================================================================
// TcEvent

void polycross::TcEvent::sweep(XQ& eventQ, YQ& sweepline)
{
   SegmentThread* below = sweepline.swapThreads(_threadAboveID, _threadBelowID);
   SegmentThread* above = below->threadAbove();
   
   // check for intersections with the new neighbours
   TP* CPB = below->cseg()->checkIntersect(below->threadBelow()->cseg());
   if (NULL != CPB)
      insertCrossPoint(CPB, below->cseg(), below->threadBelow()->cseg(), eventQ);

   TP* CPA = above->cseg()->checkIntersect(above->threadAbove()->cseg());
   if (NULL != CPA)
      insertCrossPoint(CPA, above->cseg(), above->threadAbove()->cseg(), eventQ);

}

//==============================================================================
// EventVertex

polycross::EventVertex::EventVertex(TEvent* tevent)
{
   _evertex = tevent->evertex(); _events.push_back(tevent);
}

void polycross::EventVertex::addEvent(TEvent* tevent)
{
   _events.push_back(tevent);
   assert((_events.front()->shapeID() == 0) ||
         _events.front()->shapeID() == tevent->shapeID());
}

void polycross::EventVertex::sweep(YQ& sweepline, XQ& eventq)
{
#ifdef BO2_DEBUG
      printf("______________ POINT = ( %i , %i ) ___________\n", _evertex->x(), _evertex->y());
#endif
   while (!_events.empty())
   {
      TEvent* cevent = _events.front(); _events.pop_front();
      cevent->sweep(eventq, sweepline);
   }
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
polycross:: YQ::YQ(DBbox& overlap)
{
   TP* bl = new TP(overlap.p1().x()-1, overlap.p1().y()-1);
   TP* br = new TP(overlap.p2().x()+1, overlap.p1().y()-1);
   TP* tl = new TP(overlap.p1().x()-1, overlap.p2().y()+1);
   TP* tr = new TP(overlap.p2().x()+1, overlap.p2().y()+1);
   _bottomSentinel = new BottomSentinel(new polysegment(bl, br, -1, 0));
   _cthreads[-2] = _bottomSentinel;
   _topSentinel = new TopSentinel(new polysegment(tl, tr, -1, 0));
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
   assert(0 == startseg->threadID());
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
   assert(_cthreads.end() != threadP);
   SegmentThread* thread = threadP->second;
   // relink above/below threads
   SegmentThread* nextT = thread->threadAbove();
   assert(NULL != nextT);
   nextT->set_threadBelow(thread->threadBelow());
   SegmentThread* prevT = thread->threadBelow();
   assert(NULL != prevT);
   prevT->set_threadAbove(thread->threadAbove());
   // erase it
   _cthreads.erase(threadP);
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
   assert(_cthreads.end() != threadP);
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
   assert(_cthreads.end() != tiAbove);
   assert(_cthreads.end() != tiBelow);
   SegmentThread* tAbove = tiAbove->second;
   SegmentThread* tBelow = tiBelow->second;
   // relink above/below threads
#ifdef BO2_DEBUG
      printf("Swap threads\n");
      BO_printseg(tAbove->cseg());
      BO_printseg(tBelow->cseg());
#endif

   assert(tAbove == tBelow->threadAbove());
   assert(tBelow == tAbove->threadBelow());
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
   assert(_cthreads.end() != ti);
   return ti->second;
}

int polycross::YQ::sCompare(const polysegment* seg0, const polysegment* seg1)
{
   // Current point is always lp of seg0
   assert(seg0 != seg1);
   //
   int ori;
   ori = orientation(seg1->lP(), seg1->rP(), seg0->lP());
   if (ori != 0) return ori;
   // if it is a crossing point -> compare the slope
   ori = orientation(seg1->lP(), seg1->rP(), seg0->rP());
   if (ori != 0) return ori;
   // if it is still the same => we have coinciding segments
   assert(false);
}

//==============================================================================
// XQ

polycross::XQ::XQ( const segmentlist& seg1, const segmentlist& seg2 ) :
      _overlap(*(seg1[0]->lP()))
{
   _xqueue = avl_create(E_compare, NULL, NULL);
   createEvents(seg1,1);
   createEvents(seg2,2);
   _sweepline = new YQ(_overlap);
}

void polycross::XQ::createEvents(const segmentlist& seg, byte shapeID)
{
   unsigned s1, s2;
   TEvent*      evt;
   for(s1 = 0, s2 = 1 ; s1 < seg.size(); s1++, ++s2 %= seg.size())
   {
      // determine the type of event from the neighboring segments
      // and create the thread event
      if (seg[s1]->lP() == seg[s2]->lP())
         evt = new TbEvent(seg[s1], seg[s2], shapeID);
      else if (seg[s1]->rP() == seg[s2]->rP())
         evt = new TeEvent(seg[s1], seg[s2], shapeID);
      else
         evt = new TmEvent(seg[s1], seg[s2], shapeID);
      // update overlapping box
      _overlap.overlap(*(seg[s1]->lP()));
      _overlap.overlap(*(seg[s1]->rP()));
      // now create the vertex with the event inside
      EventVertex* vrtx = new EventVertex(evt);
      // and try to stick it in the AVL tree
      void** retitem =  avl_probe(_xqueue,vrtx);
      if ((*retitem) != vrtx)
      {
         // coinsiding vertexes from different polygons
         static_cast<EventVertex*>(*retitem)->addEvent(evt);
         delete(vrtx);
      }
   }
}

void polycross::XQ::addCrossEvent(TP* CP, unsigned thr1, unsigned thr2)
{
   TcEvent* evt = new TcEvent(CP, thr1, thr2);
   // now create the vertex with the event inside
   EventVertex* vrtx = new EventVertex(evt);
      // and try to stick it in the AVL tree
   void** retitem =  avl_probe(_xqueue,vrtx);
   if ((*retitem) != vrtx)
   {
         // coinsiding vertexes from different polygons
      static_cast<EventVertex*>(*retitem)->addEvent(evt);
      delete(vrtx);
   }
}

int polycross::XQ::E_compare( const void* v1, const void* v2, void*)
{
   const TP*  p1 = (*(EventVertex*)v1)();
   const TP*  p2 = (*(EventVertex*)v2)();
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
   }
}

//==============================================================================
// logic

polycross::logic::logic(const pointlist& poly1, const pointlist& poly2) :
      _poly1(poly1), _poly2(poly2)
{
   segmentlist _segl1(poly1,1);
   segmentlist _segl2(poly2,2);
   XQ* _eq = new XQ(_segl1, _segl2); // create the event queue
   _eq->sweep();
/*   unsigned crossp1 = _segl1.normalize(poly1);
   unsigned crossp2 = _segl2.normalize(poly2);
   assert(crossp1 == crossp2);
   _crossp = crossp1;
   delete _eq;
   _shape1 = _segl1.dump_points();
   _shape2 = _segl2.dump_points();*/
}
