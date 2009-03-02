/*===========================================================================
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
//           $URL: https://gaitukevich@svn.berlios.de/svnroot/repos/toped/trunk/tpd_ifaces/cif_lex.ll $
//        Created: Sun May 18 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Calibre drc results scanner
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 735 $
//          $Date: 2008-08-04 04:02:58 +0800 (Пн, 04 авг 2008) $
//        $Author: s_krustev $
//===========================================================================*/

/*
 * To create scanner class:
 *
 *	%option c++
 *
 * (see '%option yyclass' also!)
 *
 */

lexcif_digit    [0-9]
lexcif_upchar   [A-Z]

%{
#include "calbr_yacc.h"
%}

%%

<<EOF>>                       {  yyterminate();      }
"E"                           {    return tknCend;         }
.                                                        return tknERROR;
%%

/*<cdefines>{lexcif_digit}+  {                       return tknTword;        }*/

/**************************************************************************/
/*Support functions for the flex parser*/
/**************************************************************************/

int calbrwrap() {
   return 1;/*line by line*/
}

void delete_calbr_lex_buffer( void* b ) 
{
   calbr_delete_buffer((YY_BUFFER_STATE) b);
}

void* new_calbr_lex_buffer( FILE* cifin )
{
   YY_BUFFER_STATE b = yy_create_buffer( yyin, YY_BUF_SIZE );
   yy_switch_to_buffer(b);
   return (void*) b;
}


