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
//        Created: Thu Apr 19 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED cell related functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "tpdf_cells.h"

#include "../tpd_DB/datacenter.h"
#include "../tpd_common/tuidefs.h"
#include "../tpd_DB/browsers.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern wxWindow*                 TopedCanvasW;

//=============================================================================
tellstdfunc::stdNEWCELL::stdNEWCELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdNEWCELL::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
}

void tellstdfunc::stdNEWCELL::undo()
{
   // get the name of the DEBUG_NEW cell
   std::string  nm = getStringValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->removecell(nm,NULL, DATC->TEDLIB());
   DATC->unlockDB();
}

int tellstdfunc::stdNEWCELL::execute()
{
   std::string nm = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
   laydata::tdtcell* new_cell = ATDB->addcell(nm);
   DATC->unlockDB();
   if (NULL != new_cell)
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttstring(nm));
      LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
   }
   else
   {
      std::string news = "Cell \"";
      news += nm; news += "\" already exists";
      tell_log(console::MT_ERROR,news);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREMOVECELL::stdREMOVECELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdREMOVECELL::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdREMOVECELL::undo()
{
   TEUNDO_DEBUG("removecell( string ) UNDO");
   // get the contents of the removed cell
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   // get the name of the removed cell
   std::string  nm = getStringValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
   // first add a cell
   laydata::tdtcell* restored_cell = ATDB->addcell(nm);
   assert(NULL != restored_cell);
   // add the cell contents back
   // no validation required, because the cell is not referenced
   restored_cell->addlist(ATDB, get_shlaylist(pl));
   DATC->unlockDB();
   // finally - clean-up behind
   delete pl;
}

int tellstdfunc::stdREMOVECELL::execute()
{
   std::string nm = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      laydata::atticList* cell_contents = DEBUG_NEW laydata::atticList();
      bool removed = ATDB->removecell(nm,cell_contents, DATC->TEDLIB());
   DATC->unlockDB();
   if (removed)
   {  // removal has been successfull
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttstring(nm));
      UNDOPstack.push_front(make_ttlaylist(cell_contents));
      LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
   }
   clean_atticlist(cell_contents, true);
   delete(cell_contents);
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::stdOPENCELL::stdOPENCELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdOPENCELL::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdOPENCELL::undo() {
   TEUNDO_DEBUG("opencell( string ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      VERIFY(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
      DBbox* ovl  = DEBUG_NEW DBbox(ATDB->activeoverlap());
   DATC->unlockDB();
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(ovl));
   wxPostEvent(TopedCanvasW, eventZOOM);
}

int tellstdfunc::stdOPENCELL::execute()
{
   std::string nm = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      std::string oldnm = ATDB->activecellname();
      telldata::ttlist* selected = NULL;
      if ("" != oldnm)  selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->opencell(nm))
      {
         DATC->clearRulers();
         if (oldnm != "")
         {
            UNDOcmdQ.push_front(this);
            UNDOPstack.push_front(selected);
         }
         DBbox* ovl  = DEBUG_NEW DBbox(ATDB->activeoverlap());
/*-!-*/  DATC->unlockDB();
         if (*ovl == DEFAULT_OVL_BOX) *ovl = DEFAULT_ZOOM_BOX;
         browsers::celltree_open(nm);
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(tui::ZOOM_WINDOW);
         eventZOOM.SetClientData(static_cast<void*>(ovl));
         wxPostEvent(TopedCanvasW, eventZOOM);
         LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
      }
      else
      {
/*-!-*/  DATC->unlockDB();
         std::string news = "cell \"";news += nm;
         laydata::refnamepair striter;
         if (DATC->TEDLIB()->getLibCellRNP(nm, striter))
            news += "\" is a library cell and can't be edited";
         else
            news += "\" is not defined";
         tell_log(console::MT_ERROR,news);
         if (selected) delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPUSH::stdEDITPUSH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdEDITPUSH::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITPUSH::undo() {
   TEUNDO_DEBUG("editpush( point ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      VERIFY(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITPUSH::execute() {
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP p1DB = TP(p1->x(), p1->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->editpush(p1DB)) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"No editable cell reference found on this location");
         delete selected;
      }
   delete p1;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPOP::stdEDITPOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdEDITPOP::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITPOP::undo() {
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      VERIFY(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();   
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITPOP::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
   telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->editpop()) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"Already on the top level of the curent hierarchy");
         delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPREV::stdEDITPREV(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdEDITPREV::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITPREV::undo() {
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      VERIFY(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITPREV::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->editprev(false)) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"This is the first cell open during this session");
         delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITTOP::stdEDITTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{}

void tellstdfunc::stdEDITTOP::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITTOP::undo() {
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      VERIFY(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();   
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITTOP::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->edittop()) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"Already on the top level of the curent hierarchy");
         delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGROUP::stdGROUP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdGROUP::undo_cleanup() {
   getStringValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdGROUP::undo() {
   TEUNDO_DEBUG("group(string) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   // get the name of the removed cell
   std::string  name = getStringValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_fromList(get_ttlaylist(pl));
      ATDB->ungroup_this(ATDB->ungroup_prep(DATC->TEDLIB()));
      VERIFY(ATDB->removecell(name,NULL, DATC->TEDLIB()));
   DATC->unlockDB();
   delete pl;
   UpdateLV();
}

int tellstdfunc::stdGROUP::execute() {
   std::string name = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      bool group_sucessful = ATDB->group_selected(name, DATC->TEDLIB());
   DATC->unlockDB();
   if (group_sucessful)
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttstring(name));
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
      LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
      UpdateLV();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNGROUP::stdUNGROUP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdUNGROUP::undo_cleanup() {
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
   delete pl1;
}

void tellstdfunc::stdUNGROUP::undo() {
   TEUNDO_DEBUG("ungroup() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // first save the list of all currently selected components
      laydata::selectList *savelist = ATDB->copy_selist();
      // now unselect all
      ATDB->unselect_all();
      // get the list of shapes produced by the ungroup from the UNDO stack
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      // now get the list of the ungroupped cell ref's from the UNDO stack
      telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // and add them to the target cell
      ATDB->addlist(get_shlaylist(pl1)); 
      // select the restored cell refs
      ATDB->select_fromList(get_ttlaylist(pl1)); 
      // now restore selection
      ATDB->select_fromList(savelist);
      // and add the list of restored cells to the selection
      ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   // finally - clean-up behind
   delete pl;
   delete pl1;
   UpdateLV();
}

int tellstdfunc::stdUNGROUP::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::shapeList* cells4u = ATDB->ungroup_prep(DATC->TEDLIB());
   DATC->unlockDB();
   if (cells4u->empty()) {
      tell_log(console::MT_ERROR,"Nothing to ungroup");
      delete cells4u;
   }
   else {
      laydata::atticList* undol = DEBUG_NEW laydata::atticList();
      UNDOcmdQ.push_front(this);
      // Push the list of the cells to be ungroupped first
      (*undol)[0] = cells4u;
      UNDOPstack.push_front(make_ttlaylist(undol));
      ATDB = DATC->lockDB();
         // and then ungroup and push the list of the shapes produced in
         //result of the ungroup
         laydata::atticList* undol2 = ATDB->ungroup_this(cells4u);
      DATC->unlockDB();
      UNDOPstack.push_front(make_ttlaylist(undol2));
      // a bit funny, but effective way of cleaning-up cells4u
      // acutually - similar approach was used above to convert cells4u to a
      // layout list - all this using a atticList structure undol
      clean_atticlist(undol, false);
      delete undol;
      clean_atticlist(undol2, false);
      delete undol2;
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
      UpdateLV();
   }
   return EXEC_NEXT;
}

