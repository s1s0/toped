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
//    Description: polygon logic operations
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <assert.h>
#include <sstream>
#include "logicop.h"
#include "tedat.h"
#include "ttt.h"
#include "outbox.h"

//#define POLYFIX_DEBUG
#ifdef POLYFIX_DEBUG
#define REPORT_POLY_DEBUG(pl, msg) {  printf("=========================== %s ============================\n",msg); \
   polycross::VPoint* centinel = pl;  \
   polycross::VPoint* looper = centinel;  \
   int pno = 1;   \
   do    {  \
      printf("%.2i -%s-> ( %i , %i )\n", pno++, looper->visited() ? "-" : "C", looper->cp()->x(), looper->cp()->y());   \
      looper = looper->next();   \
   } while (centinel != looper);}
#else
#define REPORT_POLY_DEBUG(pl,msg)
#endif

//#define POLYBIND_DEBUG
#ifdef POLYBIND_DEBUG
#define REPORT_POLYBIND_DEBUG(polyObject) \
   printf("=======================================================\n"); \
   for (PointVector::const_iterator CP = polyObject->begin(); CP != polyObject->end(); CP++) \
      printf("( %i , %i )\n", CP->x(), CP->y());
#else
#define REPORT_POLYBIND_DEBUG(polyObject)
#endif

//#define POLYLOOP_DEBUG
#ifdef POLYLOOP_DEBUG
#define PLYDUMP_POINT(p) \
   printf("(%i,%i)\n", p->x(), p->y());
#define PLYDUMP_START \
   printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
#define PLYDUMP_END \
   printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
#else
#define PLYDUMP_START
#define PLYDUMP_POINT(p)
#define PLYDUMP_END
#endif


//-----------------------------------------------------------------------------
// class logic
//-----------------------------------------------------------------------------
/*!This is the place where Bentley-Ottmann algorithm is prepared and invoked.
As a result of it, all crossing points between the two initial polygons are
produced. As a second step, two raw data structures are produced, that replicate
the input polygons, but contain also the crossing points with the required links
between them. These data structures will be used in all subsequently called
methods, implementing the actual logic operations*/
logicop::logic::logic(const PointVector& poly1, const PointVector& poly2) :
                                                _poly1(poly1), _poly2(poly2) {
   _segl1 = DEBUG_NEW polycross::segmentlist(poly1,1,true);
   _segl2 = DEBUG_NEW polycross::segmentlist(poly2,2,true);
   _shape1 = NULL;
   _shape2 = NULL;
}

void logicop::logic::findCrossingPoints()
{
   // create the event queue
   polycross::XQ* _eq = DEBUG_NEW polycross::XQ(*_segl1, *_segl2);
   // BO modified algorithm
   _eq->sweep(false, true);
   unsigned crossp1 = _segl1->normalize(_poly1, true);
   unsigned crossp2 = _segl2->normalize(_poly2, true);
   assert(crossp1 == crossp2);
   _crossp = crossp1;
   if (1 == _crossp)
      throw EXPTNpolyCross("Only one crossing point found. Can't generate polygons");
   delete _eq;
   _shape1 = _segl1->dump_points();
   _shape2 = _segl2->dump_points();
   reorderCross();
   REPORT_POLY_DEBUG(_shape1, "After reorderCross")
   REPORT_POLY_DEBUG(_shape2, "After reorderCross")
}

