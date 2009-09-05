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

#include "tpdph.h"
#include <iostream>
#include <sstream>
#include <iostream>
#include "tedcell.h"
#include "tedat.h"
#include "viewprop.h"
#include "tedesign.h"
#include "tenderer.h"
#include "ps_out.h"
#include "../tpd_ifaces/cif_io.h"
#include "../tpd_ifaces/gds_io.h"
#include "../tpd_common/outbox.h"

extern layprop::FontLibrary* fontLib;

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
   if (_activeref) _activecell = _activeref->cstructure();
   else            _activecell = vcell;
   _peditchain = crs;
   _ARTM = trans;
}

DBbox laydata::editobject::overlap() const{
   if (_activecell)
      return _activecell->cellOverlap().overlap(_ARTM);
   else return DEFAULT_OVL_BOX;
}

void laydata::editobject::reset() {
   if (_activecell) {
      _activecell->unselect_all();
      _editstack.push_front(DEBUG_NEW editobject(_activeref, _viewcell, _peditchain, _ARTM));
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
   _activecell = _activeref->cstructure();
   _viewcell = vref;
   _peditchain = crs;
   _ARTM = trans;
   blockfill();
}

bool laydata::editobject::pop() {
   if ((NULL == _activeref) || (!_peditchain) || (0 == _peditchain->size())) return false;
   if (_activecell) _activecell->unselect_all();
   //prepare current for pushing in the _editstack
   editobject* pres = DEBUG_NEW editobject(_activeref, _viewcell, _peditchain, _ARTM);
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
      _peditchain = DEBUG_NEW cellrefstack(*_peditchain);
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
      _activecell = _activeref->cstructure();
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
   _editstack.push_front(DEBUG_NEW editobject(_activeref, _viewcell, _peditchain, _ARTM));
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
      pres = DEBUG_NEW editobject(_activeref, _viewcell, _peditchain, _ARTM);
   editobject* prev = _editstack.front();
   _activeref = prev->_activeref;
   _activecell = prev->_activecell;
   cellrefstack* peditchain= prev->_peditchain;
   if (peditchain==NULL)
   {
      _peditchain = DEBUG_NEW cellrefstack;
   }
   else
   _peditchain = DEBUG_NEW cellrefstack(*peditchain);
   //_peditchain = DEBUG_NEW cellrefstack(*(prev->_peditchain));
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

bool laydata::editobject::securelaydef(unsigned layno)
{
   if (layno != REF_LAY)
   {
      bool newlay = _viewprop->addlayer( layno);
      if (newlay)
         _viewprop->addUnpublishedLay(layno);
      return newlay;
   }
   else
      return false;
}

std::string laydata::editobject::name() const
{
   if (_activecell) return _activecell->name();
   else return std::string("");
}

laydata::editobject::~editobject()
{
   if (_peditchain) delete _peditchain;
}

//-----------------------------------------------------------------------------
// class tdtdefaultcell
//-----------------------------------------------------------------------------
laydata::tdtdefaultcell::tdtdefaultcell(std::string name, int libID, bool orphan) :
      _orphan(orphan), _name(name), _libID(libID)  {}

void laydata::tdtdefaultcell::openGL_draw(layprop::DrawProperties&, bool active) const
{
}

void laydata::tdtdefaultcell::openGL_render(tenderer::TopRend& rend, const CTM& trans,
                                           bool selected, bool) const
{
   CTM ftm(TP(), 3000/OPENGL_FONT_UNIT, 45, false);
   DBbox pure_ovl(0,0,0,0);
   assert(NULL != fontLib); // check that font library is initialised
   fontLib->getStringBounds(&_name, &pure_ovl);
   //
   rend.pushCell(_name, trans, DEFAULT_ZOOM_BOX, false, selected);
   rend.setLayer(ERR_LAY, false);
   rend.text(&_name, ftm, pure_ovl, TP(), false);
   rend.popCell();
   //
}


void laydata::tdtdefaultcell::motion_draw(const layprop::DrawProperties&, ctmqueue&, bool active) const
{
}

void laydata::tdtdefaultcell::PSwrite(PSFile&, const layprop::DrawProperties&,
      const cellList*, const TDTHierTree*) const
{
}

laydata::TDTHierTree* laydata::tdtdefaultcell::hierout(laydata::TDTHierTree*& Htree, 
                                    tdtcell* parent, cellList* celldefs, const laydata::tdtlibdir* libdir)
{
   return Htree = DEBUG_NEW TDTHierTree(this, parent, Htree);
}

void laydata::tdtdefaultcell::updateHierarchy(tdtlibdir*)
{
}

bool laydata::tdtdefaultcell::relink(tdtlibdir*)
{
   return false;
}

void laydata::tdtdefaultcell::relinkThis(std::string, laydata::CellDefin, laydata::tdtlibdir* libdir)
{
}

DBbox laydata::tdtdefaultcell::cellOverlap() const
{
   return DEFAULT_ZOOM_BOX;
}

void laydata::tdtdefaultcell::write(TEDfile* const, const cellList&, const TDTHierTree*) const
{
   assert(false);
}

void laydata::tdtdefaultcell::GDSwrite(GDSin::GdsFile&, const cellList&, const TDTHierTree*, real, bool) const
{
   assert(false);
}

void laydata::tdtdefaultcell::CIFwrite(CIFin::CifExportFile&, const cellList&, const TDTHierTree*, real, bool) const
{
   assert(false);
}

void laydata::tdtdefaultcell::collect_usedlays(const tdtlibdir*, bool, WordList&) const
{
}

void laydata::tdtdefaultcell::invalidateParents(laydata::tdtlibrary* ATDB)
{
   TDTHierTree* hc = ATDB->hiertree()->GetMember(this);
   while(hc)
   {
      if (hc->Getparent())
      {
         layerList llist = hc->Getparent()->GetItem()->_layers;
         if (llist.end() != llist.find(REF_LAY)) llist[REF_LAY]->invalidate();
      }
      hc = hc->GetNextMember(this);
   }
}

//-----------------------------------------------------------------------------
// class tdtcell
//-----------------------------------------------------------------------------
laydata::tdtcell::tdtcell(std::string name) :
         tdtdefaultcell(name, TARGETDB_LIB, true), _cellOverlap(DEFAULT_OVL_BOX) {}


laydata::tdtcell::tdtcell(TEDfile* const tedfile, std::string name, int lib) :
         tdtdefaultcell(name, lib, true), _cellOverlap(DEFAULT_OVL_BOX)
{
   byte recordtype;
   word  layno;
   // now get the layers
   if       ((0 == tedfile->revision()) && (6 == tedfile->subrevision()))
   {
      while (tedf_CELLEND != (recordtype = tedfile->getByte()))
      {
         switch (recordtype)
         {
            case    tedf_LAYER: 
               layno = tedfile->getWord();
               if (0 != layno) _layers[layno]   = DEBUG_NEW tdtlayer(tedfile);
               else            _layers[REF_LAY] = DEBUG_NEW quadTree(tedfile);
               if (0 == layno) tedfile->get_cellchildnames(_children);
               break;
            default: throw EXPTNreadTDT("LAYER record type expected");
         }
      }
   }
   else
   {
      while (tedf_CELLEND != (recordtype = tedfile->getByte()))
      {
         switch (recordtype)
         {
            case    tedf_LAYER:
               layno = tedfile->getWord();
               _layers[layno] = DEBUG_NEW tdtlayer(tedfile);
               break;
            case    tedf_REFS:
               _layers[REF_LAY] = DEBUG_NEW quadTree(tedfile);
               tedfile->get_cellchildnames(_children);
               break;
            default: throw EXPTNreadTDT("LAYER record type expected");
         }
      }
   }

   getCellOverlap();
}

laydata::quadTree* laydata::tdtcell::securelayer(unsigned layno)
{
   if (_layers.end() == _layers.find(layno))
   {
      if (REF_LAY != layno) _layers[layno] = DEBUG_NEW tdtlayer();
      else                  _layers[layno] = DEBUG_NEW quadTree();
   }
   return _layers[layno];
}

laydata::tdtcellref* laydata::tdtcell::addcellref(laydata::tdtdesign* ATDB,
                                 CellDefin str, CTM trans, bool sortnow)
{
   if (!addchild(ATDB, str)) return NULL;
   quadTree *cellreflayer = securelayer(REF_LAY);
   laydata::tdtcellref* cellref = DEBUG_NEW tdtcellref(str, trans);
   if (sortnow) cellreflayer->add(cellref);
   else         cellreflayer->put(cellref);
   return cellref;
}

laydata::tdtcellaref* laydata::tdtcell::addcellaref(laydata::tdtdesign* ATDB,
          CellDefin str, CTM trans, ArrayProperties& arrprops, bool sortnow)
{
   if (!addchild(ATDB, str)) return NULL;
   quadTree *cellreflayer = securelayer(REF_LAY);
   laydata::tdtcellaref* cellaref =
                       DEBUG_NEW tdtcellaref(str, trans, arrprops);
   if (sortnow) cellreflayer->add(cellaref);
   else         cellreflayer->put(cellaref);
   return cellaref;
}

bool laydata::tdtcell::addchild(laydata::tdtdesign* ATDB, tdtdefaultcell* child) 
{
   // check for circular reference, i.e. the child is a father of some of its ancestors
   if (ATDB->dbHierCheckAncestors(this, child))
      //Circular reference found. child is already an ancestor of this
      return false;
   //leave a mark that child is not orphan
   child->parentfound();
   // update the list of children of the current cell
   _children.insert(child->name());
   // update the hierarchical tree
   ATDB->dbHierAddParent(child, this);
   return true;
}

void laydata::tdtcell::openGL_draw(layprop::DrawProperties& drawprop, bool active) const
{
   // Draw figures
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      unsigned curlayno = lay->first;
      if (!drawprop.layerHidden(curlayno)) drawprop.setCurrentColor(curlayno);
      else continue;
      // fancy like this (dlist iterator) , besause a simple
      // _shapesel[curlayno] complains about loosing qualifiers (const)
      selectList::const_iterator dlst;
      bool fill = drawprop.setCurrentFill(false);// honour block_fill state)
      if ((active) && (_shapesel.end() != (dlst = _shapesel.find(curlayno))))
         lay->second->openGL_draw(drawprop,dlst->second, fill);
      else
         lay->second->openGL_draw(drawprop, NULL, fill);
   }
}

void laydata::tdtcell::openGL_render(tenderer::TopRend& rend, const CTM& trans,
                                     bool selected, bool active) const
{
   rend.pushCell(_name, trans, _cellOverlap, active, selected);
   // Draw figures
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      unsigned curlayno = lay->first;
      if (rend.layerHidden(curlayno)) continue;
      // retrieve the selected objects (if they exists)
      selectList::const_iterator dlsti;
      const dataList* dlist;
      if (active && (_shapesel.end() != (dlsti = _shapesel.find(curlayno))))
         dlist = dlsti->second;
      else
         dlist = NULL;
      // traversing depends on the area overlap between the layer chunk and
      // current location of the view port. In other words traverse the
      // cells only if they are not already traversed. For more info -
      // see the documentation of tenderer.h::TenderReTV class
      if (REF_LAY != curlayno)
      {
         short cltype = lay->second->clip_type(rend);
         switch (cltype)
         {
            case -1: {// full overlap - conditional rendering
               if ( !rend.chunkExists(curlayno, (NULL != dlist)) )
                  lay->second->openGL_render(rend, dlist);
               break;
            }
            case  1: {//partial clip - render always
               rend.setLayer(curlayno, (NULL != dlist));
               lay->second->openGL_render(rend, dlist);
               break;
            }
            default: assert(0 == cltype);
         }
      }
      else
         lay->second->openGL_render(rend, dlist);
   }
   rend.popCell();
}

