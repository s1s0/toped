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
#include "../tpd_common/outbox.h"
#include "../tpd_common/glf.h"

//GLubyte select_mark[30] = {0x00, 0x00, 0x00, 0x00, 0x3F, 0xF8, 0x3F, 0xF8, 0x30, 0x18,
//                           0x30, 0x18, 0x30, 0x18, 0x30, 0x18, 0x30, 0x18, 0x30, 0x18, 
//                           0x30, 0x18, 0x3F, 0xF8, 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00};

                           
//-----------------------------------------------------------------------------
// Initialize the GLU tessellator static member
//-----------------------------------------------------------------------------
GLUtriangulatorObj *laydata::tdtdata::tessellObj = gluNewTess();

/*===========================================================================
      Select and subsequent operations over the existing tdtdata
=============================================================================
It appears this is not so strightforward as it seems at first sight. There
are several important points to consider here.
   - On first place of course fitting this operatoins within the existing
     data base structure and esspecially make quadTree trasparent to them.
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
     are placed in a quadTree. The objects of this class however "by nature"
     are like moving sands. They might change dramatically potentially with 
     every little change in the data base. This effectively means that the
     pointer to the shape placeholder is not that reliable as one would wish to.
     It is OK as long as data base hasn't changed after the select operation.
   - By definition TELL should be able to operate with TOPED shapes (or list of 
     them). Select operation should be able to return to TELL a reliable list
     of TOPED shapes, that can be used in any moment. A "small detail" here is 
     the modify or info operation on a TOPED shape. We certainly need to know the 
     layer, shape is placed on, and maybe the cell. TELL doesn't know (and doesn't 
     want to know) about the TOPED database structure, quadTree's etc. shit.
   - Undo list - this is not started yet, but seems obvoius that in order to 
     execute undelete for example quadTree pointers will not be very helpfull, 
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
   - Pointer to a quadTree as a parent object to the selected shape seems to 
     be inevitable despite the unstable nature of teh quadTree - otherwise every
     subsequent operation on the selected objects will require searching of the
     marked components one by one troughout entire cell or at least layer. This 
     might take more time than the select itself, which doesn't seems to be very 
     wise.
   - For undo purposes we will need separate list type for selected components 
     that will contain layer number instead of quadTree.
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
     another there is still a way: opencell<source>->select->group<new_cell>
     ->opencell<destination>->addref<new_cell>
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
   - The only placeholder type is quadTree. This means that ref/arefs should be
     placed in a quadTree instead of a C++ standard container  
   - No tdtdata is deleted during the session. Delete operation will move
     the shape in a different placeholder (Attic), or alternatively will mark it
     as deleted. This is to make undo operations possible. It seems appropriate,
     every cell to have an attic where the rubbish will be stored.
   - Select list contains a pointer to the lowest quadTree placeholder that
     contains seleted shapes.
   - tdtdata contains a field that will indicate the status of the shape - 
     selected or not. The same field might be used to indicate the shape is 
     deleted or any other status. This adds a byte to the every single component.
     It seems however that it will pay-back when drawing a large amount of 
     selected data. As usual - speed first!
   - The select list is invalidated with every cell change. The new shape might
     be selected.
   -----------------------------------------------------------------------------
   Now when already the only placeholder is quadTree ...
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

                           tdtcell
tdtlayer tdtlayer etc.
                      
quadTree                   quadTree
                           
shapes                     tdtcellref/tdtaref

*/

//-----------------------------------------------------------------------------
// class tdtdata
//-----------------------------------------------------------------------------
GLvoid laydata::tdtdata::polyVertex (GLvoid *point) {
   TP *pnt = (TP *) point;
   glVertex2i(pnt->x(), pnt->y());
}

bool laydata::tdtdata::point_inside(const TP pnt) {
   DBbox ovl = overlap();ovl.normalize();
   if (ovl.inside(pnt)) return true;
   else return false;
}

void laydata::tdtdata::select_this(dataList* selist)
{
   if (sh_partsel == _status)
   {
      // if the shape has already been partially selected
      for (dataList::iterator SI = selist->begin(); SI != selist->end(); SI++)
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
      selist->push_back(selectDataPair(this,SGBitSet()));
   _status = sh_selected; 
}

void laydata::tdtdata::select_inBox(DBbox& select_in, dataList* selist, bool pselect)
{
   // if shape is already fully selected, nothing to do here
   if (sh_selected == _status) return;
   float clip;
   // get the clip area and if it is 0 - run away
   if (0 == (clip = select_in.cliparea(overlap()))) return;
   if (-1 == clip) select_this(selist); // entire shape is in
   else if ((clip > 0) && pselect)
   { // shape partially is in the select box
      SGBitSet pntlst;
      if (sh_partsel == _status)
      {
      // if the shape has already been partially selected
         dataList::iterator SI;
         for (SI = selist->begin(); SI != selist->end(); SI++)
            // get the pointlist
            if (SI->first == this) { pntlst = SI->second; break; }
         assert(0 != pntlst.size());
         // select some more points using shape specific procedures
         select_points(select_in, pntlst);
         // check that after the selection shape doesn't end up fully selected
         if (pntlst.isallset()) {
            _status = sh_selected;
            pntlst.clear();
//            delete pntlst; SI->second = NULL;
         }
      }
      else
      {
         // otherwise create a new pointlist
         pntlst = SGBitSet(numpoints());
         // select some more points using shape specific procedures
         select_points(select_in, pntlst);
         // and check 
         if (!pntlst.isallclear()) {
            _status = sh_partsel;
            selist->push_back(selectDataPair(this, SGBitSet(pntlst)));
         }
//         else delete pntlst;
      }
   }
}

bool  laydata::tdtdata::unselect(DBbox& select_in, selectDataPair& SI, bool pselect) {
   // check the integrity of the select list
   assert((sh_selected == _status) || (sh_partsel == _status));
   float clip;
   // get the clip area and if it is 0 - run away
   if (0 == (clip = select_in.cliparea(overlap()))) return false;
   // if select_in overlaps the entire shape
   if (-1 == clip) {
      if (0 != SI.second.size()) {
         //remove the list of selected points if it exists ...
         //delete (SI.second); SI.second = NULL;
         SI.second.clear();
      }
      _status = sh_active;
      return true;// i.e. remove this from the list of selected shapes
   }   
   // if select_in intersects with the overlapping box
   else if ((clip > 0) && pselect) {
      if (sh_partsel != _status) // if the shape is already partially selected
         SI.second = SGBitSet(numpoints());
      // get an alias of the SI.second
      SGBitSet &pntlst = SI.second;
      // finally - go and unselect some points   
      unselect_points(select_in, pntlst);
      if (pntlst.isallclear()) {//none selected
         _status = sh_active; 
         pntlst.clear();
         //delete pntlst;SI.second = NULL; 
         return true;
      }
      else if (pntlst.isallset()) { //all points selected; 
         _status = sh_selected; 
         pntlst.clear();
         //delete pntlst; SI.second = NULL; 
         return false;
      }   
      else { // some selected
         _status = sh_partsel; return false;
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
   // once in openGL_precalc() and then reused in the rest of the openGL_*
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
// class tdtbox
//-----------------------------------------------------------------------------
laydata::tdtbox::tdtbox(TEDfile* const tedfile) : tdtdata() 
{
   _p1 = DEBUG_NEW TP(tedfile->getTP());
   _p2 = DEBUG_NEW TP(tedfile->getTP());
   SGBitSet dummy;
   normalize(dummy);
}

void laydata::tdtbox::normalize(SGBitSet& psel)
{
   int4b swap;
   if (_p1->x() > _p2->x())
   {
      swap = _p1->x(); _p1->setX(_p2->x()); _p2->setX(swap);
      if (0 != psel.size())
      {
         psel.swap(0,1);
         psel.swap(2,3);
      }
   }
   if (_p1->y() > _p2->y()) {
      swap = _p1->y(); _p1->setY(_p2->y()); _p2->setY(swap);
      if (0 != psel.size())
      {
         psel.swap(0,3);
         psel.swap(1,2);
      }
   }
}   

void laydata::tdtbox::openGL_precalc(layprop::DrawProperties& drawprop , pointlist& ptlist) const
{
   // translate the points using the current CTM
   ptlist.reserve(4);
   ptlist.push_back(                (*_p1) * drawprop.topCTM());
   ptlist.push_back(TP(_p2->x(), _p1->y()) * drawprop.topCTM());
   ptlist.push_back(                (*_p2) * drawprop.topCTM());
   ptlist.push_back(TP(_p1->x(), _p2->y()) * drawprop.topCTM());
}

void laydata::tdtbox::openGL_drawline(layprop::DrawProperties&, const pointlist& ptlist) const
{
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < 4; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::tdtbox::openGL_drawfill(layprop::DrawProperties&, const pointlist& ptlist) const
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

void laydata::tdtbox::openGL_drawsel(const pointlist& ptlist, const SGBitSet* pslist) const
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

void laydata::tdtbox::tmp_draw(const layprop::DrawProperties&, ctmqueue& transtack,
                              SGBitSet* plst, bool under_construct) const
{
   CTM trans = transtack.front();
   if (!(_p1)) return;
   else if (under_construct)
   {
      TP pt2 = (*_p1) * trans;
      glRecti(_p1->x(),_p1->y(),pt2.x(),pt2.y());
   }
   else if (_p2){
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
         ptlist.push_back(                (*_p1) * trans);
         ptlist.push_back(TP(_p2->x(), _p1->y()) * trans);
         ptlist.push_back(                (*_p2) * trans);
         ptlist.push_back(TP(_p1->x(), _p2->y()) * trans);
         glBegin(GL_LINE_LOOP);
         for (unsigned i = 0; i < 4; i++)
            glVertex2i(ptlist[i].x(), ptlist[i].y());
         glEnd();
         ptlist.clear();
      }
   }
}

void  laydata::tdtbox::select_points(DBbox& select_in, SGBitSet& pntlst) {
   if (select_in.inside(*_p1))  pntlst.set(0);
   if (select_in.inside(TP(_p2->x(), _p1->y())))  pntlst.set(1);
   if (select_in.inside(*_p2))  pntlst.set(2);
   if (select_in.inside(TP(_p1->x(), _p2->y())))  pntlst.set(3);
   pntlst.check_neighbours_set(false);   
}

void  laydata::tdtbox::unselect_points(DBbox& select_in, SGBitSet& pntlst) {
   if (sh_selected == _status) pntlst.setall();
   if (select_in.inside(*_p1))  pntlst.reset(0);
   if (select_in.inside(TP(_p2->x(), _p1->y())))  pntlst.reset(1);
   if (select_in.inside(*_p2))  pntlst.reset(2);
   if (select_in.inside(TP(_p1->x(), _p2->y())))  pntlst.reset(3);
}

laydata::validator* laydata::tdtbox::move(const CTM& trans, SGBitSet& plst)
{
   if (0 != plst.size())
   {// used for modify
      pointlist* nshape = movePointsSelected(plst, trans);
      *_p1 = (*nshape)[0]; *_p2 = (*nshape)[2];
      normalize(plst);
      nshape->clear(); delete nshape;
      return NULL;
   }
   else
   {// used for rotate
      laydata::valid_box* check = DEBUG_NEW valid_box(*_p1, *_p2 ,trans);
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

void laydata::tdtbox::transfer(const CTM& trans) {
   *_p1 *= trans;
   *_p2 *= trans;
   SGBitSet dummy;
   normalize(dummy);
}

laydata::tdtdata* laydata::tdtbox::copy(const CTM& trans) {
   TP *cp1 = DEBUG_NEW TP(*_p1 * trans);
   TP *cp2 = DEBUG_NEW TP(*_p2 * trans);
   return DEBUG_NEW tdtbox(cp1, cp2);
}

void laydata::tdtbox::info(std::ostringstream& ost, real DBU) const {
   ost << "box - {";
   _p1->info(ost, DBU);
   ost << " , ";
   _p2->info(ost, DBU);
   ost << "};";
}

void laydata::tdtbox::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_BOX);
   tedfile->putTP(_p1);
   tedfile->putTP(_p2);
}

void laydata::tdtbox::GDSwrite(GDSin::GDSFile& gdsf, word lay, real) const
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_BOUNDARY);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_LAYER);
   wr->add_int2b(lay);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_DATATYPE);
   wr->add_int2b(0);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_XY,5);
   wr->add_int4b(_p1->x());wr->add_int4b(_p1->y());
   wr->add_int4b(_p1->x());wr->add_int4b(_p2->y());
   wr->add_int4b(_p2->x());wr->add_int4b(_p2->y());
   wr->add_int4b(_p2->x());wr->add_int4b(_p1->y());
   wr->add_int4b(_p1->x());wr->add_int4b(_p1->y());
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ENDEL);
   gdsf.flush(wr);
}

