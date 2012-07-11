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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
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
#include "auxdat.h"
#include "viewprop.h"
#include "tedesign.h"
#include "tenderer.h"
#include "ps_out.h"
#include "outbox.h"

extern layprop::FontLibrary* fontLib;

//=============================================================================
laydata::EditObject::EditObject()
{
   _activecell = NULL;   // the curently active cell
   _viewcell = NULL;     // current topview cell - if edit in place is active
   _activeref = NULL;    // current topview reference - " ------" ------"-----
   _peditchain= NULL;
}

laydata::EditObject::EditObject(TdtCellRef* cref, TdtCell* vcell, CellRefStack* crs, const CTM& trans)
{
   _activeref = cref;
   _viewcell = vcell;
   if (_activeref) _activecell = _activeref->cStructure();
   else            _activecell = vcell;
   _peditchain = crs;
   _ARTM = trans;
}

DBbox laydata::EditObject::overlap() const
{
   if (_activecell)
      return _activecell->cellOverlap().overlap(_ARTM);
   else return DEFAULT_OVL_BOX;
}

void laydata::EditObject::reset()
{
   if (_activecell)
   {
      _activecell->unselectAll();
      _editstack.push_front(DEBUG_NEW EditObject(_activeref, _viewcell, _peditchain, _ARTM));
   }
   _peditchain = NULL;
   _activecell = NULL;
   _viewcell = NULL;
   _activeref = NULL;
   _ARTM = CTM();
}

void laydata::EditObject::setcell(TdtCell* cell)
{
   reset();
   _activecell = cell; _viewcell = cell;
}

void laydata::EditObject::push(TdtCellRef* cref, TdtCell* vref, CellRefStack* crs, CTM trans)
{
   assert(cref);
   reset(); // Unset previous active reference if it exists
   _activeref = cref;
   _activecell = _activeref->cStructure();
   _viewcell = vref;
   _peditchain = crs;
   _ARTM = trans;
}

bool laydata::EditObject::pop()
{
   if ((NULL == _activeref) || (!_peditchain) || (0 == _peditchain->size())) return false;
   if (_activecell) _activecell->unselectAll();
   //prepare current for pushing in the _editstack
   EditObject* pres = DEBUG_NEW EditObject(_activeref, _viewcell, _peditchain, _ARTM);
   if (1 == _peditchain->size())
   {
      // if _activecell is one level only under the _viewcell - edit in place will not
      // be active anymore
      _activecell = _viewcell;
      _peditchain = NULL;
      _activeref = NULL;
      _ARTM = CTM();
   }
   else
   {
      // copy the _peditchain
      _peditchain = DEBUG_NEW CellRefStack(*_peditchain);
      // pop the last reference (that use to be current) out of the stack
      _peditchain->pop_back();
      // recalculate the translation matrix - code below might seem weird. This is just because
      // it is! The name of the matrix is Active Reference translation matrix, and it seems
      // to be a good name. So the matrix has to be calculated in reverse order, because the
      // bottom of the _peditchain contains the active reference (the top contains the top view)
      // so we must go bottom-up, instead of the usual (and wrong in this case) top-down
      // That is the why _ARTM is calculated by getCellOver method initially, but it is not
      // that obvious there because of the recurse
      _ARTM = CTM();
      for( CellRefStack::reverse_iterator CCR = _peditchain->rbegin(); CCR != _peditchain->rend(); CCR++)
         _ARTM *= (*CCR)->translation();
      // get a pointer to the active reference...
      _activeref = const_cast<TdtCellRef*>(_peditchain->back());
      // ... and active cell
      _activecell = _activeref->cStructure();
      //
   }
   _editstack.push_front(pres);
   return true;
}

bool laydata::EditObject::top()
{
   if (NULL == _activeref) return false;
   if (_activecell) _activecell->unselectAll();
   _editstack.push_front(DEBUG_NEW EditObject(_activeref, _viewcell, _peditchain, _ARTM));
   _activecell = _viewcell;
   _peditchain = NULL;
   _activeref = NULL;
   _ARTM = CTM();
   return true;
}

bool laydata::EditObject::previous(const bool undo)
{
   if (0 == _editstack.size()) return false;
   if (_activecell) _activecell->unselectAll();
   EditObject* pres = NULL;
   if (!undo)
      pres = DEBUG_NEW EditObject(_activeref, _viewcell, _peditchain, _ARTM);
   EditObject* prev = _editstack.front();
   _activeref = prev->_activeref;
   _activecell = prev->_activecell;
   CellRefStack* peditchain= prev->_peditchain;
   if (peditchain==NULL)
   {
      _peditchain = DEBUG_NEW CellRefStack;
   }
   else
   _peditchain = DEBUG_NEW CellRefStack(*peditchain);
   //_peditchain = DEBUG_NEW CellRefStack(*(prev->_peditchain));
   _viewcell = prev->_viewcell;
   _ARTM = prev->_ARTM;
   if (undo)
   {
      _editstack.pop_front();
      delete prev;
   }
   else
      _editstack.push_front(pres);
   return true;
}

std::string laydata::EditObject::name() const
{
   if (_activecell) return _activecell->name();
   else return std::string("");
}

void laydata::EditObject::storeViewPort(const DBbox& viewPort)
{
   DBbox* vp = DEBUG_NEW DBbox(viewPort);

   ViewPortMap::iterator vpi = _viewPortMap.find(_viewcell->name());
   if (_viewPortMap.end() != vpi)
   {
      delete vpi->second;
      vpi->second = vp;
   }
   else
      _viewPortMap[_viewcell->name()] = vp;
}

DBbox* laydata::EditObject::getLastViewPort() const
{
   ViewPortMap::const_iterator vpi = _viewPortMap.find(_viewcell->name());
   if (_viewPortMap.end() != vpi)
      // return a copy of the object, it should be deleted by the caller
      return DEBUG_NEW DBbox(*(vpi->second));
   else
      return NULL;
}

laydata::EditObject::~EditObject()
{
   if (_peditchain) delete _peditchain;
   for(ViewPortMap::const_iterator CPM = _viewPortMap.begin(); CPM != _viewPortMap.end(); CPM++)
      delete CPM->second;
}

//-----------------------------------------------------------------------------
// class TdtDefaultCell
//-----------------------------------------------------------------------------
laydata::TdtDefaultCell::TdtDefaultCell(std::string name, int libID, bool orphan) :
   _name          (name    ),
   _orphan        (orphan  ),
   _libID         (libID   )
{}

laydata::TdtDefaultCell::~TdtDefaultCell()
{
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      lay->freeMemory();
      delete (*lay);
   }
   _layers.clear();
}

void laydata::TdtDefaultCell::openGlDraw(layprop::DrawProperties&, bool active) const
{
}

