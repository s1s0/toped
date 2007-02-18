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
//        Created: Sun Apr 18 11:35:12 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Layout cell handling
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <iostream>
#include <sstream>
#include <iostream>
#include "tedcell.h"
#include "tedat.h"
#include "viewprop.h"
#include "tedesign.h"
#include "../tpd_common/tedop.h"
#include "../tpd_common/outbox.h"
#include "../tpd_common/glf.h"

void laydata::editobject::unblockfill()
{
   _viewprop->drawprop().unblockfill();
}

void laydata::editobject::blockfill()
{
   _viewprop->drawprop().blockfill(_peditchain);
}

laydata::editobject::editobject()
{
   _activecell = NULL;   // the curently active cell
   _viewcell = NULL;     // current topview cell - if edit in place is active
   _activeref = NULL;    // current topview reference - " ------" ------"-----
   _peditchain= NULL;
}

laydata::editobject::editobject(tdtcellref* cref, tdtcell* vcell, cellrefstack* crs, const CTM& trans)
{
   _activeref = cref;
   _viewcell = vcell;
   if (_activeref) _activecell = _activeref->structure();
   else            _activecell = vcell;
   _peditchain = crs;
   _ARTM = trans;
}

DBbox laydata::editobject::overlap() const{
   if (_activecell)
      return _activecell->overlap() * _ARTM;
   else return DEFAULT_OVL_BOX;
}

void laydata::editobject::reset() {
   if (_activecell) {
      _activecell->unselect_all();
      _editstack.push_front(new editobject(_activeref, _viewcell, _peditchain, _ARTM));
   }
   if (_activeref ) unblockfill();
   _peditchain = NULL;
   _activecell = NULL;
   _viewcell = NULL;
   _activeref = NULL;
   _ARTM = CTM();
}

void laydata::editobject::setcell(tdtcell* cell) {
   reset();
   _activecell = cell; _viewcell = cell;
}

void laydata::editobject::push(tdtcellref* cref, tdtcell* vref, cellrefstack* crs, CTM trans) {
   assert(cref);
   reset(); // Unset previous active reference if it exists
   _activeref = cref;
   _activecell = _activeref->structure();
   _viewcell = vref;
   _peditchain = crs;
   _ARTM = trans;
   blockfill();
}

bool laydata::editobject::pop() {
   if ((NULL == _activeref) || (!_peditchain) || (0 == _peditchain->size())) return false;
   if (_activecell) _activecell->unselect_all();
   //prepare current for pushing in the _editstack
   editobject* pres = new editobject(_activeref, _viewcell, _peditchain, _ARTM);
   if (1 == _peditchain->size()) {
      // if _activecell is one level only under the _viewcell - edit in place will not
      // be active anymore
      _activecell = _viewcell;
      _peditchain = NULL;
      _activeref = NULL;
      _ARTM = CTM();
      unblockfill();
   }
   else {
      // copy the _peditchain
      _peditchain = new cellrefstack(*_peditchain);
      // pop the last reference (that use to be current) out of the stack
      _peditchain->pop_back();
      // recalculate the translation matrix - code below might seem weird. This is just because
      // it is! The name of the matrix is Active Reference translation matrix, and it seems
      // to be a good name. So the matrix has to be calculated in reverse order, because the
      // bottom of the _peditchain contains the active reference (the top contains the top view)
      // so we must go bottom-up, instead of the usual (and wrong in this case) top-down
      // That is the why _ARTM is calculated by getcellover method initially, but it is not
      // that obvious there because of the recurse
      _ARTM = CTM();
      for( cellrefstack::reverse_iterator CCR = _peditchain->rbegin(); CCR != _peditchain->rend(); CCR++) 
         _ARTM *= (*CCR)->translation();
      // get a pointer to the active reference...
      _activeref = const_cast<tdtcellref*>(_peditchain->back());
      // ... and active cell
      _activecell = _activeref->structure();
      // 
      blockfill();
   }
   _editstack.push_front(pres);
   return true;
}

bool laydata::editobject::top() {
   if (NULL == _activeref) return false;
   if (_activecell) _activecell->unselect_all();
   if (_activeref ) unblockfill();
   _editstack.push_front(new editobject(_activeref, _viewcell, _peditchain, _ARTM));
   _activecell = _viewcell;
   _peditchain = NULL;
   _activeref = NULL;
   _ARTM = CTM();
   unblockfill();
   return true;
}

bool laydata::editobject::previous(const bool undo) {
   if (0 == _editstack.size()) return false;
   if (_activecell) _activecell->unselect_all();
   if (_activeref ) unblockfill();
   editobject* pres = NULL;
   if (!undo)
      pres = new editobject(_activeref, _viewcell, _peditchain, _ARTM);
   editobject* prev = _editstack.front();
   _activeref = prev->_activeref;
   _activecell = prev->_activecell;
   cellrefstack* peditchain= prev->_peditchain;
   if (peditchain==NULL)
   {
      _peditchain = new cellrefstack;
   }
   else
   _peditchain = new cellrefstack(*peditchain);
   //_peditchain = new cellrefstack(*(prev->_peditchain));
   _viewcell = prev->_viewcell;
   _ARTM = prev->_ARTM;
   if (undo) {
      _editstack.pop_front();
      delete prev;
   }
   else   
      _editstack.push_front(pres);
   if (_activeref ) blockfill();
   return true;
}

std::string laydata::editobject::name() const {
   if (_activecell) return _activecell->name();
   else return std::string("");
}

laydata::editobject::~editobject() {
   if (_peditchain) delete _peditchain;
}
//-----------------------------------------------------------------------------
// class tdtcell                       
//-----------------------------------------------------------------------------
laydata::tdtcell::tdtcell(std::string name) {
   _name = name; _orphan = true;
};

laydata::tdtcell::tdtcell(TEDfile* const tedfile, std::string name) : _name(name) ,
                                                               _orphan(true) 
{
   byte recordtype;
   word  layno;
   // now get the layers
   while (tedf_CELLEND != (recordtype = tedfile->getByte())) 
   {
      switch (recordtype) 
      {
         case    tedf_LAYER: 
            layno = tedfile->getWord();
            if (0 != layno)  _layers[layno] = new tdtlayer(tedfile);
            else             _layers[layno] = new quadTree(tedfile); 
            if (0 == layno) tedfile->get_cellchildnames(&_children);
            break;
         default: throw EXPTNreadTDT("LAYER record type expected");
      }
   }
}

laydata::quadTree* laydata::tdtcell::securelayer(word layno) 
{
   if (_layers.end() == _layers.find(layno)) 
   {
      if (0 != layno)  _layers[layno] = new tdtlayer();
      else             _layers[layno] = new quadTree(); 
   }
   return _layers[layno];
}

laydata::tdtcellref* laydata::tdtcell::addcellref(laydata::tdtdesign* ATDB,
                                 refnamepair str, CTM trans, bool sortnow)
{
   if (!addchild(ATDB, str->second)) return NULL;
   quadTree *cellreflayer = securelayer(0);
   laydata::tdtcellref* cellref = new tdtcellref(str, trans);
   if (sortnow) cellreflayer->add(cellref);
   else         cellreflayer->put(cellref);
   return cellref;
}

