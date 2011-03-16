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
//        Created: Sun Mar 07 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: QuadTree - supplementary classes
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef QUADTREE_H
#define QUADTREE_H

#include "tedat.h"

namespace laydata {

   typedef unsigned            QuadsIter;
   template <typename DataT>   class QTreeTmpl;
   typedef QTreeTmpl<TdtData>  QuadTree;

#ifdef WIN32
   #pragma pack() struct  QuadProps
#else
   struct __attribute__ ((__packed__)) QuadProps
#endif
   {
                             QuadProps();
      byte                   numSubQuads() const;
      char                   getPosition(QuadIdentificators);
      void                   addQuad(QuadIdentificators);
      void                   removeQuad(QuadIdentificators);
      void                   clearQuadMap() {_quadMap = 0;}
      QuadsIter             _numObjects;
      /*! Flag indicates that the container needs to be resorted*/
      bool                   _invalid;
   private:
      char                   getNEQuad() const;
      char                   getNWQuad() const;
      char                   getSEQuad() const;
      char                   getSWQuad() const;
      byte                   _quadMap;
   };

   class QTreeTmp {
   public:
                           QTreeTmp(QuadTree* trunk) : _trunk(trunk) {};
       void                put(TdtData* shape);
       void                putBox(const TP& p1, const TP& p2);
       void                putPoly(PointVector& pl);
       void                putPoly(int4b* pl, unsigned psize);
       void                putWire(PointVector& pl,word w);
       void                putText(std::string text, CTM trans);
       void                commit();
   private:
      ShapeList            _data;
      QuadTree*           _trunk;
   };

}

#endif
