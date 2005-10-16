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
//           $URL$
//  Creation date: Wed Dec 26 2001
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision: 1.2 $
//          $Date: 2005-10-16 11:03:08 +0100 (Sun, 16 Oct 2005) $
//        $Author: skr $
//===========================================================================

#include <cmath>
#include <sstream>
#include "tldat.h"

//=============================================================================
void telldata::ttreal::set_value(tell_var* rt) {
   if (rt->get_type() == tn_real)
      _value = static_cast<ttreal*>(rt)->value();
   else if (rt->get_type() == tn_int)
      _value = static_cast<ttint*>(rt)->value();
//   else ERROR Run time error (or Warning) -> unexpected type in ..bla bla
}

void telldata::ttreal::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << value();
   wstr = ost.str();
}

const telldata::ttreal& telldata::ttreal::operator = (const ttreal& a) {
   _value = a.value();
   return *this;
}

const telldata::ttreal& telldata::ttreal::operator = (const ttint& a) {
   _value = a.value();
   return *this;
}
//=============================================================================
void telldata::ttint::set_value(tell_var* rt) {
   _value = static_cast<ttint*>(rt)->value();
}

void telldata::ttint::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << value();
   wstr = ost.str();
}

const telldata::ttint& telldata::ttint::operator = (const ttint& a) {
   _value = a.value();
   return *this;
}

const telldata::ttint& telldata::ttint::operator = (const ttreal& a) {
   // if a.value outside the limits -> send a runtime error message
//   real  rval = a.value();
//   int4b ival = (int4b)rval;
//   _value = 
   _value = (int4b)a.value();
   return *this;
}
//=============================================================================
const telldata::ttbool& telldata::ttbool::operator = (const ttbool& a) {
   _value = a.value();
   return *this;
}

void telldata::ttbool::set_value(tell_var* rt) {
   _value = static_cast<ttbool*>(rt)->value();
}

void telldata::ttbool::echo(std::string& wstr) {
   std::ostringstream ost;
   if (_value) ost << "true";
   else        ost << "false";
   wstr = ost.str();
}

//=============================================================================
void telldata::ttpnt::set_value(tell_var* rt) {
   _x = static_cast<ttpnt*>(rt)->x();
   _y = static_cast<ttpnt*>(rt)->y();
}

void telldata::ttpnt::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << "X = " << x() << ": Y = " << y();
   wstr = ost.str();
}

const telldata::ttpnt& telldata::ttpnt::operator = (const ttpnt& a) {
   _x = a.x();_y = a.y();
   return *this;
}

const telldata::ttpnt& telldata::ttpnt::operator *= (CTM op2) {
   _x = op2.a() * x() + op2.c() * y() + op2.tx();
   _y = op2.b() * x() + op2.d() * y() + op2.ty();
   return *this;
}

const telldata::ttpnt operator*( const telldata::ttpnt &op1, CTM &op2 ) {
   return telldata::ttpnt( op2.a() * op1.x() + op2.c() * op1.y() + op2.tx(),
                op2.b() * op1.x() + op2.d() * op1.y() + op2.ty());
}

const telldata::ttpnt operator-( const telldata::ttpnt &op1, telldata::ttpnt &op2 ) {
   return telldata::ttpnt( op2.x() - op1.x(), op2.y() * op1.y());
}

//=============================================================================
void telldata::ttwnd::set_value(tell_var* rt) {
   _p1 = static_cast<ttwnd*>(rt)->p1();
   _p2 = static_cast<ttwnd*>(rt)->p2();
}

void telldata::ttwnd::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << "P1: X = " << p1().x() << ": Y = " << p1().y() << " ; " <<
          "P2: X = " << p2().x() << ": Y = " << p2().y() ;
   wstr = ost.str();
}

const telldata::ttwnd& telldata::ttwnd::operator = (const ttwnd& a) {
   _p1 = a.p1();_p2 = a.p2();
   return *this;
}

//=============================================================================
const telldata::ttstring& telldata::ttstring::operator = (const ttstring& a) {
   _value = a.value();
   return *this;
}

void telldata::ttstring::set_value(tell_var* value) {
   _value = static_cast<ttstring*>(value)->_value;
}

//=============================================================================
telldata::ttlayout::ttlayout(const ttlayout& cobj) : tell_var(cobj.get_type()) {
   if (NULL != cobj._selp) _selp = new SGBitSet(cobj._selp);
   else _selp = NULL;
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
}

const telldata::ttlayout& telldata::ttlayout::operator = (const ttlayout& cobj) {
   if (_selp) {delete _selp; _selp = NULL;}
   if (NULL != cobj._selp) _selp = new SGBitSet(cobj._selp);
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
   return *this;
}

