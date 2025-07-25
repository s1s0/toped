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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Fri Nov 08 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TELL lexer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//---------------------------------------------------------------------------
// A non-reentrant lexer for tell
//===========================================================================*/

/* Define the (exclusive) start condition when the parser includes a file */
%x pINCL
%x pDEFNM
%x pUDEFNM
%x pDEFVAL
%x pIFD
%x pIFND
%x pIFED
%x pPASS
%x pPRAGMA
%x pREPEATLOOP
lex_string        \"[^"\n]*\"
lex_rexpfind      \/[^/\n]+\/
lex_rexprplc      \/[^\/\n]+\/[^\/\n]+\/
lex_ID            [a-zA-Z_][a-zA-Z0-9_]*
lxt_S             [ \t]+
lxt_D             [0-9]
lxt_E             [Ee][-+]?{lxt_D}+
%{ /**************************************************************************/
#include "tpdph.h"
#include <stdio.h>
#include <string.h>
#include "tellyzer.h"
#include "tell_yacc.hh"
#include "ted_prompt.h"

/* Definitions for handling multiply input buffers */
#define MAX_INCLUDE_DEPTH 100
parsercmd::lexer_files* include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;
/*Global console object*/
extern console::TllCmdLine*        Console;
extern parsercmd::cmdBLOCK*        CMDBlock;
extern parsercmd::TellPreProc*     tellPP;

/*****************************************************************************
Function declarations
*****************************************************************************/
#define YY_USER_ACTION  telllloc.last_column += yyleng;
#define YY_FATAL_ERROR(msg)  parsercmd::telllex_fatal(msg)
namespace parsercmd {
   void     location_step(YYLTYPE *loc);
   void     location_lines(YYLTYPE *loc, unsigned long num);
   void     ccomment(YYLTYPE *loc);
   void     cppcomment(YYLTYPE *loc);
   char*    charcopy(std::string source, bool quotes = false);
   unsigned getllint(char* source);
   int      includefile(char* name, FILE* &handler);
   int      EOfile();
   static void telllex_fatal(std::string);
   void     newPrepVar(std::string source, bool empty);
   void     newPrepVal(std::string source);
/*   void     deletePrepVal(std::string source);*/
   bool     checkPrepVar(const std::string source);
}
using namespace parsercmd;
extern YYLTYPE telllloc;
%}
%%
%{ /*******************************************************************************
     This section contains local definitions and also actions executable at each
     invocation of yylex. */
/* Mark the current position as a start of the next token */
location_step(&telllloc);
%} /*******************************************************************************/
<*>{lxt_S}                 location_step(&telllloc);
<*>\n+                     location_lines(&telllloc,yyleng);location_step(&telllloc);
<*>"/*"                    ccomment(&telllloc); /*comment block C style*/
<*>"//"                    cppcomment(&telllloc);/* comment line CPP stype*/
<pPASS>#ifdef              tellPP->ppPush();
<INITIAL>#ifdef            BEGIN(pIFD);
<pIFD>{lex_ID}           { if (!tellPP->ppIfDef(yytext))
                              BEGIN(pPASS);
                           else
                              BEGIN(INITIAL);
                         }
<pPASS>#ifndef             tellPP->ppPush();
<INITIAL>#ifndef           BEGIN(pIFND);
<pIFND>{lex_ID}          { if (!tellPP->ppIfNDef(yytext))
                              BEGIN(pPASS);
                           else
                              BEGIN(INITIAL);
                         }
<*>#else                 { if (!tellPP->ppElse(telllloc))
                              BEGIN(pPASS);
                           else
                           {
                              BEGIN(INITIAL);
                              if (tellPP->lastError())
                                 return tknERROR;
                           }
                         }
<*>#endif                { if (!tellPP->ppEndIf(telllloc))
                           {
                              BEGIN(INITIAL);
                              if (tellPP->lastError())
                                 return tknERROR;
                           }
                         }
