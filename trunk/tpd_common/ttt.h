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
//  Creation date: Mon Apr 28 22:09:54 BST 2003
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

#ifndef TTT_H_INCLUDED
#define TTT_H_INCLUDED

#include <vector>
#include <map>
#include <stack>
#include <list>
#include <assert.h>

#if WIN32
   #include <windows.h>
   #define rint floor
   #define remainder fmod
   #pragma warning( disable : 4786 ) 
#endif
//#include "config.h"
//#define MIN_X        (int4b)0x80000001      //  -2 147 483 647
//#define MAX_X        (int4b)0x7FFFFFFF      //   2 147 483 643
//#define MIN_Y        (int4b)0x80000001      //  -2 147 483 647
//#define MAX_Y        (int4b)0x7FFFFFFF      //   2 147 483 647
#define OPENGL_FONT_UNIT   119.05
#define MIN_VISUAL_AREA    10   // that would be 10 pixels
#define MAX_OVL_BOX  DBbox(MIN_X,MAX_X,MIN_Y,MIN_Y) // maximum overlapping box
#define DEFAULT_OVL_BOX DBbox(TP(0,0))
#define DEFAULT_ZOOM_BOX DBbox(TP(-2000,-2000), TP(20000,20000))

#if (SIZEOF_TIME_T == 8)
   #define TIME_TPD unsigned int
#else
   #define TIME_TPD time_t
#endif
//=============================================================================
// General type declations (compatability)
//=============================================================================
typedef          char   _sg_int8;	// 1 byte
typedef         short   _sg_int16;	// 2 bytes
typedef           int   _sg_int32;	// 4 bytes

typedef unsigned char   byte;
#define MAX_BYTE_VALUE  255
typedef unsigned short  word;
#define MAX_WORD_VALUE  65535
typedef unsigned long   _dbl_word;
typedef     _sg_int16   int2b;
typedef     _sg_int32   int4b;
typedef        double   real;

//==============================================================================
class SGBitSet {
public:
            SGBitSet(word);
            SGBitSet(SGBitSet*);
   void     set(word);
   void     reset(word);
   void     setall();
   void     check_neighbours_set(bool wire);
   bool     check(word) const;
   bool     isallclear() const;
   bool     isallset() const;
   word     size() const {return _size;};
           ~SGBitSet();
private:
            SGBitSet(const SGBitSet&);
   word     _size;
   byte*    _packet;
};

//==============================================================================   
/*** CTM *********************************************************************
  Current Translation Matrix
>>> Constructor --------------------------------------------------------------
  Apart from the default one two additional constructors available:
   - For direct initializing (6 matrix positions)
   - Initializing with scale, rotation, flip etc.
>>> Data fields --------------------------------------------------------------
> a               - Matrix positions of type double
> b               -
> c               - //  |a   b   0|
> d               - //  |c   d   0|
> tx              - //  |tx  ty  1|
> ty              -
>>> Methods ------------------------------------------------------------------
> Translate()     - Move CTM to coords X,Y
> Scale()         - Scale CTM by X and Y
> Rotate()        - Rotate by angle alfa anticlockwise
> FlipX()         - flip the image towards given X line
> FlipY()         - flip the image towards given Y line
> Reverse()       - return 1/matrix (not sure about the proper terminology)
>>> Operators ----------------------------------------------------------------
> PSCTM	operator *  (PSCTM op2) Multiplication CTM = CTM1*CTM2
> PSCTM	operator *= (PSCTM op2) Multiplication assignemt CTM *= CTM1
> PSP		operator *  (GP* op1)   Multiplication PSP = CTM*GP
> PSP		operator *  (PSP* op1)  Multiplication PSP = CTM*PSP
******************************************************************************/
class TP;
class   CTM
{
public:
   CTM() {Initialize();};
   void Initialize()                {_a = _d = 1.0;_b = _c = _tx = _ty = 0.0;};
   CTM(TP dp, real scale, real rotation, bool reflX);
   CTM(real va,real vb,real vc,real vd,real vtx,real vty) :
    _a(va), _b(vb), _c(vc), _d(vd), _tx(vtx), _ty(vty) {};
//   CTM(const CTM& mtrx) { *this = *mtrx;};
   CTM  Translate(real X, real Y) {return (*this *= CTM(1,0,0,1,X,Y));};
   CTM  Scale(real X, real Y)     {return (*this *= CTM(X,0,0,Y,0,0));};
   CTM  FlipX(real Y=0)           {return (*this *= CTM(1,0,0,-1,0,2*Y));};
   CTM  FlipY(real X=0)           {return (*this *= CTM(-1,0,0,1,2*X,0));};
   CTM  Rotate(real alfa);
   CTM  Reversed() const;
   void setCTM(real a, real b, real c, real d, real tx, real ty)
                          {_a = a; _b = b; _c = c; _d = d; _tx = tx; _ty = ty;};
   CTM  operator =  (const CTM op2);
   CTM  operator *  (const CTM op2) const;
   CTM  operator *= (const CTM op2)         {return (*this = *this * op2);};
   real a()        const            {return _a;};
   real b()        const            {return _b;};
   real c()        const            {return _c;};
   real d()        const            {return _d;};
   real tx()       const            {return _tx;};
   real ty()       const            {return _ty;};
private:
   real  _a,_b,_c,_d,_tx,_ty;
};

