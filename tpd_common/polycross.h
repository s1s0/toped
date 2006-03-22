#ifndef PLYCROSS_H_INCLUDED
#define PLYCROSS_H_INCLUDED

#include <vector>
#include "../tpd_common/ttt.h"
#include "../tpd_common/avl_def.h"

namespace polycross
{
   int xyorder(const TP*, const TP*);
   class YQ;
   class XQ;
   //===========================================================================
   // VPoint
   //===========================================================================
   class VPoint
   {
      public:
         VPoint(const TP* cp) : _cp(cp),_next(NULL),_prev(NULL) {};
         VPoint(const TP* cp, VPoint* prev);
         virtual VPoint*  follower(bool& direction, bool modify = false);
         virtual bool     inside(const pointlist&);
         virtual char     visited() const      {return 1;}
         virtual void     reset_visited()      {};
         const TP*         cp() const           {return _cp;};
         VPoint*           next() const         {return _next;};
         VPoint*           prev() const         {return _prev;};
         void              set_next(VPoint* nx) {_next = nx;};
         void              set_prev(VPoint* pr) {_prev = pr;};
         virtual          ~VPoint() {};
      protected:
         const TP*         _cp;
         VPoint*           _next;
         VPoint*           _prev;
   };

   //===========================================================================
   // CPoint
   //===========================================================================
   class CPoint : public VPoint
   {
      public:
         CPoint(const TP* cp) : VPoint(cp),_link(NULL),_visited(0) {};
         virtual VPoint*  follower(bool& direction, bool modify = false);
         bool              inside(const pointlist&) {return true;}
         char              visited() const {return _visited;}
         void              linkto(CPoint* link) {_link = link;}
         void              reset_visited() {_visited = 0;};
         void              linkage(VPoint*& prev);
      protected:
         CPoint*           _link;
         char              _visited;
   };

   //===========================================================================
   // SortLine
   //===========================================================================
   class SortLine
   {
      public:
         SortLine(const TP* p1, const TP* p2) {direction = xyorder(p1,p2);};
         bool operator    () (CPoint*, CPoint*);
      protected:
         int               direction;
   };

   //===========================================================================
   // polysegment
   //===========================================================================
   class polysegment
   {
      public:
         typedef std::vector<CPoint*> crossCList;
         polysegment(const TP*, const TP*, int, char);
         CPoint*           insertcrosspoint(const TP*);
         unsigned          normalize(const TP*, const TP*);
//          void              dump_points(VPoint*&);
         unsigned          threadID() {return _threadID;}
         void              set_threadID(unsigned ID) {_threadID = ID;}
         TP*               checkIntersect(polysegment*);
         const TP*         lP;
         const TP*         rP;
      protected:
         unsigned          _threadID;
         crossCList        crosspoints;
         char              polyNo;
         int               edge;
   };

   //===========================================================================
   // segmentlist
   //===========================================================================
   class segmentlist
   {
      public:
         typedef std::vector<polysegment*> Segments;
         segmentlist(const pointlist&, byte);
         ~segmentlist();
         polysegment*      operator [](unsigned i) const {return _segs[i];};
         unsigned          size() const {return _segs.size();};
         unsigned          normalize(const pointlist&);
         VPoint*           dump_points();
      private:
         Segments          _segs;
         const pointlist*  _originalPL;
   };
   
   //===========================================================================
   // Thread event - pure virtual
   //===========================================================================
   class TEvent
   {
      public:
         TEvent(byte shapeID) : _shapeID(shapeID) {};
         virtual void      sweep(YQ&) = 0;
         const TP*         evertex() {return _evertex;}
         byte              shapeID() {return _shapeID;}
         virtual          ~TEvent() {};
      protected:
         byte              _shapeID;
         const TP*         _evertex;
   };

   //===========================================================================
   // Thread begin event
   //===========================================================================
   class TbEvent : public TEvent
   {
      public:
         TbEvent(polysegment*, polysegment*, byte);
         void              sweep(YQ&);
      private:
         polysegment*      _tseg1;
         polysegment*      _tseg2;
   };

