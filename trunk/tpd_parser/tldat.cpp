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

#include "tedat.h"   //<< Must find a way to remove this from here. See line 243 - it's all about it!
#include "auxdat.h" //<< Must find a way to remove this from here. See line 282 - it's all about it!

extern parsercmd::cmdBLOCK*       CMDBlock;

//=============================================================================
telldata::TellVar* telldata::TCompType::initfield(const typeID ID) const
{
   telldata::TellVar* nvar;
   if (ID & telldata::tn_listmask) nvar = DEBUG_NEW telldata::TtList(ID & ~telldata::tn_listmask);
   else
      switch(ID & ~telldata::tn_listmask)
      {
         case tn_void  : assert(false);
         case tn_int   : nvar = DEBUG_NEW telldata::TtInt()    ;break;
         case tn_real  : nvar = DEBUG_NEW telldata::TtReal()   ;break;
         case tn_bool  : nvar = DEBUG_NEW telldata::TtBool()   ;break;
         case tn_string: nvar = DEBUG_NEW telldata::TtString() ;break;
         case tn_pnt   : nvar = DEBUG_NEW telldata::TtPnt()    ;break;
         case tn_box   : nvar = DEBUG_NEW telldata::TtWnd()    ;break;
         case tn_bnd   : nvar = DEBUG_NEW telldata::TtBnd()    ;break;
         case tn_hsh   : nvar = DEBUG_NEW telldata::TtHsh()    ;break;
         case tn_hshstr: nvar = DEBUG_NEW telldata::TtHshStr() ;break;
         case tn_layout: nvar = DEBUG_NEW telldata::TtLayout() ;break;
         case tn_auxilary: nvar = DEBUG_NEW telldata::TtAuxdata() ;break;
                default: {
                     assert(_tIDMAP.end() != _tIDMAP.find(ID));
                     const TType* vartype = _tIDMAP.find(ID)->second;
                     if (vartype->isComposite())
                        nvar = DEBUG_NEW telldata::TtUserStruct(static_cast<const TCompType*>(vartype));
                     else
                        nvar = DEBUG_NEW telldata::TtCallBack(ID);
                }
      }
   return nvar;
}

bool telldata::TCompType::addfield(std::string fname, typeID fID, const TType* utype)
{
   // search for a field with this name
   for (recfieldsID::const_iterator CF = _fields.begin(); CF != _fields.end(); CF++)
   {
      if (CF->first == fname) return false;
   }
   _fields.push_back(structRECID(fname,fID));
    if (NULL != utype) _tIDMAP[fID] = utype;
    return true;
}

const telldata::TType* telldata::TCompType::findtype(const typeID basetype) const
{
   assert(_tIDMAP.end() != _tIDMAP.find(basetype));
   return _tIDMAP.find(basetype)->second;
}

//=============================================================================
void telldata::TCallBackType::pushArg(typeID pID)
{
   _paramList.push_back(pID);
}

//=============================================================================
telldata::TPointType::TPointType() : TCompType(telldata::tn_pnt)
{
   addfield("x", telldata::tn_real, NULL);
   addfield("y", telldata::tn_real, NULL);
};

//=============================================================================
telldata::TBoxType::TBoxType(TPointType* pfld) : TCompType(telldata::tn_box)
{
   addfield("p1", telldata::tn_pnt, pfld);
   addfield("p2", telldata::tn_pnt, pfld);
};

//=============================================================================
telldata::TBindType::TBindType(TPointType* pfld) : TCompType(telldata::tn_bnd)
{
   addfield("p"   , telldata::tn_pnt , pfld);
   addfield("rot" , telldata::tn_real, NULL);
   addfield("flx" , telldata::tn_bool, NULL);
   addfield("sc"  , telldata::tn_real, NULL);
};

//=============================================================================
telldata::THshType::THshType() : TCompType(telldata::tn_hsh)
{
   addfield("key"   , telldata::tn_int   , NULL);
   addfield("value" , telldata::tn_string, NULL);
};

