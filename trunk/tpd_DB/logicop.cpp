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
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"

//#define POLYFIX_DEBUG
#ifdef POLYFIX_DEBUG
#define REPORT_POLY_DEBUG {  printf("=======================================================\n"); \
   polycross::VPoint* centinel = _shape;  \
   polycross::VPoint* looper = centinel;  \
   int pno = 1;   \
   do    {  \
      printf("%.2i -%s-> ( %i , %i )\n", pno++, looper->visited() ? "-" : "C", looper->cp()->x(), looper->cp()->y());   \
      looper = looper->next();   \
   } while (centinel != looper);}
#else
#define REPORT_POLY_DEBUG
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
logicop::logic::logic(const pointlist& poly1, const pointlist& poly2) :
                                                _poly1(poly1), _poly2(poly2) {
   _segl1 = DEBUG_NEW polycross::segmentlist(poly1,1);
   _segl2 = DEBUG_NEW polycross::segmentlist(poly2,2);
   _shape1 = NULL;
   _shape2 = NULL;
}
   
void logicop::logic::findCrossingPoints()
{
   // create the event queue
   polycross::XQ* _eq = DEBUG_NEW polycross::XQ(*_segl1, *_segl2);
   // BO modified algorithm
   _eq->sweep(false);
   unsigned crossp1 = _segl1->normalize(_poly1);
   unsigned crossp2 = _segl2->normalize(_poly2);
   assert(crossp1 == crossp2);
   _crossp = crossp1;
   if (1 == _crossp)
      throw EXPTNpolyCross("Only one crossing point found. Can't generate polygons");
   delete _eq;
   _shape1 = _segl1->dump_points();
   _shape2 = _segl2->dump_points();
   reorderCross();
}