void logicop::logic::reorderCross()
{
   polycross::VPoint* centinel = _shape1;
   polycross::VPoint* looper = centinel;
   unsigned shape1Num = 0;
   PLYDUMP_START
   do
   {
      shape1Num++;
      PLYDUMP_POINT(looper->cp())
      looper = looper->next();
   } while (centinel != looper);
   PLYDUMP_END
   unsigned loopcount;
   for(loopcount = 0; loopcount < shape1Num; loopcount++)
   {
      // for every non-crossing point which has cross point neighbors and
      // all 3 points coincide
      if (looper->visited() &&
          (!looper->prev()->visited() && !looper->next()->visited()) &&
           (*looper->prev()->cp() == *looper->next()->cp()) )
      {
         looper = looper->checkNreorder(_shape2, false);
      }
      else looper = looper->next();
   }
   _shape1 = looper;

   centinel = _shape2;
   looper = centinel;
   unsigned shape2Num = 0;
   PLYDUMP_START
   do
   {
      shape2Num++;
      PLYDUMP_POINT(looper->cp())
      looper = looper->next();
   } while (centinel != looper);
   PLYDUMP_END
   for(loopcount = 0; loopcount < shape2Num; loopcount++)
   {
      if (looper->visited() &&
          (!looper->prev()->visited() && !looper->next()->visited()) &&
          (*looper->prev()->cp() == *looper->next()->cp()) )
      {
         looper = looper->checkNreorder(_shape1, false);
      }
      else looper = looper->next();
   }
   _shape2 = looper;
}
/*!If more than one logical operation has to be executed over the input shapes
the raw data #_shape1 and #_shape2 can be reused, but has to be recycled beforehand
This method is traversing both fields and invokes VPoint::reset_visited() in
order to reinitialize the CPoint::_visited fields*/
void logicop::logic::reset_visited() {
   polycross::VPoint* centinel = _shape1;
   polycross::VPoint* looper = centinel;
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
   polycross::VPoint* centinel = NULL;
   bool direction = true; /*next*/
   if (0 != _crossp)
   {
      // get first external and non crossing  point
      if ((NULL == (centinel = getFirstOutside(_poly2, _shape1))) &&
           (NULL == (centinel = getFirstOutside(_poly1, _shape2))) )
      {
         assert(false);
      }
   }
   else
   {
      // If there are no crossing points found, this still does not mean
      // that the operation will fail. Polygons might be fully overlapping...
      // Check that an arbitrary point from poly1 is inside poly2 ...
      if      (_shape1->inside(_poly2)) centinel = _shape1;
      // ... if not, check that an arbitrary point from poly2 is inside poly1 ...
      else if (_shape2->inside(_poly1)) centinel = _shape2;
      // ... if not - still insisting - check that the polygons coincides
      else if (NULL == (centinel = checkCoinciding(_poly1, _shape2))) return false;
      // If we've got here means that one of the polygons is completely
      // overlapped by the other one. So we need to return the outer one
      getShape(plycol, centinel); return true;
   }
   //
   assert(centinel);
   polycross::VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         PointVector *shgen = DEBUG_NEW PointVector();
         polycross::VPoint* pickup = collector;
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
   polycross::VPoint* centinel = NULL;
   bool direction;
   if (0 != _crossp)
   {
      // get first external and non crossing  point
      if (NULL == (centinel = getFirstOutside(_poly1, _shape2)))
         if  (NULL == (centinel = getFirstOutside(_poly2, _shape1)))
            assert(false);
         else
            direction = false; /*prev*/
      else
         direction = true; /*next*/
   }
   else
   {
      // No crossing points found, but polygons still might be overlapping...
      // if poly1 is inside poly2, or both are non overlapping ->
      //      resulting shape is null
      // if poly2 is inside poly1, then we have to generate a polygon
      // combining both shapes
      if (_shape2->inside(_poly1))
      {
         pcollection dummyCollection;
         PointVector* respoly = hole2simple(_poly1, _poly2, dummyCollection);
         if (NULL != respoly)
         {
            plycol.push_back(respoly);
            return true;
         }
         else return false;
      }
      else return false;
   }
   //
   assert(centinel);
   polycross::VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         PointVector *shgen = DEBUG_NEW PointVector();
         polycross::VPoint* pickup = collector;
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
bool logicop::logic::OR(pcollection& plycol)
{
   bool direction = true; /*next*/
   pcollection lclcol; // local collection of the resulting shapes
   polycross::VPoint* centinel = NULL;
   if (0 != _crossp)
   {
      // get first external and non crossing  point
      if ((NULL == (centinel = getFirstOutside(_poly2, _shape1))) &&
          (NULL == (centinel = getFirstOutside(_poly1, _shape2))) )
      {
         assert(false);
      }
   }
   else
   {
      // If there are no crossing points found, this still does not mean
      // that the operation will fail. Polygons might be fully overlapping...
      // Check that an arbitrary point from poly1 is inside poly2 ...
      if  (_shape1->inside(_poly2)) centinel = _shape2;
      // ... if not, check that an arbitrary point from poly2 is inside poly1 ...
      else if (_shape2->inside(_poly1)) centinel = _shape1;
      // ... if not - still insisting - check that the polygons coincides
      else if (NULL == (centinel = checkCoinciding(_poly1, _shape2))) return false;
      // If we've got here means that one of the polygons is completely
      // overlapped by the other one. So we need to return the outer one
      getShape(plycol, centinel); return true;
   }
   //
   assert(centinel);
   polycross::VPoint* collector = centinel;
   bool result = false;
   do {
      if (0 == collector->visited())
      {
         PointVector *shgen = DEBUG_NEW PointVector();
         polycross::VPoint* pickup = collector;
         // The first resulting polygon must be the outer polygon
         // That's why the direction for the first only is true
         // The eventual following polygons will be inside the first - so
         // their direction is always false
         direction = (0 == lclcol.size());
         PLYDUMP_START
         do
         {
            pickup = pickup->follower(direction);
            shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
            PLYDUMP_POINT(pickup->cp())
         } while (pickup != collector);
         lclcol.push_back(shgen);
         PLYDUMP_END
         result = true;
      }
      collector = collector->next();
   } while (collector != centinel);
   if (!result) return false;
   // Validate all resulting polygons
   pcollection lclvalidated;
   while (!lclcol.empty())
   {
      PointVector* csh = lclcol.front();
      REPORT_POLYBIND_DEBUG(csh);
      laydata::ValidPoly check(*csh);
      delete csh; lclcol.pop_front();
      if (check.valid())
         lclvalidated.push_back(DEBUG_NEW PointVector(check.getValidated()));
   }
   if (lclvalidated.empty()) return false;
   // Convert all collected shapes to a single normalized polygon
   PointVector* respoly = lclvalidated.front();lclvalidated.pop_front();
   while (0 < lclvalidated.size())
   {
      PointVector* curpolyA = respoly;
      PointVector* curpolyB = lclvalidated.front();
      lclvalidated.pop_front();
      respoly = hole2simple(*curpolyA, *curpolyB, lclvalidated);
      delete curpolyA; delete curpolyB;
      if (NULL == respoly) return false;
   }
   plycol.push_back(respoly);
   return true;
}

void logicop::logic::getShape(pcollection& plycol, polycross::VPoint* centinel)
{
   PointVector *shgen = DEBUG_NEW PointVector();
   polycross::VPoint* vpnt = centinel;
   do {
      shgen->push_back(TP(vpnt->cp()->x(), vpnt->cp()->y()));
      vpnt = vpnt->next();
   }while (centinel != vpnt);
   plycol.push_back(shgen);
}

/**
 * Get the first non crossing point from the init sequence, that lies outside
   the polygon given by plst
 * @param plist represents one of the  original polygons before BOM algo
 * @param init represents the other polygon, but with the cross points inserted
 * @return a point from init that lies entirely outside the polygon described
by plist. If plist overlaps entirely init - return NULL.
 */
polycross::VPoint* logicop::logic::getFirstOutside(const PointVector& plist, polycross::VPoint* init)
{
   polycross::VPoint *cpoint = init;
   do
   {
      if (!cpoint->inside(plist, true)) return cpoint;
      else cpoint = cpoint->next();
   } while (cpoint != init);
   return NULL;
}

polycross::VPoint* logicop::logic::checkCoinciding(const PointVector& plist, polycross::VPoint* init)
{
   polycross::VPoint *cpoint = init;
   do
   {
      if (!cpoint->inside(plist, true)) return NULL;
      else cpoint = cpoint->next();
   } while (cpoint != init);
   return init;
}

//PointVector* logicop::logic::hole2simple(const PointVector& outside, const PointVector& inside, const pcollection& obstructions)
PointVector* logicop::hole2simple(const PointVector& outside, const PointVector& inside, const pcollection& obstructions)
{
   polycross::segmentlist seg1(outside,1,true);
   polycross::segmentlist seg2(inside ,2,true);
   polycross::XQ _eq(seg1, seg2); // create the event queue
   polycross::BindCollection BC;
   try
   {
      _eq.sweep2bind(BC);
   }
   catch (EXPTNpolyCross&) {return NULL;}
   polycross::BindSegment* sbc = BC.getBindSegment(obstructions);
   if (NULL == sbc) return NULL; // i.e. Can't bind those objects together
   //insert 2 bind points and link them
   polycross::BPoint* cpsegA = seg1.insertBindPoint(sbc->poly0seg(), sbc->poly0pnt());
   polycross::BPoint* cpsegB = seg2.insertBindPoint(sbc->poly1seg(), sbc->poly1pnt());
   cpsegA->linkto(cpsegB);
   cpsegB->linkto(cpsegA);
   // normalize the segment lists
   seg1.normalize(outside, true);
   seg2.normalize(inside, true);
   //
   // dump the new polygons in VList terms
   polycross::VPoint* outshape = seg1.dump_points();
   polycross::VPoint*  inshape = seg2.dump_points();
   // traverse and form the resulting shape
   polycross::VPoint* centinel = outshape;
   PointVector *shgen = DEBUG_NEW PointVector();
   bool direction = true; /*next*/
   polycross::VPoint* pickup = centinel;
   polycross::VPoint* prev = centinel->prev();
   bool modify = false;
   do
   {
      shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
      modify = (-1 == prev->visited());
      prev = pickup;
      pickup = pickup->follower(direction, modify);
   } while (pickup != centinel);
   // clean-up dumped points
   cleanupDumped(outshape);
   cleanupDumped(inshape);
   // Validate the resulting polygon
   laydata::ValidPoly check(*shgen);
//   delete shgen;
   if (!check.valid())
   {
      std::ostringstream ost;
      ost << ": Resulting shape is invalid - " << check.failType();
      tell_log(console::MT_ERROR, ost.str());
   }
   else
   {
      if (laydata::shp_OK != check.status())
         *shgen = check.getValidated();
   }
   return shgen;
}

//void logicop::logic::cleanupDumped(polycross::VPoint* centinel)
void logicop::cleanupDumped(polycross::VPoint* centinel)
{
   polycross::VPoint* shape = centinel;
   polycross::VPoint* cpnt;
   do
   {
      cpnt = shape->next();
      delete shape; shape = cpnt;
   }
   while (shape != centinel);
}

logicop::logic::~logic()
{
   if (NULL != _shape1) cleanupDumped(_shape1);
   if (NULL != _shape2) cleanupDumped(_shape2);
   delete _segl1;
   delete _segl2;
}


//-----------------------------------------------------------------------------
// class SSegment (Stretch Segment)
//-----------------------------------------------------------------------------
logicop::SSegment::SSegment(const TP& p1, const TP& p2, int distance) : PSegment(p1,p2)
{
   assert(0 != distance);
   DBline sample(TP(0,0), TP(distance, 0));
   CTM mtrx;
   real rotation = laydata::xangle(p1,p2) + 270.0;
   mtrx.Rotate(rotation);
   mtrx.Translate(p1);
   sample = sample * mtrx;
   _moved = parallel(sample.p2());
}

logicop::SSegment::~SSegment()
{
   delete _moved;
}

//-----------------------------------------------------------------------------
// class stretcher
//-----------------------------------------------------------------------------
/*!*/
logicop::stretcher::stretcher(const PointVector& poly, int bfactor) : _poly(poly)
{
   unsigned plysize = _poly.size();
   _segl.reserve(plysize);
   for (unsigned i = 0; i < plysize; i++)
      _segl.push_back(DEBUG_NEW SSegment(_poly[i],_poly[(i+1)%plysize], bfactor ));
//   _shape1 = NULL;
}

PointVector* logicop::stretcher::execute()
{
   unsigned plysize = _poly.size();
   PointVector* streched = DEBUG_NEW PointVector();
   for (unsigned i = 0; i < plysize; i++)
   {
      TP npnt;
      VERIFY(0 == _segl[i]->moved()->crossP(*(_segl[(i+1)%plysize]->moved()), npnt));
      streched->push_back(npnt);
   }
   return streched;
}

logicop::stretcher::~stretcher()
{
   unsigned segsize = _segl.size();
   for (unsigned i = 0; i < segsize; i++)
      delete _segl[i];
}

//-----------------------------------------------------------------------------
// class CrossFix
//-----------------------------------------------------------------------------
/*!*/
logicop::CrossFix::CrossFix(const PointVector& poly, bool looped) :
                                                   _poly(poly), _looped(looped)
{
   _segl = DEBUG_NEW polycross::segmentlist(poly,1, looped);
   _shape = NULL;

}

void logicop::CrossFix::findCrossingPoints()
{
   // create the event queue
   polycross::XQ* _eq = DEBUG_NEW polycross::XQ(*_segl, _looped);
   // BO modified algorithm
   _eq->sweep(true, _looped);
   delete _eq;
   _crossp = _segl->normalize(_poly, _looped);
//   if ((0 == _crossp) || (!_looped)) return;
   if (0 == _crossp) return;
   _shape = _segl->dump_points(_looped);
   REPORT_POLY_DEBUG(_shape, "Line 575")
   reorderCross();
   cleanRedundant();
   REPORT_POLY_DEBUG(_shape, "Line 578")
   countCross();
}

void logicop::CrossFix::countCross()
{
   polycross::VPoint* centinel = _shape;
   polycross::VPoint* looper = centinel;
   _crossp = 0;
   do
   {
      if (0 == looper->visited()) _crossp++;
      looper = looper->next();
   } while (centinel != looper);
}

void logicop::CrossFix::reorderCross()
{
   polycross::VPoint* centinel = _shape;
   polycross::VPoint* looper = centinel;
   unsigned shapeNum = 0;
   do
   {
      shapeNum++;
      looper = looper->next();
   } while (centinel != looper);
   unsigned loopcount;
   for(loopcount = 0; loopcount < shapeNum; loopcount++)
   {
      // for every non-crossing point which has cross point neighbors and
      // all 3 points coincide
      if (looper->visited() &&
          (!looper->prev()->visited() && !looper->next()->visited()) &&
           (*looper->prev()->cp() == *looper->next()->cp()) )
      {
         looper = looper->checkNreorder(_shape, true);
      }
      else looper = looper->next();
   }
   _shape = looper;
}

void logicop::CrossFix::cleanRedundant()
{
   polycross::VPoint* centinel = _shape;
   polycross::VPoint* looper = centinel;
   unsigned shapeNum = 0;
   do
   {
      shapeNum++;
      looper = looper->next();
   } while (centinel != looper);
   unsigned loopcount;
   for(loopcount = 0; loopcount < shapeNum; loopcount++)
   {
      // for every non-crossing point which coincides with a neighboring cross point
      if (
           (looper->visited() &&  (!looper->prev()->visited()) &&
           (*looper->prev()->cp() == *looper->cp()))                 ||
           (looper->visited() &&  (!looper->next()->visited()) &&
           (*looper->next()->cp() == *looper->cp()))
         )
      {
         looper = looper->checkRedundantCross();
      }
      else looper = looper->next();
   }
   _shape = looper;
}


void logicop::CrossFix::reset_visited()
{
   polycross::VPoint* centinel = _shape;
   polycross::VPoint* looper = centinel;
   do {
      looper->reset_visited();
      looper = looper->next();
   }  while (centinel != looper);
}

bool logicop::CrossFix::generate(pcollection& plycol, real bfactor)
{
   // the general idea behind the code below:
   // The list of points resulted from the BO algo shall be traversed to
   // generate the new polygon(s). The trick is to filter-out properly
   // redundant points (shapes). The "usual" alternative traversing although
   // almost working, isn't quite appropriate here - the problem is to find a
   // proper starting point (see the comment in the previous versions of
   // this file).
   // Another approach has been used here that seem to cover all the
   // shrink/bloat cases and on top of this is quicker and much simpler.
   // The algorithm traverses the points produced by BO-modified and creates
   // ALL shapes. It doesn't filters out anything. ALL possible polygons.
   // Next step is checking every new polygons
   // - under-sizing (shrink) Check the polygons for orientation. If it's normally
   // oriented (anti-clockwise) - fine. If it isn't - the polygon should be
   // deleted. Having in mind that all input polygons were normally oriented
   //  it means that those parts are inside out and must be removed. When the
   // input polygon is completely inside out it should disappear altogether, but
   // this case should be cough before this algo is invoked.
   // - over-sized (bloat) All resulting polygons will be normally oriented BUT
   // some of the polygons could be overlapped entirely by other polygons. The
   // overlapped fellas should be removed.
   if (0 == _crossp) return false;
   polycross::VPoint* centinel = _shape;
   // Get a non-crossing starting point
   while (0 == centinel->visited()) centinel = centinel->next();
   // traverse the resulting points recursively to get all the polygons
   traverseMulti(centinel, plycol);
//   if (1 == plycol.size()) return true;
   assert( plycol.size() > 1 );
   if (0 > bfactor)
   {  // undersize case
      // remove the invalid polygons (negative orientation)
      pcollection::iterator CI = plycol.begin();
      while (CI != plycol.end())
      {
         if (0ll >= polyarea(**CI))
         {
            delete (*CI);
            CI = plycol.erase(CI);
         }
         else CI++;
      }
   }
   else
   {  // oversize case
      // Oversizing single polygon shall result in a single polygon.
      // Find the polygon with the biggest area. The rest should be removed
      // As a sanity check (not implemented!)- the biggest polygon should
      // overlap entirely all the rest
      word the_one = -1;
      word current = 0;
      int8b biggest_area = 0ll;
      for (pcollection::const_iterator CI = plycol.begin(); CI != plycol.end(); CI++)
      {
         int8b cur_area = polyarea(**CI);
         if (biggest_area < cur_area)
         {
            biggest_area = cur_area;
            the_one = current;
         }
         current++;
      }
      assert(the_one != -1);
      // remove all except the_one
      current = 0;
      pcollection::iterator CI = plycol.begin();
      while (CI != plycol.end())
      {
         if (current != the_one)
         {
            delete (*CI);
            CI = plycol.erase(CI);
         }
         else CI++;
         current++;
      }
   }
   if (0 == plycol.size()) return false;
   else return true;
}

bool logicop::CrossFix::recoverPoly(pcollection& plycol)
{
   if (0 == _crossp) return false;
   // get a random non-crossing point
   polycross::VPoint* centinel = _shape;
   while (0 == centinel->visited()) centinel = centinel->next();
   // Get the top right non-crossing point - the idea is to make sure that it is
   // not internal to the shape
   polycross::VPoint* topRight = centinel;
   polycross::VPoint* collector   = centinel;
   do
   {
      collector = collector->next();
      if ( (1 == collector->visited()) && (polycross::xyorder(collector->cp(), topRight->cp()) > 0) )
         topRight = collector->next();
   } while (collector != centinel);
   centinel = topRight;

   pcollection lclcol; // local collection of the resulting shapes
   traverseRecoverPoly(centinel, lclcol, true);
   if ( lclcol.empty() ) return false; // i.e. no generated polygons
   // Validate all resulting polygons
   pcollection lclvalidated;
   while (!lclcol.empty())
   {
      PointVector* csh = lclcol.front();
      REPORT_POLYBIND_DEBUG(csh);
      laydata::ValidPoly check(*csh);
      delete csh; lclcol.pop_front();
      if (check.valid())
         lclvalidated.push_back(DEBUG_NEW PointVector(check.getValidated()));
   }
   if (lclvalidated.empty()) return false;
   // The last part - we have to eventually combine resulting shapes - in case
   // some of them lies inside the top one

   PointVector* respoly = lclvalidated.front();lclvalidated.pop_front();
   REPORT_POLYBIND_DEBUG(respoly);
   while (0 < lclvalidated.size())
   {
      PointVector* curpolyA = respoly;
      PointVector* curpolyB = lclvalidated.front();
      REPORT_POLYBIND_DEBUG(curpolyB);
      lclvalidated.pop_front();
      if (checkInside(*curpolyA, *curpolyB))
      {
//         pcollection boza;
         PointVector* dummy = hole2simple(*curpolyA, *curpolyB, /*boza */lclvalidated);
         if (NULL != dummy)
         {
            delete curpolyA; delete curpolyB;
            respoly = dummy;
         }
         else
            plycol.push_back(curpolyB);
      }
      else
         plycol.push_back(curpolyB);
   }
   plycol.push_back(respoly);
   return true;
}

bool logicop::CrossFix::recoverWire(pcollection& plycol)
{
   if (0 == _crossp) return false;
   polycross::VPoint* centinel = _shape;
   // Get a non-crossing starting point
   while (0 == centinel->visited()) centinel = centinel->next();
   // traverse the resulting points to get the wires
   traverseRecoverWire(centinel, plycol);
   // There is a work still to be done here. We must validate the resulting
   // wires - they could be "short-ended" hence must be converted to polygons.
   // !The above must be done in the calling function!
   return true;
}

void logicop::CrossFix::traverseMulti(polycross::VPoint* const centinel, pcollection& plycol)
{
   bool direction = true; /*next*/
   PointVector *shgen = DEBUG_NEW PointVector();
   // always push the entry point
   shgen->push_back(TP(centinel->cp()->x(), centinel->cp()->y()));
   polycross::VPoint* collector = centinel->next();
   while ( *(collector->cp()) != *(centinel->cp()) )
   {
      shgen->push_back(TP(collector->cp()->x(), collector->cp()->y()));
      if (0 == collector->visited())
      {
         traverseMulti(collector, plycol);
      }
      collector = collector->follower(direction);
   }
   plycol.push_back(shgen);
}

void logicop::CrossFix::traverseRecoverPoly(polycross::VPoint* const centinel, pcollection& plycol, bool direction)
{
   PointVector *shgen = DEBUG_NEW PointVector();
   polycross::VPoint* collector = centinel;
   do
   {
      shgen->push_back(TP(collector->cp()->x(), collector->cp()->y()));
      collector = collector->follower(direction);
      if (0 == collector->visited())
      {
         traverseRecoverPoly(collector, plycol, !direction);
      }
   }
   while ( collector->cp() != centinel->cp() );
   plycol.push_front(shgen);
}

void logicop::CrossFix::traverseRecoverWire(polycross::VPoint* const centinel, pcollection& plycol)
{
   PointVector *shgen = DEBUG_NEW PointVector();
   polycross::VPoint* collector = centinel;
   do
   {
      shgen->push_back(TP(collector->cp()->x(), collector->cp()->y()));
      collector = collector->next();
      if (0 == collector->visited())
      {
         shgen->push_back(TP(collector->cp()->x(), collector->cp()->y()));
         plycol.push_front(shgen);
         shgen = DEBUG_NEW PointVector();
      }
   }
   while ( collector->cp() != centinel->cp() );
   plycol.push_front(shgen);
}

bool logicop::CrossFix::checkInside(const PointVector& outside, const PointVector& inside)
{
   bool shInside = true;
   for (PointVector::const_iterator CP = inside.begin(); CP != inside.end(); CP++)
   {
      shInside &= polycross::pointInside(&(*CP), outside, false);
   }
   return shInside;
}

logicop::CrossFix::~CrossFix()
{
   delete _segl;
   if (NULL == _shape) return;
   polycross::VPoint* shape = _shape;
   do
   {
      polycross::VPoint* cpnt = shape->next();
      delete shape; shape = cpnt;
   }
   while (shape != _shape);
}