//=============================================================================
telldata::THshStrType::THshStrType() : TCompType(telldata::tn_hshstr)
{
   addfield("key"   , telldata::tn_string, NULL);
   addfield("value" , telldata::tn_string, NULL);
};

//=============================================================================
void telldata::TtReal::assign(TellVar* rt)
{
   if (rt->get_type() == tn_real)
   {
      _value = static_cast<TtReal*>(rt)->value();
      update_cstat();
   }
   else if (rt->get_type() == tn_int)
   {
      _value = static_cast<TtInt*>(rt)->value();
      update_cstat();
   }
//   @TODO else ERROR Run time error (or Warning) -> unexpected type in ..bla bla
}

void telldata::TtReal::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   std::scientific(ost);
   ost << value();
   wstr += ost.str();
}

const telldata::TtReal& telldata::TtReal::operator = (const TtReal& a)
{
   _value = a.value();
   return *this;
}

const telldata::TtReal& telldata::TtReal::operator = (const TtInt& a)
{
   _value = a.value();
   return *this;
}
//=============================================================================
void telldata::TtInt::assign(TellVar* rt)
{
   if (rt->get_type() == tn_real)
   {
      // Note! There is no rint() here deliberately - for compatibility
      // with normal C. See Issue 47. There is a tell function rint() with
      // the same functionality as the corresponding C function which
      // shall be used for rounding
      _value = (int4b) static_cast<TtReal*>(rt)->value();
      update_cstat();
   }
   else if (rt->get_type() == tn_int)
   {
      _value = static_cast<TtInt*>(rt)->value();
      update_cstat();
   }
   else
      assert(false);
}

void telldata::TtInt::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << value();
   wstr += ost.str();
}

const telldata::TtInt& telldata::TtInt::operator = (const TtInt& a)
{
   _value = a.value();
   return *this;
}

const telldata::TtInt& telldata::TtInt::operator = (const TtReal& a)
{
   // if a.value outside the limits -> send a runtime error message
//   real  rval = a.value();
//   int4b ival = (int4b)rval;
//   _value =
   _value = (int4b)a.value();
   return *this;
}
//=============================================================================
const telldata::TtBool& telldata::TtBool::operator = (const TtBool& a)
{
   _value = a.value();
   return *this;
}

void telldata::TtBool::assign(TellVar* rt)
{
   _value = static_cast<TtBool*>(rt)->value();
   update_cstat();
}

void telldata::TtBool::echo(std::string& wstr, real)
{
   if (_value) wstr += "true";
   else        wstr += "false";
}

//=============================================================================
const telldata::TtString& telldata::TtString::operator = (const TtString& a)
{
   _value = a.value();
   return *this;
}

void telldata::TtString::assign(TellVar* value)
{
   _value = static_cast<TtString*>(value)->_value;
   update_cstat();
}

void telldata::TtString::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << "\"" << _value << "\"";
   wstr += ost.str();
}
//=============================================================================
telldata::TtLayout::TtLayout(const TtLayout& cobj) : TellVar(cobj.get_type())
{
   if (NULL != cobj._selp) _selp = DEBUG_NEW SGBitSet(*(cobj._selp));
   else _selp = NULL;
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
}

const telldata::TtLayout& telldata::TtLayout::operator = (const TtLayout& cobj)
{
   if (_selp) {delete _selp; _selp = NULL;}
   if (NULL != cobj._selp) _selp = DEBUG_NEW SGBitSet((*cobj._selp));
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
   return *this;
}

void telldata::TtLayout::echo(std::string& wstr, real DBU)
{
   std::ostringstream ost;
   if (NULL == _data)
      ost << "< !EMPTY! >";
   else
   {
      if ( LAST_EDITABLE_LAYNUM > _layer)
         ost << "layer " << _layer << " :";
      _data->info(ost, DBU);
   }
   if (_selp && (_selp->size() > 0)) ost << " - partially selected";
   wstr += ost.str();
}