laydata::tdtcellaref* laydata::tdtcell::addcellaref(laydata::tdtdesign* ATDB,
          refnamepair str, CTM trans, ArrayProperties& arrprops, bool sortnow)
{
   if (!addchild(ATDB, str->second)) return NULL;
   quadTree *cellreflayer = securelayer(0);
   laydata::tdtcellaref* cellaref =
                       new tdtcellaref(str, trans, arrprops);
   if (sortnow) cellreflayer->add(cellaref);
   else         cellreflayer->put(cellaref);
   return cellaref;
}

bool laydata::tdtcell::addchild(laydata::tdtdesign* ATDB, tdtcell* child) {
  // check for circular reference, i.e. the child is a father of some of its ancestors
  if (ATDB->_hiertree->checkAncestors(this, child, ATDB->_hiertree)) {
    //Circular reference found. child is already an ancestor of this
    return false;
  }
  //leave a mark that child is not orphan
  child->parentfound();
  // update the list of children of the current cell
   _children.push_back(child->name());
   _children.sort();
   _children.unique();
   // update the hierarchical tree
   int res = ATDB->_hiertree->addParent(child, this, ATDB->_hiertree);
   if (res > 0)
      ATDB->btreeAddMember(child->name().c_str(), name().c_str(), res);
   return true;
}

void laydata::tdtcell::openGL_draw(layprop::DrawProperties& drawprop, bool active) const {
   // Draw figures
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++) {
//      if (0 < lay->first)
      word curlayno = lay->first;
      if (!drawprop.layerHidden(curlayno))
         const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(curlayno);
      else continue;
      // fancy like this (dlist iterator) , besause a simple
      // _shapesel[curlayno] complains about loosing qualifiers (const)
      selectList::const_iterator dlst;
      bool fill = drawprop.getCurrentFill();
//      if (fill) glfEnable(GLF_FILLING);
//      else      glfDisable(GLF_FILLING);
      if ((active) && (_shapesel.end() != (dlst = _shapesel.find(curlayno))))
         lay->second->openGL_draw(drawprop,dlst->second, fill);
      else
         lay->second->openGL_draw(drawprop, NULL, fill);
   }
}

void laydata::tdtcell::tmp_draw(const layprop::DrawProperties& drawprop,
                                          ctmqueue& transtack, bool active) const {
   if (active) {
      // If this is the active cell, then we will have to visualize the
      // selected shapes in move. Patially selected fellas are processed
      // only if the current operation is move
      console::ACTIVE_OP actop = drawprop.currentop();
      //temporary draw of the active cell - moving selected shapes
      selectList::const_iterator llst;
      dataList::const_iterator dlst;
      for (llst = _shapesel.begin(); llst != _shapesel.end(); llst++) {
         const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(llst->first);
         for (dlst = llst->second->begin(); dlst != llst->second->end(); dlst++)
            if (!((actop == console::op_copy) && (sh_partsel == dlst->first->status())))
               dlst->first->tmp_draw(drawprop, transtack, dlst->second);
      }
   }
   else {
      // Here we draw obviously a cell which reference has been selected
      // somewhere up the hierarchy. On this level - no selected shapes
      // whatsoever exists, so just perform a regular draw, but of course
      // without fill
      typedef layerList::const_iterator LCI;
      for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
         if (!drawprop.layerHidden(lay->first)) {
            const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(lay->first);
            lay->second->tmp_draw(drawprop, transtack);
         }
      transtack.pop_front();
   }
}

bool laydata::tdtcell::getshapeover(TP pnt, layprop::ViewProperties& viewprop) {
   laydata::tdtdata* shape = NULL;
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      if ((0 != lay->first) && viewprop.selectable(lay->first) &&
                               lay->second->getobjectover(pnt,shape))
            return true;
   return false;
}

laydata::atticList* laydata::tdtcell::changeselect(TP pnt, SH_STATUS status, layprop::ViewProperties& viewprop)
{
   laydata::tdtdata* prev = NULL, *shape = NULL;
   word prevlay;
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      if (viewprop.selectable(lay->first)) {
         while (lay->second->getobjectover(pnt,shape)) {
            if ((status != shape->status()) &&
                ((NULL == prev) || (prev->overlap().area() > shape->overlap().area()))) {
                  prev = shape; prevlay = lay->first;
            }
         }
      }
   if (NULL != prev) {
      laydata::atticList* retlist = new atticList();
      laydata::shapeList* atl = new shapeList();
      atl->push_back(prev);
      (*retlist)[prevlay] = atl;
      if (sh_selected == status) {
         if (_shapesel.end() == _shapesel.find(prevlay))
            _shapesel[prevlay] = new dataList();
         _shapesel[prevlay]->push_back(selectDataPair(prev,NULL));
         prev->set_status(status);
         
      }
      else {
            dataList::iterator CI = _shapesel[prevlay]->begin();
            while (_shapesel[prevlay]->end() != CI)
               if (CI->first == prev) {
                  _shapesel[prevlay]->erase(CI);
                  break;
               }
               else CI++;      
            prev->set_status(status);
      }   
      return retlist;
   }
   else return NULL;
}

laydata::tdtcellref* laydata::tdtcell::getcellover(TP pnt, ctmstack& transtack, cellrefstack* refstack, layprop::ViewProperties& viewprop) {
    if (_layers.end() == _layers.find(0)) return NULL;
    laydata::tdtdata *cellobj = NULL;
//    laydata::tdtdata *shapeobj = NULL;
    laydata::tdtcellref *cref = NULL;
    // go truough referenced cells ...
    while (_layers[0]->getobjectover(pnt,cellobj)) {
      //... and get the one that overlaps pnt.
      cref = static_cast<laydata::tdtcellref*>(cellobj);
      TP pntadj = pnt * cref->translation().Reversed();
      // if in the selected reference there are shapes that overlap pnt...
      if (cref->structure()->getshapeover(pntadj, viewprop)) {
         // ... that is what we need ...
         refstack->push_front(cref);
         // save the strack of reference translations
         transtack.push(transtack.top()*cref->translation());
         return cref;
      }   
      // ... otherwise, dive into the hierarchy
      else {
         laydata::tdtcellref *rref = cref->structure()->getcellover(pntadj,transtack,refstack, viewprop);
         if (rref) {
            refstack->push_front(cref);
            // save the strack of reference translations
            transtack.push(transtack.top()*cref->translation());
            return rref;
         }
      }
   }
   return NULL;
}

void laydata::tdtcell::write(TEDfile* const tedfile, const cellList& allcells, TDTHierTree* const root) const {
   // We going to write the cells in hierarchical order. Children - first!
   laydata::TDTHierTree* Child= root->GetChild();
   while (Child) {
//      tedfile->design()->getcellnamepair(Child->GetItem()->name())->second->write(tedfile, Child);
      allcells.find(Child->GetItem()->name())->second->write(tedfile, allcells, Child);
//      allcells[Child->GetItem()->name()]->write(tedfile, Child);
      Child = Child->GetBrother();
	}
   // If no more children and the cell has not been written yet
   if (tedfile->checkcellwritten(_name)) return;
   std::string message = "...writing " + _name;
   tell_log(console::MT_INFO, message);
   tedfile->putByte(tedf_CELL);
   tedfile->putString(_name);
   // and now the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++) {
      tedfile->putByte(tedf_LAYER);
      tedfile->putWord(wl->first);
      wl->second->write(tedfile);
      tedfile->putByte(tedf_LAYEREND);
   }   
   tedfile->putByte(tedf_CELLEND);
   tedfile->registercellwritten(_name);
}