void laydata::tdtbox::PSwrite(PSFile& gdsf, const layprop::DrawProperties&) const
{
   const pointlist box4 = shape2poly();
   gdsf.poly(box4, overlap());
}

DBbox laydata::tdtbox::overlap() const {
   DBbox ovl = DBbox(*_p1);
   ovl.overlap(*_p2);
   return ovl;
}

void  laydata::tdtbox::addpoint(TP p) {
   if (!_p1) _p1 = DEBUG_NEW TP(p);
   else {
      if (_p2) delete _p2; // This line seems to be redundant
      _p2 = DEBUG_NEW TP(p);
   }
}

void  laydata::tdtbox::rmpoint(TP& lp) {
   if (NULL != _p2) {delete _p2; _p2 = NULL;}
   if (NULL != _p1) {delete _p1; _p1 = NULL;}
};

const pointlist laydata::tdtbox::shape2poly() const {
  // convert box to polygon
   pointlist _plist;
   _plist.push_back(TP(*_p1));_plist.push_back(TP(_p2->x(), _p1->y()));
   _plist.push_back(TP(*_p2));_plist.push_back(TP(_p1->x(), _p2->y()));
   return _plist;
};

void laydata::tdtbox::polycut(pointlist& cutter, shapeList** decure)
{
   pointlist _plist = shape2poly();
   // and proceed in the same way as for the polygon
   logicop::logic operation(_plist, cutter);
   try
   {
      operation.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {return;}
   logicop::pcollection cut_shapes;
   laydata::tdtdata* newshape;
   if (operation.AND(cut_shapes))
   {
      logicop::pcollection::const_iterator CI;
      // add the resulting cut_shapes to the_cut shapeList
      for (CI = cut_shapes.begin(); CI != cut_shapes.end(); CI++) 
         if (NULL != (newshape = createValidShape(*CI)))
            decure[1]->push_back(newshape);
      cut_shapes.clear();
      // if there is a cut - there will be (most likely) be cut remains as well
      operation.reset_visited();
      logicop::pcollection rest_shapes;
      if (operation.ANDNOT(rest_shapes))
         // add the resulting cut remainings to the_rest shapeList
         for (CI = rest_shapes.begin(); CI != rest_shapes.end(); CI++) 
            if (NULL != (newshape = createValidShape(*CI)))
               decure[2]->push_back(newshape);
      rest_shapes.clear();
      // and finally add this to the_delete shapelist
      decure[0]->push_back(this);
   }
}

void laydata::tdtbox::stretch(int bfactor, shapeList** decure)
{
   if ( ((_p1->x() - _p2->x()) < 2*bfactor) &&
        ((_p1->y() - _p2->y()) < 2*bfactor)    )
   {
      tdtbox* modified = DEBUG_NEW tdtbox(
                                          DEBUG_NEW TP(_p1->x() - bfactor,
                                                      _p1->y() - bfactor ),
                                          DEBUG_NEW TP(_p2->x() + bfactor,
                                                      _p2->y() + bfactor )
                                       );
      decure[1]->push_back(modified);
   }
   decure[0]->push_back(this);
}

pointlist* laydata::tdtbox::movePointsSelected(const SGBitSet& pset, 
                                    const CTM&  movedM, const CTM& stableM) const {
  // convert box to polygon
   pointlist* mlist = DEBUG_NEW pointlist();
   mlist->push_back(TP(*_p1));mlist->push_back(TP(_p2->x(), _p1->y()));
   mlist->push_back(TP(*_p2));mlist->push_back(TP(_p1->x(), _p2->y()));

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

laydata::tdtbox::~tdtbox() {
   if (_p1) delete _p1; 
   if (_p2) delete _p2;
}

//-----------------------------------------------------------------------------
// class tdtpoly
//-----------------------------------------------------------------------------
laydata::tdtpoly::tdtpoly(TEDfile* const tedfile) : tdtdata()
{
   word numpoints = tedfile->getWord();
   _plist.reserve(numpoints);
   for (word i = 0 ; i < numpoints; i++) 
      _plist.push_back(tedfile->getTP());
}

void laydata::tdtpoly::openGL_precalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // translate the points using the current CTM
   ptlist.reserve(_plist.size());
   for (unsigned i = 0; i < _plist.size(); i++)
      ptlist.push_back(_plist[i] * drawprop.topCTM());
}

void laydata::tdtpoly::openGL_drawline(layprop::DrawProperties&, const pointlist& ptlist) const
{
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < ptlist.size(); i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void laydata::tdtpoly::openGL_drawfill(layprop::DrawProperties&, const pointlist& ptlist) const
{
   // Start tessellation
   gluTessBeginPolygon(tessellObj, NULL);
   GLdouble pv[3];
   pv[2] = 0;
   for (unsigned i = 0; i < ptlist.size(); i++) {
      pv[0] = ptlist[i].x(); pv[1] = ptlist[i].y();
      gluTessVertex(tessellObj,pv,const_cast<TP*>(&ptlist[i]));
   }
   gluTessEndPolygon(tessellObj);
}

void laydata::tdtpoly::openGL_drawsel(const pointlist& ptlist, const SGBitSet* pslist) const
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

void laydata::tdtpoly::tmp_draw(const layprop::DrawProperties&, ctmqueue& transtack,
                                 SGBitSet* plst, bool under_construct) const {
   CTM trans = transtack.front();
   _dbl_word numpnts;
//   trans = transtack.top();
   if ((numpnts = _plist.size()) == 0) return;
   word i;
   if (under_construct) {
      glBegin(GL_LINE_STRIP);
      for (i = 0; i < numpnts; i++) 
         glVertex2i(_plist[i].x(), _plist[i].y());
      TP newp = _plist[numpnts-1] * trans;
      glVertex2i(newp.x(), newp.y());
      if ((numpnts > 2) || ((2 == numpnts) && (newp != _plist[numpnts-1])))
         glVertex2i(_plist[0].x(), _plist[0].y());
      glEnd();
   }
   else {
      pointlist* ptlist;
      if (sh_partsel == status()) {
         CTM strans = transtack.back();
         assert(plst);
         ptlist = movePointsSelected(*plst, trans, strans);
      }
      else {
         ptlist = DEBUG_NEW pointlist;
         ptlist->reserve(numpnts);
         for (i = 0; i < numpnts; i++) 
            ptlist->push_back(_plist[i] * trans);
      }
      glBegin(GL_LINE_LOOP);
      for (i = 0; i < numpnts; i++) 
         glVertex2i((*ptlist)[i].x(), (*ptlist)[i].y());
      glEnd();
      ptlist->clear();
      delete ptlist;
   }
}

void laydata::tdtpoly::rmpoint(TP& lp) {
   assert(_plist.size() > 0);
   _plist.pop_back();
   if (_plist.size() > 0) lp = _plist.back();
};

void  laydata::tdtpoly::select_points(DBbox& select_in, SGBitSet& pntlst) {
   for (word i = 0; i < _plist.size(); i++) 
      if (select_in.inside(_plist[i])) pntlst.set(i);
   pntlst.check_neighbours_set(false);   
}

void laydata::tdtpoly::unselect_points(DBbox& select_in, SGBitSet& pntlst) {
   if (sh_selected == _status)  // the whole shape use to be selected
      pntlst.setall();
   for (word i = 0; i < _plist.size(); i++) 
      if (select_in.inside(_plist[i])) pntlst.reset(i);
   pntlst.check_neighbours_set(false);   
}

laydata::validator* laydata::tdtpoly::move(const CTM& trans, SGBitSet& plst)
{
   if (0 != plst.size())
   {// modify i.e. move when only part of the polygon is selected
    // should get here only
      pointlist* nshape = movePointsSelected(plst, trans);
      laydata::valid_poly* check = DEBUG_NEW laydata::valid_poly(*nshape);
      if (laydata::shp_OK == check->status()) {
         // assign the modified pointlist ONLY if the resulting shape is perfect
         _plist.clear(); _plist = (*nshape);
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
      if (_plist.size() > 4)
      {
         transfer(trans);
         return NULL;
      }
      else
      {  // The whole gymnastics is because of the Rotate operation (on angles <> 90)
         // Rotated box is converted to polygon. Means that polygon after rotatoin could
         // produce a box
         pointlist* mlist = DEBUG_NEW pointlist(_plist);
         for (unsigned i = 0; i < _plist.size(); i++)
            (*mlist)[i] *= trans;
         //@TODO: It's a waste of resources to recheck every polygon again. What we need here is
         // only check for box. Validation classes has to be optimized
         laydata::valid_poly* check = DEBUG_NEW laydata::valid_poly(*mlist);
         if (!(laydata::shp_box & check->status()))
         {
            // leave the modified pointlist ONLY if the resulting shape is not a box
            _plist.clear(); _plist = *mlist;
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

void laydata::tdtpoly::transfer(const CTM& trans) {
   for (unsigned ii = 0; ii < _plist.size(); ii++) 
      _plist[ii] *= trans;
   real area = laydata::polyarea(_plist);
   if (area < 0)
      std::reverse(_plist.begin(),_plist.end());
}

laydata::tdtdata* laydata::tdtpoly::copy(const CTM& trans) {
   // copy the points of the polygon
   pointlist ptlist;
   ptlist.reserve(_plist.size());
   for (unsigned i = 0; i < _plist.size(); i++) 
      ptlist.push_back(_plist[i] * trans);
   return DEBUG_NEW tdtpoly(ptlist);
}

bool laydata::tdtpoly::point_inside(TP pnt) {
   TP p0, p1;
   byte cc = 0;
   unsigned size = _plist.size();
   for (unsigned i = 0; i < size ; i++) {
      p0 = _plist[i]; p1 = _plist[(i+1) % size];
      if (((p0.y() <= pnt.y()) && (p1.y() >  pnt.y())) 
        ||((p0.y() >  pnt.y()) && (p1.y() <= pnt.y())) ) {
         float tngns = (float) (pnt.y() - p0.y())/(p1.y() - p0.y());
         if (pnt.x() < p0.x() + tngns * (p1.x() - p0.x()))
            cc++;
      }
   }
   return (cc & 0x01) ? true : false;
}

void laydata::tdtpoly::polycut(pointlist& cutter, shapeList** decure)
{
   logicop::logic operation(_plist, cutter);
   try
   {
      operation.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {return;}
   logicop::pcollection cut_shapes;
   laydata::tdtdata* newshape;
   if (operation.AND(cut_shapes))
   {
      logicop::pcollection::const_iterator CI;
      // add the resulting cut_shapes to the_cut shapeList
      for (CI = cut_shapes.begin(); CI != cut_shapes.end(); CI++)
         if (NULL != (newshape = createValidShape(*CI)))
            decure[1]->push_back(newshape);
      cut_shapes.clear();
      // if there is a cut - there should be cut remains as well
      operation.reset_visited();
      logicop::pcollection rest_shapes;
      if (operation.ANDNOT(rest_shapes))
         // add the resulting cut remainings to the_rest shapeList
         for (CI = rest_shapes.begin(); CI != rest_shapes.end(); CI++) 
            if (NULL != (newshape = createValidShape(*CI)))
               decure[2]->push_back(newshape);
      rest_shapes.clear();
      // and finally add this to the_delete shapelist
      decure[0]->push_back(this);
   }
}

void laydata::tdtpoly::stretch(int bfactor, shapeList** decure)
{
   logicop::stretcher sh_resize(_plist, bfactor);
   pointlist* res = sh_resize.execute();
   valid_poly vsh(*res);
   if (vsh.valid() && !(laydata::shp_clock & vsh.status()))
   {
      decure[0]->push_back(this);
      decure[1]->push_back(vsh.replacement());
   }
   else if ( vsh.recoverable() && (!(laydata::shp_clock & vsh.status())) )
   {
      logicop::CrossFix fixingpoly(*res);
      try
      {
         fixingpoly.findCrossingPoints();
      }
      catch (EXPTNpolyCross) {return;}
      if (1 == fixingpoly.crossp())
      throw EXPTNpolyCross("Only one crossing point found. Can't generate polygons");

      logicop::pcollection cut_shapes;
      laydata::tdtdata* newshape;
      if ( fixingpoly.generate(cut_shapes, bfactor) )
      {
         logicop::pcollection::const_iterator CI;
         // add the resulting fixed_shapes to the_cut shapeList
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

void laydata::tdtpoly::info(std::ostringstream& ost, real DBU) const {
   ost << "polygon - {";
   unsigned lstsize = _plist.size();
   unsigned lastpnt = lstsize-1;
   for (unsigned i = 0; i < lstsize; i++) {
      _plist[i].info(ost, DBU);
      if (i != lastpnt) ost << " , ";
   }
   ost << "};";
}

void laydata::tdtpoly::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_POLY);
   tedfile->putWord(_plist.size());
   for (word i = 0; i < _plist.size(); i++)
      tedfile->putTP(&_plist[i]);
}

void laydata::tdtpoly::GDSwrite(GDSin::GDSFile& gdsf, word lay, real) const
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_BOUNDARY);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_LAYER);
   wr->add_int2b(lay);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_DATATYPE);
   wr->add_int2b(0);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_XY,_plist.size()+1);
   for (word i = 0; i < _plist.size(); i++)
   {
      wr->add_int4b(_plist[i].x());wr->add_int4b(_plist[i].y());
   }
   wr->add_int4b(_plist[0].x());wr->add_int4b(_plist[0].y());
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ENDEL);
   gdsf.flush(wr);
}

void laydata::tdtpoly::PSwrite(PSFile& gdsf, const layprop::DrawProperties&) const
{
   gdsf.poly(_plist, overlap());
}

DBbox laydata::tdtpoly::overlap() const {
   DBbox ovl = DBbox(_plist[0]);
   for (word i = 1; i < _plist.size(); i++)
      ovl.overlap(_plist[i]);
   return ovl;
}

pointlist* laydata::tdtpoly::movePointsSelected(const SGBitSet& pset, 
                                    const CTM&  movedM, const CTM& stableM) const {
   pointlist* mlist = DEBUG_NEW pointlist(_plist);
   word size = mlist->size();
   PSegment seg1,seg0;
   // See the note about this algo in tdtbox::movePointsSelected above
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

//-----------------------------------------------------------------------------
// class tdtwire
//-----------------------------------------------------------------------------
laydata::tdtwire::tdtwire(TEDfile* const tedfile) : tdtdata() 
{
   word numpoints = tedfile->getWord();
   _width = tedfile->getWord();
   _plist.reserve(numpoints);
   for (word i = 0 ; i < numpoints; i++) 
      _plist.push_back(tedfile->getTP());
}

DBbox* laydata::tdtwire::endPnts(const TP& p1, const TP& p2, bool first) const {
   double     w = _width/2;
   double denom = first ? (p2.x() - p1.x()) : (p1.x() - p2.x());
   double   nom = first ? (p2.y() - p1.y()) : (p1.y() - p2.y());
   double xcorr, ycorr; // the corrections
   if ((0 == nom) && (0 == denom)) return NULL;
   double signX = (  nom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   double signY = (denom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   if      (0 == denom)   {xcorr =signX * w ; ycorr = 0;} // vertical
   else if (0 == nom  )   {xcorr = 0 ; ycorr = signY * w;} // horizontal |----|
   else {
      double sl   = nom / denom;
      double sqsl = signY*sqrt( sl*sl + 1);
      xcorr = rint(w * (sl / sqsl)); 
      ycorr = rint(w * ( 1 / sqsl));
   }
   TP pt = first ? p1 : p2;
   return DEBUG_NEW DBbox((int4b) rint(pt.x() - xcorr), (int4b) rint(pt.y() + ycorr),
                          (int4b) rint(pt.x() + xcorr), (int4b) rint(pt.y() - ycorr));
}

DBbox* laydata::tdtwire::mdlPnts(const TP& p1, const TP& p2, const TP& p3) const {
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

void laydata::tdtwire::openGL_precalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   if (_plist.size() < 2) return;
   // first check whether to draw only the center line
//   DBbox wsquare = DBbox(TP(0,0),TP(_width,_width)) * transtack.top();
//   wsquare *= drawprop.ScrCTM();
//   bool center_line_only = (wsquare.area() < MIN_VISUAL_AREA);
   bool center_line_only = false;
   unsigned num_points = _plist.size();
   if (center_line_only)
      ptlist.reserve(num_points);
   else
      ptlist.reserve(3 * num_points);
   // translate the points using the current CTM
   for (unsigned i = 0; i < num_points; i++)
      ptlist.push_back(_plist[i] * drawprop.topCTM());
   if (!center_line_only)
      precalc(ptlist, num_points);
}

void laydata::tdtwire::openGL_drawline(layprop::DrawProperties&, const pointlist& ptlist) const
{
   _dbl_word num_points = ptlist.size();
   if (0 == ptlist.size()) return;
   // to keep MS VC++ happy - define the counter outside the loops
   _dbl_word i;
   _dbl_word num_cpoints = (num_points == _plist.size()) ? num_points : num_points / 3;
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

void laydata::tdtwire::openGL_drawfill(layprop::DrawProperties&, const pointlist& ptlist) const
{
   if (_plist.size() == ptlist.size()) return;
   unsigned i;
   // Start tessellation
   gluTessBeginPolygon(tessellObj, NULL);
   GLdouble pv[3];
   pv[2] = 0;
   for (i = _plist.size(); i < 3*_plist.size(); i = i + 2)
   {
      pv[0] = ptlist[i].x(); pv[1] = ptlist[i].y();
      gluTessVertex(tessellObj,pv,const_cast<TP*>(&ptlist[i]));
   }
   for (i = 3*_plist.size() - 1; i > _plist.size(); i = i - 2)
   {
      pv[0] = ptlist[i].x(); pv[1] = ptlist[i].y();
      gluTessVertex(tessellObj,pv,const_cast<TP*>(&ptlist[i]));
   }
   gluTessEndPolygon(tessellObj);
}

void laydata::tdtwire::openGL_drawsel(const pointlist& ptlist, const SGBitSet* pslist) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_STRIP);
      for (unsigned i = 0; i < _plist.size(); i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
   else if (sh_partsel == status())
   {
      assert(pslist);
      unsigned numpoints = _plist.size();
      glBegin(GL_LINES);
      for (unsigned i = 0; i < numpoints-1; i++)
      {
         if (pslist->check(i) && pslist->check((i+1)%numpoints))
         {
            glVertex2i(ptlist[i].x(), ptlist[i].y());
            glVertex2i(ptlist[(i+1)%numpoints].x(), ptlist[(i+1)%numpoints].y());
         }
      }
      if (pslist->check(0))
      {// if only the first is selected
         glVertex2i(ptlist[numpoints].x(), ptlist[numpoints].y());
         glVertex2i(ptlist[numpoints+1].x(), ptlist[numpoints+1].y());
      }
      if (pslist->check(numpoints-1))
      {// if only the last is selected
         glVertex2i(ptlist[3*numpoints-1].x(), ptlist[3*numpoints-1].y());
         glVertex2i(ptlist[3*numpoints-2].x(), ptlist[3*numpoints-2].y());
      }
      glEnd();
   }
}

void laydata::tdtwire::tmp_draw(const layprop::DrawProperties& drawprop,
               ctmqueue& transtack, SGBitSet* plst, bool under_construct) const
{
   CTM trans = transtack.front();
   pointlist* ptlist;
   _dbl_word num_points = _plist.size();
   if (under_construct)
   {
      if (num_points == 0) return;
      ptlist = DEBUG_NEW pointlist;
      ptlist->reserve(3*(num_points + 1));
      for (unsigned i = 0; i < num_points; i++)
         ptlist->push_back(_plist[i]);
      ptlist->push_back(_plist[num_points-1] * trans);
      num_points++;
   }
   else
   {
      if (num_points < 2) return;
//      ptlist->reserve(3*num_points);
      if (sh_partsel == status())
      {
         CTM strans = transtack.back();
         assert(plst);
         ptlist = movePointsSelected(*plst, trans, strans);
      }
      else
      {
         ptlist = DEBUG_NEW pointlist;
         for (unsigned i = 0; i < num_points; i++)
            ptlist->push_back(_plist[i] * trans);
      }
   }
   precalc((*ptlist), num_points);
   openGL_drawline(const_cast<layprop::DrawProperties&>(drawprop), *ptlist);
//      if (drawprop.getCurrentFill())
//         openGL_drawfill(ptlist);
   ptlist->clear(); delete ptlist;
}

void laydata::tdtwire::precalc(pointlist& ptlist, _dbl_word num_points) const
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

void  laydata::tdtwire::rmpoint(TP& lp) {
   assert(_plist.size() > 0);
   _plist.pop_back();
   if (_plist.size() > 0) lp = _plist.back();
};

bool laydata::tdtwire::point_inside(TP pnt)
{
   TP p0, p1;
   for (unsigned i = 0; i < _plist.size() - 1 ; i++) {
      p0 = _plist[i]; p1 = _plist[i+1];
      float distance = get_distance(p0,p1,pnt);
      if ((distance > 0) && (distance < _width/2))
         return true;
   }
   return false;
}

float laydata::tdtwire::get_distance(TP p1, TP p2, TP p0)
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

void  laydata::tdtwire::select_points(DBbox& select_in, SGBitSet& pntlst)
{
   for (word i = 0; i < _plist.size(); i++) 
      if (select_in.inside(_plist[i])) pntlst.set(i);
   pntlst.check_neighbours_set(true);   
}

void laydata::tdtwire::unselect_points(DBbox& select_in, SGBitSet& pntlst)
{
   if (sh_selected == _status) // the whole shape use to be selected
      pntlst.setall();      
   for (word i = 0; i < _plist.size(); i++) 
      if (select_in.inside(_plist[i])) pntlst.reset(i);
   pntlst.check_neighbours_set(true);   
}

laydata::validator* laydata::tdtwire::move(const CTM& trans, SGBitSet& plst)
{
   if (0 != plst.size()) 
   {
      pointlist* nshape = movePointsSelected(plst, trans);
      laydata::valid_wire* check = DEBUG_NEW laydata::valid_wire(*nshape, _width);
      if (laydata::shp_OK == check->status()) {
         // assign the modified pointlist ONLY if the resulting shape is perfect
         _plist.clear(); _plist = *nshape;
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

void laydata::tdtwire::transfer(const CTM& trans)
{
   for (unsigned i = 0; i < _plist.size(); i++) 
      _plist[i] *= trans;
}

laydata::tdtdata* laydata::tdtwire::copy(const CTM& trans) {
   // copy the points of the wire
   pointlist ptlist;
   ptlist.reserve(_plist.size());
   for (unsigned i = 0; i < _plist.size(); i++) 
      ptlist.push_back(_plist[i] * trans);
   return DEBUG_NEW tdtwire(ptlist,_width);
}

void laydata::tdtwire::stretch(int bfactor, shapeList** decure)
{
   //@TODO cut bfactor from both sides
   if ((2*bfactor + _width) > 0)
   {
      tdtwire* modified = DEBUG_NEW tdtwire(_plist, 2*bfactor + _width);
      decure[1]->push_back(modified);
   }
   decure[0]->push_back(this);
}

void laydata::tdtwire::info(std::ostringstream& ost, real DBU) const {
   ost << "wire " << _width/DBU << " - {";
   unsigned lstsize = _plist.size();
   unsigned lastpnt = lstsize-1;
   for (unsigned i = 0; i < lstsize; i++) {
      _plist[i].info(ost, DBU);
      if (i != lastpnt) ost << " , ";
   }
   ost << "};";
}

void laydata::tdtwire::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_WIRE);
   tedfile->putWord(_plist.size());
   tedfile->putWord(_width);
   for (word i = 0; i < _plist.size(); i++)
      tedfile->putTP(&_plist[i]);
}

void laydata::tdtwire::GDSwrite(GDSin::GDSFile& gdsf, word lay, real) const
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_PATH);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_LAYER);
   wr->add_int2b(lay);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_DATATYPE);
   wr->add_int2b(0);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_WIDTH);
   wr->add_int4b(_width);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_XY,_plist.size());
   for (word i = 0; i < _plist.size(); i++)
   {
      wr->add_int4b(_plist[i].x());wr->add_int4b(_plist[i].y());
   }
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ENDEL);
   gdsf.flush(wr);
}

