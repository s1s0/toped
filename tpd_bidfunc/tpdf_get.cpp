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
#include "tpdf_get.h"
#include <sstream>
//#include "../tpd_DB/datacenter.h"
//#include "../tpd_common/tuidefs.h"
//#include "../tpd_DB/browsers.h"

//extern DataCenter*               DATC;
//extern console::toped_logfile    LogFile;
extern void tellerror(std::string s);

//=============================================================================
tellstdfunc::stdGETLAYTYPE::stdGETLAYTYPE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlayout()));
}

int tellstdfunc::stdGETLAYTYPE::execute()
{
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   OPstack.push(new telldata::ttint(tx->data()->ltype()));
   delete tx;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGETLAYTEXTSTR::stdGETLAYTEXTSTR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlayout()));
}

int tellstdfunc::stdGETLAYTEXTSTR::execute()
{
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   if (laydata::_lmtext != tx->data()->ltype())
   {
      tellerror("Runtime error.Invalid layout type");
      delete tx;
      return EXEC_ABORT;
   }
   else
   {
      OPstack.push(new telldata::ttstring(static_cast<laydata::tdttext*>(tx->data())->text()));
      delete tx;
      return EXEC_NEXT;
   }
}

//=============================================================================
tellstdfunc::stdGETLAYREFSTR::stdGETLAYREFSTR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlayout()));
}

int tellstdfunc::stdGETLAYREFSTR::execute()
{
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   if ((laydata::_lmref != tx->data()->ltype()) && (laydata::_lmaref != tx->data()->ltype()))
   {
      tellerror("Runtime error.Invalid layout type");
      delete tx;
      return EXEC_ABORT;
   }
   else
   {
      OPstack.push(new telldata::ttstring(static_cast<laydata::tdtcellref*>(tx->data())->cellname()));
      delete tx;
      return EXEC_NEXT;
   }
}
