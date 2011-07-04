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

#include "tpdph.h"
#include "quadtree.h"
#include "qtree_tmpl.h"
#include "auxdat.h"

laydata::QuadProps::QuadProps(): _numObjects(0), _invalid(false), _quadMap(0)
{}

byte laydata::QuadProps::numSubQuads() const
{
   assert(_quadMap < 16);
   switch (_quadMap)
   {
      case  0: return 0;
      case  1:
      case  2:
      case  4:
      case  8: return 1;
      case  7:
      case 11:
      case 13:
      case 14: return 3;
      case 15: return 4;
      default: return 2;
   }
}

char laydata::QuadProps::getPosition(QuadIdentificators quad)
{
   switch(quad)
   {
      case qidNW: return getNWQuad();
      case qidNE: return getNEQuad();
      case qidSE: return getSEQuad();
      case qidSW: return getSWQuad();
      default: assert(false);
   }
   // dummy statement to prevent compiler warnings
   return -1;
}

void laydata::QuadProps::addQuad(QuadIdentificators quad)
{
   _quadMap |= 0x01 << quad;
}

void laydata::QuadProps::removeQuad(QuadIdentificators quad)
{
   _quadMap &= ~(0x01 << quad);
}

char laydata::QuadProps::getSWQuad() const
{
   assert(_quadMap < 16);
   switch (_quadMap)
   {
      case  8: return  0;
      case  9:
      case 10: return  1;
      case 12: return  1;
      case 11:
      case 13:
      case 14: return  2;
      case 15: return  3;
      default: return -1;
   }
}

char laydata::QuadProps::getSEQuad() const
{
   assert(_quadMap < 16);
   switch (_quadMap % 8)
   {
      case  4: return  0;
      case  5:
      case  6: return  1;
      case  7: return  2;
      default: return -1;
   }
}

char laydata::QuadProps::getNEQuad() const
{
   assert(_quadMap < 16);
   switch (_quadMap % 4)
   {
      case 2: return 0;
      case 3: return 1;
      default: return -1;
   }

}

char laydata::QuadProps::getNWQuad() const
{
   assert(_quadMap < 16);
   if (_quadMap % 2) return 0;
   return -1;
}

//=============================================================================

template <typename DataT>
void laydata::QTStoreTmpl<DataT>::put(DataT* shape)
{
   _trunk->updateOverlap(shape->overlap());
   _data.push_back(shape);
}

///*!Create new TdtBox. Depending on sortnow input variable the new shape is
//just added to the QuadTree (using QuadTree::put()) without sorting or fit on
//the proper place (using add() */
//template <typename DataT>
//void laydata::QTStoreTmpl<DataT>::putBox(const TP& p1, const TP& p2)
//{
//   laydata::TdtBox *shape = DEBUG_NEW TdtBox(p1,p2);
//   put(shape);
//}
///*!Create new TdtPoly. Depending on sortnow input variable the new shape is
//just added to the QuadTree (using QuadTree::put()) without sorting or fit on
//the proper place (using add() */
//template <typename DataT>
//void laydata::QTStoreTmpl<DataT>::putPoly(PointVector& pl)
//{
//   laydata::TdtPoly *shape = DEBUG_NEW TdtPoly(pl);
//   put(shape);
//}
//
//template <typename DataT>
//void laydata::QTStoreTmpl<DataT>::putPoly(int4b* pl, unsigned psize)
//{
//   laydata::TdtPoly *shape = DEBUG_NEW TdtPoly(pl, psize);
//   put(shape);
//}
//
///*!Create new TdtWire. Depending on sortnow input variable the new shape is
//just added to the QuadTree (using QuadTree::put()) without sorting or fit on
//the proper place (using add() */
//template <typename DataT>
//void laydata::QTStoreTmpl<DataT>::putWire(PointVector& pl,word w)
//{
//   laydata::TdtWire *shape = DEBUG_NEW TdtWire(pl,w);
//   put(shape);
//}
///*!Create new TdtText. Depending on sortnow input variable the new shape is
//just added to the QuadTree (using QuadTree::put()) without sorting or fit on
//the proper place (using add() */
//template <typename DataT>
//void laydata::QTStoreTmpl<DataT>::putText(std::string text,CTM trans)
//{
//   laydata::TdtText *shape = DEBUG_NEW TdtText(text,trans);
//   put(shape);
//}

