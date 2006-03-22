#include <algorithm>
#include "polycross.h"

//-----------------------------------------------------------------------------
// The declaratoin of the avl related functions. They are declared originally
// in avl.h and redeclared here in C++ manner with extern "C" clause
extern "C" {
   avl_table* avl_create (avl_comparison_func *, const void *, libavl_allocator *);
   void **avl_probe (struct avl_table *, void *);
//   void       avl_destroy (struct avl_table *, avl_item_func *);
//   void*      avl_t_next (avl_traverser *);
//   void*      avl_t_prev (avl_traverser *);
//   void*      avl_delete (avl_table *, const void *);
//   void*      avl_t_find (avl_traverser *, avl_table *, void *);
//   void*      avl_t_insert (avl_traverser *, avl_table *, void *);
//   void*      avl_t_first (avl_traverser *, avl_table *);
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
   if (xyorder(p1, p2) < 0) { lP = p1; rP = p2; }
   else                     { lP = p2; rP = p1; }
   edge = num; polyNo = plyn;
}

polycross::CPoint* polycross::polysegment::insertcrosspoint(const TP* pnt) {
   CPoint* cp = new CPoint(pnt);
   crosspoints.push_back(cp);
   return cp;
}

unsigned polycross::polysegment::normalize(const TP* p1, const TP* p2) {
   lP = p1; rP = p2;
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

//==============================================================================
// TbEvent

polycross::TbEvent::TbEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   assert(seg1->lP == seg2->lP);
   _tseg1 = seg1; _tseg2 = seg2;
   _evertex = seg1->lP;
}

void polycross::TbEvent::sweep (YQ& sweepline)
{
   sweepline.beginThread(_tseg1);
   sweepline.beginThread(_tseg2);
}

//==============================================================================
// TeEvent

polycross::TeEvent::TeEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   assert(seg1->rP == seg2->rP);
   _tseg1 = seg1; _tseg2 = seg2;
   _evertex = seg1->rP;
}

void polycross::TeEvent::sweep (YQ& sweepline)
{
   sweepline.endThread(_tseg1->threadID());
   sweepline.endThread(_tseg2->threadID());
}

//==============================================================================
// TmEvent

polycross::TmEvent::TmEvent (polysegment* seg1, polysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   if       (seg1->rP == seg2->lP)
   {
      _tseg1 = seg1; _tseg2 = seg2;
      _evertex = seg1->rP;
   }
   else if (seg2->rP == seg1->lP)
   {
      _tseg1 = seg2; _tseg2 = seg1;
      _evertex = seg2->rP;
   }
   else assert(false);
}

void polycross::TmEvent::sweep (YQ& sweepline)
{
   assert(0 != _tseg1->threadID());
   sweepline.modifyThread(_tseg1->threadID(), _tseg2);
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
   _cthreads[-2] = new BottomSentinel(new polysegment(bl, br, -1, 0));
   _cthreads[-1] = new TopSentinel(new polysegment(tl, tr, -1, 0));
   _lastThreadID = 0;
}

void polycross::YQ::beginThread(polycross::polysegment* startseg)
{
   assert(0 == startseg->threadID());
   Threads::const_iterator CT = _cthreads.begin();
   SegmentThread* below = NULL;
   SegmentThread* above = NULL;
   while ((CT != _cthreads.end()) && (sCompare(startseg, CT->second->cseg()) < 0))
   {
      below = CT->second;above = below->threadAbove();
      CT++;
   }
   assert(NULL != below);
   assert(NULL != above);
   _cthreads[++_lastThreadID] = new SegmentThread(startseg, above, below);
   startseg->set_threadID(_lastThreadID);
}

void polycross::YQ::endThread(unsigned threadID)
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
}

void polycross::YQ::modifyThread(unsigned threadID, polysegment* newsegment)
{
   // get the thread from the _cthreads
   Threads::iterator threadP = _cthreads.find(threadID);
   assert(_cthreads.end() != threadP);
   SegmentThread* thread = threadP->second;
   newsegment->set_threadID(threadID);
   polysegment* oldsegment = thread->set_cseg(newsegment);
   oldsegment->set_threadID(0);
}

//==============================================================================
// XQ

polycross::XQ::XQ( const segmentlist& seg1, const segmentlist& seg2 ) :
      overlap(*(seg1[0]->lP))
{
   _xqueue = avl_create(E_compare, NULL, NULL);
   createEvents(seg1,1);
   createEvents(seg2,2);
   sweepline = new YQ(overlap);
}

void polycross::XQ::createEvents(const segmentlist& seg, byte shapeID)
{
   unsigned s1, s2;
   TEvent*      evt;
   for(s1 = 0, s2 = 1 ; s1 < seg.size(); s1++, ++s2 %= seg.size())
   {
      // determine the type of event from the neighboring segments
      // and create the thread event
      if (seg[s1]->lP == seg[s2]->lP)
         evt = new TbEvent(seg[s1], seg[s2], shapeID);
      else if (seg[s1]->rP == seg[s2]->rP)
         evt = new TeEvent(seg[s1], seg[s2], shapeID);
      else
         evt = new TmEvent(seg[s1], seg[s2], shapeID);
      // update overlapping box
      overlap.overlap(*(seg[s1]->lP));
      overlap.overlap(*(seg[s1]->rP));
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

int polycross::XQ::E_compare( const void* v1, const void* v2, void*)
{
   const TP*  p1 = (*(EventVertex*)v1)();
   const TP*  p2 = (*(EventVertex*)v2)();
   return xyorder(p1,p2);
}