void laydata::tdtcell::GDSwrite(GDSin::GDSFile& gdsf, const cellList& allcells,
                                         TDTHierTree* const root, real UU, bool recur) const
{
   // We going to write the cells in hierarchical order. Children - first!
   if (recur)
   {
      laydata::TDTHierTree* Child= root->GetChild();
      while (Child)
      {
         allcells.find(Child->GetItem()->name())->second->GDSwrite(gdsf, allcells, Child, UU, recur);
         Child = Child->GetBrother();
      }
   }
   // If no more children and the cell has not been written yet
   if (gdsf.checkCellWritten(_name)) return;
   //
   std::string message = "...converting " + _name;
   tell_log(console::MT_INFO, message);
   GDSin::GDSrecord* wr = gdsf.SetNextRecord(gds_BGNSTR);
   gdsf.SetTimes(wr);gdsf.flush(wr);
   wr = gdsf.SetNextRecord(gds_STRNAME, _name.size());
   wr->add_ascii(_name.c_str()); gdsf.flush(wr);
   // and now the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++)
      wl->second->GDSwrite(gdsf, wl->first, UU);
   wr = gdsf.SetNextRecord(gds_ENDSTR);gdsf.flush(wr);
   gdsf.registerCellWritten(_name);
}

void laydata::tdtcell::PSwrite(PSFile& psf, const layprop::DrawProperties& drawprop,
                               const cellList* allcells, TDTHierTree* const root) const
{
   if (psf.hier())
   {
      assert( root );
      assert( allcells );
      // We going to write the cells in hierarchical order. Children - first!
      laydata::TDTHierTree* Child= root->GetChild();
      while (Child)
      {
         allcells->find(Child->GetItem()->name())->second->PSwrite(psf, drawprop, allcells, Child);
         Child = Child->GetBrother();
      }
      // If no more children and the cell has not been written yet
      if (psf.checkCellWritten(_name)) return;
      //
      std::string message = "...converting " + _name;
      tell_log(console::MT_INFO, message);
   }
   psf.cellHeader(_name,overlap());
   // and now the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      word curlayno = wl->first;
      if (!drawprop.layerHidden(curlayno))
      {
         if (0 != curlayno)
            psf.propSet(drawprop.getColorName(curlayno), drawprop.getFillName(curlayno));
         wl->second->PSwrite(psf, drawprop);
      }
   }
   psf.cellFooter();
   if (psf.hier())
      psf.registerCellWritten(_name);
}

laydata::TDTHierTree* laydata::tdtcell::hierout(laydata::TDTHierTree*& Htree, 
                                           tdtcell* parent, cellList* celldefs) {
   // collecting hierarchical information
   Htree = new TDTHierTree(this, parent, Htree);
   nameList::const_iterator wn;
   for (wn = _children.begin(); wn != _children.end(); wn++) 
      (*celldefs)[*wn]->hierout(Htree, this, celldefs); // yahoooo!
   return  Htree;
}   

DBbox laydata::tdtcell::overlap() const {
   DBbox ovlap = DEFAULT_OVL_BOX;
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++) 
      ovlap.overlap(lay->second->overlap());
   return ovlap;
}   

void laydata::tdtcell::select_inBox(DBbox select_in, layprop::ViewProperties& viewprop, bool pntsel) {
   // check that current cell is within 
   if (select_in.cliparea(overlap()) != 0) {
      // Select figures within the active layers
      typedef layerList::const_iterator LCI;
      for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
         if (viewprop.selectable(lay->first)) {
            dataList* ssl;
            if (_shapesel.end() != _shapesel.find(lay->first))  
               ssl = _shapesel[lay->first];
            else
               ssl = new dataList();
/***/       lay->second->select_inBox(select_in, ssl, pntsel);
            if (ssl->empty())  delete ssl; 
            else               _shapesel[lay->first] = ssl; 
         }
   }
}

void laydata::tdtcell::unselect_inBox(DBbox select_in, bool pntsel, layprop::ViewProperties& viewprop)
{
   // check that current cell is within 
   if (select_in.cliparea(overlap()) != 0)
   {
      // Unselect figures within the active layers
      typedef layerList::const_iterator LCI;
      for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      {
         if (viewprop.selectable(lay->first))
         {
            dataList* ssl;
            if (_shapesel.end() != _shapesel.find(lay->first))  {
               ssl = _shapesel[lay->first];
/***/          lay->second->unselect_inBox(select_in, ssl, pntsel);
               if (ssl->empty())
               {
                  delete ssl; 
                  _shapesel.erase(_shapesel.find(lay->first));
               }   
               else _shapesel[lay->first] = ssl; 
            }   
         }
      }
   }
}

void laydata::tdtcell::select_fromList(selectList* slist, layprop::ViewProperties& viewprop) {
   dataList* ssl;
   for (selectList::const_iterator CL = slist->begin(); CL != slist->end(); CL++) {
      // if the layer from the list exists in the layout and is not hidden
      if ((_layers.end() != _layers.find(CL->first)) && viewprop.selectable(CL->first)) {
         if (_shapesel.end() != _shapesel.find(CL->first))  
            ssl = _shapesel[CL->first];
         else                                        
            ssl = new dataList();
         _layers[CL->first]->select_fromList(CL->second, ssl);
         if (ssl->empty())  delete ssl; 
         else              _shapesel[CL->first] = ssl;
      }
      delete CL->second;
   }
   delete slist;   
}

 void laydata::tdtcell::unselect_fromList(selectList* unslist, layprop::ViewProperties& viewprop) {
   dataList* lslct = NULL;
   dataList::iterator CI;
   // for every layer in the unselect list
   for (selectList::const_iterator CUL = unslist->begin();
                                   CUL != unslist->end(); CUL++) {
      // if the corresponding layer in the select list exists and is not locked
      if ((_shapesel.end() != _shapesel.find(CUL->first)) &&
                                       viewprop.selectable(CUL->first)) {
         // start looping every shape to be unselected
         for(dataList::iterator CUI = CUL->second->begin();
                                      CUI != CUL->second->end(); CUI++) {
            bool part_unselect = sh_partsel == CUI->first->status();
            // and check it against the shapes selected
            lslct = _shapesel[CUL->first];
            CI = lslct->begin();
            while (lslct->end() != CI)
               if (CI->first == CUI->first) {
                  // if shapes matched - there are four cases of unselect:
                  // - full select - full unselect
                  // - full select - part unselect
                  // - part select - full unselect
                  // - part select - part unselect
                  if (sh_partsel == CI->first->status())
                     if (part_unselect)  {// part - part
                        if (unselect_pointlist(*CI,*CUI)) lslct->erase(CI);
                     }   
                     else { // part - full   
                        delete CI->second;
                        CI->first->set_status(sh_active);
                        lslct->erase(CI);
                     }   
                  else 
                     if (part_unselect) {// full - part
                        if (unselect_pointlist(*CI,*CUI)) lslct->erase(CI);
                     }   
                     else { // full - full
                        CI->first->set_status(sh_active);
                        lslct->erase(CI);
                     }   
                  break;
               }
               else CI++;
         }
         // at the end, if the container of the selected shapes is empty -
         // delete it
         if (lslct->empty()) {
            delete lslct; _shapesel.erase(_shapesel.find(CUL->first));
         }   
      }
      // take care to clean up the memory from the unselect list
      delete CUL->second;
   }
   delete unslist;
}

