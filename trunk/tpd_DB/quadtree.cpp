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
//        Created: Sun Mar 07 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Database structure in the memory - clipping
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <iostream>
#include "quadtree.h"
#include "viewprop.h"
#include "tenderer.h"
#include "outbox.h"

//-----------------------------------------------------------------------------
// class QuadTree
//-----------------------------------------------------------------------------

/*! The main constructor of the class*/
laydata::QuadTree::QuadTree() : _overlap(DEFAULT_OVL_BOX){
   _quads[0] = _quads[1] = _quads[2] = _quads[3] = NULL;
   _first = NULL;_invalid = false;
}

/*! Used for reading the QuadTree from the TDT file. A new shape is added to
the tree using the put() method. Entire tree is recreated when there is no more
data to read using resort() method.*/
laydata::QuadTree::QuadTree(TEDfile* const tedfile) : _overlap(DEFAULT_OVL_BOX)
{
   _quads[0] = _quads[1] = _quads[2] = _quads[3] = NULL;
   _first = NULL;_invalid = false;
   byte recordtype;
   if       ((0 == tedfile->revision()) && (6 == tedfile->subrevision()))
   {
      while (tedf_LAYEREND != (recordtype = tedfile->getByte()))
      {
         switch (recordtype)
         {
            case  tedf_CELLREF: put(DEBUG_NEW TdtCellRef(tedfile));break;
            case tedf_CELLAREF: put(DEBUG_NEW TdtCellAref(tedfile));break;
            //--------------------------------------------------
            default: throw EXPTNreadTDT("Unexpected record type");
         }
      }
   }
   else
   {
      while (tedf_REFSEND != (recordtype = tedfile->getByte()))
      {
         switch (recordtype)
         {
            case  tedf_CELLREF: put(DEBUG_NEW TdtCellRef(tedfile));break;
            case tedf_CELLAREF: put(DEBUG_NEW TdtCellAref(tedfile));break;
            //--------------------------------------------------
            default: throw EXPTNreadTDT("Unexpected record type");
         }
      }
   }

   resort();
}

/*! Add a single layout object shape into the QuadTree.
It first checks whether or not another layout object is already placed into this
QuadTree. If not it simply links the shape and exits. If another object is already
here, then the new overlap area is calculated. \n In case the new area is the same as
the existing one, then the object might be (possibly) fitted into one of the children
QuadTree. To check this fitintree() method is called. If this is unsuccessful, just
then the layout object is linked to this QuadTree. \n If the new overlapping area is
bigger that the existing one, then the layout object is linked to the current QuadTree
after what the current QuadTree as well as its successors has to be rebuild using
resort().\n The method might be called recursively via fitintree() method.
*/
void laydata::QuadTree::add(TdtData* shape)
{
   DBbox shovl(shape->overlap());
   if (empty()) {
   // first shape in the container
      _overlap = shovl;
      _first = shape;shape->nextis(NULL);
   }
   else {
      // save the old overlap
      DBbox oldovl = _overlap;
      // calculate the new container overlap
      _overlap.overlap(shovl);
      int8b areaold = oldovl.boxarea();
      int8b areanew = _overlap.boxarea();
// The equation below produce problems with severe consequences.
// It seems to be because of the type of the conversion
//      if (oldovl.area() == _overlap->area()) {
      if (areaold == areanew)
      {
         // if the overlapping box hasn't changed,
         // try to fit the shape into subtree
         if ((areanew <= 4ll * shovl.boxarea()) || !fitintree(shape))
         {
            // shape doesn't fit into the subtree, so place it here
            shape->nextis(_first); _first = shape;
         }
      }
      else { // the overlapping box has had blown-up
         shape->nextis(_first); _first = shape;
         resort(); // re-sort the entire tree
      }
   }
}

/*! Checks whether a single layout object shape will fit into one of the
children's QuadTree. It calls add() and returns success if the new layout object
fits entirely into one of the possible subtrees or if it blows up its
overlapping area not more than 10%. Returns false if the shape does not fit
anywhere - means it should be placed higher into the QuadTree structure.\n
The method might be called recursively via the add() method.
*/
bool laydata::QuadTree::fitintree(TdtData* shape)
{
   DBbox shovl = shape->overlap();
   int8b clipedarea[4];
   // check the clipping to see in witch region to place the shape
   for (byte i = 0; i < 4 ; i++) {
      DBbox subbox = _overlap.getcorner(i);
      clipedarea[i] = subbox.cliparea(shovl,true);
      if (-1ll == clipedarea[i]) {//entirely inside the area
         if (!_quads[i]) _quads[i] = DEBUG_NEW QuadTree();
         _quads[i]->add(shape);
         return true;
      }
   }
   // if we got to this point - means that the shape does not fit
   // entirely inside neither of the four sub-areas.
   // It is a decision time then
   byte candidate = biggest(clipedarea);
   // now calculate the eventual new overlapping box
   DBbox newovl = _overlap.getcorner(candidate);
   newovl.overlap(shovl);
   // if the max area of the candidate does not blow more than 10% -
   // then seems to be OK to get it
   if ((40ll * newovl.boxarea()) < (11ll * _overlap.boxarea()))
   {
      if (!_quads[candidate]) _quads[candidate] = DEBUG_NEW QuadTree();
      _quads[candidate]->add(shape);
      return true;
   }
   return false; // shape can not be fit into any subtree
}

