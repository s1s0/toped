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
//    Description: Definition of all TOPED database functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef  TPDF_GET_H
#define  TPDF_GET_H

#include "tpdf_common.h"
namespace tellstdfunc {
   using namespace parsercmd;
   using telldata::argumentQ;

   TELL_STDCMD_CLASSA(stdGETLAYTYPE       );
   TELL_STDCMD_CLASSA(stdGETLAYTEXTSTR    );
   TELL_STDCMD_CLASSA(stdGETLAYREFSTR     );
   TELL_STDCMD_CLASSA(grcGETCELLS         );
   TELL_STDCMD_CLASSA(grcGETLAYERS        );
   TELL_STDCMD_CLASSA(grcGETDATA          );
   TELL_STDCMD_CLASSA_UNDO(grcCLEANALAYER );
   TELL_STDCMD_CLASSA_UNDO(grcREPAIRDATA  );
}
#endif