void laydata::TdtDefaultCell::openGlRender(tenderer::TopRend& rend, const CTM& trans,
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


void laydata::TdtDefaultCell::motionDraw(const layprop::DrawProperties&, CtmQueue&, bool active) const
{
}

void laydata::TdtDefaultCell::psWrite(PSFile&, const layprop::DrawProperties&,
      const CellMap*, const TDTHierTree*) const
{
}

laydata::TDTHierTree* laydata::TdtDefaultCell::hierOut(laydata::TDTHierTree*& Htree,
                                    TdtCell* parent, CellMap* celldefs, const laydata::TdtLibDir* libdir)
{
   return Htree = DEBUG_NEW TDTHierTree(this, parent, Htree);
}

void laydata::TdtDefaultCell::updateHierarchy(TdtLibDir*)
{
}

bool laydata::TdtDefaultCell::relink(TdtLibDir*)
{
   return false;
}

void laydata::TdtDefaultCell::relinkThis(std::string, laydata::CellDefin, laydata::TdtLibDir* libdir)
{
}

DBbox laydata::TdtDefaultCell::cellOverlap() const
{
   return DEFAULT_ZOOM_BOX;
}

DBbox laydata::TdtDefaultCell::getVisibleOverlap(const layprop::DrawProperties&)
{
   return DEFAULT_ZOOM_BOX;
}

void laydata::TdtDefaultCell::write(OutputTdtFile* const, const CellMap&, const TDTHierTree*) const
{
   assert(false);
}

void laydata::TdtDefaultCell::dbExport(DbExportFile&, const CellMap&, const TDTHierTree*) const
{
   assert(false);
}

void laydata::TdtDefaultCell::collectUsedLays(const TdtLibDir*, bool, LayerTMPList&) const
{
}

void laydata::TdtDefaultCell::invalidateParents(laydata::TdtLibrary* ATDB)
{
   TDTHierTree* hc = ATDB->hiertree()->GetMember(this);
   while(hc)
   {
      if (hc->Getparent())
      {
         LayerHolder llist = hc->Getparent()->GetItem()->_layers;
         if (llist.end() != llist.find(REF_LAY_DEF)) llist[REF_LAY_DEF]->invalidate();
      }
      hc = hc->GetNextMember(this);
   }
}

bool laydata::TdtDefaultCell::checkLayer(const LayerDef& laydef) const
{
   return (_layers.end() != _layers.find(laydef));
}

//-----------------------------------------------------------------------------
// class TdtCell
//-----------------------------------------------------------------------------
laydata::TdtCell::TdtCell(std::string name) :
         TdtDefaultCell(name, TARGETDB_LIB, true), _cellOverlap(DEFAULT_OVL_BOX) {}


laydata::TdtCell::TdtCell(InputTdtFile* const tedfile, std::string name, int lib) :
         TdtDefaultCell(name, lib, true), _cellOverlap(DEFAULT_OVL_BOX)
{
   byte recordtype;
   while (tedf_CELLEND != (recordtype = tedfile->getByte()))
   {
      switch (recordtype)
      {
         case    tedf_LAYER:
            readTdtLay(tedfile);
            break;
         case    tedf_REFS:
            readTdtRef(tedfile);
            tedfile->getCellChildNames(_children);
            break;
         case    tedf_GRC:
         {
            auxdata::GrcCell* grc_structure = DEBUG_NEW auxdata::GrcCell(tedfile, _name);
            addAuxRef(grc_structure);
            break;
         }
         default: throw EXPTNreadTDT("LAYER record type expected");
      }
   }
   fixUnsorted();
}


void laydata::TdtCell::readTdtLay(InputTdtFile* const tedfile)
{
   byte      recordtype;
   TdtData*  newData;
   word  layno    = tedfile->getWord();
   QTreeTmp* tmpLayer = secureUnsortedLayer(tell2DBLayer(layno));
   while (tedf_LAYEREND != (recordtype = tedfile->getByte()))
   {
      switch (recordtype)
      {
         case     tedf_BOX : newData = DEBUG_NEW TdtBox(tedfile) ;break;
         case     tedf_POLY: newData = DEBUG_NEW TdtPoly(tedfile);break;
         case     tedf_WIRE: newData = DEBUG_NEW TdtWire(tedfile);break;
         case     tedf_TEXT: newData = DEBUG_NEW TdtText(tedfile);break;
         //--------------------------------------------------
         default: throw EXPTNreadTDT("Unexpected record type");
      }
      tmpLayer->put(newData);
   }
}

void laydata::TdtCell::readTdtRef(InputTdtFile* const tedfile)
{
   byte      recordtype;
   TdtData*  newData;
   QTreeTmp* tmpLayer = secureUnsortedLayer(REF_LAY_DEF);
   while (tedf_REFSEND != (recordtype = tedfile->getByte()))
   {
      switch (recordtype)
      {
         case  tedf_CELLREF: newData = DEBUG_NEW TdtCellRef(tedfile) ;break;
         case tedf_CELLAREF: newData = DEBUG_NEW TdtCellAref(tedfile);break;
         //--------------------------------------------------
         default: throw EXPTNreadTDT("Unexpected record type");
      }
      tmpLayer->put(newData);
   }
}

laydata::QuadTree* laydata::TdtCell::secureLayer(const LayerDef& laydef)
{
   // TODO Would be nice to update the code in such a way so the assert below holds
   // assert((layno < LAST_EDITABLE_LAYNUM) || (layno == REF_LAY));
   if (_layers.end() == _layers.find(laydef))
      _layers.add(laydef, DEBUG_NEW QuadTree());
   return _layers[laydef];
}

laydata::QTreeTmp* laydata::TdtCell::secureUnsortedLayer(const LayerDef& laydef)
{
   // TODO Would be nice to update the code in such a way so the assert below holds
   // assert((layno < LAST_EDITABLE_LAYNUM) || (layno == REF_LAY));
   if (_tmpLayers.end() == _tmpLayers.find(laydef))
      _tmpLayers.add(laydef, DEBUG_NEW QTreeTmp(secureLayer(laydef)));
   return _tmpLayers[laydef];
}

void laydata::TdtCell::registerCellRef(CellDefin str, CTM trans)
{
   QTreeTmp *cellreflayer = secureUnsortedLayer(REF_LAY_DEF);
   cellreflayer->put(DEBUG_NEW TdtCellRef(str, trans));
   _children.insert(str->name());
}

void laydata::TdtCell::registerCellARef(CellDefin str, CTM trans, ArrayProps& arrprops)
{
   QTreeTmp *cellreflayer = secureUnsortedLayer(REF_LAY_DEF);
   cellreflayer->put(DEBUG_NEW TdtCellAref(str, trans, arrprops));
  _children.insert(str->name());
}

laydata::TdtCellRef* laydata::TdtCell::addCellRef(laydata::TdtDesign* ATDB,
                                 CellDefin str, CTM trans)
{
   if (!addChild(ATDB, str)) return NULL;
   QuadTree *cellreflayer = secureLayer(REF_LAY_DEF);
   laydata::TdtCellRef* cellref = DEBUG_NEW TdtCellRef(str, trans);
   cellreflayer->add(cellref);
   return cellref;
}

laydata::TdtCellAref* laydata::TdtCell::addCellARef(laydata::TdtDesign* ATDB,
          CellDefin str, CTM trans, ArrayProps& arrprops)
{
   if (!addChild(ATDB, str)) return NULL;
   QuadTree *cellreflayer = secureLayer(REF_LAY_DEF);
   laydata::TdtCellAref* cellaref =
                       DEBUG_NEW TdtCellAref(str, trans, arrprops);
   cellreflayer->add(cellaref);
   return cellaref;
}

void laydata::TdtCell::addAuxRef(auxdata::GrcCell* str)
{
   QuadTree *cellreflayer = secureLayer(GRC_LAY_DEF);
   laydata::TdtAuxRef* cellref = DEBUG_NEW TdtAuxRef(str);
   cellreflayer->add(cellref);
//   TpdPost::treeMarkGrcMember(_name, true);
}

bool laydata::TdtCell::addChild(laydata::TdtDesign* ATDB, TdtDefaultCell* child)
{
   // check for circular reference, i.e. the child is a father of some of its ancestors
   if (ATDB->dbHierCheckAncestors(this, child))
      //Circular reference found. child is already an ancestor of this
      return false;
   //leave a mark that child is not orphan
   child->setOrphan(false);
   // update the list of children of the current cell
   _children.insert(child->name());
   // update the hierarchical tree
   ATDB->dbHierAddParent(child, this);
   return true;
}

void laydata::TdtCell::openGlDraw(layprop::DrawProperties& drawprop, bool active) const
{
   // Draw figures
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      LayerNumber curlayno = drawprop.getTenderLay(lay.number());
      if (!drawprop.layerHidden(curlayno)) drawprop.setCurrentColor(curlayno);
      else continue;
      // fancy like this (dlist iterator) , because a simple
      // _shapesel[curlayno] complains about loosing qualifiers (const)
      SelectList::Iterator dlst;
      bool fill = drawprop.setCurrentFill(false);// honour block_fill state)
      if ((active) && (_shapesel.end() != (dlst = _shapesel.find(curlayno))))
         lay->openGlDraw(drawprop,*dlst, fill);
      else
         lay->openGlDraw(drawprop, NULL, fill);
   }
}

void laydata::TdtCell::openGlRender(tenderer::TopRend& rend, const CTM& trans,
                                     bool selected, bool active) const
{
   rend.pushCell(_name, trans, _cellOverlap, active, selected);
   // Draw figures
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      //first - to check visibility of the layer
      if (rend.layerHidden(lay.number())) continue;
      //second - get internal layer number:
      //       - for regular database it is equal to the TDT layer number
      //       - for DRC database it is common for all layers - DRC_LAY
      LayerNumber curlayno = rend.getTenderLay(lay.number());
      // retrieve the selected objects (if they exists)
      SelectList::Iterator dlsti;
      const DataList* dlist;
      if (active && (_shapesel.end() != (dlsti = _shapesel.find(curlayno))))
         dlist = *dlsti;
      else
         dlist = NULL;
      // traversing depends on the area overlap between the layer chunk and
      // current location of the view port. In other words traverse the
      // cells only if they are not already traversed. For more info -
      // see the documentation of tenderer.h::TenderReTV class.
      // The exceptions here are:
      // - the cell references of course
      // - the DRC DB - because it is using a single layer besides it doesn't
      //   utilize the reference mechanism at all.
      switch (curlayno)
      {
         case GRC_LAY:
         case REF_LAY: lay->openGlRender(rend, dlist); break;
         case DRC_LAY: rend.setLayer(curlayno, (NULL != dlist));
                       lay->openGlRender(rend, dlist); break;
         default     :
         {
            short cltype = lay->clipType(rend);
            switch (cltype)
            {
               case -1: {// full overlap - conditional rendering
                  if ( !rend.chunkExists(curlayno, (NULL != dlist)) )
                     lay->openGlRender(rend, dlist);
                  break;
               }
               case  1: {//partial clip - render always
                  rend.setLayer(curlayno, (NULL != dlist));
                  lay->openGlRender(rend, dlist);
                  break;
               }
               default: assert(0 == cltype);
            }
            break;
         }
      }
   }
   rend.popCell();
}

void laydata::TdtCell::motionDraw(const layprop::DrawProperties& drawprop,
                                          CtmQueue& transtack, bool active) const
{
   if (active)
   {
      // If this is the active cell, then we will have to visualize the
      // selected shapes in move. Partially selected fellas are processed
      // only if the current operation is move
      console::ACTIVE_OP actop = drawprop.currentOp();
      //temporary draw of the active cell - moving selected shapes
      SelectList::Iterator llst;
      DataList::iterator dlst;
      for (llst = _shapesel.begin(); llst != _shapesel.end(); llst++) {
         const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(llst.number());
         for (dlst = llst->begin(); dlst != llst->end(); dlst++)
            if (!((actop == console::op_copy) && (sh_partsel == dlst->first->status())))
               dlst->first->motionDraw(drawprop, transtack, &(dlst->second));
      }
   }
   else {
      // Here we draw obviously a cell which reference has been selected
      // somewhere up the hierarchy. On this level - no selected shapes
      // whatsoever exists, so just perform a regular draw, but of course
      // without fill
      for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
         if (!drawprop.layerHidden(lay.number()))
         {
            const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(lay.number());
            for (QuadTree::DrawIterator CI = lay->begin(drawprop, transtack); CI != lay->end(); CI++)
               CI->motionDraw(drawprop, transtack, NULL);
         }
      transtack.pop_front();
   }
}