bool laydata::tdtcell::copy_selected(laydata::tdtdesign* ATDB, const CTM& trans) {
   DBbox old_overlap = overlap();
   dataList copyList;
   dataList::iterator DI;
   tdtdata *data_copy;
   _dbl_word numshapes;
   selectList::iterator CL = _shapesel.begin(); 
   while (CL != _shapesel.end()) {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes
      if (0 == (numshapes =  getFullySelected(CL->second))) {
         CL++; continue;
      }   
      // Now - Go to copy and sort
      DI = CL->second->begin();
      while (CL->second->end() != DI) {
         // copy only fully selected shapes !
         if (sh_partsel == DI->first->status()) {
            DI++; continue;
         }   
         data_copy = DI->first->copy(trans);
         data_copy->set_status(sh_selected); DI->first->set_status(sh_active);
         if (1 != numshapes)  _layers[CL->first]->put(data_copy);
         else                 _layers[CL->first]->add(data_copy);
         // replace the data into the selected list
         DI = CL->second->erase(DI);
         CL->second->push_front(selectDataPair(data_copy,NULL));
      }
      if (1 != numshapes)  _layers[CL->first]->resort();
      CL++;
   }
   return overlapChanged(old_overlap, ATDB);
};

bool laydata::tdtcell::addlist(laydata::tdtdesign* ATDB, atticList* nlst) {
   DBbox old_overlap = overlap();
   for (atticList::const_iterator CL = nlst->begin();
                                   CL != nlst->end(); CL++) {
      // secure the target layer
      quadTree* wl = securelayer(CL->first);
      for (shapeList::const_iterator DI = CL->second->begin();
                                    DI != CL->second->end(); DI++) {
         // add it to the corresponding layer
         (*DI)->set_status(sh_active);
         wl->put(*DI);
         //update the hierarchy tree if this is a cell
         if (0 == CL->first) addchild(ATDB, 
                            static_cast<tdtcellref*>(*DI)->structure());
      }
      wl->invalidate();
   }
   delete nlst;
   validate_layers(); // because put is used
   return overlapChanged(old_overlap, ATDB);
}

laydata::dataList* laydata::tdtcell::secure_dataList(selectList& slst, word layno) {
   dataList* ssl;
   if (slst.end() != slst.find(layno)) ssl = slst[layno];
   else {
      ssl = new dataList();
      slst[layno] = ssl;
   }
   return ssl;
}

laydata::tdtdata* laydata::tdtcell::checkNreplacePoly(selectDataPair& sel, validator* check, 
                                             word layno, selectList** fadead) {
   if (check->valid()) { // shape is valid ...
      if (shp_OK == check->status())  // entirely
         return NULL;
      else {// ... well, maybe not entirely...
         // it is still not clear here why and how the modefied polygon can be
         // in clockwise order. The case with Sergei's gds - metki
         // assert(!(shp_clock & check->status()));
         laydata::tdtdata* newshape = check->replacement();
         // add the new shape to the list of new shapes ...
         secure_dataList(*(fadead[2]),layno)->push_back(selectDataPair(newshape, NULL));
         // ... and put the initial shape(that is to be modified) in the
         // list of deleted shapes
         secure_dataList(*(fadead[1]),layno)->push_back(selectDataPair(sel.first, sel.second));
         return newshape;
      }
   }
   else {// the produced shape is invalid, so keep the old one and add it to the list of failed
      secure_dataList(*(fadead[0]),layno)->push_back(selectDataPair(sel.first, sel.second));
      return NULL;
   }
}

laydata::tdtdata* laydata::tdtcell::checkNreplaceBox(selectDataPair& sel, validator* check,
                                             word layno, selectList** fadead) {
   if (check->valid())
   { // shape is valid, but obviously not a box (if it gets here)
      laydata::tdtdata* newshape = check->replacement();
      // add the new shape to the list of new shapes ...
      secure_dataList(*(fadead[2]),layno)->push_back(selectDataPair(newshape, NULL));
      // ... and put the initial shape(that has been modified) in the
      // list of deleted shapes
      secure_dataList(*(fadead[1]),layno)->push_back(selectDataPair(sel.first, sel.second));
      return newshape;
   }
   else {// the produced shape is invalid, so keep the old one and add it to the list of failed
      secure_dataList(*(fadead[0]),layno)->push_back(selectDataPair(sel.first, sel.second));
      return NULL;
   }
}

bool laydata::tdtcell::move_selected(laydata::tdtdesign* ATDB, const CTM& trans, selectList** fadead)
{
   DBbox old_overlap = overlap();
   validator* checkS = NULL;
   // for every single layer selected
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // before all remove the selected and partially shapes 
      // from the data holders ...
      if (_layers[CL->first]->delete_marked(sh_selected, true)) 
         // ... and validate quadTrees if needed
         if (!_layers[CL->first]->empty()) _layers[CL->first]->validate();
      // now for every single shape...

      dataList* lslct = CL->second;
      dataList::iterator DI = lslct->begin();
      while (DI != lslct->end())
      {
         // .. restore the status byte, of the fully selected shapes, because
         // it was modified to sh_deleted from the quadtree::delete_selected method
         if (sh_partsel != DI->first->status()) DI->first->set_status(sh_selected);
         // ... move it ...
         if (NULL != (checkS = DI->first->move(trans, DI->second)))
         {
            // modify has been performed and a shape needs validation
            laydata::tdtdata *newshape = NULL;
            if (NULL != (newshape = checkNreplacePoly(*DI, checkS, CL->first, fadead)))
            {
               // remove the shape from list of selected shapes and mark it as selected
               DI = lslct->erase(DI);
               _layers[CL->first]->add(newshape);
            }
            else
            {
               _layers[CL->first]->add(DI->first);
               DI++;
            }
         }
         else
         {
            // move has been performed, so just add the shape back to the data holder   
            _layers[CL->first]->add(DI->first);
            DI++;
         }
      }
      _layers[CL->first]->resort();
      if (lslct->empty())
      {
         // at the end, if the container of the selected shapes is empty -
         // Note! _shapesel.erase(CL) will invalidate the iterator which means that
         // it can't be incremened afterwards
         // This on some platforms/compilers/STL implementations or some combinations
         // of the above might work fine sometimes which makes the bug hunt a real fun!
         // (thanks to Sergey)
         delete lslct; 
         selectList::iterator deliter = CL++;
         _shapesel.erase(deliter);
      }
      else CL++;
   }
   return overlapChanged(old_overlap, ATDB);
}

