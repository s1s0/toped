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
// ------------------------------------------------------------------------ =
//           $URL$
//  Creation date: Sun Apr 18 10:59:37 BST 2004
//         Author: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <sstream>
#include "../tpd_common/ttt.h"
#include "tedesign.h"
#include "tedat.h"
#include "../tpd_common/tedop.h"
#include "viewprop.h"
#include "../tpd_common/outbox.h"

//! the stack of all previously edited (opened) cells
laydata::editcellstack laydata::editobject::_editstack;
//-----------------------------------------------------------------------------
// class tdtdesign
//-----------------------------------------------------------------------------
laydata::tdtdesign::tdtdesign(std::string name, real DBU, real UU) {
   _name = name;
   _DBU  = DBU; _UU   = UU;
   _tmpdata = NULL;
   modified = false;
   _hiertree = NULL;
}

void laydata::tdtdesign::read(TEDfile* const tedfile) {
   std::string cellname;
   while (tedf_CELL == tedfile->getByte()) {
      cellname = tedfile->getString(); if (!tedfile->status()) return;
      tell_log(console::MT_CELLNAME, cellname.c_str());
//      laydata::AddLog('C', cellname);
      tedfile->registercellread(cellname, new tdtcell(tedfile, cellname));
      if (!tedfile->status()) return;
   }
   recreate_hierarchy();
   _tmpdata = NULL;
   modified = false;
//   AddLog('N', "Done");
   tell_log(console::MT_INFO, "Done");
}

// !!! Do not forget that the indexing[] operations over std::map can alter the structure !!!
// use find for searching.
laydata::tdtcell* laydata::tdtdesign::addcell(std::string name) {
   if (_cells.end() != _cells.find(name)) return NULL; // cell already exists
   else {
      modified = true;
      tdtcell* ncl = _cells[name] = new tdtcell(name);
      _hiertree = new TDTHierTree(ncl, NULL, _hiertree);
       btreeAddMember(_hiertree->GetItem()->name().c_str(), NULL, 0);
      return ncl;
   }
}

laydata::tdtdata* laydata::tdtdesign::addbox(word la, TP* p1, TP* p2) {
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   (*p1) *= _target.rARTM();
   (*p2) *= _target.rARTM();
   return actlay->addbox(p1,p2);
}

laydata::tdtdata* laydata::tdtdesign::addpoly(word la, pointlist& pl) {
   laydata::valid_poly check(pl);
   if (!check.valid()) {
      std::ostringstream ost; 
      ost << "Polygon check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str().c_str());
      pl.clear();
      return NULL;
   }
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   // get rid of the original point list
   pl.clear();
   pl = check.get_validated();
   if (check.box()) {
      TP* p1= new TP(pl[0] *_target.rARTM());
      TP* p2= new TP(pl[2] *_target.rARTM());
      pl.clear();
      return actlay->addbox(p1,p2);
   }
   for(pointlist::iterator PL = pl.begin(); PL != pl.end(); PL++)
      (*PL) *= _target.rARTM();
   return actlay->addpoly(pl);
}

laydata::tdtdata* laydata::tdtdesign::addwire(word la, pointlist& pl, word w) {
   laydata::valid_wire check(pl,w);
   if (!check.valid()) {
      std::ostringstream ost; 
      ost << "Wire check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str().c_str());
      pl.clear();
      return NULL;
   }   
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   // get rid of the original point list
   pl.clear();
   pl = check.get_validated();
   for(pointlist::iterator PL = pl.begin(); PL != pl.end(); PL++)
      (*PL) *= _target.rARTM();
   return actlay->addwire(pl,w);
}

laydata::tdtdata* laydata::tdtdesign::addtext(word la, std::string& text, CTM& ori) {
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   ori *= _target.rARTM();
   return actlay->addtext(text,ori);
}
 
laydata::tdtdata* laydata::tdtdesign::addcellref(std::string& name, CTM& ori) {
   if (checkcell(name)) {
      laydata::refnamepair striter = getcellnamepair(name);
      modified = true;
      ori *= _target.rARTM();
      tdtdata* ncrf = _target.edit()->addcellref(this, striter, ori);
      if (NULL == ncrf) {
        tell_log(console::MT_ERROR, "Circular reference is forbidden");
      }
      return ncrf;
   }   
   else {
      std::string news = "Cell \"";
      news += name; news += "\" is not defined";
      tell_log(console::MT_ERROR,news.c_str());
      return NULL;
   }
}

