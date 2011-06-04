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
//        Created: Tue Mar 15 2011
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Database structure in the memory - clipping template
//                 (spin off from quadtree.h)
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <iostream>
#include "qtree_tmpl.h"
#include "viewprop.h"
#include "tenderer.h"
#include "outbox.h"
#include "grccell.h"

//-----------------------------------------------------------------------------
// class QTreeTmpl
//-----------------------------------------------------------------------------

/*! The main and only constructor of the class*/
template <typename DataT>
laydata::QTreeTmpl<DataT>::QTreeTmpl() : _overlap(DEFAULT_OVL_BOX), _subQuads(NULL), _data(NULL), _props()
{
}

/*! A convenience method initializing the QTreeTmpl::Iterator*/
template <typename DataT>
const typename laydata::QTreeTmpl<DataT>::Iterator laydata::QTreeTmpl<DataT>::begin()
{
   return Iterator(*this);
}

/*! A convenience method initializing the QTreeTmpl::ClipIterator*/
template <typename DataT>
const typename laydata::QTreeTmpl<DataT>::ClipIterator laydata::QTreeTmpl<DataT>::begin(const DBbox& clip)
{
   return ClipIterator(*this, clip);
}

/*! A convenience method initializing the QTreeTmpl::DrawIterator*/
template <typename DataT>
const typename laydata::QTreeTmpl<DataT>::DrawIterator laydata::QTreeTmpl<DataT>::begin(const layprop::DrawProperties& drawprop, const CtmQueue& transtack)
{
   return DrawIterator(*this, drawprop, transtack);
}

/*! A convenience method returning an empty iterator*/
template <typename DataT>
const typename laydata::QTreeTmpl<DataT>::Iterator laydata::QTreeTmpl<DataT>::end()
{
   return Iterator();
}

/*! Add a single layout object shape into the QTreeTmpl.\n
 * It first checks whether or not another layout object is already placed into
 * this QTreeTmpl. If not it simply adds the object and exits. If another object
 * is already here, then the new overlap area is calculated. \n
 * In case the new area is the same as the existing one, then the object might
 * be (possibly) fitted into one of the children QTreeTmpl. To check this
 * fitInTree() method is called. If this is unsuccessful, just then the layout
 * object is added to this QTreeTmpl. \n
 * If the new overlapping area is bigger that the existing one, then the layout
 * object is linked to the current QTreeTmpl after what the current QTreeTmpl as
 * well as its successors has to be rebuild using resort().\n The method might
 * be called recursively via fitInTree() method.\n
 * NOTE! This method is quite expensive (slow!) and its usage shall be
 * limited only to the operations dealing with a single object and requiring
 * sorted tree at the end. In all other cases when a group of objects havr to be
 * added to the tree - an auxiliary object of laydata::QTreeTmp class shall be
 * used.
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::add(DataT* shape)
{
   DBbox shovl(shape->overlap());
   if (empty())
   {
      // first shape in the container
      _overlap = shovl;
      _data = DEBUG_NEW DataT*[1];
      _data[0] = shape;
      _props._numObjects = 1;
   }
   else
   {
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
         if ((areanew <= 4ll * shovl.boxarea()) || !fitInTree(shape))
         {
            // shape doesn't fit into the subtree, so place it here
            DataT** newdata = DEBUG_NEW DataT*[_props._numObjects+1];
            memcpy(newdata, _data, sizeof(DataT*) * _props._numObjects);
            newdata[_props._numObjects++] = shape;
            delete [] _data;
            _data = newdata;
         }
      }
      else
      { // the overlapping box has had blown-up
         resort(shape); // re-sort the entire tree
      }
   }
}

/*! Checks whether a single layout object shape will fit into one of the
 * children's QTreeTmpl. It calls add() and returns success if the new layout
 * object fits entirely into one of the possible sub-quads or if it blows up
 * its overlapping area not more than 10%. Returns false if the shape does not
 * fit anywhere - means it should be placed higher into the QTreeTmpl structure.\n
 * The method might be called recursively via the add() method.
 */
