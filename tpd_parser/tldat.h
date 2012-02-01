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

#ifndef CTM_H
#define CTM_H

#include <string>
#include <vector>
#include <algorithm>
#include "ttt.h"

#define NUMBER_TYPE(op) ((op > telldata::tn_void) && (op < telldata::tn_bool  ) && !(op & telldata::tn_listmask))
#define PRINTF_ABLE(op) ((op > telldata::tn_void) && (op < telldata::tn_layout) && !(op & telldata::tn_listmask))
#define TLISTOF(op) (op | telldata::tn_listmask)
#define TLISALIST(op) (op & telldata::tn_listmask)
#define TLCOMPOSIT_TYPE(op) (op > telldata::tn_composite)
#define TLUNKNOWN_TYPE(op) ((op == telldata::tn_composite) || (op == telldata::tn_anyfref))

namespace laydata {
   class TdtData;
}

namespace auxdata {
   class TdtAuxData;
}
//=============================================================================
// TELL types
//=============================================================================
namespace telldata {
   typedef unsigned int typeID;
   const typeID tn_NULL       =  0;
   const typeID tn_void       =  1;
   const typeID tn_int        =  2;
   const typeID tn_real       =  3;
   const typeID tn_bool       =  4;
   const typeID tn_string     =  5;
   const typeID tn_layout     =  6;
   const typeID tn_auxilary   =  7;
   const typeID tn_anyfref    =  9;
   const typeID tn_composite  = 10;
   const typeID tn_pnt        = 11;
   const typeID tn_box        = 12;
   const typeID tn_bnd        = 13;
   const typeID tn_hsh        = 14;
   const typeID tn_hshstr     = 15;
   const typeID tn_usertypes  = 16;
   // the most significant bit is a mask flag
   const typeID tn_listmask = typeID(1) << (8 * sizeof(typeID) - 1);

   class TellVar;
   class TType;
   class TtInt;
   class ArgumentID;

   typedef std::pair<std::string, typeID>    structRECID;
   typedef std::pair<std::string, TellVar*>  structRECNAME;
   typedef std::deque<structRECID>           recfieldsID;
   typedef std::deque<structRECNAME>         recfieldsNAME;
   typedef std::map<std::string, TType*>     typeMAP;
   typedef std::map<std::string, TellVar* >  variableMAP;
   typedef std::vector<TellVar*>             memlist;
   typedef std::deque<ArgumentID*>           argumentQ;
   typedef std::stack<telldata::TellVar*>    operandSTACK;
   typedef std::deque<telldata::TellVar*>    UNDOPerandQUEUE;
   typedef std::list<typeID>                 TypeIdList;
   typedef std::list<TType*>                 TypeList;


   //==============================================================================
   /*Every block (parsercmd::cmdBLOCK) defined maintains a table (map) to the
     locally defined user types in a form <typename - TCompType>. Every user type
     (telldata::TCompType), maintains a table (map) to the user defined types
     (telldata::TCompType) used in this type in a form <ID - TCompType. The latter is
     updated by addfield method. Thus the TCompType can execute its own copy
     constructor
   */
   class TType {
      public:
                              TType(typeID ID) : _ID(ID) {}
         virtual             ~TType(){}
         const typeID         ID() const        {return _ID;}
         virtual bool         isComposite() const = 0;
      protected:
         typeID               _ID;
   };
   //! Tell Composite Type
   class TCompType : public TType {
   public:
                           TCompType(typeID ID) : TType(ID) {assert(TLCOMPOSIT_TYPE(ID));}
      virtual             ~TCompType(){}
      bool                 addfield(std::string, typeID, const TType* utype);
      TellVar*             initfield(const typeID) const;
      const TType*         findtype(const typeID) const;
      const recfieldsID&   fields() const    {return _fields;}
      virtual bool         isComposite() const    {return true;}
   private:
      typedef std::map<const typeID, const TType*> typeIDMAP;
      recfieldsID          _fields;
      typeIDMAP            _tIDMAP;
   };

   class TCallBackType : public TType {
   public:
                           TCallBackType(typeID ID) : TType(ID) {assert(TLCOMPOSIT_TYPE(ID));}
      virtual             ~TCallBackType(){}
      void                 pushArg(typeID);
      void                 setFType(typeID ft)    {_fType = ft; }
      typeID               fType() const          {return _fType;}
      const TypeIdList&    paramList() const      {return _paramList;}
      virtual bool         isComposite() const    {return false;}
   private:
      typeID               _fType; //! Function return type
      TypeIdList           _paramList;
   };

