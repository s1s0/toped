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
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <cmath>
#include <sstream>
#include "tldat.h"
#include "tellyzer.h"

//=============================================================================
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
                     // the default is effectively nvar = new telldata::user_struct(_tIDMAP[ID]),
                     // but it is not not keeping constness
                }
      }
   return nvar;
}

bool telldata::tell_type::addfield(std::string fname, typeID fID, const tell_type* utype) {
   // search for a field with this name
   for (recfieldsID::const_iterator CF = _fields.begin(); CF != _fields.end(); CF++) {
      if (CF->first == fname) return false;
   }
   _fields.push_back(structRECID(fname,fID));
    if (NULL != utype) _tIDMAP[fID] = utype;
    return true;
}

const telldata::tell_type* telldata::tell_type::findtype(const typeID basetype) const
{
   assert(_tIDMAP.end() != _tIDMAP.find(basetype));
   return _tIDMAP.find(basetype)->second;
}

//=============================================================================
telldata::point_type::point_type() : tell_type(telldata::tn_pnt) {
   addfield("x", telldata::tn_real, NULL);
   addfield("y", telldata::tn_real, NULL);
};

//=============================================================================
telldata::box_type::box_type(point_type* pfld) : tell_type(telldata::tn_box) {
   addfield("p1", telldata::tn_pnt, pfld);
   addfield("p2", telldata::tn_pnt, pfld);
};

//=============================================================================
void telldata::ttreal::assign(tell_var* rt) {
   if (rt->get_type() == tn_real)
      _value = static_cast<ttreal*>(rt)->value();
   else if (rt->get_type() == tn_int)
      _value = static_cast<ttint*>(rt)->value();
//   else ERROR Run time error (or Warning) -> unexpected type in ..bla bla
}

void telldata::ttreal::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << value();
   wstr += ost.str();
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
void telldata::ttint::assign(tell_var* rt) {
   if (rt->get_type() == tn_real)
      _value = static_cast<ttreal*>(rt)->value();
   else if (rt->get_type() == tn_int)
      _value = static_cast<ttint*>(rt)->value();
}

void telldata::ttint::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << value();
   wstr += ost.str();
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

void telldata::ttbool::assign(tell_var* rt) {
   _value = static_cast<ttbool*>(rt)->value();
}

void telldata::ttbool::echo(std::string& wstr) {
   if (_value) wstr += "true";
   else        wstr += "false";
}

//=============================================================================
const telldata::ttstring& telldata::ttstring::operator = (const ttstring& a) {
   _value = a.value();
   return *this;
}

void telldata::ttstring::assign(tell_var* value) {
   _value = static_cast<ttstring*>(value)->_value;
}

void telldata::ttstring::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << "\"" << _value << "\"";
   wstr += ost.str();
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
   wstr += ost.str();
}

void telldata::ttlayout::assign(tell_var* data) {
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
   unsigned count = _mlist.size();
   unsigned i;
   for (i = 0; i < count; i++) 
      delete _mlist[i];
   _mlist.clear();   
   count = cobj._mlist.size();
   _mlist.reserve(count);
   for (i = 0; i < count; i++) 
      _mlist.push_back(cobj._mlist[i]->selfcopy());
   return *this;
}

void telldata::ttlist::echo(std::string& wstr) {
   std::ostringstream ost;
   if (_mlist.empty()) {wstr += "empty list";}
   else {
      wstr += " list members: { ";
      for (unsigned i = 0; i < _mlist.size(); i++) {
         if (i > 0)  wstr += " , ";
         (_mlist[i])->echo(wstr);
      }
      wstr += " } ";
   }
}

void telldata::ttlist::assign(tell_var* rt) {
   this->operator = (*(static_cast<ttlist*>(rt)));
}

telldata::ttlist::~ttlist() {
   for (unsigned long i = 0 ; i < _mlist.size(); i++)
      delete _mlist[i];
}

//=============================================================================
telldata::user_struct::user_struct(const tell_type* tltypedef) : tell_var(tltypedef->ID()) {
   const recfieldsID& typefields = tltypedef->fields();
   for (recfieldsID::const_iterator CI = typefields.begin(); CI != typefields.end(); CI++)
      _fieldList.push_back(structRECNAME(CI->first,tltypedef->initfield(CI->second)));
}

