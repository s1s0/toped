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
//        Created: Fri Nov 08 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TELL interpreter
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <algorithm>
#include "tellyzer.h"
#include "tldat.h"
#include "outbox.h"

//-----------------------------------------------------------------------------
// Definition of tell debug macros
//-----------------------------------------------------------------------------
//#define TELL_DEBUG_ON
#ifdef TELL_DEBUG_ON
#define TELL_DEBUG(a)  printf("%s \n", #a);
#else
#define TELL_DEBUG(a)
#endif

extern void tellerror(std::string s, TpdYYLtype loc);
extern void tellerror(std::string s);

parsercmd::cmdBLOCK*       CMDBlock = NULL;
parsercmd::TellPreProc*    tellPP   = NULL;
console::toped_logfile     LogFile;
//-----------------------------------------------------------------------------
// Initialize some static members
//-----------------------------------------------------------------------------
// Table of defined functions
parsercmd::FunctionMAP        parsercmd::cmdBLOCK::_funcMAP;
// Table of internal functions (used by the parser internally)
parsercmd::FunctionMAP        parsercmd::cmdBLOCK::_internalFuncMap;
// Table of current nested blocks
parsercmd::BlockSTACK         parsercmd::cmdBLOCK::_blocks;
//The state (to be) of the DB after the last function call
bool                          parsercmd::cmdBLOCK::_dbUnsorted = false;
// Operand stack
telldata::operandSTACK        parsercmd::cmdVIRTUAL::OPstack;
// UNDO Operand stack
telldata::UNDOPerandQUEUE     parsercmd::cmdVIRTUAL::UNDOPstack;
// UNDO utility stack (used for deleted cell definitions)
parsercmd::UndoUQUEUE         parsercmd::cmdVIRTUAL::UNDOUstack;
// UNDO command queue
parsercmd::UndoQUEUE          parsercmd::cmdVIRTUAL::UNDOcmdQ;
// Recovery status
bool parsercmd::cmdSTDFUNC::_ignoreOnRecovery = false;
// Thread execution indicator
bool parsercmd::cmdSTDFUNC::_threadExecution = false;
// Depth of the UNDO stack
word parsercmd::cmdBLOCK::_undoDepth = 100;


//=============================================================================
parsercmd::TellPreProc::TellPreProc() :
   _preDef      (""   ),
   _lastError   (false),
   _repeatLC    (false)
{
   _ppState.push(ppINACTIVE);
   _ifdefDepth.push(0);
}

void parsercmd::TellPreProc::define(std::string var, std::string val, const TpdYYLtype& loc)
{
   assert("" == _preDef);
   if (_variables.end() != _variables.find(var))
   {
      std::ostringstream ost;
      ost << "Variable \""<< var <<"\" redefined";
      ppWarning(ost.str(), loc);
   }
   _variables[var] = val;
}

void parsercmd::TellPreProc::undefine(std::string var, const TpdYYLtype& loc)
{
   VariableMap::iterator CV = _variables.find(var);
   if (_variables.end() == CV)
   {
      std::ostringstream ost;
      ost << "Variable \""<< var <<"\" is not defined. #undef is ignored";
      ppWarning(ost.str(), loc);
   }
   else
      _variables.erase(CV);
}

void parsercmd::TellPreProc::define(std::string val)
{
   assert("" != _preDef);
   _variables[_preDef] = val;
   _preDef = "";
}

void parsercmd::TellPreProc::cmdlDefine(std::string varval)
{
   size_t eqMark = varval.find('=');
   if (std::string::npos == eqMark)
      _variables[varval] = std::string("");
   else
      _variables[varval.substr(0,eqMark)] = varval.substr(eqMark+1);
}

void parsercmd::TellPreProc::preDefine(std::string var, const TpdYYLtype& loc)
{
   assert("" == _preDef);
   if (_variables.end() != _variables.find(var))
   {
      std::ostringstream ost;
      ost << "Variable \""<< var <<"\" redefined";
      ppWarning(ost.str(), loc);
   }
   _preDef = var;
}

bool parsercmd::TellPreProc::check(std::string var, std::string& val)
{
   VariableMap::iterator CV = _variables.find(var);
   if (_variables.end() == CV)
   {
      val = var; return false;
   }
   else
   {
      val = CV->second; return true;
   }
}

bool parsercmd::TellPreProc::ppIfDef(std::string var)
{
   if (_variables.end() != _variables.find(var))
   {
      _ppState.push(ppACTIVE);
      return true;
   }
   _ppState.push(ppBYPASS);
   return false;
}

bool parsercmd::TellPreProc::ppIfNDef(std::string var)
{
   if (_variables.end() == _variables.find(var))
   {
      _ppState.push(ppACTIVE);
      return true;
   }
   _ppState.push(ppBYPASS);
   return false;
}

void parsercmd::TellPreProc::ppPush()
{
   _ppState.push(ppDEEPBYPASS);
}

bool parsercmd::TellPreProc::ppElse(const TpdYYLtype& loc)
{
   switch(_ppState.top())
   {
      case ppACTIVE    : _ppState.pop(); _ppState.push(ppBYPASS); return false;
      case ppBYPASS    : _ppState.pop(); _ppState.push(ppACTIVE); return true;
      case ppDEEPBYPASS:                                          return false;
      default      : {
         _lastError = true;
         ppError("Unexpected #else", loc);
         return true;
      }
   }
   return true; // dummy statement, to prevent compiler warnings
}


bool parsercmd::TellPreProc::ppEndIf(const TpdYYLtype& loc)
{
   bool bypassStatus = false;
   if (1 == _ppState.size())
   {
      assert(ppINACTIVE == _ppState.top());
      _lastError = true;
      ppError("Unexpected #endif", loc);
   }
   else
   {
      _ppState.pop();
      bypassStatus = ((ppBYPASS     == _ppState.top()) ||
                      (ppDEEPBYPASS == _ppState.top())   );
   }
   return bypassStatus;
}

bool parsercmd::TellPreProc::lastError()
{
   bool ret = _lastError;
   _lastError = false;
   return ret;
}

void parsercmd::TellPreProc::checkEOF()
{
   assert(0 < _ifdefDepth.size());
   if (_ifdefDepth.top() != _ppState.size())
   {
      _lastError = true;
      tell_log(console::MT_ERROR,"Unterminated #if at the end of the file");
      while (1 < _ppState.size())
         _ppState.pop();
      assert(ppINACTIVE == _ppState.top());
      while (1 < _ifdefDepth.size())
         _ifdefDepth.pop();
   }
   else
      _ifdefDepth.pop();
}

void parsercmd::TellPreProc::markBOF()
{
   _ifdefDepth.push(_ppState.size());
}

void parsercmd::TellPreProc::ppError(std::string msg, const TpdYYLtype& loc)
{
   std::ostringstream ost;
   ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
   if (loc.filename) {
      std::string fn = loc.filename;
      ost << "in file \"" << fn << "\" : ";
   }
   ost << msg;
   tell_log(console::MT_ERROR,ost.str());
}

void parsercmd::TellPreProc::ppWarning(std::string msg, const TpdYYLtype& loc)
{
   std::ostringstream ost;
   ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
   if (loc.filename) {
      std::string fn = loc.filename;
      ost << "in file \"" << fn << "\" : ";
   }
   ost << "[PREP]" << msg;
   tell_log(console::MT_WARNING,ost.str());
}

std::string parsercmd::TellPreProc::popTllFileName()
{
   assert(!_tllFN.empty());
   _tllFN.pop();
   if (!_tllFN.empty())
      return _tllFN.top();
   else return std::string("");
}

bool parsercmd::TellPreProc::pragOnce()
{
   if(_tllFN.empty())
   {
      tell_log(console::MT_WARNING, "\"pragma once\" found outside a file. Ignored");
      return true;
   }
   else
   {
      if (_parsedFiles.end() == _parsedFiles.find(_tllFN.top()))
      {
         _parsedFiles.insert(_tllFN.top());
         return false;
      }
      else
         return true;
   }
}

void parsercmd::TellPreProc::reset()
{
   _variables.clear();
   _parsedFiles.clear();
   _preDef = "";
   _lastError = false;

   while (1 < _ppState.size())    _ppState.pop();
   assert(ppINACTIVE == _ppState.top());
   while (1 < _ifdefDepth.size()) _ifdefDepth.pop();
   assert(0 == _ifdefDepth.top());
   // std::string       _tllFN; <- this one can't be cleared. pragma once might be following
}

void parsercmd::TellPreProc::setRepeatLC(std::string repeatLC, const TpdYYLtype& loc)
{
   if (repeatLC == std::string("default"))
      _repeatLC = false;
   else if (repeatLC == std::string("buggy"))
      _repeatLC = true;
   else
   {
      std::ostringstream ost;
      ost << "\"#pragma repeatloop\" - wrong argument. \"default\" or \"buggy\" expected";
      ppError(ost.str(), loc);
   }
}

//=============================================================================
real parsercmd::cmdVIRTUAL::getOpValue(telldata::operandSTACK& OPs)
{
   real value = 0;
   telldata::TellVar *op = OPs.top();OPs.pop();
   if (op->get_type() == telldata::tn_real)
      value = static_cast<telldata::TtReal*>(op)->value();
   else if (op->get_type() == telldata::tn_int)
      value = static_cast<telldata::TtInt*>(op)->value();
   else if (op->get_type() == telldata::tn_uint)
      value = static_cast<telldata::TtUInt*>(op)->value();
   else
      assert(false);
   delete op;
   return value;
}

real parsercmd::cmdVIRTUAL::getOpValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   real value = 0;
   telldata::TellVar *op;
   if (front) {op = OPs.front();OPs.pop_front();}
   else       {op = OPs.back();OPs.pop_back();}
   if (op->get_type() == telldata::tn_real)
      value = static_cast<telldata::TtReal*>(op)->value();
   else if (op->get_type() == telldata::tn_int)
      value = static_cast<telldata::TtInt*>(op)->value();
   else if (op->get_type() == telldata::tn_uint)
      value = static_cast<telldata::TtUInt*>(op)->value();
   else
      assert(false);
   delete op;
   return value;
}

word parsercmd::cmdVIRTUAL::getWordValue(telldata::operandSTACK& OPs)
//@TODO - make sure that _opstackerr is utilised by the callers!
{
   word value = 0;
   telldata::TellVar *op = OPs.top();OPs.pop();

   if (op->get_type() == telldata::tn_int)
   {
      telldata::TtInt  *opi = static_cast<telldata::TtInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_WORD_VALUE))
         _opstackerr = true;
      else value = word(opi->value());
   }
   else if (op->get_type() == telldata::tn_uint)
   {
      telldata::TtUInt  *opi = static_cast<telldata::TtUInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_WORD_VALUE))
         _opstackerr = true;
      else value = word(opi->value());
   }
   else
      assert(false);

   delete op;
   return value;
}

word parsercmd::cmdVIRTUAL::getWordValue(telldata::UNDOPerandQUEUE& OPs, bool front)
//@TODO - make sure that _opstackerr is utilised by the callers!
{
   word value = 0;
   telldata::TellVar  *op;
   if (front) {op = OPs.front();OPs.pop_front();}
   else       {op = OPs.back() ;OPs.pop_back() ;}
   if (op->get_type() == telldata::tn_int)
   {
      telldata::TtInt  *opi = static_cast<telldata::TtInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_WORD_VALUE))
         _opstackerr = true;
      else value = word(opi->value());
   }
   else if (op->get_type() == telldata::tn_uint)
   {
      telldata::TtUInt  *opi = static_cast<telldata::TtUInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_WORD_VALUE))
         _opstackerr = true;
      else value = word(opi->value());
   }
   else
      assert(false);

   delete op;
   return value;
}

dword parsercmd::cmdVIRTUAL::getIndexValue(telldata::operandSTACK& OPs)
{
   dword value = 0;
   telldata::TellVar *op = OPs.top();OPs.pop();
   if (op->get_type() == telldata::tn_real)
   {
      real realvalue = static_cast<telldata::TtReal*>(op)->value();
      if ((realvalue < 0) || ((realvalue - int(realvalue)) != 0.0 ) )
         _opstackerr = true;
      else
         value = (dword) rint(realvalue);
   }
   else if (op->get_type() == telldata::tn_int)
   {
      int4b intvalue = (int4b) rint(static_cast<telldata::TtInt*>(op)->value());
      if (intvalue < 0)
         _opstackerr = true;
      else
         value = intvalue;
   }
   else if (op->get_type() == telldata::tn_uint)
   {
     value = static_cast<telldata::TtUInt*>(op)->value();
   }
   else
      assert(false);
   delete op;
   return value;
}

byte parsercmd::cmdVIRTUAL::getByteValue(telldata::operandSTACK& OPs)
//@TODO - make sure that _opstackerr is utilised by the callers!
{
   byte value = 0;
   telldata::TellVar *op = OPs.top();OPs.pop();

   if (op->get_type() == telldata::tn_int)
   {
      telldata::TtInt  *opi = static_cast<telldata::TtInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_BYTE_VALUE))
         _opstackerr = true;
      else value = byte(opi->value());
   }
   else if (op->get_type() == telldata::tn_uint)
   {
      telldata::TtUInt  *opi = static_cast<telldata::TtUInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_BYTE_VALUE))
         _opstackerr = true;
      else value = byte(opi->value());
   }
   else
      assert(false);

   delete op;
   return value;
}

byte parsercmd::cmdVIRTUAL::getByteValue(telldata::UNDOPerandQUEUE& OPs, bool front)
//@TODO - make sure that _opstackerr is utilised by the callers!
{
   byte value = 0;
   telldata::TellVar  *op;
   if (front) {op = OPs.front();OPs.pop_front();}
   else       {op = OPs.back() ;OPs.pop_back() ;}
   if (op->get_type() == telldata::tn_int)
   {
      telldata::TtInt  *opi = static_cast<telldata::TtInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_BYTE_VALUE))
         _opstackerr = true;
      else value = byte(opi->value());
   }
   else if (op->get_type() == telldata::tn_uint)
   {
      telldata::TtUInt  *opi = static_cast<telldata::TtUInt*>(op);
      if ((opi->value() < 0 ) || (opi->value() > MAX_BYTE_VALUE))
         _opstackerr = true;
      else value = byte(opi->value());
   }
   else
      assert(false);

   delete op;
   return value;
}

std::string parsercmd::cmdVIRTUAL::getStringValue(telldata::operandSTACK& OPs)
{
   telldata::TtString  *op = static_cast<telldata::TtString*>(OPs.top());OPs.pop();
   std::string value = op->value();
   delete op;
   return value;
}

std::string parsercmd::cmdVIRTUAL::getStringValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   telldata::TtString  *op;
   if (front) {op = static_cast<telldata::TtString*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::TtString*>(OPs.back());OPs.pop_back();}
   std::string value = op->value();
   delete op;
   return value;
}

bool parsercmd::cmdVIRTUAL::getBoolValue(telldata::operandSTACK& OPs)
{
   telldata::TtBool  *op = static_cast<telldata::TtBool*>(OPs.top());OPs.pop();
   bool value = op->value();
   delete op;
   return value;
}

