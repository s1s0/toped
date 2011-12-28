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
//        Created: Mon Apr 28 22:09:54 BST 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Basic common Toped types&classes
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TTT_H_INCLUDED
#define TTT_H_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <list>
#include <set>
#include <assert.h>

//=============================================================================
// General type declations (compatability)
//=============================================================================
typedef unsigned char            byte     ; // 1 byte
typedef unsigned short           word     ; // 2 bytes
typedef unsigned int             dword    ; // 4 bytes
typedef unsigned long long int   qword    ; // 8 bytes
typedef short    int             int2b    ; // 2 bytes
typedef          int             int4b    ; // 4 bytes
typedef long long int            int8b    ; // 8 bytes
typedef          double          real     ; // 8 bytes

typedef  dword                            WireWidth;
typedef  std::list<std::string>           NameList;
typedef  std::set<std::string>            NameSet;
typedef  std::list<word>                  WordList;
typedef  std::set<word>                   WordSet;
typedef  std::set<dword>                  DWordSet;
typedef  std::map<word, WordSet>          ExtLayers;
typedef  std::map<std::string, int>       SIMap;       // name
typedef  std::map<unsigned, std::string>  USMap;      // Unsigned - String Map
typedef  std::map<word, unsigned long>    SLMap;

enum QuadIdentificators{ qidNW = 0,
                         qidNE = 1,
                         qidSE = 2,
                         qidSW = 3,
                         qidNULL = 4};

// The definition below is a "strongly typed enum". Very tempting to use, but too new
// and too risky for portability. gcc requires -std=c++0x option to stop the warnings
// It's here just as a reminder for the future
//   enum class SH_STATUS:byte { sh_active, sh_deleted, sh_selected, sh_partsel, sh_merged, sh_preserved } ;
typedef enum { sh_active, sh_deleted, sh_selected, sh_partsel, sh_merged, sh_preserved } SH_STATUS;

//=============================================================================
// Some common constants (instead of #defines)
//=============================================================================
const int         ALL_LIB           = -2;
const int         TARGETDB_LIB      = -1;
const int         UNDEFCELL_LIB     =  0;


#ifdef WIN32
   #include <windows.h>
   double round(double x);
   int8b lround (double x);
   #define rint  round
   #define llabs _abs64
   //#define round floor
   #define remainder fmod
   #define M_PI   3.1415926535897932384626433832795
   #pragma warning( disable : 4786 )
   #include <wx/msw/winundef.h>
//#else
//   #include "config.h"
#endif // WIN32

// macros for tracking down errors
#ifdef NDEBUG
#define VERIFY( exp )           ((void)(exp))
#else
#define VERIFY( exp )           assert( exp )
#endif  // NDEBUG