bool laydata::TdtCell::getShapeOver(TP pnt, const DWordSet& unselable)
{
   laydata::TdtData* shape = NULL;
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
      if ( (REF_LAY_DEF != lay.layDef())
          && (unselable.end() == unselable.find(lay.number()))
          && lay->getObjectOver(pnt,shape)
          )
         return true;
   return false;
}

laydata::AtticList* laydata::TdtCell::changeSelect(TP pnt, SH_STATUS status, const DWordSet& unselable)
{
   laydata::TdtData* prev = NULL;
   LayerDef prevlay(ERR_LAY, DEFAULT_LAY_DATATYPE);
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if (unselable.end() == unselable.find(lay.number()))
      {
         laydata::TdtData* shape = NULL;
         while (lay->getObjectOver(pnt,shape))
         {
            if ((status != shape->status()) &&
                ((NULL == prev) || (prev->overlap().boxarea() > shape->overlap().boxarea())))
            {
                  prev = shape; prevlay = lay.layDef();
            }
         }
      }
   }
   if (NULL != prev)
   {
      laydata::AtticList* retlist = DEBUG_NEW AtticList();
      laydata::ShapeList* atl = DEBUG_NEW ShapeList();
      atl->push_back(prev);
      retlist->add(prevlay, atl);
      if (sh_selected == status)
      {
         if (_shapesel.end() == _shapesel.find(prevlay))
            _shapesel.add(prevlay, DEBUG_NEW DataList());
         prev->selectThis(_shapesel[prevlay]);
      }
      else
      {
         DataList::iterator CI = _shapesel[prevlay]->begin();
         while (_shapesel[prevlay]->end() != CI)
         {
            if (CI->first == prev)
            {
               _shapesel[prevlay]->erase(CI);
               break;
            }
            else CI++;
         }
         prev->setStatus(status);
      }
      return retlist;
   }
   else return NULL;
}

void laydata::TdtCell::mouseHoover(TP& position, layprop::DrawProperties& drawprop, const DWordSet& unselable)
{
   laydata::TdtData* prev = NULL;
   LayerNumber prevlay = 0;
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if ( (unselable.end() == unselable.find(lay.number())) )
      {
         laydata::TdtData* shape = NULL;
         while (lay->getObjectOver(position,shape))
         {
            if ((sh_active == shape->status()) &&
                ((NULL == prev) || (prev->overlap().boxarea() > shape->overlap().boxarea())))
            {
               prev = shape; prevlay = lay.number();
            }
         }
      }
   }
   if (NULL == prev) return;
   //-------------------------------------------------------------
   PointVector points;

   prev->openGlPrecalc(drawprop, points);
   if(0 != points.size())
   {
      LayerNumber curlayno = drawprop.getTenderLay(prevlay);
      drawprop.setCurrentColor(curlayno);
      glLineWidth(5);
      prev->setStatus(sh_selected);
      prev->openGlDrawSel(points, NULL);
      prev->setStatus(sh_active);
      glLineWidth(1);
   }
   prev->openGlPostClean(drawprop, points);
}


laydata::AtticList* laydata::TdtCell::findSelected(TP pnt)
{
   laydata::AtticList *errList = DEBUG_NEW AtticList();

   laydata::TdtData *shape = NULL;
   //unsigned prevlay;
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      laydata::ShapeList* atl = DEBUG_NEW ShapeList();
      errList->add(lay.layDef(), atl);
      while(lay->getObjectOver(pnt,shape))
      {
         atl->push_back(shape);
      }
   }
   return errList;
}

laydata::TdtCellRef* laydata::TdtCell::getCellOver(TP pnt, CtmStack& transtack,
                     CellRefStack* refstack, const DWordSet& unselable)
{
    if (_layers.end() == _layers.find(REF_LAY_DEF)) return NULL;
    laydata::TdtData *cellobj = NULL;
//    laydata::TdtData *shapeobj = NULL;
    laydata::TdtCellRef *cref = NULL;
    // go trough referenced cells ...
    while (_layers[REF_LAY_DEF]->getObjectOver(pnt,cellobj))
    {
      //... and get the one that overlaps pnt.
      cref = static_cast<laydata::TdtCellRef*>(cellobj);
      if (cref->cStructure() && (TARGETDB_LIB == cref->structure()->libID()) )
      {// avoid undefined & library cells
         TP pntadj = pnt * cref->translation().Reversed();
         // if in the selected reference there are shapes that overlap pnt...
         if (cref->cStructure()->getShapeOver(pntadj, unselable))
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
            laydata::TdtCellRef *rref = cref->cStructure()->getCellOver(pntadj,transtack,refstack, unselable);
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

void laydata::TdtCell::write(OutputTdtFile* const tedfile, const CellMap& allcells, const TDTHierTree* root) const
{
   // We going to write the cells in hierarchical order. Children - first!
   const laydata::TDTHierTree* Child= root->GetChild(TARGETDB_LIB);
   while (Child) {
//      tedfile->design()->getCellNamePair(Child->GetItem()->name())->second->write(tedfile, Child);
      allcells.find(Child->GetItem()->name())->second->write(tedfile, allcells, Child);
//      allcells[Child->GetItem()->name()]->write(tedfile, Child);
      Child = Child->GetBrother(TARGETDB_LIB);
   }
   // If no more children and the cell has not been written yet
   if (tedfile->checkCellWritten(name())) return;
   std::string message = "...writing " + name();
   tell_log(console::MT_INFO, message);
   tedfile->putByte(tedf_CELL);
   tedfile->putString(name());
   // and now the layers
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if (REF_LAY_DEF == lay.layDef())
      {
         tedfile->putByte(tedf_REFS);
         for (QuadTree::Iterator DI = lay->begin(); DI != lay->end(); DI++)
            DI->write(tedfile);
         tedfile->putByte(tedf_REFSEND);
      }
      else if ( LAST_EDITABLE_LAYNUM >= lay.number() )
      {
         tedfile->putByte(tedf_LAYER);
         tedfile->putWord(lay.number());
         for (QuadTree::Iterator DI = lay->begin(); DI != lay->end(); DI++)
            DI->write(tedfile);
         tedfile->putByte(tedf_LAYEREND);
      }
      else if (GRC_LAY == lay.number())
      {
         tedfile->putByte(tedf_GRC);
         for (QuadTree::Iterator DI = lay->begin(); DI != lay->end(); DI++)
            DI->write(tedfile);
         tedfile->putByte(tedf_GRCEND);
      }
   }
   tedfile->putByte(tedf_CELLEND);
   tedfile->registerCellWritten(name());
}

void laydata::TdtCell::dbExport(DbExportFile& exportf, const CellMap& allcells,
                                const TDTHierTree* root) const
{
   // We going to write the cells in hierarchical order. Children - first!
   if (exportf.recur())
   {
      const laydata::TDTHierTree* Child= root->GetChild(TARGETDB_LIB);
      while (Child)
      {
         allcells.find(Child->GetItem()->name())->second->dbExport(exportf, allcells, Child);
         Child = Child->GetBrother(TARGETDB_LIB);
      }
   }
   // If no more children and the cell has not been written yet
   if (exportf.checkCellWritten(name())) return;
   //
   exportf.definitionStart(name());
   // @TODO! See Bug#15242 (CIF related)
   // Currently all coordinates are exported in DBU units and in the DS line of CIF
   // we put the ratio between DBU and the CIF precision (constant). This makes the
   // resulting CIF files ineffective, because normally there will be too many
   // pointless zeros. What we need is the smallest step which is used in each of
   // the cells. This can be calculated here, but also can be a value pre-calculated
   // on the fly when each and every points gets stored in the cell structure.
   // In practice this should be done in the following way
   // - calculate the reciprocal value of DBU ( (unsigned) 1/ DBU ). Let's name it
   //   DBUR. Make sure that the conversion error is cleaned-up.
   // - define the step and assign to it initially DBUR
   // - get the greatest common denominator between each of the coordinates and DBUR
   //   and compare it with the current step. If it is smaller - replace the step
   //   with the new value.

   // loop the layers
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if ( (REF_LAY_DEF != lay.layDef()) &&
           (GRC_LAY_DEF != lay.layDef()) &&
           !exportf.layerSpecification(lay.number()))
         continue;
      for (QuadTree::Iterator DI = lay->begin(); DI != lay->end(); DI++)
         DI->dbExport(exportf);
   }
   exportf.definitionFinish();
}

void laydata::TdtCell::psWrite(PSFile& psf, const layprop::DrawProperties& drawprop,
                               const CellMap* allcells, const TDTHierTree* root) const
{
   if (psf.hier())
   {
      assert( root );
      assert( allcells );
      // We going to write the cells in hierarchical order. Children - first!
      const laydata::TDTHierTree* Child= root->GetChild(ALL_LIB);
      while (Child)
      {
         allcells->find(Child->GetItem()->name())->second->psWrite(psf, drawprop, allcells, Child);
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
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      LayerNumber curlayno = lay.number();
      if (!drawprop.layerHidden(curlayno))
      {
         if (REF_LAY != curlayno)
            psf.propSet(drawprop.getColorName(curlayno), drawprop.getFillName(curlayno));
         for (QuadTree::Iterator DI = lay->begin(); DI != lay->end(); DI++)
            DI->psWrite(psf, drawprop);
      }
   }
   psf.cellFooter();
   if (psf.hier())
      psf.registerCellWritten(name());
}

laydata::TDTHierTree* laydata::TdtCell::hierOut(laydata::TDTHierTree*& Htree,
                   TdtCell* parent, CellMap* celldefs, const laydata::TdtLibDir* libdir)
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
         (*celldefs)[*wn]->hierOut(Htree, this, celldefs, libdir);
      else
      {
         laydata::TdtDefaultCell* celldef = libdir->getLibCellDef(*wn, libID());
         if (NULL != celldef)
            celldef->hierOut(Htree, this, celldefs, libdir);
         else
            // This assert is here on purpose! If you hit it - go fix your
            // problem somewhere else. It's not here! See the note above.
            assert(false);
      }
   }
   return  Htree;
}

