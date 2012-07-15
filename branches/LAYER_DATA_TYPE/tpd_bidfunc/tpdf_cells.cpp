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
#include <sstream>
#include "tpdf_cells.h"
#include "tuidefs.h"
#include "datacenter.h"
#include "viewprop.h"


extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern wxWindow*                 TopedCanvasW;

//=============================================================================
tellstdfunc::stdNEWCELL::stdNEWCELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdNEWCELL::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
}

void tellstdfunc::stdNEWCELL::undo()
{
   // get the name of the DEBUG_NEW cell
   std::string  cname = getStringValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
       // make sure cname exists ...
      assert(tDesign->checkCell(cname));
      // ... and is not active
      assert(cname != tDesign->activeCellName());
      // gather the parent cells
      laydata::CellDefList parentCells;
      tDesign->collectParentCells(cname, parentCells);
      if (parentCells.empty())
      {
         // if no parent cells - it means that a simple "newcell" was
         // executed - so use the conventional remove cell
         laydata::TdtCell* rmvdcell = tDesign->removeTopCell(cname,NULL, dbLibDir);
         delete (rmvdcell);
      }
      else
         // parent cells found - so new cell was created on top of an
         // existing library cell or on top of an undefined, but referenced
         // cell. Use remove referenced cells
         tDesign->removeRefdCell(cname, parentCells, NULL, dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdNEWCELL::execute()
{
   std::string nm = getStringValue();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::TdtCell* new_cell = tDesign->addCell(nm, dbLibDir);
      if (NULL != new_cell)
      {
         if (1 < tDesign->cells().size())
         {
            UNDOcmdQ.push_front(this);
            UNDOPstack.push_front(DEBUG_NEW telldata::TtString(nm));
         }
         // else
         // No undo for the first cell in the library. We don't delete the active
         // cell at the moment
         LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
      }
      else
      {
         std::string news = "Cell \"";
         news += nm; news += "\" already exists in the target DB";
         tell_log(console::MT_ERROR,news);
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREMOVECELL::stdREMOVECELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdREMOVECELL::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::TtList*       pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   laydata::TdtCell* rmvdcell = static_cast<laydata::TdtCell*>(UNDOUstack.back());UNDOUstack.pop_back();
   clean_ttlaylist(pl);
   delete pl;
   delete rmvdcell;
}

void tellstdfunc::stdREMOVECELL::undo()
{
   TEUNDO_DEBUG("removecell( string ) UNDO");
   // get the removed cell itself (empty)
   laydata::TdtCell* rmvdcell = static_cast<laydata::TdtCell*>(UNDOUstack.front());UNDOUstack.pop_front();
   // get the contents of the removed cell
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   // get the name of the removed cell
   std::string  nm = getStringValue(UNDOPstack, true);

   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // first add a cell
      tDesign->addThisCell(rmvdcell, dbLibDir);
      // add the cell contents back
      tDesign->addList(get_shlaylist(pl), rmvdcell);
   }
   DATC->unlockTDT(dbLibDir, true);
   // finally - clean-up behind
   delete pl;
}

int tellstdfunc::stdREMOVECELL::execute()
{
   std::string cname = getStringValue();
   laydata::AtticList* cell_contents = NULL;
   laydata::TdtCell*   rmvdcell      = NULL;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      if (!tDesign->checkCell(cname))
      {
         std::string news = "Cell \"";
         news += cname; news += "\" doesn't exists. Nothing to remove";
         tell_log(console::MT_ERROR,news);

      }
      else if (cname == tDesign->activeCellName())
      {
         tell_log(console::MT_ERROR,"Active cell can't be removed");
      }
      else
      {
         laydata::CellDefList parentCells;
         tDesign->collectParentCells(cname, parentCells);
         if (parentCells.empty())
         {
            cell_contents = DEBUG_NEW laydata::AtticList();
            rmvdcell = tDesign->removeTopCell(cname,cell_contents, dbLibDir);
         }
         else
         {
            std::string news = "Cell \"";
            news += cname; news += "\" is referenced and can't be removed";
            tell_log(console::MT_ERROR,news);
         }
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   if (NULL != cell_contents)
   {  // removal has been successfull
      assert(NULL != rmvdcell);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtString(cname));
      UNDOPstack.push_front(make_ttlaylist(cell_contents));
      UNDOUstack.push_front(rmvdcell);
      clean_atticlist(cell_contents, false);
      delete(cell_contents);
      LogFile << LogFile.getFN() << "(\""<< cname << "\");"; LogFile.flush();
   }
   return EXEC_NEXT;
}

//=============================================================================
//tellstdfunc::stdREMOVEREFDCELL::stdREMOVEREFDCELL(telldata::typeID retype, bool eor) :
//      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
//{
//   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
//}
//
//void tellstdfunc::stdREMOVEREFDCELL::undo_cleanup()
//{
//}
//
//void tellstdfunc::stdREMOVEREFDCELL::undo()
//{
//}
//
//int tellstdfunc::stdREMOVEREFDCELL::execute()
//{
//   std::string cname = getStringValue();
//   laydata::TdtDesign* ATDB = DATC->lockDB(false);
//      if (!ATDB->checkCell(cname))
//      {
//         std::string news = "Cell \"";
//         news += cname; news += "\" doesn't exists. Nothing to remove";
//         tell_log(console::MT_ERROR,news);
//
//      }
//      else if (cname == ATDB->activeCellName())
//      {
//         tell_log(console::MT_ERROR,"Active cell can't be removed");
//      }
//      else
//      {
//         laydata::CellDefList parentCells;
//         ATDB->collectParentCells(cname, parentCells);
//         if (parentCells.empty())
//         {
//            //@TODO -  fold down to stdREMOVECELL
//         }
//         else
//         {
//            laydata::AtticList* cell_contents = DEBUG_NEW laydata::AtticList();
//            ATDB->removeRefdCell(cname, parentCells, cell_contents, DATC->TEDLIB());
//         }
//      }
//   DATC->unlockDB();
//   //if (removed)
//   //{  // removal has been successfull
//   //   UNDOcmdQ.push_front(this);
//   //   UNDOPstack.push_front(DEBUG_NEW telldata::TtString(nm));
//   //   UNDOPstack.push_front(make_ttlaylist(cell_contents));
//   //   LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
//   //}
//   //clean_atticlist(cell_contents, true);
//   //delete(cell_contents);
//   return EXEC_NEXT;
//}

//=============================================================================
tellstdfunc::stdOPENCELL::stdOPENCELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdOPENCELL::undo_cleanup()
{
   telldata::TtList* selected = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete selected; //TtList does not have active destructor
}

void tellstdfunc::stdOPENCELL::undo()
{
   TEUNDO_DEBUG("opencell( string ) UNDO");
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      VERIFY(tDesign->editPrev(true));
      TpdPost::celltree_open(tDesign->activeCellName());
      telldata::TtList* selected = TELL_UNDOOPS_UNDO(telldata::TtList*);
      LayerDefSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(selected), unselable);
      DBbox* ovl  = DEBUG_NEW DBbox(tDesign->activeOverlap());
      wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
      eventZOOM.SetInt(tui::ZOOM_WINDOW);
      eventZOOM.SetClientData(static_cast<void*>(ovl));
      wxPostEvent(TopedCanvasW, eventZOOM);
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdOPENCELL::execute()
{
   std::string nm = getStringValue();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      std::string oldnm = tDesign->activeCellName();
      telldata::TtList* selected = NULL;
      if ("" != oldnm)
      {
         selected = make_ttlaylist(tDesign->shapeSel());
         // Store the current view port
         DBbox cellViewPort(DEFAULT_ZOOM_BOX);
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            cellViewPort = drawProp->clipRegion();
         }
         PROPC->unlockDrawProp(drawProp, false);
         if (cellViewPort != DEFAULT_OVL_BOX)
            tDesign->storeViewPort(cellViewPort);
      }
      if (tDesign->openCell(nm))
      {
         PROPC->clearRulers();
         if (oldnm != "")
         {
            UNDOcmdQ.push_front(this);
            UNDOPstack.push_front(selected);
         }
         DBbox* ovl  = tDesign->getLastViewPort();
         if (NULL == ovl)
            ovl = DEBUG_NEW DBbox(tDesign->activeOverlap());
         if (*ovl == DEFAULT_OVL_BOX) *ovl = DEFAULT_ZOOM_BOX;
         TpdPost::celltree_open(nm);
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(tui::ZOOM_WINDOW);
         eventZOOM.SetClientData(static_cast<void*>(ovl));
         wxPostEvent(TopedCanvasW, eventZOOM);
         LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         std::string news = "Cell \"";news += nm;
         laydata::CellDefin strdefn;
         if (dbLibDir->getLibCellRNP(nm, strdefn))
            news += "\" is a library cell and can't be edited";
         else
            news += "\" is not defined";
         tell_log(console::MT_ERROR,news);
         if (selected) delete selected;
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPUSH::stdEDITPUSH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

void tellstdfunc::stdEDITPUSH::undo_cleanup()
{
   telldata::TtList* selected = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete selected; //TtList does not have active destructor
}

void tellstdfunc::stdEDITPUSH::undo()
{
   TEUNDO_DEBUG("editpush( point ) UNDO");
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      VERIFY(tDesign->editPrev(true));
      TpdPost::celltree_open(tDesign->activeCellName());
      telldata::TtList* selected = TELL_UNDOOPS_UNDO(telldata::TtList*);
      LayerDefSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(selected), unselable);
      std::string news("Cell "); news += tDesign->activeCellName(); news += " is opened";
      tell_log(console::MT_INFO,news);
      delete selected;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdEDITPUSH::execute()
{
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP p1DB = TP(p1->x(), p1->y(), DBscale);
   LayerDefSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtList* selected = make_ttlaylist(tDesign->shapeSel());
      if (tDesign->editPush(p1DB, unselable))
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = tDesign->activeCellName();
         TpdPost::celltree_highlight(name);
         std::string news("Cell "); news += name; news += " is opened";
         tell_log(console::MT_INFO,news);
         LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         tell_log(console::MT_ERROR,"No editable cell reference found on this location");
         delete selected;
      }
   }
   delete p1;
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPOP::stdEDITPOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

void tellstdfunc::stdEDITPOP::undo_cleanup()
{
   telldata::TtList* selected = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete selected; //TtList does not have active destructor
}

void tellstdfunc::stdEDITPOP::undo()
{
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      VERIFY(tDesign->editPrev(true));
      TpdPost::celltree_open(tDesign->activeCellName());
      telldata::TtList* selected = TELL_UNDOOPS_UNDO(telldata::TtList*);
      LayerDefSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(selected), unselable);
      std::string news("Cell "); news += tDesign->activeCellName(); news += " is opened";
      tell_log(console::MT_INFO,news);
      delete selected;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdEDITPOP::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtList* selected = make_ttlaylist(tDesign->shapeSel());
      if (tDesign->editPop())
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = tDesign->activeCellName();
         TpdPost::celltree_highlight(name);
         std::string news("Cell "); news += name; news += " is opened";
         tell_log(console::MT_INFO,news);
         UpdateLV(tDesign->numSelected());
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"Already on the top level of the curent hierarchy");
         delete selected;
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPREV::stdEDITPREV(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

void tellstdfunc::stdEDITPREV::undo_cleanup()
{
   telldata::TtList* selected = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete selected; //TtList does not have active destructor
}

void tellstdfunc::stdEDITPREV::undo()
{
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      VERIFY(tDesign->editPrev(true));
      TpdPost::celltree_open(tDesign->activeCellName());
      telldata::TtList* selected = TELL_UNDOOPS_UNDO(telldata::TtList*);
      LayerDefSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(selected), unselable);
      std::string news("Cell "); news += tDesign->activeCellName(); news += " is opened";
      tell_log(console::MT_INFO,news);
      delete selected;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdEDITPREV::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtList* selected = make_ttlaylist(tDesign->shapeSel());
      if (tDesign->editPrev(false))
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = tDesign->activeCellName();
         std::string news("Cell "); news += name; news += " is opened";
         tell_log(console::MT_INFO,news);
         TpdPost::celltree_highlight(name);
         UpdateLV(tDesign->numSelected());
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"This is the first cell open during this session");
         delete selected;
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITTOP::stdEDITTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{}

void tellstdfunc::stdEDITTOP::undo_cleanup()
{
   telldata::TtList* selected = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete selected; //TtList does not have active destructor
}

void tellstdfunc::stdEDITTOP::undo()
{
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      VERIFY(tDesign->editPrev(true));
      TpdPost::celltree_open(tDesign->activeCellName());
      telldata::TtList* selected = TELL_UNDOOPS_UNDO(telldata::TtList*);
      LayerDefSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(selected), unselable);
      std::string news("Cell "); news += tDesign->activeCellName(); news += " is opened";
      tell_log(console::MT_INFO,news);
      delete selected;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdEDITTOP::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtList* selected = make_ttlaylist(tDesign->shapeSel());
      if (tDesign->editTop())
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = tDesign->activeCellName();
         TpdPost::celltree_highlight(name);
         std::string news("Cell "); news += name; news += " is opened";
         tell_log(console::MT_INFO,news);
         UpdateLV(tDesign->numSelected());
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"Already on the top level of the current hierarchy");
         delete selected;
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGROUP::stdGROUP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdGROUP::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete pl;
}