void laydata::tdtcell::motion_draw(const layprop::DrawProperties& drawprop,
                                          ctmqueue& transtack, bool active) const
{
   if (active)
   {
      // If this is the active cell, then we will have to visualize the
      // selected shapes in move. Patially selected fellas are processed
      // only if the current operation is move
      console::ACTIVE_OP actop = drawprop.currentop();
      //temporary draw of the active cell - moving selected shapes
      selectList::const_iterator llst;
      dataList::iterator dlst;
      for (llst = _shapesel.begin(); llst != _shapesel.end(); llst++) {
         const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(llst->first);
         for (dlst = llst->second->begin(); dlst != llst->second->end(); dlst++)
            if (!((actop == console::op_copy) && (sh_partsel == dlst->first->status())))
               dlst->first->motion_draw(drawprop, transtack, &(dlst->second));
      }
   }
   else {
      // Here we draw obviously a cell which reference has been selected
      // somewhere up the hierarchy. On this level - no selected shapes
      // whatsoever exists, so just perform a regular draw, but of course
      // without fill
      typedef layerList::const_iterator LCI;
      for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
         if (!drawprop.layerHidden(lay->first))
         {
            const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(lay->first);
            lay->second->motion_draw(drawprop, transtack);
         }
      transtack.pop_front();
   }
}

