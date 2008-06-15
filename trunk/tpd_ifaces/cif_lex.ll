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
#include "tpdph.h"
#include <stdio.h>
#include "cif_io.h"
#include "cif_yacc.h"
namespace CIFin {
   void     location_step(YYLTYPE *loc);
   void     location_lines(YYLTYPE *loc, int num);
   void     location_comment(YYLTYPE *loc, char* source);
   char*    charcopy(std::string source, bool quotes = false);
   unsigned getllint(char* source);
//   int      includefile(char* name, FILE* &handler);
//   int      EOfile();
   int      cifsRemDepth = 0; // depth of comments
}
using namespace CIFin;
extern YYLTYPE ciflloc;
#define YY_USER_ACTION  ciflloc.last_column += yyleng;
%}

/*%option debug*/

%%
%{
location_step(&ciflloc);
%}
<*>\n+                     location_lines(&ciflloc,yyleng);location_step(&ciflloc);
<<EOF>>                    { /*if (!parsercmd::EOfile())*/ yyterminate();   }
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
<*>"("                     { if (0 == cifsRemDepth) BEGIN( comment);
                             cifsRemDepth = cifsRemDepth + 1;
                                                   return tknPremB;        }
<*>")"                     { cifsRemDepth = cifsRemDepth - 1;
                             if (0 == cifsRemDepth) BEGIN( INITIAL);
                                                   return tknPremE;        }
{lexcif_digit}{1,1}        { BEGIN( usercmd);
                             ciflval.word    = CIFin::getllint(yytext);
                                                   return tknPdigit;       }
<cifcmd>-?{lexcif_digit}+  { ciflval.integer = CIFin::getllint(yytext);
                                                   return tknTint;         }
<cifcmd>{lexcif_upchar}    {                       return tknTupchar;      }
<layer>{lexcif_snc}{1,4}   { ciflval.identifier = CIFin::charcopy(yytext);
                                                   return tknTshortname;   }
<comment>{lexcif_comment}  { location_comment(&ciflloc,yytext);
                             ciflval.identifier = CIFin::charcopy(yytext);
                                                   return tknTremtext;     }
<usercmd>{lexcif_usertext} { BEGIN( INITIAL);      return tknTusertext;    }
<*>{lexcif_blank}          {location_comment(&ciflloc,yytext);
                                                   return tknTblank;       }
.                                                  return tknERROR;
%%

/*<cdefines>{lexcif_digit}+  {                       return tknTword;        }*/
int cifwrap() {
   return 1;/*line by line*/
}

//=============================================================================
// define scanner location tracking functions -
// taken form tell_lex.ll
//=============================================================================

void CIFin::location_step(YYLTYPE *loc)
{
   loc->first_column = loc->last_column;
   loc->first_line = loc->last_line;
}

void CIFin::location_lines(YYLTYPE *loc, int num)
{
   loc->last_column = 1;
   loc->last_line += num;
}

void CIFin::location_comment(YYLTYPE *loc, char* source)
{
   unsigned endlnum = 0;
   unsigned posnum = 0;
   while (0x00 != *source)
   {
      if (0x0A == *source)
      {
         endlnum++;
         posnum = 1;
      }
      else posnum++;
      source++;
   }
   if (0 != endlnum)
   {
      loc->last_line += endlnum;
      loc->last_column = posnum;
      location_step(loc);
   }
}

unsigned CIFin::getllint(char* source)
{
   char* dummy;
   unsigned result = strtoul(source, &dummy, 0);
   return result;
}

char* CIFin::charcopy(std::string source, bool quotes)
{
   int length = source.length() - (quotes ? 2 : 0);
   char* newstr = DEBUG_NEW char[length+2];
   memcpy(newstr,&(source.c_str()[quotes ? 1 : 0]),length);
   newstr[length] = 0x00;
   return newstr;
}
