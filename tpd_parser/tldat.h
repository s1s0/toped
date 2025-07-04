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

#define NUMBER_TYPE(op) ((op > telldata::tn_void) && (op < telldata::tn_bool ))
#define INTEGER_TYPE(op) ((op == telldata::tn_uint) || (op == telldata::tn_int ))
#define PRINTF_ABLE(op) ((op > telldata::tn_void) && (op < telldata::tn_layout))
#define TLISTOF(op) (op | telldata::tn_listmask)
#define TLISALIST(op) (op & telldata::tn_listmask)
#define TLCOMPOSIT_TYPE(op) (op > telldata::tn_composite)
#define TLUNKNOWN_TYPE(op) ((op == telldata::tn_composite) || (op == telldata::tn_anyfref))

namespace laydata {
   class TdtData;
}

namespace auxdata {
   class GrcData;
}
//=============================================================================
// TELL types
//=============================================================================
namespace telldata {
   typedef unsigned int typeID;
   const typeID tn_NULL        =  0;//
   const typeID tn_void        =  1;//
   const typeID tn_uint        =  2;// number type
   const typeID tn_int         =  3;// number type
   const typeID tn_real        =  4;// number type
   const typeID tn_bool        =  5;//
   const typeID tn_string      =  6;//
   const typeID tn_layout      =  7;//
   const typeID tn_auxilary    =  8;//
   const typeID tn_anyfref     =  9;//
   const typeID tn_composite   = 10;// from this position on - only composites
   const typeID tn_pnt         = 11;//
   const typeID tn_box         = 12;//
   const typeID tn_bnd         = 13;//
   const typeID tn_laymap      = 14;//
   const typeID tn_hshstr      = 15;//
   const typeID tn_layer       = 16;//
   const typeID tn_usertypes   = 17;//
   // the most significant bit is a mask flag
   const typeID tn_listmask = typeID(1) << (8 * sizeof(typeID) - 1);

   class TellVar;
   class TType;
   class TtInt;
   class TtList;
   class ArgumentID;

   struct UserTypeField {
                    UserTypeField (std::string fName, typeID fType, unsigned lSize):
                       _fName(fName),
                       _fType(fType),
                       _lSize(lSize){if (!TLISALIST(_fType)) assert(0 == lSize);}
      std::string   _fName;
      typeID        _fType;
      unsigned      _lSize; // initial list size
   };

   typedef std::pair<std::string, TellVar*>  structRECNAME;
   typedef std::list<UserTypeField>          recfieldsID;
   typedef std::list<structRECNAME>          recfieldsNAME;
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
     (telldata::TCompType) used in this type in a form <ID - TCompType>. The latter is
     updated by addfield method. Thus the TtUserStruct can execute its own copy
     constructor
   */
   class TType {
      public:
                              TType(typeID ID) : _ID(ID) {}
         virtual             ~TType(){}
         typeID               ID() const        {return _ID;}
         virtual bool         isComposite() const = 0;
      protected:
         typeID               _ID;
   };
   //! Tell Composite Type
   class TCompType : public TType {
   public:
                           TCompType(typeID ID) : TType(ID) {assert(TLCOMPOSIT_TYPE(ID));}
      virtual             ~TCompType(){}
      bool                 addfield(std::string, typeID, const TType*, unsigned lSize = 0);
      TellVar*             initfield(const typeID, unsigned lSize) const;
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
                           TCallBackType(typeID ID) : TType(ID), _fType(tn_NULL) {assert(TLCOMPOSIT_TYPE(ID));}
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
   class TLayerType : public TCompType {
      public:
                           TLayerType();
   };

   //==============================================================================
   class TLMapType : public TCompType {
      public:
                           TLMapType(TLayerType*);
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
      virtual typeID       get_type() const {return _ID;}
      virtual void         assign(TellVar*) = 0;
      virtual TellVar*     field_var(char*& /*event*/) {return NULL;}
      virtual TellVar*     index_var(dword /*event*/) {return NULL;}
      virtual TtList*      index_range_var(dword, dword) {return NULL;}
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
      virtual void         assign(TellVar*);
      int4b                value() const        {return _value;};
      void                 uminus()             {_value  = -_value;   };
      virtual TellVar*     selfcopy() const     {return DEBUG_NEW TtInt(_value);};
      void                 NOT()                {_value = ~_value;}
      friend class TtLayer;
   private:
      int4b               _value;
   };

   //==============================================================================
   class TtUInt:public TellVar {
   public:
                           TtUInt(int4b  num = 0) : TellVar(tn_uint), _value(num) {}
                           TtUInt(const TtUInt& cobj) :
                                          TellVar(tn_uint), _value(cobj.value()) {}
      const TtUInt&        operator =(const TtUInt&);
      const TtUInt&        operator =(const TtReal&);
      virtual void         initialize() {_value = 0;}
      virtual void         assign(TellVar*);
      dword                value() const        {return _value;};
//      void                 uminus()             {_value  = -_value;   };
      virtual TellVar*     selfcopy() const     {return DEBUG_NEW TtUInt(_value);};
      void                 NOT()                {_value = ~_value;}
//      friend class TtLayer;
   private:
      dword               _value;
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
      virtual void         assign(TellVar*);
      void                 part_assign(dword,TtString*);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtString(_value);}
      const std::string    value() const       {return _value;}
      unsigned            length() const     {return static_cast<unsigned>(_value.length());}
   private:
      std::string         _value;
   };

