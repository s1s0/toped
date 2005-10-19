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

#ifndef CTM_H
#define CTM_H

#include <string>
#include <vector>
#include <algorithm>
#include "../tpd_common/common.h"
#include "../tpd_DB/tedat.h"
#include "../tpd_common/ttt.h"

#define NUMBER_TYPE(op) ((op > telldata::tn_void) && (op < telldata::tn_bool) && !(op & telldata::tn_listmask))
#define TLISTOF(op) (op | telldata::tn_listmask)
#define TLUSER_TYPE(op) (op > telldata::tn_layout)

//=============================================================================
// TELL types
//=============================================================================
namespace telldata {   
   typedef unsigned int typeID;
   const typeID tn_void       = 0 ;
   const typeID tn_int        = 1 ;
   const typeID tn_real       = 2 ;
   const typeID tn_bool       = 3 ;
   const typeID tn_string     = 4 ;
   const typeID tn_pnt        = 5 ;
   const typeID tn_box        = 6 ;
   const typeID tn_layout     = 7 ;
   const typeID tn_usertypes  = 10;
   const typeID tn_listmask = typeID(1) << (8 * sizeof(typeID) - 1);

   class tell_var;
   class tell_type;
   class ttint;
   
   typedef std::map<std::string, typeID    > recfieldsMAP;
   typedef std::map<std::string, tell_type*> typeMAP;
   typedef std::map<std::string, tell_var* > variableMAP;
   typedef std::vector<tell_var*> memlist;


   class tell_var {
   public:
                           tell_var(typeID ID) : _ID(ID) {}
      virtual tell_var*    selfcopy() const = 0;
      virtual void         echo(std::string&) = 0;
      virtual const typeID get_type() const {return _ID;}
      virtual void         set_value(tell_var*) = 0;
      virtual             ~tell_var() {};
   protected:
      typeID              _ID;
   };

   class ttreal:public tell_var {
   public:
                           ttreal(real  num=0.0) : tell_var(tn_real), _value(num) {}
                           ttreal(const ttreal& cobj) : tell_var(tn_real), _value(cobj.value()) {};
      const ttreal&        operator =(const ttreal&);
      const ttreal&        operator =(const ttint&);
      void                 echo(std::string&);
      void                 set_value(tell_var*);
      real                 value() const        {return _value;};
      void                 uminus()             {_value  = -_value;   };
      tell_var*            selfcopy() const     {return new ttreal(_value);};
   private:
      real  _value;
   };

   class ttint:public tell_var {
   public:
                         ttint(int4b  num = 0) : tell_var(tn_int), _value(num) { }
                         ttint(const ttint& cobj) : tell_var(tn_int), _value(cobj.value()) {};
      const ttint&       operator =(const ttint&);
      const ttint&       operator =(const ttreal&);
      void               echo(std::string&);
      void               set_value(tell_var*);
      int4b              value() const        {return _value;};
      void               uminus()             {_value  = -_value;   };
      tell_var*          selfcopy() const     {return new ttint(_value);};
   private:
      int4b  _value;
   };

   class ttbool:public tell_var {
   public:
                         ttbool(bool value = false) : tell_var(tn_bool), _value(value) {}
                         ttbool(const ttbool& cobj) : tell_var(tn_bool), _value(cobj.value()) {};
      const ttbool&      operator = (const ttbool&);
      void               echo(std::string&);
      void               set_value(tell_var*);
      bool               value() const        {return _value;};
      tell_var*          selfcopy() const     {return new ttbool(_value);};
      void               AND(bool op)         {_value = _value && op;};
      void               OR(bool op)          {_value = _value || op; };
   private:
      bool  _value;
   };

   class ttpnt:public tell_var {
   public:
                         ttpnt( real xpos=0, real ypos=0) : tell_var(tn_pnt), _x(xpos), _y(ypos) {}
                         ttpnt(const ttpnt& cobj) : tell_var(tn_pnt), _x(cobj.x()), _y(cobj.y()) {};
      const ttpnt&       operator = (const ttpnt&);
      const ttpnt&       operator *= (CTM op2);
      bool               operator == (ttpnt op2) {return ((_x == op2.x()) && (_y==op2.y()));};
      friend const ttpnt operator*( const ttpnt &, CTM & );
      friend const ttpnt operator-( const ttpnt &, ttpnt & );
      void               echo(std::string&);
      void               set_value(tell_var*);
      real               x() const           {return _x;};
      real               y() const           {return _y;};
      tell_var*          selfcopy() const    {return new ttpnt(_x, _y);};
      void               scale(real sf)      {_x *= sf;_y *= sf;};
   private:
      real     _x;
      real     _y;
   };


