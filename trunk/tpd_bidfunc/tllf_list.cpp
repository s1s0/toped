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
int tellstdfunc::lstLENGTH::argsOK(argumentQ* amap, bool& strict)
{
   strict = true;
   return (!((amap->size() == 1) && ( (*((*amap)[0]))() &  telldata::tn_listmask  )));
}

NameList* tellstdfunc::lstLENGTH::callingConv(const telldata::typeMAP*)
{
   NameList* argtypes = DEBUG_NEW NameList();
   argtypes->push_back("int");
   argtypes->push_back("<...anything...> list");
   return argtypes;
}

int tellstdfunc::lstLENGTH::execute()
{
   telldata::TtList* pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   OPstack.push(DEBUG_NEW telldata::TtInt(pl->size()));
   delete pl;
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::lytPOINTDUMP::lytPOINTDUMP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayout()));
}

int tellstdfunc::lytPOINTDUMP::execute()
{
   telldata::TtLayout* layobject = static_cast<telldata::TtLayout*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   // The line below will crash spectacularly if the the target DB was changed after the
   // layout objects were selected. This is a general problem with all the functions of
   // this type
   PointVector plst = layobject->data()->dumpPoints();

   telldata::TtList *pl = DEBUG_NEW telldata::TtList(telldata::tn_pnt);
   for (unsigned i = 0; i < plst.size(); i++)
   {
      telldata::TtPnt* pp = DEBUG_NEW telldata::TtPnt(((real)plst[i].x()) / DBscale,
                                                      ((real)plst[i].y()) / DBscale );
      pl->add(pp);
   }
   OPstack.push(pl);
   delete layobject;
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::lytTYPEOF::lytTYPEOF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayout()));
}

int tellstdfunc::lytTYPEOF::execute()
{
   telldata::TtLayout* layobject = static_cast<telldata::TtLayout*>(OPstack.top());OPstack.pop();
   // The line below will crash spectacularly if the the target DB was changed after the
   // layout objects were selected. This is a general problem with all the functions of
   // this type
   word ltype = layobject->data()->lType();

   telldata::TtInt *lt = DEBUG_NEW telldata::TtInt(ltype);
   OPstack.push(lt);
   delete layobject;
   return EXEC_NEXT;
}

//============================================================================
int tellstdfunc::stdABS::argsOK(argumentQ* amap, bool& strict)
{
   strict = true;
   return !((amap->size() == 1) && (( (*((*amap)[0]))() == telldata::tn_real  ) ||
                                    ( (*((*amap)[0]))() == telldata::tn_int   )   ));
}

NameList* tellstdfunc::stdABS::callingConv(const telldata::typeMAP*)
{
   NameList* argtypes = DEBUG_NEW NameList();
   argtypes->push_back("real");
   argtypes->push_back("real");
   return argtypes;
}

int tellstdfunc::stdABS::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::TtReal(fabs(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdSIN::stdSIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdSIN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::TtReal(sin(angle)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdASIN::stdASIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdASIN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (asin(value) * 180.0) / M_PI;// translate in degrees
   OPstack.push(DEBUG_NEW telldata::TtReal(angle));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdCOS::stdCOS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdCOS::execute()
{
   real value = getOpValue(OPstack);
   real angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::TtReal(cos(angle)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdACOS::stdACOS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdACOS::execute()
{
   real value = getOpValue(OPstack);
   real angle = (acos(value) * 180.0) / M_PI;// translate in degrees
   OPstack.push(DEBUG_NEW telldata::TtReal(angle));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdTAN::stdTAN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdTAN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (value / 180.0 * M_PI);// translate in radians
   OPstack.push(DEBUG_NEW telldata::TtReal(tan(angle)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdATAN::stdATAN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdATAN::execute()
{
   real value = getOpValue(OPstack);
   real angle = (atan(value) * 180.0) / M_PI;// translate in degrees
   OPstack.push(DEBUG_NEW telldata::TtReal(angle));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdROUND::stdROUND(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdROUND::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) rint(value);
   OPstack.push(DEBUG_NEW telldata::TtInt(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdCEIL::stdCEIL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdCEIL::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) ceil(value);
   OPstack.push(DEBUG_NEW telldata::TtInt(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdFLOOR::stdFLOOR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdFLOOR::execute()
{
   real value = getOpValue(OPstack);
   int4b result = (int4b) floor(value);
   OPstack.push(DEBUG_NEW telldata::TtInt(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdFMODULO::stdFMODULO(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdFMODULO::execute()
{
   real valueY = getOpValue(OPstack);
   real valueX = getOpValue(OPstack);
   real result = fmod(valueX,valueY);
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdSQRT::stdSQRT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdSQRT::execute()
{
   real value = getOpValue(OPstack);
   real result = sqrt(value);
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdPOW::stdPOW(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdPOW::execute()
{
   real valueY = getOpValue(OPstack);
   real valueX = getOpValue(OPstack);
   real result = pow(valueX,valueY);
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdEXP::stdEXP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdEXP::execute()
{
   real value = getOpValue(OPstack);
   real result = exp(value);
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdLOG::stdLOG(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdLOG::execute()
{
   real value = getOpValue(OPstack);
   real result = log(value);
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdLOG10::stdLOG10(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdLOG10::execute()
{
   real value = getOpValue(OPstack);
   real result = log10(value);
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdSINH::stdSINH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdSINH::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::TtReal(sinh(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdASINH::stdASINH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdASINH::execute()
{
   real value = getOpValue(OPstack);
#ifdef WIN32
   real result = log(value + sqrt(value*value + 1.0));
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
#else
   OPstack.push(DEBUG_NEW telldata::TtReal(asinh(value)));
#endif
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdCOSH::stdCOSH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdCOSH::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::TtReal(cosh(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdACOSH::stdACOSH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdACOSH::execute()
{
   real value = getOpValue(OPstack);
#ifdef WIN32
   real result = log( value + sqrt( value*value - 1.0));
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
#else
   OPstack.push(DEBUG_NEW telldata::TtReal(acosh(value)));
#endif
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdTANH::stdTANH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdTANH::execute()
{
   real value = getOpValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::TtReal(tanh(value)));
   return EXEC_NEXT;
}

//============================================================================
tellstdfunc::stdATANH::stdATANH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdATANH::execute()
{
   real value = getOpValue(OPstack);
#ifdef WIN32
   real result = log((1.0 + value) / (1.0 - value)) / 2.0;
   OPstack.push(DEBUG_NEW telldata::TtReal(result));
#else
   OPstack.push(DEBUG_NEW telldata::TtReal(atanh(value)));
#endif
   return EXEC_NEXT;
}