<pPASS>.                 /*nothing to do here*/
#pragma                    BEGIN(pPRAGMA);
<pPRAGMA>once            { BEGIN(INITIAL);
                           if (tellPP->pragOnce())
                           {
                              if (parsercmd::EOfile())
                              {
                                 tellPP->checkEOF();
                                 if (tellPP->lastError())
                                    return tknERROR;
                              } 
                              else 
                                 yyterminate();
                           }
                         }
<pPRAGMA>prepreset       { BEGIN(INITIAL);
                           tellPP->reset();
                         }
<pPRAGMA>fullreset       { BEGIN(INITIAL);
                           tellPP->reset();
                           TpdPost::reloadTellFuncs();
                         }
<pPRAGMA>repeatloop        BEGIN(pREPEATLOOP);
<pREPEATLOOP>{lex_ID}    { BEGIN(INITIAL);
                           tellPP->setRepeatLC(yytext, telllloc);
                         }
#define                    BEGIN(pDEFNM);
<pDEFNM>{lex_ID}[ \t]*\n { parsercmd::newPrepVar(yytext, true);
                           location_lines(&telllloc,1);location_step(&telllloc);
                           BEGIN(INITIAL);
                         }
<pDEFNM>{lex_ID}         { parsercmd::newPrepVar(yytext, false);
                           BEGIN(pDEFVAL);
                         }
<pDEFVAL>.*              { parsercmd::newPrepVal(yytext);
                           BEGIN(INITIAL);
                         }
#undef                     BEGIN(pUDEFNM);
<pUDEFNM>{lex_ID}        { tellPP->undefine(yytext, telllloc);
                           BEGIN(INITIAL);
                         }
#include                   BEGIN(pINCL);
<pINCL>{lex_string}      { /*first change the scanner state, otherwise there
                             is a risk to remain in <pINCL>*/
                           BEGIN(INITIAL); 
                           tellPP->markBOF(); 
                           if (! parsercmd::includefile(parsercmd::charcopy(yytext, true), yyin)) 
                              yyterminate();
                         }
<pINCL><<EOF>>           { BEGIN(INITIAL); return tknERROR; }
<<EOF>>                  { int res = parsercmd::EOfile();
                           if (res)
                           {
                              tellPP->checkEOF();
                              if (tellPP->lastError())
                              {
                                 BEGIN(INITIAL); 
                                 return tknERROR;
                              }
                           } 
                           else 
                              yyterminate();
                         }
void                       return tknVOIDdef;
real                       return tknREALdef;
int                        return tknINTdef;
unsigned                   return tknUNSIGNEDdef;
bool                       return tknBOOLdef;
string                     return tknSTRINGdef;
layout                     return tknLAYOUTdef;
callback                   return tknCALLBACKdef;
auxdata                    return tknAUXDATAdef;
list                       return tknLISTdef;
return                     return tknRETURN;
true                       return tknTRUE;
false                      return tknFALSE;
if                         return tknIF;
else                       return tknELSE;
while                      return tknWHILE;
repeat                     return tknREPEAT;
foreach                    return tknFOREACH;
for                        return tknFOR;
break                      return tknBREAK;
continue                   return tknCONTINUE;
struct                     return tknSTRUCTdef;
until                      return tknUNTIL;
const                      return tknCONST;
{lex_string}             { telllval.parsestr = parsercmd::charcopy(yytext, true);
                           return tknSTRING;                               }