   class ttwnd:public tell_var {
   public:   
                         ttwnd( real bl_x=0.0, real bl_y=0.0, 
                           real tr_x=0.0, real tr_y=0.0) : tell_var(tn_box), _p1(bl_x,bl_y),
                                                           _p2(tr_x, tr_y) {};
                         ttwnd( ttpnt tl, ttpnt br) : tell_var(tn_box), _p1(tl),_p2(br) {};
                         ttwnd(const ttwnd& cobj) : tell_var(tn_box), 
                                          _p1(cobj.p1()), _p2(cobj.p2()) {};
      const ttwnd&       operator = (const ttwnd&);
      void               echo(std::string&);
      void               set_value(tell_var*);
      const ttpnt&       p1() const          {return _p1;};
      const ttpnt&       p2() const          {return _p2;};
      tell_var*          selfcopy() const    {return new ttwnd(_p1, _p2);};
      void               scale(real sf)      {_p1.scale(sf); _p1.scale(sf);};
   private:
      ttpnt _p1;
      ttpnt _p2;
   };

   class ttstring: public tell_var {
   public:
                         ttstring() : tell_var(tn_string) {}
                         ttstring(char* value) : tell_var(tn_layout), _value(value) {}
                         ttstring(const std::string& value): tell_var(tn_string), _value(value) {};
                         ttstring(const ttstring& cobj) : tell_var(tn_string), _value(cobj.value()){};
      const ttstring&    operator = (const ttstring&);
      void               echo(std::string& wstr) {wstr = _value;};
      void               set_value(tell_var*);
      tell_var*          selfcopy() const    {return new ttstring(_value);}
      const std::string  value() const       {return _value;};
   private:
      std::string        _value;
   };

   class ttlayout: public tell_var {
   public:
                        ttlayout(): tell_var(tn_layout), _data(NULL), _layer(65535), _selp(NULL) {};
                        ttlayout(laydata::tdtdata* pdat, word lay, 
                         SGBitSet* selp = NULL): tell_var(tn_layout), _data(pdat), _layer(lay),
                                                                _selp(selp) {};
                        ttlayout(const ttlayout& cobj);
      const ttlayout&   operator = (const ttlayout&);                        
      void              echo(std::string& wstr);
      void              set_value(tell_var*);
      tell_var*         selfcopy() const {return new ttlayout(*this);};
      laydata::tdtdata* data() const     {return _data;};
      word              layer() const    {return _layer;};
      SGBitSet*         selp() const     {return _selp;};
                       ~ttlayout()       {if (_selp) delete _selp;};
   private:
      laydata::tdtdata* _data;
      word              _layer;
      SGBitSet*         _selp; // selected points;
   };

   class ttlist:public tell_var {
   public:
                        ttlist(typeID ltype): tell_var(ltype) {};
                        ttlist(const ttlist& cobj);
      const ttlist&     operator = (const ttlist&);
      void              echo(std::string&);
      void              set_value(tell_var*);
      tell_var*         selfcopy() const  {return new ttlist(*this);};
      const typeID      get_type() const  {return _ID | tn_listmask;};
      memlist           mlist() const     {return _mlist;};
      void              add(tell_var* p) {_mlist.push_back(p);};
      void              reserve(unsigned num) {_mlist.reserve(num);};
      void              reverse()         {std::reverse(_mlist.begin(), _mlist.end());};
      unsigned          size() const      {return _mlist.size();};
                       ~ttlist();
   private:
      memlist           _mlist;    // the list itself
   };
/*Every block (parsercmd::cmdBLOCK) defined maintains a table (map) to the locally defined user types
in a form <typename - tell_type>. Every user type (telldata::tell_type), maintains a table (map) to the
user defined types (telldata::tell_type) used in this type in a form <ID - tell_type. The latter is
updated by addfield method. Thus the tell_type can execute its own copy constructor*/
   class tell_type {
   public:
                           tell_type(typeID ID) : _ID(ID) {assert(TLUSER_TYPE(ID));}
      bool                 addfield(std::string, typeID, const tell_type* utype);
      const recfieldsMAP&  fields() const {return _fields;}
      const typeID         ID() const {return _ID;}
      tell_var*            initfield(const typeID) const;
      typedef std::map<typeID, const tell_type*> typeIDMAP;
   protected:
      typeID               _ID;
      recfieldsMAP         _fields;
      typeIDMAP            _tIDMAP;
   };

   class user_struct : public tell_var {
   public:
                           user_struct(const tell_type*);
                           user_struct(const user_struct&);
      tell_var*            selfcopy() const  {return new user_struct(*this);}
      void                 echo(std::string&);
      void                 set_value(tell_var*);
   protected:
      variableMAP         _fieldmap;
   };

}

#endif
/*   typedef enum {
      tn_void        = 0x00,
      tn_num_mask    = 0x10,  // number mask
      tn_int         = 0x11,
      tn_real        = 0x12,
      tn_other_mask  = 0x20,  // non arithmetic type mask
      tn_bool        = 0x21,
      tn_string      = 0x22,
   //   tn_struct_mask = 0x40,  // data structure mask
      tn_pnt         = 0x41,
      tn_box         = 0x42,
      tn_layout      = 0x48,
      //---------------------
      tn_list_mask   = 0x80,
      tn_int_list    = 0x91,
      tn_real_list   = 0x92,
      tn_bool_list   = 0xA1,
      tn_string_list = 0xA2,
      tn_pnt_list    = 0xC1,
      tn_box_list    = 0xC2,
      tn_layout_list = 0xC8 
   } type;*/
   
