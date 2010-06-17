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
//        Created: Tue Feb 25 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Layout primitives
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#ifdef WIN32
//For GUI application with glf we need "windows.h"
#include <windows.h>
#endif

#include <sstream>
#include <iostream>
#include <math.h>
#include <algorithm>
#include "tedat.h"
#include "tedcell.h"
#include "logicop.h"
#include "tenderer.h"
#include "ps_out.h"
#include "outbox.h"

//GLubyte select_mark[30] = {0x00, 0x00, 0x00, 0x00, 0x3F, 0xF8, 0x3F, 0xF8, 0x30, 0x18,
//                           0x30, 0x18, 0x30, 0x18, 0x30, 0x18, 0x30, 0x18, 0x30, 0x18,
//                           0x30, 0x18, 0x3F, 0xF8, 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00};

extern layprop::FontLibrary* fontLib;

/*===========================================================================
      Select and subsequent operations over the existing TdtData
=============================================================================
It appears this is not so strightforward as it seems at first sight. There
are several important points to consider here.
   - On first place of course fitting this operatoins within the existing
     data base structure and esspecially make QuadTree trasparent to them.
     The latter appears to be quite important, because this structure is
     not existing for the user, yet plays a major role in the database
     integrity. The other thing of a significant importance is that the
     selected structures have to be drawn differently - i.e. they need to be
     easyly distinguishable on the screen from the other shapes.
   - Minimization of the database. As now the database is build strictly
     on a hierarchical and "conspirative" principles so that every part of
     it holds the absolute minimum of data. For example the layer structure
     does not now shit about the cell it belongs to, it even doesn't know
     what is its layer number. Similarly the shape doesn't know neither it's
     placeholder nor its layer or cell. Everything is on kind of "need to know
     basis". Lower levels of the database are unaware of the hier levels.
     Furthermore on certain hierarchical level every component knows only about
     the neithboring component on the right (next one), but nothing about the
     one on the left (previous one)
   - Select operation assumes however that an operation like move/delete/copy
     etc. is about to follow, i.e. the selected shapes will be accessed shortly
     and this access has to be swift and reliable. Certainly we need some short
     way to access the already selected structures.
     There are generally two possible ways to do this
     -> mark every selected figure
     -> create a list that contains a pointers to the selected figures
     The irony here is that both possibilities suffer from the "conspirative"
     principle on which database is build. In other words it appears that it
     is not enough to know what is the selected shape. Why?
     How on earth you are going to delete a shape if you know only its next of
     kin? As a matter of fact it is possible ;), the only detail is that
     "Segmentatoin fault" will follow inevitably.
     Appparently you need to know what is the structure in the upper level
     of the hierarchy (it's father if you want).
   - The vast majority  of the shapes (may be all of them in the near feature)
     are placed in a QuadTree. The objects of this class however "by nature"
     are like moving sands. They might change dramatically potentially with
     every little change in the data base. This effectively means that the
     pointer to the shape placeholder is not that reliable as one would wish to.
     It is OK as long as data base hasn't changed after the select operation.
   - By definition TELL should be able to operate with TOPED shapes (or list of
     them). Select operation should be able to return to TELL a reliable list
     of TOPED shapes, that can be used in any moment. A "small detail" here is
     the modify or info operation on a TOPED shape. We certainly need to know the
     layer, shape is placed on, and maybe the cell. TELL doesn't know (and doesn't
     want to know) about the TOPED database structure, QuadTree's etc. shit.
   - Undo list - this is not started yet, but seems obvoius that in order to
     execute undelete for example QuadTree pointers will not be very helpfull,
     because they migh be gone already. Instead a layer number will be
     appreciated. All this in case the shape itself is not deleted, but just moved
     in the Attic.
   ----------------------------------------------------------------------------
   In short:
   - It doesn't seems a good idea to sacrifice the database (in terms of size
     and speed of drawing) for select operations. Means I don't want to add
     more data to every single shape object.
   - The TELL operations with TOPED shapes on second though seem not to be so
     convinient and required "by definition" (see the paragraph below)
   - Pointer to a QuadTree as a parent object to the selected shape seems to
     be inevitable despite the unstable nature of teh QuadTree - otherwise every
     subsequent operation on the selected objects will require searching of the
     marked components one by one troughout entire cell or at least layer. This
     might take more time than the select itself, which doesn't seems to be very
     wise.
   - For undo purposes we will need separate list type for selected components
     that will contain layer number instead of QuadTree.
   -----------------------------------------------------------------------------
   Now about TELL operations over TOPED shapes.
   What we need in TELL without doubt is;
   -> select TOPED objects
   -> move/copy/delete etc. selected objects.
   -> modify selected object - that is the same as previous, but also - change
      the contents and size of text data, change the cell in ref/aref data etc.
   Do we need to be able to keep more than one set of selected objects?
   This is quite tempting. A lot I would say. So what it will give to TELL?
   Ability to keep a list of TOPED objects in a variable, that can be used
   later... and to make a big mess !!!
   - Object selection in TOPED is done ALWAYS withing a cell. Having a TELL
     variable stored somewhere, how the parser will control where the selection
     is from and what is the active cell at the moment?
   - Even if you want to copy or move some group of shapes from one cell to
     another there is still a way: openCell<source>->select->group<new_cell>
     ->openCell<destination>->addref<new_cell>
   - What happens if the TOPED components listed in the TELL variable or part
     of them are deleted or moved subsequently? That will just confuse the
     TELL user. (Who the fuck is messing with my list?)
   - I can not think of a case when two or more separate lists of TOPED shapes,
     will be needed in the same time. We don't have TELL operation that works
     with more than one list (we don't have even with one yet ;)).
   - At the end of the day if for some reason somebody needs to keep certain
     list he should better call the TELL code that selects this list. It seems
     that simple.
   The old question how eventually TELL can benefit from the TOPED data base.
   The connection TELL - TOPED looks at the moment like a one way road, but
   this doen't seem to harm the abilities of the language.
   -----------------------------------------------------------------------------
   Then here are the rules:
   - There is ONE select list in TOPED and in TELL. That will be internal TELL
     variable(the first one). We should think about a convention for internal
     variables - $selected is the first idea. The variable will be updated
     automatically as long as it will point to the list of the selected
     components in the active cell (oops - here it is the way to maintain more
     than one list - per cell basis!)
   - The only placeholder type is QuadTree. This means that ref/arefs should be
     placed in a QuadTree instead of a C++ standard container
   - No TdtData is deleted during the session. Delete operation will move
     the shape in a different placeholder (Attic), or alternatively will mark it
     as deleted. This is to make undo operations possible. It seems appropriate,
     every cell to have an attic where the rubbish will be stored.
   - Select list contains a pointer to the lowest QuadTree placeholder that
     contains seleted shapes.
   - TdtData contains a field that will indicate the status of the shape -
     selected or not. The same field might be used to indicate the shape is
     deleted or any other status. This adds a byte to the every single component.
     It seems however that it will pay-back when drawing a large amount of
     selected data. As usual - speed first!
   - The select list is invalidated with every cell change. The new shape might
     be selected.
   -----------------------------------------------------------------------------
   Now when already the only placeholder is QuadTree ...
   -----------------------------------------------------------------------------
   Having a possibility to keep the list of components in TELL still seems
   quite tempting. It still seems that this might speed-up a lot of TELL
   operations, although it is not clear at the moment how exactly. In the same
   time all written above about the possible mess is absolutely true.
   It looks however that there is bright way out of the situation so that
   "the wolf is happy and the sheep is alive". Here it is:
   - All of the above rules are still in place EXCEPT the first one
   - TOPED has still ONE active list of selected components. It is stored in
     the active cell structure and can be obtained from TELL using the internal
     variable $seleted(or similar).
   - All select operatoins return a list of ttlayout that can be stored in
     a TELL variable of the same type. It is not possible to use this variables
     directly as a parameter for any modification operations (copy/move/delete
     etc.)
   - All modification related operations work with the TOPED list of selected
     components
   - The TELL component list variables will be used mainly as a parameter of a
     dedicated select function, so that the stored lists can be reselected later.
   The bottom line is: There is always ONE active TOPED list of selected shapes.
   TELL has an oportunity to reselect a certain list of shapes using a dedicated
   select function. All modifications are executed over the current TOPED list.
   TOPED list of selected components is invalidated after each cell change.
   Thus the mess with the multiply selected lists seems to be sorted.

*/

/*===========================================================================
                        Toped DaTa (TDT) databse structure
=============================================================================

                           TdtCell
TdtLayer TdtLayer etc.

QuadTree                   QuadTree

shapes                     TdtCellRef/tdtaref

*/

bool laydata::TdtData::pointInside(const TP pnt)
{
   DBbox ovl = overlap();ovl.normalize();
   if (ovl.inside(pnt)) return true;
   else return false;
}

void laydata::TdtData::selectThis(DataList* selist)
{
   if (sh_partsel == _status)
   {
      // if the shape has already been partially selected
      for (DataList::iterator SI = selist->begin(); SI != selist->end(); SI++)
      // find it in the select list and remove the list of selected points
         if (SI->first == this)
         {
            SI->second.clear();
//            delete (SI->second); SI->second = NULL;
            break;
         }
   }
   else
      // otherwise - simply add it to the list
      selist->push_back(SelectDataPair(this,SGBitSet()));
   _status = sh_selected;
}

void laydata::TdtData::selectInBox(DBbox& select_in, DataList* selist, bool pselect)
{
   // if shape is already fully selected, nothing to do here
   if (sh_selected == _status) return;
   real clip;
   // get the clip area and if it is 0 - run away
   if (0ll == (clip = select_in.cliparea(overlap()))) return;
   if (-1.0 == clip) selectThis(selist); // entire shape is in
   else if ((clip > 0.0) && pselect)
   { // shape partially is in the select box
      if (sh_partsel == _status)
      {
      // if the shape has already been partially selected
         DataList::iterator SI;
         for (SI = selist->begin(); SI != selist->end(); SI++)
            // get the pointlist
            if (SI->first == this) break;
         assert(0 != SI->second.size());
         // select some more points using shape specific procedures
         selectPoints(select_in, SI->second);
         // check that after the selection shape doesn't end up fully selected
         if (SI->second.isallset())
         {
            _status = sh_selected;
            SI->second.clear();
         }
      }
      else
      {
         // otherwise create a new pointlist
         SGBitSet pntlst(numPoints());
         // select some more points using shape specific procedures
         selectPoints(select_in, pntlst);
         // and check
         if (!pntlst.isallclear()) {
            _status = sh_partsel;
            selist->push_back(SelectDataPair(this, SGBitSet(pntlst)));
         }
//         else delete pntlst;
      }
   }
}

bool  laydata::TdtData::unselect(DBbox& select_in, SelectDataPair& SI, bool pselect)
{
   // check the integrity of the select list
   assert((sh_selected == _status) || (sh_partsel == _status));
   real clip;
   // get the clip area and if it is 0 - run away
   if (0ll == (clip = select_in.cliparea(overlap()))) return false;
   // if select_in overlaps the entire shape
   if (-1.0 == clip) {
      if (0 != SI.second.size()) {
         //remove the list of selected points if it exists ...
         //delete (SI.second); SI.second = NULL;
         SI.second.clear();
      }
      _status = sh_active;
      return true;// i.e. remove this from the list of selected shapes
   }
   // if select_in intersects with the overlapping box
   else if ((clip > 0.0) && pselect)
   {
      // for cell refernces and texts - dont't unselect if they are
      // partially covered
      if (1 == numPoints()) return false;
      // get all points if the shape has not been already partially selected
      if (sh_partsel != _status)
         SI.second = SGBitSet(numPoints());
      // get an alias of the SI.second
      SGBitSet &pntlst = SI.second;
      // finally - go and unselect some points
      unselectPoints(select_in, pntlst);
      if (pntlst.isallclear())
      {//none selected
         _status = sh_active;
         pntlst.clear();
         //delete pntlst;SI.second = NULL;
         return true;
      }
      else if (pntlst.isallset())
      { //all points selected;
         _status = sh_selected;
         pntlst.clear();
         //delete pntlst; SI.second = NULL;
         return false;
      }
      else
      { // some selected
         _status = sh_partsel;
         return false;
      }
   }
   // if pselect is false and the shape is not fully overlapped by
   // the select_in - do nothing
   return false;
}

   // Some thoughts here about the drawing optimization.
   // For optimal drawing speed we have to fulfill several requirements
   // 1. Data processing has to be minimized - i.e. calculations of
   //    the overlapping boxes, wire generation etc. has to be performed
   //    once. The same concerns the coordinate conversion.
   // 2. Minimize the altertion of the line&fill styles and colors, i.e.
   //    Objects with the same drawing properties have to be groupped
   //    together
   // (The structure of the database in memory is out of discussion here)
   // Achieving the first objective is simple - calculation are performed
   // once in openGlPrecalc() and then reused in the rest of the openGL_*
   // functions. It is quite not clear however where is the balance here,
   // because calculations might be cheaper than the memory access in
   // terms of CPU cycles.
   // There is a big exception here - texts. Esspecially solid texts
   // As all other shapes every symbol should be drawn twice - once the
   // outline (wired symbol in glf terms) and then the fill (the solid part)
   // Here data processing is done twice. To avoid this - we have to
   // code glf procedures that follow the rule 1 - i.e. to calculate first
   // the entire string in terms of polygons and then to draw the lines
   // and the fill as appripriate using the new data.
   //
   // The second objective seems to be a problem as well
   // - drawing color as a rule is the same for the entire layer.
   //   -- this means that the reference marks and overlapping boxes of the
   //      texts shold be drawn in the color of the layer (not in gray)
   //   -- for the reference marks of the cell references as well as for
   //      their overlapping boxes this will be a problem because we'll
   //      have to change the color for one box only (and one mark)
   // - line style - here the things are more compicated. The partially
   //   selected shapes will certainly need a change in the line style
   //   and possibly even the color. This also means that the tricks with
   //   drawing first the unslected shapes and then the selected ones
   //   will barely improve the things - how many shapes will be grouped
   //   in a single quadree cell? - they shouldn't be much if the quadtree
   //   algo works properly. Then why bother ?
   //

