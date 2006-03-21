#ifndef PLYCROSS_H_INCLUDED
#define PLYCROSS_H_INCLUDED

#include <vector>
#include "../tpd_common/ttt.h"
#include "../tpd_common/avl_def.h"

namespace polycross
{
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
   // plysegment
   //===========================================================================
   class plysegment
   {
      public:
         typedef std::vector<CPoint*> crossCList;
         plysegment(const TP*, const TP*, int, char);
         CPoint*           insertcrosspoint(const TP*);
         unsigned          normalize(const TP*, const TP*);
         void              dump_points(VPoint*&);
         bool              trivial() const {return (lP == rP);}
         int               edge;
         const TP*         lP;
         const TP*         rP;
         crossCList        crosspoints;
         char              polyNo;
   };

   //===========================================================================
   // segmentlist
   //===========================================================================
   class segmentlist
   {
      public:
         segmentlist(const pointlist&, byte);
         ~segmentlist();
         plysegment*       operator [](unsigned int& i) const {return _segs[i];};
         unsigned          size() const {return _segs.size();};
         unsigned          normalize(const pointlist&);
         VPoint*           dump_points();
      private:
         std::vector<plysegment*> _segs;
   };
   
   //===========================================================================
   // Thread event - pure virtual
   //===========================================================================
   class TEvent
   {
      public:
         TEvent(byte shapeID) : _shapeID(shapeID) {};
         virtual const TP* evertex() const = 0;
         virtual void      sweep() = 0;
                 byte      shapeID() {return _shapeID;}
         virtual          ~TEvent() {};
      private:
         byte              _shapeID;
   };

   //===========================================================================
   // Thread begin event
   //===========================================================================
   class TbEvent : public TEvent
   {
      public:
         TbEvent(plysegment*, plysegment*, byte);
         void              sweep();
         const TP*         evertex() const {return _evertex;}
      private:
         plysegment*       _tseg1;
         plysegment*       _tseg2;
         const TP*         _evertex;
   };

   //===========================================================================
   // Thread end event
   //===========================================================================
   class TeEvent : public TEvent
   {
      public:
         TeEvent(plysegment*, plysegment*, byte);
         void              sweep();
         const TP*         evertex() const {return _evertex;}
      private:
         plysegment*       _tseg1;
         plysegment*       _tseg2;
         const TP*         _evertex;
   };

   //===========================================================================
   // Thread modify event
   //===========================================================================
   class TmEvent : public TEvent
   {
      public:
         TmEvent(plysegment*, plysegment*, byte);
         void              sweep();
         const TP*         evertex() const {return _evertex;}
      private:
         plysegment*       _tseg1;
         plysegment*       _tseg2;
         const TP*         _evertex;
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
         SegmentThread(plysegment* cseg, SegmentThread* tb, SegmentThread* ta) :
            _cseg(cseg) , _threadBelow(tb), _threadAbove(ta) {};
         void              set_threadBelow(SegmentThread* td) {_threadBelow = td;}
         void              set_threadAbove(SegmentThread* td) {_threadAbove = td;}
         plysegment*       cseg()         {return _cseg;}
         SegmentThread*    threadBelow()  {return _threadBelow;}
         SegmentThread*    threadAbove()  {return _threadAbove;}
      private:
         plysegment*       _cseg;
         SegmentThread*    _threadBelow;
         SegmentThread*    _threadAbove;
   };

   //===========================================================================
   // X queue
   //===========================================================================
   class XQ {
      public:
         XQ(const segmentlist &, const segmentlist&);
      protected:
         void              createEvents(const segmentlist&, byte);
         static int        E_compare( const void*, const void*, void* );
         avl_table*        _xqueue;
   };

}

#endif