#define YYLTYPE TpdYYLtype
#define YYLLOC_DEFAULT(Current, Rhs, N)                         \
   do                                                           \
      if (N)                                                    \
      {                                                         \
      (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;    \
      (Current).first_column = YYRHSLOC (Rhs, 1).first_column;  \
      (Current).last_line    = YYRHSLOC (Rhs, N).last_line;     \
      (Current).last_column  = YYRHSLOC (Rhs, N).last_column;   \
      (Current).filename     = YYRHSLOC (Rhs, N).filename;      \
      }                                                         \
      else                                                      \
      {                                                         \
      (Current).first_line   = (Current).last_line   =          \
                           YYRHSLOC (Rhs, 0).last_line;         \
      (Current).first_column = (Current).last_column =          \
                        YYRHSLOC (Rhs, 0).last_column;          \
         (Current).filename = YYRHSLOC (Rhs, 0).filename;       \
      }                                                         \
   while (0)

// to cast properly the indices parameter in glDrawElements when
// drawing from VBO
#define VBO_BUFFER_OFFSET(i) ((void *)NULL + (i))


//==============================================================================
class SGBitSet {
public:
            SGBitSet(word);
            SGBitSet(const SGBitSet&);
            SGBitSet(): _size(0), _packet(NULL) {};
   void     set(word);
   void     reset(word);
   void     setall();
   void     check_neighbours_set(bool wire);
   bool     check(word) const;
   bool     isallclear() const;
   bool     isallset() const;
   word     size() const {return _size;};
   void     swap(word, word);
   void     clear();
   bool     operator == (const SGBitSet&) const;
   SGBitSet operator =  (const SGBitSet&);
           ~SGBitSet();
private:
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
> PSCTM   operator *  (PSCTM op2) Multiplication CTM = CTM1*CTM2
> PSCTM   operator *= (PSCTM op2) Multiplication assignemt CTM *= CTM1
> PSP     operator *  (GP* op1)   Multiplication PSP = CTM*GP
> PSP     operator *  (PSP* op1)  Multiplication PSP = CTM*PSP
******************************************************************************/
class TP;
class   CTM
{
public:
   CTM() {Initialize();};
   void Initialize()                {_a = _d = 1.0;_b = _c = _tx = _ty = 0.0;};
   CTM(const TP&, real, real, bool);
   CTM(real va,real vb,real vc,real vd,real vtx,real vty) :
    _a(va), _b(vb), _c(vc), _d(vd), _tx(vtx), _ty(vty) {};
//   CTM(const CTM& mtrx) { *this = *mtrx;};
   CTM  Translate(real X, real Y) {return (*this *= CTM(1,0,0,1,X,Y));}
   CTM  Translate(const TP&);
   CTM  Scale(real X, real Y)     {return (*this *= CTM(X,0,0,Y,0,0));}
   CTM  FlipX(real Y=0)           {return (*this *= CTM(1,0,0,-1,0,2*Y));}
   CTM  FlipY(real X=0)           {return (*this *= CTM(-1,0,0,1,2*X,0));}
   CTM  Rotate(const real);
   CTM  Rotate(const real, const TP&);
   CTM  Rotate(const TP&);
   CTM  Reversed() const;
   void Decompose(TP&, real&, real&, bool&) const;
   void oglForm(real* const) const;
   void setCTM(real a, real b, real c, real d, real tx, real ty)
                          {_a = a; _b = b; _c = c; _d = d; _tx = tx; _ty = ty;};
   CTM  operator =  (const CTM& op2);
   CTM  operator *  (const CTM& op2) const;
   CTM  operator *  (const real) const;
   CTM  operator *= (const CTM& op2) {return (*this = *this * op2);};
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
   void     info(std::ostringstream&,real) const;
   const int4b    x()  const {return _x;};
   const int4b    y()  const {return _y;};
   TP   operator *  ( const CTM& ) const;
   TP   operator *= ( const CTM& );
   TP   operator *= ( const real );
   TP   operator /= ( const real );
   TP   operator =  ( const TP& np)        {_x = np.x(); _y = np.y(); return *this;};
   TP   operator -  ( const TP& pnt) const {return TP(_x - pnt.x(),_y - pnt.y());};
   TP   operator +  ( const TP& pnt) const {return TP(_x + pnt.x(),_y + pnt.y());};
   bool operator == ( const TP& np ) const {return ((np.x() == _x) && (np.y() == _y));};
   bool operator != ( const TP& np ) const {return ((np.x() != _x) || (np.y() != _y));};
   void     setX(const int4b x) {_x = x;};
   void     setY(const int4b y) {_y = y;};
private:
   int4b    _x;
   int4b    _y;
};

//==============================================================================
// The DBbox class is used primarily in the quadtree algo, to keep the overlap
// variables of the DB objects and to implement the corresponding calculations.
// Its methods shall comply with the specific requirements of the algorithm.
//
class DBbox {
public:
   DBbox(const TP& p) :        _p1(p)       , _p2(p)    {};
   DBbox(int4b x1, int4b y1) : _p1(x1,y1) , _p2(x1,y1)  {};
   DBbox(int4b x1, int4b y1, int4b x2, int4b y2) : _p1(x1,y1) , _p2(x2,y2)  {};
   DBbox(const DBbox& bx) :    _p1(bx.p1()) , _p2(bx.p2())    {};
   DBbox(const TP& p1, const TP& p2): _p1(p1)      , _p2(p2)   {};
   void  overlap(int4b, int4b);
   void  overlap(const TP& p);
   void  overlap(const DBbox& bx);
   DBbox overlap(const CTM&) const;
   void  normalize();
   int8b cliparea(const DBbox& bx, bool calculate = false) const;
   int   clipbox(DBbox& bx);
   bool  inside(const TP& );
   int8b boxarea() const;
   bool  visible(const CTM&, int8b) const;
   DBbox getcorner(QuadIdentificators corner);
   const TP&    p1()  const {return _p1;};
   const TP&    p2()  const {return _p2;};
   DBbox operator * (const CTM&) const;
   DBbox operator = (const DBbox&);
   bool  operator == (const DBbox&) const;
   bool  operator != (const DBbox&) const;
private:
   TP    _p1;
   TP    _p2;
};
//==============================================================================
class DBline {
public:
   DBline(): _p1(TP()), _p2(TP()) {};
   DBline(const DBline& ln) :    _p1(ln.p1()) , _p2(ln.p2()) {};
   DBline(const TP& p1, const TP& p2): _p1(p1), _p2(p2)      {};
   const TP&    p1()  const {return _p1;};
   const TP&    p2()  const {return _p2;};
   DBline operator *  (const CTM&) const;
   DBline operator  =  (const DBline&);
private:
   TP    _p1;
   TP    _p2;
};

typedef  std::vector<TP>         PointVector;
typedef  std::stack<CTM>         CtmStack;
typedef  std::deque<CTM>         CtmQueue;
typedef  std::list<PointVector*> pcollection; // point list collection


struct TpdYYLtype {
   int          first_line;
   int          first_column;
   int          last_line;
   int          last_column;
   char*        filename;
};

std::vector<std::string> split (const std::string& str, char delim);
int8b polyarea(const PointVector& shape);
unsigned GCD(unsigned arg1, unsigned arg2);

//=============================================================================
// A template of a cell hierarchy used for all layout databases in Toped (TDT,
// GDS, CIF, Oasis). The cell browser panels only mirror the hierarchy build in
// the data bases of this type.
// Layout hierarchies have their specifics.
//  - Cell instances don't have instance names. This means that if a certain
//    cell is instantiated several times in another cell - this will be shown
//    in the data base as a single cell instance
//  - Cells which are not instantiated in other cells shall be shown on the
//    top of the hierarchy.
// All the above makes this structure different from a traditional instance
// hierarchy structure. Each member of the hierarchy has:
//  - A pointer to the actual cell structure. The cell structure can be pointed
//    to from more than one hierarchy component.
//  - None or more brothers. There can not be "twins" among the brothers though.
//    Means that it doesn't matter how many times a cell is instantiated in
//    another cell. It matters only whether or not it is instantiated.
//  - None or more children. Again - there should not be "twins" among the
//    children.
//  - At most one parent. Means that if a cell is instantiated in several other
//    cells there will be exactly one member of hierarchy for each of those
//    cases. Cells with no parent (orphans) are the cells on the top of the
//    hierarchy tree.
//  - A linear pointer to the last member of the hierarchy structure. It is used
//    to search and traverse the entire tree. Only one member can have it's
//    "last" field pointing to NULL. The code shall always keep a pointer to the
//    component added last to the hierarchy.
// The structure can be used in two ways.
// - to build a hierarchy of a data base in memory. This is used after an
//   external DB or library was loaded and also when external DBs are parsed.
// - dynamically - when the layout is updated interactively. Beware that this
//   method can be painfully slow when used intensively for big DBs

template <class TYPE> class SGHierTree {
public:
   SGHierTree(const TYPE* comp, const TYPE* prnt, SGHierTree* lst);
   SGHierTree(const SGHierTree* cousin, SGHierTree* prnt, SGHierTree* lst);
   SGHierTree*       GetFirstRoot(int libID);
   SGHierTree*       GetNextRoot(int libID);
   const SGHierTree* GetBrother(int libID) const;
   const SGHierTree* GetChild(int libID) const;
   SGHierTree*       GetMember(const TYPE* comp);
   SGHierTree*       GetNextMember(const TYPE* comp);
   bool              checkAncestors(const TYPE* comp, const TYPE* prnt, SGHierTree* lst) const;
   int               addParent(const TYPE* comp, const TYPE* prnt, SGHierTree*& lst);
   int               removeParent(const TYPE* comp, const TYPE* prnt, SGHierTree*& lst);
//   void              replaceChild(const TYPE* oldchild, const TYPE* newchild, SGHierTree*& lst, int libID);
   bool              removeRootItem(const TYPE*comp, SGHierTree*& lst);
   bool              itemRefdIn(int libID) const;
   const TYPE*       GetItem() const           {return component;}
   const SGHierTree* GetLast() const           {return last;}
   const SGHierTree* Getparent() const         {return parent;}
   void              relink(const SGHierTree* comp)  {last = comp->last;}
private:
   bool              thisLib(int libID) const;
   bool              thisParent(int libID);
   const TYPE       *component; // points to the component
   SGHierTree*       last;      // last in the linear list of components
   SGHierTree*       parent;    // points up
   SGHierTree*       brother;   // points right (siblings)
   SGHierTree*       Fchild;    // points down to the first child
};

// The constructor
template <class TYPE>
SGHierTree<TYPE>::SGHierTree(const TYPE* comp, const TYPE* prnt, SGHierTree* lst)
{
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
      brother = parent->Fchild;
      parent->Fchild = this;
   }
   else
      brother = NULL;
   Fchild = NULL;
};

template <class TYPE>
SGHierTree<TYPE>::SGHierTree(const SGHierTree* cousin, SGHierTree* prnt, SGHierTree* lst)
{
   component = cousin->component;
   parent = prnt;
   brother = prnt->Fchild;
   prnt->Fchild = this;
   Fchild = NULL;
   if (cousin->Fchild)
   {
      // deep copy
      SGHierTree* wv = cousin->Fchild;
      do lst = DEBUG_NEW SGHierTree(wv, this, lst);
      while (NULL != (wv = wv->brother));
   }
   last = lst;
};

template <class TYPE>
   bool SGHierTree<TYPE>::itemRefdIn(int libID) const {
      if (libID == component->libID()) return true;
      else
      {
         SGHierTree* wvparent = parent;
         while (NULL != wvparent)
         {
            if (libID == wvparent->component->libID())
               return true;
            else
               wvparent = wvparent->parent;
         }
         return false;
      }
   }

template <class TYPE>
      bool   SGHierTree<TYPE>::thisLib(int libID) const {
         /*! Any libID < TARGETDB_LIB will make the functions to ignore it.
             Idea is to have a possibility to traverse the entire
             tree no matter where the cell belongs */
         return (libID < TARGETDB_LIB) ? true : (libID == component->libID());
      }

template <class TYPE>
      bool   SGHierTree<TYPE>::thisParent(int libID) {
         if      ( NULL == parent       )
            return false;
         else if ( libID <  TARGETDB_LIB )
            // Any libID < TARGETDB_LIB will make the functions to ignore it.
            // Idea is to have a possibility to traverse the entire
            // tree no matter where the cell belongs
            return true;
         else if ( TARGETDB_LIB == component->libID())
            // if current cell belongs to the target DB
            return (TARGETDB_LIB == parent->component->libID());
         else
         {
            // It's more complicated with the libraries here. Here is the problem:
            // 1. We want to show database hierarchy including referenced library
            //    cells.
            // 2. Library hierarchy should not be influenced by the changes in the
            //    database and particularly by the changes in the hierarchy of the
            //    database.
            // To achieve this for library cells we have to check whether all
            // instances of this type have parent from libID library
            SGHierTree* wv = GetMember(component);
            while (NULL != wv)
            {
               if (NULL != wv->parent)
                  if (libID == wv->parent->component->libID())
                     return true;
               wv = wv->GetNextMember(component);
            }
            return false;
         }
      }

template <class TYPE>
   SGHierTree<TYPE>*   SGHierTree<TYPE>::GetFirstRoot(int libID) {
      SGHierTree* wv = this;
      while (wv && (wv->thisParent(libID) || !wv->thisLib(libID) ) ) wv = wv->last;
      return wv;
   }

template <class TYPE>
   SGHierTree<TYPE>*   SGHierTree<TYPE>::GetNextRoot(int libID)  {
      SGHierTree* wv = this->last;
      while (wv && (wv->thisParent(libID) || !wv->thisLib(libID) ) ) wv = wv->last;
      return wv;
   }

template <class TYPE>
   const SGHierTree<TYPE>* SGHierTree<TYPE>::GetChild(int libID) const {
   if ( (NULL == Fchild) || Fchild->thisLib(libID) ) return Fchild;
      SGHierTree* wv = Fchild;
      while ( wv && !wv->thisLib(libID) ) wv = wv->brother;
      return wv;
   }

template <class TYPE>
   const SGHierTree<TYPE>* SGHierTree<TYPE>::GetBrother(int libID) const {
      SGHierTree* wv = brother;
      while (wv && !wv->thisLib(libID) ) wv = wv->brother;
      return wv;
   }

template <class TYPE>
   SGHierTree<TYPE>*  SGHierTree<TYPE>::GetMember(const TYPE* comp) {
      SGHierTree* wv = this;
      while (wv && (wv->component != comp)) wv = wv->last;
      return wv;
   }

template <class TYPE>
   SGHierTree<TYPE>*  SGHierTree<TYPE>::GetNextMember(const TYPE* comp) {
      SGHierTree* wv = this->last;
      while (wv && (wv->component != comp)) wv = wv->last;
      return wv;
   }

template <class TYPE>
   bool SGHierTree<TYPE>::checkAncestors(const TYPE* comp, const TYPE* prnt, SGHierTree* lst) const
{
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
int SGHierTree<TYPE>::addParent(const TYPE* comp, const TYPE* prnt, SGHierTree*& lst)
{
   // returns 0 -> nothin's changed (this parent is already in the list)
   //         1 -> first parrent added (component use to be orphan)
   //         2 -> new parent added
   //         3 -> first parrent added for library component
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
      if (TARGETDB_LIB == wv->component->libID()) return 1;
      else                                        return 3;
   }
   else {
      // component is not an orphan, so first check that this comp
      //already has this prnt.
      SGHierTree* wv2 = wv;
      do {
         if (wv2->parent->GetItem() == prnt) return 0;
      }
      while (NULL != (wv2 = wv2->GetNextMember(comp)));
      // if not, for every appearance of the parent, we need to add a child comp
      do {
         // here -> we need a "deep" copy. Means - all the children of wv
         // and their children too
         lst = DEBUG_NEW SGHierTree(wv, wvP, lst);
      }
      while (NULL != (wvP = wvP->GetNextMember(prnt)));
   }
   return 2;
};

template <class TYPE>
int  SGHierTree<TYPE>::removeParent(const TYPE* comp, const TYPE* prnt, SGHierTree*& lst)
{
   // returns 0 -> not much changed (the component has another parent)
   //         1 -> A DB component which is now an orphan
   //         2 -> A library component which is no more referenced in the DB
   //         3 -> Component not found in the tree
   SGHierTree* citem;

   SGHierTree* cparent = lst->GetMember(prnt);
   while (cparent) {
      // first unlink comp from its brothers, because comp will be deleted
      assert(cparent->Fchild);
      if (cparent->Fchild->GetItem() == comp)
      {
         citem = cparent->Fchild;
         cparent->Fchild = citem->brother;
      }
      else
      {
         SGHierTree* child = cparent->Fchild;
         while ((child->brother) && (child->brother->GetItem() != comp))
            child = child->brother;
         citem = child->brother;
         if (NULL == citem) return 3; // means that comp is not found
         child->brother = citem->brother;
      }
      // Don't get confused here! check has nothing to do with anything. It is
      // ONLY used to find out whether more components of type comp remain in the
      // structure. citem can't be used for this purpose, because it simply
      // can be the last in the structure and the latter is not circular.
      SGHierTree* check = lst->GetMember(comp);
      assert(check);
      if (check->GetNextMember(comp))
      {
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
      else if (citem) {
         // This is the last component of this type, so it has to
         // be flagged as an orphan and preserved at the top of the
         // hierarchy, but only if it is NOT a library cell.
         // library cells can't be removed from libraries and when removed from
         // the DB, they are preserved in the library hierarchy anyway.
         citem->brother = NULL;
         citem->parent = NULL;
         if   (TARGETDB_LIB == citem->component->libID()) return 2;
         else                                             return 1;
      }
      cparent = cparent->GetNextMember(prnt);
   }
   return 0;
}

//template <class TYPE>
//void  SGHierTree<TYPE>::replaceChild(const TYPE* oldchild, const TYPE* newchild, SGHierTree*& lst, int libID)
//{
//   SGHierTree* thechild = lst->GetMember(oldchild);
//   while (thechild)
//   {
//      if (thechild->Getparent()->thisLib(libID))
//      {
//         thechild->component = newchild;
//      }
//      thechild = thechild->GetNextMember(oldchild);
//   }
//}

/*! Requires root childless item */
template <class TYPE>
bool  SGHierTree<TYPE>::removeRootItem(const TYPE* comp, SGHierTree*& lst)
{
   SGHierTree* wv = lst;
   SGHierTree* wvp = NULL;
   while (wv)
   {// Find the coponent and its place in the list
      if (wv->component != comp)
      {
         wvp = wv;
         wv = wv->last;
      }
      else
      {// make sure that's a root component
         assert(NULL == wv->parent);
         // make sure that it's childless
         assert(NULL == wv->Fchild);
         // unlink it from the list
         if (wvp) wvp->last = wv->last;
         else lst = wv->last;
         // finally we can delete the comp
         delete wv;
         return true;
      }
   }
   return false;
}

//=============================================================================
// More common constants (instead of #defines)
//=============================================================================
const byte        MAX_BYTE_VALUE       = 255;
const word        MAX_WORD_VALUE       = 65535;
const int4b       MIN_INT4B            = (int4b)0x80000001; //  -2 147 483 647
const int4b       MAX_INT4B            = (int4b)0x7FFFFFFF; //   2 147 483 643
const WireWidth   MAX_WIRE_WIDTH       = 0x0FFFFFFF;
//const DBbox MAX_OVL_BOX        = DBbox(MIN_X,MAX_X,MIN_Y,MIN_Y); // maximum overlapping box
const unsigned    REF_LAY              = 0xffffffff;
const unsigned    ERR_LAY              = 0xfffffffe;
const unsigned    DRC_LAY              = 0xfffffffd;
const unsigned    GRC_LAY              = 0xfffffffc;
const unsigned    LAST_EDITABLE_LAYNUM = 0x0000ffff;
const byte        OPENGL_FONT_UNIT     = 128;
const byte        GRID_LIMIT           = 5;    // if grid step is less than _GRID_LIMIT pixels, grid is hidden
const DBbox       DEFAULT_OVL_BOX      = DBbox(TP(0,0));
const DBbox       DEFAULT_ZOOM_BOX     = DBbox(TP(-2000,-2000), TP(20000,20000));
const real        DEFAULT_DBU          = 1e-9;
const real        DEFAULT_UU           = 1e-3;

#endif