void laydata::tdtwire::PSwrite(PSFile& gdsf, const layprop::DrawProperties&) const
{
   gdsf.wire(_plist, _width, overlap());
}

DBbox laydata::tdtwire::overlap() const 
{
   word nump = _plist.size();
   DBbox* ln1 = endPnts(_plist[0],_plist[1], true);
   DBbox ovl = *ln1;delete ln1;
   if (nump > 2) 
   {
      DBbox* ln2 = NULL;
      for (word i = 1; i < nump - 1; i++) 
      {
         ln2 = mdlPnts(_plist[i-1],_plist[i],_plist[i+1]);
         ovl.overlap(*ln2);
         delete ln2; 
      }
   }
   ln1 = endPnts(_plist[nump-2],_plist[nump-1],false);
   ovl.overlap(*ln1);
   delete ln1; 
   return ovl;
}

pointlist* laydata::tdtwire::movePointsSelected(const SGBitSet& pset, 
                                    const CTM& movedM, const CTM& stableM) const {
   pointlist* mlist = DEBUG_NEW pointlist(_plist);
   word size = mlist->size();
   PSegment* seg1 = NULL;
   PSegment* seg0 = NULL;
   for (int i = 0; i < size; i++) {
      if ((size-1) == i) {
         if (pset.check(size-1))
            seg1 = seg1->ortho((*mlist)[size-1] * movedM);
         else
            seg1 = seg1->ortho((*mlist)[size-1] * stableM);
      }
      else {
         const CTM& transM = ((pset.check(i) && pset.check(i+1))) ? 
                                                               movedM : stableM;
         seg1 = DEBUG_NEW PSegment((*mlist)[(i  )] * transM, (*mlist)[(i+1)] * transM);
         if (0 == i)
            if (pset.check(0))
               seg0 = seg1->ortho((*mlist)[i] * movedM);
            else
               seg0 = seg1->ortho((*mlist)[i] * stableM);
      }
      if (!seg0->empty()) seg1->crossP(*seg0,(*mlist)[i]);
      if (NULL != seg0) delete seg0;
      seg0 = seg1;
   }
   if (NULL != seg0) delete seg0;
   return mlist;
}
//-----------------------------------------------------------------------------
// class tdtcellref
//-----------------------------------------------------------------------------
laydata::tdtcellref::tdtcellref(TEDfile* const tedfile)
{
   // read the name of the referenced cell
   std::string cellrefname = tedfile->getString();
   // get the cell definition pointer and register the cellrefname as a child 
   // to the currently parsed cell
   _structure = tedfile->linkcellref(cellrefname);
   // get the translation   
   _translation = tedfile->getCTM();
}

