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

#include "tpdph.h"
#include "tedbac.h"

//=============================================================================
template <typename DataT>
laydata::LayerIterator<DataT>::LayerIterator():
   _layerHolder (                        NULL  )
{
}

template <typename DataT>
laydata::LayerIterator<DataT>::LayerIterator( const LayerNMap* lhldr):
   _layerHolder ( lhldr                        )
{
   _cNMap = _layerHolder->begin();
   if (_layerHolder->end() == _cNMap)
      _layerHolder = NULL;
   else
   {
      _cDMap= _cNMap->second.begin();
      if (_cNMap->second.end() == _cDMap)
         _layerHolder = NULL;
   }
}

template <typename DataT>
laydata::LayerIterator<DataT>::LayerIterator( const LayerNMap* lhldr, const LayerDef& laydef):
   _layerHolder ( lhldr                      )
{
   _cNMap = _layerHolder->find(laydef.num());
   assert (_layerHolder->end() != _cNMap);
   _cDMap = _cNMap->second.find(laydef.typ());
   assert(_cNMap->second.end() != _cDMap);
}

template <typename DataT>
laydata::LayerIterator<DataT>::LayerIterator(const LayerIterator<DataT>& liter):
   _layerHolder ( liter._layerHolder           ),
   _cNMap       ( liter._cNMap                 ),
   _cDMap       ( liter._cDMap                 )
{
}

template <typename DataT>
laydata::LayerIterator<DataT>::~LayerIterator()
{
}

template <typename DataT>
const laydata::LayerIterator<DataT>& laydata::LayerIterator<DataT>::operator++()
{//Prefix
   typename LayerNMap::const_iterator nextNMap = _cNMap;
   typename LayerDMap::const_iterator nextDMap = _cDMap;
   if (++nextDMap != _cNMap->second.end())
      _cDMap = nextDMap;
   else if (++nextNMap != _layerHolder->end())
   {
      _cNMap= nextNMap;
      _cDMap = _cNMap->second.begin();
   }
   else
      _layerHolder = NULL;
   return *this;
}

template <typename DataT>
const laydata::LayerIterator<DataT> laydata::LayerIterator<DataT>::operator++(int)
{//Postfix
   LayerIterator previous(*this);
   operator++();
   return previous;
}

template <typename DataT>
bool laydata::LayerIterator<DataT>::operator==(const LayerIterator<DataT>& liter) const
{
   if ((NULL == _layerHolder) && (NULL == liter._layerHolder))
      return true;
   else
      return (    (_layerHolder == liter._layerHolder)
               && (_cNMap       == liter._cNMap      )
               && (_cDMap       == liter._cDMap      ) );
}

template <typename DataT>
bool laydata::LayerIterator<DataT>::operator!=(const LayerIterator<DataT>& liter) const
{
   return !operator==(liter);
}

template <typename DataT>
DataT laydata::LayerIterator<DataT>::operator->() const
{
   if (   (NULL == _layerHolder           )
       || (_layerHolder->end()  == _cNMap )
       || (_cNMap->second.end() == _cDMap )) return NULL;
   return _cDMap->second;
}

template <typename DataT>
DataT laydata::LayerIterator<DataT>::operator*() const
{
   return operator->();
}

template <typename DataT>
LayerNumber laydata::LayerIterator<DataT>::number()
{
   return _cNMap->first;
}

template <typename DataT>
laydata::LayerDef laydata::LayerIterator<DataT>::layDef()
{
   return LayerDef(_cNMap->first, _cDMap->first);
}

//=============================================================================
template <typename DataT>
laydata::LayerContainer<DataT>::LayerContainer()
{
   _layers = DEBUG_NEW LayerNMap;
}

template <typename DataT>
laydata::LayerContainer<DataT>::~LayerContainer()
{
   delete _layers;
}

template <typename DataT>
const typename laydata::LayerIterator<DataT> laydata::LayerContainer<DataT>::begin() const
{
   return Iterator(_layers);
}

template <typename DataT>
const typename laydata::LayerIterator<DataT> laydata::LayerContainer<DataT>::end() const
{
   return Iterator();
}

template <typename DataT>
const typename laydata::LayerIterator<DataT> laydata::LayerContainer<DataT>::find(const LayerDef& laydef) const
{
   typename LayerNMap::const_iterator layer = _layers->find(laydef.num());
   if (_layers->end() == layer) return Iterator();
   else
   {
      LayerDMap allTypes = layer->second;
      typename LayerDMap::const_iterator dtype = allTypes.find(laydef.typ());
      if (allTypes.end() == dtype) return Iterator();
      else return Iterator(_layers, laydef);
   }
}

template <typename DataT>
bool laydata::LayerContainer<DataT>::empty() const
{
   return _layers->empty();
}

template <typename DataT>
void laydata::LayerContainer<DataT>::clear()
{

}

template <typename DataT>
void laydata::LayerContainer<DataT>::add(const LayerDef& laydef, DataT quad)
{
   assert(_layers->end() == _layers->find(laydef.num()));
   std::pair<LayerDType , DataT> dtype(laydef.typ(), quad);
   LayerDMap dTypes;
   dTypes.insert(dtype);
   std::pair<LayerNumber, LayerDMap> layer(laydef.num(), dTypes);
   _layers->insert(layer);
}

template <typename DataT>
void laydata::LayerContainer<DataT>::erase(const LayerDef& laydef)
{
   typename LayerNMap::iterator layer = _layers->find(laydef.num());
   assert(_layers->end() != layer);
   LayerDMap allTypes = layer->second;
   allTypes.erase(laydef.typ());
   if (allTypes.empty())
      _layers->erase(layer);
}

template <typename DataT>
DataT laydata::LayerContainer<DataT>::operator[](const LayerDef& laydef)
{
   typename LayerNMap::iterator layer = _layers->find(laydef.num());
   if (_layers->end() == layer) return NULL;
   else
   {
      LayerDMap allTypes = layer->second;
      typename LayerDMap::iterator dtype = allTypes.find(laydef.typ());
      if (allTypes.end() == dtype) return NULL;
      else return dtype->second;
   }
}

//=============================================================================


//==============================================================================
// implicit template instantiation with a certain type parameter
template class laydata::LayerIterator<laydata::QuadTree*>;
template class laydata::LayerContainer<laydata::QuadTree*>;
template class laydata::LayerContainer<laydata::TdtData*>;
template class laydata::LayerIterator<laydata::DataList*>;
template class laydata::LayerContainer<laydata::DataList*>;
