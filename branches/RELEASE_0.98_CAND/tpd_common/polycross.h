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

#ifndef PLYCROSS_H_INCLUDED
#define PLYCROSS_H_INCLUDED

#include <vector>
#include "ttt.h"
#include "avl_def.h"

namespace polycross
{
   class YQ;
   class XQ;
   class BindCollection;
   class TEvent;
   class polysegment;
   typedef std::set<unsigned> ThreadList;
   typedef enum {_endE, _modifyE, _beginE, _crossE} EventTypes;
   typedef std::vector<polysegment*> Segments;

   int xyorder(const TP*, const TP*);
   int orientation(const TP*, const TP*, const TP*);
   float getLambda( const TP* p1, const TP* p2, const TP* p);
   char coincidingSegm(const TP*, const TP*, const TP*);
   bool pointInside(const TP*, const PointVector&, bool);

   //===========================================================================
   // Vertex Point
   //===========================================================================
   class VPoint
   {
      public:
         VPoint(const TP* cp) : _cp(cp),_next(NULL),_prev(NULL) {};
         VPoint(const TP* cp, VPoint* prev);
         virtual VPoint*  follower(bool& direction, bool modify = false);
         virtual bool     inside(const PointVector&, bool touching = false);
         virtual char     visited() const      {return 1;}
         virtual void     reset_visited()      {};
         virtual VPoint*  checkNreorder(VPoint*&, bool);
         virtual VPoint*  checkRedundantCross();
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
   // Cross Point
   //===========================================================================
   class CPoint : public VPoint
   {
      public:
         CPoint(const TP* cp, int edge) : VPoint(&_crossingP), _link(NULL), _visited(0),
         _edge(edge), _crossingP(cp->x(), cp->y()) {};
//         virtual ~CPoint() {delete _cp;}
         virtual VPoint*  follower(bool& direction, bool modify = false);
         bool              inside(const PointVector&, bool touching = false) {return true;}
         char              visited() const {return _visited;}
         void              linkto(CPoint* link) {_link = link;}
         CPoint*           link() const {return _link;}
         void              reset_visited() {_visited = 0;};
         void              linkage(VPoint*& prev);
         VPoint*           checkNreorder(VPoint*&, bool) {assert(false); return NULL;}
         VPoint*           checkRedundantCross()  {assert(false); return NULL;}
         int               edge() const  {return _edge;}
      protected:
         CPoint*           _link;
         char              _visited;
         int               _edge;
      private:
         TP                _crossingP;
   };

   //===========================================================================
   // Binding Point
   //===========================================================================
   class BPoint : public CPoint
   {
      public:
      //! Creates a new BPoint simply by calling the CPoint constructor
         BPoint(const TP* cp, int edge) : CPoint(cp, edge) {};
      //! Returns always 1 - VPoint is considered always visited
         char              visited() const      {return -1;}
      //! Returns the following point for the currently generated polygon
         VPoint*           follower(bool& direction, bool modify = false);
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
         typedef std::list<CPoint*> crossCList;
                           polysegment(const TP*, const TP*, int, byte);
                          ~polysegment();
         CPoint*           insertCrossPoint(const TP*);
         BPoint*           insertBindPoint(const TP* pnt);
         unsigned          normalize(const TP*, const TP*);
         void              dump_points(polycross::VPoint*&, const Segments&);
         bool              operator == (const polysegment&) const;
         unsigned          threadID() const           {return _threadID;}
         void              set_threadID(unsigned ID)  {_threadID = ID;}
         const TP*         lP() const                 {return _lP;}
         const TP*         rP() const                 {return _rP;}
         byte              polyNo() const             {return _polyNo;}
         int               edge() const               {return _edge;}
      protected:
         unsigned          _threadID;
         crossCList        _crossPoints;
         byte              _polyNo;
         int               _edge;
         const TP*         _lP;
         const TP*         _rP;
   };

   //===========================================================================
   // segmentlist
   //===========================================================================
   class segmentlist
   {
      public:
         segmentlist(const PointVector&, byte, bool);
         ~segmentlist();
         polysegment*      operator [](unsigned i) const {return _segs[i];};
         unsigned          size() const {return _segs.size();};
         unsigned          normalize(const PointVector&, bool);
         VPoint*           dump_points(bool looped, const segmentlist*);
         BPoint*           insertBindPoint(unsigned segno, const TP* point);
         const PointVector* originalPL() const {return _originalPL;}
      private:
         Segments          _segs;
         const PointVector* _originalPL;
   };