bool laydata::tdtcell::getshapeover(TP pnt, layprop::ViewProperties& viewprop)
{
   laydata::tdtdata* shape = NULL;
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      if ((REF_LAY != lay->first) && viewprop.selectable(lay->first) &&
                               lay->second->getobjectover(pnt,shape))
            return true;
   return false;
}

laydata::atticList* laydata::tdtcell::changeselect(TP pnt, SH_STATUS status, layprop::ViewProperties& viewprop)
{
   laydata::tdtdata* prev = NULL, *shape = NULL;
   unsigned prevlay;
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if (viewprop.selectable(lay->first))
      {
         while (lay->second->getobjectover(pnt,shape))
         {
            if ((status != shape->status()) &&
                ((NULL == prev) || (prev->overlap().area() > shape->overlap().area())))
            {
                  prev = shape; prevlay = lay->first;
            }
         }
      }
   }
   if (NULL != prev)
   {
      laydata::atticList* retlist = DEBUG_NEW atticList();
      laydata::shapeList* atl = DEBUG_NEW shapeList();
      atl->push_back(prev);
      (*retlist)[prevlay] = atl;
      if (sh_selected == status)
      {
         if (_shapesel.end() == _shapesel.find(prevlay))
            _shapesel[prevlay] = DEBUG_NEW dataList();
         prev->select_this(_shapesel[prevlay]);
      }
      else
      {
         dataList::iterator CI = _shapesel[prevlay]->begin();
         while (_shapesel[prevlay]->end() != CI)
         {
            if (CI->first == prev)
            {
               _shapesel[prevlay]->erase(CI);
               break;
            }
            else CI++;
         }
         prev->set_status(status);
      }
      return retlist;
   }
   else return NULL;
}

laydata::tdtcellref* laydata::tdtcell::getcellover(TP pnt, ctmstack& transtack, cellrefstack* refstack, layprop::ViewProperties& viewprop)
{
    if (_layers.end() == _layers.find(REF_LAY)) return NULL;
    laydata::tdtdata *cellobj = NULL;
//    laydata::tdtdata *shapeobj = NULL;
    laydata::tdtcellref *cref = NULL;
    // go trough referenced cells ...
    while (_layers[REF_LAY]->getobjectover(pnt,cellobj)) {
      //... and get the one that overlaps pnt.
      cref = static_cast<laydata::tdtcellref*>(cellobj);
      if (cref->cstructure() && (TARGETDB_LIB == cref->structure()->libID()) )
      {// avoid undefined & library cells
         TP pntadj = pnt * cref->translation().Reversed();
         // if in the selected reference there are shapes that overlap pnt...
         if (cref->cstructure()->getshapeover(pntadj, viewprop))
         {
            // ... that is what we need ...
            refstack->push_front(cref);
            // save the strack of reference translations
            transtack.push(transtack.top()*cref->translation());
            return cref;
         }   
         // ... otherwise, dive into the hierarchy
         else
         {
            laydata::tdtcellref *rref = cref->cstructure()->getcellover(pntadj,transtack,refstack, viewprop);
            if (rref) {
               refstack->push_front(cref);
               // save the stack of reference translations
               transtack.push(transtack.top()*cref->translation());
               return rref;
            }
         }
      }
   }
   return NULL;
}

void laydata::tdtcell::write(TEDfile* const tedfile, const cellList& allcells, const TDTHierTree* root) const
{
   // We going to write the cells in hierarchical order. Children - first!
   const laydata::TDTHierTree* Child= root->GetChild(TARGETDB_LIB);
   while (Child) {
//      tedfile->design()->getcellnamepair(Child->GetItem()->name())->second->write(tedfile, Child);
      allcells.find(Child->GetItem()->name())->second->write(tedfile, allcells, Child);
//      allcells[Child->GetItem()->name()]->write(tedfile, Child);
      Child = Child->GetBrother(TARGETDB_LIB);
   }
   // If no more children and the cell has not been written yet
   if (tedfile->checkcellwritten(name())) return;
   std::string message = "...writing " + name();
   tell_log(console::MT_INFO, message);
   tedfile->putByte(tedf_CELL);
   tedfile->putString(name());
   // and now the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      if (REF_LAY == wl->first)
      {
         tedfile->putByte(tedf_REFS);
         wl->second->write(tedfile);
         tedfile->putByte(tedf_REFSEND);
      }
      else if ( LAST_EDITABLE_LAYNUM >= wl->first )
      {
         tedfile->putByte(tedf_LAYER);
         tedfile->putWord(wl->first);
         wl->second->write(tedfile);
         tedfile->putByte(tedf_LAYEREND);
      }
   }
   tedfile->putByte(tedf_CELLEND);
   tedfile->registercellwritten(name());
}

void laydata::tdtcell::GDSwrite(GDSin::GdsFile& gdsf, const cellList& allcells,
                                 const TDTHierTree* root, real UU, bool recur) const
{
   // We are going to write the cells in hierarchical order. Children - first!
   if (recur)
   {
      const laydata::TDTHierTree* Child= root->GetChild(TARGETDB_LIB);
      while (Child)
      {
         allcells.find(Child->GetItem()->name())->second->GDSwrite(gdsf, allcells, Child, UU, recur);
         Child = Child->GetBrother(TARGETDB_LIB);
      }
   }
   // If no more children and the cell has not been written yet
   if (gdsf.checkCellWritten(name())) return;
   //
   std::string message = "...converting " + name();
   tell_log(console::MT_INFO, message);
   GDSin::GdsRecord* wr = gdsf.setNextRecord(gds_BGNSTR);
   gdsf.setTimes(wr);gdsf.flush(wr);
   wr = gdsf.setNextRecord(gds_STRNAME, name().size());
   wr->add_ascii(name().c_str()); gdsf.flush(wr);
   // and now the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      word dummy_lay, dummy_type;
      if ((REF_LAY != wl->first) && !gdsf.getMappedLayType(dummy_lay, dummy_type, wl->first) )
         continue;
      wl->second->GDSwrite(gdsf, wl->first, UU);
   }
   wr = gdsf.setNextRecord(gds_ENDSTR);gdsf.flush(wr);
   gdsf.registerCellWritten(name());
}