//-----------------------------------------------------------------------------
// class TdtBox
//-----------------------------------------------------------------------------
laydata::TdtBox::TdtBox(const TP& p1, const TP& p2) : TdtData()
{
   _pdata[p1x ] = p1.x();_pdata[p1y ] = p1.y();
   _pdata[p2x ] = p2.x();_pdata[p2y ] = p2.y();
   SGBitSet dummy;
   normalize(dummy);
}

laydata::TdtBox::TdtBox(TEDfile* const tedfile) : TdtData()
{
   TP point;
   point = tedfile->getTP();
   _pdata[p1x ] = point.x();_pdata[p1y ] = point.y();
   point = tedfile->getTP();
   _pdata[p2x ] = point.x();_pdata[p2y ] = point.y();
   SGBitSet dummy;
   normalize(dummy);
}

void laydata::TdtBox::normalize(SGBitSet& psel)
{
   int4b swap;
   if (_pdata[p1x] > _pdata[p2x])
   {
      swap = _pdata[p1x]; _pdata[p1x] = _pdata[p2x]; _pdata[p2x] = swap;
      if (0 != psel.size())
      {
         psel.swap(0,1);
         psel.swap(2,3);
      }
   }
   if (_pdata[p1y] > _pdata[p2y])
   {
      swap = _pdata[p1y]; _pdata[p1y] = _pdata[p2y]; _pdata[p2y] = swap;
      if (0 != psel.size())
      {
         psel.swap(0,3);
         psel.swap(1,2);
      }
   }
}

void laydata::TdtBox::openGlPrecalc(layprop::DrawProperties& drawprop , pointlist& ptlist) const
{
   // translate the points using the current CTM
   ptlist.reserve(4);
   ptlist.push_back(TP(_pdata[p1x], _pdata[p1y]) * drawprop.topCtm());
   ptlist.push_back(TP(_pdata[p2x], _pdata[p1y]) * drawprop.topCtm());
   ptlist.push_back(TP(_pdata[p2x], _pdata[p2y]) * drawprop.topCtm());
   ptlist.push_back(TP(_pdata[p1x], _pdata[p2y]) * drawprop.topCtm());
}

void laydata::TdtBox::drawRequest(tenderer::TopRend& rend) const
{
   rend.box(const_cast<int4b*>(&_pdata[0]));//TODO Fix the cast!
}

void laydata::TdtBox::drawSRequest(tenderer::TopRend& rend, const SGBitSet* pslist) const
{
   rend.box(const_cast<int4b*>(&_pdata[0]), pslist);//TODO Fix the cast!
}

void laydata::TdtBox::openGlDrawLine(layprop::DrawProperties&, const pointlist& ptlist) const
{
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < 4; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::TdtBox::openGlDrawFill(layprop::DrawProperties&, const pointlist& ptlist) const
{
   // We can't draw directly a box here because if the entire cell is rotated
   // on angle <> 90, then it's not a box anymore
   //glRecti(ptlist[0].x(),ptlist[0].y(), ptlist[2].x(),ptlist[2].y());
   //
   glBegin(GL_POLYGON);
   for (unsigned i = 0; i < 4; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::TdtBox::openGlDrawSel(const pointlist& ptlist, const SGBitSet* pslist) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
   else if (sh_partsel == status())
   {
      assert(pslist);
      glBegin(GL_LINES);
      for (unsigned i = 0; i < 4; i++)
      {
         if (pslist->check(i) && pslist->check((i+1)%4))
         {
            glVertex2i(ptlist[i].x(), ptlist[i].y());
            glVertex2i(ptlist[(i+1)%4].x(), ptlist[(i+1)%4].y());
         }
      }
      glEnd();
   }
}

void laydata::TdtBox::motionDraw(const layprop::DrawProperties&, ctmqueue& transtack,
                                                SGBitSet* plst) const
{
   CTM trans = transtack.front();
   if (sh_partsel == status())
   {
      TP pt1, pt2;
      CTM strans = transtack.back();
      assert(plst);
      pointlist* nshape = movePointsSelected(*plst, trans, strans);
      pt1 = (*nshape)[0]; pt2 = (*nshape)[2];
      glRecti(pt1.x(),pt1.y(),pt2.x(),pt2.y());
      nshape->clear(); delete nshape;
   }
   else
   {
      pointlist ptlist;
      ptlist.reserve(4);
      ptlist.push_back(TP(_pdata[p1x], _pdata[p1y]) * trans);
      ptlist.push_back(TP(_pdata[p2x], _pdata[p1y]) * trans);
      ptlist.push_back(TP(_pdata[p2x], _pdata[p2y]) * trans);
      ptlist.push_back(TP(_pdata[p1x], _pdata[p2y]) * trans);
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
      ptlist.clear();
   }
}

void  laydata::TdtBox::selectPoints(DBbox& select_in, SGBitSet& pntlst) {
   if (select_in.inside(TP(_pdata[p1x], _pdata[p1y])))  pntlst.set(0);
   if (select_in.inside(TP(_pdata[p2x], _pdata[p1y])))  pntlst.set(1);
   if (select_in.inside(TP(_pdata[p2x], _pdata[p2y])))  pntlst.set(2);
   if (select_in.inside(TP(_pdata[p1x], _pdata[p2y])))  pntlst.set(3);
   pntlst.check_neighbours_set(false);
}

void  laydata::TdtBox::unselectPoints(DBbox& select_in, SGBitSet& pntlst) {
   if (sh_selected == _status) pntlst.setall();
   if (select_in.inside(TP(_pdata[p1x], _pdata[p1y])))  pntlst.reset(0);
   if (select_in.inside(TP(_pdata[p2x], _pdata[p1y])))  pntlst.reset(1);
   if (select_in.inside(TP(_pdata[p2x], _pdata[p2y])))  pntlst.reset(2);
   if (select_in.inside(TP(_pdata[p1x], _pdata[p2y])))  pntlst.reset(3);
}

laydata::Validator* laydata::TdtBox::move(const CTM& trans, SGBitSet& plst)
{
   if (0 != plst.size())
   {// used for modify
      pointlist* nshape = movePointsSelected(plst, trans);
      _pdata[p1x ] = (*nshape)[0].x();_pdata[p1y ] = (*nshape)[0].y();
      _pdata[p2x] = (*nshape)[2].x();_pdata[p2y] = (*nshape)[2].y();;
      normalize(plst);
      nshape->clear(); delete nshape;
      return NULL;
   }
   else
   {// used for rotate
      pointlist plist;
      plist.reserve(4);
      plist.push_back(TP(_pdata[p1x], _pdata[p1y])*trans);
      plist.push_back(TP(_pdata[p2x], _pdata[p1y])*trans);
      plist.push_back(TP(_pdata[p2x], _pdata[p2y])*trans);
      plist.push_back(TP(_pdata[p1x], _pdata[p2y])*trans);
      laydata::ValidBox* check = DEBUG_NEW ValidBox(plist);
      if (laydata::shp_box & check->status())
      {
         // modify the box ONLY if we're going to get a box
         transfer(trans);
         delete check;
         return NULL;
      }
      // in all other cases keep the original box, depending on the check->status()
      // the shape will be replaced, or marked as failed to rotate
      return check;
   }

}

void laydata::TdtBox::transfer(const CTM& trans) {
   TP p1 = TP(_pdata[p1x], _pdata[p1y]) * trans;
   TP p2 = TP(_pdata[p2x], _pdata[p2y]) * trans;
   _pdata[p1x] = p1.x();_pdata[p1y] = p1.y();
   _pdata[p2x] = p2.x();_pdata[p2y] = p2.y();
   SGBitSet dummy;
   normalize(dummy);
}

laydata::TdtData* laydata::TdtBox::copy(const CTM& trans)
{
   TP cp1(_pdata[p1x], _pdata[p1y]); cp1 *= trans;
   TP cp2(_pdata[p2x], _pdata[p2y]); cp2 *= trans;
   return DEBUG_NEW TdtBox(cp1, cp2);
}

void laydata::TdtBox::info(std::ostringstream& ost, real DBU) const {
   ost << "box - {";
   TP p1 (_pdata[p1x], _pdata[p1y]);
   p1.info(ost, DBU);
   ost << " , ";
   TP p2 (_pdata[p2x], _pdata[p2y]);
   p2.info(ost, DBU);
   ost << "};";
}

void laydata::TdtBox::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_BOX);
   tedfile->put4b(_pdata[p1x]); tedfile->put4b(_pdata[p1y]);
   tedfile->put4b(_pdata[p2x]); tedfile->put4b(_pdata[p2y]);
}

void laydata::TdtBox::gdsWrite(DbExportFile& gdsf) const
{
   gdsf.box(_pdata);
}

void laydata::TdtBox::cifWrite(DbExportFile& ciff) const
{
   ciff.box(_pdata);
}

void laydata::TdtBox::psWrite(PSFile& gdsf, const layprop::DrawProperties&) const
{
   gdsf.poly(_pdata, 4, overlap());
}

DBbox laydata::TdtBox::overlap() const
{
   return DBbox(_pdata[p1x], _pdata[p1y], _pdata[p2x], _pdata[p2y]);
}

pointlist laydata::TdtBox::shape2poly() const
{
  // convert box to polygon
   pointlist _plist;
   _plist.push_back(TP(_pdata[p1x], _pdata[p1y]));
   _plist.push_back(TP(_pdata[p2x], _pdata[p1y]));
   _plist.push_back(TP(_pdata[p2x], _pdata[p2y]));
   _plist.push_back(TP(_pdata[p1x], _pdata[p2y]));
   return _plist;
};

void laydata::TdtBox::polyCut(pointlist& cutter, ShapeList** decure)
{
   pointlist _plist = shape2poly();
   // and proceed in the same way as for the polygon
   logicop::logic operation(_plist, cutter);
   try
   {
      operation.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {return;}
   pcollection cut_shapes;
   laydata::TdtData* newshape;
   if (operation.AND(cut_shapes))
   {
      pcollection::const_iterator CI;
      // add the resulting cut_shapes to the_cut ShapeList
      for (CI = cut_shapes.begin(); CI != cut_shapes.end(); CI++)
         if (NULL != (newshape = createValidShape(*CI)))
            decure[1]->push_back(newshape);
      cut_shapes.clear();
      // if there is a cut - there will be (most likely) be cut remains as well
      operation.reset_visited();
      pcollection rest_shapes;
      if (operation.ANDNOT(rest_shapes))
         // add the resulting cut remainings to the_rest ShapeList
         for (CI = rest_shapes.begin(); CI != rest_shapes.end(); CI++)
            if (NULL != (newshape = createValidShape(*CI)))
               decure[2]->push_back(newshape);
      rest_shapes.clear();
      // and finally add this to the_delete shapelist
      decure[0]->push_back(this);
   }
}

void laydata::TdtBox::stretch(int bfactor, ShapeList** decure)
{
   if ( ((_pdata[p1x] - _pdata[p2x]) < 2*bfactor) &&
        ((_pdata[p1y] - _pdata[p2y]) < 2*bfactor)    )
   {
      TP np1(_pdata[p1x] - bfactor, _pdata[p1y] - bfactor );
      TP np2(_pdata[p2x] + bfactor, _pdata[p2y] + bfactor );
      TdtBox* modified = DEBUG_NEW TdtBox(np1, np2);
      decure[1]->push_back(modified);
   }
   decure[0]->push_back(this);
}

pointlist* laydata::TdtBox::movePointsSelected(const SGBitSet& pset,
                                    const CTM&  movedM, const CTM& stableM) const {
  // convert box to polygon
   pointlist* mlist = DEBUG_NEW pointlist();
   mlist->push_back(TP(_pdata[p1x], _pdata[p1y]));
   mlist->push_back(TP(_pdata[p2x], _pdata[p1y]));
   mlist->push_back(TP(_pdata[p2x], _pdata[p2y]));
   mlist->push_back(TP(_pdata[p1x], _pdata[p2y]));

   word size = mlist->size();
   PSegment seg1,seg0;
   // Things to remember in this algo...
   // Each of the points in the initial mlist is recalculated in the seg1.crossP
   // method. This actually means that on pass 0 (i == 0), no points are
   // recalculated because seg0 at that moment is empty. On pass 1 (i == 1),
   // point mlist[1] is recalculated etc. The catch comes on the last pass
   // (i == size) when constructing the seg1, we need mlist[0] and mlist[1], but
   // mlist[1] has been already recalculated and multiplying it with CTM
   // matrix again has pretty funny effect.
   // That's why another condition is introduced -> if (i == size)
   for (unsigned i = 0; i <= size; i++) {
      if (i == size)
         if (pset.check(i%size) && pset.check((i+1) % size))
            seg1 = PSegment((*mlist)[(i  ) % size] * movedM,
                            (*mlist)[(i+1) % size]         );
         else
            seg1 = PSegment((*mlist)[(i  ) % size] * stableM,
                            (*mlist)[(i+1) % size]          );
      else
         if (pset.check(i%size) && pset.check((i+1) % size))
            seg1 = PSegment((*mlist)[(i  ) % size] * movedM,
                            (*mlist)[(i+1) % size] * movedM);
         else
            seg1 = PSegment((*mlist)[(i  ) % size] * stableM,
                            (*mlist)[(i+1) % size] * stableM);
      if (!seg0.empty()) {
         seg1.crossP(seg0,(*mlist)[i%size]);
      }
      seg0 = seg1;
   }
   return mlist;
}

laydata::TdtBox::~TdtBox()
{
}

//-----------------------------------------------------------------------------
// class TdtPoly
//-----------------------------------------------------------------------------
laydata::TdtPoly::TdtPoly(const pointlist& plst) : TdtData()
{
   _psize = plst.size();
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   unsigned index = 0;
   for (unsigned i = 0; i < _psize; i++)
   {
      _pdata[index++] = plst[i].x();
      _pdata[index++] = plst[i].y();
   }
   _teseldata.tessellate(_pdata, _psize);
}

laydata::TdtPoly::TdtPoly(int4b* pdata, unsigned psize) : _pdata(pdata), _psize(psize)
{
   _teseldata.tessellate(_pdata, _psize);
}

laydata::TdtPoly::TdtPoly(TEDfile* const tedfile) : TdtData()
{
   _psize = tedfile->getWord();
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   TP wpnt;
   for (unsigned i = 0 ; i < _psize; i++)
   {
      wpnt = tedfile->getTP();
      _pdata[2*i  ] = wpnt.x();
      _pdata[2*i+1] = wpnt.y();
   }
   _teseldata.tessellate(_pdata, _psize);
}

void laydata::TdtPoly::openGlPrecalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // translate the points using the current CTM
   ptlist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
   {
      ptlist.push_back(TP(_pdata[2*i], _pdata[2*i+1]) * drawprop.topCtm());
   }
}