telldata::user_struct::user_struct(const tell_type* tltypedef, operandSTACK& OPstack) :
                                                                tell_var(tltypedef->ID()) 
{
   assert(NULL != tltypedef);
   const recfieldsID& typefields = tltypedef->fields();
   for (recfieldsID::const_reverse_iterator CI = typefields.rbegin(); CI != typefields.rend(); CI++) 
   {// for every member of the structure
      assert(OPstack.top()->get_type() == CI->second);
      _fieldList.push_back(structRECNAME(CI->first,OPstack.top()->selfcopy()));
      delete(OPstack.top());OPstack.pop();
   }
}


telldata::user_struct::user_struct(const user_struct& cobj) : tell_var(cobj.get_type()) {
   for (recfieldsNAME::const_iterator CI = cobj._fieldList.begin(); CI != cobj._fieldList.end(); CI++)
      _fieldList.push_back(structRECNAME(CI->first, CI->second->selfcopy()));
}

telldata::user_struct::~user_struct() {
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++)
      delete CI->second;
}

void telldata::user_struct::echo(std::string& wstr) {
   wstr += "struct members:\n";
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++) {
      wstr += CI->first; wstr += ": "; CI->second->echo(wstr);
      wstr += "\n";
   }
}

void telldata::user_struct::assign(tell_var* value) {
   user_struct* n_value = static_cast<telldata::user_struct*>(value);
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++) {
      // find the corresponding field in n_value and get the tell_var
      tell_var* fieldvar = NULL;
      for(recfieldsNAME::const_iterator CIV  = n_value->_fieldList.begin();
                                        CIV != n_value->_fieldList.end(); CIV++) {
         if (CI->first == CIV->first) {
            fieldvar = CIV->second;
            break;
         }
      }
      assert(NULL != fieldvar);
      CI->second->assign(fieldvar);
   }
}

telldata::tell_var* telldata::user_struct::field_var(char*& fname) {
   std::string fieldName(fname); fieldName.erase(0,1);

   for(recfieldsNAME::const_iterator CI  = _fieldList.begin();
                                     CI != _fieldList.end(); CI++) {
         if (fieldName == CI->first) return CI->second;
   }
   return NULL;
}