"."{lex_ID}              { telllval.parsestr = parsercmd::charcopy(yytext);return tknFIELD;}
"<="                       return tknLEQ;
">="                       return tknGEQ;
"=="                       return tknEQ;
"!="                       return tknNEQ;
"=~"                     { CMDBlock->setRexExpected(true); return tknRXEQ;}
"&&"                       return tknAND;
"||"                       return tknOR;
"&"                        return tknBWAND;
"|"                        return tknBWOR;
"^"                        return tknBWXOR;
"!"                        return tknNOT;
"~"                        return tknBWNOT;
"<<"                       return tknSHL;
">>"                       return tknSHR;
"|/"                       return tknSW;
"\\|"                      return tknSE;
"/|"                       return tknNE;
"|\\"                      return tknNW;
"+:"                       return tknPREADD;
":+"                       return tknPOSTADD;
"-:"                       return tknPRESUB;
":-"                       return tknPOSTSUB;
":"                        return ':';
"("                        return '(';
")"                        return ')';
"{"                        return '{';
"}"                        return '}';
"<"                        return '<';
">"                        return '>';
"["                        return '[';
"]"                        return ']';
","                        return ',';
"+"                        return '+';
"-"                        return '-';
"*"                        return '*';
"/"                        return '/';
"="                        return '=';
";"                        return ';';
{lex_rexpfind}           { if (CMDBlock->rexExpected())
                           {
                              telllval.parsestr = parsercmd::charcopy(yytext, true);
                              return tknREXPFIND;                             
                           }
                           else
                              REJECT;
                         }
{lex_rexprplc}           { if (CMDBlock->rexExpected())
                           {
                              telllval.parsestr = parsercmd::charcopy(yytext, true);
                              return tknREXPRPLC;                             
                           }
                           else
                              REJECT;
                         }
0x[0-9A-Fa-f]+    |
{lxt_D}+                 { telllval.uint = parsercmd::getllint(yytext); return tknUINT;}
{lxt_D}+"."{lxt_D}*({lxt_E})?  |
{lxt_D}*"."{lxt_D}+({lxt_E})?  |
{lxt_D}+{lxt_E}          { telllval.real = atof(yytext); return tknREAL;}
{lex_ID}                 { if (!parsercmd::checkPrepVar(yytext))
                           {
                              const telldata::TType* ttype = CMDBlock->getTypeByName(telllval.parsestr);
                              if (NULL == ttype)
                                 return tknIDENTIFIER;
                              else
                                 return tknTYPEdef;
                           }
                         }
<*>.                       BEGIN(INITIAL);return tknERROR;
%% 
/**************************************************************************/
/*Support functions for the flex parser*/
/**************************************************************************/
int tellwrap() {
   return 1;/*line by line*/
}

void delete_tell_lex_buffer( void* b ) {
   tell_delete_buffer((YY_BUFFER_STATE) b);
}

//=============================================================================
// define scanner location tracking functions -
// see the link below - idea is taken from there
// from http://www.lrde.epita.fr/~akim/compil/gnuprog2/Advanced-Use-of-Flex.html
//=============================================================================

void parsercmd::location_step(YYLTYPE *loc)
{
   loc->first_column = loc->last_column;
   loc->first_line = loc->last_line;
}

void parsercmd::location_lines(YYLTYPE *loc, unsigned long num)
{
   loc->last_column = 1;
   loc->last_line += num;
}

void parsercmd::ccomment(YYLTYPE *loc)
{
   int c;
   while(0 < (c = yyinput()))
   {
      if(c == '\n')
      {
         loc->last_line++;
         loc->last_column = 1;
      }
      else 
      {
         loc->last_column++;
         if ('*' == c)
         {
            if ('/' == (c = yyinput()))
               break;
            else
               unput(c);
         }
      }
   }
   location_step(loc);
}

void parsercmd::cppcomment(YYLTYPE *loc)
{
   int c;
   while(0 < (c = yyinput()))
   {
      if(c == '\n')
      {
         loc->last_line++;
         loc->last_column = 1;
         break;
      }
      else 
         loc->last_column++;
   }
   location_step(loc);
}

//=============================================================================
// Some C string management stuff - avoid malloc
//=============================================================================

char* parsercmd::charcopy(std::string source, bool quotes) {
   unsigned long length = source.length() - (quotes ? 2 : 0);
   char* newstr = DEBUG_NEW char[length+2];
   memcpy(newstr,&(source.c_str()[quotes ? 1 : 0]),length);
   newstr[length] = 0x00;
   return newstr;
}