void laydata::tdtcellref::openGL_precalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // calculate the current translation matrix
   CTM newtrans = _translation * drawprop.topCTM();
   // get overlapping box of the structure ...
   DBbox obox(DEFAULT_ZOOM_BOX);
   if (structure()) 
      obox = structure()->overlap();
   // ... translate it to the current coordinates ...
   DBbox areal = obox * newtrans;
   // ... and normalize it
   areal.normalize();
   // check that the cell (or part of it) is in the visual window
   DBbox clip = drawprop.clipRegion();
   if (clip.cliparea(areal) == 0) return;
   // check that the cell area is bigger that the MIN_VISUAL_AREA
   DBbox minareal = areal * drawprop.ScrCTM();
   if (minareal.area() < MIN_VISUAL_AREA) return;
   // If we get here - means that the cell (or part of it) is visible
   ptlist.reserve(4);
   ptlist.push_back(obox.p1() * newtrans);
   ptlist.push_back(TP(obox.p2().x(), obox.p1().y()) * newtrans);
   ptlist.push_back(obox.p2() * newtrans);
   ptlist.push_back(TP(obox.p1().x(), obox.p2().y()) * newtrans);
   drawprop.pushCTM(newtrans);
   // draw the cell mark ...
//   drawprop.setCurrentColor(0);
   drawprop.draw_reference_marks(TP(0,0) * newtrans, layprop::cell_mark);
}

