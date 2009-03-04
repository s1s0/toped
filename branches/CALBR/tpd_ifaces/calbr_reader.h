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
//           $URL: https://gaitukevich@svn.berlios.de/svnroot/repos/toped/trunk/tpd_DB/browsers.h $
//        Created: Mon Aug 11 2003
//        $Author: gaitukevich.s $
//    Description: Reader of Mentor Graphics Calibre drc errors files
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 872 $
//          $Date: 2008-11-23 21:22:48 +0800 (Вс, 23 ноя 2008) $
//        $Author: s_krustev $
//===========================================================================
//      Comments :
//===========================================================================

#if !defined(CALBR_READER_H_INCLUDED)
#define CALBR_READER_H_INCLUDED

#include <string>

namespace Calbr
{

class CalbrFile
{
public:
	CalbrFile(const std::string &fileName);
	~CalbrFile();
private:
	FILE*          _calbrFile;
	std::string    _fileName;
	std::string    _cellName;
	long				_precision;

};
}
#endif //CALBR_READER_H_INCLUDED