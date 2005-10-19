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
// ------------------------------------------------------------------------ =
//    Description: Common header for all files. Currently use for 
//	windows specific things
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//===========================================================================

#ifndef COMMON_H
#define COMMON_H

#if WIN32
#include <windows.h>
#define rint floor
#define remainder fmod
#pragma warning( disable : 4786 ) 
#endif

#endif 
 