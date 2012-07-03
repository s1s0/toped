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
//        Created: Thu Jun 28 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Database access handling (Iterators)
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#ifndef TEDBAC_H_INCLUDED
#define TEDBAC_H_INCLUDED

#include "ttt.h"
#include "outbox.h"

namespace laydata {
   class TdtData;
   template <typename DataT>       class QTStoreTmpl;
   template <typename DataT>       class QTreeTmpl;
   typedef QTStoreTmpl<TdtData>    QTreeTmp;
   typedef QTreeTmpl<TdtData>      QuadTree;

   typedef  std::pair<TdtData*, SGBitSet>           SelectDataPair;
   typedef  std::list<SelectDataPair>               DataList;

//   template <typename DataT>
//   struct LayerDMap {
//      typedef std::map<LayerDType , DataT     >     Type;
//   };
//   template <typename DataT>
//   struct LayerNMap {
//      typedef std::map<LayerNumber, typename LayerDMap<DataT>::Type >  Type;
//   };


   struct LayerDef {
      LayerDef(LayerNumber num, LayerDType  typ) : _num(num), _typ(typ) {}
      LayerDef(LayerNumber num) : _num(num), _typ(DEFAULT_LAY_DATATYPE) {}
      LayerNumber num() const {return _num;}
      LayerDType  typ() const {return _typ;}
   private:
      LayerNumber _num;
      LayerDType  _typ;
   };

   template <typename DataT>
   class LayerIterator {
   public:
      typedef                       std::map<LayerDType , DataT>   LayerDMap;
      typedef std::map<LayerNumber, std::map<LayerDType , DataT> > LayerNMap;
                                LayerIterator();
                                LayerIterator(const LayerNMap*);
                                LayerIterator(const LayerNMap*, const LayerDef&);
                                LayerIterator(const LayerIterator&);
      virtual                  ~LayerIterator();
      const LayerIterator&      operator++();    //Prefix
      const LayerIterator       operator++(int); //Postfix
      bool                      operator==(const LayerIterator&) const;
      bool                      operator!=(const LayerIterator&) const;
      DataT                     operator->() const;
      DataT                     operator*() const;
      LayerNumber               number();
      LayerDef                  layDef();
   protected:
      const LayerNMap*          _layerHolder;
      typename LayerNMap::const_iterator _cNMap;
      typename LayerDMap::const_iterator _cDMap;
   };

   template <typename DataT>
   class LayerContainer {
   public:
      friend class LayerIterator<DataT>;
      typedef                       std::map<LayerDType , DataT>   LayerDMap;
      typedef std::map<LayerNumber, std::map<LayerDType , DataT> > LayerNMap;
      typedef LayerIterator<DataT> Iterator;
                                 LayerContainer();
      virtual                   ~LayerContainer();
      const Iterator             begin() const;
      const Iterator             end() const;
      const Iterator             find(const LayerDef&) const;
      bool                       empty() const;
      void                       clear();
      void                       add(const LayerDef&, DataT);
      void                       erase(const LayerDef&);
      DataT                      operator[](const LayerDef&);
   private:
      LayerNMap*                 _layers;
   };


}

#endif //TEDBAC_H_INCLUDED