void laydata::tdtcellref::openGL_drawline(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   drawprop.draw_cell_boundary(ptlist);
}

void laydata::tdtcellref::openGL_drawfill(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if ((NULL == structure()) || (0 == ptlist.size())) return;
   // draw the structure itself. Pop/push ref stuff is when edit in place is active
   byte crchain = drawprop.popref(this);
   structure()->openGL_draw(drawprop, crchain == 2);
   // push is done in the precalc()
//   drawprop.popCTM();
   if (crchain) drawprop.pushref(this);
}

void laydata::tdtcellref::openGL_drawsel(const pointlist& ptlist, const SGBitSet*) const
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

void laydata::tdtcellref::openGL_postclean(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   ptlist.clear();
   // get the font matrix out of the stack (pushed in precalc)
   drawprop.popCTM();
}

void laydata::tdtcellref::tmp_draw(const layprop::DrawProperties& drawprop,
                 ctmqueue& transtack, SGBitSet*, bool under_construct) const
{
   if (structure())
   {
      transtack.push_front(_translation * transtack.front());
      structure()->tmp_draw(drawprop, transtack);
   }
}

void laydata::tdtcellref::info(std::ostringstream& ost, real DBU) const {
   ost << "cell \"" << _structure->first << "\" - reference @ {";
   ost << _translation.tx()/DBU << " , " << _translation.ty()/DBU << "}";
}

void laydata::tdtcellref::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_CELLREF);
   tedfile->putString(_structure->first);
   tedfile->putCTM(_translation);
}

laydata::tdtcell* laydata::tdtcellref::cstructure() const
{
   laydata::tdtcell* celldef;
   if (_structure->second->libID())
      celldef =  static_cast<laydata::tdtcell*>(_structure->second);
   else
      celldef = NULL;
   return celldef;
}

void laydata::tdtcellref::GDSwrite(GDSin::GDSFile& gdsf, word lay, real) const
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_SREF);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_SNAME, _structure->first.size());
   wr->add_ascii(_structure->first.c_str());gdsf.flush(wr);
   TP trans;
   real rotation, scale;
   bool flipX;
   _translation.Decompose(trans,rotation,scale,flipX);
   wr = gdsf.SetNextRecord(gds_STRANS);
   if (flipX) wr->add_int2b(0x8000);
   else       wr->add_int2b(0x0000);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_MAG);
   wr->add_real8b(scale);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ANGLE);
   wr->add_real8b(rotation);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_XY,1);
   wr->add_int4b(trans.x());wr->add_int4b(trans.y());
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ENDEL);
   gdsf.flush(wr);
}

void laydata::tdtcellref::PSwrite(PSFile& psf, const layprop::DrawProperties& drawprop) const
{
   psf.cellref(_structure->first, _translation);
   if (!psf.hier())
   {
     _structure->second->PSwrite(psf, drawprop);
   }
}