void laydata::TdtPoly::drawRequest(tenderer::TopRend& rend) const
{
   rend.poly(_pdata, _psize, &_teseldata);
}

void laydata::TdtPoly::drawSRequest(tenderer::TopRend& rend, const SGBitSet* pslist) const
{
   rend.poly(_pdata, _psize, &_teseldata, pslist);
}

void laydata::TdtPoly::openGlDrawLine(layprop::DrawProperties&, const pointlist& ptlist) const
{
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < ptlist.size(); i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::TdtPoly::openGlDrawFill(layprop::DrawProperties&, const pointlist& ptlist) const
{
   for ( TeselChain::const_iterator CCH = _teseldata.tdata()->begin(); CCH != _teseldata.tdata()->end(); CCH++ )
   {
      glBegin(CCH->type());
      for(unsigned cindx = 0 ; cindx < CCH->size(); cindx++)
      {
         unsigned vindex = CCH->index_seq()[cindx];
         glVertex2i(ptlist[vindex].x(), ptlist[vindex].y());
      }
      glEnd();
   }
}

void laydata::TdtPoly::openGlDrawSel(const pointlist& ptlist, const SGBitSet* pslist) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < ptlist.size(); i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
   else if (sh_partsel == status())
   {
      assert(pslist);
      unsigned numpoints = ptlist.size();
      glBegin(GL_LINES);
      for (unsigned i = 0; i < numpoints; i++)
      {
         if (pslist->check(i) && pslist->check((i+1)%numpoints))
         {
            glVertex2i(ptlist[i].x(), ptlist[i].y());
            glVertex2i(ptlist[(i+1)%numpoints].x(), ptlist[(i+1)%numpoints].y());
         }
      }
      glEnd();
   }
}

void laydata::TdtPoly::motionDraw(const layprop::DrawProperties&, ctmqueue& transtack,
                                 SGBitSet* plst) const
{
   CTM trans = transtack.front();
   pointlist* ptlist;
   if (sh_partsel == status())
   {
      CTM strans = transtack.back();
      assert(plst);
      ptlist = movePointsSelected(*plst, trans, strans);
   }
   else
   {
      ptlist = DEBUG_NEW pointlist;
      ptlist->reserve(_psize);
      for (unsigned i = 0; i < _psize; i++)
      {
         ptlist->push_back( TP(_pdata[2*i], _pdata[2*i+1]) * trans);
      }
   }
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < _psize; i++)
   {
      glVertex2i((*ptlist)[i].x(), (*ptlist)[i].y());
   }
   glEnd();
   ptlist->clear();
   delete ptlist;
}

void  laydata::TdtPoly::selectPoints(DBbox& select_in, SGBitSet& pntlst) {
   for (unsigned i = 0; i < _psize; i++)
      if ( select_in.inside( TP(_pdata[2*i], _pdata[2*i+1]) ) ) pntlst.set(i);
   pntlst.check_neighbours_set(false);
}

void laydata::TdtPoly::unselectPoints(DBbox& select_in, SGBitSet& pntlst) {
   if (sh_selected == _status)  // the whole shape use to be selected
      pntlst.setall();
   for (word i = 0; i < _psize; i++)
      if ( select_in.inside( TP(_pdata[2*i], _pdata[2*i+1]) ) ) pntlst.reset(i);
   pntlst.check_neighbours_set(false);
}

laydata::Validator* laydata::TdtPoly::move(const CTM& trans, SGBitSet& plst)
{
   if (0 != plst.size())
   {// modify i.e. move when only part of the polygon is selected
    // should get here only
      pointlist* nshape = movePointsSelected(plst, trans);
      laydata::ValidPoly* check = DEBUG_NEW laydata::ValidPoly(*nshape);
      if (laydata::shp_OK == check->status()) {
         // assign the modified pointlist ONLY if the resulting shape is perfect
         delete [] _pdata;
         _psize = nshape->size();
         _pdata = DEBUG_NEW int4b[2 * _psize];
         for (unsigned i = 0; i < _psize; i++)
         {
            _pdata[2*i] = (*nshape)[i].x();_pdata[2*i+1] = (*nshape)[i].y();
         }
         // retessellate the modified shape
         _teseldata.tessellate(_pdata, _psize);
         nshape->clear(); delete nshape;
         delete check;
         return NULL;
      }
      // in all other cases keep the original pointlist, depending on the check->status()
      // the shape will be replaced, or marked as failed to modify
      nshape->clear(); delete nshape;
      return check;
   }
   else
   {// move when the whole shape is modified should get here.
      if (_psize > 4)
      {
         transfer(trans);
         return NULL;
      }
      else
      {  // The whole gymnastics is because of the Rotate operation (on angles <> 90)
         // Rotated box is converted to polygon. Means that polygon after rotation could
         // produce a box
         pointlist *mlist = DEBUG_NEW pointlist();
         mlist->reserve(_psize);
         for (unsigned i = 0; i < _psize; i++)
            mlist->push_back( TP(_pdata[2*i], _pdata[2*i+1] ) * trans );
         laydata::ValidPoly* check = DEBUG_NEW laydata::ValidPoly(*mlist);
         if (!(laydata::shp_box & check->status()))
         {
            // leave the modified pointlist ONLY if the resulting shape is not a box
            for (unsigned i = 0; i < _psize; i++)
            {
               _pdata[2*i] = (*mlist)[i].x();_pdata[2*i+1] = (*mlist)[i].y();
            }
            delete check; delete mlist;
            return NULL;
         }
         else
         {
            delete mlist;
            return check;
         }
      }
   }
}

void laydata::TdtPoly::transfer(const CTM& trans)
{
   pointlist plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back( TP( _pdata[2*i], _pdata[2*i+1] ) * trans );
   int8b area = polyarea(plist);
   unsigned index = 0;
   if (area < 0ll)
      for (unsigned i = _psize; i > 0; i--)
      {
         _pdata[index++] = plist[i-1].x();
         _pdata[index++] = plist[i-1].y();
      }
   else
      for (unsigned i = 0; i < _psize; i++)
      {
         _pdata[index++] = plist[i].x();
         _pdata[index++] = plist[i].y();
      }
   assert(index == (2*_psize));
   // retessellate the modified shape
   _teseldata.tessellate(_pdata, _psize);
}

laydata::TdtData* laydata::TdtPoly::copy(const CTM& trans)
{
   // copy the points of the polygon
   pointlist ptlist;
   ptlist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      ptlist.push_back( TP(_pdata[2*i], _pdata[2*i+1]) * trans );
   // Don't forget to validate the shape! Because of the CTM - it's
   // quite possible that (for example) the winding has changed which in turn
   // will result in bad results in logic operations! (Issue 52)
   laydata::ValidPoly check(ptlist);
   assert(check.valid());
   return DEBUG_NEW TdtPoly(check.getValidated());
}

bool laydata::TdtPoly::point_inside(TP pnt)
{
   TP p0, p1;
   byte cc = 0;
   for (unsigned i = 0; i < _psize ; i++)
   {
      p0 = TP(_pdata[2 *   i             ], _pdata[2 *   i              + 1]);
      p1 = TP(_pdata[2 * ((i+1) % _psize)], _pdata[2 * ((i+1) % _psize) + 1]);
      if (((p0.y() <= pnt.y()) && (p1.y() >  pnt.y()))
        ||((p0.y() >  pnt.y()) && (p1.y() <= pnt.y())) ) {
         float tngns = (float) (pnt.y() - p0.y())/(p1.y() - p0.y());
         if (pnt.x() < p0.x() + tngns * (p1.x() - p0.x()))
            cc++;
      }
   }
   return (cc & 0x01) ? true : false;
}

pointlist laydata::TdtPoly::shape2poly() const
{
   pointlist plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back(TP(_pdata[2*i], _pdata[2*i+1]));
   return plist;
};

