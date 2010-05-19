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
//    Description: Database structure in the memory - clipping
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

//==============================================================================
   /*! QuadTree class implements the main clipping algorithm of toped. Its main
      purpose is to speed-up the drawing of the database. All objects of type
      TdtData or its derivatives - means all layout data - are stored into the
      objects of QuadTree type. Each object of QuadTree class is responsible for a
      rectangular area defined by the _overlap field. This area is dynamic and
      updated with every added, moved or deleted layout object.\n Each QuadTree
      object might be a parent of maximum four child objects of the same QuadTree
      class, so that each of the children is responsible for one out of the
      possible four sub-rectangles. Every layout object is fitted during the
      construction into the smallest possible QuadTree object. The children are
      created dynamically when a new object is about to fit into one of the four
      possible sub-rectangles of the _overlap area. Child QuadTree objects are
      stored into an quads[4] array and each of them is responsible by convention
      for the NW(north-west), NE(north-east), SE(south-east) and SW(south-west)
      sub-rectangles of the _overlap box.\n
      The methods can be split on several groups:
         - add a single layout object - add(), fitInTree()
         - add a group of layout objects - put(), sort(), fitSubTree()
         - object selection - selectInBox(), unselectInBox(), selectFromList(),
           selectAll()
         - design modification - deleteMarked()
         - tree maintenance - validate(), fullValidate(), sort(), resort(),
           tmpStore()
            */
   class QuadTree {
   public:
                           QuadTree();
                           QuadTree(TEDfile* const, bool);
                          ~QuadTree();
      void                 openGlDraw(layprop::DrawProperties&, const DataList*, bool) const;
      void                 openGlRender(tenderer::TopRend&, const DataList*) const;
//      void                 visible_shapes(laydata::ShapeList*, const DBbox&, const CTM&, const CTM&, unsigned long&);
      short                clipType(tenderer::TopRend&) const;
      void                 motionDraw(const layprop::DrawProperties&, ctmqueue&) const;
      void                 add(TdtData* shape);
      TdtData*             addBox(const TP& p1, const TP& p2, bool sortnow = true);
      TdtData*             addPoly(pointlist& pl, bool sortnow = true);
      TdtData*             addPoly(int4b* pl, unsigned psize, bool sortnow = true);
      TdtData*             addWire(pointlist& pl,word w, bool sortnow = true);
      TdtData*             addText(std::string text, CTM trans, bool sortnow = true);
      void                 put(TdtData* shape);
      void                 write(TEDfile* const) const;
      void                 gdsWrite(DbExportFile&) const;
      void                 cifWrite(DbExportFile&) const;
      void                 psWrite(PSFile&, const layprop::DrawProperties&) const;
      void                 selectInBox(DBbox&, DataList*, bool, word /*selmask = laydata::_lmall*/);
      void                 selectFromList(DataList*, DataList*);
      void                 selectAll(DataList*, word selmask = laydata::_lmall, bool mark = true);
      void                 unselectInBox(DBbox&, DataList*, bool);
      bool                 deleteMarked(SH_STATUS stat=sh_selected, bool partselect=false);
      bool                 deleteThis(laydata::TdtData*);
      void                 cutPolySelected(pointlist&, DBbox&, ShapeList**);
      TdtData*             mergeSelected(TdtData*& shapeRef);
/*      TdtData*             getfirstover(const TP);
      TdtData*             getnextover(const TP, laydata::TdtData*, bool& check);*/
      bool                 getObjectOver(const TP pnt, laydata::TdtData*& prev);
      void                 validate();
      bool                 fullValidate();
      void                 resort(laydata::TdtData* newdata = NULL);
//      void                 resort(ShapeList&);
      bool                 empty() const;
      void                 freeMemory();
      /*! Return the overlapping box*/
      DBbox                overlap() const   {return _overlap;}
      /*! Return the overlapping box*/
      void                 vlOverlap(const layprop::DrawProperties&, DBbox&, bool) const;
      /*! Mark the tree as invalid*/
      void                 invalidate()      {_props._invalid = true;}
      /*! Return the status of _invalid flag*/
      bool                 invalid() const   {return _props._invalid;}
   private:
      typedef unsigned     ObjectIter;
      struct __attribute__ ((__packed__)) QuadProps
      {
                                QuadProps();
         byte                   numSubQuads() const;
         char                   getPosition(QuadIdentificators);
         void                   addQuad(QuadIdentificators);
         void                   removeQuad(QuadIdentificators);
         void                   clearQuadMap() {_quadMap = 0;}
         ObjectIter             _numObjects;
         /*! Flag indicates that the container needs to be resorted*/
         bool                   _invalid;
      private:
         char                   getNEQuad() const;
         char                   getNWQuad() const;
         char                   getSEQuad() const;
         char                   getSWQuad() const;
         byte                   _quadMap;
      };
      void                 sort(ShapeList&);
      bool                 fitInTree(TdtData* shape);
      char                 fitSubTree(const DBbox&, DBbox*);
      void                 tmpStore(ShapeList& store);
      byte                 biggest(int8b* array) const;
      void                 updateOverlap(const DBbox& hovl);
      byte                 sequreQuad(QuadIdentificators);
      void                 removeQuad(QuadIdentificators);
      DBbox                _overlap;//! The overlapping box of the quad
      /*! A pointers to four child QuadTree structures*/
      QuadTree**           _subQuads;
      /*! Pointer to the first TdtData stored in this QuadTree*/
      TdtData**            _data;
      QuadProps            _props;
   };
}

#endif