template <typename DataT>
bool laydata::QTreeTmpl<DataT>::fitInTree(DataT* shape)
{
   DBbox shovl = shape->overlap();
   int8b clipedarea[4];
   // check the clipping to see in witch region to place the shape
   for (byte i = 0; i < 4 ; i++)
   {
      DBbox subbox = _overlap.getcorner((QuadIdentificators)i);
      clipedarea[i] = subbox.cliparea(shovl,true);
      if (-1ll == clipedarea[i])
      {//entirely inside the area
         byte quadIndex = sequreQuad((QuadIdentificators) i);
         _subQuads[quadIndex]->add(shape);
         return true;
      }
   }
   // if we got to this point - means that the shape does not fit
   // entirely inside neither of the four sub-areas.
   // It is a decision time then
   byte candidate = biggest(clipedarea);
   // now calculate the eventual new overlapping box
   DBbox newovl = _overlap.getcorner((QuadIdentificators)candidate);
   newovl.overlap(shovl);
   // if the max area of the candidate does not blow more than 10% -
   // then seems to be OK to get it
   if ((40ll * newovl.boxarea()) < (11ll * _overlap.boxarea()))
   {
      byte quadIndex = sequreQuad((QuadIdentificators) candidate);
      _subQuads[quadIndex]->add(shape);
      return true;
   }
   return false; // shape can not be fit into any subtree
}

/*! Checks whether a single layout object will fit into one of the children's
 * QTreeTmpl. Returns the index of the child QTreeTmpl which fits the shape or
 * -1 otherwise.
 */
