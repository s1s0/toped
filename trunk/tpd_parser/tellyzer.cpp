// //===========================================================================
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
#include "../tpd_common/outbox.h"

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
// Declared here to avoid "undefined symbol" errors reported by 
// ldd -r for tpd_parser.so
// According to wxWidgets (app.h)
// this macro can be used multiple times and just allows you to use wxGetApp()
// function
//DECLARE_APP(TopedApp)

parsercmd::cmdBLOCK*       CMDBlock = NULL;
console::toped_logfile     LogFile;
//-----------------------------------------------------------------------------
// Initialize some static members
//-----------------------------------------------------------------------------
// Table of defined functions
parsercmd::functionMAP        parsercmd::cmdBLOCK::_funcMAP;
// Table of current nested blocks
parsercmd::blockSTACK         parsercmd::cmdBLOCK::_blocks;
// Operand stack
telldata::operandSTACK        parsercmd::cmdVIRTUAL::OPstack;
// UNDO Operand stack
telldata::UNDOPerandQUEUE     parsercmd::cmdVIRTUAL::UNDOPstack;
// UNDO command queue
parsercmd::undoQUEUE          parsercmd::cmdVIRTUAL::UNDOcmdQ;

bool parsercmd::cmdSTDFUNC::_ignoreOnRecovery = false;



real parsercmd::cmdVIRTUAL::getOpValue(telldata::operandSTACK& OPs)
{
   real value = 0;
   telldata::tell_var *op = OPs.top();OPs.pop();
   if (op->get_type() == telldata::tn_real) 
      value = static_cast<telldata::ttreal*>(op)->value();
   else if (op->get_type() == telldata::tn_int) 
      value = static_cast<telldata::ttint*>(op)->value();
   delete op;
   return value;
}

real parsercmd::cmdVIRTUAL::getOpValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   real value = 0;
   telldata::tell_var *op;
   if (front) {op = OPs.front();OPs.pop_front();}
   else       {op = OPs.back();OPs.pop_back();}
   if (op->get_type() == telldata::tn_real) 
      value = static_cast<telldata::ttreal*>(op)->value();
   else if (op->get_type() == telldata::tn_int) 
      value = static_cast<telldata::ttint*>(op)->value();
   delete op;
   return value;
}

word parsercmd::cmdVIRTUAL::getWordValue(telldata::operandSTACK& OPs)
{
   telldata::ttint  *op = static_cast<telldata::ttint*>(OPs.top());OPs.pop();
   word value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_WORD_VALUE))
      {
         //@TODO
         /*Error -> value out of expected range*/
      }
   else value = word(op->value());
   delete op;
   return value;
}

word parsercmd::cmdVIRTUAL::getWordValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   telldata::ttint  *op;
   if (front) {op = static_cast<telldata::ttint*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttint*>(OPs.back());OPs.pop_back();} 
   word value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_WORD_VALUE))
      {
         //@TODO
         /*Error -> value out of expected range*/
      }
   else value = word(op->value());
   delete op;
   return value;
}

_dbl_word parsercmd::cmdVIRTUAL::getIndexValue(telldata::operandSTACK& OPs)
{
   _dbl_word value = 0;
   telldata::tell_var *op = OPs.top();OPs.pop();
   if (op->get_type() == telldata::tn_real)
   {
      real realvalue = static_cast<telldata::ttreal*>(op)->value();
      if ((realvalue < 0) || ((realvalue - int(realvalue)) != 0.0 ) )
      {
         _opstackerr = true;
      }
      else
         value = (_dbl_word) rint(realvalue);
   }
   else if (op->get_type() == telldata::tn_int)
   {
      int4b intvalue = (int4b) rint(static_cast<telldata::ttint*>(op)->value());
      if (intvalue < 0)
      {
         _opstackerr = true;
      }
      else
         value = intvalue;
   }
   delete op;
   return value;
}

byte parsercmd::cmdVIRTUAL::getByteValue(telldata::operandSTACK& OPs)
{
   telldata::ttint  *op = static_cast<telldata::ttint*>(OPs.top());OPs.pop();
   byte value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_BYTE_VALUE))
   {
      //@TODO
      /*Error -> value out of expected range*/
   }
   else value = byte(op->value());
   delete op;
   return value;
}

byte parsercmd::cmdVIRTUAL::getByteValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   telldata::ttint  *op;
   if (front) {op = static_cast<telldata::ttint*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttint*>(OPs.back());OPs.pop_back();}
   byte value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_BYTE_VALUE))
   {
      //@TODO
      /*Error -> value out of expected range*/
   }
   else value = byte(op->value());
   delete op;
   return value;
}

std::string parsercmd::cmdVIRTUAL::getStringValue(telldata::operandSTACK& OPs)
{
   telldata::ttstring  *op = static_cast<telldata::ttstring*>(OPs.top());OPs.pop();
   std::string value = op->value();
   delete op;
   return value;
}

std::string parsercmd::cmdVIRTUAL::getStringValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   telldata::ttstring  *op;
   if (front) {op = static_cast<telldata::ttstring*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttstring*>(OPs.back());OPs.pop_back();}
   std::string value = op->value();
   delete op;
   return value;
}

bool parsercmd::cmdVIRTUAL::getBoolValue(telldata::operandSTACK& OPs)
{
   telldata::ttbool  *op = static_cast<telldata::ttbool*>(OPs.top());OPs.pop();
   bool value = op->value();
   delete op;
   return value;
}