void laydata::TdtCell::getCellOverlap()
{
   if (_layers.empty())
      _cellOverlap = DEFAULT_OVL_BOX;
   else
   {
      LayerHolder::Iterator LCI = _layers.begin();
      _cellOverlap = LCI->overlap();
      while (++LCI != _layers.end())
         _cellOverlap.overlap(LCI->overlap());
   }
}

DBbox laydata::TdtCell::getVisibleOverlap(const layprop::DrawProperties& prop)
{
   DBbox vlOverlap(DEFAULT_OVL_BOX);
   for (LayerHolder::Iterator LCI = _layers.begin(); LCI != _layers.end(); LCI++)
   {
      LayerNumber  layno  = LCI.number();
      if (!prop.layerHidden(layno))
      {
         if (REF_LAY == layno)
         {
            for(QuadTree::Iterator CS = LCI->begin(); CS != LCI->end(); CS++)
               CS->vlOverlap(prop, vlOverlap);
         }
         else
            vlOverlap.overlap(LCI->overlap());
      }
   }
   return vlOverlap;
}

void laydata::TdtCell::renameChild(std::string oldName, std::string newName)
{
   NameSet::iterator targetName = _children.find(oldName);
   if (_children.end() != targetName)
   {
      _children.erase(targetName);
      _children.insert(newName);
   }
}

void laydata::TdtCell::selectInBox(DBbox select_in, const DWordSet& unselable, word layselmask, bool pntsel)
{
   if (laydata::_lmnone == layselmask) return;
   // check that current cell is within
   if (0ll != select_in.cliparea(_cellOverlap))
   {
      // Select figures within the active layers
      for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
      {
         if (unselable.end() == unselable.find(lay.number()))
         {
            bool newDLHolder = false;
            DataList* ssl;
            if (_shapesel.end() != _shapesel.find(lay.layDef()))
               ssl = _shapesel[lay.layDef()];
            else
            {
               ssl = DEBUG_NEW DataList();
               newDLHolder = true;
            }
            for (QuadTree::ClipIterator CI = lay->begin(select_in); CI != lay->end(); CI++)
               if (layselmask & CI->lType())
                  CI->selectInBox(select_in, ssl, pntsel);
            if (ssl->empty())
            {
               delete ssl;
               assert(newDLHolder);
            }
            else if (newDLHolder)
               _shapesel.add(lay.layDef(), ssl);
         }
      }
   }
}

void laydata::TdtCell::unselectInBox(DBbox select_in, bool pntsel, const DWordSet& unselable)
{
   // check that current cell is within
   if (0ll != select_in.cliparea(_cellOverlap))
   {
      // Unselect figures within the active layers
      for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
      {
         if (unselable.end() == unselable.find(lay.number()))
         {
            DataList* ssl;
            if (_shapesel.end() != _shapesel.find(lay.layDef()))
            {
               ssl = _shapesel[lay.layDef()];
/*          lay->second->unselectInBox(select_in, ssl, pntsel);                */
//               void laydata::QTreeTmpl<DataT>::unselectInBox(DBbox& unselect_in, TObjDataPairList* unselist,
//                                                                                bool pselect)
               // check the entire holder for clipping...
               for (QuadTree::ClipIterator CI = lay->begin(select_in); CI != lay->end(); CI++)
               {
                  // now start unselecting from the list
                  DataList::iterator DI = ssl->begin();
                  while ( DI != ssl->end() )
                     if ((*CI == DI->first) &&
                         (DI->first->unselect(select_in, *DI, pntsel)))
                           DI = ssl->erase(DI);
                     else DI++;
               }

/*-----------------------------------------------------------------------------*/
               if (ssl->empty())
               {
                  delete ssl;
                  _shapesel.erase(lay.layDef());
               }
            }
         }
      }
   }
}

void laydata::TdtCell::selectFromList(SelectList* slist, const DWordSet& unselable)
{
   for (SelectList::Iterator CL = slist->begin(); CL != slist->end(); CL++)
   {
      // if the layer from the list exists in the layout and is not hidden
      if ( (_layers.end() != _layers.find(CL.layDef()))
         &&(unselable.end() == unselable.find(CL.number())) )
      {
         bool newDLHolder = false;
         DataList* ssl;
         if (_shapesel.end() != _shapesel.find(CL.layDef()))
            ssl = _shapesel[CL.layDef()];
         else
         {
            ssl = DEBUG_NEW DataList();
            newDLHolder = true;
         }
//         _layers[CL.number()]->selectFromList(*CL, ssl);
         selectFromListWrapper(_layers[CL.layDef()], *CL, ssl);
         if (ssl->empty())
         {
            delete ssl;
            assert(newDLHolder);
         }
         else if (newDLHolder)
            _shapesel.add(CL.layDef(), ssl);
      }
      delete *CL;
   }
   delete slist;
}

 void laydata::TdtCell::unselectFromList(SelectList* unslist, const DWordSet& unselable)
 {
   DataList* lslct = NULL;
   DataList::iterator CI;
   // for every layer in the unselect list
   for (SelectList::Iterator CUL = unslist->begin(); CUL != unslist->end(); CUL++)
   {
      // if the corresponding layer in the select list exists and is not locked
      if ( (_shapesel.end() != _shapesel.find(CUL.layDef()))
         &&(unselable.end() == unselable.find(CUL.number())) )
      {
         // start looping every shape to be unselected
         for(DataList::iterator CUI = CUL->begin(); CUI != CUL->end(); CUI++)
         {
            bool part_unselect = sh_partsel == CUI->first->status();
            // and check it against the shapes selected
            lslct = _shapesel[CUL.layDef()];
            CI = lslct->begin();
            while (lslct->end() != CI)
               if (CI->first == CUI->first)
               {
                  // if shapes matched - there are four cases of unselect:
                  // - full select - full unselect
                  // - full select - part unselect
                  // - part select - full unselect
                  // - part select - part unselect
                  if (sh_partsel == CI->first->status())
                     if (part_unselect)
                     {// part - part
                        if (unselectPointList(*CI,*CUI)) lslct->erase(CI);
                     }
                     else
                     { // part - full
                        //delete CI->second;
                        CI->second.clear();
                        CI->first->setStatus(sh_active);
                        lslct->erase(CI);
                     }
                  else
                     if (part_unselect)
                     {// full - part
                        if (unselectPointList(*CI,*CUI)) lslct->erase(CI);
                     }
                     else
                     { // full - full
                        CI->first->setStatus(sh_active);
                        lslct->erase(CI);
                     }
                  break;
               }
               else CI++;
         }
         // at the end, if the container of the selected shapes is empty -
         // delete it
         if ((NULL != lslct) && lslct->empty())
         {
            delete lslct; _shapesel.erase(CUL.layDef());
         }
      }
      // take care to clean up the memory from the unselect list
      delete *CUL;
   }
   delete unslist;
}

void laydata::TdtCell::copySelected(const CTM& trans)
{
   DataList copyList;
   DataList::iterator DI;
   TdtData *data_copy;
   dword numshapes;
   SelectList::Iterator CL = _shapesel.begin();
   while (CL != _shapesel.end())
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // omit the layer if there are no fully selected shapes
      if (0 == (numshapes =  getFullySelected(*CL)))
      {
         CL++; continue;
      }
      // Now - Go to copy and sort
      QTreeTmp* dst = secureUnsortedLayer(CL.layDef());
      DI = CL->begin();
      while (CL->end() != DI)
      {
         // copy only fully selected shapes !
         if (sh_partsel == DI->first->status())
         {
            DI++; continue;
         }
         data_copy = DI->first->copy(trans);
         data_copy->setStatus(sh_selected); DI->first->setStatus(sh_active);
         dst->put(data_copy);
         // replace the data into the selected list
         DI = CL->erase(DI);
         CL->push_front(SelectDataPair(data_copy,SGBitSet()));
      }
      CL++;
   }
   fixUnsorted();
};

void laydata::TdtCell::addList(laydata::TdtDesign* ATDB, AtticList* nlst)
{
   for (AtticList::Iterator CL = nlst->begin(); CL != nlst->end(); CL++)
   {
      // secure the target layer
      QTreeTmp* wl = secureUnsortedLayer(CL.layDef());
      // It appears that there is no need here to gather all the layers and eventually
      // to make sure that there are defined properties for them later. The point is that
      // the calls to this function are not normally using new layers. This is of course
      // a wild guess for the future and might bring troubles...
      /*newLays.insert(CL->first);*/
      for (ShapeList::const_iterator DI = CL->begin(); DI != CL->end(); DI++)
      {
         // add it to the corresponding layer
         (*DI)->setStatus(sh_active);
         wl->put(*DI);
         //update the hierarchy tree if this is a cell
         if (REF_LAY_DEF == CL.layDef()) addChild(ATDB,
                            static_cast<TdtCellRef*>(*DI)->structure());
      }
      CL->clear();
      delete (*CL);
   }
   nlst->clear();
   delete nlst;
   fixUnsorted();// because put was used
}

laydata::DataList* laydata::TdtCell::secureDataList(SelectList& slst, const LayerDef& laydef)
{
   DataList* ssl;
   if (slst.end() != slst.find(laydef)) ssl = slst[laydef];
   else {
      ssl = DEBUG_NEW DataList();
      slst.add(laydef, ssl);
   }
   return ssl;
}