void tellstdfunc::stdGROUP::undo()
{
   TEUNDO_DEBUG("group(string) UNDO");
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   LayerDefSet unselable = PROPC->allUnselectable();
   // get the name of the removed cell
   std::string  cname = getStringValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      tDesign->ungroupThis(tDesign->ungroupPrep(dbLibDir));
       // make sure cname exists ...
      assert(tDesign->checkCell(cname));
      // ... and is not active
      assert(cname != tDesign->activeCellName());
      // gather the parent cells
      laydata::CellDefList parentCells;
      tDesign->collectParentCells(cname, parentCells);
      if (parentCells.empty())
      {
         // if no parent cells - it means that a simple "group" was
         // executed - so use the conventional remove cell
         laydata::TdtCell* rmvdcell = tDesign->removeTopCell(cname,NULL, dbLibDir);
         delete (rmvdcell);
      }
      else
         // parent cells found - so new cell was created on top of an
         // existing library cell or on top of an undefined, but referenced
         // cell. Use remove referenced cells
         tDesign->removeRefdCell(cname, parentCells, NULL, dbLibDir);
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
   delete pl;
}

int tellstdfunc::stdGROUP::execute()
{
   std::string name = getStringValue();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      if (tDesign->groupSelected(name, dbLibDir))
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtString(name));
         UNDOPstack.push_front(make_ttlaylist(tDesign->shapeSel()));
         LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
         UpdateLV(tDesign->numSelected());
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNGROUP::stdUNGROUP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