bool parsercmd::cmdVIRTUAL::getBoolValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   telldata::TtBool  *op;
   if (front) {op = static_cast<telldata::TtBool*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::TtBool*>(OPs.back());OPs.pop_back();}
   bool value = op->value();
   delete op;
   return value;
}
//=============================================================================
int parsercmd::cmdPLUS::execute()
{
   TELL_DEBUG(cmdPLUS);
   if (telldata::tn_real == _retype)
   {
      real value2 = getOpValue();
      real value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtReal(value1 + value2));
   }
   else if (telldata::tn_int == _retype)
   {
      int value2 = getOpValue();
      int value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtInt(value1 + value2));
   }
   else if (telldata::tn_uint == _retype)
   {
      dword value2 = getOpValue();
      dword value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtUInt(value1 + value2));
   }
   else assert(false);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdCONCATENATE::execute()
{
   TELL_DEBUG(cmdCONCATENATE);
   std::string op2 = getStringValue(OPstack);
   std::string op1 = getStringValue(OPstack);
   OPstack.push(DEBUG_NEW telldata::TtString(op1 + op2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdMINUS::execute()
{
   TELL_DEBUG(cmdMINUS);
   if (telldata::tn_real == _retype)
   {
      real value2 = getOpValue();
      real value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtReal(value1 - value2));
   }
   else if (telldata::tn_int == _retype)
   {
      int value2 = getOpValue();
      int value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtInt(value1 - value2));
   }
//   else if (telldata::tn_uint == _retype)
//   {
//      unsigned value2 = getOpValue();
//      unsigned value1 = getOpValue();
//      OPstack.push(DEBUG_NEW telldata::TtUInt(value1 - value2));
//   }
   else assert(false);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT::execute()
{
   TELL_DEBUG(cmdSHIFTPNT);
   real shift;
   telldata::TtPnt *p;
   if (_swapOperands)
   {
      p = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
      shift = getOpValue();
   }
   else
   {
      shift = getOpValue();
      p = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   }
   telldata::TtPnt* r = DEBUG_NEW telldata::TtPnt(p->x()+_sign*shift,p->y()+_sign*shift);
   delete p;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT2::execute()
{
   TELL_DEBUG(cmdSHIFTPNT2);
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt *p  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt *r  = DEBUG_NEW telldata::TtPnt(p->x()+_sign*p1->x(),p->y()+_sign*p1->y());
   delete p; delete p1;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT3::execute()
{
   TELL_DEBUG(cmdSHIFTPNT3);
   real shift = getOpValue();
   telldata::TtPnt *p = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt *r = DEBUG_NEW telldata::TtPnt(p->x()+_signX*shift,p->y()+_signY*shift);
   delete p;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT4::execute()
{
   TELL_DEBUG(cmdSHIFTPNT4);
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt *p  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt* r = DEBUG_NEW telldata::TtPnt(p->x()+_signX*p1->x(),p->y()+_signY*p1->y());
   delete p; delete p1;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX::execute()
{
   TELL_DEBUG(cmdSHIFTBOX);
   telldata::TtPnt *p;
   telldata::TtWnd *w;
   if (_swapOperands)
   {
      w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
      p = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   }
   else
   {
      p = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
      w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   }
   telldata::TtWnd* r = DEBUG_NEW telldata::TtWnd(w->p1().x() + _sign*p->x(),w->p1().y() + _sign*p->y(),
                        w->p2().x() + _sign*p->x(),w->p2().y() + _sign*p->y());
   OPstack.push(r);
   delete p; delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX3::execute()
{
   TELL_DEBUG(cmdSHIFTBOX3);
   real shift = getOpValue();
   telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   bool swapx, swapy;
   w->normalize(swapx, swapy);
   telldata::TtWnd* r;
   if  (1 == _signX)
      if (1 == _signY)
         r = DEBUG_NEW telldata::TtWnd(w->p1().x()          , w->p1().y()         ,
                                       w->p2().x() + shift  , w->p2().y() + shift  );
      else
         r = DEBUG_NEW telldata::TtWnd(w->p1().x()          , w->p1().y() - shift ,
                                       w->p2().x() + shift  , w->p2().y()          );
   else
      if (1 == _signY)
         r = DEBUG_NEW telldata::TtWnd(w->p1().x() - shift  , w->p1().y()          ,
                                       w->p2().x()          , w->p2().y() + shift   );
      else
         r = DEBUG_NEW telldata::TtWnd(w->p1().x() - shift  , w->p1().y() - shift  ,
                                       w->p2().x()          , w->p2().y()           );
   r->denormalize(swapx, swapy);
   OPstack.push(r);
   delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX4::execute()
{
   TELL_DEBUG(cmdSHIFTBOX4);
   telldata::TtPnt *p = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   bool swapx, swapy;
   w->normalize(swapx, swapy);
   telldata::TtWnd* r;
   if  (1 == _signX)
      if (1 == _signY)
         r = DEBUG_NEW telldata::TtWnd(w->p1().x()          , w->p1().y()         ,
                                       w->p2().x() + p->x() , w->p2().y() + p->y() );
      else
         r = DEBUG_NEW telldata::TtWnd(w->p1().x()          , w->p1().y() - p->y(),
                                       w->p2().x() + p->x() , w->p2().y()          );
   else
      if (1 == _signY)
         r = DEBUG_NEW telldata::TtWnd(w->p1().x() - p->x() , w->p1().y()          ,
                                       w->p2().x()          , w->p2().y() + p->y()  );
      else
         r = DEBUG_NEW telldata::TtWnd(w->p1().x() - p->x() , w->p1().y() - p->y() ,
                                       w->p2().x()          , w->p2().y()           );
   r->denormalize(swapx, swapy);
   OPstack.push(r);
   delete p; delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBLOWBOX::execute()
{
   TELL_DEBUG(cmdBLOWBOX);
   real shift;
   telldata::TtWnd *w;
   if (_swapOperands)
   {
      w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
      shift = getOpValue();
   }
   else
   {
      shift = getOpValue();
      w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   }
   bool swapx, swapy;
   w->normalize(swapx, swapy);
   telldata::TtWnd* r;
   if  (1 == _sign)
      r = DEBUG_NEW telldata::TtWnd(w->p1().x() - shift  , w->p1().y() - shift ,
                                    w->p2().x() + shift  , w->p2().y() + shift  );
   else
      r = DEBUG_NEW telldata::TtWnd(w->p1().x() + shift  , w->p1().y() + shift  ,
                                    w->p2().x() - shift  , w->p2().y() - shift   );
   r->denormalize(swapx, swapy);
   OPstack.push(r);
   delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdMULTIPLY::execute()
{
   TELL_DEBUG(cmdMULTIPLY);
   if (telldata::tn_real == _retype)
   {
      real value2 = getOpValue();
      real value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtReal(value1 * value2));
   }
   else
   {
      real value2 = getOpValue();
      real value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtInt(value1 * value2));
   }
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdDIVISION::execute()
{
   TELL_DEBUG(cmdDIVISION);
   if (telldata::tn_real == _retype)
   {
      real value2 = getOpValue();
      real value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtReal(value1 / value2));
   }
   else
   {
      int  value2 = getOpValue();
      int  value1 = getOpValue();
      OPstack.push(DEBUG_NEW telldata::TtInt(value1 / value2));
   }
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSCALEPNT::execute()
{
   TELL_DEBUG(cmdSCALEPNT);
   real scaleFactor;
   telldata::TtPnt *p;
   if (_swapOperands)
   {
      p  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
      scaleFactor = getOpValue();
   }
   else
   {
      scaleFactor = getOpValue();
      p  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   }
   telldata::TtPnt* r;
   if (_up)
      r = DEBUG_NEW telldata::TtPnt(p->x() * scaleFactor,p->y() * scaleFactor);
   else
      r = DEBUG_NEW telldata::TtPnt(p->x() / scaleFactor, p->y() / scaleFactor);
   OPstack.push(r);
   delete p;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSCALEBOX::execute()
{
   TELL_DEBUG(cmdSCALEPNT);
   real scaleFactor;
   telldata::TtWnd* w;
   if (_swapOperands)
   {
      w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
      scaleFactor = getOpValue();
   }
   else
   {
      scaleFactor = getOpValue();
      w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   }

   telldata::TtWnd* r;
   if (_up)
      r = DEBUG_NEW telldata::TtWnd(w->p1().x() * scaleFactor , w->p1().y() * scaleFactor ,
                                    w->p2().x() * scaleFactor , w->p2().y() * scaleFactor  );
   else
      r = DEBUG_NEW telldata::TtWnd(w->p1().x() / scaleFactor , w->p1().y() / scaleFactor ,
                                    w->p2().x() / scaleFactor , w->p2().y() / scaleFactor  );
   OPstack.push(r);
   delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLT::execute()
{
   TELL_DEBUG(cmdLT);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::TtBool(value1 < value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLET::execute()
{
   TELL_DEBUG(cmdLET);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::TtBool(value1 <= value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdGT::execute()
{
   TELL_DEBUG(cmdGT);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::TtBool(value1 > value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdGET::execute()
{
   TELL_DEBUG(cmdGET);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::TtBool(value1 >= value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdEQ::execute()
{
   TELL_DEBUG(cmdEQ);
   if (NUMBER_TYPE(OPstack.top()->get_type()))
      OPstack.push(DEBUG_NEW telldata::TtBool(getOpValue() == getOpValue()));
   else if (telldata::tn_bool == OPstack.top()->get_type())
      OPstack.push(DEBUG_NEW telldata::TtBool(getBoolValue() == getBoolValue()));
//   else if (tn_
// box & poly equal
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdNE::execute()
{
   TELL_DEBUG(cmdNE);
   if (NUMBER_TYPE(OPstack.top()->get_type()))
      OPstack.push(DEBUG_NEW telldata::TtBool(getOpValue() != getOpValue()));
   else if (telldata::tn_bool == OPstack.top()->get_type())
      OPstack.push(DEBUG_NEW telldata::TtBool(getBoolValue() != getBoolValue()));
//   else if (tn_
// box & poly equal
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdAND::execute()
{
   TELL_DEBUG(cmdAND);
   telldata::TtBool *op = static_cast<telldata::TtBool*>(OPstack.top());OPstack.pop();
   static_cast<telldata::TtBool*>(OPstack.top())->AND(op->value());
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWAND::execute()
{
   TELL_DEBUG(cmdBWAND);
   telldata::TellVar *op1 = OPstack.top();OPstack.pop();
   telldata::TellVar *op2 = OPstack.top();OPstack.pop();

   if (telldata::tn_int == _type)
   {
      telldata::TtInt  *opi1 = static_cast<telldata::TtInt*>(op1);
      telldata::TtInt  *opi2 = static_cast<telldata::TtInt*>(op2);

      OPstack.push(DEBUG_NEW telldata::TtInt(opi1->value() & opi2->value()));
   }
   else if (telldata::tn_uint == _type)
   {
      telldata::TtUInt  *opi1 = static_cast<telldata::TtUInt*>(op1);
      telldata::TtUInt  *opi2 = static_cast<telldata::TtUInt*>(op2);

      OPstack.push(DEBUG_NEW telldata::TtUInt(opi1->value() & opi2->value()));
   }
   delete(op1);
   delete(op2);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdOR::execute()
{
   TELL_DEBUG(cmdOR);
   telldata::TtBool *op = static_cast<telldata::TtBool*>(OPstack.top());OPstack.pop();
   static_cast<telldata::TtBool*>(OPstack.top())->OR(op->value());
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWOR::execute()
{
   TELL_DEBUG(cmdBWOR);
   telldata::TellVar *op1 = OPstack.top();OPstack.pop();
   telldata::TellVar *op2 = OPstack.top();OPstack.pop();

   if (telldata::tn_int == _type)
   {
      telldata::TtInt  *opi1 = static_cast<telldata::TtInt*>(op1);
      telldata::TtInt  *opi2 = static_cast<telldata::TtInt*>(op2);

      OPstack.push(DEBUG_NEW telldata::TtInt(opi1->value() | opi2->value()));
   }
   else if (telldata::tn_uint == _type)
   {
      telldata::TtUInt  *opi1 = static_cast<telldata::TtUInt*>(op1);
      telldata::TtUInt  *opi2 = static_cast<telldata::TtUInt*>(op2);

      OPstack.push(DEBUG_NEW telldata::TtUInt(opi1->value() | opi2->value()));
   }
   delete(op1);
   delete(op2);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWXOR::execute()
{
   TELL_DEBUG(cmdBWXOR);

   telldata::TellVar *op1 = OPstack.top();OPstack.pop();
   telldata::TellVar *op2 = OPstack.top();OPstack.pop();

   if (telldata::tn_int == _type)
   {
      telldata::TtInt  *opi1 = static_cast<telldata::TtInt*>(op1);
      telldata::TtInt  *opi2 = static_cast<telldata::TtInt*>(op2);

      OPstack.push(DEBUG_NEW telldata::TtInt(opi1->value() ^ opi2->value()));
   }
   else if (telldata::tn_uint == _type)
   {
      telldata::TtUInt  *opi1 = static_cast<telldata::TtUInt*>(op1);
      telldata::TtUInt  *opi2 = static_cast<telldata::TtUInt*>(op2);

      OPstack.push(DEBUG_NEW telldata::TtUInt(opi1->value() ^ opi2->value()));
   }
   delete(op1);
   delete(op2);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWSHIFT::execute()
{
   TELL_DEBUG(cmdBWSHIFT);

   dword op1 = getIndexValue();
   telldata::TellVar *op2 = OPstack.top();OPstack.pop();

   if (telldata::tn_int == _type)
   {
      telldata::TtInt  *opi2 = static_cast<telldata::TtInt*>(op2);
      if (_sright)
         OPstack.push(DEBUG_NEW telldata::TtInt(opi2->value() >> op1));
      else
         OPstack.push(DEBUG_NEW telldata::TtInt(opi2->value() << op1 ));
   }
   else if (telldata::tn_uint == _type)
   {
      telldata::TtUInt  *opi2 = static_cast<telldata::TtUInt*>(op2);
      if (_sright)
         OPstack.push(DEBUG_NEW telldata::TtUInt(opi2->value() >> op1));
      else
         OPstack.push(DEBUG_NEW telldata::TtUInt(opi2->value() << op1 ));
   }
   delete(op2);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSTRCMP::execute()
{
   bool equal = !_eq;
   std::string op1 = getStringValue();
   std::string op2 = getStringValue();
   equal ^= (op1 == op2);
   OPstack.push(DEBUG_NEW telldata::TtBool(equal));
   return EXEC_NEXT;
}
//=============================================================================
int parsercmd::cmdNOT::execute()
{
   TELL_DEBUG(cmdNOT);
   static_cast<telldata::TtBool*>(OPstack.top())->NOT();
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWNOT::execute()
{
   TELL_DEBUG(cmdNOT);
   if (telldata::tn_int == _type)
      static_cast<telldata::TtInt*>(OPstack.top())->NOT();
   else if (telldata::tn_uint == _type)
      static_cast<telldata::TtUInt*>(OPstack.top())->NOT();
   else
      assert(false);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdUMINUS::execute()
{
   TELL_DEBUG(cmdUMINUS);
   if      (telldata::tn_uint == _type)
   {
      telldata::TtUInt* varuint = static_cast<telldata::TtUInt*>(OPstack.top());OPstack.pop();
      telldata::TtInt*  varint = DEBUG_NEW telldata::TtInt(-varuint->value());
      OPstack.push(varint);
      delete varuint;
   }
   else if ( telldata::tn_int  == _type ) static_cast<telldata::TtInt*>(OPstack.top())->uminus();
   else if ( telldata::tn_real == _type ) static_cast<telldata::TtReal*>(OPstack.top())->uminus();
   else assert(false);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSTACKRST::execute()
{
   TELL_DEBUG(cmdSTACKRST);
   while (!OPstack.empty()) {
      delete OPstack.top(); OPstack.pop();
   }
   return EXEC_NEXT;
}

//=============================================================================
parsercmd::cmdLISTSIZE::cmdLISTSIZE(telldata::TellVar* var):
   _var(var)
{
   telldata::typeID ID = _var->get_type() & ~telldata::tn_listmask;
   switch (ID)
   {
      case telldata::tn_real    : _initVar = DEBUG_NEW telldata::TtReal()   ; break;
      case telldata::tn_uint    : _initVar = DEBUG_NEW telldata::TtUInt()   ; break;
      case telldata::tn_int     : _initVar = DEBUG_NEW telldata::TtInt()    ; break;
      case telldata::tn_bool    : _initVar = DEBUG_NEW telldata::TtBool()   ; break;
      case telldata::tn_pnt     : _initVar = DEBUG_NEW telldata::TtPnt()    ; break;
      case telldata::tn_box     : _initVar = DEBUG_NEW telldata::TtWnd()    ; break;
      case telldata::tn_bnd     : _initVar = DEBUG_NEW telldata::TtBnd()    ; break;
      case telldata::tn_laymap  : _initVar = DEBUG_NEW telldata::TtLMap()   ; break;
      case telldata::tn_hshstr  : _initVar = DEBUG_NEW telldata::TtHshStr() ; break;
      case telldata::tn_layer   : _initVar = DEBUG_NEW telldata::TtLayer()  ; break;
      case telldata::tn_string  : _initVar = DEBUG_NEW telldata::TtString() ; break;
      case telldata::tn_layout  : _initVar = DEBUG_NEW telldata::TtLayout() ; break;
      case telldata::tn_auxilary: _initVar = DEBUG_NEW telldata::TtAuxdata(); break;
      default:
      {
         const telldata::TType* utype = CMDBlock->getTypeByID(ID);
         if (NULL == utype)
            assert(false);// unknown base array type ?!
         else if  (utype->isComposite())
            _initVar = DEBUG_NEW telldata::TtUserStruct(static_cast<const telldata::TCompType*>(utype));
         else
            _initVar = DEBUG_NEW telldata::TtCallBack(ID);
         break;
      }
   }
}

int parsercmd::cmdLISTSIZE::execute()
{
   TELL_DEBUG(cmdLISTSIZE);
   dword idx = getIndexValue();
   static_cast<telldata::TtList*>(_var)->resize(idx, _initVar);
   return EXEC_NEXT;
}

parsercmd::cmdLISTSIZE::~cmdLISTSIZE()
{
   delete _initVar;
}
//=============================================================================
int parsercmd::cmdASSIGN::execute()
{
   TELL_DEBUG(cmdASSIGN);
   telldata::TellVar *op = OPstack.top();OPstack.pop();
   switch (_indexed)
   {
      case 0:
      {
         _var->assign(op); OPstack.push(_var->selfcopy());
         delete op;
         return EXEC_NEXT;
      }
      case 1:
      {
         dword idx = getIndexValue();
         if (telldata::tn_string == _var->get_type())
         {
            assert(telldata::tn_string == op->get_type());
            telldata::TtString* wsop = static_cast<telldata::TtString*>(op);
            telldata::TtString* tstr = static_cast<telldata::TtString*>(_var);
            if ((tstr->value().length() > idx) && (!_opstackerr) && (1 == wsop->value().length()))
            {
               tstr->part_assign(idx,wsop); OPstack.push(tstr->selfcopy());
               delete op;
               return EXEC_NEXT;
            }
            else
            {
               std::stringstream info;
               info << "Runtime error. ";
               if (1 != wsop->value().length())
               {
                  info << "Unmatched list range";
               }
               else
               {
                  unsigned maxSize = tstr->value().length();
                  info << "Invalid index in assign lvalue. Requested: "<< idx << "; Valid: ";
                  if (0 == maxSize)
                     info << " none (empty list)";
                  else if (1 == maxSize)
                     info << "[0]";
                  else
                     info << "[0 - " << maxSize - 1 << "]";
               }
               tellerror(info.str());
            }
         }
         else
         {
            telldata::TellVar* indexVar = static_cast<telldata::TtList*>(_var)->index_var(idx);
            if ((NULL != indexVar) && (!_opstackerr))
            {
               indexVar->assign(op); OPstack.push(indexVar->selfcopy());
               delete op;
               return EXEC_NEXT;
            }
            else
            {
               std::stringstream info;
               unsigned maxSize = static_cast<telldata::TtList*>(_var)->size();
               info << "Runtime error. Invalid index in assign lvalue. Requested: "<< idx << "; Valid: ";
               if (0 == maxSize)
                  info << " none (empty list)";
               else if (1 == maxSize)
                  info << "[0]";
               else
                  info << "[0 - " << maxSize - 1 << "]";
               tellerror(info.str());
            }
         }
         break;
      }
      case 2:
      {
         dword idx2 = getIndexValue();
         dword idx1 = getIndexValue();
         if (telldata::tn_string == _var->get_type())
         {
            assert(telldata::tn_string == op->get_type());
            telldata::TtString* wsop = static_cast<telldata::TtString*>(op);
            telldata::TtString* tstr = static_cast<telldata::TtString*>(_var);

            if (_opstackerr)
               tellerror("Runtime error. Invalid Index");
            if (idx1 >= idx2)
               tellerror("Runtime error.Second index is expected to be bigger than the first one");
            else if ((wsop->value().length() - 1) != (idx2 - idx1))
               tellerror("Runtime error. Unmatched list range");
            else if (!(tstr->value().length() > idx2))
               tellerror("Runtime error.Invalid Index");
            else
            {
               tstr->part_assign(idx1,wsop); OPstack.push(tstr->selfcopy());
               delete op;
               return EXEC_NEXT;
            }
         }
         else
         {
            telldata::TtList* theList = static_cast<telldata::TtList*>(_var);
            telldata::TtList* newValue = static_cast<telldata::TtList*>(op);

            if (_opstackerr)
               tellerror("Runtime error. Invalid Index");
            if (idx1 >= idx2)
               tellerror("Runtime error.Second index is expected to be bigger than the first one");
            else if ((newValue->size() - 1) != (idx2 - idx1))
               tellerror("Runtime error. Unmatched list range");
            else if (!theList->part_assign(idx1, idx2, newValue))
               tellerror("Runtime error.Invalid Index");
            else
            {
               OPstack.push(theList->index_range_var(idx1,idx2));
               delete op;
               return EXEC_NEXT;
            }
         }
         break;
      }
      default: assert(false); break;
   }
   delete op;
   return EXEC_ABORT;
}

//=============================================================================
int parsercmd::cmdLISTADD::execute()
{
   TELL_DEBUG(cmdLISTADD);

   if (telldata::tn_string == _arg->get_type())
      return stringExec(static_cast<telldata::TtString*>(_arg));
   else
   {
      assert(TLISALIST(_arg->get_type()));
      return listExec(static_cast<telldata::TtList*>(_arg));
   }
}

int parsercmd::cmdLISTADD::listExec(telldata::TtList* listarg)
{
   telldata::TellVar *op = OPstack.top();OPstack.pop();
   telldata::typeID typeis = listarg->get_type();
   assert(TLISALIST(typeis));
   typeis = typeis & ~telldata::tn_listmask;
   if ((TLCOMPOSIT_TYPE(typeis)) && (NULL == CMDBlock->getTypeByID(typeis)))
      tellerror("Bad or unsupported type in assign statement");
   else
   {
      dword idx = getIndex(listarg->size());
      if ((!_opstackerr) && (_empty_list) && (0 == idx))
      {
         listarg->insert(op);
      }
      else if ((!_opstackerr) && (listarg->validIndex(idx)))
      {
         if (!_prefix) idx++;
         listarg->insert(op, idx);
      }
      else
      {
         std::stringstream info;
         unsigned maxSize = listarg->size();
         info << "Runtime error. Invalid index in list insert. Requested: "<< idx << "; Valid: ";
         if (0 == maxSize)
            info << " none (empty list)";
         else if (1 == maxSize)
            info << "[0]";
         else
            info << "[0 - " << maxSize - 1 << "]";
         tellerror(info.str());
         return EXEC_ABORT;
      }
   }
   delete op;
   OPstack.push(listarg->selfcopy());
   return EXEC_NEXT;
}

int parsercmd::cmdLISTADD::stringExec(telldata::TtString* strtarget)
{
   telldata::TellVar *op = OPstack.top();OPstack.pop();
   assert(telldata::tn_string == op->get_type());
   std::string istr = static_cast<telldata::TtString*>(op)->value();
   std::string wstr = strtarget->value();
   dword idx = getIndex(wstr.length());
   //
   if (!_prefix) idx++;
   if (wstr.length() == idx)
      wstr.append(istr);
   else if (wstr.length() > idx)
      wstr.insert(idx,istr);
   else
   {
      std::stringstream info;
      unsigned maxSize = wstr.length();
      info << "Runtime error. Invalid index in list insert. Requested: "<< idx << "; Valid: ";
      if (0 == maxSize)
         info << " none (empty list)";
      else if (1 == maxSize)
         info << "[0]";
      else
         info << "[0 - " << maxSize - 1 << "]";
      tellerror(info.str());
      return EXEC_ABORT;
   }
   (*strtarget) = telldata::TtString(wstr);
   return EXEC_NEXT;
}

dword parsercmd::cmdLISTADD::getIndex(dword lsize)
{

//   telldata::TtList* _listarg = static_cast<telldata::TtList*>(_arg);
//   _listarg->size()
   dword idx;
   _empty_list = (0 == lsize);
   // find the index
   if      (((!_index) && ( _prefix)) || _empty_list) // first in the list
      idx = 0;
   else if ((!_index) && (!_prefix)) // last in the list
   {
      idx = lsize;
      if (!_empty_list) idx--;
   }
   else                              // get the index from the operand stack
      idx = getIndexValue();
   //
   return idx;
}

//=============================================================================
int parsercmd::cmdLISTUNION::execute()
{
   TELL_DEBUG(cmdLISTUNION);

   assert(TLISALIST(_arg->get_type()));
   telldata::TtList* listarg = static_cast<telldata::TtList*>(_arg);
   telldata::TtList *op = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   telldata::typeID typeis = listarg->get_type() & ~telldata::tn_listmask;

   if ((TLCOMPOSIT_TYPE(typeis)) && (NULL == CMDBlock->getTypeByID(typeis)))
      tellerror("Bad or unsupported type in list union statement");
   else
   {
      dword idx = getIndex(listarg->size());
      if ((!_opstackerr) && (_empty_list) && (0 == idx))
      {
         listarg->lunion(op);
      }
      else if ((!_opstackerr) && (listarg->validIndex(idx)))
      {
         if (!_prefix) idx++;
         listarg->lunion(op,idx);
      }
      else
      {
         std::stringstream info;
         unsigned maxSize = listarg->size();
         info << "Runtime error. Invalid index in list union. Requested: "<< idx << "; Valid: ";
         if (0 == maxSize)
            info << " none (empty list)";
         else if (1 == maxSize)
            info << "[0]";
         else
            info << "[0 - " << maxSize - 1 << "]";
         tellerror(info.str());
         return EXEC_ABORT;
      }
   }
   delete op;
   OPstack.push(listarg->selfcopy());
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLISTSUB::execute()
{
   TELL_DEBUG(cmdLISTSUB);
   dword idx;
   // find the index
   if      ((!_index) && ( _prefix)) // first in the list
      idx = 0;
   else if ((!_index) && (!_prefix)) // last in the list
      idx = _listarg->size() - 1;
   else                              // get the index from the operand stack
      idx = getIndexValue();
   //
   if ((!_opstackerr) && (_listarg->validIndex(idx)))
   {
      OPstack.push(_listarg->erase(idx));
      return EXEC_NEXT;
   }
   else
   {
      std::stringstream info;
      unsigned maxSize = _listarg->size();
      info << "Runtime error. Invalid index in list reduce. Requested: "<< idx << "; Valid: ";
      if (0 == maxSize)
         info << " none (empty list)";
      else if (1 == maxSize)
         info << "[0]";
      else
         info << "[0 - " << maxSize - 1 << "]";
      tellerror(info.str());
      return EXEC_ABORT;
   }
}

//=============================================================================
int parsercmd::cmdLISTSLICE::execute()
{
   TELL_DEBUG(cmdLISTSLICE);
   dword idxB, idxE, size;
   bool idxerrors = false;
   // find the index
   if ( _prefix )
   {
      if (_index)
      {
         idxE = getIndexValue(); idxerrors |= _opstackerr;
      }
      else idxE = _listarg->size() - 1;
      size = getIndexValue(); idxerrors |= _opstackerr;

      if ((0 > (idxE - size + 1)) || (0 == size))
      {
         idxerrors = true;idxB = idxE;
      }
      else
      {
         idxB = idxE - size + 1;
      }
   }
   else
   {
      size = getIndexValue(); idxerrors |= _opstackerr;
      if (_index)
      {
         idxB  = getIndexValue(); idxerrors |= _opstackerr;
      }
      else idxB = 0;
      if (0 == size)
      {
         idxerrors = true;idxE = idxB;
      }
      else
      {
         idxE = idxB + size - 1;
      }
   }
   if ((!idxerrors) && _listarg->validIndex(idxB) && _listarg->validIndex(idxE))
   {
      OPstack.push(_listarg->erase(idxB, idxE));
      return EXEC_NEXT;
   }
   else
   {
      std::stringstream info;
      unsigned maxSize = _listarg->size();
      info << "Runtime error. Invalid index in list slice. Requested: ["<< idxB << " - " << idxE << "]; Valid: ";
      if (0 == maxSize)
         info << " none (empty list)";
      else if (1 == maxSize)
         info << "[0]";
      else
         info << "[0 - " << maxSize - 1 << "]";
      tellerror(info.str());
      return EXEC_ABORT;
   }
}

//=============================================================================
int parsercmd::cmdPUSH::execute()
{
   // The temptation here is to put the constants in the operand stack directly,
   // i.e. without self-copy. It is wrong though - for many reasons - for example
   // for conditional block "while (count > 0)". It should be executed many
   // times but the variable will exists only the first time, because it will
   // be cleaned-up from the operand stack after the first execution
   TELL_DEBUG(cmdPUSH);
   switch (_indexed)
   {
      case 0:
      {
         OPstack.push(_var->selfcopy());
         return EXEC_NEXT;
      }
      case 1:
      {
         dword idx = getIndexValue();
         if (telldata::tn_string == _var->get_type())
         {
            std::string wstr = static_cast<telldata::TtString*>(_var)->value();
            if (!_opstackerr && (wstr.length() > idx))
            {
               char wch[2];
               wch[0] = wstr[idx];
               wch[1] = 0x0;
               OPstack.push(DEBUG_NEW telldata::TtString (wch));
               return EXEC_NEXT;
            }
            else
            {
               std::stringstream info;
               unsigned maxSize = wstr.length();
               info << "Runtime error. Invalid index. Requested: "<< idx << "; Valid: ";
               if (0 == maxSize)
                  info << " none (empty list)";
               else if (1 == maxSize)
                  info << "[0]";
               else
                  info << "[0 - " << maxSize - 1 << "]";
               tellerror(info.str());
               return EXEC_ABORT;
            }
         }
         else
         {
            telldata::TellVar *listcomp = static_cast<telldata::TtList*>(_var)->index_var(idx);
            if ((NULL != listcomp) && (!_opstackerr))
            {
               OPstack.push(listcomp->selfcopy());
               return EXEC_NEXT;
            }
            else
            {
               std::stringstream info;
               unsigned maxSize = static_cast<telldata::TtList*>(_var)->size();
               info << "Runtime error. Invalid index. Requested: "<< idx << "; Valid: ";
               if (0 == maxSize)
                  info << " none (empty list)";
               else if (1 == maxSize)
                  info << "[0]";
               else
                  info << "[0 - " << maxSize - 1 << "]";
               tellerror(info.str());
               return EXEC_ABORT;
            }
         }
      }
      case 2:
      {
         dword idx2 = getIndexValue();
         dword idx1 = getIndexValue();
         if (idx1 < idx2)
         {
            if (telldata::tn_string == _var->get_type())
            {
               std::string wstr = static_cast<telldata::TtString*>(_var)->value();
               if (!_opstackerr && (wstr.length() > idx2))
               {
                  std::string nstr = wstr.substr(idx1, idx2-idx1+1);
                  OPstack.push(DEBUG_NEW telldata::TtString (nstr));
                  return EXEC_NEXT;
               }
               else
               {
                  tellerror("Runtime error. Index out of range");
               }
            }
            else
            {
               telldata::TtList* nlist = static_cast<telldata::TtList*>(_var)->index_range_var(idx1,idx2);
               if ((NULL != nlist) && (!_opstackerr))
               {
                  OPstack.push(nlist);
                  return EXEC_NEXT;
               }
               else
                  tellerror("Runtime error.Index out of range");
            }
         }
         else
            tellerror("Runtime error.Second index is expected to be bigger than the first one");
         break;
      }
      default: assert(false); break;
   }
   return EXEC_ABORT;
}

//=============================================================================
telldata::TellVar* parsercmd::cmdSTRUCT::getList()
{
   telldata::typeID comptype = (*_arg)() & ~telldata::tn_listmask;
   telldata::TtList *pl = DEBUG_NEW telldata::TtList(comptype);
   unsigned llength = _arg->child().size();
   pl->reserve(llength);
   telldata::TellVar  *p;
   for (unsigned i = 0; i < llength; i++) {
      p = OPstack.top();OPstack.pop();
      pl->add(p); //Don't delete p; here! And don't get confused!
   }
   pl->reverse();
   return pl;
}

int parsercmd::cmdSTRUCT::execute()
{
   TELL_DEBUG(cmdSTRUCT);
   if (NULL == _arg)
   {
      tellerror("Structure arguments not evaluated properly. Internal parser error");
      return EXEC_RETURN;
   }
   telldata::TellVar *ustrct;
   if (TLISALIST( (*_arg)() )) ustrct = getList();
   else
   {
      switch( (*_arg)() )
      {
         case telldata::tn_pnt   : ustrct = DEBUG_NEW telldata::TtPnt(OPstack);break;
         case telldata::tn_box   : ustrct = DEBUG_NEW telldata::TtWnd(OPstack);break;
         case telldata::tn_bnd   : ustrct = DEBUG_NEW telldata::TtBnd(OPstack);break;
         case telldata::tn_laymap: ustrct = DEBUG_NEW telldata::TtLMap(OPstack);break;
         case telldata::tn_hshstr: ustrct = DEBUG_NEW telldata::TtHshStr(OPstack);break;
         case telldata::tn_layer : ustrct = DEBUG_NEW telldata::TtLayer(OPstack);break;
         default:
         {
            const telldata::TType* atype = CMDBlock->getTypeByID( (*_arg)() );
            if (atype->isComposite())
               ustrct = DEBUG_NEW telldata::TtUserStruct(static_cast<const telldata::TCompType*>(atype), OPstack);
            else
               assert(false); // parser shall never get to this point or I'm missing something ...
            break;
         }
      }
   }
   OPstack.push(ustrct);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdFUNCREF::execute()
{
   TELL_DEBUG(cmdFUNCREF);
   if ((NULL == _funcBody) || (telldata::tn_anyfref == _ID))
   {
      tellerror("Callback statement not evaluated properly. Internal parser error");
      return EXEC_RETURN;
   }
   telldata::TellVar *ustrct = DEBUG_NEW telldata::TtCallBack( _ID, _funcBody);
   OPstack.push(ustrct);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdNUMFPARAMS::execute()
{
   TELL_DEBUG(cmdNUMFPARAMS);
   OPstack.push(DEBUG_NEW telldata::TtInt(_numParams));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdFUNCCALL::execute()
{
   TELL_DEBUG(cmdFUNC);
   int fresult;
   if (_funcbody->ignoreOnRecovery() && !_funcbody->execOnRecovery())
   {
      std::string info = _funcname + " ignored";
      tell_log(console::MT_INFO, info);
      return EXEC_NEXT;
   }
   if (_funcbody->declaration())
   {
      std::string info = "Link error. Function " + _funcname + "() not defined";
      tell_log(console::MT_ERROR, info);
      return EXEC_ABORT;
   }
   LogFile.setFN(_funcname);
   try
   {
      if (!CMDBlock->checkDbSortState(_funcbody->dbSortStatus()))
      {
         cmdSTDFUNC* sortFunc = CMDBlock->getIntFuncBody("$sort_db");
         sortFunc->execute();
      }
      fresult = _funcbody->execute();
   }
   catch (EXPTN&) {return EXEC_ABORT;}
   _funcbody->reduce_undo_stack();
   return fresult;
}

//=============================================================================
bool parsercmd::cmdRETURN::checkRetype(telldata::ArgumentID* arg)
{
   if (NULL == arg) return (_retype == telldata::tn_void);

   if (TLUNKNOWN_TYPE((*arg)()))
   {
      const telldata::TType* vartype;
      if (TLISALIST(_retype))
      { // we have a list lval
          vartype = CMDBlock->getTypeByID(_retype & ~telldata::tn_listmask);
          if (NULL != vartype) arg->userStructListCheck(vartype, true);
          else arg->toList(true, _retype & ~telldata::tn_listmask);
      }
      else
      { // we have a struct only
         vartype = CMDBlock->getTypeByID(_retype);
         if (NULL != vartype) arg->userStructCheck(vartype, true);
      }
   }
   return ((_retype == (*arg)()) || (NUMBER_TYPE(_retype) && NUMBER_TYPE((*arg)())));
}
//=============================================================================
parsercmd::cmdBLOCK::cmdBLOCK()
{
   assert(!_blocks.empty());
   _nextLclTypeID = _blocks.front()->_nextLclTypeID;
   _typeLocal.clear();
   _typeAnoLo.clear();
   _varLocal.clear();
}

telldata::TellVar* parsercmd::cmdBLOCK::getID(const char* name, bool local) const
{
   TELL_DEBUG(***getID***);
   // Roll back the BlockSTACK until name is found. return NULL otherwise
   typedef BlockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = local ? ++(_blocks.begin()) : _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++) {
        if ((*cmd)->_varLocal.find(name) != (*cmd)->_varLocal.end())
            return (*cmd)->_varLocal[name];
   }
   return NULL;
}

void parsercmd::cmdBLOCK::addID(const char* name, telldata::TellVar* var)
{
   TELL_DEBUG(addID);
   _varLocal[name] = var;
}

void parsercmd::cmdBLOCK::addconstID(const char* name, telldata::TellVar* var, bool initialized)
{
   TELL_DEBUG(addID);
   _varLocal[name] = var;
   var->const_declaration();
   if (initialized) var->update_cstat();
}

void parsercmd::cmdBLOCK::addlocaltype(const char* ttypename, telldata::TType* ntype)
{
   assert(_typeLocal.end() == _typeLocal.find(ttypename));
   _nextLclTypeID = ntype->ID() + 1;
   _typeLocal[ttypename] = ntype;
}

void parsercmd::cmdBLOCK::addAnoLoType(telldata::TType* ntype)
{
   _nextLclTypeID = ntype->ID() + 1;
   _typeAnoLo.push_back(ntype);
}

telldata::TCompType* parsercmd::cmdBLOCK::secureCompType(char*& ttypename)
{
   if (_typeLocal.end() == _typeLocal.find(ttypename))
   {
      telldata::TCompType* ntype = DEBUG_NEW telldata::TCompType(_nextLclTypeID);
      return ntype;
   }
   else return NULL;
}

telldata::TCallBackType* parsercmd::cmdBLOCK::secureCallBackType(const char* ttypename)
{
   if (NULL == ttypename)
   { // anonymous type
      return DEBUG_NEW telldata::TCallBackType(_nextLclTypeID);
   }
   if (_typeLocal.end() == _typeLocal.find(ttypename))
   {
      telldata::TCallBackType* ntype = DEBUG_NEW telldata::TCallBackType(_nextLclTypeID);
      return ntype;
   }
   else return NULL;
}

const telldata::TType* parsercmd::cmdBLOCK::getTypeByName(char*& ttypename) const
{
   TELL_DEBUG(***gettypeID***);
   // Roll back the BlockSTACK until name is found. return NULL otherwise
   typedef BlockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++)
   {
        if ((*cmd)->_typeLocal.end() != (*cmd)->_typeLocal.find(ttypename))
            return (*cmd)->_typeLocal[ttypename];
   }
   return NULL;
}

const telldata::TType* parsercmd::cmdBLOCK::getTypeByID(const telldata::typeID ID) const
{
   TELL_DEBUG(***getTypeByID***);
   // Roll back the BlockSTACK until name is found. return NULL otherwise
   typedef BlockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   typedef telldata::typeMAP::const_iterator CT;
   typedef telldata::TypeList::const_iterator CAT;
   for (BS cmd = blkstart; cmd != blkend; cmd++)
   {
      for (CT ctp = (*cmd)->_typeLocal.begin(); ctp != (*cmd)->_typeLocal.end(); ctp++)
         if (ID == ctp->second->ID()) return ctp->second;
      for (CAT ctp = (*cmd)->_typeAnoLo.begin(); ctp != (*cmd)->_typeAnoLo.end(); ctp++)
         if (ID == (*ctp)->ID()) return (*ctp);
   }
   return NULL;
}

/*!
 * Creates a new TELL variable in the heap. Called from the parser predominantly
 * for all kinds of variable operations excluding function parameters
 * @param ID - the typeID of the new variable
 * @param varName - variable name. This is not used normally. It is required
 * only for variables of type TtCallBack, because here in this function an object
 * of a cmdCALLBACK class is instantiated.
 * @param loc the location of the variable identifier in the source TELL file -
 * for error reporting purposes
 * @return the new TELL variable. It also can return NULL in case of error
 */
telldata::TellVar* parsercmd::cmdBLOCK::newTellvar(telldata::typeID ID, const char* varName, TpdYYLtype loc)
{
   if (ID & telldata::tn_listmask)
   {
      return(DEBUG_NEW telldata::TtList(ID));
   }
   else
   switch (ID)
   {
      case   telldata::tn_real  : return(DEBUG_NEW telldata::TtReal());
      case    telldata::tn_int  : return(DEBUG_NEW telldata::TtInt());
      case   telldata::tn_uint  : return(DEBUG_NEW telldata::TtUInt());
      case   telldata::tn_bool  : return(DEBUG_NEW telldata::TtBool());
      case    telldata::tn_pnt  : return(DEBUG_NEW telldata::TtPnt());
      case    telldata::tn_box  : return(DEBUG_NEW telldata::TtWnd());
      case    telldata::tn_bnd  : return(DEBUG_NEW telldata::TtBnd());
      case telldata::tn_laymap  : return(DEBUG_NEW telldata::TtLMap());
      case telldata::tn_hshstr  : return(DEBUG_NEW telldata::TtHshStr());
      case  telldata::tn_layer  : return(DEBUG_NEW telldata::TtLayer());
      case telldata::tn_string  : return(DEBUG_NEW telldata::TtString());
      case telldata::tn_layout  : return(DEBUG_NEW telldata::TtLayout());
      case telldata::tn_auxilary: return(DEBUG_NEW telldata::TtAuxdata());
      default:
      {
         const telldata::TType* utype = getTypeByID(ID);
         if (NULL == utype)
            tellerror("Bad type specifier", loc);
         else if (utype->isComposite())
            return (DEBUG_NEW telldata::TtUserStruct(static_cast<const telldata::TCompType*>(utype)));
         else
         { // callback variable
            const telldata::TCallBackType* vartype = static_cast<const telldata::TCallBackType*>(utype);
            parsercmd::cmdCALLBACK* cbfp = DEBUG_NEW cmdCALLBACK(vartype->paramList(),vartype->fType(), loc);
            if (addCALLBACKDECL(varName, cbfp, loc))
               return (DEBUG_NEW telldata::TtCallBack(ID, cbfp));
            else
               delete cbfp;
         }
         break;
      }
   }
   return NULL;
}

/*!
 * Creates a new TELL variable in the heap. Called from the parser explicitly for
 * function parameters. The only reason this method exists are Tell callbacks.
 * Otherwise it is equivalent to newTellvar method.
 * @param ID- the typeID of the new argument
 * @param loc the location of the argument identifier in the source TELL file -
 * for error reporting purposes
 * @return the new TELL variable. It also can return NULL in case of error
 */
telldata::TellVar* parsercmd::cmdBLOCK::newFuncArg(telldata::typeID ID, TpdYYLtype loc)
{
   if (ID & telldata::tn_listmask)
   {
      return(DEBUG_NEW telldata::TtList(ID));
   }
   else
   switch (ID)
   {
      case   telldata::tn_real  : return(DEBUG_NEW telldata::TtReal());
      case   telldata::tn_uint  : return(DEBUG_NEW telldata::TtUInt());
      case    telldata::tn_int  : return(DEBUG_NEW telldata::TtInt());
      case   telldata::tn_bool  : return(DEBUG_NEW telldata::TtBool());
      case    telldata::tn_pnt  : return(DEBUG_NEW telldata::TtPnt());
      case    telldata::tn_box  : return(DEBUG_NEW telldata::TtWnd());
      case    telldata::tn_bnd  : return(DEBUG_NEW telldata::TtBnd());
      case telldata::tn_laymap  : return(DEBUG_NEW telldata::TtLMap());
      case telldata::tn_hshstr  : return(DEBUG_NEW telldata::TtHshStr());
      case  telldata::tn_layer  : return(DEBUG_NEW telldata::TtLayer());
      case telldata::tn_string  : return(DEBUG_NEW telldata::TtString());
      case telldata::tn_layout  : return(DEBUG_NEW telldata::TtLayout());
      case telldata::tn_auxilary: return(DEBUG_NEW telldata::TtAuxdata());
      default:
      {
         const telldata::TType* utype = getTypeByID(ID);
         if (NULL == utype)
            tellerror("Bad type specifier", loc);
         else if (utype->isComposite())
            return (DEBUG_NEW telldata::TtUserStruct(static_cast<const telldata::TCompType*>(utype)));
         else
         { // callback function argument
            const telldata::TCallBackType* vartype = static_cast<const telldata::TCallBackType*>(utype);
            parsercmd::cmdCALLBACK* cbfp = DEBUG_NEW cmdCALLBACK(vartype->paramList(),vartype->fType(), loc);
            return (DEBUG_NEW telldata::TtCallBack(ID, cbfp));
         }
         break;
      }
   }
   return NULL;
}

parsercmd::cmdBLOCK* parsercmd::cmdBLOCK::popblk()
{
   TELL_DEBUG(cmdBLOCK_popblk);
   assert(_blocks.size() > 1);
   _blocks.pop_front();
   return _blocks.front();
}

void parsercmd::cmdBLOCK::addFUNC(std::string, cmdSTDFUNC* cQ)
{
   TELL_DEBUG(addFUNC);
   tellerror("Nested function definitions are not allowed");
   if (cQ)    delete cQ;
}

void parsercmd::cmdBLOCK::addUSERFUNC(FuncDeclaration*, cmdFUNC*, TpdYYLtype)
{
   TELL_DEBUG(addFUNC);
   tellerror("Nested function definitions are not allowed");
}

parsercmd::cmdFUNC* parsercmd::cmdBLOCK::addUSERFUNCDECL(FuncDeclaration*, TpdYYLtype)
{
   TELL_DEBUG(addFUNCDECL);
   tellerror("Function definitions can be only global");
   return NULL;
}

bool parsercmd::cmdBLOCK::addCALLBACKDECL(std::string name, cmdCALLBACK* decl, TpdYYLtype loc)
{
   TELL_DEBUG(addCALLBACKDECL);
   if ( _lclFuncMAP.end() == _lclFuncMAP.find(name) )
   { // function with this name is not found in the local function map
      _lclFuncMAP[name] = decl;
      return true;
   }
   return false;
}

int parsercmd::cmdBLOCK::execute()
{
   TELL_DEBUG(cmdBLOCK_execute);
   int retexec = EXEC_NEXT; // to secure an empty block
   for (CmdQUEUE::const_iterator cmd = _cmdQ.begin(); cmd != _cmdQ.end(); cmd++) {
      if ((retexec = (*cmd)->execute())) break;
   }
   return retexec;
}

void parsercmd::cmdBLOCK::cleaner(bool fullreset)
{
   TELL_DEBUG(cmdBLOCK_cleaner);
   while (!_cmdQ.empty()) {
      cmdVIRTUAL *a = _cmdQ.front();_cmdQ.pop_front();
      delete a;
   }
   while (_blocks.size() > 1)
   {
      parsercmd::cmdBLOCK* dblk = _blocks.front(); _blocks.pop_front();
      assert(dblk != this); // i.e. you must call this method from the bottom of the block hierarchy
      delete dblk;
   }
   if (fullreset)
      _blocks.clear();
   else
      assert(this == _blocks.front());
}

parsercmd::cmdBLOCK::~cmdBLOCK()
{
   for (CmdQUEUE::iterator CMDI = _cmdQ.begin(); CMDI != _cmdQ.end(); CMDI++)
      delete *CMDI;
   _cmdQ.clear();
   for (telldata::variableMAP::iterator VMI = _varLocal.begin(); VMI != _varLocal.end(); VMI++)
      delete VMI->second;
   _varLocal.clear();
   for (telldata::typeMAP::iterator TMI = _typeLocal.begin(); TMI != _typeLocal.end(); TMI++)
      delete TMI->second;
   _typeLocal.clear();
   for (telldata::TypeList::iterator TMI = _typeAnoLo.begin(); TMI != _typeAnoLo.end(); TMI++)
      delete (*TMI);
   _typeAnoLo.clear();
   for (CBFuncMAP::iterator FMI = _lclFuncMAP.begin(); FMI != _lclFuncMAP.end(); FMI++)
      delete FMI->second;
   _lclFuncMAP.clear();
}

void parsercmd::cmdBLOCK::copyContents( cmdFUNC* cQ )
{
   for (CmdQUEUE::const_iterator cmd = _cmdQ.begin(); cmd != _cmdQ.end(); cmd++)
      cQ->pushcmd(*cmd);

   _cmdQ.clear();

   for (telldata::variableMAP::iterator VMI = _varLocal.begin(); VMI != _varLocal.end(); VMI++)
      cQ->addID(VMI->first.c_str(), VMI->second);

   _varLocal.clear();

   for (telldata::typeMAP::iterator TMI = _typeLocal.begin(); TMI != _typeLocal.end(); TMI++)
      cQ->_typeLocal[TMI->first] = TMI->second;

   _typeLocal.clear();

   for (telldata::TypeList::iterator TMI = _typeAnoLo.begin(); TMI != _typeAnoLo.end(); TMI++)
      cQ->_typeAnoLo.push_back(*TMI);

   _typeLocal.clear();

   for (CBFuncMAP::iterator FMI = _lclFuncMAP.begin(); FMI != _lclFuncMAP.end(); FMI++)
      cQ->_lclFuncMAP[FMI->first] = FMI->second;

   _lclFuncMAP.clear();

   cQ->_nextLclTypeID = _nextLclTypeID;

}

telldata::variableMAP* parsercmd::cmdBLOCK::copyVarLocal()
{
   telldata::variableMAP* varmap = DEBUG_NEW telldata::variableMAP();
   for (telldata::variableMAP::iterator VMI = _varLocal.begin(); VMI != _varLocal.end(); VMI++)
      (*varmap)[VMI->first.c_str()] = VMI->second->selfcopy();
   return varmap;
}

void parsercmd::cmdBLOCK::restoreVarLocal(telldata::variableMAP& nvars)
{
   typedef telldata::variableMAP::iterator VMIT;
   for (VMIT VMI = _varLocal.begin(); VMI != _varLocal.end(); VMI++)
   {
      VMIT coresp = nvars.find(VMI->first.c_str());
      assert(coresp != nvars.end());
      VMI->second->assign(coresp->second);
      delete coresp->second;
   }
   nvars.clear();
}

void parsercmd::cmdBLOCK::initializeVarLocal()
{
   typedef telldata::variableMAP::iterator VMIT;
   for (VMIT VMI = _varLocal.begin(); VMI != _varLocal.end(); VMI++)
   {
      VMI->second->initialize();
   }
}

bool parsercmd::cmdBLOCK::checkDbSortState(DbSortState needsDbResort)
{
   if      ( (sdbrUNSORTED == needsDbResort) && (!_dbUnsorted))
      return (_dbUnsorted = true);
   else if ( (sdbrSORTED   == needsDbResort) &&   _dbUnsorted )
   {
      return (_dbUnsorted = false);
   }
   else return true;
}

parsercmd::cmdSTDFUNC* const parsercmd::cmdBLOCK::getLocalFuncBody(const char* fn, telldata::argumentQ* amap) const
{
   // Roll back the BlockSTACK until name is found. return NULL otherwise
   typedef BlockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++)
   {
      CBFuncMAP::const_iterator lclFunc = (*cmd)->_lclFuncMAP.find(fn);
      if ((*cmd)->_lclFuncMAP.end() != lclFunc)
      {
         bool strict;
         cmdSTDFUNC *fbody = lclFunc->second;
         if (0 == fbody->argsOK(amap, strict))
            return fbody;
      }
   }
   return NULL;
}

//=============================================================================
parsercmd::cmdSTDFUNC* const parsercmd::cmdBLOCK::getFuncBody
                                        (const char* fn, telldata::argumentQ* amap, std::string& errmsg) const
{
   cmdSTDFUNC *fbody = NULL;
   telldata::argumentQ* arguMap = (NULL == amap) ? DEBUG_NEW telldata::argumentQ : amap;
   // first check for local functions
   fbody = getLocalFuncBody(fn, arguMap);
   if (NULL == fbody)
   {
      typedef FunctionMAP::iterator MM;
      std::pair<MM,MM> range = _funcMAP.equal_range(fn);
      if (range.first == range.second)
      {
         std::stringstream errstream;
         errstream << "Call to undefined function \"" << fn << "\"";
         errmsg = errstream.str();
      }
      else
      {
         cmdSTDFUNC *candFuncBody = NULL;
         std::stringstream alPrompt; // argument list prompt
         std::stringstream alaPrompt; // argument list alternative prompt
         unsigned numMismatches = 0;
         unsigned numNonStrictMaches = 0;
         for (MM fb = range.first; fb != range.second; fb++)
         {
            bool strict;
            fbody = fb->second;
            if (0 == fbody->argsOK(arguMap, strict))
            {
               if (strict)
                  break; // got the function with strict match!
               else
               {
                  if (0 < numNonStrictMaches++) alaPrompt << "\n";
                  alaPrompt << fn << "( ";
                  NameList* funcCandidate = fbody->callingConv(&_typeLocal);
                  NameList::const_iterator CD = funcCandidate->begin();
                  while(++CD != funcCandidate->end())
                     alaPrompt << *CD << " ";
                  alaPrompt << ")";
                  delete funcCandidate;
                  candFuncBody = fbody;
                  fbody = NULL;
               }
            }
            else
            {
               if (0 < numMismatches++) alPrompt << "\n";
               alPrompt << fn << "( ";
               NameList* funcCandidate = fbody->callingConv(&_typeLocal);
               NameList::const_iterator CD = funcCandidate->begin();
               while(++CD != funcCandidate->end())
                  alPrompt << *CD << " ";
               alPrompt << ")";
               delete funcCandidate;
               fbody = NULL;
            }
         }
         if (NULL == fbody)
         {
            std::stringstream errstream;
            if (NULL == candFuncBody)
            {
               if (1 < numMismatches)
                  errstream << "Wrong arguments in function call. Expecting one of the following:\n";
               else
                  errstream << "Wrong arguments in function call. Expecting: ";
               errstream << alPrompt.str();
               errmsg = errstream.str();
            }
            else
            {
               if (1 < numNonStrictMaches)
               {// more than one non-strict matches
                  errstream << "Ambiguous function call. Candidates are:\n";
                  errstream << alaPrompt.str();
                  errmsg = errstream.str();
               }
               else
                  // single function with non-strict match
                  fbody = candFuncBody;
            }
         }
      }
   }
   if (NULL == amap) delete arguMap;
   return fbody;
}

parsercmd::cmdSTDFUNC*  const parsercmd::cmdBLOCK::getIntFuncBody(std::string funcName) const
{
   // retrieve the body of funcName
   FunctionMAP::const_iterator MM = _internalFuncMap.find(funcName);
   assert(MM != _internalFuncMap.end());
   cmdSTDFUNC *fbody = MM->second;
   return fbody;
}

bool parsercmd::cmdBLOCK::defValidate(const FuncDeclaration* decl, cmdFUNC*& funcdef)
{
   std::string fn = decl->name();
   const ArgumentLIST* alst = decl->argList();
   // convert ArgumentLIST to argumentMAP
   telldata::argumentQ arguMap;
   typedef ArgumentLIST::const_iterator AT;
   for (AT arg = alst->begin(); arg != alst->end(); arg++)
      arguMap.push_back(DEBUG_NEW telldata::ArgumentID((*arg)->second->get_type()));
   // get the function definitions with this name
   typedef FunctionMAP::iterator MM;
   std::pair<MM,MM> range = _funcMAP.equal_range(fn);
   bool allow_definition = false;
   for (MM fb = range.first; fb != range.second; fb++)
   {
      bool strict;
      if (decl->type() != fb->second->gettype())
         // if function with the same name, but different type is defined
         // - overloaded functions must be the same type
         break;
      else if (0 == fb->second->argsOK(&arguMap, strict))
      {// if function with this name and parameter list is already declared
         if (strict)
         {
            if (fb->second->internal())
               // can't redefine internal function
               break;
            else
            {
               allow_definition = true;
               if (fb->second->declaration())
                  funcdef = static_cast<cmdFUNC*>(fb->second);
               else
               {//i.e. definition
                  std::ostringstream ost;
                  ost << "Warning! User function \""<< fn <<"\" is redefined";
                  tell_log(console::MT_WARNING, ost.str());
                  delete (fb->second);
                  _funcMAP.erase(fb);
               }
               break;
            }
         }
         // else - not strict - keep going
      }
   }
   telldata::argQClear(&arguMap);
   return allow_definition;
}

bool parsercmd::cmdBLOCK::declValidate(const FuncDeclaration* decl, TpdYYLtype loc)
{
   std::string fn = decl->name();
   const ArgumentLIST* alst = decl->argList();
   // convert ArgumentLIST to argumentMAP
   telldata::argumentQ arguMap;
   typedef ArgumentLIST::const_iterator AT;
   for (AT arg = alst->begin(); arg != alst->end(); arg++)
      arguMap.push_back(DEBUG_NEW telldata::ArgumentID((*arg)->second->get_type()));
   // get the function definitions with this name
   typedef FunctionMAP::iterator MM;
   std::pair<MM,MM> range = _funcMAP.equal_range(fn);
   bool allow_declaration = true;
   for (MM fb = range.first; fb != range.second; fb++)
   {
      std::ostringstream ost;
      ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
      bool strict;
      if (decl->type() != fb->second->gettype())
      {// if function with the same name, but different type is defined
         ost << "Overloaded functions must be the same type \"" << fn << "\"";
         tell_log(console::MT_ERROR, ost.str());
         allow_declaration = false;
         break;
      }
      else if (0 == fb->second->argsOK(&arguMap, strict))
      {// if function with this name and parameter list is already defined
         if (strict)
         {
            if (fb->second->internal())
            {
               ost << "Can't redeclare internal function \"" << fn << "\"";
               tell_log(console::MT_ERROR, ost.str());
               allow_declaration = false;
               break;
            }
            else if (!fb->second->declaration())
            {
               //NOTE! The warnings here and in the following block are quite confusing
               //      for the user.
               //ost << "Function \"" << fn << "\" already defined at this point.";
               //tell_log(console::MT_WARNING, ost.str());
               allow_declaration = false;
               break;
            }
            else
            {
               //ost << "Function \"" << fn << "\" already declared at this point.";
               //tell_log(console::MT_WARNING, ost.str());
               allow_declaration = false;
               break;
            }
         }
         //else
         // non strict - function overloading - keep going
      }
      // else -
      // if arguments are not matched - that's exactly the case of overloaded functions
      // so - keep going
   }
   telldata::argQClear(&arguMap);
   return allow_declaration;
}

//=============================================================================
int parsercmd::cmdSTDFUNC::argsOK(telldata::argumentQ* amap, bool& strict)
{
// This function is rather twisted, but this seems the only way to deal with
// anonymous user defined structures handled over as input function arguments.
// Otherwise we have to restrict significantly the input arguments rules for
// functions. Here is the problem.
// Functions in tell can be overloaded. In the same time we can have user defined
// structures, that have coincidental fields - for example the fields of the point
// structure coincides with the fields of an user structure defined as
// struct sameAsPoint{real a, real z}
// And on top of this we can have two overloaded functions, that have as a first
// argument a variables of type point and sameAsPoint respectively. The problem
// comes when the function is called with anonymous arguments (not with variables),
// which type can not be determined without the type of the function parameter.
// Here is the idea
// 1. If an unknown type appears in the argument list
//  a) Create a copy of the argument using ArgumentID copy constructor
//  b) Check that the new argument matches the type of the function parameter and
//     if so:
//     - assign (adjust) the type of the argument to the type of the parameter
//     - push the argument in the temporary structure
//  c) If the argument doesn't match, bail-out, but don't forget to clean-up the
//     the copies of the previously checked arguments
// 2. When the entire argument list is checked and it matches the corresponding
//    function parameter types, use the saved list of adjusted arguments to re-adjust
//    the original user defined argument types, which will be used to execute
//    properly the cmdSTRUCT commands already pushed into the command stack during
//    the bison parsing
// There is one remaining problem here. It is still possible to have two or even more
// overloaded functions defined with effectively the same parameter list. In this case,
// when that function is called with anonymous argument(s) the tellyzer will invoke the
// first function body that matches the entire list of input arguments. This will be most
// likely undefined. To prevent this we need better checks during the function definition
// parsing
   strict = true;
   unsigned unmapchedArgs = amap->size();
   if (unmapchedArgs != _arguments->size()) return -1;
   telldata::argumentQ UnknownArgsCopy;
   // :) - some fun here, but it might be confusing - '--' postfix operation is executed
   // always after the comparison, but before the cycle body. So. if all the arguments
   // are checked (match), the cycle ends-up with unmapchedArgs == -1;
   while (unmapchedArgs-- > 0)
   {
      telldata::typeID cargID = (*(*amap)[unmapchedArgs])();
      telldata::ArgumentID carg((*(*amap)[unmapchedArgs]));
      telldata::typeID lvalID = (*_arguments)[unmapchedArgs]->second->get_type();
      if (TLUNKNOWN_TYPE(cargID))
      {
         const telldata::TType* vartype;
         if (TLISALIST(lvalID))
         { // we have a list lval
            vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
            if (NULL != vartype) carg.userStructListCheck(vartype, false);
            else carg.toList(false, lvalID & ~telldata::tn_listmask);
         }
         else
         { // we have a struct only
            vartype = CMDBlock->getTypeByID(lvalID);
            if (NULL != vartype) carg.userStructCheck(vartype, false);
         }
      }

      if (!NUMBER_TYPE( carg() ))
      {  // for non-number types there is no internal conversion,
         // so check strictly the type
         if ( carg() != lvalID)
            break;
         else if (TLUNKNOWN_TYPE( (*(*amap)[unmapchedArgs])() ))
            UnknownArgsCopy.push_back(DEBUG_NEW telldata::ArgumentID(carg));
      }
      else
      {  // for number types - allow compatibility
         strict &= (carg() == lvalID);
         if ((!NUMBER_TYPE(lvalID)) || ( carg() > lvalID))
            break;
         else if (TLUNKNOWN_TYPE( (*(*amap)[unmapchedArgs])() ))
            UnknownArgsCopy.push_back(DEBUG_NEW telldata::ArgumentID(carg));
      }
   }
   unmapchedArgs++;
   if (UnknownArgsCopy.size() > 0)
   {
      if (unmapchedArgs > 0)
      {
         for (telldata::argumentQ::iterator CA = UnknownArgsCopy.begin(); CA != UnknownArgsCopy.end(); CA++)
            delete (*CA);
         UnknownArgsCopy.clear();
      }
      else
         for (telldata::argumentQ::iterator CA = amap->begin(); CA != amap->end(); CA++)
            if ( TLUNKNOWN_TYPE((**CA)()) )
            {
               (*CA)->adjustID(*(UnknownArgsCopy.back()));
               delete UnknownArgsCopy.back(); UnknownArgsCopy.pop_back();
            }
      assert(UnknownArgsCopy.size() == 0);
   }
   return (unmapchedArgs);
}

void parsercmd::cmdSTDFUNC::reduce_undo_stack()
{
   if (UNDOcmdQ.size() > CMDBlock->undoDepth())
   {
      UNDOcmdQ.back()->undo_cleanup(); UNDOcmdQ.pop_back();
   }
}

NameList* parsercmd::cmdSTDFUNC::callingConv(const telldata::typeMAP* lclTypeDef)
{
   NameList* argtypes = DEBUG_NEW NameList();
   argtypes->push_back(telldata::echoType(gettype(), lclTypeDef));
   int argnum = _arguments->size();
   for (int i = 0; i != argnum; i++)
      argtypes->push_back(telldata::echoType((*_arguments)[i]->second->get_type(), lclTypeDef));
   return argtypes;
}

parsercmd::cmdSTDFUNC::~cmdSTDFUNC()
{
   ClearArgumentList(_arguments);
   delete _arguments;
}

//=============================================================================
parsercmd::cmdFUNC::cmdFUNC(ArgumentLIST* vm, telldata::typeID tt, bool declaration, TpdYYLtype loc):
         cmdSTDFUNC(vm,tt,true, sdbrDONTCARE), cmdBLOCK(), _declaration(declaration)
{
   _recursyLevel = 0;
   if (!_declaration)
   {
      // copy the arguments in the structure of the local variables
      // callback arguments need a special attention
      typedef ArgumentLIST::const_iterator AT;
      for (AT arg = _arguments->begin(); arg != _arguments->end(); arg++)
      {
         telldata::typeID aID = (*arg)->second->get_type();
         if (TLCOMPOSIT_TYPE(aID))
         {
            const telldata::TType* utype = getTypeByID(aID);
            if ((NULL != utype) && (!utype->isComposite()))
            { // we have a callback argument. It means that right here it must
              // be converted from a variable to a function declaration
               telldata::TtCallBack* cbVar = static_cast<telldata::TtCallBack*>((*arg)->second);
               bool valid = addCALLBACKDECL((*arg)->first, cbVar->fcbBody(), loc);
               assert(valid); //the eventual error condition should've been cough during variable checks
            }
         }
         _varLocal[(*arg)->first] = (*arg)->second->selfcopy();
      }
   }
}

int parsercmd::cmdFUNC::execute()
{
   if (_recursyLevel > 0)  _VARLocalStack.push(copyVarLocal());
   _recursyLevel++;
   // get the arguments from the operands stack and replace the values
   // of the function arguments
   int i = _arguments->size();
   while (i-- > 0)
   {
      //get the argument name
      std::string   argname = (*_arguments)[i]->first;
      // get the tell variable (by name)
      telldata::TellVar* argvar = _varLocal[argname];
      // get a value from the operand stack
      telldata::TellVar* argval = OPstack.top();
      // replace the value of the local variable with the argument value
      argvar->assign(argval);
      delete argval;OPstack.pop();
   }
   std::string funcname = LogFile.getFN();
   LogFile << "// >> Entering UDF \"" << funcname << "\" .Recurse level:" << _recursyLevel;
   LogFile.flush();
   // The backup/restore operation is because of the issue with cmdSTACKRST/cmdFUNC
   // coexistence. In short - cmdSTACKRST is clearing the stack after (almost) each
   // operation, but this would crash a simple add operator which uses more than one
   // function calls as operands.
   // See http://code.google.com/p/toped/issues/detail?id=68 (Issue 68) for further info.
   BackupList* los = backupOperandStack();
   int retexec = cmdBLOCK::execute();
   restoreOperandStack(los);
   LogFile << "// << Exiting  UDF \"" << funcname << "\" .Recurse level:" << _recursyLevel;
   LogFile.flush();
   _recursyLevel--;
   if (_recursyLevel > 0)
   {
      telldata::variableMAP* top_stack = _VARLocalStack.top();_VARLocalStack.pop();
      restoreVarLocal(*top_stack);
      delete top_stack;
   }
   else
   {
      // we must reinitialise local variables - otherwise we'll get unexpected
      // results - for example local lists are not empty!
      initializeVarLocal();
   }
   if (EXEC_ABORT == retexec)
   {
      std::stringstream info;
      info << "Called in UDF \"" << funcname << "\"";
      tell_log(console::MT_ERROR, info.str());
      return retexec;
   }
   else return EXEC_NEXT;
}

parsercmd::cmdFUNC::BackupList* parsercmd::cmdFUNC::backupOperandStack()
{
   BackupList* los = DEBUG_NEW BackupList();
   while (!OPstack.empty())
   {
      los->push_front(OPstack.top());OPstack.pop();
   }
   return los;
}

void parsercmd::cmdFUNC::restoreOperandStack(parsercmd::cmdFUNC::BackupList* los)
{
   if (!OPstack.empty()) los->push_back(OPstack.top());
   while (!OPstack.empty()) OPstack.pop();
   for(BackupList::const_iterator CV = los->begin(); CV != los->end(); CV++)
   {
      OPstack.push(*CV);
   }
   los->clear();
   delete los;
}
//=============================================================================
parsercmd::cmdCALLBACK::cmdCALLBACK(const telldata::TypeIdList&  paramlist, telldata::typeID retype, TpdYYLtype loc) :
   cmdFUNC( DEBUG_NEW parsercmd::ArgumentLIST, retype, true, loc),
   _fbody ( NULL)
{
   for (telldata::TypeIdList::const_iterator CP = paramlist.begin(); CP != paramlist.end(); CP++)
   {
      telldata::TellVar* lvar = newCallBackArgument(*CP, loc);
      _arguments->push_back(DEBUG_NEW parsercmd::ArgumentTYPE(std::string(""),lvar));
   }
}

int parsercmd::cmdCALLBACK::execute()
{
   if (_declaration)
   {
      tell_log(console::MT_ERROR,"Internal parser error. Callback function body missing");
      return EXEC_ABORT;
   }
   else
   {
      assert(NULL != _fbody);
      return _fbody->execute();
   }
}
//=============================================================================
int parsercmd::cmdIFELSE::execute()
{
   TELL_DEBUG(cmdIFELSE);
   int retexec = EXEC_NEXT;
   telldata::TtBool *cond = static_cast<telldata::TtBool*>(OPstack.top());OPstack.pop();
   if (cond->value())   retexec =  _trueblock->execute();
   else if (_falseblock) retexec = _falseblock->execute();
   delete cond;
   return retexec;
}

//=============================================================================
int parsercmd::cmdWHILE::execute()
{
   TELL_DEBUG(cmdWHILE);
   int retexec;
   int retexec1 = EXEC_NEXT;
   telldata::TtBool *cond;
   bool    condvalue;
   while (true)
   {
      _condblock->execute();
      cond = static_cast<telldata::TtBool*>(OPstack.top());OPstack.pop();
      condvalue = cond->value(); delete cond;
      if (condvalue)    retexec = _body->execute();
      else              break;

      if (checkNextLoop(retexec, retexec1)) break;
   }
   return retexec1;
}

//=============================================================================
int parsercmd::cmdREPEAT::execute()
{
   TELL_DEBUG(cmdREPEAT);
   int retexec;
   int retexec1 = EXEC_NEXT;
   telldata::TtBool *cond;
   bool    condvalue;
   while (true)
   {
      retexec = _body->execute();
      if (checkNextLoop(retexec, retexec1)) break;
      _condblock->execute();
      cond = static_cast<telldata::TtBool*>(OPstack.top());OPstack.pop();
      condvalue = cond->value(); delete cond;
      if (tellPP->repeatLC() ^ condvalue) break;
//      {
//         if (!condvalue)            break;
//      }
//      else
//      {
//         if (condvalue)            break;
//      }
   }
   return retexec1;
}

//=============================================================================
int parsercmd::cmdFOREACH::execute()
{
   TELL_DEBUG(cmdFOREACH);
   int retexec;
   int retexec1 = EXEC_NEXT;

   _header->execute();

   telldata::TtList* clist;
   bool listVariable = (NULL != _listvar);
   if (listVariable)
   {
      clist = static_cast<telldata::TtList*>(_listvar);
   }
   else
   {
      clist = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   }

   telldata::memlist valist = clist->mlist();
   for (telldata::memlist::const_iterator CI = valist.begin(); CI != valist.end(); CI++)
   {
      _var->assign(*CI);
      retexec = _body->execute();
      if (listVariable)
         (*CI)->assign(_var);

      if (checkNextLoop(retexec, retexec1)) break;
   }
   if (!listVariable)
      delete clist;
   return retexec1;
}

parsercmd::cmdFOREACH::~cmdFOREACH()
{
//   delete _var;
   if (NULL != _header)
   {
      delete _header; _header = NULL;
   }
   if (NULL != _body)
   {
      delete _body; _body = NULL;
   }
}

//=============================================================================
int parsercmd::cmdFOR::execute()
{
   TELL_DEBUG(cmdFOR);
   int retexec;
   int retexec1 = EXEC_NEXT;
   telldata::TtBool *cond;
   bool    condvalue;

   _init->execute();

   while (true)
   {
      if (_cond->empty())
         condvalue = true;
      else
      {
         _cond->execute();
         cond = static_cast<telldata::TtBool*>(OPstack.top());OPstack.pop();
         condvalue = cond->value(); delete cond;
      }
      if (condvalue)    retexec = _body->execute();
      else              break;
      if (checkNextLoop(retexec, retexec1)) break;
      _loop->execute();
   }
   return retexec1;
}

parsercmd::cmdFOR::~cmdFOR()
{
//   delete _var;
   if (NULL != _init)
   {
      delete _init; _init = NULL;
   }
   if (NULL != _cond)
   {
      delete _cond; _cond = NULL;
   }
   if (NULL != _loop)
   {
      delete _loop; _loop = NULL;
   }
   if (NULL != _body)
   {
      delete _body; _body = NULL;
   }
}

//=============================================================================
int parsercmd::cmdBREAK::execute()
{
   return EXEC_BREAK;
}

//=============================================================================
int parsercmd::cmdCONTINUE::execute()
{
  return EXEC_CONTINUE;
}

//=============================================================================
int parsercmd::cmdMAIN::execute()
{
   TELL_DEBUG(cmdMAIN_execute);
   int retexec = EXEC_NEXT;
   while (!_cmdQ.empty())
   {
      cmdVIRTUAL *a = _cmdQ.front();_cmdQ.pop_front();
      if (EXEC_NEXT == retexec) retexec = a->execute();
      delete a;
   }
   try
   {
      if (_dbUnsorted)
      {
         cmdSTDFUNC* sortFunc = getIntFuncBody("$sort_db");
         sortFunc->execute();
         _dbUnsorted = false;
      }
   }
   catch (EXPTN&) {return EXEC_ABORT;}
   return retexec;
}

void parsercmd::cmdMAIN::addFUNC(std::string fname , cmdSTDFUNC* cQ)
{
   _funcMAP.insert(std::make_pair(fname,cQ));
   TpdPost::tellFnAdd(fname, cQ->callingConv(NULL));
}

void parsercmd::cmdMAIN::addIntFUNC(std::string fname , cmdSTDFUNC* cQ)
{
   _internalFuncMap.insert(std::make_pair(fname,cQ));
}

/*!
*/
void parsercmd::cmdMAIN::addUSERFUNC(FuncDeclaration* decl, cmdFUNC* cQ, TpdYYLtype loc)
{
   cmdFUNC* declfunc = NULL;
   if ((telldata::tn_void != decl->type()) && (0 == decl->numReturns()))
   {
      tellerror("function must return a value", loc);
   }
   else  if (decl->numErrors() > 0) {
      tellerror("function definition is ignored because of the errors above", loc);
   }
   /*Check whether such a function is already defined */
   else if ( CMDBlock->defValidate(decl, declfunc) )
   {
      if (declfunc)
      {// pour over the definition contents in the body created by the declaration
         cQ->copyContents(declfunc);
         declfunc->setDefined();
         TpdPost::tellFnAdd(decl->name(), cQ->callingConv(&_typeLocal));
         TpdPost::tellFnSort();
      }
      else
      {// the only reason to be here must be that the function is redefined
         _funcMAP.insert(std::make_pair(decl->name(), cQ));
         delete decl;
         return; // i.e. don't delete cQ - we need it!
      }
   }
   delete cQ;
   delete decl;
}

/*! The method calls CMDBlock::declValidate(...) to check whether a function
with such name and calling convention already exists. If it doesn't - a new cmdFUNC will
be created and will be inserted in the _funcMAP. The contents of the new cmdFUNC will be
empty and cmdFUNC._declaration will be true.
*/
parsercmd::cmdFUNC* parsercmd::cmdMAIN::addUSERFUNCDECL(FuncDeclaration* decl, TpdYYLtype loc)
{
   cmdFUNC* cQ = NULL;
   if (CMDBlock->declValidate(decl,loc))
   {
      cQ = DEBUG_NEW parsercmd::cmdFUNC(decl->argListCopy(),decl->type(), true, loc);
      _funcMAP.insert(std::make_pair(decl->name(), cQ));
   }
   return cQ;
}

parsercmd::cmdMAIN::cmdMAIN():cmdBLOCK(telldata::tn_usertypes)
{
   pushblk();
};

void  parsercmd::cmdMAIN::addGlobalType(std::string ttypename, telldata::TType* ntype)
{
   assert(_typeLocal.end() == _typeLocal.find(ttypename));
   _typeLocal[ttypename] = ntype;
}

void parsercmd::cmdMAIN::recoveryDone()
{
   cmdSTDFUNC::_ignoreOnRecovery = false;
}

parsercmd::cmdMAIN::~cmdMAIN()
{
   while (UNDOcmdQ.size() > 0)
   {
      UNDOcmdQ.back()->undo_cleanup();UNDOcmdQ.pop_back();
   }
   for (FunctionMAP::iterator FMI = _funcMAP.begin(); FMI != _funcMAP.end(); FMI ++)
      delete FMI->second;
   _funcMAP.clear();
   for (FunctionMAP::iterator FMI = _internalFuncMap.begin(); FMI != _internalFuncMap.end(); FMI ++)
      delete FMI->second;
   _internalFuncMap.clear();

};

//=============================================================================
telldata::typeID parsercmd::UMinus(telldata::typeID op1, TpdYYLtype loc1)
{
   if (NUMBER_TYPE(op1))
   {
      CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdUMINUS(op1));
      if (telldata::tn_uint == op1)
         return telldata::tn_int;
      else
         return op1;
   }
   else {
      tellerror("unexpected operand type",loc1);
      return telldata::tn_void;
   }
}

//=============================================================================
//     +      |real |point| box |
//------------+-----+-----+-----+
//   real     |  +  |shift| blow| *TBD
//   point    |shift|shift|shift| also:
//   box      |blow |shift| or* | string + string => concatenation
//-----------------------------------------------------------------------------
telldata::typeID parsercmd::Plus(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2)
{
   switch (op1)   {
      case  telldata::tn_uint:
         switch(op2) {
            case  telldata::tn_uint:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS(telldata::tn_uint));
                            return telldata::tn_uint;
            case  telldata::tn_int:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS(telldata::tn_int));
                            return telldata::tn_int;
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS(telldata::tn_real));
                            return telldata::tn_real;
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(1, true));
                            return telldata::tn_pnt;
            case telldata::tn_box: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBLOWBOX(1, true));
                            return telldata::tn_box;
                          default: tellerror("unexpected operand type",loc2);break;
         };break;
      case  telldata::tn_int:
         switch(op2) {
            case  telldata::tn_uint:
            case  telldata::tn_int:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS(telldata::tn_int));
                            return telldata::tn_int;
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS(telldata::tn_real));
                            return telldata::tn_real;
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(1, true));
                            return telldata::tn_pnt;
            case telldata::tn_box: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBLOWBOX(1, true));
                            return telldata::tn_box;
                          default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_real:
         switch(op2) {
            case  telldata::tn_uint:
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS(telldata::tn_real));
                            return telldata::tn_real;
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(1, true));
                            return telldata::tn_pnt;
            case telldata::tn_box: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBLOWBOX(1, true));
                            return telldata::tn_box;
                          default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_pnt:
         switch(op2) {
            case  telldata::tn_uint:
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(1, false));
                           return telldata::tn_pnt;
            case  telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT2());
                           return telldata::tn_pnt;
            case  telldata::tn_box:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX(1, true));
                           return telldata::tn_box;
                           default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case  telldata::tn_uint:
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBLOWBOX(1, false));
                        return telldata::tn_box;
            case telldata::tn_pnt :CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX(1, false));
                        return telldata::tn_box;
//            case telldata::tn_box:    // logical OR?
                           default: tellerror("unexpected operand type",loc2); break;
         };break;
      case telldata::tn_string:
         if (telldata::tn_string == op2) {
            CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdCONCATENATE());
            return telldata::tn_string;
         }
         else tellerror("unexpected operand type",loc2);
         break;
      default: tellerror("unexpected operand type",loc1);break;
   }
   return telldata::tn_void;
}

//=============================================================================
//     -      | real |point| box |
//------------+------+-----+-----+
//   real     |  x   |  -  |  -  |
//   point    | shift|shift|  -  | 
//   box      |shrink|shift| or* | * TBD
//-----------------------------------------------------------------------------
telldata::typeID parsercmd::Minus(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2)
{
   switch (op1)   {
      case telldata::tn_uint:
         switch(op2) {
            case   telldata::tn_uint:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS(telldata::tn_uint));
                            return telldata::tn_uint;
            case   telldata::tn_int:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS(telldata::tn_int));
                            return telldata::tn_int;
            case  telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS(telldata::tn_real));
                            return telldata::tn_real;
                  default: tellerror("unexpected operand type",loc2);break;
         };break;
       case telldata::tn_int:
          switch(op2) {
             case   telldata::tn_uint:
             case   telldata::tn_int:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS(telldata::tn_int));
                             return telldata::tn_int;
             case  telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS(telldata::tn_real));
                             return telldata::tn_real;
                   default: tellerror("unexpected operand type",loc2);break;
          };break;
      case telldata::tn_real:
         switch(op2) {
            case   telldata::tn_uint:
            case   telldata::tn_int:
            case  telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS(telldata::tn_real));
                            return telldata::tn_real;
                  default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(-1, false));
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT2(-1));
                           return telldata::tn_pnt;
                  default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case   telldata::tn_uint:
            case   telldata::tn_int:
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBLOWBOX(-1, false));
                        return telldata::tn_box;
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX(-1, false));
                        return telldata::tn_box;
            case telldata::tn_box:    // or not ???
            //case tn_ttPoly:
                  default: tellerror("Unexpected operand type",loc2);break;
         };break;
      default: tellerror("Unexpected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::PointMv(telldata::typeID op1, telldata::typeID op2,
                                   TpdYYLtype loc1, TpdYYLtype loc2, int xdir, int ydir)
{
   switch (op1)   {
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT3(xdir,ydir));
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT4(xdir,ydir));
                           return telldata::tn_pnt;
                  default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX3(xdir,ydir));
                        return telldata::tn_box;
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX4(xdir,ydir));
                        return telldata::tn_box;
                  default: tellerror("Unexpected operand type",loc2); break;
         };break;
      default: tellerror("Unexpected operand type",loc1);break;
   }
   return telldata::tn_void;
}

//=============================================================================
//     *      |real |point| box |
//------------+-----+-----+-----+
//   real     |  x  |scale|scale|
//   point    |scale|     |     |
//   box      |scale|     | and |
//-------------------------------
telldata::typeID parsercmd::Multiply(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2)
{
   switch (op1)
   {
      case telldata::tn_uint:
         switch(op2)
         {
             case telldata::tn_uint: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY(telldata::tn_uint));
                                     return telldata::tn_uint;
             case telldata::tn_int: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY(telldata::tn_int));
                                     return telldata::tn_int;
             case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY(telldata::tn_real));
                                     return telldata::tn_real;
             case telldata::tn_pnt: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEPNT(true, true));
                                     return telldata::tn_pnt;
             case telldata::tn_box:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEBOX(true, true));
                                     return telldata::tn_box;
                            default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_int:
         switch(op2)
         {
             case telldata::tn_uint:
             case telldata::tn_int: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY(telldata::tn_int));
                                     return telldata::tn_int;
             case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY(telldata::tn_real));
                                     return telldata::tn_real;
             case telldata::tn_pnt: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEPNT(true, true));
                                     return telldata::tn_pnt;
             case telldata::tn_box:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEBOX(true, true));
                                     return telldata::tn_box;
                            default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_real:
         switch(op2)
         {
             case telldata::tn_uint:
             case telldata::tn_int:
             case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY(telldata::tn_real));
                                     return telldata::tn_real;
             case telldata::tn_pnt: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEPNT(true, true));
                                     return telldata::tn_pnt;
             case telldata::tn_box:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEBOX(true, true));
                                     return telldata::tn_box;
                            default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_pnt:
         switch(op2)
         {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEPNT(true, false));
                                    return telldata::tn_pnt;
                           default: tellerror("unexpected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2)
         {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEBOX(true, false));
                                   return telldata::tn_box;
                           default: tellerror("unexpected operand type",loc2);break;
         };break;
      default: tellerror("unexpected operand type",loc1);break;
   }
   return telldata::tn_void;
}

