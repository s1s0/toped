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

#include "tpdph.h"
#include <sstream>
#include <math.h>
#include "tllf_list.h"

//============================================================================
int tellstdfunc::lstLENGTH::argsOK(argumentQ* amap)
{
   return (!((amap->size() == 1) && ( (*((*amap)[0]))() &  telldata::tn_listmask  )));
}

nameList* tellstdfunc::lstLENGTH::callingConv(const telldata::typeMAP*)
{
   nameList* argtypes = DEBUG_NEW nameList();
   argtypes->push_back("int");
   argtypes->push_back("<...anything...> list");
   return argtypes;
}

int tellstdfunc::lstLENGTH::execute()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   OPstack.push(DEBUG_NEW telldata::ttint(pl->size()));
   delete pl;
   return EXEC_NEXT;
}

//============================================================================
int tellstdfunc::stdABS::argsOK(argumentQ* amap)
{
   return !((amap->size() == 1) && (( (*((*amap)[0]))() == telldata::tn_real  ) ||
                                    ( (*((*amap)[0]))() == telldata::tn_int   )   ));
}

nameList* tellstdfunc::stdABS::callingConv(const telldata::typeMAP*)
{
   nameList* argtypes = DEBUG_NEW nameList();
   argtypes->push_back("real");
   argtypes->push_back("real");
   return argtypes;
}

int tellstdfunc::stdABS::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::ttreal(fabs(value)));
   return EXEC_NEXT;
}

//============================================================================
int tellstdfunc::stdSIN::argsOK(argumentQ* amap)
{
   return !((amap->size() == 1) && (( (*((*amap)[0]))() == telldata::tn_real  ) ||
                                    ( (*((*amap)[0]))() == telldata::tn_int   )   ));
}

nameList* tellstdfunc::stdSIN::callingConv(const telldata::typeMAP*)
{
   nameList* argtypes = DEBUG_NEW nameList();
   argtypes->push_back("real");
   argtypes->push_back("real");
   return argtypes;
}

int tellstdfunc::stdSIN::execute()
{
   real value = getOpValue(OPstack);
   double angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::ttreal(sin(angle)));
   return EXEC_NEXT;
}

//============================================================================
int tellstdfunc::stdCOS::argsOK(argumentQ* amap)
{
   return !((amap->size() == 1) && (( (*((*amap)[0]))() == telldata::tn_real  ) ||
                                    ( (*((*amap)[0]))() == telldata::tn_int   )   ));
}

nameList* tellstdfunc::stdCOS::callingConv(const telldata::typeMAP*)
{
   nameList* argtypes = DEBUG_NEW nameList();
   argtypes->push_back("real");
   argtypes->push_back("real");
   return argtypes;
}

int tellstdfunc::stdCOS::execute()
{
   real value = getOpValue(OPstack);
   double angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::ttreal(cos(angle)));
   return EXEC_NEXT;
}

//============================================================================
int tellstdfunc::stdRINT::argsOK(argumentQ* amap)
{
   return !((amap->size() == 1) && (( (*((*amap)[0]))() == telldata::tn_real  ) ||
                                    ( (*((*amap)[0]))() == telldata::tn_int   )   ));
}

nameList* tellstdfunc::stdRINT::callingConv(const telldata::typeMAP*)
{
   nameList* argtypes = DEBUG_NEW nameList();
   argtypes->push_back("int");
   argtypes->push_back("real");
   return argtypes;
}

int tellstdfunc::stdRINT::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) rint(value);
   OPstack.push(DEBUG_NEW telldata::ttint(result));
   return EXEC_NEXT;
}
