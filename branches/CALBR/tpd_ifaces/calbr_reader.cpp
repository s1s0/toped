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
#include "tpdph.h"
#include <sstream>
#include "calbr_reader.h"
#include "../tpd_common/outbox.h"


Calbr::CalbrFile::CalbrFile(const std::string &fileName)
{
	_fileName = fileName;
	std::string fname(convertString(_fileName));
	if (!(_calbrFile = fopen(fname.c_str(),"rt"))) // open the input file
   {
      //_status = cfs_FNF; 
		return;
   }

}