   //===========================================================================
   // Event Vertex - could be more than one event
   //===========================================================================
   class EventVertex
   {
      public:
                           EventVertex(const TP* evertex) :
                              _evertex(DEBUG_NEW TP(evertex->x(), evertex->y())) {};
                          ~EventVertex();
         const TP*         operator () () const {return _evertex;};
         void              addEvent(TEvent*, EventTypes);
         void              clearAllEvents();
         void              sweep(YQ&, XQ&, bool, bool);
         void              sweep2bind(YQ&, BindCollection&);
         void              CheckBEM(XQ&, TEvent&, TEvent&, bool);
      private:
         typedef std::list<TEvent*> Events;
         typedef std::map<int, Events> AllEvents;
         AllEvents         _events;
         TP*               _evertex;
         ThreadList        _threadsSweeped;
   };

   //===========================================================================
   // Thread event - pure virtual
   //===========================================================================
   class TEvent
   {
      public:
         friend void EventVertex::CheckBEM(XQ&, TEvent&, TEvent&, bool);
//         TEvent(byte shapeID) : _shapeID(shapeID) {};
         virtual void      sweep(XQ&, YQ&, ThreadList&, bool) = 0;
         virtual void      sweep2bind(YQ&, BindCollection&) = 0;
         virtual const TP* avertex() = 0;
         virtual const TP* bvertex() = 0;
         const TP*         evertex() {return _evertex;}
//         byte              shapeID() {return _shapeID;}
         polysegment*      aseg() {return _aseg;}
         polysegment*      bseg() {return _bseg;}
         virtual          ~TEvent() {};
      protected:
         void              checkIntersect(polysegment*, polysegment*, XQ&, bool, const TP* iff=NULL);
         const TP*         _evertex;
         void              insertCrossPoint(const TP*, polysegment*, polysegment*,
                                            XQ&, bool dontswap = false);
         polysegment*      _aseg;
         polysegment*      _bseg;
      private:
         TP*               joiningSegments(polysegment*, polysegment*, float, float);
         TP*               oneLineSegments(polysegment*, polysegment*, YQ*);
         TP*               getCross(polysegment*, polysegment*);
         TP*               getMiddle(const TP*, const TP*);
   };

   //===========================================================================
   // Thread begin event
   //===========================================================================
   class TbEvent : public TEvent
   {
      public:
         TbEvent(polysegment*, polysegment*);
         void              sweep(XQ&, YQ&, ThreadList&, bool);
         void              sweep2bind(YQ&, BindCollection&);
         const TP*         avertex() {return _aseg->rP();}
         const TP*         bvertex() {return _bseg->rP();};
   };

   //===========================================================================
   // Thread end event
   //===========================================================================
   class TeEvent : public TEvent
   {
      public:
         TeEvent(polysegment*, polysegment*);
         void              sweep(XQ&, YQ&, ThreadList&, bool);
         void              sweep2bind(YQ&, BindCollection&);
         const TP*         avertex() {return _aseg->lP();}
         const TP*         bvertex() {return _bseg->lP();}
   };

   //===========================================================================
   // Thread modify event
   //===========================================================================
   class TmEvent : public TEvent
   {
      public:
         TmEvent(polysegment*, polysegment*);
         void              sweep(XQ&, YQ&, ThreadList&, bool);
         void              sweep2bind(YQ&, BindCollection&);
         const TP*         avertex() {return _aseg->lP();}
         const TP*         bvertex() {return _bseg->rP();}
   };

   //===========================================================================
   // Thread begin event single segment
   //===========================================================================
   class TbsEvent : public TEvent
   {
      public:
                           TbsEvent(polysegment*);
         void              sweep(XQ&, YQ&, ThreadList&, bool);
         void              sweep2bind(YQ&, BindCollection&) {assert(false);}
         const TP*         avertex() {return _aseg->rP();}
         const TP*         bvertex() {assert(false);return NULL;}
   };

   //===========================================================================
   // Thread end event single segment
   //===========================================================================
   class TesEvent : public TEvent
   {
      public:
                           TesEvent(polysegment*);
         void              sweep(XQ&, YQ&, ThreadList&, bool);
         void              sweep2bind(YQ&, BindCollection&) {assert(false);}
         const TP*         avertex() {return _aseg->lP();}
         const TP*         bvertex() {assert(false); return NULL;}
   };

   //===========================================================================
   // Thread cross event
   //===========================================================================
   class TcEvent : public TEvent
   {
      public:
         TcEvent(const TP* ev, polysegment* aseg, polysegment* bseg): TEvent(),
         _threadAbove(aseg->threadID()),_threadBelow(bseg->threadID()),
         _eventvertex(ev->x(), ev->y())
            {_aseg = aseg; _bseg = bseg;  _evertex = &_eventvertex;}
//               ~TcEvent() { delete _evertex; }
         void              sweep(XQ&, YQ&, ThreadList&, bool);
         void              sweep2bind(YQ&, BindCollection&) {assert(false);}
         const TP*         avertex() {assert(false); return NULL;}
         const TP*         bvertex() {assert(false); return NULL;}
         bool              operator == (const TcEvent&) const;
      private:
         unsigned          _threadAbove;
         unsigned          _threadBelow;
         TP                _eventvertex;
   };

