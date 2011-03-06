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
   TELL_STDCMD_CLASSA(stdNEWDESIGNsd   );                // reset undo buffers
   TELL_STDCMD_CLASSB(stdNEWDESIGN    , stdNEWDESIGNd ); // reset undo buffers
   TELL_STDCMD_CLASSB(stdNEWDESIGNs   , stdNEWDESIGNsd); // reset undo buffers
   TELL_STDCMD_CLASSA(TDTread          );                // reset undo buffers
   TELL_STDCMD_CLASSB(TDTreadIFF      , TDTread       );
   TELL_STDCMD_CLASSA(TDTloadlib       );
   TELL_STDCMD_CLASSA(TDTunloadlib     );
   TELL_STDCMD_CLASSA(TDTsave          );
   TELL_STDCMD_CLASSB(TDTsaveIFF      , TDTsave       );
   TELL_STDCMD_CLASSA(TDTsaveas        );
   TELL_STDCMD_CLASSA(stdREPORTLAY     );
   TELL_STDCMD_CLASSB(stdREPORTLAYc   , stdREPORTLAY  );

   TELL_STDCMD_CLASSA(GDSread          );
   TELL_STDCMD_CLASSA(GDSimport        );
   TELL_STDCMD_CLASSA(GDSimportList    );
   TELL_STDCMD_CLASSA(GDSexportLIB     );
   TELL_STDCMD_CLASSA(GDSexportTOP     );
   TELL_STDCMD_CLASSA(GDSclose         );
   TELL_STDCMD_CLASSA(GDSreportlay     );
   TELL_STDCMD_CLASSA(GDSgetlaymap     );
   TELL_STDCMD_CLASSA(GDSsetlaymap     );
   TELL_STDCMD_CLASSA(GDSclearlaymap   );
   TELL_STDCMD_CLASSA(GDSsplit         );

   TELL_STDCMD_CLASSA(OASread          );
   TELL_STDCMD_CLASSA(OASimport        );
   TELL_STDCMD_CLASSA(OASimportList    );
   TELL_STDCMD_CLASSA(OASclose         );
   TELL_STDCMD_CLASSA(OASreportlay     );
   TELL_STDCMD_CLASSA(OASgetlaymap     );
   TELL_STDCMD_CLASSA(OASsetlaymap     );
   TELL_STDCMD_CLASSA(OASclearlaymap   );

   TELL_STDCMD_CLASSA(CIFread          );
   TELL_STDCMD_CLASSA(CIFimport        );
   TELL_STDCMD_CLASSA(CIFimportList    );
   TELL_STDCMD_CLASSA(CIFexportLIB     );
   TELL_STDCMD_CLASSA(CIFexportTOP     );
   TELL_STDCMD_CLASSA(CIFclose         );
   TELL_STDCMD_CLASSA(CIFreportlay     );
   TELL_STDCMD_CLASSA(CIFgetlaymap     );
   TELL_STDCMD_CLASSA(CIFclearlaymap   );
   TELL_STDCMD_CLASSA(CIFsetlaymap     );

   TELL_STDCMD_CLASSA(DRCCalibreimport );
   TELL_STDCMD_CLASSA(DRCshowerror     );
   TELL_STDCMD_CLASSA(DRCshowcluster   );
   TELL_STDCMD_CLASSA(DRCshowallerrors );
   TELL_STDCMD_CLASSA(DRChideallerrors );
   TELL_STDCMD_CLASSA(DRCexplainerror_D );
   TELL_STDCMD_CLASSA(DRCexplainerror  );
   TELL_STDCMD_CLASSA(PSexportTOP      );

   void  importGDScell(laydata::TdtLibDir*, const NameList&, const LayerMapExt&, parsercmd::undoQUEUE&, telldata::UNDOPerandQUEUE&, bool, bool, bool);
   void  importCIFcell(laydata::TdtLibDir*, const NameList&, const SIMap&      , parsercmd::undoQUEUE&, telldata::UNDOPerandQUEUE&, bool, bool, bool, real);
   void  importOAScell(laydata::TdtLibDir*, const NameList&, const LayerMapExt&, parsercmd::undoQUEUE&, telldata::UNDOPerandQUEUE&, bool, bool, bool);

}
#endif
