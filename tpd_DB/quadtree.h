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

   template <typename DataT>
   class QtPosition {
   public:
         QtPosition(const QTreeTmpl<DataT>* cQuad, byte cSubQuad) : _cQuad(cQuad), _cSubQuad(cSubQuad) {}
      const QTreeTmpl<DataT>*   _cQuad;
      const byte                _cSubQuad;
   };

   template <typename DataT>
   class Iterator {
   public:
      typedef  std::stack<QtPosition<DataT> >        QPosStack;
                                Iterator();
                                Iterator(const QTreeTmpl<DataT>&);
                                Iterator(const Iterator&);
      virtual                  ~Iterator();
      const Iterator&           operator++();    //Prefix
      const Iterator            operator++(int); //Postfix
      bool                      operator==(const Iterator&);
      bool                      operator!=(const Iterator&);
      DataT*                    operator->();
      DataT*                    operator*();
   protected:
      virtual bool              secureNonEmptyDown();
      bool                      nextSubQuad(byte, byte);
      const QTreeTmpl<DataT>*   _cQuad;
      QuadsIter                 _cData;
      QPosStack*                _qPosStack;
      bool                      _copy;
   };

   template <typename DataT>
   class ClipIterator : public Iterator<DataT> {
   public:
                                ClipIterator();
                                ClipIterator(const QTreeTmpl<DataT>&, const DBbox&);
                                ClipIterator(const ClipIterator&);
      virtual                  ~ClipIterator() {}
      const ClipIterator&       operator++();    //Prefix
      const ClipIterator        operator++(int); //Postfix
   protected:
      virtual bool              secureNonEmptyDown();
      DBbox                     _clipBox;
   };

   template <typename DataT>
   class DrawIterator : public Iterator<DataT> {
   public:
                                DrawIterator();
                                DrawIterator(const QTreeTmpl<DataT>&, const layprop::DrawProperties&, const CTM& );
                                DrawIterator(const DrawIterator&);
      virtual                  ~DrawIterator() {}
      const DrawIterator&       operator++();    //Prefix
      const DrawIterator        operator++(int); //Postfix
   protected:
      virtual bool              secureNonEmptyDown();
      const layprop::DrawProperties* _drawprop;
      const CTM                 _ctm;
   };

#ifdef WIN32
   #pragma pack()
   struct  QuadProps
#else
   struct __attribute__ ((__packed__)) QuadProps
#endif
   {
   public:
                                QuadProps();
      byte                      numSubQuads() const;
      char                      getPosition(QuadIdentificators);
      void                      addQuad(QuadIdentificators);
      void                      removeQuad(QuadIdentificators);
      void                      clearQuadMap() {_quadMap = 0;}
      QuadsIter                 _numObjects;
     /*! Flag indicates that the container needs to be resorted*/
      bool                      _invalid;
   private:
      char                      getNEQuad() const;
      char                      getNWQuad() const;
      char                      getSEQuad() const;
      char                      getSWQuad() const;
      byte                      _quadMap;
   };

   template <typename DataT>
   class QTStoreTmpl {
   public:
                                QTStoreTmpl(QTreeTmpl<DataT>* trunk) : _trunk(trunk) {};
       void                     put(DataT* shape);
       void                     commit();
       unsigned                 numObjects()  {return static_cast<unsigned>(_data.size());}
   private:
      typedef  std::list<DataT*>    ShapeList;
      ShapeList                 _data;
      QTreeTmpl<DataT>*         _trunk;
   };

   typedef QTStoreTmpl<TdtData>  QTreeTmp;
}

#endif