void logicop::logic::reorderCross()
{
   polycross::VPoint* centinel = _shape1;
   polycross::VPoint* looper = centinel;
   unsigned shape1Num = 0;
   do
   {
      shape1Num++;
      looper = looper->next();
   } while (centinel != looper);
   unsigned loopcount;
   for(loopcount = 0; loopcount < shape1Num; loopcount++)
   {
      // for every non-crossing point which has cross point neightbors and
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
   do
   {
      shape2Num++;
      looper = looper->next();
   } while (centinel != looper);

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
/*!If more than one logical operatoin has to be executed over the input shapes
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
      // Check that an arbitraty point from poly1 is inside poly2 ...
      if      (_shape1->inside(_poly2)) centinel = _shape1;
      // ... if not, check that an arbitrary point from poly2 is inside poly1 ...
      else if (_shape2->inside(_poly1)) centinel = _shape2;
      // ... if not - still insysting - check that the polygons coincides
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
         pointlist *shgen = DEBUG_NEW pointlist();
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
      if (_shape2->inside(_poly1)) {
         plycol.push_back(hole2simple(_poly1, _poly2));
         return true;
      }
      else return false;
   }
   //
   assert(centinel);
   polycross::VPoint* collector = centinel;
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = DEBUG_NEW pointlist();
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
bool logicop::logic::OR(pcollection& plycol) {
   bool result = false;
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
      // Check that an arbitraty point from poly1 is inside poly2 ...
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
   do {
      if (0 == collector->visited()) {
         pointlist *shgen = DEBUG_NEW pointlist();
         polycross::VPoint* pickup = collector;
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
   // Validate all resulting polygons
   pcollection lclvalidated;
   while (!lclcol.empty())
   {
      pointlist* csh = lclcol.front();
      laydata::valid_poly check(*csh);
      delete csh; lclcol.pop_front();
      if (check.valid())
         lclvalidated.push_back(DEBUG_NEW pointlist(check.get_validated()));
   }
   if (lclvalidated.empty()) return false;
   // Convert all collected shapes to a single normalized polygon
   pointlist* respoly = lclvalidated.front();lclvalidated.pop_front();
   while (0 < lclvalidated.size()) {
      pointlist* curpolyA = respoly;
      pointlist* curpolyB = lclvalidated.front();
      respoly = hole2simple(*curpolyA, *curpolyB);
      lclvalidated.pop_front();
      delete curpolyA; delete curpolyB;
   }
   plycol.push_back(respoly);
   return result;
}

void logicop::logic::getShape(pcollection& plycol, polycross::VPoint* centinel)
{
   pointlist *shgen = DEBUG_NEW pointlist();
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
polycross::VPoint* logicop::logic::getFirstOutside(const pointlist& plist, polycross::VPoint* init)
{
   polycross::VPoint *cpoint = init;
   do
   {
      if (!cpoint->inside(plist, true)) return cpoint;
      else cpoint = cpoint->next();
   } while (cpoint != init);
   return NULL;
}

polycross::VPoint* logicop::logic::checkCoinciding(const pointlist& plist, polycross::VPoint* init)
{
   polycross::VPoint *cpoint = init;
   do
   {
      if (!cpoint->inside(plist, true)) return NULL;
      else cpoint = cpoint->next();
   } while (cpoint != init);
   return init;
}

pointlist* logicop::logic::hole2simple(const pointlist& outside, const pointlist& inside) {
   polycross::segmentlist _seg1(outside,1);
   polycross::segmentlist _seg2(inside  ,2);
   polycross::XQ _eq(_seg1, _seg2); // create the event queue
   polycross::BindCollection BC;
   
   _eq.sweep2bind(BC);
   polycross::BindSegment* sbc = BC.get_highest();
   //insert 2 bind points and link them
   polycross::BPoint* cpsegA = _seg1.insertBindPoint(sbc->poly0seg(), sbc->poly0pnt());
   polycross::BPoint* cpsegB = _seg2.insertBindPoint(sbc->poly1seg(), sbc->poly1pnt());
   cpsegA->linkto(cpsegB);
   cpsegB->linkto(cpsegA);
   // normalize the segment lists
   _seg1.normalize(outside);
   _seg2.normalize(inside);
   //
   // dump the new polygons in VList terms
   polycross::VPoint* outshape = _seg1.dump_points();
   polycross::VPoint*  inshape = _seg2.dump_points();
   // traverse and form the resulting shape
   polycross::VPoint* centinel = outshape;
   pointlist *shgen = DEBUG_NEW pointlist();
   bool direction = true; /*next*/
   polycross::VPoint* pickup = centinel;
   polycross::VPoint* prev = centinel->prev();
   bool modify = false;
   do {
      shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
      modify = (-1 == prev->visited());
      prev = pickup;
      pickup = pickup->follower(direction, modify);
   } while (pickup != centinel);
   // clean-up dumped points
   cleanupDumped(outshape);
   cleanupDumped(inshape);
   // Validate the resulting polygon
   laydata::valid_poly check(*shgen);
//   delete shgen;
   if (!check.valid()) {
      std::ostringstream ost;
      ost << ": Resulting shape is invalid - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
   }
   else {
      if (laydata::shp_OK != check.status())
         *shgen = check.get_validated();
   }
   return shgen;
}

void logicop::logic::cleanupDumped(polycross::VPoint* centinel)
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


//-----------------------------------------------------------------------------
// class stretcher
//-----------------------------------------------------------------------------
/*!*/
logicop::stretcher::stretcher(const pointlist& poly, int bfactor) : _poly(poly)
{
   unsigned plysize = _poly.size();
   _segl.reserve(plysize);
   for (unsigned i = 0; i < plysize; i++)
      _segl.push_back(SSegment(_poly[i],_poly[(i+1)%plysize], bfactor ));
//   _shape1 = NULL;
}

pointlist* logicop::stretcher::execute()
{
   unsigned plysize = _poly.size();
   pointlist* streched = DEBUG_NEW pointlist();
   for (unsigned i = 0; i < plysize; i++)
   {
      TP npnt;
      VERIFY(0 == _segl[i].moved()->crossP(*(_segl[(i+1)%plysize].moved()), npnt));
      streched->push_back(npnt);
   }
   return streched;
}

//-----------------------------------------------------------------------------
// class CrossFix
//-----------------------------------------------------------------------------
/*!*/
logicop::CrossFix::CrossFix(const pointlist& poly) : _poly(poly)
{
   _segl = DEBUG_NEW polycross::segmentlist(poly,1);
   _shape = NULL;

}

void logicop::CrossFix::findCrossingPoints()
{
   // create the event queue
   polycross::XQ* _eq = DEBUG_NEW polycross::XQ(*_segl);
   // BO modified algorithm
   _eq->sweep(true);
   delete _eq;
   _crossp = _segl->normalize(_poly);
   if (0 == _crossp) return;
   _shape = _segl->dump_points();
   REPORT_POLY_DEBUG
   reorderCross();
   cleanRedundant();
   REPORT_POLY_DEBUG
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
      // for every non-crossing point which has cross point neightbors and
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
      // for every non-crossing point which coincides with a neightboring cross point
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

polycross::VPoint* logicop::CrossFix::findFirstValid()
{
   assert(_crossp);
   assert(_shape);
   polycross::VPoint* centinel = _shape;
   polycross::VPoint* collector = centinel;
   bool direction = true; /*next*/

   // the general idea behind the code below:
   // The list of points resulted from the BO algo shall be traversed to
   // generate the new polygon(s). It is not obvious however which part of the
   // point list shall be discarded and which one shall be used in the new polygon.
   // If we can determine a single point that is (for sure) in or out - then it's
   // easy - we can use the "usual" alternative traversing using the crossing
   // points as a switch markers. It appears though that it's not obvious how to
   // to find such point - there are a number of blow/shrink cases. Here is the idea:
   // Take an arbitraty starting point.
   // Generate the first polygon and check whether it is winding clockwise (negative area).
   // If it is - skip it and start the traversing from the point following the crossing point.
   // If it isn't - great - we've got a proper starting point - so the traversing should
   // start from there.
   // It is essential, that the traversing here and in getFixed procedure is absolutely
   // the same.
   // This certainly is not the most optimal algo, but seems to be easier to understand
   // and maintain. Once the function is fully working - it could be optimized.
   do {
      if (0 == collector->visited())
      { // crossing point found
         pointlist *shgen = DEBUG_NEW pointlist();
         polycross::VPoint* pickup = collector;
         do {
            pickup = pickup->follower(direction, true);
            shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
         } while (pickup != collector);
         reset_visited();
         // get the polygon area ...
         real area = 0;
         word size = shgen->size();
         word i,j;
         for (i = 0, j = 1; i < size; i++, j = (j+1) % size)
            area += real((*shgen)[i].x()) * real((*shgen)[j].y()) -
                  real((*shgen)[j].x()) * real((*shgen)[i].y());
         // ... and from there - the polygon orientation
         if (area >= 0)
         {
            // pick-up the first non-crossing point after the crossing one
            centinel = collector;
            do centinel = centinel->prev();
            while (0 == centinel->visited());
         }
         return centinel;
      }
      collector = collector->prev();
   } while (collector != centinel);

   return NULL;
}


bool logicop::CrossFix::getFixed(pcollection& plycol)
{
   bool result = false;
   polycross::VPoint* centinel = NULL;
   bool direction = true; /*next*/
   if (0 == _crossp) return false;
   else  centinel = findFirstValid();
   //
   assert(centinel);
   polycross::VPoint* collector = centinel;
   bool chdirection = true;
   // The "standard" traversing of the BO output points.
   // Closed polygons will be picked-up alternatively - one to keep, one to skip.
   // The trick is to get the starting point right - see the notes in findFirstValid
   do {
      if (0 == collector->visited())
      { // crossing point found
         pointlist *shgen = DEBUG_NEW pointlist();
         polycross::VPoint* pickup = collector;
         do {
            pickup = pickup->follower(direction, chdirection);
            shgen->push_back(TP(pickup->cp()->x(), pickup->cp()->y()));
         } while (pickup != collector);
         plycol.push_back(shgen);
         result = true;
      }
      collector = collector->prev();
   } while (collector != centinel);
   return result;
}

bool logicop::CrossFix::generate(pcollection& plycol)
{
   if (0 == _crossp) return false;
   polycross::VPoint* centinel = _shape;
   while (0 == centinel->visited()) centinel = centinel->next();
   traverseOne(centinel, plycol);
   return true;
}

pointlist* logicop::CrossFix::traverseOne(polycross::VPoint* const centinel, pcollection& plycol)
{
   bool direction = true; /*next*/
   pointlist *shgen = DEBUG_NEW pointlist();
   // always push the entry point
   shgen->push_back(TP(centinel->cp()->x(), centinel->cp()->y()));
   polycross::VPoint* collector = centinel->next();
   while (collector != centinel);
   {
      shgen->push_back(TP(collector->cp()->x(), collector->cp()->y()));
      if (0 == collector->visited())
      {
         traverseOne(collector, plycol);
         collector = centinel->follower(direction, false);
      }
      else collector = collector->next();
   }
   plycol.push_back(shgen);
}
