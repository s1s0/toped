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
//    Description: Definition of all TOPED property and mode related functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef  TPDF_PROPS_H
#define  TPDF_PROPS_H

#include "tpdf_common.h"
namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;

   TELL_STDCMD_CLASSA(stdPROPSAVE      );
   TELL_STDCMD_CLASSA(stdLAYPROP       );  //
   TELL_STDCMD_CLASSA(stdLINEDEF       );  //
   TELL_STDCMD_CLASSA(stdCOLORDEF      );  //
   TELL_STDCMD_CLASSA(stdFILLDEF       );  //
   TELL_STDCMD_CLASSA(stdGRIDDEF       );  //
   TELL_STDCMD_CLASSA(stdSETPARAMETER  );  //
   TELL_STDCMD_CLASSA(stdSETPARAMETERS );  //
   TELL_STDCMD_CLASSA_UNDO(stdHIDELAYER   );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDELAYERS  );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDECELLMARK);  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDETEXTMARK);  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDECELLBOND);  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDETEXTBOND);  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdLOCKLAYER   );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdLOCKLAYERS  );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdFILLLAYER   );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdFILLLAYERS  );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSAVELAYSTAT );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdLOADLAYSTAT );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdDELLAYSTAT  );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdGRID        );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSTEP        );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdAUTOPAN     );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdZEROCROSS   );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSHAPEANGLE  );  // undo - implemented

   void analyzeTopedParameters(std::string, std::string);
}
#endif
