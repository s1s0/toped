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
//        Created: Sun Apr 18 11:33:54 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Layout cell handling
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TEDCELL_H_INCLUDED
#define TEDCELL_H_INCLUDED

#include <string>
#include "qtree_tmpl.h"

namespace laydata {

//==============================================================================
   typedef  SGHierTree<TdtDefaultCell>              TDTHierTree;

   typedef LayerContainer<QuadTree*>                LayerHolder;
   typedef LayerContainer<QTreeTmp*>                TmpLayerMap;

//==============================================================================
   /*!This class is holding the information about current cell - i.e. the cell
   that is currently a target of all toped operations. The complication comes
   from edit in place operations like push/pop/top/previous as well as from the
   undo requirements of all open/edit cell operations.
   As a rule, if a cell has been opened with openCell() command, _activecell
   and _viewcell point to the same TdtCell object of the target cell, _ARTM is
   assigned to the default CTM() and _activeref and _peditchain are NULL.
   If a cell has to be open with editPush() or similar, then all of the data
   fields are coming into play.
   Another feature of this class worth mentioning is the _peditchain stack. This
   variable holds the path from the _viewcell to _activecell, but NOTE in terms
   of cell references. This is used during the rendering to figure out how to
   manipulate the fill of the shapes. The idea is that the renderer takes the
   current state of this variable and on each step down the hierarchy it checks
   whether the current cell reference is the same as the reference of the top of the
   stack. If the checks is successful the fill is blocked and the stack is reduced
   by 1. This is repeated until the stack is empty which signifies that we've reached
   current active cell. The stack is restored by the renderer on its way back up
   the hierarchy.
   When edit in place mode is not active - then the _peditchain is NULL and the
   procedure above is skipped.
   */
   class TdtCellRef;
   class TdtCellAref;
   class EditObject {
   public:
                                 EditObject();
                                 EditObject(TdtCellRef*, TdtCell*, CellRefStack*, const CTM&);
                                ~EditObject();
      void                       setcell(TdtCell*);
      bool                       previous(const bool undo);
      void                       push(TdtCellRef*, TdtCell*, CellRefStack*, CTM);
      bool                       pop();
      bool                       top();
      DBbox                      overlap() const;
      std::string                name() const;
      TdtCell*                   edit() const      {return _activecell;}
      TdtCell*                   view() const      {return _viewcell;}
      bool                       checkEdit() const {return _activecell != NULL;}
      const CTM                  rARTM() const     {return _ARTM.Reversed();}
      const CTM                  ARTM() const      {return _ARTM;}
      bool                       isCell() const    {return _activeref == NULL;}
      void                       reset();
      CellRefStack*              pEditChain()      {return _peditchain;}
      static EditCellStack       _editstack;    //! the stack of all previously edited (opened) cells
      void                       storeViewPort(const DBbox&);
      DBbox*                     getLastViewPort() const;
   private:
      typedef std::map<std::string,DBbox*> ViewPortMap;
      TdtCell*                   _activecell;   //! the currently active cell
      TdtCell*                   _viewcell;     //! current topview cell - if edit in place is active
      TdtCellRef*                _activeref;    //! current topview reference - if edit in place is active
      CellRefStack*              _peditchain;   //! the path from _viewcell to the _activeref (_activecell)
      CTM                        _ARTM;         //! active reference (cell) translation matrix
      ViewPortMap                _viewPortMap;  //!
   };

//==============================================================================
   class TdtDefaultCell  {
      public:
                             TdtDefaultCell(std::string, int , bool );
         virtual            ~TdtDefaultCell();
//         virtual void        openGlDraw(layprop::DrawProperties&, bool active=false) const;
         virtual void        openGlRender(trend::TrendBase&, const CTM&, bool, bool) const;
         virtual void        motionDraw(const layprop::DrawProperties&, CtmQueue&, bool active=false) const;
         virtual TDTHierTree* hierOut(TDTHierTree*& Htree, TdtCell*, CellMap*, const TdtLibDir*);
         virtual bool        relink(TdtLibDir*);
         virtual void        relinkThis(std::string, laydata::CellDefin, laydata::TdtLibDir* libdir);
         virtual void        updateHierarchy(TdtLibDir*);
         virtual DBbox       cellOverlap() const;
         virtual DBbox       getVisibleOverlap(const layprop::DrawProperties&);
         virtual void        write(OutputTdtFile* const, const CellMap&, const TDTHierTree*) const;
         virtual void        dbExport(DbExportFile&, const CellMap&, const TDTHierTree*) const;
         virtual void        collectUsedLays(const TdtLibDir*, bool, LayerDefList&) const;
         virtual void        renameChild(std::string, std::string) {assert(false); /* TdTDefaultCell can not be renamed */}
         bool                checkLayer(const LayerDef&) const;
         void                setName(std::string nname) {_name = nname;}
         bool                orphan() const             {return _orphan;}
         void                setOrphan(bool orph)       {_orphan = orph;}
         std::string         name() const               {return _name;}
         int                 libID() const              {return _libID;}
      protected:
         void                invalidateParents(TdtLibrary*);
         LayerHolder         _layers;       //! all layers the cell (including the reference layer)
         std::string         _name;         //! cell name
         bool                _orphan;       //! cell doesn't have a parent
      private:
         int                 _libID;        //! cell belongs to ... library
   };

//==============================================================================
   class TdtCell : public TdtDefaultCell  {
   public:
                           TdtCell(std::string);
                           TdtCell(InputTdtFile* const, std::string, int);
      virtual             ~TdtCell();
//      virtual void         openGlDraw(layprop::DrawProperties&,
//                                                          bool active=false) const;
      virtual void         openGlRender(trend::TrendBase&, const CTM&, bool, bool) const;
      virtual void         motionDraw(const layprop::DrawProperties&, CtmQueue&,
                                                          bool active=false) const;
      QuadTree*            secureLayer(const LayerDef&);
      QTreeTmp*            secureUnsortedLayer(const LayerDef&);
      void                 registerCellRef(CellDefin str, CTM trans);
      void                 registerCellARef(CellDefin str, CTM trans, ArrayProps&);
      TdtCellRef*          addCellRef(TdtDesign*, CellDefin str, CTM trans);
      void                 addAuxRef(auxdata::GrcCell*);
      TdtCellAref*         addCellARef(TdtDesign*, CellDefin, CTM, ArrayProps&);
      bool                 addChild(TdtDesign*, TdtDefaultCell*);
      virtual void         write(OutputTdtFile* const, const CellMap&, const TDTHierTree*) const;
      virtual void         dbExport(DbExportFile&, const CellMap&, const TDTHierTree*) const;
      virtual TDTHierTree* hierOut(TDTHierTree*&, TdtCell*, CellMap*, const TdtLibDir*);
      virtual DBbox        cellOverlap() const {return _cellOverlap;}
      void                 selectInBox(DBbox, const LayerDefSet&, word, bool pntsel = false);
//      void                 select_inside(const TP);
      void                 selectFromList(SelectList*, const LayerDefSet&);
//      void                 selectAll(bool select_locked = false);
      void                 selectAll(const LayerDefSet&, word);
      void                 fullSelect();
      void                 selectThis(TdtData*, const LayerDef&);
      void                 unselectInBox(DBbox, bool, const LayerDefSet&);
      void                 unselectFromList(SelectList*, const LayerDefSet&);
      void                 unselectAll(bool destroy=false);
      void                 addList(TdtDesign*, AtticList*);
      void                 copySelected(const CTM&);
      void                 moveSelected(const CTM&, SelectList**);
      void                 rotateSelected(const CTM&, SelectList**);
      void                 transferSelected(const CTM&);
      void                 deleteSelected(AtticList*, laydata::TdtLibDir* );
      void                 destroyThis(TdtLibDir*, TdtData*, const LayerDef&);
      AtticList*           groupPrep(TdtLibDir*);
      ShapeList*           ungroupPrep(TdtLibDir*);
      void                 transferLayer(const LayerDef&);
      void                 transferLayer(SelectList*, const LayerDef&);
//      void                 resort();
      void                 fixUnsorted();
      bool                 validateCells(TdtLibrary*);
      void                 validateLayers();
      unsigned int         numSelected();
      bool                 cutPolySelected(PointVector&, AtticList**);
      bool                 mergeSelected(AtticList**);
      bool                 stretchSelected(int bfactor, AtticList**);
      AtticList*           changeSelect(TP, SH_STATUS status, const LayerDefSet&);
      void                 mouseHoover(TP&, trend::TrendBase&, const LayerDefSet&);
      laydata::AtticList*  findSelected(TP);
      TdtCellRef*          getCellOver(TP, CtmStack&, CellRefStack*, const LayerDefSet&);
      SelectList*          shapeSel()        {return &_shapesel;};
      SelectList*          copySeList() const;
      virtual void         updateHierarchy(TdtLibDir*);
      virtual bool         relink(TdtLibDir*);
      virtual void         relinkThis(std::string, laydata::CellDefin, laydata::TdtLibDir*);
      void                 reportSelected(real) const;
      virtual void         collectUsedLays(const TdtLibDir*, bool, LayerDefList&) const;
      bool                 overlapChanged(DBbox&, TdtDesign*);
      virtual DBbox        getVisibleOverlap(const layprop::DrawProperties&);
      virtual void         renameChild(std::string, std::string);
      auxdata::GrcCell*    getGrcCell();
      void                 clearGrcCell();
   private:
      void                 readTdtLay(InputTdtFile* const);
      void                 readTdtRef(InputTdtFile* const);
      bool                 getShapeOver(TP, const LayerDefSet&);
      void                 getCellOverlap();
      void                 storeInAttic(AtticList&);
      dword                getFullySelected(DataList*) const;
      NameSet*             rehashChildren();
      ShapeList*           mergePrep(const LayerDef&);
      bool                 unselectPointList(SelectDataPair&, SelectDataPair&);
      ShapeList*           checkNreplacePoly(SelectDataPair&, Validator*, const LayerDef&, SelectList**);
      ShapeList*           checkNreplaceBox(SelectDataPair&, Validator*, const LayerDef&, SelectList**);
      DataList*            secureDataList(SelectList&, const LayerDef&);
      void                 selectAllWrapper(QuadTree*, DataList*, word selmask = laydata::_lmall, bool mark = true);
      void                 selectFromListWrapper(QuadTree*, DataList*, DataList*);
      TdtData*             mergeWrapper(QuadTree*, TdtData*&);
      NameSet              _children;     //! for hierarchy list purposes
      SelectList           _shapesel;     //! selected shapes
      DBbox                _cellOverlap;  //! Overlap of the entire cell
      TmpLayerMap          _tmpLayers;    //! All layers with unsorted data
   };
}
#endif
