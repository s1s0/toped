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
//        Created: Tue Sep 28 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Self crossing polygons algo
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef TEDOP_H_INCLUDED
#define TEDOP_H_INCLUDED

#include <vector>
#include <list>
#include "ttt.h"
#include "avl_def.h"

namespace tedop {
   //===========================================================================
   // Bentley-Ottmann alghorithm related definitions      
   int xyorder(const TP*, const TP*);
   int yxorder(const TP*, const TP*);
   float isLeft(const TP*, const TP*, const TP*);
   //===========================================================================
   class plysegment {
   public:
                        plysegment(const TP*, const TP*, unsigned);
      unsigned          edge; // to keep track of adjacent edges
      const TP*         lP;
      const TP*         rP;
      plysegment*       above;
      plysegment*       below;
   };
   
   //===========================================================================
   class segmentlist {
   public:
                        segmentlist(const pointlist&, bool);
      plysegment*       operator [](unsigned int& i) const {return segs[i];};
      unsigned          size() const {return segs.size();};
                       ~segmentlist();
   private:
      std::vector<plysegment*> segs;
   };
   
   //===========================================================================
   // Sweep line is using Adelson-Velskii & Landis balanced binary tree
   //===========================================================================
   class SweepLine {
   public:
                        SweepLine(const pointlist);
                       ~SweepLine(); 
     void               add(plysegment*);
     void               remove(plysegment*);
     int                intersect(plysegment*, plysegment*);
   protected:
      static int        compare_seg(const void*, const void*, void*);
      bool              coincideOK(plysegment*, plysegment*, float, float);
      float             getLambda( const TP*, const TP*, const TP*);
      pointlist         _plist;
      unsigned          _numv;          // number of vertices in the polygon
      avl_table*        _tree;          //AVL tree
   };     
         
   //===========================================================================
   class Event {
   public:
                         Event(plysegment* seg) : _seg(seg) {};
      virtual const TP* evertex() const = 0;
      virtual bool      check_valid(SweepLine&) const = 0;
      plysegment*        segment() const {return _seg;};
      //! Return the edge number for this event
      int                edgeNo() const {return _seg->edge;};
      virtual            ~Event() {};
   protected:   
      plysegment*        _seg;
   };
   
   class LEvent : public Event {
   public:
                        LEvent(plysegment* seg) : Event(seg) {};
      const TP*         evertex() const {return _seg->lP;};
      bool              check_valid(SweepLine&) const;
   };
   
   class REvent : public Event {
   public:
                        REvent(plysegment* seg) : Event(seg) {};
      const TP*         evertex() const {return _seg->rP;};
      bool              check_valid(SweepLine&) const;
   };
   
   typedef std::vector<Event*> eventlist;
   //===========================================================================
   class EventQueue {
   public:
                        EventQueue(const segmentlist& segments);
      bool              check_valid(SweepLine&);
                       ~EventQueue();
   protected:   
      static bool       E_compare( const Event* v1, const Event* v2 );
      eventlist         equeue;
   };
}
#endif
