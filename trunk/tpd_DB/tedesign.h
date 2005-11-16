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
//  Creation date: Sun Apr 18 10:57:53 BST 2004
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
   
#ifndef TEDESIGN_H_INCLUDED
#define TEDESIGN_H_INCLUDED

#include "tedcell.h"

namespace laydata {

   class tdtdesign {
   public:
                     tdtdesign(std::string,real DBU = 1e-9, real UU = 1e-3);
                    ~tdtdesign();                            
      void           read(TEDrecord* const tedfile);
      void           write(TEDrecord* const tedfile);
      tdtcell*       addcell(std::string name);
      tdtdata*       addbox(word la, TP* p1, TP* p2);
      tdtdata*       addpoly(word la, pointlist& pl);
      tdtdata*       addwire(word la, pointlist& pl, word w);
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
      void           openGL_draw(const layprop::DrawProperties&);
      void           tmp_draw(const layprop::DrawProperties&, TP, TP);
      void           recreate_hierarchy();
      void           mouseStart(int input_type);
      void           mousePoint(TP p);
      void           mouseStop();
      void           select_inBox(TP*, TP*, bool pntsel = false);
      atticList*     change_select(TP*, bool select = true);
      void           unselect_inBox(TP*, TP*, bool pntsel = false);
      void           copy_selected( TP p1, TP p2);
      void           move_selected( TP p1, TP p2, selectList**);
      void           rotate_selected( TP p, real angle);
      void           flip_selected( TP p, bool Xaxis);
      void           delete_selected(laydata::atticList*);
      void           destroy_this(tdtdata* ds, word la);
      bool           group_selected(std::string name);
      shapeList*     ungroup_prep();
      atticList*     ungroup_this(shapeList*);
      bool           cutpoly(pointlist& pl, atticList** dasao) {return _target.edit()->cutpoly_selected(pl,dasao);};
      bool           merge(atticList** dasao) {return _target.edit()->merge_selected(dasao);};
      unsigned int   numselected();
      DBbox          activeoverlap();
      //
      void           check_active();
      void           select_fromList(selectList* ss) {_target.edit()->select_fromList(ss);};
      void           unselect_fromList(selectList* ss) {_target.edit()->unselect_fromList(ss);};
      void           unselect_all()    const {_target.edit()->unselect_all(false);};
      selectList*    shapesel()        const {return _target.edit()->shapesel();};
      selectList*    copy_selist()     const {return _target.edit()->copy_selist();};
      unsigned int   numselected()     const {return _target.edit()->numselected();};
      void           select_all()      const {       _target.edit()->select_all();};
      void           report_selected() const {       _target.edit()->report_selected();};
      quadTree*      targetlayer(word layno) {return _target.edit()->securelayer(layno);};
      refnamepair    getcellnamepair(std::string name) const {return _cells.find(name);};
      std::string    activecellname()  const {return _target.name();};
      //
      std::string    name()            const {return _name;};
      real           UU()              const {return _UU;};
      TDTHierTree*   hiertree()        const {return _hiertree;};
      const cellList& cells()          const {return _cells;};
      bool           collect_usedlays(std::string, bool, usedlayList&) const;
      //
//      const ACTIVE_OP tellop()         const {return _tellop;};
      bool           modified;
      void          (*btreeAddMember)(const char*, const char*, int action);
      void          (*btreeRemoveMember)(const char*, const char*, bool orphan);
      friend         class TEDrecord;
      friend         void tdtcell::updateHierarchy(tdtdesign*);
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

   };
}

#endif //TEDESIGN_H_INCLUDED
