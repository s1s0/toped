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
//           $URL$
//        Created: Sun May 18 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: CIF scanner
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================*/

/*
 *
 * Use options
 *
 * 	%option prefix="foo"
 * 	%option outfile="lex.yy.c"
 *
 * to create multiple flex scanner in one project.
 *
 *
 * To create scanner class:
 *
 *	%option c++
 *
 * (see '%option yyclass' also!)
 *
 */
%x comment
lexcif_digit    [0-9]
lexcif_upchar   [A-Z]
lexcif_snc      [A-Z0-9]
lexcif_blank    [^-();A-Z0-9]+
lexcif_comment  [^()]*
lexcif_usertext [^;]*
%{
#include "cif_yacc.h"

%}

%option debug

%%
\n+                        ;
<<EOF>>                  { /*if (!parsercmd::EOfile())*/ yyterminate();}
"E"                        return tknCend;
"D"                        return tknCdefine;
"P"                        return tknCpolygon;
"B"                        return tknCbox;
"R"                        return tknCround;
"W"                        return tknCwire;
"L"                        return tknClayer;
"C"                        return tknCcall;
"S"                        return tknCstart;
"F"                        return tknCfinish;
";"                        return tknPsem;
"("                       {BEGIN(comment);return tknPremB;}
")"                       {return tknPremE;}
{lexcif_digit}            {return tknPdigit;}
{lexcif_digit}+           {return tknTword;}
-?{lexcif_digit}+         {return tknTint;}
{lexcif_upchar}           {return tknTupchar;}
{lexcif_snc}{1,4}         {return tknTshortname; }
<comment>{lexcif_comment} {BEGIN(INITIAL);return tknTremtext;}
{lexcif_blank}            return tknTblank;
.                         return tknERROR;
%%

int cifwrap() {
   return 1;/*line by line*/
}