void laydata::TdtPoly::polyCut(pointlist& cutter, ShapeList** decure)
{
   pointlist plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back( TP( _pdata[2*i], _pdata[2*i+1] ) );

   logicop::logic operation(plist, cutter);
   try
   {
      operation.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {return;}
   pcollection cut_shapes;
   laydata::TdtData* newshape;
   if (operation.AND(cut_shapes))
   {
      pcollection::const_iterator CI;
      // add the resulting cut_shapes to the_cut ShapeList
      for (CI = cut_shapes.begin(); CI != cut_shapes.end(); CI++)
         if (NULL != (newshape = createValidShape(*CI)))
            decure[1]->push_back(newshape);
      cut_shapes.clear();
      // if there is a cut - there should be cut remains as well
      operation.reset_visited();
      pcollection rest_shapes;
      if (operation.ANDNOT(rest_shapes))
         // add the resulting cut remainings to the_rest ShapeList
         for (CI = rest_shapes.begin(); CI != rest_shapes.end(); CI++)
            if (NULL != (newshape = createValidShape(*CI)))
               decure[2]->push_back(newshape);
      rest_shapes.clear();
      // and finally add this to the_delete shapelist
      decure[0]->push_back(this);
   }
}

void laydata::TdtPoly::stretch(int bfactor, ShapeList** decure)
{
   pointlist plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back( TP( _pdata[2*i], _pdata[2*i+1] ) );

   logicop::stretcher sh_resize(plist, bfactor);
   pointlist* res = sh_resize.execute();
   ValidPoly vsh(*res);
   if (vsh.valid() && !(laydata::shp_clock & vsh.status()))
   {
      decure[0]->push_back(this);
      decure[1]->push_back(vsh.replacement());
   }
   else if ( vsh.recoverable() && (!(laydata::shp_clock & vsh.status())) )
   {
      logicop::CrossFix fixingpoly(*res, true);
      try
      {
         fixingpoly.findCrossingPoints();
      }
      catch (EXPTNpolyCross) {return;}
      if (1 == fixingpoly.crossp())
      throw EXPTNpolyCross("Only one crossing point found. Can't generate polygons");

      pcollection cut_shapes;
      laydata::TdtData* newshape;
      if ( fixingpoly.generate(cut_shapes, bfactor) )
      {
         pcollection::const_iterator CI;
         // add the resulting fixed_shapes to the_cut ShapeList
         for (CI = cut_shapes.begin(); CI != cut_shapes.end(); CI++)
            if (NULL != (newshape = createValidShape(*CI)))
               decure[1]->push_back(newshape);
         cut_shapes.clear();
         decure[0]->push_back(this);
      }
      // Normally generate shall return always some valid shapes
//      assert(false);
   }
   else
   {
      // resulting polygon is exactly with 0 area (non-recoverable) OR
      // resulting polygon is turned completely inside out shp_clock & vsh.status()
      // in both cases - it shall dissapear
      decure[0]->push_back(this);
   }
   delete res;
}

void laydata::TdtPoly::info(std::ostringstream& ost, real DBU) const
{
   ost << "polygon - {";
   for (unsigned i = 0; i < _psize; i++)
   {
      TP cpnt(_pdata[2*i], _pdata[2*i+1]);
      cpnt.info(ost, DBU);
      if (i != _psize - 1) ost << " , ";
   }
   ost << "};";
}

void laydata::TdtPoly::write(TEDfile* const tedfile) const
{
   tedfile->putByte(tedf_POLY);
   tedfile->putWord(_psize);
   for (unsigned i = 0; i < _psize; i++)
   {
      tedfile->put4b(_pdata[2*i]); tedfile->put4b(_pdata[2*i+1]);
   }
}

void laydata::TdtPoly::gdsWrite(DbExportFile& gdsf) const
{
   gdsf.polygon(_pdata, _psize);
}

void laydata::TdtPoly::cifWrite(DbExportFile& ciff) const
{
   ciff.polygon(_pdata, _psize);
}

void laydata::TdtPoly::psWrite(PSFile& gdsf, const layprop::DrawProperties&) const
{
   gdsf.poly(_pdata, _psize, overlap());
}

DBbox laydata::TdtPoly::overlap() const
{
   DBbox ovl(_pdata[0], _pdata[1]) ;
   for (word i = 1; i < _psize; i++)
      ovl.overlap(_pdata[2*i], _pdata[2*i+1]);
   return ovl;
}

pointlist* laydata::TdtPoly::movePointsSelected(const SGBitSet& pset,
                                    const CTM&  movedM, const CTM& stableM) const {
   pointlist* mlist = DEBUG_NEW pointlist();
   mlist->reserve(_psize);
   for (unsigned i = 0 ; i < _psize; i++ )
      mlist->push_back(TP(_pdata[2*i], _pdata[2*i+1]));

   PSegment seg1,seg0;
   // See the note about this algo in TdtBox::movePointsSelected above
   for (unsigned i = 0; i <= _psize; i++) {
      if (i == _psize)
         if (pset.check(i % _psize) && pset.check((i+1) % _psize))
            seg1 = PSegment((*mlist)[(i  ) % _psize] * movedM,
                            (*mlist)[(i+1) % _psize]         );
         else
            seg1 = PSegment((*mlist)[(i  ) % _psize] * stableM,
                            (*mlist)[(i+1) % _psize]         );
      else
         if (pset.check(i % _psize) && pset.check((i+1) % _psize))
            seg1 = PSegment((*mlist)[(i  ) % _psize] * movedM,
                            (*mlist)[(i+1) % _psize] * movedM);
         else
            seg1 = PSegment((*mlist)[(i  ) % _psize] * stableM,
                            (*mlist)[(i+1) % _psize] * stableM);
      if (!seg0.empty()) {
         seg1.crossP(seg0,(*mlist)[ i % _psize]);
      }
      seg0 = seg1;
   }
   return mlist;
}

laydata::TdtPoly::~TdtPoly()
{
   delete [] _pdata;
}

//-----------------------------------------------------------------------------
// class TdtWire
//-----------------------------------------------------------------------------
laydata::TdtWire::TdtWire(const pointlist& plst, word width) : TdtData(), _width(width)
{
   _psize = plst.size();
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   for (unsigned i = 0; i < _psize; i++)
   {
      _pdata[2*i  ] = plst[i].x();
      _pdata[2*i+1] = plst[i].y();
   }
}

laydata::TdtWire::TdtWire(const int4b* pdata, unsigned psize, word width) :
      TdtData(), _width(width), _psize(psize)
{
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   memcpy(_pdata, pdata, 2*_psize);
}

laydata::TdtWire::TdtWire(TEDfile* const tedfile) : TdtData()
{
   _psize = tedfile->getWord();
   assert(_psize);
   _width = tedfile->getWord();
   _pdata = DEBUG_NEW int4b[_psize*2];
   TP wpnt;
   for (unsigned i = 0 ; i < _psize; i++)
   {
      wpnt = tedfile->getTP();
      _pdata[2*i  ]  = wpnt.x();
      _pdata[2*i+1]  = wpnt.y();
   }
}

DBbox* laydata::TdtWire::endPnts(const TP& p1, const TP& p2, bool first) const
{
   double     w = _width/2;
   double denom = first ? (p2.x() - p1.x()) : (p1.x() - p2.x());
   double   nom = first ? (p2.y() - p1.y()) : (p1.y() - p2.y());
   double xcorr, ycorr; // the corrections
   if ((0 == nom) && (0 == denom))
   { // coinciding points
      return DEBUG_NEW DBbox(/*p1,p2 */DEFAULT_OVL_BOX);
   }
   double signX = (  nom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   double signY = (denom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   if      (0 == denom)   {xcorr =signX * w ; ycorr = 0;} // vertical
   else if (0 == nom  )   {xcorr = 0 ; ycorr = signY * w;} // horizontal |----|
   else
   {
      double sl   = nom / denom;
      double sqsl = signY*sqrt( sl*sl + 1);
      xcorr = rint(w * (sl / sqsl));
      ycorr = rint(w * ( 1 / sqsl));
   }
   TP pt = first ? p1 : p2;
   return DEBUG_NEW DBbox((int4b) rint(pt.x() - xcorr), (int4b) rint(pt.y() + ycorr),
                          (int4b) rint(pt.x() + xcorr), (int4b) rint(pt.y() - ycorr));
}

DBbox* laydata::TdtWire::mdlPnts(const TP& p1, const TP& p2, const TP& p3) const
{
   double    w = _width/2;
   double  x32 = p3.x() - p2.x();
   double  x21 = p2.x() - p1.x();
   double  y32 = p3.y() - p2.y();
   double  y21 = p2.y() - p1.y();
   double   L1 = sqrt(x21*x21 + y21*y21); //the length of segment 1
   double   L2 = sqrt(x32*x32 + y32*y32); //the length of segment 2
   double denom = x32 * y21 - x21 * y32;
// @FIXME THINK about next two lines!!!    They are wrong !!!
   if ((0 == denom) || (0 == L1)) return endPnts(p2,p3,false);
   if (0 == L2) return NULL;
   // the corrections
   double xcorr = w * ((x32 * L1 - x21 * L2) / denom);
   double ycorr = w * ((y21 * L2 - y32 * L1) / denom);
   return DEBUG_NEW DBbox((int4b) rint(p2.x() - xcorr), (int4b) rint(p2.y() + ycorr),
                          (int4b) rint(p2.x() + xcorr), (int4b) rint(p2.y() - ycorr));
}

void laydata::TdtWire::openGlPrecalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(_width,_width));
   bool center_line_only = !wsquare.visible(drawprop.topCtm() * drawprop.scrCtm(), drawprop.visualLimit());
   if (center_line_only)
      ptlist.reserve(_psize);
   else
      ptlist.reserve(3 * _psize);
   // translate the points using the current CTM
   for (unsigned i = 0; i < _psize; i++)
      ptlist.push_back( TP( _pdata[2*i], _pdata[2*i+1] ) * drawprop.topCtm());
   if (!center_line_only)
      precalc(ptlist, _psize);
}

void laydata::TdtWire::drawRequest(tenderer::TopRend& rend) const
{
   rend.wire(_pdata, _psize, _width);
}

void laydata::TdtWire::drawSRequest(tenderer::TopRend& rend, const SGBitSet* pslist) const
{
   rend.wire(_pdata, _psize, _width, pslist);
}

void laydata::TdtWire::openGlDrawLine(layprop::DrawProperties&, const pointlist& ptlist) const
{
   dword num_points = ptlist.size();
   if (0 == ptlist.size()) return;
   // to keep MS VC++ happy - define the counter outside the loops
   dword i;
   dword num_cpoints = (num_points == _psize) ? num_points : num_points / 3;
   // draw the central line in all cases
   if (0 == num_cpoints) return;
   glBegin(GL_LINE_STRIP);
   for (i = 0; i < num_cpoints; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
   // now check whether to draw only the center line
   if (num_cpoints == num_points) return;
   // draw the wire contour
   glBegin(GL_LINE_LOOP);
   for (i = num_cpoints; i < 3 * num_cpoints; i = i + 2)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   for (i = 3 * num_cpoints - 1; i > num_cpoints; i = i - 2)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::TdtWire::openGlDrawFill(layprop::DrawProperties&, const pointlist& ptlist) const
{
   if (_psize == ptlist.size()) return;
   glBegin(GL_QUAD_STRIP);
   for (dword i = _psize; i < 3 *_psize; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::TdtWire::openGlDrawSel(const pointlist& ptlist, const SGBitSet* pslist) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_STRIP);
      for (unsigned i = 0; i < _psize; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
   else if (sh_partsel == status())
   {
      assert(pslist);
      glBegin(GL_LINES);
      for (unsigned i = 0; i < _psize-1; i++)
      {
         if (pslist->check(i) && pslist->check((i+1)%_psize))
         {
            glVertex2i(ptlist[i].x(), ptlist[i].y());
            glVertex2i(ptlist[(i+1)%_psize].x(), ptlist[(i+1)%_psize].y());
         }
      }
      if (pslist->check(0))
      {// if only the first is selected
         glVertex2i(ptlist[_psize].x(), ptlist[_psize].y());
         glVertex2i(ptlist[_psize+1].x(), ptlist[_psize+1].y());
      }
      if (pslist->check(_psize-1))
      {// if only the last is selected
         glVertex2i(ptlist[3*_psize-1].x(), ptlist[3*_psize-1].y());
         glVertex2i(ptlist[3*_psize-2].x(), ptlist[3*_psize-2].y());
      }
      glEnd();
   }
}

void laydata::TdtWire::motionDraw(const layprop::DrawProperties& drawprop,
               ctmqueue& transtack, SGBitSet* plst) const
{
   CTM trans = transtack.front();
   pointlist* ptlist;
   if (sh_partsel == status())
   {
      CTM strans = transtack.back();
      assert(plst);
      ptlist = movePointsSelected(*plst, trans, strans);
   }
   else
   {
      ptlist = DEBUG_NEW pointlist;
      for (unsigned i = 0; i < _psize; i++)
         ptlist->push_back( TP( _pdata[2*i], _pdata[2*i+1] ) * trans );
   }
   precalc(*ptlist, _psize);
   openGlDrawLine(const_cast<layprop::DrawProperties&>(drawprop), *ptlist);
//      if (drawprop.getCurrentFill())
//         openGlDrawFill(ptlist);
   ptlist->clear(); delete ptlist;
}

void laydata::TdtWire::precalc(pointlist& ptlist, dword num_points) const
{
   DBbox* ln1 = endPnts(ptlist[0],ptlist[1], true);
   if (NULL != ln1)
   {
      ptlist.push_back(ln1->p1());
      ptlist.push_back(ln1->p2());
   }
   delete ln1;
   for (unsigned i = 1; i < num_points - 1; i++)
   {
      ln1 = mdlPnts(ptlist[i-1],ptlist[i],ptlist[i+1]);
      if (NULL != ln1)
      {
         ptlist.push_back(ln1->p1());
         ptlist.push_back(ln1->p2());
      }
      delete ln1;
   }
   ln1 = endPnts(ptlist[num_points-2],ptlist[num_points-1],false);
   if (NULL != ln1)
   {
      ptlist.push_back(ln1->p1());
      ptlist.push_back(ln1->p2());
   }
   delete ln1;
}

bool laydata::TdtWire::point_inside(TP pnt)
{
   TP p0, p1;
   for (unsigned i = 0; i < _psize - 1 ; i++)
   {
      p0 = TP(_pdata[2* i   ], _pdata[2* i   +1]);
      p1 = TP(_pdata[2*(i+1)], _pdata[2*(i+1)+1]);
      float distance = get_distance(p0,p1,pnt);
      if ((distance >= 0) && (distance <= _width/2))
         return true;
   }
   return false;
}

float laydata::TdtWire::get_distance(TP p1, TP p2, TP p0)
{
   if (p1.x() == p2.x())
      // if the segment is parallel to Y axis
      if ( ((p0.y() >= p1.y()) && (p0.y() <= p2.y()))
         ||((p0.y() <= p1.y()) && (p0.y() >= p2.y())) )
         return fabsf(p0.x() - p1.x());
      else return -1;
   else if (p1.y() == p2.y())
      // if the segment is parallel to X axis
      if ( ((p0.x() >= p1.x()) && (p0.x() <= p2.x()))
         ||((p0.x() <= p1.x()) && (p0.x() >= p2.x())) )
         return fabsf(p0.y() - p1.y());
      else return -1;
   else {
      // segment is not parallel to any axis
      float A = p2.y() - p1.y();
      float B = p1.x() - p2.x();
      float C = - (p1.y() * B) - (p1.x() * A);
      float dist = A*A + B*B;
      float Cn = A*p0.x() + B*p0.y() + C;
      float X = p0.x() - (A / dist) * Cn;
      float Y = p0.y() - (B / dist) * Cn;
      // now check that the new coordinate is on the p1-p2 line
      if ((((Y >= p1.y()) && (Y <= p2.y()))||((Y <= p1.y()) && (Y >= p2.y()))) &&
          (((X >= p1.x()) && (X <= p2.x()))||((X <= p1.x()) && (X >= p2.x())))   )
         return fabsf(Cn / sqrt(dist));
      else return -1;
   }
}

void  laydata::TdtWire::selectPoints(DBbox& select_in, SGBitSet& pntlst)
{
   for (word i = 0; i < _psize; i++)
      if (select_in.inside( TP(_pdata[2*i], _pdata[2*i+1]) ) ) pntlst.set(i);
   pntlst.check_neighbours_set(true);
}

void laydata::TdtWire::unselectPoints(DBbox& select_in, SGBitSet& pntlst)
{
   if (sh_selected == _status) // the whole shape use to be selected
      pntlst.setall();
   for (word i = 0; i < _psize; i++)
      if (select_in.inside( TP(_pdata[2*i], _pdata[2*i+1]) ) ) pntlst.reset(i);
   pntlst.check_neighbours_set(true);
}

laydata::Validator* laydata::TdtWire::move(const CTM& trans, SGBitSet& plst)
{
   if (0 != plst.size())
   {
      pointlist* nshape = movePointsSelected(plst, trans);
      laydata::ValidWire* check = DEBUG_NEW laydata::ValidWire(*nshape, _width);
      if (laydata::shp_OK == check->status()) {
         // assign the modified pointlist ONLY if the resulting shape is perfect
         delete [] _pdata;
         _psize = nshape->size();
         _pdata = DEBUG_NEW int4b[2 * _psize];
         for (unsigned i = 0; i < _psize; i++)
         {
            _pdata[2*i] = (*nshape)[i].x();_pdata[2*i+1] = (*nshape)[i].y();
         }
         nshape->clear(); delete nshape;
         delete check;
         return NULL;
      }
      // in all other cases keep the original pointlist, depending on the check->status()
      // the shape will be replaced, or marked as failed to modify
      return check;
   }
   else transfer(trans);
   return NULL;
}

void laydata::TdtWire::transfer(const CTM& trans)
{
   for (unsigned i = 0; i < _psize; i++)
   {
      TP cpnt(_pdata[2*i], _pdata[2*i+1]);
      cpnt *= trans;
      _pdata[2*i  ] = cpnt.x();
      _pdata[2*i+1] = cpnt.y();
   }
}

laydata::TdtData* laydata::TdtWire::copy(const CTM& trans) {
   // copy the points of the wire
   pointlist ptlist;
   ptlist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      ptlist.push_back( TP( _pdata[2*i], _pdata[2*i+1] ) * trans);
   //
   ValidWire check(ptlist, _width);
   assert(check.valid());
   return DEBUG_NEW TdtWire(check.getValidated(),_width);
}

void laydata::TdtWire::stretch(int bfactor, ShapeList** decure)
{
   //@TODO cut bfactor from both sides
   if ((2*bfactor + _width) > 0)
   {
      TdtWire* modified = DEBUG_NEW TdtWire(_pdata, _psize, 2*bfactor + _width);
      decure[1]->push_back(modified);
   }
   decure[0]->push_back(this);
}

pointlist laydata::TdtWire::shape2poly() const
{
/**
 * All the gymnastics here is because of the way precalc() function is
 * providing its result. The whole thing is that we have this function
 * (precalc) implemented 3 times. In this class, in TdtTmpWire and in
 * tenderer::TenderWire. The latter is the proper implementation.
 * Because of re-usability reasons at the moment we have to use the
 * implementation in this class.
 * TODO! Clean-up all the implementations above and create a class
 * next to ValidWire in here, in tedat which only purpose will be to
 * convert wire to list of points
 */
   pointlist plist;
   plist.reserve(3*_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back( TP( _pdata[2*i], _pdata[2*i+1] ));
   precalc(plist,_psize);

   pointlist result;
   plist.reserve(2 * _psize);
   for (unsigned i = _psize; i < 3 * _psize; i = i + 2)
      result.push_back(plist[i]);
   for (unsigned i = 3 * _psize - 1; i > _psize; i = i - 2)
      result.push_back(plist[i]);

   ValidPoly check(result);
   assert(check.valid());
   return check.getValidated();
}

void laydata::TdtWire::info(std::ostringstream& ost, real DBU) const
{
   ost << "wire " << _width/DBU << " - {";
   for (unsigned i = 0; i < _psize; i++)
   {
      TP cpnt(_pdata[2*i], _pdata[2*i+1]);
      cpnt.info(ost, DBU);
      if (i != _psize - 1) ost << " , ";
   }
   ost << "};";
}

void laydata::TdtWire::write(TEDfile* const tedfile) const
{
   tedfile->putByte(tedf_WIRE);
   tedfile->putWord(_psize);
   tedfile->putWord(_width);
   for (word i = 0; i < _psize; i++)
   {
      tedfile->put4b(_pdata[2*i]); tedfile->put4b(_pdata[2*i+1]);
   }
}

void laydata::TdtWire::gdsWrite(DbExportFile& gdsf) const
{
   gdsf.wire(_pdata, _psize, _width);
}

void laydata::TdtWire::cifWrite(DbExportFile& ciff) const
{
   ciff.wire(_pdata, _psize, _width);
}

void laydata::TdtWire::psWrite(PSFile& gdsf, const layprop::DrawProperties&) const
{
   gdsf.wire(_pdata, _psize, _width, overlap());
}

DBbox laydata::TdtWire::overlap() const
{
   DBbox* ln1 = endPnts(TP( _pdata[0], _pdata[1]),
                        TP( _pdata[2], _pdata[3]),
                        true
                        );
   DBbox ovl = *ln1; delete ln1;
   DBbox* ln2 = NULL;
   for (word i = 1; i < _psize - 1; i++)
   {
      ln2 = mdlPnts(TP(_pdata[2*(i-1)], _pdata[2*(i-1) + 1]),
                    TP(_pdata[2* i   ], _pdata[2* i    + 1]),
                    TP(_pdata[2*(i+1)], _pdata[2*(i+1) + 1])
                    );
      ovl.overlap(*ln2);
      delete ln2;
   }
   ln1 = endPnts(TP( _pdata[2*(_psize-2)], _pdata[2*(_psize-2) + 1]),
                 TP( _pdata[2*(_psize-1)], _pdata[2*(_psize-1) + 1]),
                 false
                 );
   ovl.overlap(*ln1);
   delete ln1;
   return ovl;
}

pointlist* laydata::TdtWire::movePointsSelected(const SGBitSet& pset,
                                    const CTM& movedM, const CTM& stableM) const
{
   pointlist* mlist = DEBUG_NEW pointlist();
   mlist->reserve(_psize);
   for (unsigned i = 0 ; i < _psize; i++ )
      mlist->push_back(TP(_pdata[2*i], _pdata[2*i+1]));

   PSegment* seg1 = NULL;
   PSegment* seg0 = NULL;
   for (unsigned i = 0; i < _psize; i++)
   {
      if ((_psize-1) == i)
      {
         if (pset.check(_psize-1))
            seg1 = seg1->ortho((*mlist)[_psize-1] * movedM);
         else
            seg1 = seg1->ortho((*mlist)[_psize-1] * stableM);
      }
      else
      {
         const CTM& transM = ((pset.check(i) && pset.check(i+1))) ?
                                                               movedM : stableM;
         seg1 = DEBUG_NEW PSegment((*mlist)[(i  )] * transM, (*mlist)[(i+1)] * transM);
         if (0 == i)
         {
            if (pset.check(0))
               seg0 = seg1->ortho((*mlist)[i] * movedM);
            else
               seg0 = seg1->ortho((*mlist)[i] * stableM);
         }
      }
      if (!seg0->empty()) seg1->crossP(*seg0,(*mlist)[i]);
      if (NULL != seg0) delete seg0;
      seg0 = seg1;
   }
   if (NULL != seg0) delete seg0;
   return mlist;
}

laydata::TdtWire::~TdtWire()
{
   delete [] _pdata;
}

//-----------------------------------------------------------------------------
// class TdtCellRef
//-----------------------------------------------------------------------------
laydata::TdtCellRef::TdtCellRef(TEDfile* const tedfile)
{
   // read the name of the referenced cell
   std::string cellrefname = tedfile->getString();
   // get the cell definition pointer and register the cellrefname as a child
   // to the currently parsed cell
   _structure = tedfile->linkCellRef(cellrefname);
   // get the translation
   _translation = tedfile->getCTM();
}

void laydata::TdtCellRef::openGlPrecalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // calculate the current translation matrix
   CTM newtrans = _translation * drawprop.topCtm();
   // get overlapping box of the structure ...
   DBbox obox(DEFAULT_ZOOM_BOX);
   if (structure())
      obox = structure()->cellOverlap();
   // ... translate it to the current coordinates ...
   DBbox areal = obox.overlap(newtrans);
   // check that the cell (or part of it) is in the visual window
   DBbox clip = drawprop.clipRegion();
   if (0ll == clip.cliparea(areal)) return;
   // check that the cell area is bigger that the MIN_VISUAL_AREA
   if (!areal.visible(drawprop.scrCtm(), drawprop.visualLimit())) return;
   // If we get here - means that the cell (or part of it) is visible
   ptlist.reserve(4);
   ptlist.push_back(obox.p1() * newtrans);
   ptlist.push_back(TP(obox.p2().x(), obox.p1().y()) * newtrans);
   ptlist.push_back(obox.p2() * newtrans);
   ptlist.push_back(TP(obox.p1().x(), obox.p2().y()) * newtrans);
   drawprop.pushCtm(newtrans);
   // draw the cell mark ...
   drawprop.drawReferenceMarks(TP(0,0) * newtrans, layprop::cell_mark);
}

void laydata::TdtCellRef::drawRequest(tenderer::TopRend& rend) const
{
   // get overlapping box of the structure ...
   DBbox obox(structure()->cellOverlap());
   // ... translate it to the current coordinates ...
   DBbox areal = obox.overlap(_translation * rend.topCTM());
   if (!areal.visible(rend.ScrCTM(), rend.visualLimit())) return;
   // draw the cell mark ...
//   rend.drawReferenceMarks(TP(0,0) * newtrans, layprop::cell_mark);
   //

   layprop::CellRefChainType crchain;
   if (rend.preCheckCRS(this, crchain))
   {
      structure()->openGlRender(rend, _translation, false, (layprop::crc_ACTIVE == crchain));
      if ((layprop::crc_PREACTIVE == crchain) ||
          (layprop::crc_ACTIVE    == crchain)    ) rend.postCheckCRS(this);
   }
}

void laydata::TdtCellRef::vlOverlap(const layprop::DrawProperties& prop, DBbox& vlOvl) const
{
   assert(NULL != structure());
   DBbox strOverlap(structure()->getVisibleOverlap(prop));
   if (DEFAULT_OVL_BOX == strOverlap) return;
   strOverlap = strOverlap * _translation;
   strOverlap.normalize();
   vlOvl.overlap(strOverlap);
}

void laydata::TdtCellRef::drawSRequest(tenderer::TopRend& rend, const SGBitSet*) const
{
   // get overlapping box of the structure ...
   DBbox obox(structure()->cellOverlap());
   // ... translate it to the current coordinates ...
   DBbox areal = obox.overlap(_translation * rend.topCTM());
   if (!areal.visible(rend.ScrCTM(), rend.visualLimit())) return;
   // draw the cell mark ...
//   rend.drawReferenceMarks(TP(0,0) * newtrans, layprop::cell_mark);
   //
   layprop::CellRefChainType crchain;
   if (rend.preCheckCRS(this, crchain))
   {
      structure()->openGlRender(rend, _translation, true, (layprop::crc_ACTIVE == crchain));
      if ((layprop::crc_PREACTIVE == crchain) ||
          (layprop::crc_ACTIVE    == crchain)    ) rend.postCheckCRS(this);
   }

}

void laydata::TdtCellRef::openGlDrawLine(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   drawprop.drawCellBoundary(ptlist);
}


void laydata::TdtCellRef::openGlDrawFill(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if ((NULL == structure()) || (0 == ptlist.size())) return;
   // draw the structure itself. Pop/push ref stuff is when edit in place is active
   layprop::CellRefChainType crchain = drawprop.preCheckCRS(this);
   structure()->openGlDraw(drawprop, (layprop::crc_ACTIVE == crchain));
   if (layprop::crc_VIEW != crchain) drawprop.postCheckCRS(this);
}

void laydata::TdtCellRef::openGlDrawSel(const pointlist& ptlist, const SGBitSet*) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
}

void laydata::TdtCellRef::openGlPostClean(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   ptlist.clear();
   // get the font matrix out of the stack (pushed in precalc)
   drawprop.popCtm();
}

void laydata::TdtCellRef::motionDraw(const layprop::DrawProperties& drawprop,
                 ctmqueue& transtack, SGBitSet*) const
{
   if (structure())
   {
      transtack.push_front(_translation * transtack.front());
      structure()->motionDraw(drawprop, transtack);
   }
}

void laydata::TdtCellRef::info(std::ostringstream& ost, real DBU) const {
   ost << "cell \"" << _structure->name() << "\" - reference @ {";
   ost << _translation.tx()/DBU << " , " << _translation.ty()/DBU << "}";
}

void laydata::TdtCellRef::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_CELLREF);
   tedfile->putString(_structure->name());
   tedfile->putCTM(_translation);
}

laydata::TdtCell* laydata::TdtCellRef::cStructure() const
{
   laydata::TdtCell* celldef;
   if (_structure->libID())
      celldef =  static_cast<laydata::TdtCell*>(_structure);
   else
      celldef = NULL;
   return celldef;
}

std::string laydata::TdtCellRef::cellname() const
{
   return _structure->name();
}

void laydata::TdtCellRef::gdsWrite(DbExportFile& gdsf) const
{
   gdsf.ref(_structure->name(), _translation);
}

void laydata::TdtCellRef::cifWrite(DbExportFile& ciff) const
{
   ciff.ref(_structure->name(), _translation);
}

void laydata::TdtCellRef::psWrite(PSFile& psf, const layprop::DrawProperties& drawprop) const
{
   psf.cellref(_structure->name(), _translation);
   if (!psf.hier())
   {
     _structure->psWrite(psf, drawprop);
   }
}

void laydata::TdtCellRef::ungroup(laydata::TdtDesign* ATDB, TdtCell* dst, AtticList* nshp)
{
   TdtData *data_copy;
   ShapeList* ssl;
   TdtCell* cstr = cStructure();
   // select all the shapes of the referenced TdtCell
   if (NULL == cstr)
   {
      std::ostringstream ost;
      ost << "Cell \"" << structure()->name() << "\" is undefined. Ignored during ungoup.";
      tell_log(console::MT_WARNING, ost.str());
      return;
   }
   cstr->fullSelect();
   for (SelectList::const_iterator CL = cstr->shapeSel()->begin();
                                   CL != cstr->shapeSel()->end(); CL++)
   {
      // secure the target layer
      QTreeTmp* wl = dst->secureUnsortedLayer(CL->first);
      // There is no point here to ensure that the layer definition exists.
      // We are just transferring shapes from one structure to another.
      // Of course ATDB is undefined (forward defined) here, so if the method has to be
      // used here - something else should be done
      // ATDB->securelaydef( CL->first );
      // secure the select layer (for undo)
      if (nshp->end() != nshp->find(CL->first))
         ssl = (*nshp)[CL->first];
      else {
         ssl = DEBUG_NEW ShapeList();
         (*nshp)[CL->first] = ssl;
      }
      // for every single shape on the layer
      for (DataList::const_iterator DI = CL->second->begin();
                                    DI != CL->second->end(); DI++)
      {
         // create a new copy of the data
         data_copy = DI->first->copy(_translation);
         // add it to the corresponding layer of the dst cell
         wl->put(data_copy);
         // ... and to the list of the new shapes (for undo)
         ssl->push_back(data_copy);
         //update the hierarchy tree if this is a cell
         if (REF_LAY == CL->first) dst->addChild(ATDB,
                            static_cast<TdtCellRef*>(data_copy)->cStructure());
         // add it to the selection list of the dst cell
         dst->selectThis(data_copy,CL->first);
      }
   }
   cstr->unselectAll();
}

DBbox laydata::TdtCellRef::overlap() const
{
   assert(NULL != structure());
   DBbox ovl(structure()->cellOverlap().overlap(_translation));
   ovl.normalize();
   return ovl;
}

//-----------------------------------------------------------------------------
// class TdtCellAref
//-----------------------------------------------------------------------------
laydata::TdtCellAref::TdtCellAref(TEDfile* const tedfile) : TdtCellRef(tedfile)
{
   int4b _stepX = tedfile->get4b();
   int4b _stepY = tedfile->get4b();
   word _rows = tedfile->getWord();
   word _cols = tedfile->getWord();
   _arrprops = ArrayProperties(_stepX, _stepY, _cols, _rows);
}


void laydata::TdtCellAref::openGlPrecalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // make sure that the referenced structure exists
   assert(structure());
   // Get the areal of entire matrix, but NOT TRANSLATED !
   DBbox array_overlap = clearOverlap();
   // Calculate the CTM for the array
   CTM newtrans = _translation * drawprop.topCtm();
   // ... get the current visual (clipping) window, and make a REVERSE TRANSLATION
   DBbox clip = drawprop.clipRegion().overlap(newtrans.Reversed());
   // initialize the visual box from the overlap area of the array ...
   DBbox visual_box(array_overlap);
   // ... and check the visibility of entire array. if mutual_position
   // is 1, visual_box will be modified and will contain the visual region
   // of the array
   int mutual_position = clip.clipbox(visual_box);
   // if array is entirely outside the visual window - bail out
   if (0 == mutual_position) return;

   // If we get here - means that the array (or part of it) is visible
   // draw the cell mark ...
   drawprop.drawReferenceMarks(TP(0,0) * newtrans, layprop::array_mark);
   // ... and the overlapping box
   ptlist.reserve(6); //0:3 - the overlapping box; 4 - number of columns; 5 - number of rows
   ptlist.push_back(               array_overlap.p1()                  * newtrans);
   ptlist.push_back(TP(array_overlap.p2().x(), array_overlap.p1().y()) * newtrans);
   ptlist.push_back(               array_overlap.p2()                  * newtrans);
   ptlist.push_back(TP(array_overlap.p1().x(), array_overlap.p2().y()) * newtrans);

   // We are going to draw "something", so push the new translation matrix in the stack
   drawprop.pushCtm(newtrans);
   if (structure()->cellOverlap().visible(drawprop.topCtm() * drawprop.scrCtm(), drawprop.visualLimit()))
   {
      // a single structure is big enough to be visible
      // now calculate the start/stop values of the visible references in the matrix
      if (-1 == mutual_position) {
         // entire matrix is visible
         ptlist.push_back(TP(0,_arrprops.cols()));
         ptlist.push_back(TP(0,_arrprops.rows()));
      }
      else {
         int stst[4];
         real cstepX = (array_overlap.p2().x() - array_overlap.p1().x()) / _arrprops.cols();
         real cstepY = (array_overlap.p2().y() - array_overlap.p1().y()) / _arrprops.rows();
         // matrix is partially visible
         stst[0] = array_overlap.p1().x() < clip.p1().x() ?
               (int) rint((clip.p1().x() - array_overlap.p1().x()) / cstepX) : 0;
         stst[2] = array_overlap.p1().y() < clip.p1().y() ?
               (int) rint((clip.p1().y() - array_overlap.p1().y()) / cstepY) : 0;
         stst[1] = stst[0] + (int) rint((visual_box.p2().x() - visual_box.p1().x()) / cstepX);
         stst[3] = stst[2] + (int) rint((visual_box.p2().y() - visual_box.p1().y()) / cstepY);
         // add an extra row/column from both sides to ensure visibility of the`
         // border areas
         stst[0] -= (0 == stst[0]) ? 0 : 1;
         stst[2] -= (0 == stst[2]) ? 0 : 1;
         stst[1] += (_arrprops.cols() == stst[1]) ? 0 : 1;
         stst[3] += (_arrprops.rows() == stst[3]) ? 0 : 1;
         ptlist.push_back(TP(stst[0],stst[1]));
         ptlist.push_back(TP(stst[2],stst[3]));
      }
   }
   else
   {
      // a single structure is too small
      ptlist.push_back(TP(0,0));
      ptlist.push_back(TP(0,0));
   }
}

