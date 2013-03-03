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
laydata::LayerIterator<DataT>::LayerIterator( const LayerDefMap* lhldr):
   _layerHolder ( lhldr                        )
{
   _cNMap = _layerHolder->begin();
   if (_layerHolder->end() == _cNMap)
      _layerHolder = NULL;
}

template <typename DataT>
laydata::LayerIterator<DataT>::LayerIterator( const LayerDefMap* lhldr, const LayerDef& laydef):
   _layerHolder ( lhldr                      )
{
   _cNMap = _layerHolder->find(laydef);
   assert (_layerHolder->end() != _cNMap);
}

template <typename DataT>
laydata::LayerIterator<DataT>::LayerIterator(const LayerIterator<DataT>& liter):
   _layerHolder ( liter._layerHolder           ),
   _cNMap       ( liter._cNMap                 )
{
}

template <typename DataT>
laydata::LayerIterator<DataT>::~LayerIterator()
{
}

template <typename DataT>
const laydata::LayerIterator<DataT>& laydata::LayerIterator<DataT>::operator++()
{//Prefix
   typename LayerDefMap::const_iterator nextNMap = _cNMap;
   if (++nextNMap != _layerHolder->end())
      _cNMap= nextNMap;
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
               && (_cNMap       == liter._cNMap      ) );
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
       || (_layerHolder->end()  == _cNMap )) return NULL;
   return _cNMap->second;
}

template <typename DataT>
DataT laydata::LayerIterator<DataT>::operator*() const
{
   return operator->();
}

template <typename DataT>
LayerDef laydata::LayerIterator<DataT>::operator()() const
{
   return _cNMap->first;
}

template <typename DataT>
bool laydata::LayerIterator<DataT>::editable() const
{
   return _cNMap->first.editable();
}

//=============================================================================
template <typename DataT>
laydata::LayerContainer<DataT>::LayerContainer()
{
   _layers = DEBUG_NEW LayerDefMap;
   _copy   = false;
}

template <typename DataT>
laydata::LayerContainer<DataT>::LayerContainer(const LayerContainer<DataT>& init)
{
   _layers = init._layers;
   _copy = true;

}

template <typename DataT>
laydata::LayerContainer<DataT>::~LayerContainer()
{
   if (!_copy)
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
   typename LayerDefMap::const_iterator layer = _layers->find(laydef);
   if (_layers->end() == layer) return Iterator();
   else return Iterator(_layers, laydef);
}

template <typename DataT>
bool laydata::LayerContainer<DataT>::empty() const
{
   return _layers->empty();
}

template <typename DataT>
size_t laydata::LayerContainer<DataT>::size() const
{
   return _layers->size();
}

template <typename DataT>
void laydata::LayerContainer<DataT>::clear()
{
   _layers->clear();
}

template <typename DataT>
void laydata::LayerContainer<DataT>::add(const LayerDef& laydef, DataT quad)
{
   assert(_layers->end() == _layers->find(laydef));

   _layers->insert(std::pair<LayerDef, DataT>(laydef, quad));
}

template <typename DataT>
void laydata::LayerContainer<DataT>::erase(const LayerDef& laydef)
{
   typename LayerDefMap::iterator layer = _layers->find(laydef);
   assert(_layers->end() != layer);
   _layers->erase(layer);
}

//template <typename DataT>
//void laydata::LayerContainer<DataT>::erase(LayerIterator<DataT> liter)
//{
//
//}

template <typename DataT>
DataT& laydata::LayerContainer<DataT>::operator[](const LayerDef& laydef)
{
   typename LayerDefMap::iterator layer = _layers->find(laydef);
   if (_layers->end() == layer)
      assert(false);
   return (*_layers)[laydef];
}

template <typename DataT>
laydata::LayerContainer<DataT>& laydata::LayerContainer<DataT>::operator=(const LayerContainer<DataT>& init)
{
   _layers = init._layers;
   _copy = true;
   return *this;
}

//=============================================================================


//==============================================================================
// implicit template instantiation with certain type parameters
template class laydata::LayerIterator<laydata::QuadTree*>;
template class laydata::LayerContainer<laydata::QuadTree*>;
template class laydata::LayerIterator<laydata::QTreeTmp*>;
template class laydata::LayerContainer<laydata::QTreeTmp*>;
template class laydata::LayerIterator<laydata::DataList*>;
template class laydata::LayerContainer<laydata::DataList*>;
template class laydata::LayerIterator<laydata::ShapeList*>;
template class laydata::LayerContainer<laydata::ShapeList*>;
//------------------------------------------------------------------------------
template class laydata::LayerIterator<layprop::LayerSettings*>;
template class laydata::LayerContainer<layprop::LayerSettings*>;
template class laydata::LayerIterator<trend::TrendLay*>;
template class laydata::LayerContainer<trend::TrendLay*>;
//------------------------------------------------------------------------------
template class laydata::LayerIterator<auxdata::QuadTree*>;
template class laydata::LayerContainer<auxdata::QuadTree*>;
template class laydata::LayerIterator<auxdata::QTreeTmp*>;
template class laydata::LayerContainer<auxdata::QTreeTmp*>;