laydata::tdtdata* laydata::tdtdesign::addcellaref(std::string& name, CTM& ori, 
                             int4b stepX, int4b stepY, word columns, word rows) {
   if (checkcell(name)) {
      laydata::refnamepair striter = getcellnamepair(name);
      modified = true;
      ori *= _target.rARTM();
      tdtdata* ncrf = _target.edit()->addcellaref(this, striter, ori, 
                                                   stepX, stepY, columns, rows);
      if (NULL == ncrf) {
        tell_log(console::MT_ERROR, "Circular reference is forbidden");
      }
      return ncrf;
   }   
   else {
      std::string news = "Cell \"";
      news += name; news += "\" is not defined";
      tell_log(console::MT_ERROR,news.c_str());
      return NULL;
   }
}

void laydata::tdtdesign::addlist(atticList* nlst) {
   if (_target.edit()->addlist(this, nlst)) {
      // needs validation
      do {} while(validate_cells());
   }   
}

laydata::tdtcell* laydata::tdtdesign::opencell(std::string name) {
   if (_cells.end() != _cells.find(name)) {
      _target.setcell(_cells[name]);
      return _target.edit();
   }   
   else return NULL; // Active cell has not changed if name is not found
}

bool laydata::tdtdesign::editpush(const TP& pnt) {
   if (_target.checkedit()) {// 
      ctmstack transtack;
      transtack.push(CTM());
      laydata::cellrefstack* crstack = new laydata::cellrefstack();
      tdtcell* oldtvcell = _target.view();
      // Find the new active reference
      laydata::tdtcellref *new_activeref =
                      oldtvcell->getcellover(pnt,transtack,crstack);
      if (new_activeref) {
         // Set the new active reference and the chain to it
         _target.push(new_activeref,oldtvcell,crstack,transtack.top());
         return true;
      }
      else
         delete crstack;
   }
   return false;
}

bool laydata::tdtdesign::editprev(const bool undo) {
   return _target.previous(undo);
}

bool laydata::tdtdesign::editpop() {
   return _target.pop();
}

bool laydata::tdtdesign::edittop() {
   return _target.top();
}

laydata::tdtcell* laydata::tdtdesign::checkcell(std::string name) {
   if (_cells.end() != _cells.find(name)) return _cells[name];
   else return NULL;
}   

void laydata::tdtdesign::openGL_draw(const layprop::DrawProperties& drawprop) {
   if (_target.checkedit()) {
      ctmstack transtack;
      transtack.push(CTM());
      _target.view()->openGL_draw(transtack, drawprop, _target.iscell());
   }
}

void laydata::tdtdesign::tmp_draw(const layprop::DrawProperties& drawprop,
                                          TP base, TP newp) {
   ctmqueue tmp_stack;
   if (_tmpdata) {
      glColor4f(0.5, 0.5, 0.5, 0.5);
      tmp_stack.push_front(CTM(newp - base,1,0,false));
      _tmpdata->tmp_draw(drawprop, tmp_stack,NULL,true);
   }
   else if ((drawprop.currentop() != layprop::op_none) && _target.checkedit()) {
      base *= _target.rARTM();
      newp *= _target.rARTM();
      tmp_stack.push_front(CTM(_target.ARTM()));
      tmp_stack.push_front(CTM(newp - base,1,0,false)*_target.ARTM());
      _target.edit()->tmp_draw(drawprop, tmp_stack, true);
      tmp_stack.clear();
   }   
}

void laydata::tdtdesign::write(TEDfile* const tedfile) {
   tedfile->putByte(tedf_DESIGN);
   tedfile->putString(_name);
   tedfile->putReal(_DBU);
   tedfile->putReal(_UU);
   //
   laydata::TDTHierTree* root = _hiertree->GetFirstRoot();
   while (root) {
      _cells[root->GetItem()->name()]->write(tedfile, _cells, root);
      root = root->GetNextRoot();
   }
   tedfile->putByte(tedf_DESIGNEND);
   modified = false;
}

void laydata::tdtdesign::GDSwrite(GDSin::GDSFile& gdsf) {
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_LIBNAME, _name.size());
   wr->add_ascii(_name.c_str()); gdsf.flush(wr);

   wr = gdsf.SetNextRecord(gds_UNITS);
   wr->add_real8b(_UU); wr->add_real8b(_DBU);
   gdsf.flush(wr);
   //
   laydata::TDTHierTree* root = _hiertree->GetFirstRoot();
   while (root) {
      _cells[root->GetItem()->name()]->GDSwrite(gdsf, _cells, root, _UU);
      root = root->GetNextRoot();
   }
   wr = gdsf.SetNextRecord(gds_ENDLIB);gdsf.flush(wr);
}

