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
/*void ciferror (std::string s);*/
%}

%debug
/*
%union {

}
*/
%start cifFile
/* token code T - type; C - command; P - primitive*/
%token                 tknPsem tknPdigit tknPremB tknPremE
%token                 tknCend tknCdefine tknCpolygon tknCbox tknCround
%token                 tknCwire tknClayer tknCcall tknCstart tknCfinish
%token                 tknTword tknTint tknTusertext tknTremtext tknTshortname
%token                 tknTupchar tknTblank

%%
cifFile:
     commands tknCend cifBlank             {}
;

commands:
                                           {}
   | command tknPsem                       {}
   | commands command tknPsem              {}
;

command:
     primCommand                           {}
   | defDeleteCommand                      {}
   | defDefineCommand                      {}
;

primCommands:
                                           {}
   |  primCommand tknPsem                  {}
   |  primCommands primCommand tknPsem     {}
;

primCommand:
     polygonCommand                        {}
   | boxCommand                            {}
   | roundFlashCommand                     {}
   | wireCommand                           {}
   | layerCommand                          {}
   | callCommand                           {}
   | userExtensionCommand                  {}
   | commentCommand                        {}
;

defDefineCommand:
     defStartCommand tknPsem primCommands defFinishCommand {}
;

defStartCommand:
     tknCdefine cifBlank tknCstart tknTword {}
   | tknCdefine cifBlank tknCstart tknTword cifSep tknTword cifSep tknTword {}
;

defDeleteCommand:
     tknCdefine cifBlank tknCdefine tknTword {}
;

defFinishCommand:
     tknCdefine cifBlank tknCfinish        {}
;

polygonCommand:
     tknCpolygon cifPath                   {}
;

boxCommand:
     tknCbox tknTword cifSep tknTword cifSep cifPoint    {}
   | tknCbox tknTword cifSep tknTword cifSep cifPoint cifSep cifPoint  {}
;

roundFlashCommand:
     tknCround tknTword cifSep cifPoint    {}
;

wireCommand:
     tknCwire tknTword cifSep cifPath      {}
;

layerCommand:
     tknClayer cifBlank tknTshortname      {}
;

callCommand:
     tknCcall tknTword cifTrans            {}
;

userExtensionCommand:
     tknPdigit tknTusertext                {}
;

commentCommand:
     tknPremB tknTremtext tknPremE         {}
;

cifPoint:
     tknTint cifSep tknTint                {}
;

cifTrans:
;

cifPath:
;

cifBlank:
                                           {}
   | tknTblank                             {}
;

cifSep:
     tknTupchar                            {}
   | tknTblank                             {}
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