/*! Checks whether a single layout object shape will fit into one of the
children's QuadTree. Returns the index of the child QuadTree which fits
the shape or -1 otherwise.
*/
int laydata::QuadTree::fitsubtree(const DBbox& shovl, DBbox* maxsubbox )
{
   int8b clipedarea[4];
   // check the clipping to see in witch region to place the shape
   for (byte i = 0; i < 4 ; i++) {
      clipedarea[i] = maxsubbox[i].cliparea(shovl,true);
      if (-1ll == clipedarea[i]) {//entirely inside the area
         return i;
      }
   }
//   assert(shovl.area() == (clipedarea[0] + clipedarea[1] + clipedarea[3] + clipedarea[3]));
   // if we got to this point - means that the shape does not fit
   // entirely inside neither of the four sub-areas.
   // It is a decision time then
   byte candidate = biggest(clipedarea);
   // now calculate the eventual new overlaping box
   DBbox newovl(maxsubbox[candidate]);
   newovl.overlap(shovl);
   // if the max area of the candidate does not blow more than 10% -
   // then seems to be OK to get it
   if ((40ll * newovl.boxarea()) < (11ll * _overlap.boxarea()))
   {
      return candidate;
   }
   return -1; // shape can not be fit into any subtree
}

/*! Build a new QuadTree structure for the TdtData in the inlist. The method is
using the existing _overlap variable. For every layout shape fitinsubtree()
method is called in a try to put the data in the child QuadTree. At the end
the method is called for every of the child structures.
*/
void laydata::QuadTree::sort(ShapeList& inlist)
{
   // if the input list is empty - nothing to do!
   if (0 == inlist.size()) return;
   ShapeList::iterator DI = inlist.begin();
   // if the list contains only one component - link it and run away
   if (1 == inlist.size())
   {
      (*DI)->nextis(NULL); _first = *DI;
      return;
   }
   byte i;
   // the overlapping box of the current shape
//   DBbox shovl(TP(0,0));
   DBbox shovl = DEFAULT_OVL_BOX;
   // the maximum possible overlapping boxes of the 4 children
   DBbox maxsubbox[4] = {DEFAULT_OVL_BOX, DEFAULT_OVL_BOX,
                         DEFAULT_OVL_BOX, DEFAULT_OVL_BOX};
   for (i = 0; i < 4; i++) maxsubbox[i] = _overlap.getcorner(i);
   // the sub-lists data will be sorted in
   ShapeList sublist[4];
   // which is the child where current shape fits
   int fitinsubbox;
   // initialize the iterator
   int8b sharea, totalarea = _overlap.boxarea();
   while (inlist.end() != DI)
   {
      // get the overlap of the current shape
      shovl = (*DI)->overlap();
      sharea = shovl.boxarea();
      // Check it fits in some of the children
      if ((totalarea <= 4ll * sharea) ||
                        (-1 == (fitinsubbox = fitsubtree(shovl, maxsubbox))))
      {
         // no fit. The shape is sorted in the current tree
         (*DI)->nextis(_first); _first = *DI;
      }
      else
      {
         // fits in sub-tree fitinsubbox
         sublist[fitinsubbox].push_back(*DI);
         // check this child already exists
         if (_quads[fitinsubbox])  // yes ?
            _quads[fitinsubbox]->_overlap.overlap(shovl);
         else
         {
            // create the child, initialize the overlapping box
            _quads[fitinsubbox] = DEBUG_NEW QuadTree();
            _quads[fitinsubbox]->_overlap = shovl;
         }
      }
      // in all cases get rid of the current shape pointer. It is already sorted
      DI = inlist.erase(DI);
   }
   // at this point inlist MUST be empty - split over max 4+this quadTrees
   // now go and sort the children - if any
   for(i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->sort(sublist[i]);

}

/*! Removes marked shapes from the quadtree without deleting them. The removed
    shapes are still listed in the _shapesel container of the current cell.
    The new overlapping box is calculated during this process and the method
    returns true to notify its parent that the overlapping box has changed and
    resort has to be initiated on the upper levels of the tree. The tree is
    processed bottom-up, i.e. the method is recursively called first for child
    QuadTree structures.\n
    On the top level the method is called by two TdtCell methods:
    delete_selected and move selected. The difference is that for delete,
    partially selected shapes are ignored and not processed.\n
    Fully selected shapes are always marked as sh_deleted. When move operation is
    going on they will be re-marked afterwards to sh_selected by the move(copy)
    virtual methods of TdtData.
 */
bool laydata::QuadTree::delete_marked(SH_STATUS stat, bool partselect) {
   assert(!((stat != sh_selected) && (partselect == true)));
   // Create and initialize a variable "to be sorted"
   bool _2B_sorted = false;
   // save the old overlap, and initialize the new one
   DBbox oldovl = _overlap;
   _overlap = DEFAULT_OVL_BOX;
   // deleteing sequence is bottom-up, so start from the children
   for (byte i = 0; i < 4; i++)
      if (_quads[i]) {
         _2B_sorted |= _quads[i]->delete_marked(stat, partselect);
         // check that there is still something left in the child QuadTree
         if (_quads[i]->empty()) {
            delete _quads[i]; _quads[i] = NULL;
         }
         else update_overlap(_quads[i]->overlap());
      }
   // prepare a pair of pointers to TdtData
   TdtData* wds = _first;
   TdtData* wdsP = NULL;
   // loop all TdtData in the current tree and check they are selected
   while (wds)
      // if selected ...
      if ((stat == wds->status()) ||
                                (partselect && (sh_partsel == wds->status()))) {
         // mark the fully selected shapes as deleted
         if (stat == wds->status()) wds->set_status(sh_deleted);
         // Unlink the marked shape from the list
         if (wdsP) { // if this is not the first shape
            wdsP->nextis(wds->next()); wds->nextis(NULL);
            wds = wdsP->next();
         }
         else { // Special attention when this is the first shape
            _first = wds->next(); wds->nextis(NULL);
            wds = _first;
         }
      }
      // If not selected, or partially selected but partselect == false,
      // update the overlapping box, and move further
      else {
         update_overlap(wds->overlap());
         wdsP = wds; wds = wds->next();
      }
   // If _overlap is still NULL here -> means the placeholder is empty. Will
   // be deleted by the parent
   if (empty()) _invalid = true;
   else
   {
      //Now if the overlapping rectangles differ, then invalidate
      //the current QuadTree
      int8b areaold = oldovl.boxarea();
      int8b areanew = _overlap.boxarea();
      if (areaold != areanew) _invalid = true;
   }
   return _2B_sorted |= _invalid;
}

/*!Cut with polygon is pretty expensive operation and despite the fact that it
is executed over selected shapes only, there is no guarantee that the user will
not do select_all() and then polycut(), and of course nobody can trust the user.
So this method is trying to minimize the calculations by executing cutpoly only
on the shapes that overlap somehow with the cutting polygon */
void laydata::QuadTree::cutpoly_selected(pointlist& plst, DBbox& cut_overlap,
                                                           ShapeList** decure) {
   // check the entire holder for clipping...
   if (0ll == cut_overlap.cliparea(_overlap)) return;
   // now start traversing the shapes in the current horlder one by one
   TdtData* wdt = _first;
   while(wdt) {
      // for fully selected shpes if they overlap with the cutting polygon
      if ((sh_selected == wdt->status()) &&
                                    (0ll != cut_overlap.cliparea(wdt->overlap())))
         // go and clip it
         wdt->polycut(plst, decure);
      wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->cutpoly_selected(plst, cut_overlap, decure);

}

/*!*/
laydata::TdtData* laydata::QuadTree::merge_selected(TdtData*& shapeRef) {
   laydata::TdtData* mergeres = NULL;
   DBbox overlapRef = shapeRef->overlap();
   // check the entire holder for clipping...
   if (0ll == overlapRef.cliparea(_overlap)) return NULL;
   // now start traversing the shapes in the current horlder one by one
   TdtData* wdt = _first;
   while(wdt) {
      // for fully selected shapes if they overlap with the reference
      // and this is not the same shape as the reference
      if ((wdt != shapeRef) &&
          ((sh_selected == wdt->status()) || (sh_merged == wdt->status())) &&
          (0ll != overlapRef.cliparea(wdt->overlap()))) {
         // go and merge it
         mergeres = polymerge(wdt->shape2poly(), shapeRef->shape2poly());
         if (NULL != mergeres) {
            // If the merge produce a result - return the result and
            // substitute the shapeRef with its merged counterpart
            shapeRef = wdt;
            return mergeres;
         }
      }
      wdt = wdt->next();
   }
   // if we've got to this point - means that no more shapes to merge
   // in this area
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) {
         mergeres = _quads[i]->merge_selected(shapeRef);
         if (NULL != mergeres) return mergeres;
      }
   //at this point there is nothing more to traverse, so return NULL
   return NULL;
}

bool laydata::QuadTree::delete_this(laydata::TdtData* object) {
   // Create and initialize a variable "to be sorted"
   bool _2B_sorted = false;
   // save the old overlap, and initialize the new one
   DBbox oldovl = _overlap;
   _overlap = DEFAULT_OVL_BOX;
   // deleteing sequence is bottom-up, so start from the children
   for (byte i = 0; i < 4; i++)
      if (_quads[i]) {
         _2B_sorted |= _quads[i]->delete_this(object);
         // check that there is still something left in the child QuadTree
         if (_quads[i]->empty()) {
            delete _quads[i]; _quads[i] = NULL;
         }
         else update_overlap(_quads[i]->overlap());
      }
   // prepare a pair of pointers to TdtData
   TdtData* wds = _first;
   TdtData* wdsP = NULL;
   // loop all TdtData in the current tree and check they are selected
   while (wds)
      // if selected ...
      if (wds == object)
         // Unlink the marked shape from the list
         if (wdsP) { // if this is not the first shape
            wdsP->nextis(wds->next()); wds->nextis(NULL);
            wds = wdsP->next();
         }
         else { // Special attention when this is the first shape
            _first = wds->next(); wds->nextis(NULL);
            wds = _first;
         }
      // update the overlapping box, and move further if that's not the object
      else {
         update_overlap(wds->overlap());
         wdsP = wds; wds = wds->next();
      }
   // If _overlap is still NULL here -> means the placeholder is empty. Will
   // be deleted by the parent
   if (empty()) _invalid = true;
   else {
      //Now if the overlapping rectangles differ, then invalidate
      //the current QuadTree
      int8b areaold = oldovl.boxarea();
      int8b areanew = _overlap.boxarea();
      if (areaold != areanew) _invalid = true;
   }
   return _2B_sorted |= _invalid;
}

/*! Call a resort() method if _invalid flag is up. Used mainly after database
modification operations (copy, move, delete etc.) to keep the cell parents
QuadTree structures up to date \n
Validating of the tree is executed top-down. If the parent is re-sorted,
childrens will be new, so there is no point to search for invalidated among
them
*/
void laydata::QuadTree::validate() {
   if (empty()) return;
   if (_invalid) {
      resort(); _invalid = false;
   }
   else
      for(byte i = 0; i < 4; i++)
         if (_quads[i]) _quads[i]->resort();
}

/*! Exactly as resort(), rebuilds the QuadTree object as well as its children.
\n This method is executed only if _invalid flag is up. The overlapping box is
re-evaluated before the sort() method is called.
*/
bool laydata::QuadTree::full_validate()
{
   if (_invalid)
   {
      ShapeList store;
      tmpstore(store);
      DBbox oldovl = _overlap;
      _overlap = DEFAULT_OVL_BOX;
      for (ShapeList::const_iterator DI = store.begin(); DI != store.end(); DI++)
         update_overlap((*DI)->overlap());
      sort(store);
      _invalid = false;
      return (oldovl != _overlap);
   }
   return false;
}

/*! Rebuilds the entire QuadTree object as well as all of its children using the
sort() method. Beforehand all TdtData is stored in a temporary DataList structure
by calling tmpstore method\n It is important to note that this method is not
re-evaluating the _overlap variable, but is using it as is
*/
void laydata::QuadTree::resort() {
   // first save the existing data in a temporary store
   ShapeList store;
   tmpstore(store);
   sort(store);
}

/*! Takes the array of 4 floating point numbers that correspond to the clipping
areas of the new object and each of the possible four QuadTree children. Returns
the serial number of the biggest clipping area which corresponds to one of the
QuadTree children. \n
Called by fitintree(), fitsubtree() methods when a decision has to be made which
of the possible four child areas is most suitable to refuge the current layout
object (10% decision)
*/
byte laydata::QuadTree::biggest(int8b* array) const
{
   byte curmaxindx = 0;
   for (byte i = 1; i < 4; i++)
      if (array[curmaxindx] < array[i]) curmaxindx = i;
   return curmaxindx;
}

/*! Write the contents of the QuadTree in a TDT file.\n
The idea about store a sorted data does not seems to be appropriate for TDT
format. It is TOPED internal affair. Format might be used by somebody else -
 you never know - do you?*/
void laydata::QuadTree::write(TEDfile* const tedfile) const {
   TdtData* wdt = _first;
   while(wdt) {
      wdt->write(tedfile); wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->write(tedfile);
}

/*! Write the contents of the QuadTree in a GDS file.\n
Nothing special here - effectively the same as write method*/
void laydata::QuadTree::GDSwrite(DbExportFile& gdsf) const
{
   TdtData* wdt = _first;
   while(wdt)
   {
      wdt->GDSwrite(gdsf); wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->GDSwrite(gdsf);
}

/*! Write the contents of the QuadTree in a CIF file.\n
Nothing special here - effectively the same as other write method*/
void laydata::QuadTree::CIFwrite(DbExportFile& ciff) const
{
   TdtData* wdt = _first;
   while(wdt)
   {
      wdt->CIFwrite(ciff); wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->CIFwrite(ciff);
}

/*! Write the contents of the QuadTree in a PS file.\n
Nothing special here - effectively the same as GDSwrite and write method*/
void laydata::QuadTree::PSwrite(PSFile& gdsf, const layprop::DrawProperties& drawprop) const
{
   TdtData* wdt = _first;
   while(wdt)
   {
      wdt->PSwrite(gdsf, drawprop); wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->PSwrite(gdsf, drawprop);
}

/*! Draw the contents of the container on the screen using the virtual
openGL_draw methods of the tdtddata objects. This happens only if
the current QuadTree object is visible. Current clip region data is
obtained from LayoutCanvas. Draws also the select marks in case shape is
selected. \n This is the cherry of the QuadTree algorithm cake*/
void laydata::QuadTree::openGL_draw(layprop::DrawProperties& drawprop,
                                                   const DataList* slst, bool fill) const
{
   if (empty()) return;
   // check the entire holder for clipping...
   DBbox clip = drawprop.clipRegion();
   DBbox areal = _overlap.overlap(drawprop.topCTM());
   if      ( 0ll == clip.cliparea(areal)     ) return;
   else if (!areal.visible(drawprop.ScrCTM())) return;
   TdtData* wdt = _first;
   // The drawing will be faster like this for the cells without selected shapes
   // that will be the wast majority of the cases. A bit bigger code though.
   // Seems the bargain is worth it.
   if (slst)
   {
      while(wdt)
      {
         pointlist points;
         // precalculate drawing data
         wdt->openGL_precalc(drawprop, points);
         if (0 != points.size())
         {
            // draw the shape fill (contents of refs, arefs and texts)
            if (fill)  wdt->openGL_drawfill(drawprop, points);
            // draw the outline of the shapes and overlapping boxes
            wdt->openGL_drawline(drawprop, points);
            if ((sh_selected == wdt->status()) || (sh_partsel == wdt->status()))
            {
               drawprop.setLineProps(true);
               if       (sh_selected == wdt->status())
                  wdt->openGL_drawsel(points, NULL);
               else if  (sh_partsel  == wdt->status())
               {
                  DataList::const_iterator SI;
                  for (SI = slst->begin(); SI != slst->end(); SI++)
                     if (SI->first == wdt) break;
                  assert(SI != slst->end());
                  wdt->openGL_drawsel(points, &(SI->second));
               }
               drawprop.setLineProps(false);
            }
            wdt->openGL_postclean(drawprop, points);
         }
         wdt = wdt->next();
      }
   }
   else
   {
      // if there are no selected shapes
      while(wdt)
      {
         pointlist points;
         // precalculate drawing data
         wdt->openGL_precalc(drawprop, points);
         // draw the shape fill (contents of refs, arefs and texts)
         if (fill)  wdt->openGL_drawfill(drawprop, points);
         // draw the outline of the shapes and overlapping boxes
         wdt->openGL_drawline(drawprop, points);
         // clean-up
         wdt->openGL_postclean(drawprop, points);
         wdt = wdt->next();
      }
   }

   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->openGL_draw(drawprop, slst, fill);
}

short laydata::QuadTree::clip_type(tenderer::TopRend& rend) const
{
   if (empty()) return 0;
   // check the entire holder for clipping...
   DBbox clip = rend.clipRegion();
   DBbox areal = _overlap.overlap(rend.topCTM());
   int8b clip_area = clip.cliparea(areal);
   if ( ( 0ll == clip_area ) || (!areal.visible(rend.ScrCTM())) ) return 0;
   if (0ll < clip_area) return 1;
   else return -1;
}

void laydata::QuadTree::openGL_render(tenderer::TopRend& rend, const DataList* slst) const
{
   TdtData* wdt = _first;
   // The drawing will be faster like this for the cells without selected shapes
   // that will be the wast majority of the cases. A bit bigger code though.
   // Seems the bargain is worth it.
   if (slst)
   {
      while(wdt)
      {
         switch (wdt->status())
         {
            case sh_selected: wdt->draw_srequest(rend, NULL); break;
            case sh_partsel : {// partially selected - so find the pin list
               DataList::const_iterator SI;
               for (SI = slst->begin(); SI != slst->end(); SI++)
                  if (SI->first == wdt) break;
               assert(SI != slst->end());
               wdt->draw_srequest(rend, &(SI->second));
               break;
            }
            default: wdt->draw_request(rend);
         }
         wdt = wdt->next();
      }
   }
   else
   {  // if there are no selected shapes
      while(wdt)
      {
         wdt->draw_request(rend);
         wdt = wdt->next();
      }
   }
   // continue traversing down given that the objects exists and are visible
   for(byte i = 0; i < 4; i++)
      if ( (NULL != _quads[i]) && (0 != _quads[i]->clip_type(rend)) )
         _quads[i]->openGL_render(rend, slst);
}


/*! Temporary draw of the container contents on the screen using the virtual
tmp_draw methods of the tdtddata objects. This happens only if
the current QuadTree object is visible. Current clip region data is
obtained from LayoutCanvas. In a sence this method is the same as openGL_draw
without fill and not handling selected shapes*/
void laydata::QuadTree::motion_draw(const layprop::DrawProperties& drawprop,
                                                   ctmqueue& transtack) const {
   if (empty()) return;
   // check the entire holder for clipping...
   DBbox clip = drawprop.clipRegion();
   DBbox areal = _overlap.overlap(transtack.front());
   if      (0ll == clip.cliparea(areal)      ) return;
   else if (!areal.visible(drawprop.ScrCTM())) return;
   TdtData* wdt = _first;
   while(wdt) {
      wdt->motion_draw(drawprop, transtack, NULL);
      wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->motion_draw(drawprop, transtack);
}

/*! Used to copy TdtData objects from QuadTree to a DataList. This is initiated
by resort() or full_validate() when current QuadTree needs to be rebuild
*/
void laydata::QuadTree::tmpstore(ShapeList &store) {
   while (_first) {
      store.push_back(_first);
      _first = _first->next();
   }
   for (byte i = 0; i < 4; i++)
      if (_quads[i]) {
         _quads[i]->tmpstore(store);
         delete _quads[i];
         _quads[i] = NULL;
      }
}

/*! Updates the overlapping box of the current QuadTree object with the
overlapping box hovl. Used when tree is modified, but there is no need
to sort the data immediately, but only to maintain the valid overlapping
box of the structure*/
void laydata::QuadTree::update_overlap(const DBbox& hovl)
{
   if (empty())  _overlap = hovl;
   else          _overlap.overlap(hovl);
}

/*! Puts the shape into current QuadTree object without sorting. Updates
the overlapping box though. */
void laydata::QuadTree::put(TdtData* shape) {
   update_overlap(shape->overlap());
   shape->nextis(_first); _first = shape;
}

/*! Perform the data selection using select_in box. Called by the corresponding
select methods of the parent structures in the data base - TdtLayer and TdtCell
*/
void laydata::QuadTree::select_inBox(DBbox& select_in, DataList* selist,
                                                  bool pselect, word selmask)
{
   // check the entire holder for clipping...
   if ((laydata::_lmnone == selmask) || (0ll == select_in.cliparea(_overlap))) return;
   // now start selecting one by one
   TdtData* wdt = _first;
   while(wdt)
   {
      if (selmask & wdt->ltype())
      {
         wdt->select_inBox(select_in, selist, pselect);
      }
      wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->select_inBox(select_in, selist, pselect, selmask);
}

/*! Unselects already selected data using unselect_in box. Called by the corresponding
unselect methods of the parent structures in the data base - TdtLayer and TdtCell
*/
void laydata::QuadTree::unselect_inBox(DBbox& unselect_in, DataList* unselist,
                                                                 bool pselect) {
   // check the entire holder for clipping...
   if (0ll == unselect_in.cliparea(_overlap)) return;
   TdtData* wdt = _first;
   while (wdt) {
      // now start unselecting from the list
      DataList::iterator DI = unselist->begin();
      while ( DI != unselist->end() )
         if ((wdt == DI->first) &&
             (DI->first->unselect(unselect_in, *DI, pselect)))
               DI = unselist->erase(DI);
         else DI++;
      wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->unselect_inBox(unselect_in, unselist, pselect);
}

/*! Perform the data selection using list of objects. Called by the
corresponding select methods of the parent structures in the data base -
TdtLayer and TdtCell. This select operatoin is the essence of the
implementatoin of the long discussed (with myself) select lists in TELL
*/
void laydata::QuadTree::select_fromList(DataList* src, DataList* dst) {
   TdtData* wdt = _first;
   DataList::iterator DI;
   // loop the objects in the qTree first. It will be faster when there
   // are no objects in the current QuadTree
   while(wdt) {
      DI = src->begin();
      // loop the objects from the select list
      while ( DI != src->end())
         // if the objects (pointer) coinsides - that's out object
         if (wdt == DI->first) {
            // select the object
            if (DI->second.size() == wdt->numpoints()) {
               wdt->set_status(sh_partsel);
               dst->push_back(SelectDataPair(wdt,DI->second));
            }
            else {
               wdt->set_status(sh_selected);
               dst->push_back(SelectDataPair(wdt,SGBitSet()));
            }
            // remove it from the select list - it will speed up the following
            // operations
            DI = src->erase(DI);
            // there is no point looping futher, get the next object
            break;
         }
         else DI++;
      wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->select_fromList(src, dst);
}

/*! Mark all shapes in the current QuadTree and its children as sh_selected and
add a reference in the selist*/
void laydata::QuadTree::select_all(DataList* selist, word selmask, bool mark) {
   TdtData* wdt = _first;
   if (laydata::_lmnone == selmask) return;
   while(wdt)
   {
      if (selmask & wdt->ltype())
      {
         selist->push_back(SelectDataPair(wdt,SGBitSet()));
         if (mark) wdt->set_status(sh_selected);
      }
      wdt = wdt->next();
   }
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->select_all(selist, selmask, mark);
}


/*! Check whether this is empty i.e. no TdtData objects in the containter */
bool laydata::QuadTree::empty() const {
   return (DEFAULT_OVL_BOX == _overlap);
}

/*! Delete all TdtData objects in the container and free the heap*/
void laydata::QuadTree::freememory() {
   for(byte i = 0; i < 4; i++)
      if (_quads[i]) _quads[i]->freememory();
   TdtData* wdt = _first;
   TdtData* dwdt;
   while(wdt) {
      dwdt = wdt;
      wdt = dwdt->next();
      delete dwdt;
   }
}

bool laydata::QuadTree::getobjectover(const TP pnt, laydata::TdtData*& prev) {
   if (!_overlap.inside(pnt)) return false;
   TdtData* wdt = _first;
   while(wdt) {
      if (prev == NULL) {
//         DBbox ovl = wdt->overlap();ovl.normalize();
//         if (ovl.inside(pnt)) {
         if (wdt->point_inside(pnt)) {
            prev = wdt; return true;
         }
      }
      else if (prev == wdt) prev = NULL;
      wdt = wdt->next();
   }
   for (byte i = 0; i < 4; i++)
      if (_quads[i] && _quads[i]->getobjectover(pnt,prev)) return true;
   return false;
}

void laydata::QuadTree::vlOverlap(const layprop::DrawProperties& prop, DBbox& vlBox) const
{
   TdtData* wdt = _first;
   while(wdt)
   {
      wdt->vlOverlap(prop, vlBox);
      wdt = wdt->next();
   }
   for (byte i = 0; i < 4; i++)
      if (NULL != _quads[i]) _quads[i]->vlOverlap(prop, vlBox);
}

// laydata::TdtData* laydata::QuadTree::getfirstover(const TP pnt) {
//    if (!_overlap.inside(pnt)) return NULL;
//    TdtData* wdt = _first;
//    while(wdt) {
//       DBbox ovl = wdt->overlap();ovl.normalize();
//       if (ovl.inside(pnt)) return wdt;
//       else wdt = wdt->next();
//    }
//    for (byte i = 0; i < 4; i++)
//       if (_quads[i] && (wdt = _quads[i]->getfirstover(pnt))) return wdt;
//    return NULL;
// }
//
// laydata::TdtData* laydata::QuadTree::getnextover(const TP pnt, laydata::TdtData *start,bool& check) {
//    if (!_overlap.inside(pnt)) return NULL;
//    TdtData* wdt = _first;
//    while(wdt) {
//       if (check) {
//          DBbox ovl = wdt->overlap();ovl.normalize();
//          if (ovl.inside(pnt)) return wdt;
//       }
//       else check = (wdt == start);
//       wdt = wdt->next();
//    }
//    for (byte i = 0; i < 4; i++)
//       if (check) {
//          if (_quads[i] && (wdt = _quads[i]->getfirstover(pnt))) return wdt;
//       }
//       else {
//          if (_quads[i] && (wdt = _quads[i]->getnextover(pnt,start,check))) return wdt;
//       }
//    return NULL;
// }

/*!Object destructor. Cleans up only the underlaying QuadTree objects and the
_overlap box. TdtData objects are not deleted, instead they are supposed to be
 moved in another placeholder called at the moment Attic (not implemented yet!)*/
laydata::QuadTree::~QuadTree() {
   if (_quads[0]) delete _quads[0];
   if (_quads[1]) delete _quads[1];
   if (_quads[2]) delete _quads[2];
   if (_quads[3]) delete _quads[3];
   /*Do not delete tdt objects by any means here. During the session the only
     place where shapes are deleted is in the cell Attic yet ONLY by the undo
     list clean-up when the certain delete hits the bottom of the undo list */
}

//-----------------------------------------------------------------------------
// class TdtLayer
//-----------------------------------------------------------------------------
laydata::TdtLayer::TdtLayer(TEDfile* const tedfile) : QuadTree()
{
   byte recordtype;
   while (tedf_LAYEREND != (recordtype = tedfile->getByte()))
   {
      switch (recordtype)
      {
         case      tedf_BOX: put(DEBUG_NEW TdtBox(tedfile));break;
         case     tedf_POLY: put(DEBUG_NEW TdtPoly(tedfile));break;
         case     tedf_WIRE: put(DEBUG_NEW TdtWire(tedfile));break;
         case     tedf_TEXT: put(DEBUG_NEW TdtText(tedfile));break;
         //--------------------------------------------------
         default: throw EXPTNreadTDT("Unexpected record type");
      }
   }
   resort();
}

/*!Create new TdtBox. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
laydata::TdtData* laydata::TdtLayer::addbox(const TP& p1, const TP& p2, bool sortnow) {
   laydata::TdtBox *shape = DEBUG_NEW TdtBox(p1,p2);
   if (sortnow) add(shape);
   else         put(shape);
   return shape;
}
/*!Create new TdtPoly. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
laydata::TdtData* laydata::TdtLayer::addpoly(pointlist& pl, bool sortnow) {
   laydata::TdtPoly *shape = DEBUG_NEW TdtPoly(pl);
   if (sortnow) add(shape);
   else         put(shape);
   return shape;
}

laydata::TdtData* laydata::TdtLayer::addpoly(int4b* pl, unsigned psize, bool sortnow) {
   laydata::TdtPoly *shape = DEBUG_NEW TdtPoly(pl, psize);
   if (sortnow) add(shape);
   else         put(shape);
   return shape;
}

/*!Create new TdtWire. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
laydata::TdtData* laydata::TdtLayer::addwire(pointlist& pl,word w,
                                                                 bool sortnow) {
   laydata::TdtWire *shape = DEBUG_NEW TdtWire(pl,w);
   if (sortnow) add(shape);
   else         put(shape);
   return shape;
}
/*!Create new TdtText. Depending on sortnow input variable the new shape is
just added to the QuadTree (using QuadTree::put()) without sorting or fit on
the proper place (using add() */
laydata::TdtData* laydata::TdtLayer::addtext(std::string text,
                                                      CTM trans, bool sortnow) {
   laydata::TdtText *shape = DEBUG_NEW TdtText(text,trans);
   if (sortnow) add(shape);
   else         put(shape);
   return shape;
}

/*! A temporary Draw (during move/copy operations) of the container contents
 on the screen using the virtual QuadTree::tmp_draw() method of the
 parent object. */
void laydata::TdtLayer::motion_draw(const layprop::DrawProperties& drawprop,
                                                 ctmqueue& transtack) const {
   // check the entire layer for clipping...
   DBbox clip = drawprop.clipRegion();
   if (empty()) return;
   DBbox areal = overlap().overlap(transtack.front());
   if      ( 0ll == clip.cliparea(areal)     ) return;
   else if (!areal.visible(drawprop.ScrCTM())) return;
   QuadTree::motion_draw(drawprop, transtack);
}

void laydata::TdtLayer::vlOverlap(const layprop::DrawProperties&, DBbox& vlBox) const
{
   vlBox.overlap(_overlap);
}
////-----------------------------------------------------------------------------
//// class TdtRefLayer
////-----------------------------------------------------------------------------
//DBbox TdtRefLayer::vlOverlap(const layprop::DrawProperties& drawprop)
//{
//
//}