   //==============================================================================
   class TPointType : public TCompType {
   public:
                           TPointType();
   };

   //==============================================================================
   class TBoxType : public TCompType {
   public:
                           TBoxType(TPointType*);
   };

   //==============================================================================
   class TBindType : public TCompType {
   public:
                           TBindType(TPointType*);
   };

   //==============================================================================
   class THshType : public TCompType {
      public:
                           THshType();
   };

    //==============================================================================
   class THshStrType : public TCompType {
      public:
                           THshStrType();
   };

   //==============================================================================
   class TellVar {
   public:
                           TellVar(typeID ID) : _ID(ID), _changeable(2) {}
      virtual TellVar*     selfcopy() const = 0;
      virtual void         echo(std::string&, real) = 0;
      virtual const typeID get_type() const {return _ID;}
      virtual void         assign(TellVar*) = 0;
      virtual TellVar*     field_var(char*& fname) {return NULL;}
      virtual TellVar*     index_var(unsigned index) {return NULL;}
      virtual void         initialize() = 0;
      void                 update_cstat() {if (1 == _changeable) _changeable = 0;}
      bool                 constant() const {return 0 == _changeable;}
      void                 const_declaration() {_changeable = 1;}
      virtual             ~TellVar() {};
   protected:
      typeID              _ID;
      byte                _changeable;
   };

   //==============================================================================
   class TtReal:public TellVar {
   public:
                           TtReal(real  num=0.0) : TellVar(tn_real), _value(num) {}
                           TtReal(const TtReal& cobj) :
                                          TellVar(tn_real), _value(cobj.value()) {}
      const TtReal&        operator =(const TtReal&);
      const TtReal&        operator =(const TtInt&);
      virtual void         initialize() {_value = 0.0;}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      real                 value() const        {return _value;};
      void                 uminus()             {_value  = -_value;   };
      virtual TellVar*     selfcopy() const     {return DEBUG_NEW TtReal(_value);};
      friend class TtPnt;
   private:
      real  _value;
   };

   //==============================================================================
   class TtInt:public TellVar {
   public:
                           TtInt(int4b  num = 0) : TellVar(tn_int), _value(num) {}
                           TtInt(const TtInt& cobj) :
                                          TellVar(tn_int), _value(cobj.value()) {}
      const TtInt&         operator =(const TtInt&);
      const TtInt&         operator =(const TtReal&);
      virtual void         initialize() {_value = 0;}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      int4b                value() const        {return _value;};
      void                 uminus()             {_value  = -_value;   };
      virtual TellVar*     selfcopy() const     {return DEBUG_NEW TtInt(_value);};
      void                 NOT()                {_value = ~_value;}
   protected:
      int4b               _value;
   };

   //==============================================================================
   class TtBool:public TellVar {
   public:
                           TtBool(bool value = false) :
                                                TellVar(tn_bool), _value(value) {}
                           TtBool(const TtBool& cobj) :
                                         TellVar(tn_bool), _value(cobj.value()) {};
      const TtBool&        operator = (const TtBool&);
      virtual void         initialize() {_value = false;}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      bool                 value() const        {return _value;};
      virtual TellVar*     selfcopy() const     {return DEBUG_NEW TtBool(_value);};
      void                 AND(bool op)         {_value = _value && op;};
      void                 OR(bool op)          {_value = _value || op; };
      void                 NOT()                {_value = !_value;}
   private:
      bool  _value;
   };