//==============================================================================   
class TP {
public:
   friend class DBbox;
   TP(int4b x=0, int4b y=0): _x(x), _y(y) {};
   TP(real, real, real);
   void     roundTO(int4b step);
   void     move(int4b dX, int4b dY) {_x += dX; _y += dY;};
   void     info(std::ostringstream&) const;
   int4b    x()  const {return _x;};
   int4b    y()  const {return _y;};
   TP operator * ( const CTM& ) const;
   TP operator *= ( const CTM& );
   TP operator = ( const TP& np)        {_x = np.x(); _y = np.y(); return *this;};
   TP operator - ( const TP& pnt) const {return TP(_x - pnt.x(),_y - pnt.y());};
   TP operator + ( const TP& pnt) const {return TP(_x + pnt.x(),_y + pnt.y());};
   bool operator == (const TP& np) const{return ((np.x() == _x) && (np.y() == _y));};
   bool operator != (const TP& np) const{return ((np.x() != _x) || (np.y() != _y));};
   void     setX(const int4b x) {_x = x;};
   void     setY(const int4b y) {_y = y;};
private:
   int4b    _x;
   int4b    _y;
};

//==============================================================================   
class DBbox {
public:
   DBbox(const TP& p) :        _p1(p)       , _p2(p)    {};
   DBbox(const DBbox& bx) :    _p1(bx.p1()) , _p2(bx.p2())    {};
   DBbox(const TP& p1, const TP& p2): _p1(p1)      , _p2(p2)   {};
   DBbox(int4b, int4b, int4b, int4b);
   void  overlap(const TP p);
   void  overlap(const DBbox bx);
   void  normalize();
   float cliparea(const DBbox& bx, bool calculate = false);
   int   clipbox(DBbox& bx);
   bool  inside(const TP& );
   float area(); 
   DBbox getcorner(byte corner);
   TP    p1()  const {return _p1;};
   TP    p2()  const {return _p2;};
   DBbox operator * (const CTM&) const;
   DBbox operator = (const DBbox&);
   bool  operator == (const DBbox&) const;
   bool  operator != (const DBbox&) const;
private:
   TP    _p1;
   TP    _p2;
};

typedef  std::vector<TP>         pointlist;
typedef  std::stack<CTM>         ctmstack;
typedef  std::deque<CTM>         ctmqueue;
typedef  std::list<std::string>  nameList;

//=============================================================================
template <class TYPE> class SGHierTree {
public:
   SGHierTree(TYPE* comp, TYPE* prnt, SGHierTree* lst);
   SGHierTree(TYPE* comp, SGHierTree* lst) {
      component = comp; last = lst;
   };
   SGHierTree*   GetFirstRoot() {
      SGHierTree* wv = this;
      while (wv && (wv->parent)) wv = wv->last;
      return wv;
   };
   SGHierTree*   GetNextRoot()  {
      SGHierTree* wv = this->last;
      while (wv && (wv->parent)) wv = wv->last;
      return wv;
   };
   SGHierTree*  GetMember(TYPE* comp) {
      SGHierTree* wv = this;
      while (wv && (wv->component != comp)) 
         wv = wv->last;
      return wv;
   };
   SGHierTree*  GetNextMember(TYPE* comp) {
      SGHierTree* wv = this->last;
      while (wv && (wv->component != comp)) wv = wv->last;
      return wv;
   };
   int            addParent(TYPE* comp, TYPE* prnt, SGHierTree*& lst);
   bool           removeParent(TYPE* comp, TYPE* prnt, SGHierTree*& lst);
   bool           checkAncestors(TYPE* comp, TYPE* prnt, SGHierTree*& lst);
   SGHierTree*    GetBrother()         {return brother;};
   SGHierTree*    GetChild()           {return Fchild;};
   SGHierTree*    Getparent()          {return parent;};
   SGHierTree*    GetLast()            {return last;};
   TYPE*          GetItem()            {return component;};
protected:
   TYPE          *component; // points to the component 
   SGHierTree*    last;      // last in the linear list of components
   SGHierTree*    parent;    // points up
   SGHierTree*    brother;   // points right
   SGHierTree*    Fchild;    // points down to first child
};

