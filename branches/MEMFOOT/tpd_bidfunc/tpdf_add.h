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
//    Description: Definition of all TOPED functions that add a new shape
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef  TPDF_ADD_H
#define  TPDF_ADD_H

#include "tpdf_common.h"
namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;

   TELL_STDCMD_CLASSA_UNDO(stdADDBOX      );  // undo - implemented
   TELL_STDCMD_CLASSB(stdADDBOX_D     , stdADDBOX     );
   TELL_STDCMD_CLASSA_UNDO(stdDRAWBOX     );  // undo - implemented
   TELL_STDCMD_CLASSB(stdDRAWBOX_D    , stdDRAWBOX    );
   TELL_STDCMD_CLASSA_UNDO(stdADDBOXr     );  // undo - implemented8
   TELL_STDCMD_CLASSB(stdADDBOXr_D    , stdADDBOXr    );
   TELL_STDCMD_CLASSA_UNDO(stdADDBOXp     );  // undo - implemented
   TELL_STDCMD_CLASSB(stdADDBOXp_D    , stdADDBOXp    );
   TELL_STDCMD_CLASSA_UNDO(stdADDPOLY     );  // undo - implemented
   TELL_STDCMD_CLASSB(stdADDPOLY_D    , stdADDPOLY    );
   TELL_STDCMD_CLASSA_UNDO(stdDRAWPOLY    );  // undo - implemented
   TELL_STDCMD_CLASSB(stdDRAWPOLY_D   , stdDRAWPOLY   );
   TELL_STDCMD_CLASSA_UNDO(stdADDWIRE     );  // undo - implemented
   TELL_STDCMD_CLASSB(stdADDWIRE_D    , stdADDWIRE    );
   TELL_STDCMD_CLASSA_UNDO(stdDRAWWIRE    );  // undo - implemented
   TELL_STDCMD_CLASSB(stdDRAWWIRE_D   , stdDRAWWIRE   );
   TELL_STDCMD_CLASSA_UNDO(stdADDTEXT     );  // undo - implemented
   TELL_STDCMD_CLASSB(stdADDTEXT_D    , stdADDTEXT    );
   TELL_STDCMD_CLASSA_UNDO(stdCELLREF     );  // undo - implemented
   TELL_STDCMD_CLASSB(stdCELLREF_D    , stdCELLREF    );
   TELL_STDCMD_CLASSA_UNDO(stdCELLAREF    );  // undo - implemented
   TELL_STDCMD_CLASSB(stdCELLAREF_D   , stdCELLAREF   );
   TELL_STDCMD_CLASSA_UNDO(stdUSINGLAYER  );  // undo - implemented
   TELL_STDCMD_CLASSB(stdUSINGLAYER_S , stdUSINGLAYER );  //
}
#endif
