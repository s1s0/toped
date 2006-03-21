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

polycross::TbEvent::TbEvent (plysegment* seg1, plysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   assert(seg1->lP == seg2->lP);
   _tseg1 = seg1; _tseg2 = seg2;
   _evertex = seg1->lP;
}

polycross::TeEvent::TeEvent (plysegment* seg1, plysegment* seg2, byte shapeID):
      TEvent( shapeID )
{
   assert(seg1->rP == seg2->rP);
   _tseg1 = seg1; _tseg2 = seg2;
   _evertex = seg1->rP;
}

polycross::TmEvent::TmEvent (plysegment* seg1, plysegment* seg2, byte shapeID):
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

polycross::XQ::XQ( const segmentlist& seg1, const segmentlist& seg2 )
{
   _xqueue = avl_create(E_compare, NULL, NULL);
   createEvents(seg1,1);
   createEvents(seg2,2);
}

void polycross::XQ::createEvents(const segmentlist& seg, byte shapeID)
{
   unsigned s1, s2;
   EventVertex* vrtx;
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
/* Determines the lexicographical order of two points comparing X first.
   Returns:
   - +1 -> p1  > p2
   - -1 -> p1  < p2
   -  0 -> p1 == p2
 */
   const TP*  p1 = (*(EventVertex*)v1)();
   const TP*  p2 = (*(EventVertex*)v2)();

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

