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

   class tdtlibrary {
   public:
                     tdtlibrary(std::string, real, real, int);
                    ~tdtlibrary();
      void           read(TEDfile* const);
      void           GDSwrite(GDSin::GDSFile&, tdtcell*, bool);
      void           PSwrite(PSFile&, const tdtcell*, const layprop::DrawProperties&);
      tdtcell*       checkcell(std::string name);
      void           recreate_hierarchy();
//      bool           checkValidRef(std::string);
      refnamepair    getcellnamepair(std::string name) const {return _cells.find(name);};
      bool           collect_usedlays(std::string, bool, ListOfWords&) const;
      tdtdefaultcell* secure_defaultcell(std::string name);
      //
      std::string    name()            const {return _name;}
      real           UU()              const {return _UU;}
      const cellList& cells()          const {return _cells;}
      TDTHierTree*   hiertree()        const {return _hiertree;}
      int            libID()           const {return _libID;}
      // callbacks
      void          (*btreeAddMember)(const char*, const char*, int action);
      void          (*btreeRemoveMember)(const char*, const char*, bool orphan);
      friend         void tdtcell::updateHierarchy(tdtdesign*);
      friend         void tdtcell::removePrep(tdtdesign*) const;
      friend         bool tdtcell::addchild(tdtdesign*, tdtdefaultcell*);
      friend         class TEDfile;
      static void    clearHierTree(word libID);
      static void    clearEntireHierTree();
      static void    initHierTreePtr() {_hiertree = NULL;}
   protected:
      std::string          _name;         // design/library name
      int                  _libID;        // library ID
      real                 _DBU;          // Size of database units in meters
      real                 _UU;           // size of user unit in DBU
      cellList             _cells;        // list of cells in the design
      static TDTHierTree*  _hiertree;     // 
   };

   /*! Library directory or Directory of libraries.
   This object contains pointers to all loaded libraries. Current database is the 
   only exception. Instead there is one "hidden" library and this is the library
   of undefined cells. It is always defined and always located in the 0 slot of
   the Catalog.
   Here is the library ID code:
   -1 -> current database
    0 -> library of undefened cells
    1 -> first loaded library
    etc. 
   */
   class tdtlibdir {
   public:
      typedef std::pair<std::string, tdtlibrary*> LibItem;
      typedef std::vector<LibItem*> Catalog;
                     tdtlibdir();
                    ~tdtlibdir();
      void           addlibrary( tdtlibrary* const, word libRef );
      void           closelibrary(std::string);
      tdtlibrary*    getLib(word libID);
      word           getLastLibRefNo();
      bool           getCellNamePair(std::string, refnamepair&) const;
      void           adddefaultcell( std::string name );
   private:
      Catalog        _libdirectory;
   };

   class tdtdesign : public tdtlibrary {
   public:
                     tdtdesign(std::string, time_t, time_t, real DBU = 1e-9, real UU = 1e-3);                     
      virtual       ~tdtdesign();
      virtual void   read(TEDfile* const);
      void           write(TEDfile* const tedfile);
      int            readLibrary(TEDfile* const);
      tdtcell*       addcell(std::string name);
      bool           removecell(std::string&, laydata::atticList*);
      tdtdata*       addbox(word la, TP* p1, TP* p2);
      tdtdata*       addpoly(word, const pointlist*);
      tdtdata*       addwire(word, const pointlist*, word);
      tdtdata*       addtext(word la, std::string& text, CTM& ori);
      tdtdata*       addcellref(laydata::refnamepair striter, CTM& ori);
      tdtdata*       addcellaref(std::string&, CTM&, ArrayProperties&);
      void           addlist(atticList*);
      tdtcell*       opencell(std::string name);
      bool           editpush(const TP&);
      bool           editprev(const bool undo = false);
      bool           editpop();
      bool           edittop();
      void           openGL_draw(layprop::DrawProperties&);
      void           tmp_draw(const layprop::DrawProperties&, TP, TP);
      void           set_tmpdata(tdtdata* tmpdata) {_tmpdata = tmpdata;}
      void           set_tmpctm(CTM tmpctm)        {_tmpctm  = tmpctm; }
      void           mousePoint(TP p);
      void           mousePointCancel(TP&);
      void           mouseStop();
      void           mouseFlip();
      void           mouseRotate();
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
      bool           merge(atticList** dasao) {return _target.edit()->merge_selected(dasao);}
      bool           stretch(int bfactor, atticList** dasao) {return _target.edit()->stretch_selected(bfactor, dasao);}
      unsigned int   numselected();
      DBbox          activeoverlap();
      void           transferLayer(word dst);
      void           transferLayer(laydata::selectList* slst, word dst);
      atticList*     changeref(shapeList*, std::string);
      //
      void           check_active();
      bool           checkValidRef(std::string);
      void           select_fromList(selectList* ss) {_target.edit()->select_fromList(ss, _target.viewprop());};
      void           unselect_fromList(selectList* ss) {_target.edit()->unselect_fromList(ss, _target.viewprop());};
      quadTree*      targetlayer(word layno);
      bool           securelaydef(word layno) {return _target.securelaydef( layno);}
      void           unselect_all()    const {_target.edit()->unselect_all(false);};
      selectList*    shapesel()        const {return _target.edit()->shapesel();};
      selectList*    copy_selist()     const {return _target.edit()->copy_selist();};
      unsigned int   numselected()     const {return _target.edit()->numselected();};
      void           select_all()      const {       _target.edit()->select_all(_target.viewprop());};
      void           report_selected(real DBscale) const { _target.edit()->report_selected(DBscale);};
//      refnamepair    getcellnamepair(std::string name) const {return _cells.find(name);};
      std::string    activecellname()  const {return _target.name();};
      void           assign_properties(layprop::ViewProperties& viewprop) {_target.init_viewprop(&viewprop);}
      //
      time_t         created()         const {return _created;}
      time_t         lastUpdated()     const {return _lastUpdated;}
//      bool           collect_usedlays(std::string, bool, ListOfWords&) const;
      //
//      const ACTIVE_OP tellop()         const {return _tellop;};
      bool           modified;
      friend         class TEDfile;
   private:
      bool           validate_cells();
      tdtdata*       _tmpdata;      // pointer to a data under construction - for view purposes
      editobject     _target;       // edit/view target <- introduced with pedit operations
      time_t         _created;
      time_t         _lastUpdated;
      CTM            _tmpctm;
   };
}

#endif //TEDESIGN_H_INCLUDED