void telldata::TtLayout::assign(TellVar* data)
{
   TtLayout* variable = static_cast<TtLayout*>(data);
   _data  = variable->_data;
   if (NULL != variable->_selp) _selp = DEBUG_NEW SGBitSet(*(variable->_selp));
   else _selp = NULL;
   _layer = variable->_layer;
   update_cstat();
}

//=============================================================================
telldata::TtAuxdata::TtAuxdata(const TtAuxdata& cobj) :
      TellVar  ( cobj.get_type() ),
     _data     ( cobj._data      ), // don't copy the layout data!
     _layer    ( cobj._layer     )
{}

const telldata::TtAuxdata& telldata::TtAuxdata::operator = (const TtAuxdata& cobj)
{
   _layer = cobj._layer;
   _data = cobj._data; // don't copy the layout data!
   return *this;
}

void telldata::TtAuxdata::echo(std::string& wstr, real DBU)
{
   std::ostringstream ost;
   if (NULL == _data)
      ost << "< !EMPTY! >";
   else
   {
      if ( LAST_EDITABLE_LAYNUM > _layer)
         ost << "layer " << _layer << " :";
      _data->info(ost, DBU);
   }
   wstr += ost.str();
}

void telldata::TtAuxdata::assign(TellVar* data)
{
   TtAuxdata* variable = static_cast<TtAuxdata*>(data);
   _data  = variable->_data;
   _layer = variable->_layer;
   update_cstat();
}
//=============================================================================
telldata::TtList::TtList(const telldata::TtList& cobj) : TellVar(cobj.get_type())
{
   // copy constructor
   unsigned count = cobj._mlist.size();
   _mlist.resize(count);
   for (unsigned i = 0; i < count; i++)
      _mlist[i] = cobj._mlist[i]->selfcopy();
//     _mlist.push_back(cobj._mlist[i]->selfcopy());
}

const telldata::TtList& telldata::TtList::operator =(const telldata::TtList& cobj)
{
   unsigned count = _mlist.size();
   unsigned i;
   for (i = 0; i < count; i++)
      delete _mlist[i];
   _mlist.clear();
   count = cobj._mlist.size();
   _mlist.reserve(count);
   for (i = 0; i < count; i++)
   {
      typeID compID = cobj._mlist[i]->get_type();
      typeID localID = _ID & (~telldata::tn_listmask);
      if (compID == localID)
         _mlist.push_back(cobj._mlist[i]->selfcopy());
      else if (NUMBER_TYPE(compID) && NUMBER_TYPE(localID))
      {
         // The whole idea here is to keep the list component type homogeneous
         // The initial trouble comes from the internal conversion between int and real
         // and this introduces the risk to contaminate int lists with a real value
         // and vice versa
         if (telldata::tn_int == localID)
            _mlist.push_back(DEBUG_NEW TtInt(static_cast<TtReal*>(cobj._mlist[i])->value()));
         else
            _mlist.push_back(DEBUG_NEW TtReal(static_cast<TtInt*>(cobj._mlist[i])->value()));
      }
      else
      {
         // Unexpected type in the rvalue list. Component i
         assert(false);
      }
   }
   return *this;
}

void telldata::TtList::initialize()
{
   for (unsigned long i = 0 ; i < _mlist.size(); i++)
   {
      delete _mlist[i];
   }
   _mlist.clear();
}


void telldata::TtList::echo(std::string& wstr, real DBU)
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

void telldata::TtList::assign(TellVar* rt)
{
   this->operator = (*(static_cast<TtList*>(rt)));
}

telldata::TellVar* telldata::TtList::index_var(dword index)
{
   if (_mlist.empty() || (index > (_mlist.size() - 1))) return NULL;
   else return _mlist[index];
}

void telldata::TtList::resize(unsigned num, TellVar* initVar)
{
   _mlist.resize(num);
   for (unsigned i = 0; i < num; i++)
   {
//      if (NULL == _mlist[i])
         _mlist[i] = initVar->selfcopy();
   }
   delete initVar;
}

