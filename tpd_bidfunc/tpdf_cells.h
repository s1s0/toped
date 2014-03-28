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
//    Description: Definition of all TOPED cell related functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef  TPDF_CELLS_H
#define  TPDF_CELLS_H

#include "tpdf_common.h"
namespace tellstdfunc {
   using namespace parsercmd;
   using telldata::argumentQ;

   //
   TELL_STDCMD_CLASSA_UNDO(stdNEWCELL        );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdREMOVECELL     );  // undo - implemented
//   TELL_STDCMD_CLASSA_UNDO(stdREMOVEREFDCELL );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdOPENCELL       );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITPUSH       );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITPOP        );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITPREV       );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITTOP        );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdGROUP          );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNGROUP        );  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdRENAMECELL     );  // undo - implemented
   TELL_STDCMD_CLASSA(stdCHECKCELL);
}
#endif
