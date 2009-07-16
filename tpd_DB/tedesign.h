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
      virtual       ~tdtlibrary();
      virtual void   read(TEDfile* const);
      void           GDSwrite(GDSin::GdsFile&, tdtcell*, bool);
      void           CIFwrite(CIFin::CifExportFile&, tdtcell*, bool recur=true);
      void           PSwrite(PSFile&, const tdtcell*, const layprop::DrawProperties&);
      tdtcell*       checkcell(std::string name, bool undeflib = false);
      void           recreate_hierarchy(const laydata::tdtlibdir* );
      void           registercellread(std::string, tdtcell*);
      refnamepair    getcellnamepair(std::string name) const {return _cells.find(name);};
      refnamepair    secure_defaultcell(std::string name);
      void           relink(tdtlibdir*);
      void           clearLib();
      void           cleanUnreferenced();
      void           collect_usedlays(WordList&) const;
      void           dbHierAdd(const tdtdefaultcell*, const tdtdefaultcell*);
      void           dbHierAddParent(const tdtdefaultcell*, const tdtdefaultcell*);
      int            dbHierRemoveParent(const tdtdefaultcell*, const tdtdefaultcell*);
      void           dbHierRemoveRoot(const tdtdefaultcell*);
      bool           dbHierCheckAncestors(const tdtdefaultcell*, const tdtdefaultcell*);

      //
      std::string    name()            const {return _name;}
      real           UU()              const {return _UU;}
      real           DBU()             const {return _DBU;}
      const cellList& cells()          const {return _cells;}
      TDTHierTree*   hiertree()        const {return _hiertree;}
      int            libID()           const {return _libID;}
      // callbacks
      void          (*btreeAddMember)(const char*, const char*, int action);
      void          (*btreeRemoveMember)(const char*, const char*, int action);
      friend         class tdtlibdir;
      friend         class TEDfile;
      void           clearHierTree();
      static void    clearEntireHierTree();
      static void    initHierTreePtr() {_hiertree = NULL;}
   protected:
      bool                 validate_cells();
      tdtdefaultcell*      displaceCell(const std::string&);
      std::string          _name;         // design/library name
      int                  _libID;        // library ID
      real                 _DBU;          // Size of database units in meters
      real                 _UU;           // size of user unit in DBU
      cellList             _cells;        // list of cells in the design
                                          //
      static TDTHierTree*  _hiertree;     //
      time_t               _created;
      time_t               _lastUpdated;
   };

   class tdtdesign : public tdtlibrary {
   public:
                     tdtdesign(std::string, time_t, time_t, real DBU = 1e-9, real UU = 1e-3);
      virtual       ~tdtdesign();
      void           read(TEDfile* const);
      void           write(TEDfile* const tedfile);
      int            readLibrary(TEDfile* const);
      tdtcell*       addcell(std::string name, laydata::tdtlibdir*);
      bool           removecell(std::string&, laydata::atticList*, laydata::tdtlibdir*);
      tdtdata*       addbox(unsigned la, TP* p1, TP* p2);
      tdtdata*       addpoly(unsigned, const pointlist*);
      tdtdata*       addwire(unsigned, const pointlist*, word);
      tdtdata*       addtext(unsigned la, std::string& text, CTM& ori);
      tdtdata*       addcellref(laydata::refnamepair striter, CTM& ori);
      tdtdata*       addcellaref(std::string&, CTM&, ArrayProperties&);
      void           addlist(atticList*);
      tdtcell*       opencell(std::string name);
      bool           editpush(const TP&);
      bool           editprev(const bool undo = false);
      bool           editpop();
      bool           edittop();
      void           openGL_draw(layprop::DrawProperties&);
      void           openGL_render(tenderer::TopRend&);
      void           tmp_draw(const layprop::DrawProperties&, TP, TP);
      void           set_tmpdata(tdttmpdata* tmpdata) {_tmpdata = tmpdata;}
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
      void           delete_selected(laydata::atticList*, laydata::tdtlibdir*);
      void           destroy_this(tdtdata* ds, unsigned la, laydata::tdtlibdir* );
      bool           group_selected(std::string name, laydata::tdtlibdir*);
      shapeList*     ungroup_prep(laydata::tdtlibdir*);
      atticList*     ungroup_this(shapeList*);
      bool           cutpoly(pointlist& pl, atticList** dasao);
      bool           merge(atticList** dasao) {return _target.edit()->merge_selected(dasao);}
      bool           stretch(int bfactor, atticList** dasao) {return _target.edit()->stretch_selected(bfactor, dasao);}
      unsigned int   numselected() const;
      DBbox          activeoverlap();
      void           transferLayer(unsigned dst);
      void           transferLayer(laydata::selectList* slst, unsigned dst);
      atticList*     changeref(shapeList*, std::string);
      //
      void           check_active();
      bool           checkValidRef(std::string);
      void           select_fromList(selectList* ss) {_target.edit()->select_fromList(ss, _target.viewprop());};
      void           unselect_fromList(selectList* ss) {_target.edit()->unselect_fromList(ss, _target.viewprop());};
      quadTree*      targetlayer(unsigned layno);
      bool           securelaydef(unsigned layno) {return _target.securelaydef( layno);}
      void           unselect_all()    const {_target.edit()->unselect_all(false);};
      void           try_unselect_all()const;
      selectList*    shapesel()        const {return _target.edit()->shapesel();};
      selectList*    copy_selist()     const {return _target.edit()->copy_selist();};
      void           select_all()      const {       _target.edit()->select_all(_target.viewprop());};
      void           report_selected(real DBscale) const { _target.edit()->report_selected(DBscale);};
      std::string    activecellname()  const {return _target.name();};
      void           assign_properties(layprop::ViewProperties& viewprop) {_target.init_viewprop(&viewprop);}
      //
      time_t         created()         const {return _created;}
      time_t         lastUpdated()     const {return _lastUpdated;}
      //
//      const ACTIVE_OP tellop()         const {return _tellop;};
      bool           modified;
      friend         class TEDfile;
   private:
      tdttmpdata*    _tmpdata;      // pointer to a data under construction - for view purposes
      editobject     _target;       // edit/view target <- introduced with pedit operations
      CTM            _tmpctm;
   };

   /*! Library directory or Directory of libraries.
   This object contains pointers to all loaded libraries. Current database is a 
   special case. The other special case is the library of undefined cells. It is
   always defined and always located in the 0 slot of the Catalog. Undefined cell
   library is not accessible outside of the scope of this class \n
   Current database is accessible using the class functor.
   The class is using folowing library ID definitions
      ALL_LIB
      TARGETDB_LIB
      UNDEFCELL_LIB
   */
   class tdtlibdir {
   public:
      typedef std::pair<std::string, tdtlibrary*> LibItem;
      typedef std::vector<LibItem*>               Catalog;
                        tdtlibdir();
                       ~tdtlibdir();
      tdtdesign*        operator ()() {return _TEDDB;}
      void              addlibrary( tdtlibrary* const, word libRef );
      tdtlibrary*       removelibrary( std::string );
      tdtlibrary*       getLib(int);
      std::string       getLibName(int);
      void              relink();
      void              reextract_hierarchy();
      int               getLastLibRefNo();
      bool              getLibCellRNP(std::string, refnamepair&, const int libID = TARGETDB_LIB) const;
      tdtdefaultcell*   getLibCellDef(std::string, const int libID = TARGETDB_LIB) const;
      refnamepair       linkcellref(std::string, int, TDTHierTree*&);
      refnamepair       adddefaultcell( std::string name );
      bool              collect_usedlays(std::string, bool, WordList&) const;
      void              collect_usedlays(int, WordList&) const;
      void              cleanUndefLib();
      tdtdefaultcell*   displaceUndefinedCell(std::string);
      bool              modified() const {return (NULL == _TEDDB) ? false : _TEDDB->modified;};
      void              deleteDB() {delete _TEDDB;}
      void              setDB(tdtdesign* newdesign) {_TEDDB = newdesign;}
      const cellList&   getUndefinedCells() {return _libdirectory[UNDEFCELL_LIB]->second->cells();}
   private:
      Catalog           _libdirectory;
      tdtdesign*        _TEDDB;        // toped data base
   };

}

#endif //TEDESIGN_H_INCLUDED