laydata::ShapeList* laydata::TdtCell::checkNreplacePoly(SelectDataPair& sel, Validator* check,
                                                        const LayerDef& laydef, SelectList** fadead)
{
   if (check->acceptable())
   { // shape is valid ...
      if (shp_OK == check->status())  // entirely
         return NULL;
      else {// ... well, maybe not entirely...
         laydata::ShapeList* newShapes = check->replacements();
         // add the new shape to the list of new shapes ...
         for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
            secureDataList(*(fadead[2]),laydef)->push_back(SelectDataPair(*CS, SGBitSet()));
         // ... and put the initial shape(that is to be modified) in the
         // list of deleted shapes
         secureDataList(*(fadead[1]),laydef)->push_back(SelectDataPair(sel.first, sel.second));
         return newShapes;
      }
   }
   else {// the produced shape is invalid, so keep the old one and add it to the list of failed
      secureDataList(*(fadead[0]),laydef)->push_back(SelectDataPair(sel.first, sel.second));
      return NULL;
   }
}

laydata::ShapeList* laydata::TdtCell::checkNreplaceBox(SelectDataPair& sel, Validator* check,
                                                       const LayerDef& laydef, SelectList** fadead)
{
   if (check->acceptable())
   { // shape is valid, but obviously not a box (if it gets here)
      laydata::ShapeList* newShapes = check->replacements();
      // add the new shapes to the list of new shapes ...
      for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
         secureDataList(*(fadead[2]),laydef)->push_back(SelectDataPair(*CS, SGBitSet()));
      // ... and put the initial shape(that has been modified) in the
      // list of deleted shapes
      secureDataList(*(fadead[1]),laydef)->push_back(SelectDataPair(sel.first, sel.second));
      return newShapes;
   }
   else {// the produced shape is invalid, so keep the old one and add it to the list of failed
      secureDataList(*(fadead[0]),laydef)->push_back(SelectDataPair(sel.first, sel.second));
      return NULL;
   }
}

void laydata::TdtCell::moveSelected(const CTM& trans, SelectList** fadead)
{
   Validator* checkS = NULL;
   // for every single layer selected
   SelectList::Iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // before all remove the selected and partially shapes
      // from the data holders ...
      if (_layers[CL.layDef()]->deleteMarked(sh_selected, true))
         // ... and validate quadTrees
         _layers[CL.layDef()]->validate();
      // now for every single shape...

      DataList* lslct = *CL;
      DataList::iterator DI = lslct->begin();
      while (DI != lslct->end())
      {
         // .. restore the status byte, of the fully selected shapes, because
         // it was modified to sh_deleted from the quadtree::deleteSelected method
         if (sh_partsel != DI->first->status()) DI->first->setStatus(sh_selected);
         // ... move it ...
         if (NULL != (checkS = DI->first->move(trans, DI->second)))
         {
            // modify has been performed and a shape needs validation
            laydata::ShapeList* newShapes;
            if (NULL != (newShapes = checkNreplacePoly(*DI, checkS, CL.number(), fadead)))
            {
               // remove the shape from list of selected shapes and mark it as selected
               DI = lslct->erase(DI);
               for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
               {
                  // add the new shape to the list of new shapes ...
                  _layers[CL.layDef()]->add(*CS);
               }
               newShapes->clear();
               delete newShapes;
            }
            else
            {
               _layers[CL.layDef()]->add(DI->first);
               DI++;
            }
            delete checkS;
         }
         else
         {
            // move has been performed, so just add the shape back to the data holder
            _layers[CL.layDef()]->add(DI->first);
            DI++;
         }
      }
      _layers[CL.layDef()]->resort();
      if (lslct->empty())
      {
         // at the end, if the container of the selected shapes is empty -
         // Note! _shapesel.erase(CL) will invalidate the iterator which means that
         // it can't be incremented afterwards
         // This on some platforms/compilers/STL implementations or some combinations
         // of the above might work fine sometimes which makes the bug hunt a real fun!
         // (thanks to Sergey)
         delete lslct;
         _shapesel.erase(CL.layDef());
      }
      CL++;
   }
}

void laydata::TdtCell::rotateSelected(const CTM& trans, SelectList** fadead)
{
   Validator* checkS = NULL;
   // for every single layer selected
   SelectList::Iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // before all remove the selected and partially shapes
      // from the data holders ...
      if (_layers[CL.layDef()]->deleteMarked())
         // ... and validate quadTrees if needed
         _layers[CL.layDef()]->validate();
      // now for every single shape...
      DataList* lslct = *CL;
      DataList::iterator DI = lslct->begin();
      while (DI != lslct->end())
      {
         // .. restore the status byte, of the fully selected shapes, because
         // it was modified to sh_deleted from the quadtree::deleteSelected method
         if (sh_partsel != DI->first->status())
         {
            DI->first->setStatus(sh_selected);
            // ... rotate it ...
            if (NULL != (checkS = DI->first->move(trans, DI->second)))
            {// box->polygon conversion has been performed
               laydata::ShapeList* newShapes;
               if (NULL != (newShapes = checkNreplaceBox(*DI, checkS, CL.number(), fadead)))
               {
                  // remove the shape from list of selected shapes - don't delete the list of
                  // selected points BECAUSE it is (could be) used in UNDO
                  DI = lslct->erase(DI);
                  for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
                  {
                     // add the new shape to the list of new shapes ...
                     _layers[CL.layDef()]->add(*CS);
                     (*CS)->setStatus(sh_selected);
                  }
                  newShapes->clear();
                  delete newShapes;
               }
               else
               {
                  _layers[CL.layDef()]->add(DI->first);
                  DI++;
               }
               delete checkS;
            }
            else
            {// no conversion, so just add the shape back to the data holder
               _layers[CL.layDef()]->add(DI->first);
               DI++;
            }
         }
         else DI++;
      }
      _layers[CL.layDef()]->resort();
      if (lslct->empty())
      {
         // at the end, if the container of the selected shapes is empty -
         delete lslct;
         _shapesel.erase(CL.layDef());
      }
      CL++;
   }
}

void laydata::TdtCell::transferSelected(const CTM& trans)
{
   // for every single layer selected
   for (SelectList::Iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // before all, remove the selected shapes from the data holders ...
      if (_layers[CL.layDef()]->deleteMarked())
         // ... and validate quadTrees if needed
         _layers[CL.layDef()]->validate();
      // now for every single shape...
      for (DataList::iterator DI = CL->begin(); DI != CL->end(); DI++)
      {
         // .. restore the status byte, of the fully selected shapes, because
         // it is modified to sh_deleted from the quadtree::deleteSelected method
         if (sh_partsel != DI->first->status())
         {
            DI->first->setStatus(sh_selected);
            // ... transfer it ...
            DI->first->transfer(trans);
            // ... and add it back to the data Holder
            _layers[CL.layDef()]->add(DI->first);
         }
      }
      _layers[CL.layDef()]->resort();
   }
}

void laydata::TdtCell::deleteSelected(laydata::AtticList* fsel,
                                             laydata::TdtLibDir* libdir )
{
//   DBbox old_overlap(_cellOverlap);
   // for every single layer in the select list
   for (SelectList::Iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // omit the layer if there are no fully selected shapes
      if (0 == getFullySelected(*CL)) continue;
      if (_layers[CL.layDef()]->deleteMarked())
      {
         if (_layers[CL.layDef()]->empty())
         {
            delete _layers[CL.layDef()]; _layers.erase(CL.layDef());
         }
         else _layers[CL.layDef()]->validate();
      }
   }
   // Now move the selected shapes to the attic. Will be used for undo operations
   if (fsel) storeInAttic(*fsel);
   else      unselectAll(true);
   updateHierarchy(libdir);
//   return overlapChanged(old_overlap, (*libdir)());
}

bool laydata::TdtCell::cutPolySelected(PointVector& plst, AtticList** dasao)
{
   // calculate the overlap area of the cutting polygon
   DBbox cut_ovl = DBbox(plst[0]);
   for (word i = 1; i < plst.size(); cut_ovl.overlap(plst[i++]));
   // for every single layer in the select list
   for (SelectList::Iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // omit the layer if there are no fully selected shapes
      if ((REF_LAY_DEF == CL.layDef()) || (0 == getFullySelected(*CL)))  continue;
      // initialize the corresponding 3 shape lists -> one per attic list
      // DElete/CUt/REst/
      ShapeList* decure[3];
      byte i;
      for (i = 0; i < 3; decure[i++] = DEBUG_NEW ShapeList());
      // do the clipping
      QuadTree* curlay = _layers[CL.layDef()];
//      _layers[CL.number()]->cutPolySelected(plst, cut_ovl, decure);
      for (QuadTree::ClipIterator CI = curlay->begin(cut_ovl); CI != curlay->end(); CI++)
      {
//         DataT* wdt = _data[i];
         // for fully selected shapes if they overlap with the cutting polygon
         if ((sh_selected == CI->status()) &&
                                       (0ll != cut_ovl.cliparea(CI->overlap())))
            // go and clip it
            CI->polyCut(plst, decure);
      }
      // add the shapelists to the collection, but only if they are not empty
      for (i = 0; i < 3; i++)
      {
         if (decure[i]->empty()) delete decure[i];
         else dasao[i]->add(CL.layDef(), decure[i]);
      }
   }
   return !dasao[0]->empty();
}