//=============================================================================
telldata::ttpnt::ttpnt (real x, real y) : user_struct(telldata::tn_pnt),
                                         _x(new ttreal(x)), _y(new ttreal(y)) {
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

telldata::ttpnt::ttpnt(operandSTACK& OPstack) : user_struct(telldata::tn_pnt) 
{
   _y = new telldata::ttreal(); _y->assign(OPstack.top());
   delete OPstack.top(); OPstack.pop();
   _x = new telldata::ttreal(); _x->assign(OPstack.top());
   delete OPstack.top(); OPstack.pop();
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

telldata::ttpnt::ttpnt(const ttpnt& invar) : user_struct(telldata::tn_pnt) ,
                         _x(new ttreal(invar.x())), _y(new ttreal(invar.y())) {
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

void telldata::ttpnt::assign(tell_var* rt) {
   _x->_value = static_cast<ttpnt*>(rt)->x();
   _y->_value = static_cast<ttpnt*>(rt)->y();
}

void telldata::ttpnt::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << "{X = " << x() << ", Y = " << y() << "}";
   wstr += ost.str();
}

const telldata::ttpnt& telldata::ttpnt::operator = (const ttpnt& a) {
   _x->_value = a.x(); _y->_value = a.y();
   return *this;
}

//=============================================================================
telldata::ttwnd::ttwnd( real bl_x, real bl_y, real tr_x, real tr_y ) :
                                                       user_struct(tn_box),
                                       _p1(new telldata::ttpnt(bl_x,bl_y)),
                                       _p2(new telldata::ttpnt(tr_x,tr_y)) {
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}

telldata::ttwnd::ttwnd( ttpnt bl, ttpnt tr ) : user_struct(tn_box),
                 _p1(new telldata::ttpnt(bl)), _p2(new telldata::ttpnt(tr)) {
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}

telldata::ttwnd::ttwnd(const ttwnd& cobj) : user_struct(tn_box),
    _p1(new telldata::ttpnt(cobj.p1())), _p2(new telldata::ttpnt(cobj.p2())) {
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}


telldata::ttwnd::ttwnd(operandSTACK& OPstack) : user_struct(telldata::tn_box) 
{
   // Here - just get the pointer to the points in the stack...
    _p2 = static_cast<telldata::ttpnt*>(OPstack.top());
    // .. that's why - don't delete them. The alternative -
    // - to make a selfcopy and then to delete the original from the OPstack
    OPstack.pop();
    _p1 = static_cast<telldata::ttpnt*>(OPstack.top());
    OPstack.pop();
    _fieldList.push_back(structRECNAME("p1", _p1));
    _fieldList.push_back(structRECNAME("p2", _p2));
}

void telldata::ttwnd::assign(tell_var* rt) {
   (*_p1) = static_cast<ttwnd*>(rt)->p1();
   (*_p2) = static_cast<ttwnd*>(rt)->p2();
}

void telldata::ttwnd::echo(std::string& wstr) {
   std::ostringstream ost;
   ost << "P1: X = " << p1().x() << ": Y = " << p1().y() << " ; " <<
          "P2: X = " << p2().x() << ": Y = " << p2().y() ;
   wstr += ost.str();
}

const telldata::ttwnd& telldata::ttwnd::operator = (const ttwnd& a) {
   (*_p1) = a.p1(); (*_p2) = a.p2();
   return *this;
}

//=============================================================================
telldata::argumentID::argumentID(const argumentID& obj2copy) {
   _ID = obj2copy();
   _command = obj2copy._command;
   if (0 < obj2copy.child().size())
   {
      for(argumentQ::const_iterator CA = obj2copy.child().begin(); CA != obj2copy.child().end(); CA ++)
         _child.push_back(new argumentID(**CA));
   }
}

void telldata::argumentID::toList(bool cmdUpdate)
{
   telldata::typeID alistID = (*(_child[0]))();
   for(argumentQ::const_iterator CA = _child.begin(); CA != _child.end(); CA ++)
   {
      if (alistID != (**CA)()) return;
   }
   _ID = TLISTOF(alistID);
   if (cmdUpdate)
      static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
}

void telldata::argumentID::userStructCheck(const telldata::tell_type& vartype, bool cmdUpdate)
{
   const telldata::recfieldsID& recfields = vartype.fields();
   // first check that both lists have the same size
   if (_child.size() != recfields.size()) return;
   recfieldsID::const_iterator CF;
   argumentQ::iterator CA;
   for (CF = recfields.begin(), CA = _child.begin();
             (CF != recfields.end() && CA != _child.end()); CF ++, CA++)
   {
      if ( TLUNKNOWN_TYPE( (**CA)() ) )
         if (TLISALIST(CF->second))
         {// check the list fields
            if (TLCOMPOSIT_TYPE((CF->second) & ~telldata::tn_listmask))
               (*CA)->userStructListCheck(*(vartype.findtype(CF->second)), cmdUpdate);
            else
               (*CA)->toList(cmdUpdate);
         }
         else
            // call in recursion the userStructCheck method of the child
            (*CA)->userStructCheck(*(vartype.findtype(CF->second)), cmdUpdate);
      if (!NUMBER_TYPE( CF->second ))
      {
         // for non-number types there is no internal conversion,
         // so check strictly the type
         if ( (**CA)() != CF->second) return; // no match
      }
      else // for number types - allow compatablity (int to real only)
         if (!NUMBER_TYPE( (**CA)() )) return; // no match
         else if (CF->second < (**CA)() ) return; // no match
   }
   // all fields match => we can assign a known ID to the argumentID
   _ID = vartype.ID();
   if (cmdUpdate)
      static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
}

void telldata::argumentID::userStructListCheck(const telldata::tell_type& vartype, bool cmdUpdate) 
{
   for (argumentQ::iterator CA = _child.begin(); CA != _child.end(); CA++) 
      if ( TLUNKNOWN_TYPE( (**CA)() ) ) (*CA)->userStructCheck(vartype, cmdUpdate);

   toList(cmdUpdate);
}

void telldata::argumentID::adjustID(const argumentID& obj2copy)
{
   if (0 != obj2copy.child().size())
   {
      assert(obj2copy.child().size() == _child.size());
      argumentQ::const_iterator CB;
      argumentQ::iterator CA;
      for(CA = _child.begin(), CB = obj2copy.child().begin() ;
                                                      CA != _child.end() ; CA ++, CB++)
         if (TLUNKNOWN_TYPE((**CA)())) (*CA)->adjustID(**CB);
   }
      _ID = obj2copy._ID;
      static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
}

telldata::argumentID::~argumentID()
{
   for (argumentQ::iterator CA = _child.begin(); CA != _child.end(); CA++)
      delete (*CA);
   _child.clear();
}

void telldata::argQClear(argumentQ* queue)
{
   for (argumentQ::iterator CA = queue->begin(); CA != queue->end(); CA++)
      delete (*CA);
   queue->clear();
}