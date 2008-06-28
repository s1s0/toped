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
//           $URL$
//        Created: Sun May 18 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: CIF parser
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
 *    %name-prefix="cif"
 *    %yacc
 *
 * to create multiple parsers in one project.
 *
 */
/*Switch on code processing locations for more acurate error messages*/
%locations
%{
#include "tpdph.h"
#include <sstream>
#include "../tpd_common/outbox.h"
#include "../tpd_common/ttt.h"
#include "cif_io.h"
int boza;
extern CIFin::CIFFile* CIFInFile;

/*void ciferror (std::string s);*/
void ciferror(std::string s, TpdYYLtype loc);
bool checkPositive(int var, TpdYYLtype loc);

%}

%debug

%union {
   unsigned       word;
   int            integer;
   char*          identifier;
   TP*            point;
   pointlist*     path;
   CTM*           ctmp;
}

%start cifFile
/* token code T - type; C - command; P - primitive*/
%token                 tknERROR
%token                 tknPsem tknPdigit tknPremB tknPremE
%token                 tknCend tknCdefine tknCpolygon tknCbox tknCround
%token                 tknCwire tknClayer tknCcall tknCstart tknCfinish
%token                 tknTint tknTshortname tknTuserText
%token                 tknTupchar tknTblank tknTremtext
%token                 tknCtranslate tknCmirror tknCmirx tknCmiry tknCrotate
%token                 tknP9
%type  <identifier>    commentCommand tknTremtext tknTshortname tknTuserText
%type  <integer>       tknTint
%type  <point>         cifPoint
%type  <path>          cifPath
%type  <ctmp>          cifTrans cifLtrans

%%
cifFile:
     tknCend cifBlank                      {boza = 10000;}
   | commands tknCend cifBlank             {boza = 10001;}
;

commands:
     cifBlank command tknPsem              {boza = 10010;}
   | commands cifBlank command tknPsem     {boza = 10011;}
;

primCommands:
      cifBlank primCommand tknPsem         {boza = 10020;}
   |  primCommands cifBlank primCommand tknPsem     {boza = 10021;}
;

command:
     primCommand                           {boza = 10030;}
   | defDeleteCommand                      {boza = 10030;}
   | defDefineCommand                      {boza = 10030;}
;

primCommand:
     polygonCommand                        {boza = 10040;}
   | boxCommand                            {boza = 10041;}
   | roundFlashCommand                     {boza = 10042;}
   | wireCommand                           {boza = 10043;}
   | layerCommand                          {boza = 10044;}
   | callCommand                           {boza = 10045;}
   | userExtensionCommand                  {boza = 10046;}
   | UEC_cellName                          {             }
   | commentCommand                        {delete $1;}
;

defDefineCommand:
     defStartCommand tknPsem primCommands defFinishCommand {boza = 10050;}
;

defStartCommand:
     tknCdefine cifBlank tknCstart cifBlank tknTint {
      if (checkPositive($5, @5))
         CIFInFile->addStructure($5);
   }
   | tknCdefine cifBlank tknCstart cifBlank tknTint cifSep tknTint cifSep tknTint {
      bool valid = true;
      valid &= checkPositive($5,@5);
      valid &= checkPositive($7,@7);
      valid &= checkPositive($9,@9);
      if (valid)
         CIFInFile->addStructure($5, $7, $9);
   }
;

defDeleteCommand:
     tknCdefine cifBlank tknCdefine tknTint {/*check tknTword*/ciferror("defDeleteCommand - not implemented yet", @1);}
;

defFinishCommand:
     tknCdefine cifBlank tknCfinish       {
      CIFInFile->doneStructure();
   }
;

layerCommand:
     tknClayer cifBlank tknTshortname     {
      CIFInFile->secureLayer($3);
      delete $3;
   }
;

polygonCommand: /*discrepancy with the formal syntax*/
     tknCpolygon cifBlank cifPath         {
      CIFInFile->addPoly($3);
   }
;

