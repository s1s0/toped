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
//    Description: Definition of all TOPED select/unselect functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "tpdf_select.h"
#include "datacenter.h"
#include "viewprop.h"


extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdSELECT::stdSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdSELECT::undo_cleanup()
{
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdSELECT::undo()
{
   TEUNDO_DEBUG("select(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->unselectInBox(p1DB, p2DB, unselable, false);
      delete p1DB; delete p2DB;
      UpdateLV(tDesign->numSelected());
   }
   delete w;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdSELECT::execute()
{
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectInBox(p1DB, p2DB, unselable, PROPC->layselmask(), false);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(w);
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      delete p1DB; delete p2DB;
      LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
      UpdateLV(tDesign->numSelected());
   }
   //DONT delete w; - undo will delete it
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSELECT_I::stdSELECT_I(telldata::typeID retype, bool eor) :
      stdSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECT_TL::stdSELECT_TL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_layout)));
}

void tellstdfunc::stdSELECT_TL::undo_cleanup()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   delete pl;
}

void tellstdfunc::stdSELECT_TL::undo() {
}

int tellstdfunc::stdSELECT_TL::execute()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      UpdateLV(tDesign->numSelected());
   }
   delete pl;
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSELECTIN::stdSELECTIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdSELECTIN::undo_cleanup()
{
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected;
}

void tellstdfunc::stdSELECTIN::undo()
{
   TEUNDO_DEBUG("select(point) UNDO");
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselable = PROPC->allUnselectable();
      laydata::SelectList* slst = get_ttlaylist(selected);
      tDesign->unselectFromList(slst, unselable);
      delete slst;
      UpdateLV(tDesign->numSelected());
   }
   delete selected;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdSELECTIN::execute()
{
   // get the data from the stack
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselectable = PROPC->allUnselectable();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
      laydata::AtticList* selectedl = tDesign->changeSelect(p1DB, unselectable, true);
      if (NULL != selectedl)
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(make_ttlaylist(selectedl));
         OPstack.push(make_ttlaylist(selectedl));
         LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
         for(laydata::AtticList::iterator CI = selectedl->begin();CI != selectedl->end(); CI++)
            delete CI->second;
         delete selectedl;
         UpdateLV(tDesign->numSelected());
      }
      delete p1DB;
   }
   delete p1;
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTSELECT::stdPNTSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdPNTSELECT::undo_cleanup()
{
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTSELECT::undo() {
   TEUNDO_DEBUG("pselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->unselectInBox(p1DB, p2DB, unselable, true);
      delete p1DB; delete p2DB;
      UpdateLV(tDesign->numSelected());
   }
   delete w;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdPNTSELECT::execute()
{
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectInBox(p1DB,  p2DB, unselable, PROPC->layselmask(), true);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(w);
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
      delete p1DB; delete p2DB;
      UpdateLV(tDesign->numSelected());
   }
   //DONT delete w; - undo will delete it
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTSELECT_I::stdPNTSELECT_I(telldata::typeID retype, bool eor) :
      stdPNTSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdPNTSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   return stdPNTSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT::stdUNSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdUNSELECT::undo_cleanup()
{
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdUNSELECT::undo()
{
   TEUNDO_DEBUG("unselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectInBox(p1DB, p2DB, unselable, PROPC->layselmask(), false);
      delete p1DB; delete p2DB;
      UpdateLV(tDesign->numSelected());
   }
   delete w;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdUNSELECT::execute()
{
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->unselectInBox(p1DB, p2DB, unselable, false);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(w);
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
      delete p1DB; delete p2DB;
      UpdateLV(tDesign->numSelected());
   }
   //DONT delete w; - undo will delete it
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECT_I::stdUNSELECT_I(telldata::typeID retype, bool eor) :
      stdUNSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT_TL::stdUNSELECT_TL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_layout)));
}

void tellstdfunc::stdUNSELECT_TL::undo() {
}

void tellstdfunc::stdUNSELECT_TL::undo_cleanup()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   delete pl;
}