void laydata::tdtcell::CIFwrite(CIFin::CifExportFile& ciff, const cellList& allcells,
                                const TDTHierTree* root, real DBU, bool recur) const
{
   // We going to write the cells in hierarchical order. Children - first!
   if (recur)
   {
      const laydata::TDTHierTree* Child= root->GetChild(TARGETDB_LIB);
      while (Child)
      {
         allcells.find(Child->GetItem()->name())->second->CIFwrite(ciff, allcells, Child, DBU, recur);
         Child = Child->GetBrother(TARGETDB_LIB);
      }
   }
   // If no more children and the cell has not been written yet
   if (ciff.checkCellWritten(name())) return;
   //
   ciff.definitionStart(name(),DBU);
   // @TODO! See Bug#15242
   // Currently all coordinates are exported in DBU units and in the DS line of CIF
   // we put the ratio between DBU and the CIF precision (constant). This makes the
   // resulting CIF files ineffective, because normally there will be too many
   // pointless zeros. What we need is the smallest step which is used in each of
   // the cells. This can be calculated here, but also can be a value precalculated
   // on the fly when each and every points gets stored in the cell structure.
   // In practice this should be done in the folowing way
   // - calculate the reciprocal value of DBU ( (unsigned) 1/ DBU ). Let's name it
   //   DBUR. Make sure that the conversion error is cleaned-up.
   // - define the step and assign to it initially DBUR
   // - get the greatest common denominator between each of the coordinates and DBUR
   //   and compare it with the current step. If it is smaller - replace the step
   //   with the new value.

   // loop the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      if ((REF_LAY != wl->first) && !ciff.layerSpecification(wl->first)) continue;
      wl->second->CIFwrite(ciff);
   }
   ciff.definitionFinish();
}

void laydata::tdtcell::PSwrite(PSFile& psf, const layprop::DrawProperties& drawprop,
                               const cellList* allcells, const TDTHierTree* root) const
{
   if (psf.hier())
   {
      assert( root );
      assert( allcells );
      // We going to write the cells in hierarchical order. Children - first!
      const laydata::TDTHierTree* Child= root->GetChild(ALL_LIB);
      while (Child)
      {
         allcells->find(Child->GetItem()->name())->second->PSwrite(psf, drawprop, allcells, Child);
         Child = Child->GetBrother(ALL_LIB);
      }
      // If no more children and the cell has not been written yet
      if (psf.checkCellWritten(name())) return;
      //
      std::string message = "...converting " + name();
      tell_log(console::MT_INFO, message);
   }
   psf.cellHeader(name(),_cellOverlap);
   // and now the layers
   laydata::layerList::const_iterator wl;
   for (wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      word curlayno = wl->first;
      if (!drawprop.layerHidden(curlayno))
      {
         if (REF_LAY != curlayno)
            psf.propSet(drawprop.getColorName(curlayno), drawprop.getFillName(curlayno));
         wl->second->PSwrite(psf, drawprop);
      }
   }
   psf.cellFooter();
   if (psf.hier())
      psf.registerCellWritten(name());
}

laydata::TDTHierTree* laydata::tdtcell::hierout(laydata::TDTHierTree*& Htree,
                   tdtcell* parent, cellList* celldefs, const laydata::tdtlibdir* libdir)
{
   // collecting hierarchical information
   // NOTE! This function is supposed just to collect information.
   // Not to to alter it or update it! It's important to keep that rule, in order
   // to make cell instance tracking and cell hierarchy manageable.
   Htree = DEBUG_NEW TDTHierTree(this, parent, Htree);
   NameSet::const_iterator wn;
   for (wn = _children.begin(); wn != _children.end(); wn++)
   {
      if (celldefs->end() != celldefs->find(*wn) )
         (*celldefs)[*wn]->hierout(Htree, this, celldefs, libdir);
      else 
      {
         laydata::tdtdefaultcell* celldef = libdir->getLibCellDef(*wn, libID());
         if (NULL != celldef)
            celldef->hierout(Htree, this, celldefs, libdir);
         else
            // This assert is here on purpose! If you hit it - go fix your
            // problem somewhere else. It's not here! See the note above.
            assert(false);
      }
   }
   return  Htree;
}

void laydata::tdtcell::getCellOverlap() 
{
   if (0 == _layers.size())
      _cellOverlap = DEFAULT_OVL_BOX;
   else
   {
      layerList::const_iterator LCI = _layers.begin();
      _cellOverlap = LCI->second->overlap();
      while (++LCI != _layers.end())
         _cellOverlap.overlap(LCI->second->overlap());
   }
}   

void laydata::tdtcell::select_inBox(DBbox select_in, layprop::ViewProperties& viewprop, bool pntsel)
{
   // check that current cell is within 
   if (0.0 != select_in.cliparea(_cellOverlap))
   {
      // Select figures within the active layers
      typedef layerList::const_iterator LCI;
      for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      {
         if (viewprop.selectable(lay->first))
         {
            dataList* ssl;
            if (_shapesel.end() != _shapesel.find(lay->first))  
               ssl = _shapesel[lay->first];
            else
               ssl = DEBUG_NEW dataList();
/***/       lay->second->select_inBox(select_in, ssl, pntsel, viewprop.layselmask());
            if (ssl->empty())  delete ssl; 
            else               _shapesel[lay->first] = ssl; 
         }
      }
   }
}