template <typename DataT>
char laydata::QTreeTmpl<DataT>::fitSubTree(const DBbox& shovl, DBbox* maxsubbox )
{
   int8b clipedarea[4];
   // check the clipping to see in witch region to place the shape
   for (byte i = 0; i < 4 ; i++) {
      clipedarea[i] = maxsubbox[i].cliparea(shovl,true);
      if (-1ll == clipedarea[i])
      {//entirely inside the area
         return i;
      }
   }
//   assert(shovl.area() == (clipedarea[0] + clipedarea[1] + clipedarea[3] + clipedarea[3]));
   // if we got to this point - means that the shape does not fit
   // entirely inside neither of the four sub-areas.
   // It is a decision time then
   byte candidate = biggest(clipedarea);
   // now calculate the eventual new overlapping box
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

/*! Build a new QTreeTmpl structure for the DataT in the inlist. The method is
 * using the existing _overlap variable. For every layout object fitinsubtree()
 * method is called in a try to put the data in the child QTreeTmpl. At the end
 * the method is called for every of the child structures.
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::sort(TObjList& inlist)
{
   unsigned int entryListSize = inlist.size();
   // if the input list is empty - nothing to do!
   if (0 == entryListSize) return;
   typename TObjList::iterator DI = inlist.begin();
   // if the list contains only one component - link it and run away
   if (1 == entryListSize)
   {
      _data = DEBUG_NEW DataT*[1];
      _props._numObjects = 1;
      _data[0] = *DI;
      return;
   }
   // the overlapping box of the current shape
   DBbox shovl = DEFAULT_OVL_BOX;
   // the maximum possible overlapping boxes of the 4 children
   DBbox maxsubbox[4] = {DEFAULT_OVL_BOX, DEFAULT_OVL_BOX,
                         DEFAULT_OVL_BOX, DEFAULT_OVL_BOX};
   for (byte i = 0; i < 4; i++) maxsubbox[i] = _overlap.getcorner((QuadIdentificators)i);
   // the sub-lists data will be sorted in
   TObjList sublist[4];
   // which is the child where current shape fits
   char fitinsubbox;
   // initialize the iterator
   int8b sharea, totalarea = _overlap.boxarea();
   while (inlist.end() != DI)
   {
      // get the overlap of the current shape
      shovl = (*DI)->overlap();
      sharea = shovl.boxarea();
      // Check it fits in some of the children
      if (totalarea <= 4ll * sharea)
      {
         // no fit. The shape is sorted in the current tree
         DI++;
      }
      else
      {
         fitinsubbox = fitSubTree(shovl, maxsubbox);
         if (fitinsubbox < 0)
         {
            DI++;
         }
         else
         {
            // fits in sub-tree fitinsubbox
            sublist[(byte)fitinsubbox].push_back(*DI);
            // secure the subQuad (create it if it doesn't exist)
            byte quadIndex = sequreQuad((QuadIdentificators)fitinsubbox);
            // update the overlapping box of the subQuad
            _subQuads[quadIndex]->_overlap.overlap(shovl);
            // get rid of the current shape pointer. It is pushed in the sbulists
            DI = inlist.erase(DI);
         }
      }
   }
   // at this point inlist MUST contain only the shapes for this QTreeTmpl.
   // The rest was split over the underlying (maximum 4) QuadTrees.
   _props._numObjects = inlist.size();
   assert (entryListSize == (_props._numObjects +
                             sublist[0].size()  +
                             sublist[1].size()  +
                             sublist[2].size()  +
                             sublist[3].size()    )
          );
   // So - first save the local ones
   if (0 < _props._numObjects)
   {
      _data = DEBUG_NEW DataT*[_props._numObjects];
      QuadsIter j = 0;
      for (DI = inlist.begin(); DI != inlist.end(); DI++)
      {
         _data[j] = *DI;
         j++;
      }
   }
   // now go and sort the children - if any
   for (byte i = 0; i < 4; i++)
      if (!sublist[i].empty())
      {
         char quadPosition = _props.getPosition((QuadIdentificators) i);
         _subQuads[(byte)quadPosition]->sort(sublist[i]);
      }
}

/*! Removes marked shapes from the QTreeTmpl without deleting them. The removed
 *  shapes are still listed in the _shapesel container of the current cell.
 *  The new overlapping box is calculated during this process and the method
 *  returns true to notify its parent that the overlapping box has changed and
 *  resort has to be initiated on the upper levels of the tree. The tree is
 *  processed bottom-up, i.e. the method is recursively called first for child
 *  QTreeTmpl structures.\n
 *  On the top level the method is called by two TdtCell methods:-
 *  deleteSelected and move selected. The difference is that for delete,
 *  partially selected shapes are ignored and not processed.\n
 *  Fully selected shapes are always marked as sh_deleted. When move operation
 *  is going on they will be re-marked afterwards to sh_selected by the
 *  move/copy virtual methods of DataT.
 */
template <typename DataT>
bool laydata::QTreeTmpl<DataT>::deleteMarked(SH_STATUS stat, bool partselect)
{
   assert(!((stat != sh_selected) && (partselect == true)));
   // Create and initialize a variable "to be sorted"
   bool _2B_sorted = false;
   // save the old overlap, and initialize the new one
   DBbox oldovl = _overlap;
   _overlap = DEFAULT_OVL_BOX;
   // deleting sequence is bottom-up, so start from the children
   QuadIdentificators cquad = qidNW;
   while (qidNULL > cquad)
   {
      char position = _props.getPosition(cquad);
      if (-1 < position)
      {
         _2B_sorted |= _subQuads[(byte)position]->deleteMarked(stat, partselect);
         // check that there is still something left in the child QTreeTmpl
         if (_subQuads[(byte)position]->empty())
         {
            removeQuad(cquad);
         }
         else updateOverlap(_subQuads[(byte)position]->overlap());
      }
      cquad = (QuadIdentificators)(cquad + 1);
   }
   TObjList unmarkedObjects;
   bool inventoryChanged = false;
   for (QuadsIter i = 0; i < _props._numObjects; i++)
   {
      DataT* wds = _data[i];
      if ((stat == wds->status()) || (partselect && (sh_partsel == wds->status())))
      {
         // mark the fully selected shapes as deleted
         if (stat == wds->status()) wds->setStatus(sh_deleted);
         inventoryChanged = true;
      }
      else
      {
         updateOverlap(wds->overlap());
         unmarkedObjects.push_back(wds);
      }
   }
   if (inventoryChanged)
   {
      delete [] _data; _data = NULL;
      // If _overlap is still NULL here -> means the placeholder is empty. Will
      // be deleted by the parent
      if (empty())
      {
         _props._numObjects = 0;
         _props._invalid = true;
      }
      else
      {
         //Now if the overlapping rectangles differ, then invalidate
         //the current QTreeTmpl
         int8b areaold = oldovl.boxarea();
         int8b areanew = _overlap.boxarea();
         if (areaold != areanew) _props._invalid = true;
         // and finally - put the unmarked shapes back
         _props._numObjects = unmarkedObjects.size();
         _data = DEBUG_NEW DataT*[_props._numObjects];
         QuadsIter j = 0;
         for (typename TObjList::const_iterator DI = unmarkedObjects.begin();
                                                  DI != unmarkedObjects.end(); DI++)
         {
            _data[j] = *DI;
            j++;
         }
      }
   }
   return _2B_sorted |= _props._invalid;
}

template <typename DataT>
bool laydata::QTreeTmpl<DataT>::deleteThis(DataT* object)
{
   // Create and initialize a variable "to be sorted"
   bool _2B_sorted = false;
   // save the old overlap, and initialize the new one
   DBbox oldovl = _overlap;
   _overlap = DEFAULT_OVL_BOX;
   // deleting sequence is bottom-up, so start from the children
   QuadIdentificators cquad = qidNW;
   while (qidNULL > cquad)
   {
      char position = _props.getPosition(cquad);
      if (-1 < position)
      {
         _2B_sorted |= _subQuads[(byte)position]->deleteThis(object);
         // check that there is still something left in the child QTreeTmpl
         if (_subQuads[(byte)position]->empty())
         {
            removeQuad(cquad);
         }
         else updateOverlap(_subQuads[(byte)position]->overlap());
      }
      cquad = (QuadIdentificators)(cquad + 1);
   }
   TObjList unmarkedObjects;
   bool inventoryChanged = false;
   for (QuadsIter i = 0; i < _props._numObjects; i++)
   {
      DataT* wds = _data[i];
      if (wds == object)
      {
         inventoryChanged = true;
      }
      else
      {
         updateOverlap(wds->overlap());
         unmarkedObjects.push_back(wds);
      }
   }
   if (inventoryChanged)
   {
      delete [] _data; _data = NULL;
      // If _overlap is still NULL here -> means the placeholder is empty. Will
      // be deleted by the parent
      if (empty())
      {
         _props._numObjects = 0;
         _props._invalid = true;
      }
      else
      {
         //Now if the overlapping rectangles differ, then invalidate
         //the current QTreeTmpl
         int8b areaold = oldovl.boxarea();
         int8b areanew = _overlap.boxarea();
         if (areaold != areanew) _props._invalid = true;
         // and finally - put the unmarked shapes back
         _props._numObjects = unmarkedObjects.size();
         _data = DEBUG_NEW DataT*[_props._numObjects];
         QuadsIter j = 0;
         for (typename TObjList::const_iterator DI = unmarkedObjects.begin();
                                                  DI != unmarkedObjects.end(); DI++)
         {
            _data[j] = *DI;
            j++;
         }
      }
   }
   return _2B_sorted |= _props._invalid;
}

/*! Calls a resort() method if _props._invalid flag is up. Used mainly after
 * database modification operations (copy, move, delete etc.) to keep the cell
 * parents QTreeTmpl structures up to date \n
 * Validating of the tree is executed top-down. If the parent is re-sorted,
 * children will be new, so there is no point to search for invalidated among
 * them
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::validate()
{
   if (empty()) return;
   if (_props._invalid)
   {
      resort(); _props._invalid = false;
   }
   else
      for (byte i = 0; i < _props.numSubQuads(); i++)
         _subQuads[i]->resort();
}

/*! Exactly as resort(), rebuilds the QTreeTmpl object as well as its children.\n
 * This method is executed only if _props._invalid flag is up. The overlapping
 * box is re-evaluated before the sort() method is called.
 */
template <typename DataT>
bool laydata::QTreeTmpl<DataT>::fullValidate()
{
   if (_props._invalid)
   {
      TObjList store;
      tmpStore(store);
      DBbox oldovl = _overlap;
      _overlap = DEFAULT_OVL_BOX;
      for (typename TObjList::const_iterator DI = store.begin(); DI != store.end(); DI++)
         updateOverlap((*DI)->overlap());
      sort(store);
      _props._invalid = false;
      return (oldovl != _overlap);
   }
   return false;
}

/*! Rebuilds the entire QTreeTmpl object as well as all of its children using the
 * sort() method. Beforehand all DataT is stored in a temporary TObjDataPairList
 * structure by calling tmpStore method\n
 * It is important to note that this method is not re-evaluating the _overlap
 * variable, but is using it as is
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::resort(DataT* newdata)
{
   // first save the existing data in a temporary store
   TObjList store;
   if (NULL != newdata) store.push_back(newdata);
   tmpStore(store);
   sort(store);
}

template <typename DataT>
void laydata::QTreeTmpl<DataT>::resort(TObjList& store)
{
   tmpStore(store);
   sort(store);
}

/*! Takes the array of 4 floating point numbers that correspond to the clipping
 * areas of the new object and each of the possible four QTreeTmpl children.
 * Returns the serial number of the biggest clipping area which corresponds to
 * one of the QTreeTmpl children. \n
 * Called by fitInTree(), fitSubTree() methods when a decision has to be made
 * which of the possible four child areas is most suitable to refuge the current
 * layout object (10% decision)
 */
template <typename DataT>
byte laydata::QTreeTmpl<DataT>::biggest(int8b* array) const
{
   byte curmaxindx = 0;
   for (byte i = 1; i < 4; i++)
      if (array[curmaxindx] < array[i]) curmaxindx = i;
   return curmaxindx;
}

/*! Draw the contents of the container on the screen using the virtual openGlDraw
 *  methods of the tdtddata objects. This happens only if the current QTreeTmpl
 *  object is visible. Current clip region data is obtained from LayoutCanvas.
 *  Draws also the select marks in case shape is selected.\n
 *  This is the cherry of the QTreeTmpl algorithm cake
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::openGlDraw(layprop::DrawProperties& drawprop,
                                                   const TObjDataPairList* slst, bool fill) const
{
   if (empty()) return;
   // check the entire holder for clipping...
   DBbox clip = drawprop.clipRegion();
   DBbox areal = _overlap.overlap(drawprop.topCtm());
   if      ( 0ll == clip.cliparea(areal)     ) return;
   else if (!areal.visible(drawprop.scrCtm(), drawprop.visualLimit())) return;
   // The drawing will be faster like this for the cells without selected shapes
   // that will be the wast majority of the cases. A bit bigger code though.
   // Seems the bargain is worth it.
   if (slst)
   {
      for (QuadsIter i = 0; i < _props._numObjects; i++)
      {
         DataT* wdt = _data[i];
         PointVector points;
         // pre-calculate drawing data
         wdt->openGlPrecalc(drawprop, points);
         if (0 != points.size())
         {
            // draw the shape fill (contents of refs, arefs and texts)
            if (fill)  wdt->openGlDrawFill(drawprop, points);
            // draw the outline of the shapes and overlapping boxes
            wdt->openGlDrawLine(drawprop, points);
            if ((sh_selected == wdt->status()) || (sh_partsel == wdt->status()))
            {
               drawprop.setLineProps(true);
               if       (sh_selected == wdt->status())
                  wdt->openGlDrawSel(points, NULL);
               else if  (sh_partsel  == wdt->status())
               {
                  typename TObjDataPairList::const_iterator SI;
                  for (SI = slst->begin(); SI != slst->end(); SI++)
                     if (SI->first == wdt) break;
                  assert(SI != slst->end());
                  wdt->openGlDrawSel(points, &(SI->second));
               }
               drawprop.setLineProps(false);
            }
            wdt->openGlPostClean(drawprop, points);
         }
      }
   }
   else
   {
      // if there are no selected shapes
      for (QuadsIter i = 0; i < _props._numObjects; i++)
      {
         DataT* wdt = _data[i];
         PointVector points;
         // pre-calculate drawing data
         wdt->openGlPrecalc(drawprop, points);
         // draw the shape fill (contents of refs, arefs and texts)
         if (fill)  wdt->openGlDrawFill(drawprop, points);
         // draw the outline of the shapes and overlapping boxes
         wdt->openGlDrawLine(drawprop, points);
         // clean-up
         wdt->openGlPostClean(drawprop, points);
      }
   }

   for (byte i = 0; i < _props.numSubQuads(); i++)
      _subQuads[i]->openGlDraw(drawprop, slst, fill);
}

template <typename DataT>
short laydata::QTreeTmpl<DataT>::clipType(tenderer::TopRend& rend) const
{
   if (empty()) return 0;
   // check the entire holder for clipping...
   DBbox clip = rend.clipRegion();
   DBbox areal = _overlap.overlap(rend.topCTM());
   int8b clip_area = clip.cliparea(areal);
   if ( ( 0ll == clip_area ) || (!areal.visible(rend.ScrCTM(), rend.visualLimit())) ) return 0;
   if (0ll < clip_area) return 1;
   else return -1;
}

template <typename DataT>
void laydata::QTreeTmpl<DataT>::openGlRender(tenderer::TopRend& rend, const TObjDataPairList* slst) const
{
   // The drawing will be faster like this for the cells without selected shapes
   // that will be the wast majority of the cases. A bit bigger code though.
   // Seems the bargain is worth it.
   if (slst)
   {
      for (QuadsIter i = 0; i < _props._numObjects; i++)
      {
         DataT* wdt = _data[i];
         switch (wdt->status())
         {
            case sh_selected: wdt->drawSRequest(rend, NULL); break;
            case sh_partsel : {// partially selected - so find the pin list
               typename TObjDataPairList::const_iterator SI;
               for (SI = slst->begin(); SI != slst->end(); SI++)
                  if (SI->first == wdt) break;
               assert(SI != slst->end());
               wdt->drawSRequest(rend, &(SI->second));
               break;
            }
            default: wdt->drawRequest(rend);
         }
      }
   }
   else
   {  // if there are no selected shapes
      for (QuadsIter i = 0; i < _props._numObjects; i++)
      {
         _data[i]->drawRequest(rend);
      }
   }
   // continue traversing down given that the objects exists and are visible
   for (byte i = 0; i < _props.numSubQuads(); i++)
      if ( 0 != _subQuads[i]->clipType(rend))
         _subQuads[i]->openGlRender(rend, slst);
}

/*! Used to copy DataT objects from QTreeTmpl to a TObjDataPairList. This is
 * initiated by resort() or fullValidate() when current QTreeTmpl needs to be
 * rebuild
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::tmpStore(TObjList &store)
{
   if (NULL != _data)
   {
      for (QuadsIter i = 0; i < _props._numObjects; i++)
      {
         store.push_back(_data[i]);
      }
      delete [] _data; _data = NULL;
      _props._numObjects = 0;
   }
   if (NULL != _subQuads)
   {
      for (byte i = 0; i < _props.numSubQuads(); i++)
      {
         _subQuads[i]->tmpStore(store);
         delete _subQuads[i];
      }
      delete [] _subQuads;
      _subQuads = NULL;
      _props.clearQuadMap();
   }
}

/*! Updates the overlapping box of the current QTreeTmpl object with the
 * overlapping box hovl. Used when tree is modified, but there is no need to
 * sort the data immediately, but only to maintain the valid overlapping box of
 * the structure
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::updateOverlap(const DBbox& hovl)
{
   if (empty())  _overlap = hovl;
   else          _overlap.overlap(hovl);
}

template <typename DataT>
byte laydata::QTreeTmpl<DataT>::sequreQuad(QuadIdentificators quad)
{
   char quadPosition = _props.getPosition(quad);
   if (-1 == quadPosition)
   {
      QuadProps oldMap = _props;
      _props.addQuad(quad);
      QTreeTmpl** newSubQuads = DEBUG_NEW QTreeTmpl*[_props.numSubQuads()];
      for (byte i = 0; i < 4; i++)
      {
         if (-1 < oldMap.getPosition((QuadIdentificators)i))
            newSubQuads[(byte)_props.getPosition((QuadIdentificators)i)] = _subQuads[(byte)oldMap.getPosition((QuadIdentificators)i)];
         else if (i == quad)
            newSubQuads[(byte)_props.getPosition((QuadIdentificators)i)] = DEBUG_NEW QTreeTmpl();
      }
      if (_subQuads) delete [] _subQuads;
      _subQuads = newSubQuads;
      quadPosition = _props.getPosition(quad);
   }
   return quadPosition;
}

template <typename DataT>
void laydata::QTreeTmpl<DataT>::removeQuad(QuadIdentificators quad)
{
   assert(-1 != _props.getPosition(quad));
   QuadProps oldMap = _props;
   _props.removeQuad(quad);
   QTreeTmpl** newSubQuads = DEBUG_NEW QTreeTmpl*[_props.numSubQuads()];
   for (byte i = 0; i < 4; i++)
   {
      if ((i != quad) && (-1 < oldMap.getPosition((QuadIdentificators)i)))
         newSubQuads[(byte)_props.getPosition((QuadIdentificators)i)] = _subQuads[(byte)oldMap.getPosition((QuadIdentificators)i)];
      else if (i == quad)
         delete _subQuads[(byte)oldMap.getPosition((QuadIdentificators)i)];
   }
   delete [] _subQuads;
   _subQuads = newSubQuads;
}

/*! Check whether this is empty i.e. no DataT objects in the container
 */
template <typename DataT>
bool laydata::QTreeTmpl<DataT>::empty() const
{
   return (DEFAULT_OVL_BOX == _overlap);
}

template <typename DataT>
bool laydata::QTreeTmpl<DataT>::getObjectOver(const TP pnt, DataT*& prev)
{
   if (!_overlap.inside(pnt)) return false;
   for (QuadsIter i = 0; i < _props._numObjects; i++)
   {
      DataT* wdt = _data[i];
      if (prev == NULL)
      {
         if (wdt->pointInside(pnt))
         {
            prev = wdt; return true;
         }
      }
      else if (prev == wdt) prev = NULL;
   }
   for (byte i = 0; i < _props.numSubQuads(); i++)
      if (_subQuads[i]->getObjectOver(pnt,prev)) return true;
   return false;
}

/*! Delete all DataT objects in the container and free the heap. This function
 *  shall be called before the destructor in case the whole layer is to be
 *  destroyed - i.e. on exit for example
 */
template <typename DataT>
void laydata::QTreeTmpl<DataT>::freeMemory()
{
   for (byte i = 0; i < _props.numSubQuads(); i++)
      _subQuads[i]->freeMemory();
   for (QuadsIter i = 0; i < _props._numObjects; i++)
   {
      delete _data[i];
   }
   delete [] _data;
   _data = NULL;
}

/*! Object destructor. Cleans up only the underlaying QTreeTmpl objects and the
 * _overlap box. DataT objects are not deleted, instead they are supposed to be
 * moved in another placeholder called at the moment Attic (not implemented yet!)
 */
template <typename DataT>
laydata::QTreeTmpl<DataT>::~QTreeTmpl()
{
   if (NULL != _subQuads)
   {
      for (byte i = 0; i < _props.numSubQuads(); i++)
         delete _subQuads[i];
      delete [] _subQuads;
   }
   /*Do not delete tdt objects by any means here. During the session the only
     place where shapes are deleted is in the cell Attic yet ONLY by the undo
     list clean-up when the certain delete hits the bottom of the undo list */
   if (NULL != _data)     delete [] _data;
}


// template <typename DataT>
// laydata::DataT* laydata::QTreeTmpl<DataT>::getfirstover(const TP pnt) {
//    if (!_overlap.inside(pnt)) return NULL;
//    DataT* wdt = _first;
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
// template <typename DataT>
// laydata::DataT* laydata::QTreeTmpl<DataT>::getnextover(const TP pnt, laydata::DataT *start,bool& check) {
//    if (!_overlap.inside(pnt)) return NULL;
//    DataT* wdt = _first;
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

//==============================================================================
// implicit template instantiation with a certain type parameter
template class laydata::QTreeTmpl<laydata::TdtData>;
template class laydata::QTreeTmpl<auxdata::TdtAuxData>;

