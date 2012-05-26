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
//        Created: Wed Apr 25 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED edit functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tpdf_edit.h"
#include "tedat.h"
#include "datacenter.h"
#include "viewprop.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdCOPYSEL::stdCOPYSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

void tellstdfunc::stdCOPYSEL::undo_cleanup()
{
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete pl;
}

void tellstdfunc::stdCOPYSEL::undo()
{
   TEUNDO_DEBUG("copy(point point) UNDO");
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      //clean up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (pl);
   RefreshGL();
}

int tellstdfunc::stdCOPYSEL::execute()
{
   telldata::TtPnt    *p2 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt    *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(tDesign->shapeSel()));
      tDesign->copySelected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale));
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   }
   delete p1; delete p2;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOPYSEL_D::stdCOPYSEL_D(telldata::typeID retype, bool eor) :
      stdCOPYSEL(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdCOPYSEL_D::execute()
{
   unsigned numSelected = 0;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      numSelected = tDesign->numSelected();
   }
   DATC->unlockTDT(dbLibDir, true);
   if (0 == numSelected)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to copy");
      return EXEC_NEXT;
   }
   else
   {
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(console::op_copy, &OPstack)) return EXEC_ABORT;
      // get the data from the stack
      telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
      OPstack.push(DEBUG_NEW telldata::TtPnt(w->p1().x(), w->p1().y()));
      OPstack.push(DEBUG_NEW telldata::TtPnt(w->p2().x(), w->p2().y()));
      delete w;
      return stdCOPYSEL::execute();
   }
}

//=============================================================================
tellstdfunc::stdMOVESEL::stdMOVESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

void tellstdfunc::stdMOVESEL::undo_cleanup()
{
   telldata::TtPnt*       p1 = TELL_UNDOOPS_CLEAN(telldata::TtPnt*);
   telldata::TtPnt*       p2 = TELL_UNDOOPS_CLEAN(telldata::TtPnt*);
   telldata::TtList*  failed = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* deleted = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList*   added = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(deleted);
   delete added;
   delete deleted;
   delete failed;
   delete p1;
   delete p2;
}

void tellstdfunc::stdMOVESEL::undo()
{
   TEUNDO_DEBUG("move(point point) UNDO");
   telldata::TtList* added = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtList* deleted = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtList* failed = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtPnt    *p2 = TELL_UNDOOPS_UNDO(telldata::TtPnt*);
   telldata::TtPnt    *p1 = TELL_UNDOOPS_UNDO(telldata::TtPnt*);

   real DBscale = PROPC->DBscale();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->unselectFromList(get_ttlaylist(failed), unselable);
      tDesign->unselectFromList(get_ttlaylist(added), unselable);
      laydata::SelectList* fadead[3];
      byte i;
      for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::SelectList());
      tDesign->moveSelected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale),fadead);
      //@TODO Here - an internal check can be done - all 3 of the fadead lists
      // MUST be empty, otherwise - god knows what's wrong!
      for (i = 0; i < 3; delete fadead[i++]);
      tDesign->selectFromList(get_ttlaylist(failed), unselable);
      // put back the replaced (deleted) shapes
      tDesign->addList(get_shlaylist(deleted));
      // and select them
      tDesign->selectFromList(get_ttlaylist(deleted), unselable);
      // delete the added shapes
      for (word j = 0 ; j < added->mlist().size(); j++) {
         tDesign->destroyThis(             static_cast<telldata::TtLayout*>(added->mlist()[j])->data(),
                              tell2DBLayer(static_cast<telldata::TtLayout*>(added->mlist()[j])->layer()),
                              dbLibDir);
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   delete failed;
   delete deleted;
   delete added;
   delete p1; delete p2;
   RefreshGL();
}

