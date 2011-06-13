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
#include "datacenter.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;
extern void tellerror(std::string s);

//=============================================================================
tellstdfunc::stdGETLAYTYPE::stdGETLAYTYPE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlayout()));
}

int tellstdfunc::stdGETLAYTYPE::execute()
{
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   OPstack.push(DEBUG_NEW telldata::ttint(tx->data()->lType()));
   delete tx;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGETLAYTEXTSTR::stdGETLAYTEXTSTR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlayout()));
}

int tellstdfunc::stdGETLAYTEXTSTR::execute()
{
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   if (laydata::_lmtext != tx->data()->lType())
   {
      tellerror("Runtime error.Invalid layout type");
      delete tx;
      return EXEC_ABORT;
   }
   else
   {
      OPstack.push(DEBUG_NEW telldata::ttstring(static_cast<laydata::TdtText*>(tx->data())->text()));
      delete tx;
      return EXEC_NEXT;
   }
}

//=============================================================================
tellstdfunc::stdGETLAYREFSTR::stdGETLAYREFSTR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlayout()));
}

int tellstdfunc::stdGETLAYREFSTR::execute()
{
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   if ((laydata::_lmref != tx->data()->lType()) && (laydata::_lmaref != tx->data()->lType()))
   {
      tellerror("Runtime error.Invalid layout type");
      delete tx;
      return EXEC_ABORT;
   }
   else
   {
      OPstack.push(DEBUG_NEW telldata::ttstring(static_cast<laydata::TdtCellRef*>(tx->data())->cellname()));
      delete tx;
      return EXEC_NEXT;
   }
}

//=============================================================================
tellstdfunc::stdGETGRCLAYERS::stdGETGRCLAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
}

int tellstdfunc::stdGETGRCLAYERS::execute()
{
   telldata::ttlist* tllull = DEBUG_NEW telldata::ttlist(telldata::tn_int);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      DWordSet grcLays;
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      auxdata::GrcCell* grcCell = tDesign->getGrcCell();
      if (NULL != grcCell)
      {
         grcCell->reportLayers(grcLays);
         for (DWordSet::const_iterator CL = grcLays.begin(); CL != grcLays.end(); CL++)
            tllull->add(DEBUG_NEW telldata::ttint(*CL));
      }
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   OPstack.push(tllull);
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGETGRCDATA::stdGETGRCDATA(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

int tellstdfunc::stdGETGRCDATA::execute()
{
   word     la = getWordValue();
   telldata::ttlist* llist = DEBUG_NEW telldata::ttlist(telldata::tn_auxilary);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      auxdata::AuxDataList dataList;
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      auxdata::GrcCell* grcCell = tDesign->getGrcCell();
      if (NULL != grcCell)
      {
         grcCell->reportLayData(la,dataList);
         for (auxdata::AuxDataList::const_iterator CD = dataList.begin(); CD != dataList.end(); CD++)
            llist->add(DEBUG_NEW telldata::ttauxdata(*CD, la));
      }
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   OPstack.push(llist);
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}