bool laydata::tdtcell::rotate_selected(laydata::tdtdesign* ATDB, const CTM& trans, selectList** fadead)
{
   DBbox old_overlap = overlap();
   validator* checkS = NULL;
   // for every single layer selected
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // before all remove the selected and partially shapes 
      // from the data holders ...
      if (_layers[CL->first]->delete_marked()) 
         // ... and validate quadTrees if needed
         if (!_layers[CL->first]->empty()) _layers[CL->first]->validate();
      // now for every single shape...
      dataList* lslct = CL->second;
      dataList::iterator DI = lslct->begin();
      while (DI != lslct->end())
      {
         // .. restore the status byte, of the fully selected shapes, because
         // it was modified to sh_deleted from the quadtree::delete_selected method
         if (sh_partsel != DI->first->status())
         {
            DI->first->set_status(sh_selected);
            // ... rotate it ...
            if (NULL != (checkS = DI->first->move(trans, DI->second)))
            {// box->polygon conversion has been performed
               laydata::tdtdata *newshape = NULL;
               if (NULL != (newshape = checkNreplaceBox(*DI, checkS, CL->first, fadead)))
               {
                  // remove the shape from list of selected shapes - dont delete the list of
                  // selected points BECAUSE it is (could be) used in UNDO
                  DI = lslct->erase(DI);
                  _layers[CL->first]->add(newshape);
                  newshape->set_status(sh_selected);
               }
               else
               {
                  _layers[CL->first]->add(DI->first);
                  DI++;
               }
            }
            else
            {// no conversion, so just add the shape back to the data holder
               _layers[CL->first]->add(DI->first);
               DI++;
            }
         }
         else DI++;
      }
      _layers[CL->first]->resort();
      if (lslct->empty())
      {
         // at the end, if the container of the selected shapes is empty -
         delete lslct; 
         selectList::iterator deliter = CL++;
         _shapesel.erase(deliter);
      }
      else CL++;
   }
   return overlapChanged(old_overlap, ATDB);
}

bool laydata::tdtcell::transfer_selected(laydata::tdtdesign* ATDB, const CTM& trans) {
   DBbox old_overlap = overlap();
   // for every single layer selected
   for (selectList::const_iterator CL = _shapesel.begin();
                                                  CL != _shapesel.end(); CL++) {
      assert((_layers.end() != _layers.find(CL->first)));
      // before all, remove the selected shapes from the data holders ...
      if (_layers[CL->first]->delete_marked())
         // ... and validate quadTrees if needed
         if (!_layers[CL->first]->empty()) _layers[CL->first]->validate();
      // now for every single shape...
      for (dataList::iterator DI = CL->second->begin();
                                                DI != CL->second->end(); DI++) {
         // .. restore the status byte, of the fully selected shapes, because
         // it is modified to sh_deleted from the quadtree::delete_selected method
         if (sh_partsel != DI->first->status()) {
            DI->first->set_status(sh_selected);
            // ... transfer it ...
            DI->first->transfer(trans);
            // ... and add it back to the data Holder
            _layers[CL->first]->add(DI->first);
         }
      }
      _layers[CL->first]->resort();
   }
   return overlapChanged(old_overlap, ATDB);
}

bool laydata::tdtcell::delete_selected(laydata::tdtdesign* ATDB, laydata::atticList* fsel) {
   DBbox old_overlap = overlap();
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                  CL != _shapesel.end(); CL++) {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes 
      if (0 == getFullySelected(CL->second)) continue;
      if (_layers[CL->first]->delete_marked()) {
         if (_layers[CL->first]->empty()) {
            delete _layers[CL->first]; _layers.erase(_layers.find(CL->first));
         }   
         else _layers[CL->first]->validate();
      }   
   }
   // Now move the selected shapes to the attic. Will be used for undo operations
   if (fsel) store_inAttic(*fsel);
   else      unselect_all(true);   
   updateHierarchy(ATDB);
   DBbox new_overlap = overlap();
   return overlapChanged(old_overlap, ATDB);
}

bool laydata::tdtcell::cutpoly_selected(pointlist& plst, atticList** dasao) {
   // calculate the overlap area of the cutting polygon
   DBbox cut_ovl = DBbox(plst[0]);
   for (word i = 1; i < plst.size(); cut_ovl.overlap(plst[i++]));
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                  CL != _shapesel.end(); CL++) {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes 
      if ((0 == CL->first) || (0 == getFullySelected(CL->second)))  continue;
      // initialize the corresponding 3 shape lists -> one per attic list
      // DElete/CUt/REst/
      shapeList* decure[3];
      byte i;
      for (i = 0; i < 3; decure[i++] = new shapeList());
      // omit the layer if there are no fully selected shapes 
      if (0 == getFullySelected(CL->second)) continue;
      // do the clipping
      _layers[CL->first]->cutpoly_selected(plst, cut_ovl, decure);
      // add the shapelists to the collection, but only if they are not empty
      for (i = 0; i < 3; i++) {
         if (decure[i]->empty()) delete decure[i];
         else (*(dasao[i]))[CL->first] = decure[i];
      }   
   }
   return !dasao[0]->empty();
}

bool laydata::tdtcell::merge_selected(atticList** dasao) {
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                  CL != _shapesel.end(); CL++) {
      assert((_layers.end() != _layers.find(CL->first)));
      shapeList* mrgcand = NULL;
      shapeList* cleared = NULL;
      // omit the layer if there are no fully selected shapes 
      if ((0 == CL->first) || (NULL == (mrgcand = mergeprep(CL->first))))  continue;
      cleared = new shapeList();
      //
      // A rather convoluted traversing to produce all merged shapes ...
      // the while cycle traverses the shapes in the merge candidates list and sends
      // every single one of them to the quadTree::merge_selected() as a
      // reference shape. If the operation succeeds, the loop alters the traversed
      // list so that the current shape and its merged buddy are removed, and
      // the resulting shape is added at the end. 
      // The whole thing is based on the fact (I hope it's not a presumption) that
      // the shape that merges with the current (reference one) is located after
      // the current - so no shapes located in the list before the current are 
      // altered AND all new shapes are added at the end of the list.
      // All this loops until the iterator reach the end of this dynamic list
      //
      tdtdata* merged_shape = NULL;
      shapeList::iterator CS = mrgcand->begin();
      while (CS != mrgcand->end()) {
         tdtdata* ref_shape = *CS;
         if ((merged_shape = _layers[CL->first]->merge_selected(ref_shape))) {
            // if the merge_selected produced non NULL result, it also
            // has changed the value of ref_shape. Now the most disgusting part - 
            // to clear the merged shapes from the shapes tree ...
            _layers[CL->first]->delete_this(*CS);
            _layers[CL->first]->delete_this(ref_shape);
            // and to add them to the list of the deleted shapes...
            cleared->push_back(*CS);
            cleared->push_back(ref_shape);
            // and at the end, to alter the traversed list
            // ATTENTION HERE ! see the note at the top of the loop
            // the remove/erase operations MUST appear in this order, otherwise
            // the CS can get screwed-up
            mrgcand->remove(ref_shape);
            CS = mrgcand->erase(CS);
            // Now add the new shape to the list of selected shapes
            mrgcand->push_back(merged_shape);
            // add the new shape to the layer
            _layers[CL->first]->put(merged_shape);
            // and mark it as sh_merged
            merged_shape->set_status(sh_merged);
            // and finally validate the shapes tree
            _layers[CL->first]->validate();
         }
         else CS++;
      }
      // - first take care about the merge candidates and resulting shapes list
      CS = mrgcand->begin();
      while (CS != mrgcand->end()) {
         // put back everything left to _shapesel ...
         CL->second->push_back(selectDataPair(*CS,NULL));
         if (sh_selected == (*CS)->status()) 
            //... and remove it from the mrgcand list if it is unchanged ...
            CS = mrgcand->erase(CS);
         else {
            // ... or change their status to sh_selected, but leave them in mrgcand list
            (*CS)->set_status(sh_selected);
            CS++;
         }   
      }
      // - second sort the list of merged shapes - 
      CS = cleared->begin();
      while (CS != cleared->end()) {
         if (sh_merged == (*CS)->status()) {
            // this shape appears to be intermediate - it must be destroyed
            delete (*CS);
            CS = cleared->erase(CS);
         }
         else CS++;
      }   
      // finally - assign the list of deleted and resulted shapes to the 
      // input result (dasao) parameter
      if (cleared->empty()) delete cleared;
      else (*(dasao[0]))[CL->first] = cleared;
      if (mrgcand->empty()) delete mrgcand;
      else (*(dasao[1]))[CL->first] = mrgcand;
   }
   return !dasao[0]->empty();
}