bool laydata::TdtCell::stretchSelected(int bfactor, AtticList** dasao)
{
   // for every single layer in the select list
   for (SelectList::Iterator CL = _shapesel.begin();
                                                  CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // omit the layer if there are no fully selected shapes
      if ((REF_LAY_DEF == CL.layDef()) || (0 == getFullySelected(*CL)))  continue;
      // initialize the corresponding 3 shape lists -> one per attic list
      // DElete/CUt/REst/
      ShapeList* decure[2];
      byte i;
      for (i = 0; i < 2; decure[i++] = DEBUG_NEW ShapeList());
      // do the logic operation
      for (DataList::const_iterator CD = CL->begin(); CD != CL->end(); CD++)
         CD->first->stretch(bfactor, decure);
      // add the shapelists to the collection, but only if they are not empty
      for (i = 0; i < 2; i++)
      {
         if (decure[i]->empty()) delete decure[i];
         else dasao[i]->add(CL.layDef(), decure[i]);
      }
   }
   return !dasao[0]->empty();
}

bool laydata::TdtCell::mergeSelected(AtticList** dasao)
{
   // for every single layer in the select list
   for (SelectList::Iterator CL = _shapesel.begin();
                                                  CL != _shapesel.end(); CL++)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      ShapeList* mrgcand = NULL;
      ShapeList* cleared = NULL;
      // omit the layer if there are no fully selected shapes
      if ((REF_LAY_DEF == CL.layDef()) || (NULL == (mrgcand = mergePrep(CL.layDef()))))  continue;
      cleared = DEBUG_NEW ShapeList();
      //
      // A rather convoluted traversing to produce all merged shapes ...
      // the while cycle traverses the shapes in the merge candidates list and sends
      // every single one of them to the QuadTree::mergeSelected() as a
      // reference shape. If the operation succeeds, the loop alters the traversed
      // list so that the current shape and its merged buddy are removed, and
      // the resulting shape is added at the end.
      // The whole thing is based on the fact (I hope it's not a presumption) that
      // the shape that merges with the current (reference one) is located after
      // the current - so no shapes located in the list before the current are
      // altered AND all new shapes are added at the end of the list.
      // All this loops until the iterator reach the end of this dynamic list
      //
      TdtData* merged_shape = NULL;
      ShapeList::iterator CS = mrgcand->begin();
      while (CS != mrgcand->end())
      {
         TdtData* ref_shape = *CS;
         if ( (merged_shape = mergeWrapper(_layers[CL.layDef()], ref_shape)) )
//         if ((merged_shape = _layers[CL.number()]->mergeSelected(ref_shape)))
         {
            // if the mergeSelected produced non NULL result, it also
            // has changed the value of ref_shape. Now the most disgusting part -
            // to clear the merged shapes from the shapes tree ...
            _layers[CL.layDef()]->deleteThis(*CS);
            _layers[CL.layDef()]->deleteThis(ref_shape);
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
            _layers[CL.layDef()]->add(merged_shape);
            // and mark it as sh_merged
            merged_shape->setStatus(sh_merged);
         }
         else CS++;
      }
      // - first take care about the merge candidates and resulting shapes list
      CS = mrgcand->begin();
      while (CS != mrgcand->end())
      {
         // put back everything left to _shapesel ...
         CL->push_back(SelectDataPair(*CS,SGBitSet()));
         if (sh_selected == (*CS)->status())
            //... and remove it from the mrgcand list if it is unchanged ...
            CS = mrgcand->erase(CS);
         else
         {
            // ... or change their status to sh_selected, but leave them in mrgcand list
            (*CS)->setStatus(sh_selected);
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
      else  dasao[0]->add(CL.layDef(), cleared);
      if (mrgcand->empty()) delete mrgcand;
      else  dasao[1]->add(CL.layDef(), mrgcand);
   }
   return !dasao[0]->empty();
}

void laydata::TdtCell::destroyThis(laydata::TdtLibDir* libdir, TdtData* ds, const LayerDef& laydef)
{
   laydata::QuadTree* lay = *(_layers.find(laydef));
   if (!lay) return;
   // for layer la
   if (lay->deleteThis(ds))
   {
      if (lay->empty())
      {
         delete lay; _layers.erase(laydef);
      }
      else lay->validate();
   }
   delete(ds);
   if (REF_LAY_DEF == laydef) updateHierarchy(libdir);
}

void laydata::TdtCell::selectAll(const DWordSet& unselable, word layselmask)
{
   // This method might be called redundant, because the result of the
   // call selectInBox(overlap(),...) will produce the same results.
   // Several reasons though to have this method:
   // speed
   // ungroup operation requires lock/hide layer properties to be ignored

   // the idea behind unselecting all first is that there might be
   // partially selected shapes. In order to make the operation faster
   // and to avoid the call of TdtData::select_* methods (that deal with this)
   // for every single shape, it seems better to start from a clean list
   unselectAll();
   // Select figures within the active layers
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      if (unselable.end() == unselable.find(lay.number()))
      {
         DataList* ssl = DEBUG_NEW DataList();
/***/    selectAllWrapper(*lay, ssl, layselmask);
         if (ssl->empty())
         {
            delete ssl;
            assert(laydata::_lmall != layselmask);
         }
         else
            _shapesel.add(lay.layDef(),ssl);
      }
   }
}

void laydata::TdtCell::fullSelect()
{
   unselectAll();
   // Select figures within the active layers
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      DataList* ssl = DEBUG_NEW DataList();
/***/ selectAllWrapper(*lay,ssl);
      assert(!ssl->empty());
      _shapesel.add(lay.layDef(), ssl);
   }
}

void laydata::TdtCell::selectThis(TdtData* dat, const LayerDef& laydef)
{
   if (_shapesel.end() == _shapesel.find(laydef)) _shapesel.add(laydef, DEBUG_NEW DataList());
   dat->selectThis(_shapesel[laydef]);
   //   _shapesel[lay]->push_back(SelectDataPair(dat, NULL));
   //dat->setStatus(sh_selected);
}

void laydata::TdtCell::unselectAll(bool destroy)
{
   DataList* lslct;
   // for every layer with selected shapes
   for (SelectList::Iterator CL = _shapesel.begin();
                                                 CL != _shapesel.end(); CL++)
   {
      lslct = *CL;
      // for every single SelectDataPair
      for (DataList::iterator CI = lslct->begin(); CI != lslct->end(); CI++)
      {
         if (destroy)
         {
            SH_STATUS shStatus = CI->first->status();
            if ( (sh_selected == shStatus) || (sh_deleted == shStatus) )
               // destroy only fully selected objects
               delete (CI->first);
            else
               // the part selected shall remain
               CI->first->setStatus(sh_active);
         }
         else
         // unmark the shape
         CI->first->setStatus(sh_active);

      }
      // clear the SelectDataPair structures
      lslct->clear();
      // delete the DataList container
      delete lslct;
   }
   _shapesel.clear();
}

laydata::ShapeList* laydata::TdtCell::mergePrep(const LayerDef& laydef)
{
   SelectList::Iterator CL = _shapesel.find(laydef);
   if (_shapesel.end() == CL) return NULL;
   DataList* lslct = *CL;
   ShapeList* atl = DEBUG_NEW ShapeList();

   DataList::iterator CI = lslct->begin();
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

laydata::AtticList* laydata::TdtCell::groupPrep(laydata::TdtLibDir* libdir)
{
   AtticList* fsel = DEBUG_NEW AtticList();
   DataList *lslct;
   ShapeList *atl;
   SelectList::Iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = *CL;
      atl = DEBUG_NEW ShapeList();
      // unlink the fully selected selected shapes
      if (_layers[CL.layDef()]->deleteMarked())
      {
         if (_layers[CL.layDef()]->empty())
         {
            delete _layers[CL.layDef()]; _layers.erase(CL.layDef());
         }
         else _layers[CL.layDef()]->validate();
      }
      // now move every single shape in the corresponding fsel layer ...
      DataList::iterator CI = lslct->begin();
      while (CI != lslct->end())
         // ... but only if it is marked as sh_deleted
         if (sh_deleted == CI->first->status()) {
            CI->first->setStatus(sh_active);
            atl->push_back(CI->first);
            assert(0 == CI->second.size());
            CI = lslct->erase(CI);
         }
         else CI++;
      if (atl->empty()) delete atl;
      else              fsel->add(CL.layDef(), atl);
      if (lslct->empty())
      {
         delete lslct;
         _shapesel.erase(CL.layDef());
      }
      CL++;
   }
   // Don't invalidate parent cells. The reason is, that the new cell will
   // be refereced in the same place, so the final overlapping box is supposed
   // to remain the same. The quadTrees of the layers must be (and are) validated
   updateHierarchy(libdir);
   return fsel;
}