//=============================================================================
//     /      |real |point|  box |
//------------+-----+-----+------+
//   real     |  x  |     |      |
//   point    |scale|     |      |
//   box      |scale|     |andnot|
//-------------------------------
telldata::typeID parsercmd::Divide(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2)
{
   switch (op1)
   {
      case telldata::tn_uint:
         switch(op2)
         {
            case telldata::tn_uint:  CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION(telldata::tn_uint));
                                   return telldata::tn_uint;
            case telldata::tn_int: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION(telldata::tn_int));
                                   return telldata::tn_int;
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION(telldata::tn_real));
                                    return telldata::tn_real;
                           default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_int:
         switch(op2)
         {
            case telldata::tn_uint:
            case telldata::tn_int: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION(telldata::tn_int));
                                   return telldata::tn_int;
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION(telldata::tn_real));
                                    return telldata::tn_real;
                           default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_real:
         switch(op2)
         {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION(telldata::tn_real));
                                    return telldata::tn_real;
                           default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_pnt:
         switch(op2)
         {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEPNT(false, false));
                                    return telldata::tn_pnt;
                  default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      case telldata::tn_box:
         switch(op2)
         {
            case telldata::tn_uint:
            case telldata::tn_int:
            case telldata::tn_real: CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSCALEBOX(false, false));
                                    return telldata::tn_box;
                           default: tellerror("unexpected operand type",loc2);break;
         };
         break;
      default: tellerror("unexpected operand type",loc1);break;
   }
   return telldata::tn_void;
}