bool laydata::tdtcell::destroy_this(laydata::tdtdesign* ATDB, tdtdata* ds, word la) {
   DBbox old_overlap = overlap();
   laydata::quadTree* lay = (_layers.find(la))->second;
   if (!lay) return false;
   // for layer la
   if (lay->delete_this(ds)) {
      if (lay->empty()) {
         delete lay; _layers.erase(_layers.find(la));
      }   
      else lay->validate();
   }
   delete(ds);
   if (0 == la) updateHierarchy(ATDB);
   return overlapChanged(old_overlap, ATDB);
}

void laydata::tdtcell::select_all(layprop::ViewProperties& viewprop)
{
   // This method might be called redundant, because the result of the
   // call select_inBox(overlap(),...) will produce the same results.
   // Several reasons though to have this method:
   // speed
   // ungroup operation requires lock/hide layer properties to be ignored
   
   // the idea behind unselecting all first is that there might be
   // partially selected shapes. In order to make the operation faster
   // and to avoid the call of tdtdata::select_* methods (that deal with this)
   // for every single shape, it seems better to start from a clean list
   unselect_all();
   // Select figures within the active layers
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      if (viewprop.selectable(lay->first))
      {
         dataList* ssl = new dataList();
/***/    lay->second->select_all(ssl);
         assert(!ssl->empty());
         _shapesel[lay->first] = ssl; 
      }
}

void laydata::tdtcell::full_select()
{
   unselect_all();
   // Select figures within the active layers
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      dataList* ssl = new dataList();
/***/ lay->second->select_all(ssl);
      assert(!ssl->empty());
      _shapesel[lay->first] = ssl;
   }
}

void laydata::tdtcell::select_this(tdtdata* dat, word lay) {
   if (_shapesel.end() == _shapesel.find(lay)) _shapesel[lay] = new dataList();
   _shapesel[lay]->push_back(selectDataPair(dat, NULL));
   dat->set_status(sh_selected);
}   

void laydata::tdtcell::unselect_all(bool destroy) {
   dataList* lslct;
   // for every layer with selected shapes
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                 CL != _shapesel.end(); CL++) {
      lslct = CL->second;
      // for every single selectDataPair
      for (dataList::const_iterator CI = lslct->begin(); 
                                                      CI != lslct->end(); CI++) {
         // unmark the shape
         CI->first->set_status(sh_active);
         // clear the list of selected points if it exists
         if (CI->second) 
            delete (CI->second);
         else
            if (destroy) delete (CI->first);
      }
      // clear the selectDataPair structures
      lslct->clear();
      // delete the dataList container
      delete lslct;
   }
   _shapesel.clear();
}

laydata::shapeList* laydata::tdtcell::mergeprep(word layno) {
   selectList::iterator CL = _shapesel.find(layno);
   if (_shapesel.end() == CL) return NULL;
   dataList* lslct = CL->second;
   shapeList* atl = new shapeList();
   
   dataList::iterator CI = lslct->begin();
   while (CI != lslct->end())
      // ... but only if it is marked as sh_deleted
      if (sh_selected == CI->first->status()) {
         atl->push_back(CI->first);
         assert((!CI->second));
         CI = lslct->erase(CI);
      }
      else CI++;
   if (atl->empty()) {
      delete atl;
      atl = NULL;
   }
   // don't erase the select shape container, because it will take the 
   // resulted shapes
   return atl;
}

laydata::atticList* laydata::tdtcell::groupPrep(laydata::tdtdesign* ATDB) {
   atticList* fsel = new atticList();
   dataList *lslct;
   shapeList *atl;
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = CL->second;
      atl = new shapeList();
      // unlink the fully selected selected shapes
      if (_layers[CL->first]->delete_marked()) {
         if (_layers[CL->first]->empty()) {
            delete _layers[CL->first]; _layers.erase(_layers.find(CL->first));
         }   
         else _layers[CL->first]->validate();
      }   
      // now move every single shape in the corresponding fsel layer ...
      dataList::iterator CI = lslct->begin();
      while (CI != lslct->end())
         // ... but only if it is marked as sh_deleted
         if (sh_deleted == CI->first->status()) {
            CI->first->set_status(sh_active);
            atl->push_back(CI->first);
            assert((!CI->second));
            CI = lslct->erase(CI);
         }
         else CI++;
      if (atl->empty()) delete atl; 
      else             (*fsel)[CL->first] = atl;
      if (lslct->empty())  {
         delete lslct; 
         selectList::iterator deliter = CL++;
         _shapesel.erase(deliter);
      }
      else CL++;   
   }
   // Don't invalidate parent cells. The reason is, that the new cell will
   // be refereced in the same place, so the final overlapping box is supposed
   // to remain the same. The quadTrees of the layers must be (and are) validated
   updateHierarchy(ATDB);
   return fsel;
}

laydata::shapeList* laydata::tdtcell::ungroupPrep(laydata::tdtdesign* ATDB) {
   shapeList* csel = new shapeList();
   if (_shapesel.end() != _shapesel.find(0)) {
      // unlink the selected cells
      if (_layers[0]->delete_marked()) {
         if (_layers[0]->empty()) {
            delete _layers[0]; _layers.erase(_layers.find(0));
         }   
         else _layers[0]->validate();
      }   
      // now move every single shape in the corresponding fsel layer ...
      dataList::iterator CI = _shapesel[0]->begin();
      while (CI != _shapesel[0]->end())
         // ... but only if it is marked as sh_deleted
         if (sh_deleted == CI->first->status()) {
            CI->first->set_status(sh_active);
            csel->push_back(CI->first);
            assert((!CI->second));
            CI = _shapesel[0]->erase(CI);
         }
         else CI++;
      if (_shapesel[0]->empty())  {
         delete _shapesel[0]; 
         _shapesel.erase(_shapesel.find(0));
      }
   }
   // Don't invalidate parent cells. The reason is, that in result of the 
   // ungroup operation, shapes will be just regrouped and the final 
   // overlapping box is supposed to remain the same. 
   // The quadTrees of the 0 layer must be (and is) validated
   updateHierarchy(ATDB);
   return csel;
}