laydata::ShapeList* laydata::TdtCell::ungroupPrep(laydata::TdtLibDir* libdir)
{
   ShapeList* csel = DEBUG_NEW ShapeList();
   if (_shapesel.end() != _shapesel.find(REF_LAY_DEF))
   {
      DataList::iterator CI;
      // First of all - we have to preserve the references of the cells which
      // contain invalid (GRC) data. By convention those cells shall not be
      // ungrouped
      for (CI = _shapesel[REF_LAY_DEF]->begin(); CI != _shapesel[REF_LAY_DEF]->end(); CI++)
      {
         TdtCell* structure = static_cast<TdtCellRef*>(CI->first)->cStructure();
         if (structure->checkLayer(GRC_LAY))
         {
            CI->first->setStatus(sh_preserved);
            std::ostringstream ost;
            ost << "Cell \"" << structure->name() << "\" contains invalid data. Ignored during ungroup.";
            tell_log(console::MT_WARNING, ost.str());
         }
      }

      QuadTree* cellrefLayer = _layers[REF_LAY_DEF];
      // now unlink the selected cell references
      if (cellrefLayer->deleteMarked())
      {
         if (cellrefLayer->empty())
         {
            delete cellrefLayer; _layers.erase(REF_LAY_DEF);
         }
         else cellrefLayer->validate();
      }
      // now move every single shape in the corresponding csel layer and also
      // return the status of the cells with invalid data.
      CI = _shapesel[REF_LAY_DEF]->begin();
      while (CI != _shapesel[REF_LAY_DEF]->end())
      {
         // ... but only if it is marked as sh_deleted
         if (sh_deleted == CI->first->status())
         {
            CI->first->setStatus(sh_active);
            csel->push_back(CI->first);
            assert( 0 == CI->second.size() );
            CI = _shapesel[REF_LAY_DEF]->erase(CI);
         }
         else
         {
            if (sh_preserved == CI->first->status())
               CI->first->setStatus(sh_selected);
            CI++;
         }
      }
      if (_shapesel[REF_LAY_DEF]->empty())
      {
         delete _shapesel[REF_LAY_DEF];
         _shapesel.erase(REF_LAY_DEF);
      }
   }
   // Don't invalidate parent cells. The reason is, that in result of the
   // ungroup operation, shapes will be just regrouped and the final
   // overlapping box is supposed to remain the same.
   // The quadTrees of the REF_LAY must be (and are) validated
   updateHierarchy(libdir);
   return csel;
}

void laydata::TdtCell::transferLayer(const LayerDef& laydef)
{
   assert(REF_LAY_DEF != laydef);
   QTreeTmp *dstlay = secureUnsortedLayer(laydef);
   DataList* transfered;
   if (_shapesel.end() == _shapesel.find(laydef))
   {
      transfered = DEBUG_NEW DataList();
      _shapesel.add(laydef, transfered);
   }
   else
      transfered = _shapesel[laydef];
   assert(!_shapesel.empty());
   SelectList::Iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      assert((_layers.end() != _layers.find(CL.layDef())));
      // we're not doing anything if the current layer appears to be dst,
      // i.e. don't mess around if the source and destination layers are the same!
      // The same for the reference layer
      if ((laydef != CL.layDef()) && (REF_LAY_DEF != CL.layDef()))
      {
         // now remove the selected shapes from the data holders ...
         if (_layers[CL.layDef()]->deleteMarked())
         {
            // ... and validate quadTrees if needed
            if (!_layers[CL.layDef()]->empty())
               _layers[CL.layDef()]->validate();
            else
            {//..or remove the source layer if it remained empty
               delete _layers[CL.layDef()];
               _layers.erase(CL.layDef());
            }
         }
         // traverse the shapes on this layer and add them to the destination layer
         DataList* lslct = *CL;
         DataList::iterator DI = lslct->begin();
         while (DI != lslct->end())
         {
            // partially selected are not a subject of this operation, omit them
            if (sh_partsel != DI->first->status())
            {
               // restore the status byte, of the fully selected shapes, because
               // it was modified to sh_deleted from the quadtree::deleteSelected method
               DI->first->setStatus(sh_selected);
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
            _shapesel.erase(CL.layDef());
         }
         CL++;
      }
      else CL++;
   }
   // finally - validate the destination layer.
   fixUnsorted();
   // For the records:
   // The overall cell overlap shouldn't have been changed,
   // so no need to refresh the overlapping box etc.
}

