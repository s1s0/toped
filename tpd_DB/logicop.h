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
//        Created: Mon Oct 01 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: polygon logic operations
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef LOGICOP_H_INCLUDED
#define LOGICOP_H_INCLUDED

#include "../tpd_common/ttt.h"
#include "../tpd_common/polycross.h"
#include "tedstd.h"

namespace logicop {
   
   typedef std::list<pointlist*> pcollection; // point list collection
   //===========================================================================
   /*!This class is the top level wrapper or the interface to all logic 
   operations with shapes. As a general all logic operations are following
   the same procedural path:
   - find all crossing points between the input polygons
   - prepare the data for new shape generation
   - generate the new shape as a result of a certain logic operation
   
   So the class constructor is executing the first two steps, manufacturing the 
   "raw" product #_shape1 and #_shape2. The user has to call afterwards the actual
   logic operation method that is producing the final result. Most if not all 
   logic operations can produce a collection of polygons, that's why a new 
   container type pcollection is introduced. If more than one operation is 
   desired, the user has to call reset_visited(), to prepare the #_shape1/#_shape2
   structures for another traversing.\n It has to be noted that in general 
   the input polygons and repectively _poly1 and _poly2 fields are not 
   interchangable. For example polyA ANDNOT polyB produces different result from
   polyB ANDNOT polyA. Of course for some operations (AND, OR) that restriction 
   does not apply. */
   class logic {
   public:
      //! The class constructor preparing all data fields
                        logic(const pointlist&, const pointlist&);
                        ~logic();
      //! Do Benttley-Ottman modified
      void              findCrossingPoints();
      //! Perform logic AND and returns the result in plycol
      bool              AND(pcollection&);
      //! Perform logic ANDNOT and returns the result in plycol
      bool              ANDNOT(pcollection&);
      //! Perform logic OR and returns the result in plycol
      bool              OR(pcollection&);
      //! Prepare #_shape1 and #_shape2 data fields for reuse
      void              reset_visited();
   private:
      //! Convert a polygon with hole to simple polygon
      pointlist*        hole2simple(const pointlist&, const pointlist&);
      //
      void              getShape(pcollection&, polycross::VPoint*);
      //
      void              reorderCross();
      //
      polycross::VPoint* checkCoinciding(const pointlist&, polycross::VPoint*);
      //
      polycross::VPoint* getFirstOutside(const pointlist&, polycross::VPoint*);
      //
      void              cleanupDumped(polycross::VPoint*);
      //! The first input polygon
      const pointlist&        _poly1;
      //! The second input polygon
      const pointlist&        _poly2;
      //! The raw data, corresponding to _poly1, used by all logic methods
      polycross::VPoint*      _shape1;
      //! The raw data, corresponding to _poly2, used by all logic methods
      polycross::VPoint*      _shape2;
      //! Number of crossing points found from the constructor
      unsigned                _crossp;
      polycross::segmentlist* _segl1;
      polycross::segmentlist* _segl2;
      
   };

   //===========================================================================
   class SSegment : public PSegment
   {
   public:
                        SSegment(const TP&, const TP&, int);
      PSegment*         moved() {return _moved;}
   private:
      PSegment*         _moved;
   };

   //===========================================================================
   class stretcher {
   public:
      typedef std::vector<SSegment> SSegments;
                        stretcher(const pointlist&, int);
      pointlist*        execute();
   private:
      const pointlist&        _poly; //! The input polygon
      SSegments               _segl;
   };

   //===========================================================================
   class CrossFix   {
   public:
      //! The class constructor preparing all data fields
                              CrossFix(const pointlist&);
      //! Do Benttley-Ottman modified
      void                    findCrossingPoints();
      //! Get the fixed polygon
      bool                    getFixed(pcollection&);
   protected:
      void                    reset_visited();
      void                    reorderCross();
      polycross::VPoint*      findFirstValid();
      //! The raw data, corresponding to _poly, used by all logic methods
      polycross::VPoint*      _shape;
      //! The input polygon
      const pointlist&        _poly;
      polycross::segmentlist* _segl;
      word                    _crossp;
   };
}
#endif
