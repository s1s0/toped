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
//   This file is a part of Toped project (C) 2001-2008 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL: https://gaitukevich@svn.berlios.de/svnroot/repos/toped/trunk/tpd_ifaces/cif_yacc.yy $
//        Created: Sun May 18 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Calibre drc results parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 791 $
//          $Date: 2008-09-30 06:17:37 +0800 (Вт, 30 сен 2008) $
//        $Author: s_krustev $
//===========================================================================*/

/*Switch on code processing locations for more acurate error messages*/
%locations
%{
#include "tpdph.h"
#include "calbr_reader.h"

/* Switch on verbose error reporting messages*/
#define YYERROR_VERBOSE 1


%}

/*%debug*/


%start cifFile
/* token code T - type; C - command; P - primitive*/
%token                 tknERROR tknCend

%%
cifFile:
     'a' 'b'                      {}


%%