boxCommand: /*discrepancy with the formal syntax*/
     tknCbox cifBlank tknTint cifSep tknTint cifSep cifPoint    {
      bool valid = true;
      valid &= checkPositive($3,@3);
      valid &= checkPositive($5,@5);
      if (valid)
         CIFInFile->addBox($3, $5, $7);
   }
   | tknCbox cifBlank tknTint cifSep tknTint cifSep cifPoint cifSep cifPoint  {
      bool valid = true;
      valid &= checkPositive($3,@3);
      valid &= checkPositive($5,@5);
      if (valid)
         CIFInFile->addBox($3, $5, $7, $9);
   }
;

roundFlashCommand:
     tknCround tknTint cifSep cifPoint    {/*check tknTword*/ciferror("roundFlashCommand - not implemented yet", @1);}
;

wireCommand:
     tknCwire cifBlank tknTint cifSep cifPath      {
      if (checkPositive($3, @3))
         CIFInFile->addWire($5, $3);
   }
;

callCommand: /*discrepancy with the formal syntax*/
     tknCcall cifBlank tknTint cifLtrans  {
      if (checkPositive($3, @3))
         CIFInFile->addRef($3, $4);
   }
;

userExtensionCommand:
     tknPdigit tknTuserText                {ciferror("Unsupported user command", @1);}
;

UEC_cellName:
     tknP9 tknTuserText                    {
      CIFInFile->curCellName($2);
      delete $2;
   }
;

commentCommand:
     tknPremB tknTremtext tknPremE         {
      $$ = $2;
   }
   | tknPremB commentCommand tknPremE      {
      $$ = $2;
   }
;

cifPoint:
     tknTint cifSep tknTint                {
      $$ = DEBUG_NEW TP($1, $3);
   }
;

cifTrans:
     tknCtranslate cifBlank cifPoint       {
      $$ = new CTM(); $$->Translate(*$3);
   }
   | tknCmirror cifBlank tknCmirx          {
      $$ = new CTM(); $$->FlipY(); /*TODO CHECK!*/
   }
   | tknCmirror cifBlank tknCmiry          {
      $$ = new CTM(); $$->FlipX(); /*TODO CHECK!*/
   }
   | tknCrotate cifBlank cifPoint          {
      $$ = new CTM(); $$->Rotate(*($3)); /*TODO CHECK!*/
   }
;

cifLtrans:
                                           {
      $$ = new CTM();
   }
   | cifLtrans cifBlank cifTrans           {
      $$ = new CTM(*($1) * (*($3)));
      delete $1;
      delete $3;
   }
;

cifPath:
     cifPoint                             {
      $$ = DEBUG_NEW pointlist();
      $$->push_back(*($1));
      delete ($1);
   }
   | cifPath cifSep cifPoint              {
      $1->push_back(*($3));
      delete ($3);
}
;

cifBlank:
                                           {boza = 10180;}
   | tknTblank                             {boza = 10181;}
/*   | cifBlank tknTblank                    {             }*/
;

cifSep:
     tknTupchar                            {boza = 10190;}
   | tknTblank                             {boza = 10191;}
;
%%

/*-------------------------------------------------------------------------*/
int ciferror (char *s)
{  /* Called by yyparse on error */
      std::ostringstream ost;
      ost << "line " << ciflloc.first_line << ": col " << ciflloc.first_column
            << ": " << s;
//      ost << s;
      tell_log(console::MT_ERROR,ost.str());
      return 0;
}

// void ciferror (std::string s)
// {
//    if (cfd) cfd->incErrors();
//    else     yynerrs++;
//    std::ostringstream ost;
//    ost << "line " << telllloc.first_line << ": col " << telllloc.first_column << ": " << s;
//    ost << s;
//    tell_log(console::MT_ERROR,ost.str());
// }

void ciferror (std::string s, TpdYYLtype loc)
{
   yynerrs++;
   std::ostringstream ost;
   ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
   if (loc.filename) {
      std::string fn = loc.filename;
      ost << "in file \"" << fn << "\" : ";
   }
   ost << s;
   tell_log(console::MT_ERROR,ost.str());
}

bool checkPositive(int var, TpdYYLtype loc)
{
   if (var < 0)
   {
      ciferror("Positive integer expected", loc);
      return false;
   }
   return true;
}
