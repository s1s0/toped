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
//        Created: Sat Apr 28 2007
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all list related functions in TELL
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef  TLLF_LIST_H
#define  TLLF_LIST_H

#include "tpdf_common.h"

namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;

   TELL_STDCMD_CLASSC(lstLENGTH     );
   TELL_STDCMD_CLASSC(stdABS        );
   TELL_STDCMD_CLASSC(stdSIN        );
   TELL_STDCMD_CLASSC(stdCOS        );
   TELL_STDCMD_CLASSC(stdRINT       );
}

#endif  //TLLF_LIST_H
