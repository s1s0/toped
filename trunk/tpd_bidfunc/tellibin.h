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
//        Created: Fri Jan 24 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TELL build-in functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef  TELLIBIN_H
#define  TELLIBIN_H
#include "tpdf_common.h"
#include <wx/string.h>
#include <wx/regex.h>


namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::ArgumentLIST;
   using parsercmd::ArgumentTYPE;

   const wxString flags_tmpl     = wxT("[-+ #0]?");
   const wxString width_tmpl     = wxT("([[:digit:]]+|\\*)?");
   const wxString prec_tmpl      = wxT("(\\.[[:digit:]]+|\\*)?");
   const wxString spec_tmpl      = wxT("[cdieEfgGosuxXpn%]");
   const wxString esc_tmpl       = wxT("\\\\n");
   const wxString fspec_tmpl     = wxT("%")+flags_tmpl + width_tmpl + prec_tmpl + spec_tmpl;
//   const wxString dblesc_tmpl    = esc_tmpl + esc_tmpl;

   bool replaceNextFstr(std::string&, telldata::tell_var*);
   void replaceESCs(std::string&);
   bool checkFstr(const std::string&);

   TELL_STDCMD_CLASSC(stdECHO          );
   TELL_STDCMD_CLASSC(stdSPRINTF       );
   TELL_STDCMD_CLASSD(stdPRINTF , stdSPRINTF );
   TELL_STDCMD_CLASSA(stdTELLSTATUS    );
   TELL_STDCMD_CLASSA(stdUNDO          );
   TELL_STDCMD_CLASSA(stdREDRAW        );
   TELL_STDCMD_CLASSA(stdZOOMWIN       );
   TELL_STDCMD_CLASSB(stdZOOMWINb     , stdZOOMWIN    );
   TELL_STDCMD_CLASSA(stdZOOMALL       );
   TELL_STDCMD_CLASSA(stdZOOMVISIBLE   );
   TELL_STDCMD_CLASSA(getPOINT         );
   TELL_STDCMD_CLASSA(getPOINTLIST     );
   TELL_STDCMD_CLASSA(stdDISTANCE      );
   TELL_STDCMD_CLASSB(stdDISTANCE_D   , stdDISTANCE   );
   TELL_STDCMD_CLASSA(stdCLEARRULERS   );
   TELL_STDCMD_CLASSA(stdLONGCURSOR    );  //
   TELL_STDCMD_CLASSA(stdEXEC          );  //
   TELL_STDCMD_CLASSA(stdEXIT          );  //
   //
   TELL_STDCMD_CLASSA(intrnlSORT_DB    );
}

#endif
/*
---------------------------------------------------------------------------|
---------------------------------------------------------------------------|
          |                   Data Base (layout) sub-type           ||     |
          |------+------+------+------+------+------||--------------|| NB ||
 Operation|  box | poly | wire |  ref | aref | text || point | cell ||    ||
----------+------+------+------+------+------|------||-------+------||----||
----------+------+------+------+------+------|------||-------+------||----||
 select   |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||  OK   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 unselect |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||  OK   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 copy     |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||   -   |  x   || 2  ||
----------+------+------+------+------+------|------||-------+------||----||
 move     |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||  OK   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 delete   |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||   -   |  OK  || 2  ||
----------+------+------+------+------+------|------||-------+------||----||
 merge    |  v   |   v  |   x  |   -  |   -  |  -   ||   -   |  -   || 1  ||
----------+------+------+------+------+------|------||-------+------||----||
 cut      |  v   |   v  |   x  |   -  |   x  |  -   ||   -   |  -   || 1  ||
----------+------+------+------+------+------|------||-------+------||----||
 group    |  OK  |   OK |   OK |   OK |   OK |  OK  ||   -   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 ungroup  |  -   |   -  |   -  |   OK |   OK |  -   ||   -   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 info/edit|  x   |   x  |   x  |   x  |   x  |  x   ||   x   |  -   || 3  ||
----------+------+------+------+------+------|------||-------+------||----||
---------------------------------------------------------------------------|
?mtrx? - a matrix of any object (not cell only as aref in GDSII). Defined
         by: object/stepX/stepY/i/j
1. There will be an auto type conversion here. Poly is the universal,
   but less effective type. Box and wire will be used where is
   possible
2. It is not an operation on the layout type. It should be something
   similar to copy one cell under a new name or to delete a cell. Obviously
   these operations (functions) should be called differently.
3. The idea of "edit" operation is to have a manual opportunity to change the
   layout objects. It will be extremely useful for text/ref/aref objects in
   order to change the text/cell. Changing the layer will be possible
   via this operation.

*/