void laydata::tdtcell::transferLayer(word dst)
{
   assert(dst);
   quadTree *dstlay = securelayer(dst);
   dataList* transfered;
   if (_shapesel.end() == _shapesel.find(dst))
      _shapesel[dst] = transfered = new dataList();
   else
      transfered = _shapesel[dst];
   assert(!_shapesel.empty());
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // we're not doing anything if the current layer appers to be dst,
      // i.e. don't mess around if the source and destination layers are the same!
      // The same for the reference layer
      if ((dst != CL->first) && (0 != CL->first))
      {
         // now remove the selected shapes from the data holders ...
         if (_layers[CL->first]->delete_marked())
            // ... and validate quadTrees if needed
            if (!_layers[CL->first]->empty()) _layers[CL->first]->validate();
            else
            {//..or remove the source layer if it remained empty
               delete _layers[CL->first];
               _layers.erase(_layers.find(CL->first));
            }
         // traverse the shapes on this layer and add them to the destination layer
         dataList* lslct = CL->second;
         dataList::iterator DI = lslct->begin();
         while (DI != lslct->end())
         {
            // partially selected are not a subject of this operation, omit them
            if (sh_partsel != DI->first->status())
            {
               // restore the status byte, of the fully selected shapes, because
               // it was modified to sh_deleted from the quadtree::delete_selected method
               DI->first->set_status(sh_selected);
               // add it to the destination layer ...
               dstlay->put(DI->first);
               // because the shape is changing it's layer and the _shapesel structure
               // is based on layer, so we have to remove the structure from the current layer
               // which is lslct and to add it to a new one which will be added to the _shapesel
               // at the end - when we exit the layer loop
               transfered->push_back(*DI);
               DI = lslct->erase(DI);
            }
            else DI++;
         }
         if (lslct->empty())
         {
            // if the container of the selected shapes is empty -
            delete lslct;
            selectList::iterator deliter = CL++;
            _shapesel.erase(deliter);
         }
         else CL++;
      }
      else CL++;
   }
   // finally - validate the destination layer.
   dstlay->validate();
   // For the records:
   // The overall cell overlap shouldn't have been changed,
   // so no need to refresh the overlapping box etc.
}

void laydata::tdtcell::transferLayer(selectList* slst, word dst)
{
   assert(dst);
   assert(_shapesel.end() != _shapesel.find(dst));
   // get the list of the selected shapes, on the source layer
   dataList* fortransfer = _shapesel[dst];
   assert(!fortransfer->empty());
   // now remove the selected shapes from the data holders ...
   if (_layers[dst]->delete_marked())
      // ... and validate quadTrees if needed
      if (!_layers[dst]->empty()) _layers[dst]->validate();
      else
      {//..or remove the source layer if it remained empty
         delete _layers[dst];
         _layers.erase(_layers.find(dst));
      }
   // traversing the input list - it contains the destination layers
   selectList::iterator CL = slst->begin();
   while (slst->end() != CL)
   {
      // we're not doing anything if the current layer appers to be dst,
      // i.e. don't mess around if the source and destination layers are the same!
      if ((dst != CL->first) && (0 != CL->first))
      {
         quadTree *dstlay = securelayer(CL->first);
         // traverse the shapes on this layer and add them to the destination layer
         dataList* lslct = CL->second;
         dataList::iterator DI = lslct->begin();
         while (DI != lslct->end())
         {
            // partially selected are not a subject of this operation, omit them
            if (sh_partsel != DI->first->status())
            {
               // find the current shape in the list of selected shapes
               dataList::iterator DDI;
               for(DDI = fortransfer->begin(); DDI != fortransfer->end(); DDI++)
                  if (*DDI == *DI) break;
               assert(DDI != fortransfer->end());
               // and delete it
               fortransfer->erase(DDI);
               // make sure that the destination layer exists in the _shapesel
               dataList* transfered = NULL;
               if (_shapesel.end() == _shapesel.find(CL->first))
                  _shapesel[CL->first] = transfered = new dataList();
               else
                  transfered = _shapesel[CL->first];
               // restore the status byte, of the fully selected shapes, because
               // it was modified to sh_deleted from the quadtree::delete_selected method
               DI->first->set_status(sh_selected);
               // add it to the destination layer ...
               dstlay->put(DI->first);
               // because the shape is changing it's layer and the _shapesel structure
               // is based on layer, so we have to remove the structure from the current layer
               // which is lslct and to add it to a new one which will be added to the _shapesel
               // at the end - when we exit the layer loop
               transfered->push_back(*DI);
            }
            DI++;
         }
         dstlay->validate();
      }
      CL++;
   }
   if (fortransfer->empty())
   {
      // if the container of the selected shapes is empty -
      delete fortransfer;
      selectList::iterator deliter = _shapesel.find(dst);
      _shapesel.erase(deliter);
   }
   else
   {
      // what has remained here in the fortransfer list should be only partially
      // selected shapes that are not interesting for us AND the shapes that
      // were already on dst layer and were deleted in the beginnig of the
      // function. The latter must be returned to the data holders
      // so...check & find whether there are still remaining fully selected shapes
      dataList::iterator DDI;
      for(DDI = fortransfer->begin(); DDI != fortransfer->end(); DDI++)
         if (sh_partsel != DDI->first->status()) break;
      if (fortransfer->end() != DDI)
      {
         //if so put them back in the data holders
         quadTree *dstlay = securelayer(dst);
         for(DDI = fortransfer->begin(); DDI != fortransfer->end(); DDI++)
            if (sh_partsel != DDI->first->status())
            {
               DDI->first->set_status(sh_selected);
               dstlay->put(DDI->first);
            }
         dstlay->validate();
      }
   }
   // For the records:
   // The overall cell overlap shouldn't have been changed,
   // so no need to refresh the overlapping box etc.
}

void laydata::tdtcell::resort() {
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      lay->second->resort();
}

void laydata::tdtcell::store_inAttic(laydata::atticList& _Attic) {
   dataList *lslct;
   shapeList *atl;
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = CL->second;
      if (_Attic.end() != _Attic.find(CL->first))  atl = _Attic[CL->first];
      else                                         atl = new shapeList();
      // move every single tdtdata in the corresponding Attic "shelf" ...
      dataList::iterator CI = lslct->begin();
      while (CI != lslct->end())
         // ... but only if it is marked as sh_deleted ...
         if (sh_deleted == CI->first->status()) {
            atl->push_back(CI->first);
            assert((!CI->second));
            // ... and free up the _shapesel list from it
            CI = lslct->erase(CI);
         }
         else CI++;
      if (atl->empty())    delete atl; 
      else             _Attic[CL->first] = atl;
      if (lslct->empty())  {
         delete lslct; 
         selectList::iterator deliter = CL++;
         _shapesel.erase(deliter);
      }
      else CL++;   
   }
}
bool laydata::tdtcell::overlapChanged(DBbox& old_overlap, laydata::tdtdesign* ATDB)
{
   DBbox new_overlap = overlap();
//   float areaold = old_overlap.area();
//   float areanew = new_overlap.area();
   // Invalidate all parent cells
   if (old_overlap != new_overlap) {
      invalidateParents(ATDB);return true;
   }
   else return false;
}