void laydata::TdtCell::transferLayer(SelectList* slst, const LayerDef& laydef)
{
   assert(REF_LAY_DEF != laydef);
   assert(_shapesel.end() != _shapesel.find(laydef));
   // get the list of the selected shapes, on the source layer
   DataList* fortransfer = _shapesel[laydef];
   assert(!fortransfer->empty());
   // now remove the selected shapes from the data holders ...
   if (_layers[laydef]->deleteMarked())
   {
      // ... and validate quadTrees if needed
      if (!_layers[laydef]->empty())
         _layers[laydef]->validate();
      else
      {//..or remove the source layer if it remained empty
         delete _layers[laydef];
         _layers.erase(laydef);
      }
   }
   // traversing the input list - it contains the destination layers
   SelectList::Iterator CL = slst->begin();
   while (slst->end() != CL)
   {
      // we're not doing anything if the current layer appears to be dst,
      // i.e. don't mess around if the source and destination layers are the same!
      if ((laydef != CL.number()) && (REF_LAY_DEF != CL.layDef()))
      {
         QTreeTmp *dstlay = secureUnsortedLayer(CL.layDef());
         // traverse the shapes on this layer and add them to the destination layer
         DataList* lslct = *CL;
         DataList::iterator DI = lslct->begin();
         while (DI != lslct->end())
         {
            // partially selected are not a subject of this operation, omit them
            if (sh_partsel != DI->first->status())
            {
               // find the current shape in the list of selected shapes
               DataList::iterator DDI;
               for(DDI = fortransfer->begin(); DDI != fortransfer->end(); DDI++)
                  if (*DDI == *DI) break;
               assert(DDI != fortransfer->end());
               // and delete it
               fortransfer->erase(DDI);
               // make sure that the destination layer exists in the _shapesel
               DataList* transfered = NULL;
               if (_shapesel.end() == _shapesel.find(CL.layDef()))
               {
                  transfered = DEBUG_NEW DataList();
                  _shapesel.add(CL.layDef(), transfered);
               }
               else
                  transfered = _shapesel[CL.layDef()];
               // restore the status byte, of the fully selected shapes, because
               // it was modified to sh_deleted from the quadtree::deleteSelected method
               DI->first->setStatus(sh_selected);
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
      }
      CL++;
   }
   fixUnsorted();
   if (fortransfer->empty())
   {
      // if the container of the selected shapes is empty -
      delete fortransfer;
      _shapesel.erase(laydef);
   }
   else
   {
      // what has remained here in the fortransfer list should be only partially
      // selected shapes that are not interesting for us AND the shapes that
      // were already on dst layer and were deleted in the beginning of the
      // function. The latter must be returned to the data holders
      // so...check & find whether there are still remaining fully selected shapes
      DataList::iterator DDI;
      for(DDI = fortransfer->begin(); DDI != fortransfer->end(); DDI++)
         if (sh_partsel != DDI->first->status()) break;
      if (fortransfer->end() != DDI)
      {
         //if so put them back in the data holders
         QTreeTmp *dstlay = secureUnsortedLayer(laydef);
         for(DDI = fortransfer->begin(); DDI != fortransfer->end(); DDI++)
            if (sh_partsel != DDI->first->status())
            {
               DDI->first->setStatus(sh_selected);
               dstlay->put(DDI->first);
            }
         fixUnsorted();
      }
   }
   // For the records:
   // The overall cell overlap shouldn't have been changed,
   // so no need to refresh the overlapping box etc.
}

void laydata::TdtCell::fixUnsorted()
{
   for (TmpLayerMap::Iterator lay = _tmpLayers.begin(); lay != _tmpLayers.end(); lay++)
   {
      if (0 != lay->numObjects())
         lay->commit();
      else
      {
         LayerHolder::Iterator tlay = _layers.find(lay.layDef());
         assert(tlay != _layers.end());
         delete (*tlay);
         _layers.erase(lay.layDef());
      }
      delete *lay;
   }
   _tmpLayers.clear();
   getCellOverlap();
}

void laydata::TdtCell::storeInAttic(laydata::AtticList& _Attic) {
   DataList *lslct;
   ShapeList *atl;
   SelectList::Iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL) {
      lslct = *CL;
      if (_Attic.end() != _Attic.find(CL.layDef()))  atl = _Attic[CL.layDef()];
      else                                           atl = DEBUG_NEW ShapeList();
      // move every single TdtData in the corresponding Attic "shelf" ...
      DataList::iterator CI = lslct->begin();
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
      else             _Attic.add(CL.layDef(), atl);
      if (lslct->empty())  {
         delete lslct;
         _shapesel.erase(CL.layDef());
      }
      CL++;
   }
}

bool laydata::TdtCell::overlapChanged(DBbox& old_overlap, laydata::TdtDesign* ATDB)
{
   getCellOverlap();
   if (old_overlap != _cellOverlap)
   {
      // Invalidate all parent cells
      invalidateParents(ATDB);
      return true;
   }
   else return false;
}

bool laydata::TdtCell::validateCells(laydata::TdtLibrary* ATDB)
{
   QuadTree* wq = (_layers.end() != _layers.find(REF_LAY_DEF)) ? _layers[REF_LAY_DEF] : NULL;
   if (!(wq && wq->invalid())) return false;
   if (wq->fullValidate())
   {
      invalidateParents(ATDB); return true;
   }
   else return false;
}

/*! Validate all layers*/
void laydata::TdtCell::validateLayers()
{
   for (LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
      lay->validate();
}

dword laydata::TdtCell::getFullySelected(DataList* lslct) const
{
   dword numselected = 0;
   for (DataList::const_iterator CI = lslct->begin();
                                                     CI != lslct->end(); CI++)
      // cont the fully selected shapes
      if (sh_selected == CI->first->status()) numselected++;
   return numselected;
}

NameSet* laydata::TdtCell::rehashChildren()
{
   // the actual list of names of the referenced cells
   NameSet *cellnames = DEBUG_NEW NameSet();
   // get the cells layer
   QuadTree* refsTree = _layers[REF_LAY_DEF];
   TdtCellRef* wcl;
   if (refsTree)
   {  // if it is not empty...
      // get all cell refs/arefs in a list -
      DataList *refsList = DEBUG_NEW DataList();
      selectAllWrapper(refsTree, refsList, laydata::_lmref, false);
      // for every cell ref in the list
      for (DataList::const_iterator CC = refsList->begin();
                                     CC != refsList->end(); CC++)
      {
         wcl = static_cast<TdtCellRef*>(CC->first);
         cellnames->insert(wcl->cellname());
      }
      refsList->clear(); delete refsList;
   }
   return cellnames;
}

void laydata::TdtCell::updateHierarchy(laydata::TdtLibDir* libdir)
{
   laydata::TdtDesign* ATDB = (*libdir)();
   TdtDefaultCell* childref;
   // Check that there are referenced cells
   if (_layers.end() == _layers.find(REF_LAY_DEF))
      if (!_children.empty())
      {
         // Means that all child references has been wiped out by the last
         // operation, so remove all children from the hierarchy tree
         for (NameSet::const_iterator CN = _children.begin();
                                      CN != _children.end(); CN++)
         {
            childref = ATDB->checkCell(*CN);
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
      NameSet *children_upd = rehashChildren();
      std::pair<NameSet::iterator,NameSet::iterator> diff;
      while (true)
      {
         diff = std::mismatch(children_upd->begin(), children_upd->end(), _children.begin());
         if (diff.second != _children.end())
         {
            childref = ATDB->checkCell(*(diff.second));
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

bool laydata::TdtCell::relink(laydata::TdtLibDir* libdir)
{
   // get the cells layer
   if (_layers.end() == _layers.find(REF_LAY_DEF)) return false; // nothing to relink
   // if it is not empty get all cell refs/arefs in a list -
   QuadTree* refsTree = _layers[REF_LAY_DEF];
   DBbox old_overlap(_cellOverlap);
   DataList *refsList = DEBUG_NEW DataList();
   selectAllWrapper(refsTree, refsList, laydata::_lmref, false);
   // relink every single cell reference in the list
   DataList::iterator CC = refsList->begin();
   while (CC != refsList->end())
   {
      TdtCellRef* wcl = static_cast<TdtCellRef*>(CC->first);
      CellDefin newcelldef = libdir->linkCellRef(wcl->cellname(), libID());
      if (newcelldef != wcl->structure())
      {
         CTM ori = wcl->translation();
         refsTree->deleteThis(wcl);
         // remove the child-parent link of the old cell reference
         (*libdir)()->dbHierRemoveParent(wcl->structure(), this, libdir);
         // introduce the new child
         addCellRef((*libdir)(), newcelldef, ori);
         CC = refsList->erase(CC);
      }
      else CC++;
   }
   refsList->clear(); delete refsList;
   return overlapChanged(old_overlap, (*libdir)());
}

void laydata::TdtCell::relinkThis(std::string cname, laydata::CellDefin newcelldef, laydata::TdtLibDir* libdir)
{
   assert( _layers.end() != _layers.find(REF_LAY_DEF) );
   DBbox old_overlap(_cellOverlap);
   // get all cell references
   DataList *refsList = DEBUG_NEW DataList();
   QuadTree* refsTree = _layers[REF_LAY_DEF];
   selectAllWrapper(refsTree, refsList, laydata::_lmref, false);
   //relink only the references to cname
   for (DataList::iterator CC = refsList->begin(); CC != refsList->end(); CC++)
   {
      TdtCellRef* wcl = static_cast<TdtCellRef*>(CC->first);
      if (cname == wcl->cellname())
      {
         refsTree->deleteThis(wcl);
         (*libdir)()->dbHierRemoveParent(wcl->structure(), this, libdir);
         addCellRef((*libdir)(), newcelldef, wcl->translation());
      }
   }
   refsList->clear(); delete refsList;
   invalidateParents((*libdir)());
}

unsigned int laydata::TdtCell::numSelected()
{
   unsigned int num = 0;
   DataList *lslct;
   SelectList::Iterator CL = _shapesel.begin();
   while (_shapesel.end() != CL)
   {
      lslct = *CL;
      num += lslct->size();
      CL++;
   }
   return num;
}

laydata::SelectList* laydata::TdtCell::copySeList() const
{
   laydata::SelectList *copy_list = DEBUG_NEW laydata::SelectList();
   for (SelectList::Iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++)
      copy_list->add(CL.layDef(), DEBUG_NEW DataList(**CL));
   return copy_list;
}


bool laydata::TdtCell::unselectPointList(SelectDataPair& sel, SelectDataPair& unsel)
{
   SGBitSet unspntlst = unsel.second;
   assert(0 != unspntlst.size());

   SGBitSet pntlst;
   if (sh_partsel == sel.first->status()) // if the shape is already partially selected
      pntlst = sel.second;
   else
   {// otherwise (sh_selected) create a new pointlist
      pntlst = SGBitSet(sel.first->numPoints());
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
      sel.first->setStatus(sh_active);
      return true;
   }
   else
   {
      sel.first->setStatus(sh_partsel);
      return false;
   }
}

void laydata::TdtCell::reportSelected(real DBscale) const
{
   for (SelectList::Iterator CL = _shapesel.begin(); CL != _shapesel.end(); CL++)
   {
      for (DataList::const_iterator DP = CL->begin(); DP != CL->end(); DP++)
      {
         std::ostringstream ost;
         if (REF_LAY_DEF != CL.layDef())
            ost << "layer " << CL.number() << " : ";
         DP->first->info(ost, DBscale);
         tell_log(console::MT_INFO, ost.str());
      }
   }
}

void laydata::TdtCell::collectUsedLays(const TdtLibDir* LTDB, bool recursive, LayerTMPList& laylist) const
{
   // first call recursively the method on all children cells
   assert(recursive ? NULL != LTDB : true);
   if (recursive)
      for (NameSet::const_iterator CC = _children.begin(); CC != _children.end(); CC++)
         LTDB->collectUsedLays(*CC, recursive, laylist);
   // then update with the layers used in this cell
   for(LayerHolder::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
      if (LAST_EDITABLE_LAYNUM > lay.number())
         laylist.push_back(lay.number());
}

void laydata::TdtCell::selectAllWrapper(QuadTree* qtree, DataList* selist, word selmask, bool mark)
{
   if (laydata::_lmnone == selmask) return;
   for (QuadTree::Iterator CI = qtree->begin(); CI != qtree->end(); CI++)
   {
      if (selmask & CI->lType())
      {
         selist->push_back(SelectDataPair(*CI,SGBitSet()));
         if (mark) CI->setStatus(sh_selected);
      }
   }
}

laydata::TdtData* laydata::TdtCell::mergeWrapper(QuadTree* qtree, TdtData*& ref_shape)
{
   TdtData* mergeres = NULL;
   DBbox overlapRef = ref_shape->overlap();
   // now start traversing the shapes in the current holder one by one
   for (QuadTree::ClipIterator CI = qtree->begin(overlapRef); CI != qtree->end(); CI++)
   {
//      DataT* wdt = _data[i];
      // for fully selected shapes if they overlap with the reference
      // and this is not the same shape as the reference
      if ((*CI != ref_shape) &&
          ((sh_selected == CI->status()) || (sh_merged == CI->status())) &&
          (0ll != overlapRef.cliparea(CI->overlap())))
      {
         // go and merge it
         mergeres = polymerge(CI->shape2poly(), ref_shape->shape2poly());
         if (NULL != mergeres)
         {
            // If the merge produce a result - return the result and
            // substitute the ref_shape with its merged counterpart
            ref_shape = *CI;
            break;
         }
      }
   }
   return mergeres;
}

void laydata::TdtCell::selectFromListWrapper(QuadTree* qtree, DataList* src, DataList* dst)
{
   DataList::iterator DI;
   // loop the objects in the qTree first. It will be faster when there
   // are no objects in the current QTreeTmpl
   for (QuadTree::Iterator CI = qtree->begin(); CI != qtree->end(); CI++)
   {
      TdtData* wdt = *CI;
      DI = src->begin();
      // loop the objects from the select list
      while ( DI != src->end())
      {
         // if the objects (pointer) coincides - that's out object
         if (wdt == DI->first)
         {
            // select the object
            if (DI->second.size() == wdt->numPoints()) {
               wdt->setStatus(sh_partsel);
               dst->push_back(SelectDataPair(wdt,DI->second));
            }
            else {
               wdt->setStatus(sh_selected);
               dst->push_back(SelectDataPair(wdt,SGBitSet()));
            }
            // remove it from the select list - it will speed up the following
            // operations
            DI = src->erase(DI);
            // there is no point looping further, get the next object
            break;
         }
         else DI++;
      }
   }
//   for (byte i = 0; i < _props.numSubQuads(); i++)
//      _subQuads[i]->selectFromList(src, dst);
}

auxdata::GrcCell* laydata::TdtCell::getGrcCell()
{
   auxdata::GrcCell* theCell = NULL;
   if (checkLayer(GRC_LAY))
   {
      // Note! GRC_LAY by convention is supposed to have a SINGLE data object
      // of type TdtAuxRef
      unsigned numObjects = 0;
      QuadTree* wl = _layers[GRC_LAY_DEF];
      for (QuadTree::Iterator DI = wl->begin(); DI != wl->end(); DI++)
      {
         theCell = static_cast<laydata::TdtAuxRef*>(*DI)->structure();
         numObjects++;
      }
      assert(1 == numObjects);
   }
   return theCell;
}


void laydata::TdtCell::clearGrcCell()
{
   if (checkLayer(GRC_LAY))
   {
      // Note! GRC_LAY by convention is supposed to have a SINGLE data object
      // of type TdtAuxRef
      unsigned numObjects = 0;
      QuadTree* wl = _layers[GRC_LAY_DEF];
      for (QuadTree::Iterator DI = wl->begin(); DI != wl->end(); DI++)
      {
         delete *DI;
         numObjects++;
      }
      assert(1 == numObjects);
      delete (wl);
      _layers.erase(GRC_LAY_DEF);
   }
}

laydata::TdtCell::~TdtCell()
{
   unselectAll();
}