   //===========================================================================
   // Thread end event
   //===========================================================================
   class TeEvent : public TEvent
   {
      public:
         TeEvent(polysegment*, polysegment*, byte);
         void              sweep(YQ&);
      private:
         polysegment*      _tseg1;
         polysegment*      _tseg2;
   };

   //===========================================================================
   // Thread modify event
   //===========================================================================
   class TmEvent : public TEvent
   {
      public:
         TmEvent(polysegment*, polysegment*, byte);
         void              sweep(YQ&);
      private:
         polysegment*      _tseg1;
         polysegment*      _tseg2;
   };

   //===========================================================================
   // Event Vertex - could be more than one event
   //===========================================================================
   class EventVertex
   {
      public:
         EventVertex(TEvent*);
         const TP*         operator () () {return _evertex;};
         void              addEvent(TEvent*);
         void              sweep(YQ&, XQ&);
      private:
         typedef std::list<TEvent*> Events;
         Events            _events;
         const TP*         _evertex;
   };

   //===========================================================================
   // Segment Thread
   //===========================================================================
   class SegmentThread
   {
      public:
         SegmentThread(polysegment* cseg, SegmentThread* tb, SegmentThread* ta) :
            _cseg(cseg) , _threadBelow(tb), _threadAbove(ta) {};
         void              set_threadBelow(SegmentThread* td) {_threadBelow = td;}
         void              set_threadAbove(SegmentThread* td) {_threadAbove = td;}
         polysegment*      cseg()         {return _cseg;}
         polysegment*      set_cseg(polysegment* cs);
         virtual SegmentThread*    threadBelow()  {return _threadBelow;}
         virtual SegmentThread*    threadAbove()  {return _threadAbove;}
         virtual ~SegmentThread(){};
      protected:
         polysegment*      _cseg; // current segment
         SegmentThread*    _threadBelow;
         SegmentThread*    _threadAbove;
   };

   class BottomSentinel : public SegmentThread
   {
      public:
         BottomSentinel(polysegment* cseg) : SegmentThread(cseg,NULL,NULL) {};
         SegmentThread*    threadBelow()  {assert(false);}
         ~BottomSentinel() { delete _cseg;}
   };

   class TopSentinel : public SegmentThread
   {
      public:
         TopSentinel(polysegment* cseg) : SegmentThread(cseg,NULL,NULL) {};
         SegmentThread*    threadAbove()  {assert(false);}
         ~TopSentinel() { delete _cseg;}
   };
   //===========================================================================
   // Y queue
   //===========================================================================
   class YQ
   {
      public:
         typedef std::map<int,SegmentThread*> Threads;
         YQ(DBbox&);
         SegmentThread*    beginThread(polysegment*);
         SegmentThread*    endThread(unsigned);
         SegmentThread*    modifyThread(unsigned, polysegment*);
      private:
         BottomSentinel*   _bottomSentinel;
         TopSentinel*      _topSentinel;
         int               sCompare(const polysegment*, const polysegment*);
         int               orientation(const TP*, const TP*, const TP*);
         Threads           _cthreads;
         int               _lastThreadID;
   };

   //===========================================================================
   // X queue
   //===========================================================================
   class XQ {
      public:
         XQ(const segmentlist &, const segmentlist&);
         void              sweep();
      protected:
         BottomSentinel*   _bottomSentinel;
         TopSentinel*      _topSentinel;
         void              createEvents(const segmentlist&, byte);
         static int        E_compare( const void*, const void*, void* );
         avl_table*        _xqueue;
         YQ*               _sweepline;
         DBbox             _overlap;
         TP                _bottom_left;
         TP                _top_right;
   };

   //===========================================================================
   class logic
   {
      public:
         logic(const pointlist&, const pointlist&);
//         bool              AND(pcollection&);
//         bool              ANDNOT(pcollection&);
//         bool              OR(pcollection&);
//         void              reset_visited();
      private:
//         pointlist*        hole2simple(const pointlist&, const pointlist&);
         //
//         VPoint*           getFirstOutside(const pointlist&, VPoint*);
         const pointlist&  _poly1;
         const pointlist&  _poly2;
//         VPoint*           _shape1;
//         VPoint*           _shape2;
//         unsigned          _crossp;
   };
}

#endif
