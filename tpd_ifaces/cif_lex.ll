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
%x layer
%x usercmd
%x cifcmd
%x defcmd
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
<<EOF>>                    { /*if (!parsercmd::EOfile())*/ yyterminate();}
"E"                        { BEGIN( cifcmd );      return tknCend;         }
"D"                        { BEGIN( defcmd );      return tknCdefine;      }
<defcmd>"D"                { BEGIN( cifcmd );      return tknCdefine;      }
"P"                        { BEGIN( cifcmd );      return tknCpolygon;     }
"B"                        { BEGIN( cifcmd );      return tknCbox;         }
"R"                        { BEGIN( cifcmd );      return tknCround;       }
"W"                        { BEGIN( cifcmd );      return tknCwire;        }
"L"                        { BEGIN( layer  );      return tknClayer;       }
"C"                        { BEGIN( cifcmd );      return tknCcall;        }
<defcmd>"S"                { BEGIN( cifcmd );      return tknCstart;       }
<defcmd>"F"                { BEGIN( cifcmd );      return tknCfinish;      }
<*>";"                     { BEGIN( INITIAL);      return tknPsem;         }
"("                        { BEGIN( comment);      return tknPremB;        }
")"                        {                       return tknPremE;        }
{lexcif_digit}{1,1}        { BEGIN( usercmd);      return tknPdigit;       }
<cifcmd>-?{lexcif_digit}+  {                       return tknTint;         }
<cifcmd>{lexcif_upchar}    {                       return tknTupchar;      }
<layer>{lexcif_snc}{1,4}   {                       return tknTshortname;   }
<comment>{lexcif_comment}  { BEGIN( INITIAL);      return tknTremtext;     }
<usercmd>{lexcif_usertext} { BEGIN( INITIAL);      return tknTusertext;    }
<*>{lexcif_blank}                                  return tknTblank;
.                                                  return tknERROR;
%%

/*<cdefines>{lexcif_digit}+  {                       return tknTword;        }*/
int cifwrap() {
   return 1;/*line by line*/
}
