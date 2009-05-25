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

namespace layprop {
   class ViewProperties;
}

namespace laydata {

//==============================================================================
   class tdtcellref;
   class tdtcellaref;
   typedef  std::map<word, quadTree*>               layerList;
   typedef  SGHierTree<tdtdefaultcell>              TDTHierTree;


//==============================================================================
   /*!This class is holding the information about current cell - i.e. the cell
   that is currently a target of all toped operations. The complication comes
   from edit in place operations like push/pop/top/previous as well as from the 
   undo requirements of all open/edit cell operations.
   As a rule, if a cell has been opened with opencell() command, _activecell
   and _viewcell point to the same tdtcell object of the target cell, _ARTM is
   assigned to the default CTM() and _activeref and _peditchain are NULL.
   If a cell has to be open with editpush() or similar, then all of the data
   fields are comming into play.
   */
   class editobject {
   public:
                                 editobject();
                                 editobject(tdtcellref*, tdtcell*, cellrefstack*, const CTM&);
                                ~editobject();
      void                       setcell(tdtcell*);
      bool                       previous(const bool undo);
      void                       push(tdtcellref*, tdtcell*, cellrefstack*, CTM);
      bool                       pop();
      bool                       top();
      DBbox                      overlap() const;
      std::string                name() const;
      bool                       securelaydef(word layno);
      tdtcell*                   edit() const      {return _activecell;};
      tdtcell*                   view() const      {return _viewcell;};
      bool                       checkedit() const {return _activecell != NULL;};
      const CTM                  rARTM() const     {return _ARTM.Reversed();};
      const CTM                  ARTM() const      {return _ARTM;};
      bool                       iscell() const    {return _activeref == NULL;};
      layprop::ViewProperties&   viewprop() const  {return *_viewprop;}
      void                       init_viewprop(layprop::ViewProperties* viewprop) {_viewprop = viewprop;}
      static editcellstack       _editstack;    //! the stack of all previously edited (opened) cells
   private:
      void                       reset();
      void                       unblockfill();
      void                       blockfill();
      tdtcell*                   _activecell;   //! the curently active cell
      tdtcell*                   _viewcell;     //! current topview cell - if edit in place is active
      tdtcellref*                _activeref;    //! current topview reference - if edit in place is active
      cellrefstack*              _peditchain;   //! the path from _viewcell to the _activeref (_activecell)
      CTM                        _ARTM;         //! active reference (cell) translation matrix
      static layprop::ViewProperties* _viewprop;
   };

//==============================================================================
   class tdtdefaultcell  {
      public:
                             tdtdefaultcell(std::string, int , bool );
         virtual            ~tdtdefaultcell() {};
         virtual void        openGL_draw(layprop::DrawProperties&, bool active=false) const;
         virtual void        openGL_render(Tenderer&, const CTM&, DBbox&, bool, bool) const;
         virtual void        motion_draw(const layprop::DrawProperties&, ctmqueue&, bool active=false) const;
         virtual void        PSwrite(PSFile&, const layprop::DrawProperties&,
                                      const cellList* = NULL, const TDTHierTree* = NULL) const;
         virtual TDTHierTree* hierout(TDTHierTree*& Htree, tdtcell*, cellList*, const tdtlibdir*);
         virtual bool        relink(tdtlibdir*, TDTHierTree*&);
         virtual void        updateHierarchy(tdtlibdir*);
         virtual DBbox       overlap() const;
         virtual void        write(TEDfile* const, const cellList&, const TDTHierTree*) const;
         virtual void        GDSwrite(GDSin::GdsFile&, const cellList&, const TDTHierTree*, real, bool) const;
         virtual void        CIFwrite(CIFin::CifExportFile&, const cellList&, const TDTHierTree*, real, bool) const;
         virtual void        collect_usedlays(const tdtlibdir*, bool, WordList&) const;

