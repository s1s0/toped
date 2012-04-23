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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Mon Aug 09 2010
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Layout primitives
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "tedat_ext.h"

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
void   laydata::TdtBoxEXT::setClientData(void* clientData)
{
   _clientData = clientData;
}
void*   laydata::TdtBoxEXT::getClientData(void)
{
   return _clientData;
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
void   laydata::TdtPolyEXT::setClientData(void* clientData)
{
   _clientData = clientData;
}
void*   laydata::TdtPolyEXT::getClientData(void)
{
   return _clientData;
}


void laydata::TdtWireEXT::setLong(long extLong)
{
   _extLong = extLong;
}
long laydata::TdtWireEXT::getLong(void)
{
   return _extLong;
}
void laydata::TdtWireEXT::setString(const std::string &extString)
{
   _extString = extString;
}
std::string laydata::TdtWireEXT::getString(void)
{
   return _extString;
}
void   laydata::TdtWireEXT::setClientData(void* clientData)
{
   _clientData = clientData;
}
void*   laydata::TdtWireEXT::getClientData(void)
{
   return _clientData;
}

