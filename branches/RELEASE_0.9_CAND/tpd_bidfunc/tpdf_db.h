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

#ifndef  TPDF_DB_H
#define  TPDF_DB_H

#include "tpdf_common.h"
namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;
   TELL_STDCMD_CLASSA(stdNEWDESIGNd    );                // reset undo buffers
   TELL_STDCMD_CLASSB(stdNEWDESIGN   , stdNEWDESIGNd  ); // reset undo buffers
   TELL_STDCMD_CLASSA(TDTread          );                // reset undo buffers
   TELL_STDCMD_CLASSB(TDTreadIFF      , TDTread       );
   TELL_STDCMD_CLASSA(TDTloadlib       );
   TELL_STDCMD_CLASSA(TDTunloadlib     );
   TELL_STDCMD_CLASSA(TDTsave          );
   TELL_STDCMD_CLASSB(TDTsaveIFF      , TDTsave       );
   TELL_STDCMD_CLASSA(TDTsaveas        );
   TELL_STDCMD_CLASSA(GDSread          );
   TELL_STDCMD_CLASSA(GDSconvert       );
   TELL_STDCMD_CLASSA(GDSconvertAll    );
   TELL_STDCMD_CLASSA(GDSexportLIB     );
   TELL_STDCMD_CLASSA(GDSexportTOP     );
   TELL_STDCMD_CLASSA(PSexportTOP      );
   TELL_STDCMD_CLASSA(GDSclose         );
   TELL_STDCMD_CLASSA(stdREPORTLAY     );
   TELL_STDCMD_CLASSB(stdREPORTLAYc   , stdREPORTLAY  );
   TELL_STDCMD_CLASSA(GDSreportlay     );
}
#endif
