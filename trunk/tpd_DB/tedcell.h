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
// ------------------------------------------------------------------------ =
//           $URL$
//  Creation date: Sun Apr 18 11:33:54 BST 2004
//         Author: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
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
   class tdtcellref;
   class tdtcellaref;

   typedef  std::map<word, quadTree*>               layerList;
   typedef  SGHierTree<tdtcell>                     TDTHierTree;


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
      void                 setcell(tdtcell*);
      bool                 previous(const bool undo);
      void                 push(tdtcellref*, tdtcell*, cellrefstack*, CTM);
      bool                 pop();
      bool                 top();
      DBbox                overlap() const;
      std::string          name() const;
      tdtcell*             edit() const      {return _activecell;};
      tdtcell*             view() const      {return _viewcell;};
      bool                 checkedit() const {return _activecell != NULL;};
      const CTM            rARTM() const     {return _ARTM.Reversed();};
      const CTM            ARTM() const      {return _ARTM;};
      bool                 iscell() const    {return _activeref == NULL;};
      static editcellstack _editstack;    //! the stack of all previously edited (opened) cells
   private:
      void                 reset();
      tdtcell*             _activecell;   //! the curently active cell
      tdtcell*             _viewcell;     //! current topview cell - if edit in place is active
      tdtcellref*          _activeref;    //! current topview reference - if edit in place is active
      cellrefstack*        _peditchain;   //! the path from _viewcell to the _activeref (_activecell)
      CTM                  _ARTM;         //! active reference (cell) translation matrix
   };

//==============================================================================
   class tdtcell  {
   public:
                           tdtcell(std::string name);
                           tdtcell(TEDfile* const tedfile, std::string name);
                          ~tdtcell(); 
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&,
                                                          bool active=false) const;
      void                 tmp_draw(const layprop::DrawProperties&, ctmqueue&,
                                                          bool active=false) const;
      quadTree*            securelayer(word layno);
      tdtcellref*          addcellref(tdtdesign*, refnamepair str, CTM trans,
                                                          bool sortnow = true);
      tdtcellaref*         addcellaref(tdtdesign*,refnamepair str, CTM trans,  
                              int4b stepX, int4b stepY, word columns, word rows, 
                                                              bool sortnow = true);
      bool                 addchild(tdtdesign*, tdtcell*);
      void                 write(TEDfile* const, const cellList&, TDTHierTree* const) const;
      void                 GDSwrite(GDSin::GDSFile&, const cellList&, TDTHierTree* const, real) const;
      TDTHierTree*         hierout(TDTHierTree*& Htree, tdtcell* parent, 
                                                           cellList* celldefs);
      DBbox                overlap() const;
      void                 select_inBox(DBbox, bool pntsel = false);
//      void                 select_inside(const TP);
      void                 select_fromList(selectList*);
      void                 select_all(bool select_locked = false);
      void                 select_this(tdtdata*, word);
      void                 unselect_inBox(DBbox, bool);
      void                 unselect_fromList(selectList*);
      void                 unselect_all(bool destroy=false);
      bool                 addlist(tdtdesign*, atticList*);
      bool                 copy_selected(tdtdesign*, const CTM&);
      bool                 move_selected(tdtdesign*, const CTM&, selectList**);
      bool                 transfer_selected(tdtdesign*, const CTM&);
      bool                 delete_selected(tdtdesign*, atticList*);
      bool                 destroy_this(tdtdesign*, tdtdata* ds, word la);
      atticList*           groupPrep(tdtdesign*);
      shapeList*           ungroupPrep(tdtdesign*);
      void                 resort();
      bool                 validate_cells(tdtdesign*);
      void                 validate_layers();
      unsigned int         numselected();
      bool                 cutpoly_selected(pointlist&,atticList**);
      bool                 merge_selected(atticList**);
      bool                 getshapeover(TP);
      atticList*           changeselect(TP, SH_STATUS status);
      tdtcellref*          getcellover(TP, ctmstack&, cellrefstack*);
      void                 parentfound()     {_orphan = false;};
      bool                 orphan() const    {return _orphan;};
      std::string          name() const      {return _name;};
      selectList*          shapesel()        {return &_shapesel;};
      selectList*          copy_selist() const;
      void                 updateHierarchy(tdtdesign*);
      void                 report_selected() const;
      void                 collect_usedlays(const tdtdesign*, bool, usedlayList&) const;
   private:
      void                 store_inAttic(atticList&);
      void                 invalidateParents(tdtdesign*);
      _dbl_word            getFullySelected(dataList*) const;
      nameList*            rehash_children();
      shapeList*           mergeprep(word);
      bool                 unselect_pointlist(selectDataPair&, selectDataPair&);
      tdtdata*             checkNreplace(selectDataPair&, validator*, word, selectList**);
      dataList*            secure_dataList(selectList&, word);
      std::string          _name;
      bool                 _orphan;
      layerList            _layers;
      nameList             _children;     // for hierarchy list purposes
      selectList           _shapesel;     // selected shapes
   };

}

#endif
