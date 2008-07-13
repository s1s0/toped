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
%x CFS_REM
%x CFS_LCMD
%x CFS_UCMD
%x CFS_CCMD
%x CFS__CMD
%x CFS_DCMD
%x CFS_UDEF
lexcif_digit    [0-9]
lexcif_upchar   [A-Z]
lexcif_snc      [A-Z0-9]
lexcif_blank    [^-();A-Z0-9]+
lexcif_comment  [^()]*
lexcif_usertext [^;]*
lexcif_usrid    [^ \t;]+
lexcif_usrsep   [ \t]+
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
   long     getllint(char* source);
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
<*>\n+                         location_lines(&ciflloc,yyleng);location_step(&ciflloc);
<<EOF>>                       { /*if (!parsercmd::EOfile())*/ yyterminate();      }
"E"                           { BEGIN( CFS__CMD );       return tknCend;         }
"D"                           { BEGIN( CFS_DCMD );       return tknCdefine;      }
<CFS_DCMD>"D"                 { BEGIN( CFS__CMD );       return tknCdefine;      }
"P"                           { BEGIN( CFS__CMD );       return tknCpolygon;     }
"B"                           { BEGIN( CFS__CMD );       return tknCbox;         }
"R"                           { BEGIN( CFS__CMD );       return tknCround;       }
"W"                           { BEGIN( CFS__CMD );       return tknCwire;        }
"L"                           { BEGIN( CFS_LCMD );       return tknClayer;       }
"C"                           { BEGIN( CFS_CCMD );       return tknCcall;        }
<CFS_DCMD>"S"                 { BEGIN( CFS__CMD );       return tknCstart;       }
<CFS_DCMD>"F"                 { BEGIN( CFS__CMD );       return tknCfinish;      }
<CFS_CCMD>"T"                 {                          return tknCtranslate;   }
<CFS_CCMD>"M"                 {                          return tknCmirror;      }
<CFS_CCMD>"X"                 {                          return tknCmirx;        }
<CFS_CCMD>"Y"                 {                          return tknCmiry;        }
<CFS_CCMD>"R"                 {                          return tknCrotate;      }
<*>";"                        { BEGIN( INITIAL  );       return tknPsem;         }
<*>"("                        { if (0 == cifsRemDepth) BEGIN( CFS_REM);
                                cifsRemDepth = cifsRemDepth + 1;
                                                         return tknPremB;        }
<*>")"                        { cifsRemDepth = cifsRemDepth - 1;
                                if (0 == cifsRemDepth) BEGIN( INITIAL);
                                                         return tknPremE;        }
9{lexcif_usrsep}              { BEGIN( CFS_UCMD);        return tknP9;           }
4A{lexcif_usrsep}             { BEGIN( CFS__CMD);        return tknP4A;          }
4N{lexcif_usrsep}             { BEGIN( CFS_UCMD);        return tknP4N;          }
94{lexcif_usrsep}             { BEGIN( CFS_UCMD);        return tknP94;          }
{lexcif_digit}{1,1}           { BEGIN( CFS_UDEF);
                                ciflval.word    = CIFin::getllint(yytext);
                                                         return tknPdigit;       }
<CFS__CMD>-?{lexcif_digit}+  |
<CFS_CCMD>-?{lexcif_digit}+  |
<CFS_UCMD>-?{lexcif_digit}+   { ciflval.integer = CIFin::getllint(yytext);
                                                         return tknTint;         }
<CFS__CMD>{lexcif_upchar}     {                          return tknTupchar;      }
<CFS_UCMD>{lexcif_usrsep}     {                          return tknTblank;       }
<CFS_UCMD>{lexcif_usrid}      { ciflval.identifier = CIFin::charcopy(yytext);
                                                         return tknTuserid;      }
<CFS_LCMD>{lexcif_snc}{1,4}   { ciflval.identifier = CIFin::charcopy(yytext);
                                                         return tknTshortname;   }
<CFS_REM>{lexcif_comment}     { location_comment(&ciflloc,yytext);
                                ciflval.identifier = CIFin::charcopy(yytext);
                                                         return tknTremtext;     }
<CFS_UDEF>{lexcif_usertext}   { BEGIN (INITIAL);
                                ciflval.identifier = CIFin::charcopy(yytext);
                                                         return tknTuserText;    }
<INITIAL>{lexcif_blank}      |
<CFS__CMD>{lexcif_blank}     |
<CFS_CCMD>{lexcif_blank}     |
<CFS_DCMD>{lexcif_blank}      { location_comment(&ciflloc,yytext);
                                                         return tknTblank;       }
.                                                        return tknERROR;
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

long CIFin::getllint(char* source)
{
   char* dummy;
   long result = strtoul(source, &dummy, 0);
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
/*
      0 x y layer N name;           Set named node on specified layer and position
      0V x1 y1 x2 y2 ... xn yn;     Draw vectors
      2A "msg" T x y;               Place message above specified location
      2B "msg" T x y;               Place message below specified location
      2C "msg" T x y;               Place message centered at specified location
      2L "msg" T x y;               Place message left of specified location
      2R "msg" T x y;               Place message right of specified location
Done  4A lowx lowy highx highy;     Declare cell boundary
      4B instancename;              Attach instance name to cell
      4N signalname x y;            Labels a signal at a location
Done  9  cellname;                  Declare cell name
      91 instancename;              Attach instance name to cell
Done  94 label x y;                 Place label in specified location
Done  94 label x y layername;       Place label in specified location
      95 label length width x y;    Place label in specified area
 */