void laydata::TdtCellAref::drawRequest(tenderer::TopRend& rend) const
{

   // make sure that the referenced structure exists
   assert(structure());
   // Get the areal of entire matrix, but NOT TRANSLATED !
   DBbox array_overlap = clearOverlap();
   // Calculate the CTM for the array
   CTM newtrans = _translation * rend.topCTM();
   // ... get the current visual (clipping) window, and make a REVERSE TRANSLATION
   DBbox clip = rend.clipRegion().overlap(newtrans.Reversed());
   // initialize the visual box from the overlap area of the array ...
   DBbox visual_box(array_overlap);
   // ... and check the visibility of entire array. if mutual_position
   // is 1, visual_box will be modified and will contain the visual region
   // of the array
   int mutual_position = clip.clipbox(visual_box);
   // if array is entirely outside the visual window - bail out
   if (0 == mutual_position) return;

   // If we get here - means that the array (or part of it) is visible
   // draw the cell mark ...

   //@FIXME! Edit in place array of cells!
   DBbox obox(structure()->cellOverlap());
   int col_beg, col_end, row_beg, row_end;
   if (obox.visible(rend.topCTM() * rend.ScrCTM(), rend.visualLimit()))
   {
      // a single structure is big enough to be visible
      // now calculate the start/stop values of the visible references in the matrix
      if (-1 == mutual_position) {
         // entire matrix is visible
         col_beg = row_beg = 0;
         col_end = _arrprops.cols();
         row_end = _arrprops.rows();
      }
      else {
         real cstepX = (array_overlap.p2().x() - array_overlap.p1().x()) / _arrprops.cols();
         real cstepY = (array_overlap.p2().y() - array_overlap.p1().y()) / _arrprops.rows();
         // matrix is partially visible
         col_beg = array_overlap.p1().x() < clip.p1().x() ?
               (int) rint((clip.p1().x() - array_overlap.p1().x()) / cstepX) : 0;
         row_beg = array_overlap.p1().y() < clip.p1().y() ?
               (int) rint((clip.p1().y() - array_overlap.p1().y()) / cstepY) : 0;
         col_end = col_beg + (int) rint((visual_box.p2().x() - visual_box.p1().x()) / cstepX);
         row_end = row_beg + (int) rint((visual_box.p2().y() - visual_box.p1().y()) / cstepY);
         // add an extra row/column from both sides to ensure visibility of the`
         // border areas
         col_beg -= (0 == col_beg) ? 0 : 1;
         row_beg -= (0 == row_beg) ? 0 : 1;
         col_end += (_arrprops.cols() == col_end) ? 0 : 1;
         row_end += (_arrprops.rows() == row_end) ? 0 : 1;
      }
   }
   else
   {
      // a single structure is too small
      col_beg = row_beg = col_end = row_end = 0;
   }

   // finally - start drawing
   for (int i = col_beg; i < col_end; i++)
   {// start/stop rows
      for(int j = row_beg; j < row_end; j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
         // ...draw the structure itself
         structure()->openGlRender(rend, refCTM * _translation, false, false);
      }
   }
}