template <typename DataT>
void laydata::QTStoreTmpl<DataT>::commit()
{
   _trunk->resort(_data);
}



//-----------------------------------------------------------------------------
// class QTreeTmpl::Iterator
//-----------------------------------------------------------------------------
template <typename DataT>
laydata::Iterator<DataT>::Iterator() :
   _cQuad     ( NULL                 ),
   _cData     ( 0                    ),
   _qPosStack ( NULL                 ),
   _copy      ( false                )
{
}

template <typename DataT>
laydata::Iterator<DataT>::Iterator(const QTreeTmpl<DataT>& cQuad) :
   _cQuad     (&cQuad                ),
   _cData     ( 0                    ),
   _qPosStack ( DEBUG_NEW QPosStack()),
   _copy      ( false                )
{
   secureNonEmptyDown();
}

template <typename DataT>
laydata::Iterator<DataT>::Iterator(const Iterator& iter):
   _cQuad     ( iter._cQuad          ),
   _cData     ( iter._cData          ),
   _qPosStack ( iter._qPosStack      ),
   _copy      ( true                 )
{}

template <typename DataT>
const typename laydata::Iterator<DataT>& laydata::Iterator<DataT>::operator++()
{ //Prefix
   if (++_cData < _cQuad->_props._numObjects)
      return *this;
   if (nextSubQuad(0, _cQuad->_props.numSubQuads()))
      return *this;
   while (0 < _qPosStack->size())
   {
      //pop a quad
      QtPosition<DataT> prevQuad = _qPosStack->top(); _qPosStack->pop();
      _cQuad = prevQuad._cQuad;
      // Note! - if we're traversing the subquads - it means that we've already
      // traversed the eventual data in the popped quad. So go and find the next
      // quad sideways
      if (nextSubQuad(prevQuad._cSubQuad+1, _cQuad->_props.numSubQuads()))
         return *this;
   }
   // end of the container
   _cQuad = NULL;
   return *this;
}

template <typename DataT>
const typename laydata::Iterator<DataT> laydata::Iterator<DataT>::operator++(int /*unused*/)
{ //Postfix
   Iterator previous(*this);
   operator++();
   return previous;
}

template <typename DataT>
bool laydata::Iterator<DataT>::operator==(const Iterator& iter)
{
   // Presumption here is that there is no point comparing the _qPosStack fields
   // The parity of the _cQuad pointers is considered enough because they must
   // be unique anyway
   if ((NULL ==_cQuad) && (NULL == iter._cQuad)) return true;
   else if (_cQuad == iter._cQuad)
      return (_cData == iter._cData);
   else return false;
}

template <typename DataT>
bool laydata::Iterator<DataT>::operator!=(const Iterator& iter)
{
   return !operator==(iter);
}

template <typename DataT>
DataT* laydata::Iterator<DataT>::operator->()
{
   return _cQuad->_data[_cData];
}

template <typename DataT>
DataT* laydata::Iterator<DataT>::operator*()
{
   return _cQuad->_data[_cData];
}

template <typename DataT>
bool laydata::Iterator<DataT>::secureNonEmptyDown()
{
   while (0 == _cQuad->_props._numObjects)
   {
      if (0 < _cQuad->_props.numSubQuads())
      {
         _qPosStack->push(QtPosition<DataT>(_cQuad,0));
         _cQuad = _cQuad->_subQuads[0];
      }
      else assert(false); // i.e. the tree is not in traversable condition
   }
   _cData = 0;
   return true;
}

template <typename DataT>
bool laydata::Iterator<DataT>::nextSubQuad(byte quadBeg, byte quadEnd)
{
   for (byte i = quadBeg; i < quadEnd; i++)
   {
      _qPosStack->push(QtPosition<DataT>(_cQuad,i));
      _cQuad = _cQuad->_subQuads[i];
      if (secureNonEmptyDown())
         return true;
      else
      {
         QtPosition<DataT> pQuad = _qPosStack->top(); _qPosStack->pop();
         _cQuad = pQuad._cQuad;
      }
   }
   return false;
}

template <typename DataT>
laydata::Iterator<DataT>::~Iterator()
{
   if ((!_copy) && (NULL != _qPosStack))
      delete _qPosStack;
}

//-----------------------------------------------------------------------------
// class QTreeTmpl::ClipIterator
//-----------------------------------------------------------------------------
template <typename DataT>
laydata::ClipIterator<DataT>::ClipIterator() :
   Iterator<DataT> (                      ),
   _clipBox        ( TP(MIN_INT4B, MIN_INT4B),
                     TP(MAX_INT4B, MAX_INT4B))
{
}

