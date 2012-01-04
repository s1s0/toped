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
//    Description: Definition of all TOPED database functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tpdf_get.h"
#include "tedat.h"
#include "auxdat.h"
#include "datacenter.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;
extern void tellerror(std::string s);

//=============================================================================
tellstdfunc::stdGETLAYTYPE::stdGETLAYTYPE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayout()));
}

int tellstdfunc::stdGETLAYTYPE::execute()
{
   telldata::TtLayout* tx = static_cast<telldata::TtLayout*>(OPstack.top());OPstack.pop();
   OPstack.push(DEBUG_NEW telldata::TtInt(tx->data()->lType()));
   delete tx;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGETLAYTEXTSTR::stdGETLAYTEXTSTR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayout()));
}

int tellstdfunc::stdGETLAYTEXTSTR::execute()
{
   telldata::TtLayout* tx = static_cast<telldata::TtLayout*>(OPstack.top());OPstack.pop();
   if (laydata::_lmtext != tx->data()->lType())
   {
      tellerror("Runtime error.Invalid layout type"); // FIXME?! tellerror is a parser function. MUST not be used during runtime
      delete tx;
      return EXEC_ABORT;
   }
   else
   {
      OPstack.push(DEBUG_NEW telldata::TtString(static_cast<laydata::TdtText*>(tx->data())->text()));
      delete tx;
      return EXEC_NEXT;
   }
}

//=============================================================================
tellstdfunc::stdGETLAYREFSTR::stdGETLAYREFSTR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayout()));
}

int tellstdfunc::stdGETLAYREFSTR::execute()
{
   telldata::TtLayout* tx = static_cast<telldata::TtLayout*>(OPstack.top());OPstack.pop();
   if ((laydata::_lmref != tx->data()->lType()) && (laydata::_lmaref != tx->data()->lType()))
   {
      tellerror("Runtime error.Invalid layout type"); // FIXME?! tellerror is a parser function. MUST not be used during runtime
      delete tx;
      return EXEC_ABORT;
   }
   else
   {
      OPstack.push(DEBUG_NEW telldata::TtString(static_cast<laydata::TdtCellRef*>(tx->data())->cellname()));
      delete tx;
      return EXEC_NEXT;
   }
}

//=============================================================================
tellstdfunc::grcGETLAYERS::grcGETLAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
}