bool parsercmd::StructTypeCheck(telldata::typeID targett,
                                      telldata::ArgumentID* op2, TpdYYLtype loc)
{
   VERIFY(TLUNKNOWN_TYPE((*op2)()));
   const telldata::TType* vartype;
   if (TLISALIST(targett))
   { // we have a list lval
      vartype = CMDBlock->getTypeByID(targett & ~telldata::tn_listmask);
      if (NULL != vartype) op2->userStructListCheck(vartype, true);
      else op2->toList(true, targett & ~telldata::tn_listmask);
   }
   else
   { // we have a struct only
      vartype = CMDBlock->getTypeByID(targett);
      if (NULL != vartype) op2->userStructCheck(vartype, true);
   }
   return (targett == (*op2)());
}

bool parsercmd::ListIndexCheck(telldata::typeID list, TpdYYLtype lloc,
                               telldata::typeID idx, TpdYYLtype iloc)
{
   bool checkval = false;
   if       (!(list & telldata::tn_listmask) && (telldata::tn_string != list))
      tellerror("list expected",lloc);
//   else if  ((idx != telldata::tn_int) && (idx != telldata::tn_real))
   else if (!INTEGER_TYPE(idx))
      tellerror("index is expected to be an integer",iloc);
   else
      checkval = true;
   return checkval;
}