void laydata::TdtCellAref::drawSRequest(tenderer::TopRend& rend, const SGBitSet*) const
{
   //@FIXME! array of cells selected! It will give confusing results in "edit in place"
   drawRequest(rend);
}


void laydata::TdtCellAref::openGlDrawLine(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   drawprop.drawCellBoundary(ptlist);
}

void laydata::TdtCellAref::openGlDrawFill(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   for (int i = ptlist[4].x(); i < ptlist[4].y(); i++)
   {// start/stop rows
      for(int j = ptlist[5].x(); j < ptlist[5].y(); j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
         refCTM *= drawprop.topCtm();
         // ...draw the structure itself, not forgetting to push/pop the refCTM
         drawprop.pushCtm(refCTM);
         structure()->openGlDraw(drawprop);
         drawprop.popCtm();
      }
   }
   // push is done in the precalc()
//   drawprop.popCtm();
}

void laydata::TdtCellAref::openGlDrawSel(const pointlist& ptlist, const SGBitSet*) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
}

void laydata::TdtCellAref::motionDraw(const layprop::DrawProperties& drawprop,
                 ctmqueue& transtack, SGBitSet*) const
{
   assert(structure());
   for (int i = 0; i < _arrprops.cols(); i++)
   {// start/stop rows
      for(int j = 0; j < _arrprops.rows(); j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
         refCTM *= _translation;
         transtack.push_front(refCTM * transtack.front());
         structure()->motionDraw(drawprop, transtack);
      }
   }
}

void laydata::TdtCellAref::info(std::ostringstream& ost, real DBU) const {
   ost << "cell \"" << _structure->name() << "\" - array reference @ {";
   ost << _translation.tx()/DBU << " , " << _translation.ty()/DBU << "} ->";
   ost << " [" << _arrprops.cols() << " x " << _arrprops.stepX() << " , " ;
   ost <<         _arrprops.rows() << " x " << _arrprops.stepY() << "]";
}

void laydata::TdtCellAref::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_CELLAREF);
   tedfile->putString(_structure->name());
   tedfile->putCTM(_translation);
   tedfile->put4b(_arrprops.stepX());
   tedfile->put4b(_arrprops.stepY());
   tedfile->putWord(_arrprops.rows());
   tedfile->putWord(_arrprops.cols());
}

void laydata::TdtCellAref::gdsWrite(DbExportFile& gdsf) const
{
   gdsf.aref(_structure->name(), _translation, _arrprops);
}

void laydata::TdtCellAref::cifWrite(DbExportFile& ciff) const
{
   ciff.aref(_structure->name(), _translation, _arrprops);
}

void laydata::TdtCellAref::psWrite(PSFile& psf, const layprop::DrawProperties& drawprop) const
{
   for (int i = 0; i < _arrprops.cols(); i++)
   {// start/stop rows
      for(int j = 0; j < _arrprops.rows(); j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
         refCTM *= _translation;
         psf.cellref(_structure->name(), refCTM);
         if (!psf.hier())
         {
            _structure->psWrite(psf, drawprop);
         }
      }
   }
}

void  laydata::TdtCellAref::ungroup(laydata::TdtDesign* ATDB, TdtCell* dst, laydata::AtticList* nshp) {
   for (word i = 0; i < _arrprops.cols(); i++)
      for(word j = 0; j < _arrprops.rows(); j++) {
         // for each of the array figures
         CTM refCTM;
         refCTM.Translate(_arrprops.stepX() * i , _arrprops.stepY() * j );
         refCTM *= _translation;
         laydata::TdtCellRef* cellref = DEBUG_NEW TdtCellRef(_structure, refCTM);
         cellref->ungroup(ATDB, dst, nshp);
         delete cellref;
      }
}

DBbox laydata::TdtCellAref::overlap() const
{
   assert(structure());
   DBbox ovl(clearOverlap().overlap(_translation));
   ovl.normalize();
   return ovl;
}

DBbox laydata::TdtCellAref::clearOverlap() const
{
   assert(structure());
   DBbox bx(structure()->cellOverlap());
   DBbox ovl(bx);
   CTM refCTM(1.0,0.0,0.0,1.0,_arrprops.stepX() * (_arrprops.cols()-1), _arrprops.stepY() * (_arrprops.rows() - 1));
   bx = bx * refCTM; bx.normalize();
   ovl.overlap(bx);
   return ovl;
}

//-----------------------------------------------------------------------------
// class TdtText
//-----------------------------------------------------------------------------
laydata::TdtText::TdtText(std::string text, CTM trans) : TdtData(),
                          _text(text), _translation(trans), _overlap(TP())
{
   for (unsigned charnum = 0; charnum < text.length(); charnum++)
      if (!isprint(text[charnum])) text[charnum] = '?';
   assert(NULL != fontLib); // check that font library is initialised
   DBbox pure_ovl(0,0,0,0);
   fontLib->getStringBounds(&_text, &pure_ovl);
   _overlap = DBbox(TP(0,0), TP((pure_ovl.p2().x() - pure_ovl.p1().x()),
                                (pure_ovl.p2().y() - pure_ovl.p1().y())) );
   _correction = TP(-pure_ovl.p1().x(), -pure_ovl.p1().y());
}

laydata::TdtText::TdtText(TEDfile* const tedfile) : TdtData(),
   _text(tedfile->getString()), _translation(tedfile->getCTM()), _overlap(TP())
{
   assert(NULL != fontLib); // check that font library is initialised
   DBbox pure_ovl(0,0,0,0);
   fontLib->getStringBounds(&_text, &pure_ovl);
   _overlap = DBbox(TP(0,0), TP((pure_ovl.p2().x() - pure_ovl.p1().x()),
                    (pure_ovl.p2().y() - pure_ovl.p1().y())) );
   _correction = TP(-pure_ovl.p1().x(), -pure_ovl.p1().y());
}

void laydata::TdtText::replaceStr(std::string newstr)
{
   _text = newstr;
   assert(NULL != fontLib); // check that font library is initialised
   DBbox pure_ovl(0,0,0,0);
   fontLib->getStringBounds(&_text, &pure_ovl);
   _overlap = DBbox(TP(0,0), TP((pure_ovl.p2().x() - pure_ovl.p1().x()),
                    (pure_ovl.p2().y() - pure_ovl.p1().y())) );
   _correction = TP(-pure_ovl.p1().x(), -pure_ovl.p1().y());
}

void laydata::TdtText::openGlPrecalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   //  Things to remember...
   // Font has to be translated using its own matrix in which
   // tx/ty are forced to zero. Below they are not used (and not zeroed)
   // because of the conversion to the openGL matrix.
   // The text binding point is multiplied ALONE with the current
   // translation matrix, but NEVER with the font matrix.
   // All this as far as I remember is described in the PS manual
   // OpenGL seems to have more primitive font handling - no offense
   // IMHO.
   // The other "discovery" for the GLUT font rendering...
   // They are talking in the doc's that stroke fonts can vary from
   // 119.05 units down to 33.33 units. It is not quite clear however
   // how big (in pixels say) is one unit. After a lot of experiments
   // it appears that if you draw a character with font scale = 1, then
   // you will get a font with height 119.05 units. In order to translate
   // the font to DBU's I need to multiply it by DBU and divide it to 119.05
   // This is done in the tellibin - int tellstdfunc::stdADDTEXT::execute()
   // Things to consider ...
   // And the last, but not the least...
   // GDSII text justification
   //====================================================================
   // the correction is needed to fix the bottom left corner of the
   // text overlapping box to the binding point. glf library normally
   // draws the first symbol centered around the bounding point
   CTM correction;
   correction.Translate(-_overlap.p1().x(), -_overlap.p1().y());
   DBbox _over = _overlap.overlap(correction);
   // font translation matrix
   CTM ftmtrx =  _translation * drawprop.topCtm();
   DBbox wsquare(TP(0,0), TP(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT));
   if ( wsquare.visible(ftmtrx * drawprop.scrCtm(), drawprop.visualLimit()) )
   {
      // If we get here - means that the text is visible
      CTM adjTranslation = (drawprop.adjustTextOrientation()) ?
                            renderingAdjustment(ftmtrx) : _translation;
      CTM adj_ftmtrx     = (drawprop.adjustTextOrientation()) ?
                            adjTranslation * drawprop.topCtm() : ftmtrx;
      // get the text overlapping box ...
      ptlist.reserve(5);
      ptlist.push_back(_over.p1() * ftmtrx);
      ptlist.push_back(TP(_over.p2().x(), _over.p1().y()) * ftmtrx);
      ptlist.push_back(_over.p2() * ftmtrx);
      ptlist.push_back(TP(_over.p1().x(), _over.p2().y()) * ftmtrx);
      // ... and text bounding point (see the comment above)
      ptlist.push_back(TP(static_cast<int4b>(adjTranslation.tx()),
                          static_cast<int4b>(adjTranslation.ty()))  * drawprop.topCtm());
      // push the font matrix - will be used for text drawing
      drawprop.pushCtm(adj_ftmtrx);
   }
}

void laydata::TdtText::drawRequest(tenderer::TopRend& rend) const
{
   // font translation matrix
   CTM ftmtrx =  _translation * rend.topCTM();
   DBbox wsquare(TP(0,0), TP(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT));
   if (!wsquare.visible(ftmtrx * rend.ScrCTM(), rend.visualLimit()) ) return;
   // If we get here - means that the text is visible
   // draw the cell mark ...
   //   rend.drawReferenceMarks(TP(0,0) * newtrans, layprop::cell_mark);
   // Calculate the visual adjustment to make the texts easy to read
   if (rend.adjustTextOrientation())
      rend.text(&_text, renderingAdjustment(ftmtrx), _overlap, _correction, false);
   else
      rend.text(&_text, _translation, _overlap, _correction, false);
}