// The constructor
template <class TYPE>
SGHierTree<TYPE>::SGHierTree(TYPE* comp, TYPE* prnt, SGHierTree* lst) {
   component = comp;last = lst; 
   SGHierTree* wv = last;
   // look for parent
   if (prnt) {
      while (wv && (wv->component != prnt)) wv = wv->last;
      parent = wv;
   }
   else parent = NULL;
   // recognize the brothers
   if (parent) {
      wv = parent->Fchild;
      brother = wv;
      parent->Fchild = this;
   }
   else 
      brother = NULL;
   Fchild = NULL;
};

template <class TYPE> 
    bool SGHierTree<TYPE>::checkAncestors(TYPE* comp, TYPE* prnt, SGHierTree*& lst) {
   // returns true  -> prnt is already an ancestor of the comp
   //         false -> otherwise
  SGHierTree* wv = lst->GetMember(comp);
  SGHierTree* wvP = lst->GetMember(prnt);
   // protect yourself - if parent or the component are not in the list 
  assert(wvP); assert(wv);
  do {
    SGHierTree* wv2 = wv;
    do {
      if (wv2->GetItem() == prnt) return true;
    } while (NULL != (wv2 = wv2->parent));
  } while (NULL != (wv = wv->GetNextMember(comp)));
  return false;
};

template <class TYPE>
int SGHierTree<TYPE>::addParent(TYPE* comp, TYPE* prnt, SGHierTree*& lst) {
   // returns 0 -> nothin's changed (this parent is already in the list)
   //         1 -> first parrent added (component use to be orphan)
   //         2 -> new parent added
   SGHierTree* wv = lst->GetMember(comp);
   SGHierTree* wvP = lst->GetMember(prnt);
   // protect yourself - if parent or the component are not in the list 
   assert(wvP); assert(wv);
   if (NULL == wv->parent) {
      // if the component use to be orphan, don't create a new member, 
      // just link the existing one
      wv->parent = wvP;
      wv->brother = wvP->Fchild;
      wvP->Fchild = wv;
      return 1;
   }   
   else {
      // compondent is not an orphan, so first check that this comp 
      //already has this prnt. 
      SGHierTree* wv2 = wv;
      do {
         if (wv2->parent->GetItem() == prnt) return 0;
      }   
      while (NULL != (wv2 = wv2->GetNextMember(comp)));
      // if not, for every appearance of the parent, we need to add a child comp
      do {
         lst = new SGHierTree(comp, lst);
         lst->parent = wvP;
         lst->brother = wvP->Fchild;
         wvP->Fchild = lst;
         lst->Fchild = wv->Fchild;
      }   
      while (NULL != (wvP = wvP->GetNextMember(prnt))); 
   }
   return 2;
};

template <class TYPE>
bool  SGHierTree<TYPE>::removeParent(TYPE* comp, TYPE* prnt, SGHierTree*& lst) {
   SGHierTree* citem;
   SGHierTree* check;
   SGHierTree* cparent = lst->GetMember(prnt);
   while (cparent) {
      // first unlink comp form its brothers, because comp will be deleted
      assert(cparent->Fchild);
      if (cparent->Fchild->GetItem() == comp) {
         citem = cparent->Fchild;
         cparent->Fchild = citem->brother;
      }   
      else {
         SGHierTree* child = cparent->Fchild;
         while ((child->brother) && (child->brother->GetItem() != comp))
            child = child->brother;
         assert(child);
         citem = child->brother;
         child->brother = citem->brother;
      }
      assert(check = lst->GetMember(comp));
      if (check->GetNextMember(comp)) {
         // Means that is not the last component of this type
         // So it has to be deleted, though unlinked first
         if (lst == citem) lst = citem->last;
         else {
            SGHierTree* witem = lst;
            while (witem && (witem->last != citem)) 
               witem = witem->last;
            assert(witem);
            witem->last = citem->last;
         }
         delete citem;
      }   
      else {
         // This is the last component of this type, so it has to 
         // be flagged as an orphan and preserved at the top of the
         // hierarchy
         citem->brother = NULL;
         citem->parent = NULL;
         return true;
      }
      cparent = cparent->GetNextMember(prnt);   
   }
   return false;
}

#endif