bool parsercmd::ListSliceCheck(telldata::typeID list, TpdYYLtype lloc,
   telldata::typeID idx, TpdYYLtype iloc, telldata::typeID sidx, TpdYYLtype sloc)
{
//   if  ((sidx != telldata::tn_int) && (sidx != telldata::tn_real))
   if (!INTEGER_TYPE(sidx))
   {
      tellerror("slice size is expected to be an integer",iloc);
      return false;
   }
   else
      return ListIndexCheck(list, lloc, idx, iloc);
}

bool parsercmd::ListSliceCheck(telldata::typeID list, TpdYYLtype lloc,
                                    telldata::typeID sidx, TpdYYLtype sloc)
{
//   if  ((sidx != telldata::tn_int) && (sidx != telldata::tn_real))
   if (!INTEGER_TYPE(sidx))
   {
      tellerror("slice size is expected to be an integer",sloc);
      return false;
   }
   else if (!(list & telldata::tn_listmask))
   {
      tellerror("list expected",lloc);
      return false;
   }
   return true;
}

telldata::typeID parsercmd::Assign(telldata::TellVar* lval, byte indexed, telldata::ArgumentID* op2,
                                                                 TpdYYLtype loc)
{
   if (!lval)
   {
      tellerror("Lvalue undefined in assign statement", loc);
      return telldata::tn_void;
   }
   else if (lval->constant())
   {
      tellerror("Constant lvalue can't be changed", loc);
      return telldata::tn_void;
   }
   telldata::typeID lvalID = lval->get_type();
   if (1 == indexed)
   {
      // A slight complication here - when a single list component is an lvalue - then
      // we have to remove the list flag from the type ID. That's because the entire
      // list is handled over as lval, instead of the component itself. The latter is
      // because the lists are dynamic and the target component will be retrieved
      // during runtime
      // The above is not valid for index ranges (i.e. 2 == indexed), because an
      // index range results in a list
      lvalID &= ~telldata::tn_listmask;
   }
   // Here if user structure is used - clarify that it is compatible
   // The thing is that op2 could be a struct of a struct list or a list of
   // tell basic types. This should be checked in the following order:
   // 1. Get the type of the recipient (lval)
   // 2. If it is a list
   //    a) strip the list atribute and get the type of the list component
   //    b) if the type of the lval list component is compound (struct list), check the
   //       input structure for struct list
   //    c) if the type of the list component is basic, check directly that
   //       op2 is a list
   // 3. If it is not a list
   //    a) if the type of the lval is compound (struct), check the
   //       input structure for struct
   if (TLUNKNOWN_TYPE((*op2)()))
   {
      const telldata::TType* vartype;
      if (TLISALIST(lvalID))
      { // we have a list lval
          vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
          if (NULL != vartype) op2->userStructListCheck(vartype, true);
          else op2->toList(true, lvalID & ~telldata::tn_listmask);
      }
      else
      { // we have a struct only
         vartype = CMDBlock->getTypeByID(lvalID);
         if (NULL != vartype) op2->userStructCheck(vartype, true);
      }
   }
   if ((lvalID == (*op2)()) || (NUMBER_TYPE(lvalID) && NUMBER_TYPE((*op2)())))
   {
      CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdASSIGN(lval, indexed));
      // don't forget to update cmdSTRUCT (if this is the rval) with the
      // validated ArgumentID
//      if (NULL != op2->command())
//         static_cast<cmdSTRUCT*>(op2->command())->setargID(op2);
      return lvalID;
   }
   else
   {
      tellerror("Incompatible operand types in assignment", loc);
      return telldata::tn_void;
   }
}

