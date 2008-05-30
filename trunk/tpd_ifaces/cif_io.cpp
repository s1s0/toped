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
//        Created: Sun May 04 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: CIF parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <sstream>
#include "tpdph.h"
#include "cif_io.h"
#include "../tpd_common/outbox.h"

extern void*   cif_scan_string(const char *str);
extern void    my_delete_yy_buffer( void* b );
extern int     cifparse(); // Calls the bison generated parser
extern FILE*   cifin;
extern int     cifdebug;

CIFin::CIFFile::CIFFile(std::string filename)
{
//   void* buf = cif_scan_string( command.mb_str() );

   if (!(cifin = fopen(filename.c_str(),"r")))
   {// open the input file
//      std::ostringstream info;
//      info << "File "<< filename <<" can NOT be opened";
//      tell_log(console::MT_ERROR,info.str());
      _status = 0;
      return;
   }
   // feed the flex with the buffer of the input file
   //(cifin is a global variable defined in the flex generated scanner)
   cifin = fopen(filename.c_str(), "r");
   cifdebug = 1;
   // run the bison generated parser
   cifparse();
//   my_delete_yy_buffer( buf );
   fclose(cifin);
}