int tellstdfunc::grcGETLAYERS::execute()
{
   telldata::TtList* tllull = DEBUG_NEW telldata::TtList(telldata::tn_int);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      DWordSet grcLays;
      laydata::TdtCell*   tCell   = (*dbLibDir)()->targetECell();
      auxdata::GrcCell* grcCell   = tCell->getGrcCell();
      if (NULL != grcCell)
      {
         grcCell->reportLayers(grcLays);
         for (DWordSet::const_iterator CL = grcLays.begin(); CL != grcLays.end(); CL++)
            tllull->add(DEBUG_NEW telldata::TtInt(*CL));
      }
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   OPstack.push(tllull);
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::grcGETDATA::grcGETDATA(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

int tellstdfunc::grcGETDATA::execute()
{
   word     la = getWordValue();
   telldata::TtList* llist = DEBUG_NEW telldata::TtList(telldata::tn_auxilary);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      auxdata::AuxDataList dataList;
      laydata::TdtCell*   tCell   = (*dbLibDir)()->targetECell();
      auxdata::GrcCell* grcCell   = tCell->getGrcCell();
      if (NULL != grcCell)
      {
         grcCell->reportLayData(la,dataList);
         for (auxdata::AuxDataList::const_iterator CD = dataList.begin(); CD != dataList.end(); CD++)
            llist->add(DEBUG_NEW telldata::TtAuxdata(*CD, la));
      }
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   OPstack.push(llist);
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::grcCLEANALAYER::grcCLEANALAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

int tellstdfunc::grcCLEANALAYER::execute()
{
   word     la = getWordValue();
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::TdtCell*   tCell   = tDesign->targetECell();
      auxdata::GrcCell* grcCell   = tCell->getGrcCell();
      if (NULL != grcCell)
      {
         bool cleanCell = grcCell->cleanLay(la);
         if (cleanCell)
         {
            tCell->clearGrcCell();
            TpdPost::treeMarkGrcMember(tDesign->activeCellName().c_str(), false);
         }
      }
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::grcREPAIRDATA::grcREPAIRDATA(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::grcREPAIRDATA::undo_cleanup()
{
//   telldata::TtPnt    *p1 = static_cast<telldata::TtPnt*>(UNDOPstack.back());UNDOPstack.pop_back();
//   telldata::TtPnt    *p2 = static_cast<telldata::TtPnt*>(UNDOPstack.back());UNDOPstack.pop_back();
//   telldata::TtList* failed = static_cast<telldata::TtList*>(UNDOPstack.back());UNDOPstack.pop_back();
//   telldata::TtList* deleted = static_cast<telldata::TtList*>(UNDOPstack.back());UNDOPstack.pop_back();
//   telldata::TtList* added = static_cast<telldata::TtList*>(UNDOPstack.back());UNDOPstack.pop_back();
//   clean_ttlaylist(deleted);
//   delete added;
//   delete deleted;
//   delete failed;
//   delete p1;
//   delete p2;
}

void tellstdfunc::grcREPAIRDATA::undo()
{
//   TEUNDO_DEBUG("move(point point) UNDO");
//   telldata::TtList* added = static_cast<telldata::TtList*>(UNDOPstack.front());UNDOPstack.pop_front();
//   telldata::TtList* deleted = static_cast<telldata::TtList*>(UNDOPstack.front());UNDOPstack.pop_front();
//   telldata::TtList* failed = static_cast<telldata::TtList*>(UNDOPstack.front());UNDOPstack.pop_front();
//   telldata::TtPnt    *p2 = static_cast<telldata::TtPnt*>(UNDOPstack.front());UNDOPstack.pop_front();
//   telldata::TtPnt    *p1 = static_cast<telldata::TtPnt*>(UNDOPstack.front());UNDOPstack.pop_front();
//
//   real DBscale = PROPC->DBscale();
//   DWordSet unselable = PROPC->allUnselectable();
//   laydata::TdtLibDir* dbLibDir = NULL;
//   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
//   {
//      laydata::TdtDesign* tDesign = (*dbLibDir)();
//      tDesign->unselectFromList(get_ttlaylist(failed), unselable);
//      tDesign->unselectFromList(get_ttlaylist(added), unselable);
//      laydata::SelectList* fadead[3];
//      byte i;
//      for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::SelectList());
//      tDesign->moveSelected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale),fadead);
//      //@TODO Here - an internal check can be done - all 3 of the fadead lists
//      // MUST be empty, otherwise - god knows what's wrong!
//      for (i = 0; i < 3; delete fadead[i++]);
//      tDesign->selectFromList(get_ttlaylist(failed), unselable);
//      // put back the replaced (deleted) shapes
//      tDesign->addList(get_shlaylist(deleted));
//      // and select them
//      tDesign->selectFromList(get_ttlaylist(deleted), unselable);
//      // delete the added shapes
//      for (word j = 0 ; j < added->mlist().size(); j++) {
//         tDesign->destroyThis(static_cast<telldata::TtLayout*>(added->mlist()[j])->data(),
//                              static_cast<telldata::TtLayout*>(added->mlist()[j])->layer(),
//                              dbLibDir);
//      }
//   }
//   DATC->unlockTDT(dbLibDir, true);
//   delete failed;
//   delete deleted;
//   delete added;
//   delete p1; delete p2;
//   RefreshGL();
}

int tellstdfunc::grcREPAIRDATA::execute()
{
   word     la = getWordValue();
//   telldata::TtList* llist = DEBUG_NEW telldata::TtList(telldata::tn_auxilary);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
//      auxdata::AuxDataList dataList;
      laydata::TdtCell*   tCell   = (*dbLibDir)()->targetECell();
      auxdata::GrcCell* grcCell   = tCell->getGrcCell();
      if (NULL != grcCell)
      {
         laydata::ShapeList newData;
         grcCell->repairData(la, newData);
//         grcCell->reportLayData(la,dataList);
//         for (auxdata::AuxDataList::const_iterator CD = dataList.begin(); CD != dataList.end(); CD++)
//            llist->add(DEBUG_NEW telldata::TtAuxdata(*CD, la));
      }
      LogFile << LogFile.getFN() << "(" << la << ");"; LogFile.flush();
   }
//   OPstack.push(llist);
   DATC->unlockTDT(dbLibDir, true);

//   telldata::TtPnt    *p2 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
//   telldata::TtPnt    *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
//   real DBscale = PROPC->DBscale();
//   // moveSelected returns 3 select lists : Failed/Deleted/Added
//   // This is because of the modify operations
//   laydata::SelectList* fadead[3];
//   for (byte i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::SelectList());
//   laydata::TdtLibDir* dbLibDir = NULL;
//   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
//   {
//      laydata::TdtDesign* tDesign = (*dbLibDir)();
//      UNDOcmdQ.push_front(this);
//      UNDOPstack.push_front(p2->selfcopy());
//      UNDOPstack.push_front(p1->selfcopy());
//      tDesign->moveSelected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale), fadead);
//      // save for undo operations ...
//      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
//      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
//      UNDOPstack.push_front(make_ttlaylist(fadead[2])); // and added
//      cleanFadeadList(fadead);
//      LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
//   }
//   delete p1; delete p2;
//   DATC->unlockTDT(dbLibDir, true);
//   RefreshGL();
   return EXEC_NEXT;
}