void laydata::tdtdesign::recreate_hierarchy() {
    // get rid of the hierarchy tree
   TDTHierTree* droot;
   while (_hiertree) {
      droot = _hiertree; _hiertree = droot->GetLast();
      delete droot;
   }
   laydata::cellList::const_iterator wc;
   for (wc = _cells.begin(); wc != _cells.end(); wc++) {
   // here - run the hierarchy extractoin on orphans only   
      if (wc->second->orphan()) _hiertree = wc->second->hierout(_hiertree, NULL, &_cells);
   }   
}      

void laydata::tdtdesign::mouseStart(int input_type) {
   if      ( 0  < input_type)  _tmpdata = new tdtwire(input_type);
   else if ( 0 == input_type)  _tmpdata = new tdtbox();
   else if (-1 == input_type)  _tmpdata = new tdtpoly();
//   else if (-2 == input_type)  _tellop = op_move;
//   else if (-3 == input_type)  _tellop = op_copy;
}

void laydata::tdtdesign::mousePoint(TP p) {
   if (_tmpdata) _tmpdata->addpoint(p);
}

void laydata::tdtdesign::mousePointCancel(TP& lp) {
   if (_tmpdata) _tmpdata->rmpoint(lp);
}

void laydata::tdtdesign::mouseStop() {
   if (_tmpdata) delete _tmpdata;
   _tmpdata = NULL;
}

void laydata::tdtdesign::select_inBox(TP* p1, TP* p2, bool pntsel) {
   if (_target.checkedit()) {
      DBbox select_in((*p1)*_target.rARTM(), (*p2)*_target.rARTM());
      select_in.normalize();
      _target.edit()->select_inBox(select_in, pntsel);
   }
}

laydata::atticList* laydata::tdtdesign::change_select(TP* p1, bool select) {
   if (_target.checkedit()) {
      TP selp = (*p1) * _target.rARTM();
      return _target.edit()->changeselect(selp, select ? sh_selected:sh_active);
   }
   else return NULL;
}

void laydata::tdtdesign::unselect_inBox(TP* p1, TP* p2, bool pntsel) {
   if (_target.checkedit()) {
      DBbox unselect_in((*p1)*_target.rARTM(), (*p2)*_target.rARTM());
      unselect_in.normalize();
      _target.edit()->unselect_inBox(unselect_in, pntsel);
   }
}

void laydata::tdtdesign::copy_selected( TP p1, TP p2) {
   CTM trans;
   p1 *= _target.rARTM();
   p2 *= _target.rARTM();
   int4b dX = p2.x() - p1.x();
   int4b dY = p2.y() - p1.y();
   trans.Translate(dX,dY);
   if (_target.edit()->copy_selected(this, trans)) {
      // needs validation
      do {} while(validate_cells());
   }   
}   

void laydata::tdtdesign::move_selected( TP p1, TP p2, selectList** fadead) {
   CTM trans;
   p1 *= _target.rARTM();
   p2 *= _target.rARTM();
   int4b dX = p2.x() - p1.x();
   int4b dY = p2.y() - p1.y();
   trans.Translate(dX,dY);
   if (_target.edit()->move_selected(this, trans, fadead)) {
      // needs validation
      do {} while(validate_cells());
   }   
}   

void laydata::tdtdesign::rotate_selected( TP p, real angle) {
   // Things to remember...
   // To deal with edit in place, you have to :
   // - get the current translation matrix of the active cell
   // - apply the transformation - in this case rotation around a point
   //   than is done in 3 steps - translate to 0, rotate, translate back
   // - multiply with the reversed CTM to get back in the current 
   //   coordinates
   // If just rotation is applied (second step) operation can not deal 
   // with the cases when the referenced cell is flipped
   CTM trans = _target.ARTM();
   trans.Translate(-p.x(),-p.y());
   trans.Rotate(angle);
   trans.Translate(p.x(),p.y());
   trans *= _target.rARTM();
   if (_target.edit()->transfer_selected(this, trans)) {
      // needs validation
      do {} while(validate_cells());
   }   
}   

void laydata::tdtdesign::flip_selected( TP p, bool Xaxis) {
   // Here - same principle as for rotate_selected 
   // othrwise flip toggles between X and Y depending on the angle
   // of rotation of the active cell reference
   CTM trans = _target.ARTM();
   if (Xaxis)  trans.FlipX(p.y());
   else        trans.FlipY(p.x());
   trans *= _target.rARTM();
   if (_target.edit()->transfer_selected(this, trans)) {
      // needs validation
      do {} while(validate_cells());
   }   
}   

void laydata::tdtdesign::delete_selected(laydata::atticList* fsel) {
   if (_target.edit()->delete_selected(this, fsel)) {
      // needs validation
      do {} while(validate_cells());
   }
}   

