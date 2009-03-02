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
//   This file is a part of Toped project (C) 2001-2007 Toped developers    =
// ------------------------------------------------------------------------ =
//          $URL: https://gaitukevich@svn.berlios.de/svnroot/repos/toped/trunk/tpd_DB/browsers.cpp $
//        Created: Mon Aug 11 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GDSII/TDT hierarchy browser, layer browser, TELL fuction
//                 definition browser
//===========================================================================
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 955 $
//          $Date: 2009-02-25 08:12:18 +0800 (—р, 25 фев 2009) $
//        $Author: gaitukevich.s $
//    Description: Reader of Mentor Graphics Calibre drc errors files
//===========================================================================
//      Comments :
//===========================================================================

#include <sstream>
#include "calbr_reader.h"
#include "calbr_yacc.h"
#include "../tpd_common/outbox.h"

extern void*   new_calbr_lex_buffer( FILE* cifin );
extern void    delete_calbr_lex_buffer( void* b ) ;
extern int     calbrparse(); // Calls the bison generated parser
//extern FILE*   cifin;
//extern int     cifdebug;
//extern int     cifnerrs;



Calbr::CalbrFile::CalbrFile(const std::string &fileName)
{
	_fileName = fileName;
	std::string fname(convertString(_fileName));
	if (!(_calbrFile = fopen(fname.c_str(),"rt"))) // open the input file
   {
      //_status = cfs_FNF; 
		return;
   }
	// feed the flex with the buffer of the input file
   void* b = new_calbr_lex_buffer( _calbrFile );
	std::ostringstream info;
   info << "Parsing \"" << _fileName << "\" using CIF grammar";
   tell_log(console::MT_INFO,info.str());
  
   // run the bison generated parser
//   ciflloc.first_column = ciflloc.first_line = 1;
//   ciflloc.last_column  = ciflloc.last_line  = 1;
/*   cifdebug = 1;*/
   calbrparse();
   delete_calbr_lex_buffer( b );
}