template <typename DataT>
laydata::ClipIterator<DataT>::ClipIterator(const QTreeTmpl<DataT>& cQuad, const DBbox& clip) :
   Iterator<DataT> ( cQuad                ),
   _clipBox        ( clip                 )
{
   secureNonEmptyDown();
}

template <typename DataT>
laydata::ClipIterator<DataT>::ClipIterator(const ClipIterator& iter):
   Iterator<DataT> ( iter                 ),
   _clipBox        ( iter._clipBox        )
{}

template <typename DataT>
const typename laydata::ClipIterator<DataT>& laydata::ClipIterator<DataT>::operator++()
{ //Prefix
   Iterator<DataT>::operator++();
   return(*this);
}

template <typename DataT>
const typename laydata::ClipIterator<DataT> laydata::ClipIterator<DataT>::operator++(int /*unused*/)
{ //Postfix
   ClipIterator previous(*this);
   Iterator<DataT>::operator++();
   return previous;
}

template <typename DataT>
bool laydata::ClipIterator<DataT>::secureNonEmptyDown()
{
   if (0ll == _clipBox.cliparea(Iterator<DataT>::_cQuad->_overlap)) return false;
   while (0 == Iterator<DataT>::_cQuad->_props._numObjects)
   {
      return nextSubQuad(0,Iterator<DataT>::_cQuad->_props.numSubQuads());
   }
   Iterator<DataT>::_cData = 0;
   return true;
}

//-----------------------------------------------------------------------------
// class QTreeTmpl::DrawIterator
//-----------------------------------------------------------------------------
template <typename DataT>
laydata::DrawIterator<DataT>::DrawIterator() :
   Iterator<DataT>   (                      ),
   drawprop          ( NULL                 ),
   _transtack        ( NULL                 )
{
}

template <typename DataT>
laydata::DrawIterator<DataT>::DrawIterator(const QTreeTmpl<DataT>& cQuad, const layprop::DrawProperties& props, const CtmQueue& transtack) :
   Iterator<DataT>   ( cQuad                ),
   drawprop          (&props                ),
   _transtack        (&transtack            )
{
   secureNonEmptyDown();
}

template <typename DataT>
laydata::DrawIterator<DataT>::DrawIterator(const DrawIterator& iter):
   Iterator<DataT>   ( iter                 ),
   drawprop          ( iter.drawprop        ),
   _transtack        ( iter._transtack      )
{}

template <typename DataT>
const typename laydata::DrawIterator<DataT>& laydata::DrawIterator<DataT>::operator++()
{ //Prefix
   Iterator<DataT>::operator++();
   return(*this);
}

template <typename DataT>
const typename laydata::DrawIterator<DataT> laydata::DrawIterator<DataT>::operator++(int /*unused*/)
{ //Postfix
   DrawIterator previous(*this);
   Iterator<DataT>::operator++();
   return previous;
}

template <typename DataT>
bool laydata::DrawIterator<DataT>::secureNonEmptyDown()
{
   assert(drawprop);
   assert(_transtack);
   DBbox clip = drawprop->clipRegion();
   DBbox areal = Iterator<DataT>::_cQuad->_overlap.overlap(_transtack->front());
   if      (0ll == clip.cliparea(areal)      ) return false;
   else if (!areal.visible(drawprop->scrCtm(), drawprop->visualLimit())) return false;
   while (0 == Iterator<DataT>::_cQuad->_props._numObjects)
   {
      return nextSubQuad(0,Iterator<DataT>::_cQuad->_props.numSubQuads());
   }
   Iterator<DataT>::_cData = 0;
   return true;
}

//==============================================================================
// implicit template instantiation with a certain type parameter
template class laydata::Iterator<laydata::TdtData>;
template class laydata::Iterator<auxdata::TdtAuxData>;
template class laydata::ClipIterator<laydata::TdtData>;
template class laydata::ClipIterator<auxdata::TdtAuxData>;
template class laydata::DrawIterator<laydata::TdtData>;
template class laydata::DrawIterator<auxdata::TdtAuxData>;
template class laydata::QTStoreTmpl<laydata::TdtData>;
template class laydata::QTStoreTmpl<auxdata::TdtAuxData>;
