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
//        Created: Sun Apr 18 10:59:37 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: The top class in the layout database
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
laydata::editcellstack      laydata::editobject::_editstack;
layprop::ViewProperties*    laydata::editobject::_viewprop = NULL;

//-----------------------------------------------------------------------------
// class tdtdesign
//-----------------------------------------------------------------------------
laydata::tdtdesign::tdtdesign(std::string name, time_t created,
                              time_t lastUpdated, real DBU, real UU) {
   _name = name;
   _DBU  = DBU; _UU   = UU;
   _tmpdata = NULL;
   modified = false;
   _hiertree = NULL;
   _created = created;
   _lastUpdated = lastUpdated;
}

void laydata::tdtdesign::read(TEDfile* const tedfile) 
{
   std::string cellname;
   while (tedf_CELL == tedfile->getByte()) 
   {
      cellname = tedfile->getString();
      tell_log(console::MT_CELLNAME, cellname);
      tedfile->registercellread(cellname, new tdtcell(tedfile, cellname));
   }
   recreate_hierarchy();
   _tmpdata = NULL;
   modified = false;
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

bool laydata::tdtdesign::removecell(std::string& name, laydata::atticList* fsel)
{
   if (_cells.end() == _cells.find(name))
   {
      std::string news = "Cell \"";
      news += name; news += "\" doesn't exists";
      tell_log(console::MT_ERROR,news);
      return false; // cell doesn't exists
   }
   else
   {
      TDTHierTree* ccl = _hiertree->GetMember(_cells[name]);
      if (ccl->Getparent())
      {
         std::string news = "Cell \"";
         news += name; news += "\" is referenced and can't be removed";
         tell_log(console::MT_ERROR,news);
         return false;
      }
      else if (_target.edit()->name() == name)
      {
         tell_log(console::MT_ERROR,"Active cell can't be removed");
         return false;
      }
      else
      {
         modified = true;
         // get the cell by name
         tdtcell* remcl = _cells[name];
         // update the _hiertree
         remcl->removePrep(this);
         // remove the cell from the list of all design cells
         _cells.erase(_cells.find(name));
         //empty the contents of the removed cell and return it in atticList
         remcl->full_select();
         remcl->delete_selected(this, fsel); // validation is not required here
         // finally - delete the cell. Cell is already empty
         delete remcl;
         return true;
      }
   }
}

laydata::tdtdata* laydata::tdtdesign::addbox(word la, TP* p1, TP* p2)
{
   DBbox old_overlap = _target.edit()->overlap();
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   (*p1) *= _target.rARTM();
   (*p2) *= _target.rARTM();
   laydata::tdtdata* newshape = actlay->addbox(p1,p2);
   if (_target.edit()->overlapChanged(old_overlap, this))
      do {} while(validate_cells());
   return newshape;
}

laydata::tdtdata* laydata::tdtdesign::addpoly(word la, const pointlist* pl) {
   laydata::valid_poly check(*pl);
   if (!check.valid()) {
      std::ostringstream ost; 
      ost << "Polygon check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
      return NULL;
   }
   laydata::tdtdata* newshape;
   DBbox old_overlap = _target.edit()->overlap();
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   pointlist vpl = check.get_validated();
   if (check.box()) {
      TP* p1= new TP(vpl[0] *_target.rARTM());
      TP* p2= new TP(vpl[2] *_target.rARTM());
      newshape = actlay->addbox(p1,p2);
   }
   for(pointlist::iterator PL = vpl.begin(); PL != vpl.end(); PL++)
      (*PL) *= _target.rARTM();
   newshape = actlay->addpoly(vpl);
   if (_target.edit()->overlapChanged(old_overlap, this))
      do {} while(validate_cells());
   return newshape;
}

laydata::tdtdata* laydata::tdtdesign::addwire(word la, const pointlist* pl, word w) {
   laydata::valid_wire check(*pl,w);
   if (!check.valid()) {
      std::ostringstream ost; 
      ost << "Wire check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
      return NULL;
   }   
   DBbox old_overlap = _target.edit()->overlap();
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   pointlist vpl = check.get_validated();
   for(pointlist::iterator PL = vpl.begin(); PL != vpl.end(); PL++)
      (*PL) *= _target.rARTM();
   laydata::tdtdata* newshape = actlay->addwire(vpl,w);
   if (_target.edit()->overlapChanged(old_overlap, this))
      do {} while(validate_cells());
   return newshape;
}

laydata::tdtdata* laydata::tdtdesign::addtext(word la, std::string& text, CTM& ori) {
   DBbox old_overlap = _target.edit()->overlap();
   tdtlayer *actlay = static_cast<tdtlayer*>(targetlayer(la));
   modified = true;
   ori *= _target.rARTM();
   laydata::tdtdata* newshape = actlay->addtext(text,ori);
   if (_target.edit()->overlapChanged(old_overlap, this))
      do {} while(validate_cells());
   return newshape;
}
 
laydata::tdtdata* laydata::tdtdesign::addcellref(std::string& name, CTM& ori) {
   if (checkcell(name)) {
      laydata::refnamepair striter = getcellnamepair(name);
      modified = true;
      ori *= _target.rARTM();
      DBbox old_overlap = _target.edit()->overlap();
      tdtdata* ncrf = _target.edit()->addcellref(this, striter, ori);
      if (NULL == ncrf) {
        tell_log(console::MT_ERROR, "Circular reference is forbidden");
      }
      else
      {
         if (_target.edit()->overlapChanged(old_overlap, this))
            do {} while(validate_cells());
      }
      return ncrf;
   }
   else {
      std::string news = "Cell \"";
      news += name; news += "\" is not defined";
      tell_log(console::MT_ERROR,news);
      return NULL;
   }
}

laydata::tdtdata* laydata::tdtdesign::addcellaref(std::string& name, CTM& ori, 
                             int4b stepX, int4b stepY, word columns, word rows) {
   if (checkcell(name)) {
      laydata::refnamepair striter = getcellnamepair(name);
      modified = true;
      ori *= _target.rARTM();
      DBbox old_overlap = _target.edit()->overlap();
      tdtdata* ncrf = _target.edit()->addcellaref(this, striter, ori, 
                                                   stepX, stepY, columns, rows);
      if (NULL == ncrf) {
        tell_log(console::MT_ERROR, "Circular reference is forbidden");
      }
      else
      {
         if (_target.edit()->overlapChanged(old_overlap, this))
            do {} while(validate_cells());
      }
      return ncrf;
   }   
   else {
      std::string news = "Cell \"";
      news += name; news += "\" is not defined";
      tell_log(console::MT_ERROR,news);
      return NULL;
   }
}

void laydata::tdtdesign::addlist(atticList* nlst)
{
   if (_target.edit()->addlist(this, nlst))
   {
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
                      oldtvcell->getcellover(pnt,transtack,crstack,_target.viewprop());
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

void laydata::tdtdesign::openGL_draw(layprop::DrawProperties& drawprop) {
   if (_target.checkedit())
   {
      ctmstack transtack;
      drawprop.initCTMstack();
      _target.view()->openGL_draw(drawprop, _target.iscell());
      drawprop.clearCTMstack();
   }
}

void laydata::tdtdesign::tmp_draw(const layprop::DrawProperties& drawprop,
                                          TP base, TP newp) {
   ctmqueue tmp_stack;
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   if (_tmpdata)
   {
      glColor4f(1.0, 1.0, 1.0, 0.7);
      tmp_stack.push_front(CTM(newp - base,1,0,false));
      _tmpdata->tmp_draw(drawprop, tmp_stack,NULL,true);
   }
   else if ((drawprop.currentop() != console::op_none) && _target.checkedit())
   {
      if ((console::op_copy == drawprop.currentop()) || (console::op_move == drawprop.currentop()))
      {
         base *= _target.rARTM();
         newp *= _target.rARTM();
         tmp_stack.push_front(CTM(_target.ARTM()));
         tmp_stack.push_front(CTM(newp - base,1,0,false)*_target.ARTM());
      }
      else if ((console::op_flipX == drawprop.currentop()) || (console::op_flipY == drawprop.currentop()))
      {
         CTM newpos = _target.ARTM();
//         tmp_stack.push_front(newpos);
         if (console::op_flipX == drawprop.currentop())
            newpos.FlipX(newp.y());
         else
            newpos.FlipY(newp.x());
         tmp_stack.push_front(newpos * _target.rARTM());
      }
      else if (console::op_rotate == drawprop.currentop())
      {
         CTM newpos = _target.ARTM();
         tmp_stack.push_front(_target.ARTM());
         newpos.Translate(-newp.x(),-newp.y());
         newpos *= _tmpctm;
         newpos.Translate(newp.x(),newp.y());
         tmp_stack.push_front(newpos);
      }
      _target.edit()->tmp_draw(drawprop, tmp_stack, true);
      tmp_stack.clear();
   }
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

void laydata::tdtdesign::GDSwrite(GDSin::GDSFile& gdsf, tdtcell* top, bool recur)
{
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_LIBNAME, _name.size());
   wr->add_ascii(_name.c_str()); gdsf.flush(wr);

   wr = gdsf.SetNextRecord(gds_UNITS);
   wr->add_real8b(_UU); wr->add_real8b(_DBU);
   gdsf.flush(wr);
   //
   if (NULL == top)
   {
      laydata::TDTHierTree* root = _hiertree->GetFirstRoot();
      while (root) {
         _cells[root->GetItem()->name()]->GDSwrite(gdsf, _cells, root, _UU, recur);
         root = root->GetNextRoot();
      }
   }
   else
   {
      laydata::TDTHierTree* root_cell = _hiertree->GetMember(top);
      top->GDSwrite(gdsf, _cells, root_cell, _UU, recur);
   }
   wr = gdsf.SetNextRecord(gds_ENDLIB);gdsf.flush(wr);
}

void laydata::tdtdesign::PSwrite(PSFile& psf, const tdtcell* top, const layprop::DrawProperties& drawprop)
{
   //
   laydata::TDTHierTree* root_cell = _hiertree->GetMember(top);
   if (psf.hier())
   {
      top->PSwrite(psf, drawprop, &_cells, root_cell);
      psf.pspage_header(top->overlap());
      psf.pspage_footer(top->name());
   }
   else
   {
      psf.pspage_header(top->overlap());
      top->PSwrite(psf, drawprop, &_cells, root_cell);
      psf.pspage_footer(top->name());
   }
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

void laydata::tdtdesign::mouseStart(int input_type, std::string name, const CTM trans,
                                   int4b stepX, int4b stepY, word cols, word rows)
{
   if      ( 0  < input_type)  _tmpdata = new tdtwire(input_type);
   else if ( console::op_dbox  == input_type)  _tmpdata = new tdtbox();
   else if ( console::op_dpoly == input_type)  _tmpdata = new tdtpoly();
   else if ( console::op_cbind  == input_type)
   {
      assert ("" != name);
      laydata::refnamepair striter = getcellnamepair(name);
      CTM eqm;
      _tmpdata = new tdtcellref(striter, eqm);
   }
   else if ( console::op_abind  == input_type)
   {
      assert ("" != name);
      assert(0 != cols);assert(0 != rows);assert(0 != stepX);assert(0 != stepY);
      laydata::refnamepair striter = getcellnamepair(name);
      CTM eqm;
      _tmpdata = new tdtcellaref(striter, eqm, stepX, stepY, cols, rows);
   }
   else if ( console::op_tbind == input_type)
   {
      assert ("" != name);
      CTM eqm(trans);
      eqm.Scale(1/(_UU*OPENGL_FONT_UNIT), 1/(_UU*OPENGL_FONT_UNIT));
      _tmpdata = new tdttext(name, eqm);
   }
   else if ( console::op_rotate == input_type)
   {
      _tmpctm = trans;
   }
}

void laydata::tdtdesign::mousePoint(TP p)
{
   if (_tmpdata) _tmpdata->addpoint(p);
}

void laydata::tdtdesign::mousePointCancel(TP& lp)
{
   if (_tmpdata) _tmpdata->rmpoint(lp);
}

void laydata::tdtdesign::mouseStop()
{
   if (_tmpdata) delete _tmpdata;
   _tmpdata = NULL;
}

void laydata::tdtdesign::mouseFlip()
{
   if (_tmpdata) _tmpdata->objFlip();
}

void laydata::tdtdesign::mouseRotate()
{
   if (_tmpdata) _tmpdata->objRotate();
}

void laydata::tdtdesign::select_inBox(TP* p1, TP* p2, bool pntsel)
{
   if (_target.checkedit())
   {
      DBbox select_in((*p1)*_target.rARTM(), (*p2)*_target.rARTM());
      select_in.normalize();
      _target.edit()->select_inBox(select_in, _target.viewprop(), pntsel);
   }
}

laydata::atticList* laydata::tdtdesign::change_select(TP* p1, bool select) {
   if (_target.checkedit()) {
      TP selp = (*p1) * _target.rARTM();
      return _target.edit()->changeselect(selp, select ? sh_selected:sh_active, _target.viewprop());
   }
   else return NULL;
}

void laydata::tdtdesign::unselect_inBox(TP* p1, TP* p2, bool pntsel) {
   if (_target.checkedit()) {
      DBbox unselect_in((*p1)*_target.rARTM(), (*p2)*_target.rARTM());
      unselect_in.normalize();
      _target.edit()->unselect_inBox(unselect_in, pntsel, _target.viewprop());
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

void laydata::tdtdesign::move_selected( TP p1, TP p2, selectList** fadead)
{
   CTM trans;
   p1 *= _target.rARTM();
   p2 *= _target.rARTM();
   int4b dX = p2.x() - p1.x();
   int4b dY = p2.y() - p1.y();
   trans.Translate(dX,dY);
   if (_target.edit()->move_selected(this, trans, fadead))
      // needs validation
      do {} while(validate_cells());
}   

bool laydata::tdtdesign::cutpoly(pointlist& pl, atticList** dasao)
{
   for (pointlist::iterator CP = pl.begin(); CP != pl.end(); CP ++)
      (*CP) *= _target.rARTM();
   return _target.edit()->cutpoly_selected(pl,dasao); 
}

void laydata::tdtdesign::rotate_selected( TP p, real angle, selectList** fadead)
{
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
   if (_target.edit()->rotate_selected(this, trans, fadead))
//   if (_target.edit()->transfer_selected(this, trans))
      // needs validation
      do {} while(validate_cells());
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

bool laydata::tdtdesign::checkValidRef(std::string newref)
{
   if ( _cells.end() == _cells.find(newref) )
   {
      std::string news = "Cell \"";
      news += newref; news += "\" is not defined";
      tell_log(console::MT_ERROR,news);
      return false;
   }
   laydata::tdtcell* child = _cells[newref];
   if (_hiertree->checkAncestors(_target.edit(), child, _hiertree))
   {
      tell_log(console::MT_ERROR, "Circular reference is forbidden.");
      return false;
   }
   return true;
}

laydata::atticList* laydata::tdtdesign::changeref(shapeList* cells4u, std::string newref)
{
   assert(checkcell(newref));
   assert((!cells4u->empty()));
   laydata::shapeList* cellsUngr = new laydata::shapeList();
   laydata::refnamepair striter = getcellnamepair(newref);
   DBbox old_overlap = _target.edit()->overlap();

   for (shapeList::const_iterator CC = cells4u->begin(); CC != cells4u->end(); CC++)
   {
      CTM ori = static_cast<tdtcellref*>(*CC)->translation();
      tdtdata* ncrf = _target.edit()->addcellref(this, striter, ori);
      assert(NULL != ncrf);
      ncrf->set_status(sh_selected);
      _target.edit()->select_this(ncrf,0);
      cellsUngr->push_back(ncrf);
   }
   laydata::atticList* shapeUngr = new laydata::atticList();
   (*shapeUngr)[0] = cellsUngr;
   if (_target.edit()->overlapChanged(old_overlap, this))
      do {} while(validate_cells());
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
      tell_log(console::MT_INFO, ost.str());
      return true;
   }
   else return false;
}

laydata::tdtdesign::~tdtdesign() {
   //clean-up the _target stack
   for( editcellstack::iterator CECS = _target._editstack.begin();
                                CECS != _target._editstack.end(); CECS++)
      delete (*CECS);
   _target._editstack.clear();
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
