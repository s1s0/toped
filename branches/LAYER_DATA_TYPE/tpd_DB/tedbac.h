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

namespace layprop {
   class LayerSettings;
}

namespace tenderer {
   class TenderLay;
}

namespace laydata {
   class TdtData;
   template <typename DataT>       class QTStoreTmpl;
   template <typename DataT>       class QTreeTmpl;
   typedef QTStoreTmpl<TdtData>    QTreeTmp;
   typedef QTreeTmpl<TdtData>      QuadTree;

   typedef  std::pair<TdtData*, SGBitSet>           SelectDataPair;
   typedef  std::list<SelectDataPair>               DataList;
   typedef  std::list<TdtData*>                     ShapeList;


//   template <typename DataT>
//   struct LayerDMap {
//      typedef std::map<LayerDType , DataT     >     Type;
//   };
//   template <typename DataT>
//   struct LayerNMap {
//      typedef std::map<LayerNumber, typename LayerDMap<DataT>::Type >  Type;
//   };


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
      LayerNumber               number() const;
      LayerDef                  layDef() const;
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
                                 LayerContainer(const LayerContainer<DataT>&);
      virtual                   ~LayerContainer();
      const Iterator             begin() const;
      const Iterator             end() const;
      const Iterator             find(const LayerDef&) const;
      bool                       empty() const;
      size_t                     size() const;
      void                       clear();
      void                       add(const LayerDef&, DataT);
      void                       erase(const LayerDef&);
      DataT&                     operator[](const LayerDef&);
      LayerContainer<DataT>&     operator=(const LayerContainer<DataT>&);
   private:
      LayerNMap*                 _layers;
      bool                       _copy;
   };

   const LayerDef  REF_LAY_DEF(REF_LAY, DEFAULT_LAY_DATATYPE);
   const LayerDef  GRC_LAY_DEF(GRC_LAY, DEFAULT_LAY_DATATYPE);

}

namespace auxdata {
   class TdtAuxData;
   typedef laydata::QTStoreTmpl<TdtAuxData>     QTreeTmp;
   typedef laydata::QTreeTmpl<TdtAuxData>       QuadTree;
   typedef laydata::LayerContainer<QuadTree*>   LayerHolder;
//   typedef std::list<TdtAuxData*>               AuxDataList;
   typedef laydata::LayerContainer<QTreeTmp*>   TmpLayerMap;
}


typedef  std::list<LayerDef>             LayerDefList;

#endif //TEDBAC_H_INCLUDED