telldata::typeID parsercmd::Uninsert(telldata::TellVar* lval, telldata::ArgumentID* op2,
                                                    parsercmd::cmdLISTADD* unins_cmd, TpdYYLtype loc)
{
   // List add/insert operators can be described as "composite" operators - i.e.
   // they are constituted by two operators:
   // -> insert index (<idx>:+ / +:<idx> ) will add/insert a component in
   //    the list before/after the idx and will copy the value of the
   //    idx-th component into the new one - i.e. will assign a temporary
   //    value to it.
   // -> assignment -> a proper value will be assigned to the new component.
   // It's perfectly possible to leave insert index operator as completely
   // independent one, then operators like
   //    listvar[:+]; listvar[+:];
   // will be allowed. The point is though that they don't make much sense
   // alone. Then why keeping them? What we need is
   //    listvar[:+] = newValue;
   // and the parser must treat it as a single operator despite the
   // distributed syntax of the operator
   //
   // To make the things simpler (?!?) for the user we are adding the list
   // union operation with the same syntax. It should benefit from the
   // same index syntax - i.e the operator should be quite flexible because
   // list can be added/inserted anywhere in the target list. The beauty is
   // not coming for free though.
   // As an operation list union is not composite - i.e. we don't need to
   // insert an index - just to calculate it, and to make the things even
   // worse - no assignment operation required as well. Instead we need to
   // overload the assignment operator with list union operator which
   // appears to share the same '=' symbol. Then obviously the parser
   // won't know whether it will be assignment or union operator until
   // the clause of the right side of the '=' is not parsed (the rvalue).
   // By that time however a cmdLISTADD command should be normally inserted in
   // the operand stack and this must be avoided, otherwise we'll hide a
   // can of warms.
   // On top of this rvalue can be anonymous const, which type is initially
   // unknown and has to be determined using the type of the lvalue
   //
   // During the parsing of the listinsertindex clauses cmdLISTADD command
   // is created, but not pushed in the operand stack (it lacks its rvalue!).
   // When the entire '=' statement is parsed, this function has to clarify
   // whether the operands are compatible and what is the operator -
   // add/insert or union. If it is add/insert cmdLISTADD is pushed in the
   // stack (it's rvalue is already in the operand stack). Otherwise it is
   // scrapped. Instead new cmdLISTUNION is created and pushed in the stack
   if (!lval)
   {
      tellerror("Lvalue undefined in list union/insert statement", loc);
      return telldata::tn_void;
   }
   telldata::typeID lvalID = lval->get_type();
   if (NULL == unins_cmd) return lvalID;
   if (telldata::tn_string == lvalID)
   {
      if (telldata::tn_string != (*op2)())
      {
         tellerror("string expected as rvalue", loc);
         return telldata::tn_void;
      }
      CMDBlock->pushcmd(unins_cmd);
      return telldata::tn_string;
   }
   else
   {
      // Here if user structure is used - clarify that it is compatible
      // The thing is that op2 could be a struct of a struct list or a list of
      // tell basic types. This should be checked in the following order:
      // 1. Get the type of the recipient (lval)
      //    It must be a list
      //    a) strip the list attribute and get the type of the list component
      //    b) if the type of the lval list component is compound (struct list), check the
      //       input structure for struct list
      //    c) if the type of the list component is basic, check directly that
      //       op2 is a list
      //     d) if after all the above the op2 is still unknown i.e. not a list - try to
      //       convert it into a structure
      if (TLUNKNOWN_TYPE((*op2)()))
      {
         // lvalue in this function must be a list
         assert(TLISALIST(lvalID));
         const telldata::TType* vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
         if (NULL != vartype) op2->userStructListCheck(vartype, true);
         else op2->toList(true, lvalID & ~telldata::tn_listmask);
         if (TLUNKNOWN_TYPE((*op2)()))
            op2->userStructCheck(vartype, true);
      }
      if (TLISALIST((*op2)()))
      {  // operation is union
         CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdLISTUNION(unins_cmd));
         delete(unins_cmd);
         return lvalID;
      }
      else
      {  // operation is list add/insert
         lvalID &= ~telldata::tn_listmask;
         if ((lvalID == (*op2)()) || (NUMBER_TYPE(lvalID) && NUMBER_TYPE((*op2)())))
         {
            // the lval is already in unins_cmd - that's why it's not used here
            CMDBlock->pushcmd(unins_cmd);
            return lvalID |= telldata::tn_listmask;
         }
         else
         {
            delete unins_cmd;
            tellerror("Incompatible operand types in list add/insert", loc);
            return telldata::tn_void;
         }
      }
   }
}

