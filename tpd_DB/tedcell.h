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
#include "quadtree.h"

namespace laydata {

//==============================================================================
   class TdtCellRef;
   class TdtCellAref;
   typedef  std::map<unsigned, QuadTree*>           LayerList;
   typedef  SGHierTree<TdtDefaultCell>              TDTHierTree;


//==============================================================================
   /*!This class is holding the information about current cell - i.e. the cell
   that is currently a target of all toped operations. The complication comes
   from edit in place operations like push/pop/top/previous as well as from the
   undo requirements of all open/edit cell operations.
   As a rule, if a cell has been opened with opencell() command, _activecell
   and _viewcell point to the same TdtCell object of the target cell, _ARTM is
   assigned to the default CTM() and _activeref and _peditchain are NULL.
   If a cell has to be open with editpush() or similar, then all of the data
   fields are coming into play.
   */
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
      bool                       securelaydef(unsigned layno);
      TdtCell*                   edit() const      {return _activecell;};
      TdtCell*                   view() const      {return _viewcell;};
      bool                       checkedit() const {return _activecell != NULL;};
      const CTM                  rARTM() const     {return _ARTM.Reversed();};
      const CTM                  ARTM() const      {return _ARTM;};
      bool                       iscell() const    {return _activeref == NULL;};
      layprop::PropertyCenter&   viewprop() const  {return *_viewprop;}
      void                       init_viewprop(layprop::PropertyCenter* viewprop) {_viewprop = viewprop;}
      void                       reset();
      static EditCellStack       _editstack;    //! the stack of all previously edited (opened) cells
   private:
      void                       unblockfill();
      void                       blockfill();
      TdtCell*                   _activecell;   //! the curently active cell
      TdtCell*                   _viewcell;     //! current topview cell - if edit in place is active
      TdtCellRef*                _activeref;    //! current topview reference - if edit in place is active
      CellRefStack*              _peditchain;   //! the path from _viewcell to the _activeref (_activecell)
      CTM                        _ARTM;         //! active reference (cell) translation matrix
      static layprop::PropertyCenter* _viewprop;
   };

//==============================================================================
   class TdtDefaultCell  {
      public:
                             TdtDefaultCell(std::string, int , bool );
         virtual            ~TdtDefaultCell() {};
         virtual void        openGL_draw(layprop::DrawProperties&, bool active=false) const;
         virtual void        openGL_render(tenderer::TopRend&, const CTM&, bool, bool) const;
         virtual void        motion_draw(const layprop::DrawProperties&, ctmqueue&, bool active=false) const;
         virtual void        PSwrite(PSFile&, const layprop::DrawProperties&,
                                      const CellList* = NULL, const TDTHierTree* = NULL) const;
         virtual TDTHierTree* hierout(TDTHierTree*& Htree, TdtCell*, CellList*, const TdtLibDir*);
         virtual bool        relink(TdtLibDir*);
         virtual void        relinkThis(std::string, laydata::CellDefin, laydata::TdtLibDir* libdir);
         virtual void        updateHierarchy(TdtLibDir*);
         virtual DBbox       cellOverlap() const;
         virtual DBbox       updateVisibleOverlap(const layprop::DrawProperties&);
         virtual void        write(TEDfile* const, const CellList&, const TDTHierTree*) const;
         virtual void        GDSwrite(DbExportFile&, const CellList&, const TDTHierTree*) const;
         virtual void        CIFwrite(DbExportFile&, const CellList&, const TDTHierTree*) const;
         virtual void        collect_usedlays(const TdtLibDir*, bool, WordList&) const;

         void                 parentfound()     {_orphan = false;};
         bool                 orphan() const    {return _orphan;};
         std::string          name() const      {return _name;};
         int                  libID() const     {return _libID;}
         //@FIXME! the _orphan must be protected!
         bool                 _orphan;       //! cell doesn't have a parent
      protected:
         void                 invalidateParents(TdtLibrary*);
         LayerList            _layers;       //! all layers the cell (including the reference layer)
         std::string          _name;         //! cell name
      private:
         int                  _libID;        //! cell belongs to ... library
   };

