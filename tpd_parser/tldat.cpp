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
//        Created: Wed Dec 26 2001
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TELL data types
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <cmath>
#include <sstream>
#include "tldat.h"
#include "tellyzer.h"

//=============================================================================
telldata::tell_var* telldata::tell_type::initfield(const typeID ID) const {
   telldata::tell_var* nvar;
   if (ID & telldata::tn_listmask) nvar = DEBUG_NEW telldata::ttlist(ID & ~telldata::tn_listmask);
   else 
      switch(ID & ~telldata::tn_listmask) {
         case tn_void  : assert(false);
         case tn_int   : nvar = DEBUG_NEW telldata::ttint()    ;break;
         case tn_real  : nvar = DEBUG_NEW telldata::ttreal()   ;break;
         case tn_bool  : nvar = DEBUG_NEW telldata::ttbool()   ;break;
         case tn_string: nvar = DEBUG_NEW telldata::ttstring() ;break;
         case tn_pnt   : nvar = DEBUG_NEW telldata::ttpnt()    ;break;
         case tn_box   : nvar = DEBUG_NEW telldata::ttwnd()    ;break;
         case tn_bnd   : nvar = DEBUG_NEW telldata::ttbnd()    ;break;
         case tn_layout: nvar = DEBUG_NEW telldata::ttlayout() ;break;
                default: {
                     assert(_tIDMAP.end() != _tIDMAP.find(ID));
                     nvar = DEBUG_NEW telldata::user_struct(_tIDMAP.find(ID)->second);
                     // the default is effectively nvar = DEBUG_NEW telldata::user_struct(_tIDMAP[ID]),
                     // but it is not keeping constness
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
telldata::point_type::point_type() : tell_type(telldata::tn_pnt)
{
   addfield("x", telldata::tn_real, NULL);
   addfield("y", telldata::tn_real, NULL);
};

//=============================================================================
telldata::box_type::box_type(point_type* pfld) : tell_type(telldata::tn_box)
{
   addfield("p1", telldata::tn_pnt, pfld);
   addfield("p2", telldata::tn_pnt, pfld);
};

//=============================================================================
telldata::bnd_type::bnd_type(point_type* pfld) : tell_type(telldata::tn_bnd)
{
   addfield("p"   , telldata::tn_pnt , pfld);
   addfield("rot" , telldata::tn_real, NULL);
   addfield("flx" , telldata::tn_bool, NULL);
   addfield("sc"  , telldata::tn_real, NULL);
};

//=============================================================================
void telldata::ttreal::assign(tell_var* rt)
{
   if (rt->get_type() == tn_real)
   {
      _value = static_cast<ttreal*>(rt)->value();
      update_cstat();
   }
   else if (rt->get_type() == tn_int)
   {
      _value = static_cast<ttint*>(rt)->value();
      update_cstat();
   }
//   @TODO else ERROR Run time error (or Warning) -> unexpected type in ..bla bla
}

void telldata::ttreal::echo(std::string& wstr, real) {
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
   {
      _value = (int4b) rint(static_cast<ttreal*>(rt)->value());
      update_cstat();
   }
   else if (rt->get_type() == tn_int)
   {
      _value = static_cast<ttint*>(rt)->value();
      update_cstat();
   }
   else
      assert(false);
}

void telldata::ttint::echo(std::string& wstr, real) {
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
   update_cstat();
}

void telldata::ttbool::echo(std::string& wstr, real) {
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
   update_cstat();
}

void telldata::ttstring::echo(std::string& wstr, real) {
   std::ostringstream ost;
   ost << "\"" << _value << "\"";
   wstr += ost.str();
}
//=============================================================================
telldata::ttlayout::ttlayout(const ttlayout& cobj) : tell_var(cobj.get_type()) {
   if (NULL != cobj._selp) _selp = DEBUG_NEW SGBitSet(*(cobj._selp));
   else _selp = NULL;
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
}

const telldata::ttlayout& telldata::ttlayout::operator = (const ttlayout& cobj) {
   if (_selp) {delete _selp; _selp = NULL;}
   if (NULL != cobj._selp) _selp = DEBUG_NEW SGBitSet((*cobj._selp));
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
   return *this;
}

void telldata::ttlayout::echo(std::string& wstr, real DBU)
{
   std::ostringstream ost;
   ost << "layer " << _layer << " :";
   _data->info(ost, DBU);
   if (_selp) ost << " - partially selected";
   wstr += ost.str();
}

void telldata::ttlayout::assign(tell_var* data) {
   _data = static_cast<ttlayout*>(data)->_data;
   _selp = static_cast<ttlayout*>(data)->_selp;
   update_cstat();
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

void telldata::ttlist::echo(std::string& wstr, real DBU)
{
   std::ostringstream ost;
   if (_mlist.empty()) {wstr += "empty list";}
   else
   {
      wstr += " list members: { ";
      for (unsigned i = 0; i < _mlist.size(); i++)
      {
         if (i > 0)  wstr += " , ";
         (_mlist[i])->echo(wstr, DBU);
      }
      wstr += " } ";
   }
}

void telldata::ttlist::assign(tell_var* rt) {
   this->operator = (*(static_cast<ttlist*>(rt)));
}

telldata::tell_var* telldata::ttlist::index_var(_dbl_word index)
{
   if (_mlist.empty() || (index > (_mlist.size() - 1))) return NULL;
   else return _mlist[index];
}

bool telldata::ttlist::validIndex(_dbl_word index)
{
   _dbl_word cursize = _mlist.size();
   if ((0 == cursize) || (index > (cursize - 1))) return false;
   else return true;
}

void telldata::ttlist::insert(telldata::tell_var* newval)
{
   _mlist.push_back(newval->selfcopy());
}

void telldata::ttlist::insert(telldata::tell_var* newval, _dbl_word index)
{
   assert(index >=0); assert(index <= _mlist.size());
   if (index == _mlist.size())
   {
      // add a position in the list and copy the last component into it
//      _mlist.push_back(_mlist[_mlist.size()-1]->selfcopy());
      _mlist.push_back(newval->selfcopy());
   }
   else
   {
      // insert a position before index and copy the contents of index position there
      memlist::iterator CI;
      unsigned idx = 0;
      for(CI = _mlist.begin(); CI != _mlist.end(); CI++, idx++)
      {
         if (idx == index) break;
      }
      assert(NULL != (*CI));
      _mlist.insert(CI,newval->selfcopy());
   }
}

void telldata::ttlist::lunion(telldata::ttlist* inlist)
{
   for (memlist::const_iterator CI = inlist->_mlist.begin(); CI != inlist->_mlist.end(); CI++)
      _mlist.push_back((*CI)->selfcopy());
}

void telldata::ttlist::lunion(telldata::ttlist* inlist, _dbl_word index)
{
   assert(index >=0); assert(index <= _mlist.size());
   if (index == _mlist.size())
   {
      for (telldata::memlist::const_iterator CCI = inlist->_mlist.begin(); CCI != inlist->_mlist.end(); CCI++)
         _mlist.push_back((*CCI)->selfcopy());
   }
   else
   {
      memlist::iterator CI;
      unsigned idx = 0;
      for(CI = _mlist.begin(); CI != _mlist.end(); CI++, idx++)
      {
         if (idx == index) break;
      }
      assert(NULL != (*CI));
      for (telldata::memlist::const_iterator CCI = inlist->_mlist.begin(); CCI != inlist->_mlist.end(); CCI++)
         _mlist.insert(CI, (*CCI)->selfcopy());
   }
}

telldata::tell_var* telldata::ttlist::erase(_dbl_word index)
{
   assert(index >=0); assert(index < _mlist.size());
   telldata::tell_var* erased = _mlist[index];
   if (index == (_mlist.size() - 1))
   {
      // remove the last component
      _mlist.pop_back();
   }
   else
   {
      // erase the position at the index
      memlist::iterator CI;
      unsigned idx = 0;
      for(CI = _mlist.begin(); CI != _mlist.end(); CI++, idx++)
      {
         if (idx == index) break;
      }
      _mlist.erase(CI);
   }
   return erased;
}

telldata::tell_var* telldata::ttlist::erase(_dbl_word idxB, _dbl_word idxE)
{
   assert(idxB >=0); assert(idxB < _mlist.size());
   assert(idxE >=0); assert(idxE < _mlist.size());
   telldata::ttlist* erased = DEBUG_NEW telldata::ttlist(get_type());
   // erase the position at the index
   memlist::iterator CI, CIB, CIE;
   unsigned idx = 0;
   for(CIB = _mlist.begin(); CIB != _mlist.end(); CIB++, idx++)
   {
      if (idx == idxB) break;
   }
   for(CIE = _mlist.begin(), idx = 0; CIE != _mlist.end(); CIE++, idx++)
   {
      if (idx == idxE+1) break;
   }
   for(CI = CIB; CI != CIE; CI++)
      erased->add((*CI)/*->selfcopy()*/);
   // If I remember correctly erase method from the STDLIB should not delete
   // the components. Means that we can reuse them - i.e. - don't need a selfcopy
   _mlist.erase(CIB, CIE);

   return erased;
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

void telldata::user_struct::echo(std::string& wstr, real DBU) {
   wstr += "struct members:\n";
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++) {
      wstr += CI->first; wstr += ": "; CI->second->echo(wstr, DBU);
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
                                         _x(DEBUG_NEW ttreal(x)), _y(DEBUG_NEW ttreal(y)) 
{
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

telldata::ttpnt::ttpnt(operandSTACK& OPstack) : user_struct(telldata::tn_pnt) 
{
   _y = DEBUG_NEW telldata::ttreal(); _y->assign(OPstack.top());
   delete OPstack.top(); OPstack.pop();
   _x = DEBUG_NEW telldata::ttreal(); _x->assign(OPstack.top());
   delete OPstack.top(); OPstack.pop();
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

telldata::ttpnt::ttpnt(const ttpnt& invar) : user_struct(telldata::tn_pnt) ,
                         _x(DEBUG_NEW ttreal(invar.x())), _y(DEBUG_NEW ttreal(invar.y())) 
{
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

void telldata::ttpnt::assign(tell_var* rt) 
{
   _x->_value = static_cast<ttpnt*>(rt)->x();
   _y->_value = static_cast<ttpnt*>(rt)->y();
}

void telldata::ttpnt::echo(std::string& wstr, real) 
{
   std::ostringstream ost;
   ost << "{X = " << x() << ", Y = " << y() << "}";
   wstr += ost.str();
}

const telldata::ttpnt& telldata::ttpnt::operator = (const ttpnt& a) 
{
   _x->_value = a.x(); _y->_value = a.y();
   return *this;
}

//=============================================================================
telldata::ttwnd::ttwnd( real bl_x, real bl_y, real tr_x, real tr_y ) :
                                                       user_struct(tn_box),
                                       _p1(DEBUG_NEW telldata::ttpnt(bl_x,bl_y)),
                                       _p2(DEBUG_NEW telldata::ttpnt(tr_x,tr_y)) {
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}

telldata::ttwnd::ttwnd( ttpnt bl, ttpnt tr ) : user_struct(tn_box),
                 _p1(DEBUG_NEW telldata::ttpnt(bl)), _p2(DEBUG_NEW telldata::ttpnt(tr)) {
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}

telldata::ttwnd::ttwnd(const ttwnd& cobj) : user_struct(tn_box),
    _p1(DEBUG_NEW telldata::ttpnt(cobj.p1())), _p2(DEBUG_NEW telldata::ttpnt(cobj.p2())) {
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

void telldata::ttwnd::echo(std::string& wstr, real) {
   std::ostringstream ost;
   ost << "P1: X = " << p1().x() << ": Y = " << p1().y() << " ; " <<
          "P2: X = " << p2().x() << ": Y = " << p2().y() ;
   wstr += ost.str();
}

const telldata::ttwnd& telldata::ttwnd::operator = (const ttwnd& a) {
   (*_p1) = a.p1(); (*_p2) = a.p2();
   return *this;
}

void telldata::ttwnd::normalize(bool& swapx, bool& swapy)
{
   real swap;
   swapx = swapy = false;
   if (p1().x() > p2().x())
   {
      swap = p1().x(); _p1->set_x(p2().x()); _p2->set_x(swap);
      swapx = true;
   }
   if (p1().y() > p2().y())
   {
      swap = p1().y(); _p1->set_y(p2().y()); _p2->set_y(swap);
      swapy = true;
   }
}

void telldata::ttwnd::denormalize(bool swapx, bool swapy)
{
   real swap;
   if (swapx)
   {
      swap = p1().x(); _p1->set_x(p2().x()); _p2->set_x(swap);
   }
   if (swapy)
   {
      swap = p1().y(); _p1->set_y(p2().y()); _p2->set_y(swap);
   }
}

//=============================================================================
telldata::ttbnd::ttbnd( real p_x, real p_y, real rot, bool flip, real scale) :
      user_struct(tn_bnd),
      _p(DEBUG_NEW telldata::ttpnt(p_x, p_y)),
      _rot(DEBUG_NEW telldata::ttreal(rot)),
      _flx(DEBUG_NEW telldata::ttbool(flip)),
      _sc(DEBUG_NEW telldata::ttreal(scale))
{
   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}

telldata::ttbnd::ttbnd( ttpnt p, ttreal rot, ttbool flip, ttreal scale) :
      user_struct(tn_bnd),
      _p(DEBUG_NEW telldata::ttpnt(p)),
      _rot(DEBUG_NEW telldata::ttreal(rot)),
      _flx(DEBUG_NEW telldata::ttbool(flip)),
      _sc(DEBUG_NEW telldata::ttreal(scale))
{
   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}

telldata::ttbnd::ttbnd(const ttbnd& cobj) : user_struct(tn_bnd),
      _p(DEBUG_NEW telldata::ttpnt(cobj.p())),
      _rot(DEBUG_NEW telldata::ttreal(cobj.rot())),
      _flx(DEBUG_NEW telldata::ttbool(cobj.flx())),
      _sc(DEBUG_NEW telldata::ttreal(cobj.sc()))
{
   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}


telldata::ttbnd::ttbnd(operandSTACK& OPstack) : user_struct(telldata::tn_bnd)
{
   // Here - get the data from the stack and reuse it ... don't delete it.
   // The alternative - to make a selfcopy and then delete the original from the OPstack
    _sc = static_cast<telldata::ttreal*>(OPstack.top()); OPstack.pop();
    _flx = static_cast<telldata::ttbool*>(OPstack.top()); OPstack.pop();
    _rot = static_cast<telldata::ttreal*>(OPstack.top()); OPstack.pop();
    _p = static_cast<telldata::ttpnt*>(OPstack.top()); OPstack.pop();

   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}

void telldata::ttbnd::assign(tell_var* rt)
{
   (*_p  ) = static_cast<ttbnd*>(rt)->p();
   (*_rot) = static_cast<ttbnd*>(rt)->rot();
   (*_flx) = static_cast<ttbnd*>(rt)->flx();
   (*_sc ) = static_cast<ttbnd*>(rt)->sc();
}

void telldata::ttbnd::echo(std::string& wstr, real) {
   std::ostringstream ost;
   ost << "P: X = " << p().x() << ": Y = " << p().y() << " ; " <<
          "rot = "  << rot().value() << ": flipX " << (flx().value() ? "true" : "false") << " ; "  <<
          "scale = " << sc().value();
   wstr += ost.str();
}

const telldata::ttbnd& telldata::ttbnd::operator = (const ttbnd& a) {
   (*_p) = a.p();
   (*_rot) = a.rot();
   (*_flx) = a.flx();
   (*_sc) = a.sc();
   return *this;
}


//=============================================================================
telldata::argumentID::argumentID(const argumentID& obj2copy) {
   _ID = obj2copy();
   _command = obj2copy._command;
   if (0 < obj2copy.child().size())
   {
      for(argumentQ::const_iterator CA = obj2copy.child().begin(); CA != obj2copy.child().end(); CA ++)
         _child.push_back(DEBUG_NEW argumentID(**CA));
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
            if (TLCOMPOSIT_TYPE((CF->second) & (~telldata::tn_listmask)))
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

std::string telldata::echoType( const telldata::typeID tID,
                                 const telldata::typeMAP* lclTypeDef)
{
   std::string atype;
   switch (tID & ~telldata::tn_listmask)
   {
      case telldata::tn_void  : atype = "void"  ; break;
      case telldata::tn_int   : atype = "int"   ; break;
      case telldata::tn_real  : atype = "real"  ; break;
      case telldata::tn_bool  : atype = "bool"  ; break;
      case telldata::tn_string: atype = "string"; break;
      case telldata::tn_layout: atype = "layout"; break;
      case telldata::tn_pnt   : atype = "point" ; break;
      case telldata::tn_box   : atype = "box"   ; break;
      default                 :
      {
         atype = "?UNKNOWN TYPE?";
         if (NULL != lclTypeDef)
         {
            for( telldata::typeMAP::const_iterator CT = lclTypeDef->begin();
                 CT != lclTypeDef->end(); CT++)
            {
               if (tID == CT->second->ID())
               {
                  atype = CT->first; break;
               }
            }
         }
      }
   }
   if (tID & telldata::tn_listmask)
      atype +=" list";
   return atype;
}