int tellstdfunc::stdMOVESEL::execute()
{
   telldata::TtPnt    *p2 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt    *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   // moveSelected returns 3 select lists : Failed/Deleted/Added
   // This is because of the modify operations
   laydata::SelectList* fadead[3];
   for (byte i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::SelectList());
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(p2->selfcopy());
      UNDOPstack.push_front(p1->selfcopy());
      tDesign->moveSelected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale), fadead);
      // save for undo operations ...
      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
      UNDOPstack.push_front(make_ttlaylist(fadead[2])); // and added
      cleanFadeadList(fadead);
      LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   }
   delete p1; delete p2;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdMOVESEL_D::stdMOVESEL_D(telldata::typeID retype, bool eor) :
      stdMOVESEL(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdMOVESEL_D::execute()
{
   unsigned numSelected = 0;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      numSelected = tDesign->numSelected();
   }
   DATC->unlockTDT(dbLibDir, true);
   if (0 == numSelected)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to move");
      return EXEC_NEXT;
   }
   else
   {
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(console::op_move, &OPstack)) return EXEC_ABORT;
      // get the data from the stack
      telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
      OPstack.push(DEBUG_NEW telldata::TtPnt(w->p1().x(), w->p1().y()));
      OPstack.push(DEBUG_NEW telldata::TtPnt(w->p2().x(), w->p2().y()));
      delete w;
      return stdMOVESEL::execute();
   }
}

//=============================================================================
tellstdfunc::stdROTATESEL::stdROTATESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

void tellstdfunc::stdROTATESEL::undo_cleanup()
{
   telldata::TtPnt       *p1 = TELL_UNDOOPS_CLEAN(telldata::TtPnt*);
   getOpValue(UNDOPstack, false);
   telldata::TtList* failed  = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* deleted = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* added   = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(deleted);
   delete added;
   delete deleted;
   delete failed;
   delete p1;
}

void tellstdfunc::stdROTATESEL::undo()
{
   TEUNDO_DEBUG("rotate(point real) UNDO");
   telldata::TtList* added = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtList* deleted = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtList* failed = TELL_UNDOOPS_UNDO(telldata::TtList*);
   real   angle  = 360 - getOpValue(UNDOPstack, true);
   telldata::TtPnt    *p1 = TELL_UNDOOPS_UNDO(telldata::TtPnt*);
   DWordSet unselable = PROPC->allUnselectable();
   real DBscale = PROPC->DBscale();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->unselectFromList(get_ttlaylist(failed), unselable);
      tDesign->unselectFromList(get_ttlaylist(added), unselable);
      laydata::SelectList* fadead[3];
      byte i;
      for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::SelectList());
      tDesign->rotateSelected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      //@TODO Here - an internal check can be done - all 3 of the fadead lists
      // MUST be empty, otherwise - god knows what's wrong!
      for (i = 0; i < 3; delete fadead[i++]);
      tDesign->selectFromList(get_ttlaylist(failed), unselable);
      // put back the replaced (deleted) shapes
      tDesign->addList(get_shlaylist(deleted));
      // and select them
      tDesign->selectFromList(get_ttlaylist(deleted), unselable);
      // delete the added shapes
      for (word j = 0 ; j < added->mlist().size(); j++)
      {
         tDesign->destroyThis(             static_cast<telldata::TtLayout*>(added->mlist()[j])->data(),
                              tell2DBLayer(static_cast<telldata::TtLayout*>(added->mlist()[j])->layer()),
                              dbLibDir);
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   delete failed;
   delete deleted;
   delete added;
   delete p1;
   RefreshGL();
}

int tellstdfunc::stdROTATESEL::execute()
{
   telldata::TtPnt    *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   real   angle  = getOpValue();
   real DBscale = PROPC->DBscale();
   // rotateSelected returns 3 select lists : Failed/Deleted/Added
   // This is because of the box rotation in which case box has to be converted to polygon
   // Failed shapes here should not exist but no explicit check for this
   laydata::SelectList* fadead[3];
   byte i;
   for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::SelectList());
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->rotateSelected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      telldata::TtList* added = make_ttlaylist(fadead[2]);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(added), unselable);
      // save for undo operations ...
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(p1->selfcopy());
      UNDOPstack.push_front(DEBUG_NEW telldata::TtReal(angle));
      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
      UNDOPstack.push_front(added); // and added
      cleanFadeadList(fadead);
      LogFile << LogFile.getFN() << "("<< angle << "," << *p1 << ");"; LogFile.flush();
   }
   delete p1;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdROTATESEL_D::stdROTATESEL_D(telldata::typeID retype, bool eor) :
      stdROTATESEL(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdROTATESEL_D::execute()
{
   real   angle  = getOpValue();
   unsigned numSelected = 0;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      numSelected = tDesign->numSelected();
   }
   DATC->unlockTDT(dbLibDir, true);
   if (0 == numSelected)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to rotate");
      return EXEC_NEXT;
   }
   else
   {
      CTM rct;
      rct.Rotate(angle);
      OPstack.push(DEBUG_NEW telldata::TtReal(angle));
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(console::op_rotate, &OPstack, "", rct)) return EXEC_ABORT;
      return stdROTATESEL::execute();
   }
}