//=============================================================================
// in case real/bool -> real is casted to bool automatically during the
// execution such as:
// 0 -> false, everything else -> true
//
// </>,<=/>=  |real |point| box | poly|
//------------+-----+-----+-----+-----+
//   real     |  x  |  -  |  -  |  -  |
//   point    |  -  |  -  |  -  |  -  |
//   box      |  -  |  -  |  -  |  -  |
//   poly     |  -  |  -  |  -  |  -  |
//---------------------------------------
//     =/!=   |real |point| box | poly|
//------------+-----+-----+-----+-----+
//   real     |  x  |  -  |  -  |  -  |
//   point    |  -  |  x  |  -  |  -  |
//   box      |  -  |  -  |  x  |  -  |
//   poly     |  -  |  -  |  -  |  x  |
//---------------------------------------
//    &&/||   |real |point| box | poly|
//------------+-----+-----+-----+-----+
//   real     |  x  |  -  |  -  |  -  |
//   point    |  -  |  -  |  -  |  -  |
//   box      |  -  |  -  | ??? |  -  |  how about +-*/
//   poly     |  -  |  -  |  -  | ??? |  how about +-*/
//---------------------------------------
telldata::typeID parsercmd::BoolEx(telldata::typeID op1, telldata::typeID op2,
                                 std::string ope, TpdYYLtype loc1, TpdYYLtype loc2)
{
   if (NUMBER_TYPE(op1) && NUMBER_TYPE(op2))
   {
      if      (ope == "<" ) CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdLT());
      else if (ope == "<=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdLET());
      else if (ope ==  ">") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdGT());
      else if (ope == ">=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdGET());
      else if (ope == "==") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdEQ());
      else if (ope == "!=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdNE());
      else if((ope == "<<") || (ope == ">>"))
      {
         if (INTEGER_TYPE(op1) && INTEGER_TYPE(op2))
         {

            CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWSHIFT(op1,(ope == ">>")));
            return op1;
         }
         else
         {
            tellerror("unexpected operand type",loc1);
            return telldata::tn_void;
         }
      }
      else if (INTEGER_TYPE(op1) && INTEGER_TYPE(op2) && (op1 == op2))
      {
         if      (ope == "&") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWAND(op1));
         else if (ope == "|") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWOR(op1));
         else if (ope == "^") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWXOR(op1));
         else
         {
            tellerror("unexpected operand type",loc1);
            return telldata::tn_void;
         }
         return telldata::tn_int;
      }
      else
      {
         tellerror("unexpected operand type",loc1);
         return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else if ((telldata::tn_bool == op1) && (telldata::tn_bool == op2))
   {
      if      (ope == "&&") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdAND());
      else if (ope == "||") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdOR());
      else if (ope == "==") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdEQ());
      else if (ope == "!=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdNE());
      else
      {
         tellerror("unexpected operand type",loc1);return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else if ((telldata::tn_string == op1) && (telldata::tn_string == op2))
   {
      if      (ope == "==") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSTRCMP(true));
      else if (ope == "!=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSTRCMP(false));
      else
      {
         tellerror("unexpected operand type",loc1);return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else
   {
      tellerror("unexpected operand type",loc2);
      return telldata::tn_void;
   }
}

telldata::typeID parsercmd::BoolEx(telldata::typeID op1, std::string ope, TpdYYLtype loc1)
{
   if      (INTEGER_TYPE(op1))
   {
      if      (ope == "~" ) CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWNOT(op1));
      else
      {
         tellerror("unexpected operand type",loc1);return telldata::tn_void;
      }
      return op1;
   }
   else if (telldata::tn_bool == op1)
   {
      if      (ope == "!") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdNOT());
      else
      {
         tellerror("unexpected operand type",loc1);return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else
   {
      tellerror("unexpected operand type",loc1);
      return telldata::tn_void;
   }
}

void parsercmd::ClearArgumentList(ArgumentLIST* alst)
{
   if (NULL == alst) return;
   for (ArgumentLIST::iterator ALI = alst->begin(); ALI != alst->end(); ALI++) {
      delete (*ALI)->second;
      delete (*ALI);
   }
   alst->clear();
}

/*!
 * Variable Parameter List TELL functions require a special attention in parse
 * time. This function checks whether the input string fname matches any of the
 * names in the VPL functions.
 * @param fname - the function name to check
 * @return true if the function name corresponds to a VPL function
 */
bool parsercmd::vplFunc(std::string fname)
{
   return  (   (std::string("printf")  == fname )
            || (std::string("sprintf") == fname )
           );
}

bool parsercmd::checkNextLoop(int retexec, int&retexec1)
{
   switch (retexec)
   {
      case EXEC_NEXT:
      case EXEC_CONTINUE:
      case EXEC_BREAK: retexec1 = EXEC_NEXT; break;
      default: /*EXEC_RETURN & EXEC_ABORT*/retexec1 = retexec; break;
   }
   if ((EXEC_NEXT == retexec) || (EXEC_CONTINUE == retexec))
      return false;
   else
      return true;
}

//-----------------------------------------------------------------------------
// class FuncDeclaration
//-----------------------------------------------------------------------------
parsercmd::ArgumentLIST* parsercmd::FuncDeclaration::argListCopy() const
{
   parsercmd::ArgumentLIST* arglist = DEBUG_NEW parsercmd::ArgumentLIST;
   typedef ArgumentLIST::const_iterator AT;
   for (AT arg = _argList->begin(); arg != _argList->end(); arg++)
      arglist->push_back(DEBUG_NEW parsercmd::ArgumentTYPE((*arg)->first,(*arg)->second->selfcopy()));
   return arglist;
}

parsercmd::FuncDeclaration::~FuncDeclaration()
{
   ClearArgumentList(_argList);
   delete _argList;
}

//-----------------------------------------------------------------------------
// class toped_logfile
//-----------------------------------------------------------------------------
// TOPED log file header
void console::toped_logfile::init(const std::string logFileName, bool append)
{
   const std::string LFH_SEPARATOR = "//==============================================================================";
   const std::string LFH_HEADER    = "//                                TOPED log file";
   const std::string LFH_REVISION  = "//    TOPED revision: ";
   const std::string LFH_ENVIRONM  = "// Current directory: ";
   const std::string LFH_TIMESTAMP = "//   Session started: ";
   const std::string LFH_RECOSTAMP = "// Session recovered: ";

   char *locale=setlocale(LC_ALL, "");
   if (append)
   {
      _file.open(logFileName.c_str(), std::ios::out | std::ios::app);
      TpdTime timec(time(NULL));
      _file << LFH_SEPARATOR << std::endl;
      _file << LFH_RECOSTAMP << timec() << std::endl;
      _file << LFH_SEPARATOR << std::endl;
   }
   else
   {
      _file.open(logFileName.c_str(), std::ios::out);
      TpdTime timec(time(NULL));
      _file << LFH_SEPARATOR << std::endl;
      _file << LFH_HEADER    << std::endl;
      _file << LFH_SEPARATOR << std::endl;
      _file << LFH_REVISION  << "0.9.x" << std::endl;
      _file << LFH_ENVIRONM  << std::string(wxGetCwd().mb_str(wxConvFile )) << std::endl;
      _file << LFH_TIMESTAMP << timec() << std::endl;
      _file << LFH_SEPARATOR << std::endl;
   }
   setlocale(LC_ALL, "English");
}

console::toped_logfile& console::toped_logfile::operator<< (const byte _i)
{
   if (_enabled)
      _file << static_cast<unsigned short>(_i) ;
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const word _i)
{
   if (_enabled)
      _file << _i ;
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const int4b _i)
{
   if (_enabled)
      _file << _i ;
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const real _r)
{
   if (_enabled)
      _file << _r ;
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtPnt& _p)
{
   if (_enabled)
      _file << "{" << _p.x() << "," << _p.y() << "}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const std::string& _s)
{
   if (_enabled)
      _file << _s ;
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtWnd& _w)
{
   if (_enabled)
      _file << "{{" << _w.p1().x() << "," << _w.p1().y() << "}," <<
                "{" << _w.p2().x() << "," << _w.p2().y() << "}}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtBnd& _b)
{
   if (_enabled)
      _file << "{{" << _b.p().x() << "," << _b.p().y() << "}," <<
            _b.rot().value() << "," << (_b.flx().value() ? "true" : "false") << "," <<
            _b.sc().value() << "}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtLMap& _h)
{
   if (_enabled)
      _file << "{" << _h.layer().value() << ",\"" << _h.value().value() << "\"}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtHshStr& _h)
{
   if (_enabled)
      _file << "{\"" << _h.key().value() << "\",\"" << _h.value().value() << "\"}";
   return *this;
}
console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtLayer& _l)
{
   if (_enabled)
      _file << "{" << _l.num() << "," << _l.typ() << "}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::TtList& _tl)
{
   if (_enabled)
   {
      _file << "{";
      for (unsigned i = 0; i < _tl.size(); i++) {
         if (i != 0) _file << ",";
         switch (~telldata::tn_listmask & _tl.get_type()) {
            case telldata::tn_uint:
               _file << static_cast<telldata::TtUInt*>((_tl.mlist())[i])->value();
               break;
            case telldata::tn_int:
               _file << static_cast<telldata::TtInt*>((_tl.mlist())[i])->value();
               break;
            case telldata::tn_real:
               _file << static_cast<telldata::TtReal*>((_tl.mlist())[i])->value();
               break;
            case telldata::tn_bool:
               *this << _2bool(static_cast<telldata::TtBool*>((_tl.mlist())[i])->value());
               break;
            case telldata::tn_string:
               _file << "\"" << static_cast<telldata::TtString*>((_tl.mlist())[i])->value() << "\"";
               break;
            case telldata::tn_pnt:
               *this << *(static_cast<telldata::TtPnt*>((_tl.mlist())[i]));
               break;
            case telldata::tn_box:
               *this << *(static_cast<telldata::TtWnd*>((_tl.mlist())[i]));
               break;
            case telldata::tn_bnd:
               *this << *(static_cast<telldata::TtBnd*>((_tl.mlist())[i]));
               break;
            case telldata::tn_laymap:
               *this << *(static_cast<telldata::TtLMap*>((_tl.mlist())[i]));
               break;
            case telldata::tn_layer:
               *this << *(static_cast<telldata::TtLayer*>((_tl.mlist())[i]));
               break;
            case telldata::tn_hshstr:
               *this << *(static_cast<telldata::TtHshStr*>((_tl.mlist())[i]));
               break;
   //         case tn_layout:
            default:assert(false); break;
         }
      }
      _file << "}";
   }
   return *this;
}
console::toped_logfile& console::toped_logfile::flush()
{
   if (_enabled)
      _file << std::endl;
   return *this;
}

void console::toped_logfile::close()
{
   if (_enabled)
      _file.close();
}

//==============================================================================
telldata::TtCallBack::TtCallBack(typeID ID, parsercmd::cmdCALLBACK* fcbBody) :
   TellVar      ( ID          ),
   _fcbBody     ( fcbBody     ),
   _fBody       ( NULL        ),
   _definition  ( false       )
{}

telldata::TtCallBack::TtCallBack(typeID ID, parsercmd::cmdSTDFUNC* fBody) :
   TellVar      ( ID          ),
   _fcbBody     ( NULL        ),
   _fBody       ( fBody       ),
   _definition  ( true        )
{}

telldata::TtCallBack::TtCallBack(const TtCallBack& cobj) :
   TellVar      ( cobj.get_type()  ),
   _fcbBody     ( cobj._fcbBody    ),
   _fBody       ( cobj._fBody      ),
   _definition  ( cobj._definition )
{}

telldata::TtCallBack::TtCallBack(typeID ID) :
   TellVar      ( ID          ),
   _fcbBody     ( NULL        ),
   _fBody       ( NULL        ),
   _definition  ( false       )
{}

void telldata::TtCallBack::initialize()
{
   // clean-up the function body for the next call
   // - effectively undo what has been done in assign
   _fBody = NULL;
   _fcbBody->unSetFBody();
}

telldata::TellVar* telldata::TtCallBack::selfcopy() const
{
   TtCallBack* nvar = DEBUG_NEW TtCallBack(_ID);
   nvar->_definition = _definition;
   nvar->_fcbBody    = _fcbBody;
   nvar->_fBody      = _fBody;
   return nvar;
}

//void telldata::TtCallBack::echo(std::string& wstr, real)
//{
//   std::ostringstream ost;
//   if ( (NULL == _fcbBody) || (_fcbBody->declaration()) )
//      ost << "NULL";
//   else
//      ost << "pointing to <TODO> function";
//   wstr += ost.str();
//}

void telldata::TtCallBack::assign(TellVar* value)
{
   TtCallBack* n_value = static_cast<telldata::TtCallBack*>(value);

   _definition = n_value->_definition;
   _fBody      = n_value->_fBody;
   if (_definition)
   {
      assert(NULL != _fBody);
      if (NULL != _fcbBody)
         _fcbBody->setFBody(_fBody);
   }
   else
   {
      _fBody = NULL;
      _fcbBody->unSetFBody();
   }
}

//==============================================================================
telldata::TellVar* parsercmd::newCallBackArgument(telldata::typeID ID, TpdYYLtype loc)
{
   if (ID & telldata::tn_listmask)
   {
      return(DEBUG_NEW telldata::TtList(ID));
   }
   else
   switch (ID)
   {
      case   telldata::tn_real  : return(DEBUG_NEW telldata::TtReal());
      case    telldata::tn_int  : return(DEBUG_NEW telldata::TtInt());
      case   telldata::tn_bool  : return(DEBUG_NEW telldata::TtBool());
      case    telldata::tn_pnt  : return(DEBUG_NEW telldata::TtPnt());
      case    telldata::tn_box  : return(DEBUG_NEW telldata::TtWnd());
      case    telldata::tn_bnd  : return(DEBUG_NEW telldata::TtBnd());
      case telldata::tn_laymap  : return(DEBUG_NEW telldata::TtLMap());
      case telldata::tn_hshstr  : return(DEBUG_NEW telldata::TtHshStr());
      case  telldata::tn_layer  : return(DEBUG_NEW telldata::TtLayer());
      case telldata::tn_string  : return(DEBUG_NEW telldata::TtString());
      case telldata::tn_layout  : return(DEBUG_NEW telldata::TtLayout());
      case telldata::tn_auxilary: return(DEBUG_NEW telldata::TtAuxdata());
      default:
      {
         const telldata::TType* utype = CMDBlock->getTypeByID(ID);
         if (NULL == utype)
            tellerror("Bad type specifier", loc);
         else if (utype->isComposite())
            return (DEBUG_NEW telldata::TtUserStruct(static_cast<const telldata::TCompType*>(utype)));
         else // callback variable
            assert(0);
         break;
      }
   }
   return NULL;
}