   //==============================================================================
   class TtString: public TellVar {
   public:
                           TtString() : TellVar(tn_string) {}
                           TtString(const std::string& value):
                                              TellVar(tn_string), _value(value) {};
                           TtString(const TtString& cobj) :
                                        TellVar(tn_string), _value(cobj.value()){};
      const TtString&      operator = (const TtString&);
      virtual void         initialize() {_value = "";}
      virtual void         echo(std::string&, real);// {wstr += _value;};
      virtual void         assign(TellVar*);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtString(_value);}
      const std::string    value() const       {return _value;};
   private:
      std::string         _value;
   };

   //==============================================================================
   class TtLayout: public TellVar {
   public:
                           TtLayout(): TellVar(tn_layout), _data(NULL),
                                                      _layer(ERR_LAY), _selp(NULL) {};
                           TtLayout(laydata::TdtData* pdat, unsigned lay, SGBitSet* selp = NULL):
                             TellVar(tn_layout), _data(pdat), _layer(lay), _selp(selp) {};
                           TtLayout(const TtLayout& cobj);
      const TtLayout&      operator = (const TtLayout&);
      virtual void         initialize() {if (_selp) delete _selp;_data = NULL;}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      virtual TellVar*     selfcopy() const {return DEBUG_NEW TtLayout(*this);};
      laydata::TdtData*    data() const     {return _data;};
      unsigned             layer() const    {return _layer;};
      SGBitSet*            selp() const     {return _selp;};
      virtual             ~TtLayout()       {if (_selp) delete _selp;}
   private:
      laydata::TdtData*    _data;
      unsigned             _layer;
      SGBitSet*            _selp; // selected points;
   };

   //==============================================================================
   class TtAuxdata: public TellVar {
   public:
                           TtAuxdata(): TellVar(tn_auxilary), _data(NULL),
                             _layer(ERR_LAY) {};
                           TtAuxdata(auxdata::TdtAuxData* pdat, unsigned lay):
                             TellVar(tn_auxilary), _data(pdat), _layer(lay) {};
                           TtAuxdata(const TtAuxdata& cobj);
      const TtAuxdata&     operator = (const TtAuxdata&);
      virtual void         initialize() {_data = NULL;}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      virtual TellVar*     selfcopy() const {return DEBUG_NEW TtAuxdata(*this);};
      auxdata::TdtAuxData* data() const     {return _data;};
      unsigned             layer() const    {return _layer;};
   private:
      auxdata::TdtAuxData* _data;
      unsigned             _layer;
   };

   //==============================================================================
   class TtList:public TellVar {
   public:
                           TtList(typeID ltype): TellVar(ltype) {};
                           TtList(const TtList& cobj);
      const TtList&        operator = (const TtList&);
      virtual void         initialize();
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      virtual TellVar*     selfcopy() const  {return DEBUG_NEW TtList(*this);}
      virtual const typeID get_type() const  {return _ID | tn_listmask;}
      memlist              mlist() const     {return _mlist;}
      void                 add(TellVar* p) {_mlist.push_back(p);}
      void                 reserve(unsigned num) {_mlist.reserve(num);}
      void                 resize(unsigned num, TellVar* initVar);
      void                 reverse()         {std::reverse(_mlist.begin(), _mlist.end());}
      unsigned             size() const      {return _mlist.size();}
      virtual TellVar*     index_var(dword);
      bool                 validIndex(dword);
      void                 insert(telldata::TellVar*, dword);
      void                 insert(telldata::TellVar*);
      void                 lunion(telldata::TtList*, dword);
      void                 lunion(telldata::TtList*);
      TellVar*             erase(dword);
      TellVar*             erase(dword, dword);
      virtual             ~TtList();
   private:
      memlist             _mlist;    // the list itself
   };

   //==============================================================================
   class TtUserStruct : public TellVar {
   public:
                           TtUserStruct(const typeID ID) : TellVar(ID) {};
                           TtUserStruct(const TCompType*);
                           TtUserStruct(const TCompType*, operandSTACK&);
                           TtUserStruct(const TtUserStruct&);
      virtual             ~TtUserStruct();
      virtual void         initialize();
      virtual TellVar*     selfcopy() const  {return DEBUG_NEW TtUserStruct(*this);}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      virtual TellVar*     field_var(char*& fname);
   protected:
      recfieldsNAME        _fieldList;
   };

   //==============================================================================
   // Don't destruct _x and _y here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtPnt : public TtUserStruct {
   public:
                           TtPnt (real x=0, real y=0);
                           TtPnt(const TtPnt&);
                           TtPnt(operandSTACK& OPStack);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtPnt(*this);}
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      const real           x() const           {return _x->value();}
      const real           y() const           {return _y->value();}
      void                 scale(real sf)      {_x->_value *= sf;_y->_value *= sf;};
      void                 set_x(const real x) {_x->_value = x; }
      void                 set_y(const real y) {_y->_value = y; }
      const TtPnt&         operator = (const TtPnt&);
   private:
      TtReal*              _x;
      TtReal*              _y;
   };

   //==============================================================================
   // Don't destruct _p1 and _p1 here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtWnd : public TtUserStruct {
   public:
                           TtWnd( real bl_x=0.0, real bl_y=0.0,
                                  real tr_x=0.0, real tr_y=0.0);
                           TtWnd( TtPnt tl, TtPnt br);
                           TtWnd(operandSTACK& OPStack);
                           TtWnd(const TtWnd& cobj);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtWnd(*this);};
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      const TtPnt&         p1() const          {return *_p1;};
      const TtPnt&         p2() const          {return *_p2;};
      void                 scale(real sf)      {_p1->scale(sf); _p1->scale(sf);};
      const TtWnd&         operator = (const TtWnd&);
      void                 normalize(bool&, bool&);
      void                 denormalize(bool, bool);
   private:
      TtPnt*               _p1;
      TtPnt*               _p2;
   };

   //==============================================================================
   // Don't destruct _p, _rot etc. here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtBnd : public TtUserStruct {
   public:
                           TtBnd( real p_x=0.0, real p_y=0.0,
                                  real rot=0.0, bool flx=false, real scale = 1.0);
                           TtBnd( TtPnt p, TtReal rot, TtBool flx, TtReal sc);
                           TtBnd(operandSTACK& OPStack);
                           TtBnd(const TtBnd& cobj);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtBnd(*this);};
      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      const TtPnt&         p() const           {return *_p;}
      const TtReal&        rot() const         {return *_rot;}
      const TtBool&        flx() const         {return *_flx;}
      const TtReal&        sc() const          {return *_sc;}
      const TtBnd&         operator = (const TtBnd&);
   private:
      TtPnt*               _p;
      TtReal*              _rot;
      TtBool*              _flx;
      TtReal*              _sc;
   };

   //==============================================================================
   // Don't destruct _number and _name here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtHsh : public TtUserStruct {
      public:
                              TtHsh (int4b number=1, std::string name = "");
                              TtHsh(const TtHsh&);
                              TtHsh(operandSTACK& OPStack);
         virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtHsh(*this);}
         virtual void         echo(std::string&, real);
         virtual void         assign(TellVar*);
         const TtInt&         key()   const        {return *_key;}
         const TtString&      value() const        {return *_value;}