void telldata::ttlayout::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << "layer " << _layer << " :";
   _data->info(ost);
   if (_selp) ost << " - partially selected";
   wstr = ost.str();
}

void telldata::ttlayout::set_value(tell_var* data) {
   _data = static_cast<ttlayout*>(data)->_data;
   _selp = static_cast<ttlayout*>(data)->_selp;
}

//=============================================================================
telldata::ttlist::ttlist(const telldata::ttlist& cobj) : tell_var(cobj.get_type()) {
   // copy constructor
   unsigned count = cobj._mlist.size();
   _mlist.reserve(count);
   for (unsigned i = 0; i < count; i++) 
     _mlist.push_back(cobj._mlist[i]->selfcopy());
}

const telldata::ttlist& telldata::ttlist::operator =(const telldata::ttlist& cobj) {
//   _ltype = cobj._ltype;
   unsigned count = _mlist.size();
   for (unsigned i = 0; i < count; i++) 
      delete _mlist[i];
   _mlist.clear();   
   count = cobj._mlist.size();
   _mlist.reserve(count);
   for (unsigned i = 0; i < count; i++) 
      _mlist.push_back(cobj._mlist[i]->selfcopy());
   return *this;   
}

void telldata::ttlist::echo(std::string& wstr) {
   std::ostringstream ost;
   if (_mlist.empty()) {ost << "empty list";}
   else {
      ost << "list members:";
      for (unsigned i = 0; i < _mlist.size(); i++) {
         (_mlist[i])->echo(wstr);
         ost  <<"\n" << "[" << i << "]" << " : " << wstr;
      }
   }
   wstr = ost.str();
}

void telldata::ttlist::set_value(tell_var* rt) {
   _mlist = static_cast<ttlist*>(rt)->mlist();
}

telldata::ttlist::~ttlist() {
   for (unsigned long i = 0 ; i < _mlist.size(); i++)
      delete _mlist[i];
}

telldata::tell_var* telldata::tell_type::initfield(const typeID ID) const {
   telldata::tell_var* nvar;
   if (ID & telldata::tn_listmask) nvar = new telldata::ttlist(ID & ~telldata::tn_listmask);
   else 
      switch(ID & ~telldata::tn_listmask) {
         case tn_void  : assert(false);
         case tn_int   : nvar = new telldata::ttint()    ;break;
         case tn_real  : nvar = new telldata::ttreal()   ;break;
         case tn_bool  : nvar = new telldata::ttbool()   ;break;
         case tn_string: nvar = new telldata::ttstring() ;break;
         case tn_pnt   : nvar = new telldata::ttpnt()    ;break;
         case tn_box   : nvar = new telldata::ttwnd()    ;break;
         case tn_layout: nvar = new telldata::ttlayout() ;break;
                default: {
                     assert(_tIDMAP.end() != _tIDMAP.find(ID));
                     nvar = new telldata::user_struct(_tIDMAP.find(ID)->second);
                }
               // the default is effectively nvar = new telldata::user_struct(_tIDMAP[ID]),
               // but it is not not keeping constness
      }
   return nvar;
}

bool telldata::tell_type::addfield(std::string fname, typeID fID, const tell_type* utype) {
   if (_fields.end() == _fields.find(fname)) {
      _fields[fname] = fID;
      if (NULL != utype) _tIDMAP[fID] = utype;
      return true;
   }
   else return false;
}

telldata::user_struct::user_struct(const tell_type* tltypedef) : tell_var(tltypedef->ID()) {
   const recfieldsMAP& typefields = tltypedef->fields();
   for (recfieldsMAP::const_iterator CI = typefields.begin(); CI != typefields.end(); CI++)
      _fieldmap[CI->first] = tltypedef->initfield(CI->second);
}

telldata::user_struct::user_struct(const user_struct& cobj) : tell_var(cobj.get_type()) {
   for (variableMAP::const_iterator CI = cobj._fieldmap.begin(); CI != cobj._fieldmap.end(); CI++)
      _fieldmap[CI->first] = CI->second->selfcopy();
}

void telldata::user_struct::echo(std::string&) {
// I'm lazyyyyyy
}

void telldata::user_struct::set_value(tell_var* value) {
   user_struct* n_value = static_cast<telldata::user_struct*>(value);
   for (variableMAP::const_iterator CI = _fieldmap.begin(); CI != _fieldmap.end(); CI++) {
      // assert the field is existing
      assert(_fieldmap.end() != n_value->_fieldmap.find(CI->first));
      // and assign the new value
      _fieldmap[CI->first]->set_value(n_value->_fieldmap[CI->first]);
   }
}

