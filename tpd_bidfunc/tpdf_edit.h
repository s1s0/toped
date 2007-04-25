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
//    Description: Definition of all TOPED edit functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef  TPDF_EDIT_H
#define  TPDF_EDIT_H

#include "tpdf_commom.h"
namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;

   TELL_STDCMD_CLASSA_UNDO(stdCOPYSEL     )  // undo - implemented
   TELL_STDCMD_CLASSB(stdCOPYSEL_D    , stdCOPYSEL    )
   TELL_STDCMD_CLASSA_UNDO(stdMOVESEL     )  // undo - implemented
   TELL_STDCMD_CLASSB(stdMOVESEL_D    , stdMOVESEL    )
   TELL_STDCMD_CLASSA_UNDO(stdROTATESEL   )  // undo - implemented
   TELL_STDCMD_CLASSB(stdROTATESEL_D  , stdROTATESEL  )
   TELL_STDCMD_CLASSA_UNDO(stdFLIPXSEL    )  // undo - implemented
   TELL_STDCMD_CLASSB(stdFLIPXSEL_D   , stdFLIPXSEL   )
   TELL_STDCMD_CLASSA_UNDO(stdFLIPYSEL    )  // undo - implemented
   TELL_STDCMD_CLASSB(stdFLIPYSEL_D   , stdFLIPYSEL   )
   TELL_STDCMD_CLASSA_UNDO(stdDELETESEL   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(lgcCUTPOLY     )  // undo - implemented
   TELL_STDCMD_CLASSB(lgcCUTPOLY_I    , lgcCUTPOLY    )
   TELL_STDCMD_CLASSA_UNDO(lgcMERGE       )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdCHANGELAY   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdCHANGEREF   )  // undo - implemented

}
#endif