bool parsercmd::cmdVIRTUAL::getBoolValue(telldata::UNDOPerandQUEUE& OPs, bool front)
{
   telldata::ttbool  *op;
   if (front) {op = static_cast<telldata::ttbool*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttbool*>(OPs.back());OPs.pop_back();}
   bool value = op->value();
   delete op;
   return value;
}
//=============================================================================
int parsercmd::cmdPLUS::execute() {
   TELL_DEBUG(cmdPLUS);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttreal(value1 + value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdMINUS::execute() {
   TELL_DEBUG(cmdMINUS);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttreal(value1 - value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT::execute() {
   TELL_DEBUG(cmdSHIFTPNT);
   real shift = getOpValue();
   telldata::ttpnt  *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt* r = DEBUG_NEW telldata::ttpnt(p->x()+_sign*shift,p->y()+_sign*shift);
   delete p; 
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT2::execute() {
   TELL_DEBUG(cmdSHIFTPNT2);
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p  = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *r  = DEBUG_NEW telldata::ttpnt(p->x()+_sign*p1->x(),p->y()+_sign*p1->y());
   delete p; delete p1;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT3::execute() {
   TELL_DEBUG(cmdSHIFTPNT3);
   real shift = getOpValue();
   telldata::ttpnt *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *r = DEBUG_NEW telldata::ttpnt(p->x()+_signX*shift,p->y()+_signY*shift);
   delete p; 
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT4::execute() {
   TELL_DEBUG(cmdSHIFTPNT4);
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p  = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt* r = DEBUG_NEW telldata::ttpnt(p->x()+_signX*p1->x(),p->y()+_signY*p1->y());
   delete p; delete p1;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX::execute() {
   TELL_DEBUG(cmdSHIFTBOX);
   telldata::ttpnt *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   telldata::ttwnd* r = DEBUG_NEW telldata::ttwnd(w->p1().x() + _sign*p->x(),w->p1().y() + _sign*p->y(),
                        w->p2().x() + _sign*p->x(),w->p2().y() + _sign*p->y());
   OPstack.push(r);
   delete p; delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX3::execute() {
   TELL_DEBUG(cmdSHIFTBOX3);
   real shift = getOpValue();
//   telldata::ttpnt *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   bool swapx, swapy;
   w->normalize(swapx, swapy);
   telldata::ttwnd* r;
   if  (1 == _signX) 
      if (1 == _signY)
         r = DEBUG_NEW telldata::ttwnd(w->p1().x()          , w->p1().y()         ,
                                 w->p2().x() + shift  , w->p2().y() + shift  );
      else
         r = DEBUG_NEW telldata::ttwnd(w->p1().x()          , w->p1().y() - shift ,
                                 w->p2().x() + shift  , w->p2().y()          );
   else 
      if (1 == _signY)
         r = DEBUG_NEW telldata::ttwnd(w->p1().x() - shift  , w->p1().y()          ,
                                 w->p2().x()          , w->p2().y() + shift   );
      else
         r = DEBUG_NEW telldata::ttwnd(w->p1().x() - shift  , w->p1().y() - shift  ,
                                 w->p2().x()          , w->p2().y()           );
   r->denormalize(swapx, swapy);
   OPstack.push(r);
   delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX4::execute() {
   TELL_DEBUG(cmdSHIFTBOX4);
   telldata::ttpnt *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   bool swapx, swapy;
   w->normalize(swapx, swapy);
   telldata::ttwnd* r;
   if  (1 == _signX) 
      if (1 == _signY)
         r = DEBUG_NEW telldata::ttwnd(w->p1().x()          , w->p1().y()         ,
                                 w->p2().x() + p->x() , w->p2().y() + p->y() );
      else
         r = DEBUG_NEW telldata::ttwnd(w->p1().x()          , w->p1().y() - p->y(),
                                 w->p2().x() + p->x() , w->p2().y()          );
   else 
      if (1 == _signY)
         r = DEBUG_NEW telldata::ttwnd(w->p1().x() - p->x() , w->p1().y()          ,
                                 w->p2().x()          , w->p2().y() + p->y()  );
      else
         r = DEBUG_NEW telldata::ttwnd(w->p1().x() - p->x() , w->p1().y() - p->y() ,
                                 w->p2().x()          , w->p2().y()           );
   r->denormalize(swapx, swapy);
   OPstack.push(r);
   delete p; delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdMULTIPLY::execute() {
   TELL_DEBUG(cmdMULTIPLY);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttreal(value1 * value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdDIVISION::execute() {
   TELL_DEBUG(cmdDIVISION);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttreal(value1 / value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLT::execute() {
   TELL_DEBUG(cmdLT);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttbool(value1 < value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLET::execute() {
   TELL_DEBUG(cmdLET);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttbool(value1 <= value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdGT::execute() {
   TELL_DEBUG(cmdGT);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttbool(value1 > value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdGET::execute() {
   TELL_DEBUG(cmdGET);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(DEBUG_NEW telldata::ttbool(value1 >= value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdEQ::execute() {
   TELL_DEBUG(cmdEQ);
   if (NUMBER_TYPE(OPstack.top()->get_type()))
      OPstack.push(DEBUG_NEW telldata::ttbool(getOpValue() == getOpValue()));
//   else if (tn_
// box & poly equal
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdNE::execute() {
   TELL_DEBUG(cmdNE);
   if (NUMBER_TYPE(OPstack.top()->get_type()))
      OPstack.push(DEBUG_NEW telldata::ttbool(getOpValue() != getOpValue()));
//   else if (tn_
// box & poly equal
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdAND::execute() {
   TELL_DEBUG(cmdAND);
   telldata::ttbool *op = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
   static_cast<telldata::ttbool*>(OPstack.top())->AND(op->value());
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWAND::execute() {
   TELL_DEBUG(cmdBWAND);
   word op1 = getWordValue();
   word op2 = getWordValue();
   OPstack.push(DEBUG_NEW telldata::ttint(op1 & op2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdOR::execute() {
   TELL_DEBUG(cmdOR);
   telldata::ttbool *op = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
   static_cast<telldata::ttbool*>(OPstack.top())->OR(op->value());
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWOR::execute() {
   TELL_DEBUG(cmdBWAND);
   word op1 = getWordValue();
   word op2 = getWordValue();
   OPstack.push(DEBUG_NEW telldata::ttint(op1 | op2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdNOT::execute() {
   TELL_DEBUG(cmdNOT);
   static_cast<telldata::ttbool*>(OPstack.top())->NOT();
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdBWNOT::execute() {
   TELL_DEBUG(cmdNOT);
   static_cast<telldata::ttint*>(OPstack.top())->NOT();
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdUMINUS::execute() {
   TELL_DEBUG(cmdUMINUS);
   if      (_type == telldata::tn_real) static_cast<telldata::ttreal*>(OPstack.top())->uminus();
   else if (_type == telldata::tn_int ) static_cast<telldata::ttint*>(OPstack.top())->uminus();
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSTACKRST::execute() {
   TELL_DEBUG(cmdSTACKRST);
   while (!OPstack.empty()) {
      delete OPstack.top(); OPstack.pop();
   }   
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdASSIGN::execute()
{
   TELL_DEBUG(cmdREALASSIGN);
   telldata::tell_var *op = OPstack.top();OPstack.pop();
   telldata::typeID typeis = _var->get_type();
   if (TLISALIST(typeis))
   {
      typeis = typeis & ~telldata::tn_listmask;
   }
   if ((TLCOMPOSIT_TYPE(typeis)) && (NULL == CMDBlock->getTypeByID(typeis)))
      tellerror("Bad or unsupported type in assign statement");
   else
   {
      if (_indexed)
      {
         _dbl_word idx = getIndexValue();
         _var = static_cast<telldata::ttlist*>(_var)->index_var(idx);
         if ((NULL != _var) && (!_opstackerr))
         {
            _var->assign(op); OPstack.push(_var->selfcopy());
         }
         else
         {
            tellerror("Runtime error.Invalid Index");
            delete op;
            return EXEC_ABORT;
         }
      }
      else
      {
         _var->assign(op); OPstack.push(_var->selfcopy());
      }
   }
   delete op;
   return EXEC_NEXT;

}

//=============================================================================
int parsercmd::cmdLISTADD::execute()
{
   TELL_DEBUG(cmdLISTADD);

   telldata::tell_var *op = OPstack.top();OPstack.pop();
   telldata::typeID typeis = _listarg->get_type();
   if (TLISALIST(typeis))
   {
      typeis = typeis & ~telldata::tn_listmask;
   }
   if ((TLCOMPOSIT_TYPE(typeis)) && (NULL == CMDBlock->getTypeByID(typeis)))
      tellerror("Bad or unsupported type in assign statement");
   else
   {
      _dbl_word idx = getIndex();
      if ((!_opstackerr) && (_empty_list) && (0 == idx))
      {
         _listarg->insert(op);
      }
      else if ((!_opstackerr) && (_listarg->validIndex(idx)))
      {
         if (!_prefix) idx++;
         _listarg->insert(op, idx);
      }
      else
      {
         tellerror("Runtime error.Invalid index");
         return EXEC_ABORT;
      }
   }
   delete op;
   OPstack.push(_listarg->selfcopy());
   return EXEC_NEXT;
}


_dbl_word parsercmd::cmdLISTADD::getIndex()
{
   _dbl_word idx;
   _empty_list = (0 == _listarg->size());
   // find the index
   if      (((!_index) && ( _prefix)) || _empty_list) // first in the list
      idx = 0;
   else if ((!_index) && (!_prefix)) // last in the list
   {
      idx = _listarg->size();
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
   telldata::ttlist *op = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   telldata::typeID typeis = _listarg->get_type() & ~telldata::tn_listmask;

   if ((TLCOMPOSIT_TYPE(typeis)) && (NULL == CMDBlock->getTypeByID(typeis)))
      tellerror("Bad or unsupported type in list union statement");
   else
   {
      _dbl_word idx = getIndex();
      if ((!_opstackerr) && (_empty_list) && (0 == idx))
      {
         _listarg->lunion(op);
      }
      else if ((!_opstackerr) && (_listarg->validIndex(idx)))
      {
         if (!_prefix) idx++;
         _listarg->lunion(op,idx);
      }
      else
      {
         tellerror("Runtime error.Invalid Index");
         return EXEC_ABORT;
      }
   }
   delete op;
   OPstack.push(_listarg->selfcopy());
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLISTSUB::execute()
{
   TELL_DEBUG(cmdLISTSUB);
   _dbl_word idx;
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
      tellerror("Runtime error.Invalid index");
      return EXEC_ABORT;
   }
}

//=============================================================================
int parsercmd::cmdLISTSLICE::execute()
{
   TELL_DEBUG(cmdLISTSLICE);
   _dbl_word idxB, idxE, size;
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
      tellerror("Runtime error.Invalid index");
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
   if (!_indexed)
   {
     OPstack.push(_var->selfcopy());
     return EXEC_NEXT;
   }
   else
   {
      // another class cmdLISTINDEX could be appropriate instead of the logical
      // branch below. It looks to me that it is quite the same as above, apart
      // from the index checks
      _dbl_word idx = getIndexValue();
      telldata::tell_var *listcomp = static_cast<telldata::ttlist*>(_var)->index_var(idx);
      if ((NULL != listcomp) && (!_opstackerr))
      {
         OPstack.push(listcomp->selfcopy());
         return EXEC_NEXT;
      }
      else
      {
         tellerror("Runtime error.Invalid index");
         return EXEC_ABORT;
      }
   }
}

telldata::tell_var* parsercmd::cmdSTRUCT::getList() {
   telldata::typeID comptype = (*_arg)() & ~telldata::tn_listmask;
   telldata::ttlist *pl = DEBUG_NEW telldata::ttlist(comptype);
   unsigned llength = _arg->child().size();
   pl->reserve(llength);
   telldata::tell_var  *p;
   for (unsigned i = 0; i < llength; i++) {
      p = OPstack.top();OPstack.pop();
      pl->add(p); //Dont delete p; here! And don't get confused!
   }
   pl->reverse();
   return pl;
}

int parsercmd::cmdSTRUCT::execute()
{
   TELL_DEBUG(cmdSTRUCT);
   if (NULL == _arg)
   {
      tellerror("Stucture arguments not evaluated properly. Internal parser error");
      return EXEC_RETURN;
   }
   telldata::tell_var *ustrct;
   if (TLISALIST( (*_arg)() )) ustrct = getList();
   else 
   {
      switch( (*_arg)() ) 
      {
         case telldata::tn_pnt: ustrct = DEBUG_NEW telldata::ttpnt(OPstack);break;
         case telldata::tn_box: ustrct = DEBUG_NEW telldata::ttwnd(OPstack);break;
         case telldata::tn_bnd: ustrct = DEBUG_NEW telldata::ttbnd(OPstack);break;
         case telldata::tn_hsh: ustrct = DEBUG_NEW telldata::tthsh(OPstack);break;
         default:ustrct = DEBUG_NEW telldata::user_struct(CMDBlock->getTypeByID( (*_arg)() ), OPstack);
      }
   }
   OPstack.push(ustrct);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdFUNCCALL::execute()
{
   TELL_DEBUG(cmdFUNC);
   int fresult;
   if (funcbody->ignoreOnRecovery() && !funcbody->execOnRecovery())
   {
      std::string info = funcname + " ignored";
      tell_log(console::MT_INFO, info);
      return EXEC_NEXT;
   }
   if (funcbody->declaration())
   {
      std::string info = "Link error. Function " + funcname + "() not defined";
      tell_log(console::MT_ERROR, info);
      return EXEC_ABORT;
   }
   LogFile.setFN(funcname);
   try {fresult = funcbody->execute();}
   catch (EXPTN) {return EXEC_ABORT;}
   funcbody->reduce_undo_stack();
   return fresult;
}

//=============================================================================
bool parsercmd::cmdRETURN::checkRetype(telldata::argumentID* arg) {
   if (NULL == arg) return (_retype == telldata::tn_void);

   if (TLUNKNOWN_TYPE((*arg)())) {
      const telldata::tell_type* vartype;
      if (TLISALIST(_retype)) { // we have a list lval
          vartype = CMDBlock->getTypeByID(_retype & ~telldata::tn_listmask);
          if (NULL != vartype) arg->userStructListCheck(*vartype, true);
          else arg->toList(true);
      }
      else { // we have a struct only
         vartype = CMDBlock->getTypeByID(_retype);
         if (NULL != vartype) arg->userStructCheck(*vartype, true);
      }
   }
   return ((_retype == (*arg)()) || (NUMBER_TYPE(_retype) && NUMBER_TYPE((*arg)())));
}
//=============================================================================
parsercmd::cmdBLOCK::cmdBLOCK() {
   assert(!_blocks.empty());
   _next_lcl_typeID = _blocks.front()->_next_lcl_typeID;
   TYPElocal.clear();
   VARlocal.clear();
}

telldata::tell_var* parsercmd::cmdBLOCK::getID(char*& name, bool local){
   TELL_DEBUG(***getID***);
   // Roll back the blockSTACK until name is found. return NULL otherwise
   typedef blockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = local ? _blocks.begin() : _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++) {
        if ((*cmd)->VARlocal.find(name) != (*cmd)->VARlocal.end())
            return (*cmd)->VARlocal[name];
   }
   return NULL;
}

void parsercmd::cmdBLOCK::addID(const char* name, telldata::tell_var* var) {
   TELL_DEBUG(addID);
   VARlocal[name] = var;
}

void parsercmd::cmdBLOCK::addconstID(const char* name, telldata::tell_var* var, bool initialized) {
   TELL_DEBUG(addID);
   VARlocal[name] = var;
   var->const_declaration();
   if (initialized) var->update_cstat();
}

void parsercmd::cmdBLOCK::addlocaltype(const char* ttypename, telldata::tell_type* ntype) {
   assert(TYPElocal.end() == TYPElocal.find(ttypename));
   _next_lcl_typeID = ntype->ID() + 1;
   TYPElocal[ttypename] = ntype;
}

telldata::tell_type* parsercmd::cmdBLOCK::requesttypeID(char*& ttypename) {
   if (TYPElocal.end() == TYPElocal.find(ttypename)) {
      telldata::tell_type* ntype = DEBUG_NEW telldata::tell_type(_next_lcl_typeID);
      return ntype;
   }
   else return NULL;
}

const telldata::tell_type* parsercmd::cmdBLOCK::getTypeByName(char*& ttypename) const {
   TELL_DEBUG(***gettypeID***);
   // Roll back the blockSTACK until name is found. return NULL otherwise
   typedef blockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++) {
        if ((*cmd)->TYPElocal.end() != (*cmd)->TYPElocal.find(ttypename))
            return (*cmd)->TYPElocal[ttypename];
   }
   return NULL;
}

const telldata::tell_type* parsercmd::cmdBLOCK::getTypeByID(const telldata::typeID ID) const {
   TELL_DEBUG(***getTypeByID***);
   // Roll back the blockSTACK until name is found. return NULL otherwise
   typedef blockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   typedef telldata::typeMAP::const_iterator CT;
   for (BS cmd = blkstart; cmd != blkend; cmd++) 
      for (CT ctp = (*cmd)->TYPElocal.begin(); ctp != (*cmd)->TYPElocal.end(); ctp++)
         if (ID == ctp->second->ID()) return ctp->second;
   return NULL;
}

telldata::tell_var* parsercmd::cmdBLOCK::newTellvar(telldata::typeID ID, TpdYYLtype loc)
{
   if (ID & telldata::tn_listmask) 
      return(DEBUG_NEW telldata::ttlist(ID));
   else
   switch (ID)
   {
      case   telldata::tn_real: return(DEBUG_NEW telldata::ttreal());
      case    telldata::tn_int: return(DEBUG_NEW telldata::ttint());
      case   telldata::tn_bool: return(DEBUG_NEW telldata::ttbool());
      case    telldata::tn_pnt: return(DEBUG_NEW telldata::ttpnt());
      case    telldata::tn_box: return(DEBUG_NEW telldata::ttwnd());
      case    telldata::tn_bnd: return(DEBUG_NEW telldata::ttbnd());
      case    telldata::tn_hsh: return(DEBUG_NEW telldata::tthsh());
      case telldata::tn_string: return(DEBUG_NEW telldata::ttstring());
      case telldata::tn_layout: return(DEBUG_NEW telldata::ttlayout());
      default:
      {
         const telldata::tell_type* utype = getTypeByID(ID);
         if (NULL == utype) 
            tellerror("Bad type specifier", loc);
         else 
            return (DEBUG_NEW telldata::user_struct(utype));
      }
   }
   return NULL;
}

parsercmd::cmdBLOCK* parsercmd::cmdBLOCK::popblk() {
   TELL_DEBUG(cmdBLOCK_popblk);
   _blocks.pop_front();
   return _blocks.front();;
}

void parsercmd::cmdBLOCK::addFUNC(std::string, cmdSTDFUNC* cQ) {
   TELL_DEBUG(addFUNC);
   tellerror("Nested function definitions are not allowed");
   if (cQ)    delete cQ;
}

void parsercmd::cmdBLOCK::addUSERFUNC(FuncDeclaration*, cmdFUNC*, TpdYYLtype) {
   TELL_DEBUG(addFUNC);
   tellerror("Nested function definitions are not allowed");
}

void parsercmd::cmdBLOCK::addUSERFUNCDECL(FuncDeclaration*, TpdYYLtype) {
   TELL_DEBUG(addFUNCDECL);
   tellerror("Function definitions can be only global");
}

int parsercmd::cmdBLOCK::execute() {
   TELL_DEBUG(cmdBLOCK_execute);
   int retexec = EXEC_NEXT; // to secure an empty block
   for (cmdQUEUE::const_iterator cmd = cmdQ.begin(); cmd != cmdQ.end(); cmd++) {
      if ((retexec = (*cmd)->execute())) break;
   }
   return retexec;
}

parsercmd::cmdBLOCK* parsercmd::cmdBLOCK::cleaner() {
   TELL_DEBUG(cmdBLOCK_cleaner);
   while (!cmdQ.empty()) {
      cmdVIRTUAL *a = cmdQ.front();cmdQ.pop_front();
      delete a;
   }
   if (_blocks.size() > 1)
   {
      parsercmd::cmdBLOCK* dblk = _blocks.front(); _blocks.pop_front();
      delete dblk;
      return _blocks.front();
   }
   else return this;
}

parsercmd::cmdBLOCK::~cmdBLOCK() {
   for (cmdQUEUE::iterator CMDI = cmdQ.begin(); CMDI != cmdQ.end(); CMDI++)
      delete *CMDI;
   cmdQ.clear();
   for (telldata::variableMAP::iterator VMI = VARlocal.begin(); VMI != VARlocal.end(); VMI++)
      delete VMI->second;
   VARlocal.clear();
   for (telldata::typeMAP::iterator TMI = TYPElocal.begin(); TMI != TYPElocal.end(); TMI++)
      delete TMI->second;
   TYPElocal.clear();

}

void parsercmd::cmdBLOCK::copyContents( cmdFUNC* cQ )
{
   for (cmdQUEUE::const_iterator cmd = cmdQ.begin(); cmd != cmdQ.end(); cmd++)
      cQ->pushcmd(*cmd);

   cmdQ.clear();

   for (telldata::variableMAP::iterator VMI = VARlocal.begin(); VMI != VARlocal.end(); VMI++)
      cQ->addID(VMI->first.c_str(), VMI->second);

   VARlocal.clear();

   for (telldata::typeMAP::iterator TMI = TYPElocal.begin(); TMI != TYPElocal.end(); TMI++)
      cQ->addlocaltype(TMI->first.c_str(), TMI->second);

   TYPElocal.clear();
}

telldata::variableMAP* parsercmd::cmdBLOCK::copyVarLocal()
{
   telldata::variableMAP* varmap = DEBUG_NEW telldata::variableMAP();
   for (telldata::variableMAP::iterator VMI = VARlocal.begin(); VMI != VARlocal.end(); VMI++)
      (*varmap)[VMI->first.c_str()] = VMI->second->selfcopy();
   return varmap;
}

void parsercmd::cmdBLOCK::restoreVarLocal(telldata::variableMAP& nvars)
{
   typedef telldata::variableMAP::iterator VMIT;
   for (VMIT VMI = VARlocal.begin(); VMI != VARlocal.end(); VMI++)
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
   for (VMIT VMI = VARlocal.begin(); VMI != VARlocal.end(); VMI++)
   {
      VMI->second->initialize();
   }
}
//=============================================================================
parsercmd::cmdSTDFUNC* const parsercmd::cmdBLOCK::getFuncBody
                                        (char*& fn, telldata::argumentQ* amap) const {
   cmdSTDFUNC *fbody = NULL;
   typedef functionMAP::iterator MM;
   std::pair<MM,MM> range = _funcMAP.equal_range(fn);
   telldata::argumentQ* arguMap = (NULL == amap) ? DEBUG_NEW telldata::argumentQ : amap;
   for (MM fb = range.first; fb != range.second; fb++) {
      fbody = fb->second;
      if (0 == fbody->argsOK(arguMap)) break;
      else fbody = NULL;
   }
   if (NULL == amap) delete arguMap;
   return fbody;
}

bool  parsercmd::cmdBLOCK::defValidate(const std::string& fn, const argumentLIST* alst, cmdFUNC*& funcdef)
{
   // convert argumentLIST to argumentMAP
   telldata::argumentQ arguMap;
   typedef argumentLIST::const_iterator AT;
   for (AT arg = alst->begin(); arg != alst->end(); arg++)
      arguMap.push_back(DEBUG_NEW telldata::argumentID((*arg)->second->get_type()));
   // get the function definitions with this name
   typedef functionMAP::iterator MM;
   std::pair<MM,MM> range = _funcMAP.equal_range(fn);
   bool allow_definition = true;
   for (MM fb = range.first; fb != range.second; fb++)
   {
      if (0 == fb->second->argsOK(&arguMap))
      {// if function with this name and parameter list is already defined
         if (fb->second->internal())
         {
            // can't redefine internal function
            allow_definition = false;
            break;
         }
         else
         {
            if (!fb->second->declaration())
            {
               std::ostringstream ost;
               ost << "Warning! User function \""<< fn <<"\" is redefined";
               tell_log(console::MT_WARNING, ost.str());
               delete (fb->second);
               _funcMAP.erase(fb);
            }
            else
               funcdef = static_cast<cmdFUNC*>(fb->second);
            break;
         }
      }
   }
   telldata::argQClear(&arguMap);
   return allow_definition;
}

bool  parsercmd::cmdBLOCK::declValidate(const std::string& fn, const argumentLIST* alst, TpdYYLtype loc)
{
   // convert argumentLIST to argumentMAP
   telldata::argumentQ arguMap;
   typedef argumentLIST::const_iterator AT;
   for (AT arg = alst->begin(); arg != alst->end(); arg++)
      arguMap.push_back(DEBUG_NEW telldata::argumentID((*arg)->second->get_type()));
   // get the function definitions with this name
   typedef functionMAP::iterator MM;
   std::pair<MM,MM> range = _funcMAP.equal_range(fn);
   bool allow_declaration = true;
   for (MM fb = range.first; fb != range.second; fb++)
   {
      if (0 == fb->second->argsOK(&arguMap))
      {// if function with this name and parameter list is already defined
         std::ostringstream ost;
         ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
         if (fb->second->internal())
         {
            ost << "Can't redeclare internal function \"" << fn << "\"";
            tell_log(console::MT_ERROR, ost.str());
            allow_declaration = false;
            break;
         }
         else if (!fb->second->declaration())
         {
            ost << "Function \"" << fn << "\" already defined. Declaration ignored";
            tell_log(console::MT_WARNING, ost.str());
            allow_declaration = false;
            break;
         }
         else
         {
            ost << "Function \"" << fn << "\" already declared. Declaration ignored";
            tell_log(console::MT_WARNING, ost.str());
            allow_declaration = false;
            break;
         }
      }
   }
   telldata::argQClear(&arguMap);
   return allow_declaration;
}

//=============================================================================
int parsercmd::cmdSTDFUNC::argsOK(telldata::argumentQ* amap) 
{
// This function is rather twisted, but this seems the only way to deal with
// anonimous user defined structures handled over as input function arguments.
// Otherwise we have to restrict significantly the input arguments rules for
// functions. Here is the problem.
// Functoins in tell can be overloaded. In the same time we can have user defined
// structures, that have coincidental fields - for example the fields of the point
// structure coincides with the fields of an user structure defined as
// struct sameAsPoint{real a, real z}
// And on top of this we can have two overloaded functions, that have as a first
// argument a variables of type point and sameAsPoint respectively. The problem
// comes when the function is called with anonymous arguments (not with variables),
// which type can not be determined without the type of the function parameter.
// Here is the idea
// 1. If an unknown type appears in the argument list
//  a) Create a copy of the argument using argumentID copy constructor
//  b) Check that the DEBUG_NEW argument matches the type of the function parameter and
//     if so:
//     - assign (adjust) the type of the argument to the type of the parameter
//     - push the argument in the temporary structure
//  c) If the argument doesn't match, bail-out, but don't forget to clean-up the
//     the copies of the previously checked arguments
// 2. When the entire argument list is checked and it matches the corresponding
//    function parameter types, use the saved list of adjusted arguments to readjust
//    the original user defined argument types, which will be used to execute
//    properly the cmdSTRUCT commands already pushed into the command stack during
//    the bison parsing
// There is one remaining problem here. It is still possible to have two or even more
// overloaded functions defined with effectively the same parameter list. In this case,
// when that function is called with anonymous argument(s) the tellyzer will invoke the
// first function body that matches the entire list of input arguments. This will be most
// likely undefined. To prevent this we need beter checks during the function definition
// parsing
   unsigned i = amap->size();
   if (i != arguments->size()) return -1;
   telldata::argumentQ UnknownArgsCopy;
   // :) - some fun here, but it might be confusing - '--' postfix operation is executed
   // always after the comparison, but before the cycle body. So. if all the arguments
   // are checked (match), the cycle ends-up with i == -1;
   while (i-- > 0) 
   {
      telldata::typeID cargID = (*(*amap)[i])();
      telldata::argumentID carg((*(*amap)[i]));
      telldata::typeID lvalID = (*arguments)[i]->second->get_type();
      if (TLUNKNOWN_TYPE(cargID)) 
      {
         const telldata::tell_type* vartype;
         if (TLISALIST(lvalID)) 
         { // we have a list lval
            vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
            if (NULL != vartype) carg.userStructListCheck(*vartype, false);
            else carg.toList(false);
         }
         else 
         { // we have a struct only
            vartype = CMDBlock->getTypeByID(lvalID);
            if (NULL != vartype) carg.userStructCheck(*vartype, false);
         }
      }

      if (!NUMBER_TYPE( carg() )) 
      {  // for non-number types there is no internal conversion,
         // so check strictly the type
         if ( carg() != lvalID) 
            break;
         else if (TLUNKNOWN_TYPE( (*(*amap)[i])() ))
            UnknownArgsCopy.push_back(DEBUG_NEW telldata::argumentID(carg));
      }
      else 
      {  // for number types - allow compatablity
         if ((!NUMBER_TYPE(lvalID)) || ( carg() > lvalID)) 
            break;
         else if (TLUNKNOWN_TYPE( (*(*amap)[i])() ))
            UnknownArgsCopy.push_back(DEBUG_NEW telldata::argumentID(carg));
      }
   }
   i++;
   if (UnknownArgsCopy.size() > 0)
   {
      if (i > 0)
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
   return (i);
}

void parsercmd::cmdSTDFUNC::reduce_undo_stack() {
   if (UNDOcmdQ.size() > 10) {
      UNDOcmdQ.back()->undo_cleanup(); UNDOcmdQ.pop_back();
   }
}

nameList* parsercmd::cmdSTDFUNC::callingConv(const telldata::typeMAP* lclTypeDef)
{
   nameList* argtypes = DEBUG_NEW nameList();
   argtypes->push_back(telldata::echoType(gettype(), lclTypeDef));
   int argnum = arguments->size();
   for (int i = 0; i != argnum; i++)
      argtypes->push_back(telldata::echoType((*arguments)[i]->second->get_type(), lclTypeDef));
   return argtypes;
}

parsercmd::cmdSTDFUNC::~cmdSTDFUNC() {
   ClearArgumentList(arguments);
   delete arguments;
}   

//=============================================================================
parsercmd::cmdFUNC::cmdFUNC(argumentLIST* vm, telldata::typeID tt, bool declaration):
                        cmdSTDFUNC(vm,tt,true), cmdBLOCK(), _declaration(declaration)
{
   _recursyLevel = 0;
   if (!_declaration)
   {
      // copy the arguments in the structure of the local variables
      typedef argumentLIST::const_iterator AT;
      for (AT arg = arguments->begin(); arg != arguments->end(); arg++) 
      {
         VARlocal[(*arg)->first] = (*arg)->second->selfcopy();
      }
   }
}

int parsercmd::cmdFUNC::execute()
{
   if (_recursyLevel > 0)  _VARLocalStack.push(copyVarLocal());
   _recursyLevel++;
   // get the arguments from the operands stack and replace the values
   // of the function arguments
   int i = arguments->size();
   while (i-- > 0)
   {
      //get the argument name
      std::string   argname = (*arguments)[i]->first;
      // get the tell variable (by name)
      telldata::tell_var* argvar = VARlocal[argname];
      // get a value from the operand stack
      telldata::tell_var* argval = OPstack.top();
      // replace the value of the local variable with the argument value
      argvar->assign(argval);
      delete argval;OPstack.pop();
   }
   std::string funcname = LogFile.getFN();
   LogFile << "// >> Entering UDF \"" << funcname << "\" .Recursy level:" << _recursyLevel;
   LogFile.flush();
   int retexec = cmdBLOCK::execute();
   LogFile << "// << Exiting  UDF \"" << funcname << "\" .Recursy level:" << _recursyLevel;
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
      // we must reinitialize local variables - otherwise we'll get unexpected
      // results - for example local lists are not empty!
      initializeVarLocal();
   }
   if (EXEC_ABORT == retexec) return retexec;
   else return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdIFELSE::execute() {
   TELL_DEBUG(cmdIFELSE);
   int retexec = EXEC_NEXT;
   telldata::ttbool *cond = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
   if (cond->value())   retexec =  trueblock->execute();
   else if (falseblock) retexec = falseblock->execute();
   delete cond;
   return retexec;
}

//=============================================================================
int parsercmd::cmdWHILE::execute() {
   TELL_DEBUG(cmdWHILE);
   int retexec = EXEC_NEXT;
   telldata::ttbool *cond;
   bool    condvalue;
   while (true) {
      condblock->execute();
      cond = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
      condvalue = cond->value(); delete cond;
      if (condvalue)    retexec = body->execute();
      else              return retexec;
      if (EXEC_NEXT != retexec) return retexec;
   }
}

//=============================================================================
int parsercmd::cmdREPEAT::execute() {
   TELL_DEBUG(cmdREPEAT);
   int retexec;
   telldata::ttbool *cond;
   bool    condvalue;
   while (true) {
      retexec = body->execute();
      if (EXEC_NEXT != retexec) return retexec;
      condblock->execute();
      cond = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
      condvalue = cond->value(); delete cond;
      if (!condvalue)           return retexec;
   }
}

//=============================================================================
int parsercmd::cmdFOREACH::execute() {
   TELL_DEBUG(cmdFOREACH);
   int retexec;

   _header->execute();
   telldata::ttlist* clist = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   telldata::memlist valist = clist->mlist();

   for (telldata::memlist::const_iterator CI = valist.begin(); CI != valist.end(); CI++)
   {
      _var->assign(*CI);
      retexec = _body->execute();
      if (EXEC_NEXT != retexec) break;
   }
   delete clist;
   return retexec;
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
int parsercmd::cmdMAIN::execute()
{
   TELL_DEBUG(cmdMAIN_execute);
   int retexec = EXEC_NEXT;
   while (!cmdQ.empty())
   {
      cmdVIRTUAL *a = cmdQ.front();cmdQ.pop_front();
      if (EXEC_NEXT == retexec) retexec = a->execute();
      delete a;
   }
   return retexec;
}

void parsercmd::cmdMAIN::addFUNC(std::string fname , cmdSTDFUNC* cQ)
{
   _funcMAP.insert(std::make_pair(fname,cQ));
   console::TellFnAdd(fname, cQ->callingConv(NULL));
}

/*! 
*/
void parsercmd::cmdMAIN::addUSERFUNC(FuncDeclaration* decl, cmdFUNC* cQ, TpdYYLtype loc)
{
   cmdFUNC* declfunc = NULL;
   if ((telldata::tn_void != decl->type()) && (0 == decl->numReturns())) {
      tellerror("function must return a value", loc);
   }
   else  if (decl->numErrors() > 0) {
      tellerror("function definition is ignored because of the errors above", loc);
   }
   /*Check whether such a function is already defined */
   else if ( CMDBlock->defValidate(decl->name().c_str(), decl->argList(), declfunc) )
   {
      if (declfunc)
      {// pour over the definition contents in the body created by the declaration
         cQ->copyContents(declfunc);
         declfunc->set_defined();
         console::TellFnAdd(decl->name(), cQ->callingConv(&TYPElocal));
         console::TellFnSort();
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
empty and cmdFUNC._declarationit will be true. 
*/
void parsercmd::cmdMAIN::addUSERFUNCDECL(FuncDeclaration* decl, TpdYYLtype loc)
{
   if (CMDBlock->declValidate(decl->name().c_str(),decl->argList(),loc))
   {
      cmdSTDFUNC* cQ = DEBUG_NEW parsercmd::cmdFUNC(decl->argListCopy(),decl->type(), true);
      _funcMAP.insert(std::make_pair(decl->name(), cQ));
   }
}

parsercmd::cmdMAIN::cmdMAIN():cmdBLOCK(telldata::tn_usertypes) {
   pushblk();
};

void  parsercmd::cmdMAIN::addGlobalType(char* ttypename, telldata::tell_type* ntype) {
   assert(TYPElocal.end() == TYPElocal.find(ttypename));
   TYPElocal[ttypename] = ntype;
}

void parsercmd::cmdMAIN::recoveryDone()
{
   cmdSTDFUNC::_ignoreOnRecovery = false;
}

parsercmd::cmdMAIN::~cmdMAIN(){
   while (UNDOcmdQ.size() > 0)
   {
      UNDOcmdQ.back()->undo_cleanup();UNDOcmdQ.pop_back();
   }
   for (functionMAP::iterator FMI = _funcMAP.begin(); FMI != _funcMAP.end(); FMI ++)
      delete FMI->second;
   _funcMAP.clear();
};

//=============================================================================
telldata::typeID parsercmd::UMinus(telldata::typeID op1, TpdYYLtype loc1) {
   if (NUMBER_TYPE(op1)) {
      CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdUMINUS(op1));
      return op1;}
   else {
      tellerror("unexepected operand type",loc1);
      return telldata::tn_void;
   }
}

//=============================================================================
//     +/-    |real |point| box |
//------------+-----+-----+-----+
//   real     |  x  |  -  |  -  |
//   point    |shift|shift|  -  |
//   box      |blow |shift| or  |
//---------------------------------------
telldata::typeID parsercmd::Plus(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2) {
   switch (op1)   {
      case  telldata::tn_int: 
      case telldata::tn_real:
         switch(op2) {
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdPLUS());
                            return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT());
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT2());
                           return telldata::tn_pnt;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case telldata::tn_real: // inflate
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX());
                        return telldata::tn_box;
            case telldata::tn_box:    // or
//            case tn_poly: // or
                  default: tellerror("unexepected operand type",loc2); break;
         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::Minus(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2) {
   switch (op1)   {
       case telldata::tn_int:
      case telldata::tn_real:
         switch(op2) {
            case   telldata::tn_int:
            case  telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMINUS());
                            return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(-1));
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT2(-1));
                           return telldata::tn_pnt;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case telldata::tn_real: // deflate
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX(-1));
                        return telldata::tn_box;
            case telldata::tn_box:    // or not ???
            //case tn_ttPoly:
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::PointMv(telldata::typeID op1, telldata::typeID op2,
                                   TpdYYLtype loc1, TpdYYLtype loc2, int xdir, int ydir) {
   switch (op1)   {
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT3(xdir,ydir));
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT4(xdir,ydir));
                           return telldata::tn_pnt;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX3(xdir,ydir));
                        return telldata::tn_box;
            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX4(xdir,ydir));
                        return telldata::tn_box;
                  default: tellerror("unexepected operand type",loc2); break;
         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

//=============================================================================
//     *//    |real |point| box |
//------------+-----+-----+-----+
//   real     |  x  |  -  |  -  |
//   point    |scale| rota|  -  |
//   box      |scale| rota| and |
//-------------------------------
telldata::typeID parsercmd::Multiply(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2) {
   switch (op1)   {
       case telldata::tn_int:
      case telldata::tn_real:
         switch(op2) {
              case telldata::tn_int:
             case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdMULTIPLY());
                         return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
//      case telldata::tn_pnt:
//         switch(op2) {
//            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(-1));
//                           return telldata::tn_pnt;
//            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT2(-1));
//                           return telldata::tn_pnt;
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
//      case telldata::tn_box:
//         switch(op2) {
//            case telldata::tn_real: // deflate
//            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX(-1));
//                        return telldata::tn_box;
//            case telldata::tn_box:    // or not ???
//            //case tn_ttPoly:
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::Divide(telldata::typeID op1, telldata::typeID op2,
                                                  TpdYYLtype loc1, TpdYYLtype loc2) {
   switch (op1)   {
       case telldata::tn_int:
      case telldata::tn_real:
         switch(op2) {
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdDIVISION());
                         return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
//      case telldata::tn_pnt:
//         switch(op2) {
//            case telldata::tn_real:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT(-1));
//                           return telldata::tn_pnt;
//            case    telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTPNT2(-1));
//                           return telldata::tn_pnt;
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
//      case telldata::tn_box:
//         switch(op2) {
//            case telldata::tn_real: // deflate
//            case telldata::tn_pnt:CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdSHIFTBOX(-1));
//                        return telldata::tn_box;
//            case telldata::tn_box:    // or not ???
//            //case tn_ttPoly:
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

bool parsercmd::StructTypeCheck(telldata::typeID targett, 
                                      telldata::argumentID* op2, TpdYYLtype loc)
{
   VERIFY(TLUNKNOWN_TYPE((*op2)()));
   const telldata::tell_type* vartype;
   if (TLISALIST(targett))
   { // we have a list lval
      vartype = CMDBlock->getTypeByID(targett & ~telldata::tn_listmask);
      if (NULL != vartype) op2->userStructListCheck(*vartype, true);
      else op2->toList(true);
   }
   else
   { // we have a struct only
      vartype = CMDBlock->getTypeByID(targett);
      if (NULL != vartype) op2->userStructCheck(*vartype, true);
   }
   return (targett == (*op2)());
}

bool parsercmd::ListIndexCheck(telldata::typeID list, TpdYYLtype lloc,
                               telldata::typeID idx, TpdYYLtype iloc)
{
   bool checkval = false;
   if       (!(list & telldata::tn_listmask))
      tellerror("list expected",lloc);
   else if  ((idx != telldata::tn_int) && (idx != telldata::tn_real))
      tellerror("index is expected to be a number",iloc);
   else
      checkval = true;
   return checkval;
}

bool parsercmd::ListSliceCheck(telldata::typeID list, TpdYYLtype lloc,
   telldata::typeID idx, TpdYYLtype iloc, telldata::typeID sidx, TpdYYLtype sloc)
{
   if  ((sidx != telldata::tn_int) && (sidx != telldata::tn_real))
   {
      tellerror("slice size is expected to be a number",iloc);
      return false;
   }
   else
      return ListIndexCheck(list, lloc, idx, iloc);
}

bool parsercmd::ListSliceCheck(telldata::typeID list, TpdYYLtype lloc,
                                    telldata::typeID sidx, TpdYYLtype sloc)
{
   if  ((sidx != telldata::tn_int) && (sidx != telldata::tn_real))
   {
      tellerror("slice size is expected to be a number",sloc);
      return false;
   }
   else if (!(list & telldata::tn_listmask))
   {
      tellerror("list expected",lloc);
      return false;
   }
   return true;
}

telldata::typeID parsercmd::Assign(telldata::tell_var* lval, bool indexed, telldata::argumentID* op2,
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
   if (indexed)
   {
      //A slight complication here - when a list component is an lvalue - then we're going
      //to cheat a little and remove the list flag from the type ID. That's because the entire
      //list is handled over as lval, instead of the component itself. That's because the
      //lists are dynamic and the target component will be retrieved during runtime
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
      const telldata::tell_type* vartype;
      if (TLISALIST(lvalID))
      { // we have a list lval
          vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
          if (NULL != vartype) op2->userStructListCheck(*vartype, true);
          else op2->toList(true);
      }
      else
      { // we have a struct only
         vartype = CMDBlock->getTypeByID(lvalID);
         if (NULL != vartype) op2->userStructCheck(*vartype, true);
      }
   }
   if ((lvalID == (*op2)()) || (NUMBER_TYPE(lvalID) && NUMBER_TYPE((*op2)())))
   {
      CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdASSIGN(lval, indexed));
      // don't forget to update cmdSTRUCT (if this is the rval) with the
      // validated argumentID
//      if (NULL != op2->command())
//         static_cast<cmdSTRUCT*>(op2->command())->setargID(op2);
      return lvalID;
   }
   else
   {
      tellerror("Incompatable operand types in assignment", loc);
      return telldata::tn_void;
   }
}

telldata::typeID parsercmd::Uninsert(telldata::tell_var* lval, telldata::argumentID* op2,
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
   // union operation whith the same syntax. It should benefit from the
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
   // whether the operands are compartible and what is the operator -
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
   // Here if user structure is used - clarify that it is compatible
   // The thing is that op2 could be a struct of a struct list or a list of
   // tell basic types. This should be checked in the following order:
   // 1. Get the type of the recipient (lval)
   //    It must be a list
   //    a) strip the list atribute and get the type of the list component
   //    b) if the type of the lval list component is compound (struct list), check the
   //       input structure for struct list
   //    c) if the type of the list component is basic, check directly that
   //       op2 is a list
   if (TLUNKNOWN_TYPE((*op2)()))
   {
      if (TLISALIST(lvalID))
      { // we have a list lval
         const telldata::tell_type* vartype;
          vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
          if (NULL != vartype) op2->userStructListCheck(*vartype, true);
          else op2->toList(true);
      }
      else
         // lvalue in this function must be a list
         assert(TLISALIST(lvalID));
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
         tellerror("Incompatable operand types in list add/insert", loc);
         return telldata::tn_void;
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
//   std::ostringstream ost;
//   ost << "bad or unsuported pair of operands in " << ope << "operator";
//   if ((op1 != op2) and (((op1 != telldata::tn_real) and (op1 != telldata::tn_bool)) or
//                         ((op2 != telldata::tn_real) and (op2 != telldata::tn_bool))))  {
   
   if (NUMBER_TYPE(op1) && NUMBER_TYPE(op2))
   {
      if      (ope == "<" ) CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdLT());
      else if (ope == "<=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdLET());
      else if (ope ==  ">") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdGT());
      else if (ope == ">=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdGET());
      else if (ope == "==") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdEQ());
      else if (ope == "!=") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdNE());
      else if ((telldata::tn_int == op1) && (telldata::tn_int == op2))
      {
         if      (ope == "&") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWAND());
         else if (ope == "|") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWOR());
         else
         {
            tellerror("unexepected operand type",loc1);
            return telldata::tn_void;
         }
         return telldata::tn_int;
      }
      else
      {
         tellerror("unexepected operand type",loc1);
         return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else if ((telldata::tn_bool == op1) && (telldata::tn_bool == op2))
   {
      if      (ope == "&&") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdAND());
      else if (ope == "||") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdOR());
      else
      {
         tellerror("unexepected operand type",loc1);return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else
   {
      tellerror("unexepected operand type",loc2);
      return telldata::tn_void;
   }
}

telldata::typeID parsercmd::BoolEx(telldata::typeID op1, std::string ope, TpdYYLtype loc1)
{
   if      (telldata::tn_int == op1)
   {
      if      (ope == "~" ) CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdBWNOT());
      else
      {
         tellerror("unexepected operand type",loc1);return telldata::tn_void;
      }
      return telldata::tn_int;
   }
   else if (telldata::tn_bool == op1)
   {
      if      (ope == "!") CMDBlock->pushcmd(DEBUG_NEW parsercmd::cmdNOT());
      else
      {
         tellerror("unexepected operand type",loc1);return telldata::tn_void;
      }
      return telldata::tn_bool;
   }
   else
   {
      tellerror("unexepected operand type",loc1);
      return telldata::tn_void;
   }
}

void parsercmd::ClearArgumentList(argumentLIST* alst) {
   if (NULL == alst) return;
   for (argumentLIST::iterator ALI = alst->begin(); ALI != alst->end(); ALI++) {
      delete (*ALI)->second;
      delete (*ALI);
   }
   alst->clear();
}

//-----------------------------------------------------------------------------
// class FuncDeclaration
//-----------------------------------------------------------------------------
parsercmd::argumentLIST* parsercmd::FuncDeclaration::argListCopy() const
{
   parsercmd::argumentLIST* arglist = DEBUG_NEW parsercmd::argumentLIST;
   typedef argumentLIST::const_iterator AT;
   for (AT arg = _argList->begin(); arg != _argList->end(); arg++)
      arglist->push_back(DEBUG_NEW parsercmd::argumentTYPE((*arg)->first,(*arg)->second->selfcopy()));
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
#define LFH_SEPARATOR "//=============================================================================="
#define LFH_HEADER    "//                                TOPED log file"
#define LFH_REVISION  "//    TOPED revision: "
#define LFH_ENVIRONM  "// Current directory: "
#define LFH_TIMESTAMP "//   Session started: "
#define LFH_RECOSTAMP "// Session recovered: "

void console::toped_logfile::init(const std::string logFileName, bool append)
{
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
      _file << LFH_ENVIRONM  << std::string(wxGetCwd().mb_str()) << std::endl;
      _file << LFH_TIMESTAMP << timec() << std::endl;
      _file << LFH_SEPARATOR << std::endl;
   }
	setlocale(LC_ALL, "English");
}

console::toped_logfile& console::toped_logfile::operator<< (const byte _i) {
   _file << static_cast<unsigned short>(_i) ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const word _i) {
   _file << _i ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const int4b _i) {
   _file << _i ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const real _r) {
   _file << _r ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttpnt& _p) {
   _file << "{" << _p.x() << "," << _p.y() << "}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const std::string& _s) {
   _file << _s ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttwnd& _w) {
   _file << "{{" << _w.p1().x() << "," << _w.p1().y() << "}," <<
         "{" << _w.p2().x() << "," << _w.p2().y() << "}}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttbnd& _b) {
   _file << "{{" << _b.p().x() << "," << _b.p().y() << "}," <<
         _b.rot().value() << "," << (_b.flx().value() ? "true" : "false") << "," <<
         _b.sc().value() << "}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::tthsh& _h) {
   _file << "{" << _h.number().value() << "," << _h.name().value() << "}";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttlist& _tl) {
   _file << "{";
   for (unsigned i = 0; i < _tl.size(); i++) {
      if (i != 0) _file << ",";
      switch (~telldata::tn_listmask & _tl.get_type()) {
         case telldata::tn_int:
            _file << static_cast<telldata::ttint*>((_tl.mlist())[i])->value();
            break;
         case telldata::tn_real:
            _file << static_cast<telldata::ttreal*>((_tl.mlist())[i])->value();
            break;
         case telldata::tn_bool:
            *this << _2bool(static_cast<telldata::ttbool*>((_tl.mlist())[i])->value());
            break;
         case telldata::tn_string:
            _file << static_cast<telldata::ttstring*>((_tl.mlist())[i])->value();
            break;
         case telldata::tn_pnt:
            *this << *(static_cast<telldata::ttpnt*>((_tl.mlist())[i]));
            break;
         case telldata::tn_box:
            *this << *(static_cast<telldata::ttwnd*>((_tl.mlist())[i]));
            break;
         case telldata::tn_bnd:
            *this << *(static_cast<telldata::ttbnd*>((_tl.mlist())[i]));
            break;
         case telldata::tn_hsh:
            *this << *(static_cast<telldata::tthsh*>((_tl.mlist())[i]));
            break;
//         case tn_layout:
            default:{assert(false);}
      }
   }
   _file << "}";
   return *this;
}
console::toped_logfile& console::toped_logfile::flush() {
   _file << std::endl; return *this;
}
 
void console::toped_logfile::close()
{
   _file.close();
}