//=============================================================================
tellstdfunc::stdFLIPSEL::stdFLIPSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

void tellstdfunc::stdFLIPSEL::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtPnt    *p1 = TELL_UNDOOPS_CLEAN(telldata::TtPnt*);
   delete p1;
}

void tellstdfunc::stdFLIPSEL::undo()
{
   TEUNDO_DEBUG("flip(direction, point) UNDO");
   telldata::TtPnt    *p1 = TELL_UNDOOPS_UNDO(telldata::TtPnt*);
   word         direction = getWordValue(UNDOPstack, true);
   real           DBscale = PROPC->DBscale();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->flipSelected(TP(p1->x(), p1->y(), DBscale), (1 == direction));
   }
   DATC->unlockTDT(dbLibDir, true);
   delete p1;
   RefreshGL();
}

int tellstdfunc::stdFLIPSEL::execute()
{
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   word      direction = getWordValue();
   real        DBscale = PROPC->DBscale();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->flipSelected(TP(p1->x(), p1->y(), DBscale), (1==direction));
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(direction));
      UNDOPstack.push_front(p1->selfcopy());
      std::string dirstr = (direction == 1) ? "_vertical" : "_horizontal";
      LogFile << LogFile.getFN() << "("<< dirstr
                                 <<"," << *p1 << ");"; LogFile.flush();
   }
   delete p1;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPSEL_D::stdFLIPSEL_D(telldata::typeID retype, bool eor) :
      stdFLIPSEL(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

int tellstdfunc::stdFLIPSEL_D::execute()
{
   unsigned numSelected = 0;
   word      direction = getWordValue();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      numSelected = tDesign->numSelected();
   }
   DATC->unlockTDT(dbLibDir, true);
   if (0 == numSelected)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to flip");
      return EXEC_NEXT;
   }
   else
   {
      OPstack.push(DEBUG_NEW telldata::TtInt(direction));
      console::ACTIVE_OP cop = (1 == direction) ? console::op_flipX : console::op_flipY;
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(cop, &OPstack)) return EXEC_ABORT;
      return stdFLIPSEL::execute();
   }
}


//=============================================================================
tellstdfunc::stdDELETESEL::stdDELETESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

void tellstdfunc::stdDELETESEL::undo_cleanup()
{
   laydata::CellMap* udurcells = static_cast<laydata::CellMap*>(UNDOUstack.back());UNDOUstack.pop_back();
   telldata::TtList* und = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(und);
   delete und;
   for (laydata::CellMap::const_iterator CUDU = udurcells->begin(); CUDU != udurcells->end(); CUDU++)
   {
      delete CUDU->second;
   }
   udurcells->clear();
   delete(udurcells);
}

void tellstdfunc::stdDELETESEL::undo()
{
   TEUNDO_DEBUG("delete() UNDO");
   // get the removed undefined cells (if any)
   telldata::TtList* und = TELL_UNDOOPS_UNDO(telldata::TtList*);
   laydata::CellMap* udurcells = static_cast<laydata::CellMap*>(UNDOUstack.front());UNDOUstack.pop_front();
   std::string prnt_name = "";
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      for (laydata::CellMap::const_iterator CUDU = udurcells->begin(); CUDU != udurcells->end(); CUDU++)
      {
         dbLibDir->addThisUndefCell(CUDU->second);
         TpdPost::treeAddMember(CUDU->second->name().c_str(), prnt_name.c_str(), 0);
      }
      udurcells->clear();
      delete(udurcells);
   //
      tDesign->addList(get_shlaylist(und));
      tDesign->selectFromList(get_ttlaylist(und), unselable);

      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (und);
}