//         void                 set_number(const int4b number)   {_number->_value = number; }
//         void                 set_name(const std::string name) {_name->_value = name; }
      private:
         TtInt*               _key;
         TtString*            _value;
   };

      //==============================================================================
   // Don't destruct _number and _name here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtHshStr : public TtUserStruct {
      public:
                              TtHshStr (std::string number="", std::string name = "");
                              TtHshStr(const TtHshStr&);
                              TtHshStr(operandSTACK& OPStack);
         virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtHshStr(*this);}
         virtual void         echo(std::string&, real);
         virtual void         assign(TellVar*);
         const TtString&      key()   const        {return *_key;}
         const TtString&      value() const        {return *_value;}
//         void                 set_number(const int4b number)   {_number->_value = number; }
//         void                 set_name(const std::string name) {_name->_value = name; }
      private:
         TtString*            _key;
         TtString*            _value;
   };

   //==============================================================================
   class ArgumentID {
   public:
                           ArgumentID(telldata::typeID ID = telldata::tn_NULL) :
                                                          _ID(ID), _command(NULL){};
                           ArgumentID(argumentQ* child, void* cmd) :
                                                         _ID(telldata::tn_composite),
                                                    _child(*child), _command(cmd) {};
                           ArgumentID(const ArgumentID&);
                           ArgumentID(void* cmd) : _ID(telldata::tn_anyfref),
                                                   _command(cmd) {};
                           ~ArgumentID();//               {_child.clear();}
      void                 toList(bool, typeID alistID = tn_NULL);
      void                 adjustID(const ArgumentID&);
      void                 userStructCheck(const telldata::TType*, bool);
      void                 userStructListCheck(const telldata::TType*, bool);
      telldata::typeID     operator () () const        {return _ID;}
      const argumentQ&     child() const               {return _child;}
   private:
      telldata::typeID     _ID;
      argumentQ            _child;
      void*                _command;
   };



   void argQClear(argumentQ*);
   std::string echoType( const telldata::typeID, const telldata::typeMAP*);
}

#endif