unsigned parsercmd::getllint(char* source) {
   char* boza;
   unsigned result = static_cast<unsigned>(strtoul(source, &boza, 0));
   return result;
}

//=============================================================================
// File include handling
//=============================================================================
int parsercmd::includefile(char* name, FILE* &handler)
{
   if ( include_stack_ptr >= MAX_INCLUDE_DEPTH )
      tell_log(console::MT_ERROR,"Too many nested includes");
   else
   {
      std::string nfname;
      std::string infomsg;
      if (Console->findTellFile(name, nfname))
      {
         FILE* newfilehandle = fopen(nfname.c_str(), "r");
         if (NULL != newfilehandle)
         {
            infomsg = "Parsing \"" + nfname + "\" ...";
            tell_log(console::MT_INFO,infomsg);
            handler = newfilehandle;
            /* create a new record with file handler and location objects*/
            include_stack[include_stack_ptr++] = DEBUG_NEW parsercmd::lexer_files(
                       static_cast<void*>(YY_CURRENT_BUFFER), DEBUG_NEW YYLTYPE(telllloc));
            /* switch the buffer to the new file */
            yy_switch_to_buffer(yy_create_buffer( newfilehandle, YY_BUF_SIZE ) );
            /* initialize the current error location object */
            telllloc.first_column = telllloc.last_column = 1;
            telllloc.first_line = telllloc.last_line = 1;
            telllloc.filename = name;
            /* keep the full file name (in case of #pragma once) */
            tellPP->pushTllFileName(nfname);
         }
         else
         {
            infomsg = "File \"" + nfname + "\" can't be open";
            tell_log(console::MT_ERROR,infomsg);
         }
      }
      else
      {
         infomsg = "File \"" + nfname + "\" not found";
         tell_log(console::MT_ERROR,infomsg);
      }
   }
   return include_stack_ptr;
}

int parsercmd::EOfile()
{
   if ( include_stack_ptr > 0 )
   {
      std::string nfname;
      std::string infomsg;
      /* get the previous file record from the array */
      parsercmd::lexer_files* prev = include_stack[--include_stack_ptr];
      /* take care to free the memory from the current file name */
      if (telllloc.filename) delete [] telllloc.filename;
	  /* restore the error location object*/
      telllloc = *(prev->location);
     /*close the file*/
      fclose(YY_CURRENT_BUFFER->yy_input_file);
     /* delete the current file buffer (the function below doesn't close it)*/
      yy_delete_buffer( YY_CURRENT_BUFFER );
      /* switch to the restored buffer */
      yy_switch_to_buffer(static_cast<YY_BUFFER_STATE>(prev->lexfilehandler));
      delete prev;
      nfname = tellPP->popTllFileName();
      if (!nfname.empty())
      {
         infomsg = "Back to \"" + nfname + "\" ...";
         tell_log(console::MT_INFO,infomsg);
      }
      return 1;
   }
  return 0;
}

static void parsercmd::telllex_fatal(std::string message)
{
   throw EXPTNtell_parser(message);
}

void parsercmd::newPrepVar(std::string source, bool empty)
{
   if (empty)
   {
      source.erase(source.find_first_of(" \n"));
      tellPP->define(source, std::string(""), telllloc);
   }
   else
      tellPP->preDefine(source, telllloc);
}

void parsercmd::newPrepVal(std::string source)
{
   tellPP->define(source);
}

bool parsercmd::checkPrepVar(const std::string source)
{
   std::string replacement;
   if (tellPP->check(source,replacement))
   {
      const char* rchar = replacement.c_str();
      for (unsigned long i = replacement.length() - 1; i >= 0; --i)
         unput(rchar[i]);
      telllloc.last_column -= replacement.length();
      return true;
   }
   else
   {
      telllval.parsestr = charcopy(source);
      return false;
   }
}
