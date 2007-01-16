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
//        Created: Sun Apr 18 10:57:53 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: The top class in the layout database
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
   
#ifndef TEDESIGN_H_INCLUDED
#define TEDESIGN_H_INCLUDED

#include "tedcell.h"
namespace laydata {

   class tdtdesign {
   public:
      tdtdesign(std::string, time_t, time_t, real DBU = 1e-9, real UU = 1e-3);
      ~tdtdesign();
      void           read(TEDfile* const tedfile);
      void           write(TEDfile* const tedfile);
      void           GDSwrite(GDSin::GDSFile&, tdtcell*, bool);
      void           PSwrite(PSFile&, const tdtcell*, const layprop::DrawProperties&);
      tdtcell*       addcell(std::string name);
      bool           removecell(std::string&, laydata::atticList*);
      tdtdata*       addbox(word la, TP* p1, TP* p2);
      tdtdata*       addpoly(word, const pointlist*);
      tdtdata*       addwire(word, const pointlist*, word);
      tdtdata*       addtext(word la, std::string& text, CTM& ori);
      tdtdata*       addcellref(std::string& name, CTM& ori);
      tdtdata*       addcellaref(std::string& name, CTM& ori, int4b stepX, 
                                          int4b stepY, word columns, word rows);
      void           addlist(atticList*);
      tdtcell*       opencell(std::string name);
      bool           editpush(const TP&);
      bool           editprev(const bool undo = false);
      bool           editpop();
      bool           edittop();
      tdtcell*       checkcell(std::string name);
      void           openGL_draw(layprop::DrawProperties&);
      void           tmp_draw(const layprop::DrawProperties&, TP, TP);
      void           recreate_hierarchy();
      void           mouseStart(int input_type);
      void           mousePoint(TP p);
      void           mousePointCancel(TP&);
      void           mouseStop();
      void           select_inBox(TP*, TP*, bool pntsel = false);
      atticList*     change_select(TP*, bool select = true);
      void           unselect_inBox(TP*, TP*, bool pntsel = false);
      void           copy_selected( TP p1, TP p2);
      void           move_selected( TP p1, TP p2, selectList**);
      void           rotate_selected( TP p, real angle, selectList**);
      void           flip_selected( TP p, bool Xaxis);
      void           delete_selected(laydata::atticList*);
      void           destroy_this(tdtdata* ds, word la);
      bool           group_selected(std::string name);
      shapeList*     ungroup_prep();
      atticList*     ungroup_this(shapeList*);
      bool           cutpoly(pointlist& pl, atticList** dasao);
      bool           merge(atticList** dasao) {return _target.edit()->merge_selected(dasao);};
      unsigned int   numselected();
      DBbox          activeoverlap();
      //
      void           check_active();
      void           select_fromList(selectList* ss) {_target.edit()->select_fromList(ss, _target.viewprop());};
      void           unselect_fromList(selectList* ss) {_target.edit()->unselect_fromList(ss, _target.viewprop());};
      void           unselect_all()    const {_target.edit()->unselect_all(false);};
      selectList*    shapesel()        const {return _target.edit()->shapesel();};
      selectList*    copy_selist()     const {return _target.edit()->copy_selist();};
      unsigned int   numselected()     const {return _target.edit()->numselected();};
      void           select_all()      const {       _target.edit()->select_all(_target.viewprop());};
      void           report_selected() const {       _target.edit()->report_selected();};
      quadTree*      targetlayer(word layno) {return _target.edit()->securelayer(layno);};
      refnamepair    getcellnamepair(std::string name) const {return _cells.find(name);};
      std::string    activecellname()  const {return _target.name();};
      void           assign_properties(layprop::ViewProperties& viewprop) {_target.init_viewprop(&viewprop);}
      //
      std::string    name()            const {return _name;};
      real           UU()              const {return _UU;};
      TDTHierTree*   hiertree()        const {return _hiertree;};
      const cellList& cells()          const {return _cells;};
      time_t         created()         const {return _created;}
      time_t         lastUpdated()     const {return _lastUpdated;}
      bool           collect_usedlays(std::string, bool, usedlayList&) const;
      //
//      const ACTIVE_OP tellop()         const {return _tellop;};
      bool           modified;
      void          (*btreeAddMember)(const char*, const char*, int action);
      void          (*btreeRemoveMember)(const char*, const char*, bool orphan);
      friend         class TEDfile;
      friend         void tdtcell::updateHierarchy(tdtdesign*);
      friend         void tdtcell::removePrep(tdtdesign*) const;
      friend         bool tdtcell::addchild(tdtdesign*, tdtcell*);
   private:
      bool           validate_cells();
      std::string    _name;         // design name
      real           _DBU;          // Size of database units in meters
      real           _UU;           // size of user unit in DBU
      cellList       _cells;        // list of cells in the design
      TDTHierTree*   _hiertree;     // 
      tdtdata*       _tmpdata;      // pointer to a data under construction - for view purposes
      editobject     _target;       // edit/view target <- introduced with pedit operations
      time_t         _created;
      time_t         _lastUpdated;

   };
}

#endif //TEDESIGN_H_INCLUDED
