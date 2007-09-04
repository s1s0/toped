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
//        Created: Wed Apr 25 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED select/unselect functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef  TPDF_SELECT_H
#define  TPDF_SELECT_H

#include "tpdf_common.h"
namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;

   TELL_STDCMD_CLASSA_UNDO(stdSELECT      )  // undo - implemented
   TELL_STDCMD_CLASSB(stdSELECT_I     , stdSELECT     )
   TELL_STDCMD_CLASSA_UNDO(stdSELECTIN    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSELECT_TL   )  //
   TELL_STDCMD_CLASSA_UNDO(stdPNTSELECT   )  // undo - implemented
   TELL_STDCMD_CLASSB(stdPNTSELECT_I  , stdPNTSELECT  )
   TELL_STDCMD_CLASSA_UNDO(stdSELECTALL   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECT    )  // undo - implemented
   TELL_STDCMD_CLASSB(stdUNSELECT_I   , stdUNSELECT   )
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECTIN  )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECT_TL )  //
   TELL_STDCMD_CLASSA_UNDO(stdPNTUNSELECT )  // undo - implemented
   TELL_STDCMD_CLASSB(stdPNTUNSELECT_I, stdPNTUNSELECT)
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECTALL )  // undo - implemented
   TELL_STDCMD_CLASSA(stdREPORTSLCTD      )
   TELL_STDCMD_CLASSA_UNDO(stdSETSELECTMASK)

}
#endif
