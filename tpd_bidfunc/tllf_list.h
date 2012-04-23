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
   using parsercmd::ArgumentLIST;
   using parsercmd::ArgumentTYPE;

   TELL_STDCMD_CLASSC(lstLENGTH     );
   TELL_STDCMD_CLASSA(lytPOINTDUMP  );
   TELL_STDCMD_CLASSA(lytTYPEOF     );

   TELL_STDCMD_CLASSC(stdABS        );

   TELL_STDCMD_CLASSA(stdSIN        );
   TELL_STDCMD_CLASSA(stdCOS        );
   TELL_STDCMD_CLASSA(stdTAN        );
   TELL_STDCMD_CLASSA(stdASIN       );
   TELL_STDCMD_CLASSA(stdACOS       );
   TELL_STDCMD_CLASSA(stdATAN       );

   TELL_STDCMD_CLASSA(stdSINH       );
   TELL_STDCMD_CLASSA(stdCOSH       );
   TELL_STDCMD_CLASSA(stdTANH       );
   TELL_STDCMD_CLASSA(stdASINH      );
   TELL_STDCMD_CLASSA(stdACOSH      );
   TELL_STDCMD_CLASSA(stdATANH      );

   TELL_STDCMD_CLASSA(stdROUND      );
   TELL_STDCMD_CLASSA(stdCEIL       );
   TELL_STDCMD_CLASSA(stdFLOOR      );
   TELL_STDCMD_CLASSA(stdFMODULO    );
   TELL_STDCMD_CLASSA(stdSQRT       );
   TELL_STDCMD_CLASSA(stdPOW        );

   TELL_STDCMD_CLASSA(stdEXP        );
   TELL_STDCMD_CLASSA(stdLOG        );
   TELL_STDCMD_CLASSA(stdLOG10      );
}

#endif  //TLLF_LIST_H