int tellstdfunc::stdDELETESEL::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      UNDOcmdQ.push_front(this);
      laydata::AtticList* sh_delist = DEBUG_NEW laydata::AtticList();
      tDesign->deleteSelected(sh_delist, dbLibDir);
      UNDOPstack.push_front(make_ttlaylist(sh_delist));
      clean_atticlist(sh_delist); delete sh_delist;
      laydata::CellMap* udurCells = DEBUG_NEW laydata::CellMap();
      dbLibDir->getHeldCells(udurCells);
      UNDOUstack.push_front(udurCells);
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY::lgcCUTPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_pnt)));
}

void tellstdfunc::lgcCUTPOLY::undo_cleanup()
{
   telldata::TtList* pl4 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl3 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl2 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl1 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(pl3); // deleted shapes
   delete pl1; delete pl2;
   delete pl3; delete pl4;
}

void tellstdfunc::lgcCUTPOLY::undo()
{
   TEUNDO_DEBUG("cutpoly() UNDO");
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // now unselect all
      tDesign->unselectAll();
      // get the list of cut-offs
      telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      delete pl;
      // now get the list of cuts ...
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      delete pl;
      // now get the list of deleted shapes
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // put them back
      tDesign->addList(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // ... and restore the selection
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      delete pl;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::lgcCUTPOLY::execute()
{
   // get the data from the stack
   telldata::TtList *pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   PointVector *plist = t2tpoints(pl,DBscale);
   laydata::ValidPoly check(*plist);
   delete plist;
   if (!check.valid()) {
      tell_log(console::MT_ERROR, "Invalid cutting polygon encountered");
   }
   else
   {
      //cutPoly returns 3 Attic lists -> Delete/AddSelect/AddOnly,
      // create and initialize them here
      PointVector theShape = check.getValidated();
      DWordSet unselable = PROPC->allUnselectable();
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         if (0 == tDesign->numSelected())
            tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
         else
         {
            laydata::AtticList* dasao[3];
            for (byte i = 0; i < 3; dasao[i++] = DEBUG_NEW laydata::AtticList());
            if (tDesign->cutPoly( theShape, dasao))
            {
               // push the command for undo
               UNDOcmdQ.push_front(this);
               UNDOPstack.push_front(make_ttlaylist(tDesign->shapeSel()));
               // unselect everything
               tDesign->unselectAll();

               telldata::TtList* shdeleted = make_ttlaylist(dasao[0]);
               // select the shapes to delete & delete them ...
               tDesign->selectFromList(get_ttlaylist(shdeleted), unselable);
               laydata::AtticList* sh_delist = DEBUG_NEW laydata::AtticList();
               tDesign->deleteSelected(sh_delist, dbLibDir);
               // ... not forgetting to save them in the undo data stack for undo
               UNDOPstack.push_front(make_ttlaylist(sh_delist));
               // clean-up the delete attic list
               clean_atticlist(sh_delist); delete sh_delist;
               delete shdeleted;

               // add the result of the cut...
               telldata::TtList* shaddselect = make_ttlaylist(dasao[1]);
               telldata::TtList* shaddonly = make_ttlaylist(dasao[2]);
               tDesign->addList(dasao[1]);
               UNDOPstack.push_front(shaddselect);
               // ... the cut-offs ....
               tDesign->addList(dasao[2]);
               UNDOPstack.push_front(shaddonly);
               // and finally select the_cut
               tDesign->selectFromList(get_ttlaylist(shaddselect), unselable);
               LogFile << "polycut("<< *pl << ");"; LogFile.flush();
               clean_atticlist(dasao[0]); delete (dasao[0]);
               // delete dasao[1]; delete dasao[2]; - deleted by tDesign->addList
               UpdateLV(tDesign->numSelected());
            }
            else
            {
               for (byte i = 0; i < 3; delete dasao[i++]);
               RefreshGL();
            }
         }
      }
      DATC->unlockTDT(dbLibDir, true);
   }
   delete pl;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY_I::lgcCUTPOLY_I(telldata::typeID retype, bool eor) :
      lgcCUTPOLY(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::lgcCUTPOLY_I::execute()
{
   unsigned numSelected = 0;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      numSelected = tDesign->numSelected();
   }
   DATC->unlockTDT(dbLibDir, true);
   if (0 == numSelected)
   {
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
      return EXEC_NEXT;
   }
   else
   {
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(console::op_dpoly, &OPstack)) return EXEC_ABORT;
      return lgcCUTPOLY::execute();
   }
}

//=============================================================================
tellstdfunc::lgcCUTBOX_I::lgcCUTBOX_I(telldata::typeID retype, bool eor) :
      lgcCUTPOLY(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::lgcCUTBOX_I::execute()
{
   unsigned numSelected = 0;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      numSelected = tDesign->numSelected();
   }
   DATC->unlockTDT(dbLibDir, true);
   if (0 == numSelected)
   {
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
      return EXEC_NEXT;
   }
   else
   {
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
      telldata::TtWnd *bx = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();

      telldata::TtList *pl = DEBUG_NEW telldata::TtList(telldata::tn_pnt);
      pl->add(DEBUG_NEW telldata::TtPnt(bx->p1().x(), bx->p1().y()));
      pl->add(DEBUG_NEW telldata::TtPnt(bx->p1().x(), bx->p2().y()));
      pl->add(DEBUG_NEW telldata::TtPnt(bx->p2().x(), bx->p2().y()));
      pl->add(DEBUG_NEW telldata::TtPnt(bx->p2().x(), bx->p1().y()));
      OPstack.push(pl);
      delete bx;
      return lgcCUTPOLY::execute();
   }
}

//=============================================================================
tellstdfunc::lgcMERGE::lgcMERGE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

void tellstdfunc::lgcMERGE::undo_cleanup()
{
   telldata::TtList* pl3 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl2 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl1 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(pl2);
   delete pl1; delete pl2;
   delete pl3;
}

void tellstdfunc::lgcMERGE::undo()
{
   TEUNDO_DEBUG("merge() UNDO");
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // now unselect all
      tDesign->unselectAll();
      // get the shapes resulted from the merge operation
      telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      delete pl;
      // now get the list of deleted shapes
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // put them back
      tDesign->addList(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // ... and restore the selection
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      delete pl;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::lgcMERGE::execute()
{
   //merge returns 2 Attic lists -> Delete/AddMerged
   // create and initialize them here
   laydata::AtticList* dasao[2];
   byte i;
   for (i = 0; i < 2; dasao[i++] = DEBUG_NEW laydata::AtticList());
   // create a list of currently selected shapes
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtList* listselected = make_ttlaylist(tDesign->shapeSel());
      if (0 == listselected->size())
      {
         tell_log(console::MT_ERROR,"No objects selected. Nothing to merge");
         delete listselected;
      }
      else if (tDesign->merge(dasao))
      {
         // push the command for undo
         UNDOcmdQ.push_front(this);
         // save the list of originally selected shapes
         UNDOPstack.push_front(listselected);
         // save the list of deleted shapes
         UNDOPstack.push_front(make_ttlaylist(dasao[0]));
         // add the result of the merge...
         UNDOPstack.push_front(make_ttlaylist(dasao[1]));
         LogFile << "merge( );"; LogFile.flush();
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         delete listselected;
      }
   }
   // clean-up the lists
   for (i = 0; i < 2; i++)
   {
      clean_atticlist(dasao[i]);
      delete(dasao[i]);
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}



//=============================================================================
tellstdfunc::lgcSTRETCH::lgcSTRETCH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

void tellstdfunc::lgcSTRETCH::undo_cleanup()
{
   telldata::TtList* pl3 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl2 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl1 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(pl3); // deleted shapes
   delete pl1; delete pl2;
   delete pl3;
}

void tellstdfunc::lgcSTRETCH::undo()
{
   TEUNDO_DEBUG("bloat() UNDO");
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // now unselect all
      tDesign->unselectAll();
      // now get the list of cuts ...
      telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      delete pl;
      // now get the list of deleted shapes
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // put them back
      tDesign->addList(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // ... and restore the selection
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      delete pl;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::lgcSTRETCH::execute()
{
   real bfactor = getOpValue();
   if (0.0 == bfactor)
   {
      tell_log(console::MT_WARNING,"Resize argument is 0. Nothing was changed");
   }
   else
   {
      //expand/shrink returns 2 Attic lists -> Delete/AddSelect,
      // create and initialize them here
      laydata::AtticList* dasao[2];
      DWordSet unselable = PROPC->allUnselectable();
      for (byte i = 0; i < 2; dasao[i++] = DEBUG_NEW laydata::AtticList());
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         real DBscale = PROPC->DBscale();
         if (0 == tDesign->numSelected())
         {
            tell_log(console::MT_ERROR,"No object selected. Nothing to modify");
            for (byte i = 0; i < 2; delete dasao[i++]);
         }
         else if (tDesign->stretch((int) rint(bfactor * DBscale), dasao))
         {
            // push the command for undo
            UNDOcmdQ.push_front(this);
            // put the list of selected shapes in undo stack
            UNDOPstack.push_front(make_ttlaylist(tDesign->shapeSel()));
            // unselect everything
            tDesign->unselectAll();

            telldata::TtList* shdeleted = make_ttlaylist(dasao[0]);
            // select the shapes to delete & delete them ...
            tDesign->selectFromList(get_ttlaylist(shdeleted), unselable);
            laydata::AtticList* sh_delist = DEBUG_NEW laydata::AtticList();
            tDesign->deleteSelected(sh_delist, dbLibDir);
            // ... not forgetting to save them in the undo data stack for undo
            UNDOPstack.push_front(make_ttlaylist(sh_delist));
            // clean-up the delete attic list
            clean_atticlist(sh_delist); delete sh_delist;
            delete shdeleted;

            // add the result of the expand/shrink...
            telldata::TtList* shaddselect = make_ttlaylist(dasao[1]);
            tDesign->addList(dasao[1]);
            UNDOPstack.push_front(shaddselect);
            // and finally select the_cut
            tDesign->selectFromList(get_ttlaylist(shaddselect), unselable);
            LogFile << "resize("<< bfactor << ");"; LogFile.flush();
            clean_atticlist(dasao[0]); delete (dasao[0]);
            // delete dasao[1]; delete dasao[2]; - deleted by tDesign->addList
            UpdateLV(tDesign->numSelected());
         }
         else
         {
            for (byte i = 0; i < 2; delete dasao[i++]);
         }
      }
      DATC->unlockTDT(dbLibDir, true);
   }
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdCHANGELAY::stdCHANGELAY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdCHANGELAY::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete pl;
}

void tellstdfunc::stdCHANGELAY::undo()
{
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   word src = getWordValue(UNDOPstack, true);
   secureLayDef(tell2DBLayer(src));
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->transferLayer(get_ttlaylist(pl), tell2DBLayer(src));
   }
   DATC->unlockTDT(dbLibDir, true);
   delete pl;
   RefreshGL();
}

int tellstdfunc::stdCHANGELAY::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::SelectList *listselected = tDesign->shapeSel();
      if (listselected->empty())
      {
         std::ostringstream ost;
         ost << "No objects selected";
         tell_log(console::MT_ERROR,ost.str());
      }
      else
      {
         word target = getWordValue();
         secureLayDef(tell2DBLayer(target));
         tDesign->transferLayer(tell2DBLayer(target));
         // prepare undo stacks
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(target));
         UNDOPstack.push_front(make_ttlaylist(listselected));
         LogFile << "changelayer("<< target << ");";LogFile.flush();
         RefreshGL();
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCHANGEREF::stdCHANGEREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdCHANGEREF::undo_cleanup()
{
   telldata::TtList* pl1 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete pl;
   delete pl1;
}

void tellstdfunc::stdCHANGEREF::undo()
{
   TEUNDO_DEBUG("ungroup() CHANGEREF");
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // first save the list of all currently selected components
      laydata::SelectList *savelist = tDesign->copySeList();
      // now unselect all
      tDesign->unselectAll();
      // get the list of new references from the UNDO stack
      telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      // now get the list of the old cell ref's from the UNDO stack
      telldata::TtList* pl1 = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // and add them to the target cell
      tDesign->addList(get_shlaylist(pl1));
      // select the restored cell refs
      tDesign->selectFromList(get_ttlaylist(pl1), unselable);
      // now restore selection
      tDesign->selectFromList(savelist, unselable);
      // finally - clean-up behind
      delete pl;
      delete pl1;
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdCHANGEREF::execute()
{
   std::string newref = getStringValue();
   laydata::ShapeList* cells4u = NULL;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      bool refok = tDesign->checkValidRef(newref);
      if (refok)
      {
         cells4u = tDesign->ungroupPrep(dbLibDir);
         if (cells4u->empty())
         {
            tell_log(console::MT_ERROR,"No cell references selected");
            delete cells4u;
         }
         else
         {
            laydata::AtticList* undol2 = tDesign->changeRef(cells4u, newref);
            assert(NULL != undol2);
            UNDOcmdQ.push_front(this);
            // Push the list of the cells to be ungroupped first
            laydata::AtticList undol;
            undol[0] = cells4u;
            UNDOPstack.push_front(make_ttlaylist(&undol));
            UNDOPstack.push_front(make_ttlaylist(undol2));
            delete cells4u;
            delete undol2;
            LogFile << LogFile.getFN() << "(\"" << newref << "\");"; LogFile.flush();
            RefreshGL();
         }
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdCHANGESTRING::stdCHANGESTRING(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdCHANGESTRING::undo_cleanup()
{
   telldata::TtList* pl1 = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   clean_ttlaylist(pl1);delete pl;
   delete pl1;
}

void tellstdfunc::stdCHANGESTRING::undo()
{
   TEUNDO_DEBUG("ungroup() CHANGESTR");
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // first save the list of all currently selected components
      laydata::SelectList *savelist = tDesign->copySeList();
      // now unselect all
      tDesign->unselectAll();
      // get the list of new texts from the UNDO stack
      telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // select them ...
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      //... and delete them cleaning up the memory (don't store in the Attic)
      tDesign->deleteSelected(NULL, dbLibDir);
      // now get the list of the old text objects from the UNDO stack
      telldata::TtList* pl1 = TELL_UNDOOPS_UNDO(telldata::TtList*);
      // and add them to the target cell
      tDesign->addList(get_shlaylist(pl1));
      // select the restored cell refs
      tDesign->selectFromList(get_ttlaylist(pl1), unselable);
      // now restore selection
      tDesign->selectFromList(savelist, unselable);
      // finally - clean-up behind
      delete pl;
      delete pl1;
   }
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
}

int tellstdfunc::stdCHANGESTRING::execute()
{
   std::string newstring = getStringValue();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      // first save the list of all currently selected components ...
      laydata::SelectList* savelist = tDesign->copySeList();
      // get a list of selected texts only
      laydata::SelectList* texts4u = filter_selist(savelist, laydata::_lmtext);
      if (texts4u->empty())
      {
         for (laydata::SelectList::const_iterator CL = savelist->begin(); CL != savelist->end(); CL++)
            delete CL->second;
         delete savelist;
         for (laydata::SelectList::const_iterator CL = texts4u->begin(); CL != texts4u->end(); CL++)
            delete CL->second;
         delete texts4u;
         tell_log(console::MT_ERROR,"No text objects selected");
      }
      else
      {// just if we have selected texts
         UNDOcmdQ.push_front(this);
         // now unselect all ...
         tDesign->unselectAll();
         // ... and select back only text shapes
         tDesign->selectFromList(texts4u, unselable);
         // delete them from the DB - get back the list of deleted shapes.
         laydata::AtticList* fha = DEBUG_NEW laydata::AtticList();
         tDesign->deleteSelected(fha, dbLibDir);
         // save the deleted shapes in the UNDO data stack
         UNDOPstack.push_front(make_ttlaylist(fha));
         // replace the strings
         laydata::AtticList* fhba = replace_str(fha, newstring);
         telldata::TtList* fhb = make_ttlaylist(fhba);
         // save the new texts in the UNDO data stack
         UNDOPstack.push_front(fhb);
         // add the new objects back to the DB
         tDesign->addList(get_shlaylist(fhb));
         // now restore selection
         tDesign->selectFromList(savelist, unselable);
         tDesign->selectFromList(get_ttlaylist(fhb), unselable);
         // that's it!
         clean_atticlist(fha); delete fha;
         clean_atticlist(fhba);delete fhba;
         LogFile << LogFile.getFN() << "(\"" << newstring << "\");"; LogFile.flush();
         RefreshGL();
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}