void laydata::TdtText::drawSRequest(tenderer::TopRend& rend, const SGBitSet*) const
{
   // font translation matrix
   CTM ftmtrx =  _translation * rend.topCTM();
   DBbox wsquare(TP(0,0), TP(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT));
   if (!wsquare.visible(ftmtrx * rend.ScrCTM(), rend.visualLimit()) ) return;
   // If we get here - means that the text is visible
   // draw the cell mark ...
   //   rend.drawReferenceMarks(TP(0,0) * newtrans, layprop::cell_mark);
   rend.text(&_text, _translation, _overlap, _correction, true);
}

void laydata::TdtText::openGlDrawLine(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   drawprop.drawTextBoundary(ptlist);
   drawprop.drawReferenceMarks(ptlist[4], layprop::text_mark);
   // draw the text itself
   glPushMatrix();
   double ori_mtrx[] = { drawprop.topCtm().a(), drawprop.topCtm().b(),0,0,
                         drawprop.topCtm().c(), drawprop.topCtm().d(),0,0,
                                             0,                     0,0,0,
                                 ptlist[4].x(),         ptlist[4].y(),0,1};
   glMultMatrixd(ori_mtrx);
   // correction of the glf shift - as explained in the openGlPrecalc above
   glTranslatef(_correction.x(), _correction.y(), 1);
   // The only difference between glut and glf appears to be the size:-
   // glf is not using the font unit, so we need to scale it back up (see below)
   // but... it uses real numbers - that is not what we need. That's why -
   // keeping the font unit will help to convert the font metrics back to
   // integer coordinates
   glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);

   assert(NULL != fontLib); // check that font library is initialised
   fontLib->drawString(&_text, false);
   glPopMatrix();
}

void laydata::TdtText::openGlDrawFill(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   glPushMatrix();
   double ori_mtrx[] = { drawprop.topCtm().a(), drawprop.topCtm().b(),0,0,
                         drawprop.topCtm().c(), drawprop.topCtm().d(),0,0,
                                             0,                     0,0,0,
                                 ptlist[4].x(),         ptlist[4].y(),0,1};
   glMultMatrixd(ori_mtrx);
   // correction of the glf shift - as explained in the openGlPrecalc above
   glTranslatef(_correction.x(), _correction.y(), 1);
   // The only difference between glut and glf appears to be the size:-
   // glf is not using the font unit, so we need to scale it back up (see below)
   // but... it uses real numbers - that is not what we need. That's why -
   // keeping the font unit will help to convert the font metrics back to
   // integer coordinates
   glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
   fontLib->drawString(&_text, true);
   glPopMatrix();
}

void laydata::TdtText::openGlDrawSel(const pointlist& ptlist, const SGBitSet*) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < 4; i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
}

void laydata::TdtText::openGlPostClean(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   ptlist.clear();
   // get the font matrix out of the stack (pushed in precalc)
   drawprop.popCtm();
}

void laydata::TdtText::motionDraw(const layprop::DrawProperties& drawprop,
               ctmqueue& transtack, SGBitSet*) const
{
   //====================================================================
   // font translation matrix
   CTM ftmtrx =  _translation * transtack.front();
   DBbox wsquare(TP(0, 0),TP(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT));
   if (wsquare.visible(ftmtrx * drawprop.scrCtm(), drawprop.visualLimit()))
   {
      if (drawprop.adjustTextOrientation())
         ftmtrx = renderingAdjustment(ftmtrx) * transtack.front();
      glPushMatrix();
      double ori_mtrx[] = { ftmtrx.a(), ftmtrx.b(),0,0,
                            ftmtrx.c(), ftmtrx.d(),0,0,
                                     0,          0,0,0,
                           ftmtrx.tx(),ftmtrx.ty(),0,1};
      glMultMatrixd(ori_mtrx);
      // correction of the glf shift - as explained in the openGlPrecalc above
      glTranslatef(_correction.x(), _correction.y(), 1);
      glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
      fontLib->drawWiredString(_text);
      glPopMatrix();
   }
}

void laydata::TdtText::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_TEXT);
   tedfile->putString(_text);
   tedfile->putCTM(_translation);
}

void laydata::TdtText::gdsWrite(DbExportFile& gdsf) const
{
   gdsf.text(_text, _translation);
}

void laydata::TdtText::cifWrite(DbExportFile& ciff) const
{
   ciff.text(_text, _translation);
}

void laydata::TdtText::psWrite(PSFile& gdsf, const layprop::DrawProperties& drawprop) const
{
   CTM fmtrx(_translation);
   fmtrx.Scale(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT);
   CTM ffmtrx(fmtrx.a(), fmtrx.b(), fmtrx.c(), fmtrx.d(), _translation.tx(), _translation.ty());
   gdsf.text(_text, ffmtrx);
}

DBbox laydata::TdtText::overlap() const
{
   DBbox ovl(_overlap.overlap(_translation));
   ovl.normalize();
   return ovl;
}

void laydata::TdtText::info(std::ostringstream& ost, real DBU) const
{
   ost << "text \"" << _text << "\" @ {";
   ost << _translation.tx()/DBU << " , " << _translation.ty()/DBU << "}";
}

/*!Adjusts the text object during the rendering in such a way that it is
 * always lined-up form left to right or from bottom to top. The function
 * returns the adjusted translation matrix which should be used instead of
 * the original one (_translation) during the drawing.
 * The mats behind this method: The input parameter mtrx contains the final
 * CTM that will be used to draw the text. It is decomposed and if the resulting
 * rotation is in the range of 90 - 270 degrees - the text is rotated on 180
 * degrees around its central point. Simple, but a number of complications:
 * - When the text is fliped it needs to be un-flipped and this must be done
 *   using only local placement parameters. This in turn means that the local
 *   _translation matrix must be decomposed as well in order to extract the
 *   rotation of the text itself. It seems to be the only way to un-flip
 *   properly the text. It must be done before the main operation
 * - The rotation point are very important.
 *
 * The method introduces additional calculations executed during rendering which
 * might slow down the graphics.
 */
CTM laydata::TdtText::renderingAdjustment(const CTM& mtrx) const
{
   TP    trans   ; // dummy, not used here
   real  scale   ; // dummy, not used here
   real  rotation; // the final text rotation (after all CTM operations)
   bool  flipX   ; // the final text flip (after all CTM operations)
   DBbox adjoverlap = _overlap * _translation;
   int4b centerX = (adjoverlap.p1().x() + adjoverlap.p2().x()) / 2;
   int4b centerY = (adjoverlap.p1().y() + adjoverlap.p2().y()) / 2;
   TP    textCenter(centerX, centerY);
   mtrx.Decompose(trans, rotation, scale, flipX);
   CTM adjmatrix(_translation);
   if (flipX)
   {
      real lrotation;
      bool lflipX;
      _translation.Decompose(trans, lrotation, scale, lflipX);
      adjmatrix.Rotate(-lrotation, textCenter);
      adjmatrix.FlipX(centerY);
      adjmatrix.Rotate( lrotation, textCenter);
   }
   if (0 > rotation) rotation += 360.0;
   if ((90.0 < rotation) && (rotation <= 270.0))
   {
      adjmatrix.Rotate(180, textCenter);
   }
   return adjmatrix;
}

//-----------------------------------------------------------------------------
// class ValidBox
//-----------------------------------------------------------------------------
laydata::ValidBox::ValidBox(pointlist& plist) : Validator(plist)
{
   word i,j;
   _area = 0;
   for (i = 0, j = 1; i < 4; i++, j = (j+1) % 4)
      _area += _plist[i].x()*_plist[j].y() - _plist[j].x()*_plist[i].y();
   if (_area < 0)
   {
      std::reverse(_plist.begin(),_plist.end());
      _status |= laydata::shp_clock;
   }
   if ((_area == 0) || (_plist[0] == _plist[1]))
      _status |= laydata::shp_null;
   else if (0 == remainder(xangle(_plist[0], _plist[1]),90.0))
      _status |= laydata::shp_box;
   _area = fabs(_area);
}

laydata::TdtData* laydata::ValidBox::replacement()
{
   if (box())
      return DEBUG_NEW laydata::TdtBox(_plist[0], _plist[2]);
   else return DEBUG_NEW laydata::TdtPoly(_plist);
}

std::string laydata::ValidBox::failType()
{
   if      (!(_status & shp_box))  return "Rotated box";
   else return "OK";
}

//-----------------------------------------------------------------------------
// class ValidPoly
//-----------------------------------------------------------------------------
laydata::ValidPoly::ValidPoly(pointlist& plist) : Validator(plist) {
   angles();
   if (_status > 0x10) return;
   //reorder the chain (if needed) to get the points in anticlockwise order
   normalize();
   if (_status > 0x10) return;
   //check self crossing
   selfcrossing();
}

laydata::TdtData* laydata::ValidPoly::replacement() {
   laydata::TdtData* newshape;
   if (box())
      newshape = DEBUG_NEW laydata::TdtBox(_plist[0], _plist[2]);
   else newshape = DEBUG_NEW laydata::TdtPoly(_plist);
   return newshape;
}

/**
 * Checks the polygon angles. Filters out intermediate points if 0 or 180 deg
   angle is found as well as coinciding points. Function also flags a box if
   found as well as the acute angles
 */
void laydata::ValidPoly::angles()
{
   pointlist::iterator cp1 = _plist.end(); cp1--;
   pointlist::iterator cp2 = _plist.begin();
   real cAngle = 0.0;
   std::stack<real> angle_stack;
   bool prev_cleared = false;
   while ((angle_stack.size() <= _plist.size()) && (2 < _plist.size()))
   {
      bool eraseP1 = false;
      if (*cp1 == *cp2)
      {
         eraseP1 = true;
         if (!prev_cleared)
            angle_stack.push(0);
      }
      else if (!prev_cleared)
      {
         cAngle = xangle(*cp1, *cp2);
         if (!angle_stack.empty())
         {
            real ang = fabs(cAngle - angle_stack.top());
            if ((0 == ang) || (180 == ang))
               eraseP1 = true;
            else if (ang < 90 || ang > 270)
               _status |= laydata::shp_acute;
         }
         angle_stack.push(cAngle);
      }
      if (eraseP1)
      {
         cp2 = _plist.erase(cp1);
         cp1 = cp2;
         if (cp2 == _plist.begin()) cp1 = _plist.end();
         cp1--;
         angle_stack.pop();
         _status |= laydata::shp_ident;
         prev_cleared = true;
      }
      else
      {
         cp1 = cp2; cp2++;
         prev_cleared = false;
      }
      if (_plist.end() == cp2) cp2 = _plist.begin();
      if (_plist.end() == cp1) cp1--;
   }
   // check for a single segment
   if (_plist.size() < 3)
      _status |= shp_null;
   // finally check whether it's a box - if any segment of 4 vertex polygon
   // (already checked for acute angles) has an angle towards x axis
   // that is multiply on 90 - then it should be a box
   else if ((4 == _plist.size()) && (!(_status & shp_acute)) &&
        (0 == remainder(cAngle,90.0)))
      _status |= laydata::shp_box;
}

void laydata::ValidPoly::normalize()
{
   int8b area = polyarea(_plist);
   if (area == 0ll)
   {
      _status |= shp_null; return;
   }
   else if (area < 0ll)
   {
      std::reverse(_plist.begin(),_plist.end());
      _status |= laydata::shp_clock;
   }
}

