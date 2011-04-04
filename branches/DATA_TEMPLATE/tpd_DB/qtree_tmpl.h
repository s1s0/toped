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
//        Created: Tue Mar 15 2011
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Database structure in the memory - clipping template
//                 (spin off from quadtree.h)
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================


#ifndef QTREE_TMPL_H_
#define QTREE_TMPL_H_

#include "quadtree.h"

namespace laydata {
   //==============================================================================
   /*!QTreeTmpl class implements the main clipping algorithm of Toped. Its
    * main purpose is to speed-up the drawing of the database. All objects of
    * type DataT or its derivatives - means all layout data - are stored into
    * objects of QTreeTmpl type.\n
    * QTreeTmpl is entirely  dynamic entity by nature. The contents of each
    * particular QTreeTmpl depends on the properties of all objects in the
    * database in terms of quantity and overlapping area. Each object of
    * QTreeTmpl class is responsible for a dynamically defined rectangular
    * area. This area is updated with every added, moved or deleted layout
    * object.\n
    * Each QTreeTmpl object might be a parent of maximum four child objects
    * of the same QTreeTmpl class, so that each of the children is responsible
    * for one out of the possible four sub-rectangles. Every layout object is
    * fitted during the construction into the smallest possible QTreeTmpl
    * object. Special attention is taken for the small objects located along
    * the central strips of the QTreeTmpl.\n
    * The children QTreeTmpl objects are created dynamically when a new object
    * is about to fit into one of the four possible sub-rectangles of the
    * overlapping area. Child QuadTree objects are stored into a dynamic
    * array of up to 4 QTreeTmpl objects (_subQuads). \n
    * From outside a QTreeTmpl object shall behave like a container -
    * abstracting out as much as possible all of the clipping, sorting etc.
    * To achieve that appropriate iterators are defined.
    */
   template <typename DataT>
   class QTreeTmpl {
   public:
      friend class Iterator<DataT>;
      friend class ClipIterator<DataT>;
      friend class DrawIterator<DataT>;
      friend class QTStoreTmpl<DataT>;
      typedef     std::list<DataT*>             TObjList;
      typedef     std::pair<DataT*, SGBitSet>   TObjDataPair;
      typedef     std::list<TObjDataPair>       TObjDataPairList;
      typedef laydata::Iterator<DataT>          Iterator;
      typedef laydata::ClipIterator<DataT>      ClipIterator;
      typedef laydata::DrawIterator<DataT>      DrawIterator;

                           QTreeTmpl();
                          ~QTreeTmpl();
      const Iterator       begin();
      const ClipIterator   begin(const DBbox&);
      const DrawIterator   begin(const layprop::DrawProperties&, const CtmQueue&);
      const Iterator       end();
      void                 openGlDraw(layprop::DrawProperties&, const TObjDataPairList*, bool) const;
      void                 openGlRender(tenderer::TopRend&, const TObjDataPairList*) const;
      short                clipType(tenderer::TopRend&) const;
      void                 add(DataT* shape);
      bool                 deleteMarked(SH_STATUS stat=sh_selected, bool partselect=false);
      bool                 deleteThis(DataT*);
      bool                 getObjectOver(const TP pnt, DataT*& prev);
      void                 validate();
      bool                 fullValidate();
      void                 resort(DataT* newdata = NULL);
      bool                 empty() const;
      void                 freeMemory();
      //! Return the overlapping box
      DBbox                overlap() const   {return _overlap;}
      //! Return the status of _invalid flag*/
      bool                 invalid() const   { return _props._invalid;}
      //! Mark the tree as invalid*/
      void                 invalidate()      {_props._invalid = true;}
   private:
      void                 resort(TObjList&);
      void                 sort(TObjList&);
      bool                 fitInTree(DataT* shape);
      char                 fitSubTree(const DBbox&, DBbox*);
      void                 tmpStore(TObjList& store);
      byte                 biggest(int8b* array) const;
      void                 updateOverlap(const DBbox& hovl);
      byte                 sequreQuad(QuadIdentificators);
      void                 removeQuad(QuadIdentificators);
      DBbox                _overlap;   //! The overlapping box of the quad
      QTreeTmpl**          _subQuads;  //! A pointers to the child QTreeTmpl structures
      DataT**              _data;      //! Pointer to The array of objects stored in this QTreeTmpl
      QuadProps            _props;     //! The structure holding the properties of this QTreeTmpl
   };
}
//    void                 visible_shapes(laydata::ShapeList*, const DBbox&, const CTM&, const CTM&, unsigned long&);
//    DataT*               getfirstover(const TP);
//    DataT*               getnextover(const TP, laydata::DataT*, bool& check);*/

#endif /* QTREE_TMPL_H_ */