void laydata::tdtcellref::ungroup(laydata::tdtdesign* ATDB, tdtcell* dst, atticList* nshp)
{
   tdtdata *data_copy;
   shapeList* ssl;
   // select all the shapes of the referenced tdtcell
   if (NULL == cstructure())
   {
      std::ostringstream ost;
      ost << "Cell \"" << structure()->name() << "\" is undefined. Ignored during ungoup.";
      tell_log(console::MT_WARNING, ost.str());
      return;
   }
   cstructure()->full_select();
   for (selectList::const_iterator CL = cstructure()->shapesel()->begin();
                                   CL != cstructure()->shapesel()->end(); CL++)
   {
      // secure the target layer
      quadTree* wl = dst->securelayer(CL->first);
      // There is no point here to ensure that the layer definition exists.
      // We are just transfering shapes from one structure to another.
      // Of course ATDB is undefined (forward defined) here, so if the method has to be
      // used here - something else should be done
      // ATDB->securelaydef( CL->first );
      // secure the select layer (for undo)
      if (nshp->end() != nshp->find(CL->first))  
         ssl = (*nshp)[CL->first];
      else {
         ssl = DEBUG_NEW shapeList();
         (*nshp)[CL->first] = ssl;
      }   
      // for every single shape on the layer
      for (dataList::const_iterator DI = CL->second->begin();
                                    DI != CL->second->end(); DI++)
      {
         // create a new copy of the data
         data_copy = DI->first->copy(_translation);
         // add it to the corresponding layer of the dst cell
         wl->put(data_copy);
         // ... and to the list of the new shapes (for undo)
         ssl->push_back(data_copy);
         //update the hierarchy tree if this is a cell
         if (0 == CL->first) dst->addchild(ATDB, 
                            static_cast<tdtcellref*>(data_copy)->cstructure());
         // add it to the selection list of the dst cell
         dst->select_this(data_copy,CL->first);
      }
      wl->invalidate();
   }
   cstructure()->unselect_all();
}

DBbox laydata::tdtcellref::overlap() const
{
   return structure()->overlap() * _translation;
}

//-----------------------------------------------------------------------------
// class tdtcellaref
//-----------------------------------------------------------------------------
laydata::tdtcellaref::tdtcellaref(TEDfile* const tedfile) : tdtcellref(tedfile)
{
   int4b _stepX = tedfile->get4b();
   int4b _stepY = tedfile->get4b();
   word _rows = tedfile->getWord();
   word _cols = tedfile->getWord();
   _arrprops = ArrayProperties(_stepX, _stepY, _cols, _rows);
}

// bool laydata::tdtcellaref::aref_visible(layprop::DrawProperties& drawprop, int* stst) const {
//    // make sure that the referenced structure exists
//    if (NULL == structure()) return false;
//    // Get the areal of entire matrix, but NOT TRANSLATED !
//    DBbox array_overlap = clear_overlap();
//    // Calculate the CTM for the array
//    CTM newtrans = _translation * drawprop.topCTM();
//    // ... get the current visual (clipping) window, and make a REVERSE TRANSLATION
//    DBbox clip = drawprop.clipRegion() * newtrans.Reversed();
//    clip.normalize();
//    // initialize the visual box from the overlap area of the array ...
//    DBbox visual_box(array_overlap);
//    // ... and check the visibility of entire array. if mutual_position
//    // is 1, visual_box will be modified and will contain the visual region
//    // of the array
//    int mutual_position = clip.clipbox(visual_box);
//    // if array is entirely outside the visual window - bail out
//    if (0 == mutual_position) return false;
// 
//    // If we get here - means that the array (or part of it) is visible
//    // draw the cell mark ...
//    drawprop.draw_reference_marks(TP(0,0) * newtrans, layprop::array_mark);
//    // ... and the overlapping box
//    draw_overlapping_box(array_overlap, newtrans, 0xf18f);
//    // now check that a single structure is big enough to be visible
//    DBbox structure_overlap = structure()->overlap();
//    DBbox minareal = structure_overlap * drawprop.topCTM() * drawprop.ScrCTM();
//    if (minareal.area() < MIN_VISUAL_AREA) return false;
//    // We are going to draw cells, so push the new translation matrix in the stack
//    drawprop.pushCTM(newtrans);
//    // now calculate the start/stop values of the visible references in the matrix
//    if (-1 == mutual_position) {
//       // entire matrix is visible
//       stst[0] = 0; stst[1] = _cols;
//       stst[2] = 0; stst[3] = _rows;
//    }
//    else {
//       real cstepX = (array_overlap.p2().x() - array_overlap.p1().x()) / _cols;
//       real cstepY = (array_overlap.p2().y() - array_overlap.p1().y()) / _rows;
//       // matrix is partially visible
//       stst[0] = array_overlap.p1().x() < clip.p1().x() ?
//           (int) rint((clip.p1().x() - array_overlap.p1().x()) / cstepX) : 0;
//       stst[2] = array_overlap.p1().y() < clip.p1().y() ?
//           (int) rint((clip.p1().y() - array_overlap.p1().y()) / cstepY) : 0;
//       stst[1] = stst[0] + (int) rint((visual_box.p2().x() - visual_box.p1().x()) / cstepX);
//       stst[3] = stst[2] + (int) rint((visual_box.p2().y() - visual_box.p1().y()) / cstepY);
//       // add an extra row/column from both sides to ensure visibility of the`
//       // border areas
//       stst[0] -= (0 == stst[0]) ? 0 : 1;
//       stst[2] -= (0 == stst[2]) ? 0 : 1;
//       stst[1] += (_cols == stst[1]) ? 0 : 1;
//       stst[3] += (_rows == stst[3]) ? 0 : 1;
// 
//    }
//    return true;
// }

void laydata::tdtcellaref::openGL_precalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   // make sure that the referenced structure exists
   //if (NULL == structure()) return;
   assert(structure());
   if (0 != drawprop.drawinglayer())
   {
      int boza = drawprop.drawinglayer();
      boza++;
   }
   // Get the areal of entire matrix, but NOT TRANSLATED !
   DBbox array_overlap = clear_overlap();
   // Calculate the CTM for the array
   CTM newtrans = _translation * drawprop.topCTM();
   // ... get the current visual (clipping) window, and make a REVERSE TRANSLATION
   DBbox clip = drawprop.clipRegion() * newtrans.Reversed();
   clip.normalize();
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
//   drawprop.setCurrentColor(0);
   drawprop.draw_reference_marks(TP(0,0) * newtrans, layprop::array_mark);
   // ... and the overlapping box
   // If we get here - means that the cell (or part of it) is visible
   
   ptlist.reserve(6);
   ptlist.push_back(array_overlap.p1() * newtrans);
   ptlist.push_back(TP(array_overlap.p2().x(), array_overlap.p1().y()) * newtrans);
   ptlist.push_back(array_overlap.p2() * newtrans);
   ptlist.push_back(TP(array_overlap.p1().x(), array_overlap.p2().y()) * newtrans);
   
   // now check that a single structure is big enough to be visible
   DBbox structure_overlap = structure()->overlap();
   DBbox minareal = structure_overlap * drawprop.topCTM() * drawprop.ScrCTM();
   // We are going to draw "something", so push the new translation matrix in the stack
   drawprop.pushCTM(newtrans);
   if (minareal.area() < MIN_VISUAL_AREA)
   {
      ptlist.push_back(TP(0,0));
      ptlist.push_back(TP(0,0));
   }
   else
   {
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
}

void laydata::tdtcellaref::openGL_drawline(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   // draw the overlapping box
   glColor4f(1.0, 1.0, 1.0, 0.5);
   glLineStipple(1,0xf18f);
   glEnable(GL_LINE_STIPPLE);
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < 4; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
   glDisable(GL_LINE_STIPPLE);
}

void laydata::tdtcellaref::openGL_drawfill(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   for (int i = ptlist[4].x(); i < ptlist[4].y(); i++)
   {// start/stop rows
      for(int j = ptlist[5].x(); j < ptlist[5].y(); j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
         refCTM *= drawprop.topCTM();
         // ...draw the structure itself, not forgeting to push/pop the refCTM
         drawprop.pushCTM(refCTM);
         structure()->openGL_draw(drawprop);
         drawprop.popCTM();
      }
   }
   // push is done in the precalc()
//   drawprop.popCTM();
}

void laydata::tdtcellaref::openGL_drawsel(const pointlist& ptlist, const SGBitSet*) const
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

void laydata::tdtcellaref::tmp_draw(const layprop::DrawProperties& drawprop,
                 ctmqueue& transtack, SGBitSet*, bool under_construct) const
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
         structure()->tmp_draw(drawprop, transtack);
      }
   }
}

void laydata::tdtcellaref::info(std::ostringstream& ost, real DBU) const {
   ost << "cell \"" << _structure->first << "\" - array reference @ {";
   ost << _translation.tx()/DBU << " , " << _translation.ty()/DBU << "} ->";
   ost << " [" << _arrprops.cols() << " x " << _arrprops.stepX() << " , " ;
   ost <<         _arrprops.rows() << " x " << _arrprops.stepY() << "]";
}

void laydata::tdtcellaref::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_CELLAREF);
   tedfile->putString(_structure->first);
   tedfile->putCTM(_translation);
   tedfile->put4b(_arrprops.stepX());
   tedfile->put4b(_arrprops.stepY());
   tedfile->putWord(_arrprops.rows());
   tedfile->putWord(_arrprops.cols());
}