void laydata::tdtcell::invalidateParents(laydata::tdtdesign* ATDB)
{
   TDTHierTree* hc = ATDB->hiertree()->GetMember(this);
   while(hc)
   {
      if (hc->Getparent())
      {
         layerList llist = hc->Getparent()->GetItem()->_layers;
         if (llist.end() != llist.find(0)) llist[0]->invalidate();
      }      
      hc = hc->GetNextMember(this);
   }
}

bool laydata::tdtcell::validate_cells(laydata::tdtdesign* ATDB) {
   quadTree* wq = (_layers.end() != _layers.find(0)) ? _layers[0] : NULL;
   if (!(wq && wq->invalid())) return false;
   if (wq->full_validate()) {
      invalidateParents(ATDB); return true;
   }   
   else return false;
}

void laydata::tdtcell::validate_layers() {
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++) {
      if (0 == lay->first) continue;
      lay->second->validate();
   }
}

_dbl_word laydata::tdtcell::getFullySelected(dataList* lslct) const {
   _dbl_word numselected = 0;   
   for (dataList::const_iterator CI = lslct->begin(); 
                                                     CI != lslct->end(); CI++)
      // cont the fully selected shapes
      if (sh_selected == CI->first->status()) numselected++;
   return numselected;      
}

nameList* laydata::tdtcell::rehash_children() {
   // the actual list of names of the referenced cells
   nameList *cellnames = new nameList(); 
   // get the cells layer
   quadTree* refsTree = _layers[0];
   tdtcellref* wcl;
   if (refsTree) { // if it is not empty...
      // get all cell refs/arefs in a list - 
      dataList *refsList = new dataList();
      refsTree->select_all(refsList, false);
      // for every cell ref in the list
      for (dataList::const_iterator CC = refsList->begin(); 
                                     CC != refsList->end(); CC++) {
         wcl = static_cast<tdtcellref*>(CC->first);
         cellnames->push_back(wcl->cellname());
      }   
      cellnames->sort();
      cellnames->unique();
   }
   return cellnames;
}

void laydata::tdtcell::updateHierarchy(laydata::tdtdesign* ATDB) {
   tdtcell* childref;
   // Check that there are referenced cells
   if (_layers.end() == _layers.find(0))
      if (!_children.empty()) {
      // Means that all child references has been wiped out by the last 
      // operation, so remove all children from the hierarchy tree
         for (nameList::const_iterator CN = _children.begin(); 
                                       CN != _children.end(); CN++) {
            childref = ATDB->checkcell(*CN);
            childref->_orphan = ATDB->_hiertree->removeParent(
                                             childref, this, ATDB->_hiertree);
            ATDB->btreeRemoveMember(childref->name().c_str(), name().c_str(), 
                                                            childref->orphan());
         }   
         _children.clear();
      }
      else return; // there were no children before the last operation
   else {
      // Recollect the children.
      nameList *children_upd = rehash_children();
      std::pair<nameList::iterator,nameList::iterator> diff;
      do {
		  diff = std::mismatch(children_upd->begin(), children_upd->end(), _children.begin());
         if (diff.second != _children.end()) {
            childref = ATDB->checkcell(*(diff.second));
            // remove it from the hierarchy
            childref->_orphan = ATDB->_hiertree->removeParent(
                                             childref, this, ATDB->_hiertree);
            ATDB->btreeRemoveMember(childref->name().c_str(), name().c_str(), 
                                                            childref->orphan());
            _children.erase(diff.second);
         }   
      }      
      while (diff.second != _children.end());
   }
}   

void laydata::tdtcell::removePrep(laydata::tdtdesign* ATDB) const {
   // Check that there are referenced cells
   if (_layers.end() != _layers.find(0))
   {
      tdtcell* childref;
      assert(!_children.empty());
      // remove all children from the hierarchy tree
      for (nameList::const_iterator CN = _children.begin(); CN != _children.end(); CN++)
      {
         childref = ATDB->checkcell(*CN);
         childref->_orphan = ATDB->_hiertree->removeParent(
               childref, this, ATDB->_hiertree);
         ATDB->btreeRemoveMember(childref->name().c_str(), name().c_str(),
                                 childref->orphan());
      }
   }
   // remove this form _hiertree
   ATDB->_hiertree->removeRootItem(this, ATDB->_hiertree);
   // and browser tab
   ATDB->btreeRemoveMember(name().c_str(), NULL, false);
   // don't clear children, the cell will be moved to Attic
   //_children.clear();
}

unsigned int laydata::tdtcell::numselected() {
   unsigned int num = 0;
   dataList *lslct;
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = CL->second;
      num += lslct->size();
      CL++;
   }
   return num;
}

laydata::selectList* laydata::tdtcell::copy_selist() const {
   laydata::selectList *copy_list = new laydata::selectList();
   for (selectList::const_iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++) 
      (*copy_list)[CL->first] = new dataList(*(CL->second));
   return copy_list;
}


bool laydata::tdtcell::unselect_pointlist(selectDataPair& sel, selectDataPair& unsel) {
   SGBitSet* unspntlst = unsel.second;
   assert(NULL != unspntlst);

   SGBitSet* pntlst = NULL;
   if (sh_partsel == sel.first->status()) // if the shape is already partially selected
      pntlst = sel.second;
   else {// otherwise (sh_selected) create a new pointlist
      pntlst = new SGBitSet(sel.first->numpoints());
      pntlst->setall();
   }   
   assert(NULL != pntlst);
   // Check that the shape hasn't changed since
   if (pntlst->size() != unspntlst->size()) return false;
   // process select list
   for (word i = 0; i < pntlst->size(); i++) 
      if (unspntlst->check(i)) pntlst->reset(i);
   // finally check what left selected   
   if (pntlst->isallclear()) {
      delete pntlst;
      sel.first->set_status(sh_active);
      return true;
   }
   else {
      sel.first->set_status(sh_partsel);
      return false;
   }   
}   

void laydata::tdtcell::report_selected(real DBscale) const
{
   for (selectList::const_iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++)
   {
      for (dataList::const_iterator DP = CL->second->begin(); DP != CL->second->end(); DP++)
      {
         std::ostringstream ost;
         ost << "layer " << CL->first << " : ";
         DP->first->info(ost, DBscale);
         tell_log(console::MT_INFO, ost.str());
      }
   }
}

void laydata::tdtcell::collect_usedlays(const tdtdesign* ATDB, bool recursive, usedlayList& laylist) const{
   // first call recursively the method on all children cells
   if (recursive)
      for (nameList::const_iterator CC = _children.begin(); CC != _children.end(); CC++)
         ATDB->getcellnamepair(*CC)->second->collect_usedlays(ATDB, recursive, laylist);
   // then update with the layers used in this cell
   for(layerList::const_iterator CL = _layers.begin(); CL != _layers.end(); CL++)
      laylist.push_back(CL->first);
}

laydata::tdtcell::~tdtcell() 
{
   for (layerList::iterator lay = _layers.begin(); lay != _layers.end(); lay++) 
   {
      if (0 == lay->first) 
         lay->second->freememory();
      delete lay->second;
   }
   _layers.clear();
}