         void                 parentfound()     {_orphan = false;};
         bool                 orphan() const    {return _orphan;};
         std::string          name() const      {return _name;};
         int                  libID() const     {return _libID;}
         layerList&           layers();      //! all layers the cell
         //@FIXME! the _orphan must be protected!
         bool                 _orphan;       //! cell doesn't have a parent
      protected:
         void                 invalidateParents(tdtlibrary*);
         layerList            _layers;       //! all layers the cell
         std::string          _name;         //! cell name
      private:
         int                  _libID;        //! cell belongs to ... library
   };

//==============================================================================
   class tdtcell : public tdtdefaultcell  {
   public:
                           tdtcell(std::string);
                           tdtcell(TEDfile* const, std::string, int);
      virtual             ~tdtcell(); 
      void                 openGL_draw(layprop::DrawProperties&,
                                                          bool active=false) const;
      void                 openGL_render(Tenderer&, const CTM&, DBbox&, bool, bool) const;
      void                 motion_draw(const layprop::DrawProperties&, ctmqueue&,
                                                          bool active=false) const;
      quadTree*            securelayer(word layno);
      tdtcellref*          addcellref(tdtdesign*, refnamepair str, CTM trans,
                                                          bool sortnow = true);
      tdtcellaref*         addcellaref(tdtdesign*, refnamepair, CTM,
                                          ArrayProperties&, bool sortnow = true);
      bool                 addchild(tdtdesign*, tdtdefaultcell*);
      void                 write(TEDfile* const, const cellList&, const TDTHierTree*) const;
      void                 GDSwrite(GDSin::GdsFile&, const cellList&,
                                                 const TDTHierTree*, real, bool) const;
      void                 CIFwrite(CIFin::CifExportFile&, const cellList&,
                                    const TDTHierTree*, real, bool) const;
      void                 PSwrite(PSFile&, const layprop::DrawProperties&,
                                   const cellList* = NULL, const TDTHierTree* = NULL) const;
      TDTHierTree*         hierout(TDTHierTree*&, tdtcell*, cellList*, const tdtlibdir*);
      DBbox                overlap() const;
      void                 select_inBox(DBbox, layprop::ViewProperties&, bool pntsel = false);
//      void                 select_inside(const TP);
      void                 select_fromList(selectList*, layprop::ViewProperties&);
//      void                 select_all(bool select_locked = false);
      void                 select_all(layprop::ViewProperties&);
      void                 full_select();
      void                 select_this(tdtdata*, word);
      void                 unselect_inBox(DBbox, bool, layprop::ViewProperties&);
      void                 unselect_fromList(selectList*, layprop::ViewProperties&);
      void                 unselect_all(bool destroy=false);
      bool                 addlist(tdtdesign*, atticList*);
      bool                 copy_selected(tdtdesign*, const CTM&);
      bool                 move_selected(tdtdesign*, const CTM&, selectList**);
      bool                 rotate_selected(laydata::tdtdesign*, const CTM&, selectList**);
      bool                 transfer_selected(tdtdesign*, const CTM&);
      bool                 delete_selected(atticList*, laydata::tdtlibdir* );
      bool                 destroy_this(tdtlibdir*, tdtdata* ds, word la);
      atticList*           groupPrep(tdtlibdir*);
      shapeList*           ungroupPrep(tdtlibdir*);
      void                 transferLayer(word);
      void                 transferLayer(selectList*, word);
      void                 resort();
      bool                 validate_cells(tdtlibrary*);
      void                 validate_layers();
      unsigned int         numselected();
      bool                 cutpoly_selected(pointlist&, atticList**);
      bool                 merge_selected(atticList**);
      bool                 stretch_selected(int bfactor, atticList**);
      atticList*           changeselect(TP, SH_STATUS status, layprop::ViewProperties&);
      tdtcellref*          getcellover(TP, ctmstack&, cellrefstack*, layprop::ViewProperties&);
      selectList*          shapesel()        {return &_shapesel;};
      selectList*          copy_selist() const;
      void                 updateHierarchy(tdtlibdir*);
      bool                 relink(tdtlibdir*, TDTHierTree*&);
      void                 removePrep(laydata::tdtdesign* ATDB) const;
      void                 report_selected(real) const;
      void                 collect_usedlays(const tdtlibdir*, bool, WordList&) const;
      bool                 overlapChanged(DBbox&, tdtdesign*);
   private:
      bool                 getshapeover(TP, layprop::ViewProperties&);
      void                 store_inAttic(atticList&);
      _dbl_word            getFullySelected(dataList*) const;
      nameList*            rehash_children();
      shapeList*           mergeprep(word);
      bool                 unselect_pointlist(selectDataPair&, selectDataPair&);
      tdtdata*             checkNreplacePoly(selectDataPair&, validator*, word, selectList**);
      tdtdata*             checkNreplaceBox(selectDataPair&, validator*, word, selectList**);
      dataList*            secure_dataList(selectList&, word);
      nameList             _children;     //! for hierarchy list purposes
      selectList           _shapesel;     //! selected shapes
   };
}
#endif
