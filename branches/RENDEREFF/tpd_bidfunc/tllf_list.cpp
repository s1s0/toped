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
#include "tedat.h"
#include "viewprop.h"

extern layprop::PropertyCenter*  PROPC;

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
tellstdfunc::lytPOINTDUMP::lytPOINTDUMP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlayout()));
}

int tellstdfunc::lytPOINTDUMP::execute()
{
   telldata::ttlayout* layobject = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   // The line below will crash spectacularly if the the target DB was changed after the
   // layout objects were selected. This is a general problem with all the functions of
   // this type
   pointlist plst = layobject->data()->dumpPoints();

   telldata::ttlist *pl = DEBUG_NEW telldata::ttlist(telldata::tn_pnt);
   for (unsigned i = 0; i < plst.size(); i++)
   {
      telldata::ttpnt* pp = DEBUG_NEW telldata::ttpnt(((real)plst[i].x()) / DBscale,
                                                      ((real)plst[i].y()) / DBscale );
      pl->add(pp);
   }
   OPstack.push(pl);
   delete layobject;
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::lytTYPEOF::lytTYPEOF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlayout()));
}

int tellstdfunc::lytTYPEOF::execute()
{
   telldata::ttlayout* layobject = static_cast<telldata::ttlayout*>(OPstack.top());OPstack.pop();
   // The line below will crash spectacularly if the the target DB was changed after the
   // layout objects were selected. This is a general problem with all the functions of
   // this type
   word ltype = layobject->data()->lType();

   telldata::ttint *lt = DEBUG_NEW telldata::ttint(ltype);
   OPstack.push(lt);
   delete layobject;
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
tellstdfunc::stdSIN::stdSIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdSIN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::ttreal(sin(angle)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdASIN::stdASIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdASIN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (asin(value) * 180.0) / M_PI;// translate in degrees
   OPstack.push(DEBUG_NEW telldata::ttreal(angle));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdCOS::stdCOS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdCOS::execute()
{
   real value = getOpValue(OPstack);
   real angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::ttreal(cos(angle)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdACOS::stdACOS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdACOS::execute()
{
   real value = getOpValue(OPstack);
   real angle = (acos(value) * 180.0) / M_PI;// translate in degrees
   OPstack.push(DEBUG_NEW telldata::ttreal(angle));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdTAN::stdTAN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdTAN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::ttreal(tan(angle)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdATAN::stdATAN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdATAN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (atan(value) * 180.0) / M_PI;// translate in degrees
   OPstack.push(DEBUG_NEW telldata::ttreal(angle));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdROUND::stdROUND(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdROUND::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) rint(value);
   OPstack.push(DEBUG_NEW telldata::ttint(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdCEIL::stdCEIL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdCEIL::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) ceil(value);
   OPstack.push(DEBUG_NEW telldata::ttint(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdFLOOR::stdFLOOR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdFLOOR::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) floor(value);
   OPstack.push(DEBUG_NEW telldata::ttint(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdFMODULO::stdFMODULO(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdFMODULO::execute()
{
   real valueY = getOpValue(OPstack);
   real valueX = getOpValue(OPstack);
   real result = fmod(valueX,valueY);
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdSQRT::stdSQRT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdSQRT::execute()
{
   real value = getOpValue(OPstack);
   real result = sqrt(value);
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdPOW::stdPOW(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdPOW::execute()
{
   real valueY = getOpValue(OPstack);
   real valueX = getOpValue(OPstack);
   real result = pow(valueX,valueY);
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdEXP::stdEXP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdEXP::execute()
{
   real value = getOpValue(OPstack);
   real result = exp(value);
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdLOG::stdLOG(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdLOG::execute()
{
   real value = getOpValue(OPstack);
   real result = log(value);
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdLOG10::stdLOG10(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdLOG10::execute()
{
   real value = getOpValue(OPstack);
   real result = log10(value);
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdSINH::stdSINH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdSINH::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::ttreal(sinh(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdASINH::stdASINH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdASINH::execute()
{
   real value = getOpValue(OPstack);
#ifdef WIN32
   real result = log(value + sqrt(value*value + 1.0));
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
#else
   OPstack.push(DEBUG_NEW telldata::ttreal(asinh(value)));
#endif
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdCOSH::stdCOSH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdCOSH::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::ttreal(cosh(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdACOSH::stdACOSH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdACOSH::execute()
{
   real value = getOpValue(OPstack);
#ifdef WIN32
   real result = log( value + sqrt( value*value - 1.0));
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
#else
   OPstack.push(DEBUG_NEW telldata::ttreal(acosh(value)));
#endif
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdTANH::stdTANH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdTANH::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::ttreal(tanh(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdATANH::stdATANH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdATANH::execute()
{
   real value = getOpValue(OPstack);
#ifdef WIN32
   real result = log((1.0 + value) / (1.0 - value)) / 2.0;
   OPstack.push(DEBUG_NEW telldata::ttreal(result));
#else
   OPstack.push(DEBUG_NEW telldata::ttreal(atanh(value)));
#endif
   return EXEC_NEXT;
}
