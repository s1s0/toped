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
//           $URL: https://toped.googlecode.com/svn/branches/MULTI_DB2/tpd_DB/tedat_ext.cpp $
//        Created: Mon Aug 09 2010
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Layout primitives
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 1559 $
//          $Date: 2010-06-29 00:03:32 +0300 (Âò, 29 èþí 2010) $
//        $Author: krustev.svilen $
//===========================================================================

#include "tpdph.h"
#include "tedat_ext.h"

void laydata::TdtBoxEXT::setInt(int extInt)
{
	_extInt = extInt;
}
int laydata::TdtBoxEXT::getInt(void)
{
	return _extInt;
}
void laydata::TdtBoxEXT::setLong(long extLong)
{
	_extLong = extLong;
}
long laydata::TdtBoxEXT::getLong(void)
{
	return _extLong;
}
void laydata::TdtBoxEXT::setString(const std::string &extString)
{
	_extString = extString;
}
std::string laydata::TdtBoxEXT::getString(void)
{
	return _extString;
}
void	laydata::TdtBoxEXT::setClientData(void* clientData)
{
	_clientData = clientData;
}
void*	laydata::TdtBoxEXT::getClientData(void)
{
	return _clientData;
}


void laydata::TdtPolyEXT::setInt(int extInt)
{
	_extInt = extInt;
}
int laydata::TdtPolyEXT::getInt(void)
{
	return _extInt;
}
void laydata::TdtPolyEXT::setLong(long extLong)
{
	_extLong = extLong;
}
long laydata::TdtPolyEXT::getLong(void)
{
	return _extLong;
}
void laydata::TdtPolyEXT::setString(const std::string &extString)
{
	_extString = extString;
}
std::string laydata::TdtPolyEXT::getString(void)
{
	return _extString;
}
void	laydata::TdtPolyEXT::setClientData(void* clientData)
{
	_clientData = clientData;
}
void*	laydata::TdtPolyEXT::getClientData(void)
{
	return _clientData;
}