//==============================================================================
   class TdtCell : public TdtDefaultCell  {
   public:
                           TdtCell(std::string);
                           TdtCell(TEDfile* const, std::string, int);
      virtual             ~TdtCell();
      void                 openGL_draw(layprop::DrawProperties&,
                                                          bool active=false) const;
      void                 openGL_render(tenderer::TopRend&, const CTM&, bool, bool) const;
      void                 motion_draw(const layprop::DrawProperties&, ctmqueue&,
                                                          bool active=false) const;
      QuadTree*            securelayer(unsigned layno);
      TdtCellRef*          addcellref(TdtDesign*, CellDefin str, CTM trans,
                                                          bool sortnow = true);
      void                 registerCellRef(CellDefin str, CTM trans);
      TdtCellAref*         addcellaref(TdtDesign*, CellDefin, CTM,
                                          ArrayProperties&, bool sortnow = true);
      void                 registerCellARef(CellDefin str, CTM trans, ArrayProperties&);
      bool                 addchild(TdtDesign*, TdtDefaultCell*);
      void                 write(TEDfile* const, const CellList&, const TDTHierTree*) const;
      void                 GDSwrite(DbExportFile&, const CellList&, const TDTHierTree*) const;
      void                 CIFwrite(DbExportFile&, const CellList&, const TDTHierTree*) const;
      void                 PSwrite(PSFile&, const layprop::DrawProperties&,
                                   const CellList* = NULL, const TDTHierTree* = NULL) const;
      TDTHierTree*         hierout(TDTHierTree*&, TdtCell*, CellList*, const TdtLibDir*);
      DBbox                cellOverlap() const {return _cellOverlap;}
      DBbox                cellVisibleOverlap() const {return _vlOverlap;}
      void                 select_inBox(DBbox, layprop::PropertyCenter&, bool pntsel = false);
//      void                 select_inside(const TP);
      void                 select_fromList(SelectList*, layprop::PropertyCenter&);
//      void                 select_all(bool select_locked = false);
      void                 select_all(layprop::PropertyCenter&);
      void                 full_select();
      void                 select_this(TdtData*, unsigned);
      void                 unselect_inBox(DBbox, bool, layprop::PropertyCenter&);
      void                 unselect_fromList(SelectList*, layprop::PropertyCenter&);
      void                 unselect_all(bool destroy=false);
      bool                 addlist(TdtDesign*, AtticList*);
      bool                 copy_selected(TdtDesign*, const CTM&);
      bool                 move_selected(TdtDesign*, const CTM&, SelectList**);
      bool                 rotate_selected(laydata::TdtDesign*, const CTM&, SelectList**);
      bool                 transfer_selected(TdtDesign*, const CTM&);
      bool                 delete_selected(AtticList*, laydata::TdtLibDir* );
      bool                 destroy_this(TdtLibDir*, TdtData* ds, unsigned la);
      AtticList*           groupPrep(TdtLibDir*);
      ShapeList*           ungroupPrep(TdtLibDir*);
      void                 transferLayer(unsigned);
      void                 transferLayer(SelectList*, unsigned);
      void                 resort();
      bool                 validate_cells(TdtLibrary*);
      void                 validate_layers();
      unsigned int         numselected();
      bool                 cutpoly_selected(pointlist&, AtticList**);
      bool                 merge_selected(AtticList**);
      bool                 stretch_selected(int bfactor, AtticList**);
      AtticList*           changeselect(TP, SH_STATUS status, layprop::PropertyCenter&);
      TdtCellRef*          getcellover(TP, ctmstack&, CellRefStack*, layprop::PropertyCenter&);
      SelectList*          shapesel()        {return &_shapesel;};
      SelectList*          copy_selist() const;
      void                 updateHierarchy(TdtLibDir*);
      bool                 relink(TdtLibDir*);
      void                 relinkThis(std::string, laydata::CellDefin, laydata::TdtLibDir*);
      void                 report_selected(real) const;
      void                 collect_usedlays(const TdtLibDir*, bool, WordList&) const;
      bool                 overlapChanged(DBbox&, TdtDesign*);
      DBbox                updateVisibleOverlap(const layprop::DrawProperties&);
   private:
      bool                 getshapeover(TP, layprop::PropertyCenter&);
      void                 getCellOverlap();
      void                 store_inAttic(AtticList&);
      dword                getFullySelected(DataList*) const;
      NameSet*             rehash_children();
      ShapeList*           mergeprep(unsigned);
      bool                 unselect_pointlist(SelectDataPair&, SelectDataPair&);
      TdtData*             checkNreplacePoly(SelectDataPair&, Validator*, unsigned, SelectList**);
      TdtData*             checkNreplaceBox(SelectDataPair&, Validator*, unsigned, SelectList**);
      DataList*            secure_dataList(SelectList&, unsigned);
      NameSet              _children;     //! for hierarchy list purposes
      SelectList           _shapesel;     //! selected shapes
      DBbox                _cellOverlap;  //! Overlap of the entire cell
      DBbox                _vlOverlap;    //! Overlap of the currently visible layers only
   };
}
#endif
