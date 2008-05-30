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

%{
#include <sstream>
#include "../tpd_common/outbox.h"
#include "cif_io.h"
int boza;
/*void ciferror (std::string s);*/
%}

%debug
/*
%union {

}
*/
%start cifFile
/* token code T - type; C - command; P - primitive*/
%token                 tknERROR
%token                 tknPsem tknPdigit tknPremB tknPremE
%token                 tknCend tknCdefine tknCpolygon tknCbox tknCround
%token                 tknCwire tknClayer tknCcall tknCstart tknCfinish
%token                 tknTint tknTusertext tknTremtext tknTshortname
%token                 tknTupchar tknTblank

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
   | commentCommand                        {boza = 10047;}
;

defDefineCommand:
     defStartCommand tknPsem primCommands defFinishCommand {boza = 10050;}
;

defStartCommand:
     tknCdefine cifBlank tknCstart cifBlank tknTint       {/*check tknTword*/boza = 10060;}
   | tknCdefine cifBlank tknCstart cifBlank tknTint cifSep tknTint cifSep tknTint {/*check 3*tknTword*/boza = 10061;}
;

defDeleteCommand:
     tknCdefine cifBlank tknCdefine tknTint {/*check tknTword*/boza = 10070;}
;

defFinishCommand:
     tknCdefine cifBlank tknCfinish        {boza = 10080;}
;

polygonCommand:
     tknCpolygon cifPath                   {boza = 10090;}
;

boxCommand: /*discrepancy with the formal syntax*/
     tknCbox cifBlank tknTint cifSep tknTint cifSep cifPoint    {/*check 2*tknTword*/boza = 10100;}
   | tknCbox cifBlank tknTint cifSep tknTint cifSep cifPoint cifSep cifPoint  {/*check 2*tknTword*/boza = 10101;}
;

roundFlashCommand:
     tknCround tknTint cifSep cifPoint    {/*check tknTword*/boza = 10110;}
;

wireCommand:
     tknCwire tknTint cifSep cifPath      {/*check tknTword*/boza = 10120;}
;

layerCommand:
     tknClayer cifBlank tknTshortname      {boza = 10130;}
;

callCommand: /*discrepancy with the formal syntax*/
     tknCcall cifBlank tknTint cifTrans            {/*check tknTword*/boza = 10140;}
;

userExtensionCommand:
     tknPdigit tknTusertext                {boza = 10150;}
;

commentCommand:
     tknPremB tknTremtext tknPremE         {boza = 10160;}
   | tknPremB commentCommand tknPremE      {boza = 10161;}
;

cifPoint:
     tknTint cifSep tknTint                {boza = 10170;}
;

cifTrans:
;

cifPath:
;

cifBlank:
                                           {boza = 10180;}
   | tknTblank                             {boza = 10181;}
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
/*      ost << "line " << telllloc.first_line << ": col " << telllloc.first_column
            << ": " << s;*/
      ost << s;
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