int tellstdfunc::stdUNSELECT_TL::execute()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselable = PROPC->allUnselectable();
      laydata::SelectList* slst = get_ttlaylist(pl);
      tDesign->unselectFromList(slst, unselable);
      delete slst;
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECTIN::stdUNSELECTIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdUNSELECTIN::undo_cleanup()
{
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected;
}

void tellstdfunc::stdUNSELECTIN::undo()
{
   TEUNDO_DEBUG("unselect(point) UNDO");
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(selected), unselable);
      UpdateLV(tDesign->numSelected());
   }
   delete selected;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdUNSELECTIN::execute()
{
   // get the data from the stack
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
      DWordSet unselectable = PROPC->allUnselectable();
      laydata::AtticList* selectedl = tDesign->changeSelect(p1DB,unselectable,false);
      delete p1DB;
      if (NULL != selectedl)
      {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(make_ttlaylist(selectedl));
         OPstack.push(make_ttlaylist(selectedl));
         LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
         delete selectedl;
         UpdateLV(tDesign->numSelected());
      }
   }
   delete p1;
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTUNSELECT::stdPNTUNSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdPNTUNSELECT::undo_cleanup()
{
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTUNSELECT::undo()
{
   TEUNDO_DEBUG("punselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectInBox(p1DB, p2DB, unselable, PROPC->layselmask(), true);
      delete p1DB; delete p2DB;
      UpdateLV(tDesign->numSelected());
   }
   delete w;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdPNTUNSELECT::execute()
{
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      real DBscale = PROPC->DBscale();
      TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
      TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->unselectInBox(p1DB, p2DB, unselable, true);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(w);
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      delete p1DB; delete p2DB;
      LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
      UpdateLV(tDesign->numSelected());
   }
   //DON'T delete w; - undo will delete it
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTUNSELECT_I::stdPNTUNSELECT_I(telldata::typeID retype, bool eor) :
      stdPNTUNSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdPNTUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdPNTUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECTALL::stdSELECTALL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdSELECTALL::undo_cleanup()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdSELECTALL::undo()
{
   TEUNDO_DEBUG("select_all() UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->unselectAll();
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      UpdateLV(tDesign->numSelected());
   }
   delete pl;
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdSELECTALL::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselectable = PROPC->allUnselectable();
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(tDesign->shapeSel()));
      tDesign->selectAll(unselectable, PROPC->layselmask());
      OPstack.push(make_ttlaylist(tDesign->shapeSel()));
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECTALL::stdUNSELECTALL(telldata::typeID retype, bool eor) :
                               cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdUNSELECTALL::undo_cleanup()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdUNSELECTALL::undo()
{
   TEUNDO_DEBUG("unselect_all() UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DWordSet unselable = PROPC->allUnselectable();
      tDesign->selectFromList(get_ttlaylist(pl), unselable);
      UpdateLV(tDesign->numSelected());
   }
   delete (pl);
   DATC->unlockTDT(dbLibDir, true);
}

int tellstdfunc::stdUNSELECTALL::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(tDesign->shapeSel()));
      tDesign->unselectAll();
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
      UpdateLV(tDesign->numSelected());
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTSLCTD::stdREPORTSLCTD(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdREPORTSLCTD::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      if (0 == tDesign->numSelected())
         tell_log(console::MT_ERROR,"No objects selected.");
      else
         tDesign->reportSelected(PROPC->DBscale());
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdSETSELECTMASK::stdSETSELECTMASK(telldata::typeID retype, bool eor) :
                               cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdSETSELECTMASK::undo_cleanup()
{
   getWordValue(UNDOPstack,false);
}

void tellstdfunc::stdSETSELECTMASK::undo()
{
   TEUNDO_DEBUG("setselectmask() UNDO");
   word mask = getWordValue(UNDOPstack,true);
   PROPC->setLaySelMask(mask);
//   UpdateLV();
}

int tellstdfunc::stdSETSELECTMASK::execute()
{
   UNDOcmdQ.push_front(this);
   word mask = getWordValue();
   word oldmask = PROPC->layselmask();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(oldmask));
   PROPC->setLaySelMask(mask);
   OPstack.push(DEBUG_NEW telldata::ttint(oldmask));
   LogFile << LogFile.getFN() << "("<< mask <<");"; LogFile.flush();
   return EXEC_NEXT;
}