bool telldata::TtList::validIndex(dword index)
{
   dword cursize = _mlist.size();
   if ((0 == cursize) || (index > (cursize - 1))) return false;
   else return true;
}

void telldata::TtList::insert(telldata::TellVar* newval)
{
   _mlist.push_back(newval->selfcopy());
}

void telldata::TtList::insert(telldata::TellVar* newval, dword index)
{
   assert(index >=0); assert(index <= _mlist.size());
   if (index == _mlist.size())
   {
      // add a position in the list and copy the last component into it
//      _mlist.push_back(_mlist[_mlist.size()-1]->selfcopy());
      _mlist.push_back(newval->selfcopy());
//      _mlist[index] = newval->selfcopy();
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

void telldata::TtList::lunion(telldata::TtList* inlist)
{
   for (memlist::const_iterator CI = inlist->_mlist.begin(); CI != inlist->_mlist.end(); CI++)
      _mlist.push_back((*CI)->selfcopy());
}

void telldata::TtList::lunion(telldata::TtList* inlist, dword index)
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

telldata::TellVar* telldata::TtList::erase(dword index)
{
   assert(index >=0); assert(index < _mlist.size());
   telldata::TellVar* erased = _mlist[index];
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

telldata::TellVar* telldata::TtList::erase(dword idxB, dword idxE)
{
   assert(idxB >=0); assert(idxB < _mlist.size());
   assert(idxE >=0); assert(idxE < _mlist.size());
   telldata::TtList* erased = DEBUG_NEW telldata::TtList(get_type());
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

telldata::TtList::~TtList()
{
   for (unsigned long i = 0 ; i < _mlist.size(); i++)
      delete _mlist[i];
}

//=============================================================================
telldata::TtUserStruct::TtUserStruct(const TCompType* tltypedef) : TellVar(tltypedef->ID())
{
   const recfieldsID& typefields = tltypedef->fields();
   for (recfieldsID::const_iterator CI = typefields.begin(); CI != typefields.end(); CI++)
      _fieldList.push_back(structRECNAME(CI->first,tltypedef->initfield(CI->second)));
}

telldata::TtUserStruct::TtUserStruct(const TCompType* tltypedef, operandSTACK& OPstack) :
   TellVar(tltypedef->ID())
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


telldata::TtUserStruct::TtUserStruct(const TtUserStruct& cobj) : TellVar(cobj.get_type())
{
   for (recfieldsNAME::const_iterator CI = cobj._fieldList.begin(); CI != cobj._fieldList.end(); CI++)
      _fieldList.push_back(structRECNAME(CI->first, CI->second->selfcopy()));
}

telldata::TtUserStruct::~TtUserStruct()
{
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++)
      delete CI->second;
}

void telldata::TtUserStruct::initialize()
{
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++)
      CI->second->initialize();
}

void telldata::TtUserStruct::echo(std::string& wstr, real DBU)
{
   wstr += "struct members:\n";
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++) {
      wstr += CI->first; wstr += ": "; CI->second->echo(wstr, DBU);
      wstr += "\n";
   }
}

void telldata::TtUserStruct::assign(TellVar* value)
{
   TtUserStruct* n_value = static_cast<telldata::TtUserStruct*>(value);
   for (recfieldsNAME::const_iterator CI = _fieldList.begin(); CI != _fieldList.end(); CI++) {
      // find the corresponding field in n_value and get the TellVar
      TellVar* fieldvar = NULL;
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

telldata::TellVar* telldata::TtUserStruct::field_var(char*& fname)
{
   std::string fieldName(fname); fieldName.erase(0,1);

   for(recfieldsNAME::const_iterator CI  = _fieldList.begin();
                                     CI != _fieldList.end(); CI++)
   {
         if (fieldName == CI->first) return CI->second;
   }
   return NULL;
}

//=============================================================================
telldata::TtPnt::TtPnt (real x, real y) : TtUserStruct(telldata::tn_pnt),
                                         _x(DEBUG_NEW TtReal(x)), _y(DEBUG_NEW TtReal(y))
{
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

telldata::TtPnt::TtPnt(operandSTACK& OPstack) : TtUserStruct(telldata::tn_pnt)
{
   _y = DEBUG_NEW telldata::TtReal(); _y->assign(OPstack.top());
   delete OPstack.top(); OPstack.pop();
   _x = DEBUG_NEW telldata::TtReal(); _x->assign(OPstack.top());
   delete OPstack.top(); OPstack.pop();
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

telldata::TtPnt::TtPnt(const TtPnt& invar) : TtUserStruct(telldata::tn_pnt) ,
                         _x(DEBUG_NEW TtReal(invar.x())), _y(DEBUG_NEW TtReal(invar.y()))
{
   _fieldList.push_back(structRECNAME("x", _x));
   _fieldList.push_back(structRECNAME("y", _y));
}

void telldata::TtPnt::assign(TellVar* rt)
{
   _x->_value = static_cast<TtPnt*>(rt)->x();
   _y->_value = static_cast<TtPnt*>(rt)->y();
}

void telldata::TtPnt::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << "{X = " << x() << ", Y = " << y() << "}";
   wstr += ost.str();
}

const telldata::TtPnt& telldata::TtPnt::operator = (const TtPnt& a)
{
   _x->_value = a.x(); _y->_value = a.y();
   return *this;
}

//=============================================================================
telldata::TtWnd::TtWnd( real bl_x, real bl_y, real tr_x, real tr_y ) :
                                                       TtUserStruct(tn_box),
                                       _p1(DEBUG_NEW telldata::TtPnt(bl_x,bl_y)),
                                       _p2(DEBUG_NEW telldata::TtPnt(tr_x,tr_y))
{
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}

telldata::TtWnd::TtWnd( TtPnt bl, TtPnt tr ) : TtUserStruct(tn_box),
                 _p1(DEBUG_NEW telldata::TtPnt(bl)), _p2(DEBUG_NEW telldata::TtPnt(tr))
{
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}

telldata::TtWnd::TtWnd(const TtWnd& cobj) : TtUserStruct(tn_box),
    _p1(DEBUG_NEW telldata::TtPnt(cobj.p1())), _p2(DEBUG_NEW telldata::TtPnt(cobj.p2()))
{
   _fieldList.push_back(structRECNAME("p1", _p1));
   _fieldList.push_back(structRECNAME("p2", _p2));
}


telldata::TtWnd::TtWnd(operandSTACK& OPstack) : TtUserStruct(telldata::tn_box)
{
   // Here - just get the pointer to the points in the stack...
    _p2 = static_cast<telldata::TtPnt*>(OPstack.top());
    // .. that's why - don't delete them. The alternative -
    // - to make a selfcopy and then to delete the original from the OPstack
    OPstack.pop();
    _p1 = static_cast<telldata::TtPnt*>(OPstack.top());
    OPstack.pop();
    _fieldList.push_back(structRECNAME("p1", _p1));
    _fieldList.push_back(structRECNAME("p2", _p2));
}

void telldata::TtWnd::assign(TellVar* rt)
{
   (*_p1) = static_cast<TtWnd*>(rt)->p1();
   (*_p2) = static_cast<TtWnd*>(rt)->p2();
}

void telldata::TtWnd::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << "P1: X = " << p1().x() << ": Y = " << p1().y() << " ; " <<
          "P2: X = " << p2().x() << ": Y = " << p2().y() ;
   wstr += ost.str();
}

const telldata::TtWnd& telldata::TtWnd::operator = (const TtWnd& a)
{
   (*_p1) = a.p1(); (*_p2) = a.p2();
   return *this;
}

void telldata::TtWnd::normalize(bool& swapx, bool& swapy)
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

void telldata::TtWnd::denormalize(bool swapx, bool swapy)
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
telldata::TtBnd::TtBnd( real p_x, real p_y, real rot, bool flip, real scale) :
      TtUserStruct(tn_bnd),
      _p(DEBUG_NEW telldata::TtPnt(p_x, p_y)),
      _rot(DEBUG_NEW telldata::TtReal(rot)),
      _flx(DEBUG_NEW telldata::TtBool(flip)),
      _sc(DEBUG_NEW telldata::TtReal(scale))
{
   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}

telldata::TtBnd::TtBnd( TtPnt p, TtReal rot, TtBool flip, TtReal scale) :
      TtUserStruct(tn_bnd),
      _p(DEBUG_NEW telldata::TtPnt(p)),
      _rot(DEBUG_NEW telldata::TtReal(rot)),
      _flx(DEBUG_NEW telldata::TtBool(flip)),
      _sc(DEBUG_NEW telldata::TtReal(scale))
{
   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}

telldata::TtBnd::TtBnd(const TtBnd& cobj) : TtUserStruct(tn_bnd),
      _p(DEBUG_NEW telldata::TtPnt(cobj.p())),
      _rot(DEBUG_NEW telldata::TtReal(cobj.rot())),
      _flx(DEBUG_NEW telldata::TtBool(cobj.flx())),
      _sc(DEBUG_NEW telldata::TtReal(cobj.sc()))
{
   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}


telldata::TtBnd::TtBnd(operandSTACK& OPstack) : TtUserStruct(telldata::tn_bnd)
{
   // Here - get the data from the stack and reuse it ... don't delete it.
   // The alternative - to make a selfcopy and then delete the original from the OPstack
    _sc = static_cast<telldata::TtReal*>(OPstack.top()); OPstack.pop();
    _flx = static_cast<telldata::TtBool*>(OPstack.top()); OPstack.pop();
    _rot = static_cast<telldata::TtReal*>(OPstack.top()); OPstack.pop();
    _p = static_cast<telldata::TtPnt*>(OPstack.top()); OPstack.pop();

   _fieldList.push_back(structRECNAME("p"  , _p  ));
   _fieldList.push_back(structRECNAME("rot", _rot));
   _fieldList.push_back(structRECNAME("flx", _flx));
   _fieldList.push_back(structRECNAME("sc" , _sc ));
}

void telldata::TtBnd::assign(TellVar* rt)
{
   (*_p  ) = static_cast<TtBnd*>(rt)->p();
   (*_rot) = static_cast<TtBnd*>(rt)->rot();
   (*_flx) = static_cast<TtBnd*>(rt)->flx();
   (*_sc ) = static_cast<TtBnd*>(rt)->sc();
}

void telldata::TtBnd::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << "P: X = " << p().x() << ": Y = " << p().y() << " ; " <<
          "rot = "  << rot().value() << ": flipX " << (flx().value() ? "true" : "false") << " ; "  <<
          "scale = " << sc().value();
   wstr += ost.str();
}

const telldata::TtBnd& telldata::TtBnd::operator = (const TtBnd& a)
{
   (*_p) = a.p();
   (*_rot) = a.rot();
   (*_flx) = a.flx();
   (*_sc) = a.sc();
   return *this;
}

//=============================================================================
/*   class TtHsh : public TtUserStruct {*/
telldata::TtHsh::TtHsh (int4b number, std::string name) : TtUserStruct(tn_hsh),
      _key(DEBUG_NEW telldata::TtInt(number)),
      _value(DEBUG_NEW telldata::TtString(name))
{
   _fieldList.push_back(structRECNAME("key"  , _key  ));
   _fieldList.push_back(structRECNAME("value", _value  ));
}

telldata::TtHsh::TtHsh(const TtHsh& cobj) : TtUserStruct(tn_hsh),
      _key(DEBUG_NEW telldata::TtInt(cobj.key())),
      _value(DEBUG_NEW telldata::TtString(cobj.value()))
{
   _fieldList.push_back(structRECNAME("key"  , _key  ));
   _fieldList.push_back(structRECNAME("value", _value  ));
}

telldata::TtHsh::TtHsh(operandSTACK& OPstack) : TtUserStruct(tn_hsh)
{
   // Here - get the data from the stack and reuse it ... don't delete it.
   // The alternative - to make a selfcopy and then delete the original from the OPstack
   _value  = static_cast<telldata::TtString*>(OPstack.top()); OPstack.pop();
   _key    = static_cast<telldata::TtInt*>(OPstack.top()); OPstack.pop();

   _fieldList.push_back(structRECNAME("key"  , _key   ));
   _fieldList.push_back(structRECNAME("value", _value ));
}

void telldata::TtHsh::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << "key = "  << key().value() << " : value = \"" << value().value() << "\"";
   wstr += ost.str();
}

void telldata::TtHsh::assign(TellVar* rt)
{
   (*_key  ) = static_cast<TtHsh*>(rt)->key();
   (*_value) = static_cast<TtHsh*>(rt)->value();
}

//=============================================================================
/*   class TtHshStr : public TtUserStruct {*/
telldata::TtHshStr::TtHshStr (std::string number, std::string name) : TtUserStruct(tn_hshstr),
      _key(DEBUG_NEW telldata::TtString(number)),
      _value(DEBUG_NEW telldata::TtString(name))
{
   _fieldList.push_back(structRECNAME("key"  , _key  ));
   _fieldList.push_back(structRECNAME("value", _value  ));
}

telldata::TtHshStr::TtHshStr(const TtHshStr& cobj) : TtUserStruct(tn_hshstr),
      _key(DEBUG_NEW telldata::TtString(cobj.key())),
      _value(DEBUG_NEW telldata::TtString(cobj.value()))
{
   _fieldList.push_back(structRECNAME("key"  , _key  ));
   _fieldList.push_back(structRECNAME("value", _value  ));
}

telldata::TtHshStr::TtHshStr(operandSTACK& OPstack) : TtUserStruct(tn_hshstr)
{
   // Here - get the data from the stack and reuse it ... don't delete it.
   // The alternative - to make a selfcopy and then delete the original from the OPstack
   _value  = static_cast<telldata::TtString*>(OPstack.top()); OPstack.pop();
   _key    = static_cast<telldata::TtString*>(OPstack.top()); OPstack.pop();

   _fieldList.push_back(structRECNAME("key"  , _key   ));
   _fieldList.push_back(structRECNAME("value", _value ));
}

void telldata::TtHshStr::echo(std::string& wstr, real)
{
   std::ostringstream ost;
   ost << "key = "  << key().value() << " : value = \"" << value().value() << "\"";
   wstr += ost.str();
}

void telldata::TtHshStr::assign(TellVar* rt)
{
   (*_key  ) = static_cast<TtHshStr*>(rt)->key();
   (*_value) = static_cast<TtHshStr*>(rt)->value();
}


//=============================================================================
telldata::ArgumentID::ArgumentID(const ArgumentID& obj2copy)
{
   _ID = obj2copy();
   _command = obj2copy._command;
   if (0 < obj2copy.child().size())
   {
      for(argumentQ::const_iterator CA = obj2copy.child().begin(); CA != obj2copy.child().end(); CA ++)
         _child.push_back(DEBUG_NEW ArgumentID(**CA));
   }
}

void telldata::ArgumentID::toList(bool cmdUpdate, telldata::typeID alistID)
{
   if (0 < _child.size())
   {// i.e. list is not empty
      for(argumentQ::const_iterator CA = _child.begin(); CA != _child.end(); CA ++)
      {
         if (!(   (alistID == (**CA)())
               || (NUMBER_TYPE( alistID ) && NUMBER_TYPE( (**CA)() ))
               )) return;
      }
   }
   else // empty list
      assert (tn_NULL != alistID);
   _ID = TLISTOF(alistID);
   if (cmdUpdate)
      static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
}

void telldata::ArgumentID::userStructCheck(const telldata::TType* vtype, bool cmdUpdate)
{
   if (vtype->isComposite())
   {
      const telldata::TCompType* vartype = static_cast<const telldata::TCompType*>(vtype);
      const telldata::recfieldsID& recfields = vartype->fields();
      // first check that both lists have the same size
      if (_child.size() != recfields.size()) return;
      recfieldsID::const_iterator CF;
      argumentQ::iterator CA;
      for (CF = recfields.begin(), CA = _child.begin();
                (CF != recfields.end() && CA != _child.end()); CF ++, CA++)
      {
         if ( TLUNKNOWN_TYPE( (**CA)() ) )
         {
            if (TLISALIST(CF->second))
            {// check the list fields
               if (TLCOMPOSIT_TYPE((CF->second & (~telldata::tn_listmask))))
                  (*CA)->userStructListCheck(vartype->findtype(CF->second), cmdUpdate);
               else
                  (*CA)->toList(cmdUpdate, CF->second & ~telldata::tn_listmask);
            }
            else
               // call in recursion the userStructCheck method of the child
               (*CA)->userStructCheck(vartype->findtype(CF->second), cmdUpdate);
         }
         if (!NUMBER_TYPE( CF->second ))
         {
            // for non-number types there is no internal conversion,
            // so check strictly the type
            if ( (**CA)() != CF->second) return; // no match
         }
         else // for number types - allow compatibility (int to real only)
            if (!NUMBER_TYPE( (**CA)() )) return; // no match
            else if (CF->second < (**CA)() ) return; // no match
      }
      // all fields match => we can assign a known ID to the ArgumentID
      _ID = vartype->ID();
      if (cmdUpdate)
         static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
   }
   else
   {
      if (telldata::tn_anyfref != _ID) return; // the rval is not a function reference
      assert(NULL != _command);
      const telldata::TCallBackType* vartype = static_cast<const telldata::TCallBackType*>(vtype);
      parsercmd::cmdFUNCREF* frefCmd = static_cast<parsercmd::cmdFUNCREF*>(_command);

      const TypeIdList& fParamList = vartype->paramList();
      for (TypeIdList::const_iterator CP = fParamList.begin(); CP != fParamList.end(); CP++)
         _child.push_back(DEBUG_NEW ArgumentID(*CP));

      std::string fname = frefCmd->funcName();
      parsercmd::cmdSTDFUNC *fc = CMDBlock->getFuncBody(fname.c_str(),&_child);
      // whatever the result - clean-up the _child structure
      for (argumentQ::iterator CA = _child.begin(); CA != _child.end(); CA++)
         delete (*CA);
      _child.clear();

      if (NULL == fc) return; // can't find function with this name and argument list
      _ID = vtype->ID();
      if (cmdUpdate)
         frefCmd->setFuncBody( fc, _ID);
      else
         frefCmd->setPreCheckfBody( fc );
   }
}

void telldata::ArgumentID::userStructListCheck(const telldata::TType* vartype, bool cmdUpdate)
{
   for (argumentQ::iterator CA = _child.begin(); CA != _child.end(); CA++)
      if ( TLUNKNOWN_TYPE( (**CA)() ) ) (*CA)->userStructCheck(vartype, cmdUpdate);

   toList(cmdUpdate, vartype->ID());
}

void telldata::ArgumentID::adjustID(const ArgumentID& obj2copy)
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
   if (TLCOMPOSIT_TYPE( (_ID  & ~telldata::tn_listmask) ))
   {
      const telldata::TType* vartype = CMDBlock->getTypeByID(_ID  & ~telldata::tn_listmask);
      if (vartype->isComposite())
         static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
      else
         static_cast<parsercmd::cmdFUNCREF*>(_command)->setFuncBody(_ID);
   }
   else
      static_cast<parsercmd::cmdSTRUCT*>(_command)->setargID(this);
}

telldata::ArgumentID::~ArgumentID()
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
      case telldata::tn_auxilary: atype = "auxdata"; break;
      case telldata::tn_pnt   : atype = "point" ; break;
      case telldata::tn_box   : atype = "box"   ; break;
      case telldata::tn_hsh   : atype = "lmap"  ; break;
      case telldata::tn_hshstr: atype = "strmap"; break;
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

