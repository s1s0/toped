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
      subrectangles of the _overlap box.\n
      The methods can be split on several groups:
         - add a single layout object - add(), fitintree()
         - add a group of layout objects - put(), sort(), fitsubtree()
         - object selection - select_inBox(), unselect_inBox(), select_fromList(),
           select_all()
         - design modification - delete_marked()
         - tree maintanence - validate(), full_validate(), sort(), resort(),
           tmpstore()
            */
   class QuadTree {
   public:
                           QuadTree();
                           QuadTree(TEDfile* const tedfile);
      virtual             ~QuadTree();
      void                 openGL_draw(layprop::DrawProperties&, const DataList*, bool) const;
      void                 openGL_render(tenderer::TopRend&, const DataList*) const;
//      void                 visible_shapes(laydata::ShapeList*, const DBbox&, const CTM&, const CTM&, unsigned long&);
      short                clip_type(tenderer::TopRend&) const;
      virtual void         motion_draw(const layprop::DrawProperties&, ctmqueue&) const;
      void                 add(TdtData* shape);
      void                 put(TdtData* shape);
      void                 write(TEDfile* const) const;
      void                 GDSwrite(DbExportFile&) const;
      void                 CIFwrite(DbExportFile&) const;
      void                 PSwrite(PSFile&, const layprop::DrawProperties&) const;
      void                 select_inBox(DBbox&, DataList*, bool, word /*selmask = laydata::_lmall*/);
      void                 select_fromList(DataList*, DataList*);
      void                 select_all(DataList*, word selmask = laydata::_lmall, bool mark = true);
      void                 unselect_inBox(DBbox&, DataList*, bool);
      bool                 delete_marked(SH_STATUS stat=sh_selected, bool partselect=false);
      bool                 delete_this(laydata::TdtData*);
      void                 cutpoly_selected(pointlist&, DBbox&, ShapeList**);
      TdtData*             merge_selected(TdtData*& shapeRef);
/*      TdtData*             getfirstover(const TP);
      TdtData*             getnextover(const TP, laydata::TdtData*, bool& check);*/
      bool                 getobjectover(const TP pnt, laydata::TdtData*& prev);
      void                 validate();
      bool                 full_validate();
      void                 resort();
      bool                 empty() const;
      void                 freememory();
      /*! Return the overlapping box*/
      DBbox                overlap() const   {return _overlap;};
      /*! Return the overlapping box*/
      virtual void         vlOverlap(const layprop::DrawProperties&, DBbox&) const;
      /*! Mark the tree as invalid*/
      void                 invalidate()      {_invalid = true;};
      /*! Return the status of _invalid flag*/
      bool                 invalid() const   {return _invalid;};
   protected:
      DBbox               _overlap;//! The overlapping box
   private:
      void                 sort(ShapeList&);
      bool                 fitintree(TdtData* shape);
      int                  fitsubtree(const DBbox&, DBbox*);
      void                 tmpstore(ShapeList& store);
      byte                 biggest(int8b* array) const;
      void                 update_overlap(const DBbox& hovl);
      /*! A pointers to four child QuadTree structures*/
      QuadTree*           _quads[4];
      /*! Pointer to the first TdtData stored in this QuadTree*/
      TdtData*            _first;
      /*! Flag indicates that the container needs to be resorted*/
      bool                _invalid;
   };

//==============================================================================
/*! Represent the layer place holder of the tedat database. The expected
functionality is mostly implemented in the parent class. This class holds all
natural layers which means it doesn't hold cell references
*/
   class TdtLayer : public QuadTree {
   public:
                           TdtLayer() : QuadTree() {};
                           TdtLayer(TEDfile* const tedfile);
                          ~TdtLayer() {freememory();};
      void                 motion_draw(const layprop::DrawProperties&, ctmqueue& ) const;
      TdtData*             addbox(const TP& p1, const TP& p2, bool sortnow = true);
      TdtData*             addpoly(pointlist& pl, bool sortnow = true);
      TdtData*             addpoly(int4b* pl, unsigned psize, bool sortnow = true);
      TdtData*             addwire(pointlist& pl,word w, bool sortnow = true);
      TdtData*             addtext(std::string text, CTM trans, bool sortnow = true);
      virtual void         vlOverlap(const layprop::DrawProperties&, DBbox&) const;
   };

//   class TdtRefLayer : public QuadTree {
//   public:
//                           TdtRefLayer() : QuadTree(), _vlOverlap(_overlap) {}
//      virtual DBbox        vlOverlap(layprop::DrawProperties& prop);
//   private:
//      DBbox                _vlOverlap;
//   };
//
}

#endif