   //===========================================================================
   // Segment Thread
   //===========================================================================
   class SegmentThread
   {
      public:
         SegmentThread(polysegment* cseg, SegmentThread* tb, SegmentThread* ta) :
            _cseg(cseg) , _threadBelow(tb), _threadAbove(ta) {};
         bool              operator == (const SegmentThread&) const;
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


   //===========================================================================
   // Y queue
   //===========================================================================
   class YQ
   {
      public:
         typedef std::map<int,SegmentThread*> Threads;
                           YQ(DBbox&, const segmentlist*, const segmentlist*);
                           YQ(DBbox&, const segmentlist*);
                          ~YQ();
         SegmentThread*    beginThread(polysegment*);
         SegmentThread*    endThread(unsigned);
         SegmentThread*    modifyThread(unsigned, polysegment*, bool& );
         SegmentThread*    swapThreads(unsigned, unsigned);
         SegmentThread*    getThread(unsigned);
         const PointVector* locateOriginals(const polysegment*, unsigned&, unsigned&, bool&);
         void              report();
         const PointVector* opl1() const {return _osl1->originalPL();}
         const PointVector* opl2() const {return _osl2->originalPL();}
      private:
         class TopSentinel : public SegmentThread
         {
            public:
               TopSentinel(polysegment* cseg) : SegmentThread(cseg,NULL,NULL) {};
               SegmentThread*    threadAbove()  {assert(false); return NULL;}
               ~TopSentinel() {delete _cseg;}
         };
         class BottomSentinel : public SegmentThread
         {
            public:
               BottomSentinel(polysegment* cseg) : SegmentThread(cseg,NULL,NULL) {};
               SegmentThread*    threadBelow()  {assert(false); return NULL;}
               ~BottomSentinel() {delete _cseg;}
         };
         BottomSentinel*   _bottomSentinel;
         TopSentinel*      _topSentinel;
         void              initialize(DBbox&);
         int               sCompare(const polysegment*, const polysegment*);
         Threads           _cthreads;
         int               _lastThreadID;
         const segmentlist* _osl1;
         const segmentlist* _osl2;
         TP*               _blSent;
         TP*               _brSent;
         TP*               _tlSent;
         TP*               _trSent;

   };

   //===========================================================================
   // X queue
   //===========================================================================
   class XQ {
      public:
                           XQ(const segmentlist&, const segmentlist&);
                           XQ(const segmentlist&, bool loopsegs);
         ~XQ();
         void              sweep(bool, bool);
         void              sweep2bind(BindCollection&);
         void              addCrossEvent(const TP*, polysegment*, polysegment*);
         YQ*               sweepline() {return _sweepLine;}
      protected:
         void              createEvents(const segmentlist&);
         void              createSEvents(const segmentlist&);
         void              addEvent(polysegment*, TEvent*, EventTypes);
         static int        E_compare( const void*, const void*, void* );
         avl_table*        _xQueue;
         avl_table*        _xOldQueue;
         YQ*               _sweepLine;
         DBbox             _overlap;
         bool              _loopSegs;
   };

   //===========================================================================
   // Bind segment
   //===========================================================================
   class BindSegment
   {
      public:
         BindSegment(unsigned p0s, unsigned p1s, TP* p0p, const TP* p1p,
                     real dist) : _poly0seg(p0s), _poly1seg(p1s), _poly0pnt(p0p),
         _poly1pnt(p1p), _distance(dist) {};
         ~BindSegment() {delete _poly0pnt;}
         unsigned          poly0seg() { return _poly0seg;};
         unsigned          poly1seg() { return _poly1seg;};
         const TP*         poly0pnt() const {return _poly0pnt;}
         const TP*         poly1pnt() const {return _poly1pnt;}
         const real        distance() const {return _distance;};
      private:
         unsigned          _poly0seg;
         unsigned          _poly1seg;
               TP*         _poly0pnt;
         const TP*         _poly1pnt;
         real              _distance;
   };

   //===========================================================================
   // BindCollection
   //===========================================================================

   class BindCollection
   {
      public:
                          ~BindCollection();
         void              update_BL(polysegment*, unsigned, const TP*);
         BindSegment*      getBindSegment(const pcollection&);
      protected:
         typedef std::list<BindSegment*> BindList;
         bool              is_shorter(unsigned segno, real dist);
         static bool       compareSegments(BindSegment*, BindSegment*);
         bool              obstructed(const BindSegment&, const PointVector&);
         BindList          _blist;
   };

}

#endif