   //==============================================================================
   class TtLayout: public TellVar {
   public:
                           TtLayout(): TellVar(tn_layout), _data(NULL),
                                                      _layer(ERR_LAY_DEF), _selp(NULL) {};
                           TtLayout(laydata::TdtData* pdat, const LayerDef& laydef, SGBitSet* selp = NULL):
                             TellVar(tn_layout), _data(pdat), _layer(laydef), _selp(selp) {};
                           TtLayout(const TtLayout& cobj);
      const TtLayout&      operator = (const TtLayout&);
      virtual void         initialize() {if (_selp) delete _selp;_data = NULL;}
      virtual void         assign(TellVar*);
      virtual TellVar*     selfcopy() const {return DEBUG_NEW TtLayout(*this);};
      laydata::TdtData*    data() const     {return _data;};
      LayerDef             layer() const    {return _layer;};
      SGBitSet*            selp() const     {return _selp;};
      virtual             ~TtLayout()       {if (_selp) delete _selp;}
   private:
      laydata::TdtData*    _data;
      LayerDef             _layer;
      SGBitSet*            _selp; // selected points;
   };

   //==============================================================================
   class TtAuxdata: public TellVar {
   public:
                           TtAuxdata(): TellVar(tn_auxilary), _data(NULL),
                             _layer(ERR_LAY_DEF) {};
                           TtAuxdata(auxdata::GrcData* pdat, const LayerDef& laydef):
                             TellVar(tn_auxilary), _data(pdat), _layer(laydef) {};
                           TtAuxdata(const TtAuxdata& cobj);
      const TtAuxdata&     operator = (const TtAuxdata&);
      virtual void         initialize() {_data = NULL;}
      virtual void         assign(TellVar*);
      virtual TellVar*     selfcopy() const {return DEBUG_NEW TtAuxdata(*this);};
      auxdata::GrcData* data() const     {return _data;};
      LayerDef             layer() const    {return _layer;};
   private:
      auxdata::GrcData* _data;
      LayerDef             _layer;
   };

   //==============================================================================
   class TtList:public TellVar {
   public:
                           TtList(typeID ltype): TellVar(ltype) {};
                           TtList(const TtList& cobj);
      const TtList&        operator = (const TtList&);
      virtual void         initialize();
      virtual void         assign(TellVar*);
      bool                 part_assign(dword, dword, const TtList*);
      virtual TellVar*     selfcopy() const  {return DEBUG_NEW TtList(*this);}
      virtual typeID       get_type() const  {return _ID | tn_listmask;}
      memlist              mlist() const     {return _mlist;}
      void                 add(TellVar* p) {_mlist.push_back(p);}
      void                 reserve(unsigned num) {_mlist.reserve(num);}
      void                 resize(unsigned num, TellVar* initVar);
      void                 reverse()         {std::reverse(_mlist.begin(), _mlist.end());}
      unsigned             size() const      {return static_cast<unsigned>(_mlist.size());}
      virtual TellVar*     index_var(dword);
      virtual TtList*      index_range_var(dword, dword);
      bool                 validIndex(dword);
      void                 insert(telldata::TellVar*, dword);
      void                 insert(telldata::TellVar*);
      void                 lunion(telldata::TtList*, dword);
      void                 lunion(telldata::TtList*);
      TellVar*             erase(dword);
      TellVar*             erase(dword, dword);
//      unsigned long        size() {return _mlist.size();}
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
      virtual void         assign(TellVar*);
      virtual TellVar*     field_var(char*& fname);
      const recfieldsNAME& fieldList() {return _fieldList;}
   protected:
      recfieldsNAME        _fieldList;
   };

   //==============================================================================
   // Don't destruct _x and _y here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtLayer : public TtUserStruct {
   public:
                           TtLayer (const LayerDef& = TLL_LAY_DEF);
                           TtLayer(const TtLayer&);
                           TtLayer(operandSTACK& OPStack);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtLayer(*this);}
      virtual void         assign(TellVar*);
      word                 num() const             {return _num->value();}
      word                 typ() const             {return _typ->value();}
      LayerDef             value() const           {return LayerDef(_num->value(), _typ->value());}
      void                 set_num(const word num) {_num->_value = num; }
      void                 set_typ(const word typ) {_typ->_value = typ; }
      const TtLayer&       operator = (const TtLayer&);
   private:
      TtInt*              _num;
      TtInt*              _typ;
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
      virtual void         assign(TellVar*);
      real                 x() const           {return _x->value();}
      real                 y() const           {return _y->value();}
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
   class TtBox : public TtUserStruct {
   public:
                           TtBox( real bl_x=0.0, real bl_y=0.0,
                                  real tr_x=0.0, real tr_y=0.0);
                           TtBox( TtPnt tl, TtPnt br);
                           TtBox(operandSTACK& OPStack);
                           TtBox(const TtBox& cobj);
      virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtBox(*this);};
      virtual void         assign(TellVar*);
      const TtPnt&         p1() const          {return *_p1;};
      const TtPnt&         p2() const          {return *_p2;};
      void                 scale(real sf)      {_p1->scale(sf); _p1->scale(sf);};
      const TtBox&         operator = (const TtBox&);
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
   // Don't destruct _layer and _value here. They are just pointing to the structures in
   // the parent _fieldList and obviously should be destroyed there
   class TtLMap : public TtUserStruct {
      public:
                              TtLMap (const LayerDef& laydef = TLL_LAY_DEF, std::string value = "");
                              TtLMap(const TtLMap&);
                              TtLMap(operandSTACK& OPStack);
         virtual TellVar*     selfcopy() const    {return DEBUG_NEW TtLMap(*this);}
         virtual void         assign(TellVar*);
         const TtLayer&       layer() const        {return *_layer;}
         const TtString&      value() const        {return *_value;}
//         void                 set_number(const int4b number)   {_number->_value = number; }
//         void                 set_name(const std::string name) {_name->_value = name; }
      private:
         TtLayer*             _layer;
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