void tellstdfunc::stdUNGROUP::undo_cleanup()
{
   telldata::TtList* pl1 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl  = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(pl1);
   clean_ttlaylist(pl);
   delete pl;
   delete pl1;
}

void tellstdfunc::stdUNGROUP::undo()
{
   TEUNDO_DEBUG("ungroup() UNDO");
   LayerDefSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // first save the list of all currently selected components
      laydata::SelectList *savelist = tDesign->copySeList();
      // now unselect all
      tDesign->unselectAll();
      // get the list of shapes produced by the ungroup from the UNDO stack
      telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      // now get the list of the ungroupped cell ref's from the UNDO stack
      telldata::TtList* pl1 = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // and add them to the target cell
      tDesign->addList(get_shlaylist(pl1));
      // select the restored cell refs
      tDesign->selectFromList(get_ttlaylist(pl1), unselable);
      // now restore selection
      tDesign->selectFromList(savelist, unselable);
      // and add the list of restored cells to the selection
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      // finally - clean-up behind
      clean_ttlaylist(pl);
      delete pl;
      delete pl1;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdUNGROUP::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::ShapeList* cells4u = tDesign->ungroupPrep(dbLibDir);
      if (cells4u->empty())
      {
         tell_log(console::MT_ERROR,"Nothing to ungroup");
         delete cells4u;
      }
      else
      {
         laydata::AtticList* undol = DEBUG_NEW laydata::AtticList();
         UNDOcmdQ.push_front(this);
         // Push the list of the cells to be ungroupped first
         undol->add(REF_LAY, cells4u);
         UNDOPstack.push_front(make_ttlaylist(undol));
         // and then ungroup and push the list of the shapes produced in
         //result of the ungroup
         laydata::AtticList* undol2 = tDesign->ungroupThis(cells4u);
         UNDOPstack.push_front(make_ttlaylist(undol2));
         // a bit funny, but effective way of cleaning-up cells4u
         // acutually - similar approach was used above to convert cells4u to a
         // layout list - all this using a AtticList structure undol
         clean_atticlist(undol, false);
         delete undol;
         clean_atticlist(undol2, false);
         delete undol2;
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
         UpdateLV(tDesign->numSelected());
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdRENAMECELL::stdRENAMECELL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdRENAMECELL::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   getStringValue(UNDOPstack, false);
}

void tellstdfunc::stdRENAMECELL::undo()
{
   std::string curName  = getStringValue(UNDOPstack, true);
   std::string origName = getStringValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::TdtDefaultCell* targetCell = tDesign->checkCell(curName);
      laydata::TdtDefaultCell* existCell  = tDesign->checkCell(origName);
      assert(NULL != targetCell); // Can't find the cell in the DB
      assert(NULL == existCell);  // Cell with the new name already exists

      tDesign->renameCell(targetCell, origName);
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdRENAMECELL::execute()
{
   std::string newName = getStringValue();
   std::string oldName = getStringValue();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::TdtDefaultCell* targetCell = tDesign->checkCell(oldName);
      laydata::TdtDefaultCell* existCell  = tDesign->checkCell(newName);
      std::stringstream errMsg;
      if (NULL == targetCell)
      {
         errMsg << "Cell \"" << oldName << "\" not found in the database.";
         tell_log(console::MT_ERROR,errMsg.str());
      }
      else if (NULL != existCell)
      {
         errMsg << "Cell \"" << newName << "\" already exists in the database.";
         tell_log(console::MT_ERROR,errMsg.str());
      }
      else
      {
         tDesign->renameCell(targetCell, newName);
         LogFile << LogFile.getFN() << "(\""<< newName << "\",\"" << oldName << "\");"; LogFile.flush();
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtString(oldName));
         UNDOPstack.push_front(DEBUG_NEW telldata::TtString(newName));
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}