/*! Implements  algorithm to check that the polygon is not
self crossing. Alters the laydata::shp_cross bit of _status if the polygon
is selfcrossing
*/
void laydata::ValidPoly::selfcrossing()
{
   //using BO modified
   logicop::CrossFix fixingpoly(_plist,true);
   try
   {
      fixingpoly.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {_status |= laydata::shp_cross; return;}
   if (0 != fixingpoly.crossp() )
      _status |= laydata::shp_cross;

/* @TODO Clean-up the old self-cross check algo as soon as the resize
   works properly
   Old algo
   tedop::segmentlist segs(_plist, false);
   tedop::EventQueue Eq(segs); // initialize the event queue
   tedop::SweepLine  SL(_plist); // initialize the sweep line
   if (!Eq.check_valid(SL))
      _status |= laydata::shp_cross;*/
}

std::string laydata::ValidPoly::failType() {
   if      (_status & shp_null) return "NULL area polygon";
   else if (_status & shp_cross) return "Self-crossing";
//   else if (_status & shp_acute) return "Acute angle";
   else return "OK";
}

//-----------------------------------------------------------------------------
// class ValidWire
//-----------------------------------------------------------------------------
laydata::ValidWire::ValidWire(pointlist& plist, word width) :
                                     Validator(plist), _width(width) {
   angles();
   if (_status > 0x10) return;
   if (numpoints() > 3)
      selfcrossing();
}

/*! Checks the wire angles. Filters out intermediate and coinciding points.
Flags acute angles and null wires
 */
void laydata::ValidWire::angles()
{
   // check for a single point
   if (_plist.size() < 2) _status |= shp_null;
   pointlist::iterator cp1 = _plist.end(); cp1--;
   pointlist::iterator cp2 = _plist.begin();
   real cAngle = 0.0;
   std::stack<real> angle_stack;
   bool prev_cleared = false;
   while ((angle_stack.size() <= _plist.size()) && (2 < _plist.size()))
   {
      bool eraseP1 = false;
      if (*cp1 == *cp2)
      {
         eraseP1 = true;
         if (!prev_cleared)
            angle_stack.push(0);
      }
      else if (!prev_cleared)
      {
         cAngle = xangle(*cp1, *cp2);
         if (!angle_stack.empty())
         {
            real ang = fabs(cAngle - angle_stack.top());
            if ((0 == ang) || (180 == ang))
               eraseP1 = true;
            else if (ang < 90 || ang > 270)
               _status |= laydata::shp_acute;
         }
         angle_stack.push(cAngle);
      }
      if (eraseP1)
      {
         cp2 = _plist.erase(cp1);
         cp1 = cp2;
         if (cp2 == _plist.begin()) cp1 = _plist.end();
         cp1--;
         angle_stack.pop();
         _status |= laydata::shp_ident;
         prev_cleared = true;
      }
      else
      {
         cp1 = cp2; cp2++;
         prev_cleared = false;
      }
      if (_plist.end() == cp2) cp2 = _plist.begin();
      if (_plist.end() == cp1) cp1--;
   }
//   pointlist::iterator cp2 = _plist.begin();
//   pointlist::iterator cp1 = cp2; cp2++;
//   real pAngle = 0.0;
//   real cAngle = 0.0;
//   bool pAngleValid = false;
//   if (_plist.size()>2)
//   do
//   {
//      bool eraseP1 = false;
//      if (*cp1 == *cp2)
//         eraseP1 = true;
//      else
//      {
//         cAngle = xangle(*cp1, *cp2);
//         if (pAngleValid)
//         {
//            real ang = fabs(cAngle - pAngle);
//            if ((0 == ang) || (180 == ang))
//               eraseP1 = true;
//            else if (ang < 90 || ang > 270)
//               _status |= laydata::shp_acute;
//         }
//         pAngleValid = true;
//         pAngle = cAngle;
//      }
//      if (eraseP1)
//      {
//         cp2 = _plist.erase(cp1);
//         cp1 = cp2; cp2++;
//         _status |= laydata::shp_ident;
//      }
//      else
//      {
//         cp1 = cp2; cp2++;
//      }
//   }
//   while(cp2 != _plist.end());

   if (((_plist.size() == 2) && (*(_plist.begin()) == *(_plist.end()-1))))
      _status |= shp_null;
}


/*! Implements  algorithm to check that the wire is not simple crossing.
Alters the laydata::shp_cross bit of _status if the wire is selfcrossing
*/
void laydata::ValidWire::selfcrossing() {

   //using BO modified
   logicop::CrossFix fixingpoly(_plist, false);
   try
   {
      fixingpoly.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {_status |= laydata::shp_cross; return;}
   if (0 != fixingpoly.crossp() )
      _status |= laydata::shp_cross;

/*   tedop::segmentlist segs(_plist, true);
   tedop::EventQueue Eq(segs); // initialize the event queue
   tedop::SweepLine  SL(_plist); // initialize the sweep line
   if (!Eq.check_valid(SL))
      _status |= laydata::shp_cross;*/
}

laydata::TdtData* laydata::ValidWire::replacement() {
   return DEBUG_NEW laydata::TdtWire(_plist, _width);
}

std::string laydata::ValidWire::failType() {
//   if (_status & shp_null)  return "Zero area";
   if      (_status & shp_cross) return "Self-crossing";
   else if (_status & shp_null ) return "NULL area object";
   else return "OK";
}

//-----------------------------------------------------------------------------
/*! Returns the angle between the line and the X axis
*/
int laydata::xangle(const TP& p1, const TP& p2) {
//   printf("-- Calculating angle between X axis and (%i, %i) - (%i, %i) line\n",
//          p1.x(), p1.y(), p2.x(), p2.y());
   const long double Pi = 3.1415926535897932384626433832795;
   if (p1.x() == p2.x()) { //vertcal line
      assert(p1.y() != p2.y()); // make sure both points do not coinside
      if   (p2.y() > p1.y()) return  90;
      else                   return -90;
   }
   else if (p1.y() == p2.y()) { // horizontal line
      if (p2.x() > p1.x()) return 0;
      else                 return 180;
   }
   else
      return (int)rint(180*atan2(double(p2.y() - p1.y()), p2.x() - p1.x())/Pi);
}

//-----------------------------------------------------------------------------
// other...
//-----------------------------------------------------------------------------
laydata::TdtData* laydata::createValidShape(pointlist* pl) {
   laydata::ValidPoly check(*pl);
   delete pl;
   if (!check.valid()) {
      std::ostringstream ost;
      ost << "Resulting shape is invalid - " << check.failType();
      tell_log(console::MT_ERROR, ost.str());
      return NULL;
   }
   laydata::TdtData* newshape;
   pointlist npl = check.getValidated();
   if (check.box())
      newshape = DEBUG_NEW laydata::TdtBox(npl[2], npl[0]);
   else
      newshape = DEBUG_NEW laydata::TdtPoly(npl);
   npl.clear();
   return newshape;
}

laydata::TdtData* laydata::polymerge(const pointlist& _plist0, const pointlist& _plist1)
{
   if(_plist0.empty() || _plist1.empty()) return NULL;
   logicop::logic operation(_plist0, _plist1);
   try
   {
      operation.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {return NULL;}
   pcollection merge_shape;
   laydata::TdtData* resShape = NULL;
   if (operation.OR(merge_shape))
   {
      assert(1 == merge_shape.size());
      resShape = createValidShape(*merge_shape.begin());
   }
   return resShape;
}

//void laydata::draw_select_mark(const TP& pnt) {
//   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//   glRasterPos2i(pnt.x(), pnt.y());
//   glBitmap(15,15,7,7,0,0, select_mark);
//}

// void laydata::draw_select_marks(const DBbox& areal, const CTM& trans) {
//    TP ptlist[4];
//    ptlist[0] = areal.p1() * trans;
//    ptlist[1] = TP(areal.p2().x(), areal.p1().y()) * trans;
//    ptlist[2] = areal.p2() * trans;
//    ptlist[3] = TP(areal.p1().x(), areal.p2().y()) * trans;
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    for (byte i = 0; i < 4; i++)
//    {
//       glRasterPos2i(ptlist[i].x(), ptlist[i].y());
//       glBitmap(15,15,7,7,0,0, select_mark);
//    }
// }

//==============================================================================
//
void laydata::TdtTmpBox::draw(const layprop::DrawProperties&, ctmqueue& transtack) const
{
   CTM trans = transtack.front();
   if (!(_p1)) return;
   TP pt2 = (*_p1) * trans;
   glRecti(_p1->x(),_p1->y(),pt2.x(),pt2.y());
}

void  laydata::TdtTmpBox::addpoint(TP p)
{
   if (!_p1) _p1 = DEBUG_NEW TP(p);
   else
   {
      if (_p2) delete _p2; // This line seems to be redundant
      _p2 = DEBUG_NEW TP(p);
   }
}

void  laydata::TdtTmpBox::rmpoint(TP& lp)
{
   if (NULL != _p2) {delete _p2; _p2 = NULL;}
   if (NULL != _p1) {delete _p1; _p1 = NULL;}
};

laydata::TdtTmpBox::~TdtTmpBox()
{
   if (NULL != _p1) delete _p1;
   if (NULL != _p2) delete _p2;
}

//==============================================================================
//
void laydata::TdtTmpPoly::draw(const layprop::DrawProperties&, ctmqueue& transtack) const
{
   CTM trans = transtack.front();
   dword numpnts;
   if ((numpnts = _plist.size()) == 0) return;
   word i;
   glBegin(GL_LINE_STRIP);
   for (i = 0; i < numpnts; i++)
      glVertex2i(_plist[i].x(), _plist[i].y());
   TP newp = _plist[numpnts-1] * trans;
   glVertex2i(newp.x(), newp.y());
   if ((numpnts > 2) || ((2 == numpnts) && (newp != _plist[numpnts-1])))
      glVertex2i(_plist[0].x(), _plist[0].y());
   glEnd();
}

void laydata::TdtTmpPoly::rmpoint(TP& lp)
{
   assert(_plist.size() > 0);
   _plist.pop_back();
   if (_plist.size() > 0) lp = _plist.back();
};

//==============================================================================
//
void laydata::TdtTmpWire::draw(const layprop::DrawProperties& drawprop, ctmqueue& transtack) const
{
   dword num_points = _plist.size();
   if (num_points == 0) return;
   CTM trans = transtack.front();
   pointlist centerLine;
   pointlist contourLine;
   bool ignoreCurrentPoint = (0 == trans.tx()) && (0 == trans.ty());
   if ( ignoreCurrentPoint )
      centerLine.reserve(num_points);
   else
      centerLine.reserve(num_points+1);
   for (unsigned i = 0; i < num_points; i++)
      centerLine.push_back(_plist[i]);
   if (!ignoreCurrentPoint)
      centerLine.push_back(_plist[num_points-1] * trans);

   precalc(centerLine, contourLine);

   drawline(centerLine, contourLine);
   centerLine.clear();
}

void  laydata::TdtTmpWire::rmpoint(TP& lp)
{
   assert(_plist.size() > 0);
   _plist.pop_back();
   if (_plist.size() > 0) lp = _plist.back();
};

void laydata::TdtTmpWire::precalc(const pointlist& centerLine, pointlist& contourLine) const
{
   std::list<TP> tmpPlist;
   int num_points = centerLine.size();
   DBbox* ln1 = endPnts(centerLine[0],centerLine[1], true);
   if (NULL != ln1)
   {
      tmpPlist.push_back(ln1->p1());
      tmpPlist.push_front(ln1->p2());
   }
   delete ln1;
   for (int i = 1; i < num_points - 1; i++)
   {
      ln1 = mdlPnts(centerLine[i-1],centerLine[i],centerLine[i+1]);
      if (NULL != ln1)
      {
         tmpPlist.push_back(ln1->p1());
         tmpPlist.push_front(ln1->p2());
      }
      delete ln1;
   }
   ln1 = endPnts(centerLine[num_points-2],centerLine[num_points-1],false);
   if (NULL != ln1)
   {
      tmpPlist.push_back(ln1->p1());
      tmpPlist.push_front(ln1->p2());
   }
   delete ln1;
   word contourSize = tmpPlist.size();
   if (0 == contourSize) return;
   contourLine.reserve(contourSize);
   for (std::list<TP>::const_iterator CP = tmpPlist.begin(); CP != tmpPlist.end(); CP++)
      contourLine.push_back(*CP);
}

DBbox* laydata::TdtTmpWire::endPnts(const TP& p1, const TP& p2, bool first) const
{
   double     w = _width/2;
   double denom = first ? (p2.x() - p1.x()) : (p1.x() - p2.x());
   double   nom = first ? (p2.y() - p1.y()) : (p1.y() - p2.y());
   double xcorr, ycorr; // the corrections
   if ((0 == nom) && (0 == denom)) return NULL;
   double signX = (  nom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   double signY = (denom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   if      (0 == denom)   {xcorr =signX * w ; ycorr = 0;} // vertical
   else if (0 == nom  )   {xcorr = 0 ; ycorr = signY * w;} // horizontal |----|
   else
   {
      double sl   = nom / denom;
      double sqsl = signY*sqrt( sl*sl + 1);
      xcorr = rint(w * (sl / sqsl));
      ycorr = rint(w * ( 1 / sqsl));
   }
   TP pt = first ? p1 : p2;
   return DEBUG_NEW DBbox((int4b) rint(pt.x() - xcorr), (int4b) rint(pt.y() + ycorr),
                           (int4b) rint(pt.x() + xcorr), (int4b) rint(pt.y() - ycorr));
}

DBbox* laydata::TdtTmpWire::mdlPnts(const TP& p1, const TP& p2, const TP& p3) const
{
   double    w = _width/2;
   double  x32 = p3.x() - p2.x();
   double  x21 = p2.x() - p1.x();
   double  y32 = p3.y() - p2.y();
   double  y21 = p2.y() - p1.y();
   double   L1 = sqrt(x21*x21 + y21*y21); //the length of segment 1
   double   L2 = sqrt(x32*x32 + y32*y32); //the length of segment 2
   double denom = x32 * y21 - x21 * y32;
   if ((0 == denom) || (0 == L1) || (0 == L2)) return NULL;
   // the corrections
   double xcorr = w * ((x32 * L1 - x21 * L2) / denom);
   double ycorr = w * ((y21 * L2 - y32 * L1) / denom);
   return DEBUG_NEW DBbox((int4b) rint(p2.x() - xcorr), (int4b) rint(p2.y() + ycorr),
                           (int4b) rint(p2.x() + xcorr), (int4b) rint(p2.y() - ycorr));
}

void laydata::TdtTmpWire::drawline(const pointlist& centerLine, const pointlist& contourLine) const
{
   int num_lpoints = centerLine.size();
   if (0 == num_lpoints) return;
   // to keep MS VC++ happy - define the counter outside the loops
   int i;
   glBegin(GL_LINE_STRIP);
   for (i = 0; i < num_lpoints; i++)
      glVertex2i(centerLine[i].x(), centerLine[i].y());
   glEnd();
   // now check whether to draw only the center line
   int num_cpoints = contourLine.size();
   if (0 == num_cpoints) return;
   // draw the wire contour
   glBegin(GL_LINE_LOOP);
   for (i = 0; i < num_cpoints; i ++)
      glVertex2i(contourLine[i].x(), contourLine[i].y());
   glEnd();
}

//==============================================================================
//
void laydata::TdtTmpCellRef::draw(const layprop::DrawProperties& drawprop, ctmqueue& transtack) const
{
   if (NULL != _structure)
   {
      transtack.push_front(_translation * transtack.front());
      _structure->motionDraw(drawprop, transtack);
   }
}

//==============================================================================
//
void laydata::TdtTmpCellAref::draw(const layprop::DrawProperties& drawprop,
                                       ctmqueue& transtack) const
{
   if (NULL != _structure)
   {
      for (int i = 0; i < _arrprops.cols(); i++)
      {// start/stop rows
         for(int j = 0; j < _arrprops.rows(); j++)
         { // start/stop columns
            // for each of the visual array figures...
            // ... get the translation matrix ...
            CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
            refCTM *= _translation;
            transtack.push_front(refCTM * transtack.front());
            _structure->motionDraw(drawprop, transtack);
         }
      }
   }
}

//==============================================================================
//
laydata::TdtTmpText::TdtTmpText(std::string text, CTM trans) : _text(text),
                                _translation(trans), _overlap(TP())
{
   for (unsigned charnum = 0; charnum < text.length(); charnum++)
      if (!isprint(text[charnum])) text[charnum] = '?';
   assert(NULL != fontLib); // check that font library is initialised
   fontLib->getStringBounds(&_text, &_overlap);
}


void laydata::TdtTmpText::draw(const layprop::DrawProperties&, ctmqueue& transtack) const
{
   //====================================================================
   // font translation matrix
   CTM ftmtrx =  _translation * transtack.front();
   glPushMatrix();
   double ori_mtrx[] = { ftmtrx.a(), ftmtrx.b(),0,0,
   ftmtrx.c(), ftmtrx.d(),0,0,
            0,          0,0,0,
            ftmtrx.tx(),ftmtrx.ty(),0,1};
            glMultMatrixd(ori_mtrx);
            // correction of the glf shift - as explained in the openGlPrecalc above
            glTranslatef(-_overlap.p1().x(), -_overlap.p1().y(), 1);
            glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
            fontLib->drawWiredString(_text);
            glPopMatrix();
}