void laydata::tdtcell::unselect_inBox(DBbox select_in, bool pntsel, layprop::ViewProperties& viewprop)
{
   // check that current cell is within 
   if (0.0 != select_in.cliparea(_cellOverlap))
   {
      // Unselect figures within the active layers
      typedef layerList::const_iterator LCI;
      for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      {
         if (viewprop.selectable(lay->first))
         {
            dataList* ssl;
            if (_shapesel.end() != _shapesel.find(lay->first))
            {
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

void laydata::tdtcell::select_fromList(selectList* slist, layprop::ViewProperties& viewprop)
{
   dataList* ssl;
   for (selectList::const_iterator CL = slist->begin(); CL != slist->end(); CL++)
   {
      // if the layer from the list exists in the layout and is not hidden
      if ((_layers.end() != _layers.find(CL->first)) && viewprop.selectable(CL->first))
      {
         if (_shapesel.end() != _shapesel.find(CL->first))  
            ssl = _shapesel[CL->first];
         else
            ssl = DEBUG_NEW dataList();
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
                        //delete CI->second;
                        CI->second.clear();
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

bool laydata::tdtcell::copy_selected(laydata::tdtdesign* ATDB, const CTM& trans)
{
   DBbox old_overlap(_cellOverlap);
   dataList copyList;
   dataList::iterator DI;
   tdtdata *data_copy;
   _dbl_word numshapes;
   selectList::iterator CL = _shapesel.begin(); 
   while (CL != _shapesel.end())
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes
      if (0 == (numshapes =  getFullySelected(CL->second)))
      {
         CL++; continue;
      }
      // Now - Go to copy and sort
      DI = CL->second->begin();
      while (CL->second->end() != DI)
      {
         // copy only fully selected shapes !
         if (sh_partsel == DI->first->status())
         {
            DI++; continue;
         }
         data_copy = DI->first->copy(trans);
         data_copy->set_status(sh_selected); DI->first->set_status(sh_active);
         if (1 != numshapes)  _layers[CL->first]->put(data_copy);
         else                 _layers[CL->first]->add(data_copy);
         // replace the data into the selected list
         DI = CL->second->erase(DI);
         CL->second->push_front(selectDataPair(data_copy,SGBitSet()));
      }
      if (1 != numshapes)  _layers[CL->first]->resort();
      CL++;
   }
   return overlapChanged(old_overlap, ATDB);
};

bool laydata::tdtcell::addlist(laydata::tdtdesign* ATDB, atticList* nlst) {
   DBbox old_overlap(_cellOverlap);
   for (atticList::const_iterator CL = nlst->begin();
                                   CL != nlst->end(); CL++) {
      // secure the target layer
      quadTree* wl = securelayer(CL->first);
      ATDB->securelaydef(CL->first);
      for (shapeList::const_iterator DI = CL->second->begin();
                                    DI != CL->second->end(); DI++) {
         // add it to the corresponding layer
         (*DI)->set_status(sh_active);
         wl->put(*DI);
         //update the hierarchy tree if this is a cell
         if (REF_LAY == CL->first) addchild(ATDB,
                            static_cast<tdtcellref*>(*DI)->structure());
      }
      wl->invalidate();
      CL->second->clear();
      delete (CL->second);
   }
   nlst->clear();
   delete nlst;
   validate_layers(); // because put is used
   return overlapChanged(old_overlap, ATDB);
}

laydata::dataList* laydata::tdtcell::secure_dataList(selectList& slst, unsigned layno)
{
   dataList* ssl;
   if (slst.end() != slst.find(layno)) ssl = slst[layno];
   else {
      ssl = DEBUG_NEW dataList();
      slst[layno] = ssl;
   }
   return ssl;
}

laydata::tdtdata* laydata::tdtcell::checkNreplacePoly(selectDataPair& sel, validator* check, 
                                             unsigned layno, selectList** fadead)
{
   if (check->valid())
   { // shape is valid ...
      if (shp_OK == check->status())  // entirely
         return NULL;
      else {// ... well, maybe not entirely...
         // it is still not clear here why and how the modefied polygon can be
         // in clockwise order. The case with Sergei's gds - metki
         // assert(!(shp_clock & check->status()));
         laydata::tdtdata* newshape = check->replacement();
         // add the new shape to the list of new shapes ...
         secure_dataList(*(fadead[2]),layno)->push_back(selectDataPair(newshape, SGBitSet()));
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
                                             unsigned layno, selectList** fadead)
{
   if (check->valid())
   { // shape is valid, but obviously not a box (if it gets here)
      laydata::tdtdata* newshape = check->replacement();
      // add the new shape to the list of new shapes ...
      secure_dataList(*(fadead[2]),layno)->push_back(selectDataPair(newshape, SGBitSet()));
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
   DBbox old_overlap(_cellOverlap);
   validator* checkS = NULL;
   // for every single layer selected
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // before all remove the selected and partially shapes 
      // from the data holders ...
      if (_layers[CL->first]->delete_marked(sh_selected, true))
         // ... and validate quadTrees
         _layers[CL->first]->validate();
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
            delete checkS;
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
   DBbox old_overlap(_cellOverlap);
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
         _layers[CL->first]->validate();
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

bool laydata::tdtcell::transfer_selected(laydata::tdtdesign* ATDB, const CTM& trans)
{
   DBbox old_overlap(_cellOverlap);
   // for every single layer selected
   for (selectList::const_iterator CL = _shapesel.begin();
                                                  CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // before all, remove the selected shapes from the data holders ...
      if (_layers[CL->first]->delete_marked())
         // ... and validate quadTrees if needed
         _layers[CL->first]->validate();
      // now for every single shape...
      for (dataList::iterator DI = CL->second->begin();
                                                DI != CL->second->end(); DI++)
      {
         // .. restore the status byte, of the fully selected shapes, because
         // it is modified to sh_deleted from the quadtree::delete_selected method
         if (sh_partsel != DI->first->status())
         {
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

bool laydata::tdtcell::delete_selected(laydata::atticList* fsel, 
                                             laydata::tdtlibdir* libdir )
{
   DBbox old_overlap(_cellOverlap);
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++) 
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes 
      if (0 == getFullySelected(CL->second)) continue;
      if (_layers[CL->first]->delete_marked())
      {
         if (_layers[CL->first]->empty())
         {
            delete _layers[CL->first]; _layers.erase(_layers.find(CL->first));
         }
         else _layers[CL->first]->validate();
      }
   }
   // Now move the selected shapes to the attic. Will be used for undo operations
   if (fsel) store_inAttic(*fsel);
   else      unselect_all(true);   
   updateHierarchy(libdir);
   return overlapChanged(old_overlap, (*libdir)());
}

bool laydata::tdtcell::cutpoly_selected(pointlist& plst, atticList** dasao)
{
   // calculate the overlap area of the cutting polygon
   DBbox cut_ovl = DBbox(plst[0]);
   for (word i = 1; i < plst.size(); cut_ovl.overlap(plst[i++]));
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                  CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes 
      if ((REF_LAY == CL->first) || (0 == getFullySelected(CL->second)))  continue;
      // initialize the corresponding 3 shape lists -> one per attic list
      // DElete/CUt/REst/
      shapeList* decure[3];
      byte i;
      for (i = 0; i < 3; decure[i++] = DEBUG_NEW shapeList());
      // do the clipping
      _layers[CL->first]->cutpoly_selected(plst, cut_ovl, decure);
      // add the shapelists to the collection, but only if they are not empty
      for (i = 0; i < 3; i++)
      {
         if (decure[i]->empty()) delete decure[i];
         else (*(dasao[i]))[CL->first] = decure[i];
      }
   }
   return !dasao[0]->empty();
}

bool laydata::tdtcell::stretch_selected(int bfactor, atticList** dasao)
{
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                  CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      // omit the layer if there are no fully selected shapes 
      if ((REF_LAY == CL->first) || (0 == getFullySelected(CL->second)))  continue;
      // initialize the corresponding 3 shape lists -> one per attic list
      // DElete/CUt/REst/
      shapeList* decure[2];
      byte i;
      for (i = 0; i < 2; decure[i++] = DEBUG_NEW shapeList());
      // do the logic operation
      for (dataList::const_iterator CD = CL->second->begin(); CD != CL->second->end(); CD++)
         CD->first->stretch(bfactor, decure);
      // add the shapelists to the collection, but only if they are not empty
      for (i = 0; i < 2; i++)
      {
         if (decure[i]->empty()) delete decure[i];
         else (*(dasao[i]))[CL->first] = decure[i];
      }
   }
   return !dasao[0]->empty();
}

bool laydata::tdtcell::merge_selected(atticList** dasao)
{
   // for every single layer in the select list
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                  CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL->first)));
      shapeList* mrgcand = NULL;
      shapeList* cleared = NULL;
      // omit the layer if there are no fully selected shapes 
      if ((REF_LAY == CL->first) || (NULL == (mrgcand = mergeprep(CL->first))))  continue;
      cleared = DEBUG_NEW shapeList();
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
      while (CS != mrgcand->end())
      {
         tdtdata* ref_shape = *CS;
         if ((merged_shape = _layers[CL->first]->merge_selected(ref_shape)))
         {
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
      while (CS != mrgcand->end())
      {
         // put back everything left to _shapesel ...
         CL->second->push_back(selectDataPair(*CS,SGBitSet()));
         if (sh_selected == (*CS)->status()) 
            //... and remove it from the mrgcand list if it is unchanged ...
            CS = mrgcand->erase(CS);
         else
         {
            // ... or change their status to sh_selected, but leave them in mrgcand list
            (*CS)->set_status(sh_selected);
            CS++;
         }
      }
      // - second sort the list of merged shapes - 
      CS = cleared->begin();
      while (CS != cleared->end())
      {
         if (sh_merged == (*CS)->status())
         {
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

bool laydata::tdtcell::destroy_this(laydata::tdtlibdir* libdir, tdtdata* ds, unsigned la)
{
   DBbox old_overlap(_cellOverlap);
   laydata::quadTree* lay = (_layers.find(la))->second;
   if (!lay) return false;
   // for layer la
   if (lay->delete_this(ds))
   {
      if (lay->empty())
      {
         delete lay; _layers.erase(_layers.find(la));
      }
      else lay->validate();
   }
   delete(ds);
   if (REF_LAY == la) updateHierarchy(libdir);
   return overlapChanged(old_overlap, (*libdir)());
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
   {
      if (viewprop.selectable(lay->first))
      {
         dataList* ssl = DEBUG_NEW dataList();
/***/    lay->second->select_all(ssl, viewprop.layselmask());
         if (ssl->empty())
         {
            delete ssl;
            assert(viewprop.layselmask() != laydata::_lmall);
         }
         else
            _shapesel[lay->first] = ssl;
      }
   }
}

void laydata::tdtcell::full_select()
{
   unselect_all();
   // Select figures within the active layers
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      dataList* ssl = DEBUG_NEW dataList();
/***/ lay->second->select_all(ssl);
      assert(!ssl->empty());
      _shapesel[lay->first] = ssl;
   }
}

void laydata::tdtcell::select_this(tdtdata* dat, unsigned lay)
{
   if (_shapesel.end() == _shapesel.find(lay)) _shapesel[lay] = DEBUG_NEW dataList();
   dat->select_this(_shapesel[lay]);
   //   _shapesel[lay]->push_back(selectDataPair(dat, NULL));
   //dat->set_status(sh_selected);
}

void laydata::tdtcell::unselect_all(bool destroy) 
{
   dataList* lslct;
   // for every layer with selected shapes
   for (selectList::const_iterator CL = _shapesel.begin(); 
                                                 CL != _shapesel.end(); CL++) 
   {
      lslct = CL->second;
      // for every single selectDataPair
      for (dataList::iterator CI = lslct->begin(); CI != lslct->end(); CI++) 
      {
         // unmark the shape
         CI->first->set_status(sh_active);
         // clear the list of selected points if it exists
         if (destroy)
         {
            if (0 != CI->second.size()) 
               CI->second.clear();
               //delete (CI->second);
            delete (CI->first);
         }
      }
      // clear the selectDataPair structures
      lslct->clear();
      // delete the dataList container
      delete lslct;
   }
   _shapesel.clear();
}

laydata::shapeList* laydata::tdtcell::mergeprep(unsigned layno)
{
   selectList::iterator CL = _shapesel.find(layno);
   if (_shapesel.end() == CL) return NULL;
   dataList* lslct = CL->second;
   shapeList* atl = DEBUG_NEW shapeList();
   
   dataList::iterator CI = lslct->begin();
   while (CI != lslct->end())
      // ... but only if it is marked as sh_deleted
      if (sh_selected == CI->first->status()) {
         atl->push_back(CI->first);
         assert(0 == CI->second.size());
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

laydata::atticList* laydata::tdtcell::groupPrep(laydata::tdtlibdir* libdir)
{
   atticList* fsel = DEBUG_NEW atticList();
   dataList *lslct;
   shapeList *atl;
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = CL->second;
      atl = DEBUG_NEW shapeList();
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
            assert(0 == CI->second.size());
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
   updateHierarchy(libdir);
   return fsel;
}

laydata::shapeList* laydata::tdtcell::ungroupPrep(laydata::tdtlibdir* libdir)
{
   shapeList* csel = DEBUG_NEW shapeList();
   if (_shapesel.end() != _shapesel.find(REF_LAY))
   {
      // unlink the selected cells
      if (_layers[REF_LAY]->delete_marked())
      {
         if (_layers[REF_LAY]->empty())
         {
            delete _layers[REF_LAY]; _layers.erase(_layers.find(REF_LAY));
         }
         else _layers[REF_LAY]->validate();
      }
      // now move every single shape in the corresponding fsel layer ...
      dataList::iterator CI = _shapesel[REF_LAY]->begin();
      while (CI != _shapesel[REF_LAY]->end())
      {
         // ... but only if it is marked as sh_deleted
         if (sh_deleted == CI->first->status())
         {
            CI->first->set_status(sh_active);
            csel->push_back(CI->first);
            assert( 0 == CI->second.size() );
            CI = _shapesel[REF_LAY]->erase(CI);
         }
         else CI++;
      }
      if (_shapesel[REF_LAY]->empty())
      {
         delete _shapesel[REF_LAY]; 
         _shapesel.erase(_shapesel.find(REF_LAY));
      }
   }
   // Don't invalidate parent cells. The reason is, that in result of the 
   // ungroup operation, shapes will be just regrouped and the final 
   // overlapping box is supposed to remain the same. 
   // The quadTrees of the REF_LAY must be (and is) validated
   updateHierarchy(libdir);
   return csel;
}

void laydata::tdtcell::transferLayer(unsigned dst)
{
   assert(REF_LAY != dst);
   quadTree *dstlay = securelayer(dst);
   dataList* transfered;
   if (_shapesel.end() == _shapesel.find(dst))
      _shapesel[dst] = transfered = DEBUG_NEW dataList();
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
      if ((dst != CL->first) && (REF_LAY != CL->first))
      {
         // now remove the selected shapes from the data holders ...
         if (_layers[CL->first]->delete_marked())
         {
            // ... and validate quadTrees if needed
            if (!_layers[CL->first]->empty()) 
               _layers[CL->first]->validate();
            else
            {//..or remove the source layer if it remained empty
               delete _layers[CL->first];
               _layers.erase(_layers.find(CL->first));
            }
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

void laydata::tdtcell::transferLayer(selectList* slst, unsigned dst)
{
   assert(REF_LAY != dst);
   assert(_shapesel.end() != _shapesel.find(dst));
   // get the list of the selected shapes, on the source layer
   dataList* fortransfer = _shapesel[dst];
   assert(!fortransfer->empty());
   // now remove the selected shapes from the data holders ...
   if (_layers[dst]->delete_marked())
   {
      // ... and validate quadTrees if needed
      if (!_layers[dst]->empty()) 
         _layers[dst]->validate();
      else
      {//..or remove the source layer if it remained empty
         delete _layers[dst];
         _layers.erase(_layers.find(dst));
      }
   }
   // traversing the input list - it contains the destination layers
   selectList::iterator CL = slst->begin();
   while (slst->end() != CL)
   {
      // we're not doing anything if the current layer appers to be dst,
      // i.e. don't mess around if the source and destination layers are the same!
      if ((dst != CL->first) && (REF_LAY != CL->first))
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
                  _shapesel[CL->first] = transfered = DEBUG_NEW dataList();
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

void laydata::tdtcell::resort()
{
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      lay->second->resort();
   getCellOverlap();
}

void laydata::tdtcell::store_inAttic(laydata::atticList& _Attic) {
   dataList *lslct;
   shapeList *atl;
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = CL->second;
      if (_Attic.end() != _Attic.find(CL->first))  atl = _Attic[CL->first];
      else                                         atl = DEBUG_NEW shapeList();
      // move every single tdtdata in the corresponding Attic "shelf" ...
      dataList::iterator CI = lslct->begin();
      while (CI != lslct->end())
         // ... but only if it is marked as sh_deleted ...
         if (sh_deleted == CI->first->status()) {
            atl->push_back(CI->first);
            assert(0 == CI->second.size());
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
   getCellOverlap();
   // Invalidate all parent cells
   if (old_overlap != _cellOverlap)
   {
      invalidateParents(ATDB);return true;
   }
   else return false;
}

bool laydata::tdtcell::validate_cells(laydata::tdtlibrary* ATDB) 
{
   quadTree* wq = (_layers.end() != _layers.find(REF_LAY)) ? _layers[REF_LAY] : NULL;
   if (!(wq && wq->invalid())) return false;
   if (wq->full_validate())
   {
      invalidateParents(ATDB); return true;
   }
   else return false;
}

/*! Validate all layers*/
void laydata::tdtcell::validate_layers()
{
   typedef layerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
      lay->second->validate();
}

_dbl_word laydata::tdtcell::getFullySelected(dataList* lslct) const 
{
   _dbl_word numselected = 0;
   for (dataList::const_iterator CI = lslct->begin(); 
                                                     CI != lslct->end(); CI++)
      // cont the fully selected shapes
      if (sh_selected == CI->first->status()) numselected++;
   return numselected;      
}

NameSet* laydata::tdtcell::rehash_children()
{
   // the actual list of names of the referenced cells
   NameSet *cellnames = DEBUG_NEW NameSet();
   // get the cells layer
   quadTree* refsTree = _layers[REF_LAY];
   tdtcellref* wcl;
   if (refsTree)
   {  // if it is not empty...
      // get all cell refs/arefs in a list - 
      dataList *refsList = DEBUG_NEW dataList();
      refsTree->select_all(refsList, laydata::_lmref, false);
      // for every cell ref in the list
      for (dataList::const_iterator CC = refsList->begin(); 
                                     CC != refsList->end(); CC++)
      {
         wcl = static_cast<tdtcellref*>(CC->first);
         cellnames->insert(wcl->cellname());
      }
      refsList->clear(); delete refsList;
   }
   return cellnames;
}

void laydata::tdtcell::updateHierarchy(laydata::tdtlibdir* libdir)
{
   laydata::tdtdesign* ATDB = (*libdir)();
   tdtdefaultcell* childref;
   // Check that there are referenced cells
   if (_layers.end() == _layers.find(REF_LAY))
      if (!_children.empty())
      {
         // Means that all child references has been wiped out by the last 
         // operation, so remove all children from the hierarchy tree
         for (NameSet::const_iterator CN = _children.begin();
                                      CN != _children.end(); CN++)
         {
            childref = ATDB->checkcell(*CN);
            if (NULL == childref)
               childref = libdir->getLibCellDef(*CN);
            ATDB->dbHierRemoveParent(childref, this, libdir);
         }
         _children.clear();
      }
      else return; // there were no children before the last operation
   else
   {
      // Recollect the children.
      NameSet *children_upd = rehash_children();
      std::pair<NameSet::iterator,NameSet::iterator> diff;
      while (true) 
      {
         diff = std::mismatch(children_upd->begin(), children_upd->end(), _children.begin());
         if (diff.second != _children.end())
         {
            childref = ATDB->checkcell(*(diff.second));
            if (NULL == childref)
               childref = libdir->getLibCellDef(*(diff.second));
            if (NULL != childref)
            {
               // remove it from the hierarchy
               ATDB->dbHierRemoveParent(childref, this, libdir);
            }
            _children.erase(diff.second);
         }
         else break;
      }
      children_upd->clear(); delete children_upd;
   }
}

bool laydata::tdtcell::relink(laydata::tdtlibdir* libdir)
{
   // get the cells layer
   if (_layers.end() == _layers.find(REF_LAY)) return false; // nothing to relink
   // if it is not empty get all cell refs/arefs in a list -
   quadTree* refsTree = _layers[REF_LAY];
   DBbox old_overlap(_cellOverlap);
   dataList *refsList = DEBUG_NEW dataList();
   refsTree->select_all(refsList, laydata::_lmref, false);
   // relink every single cell ref in the list
   dataList::iterator CC = refsList->begin();
   while (CC != refsList->end())
   {
      tdtcellref* wcl = static_cast<tdtcellref*>(CC->first);
      CellDefin newcelldef = libdir->linkcellref(wcl->cellname(), libID());
      if (newcelldef != wcl->structure())
      {
         CTM ori = wcl->translation();
         refsTree->delete_this(wcl);
         // remove the child-parent link of the old cell reference
         (*libdir)()->dbHierRemoveParent(wcl->structure(), this, libdir);
         // introduce the new child
         addcellref((*libdir)(), newcelldef, ori);
         CC = refsList->erase(CC);
      }
      else CC++;
   }
   refsList->clear(); delete refsList;
   return overlapChanged(old_overlap, (*libdir)());
}

void laydata::tdtcell::relinkThis(std::string cname, laydata::CellDefin newcelldef, laydata::tdtlibdir* libdir)
{
   assert( _layers.end() != _layers.find(REF_LAY) );
   DBbox old_overlap(_cellOverlap);
   // get all cell references
   dataList *refsList = DEBUG_NEW dataList();
   quadTree* refsTree = _layers[REF_LAY];
   refsTree->select_all(refsList, laydata::_lmref, false);
   //relink only the references to cname
   for (dataList::iterator CC = refsList->begin(); CC != refsList->end(); CC++)
   {
      tdtcellref* wcl = static_cast<tdtcellref*>(CC->first);
      if (cname == wcl->cellname())
      {
         refsTree->delete_this(wcl);
         (*libdir)()->dbHierRemoveParent(wcl->structure(), this, libdir);
         addcellref((*libdir)(), newcelldef, wcl->translation(), false);
         _layers[REF_LAY]->validate();
      }
   }
   refsList->clear(); delete refsList;
   invalidateParents((*libdir)());
}

unsigned int laydata::tdtcell::numselected()
{
   unsigned int num = 0;
   dataList *lslct;
   selectList::iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      lslct = CL->second;
      num += lslct->size();
      CL++;
   }
   return num;
}

laydata::selectList* laydata::tdtcell::copy_selist() const
{
   laydata::selectList *copy_list = DEBUG_NEW laydata::selectList();
   for (selectList::const_iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++) 
      (*copy_list)[CL->first] = DEBUG_NEW dataList(*(CL->second));
   return copy_list;
}


bool laydata::tdtcell::unselect_pointlist(selectDataPair& sel, selectDataPair& unsel)
{
   SGBitSet unspntlst = unsel.second;
   assert(0 != unspntlst.size());

   SGBitSet pntlst;
   if (sh_partsel == sel.first->status()) // if the shape is already partially selected
      pntlst = sel.second;
   else
   {// otherwise (sh_selected) create a new pointlist
      pntlst = SGBitSet(sel.first->numpoints());
      pntlst.setall();
   }
   assert(0 != pntlst.size());
   // Check that the shape hasn't changed since
   if (pntlst.size() != unspntlst.size()) return false;
   // process select list
   for (word i = 0; i < pntlst.size(); i++) 
      if (unspntlst.check(i)) pntlst.reset(i);
   // finally check what left selected   
   if (pntlst.isallclear())
   {
      pntlst.clear();
      sel.first->set_status(sh_active);
      return true;
   }
   else
   {
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
         if (REF_LAY != CL->first)
            ost << "layer " << CL->first << " : ";
         DP->first->info(ost, DBscale);
         tell_log(console::MT_INFO, ost.str());
      }
   }
}

void laydata::tdtcell::collect_usedlays(const tdtlibdir* LTDB, bool recursive, WordList& laylist) const
{
   // first call recursively the method on all children cells
   assert(recursive ? NULL != LTDB : true);
   if (recursive)
      for (NameSet::const_iterator CC = _children.begin(); CC != _children.end(); CC++)
         LTDB->collect_usedlays(*CC, recursive, laylist);
   // then update with the layers used in this cell
   for(layerList::const_iterator CL = _layers.begin(); CL != _layers.end(); CL++)
      if (LAST_EDITABLE_LAYNUM > CL->first) 
         laylist.push_back(CL->first);
}

laydata::tdtcell::~tdtcell() 
{
   unselect_all();
   for (layerList::iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if (REF_LAY == lay->first)
         lay->second->freememory();
      delete lay->second;
   }
   _layers.clear();
}
