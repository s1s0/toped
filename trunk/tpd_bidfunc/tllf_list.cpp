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
//           $URL$
//        Created: Sat Apr 28 2007
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all list related functions in TELL
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#include <sstream>
#include "tllf_list.h"

//=============================================================================
int tellstdfunc::lstLENGTH::argsOK(argumentQ* amap)
{
   return (!((amap->size() == 1) && ( (*((*amap)[0]))() &  telldata::tn_listmask  )));
}

nameList* tellstdfunc::lstLENGTH::callingConv(const telldata::typeMAP*)
{
   nameList* argtypes = new nameList();
   argtypes->push_back("int");
   argtypes->push_back("<...anything...> list");
   return argtypes;
}

int tellstdfunc::lstLENGTH::execute()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   OPstack.push(new telldata::ttint(pl->size()));
   delete pl;
   return EXEC_NEXT;
}