void laydata::tdtdesign::destroy_this(tdtdata* ds, word la) {
   if (_target.edit()->destroy_this(this, ds,la)) {
      // needs validation
      do {} while(validate_cells());
   }   
}   

bool laydata::tdtdesign::group_selected(std::string name) {
   // first check that the cell with this name does not exist already
   if (_cells.end() != _cells.find(name)) {
      tell_log(console::MT_ERROR, "Cell with this name already exists. Group impossible");
      return false;
   }
   //unlink the fully selected shapes from the quadTree of the current cell
   atticList* TBgroup = _target.edit()->groupPrep(this);
   if (TBgroup->empty()) {
      tell_log(console::MT_WARNING, "Nothing to group");
      delete TBgroup; return false;
   }
   //Create a new cell
   tdtcell* newcell = addcell(name);
   assert(newcell);
   //Get the selected shapes from the current cell and add them to the new cell
   for(atticList::const_iterator CL = TBgroup->begin(); 
                                                   CL != TBgroup->end(); CL++) {
      shapeList* lslct = CL->second;
      quadTree* wl = newcell->securelayer(CL->first);
      for(shapeList::const_iterator CI = lslct->begin(); 
                                                     CI != lslct->end(); CI++) {
         wl->put(*CI);
         if (0 == CL->first) newcell->addchild(this,
                                    static_cast<tdtcellref*>(*CI)->structure());
      }                  
      lslct->clear();   
   }
   TBgroup->clear();delete TBgroup;
   newcell->resort();
   //reference the new cell into the current one.
   tdtdata* ref = _target.edit()->addcellref(this, 
                                    getcellnamepair(name),CTM(TP(0,0),1,0,false));
   //select the new cell
   ref->set_status(sh_selected);
   _target.edit()->select_this(ref,0);
   //Just for the records. No shapes are moved to the Attic during this operation
   //Undo is possible simply by ungrouping the new cell
   return true;
}

laydata::shapeList* laydata::tdtdesign::ungroup_prep() {
   //unlink the selected ref/aref's from the quadTree of the current cell
   return _target.edit()->ungroupPrep(this);
}

laydata::atticList* laydata::tdtdesign::ungroup_this(laydata::shapeList* cells4u) {
   laydata::atticList* shapeUngr = new laydata::atticList();
   for (shapeList::const_iterator CC = cells4u->begin(); 
                                                     CC != cells4u->end(); CC++)
      static_cast<tdtcellref*>(*CC)->ungroup(this, _target.edit(), shapeUngr);      
   _target.edit()->validate_layers();
   return shapeUngr;
}

unsigned int laydata::tdtdesign::numselected() {
   if (_target.checkedit()) return _target.edit()->numselected();
   else return 0;
}   

DBbox laydata::tdtdesign::activeoverlap() {
   return _target.overlap();
//   if (_target.checkedit())
//      return _target.edit()->overlap() * _ARTM;
//   else return DEFAULT_OVL_BOX;
}      

bool laydata::tdtdesign::validate_cells() {
   bool invalidParents = false;
   laydata::cellList::const_iterator wc;
   for (wc = _cells.begin(); wc != _cells.end(); wc++)
      invalidParents |= wc->second->validate_cells(this);
   return invalidParents;
}

void laydata::tdtdesign::check_active() {
   if (NULL == _target.edit()) throw EXPTNactive_cell();
};

bool laydata::tdtdesign::collect_usedlays(std::string cellname, bool recursive, usedlayList& laylist) const {
   tdtcell* targetcell;
   if ("" == cellname) targetcell = _target.edit();
   else                targetcell = getcellnamepair(cellname)->second;
   if (NULL != targetcell) {
      targetcell->collect_usedlays(this, recursive, laylist);
      laylist.sort();
      laylist.unique();
      std::ostringstream ost;
      ost << "used layers: {";
      for(usedlayList::const_iterator CL = laylist.begin() ; CL != laylist.end();CL++ )
         ost << " " << *CL << " ";
      ost << "}";
      tell_log(console::MT_INFO, ost.str().c_str());
      return true;
   }
   else return false;
}

laydata::tdtdesign::~tdtdesign() {
   //clean-up the _target stack
   for( editcellstack::iterator CECS = _target._editstack.begin();
                                CECS != _target._editstack.end(); CECS++)
      delete (*CECS);
   // get rid of the hierarchy tree
   TDTHierTree* droot;
   while (_hiertree) {
      droot = _hiertree; _hiertree = droot->GetLast();
      delete droot;
   }
   // now delete the cells
   laydata::cellList::const_iterator wc;
   for (wc = _cells.begin(); wc != _cells.end(); wc++)
      delete wc->second;
   _cells.clear();
}