void laydata::tdtcellaref::GDSwrite(GDSin::GDSFile& gdsf, word lay, real) const
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_AREF);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_SNAME, _structure->first.size());
   wr->add_ascii(_structure->first.c_str());gdsf.flush(wr);
   TP trans;
   real rotation, scale;
   bool flipX;
   _translation.Decompose(trans,rotation,scale,flipX);
   wr = gdsf.SetNextRecord(gds_STRANS);
   if (flipX) wr->add_int2b(0x8000);
   else       wr->add_int2b(0x0000);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_MAG);
   wr->add_real8b(scale);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ANGLE);
   wr->add_real8b(rotation);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_COLROW);
   wr->add_int2b(_arrprops.cols());wr->add_int2b(_arrprops.rows());
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_XY,3);
   wr->add_int4b(trans.x());wr->add_int4b(trans.y());
   wr->add_int4b(trans.x() + _arrprops.cols() * _arrprops.stepX());wr->add_int4b(trans.y());
   wr->add_int4b(trans.x());wr->add_int4b(trans.y() + _arrprops.rows() * _arrprops.stepY());
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ENDEL);
   gdsf.flush(wr);
}

void laydata::tdtcellaref::PSwrite(PSFile& psf, const layprop::DrawProperties& drawprop) const
{
   for (int i = 0; i < _arrprops.cols(); i++)
   {// start/stop rows
      for(int j = 0; j < _arrprops.rows(); j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(TP(_arrprops.stepX() * i , _arrprops.stepY() * j ), 1, 0, false);
         refCTM *= _translation;
         psf.cellref(_structure->first, refCTM);
         if (!psf.hier())
         {
            _structure->second->PSwrite(psf, drawprop);
         }
      }
   }
}

void  laydata::tdtcellaref::ungroup(laydata::tdtdesign* ATDB, tdtcell* dst, laydata::atticList* nshp) {
   for (word i = 0; i < _arrprops.cols(); i++)
      for(word j = 0; j < _arrprops.rows(); j++) {
         // for each of the array figures
         CTM refCTM;
         refCTM.Translate(_arrprops.stepX() * i , _arrprops.stepY() * j );
         refCTM *= _translation;
         laydata::tdtcellref* cellref = DEBUG_NEW tdtcellref(_structure, refCTM);
         cellref->ungroup(ATDB, dst, nshp);
         delete cellref;
      }
}

DBbox laydata::tdtcellaref::overlap() const {
   assert(structure());
   {
     DBbox ovl(clear_overlap() * _translation);
     ovl.normalize();
     return ovl;
   }
//   else return DBbox(TP(static_cast<int4b>(rint(_translation.tx())),
//                         static_cast<int4b>(rint(_translation.ty()))));
}

DBbox laydata::tdtcellaref::clear_overlap() const
{
   assert(structure());
   DBbox bx = structure()->overlap();
   DBbox ovl = bx;
   CTM refCTM(1.0,0.0,0.0,1.0,_arrprops.stepX() * (_arrprops.cols()-1), _arrprops.stepY() * (_arrprops.rows() - 1));
   ovl.overlap(bx * refCTM);
   return ovl;
}

//-----------------------------------------------------------------------------
// class tdttext
//-----------------------------------------------------------------------------
laydata::tdttext::tdttext(std::string text, CTM trans) : tdtdata(), _overlap(TP()) {
   for (unsigned charnum = 0; charnum < text.length(); charnum++)
      if (!isprint(text[charnum])) text[charnum] = '?';
   _text = text;
   _translation = trans;
   float minx, miny, maxx, maxy;
   glfGetStringBounds(_text.c_str(),&minx, &miny, &maxx, &maxy);
   _overlap = DBbox(TP(minx,miny,OPENGL_FONT_UNIT), TP(maxx,maxy,OPENGL_FONT_UNIT));
}
   
laydata::tdttext::tdttext(TEDfile* const tedfile) : tdtdata(), _overlap(TP()) 
{
   _text = tedfile->getString();
   _translation = tedfile->getCTM();
   float minx, miny, maxx, maxy;
   glfGetStringBounds(_text.c_str(),&minx, &miny, &maxx, &maxy);
   _overlap = DBbox(TP(minx,miny,OPENGL_FONT_UNIT), TP(maxx,maxy,OPENGL_FONT_UNIT));
}

void laydata::tdttext::replace_str(std::string newstr)
{
   _text = newstr;
   float minx, miny, maxx, maxy;
   glfGetStringBounds(_text.c_str(),&minx, &miny, &maxx, &maxy);
   _overlap = DBbox(TP(minx,miny,OPENGL_FONT_UNIT), TP(maxx,maxy,OPENGL_FONT_UNIT));
}

void laydata::tdttext::openGL_precalc(layprop::DrawProperties& drawprop, pointlist& ptlist) const
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
   // Independently of the orientation (and flip) font matrix 
   // can be trimmed always so that texts to appear either left-to-right
   // or bottom-to top (Remember Catena?). In order to compensate the text 
   // placement, the binding point (justification) can be compensated
   // And the last, but not the least...
   // GDSII text justification
   //====================================================================
   // the correction is needed to fix the bottom left corner of the
   // text overlapping box to the binding point. glf library normally
   // draws the first symbol centerd around the bounding point
   CTM correction;
   correction.Translate(-_overlap.p1().x(), -_overlap.p1().y());
   DBbox _over = _overlap * correction;
   // font translation matrix
   CTM ftmtrx =  _translation * drawprop.topCTM();
   valid_box wsquare(TP(0, 0),TP(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT), ftmtrx * drawprop.ScrCTM());
   if (wsquare.area() > MIN_VISUAL_AREA)
   {
      // If we get here - means that the text is visible
      // get the text overlapping box ...
      ptlist.reserve(5);
      ptlist.push_back(_over.p1() * ftmtrx);
      ptlist.push_back(TP(_over.p2().x(), _over.p1().y()) * ftmtrx);
      ptlist.push_back(_over.p2() * ftmtrx);
      ptlist.push_back(TP(_over.p1().x(), _over.p2().y()) * ftmtrx);
      // ... and text bounding point (see the comment above)
      ptlist.push_back(TP(static_cast<int4b>(_translation.tx()),
                       static_cast<int4b>(_translation.ty()))  * drawprop.topCTM());
      // push the font matrix - will be used for text drawing
      drawprop.pushCTM(ftmtrx);
   }
}

void laydata::tdttext::openGL_drawline(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   drawprop.draw_text_boundary(ptlist);
   drawprop.draw_reference_marks(ptlist[4], layprop::text_mark);
   // draw the text itself
   glPushMatrix();
   double ori_mtrx[] = { drawprop.topCTM().a(), drawprop.topCTM().b(),0,0,
                         drawprop.topCTM().c(), drawprop.topCTM().d(),0,0,
                                             0,                     0,0,0,
                                 ptlist[4].x(),         ptlist[4].y(),0,1};
   glMultMatrixd(ori_mtrx);
   // correction of the glf shift - as explained in the openGL_precalc above
   glTranslatef(-_overlap.p1().x(), -_overlap.p1().y(), 1);
   // The only difference between glut and glf appears to be the size:-
   // glf is not using the font unit, so we need to scale it back up (see below)
   // but... it uses real numbers - that is not what we need. That's why -
   // keeping the font unit will help to convert the font metrics back to
   // integer coordinates
   glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
   glfDrawTopedString(_text.c_str(),0);
   glPopMatrix();
}

void laydata::tdttext::openGL_drawfill(layprop::DrawProperties& drawprop, const pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   glPushMatrix();
   double ori_mtrx[] = { drawprop.topCTM().a(), drawprop.topCTM().b(),0,0,
                         drawprop.topCTM().c(), drawprop.topCTM().d(),0,0,
                                             0,                     0,0,0,
                                 ptlist[4].x(),         ptlist[4].y(),0,1};
   glMultMatrixd(ori_mtrx);
   // correction of the glf shift - as explained in the openGL_precalc above
   glTranslatef(-_overlap.p1().x(), -_overlap.p1().y(), 1);
   // The only difference between glut and glf appears to be the size:-
   // glf is not using the font unit, so we need to scale it back up (see below)
   // but... it uses real numbers - that is not what we need. That's why -
   // keeping the font unit will help to convert the font metrics back to
   // integer coordinates
   glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
   glfDrawTopedString(_text.c_str(),1);
   glPopMatrix();
}

void laydata::tdttext::openGL_drawsel(const pointlist& ptlist, const SGBitSet*) const
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

void laydata::tdttext::openGL_postclean(layprop::DrawProperties& drawprop, pointlist& ptlist) const
{
   if (0 == ptlist.size()) return;
   ptlist.clear();
   // get the font matrix out of the stack (pushed in precalc)
   drawprop.popCTM();
}

void laydata::tdttext::tmp_draw(const layprop::DrawProperties& drawprop,
               ctmqueue& transtack, SGBitSet*, bool under_construct) const
{
   //====================================================================
   // font translation matrix
   CTM ftmtrx =  _translation * transtack.front();
   valid_box wsquare(TP(0, 0),TP(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT), ftmtrx * drawprop.ScrCTM());
   if (wsquare.area() > MIN_VISUAL_AREA)
   {
      glPushMatrix();
      double ori_mtrx[] = { ftmtrx.a(), ftmtrx.b(),0,0,
                            ftmtrx.c(), ftmtrx.d(),0,0,
                                     0,          0,0,0,
                           ftmtrx.tx(),ftmtrx.ty(),0,1};
      glMultMatrixd(ori_mtrx);
      // correction of the glf shift - as explained in the openGL_precalc above
      glTranslatef(-_overlap.p1().x(), -_overlap.p1().y(), 1);
      glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
      glfDrawWiredString(_text.c_str());
      glPopMatrix();
   }
}

void laydata::tdttext::write(TEDfile* const tedfile) const {
   tedfile->putByte(tedf_TEXT);
   tedfile->putString(_text);
   tedfile->putCTM(_translation);
}

void laydata::tdttext::GDSwrite(GDSin::GDSFile& gdsf, word lay, real UU) const
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_TEXT);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_LAYER);
   wr->add_int2b(lay);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_TEXTTYPE);
   wr->add_int2b(0);gdsf.flush(wr);
   TP trans;
   real rotation, scale;
   bool flipX;
   _translation.Decompose(trans,rotation,scale,flipX);
   wr = gdsf.SetNextRecord(gds_STRANS);
   if (flipX) wr->add_int2b(0x8000);
   else       wr->add_int2b(0x0000);
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_MAG);
   wr->add_real8b(scale * OPENGL_FONT_UNIT * UU);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ANGLE);
   wr->add_real8b(rotation);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_XY,1);
   wr->add_int4b(trans.x());wr->add_int4b(trans.y());
   gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_STRING, _text.size());
   wr->add_ascii(_text.c_str());gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_ENDEL);
   gdsf.flush(wr);
}

void laydata::tdttext::PSwrite(PSFile& gdsf, const layprop::DrawProperties& drawprop) const
{
   CTM fmtrx(_translation);
   fmtrx.Scale(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT);
   CTM ffmtrx(fmtrx.a(), fmtrx.b(), fmtrx.c(), fmtrx.d(), _translation.tx(), _translation.ty());
   gdsf.text(_text, ffmtrx);
}

DBbox laydata::tdttext::overlap() const {
   CTM correction;
   correction.Translate(-_overlap.p1().x(), -_overlap.p1().y());
   return (_overlap * correction * _translation);
}

void laydata::tdttext::info(std::ostringstream& ost, real DBU) const {
   ost << "text \"" << _text << "\" @ {";
   ost << _translation.tx()/DBU << " , " << _translation.ty()/DBU << "}";
}

//-----------------------------------------------------------------------------
// class valid_box
//-----------------------------------------------------------------------------
laydata::valid_box::valid_box(const TP& p1, const TP& p2, const CTM& trans) : validator()
{
   _plist.push_back(p1*trans);
   _plist.push_back(TP(p2.x(), p1.y())*trans);
   _plist.push_back(p2*trans);
   _plist.push_back(TP(p1.x(), p2.y())*trans);
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

laydata::tdtdata* laydata::valid_box::replacement()
{
   if (box()) {
      TP* p1= DEBUG_NEW TP(_plist[0]); TP* p2= DEBUG_NEW TP(_plist[2]);
      return DEBUG_NEW laydata::tdtbox(p1, p2);
   }
   else return DEBUG_NEW laydata::tdtpoly(_plist);
}

char* laydata::valid_box::failtype()
{
   if      (!(_status & shp_box))  return "Rotated box";
   else return "OK";
}

//-----------------------------------------------------------------------------
// class valid_poly
//-----------------------------------------------------------------------------
laydata::valid_poly::valid_poly(const pointlist& plist) : validator(plist) { 
   angles();
   if (_status > 0x10) return;
   //reorder the chain (if needed) to get the points in anticlockwise order
   normalize();
   if (_status > 0x10) return;
   //check self crossing
   selfcrossing();
}

laydata::tdtdata* laydata::valid_poly::replacement() {
   laydata::tdtdata* newshape;
   if (box()) {
      TP* p1= DEBUG_NEW TP(_plist[0]); TP* p2= DEBUG_NEW TP(_plist[2]);
      newshape = DEBUG_NEW laydata::tdtbox(p1, p2);
   }
   else newshape = DEBUG_NEW laydata::tdtpoly(_plist);
   return newshape;
}

/**
 * Checks the polygon angles. Filters out intermediate points if 0 or 180 deg
   anngle is found as well as coinciding points. Function also flags a box if
   found as well as the acute angles
 */
void laydata::valid_poly::angles()
{
   pointlist::iterator cp1 = _plist.end(); cp1--;
   pointlist::iterator cp2 = _plist.begin();
   real cAngle;
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

void laydata::valid_poly::normalize() {
   real area = laydata::polyarea(_plist);
   if (area == 0) {
      _status |= shp_null; return;
   }
   else if (area < 0)  {
	   std::reverse(_plist.begin(),_plist.end());
      _status |= laydata::shp_clock;
   }
}

/*! Implements  algorithm to check that the polygon is not
self crossing. Alters the laydata::shp_cross bit of _status if the polygon
is selfcrossing
*/ 
void laydata::valid_poly::selfcrossing()
{
   //using BO modified
   logicop::CrossFix fixingpoly(_plist);
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

char* laydata::valid_poly::failtype() {
   if      (_status & shp_null) return "NULL area polygon";
   else if (_status & shp_cross) return "Self-crossing";
//   else if (_status & shp_acute) return "Acute angle";
   else return "OK";
}   

//-----------------------------------------------------------------------------
// class valid_wire
//-----------------------------------------------------------------------------
laydata::valid_wire::valid_wire(const pointlist& plist, word width) : 
                                     validator(plist), _width(width) { 
   angles();
   if (_status > 0x10) return;
   if (numpoints() > 3)
      selfcrossing();
}

/*! Checks the wire angles. Filters out intermediate and coinciding points.
Flags acute angles and null wires
 */
void laydata::valid_wire::angles()
{
   // check for a single point
   if (_plist.size() < 2) _status |= shp_null;
   pointlist::iterator cp2 = _plist.begin();
   pointlist::iterator cp1 = cp2; cp2++;
   real pAngle, cAngle; //
   bool pAngleValid = false;
	if (_plist.size()>2)
   do
   {
      bool eraseP1 = false;
      if (*cp1 == *cp2)
         eraseP1 = true;
      else
      {
         cAngle = xangle(*cp1, *cp2);
         if (pAngleValid)
         {
            real ang = fabs(cAngle - pAngle);
            if ((0 == ang) || (180 == ang))
               eraseP1 = true;
            else if (ang < 90 || ang > 270)
               _status |= laydata::shp_acute;
         }
         pAngleValid = true;
         pAngle = cAngle;
      }
      if (eraseP1)
      {
         cp2 = _plist.erase(cp1);
			cp1 = cp2; cp2++;
         _status |= laydata::shp_ident;
      }
      else
      {
         cp1 = cp2; cp2++;
      }
   }
   while(cp2 != _plist.end());
   if (((_plist.size() == 2) && (*(_plist.begin()) == *(_plist.end()-1))) ||
            (_plist.size() < 2))
      _status |= shp_null;
}


/*! Implements  algorithm to check that the wire is not simple crossing. 
Alters the laydata::shp_cross bit of _status if the wire is selfcrossing
*/ 
void laydata::valid_wire::selfcrossing() {
   
   //using BO modified
   logicop::CrossFix fixingpoly(_plist);
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

laydata::tdtdata* laydata::valid_wire::replacement() {
   return DEBUG_NEW laydata::tdtwire(_plist, _width);
}   

char* laydata::valid_wire::failtype() {
//   if (_status & shp_null)  return "Zero area";
   if      (_status & shp_cross) return "Self-crossing";
   else if (_status & shp_null ) return "Unsuficcient points";
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
laydata::tdtdata* laydata::createValidShape(pointlist* pl) {
   laydata::valid_poly check(*pl);
   delete pl;
   if (!check.valid()) {
      std::ostringstream ost;
      ost << "Resulting shape is invalid - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
      return NULL;
   }
   laydata::tdtdata* newshape;
   pointlist npl = check.get_validated();
   if (check.box())
      newshape = DEBUG_NEW laydata::tdtbox(DEBUG_NEW TP(npl[2]),DEBUG_NEW TP(npl[0]));
   else
      newshape = DEBUG_NEW laydata::tdtpoly(npl);
   npl.clear();
   return newshape;
}

laydata::tdtdata* laydata::polymerge(const pointlist& _plist0, const pointlist& _plist1) {
   if(_plist0.empty() || _plist1.empty()) return NULL;
   logicop::logic operation(_plist0, _plist1);
   try
   {
      operation.findCrossingPoints();
   }
   catch (EXPTNpolyCross) {return NULL;}
   logicop::pcollection merge_shape;
   laydata::tdtdata* resShape = NULL;
   if (operation.OR(merge_shape)) {
      assert(1 == merge_shape.size());
      resShape = createValidShape(*merge_shape.begin());
//      return new laydata::tdtpoly(**merge_shape.begin());
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

