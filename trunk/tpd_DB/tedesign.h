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

   class TdtLibrary {
   public:
                     TdtLibrary(std::string, real, real, int);
      virtual       ~TdtLibrary();
      virtual void   read(TEDfile* const);
      void           GDSwrite(DbExportFile&);
      void           CIFwrite(DbExportFile&);
      void           PSwrite(PSFile&, const TdtCell*, const layprop::DrawProperties&);
      TdtDefaultCell* checkcell(std::string name, bool undeflib = false);
      void           recreate_hierarchy(const laydata::TdtLibDir* );
      void           registercellread(std::string, TdtCell*);
      CellDefin      getcellnamepair(std::string name) const;
      CellDefin      secure_defaultcell(std::string name, bool);
      void           addThisUndefCell(laydata::TdtDefaultCell*);
      void           relink(TdtLibDir*);
      void           clearLib();
      void           cleanUnreferenced();
      void           collect_usedlays(WordList&) const;
      void           dbHierAdd(const TdtDefaultCell*, const TdtDefaultCell*);
      void           dbHierAddParent(const TdtDefaultCell*, const TdtDefaultCell*);
      void           dbHierRemoveParent(TdtDefaultCell*, const TdtDefaultCell*, laydata::TdtLibDir*);
      void           dbHierRemoveRoot(const TdtDefaultCell*);
      bool           dbHierCheckAncestors(const TdtDefaultCell*, const TdtDefaultCell*);

      //
      std::string    name()            const {return _name;}
      real           UU()              const {return _UU;}
      real           DBU()             const {return _DBU;}
      const CellList& cells()          const {return _cells;}
      TDTHierTree*   hiertree()        const {return _hiertree;}
      int            libID()           const {return _libID;}
      friend         class TdtLibDir;
      friend         class TEDfile;
      void           clearHierTree();
      static void    clearEntireHierTree();
      static void    initHierTreePtr() {_hiertree = NULL;}
   protected:
      bool                 validate_cells();
      TdtDefaultCell*      displaceCell(const std::string&);
      std::string          _name;         // design/library name
      int                  _libID;        // library ID
      real                 _DBU;          // Size of database units in meters
      real                 _UU;           // size of user unit in DBU
      CellList             _cells;        // list of cells in the design
                                          //
      static TDTHierTree*  _hiertree;     //
      time_t               _created;
      time_t               _lastUpdated;
   };

   class TdtDesign : public TdtLibrary {
   public:
                     TdtDesign(std::string, time_t, time_t, real DBU = 1e-9, real UU = 1e-3);
      virtual       ~TdtDesign();
      void           read(TEDfile* const);
      void           write(TEDfile* const tedfile);
      int            readLibrary(TEDfile* const);
      TdtCell*       addcell(std::string name, laydata::TdtLibDir*);
      void           addthiscell(laydata::TdtCell* strdefn, laydata::TdtLibDir*);
      TdtCell*       removecell(std::string&, laydata::AtticList*, laydata::TdtLibDir*);
      void           removeRefdCell(std::string&, CellDefList&, laydata::AtticList*, laydata::TdtLibDir*);
      TdtData*       addbox(unsigned la, TP* p1, TP* p2, bool sortnow = true);
      TdtData*       addpoly(unsigned, pointlist*, bool sortnow = true);
      TdtData*       addwire(unsigned, pointlist*, word, bool sortnow = true);
//      void           resortlayer(unsigned);
      TdtData*       addtext(unsigned la, std::string& text, CTM& ori);
      TdtData*       addcellref(laydata::CellDefin strdefn, CTM& ori);
      TdtData*       addcellaref(std::string&, CTM&, ArrayProperties&);
      void           addlist(AtticList*/*, DWordSet&*/);
      TdtCell*       opencell(std::string name);
      bool           editpush(const TP&, const DWordSet&);
      bool           editprev(const bool undo = false);
      bool           editpop();
      bool           edittop();
      void           openGL_draw(layprop::DrawProperties&);
      void           openGL_render(tenderer::TopRend&);
      void           tmp_draw(const layprop::DrawProperties&, TP, TP);
      void           set_tmpdata(TdtTmpData* tmpdata) {_tmpdata = tmpdata;}
      void           set_tmpctm(CTM tmpctm)        {_tmpctm  = tmpctm; }
      void           mousePoint(TP p);
      void           mousePointCancel(TP&);
      void           mouseStop();
      void           mouseFlip();
      void           mouseRotate();
      void           copy_selected( TP p1, TP p2);
      void           move_selected( TP p1, TP p2, SelectList**);
      void           rotate_selected( TP p, real angle, SelectList**);
      void           flip_selected( TP p, bool Xaxis);
      void           delete_selected(laydata::AtticList*, laydata::TdtLibDir*);
      void           destroy_this(TdtData* ds, unsigned la, laydata::TdtLibDir* );
      bool           group_selected(std::string name, laydata::TdtLibDir*);
      ShapeList*     ungroup_prep(laydata::TdtLibDir*);
      AtticList*     ungroup_this(ShapeList*);
      bool           cutpoly(pointlist& pl, AtticList** dasao);
      bool           merge(AtticList** dasao) {return _target.edit()->merge_selected(dasao);}
      bool           stretch(int bfactor, AtticList** dasao) {return _target.edit()->stretch_selected(bfactor, dasao);}
      unsigned int   numselected() const;
      DBbox          activeoverlap();
      DBbox          visibleOverlap();
      void           updateVisibleOverlap(layprop::DrawProperties&);
      void           transferLayer(unsigned dst);
      void           transferLayer(laydata::SelectList* slst, unsigned dst);
      AtticList*     changeref(ShapeList*, std::string);
      //
      void           collectParentCells(std::string&, CellDefList&);
      void           check_active();
      bool           checkValidRef(std::string);

      void           selectFromList(SelectList* ss, const DWordSet& unselable)
                                            {_target.edit()->selectFromList(ss, unselable);};
      void           unselectFromList(SelectList* ss, const DWordSet& unselable)
                                            {_target.edit()->unselectFromList(ss, unselable);};
      void           selectInBox(TP*, TP*, const DWordSet&, bool pntsel = false);
      void           unselectInBox(TP*, TP*, const DWordSet&, bool pntsel = false);
      AtticList*     changeSelect(TP*, const DWordSet&, bool select = true);
      void           unselectAll()    const {_target.edit()->unselectAll(false);};
      void           selectAll(const DWordSet& unselable, word layselmask) const
                                             {       _target.edit()->selectAll(unselable, layselmask);}
//      QuadTree*      targetlayer(unsigned layno);
      void           try_unselect_all()const;
      SelectList*    shapesel()        const {return _target.edit()->shapesel();};
      SelectList*    copy_selist()     const {return _target.edit()->copy_selist();};
      void           report_selected(real DBscale) const { _target.edit()->report_selected(DBscale);};
      std::string    activecellname()  const {return _target.name();};
      //
      time_t         created()         const {return _created;}
      time_t         lastUpdated()     const {return _lastUpdated;}
      //
      bool           modified;
      friend         class TEDfile;
   private:
      TdtTmpData*    _tmpdata;      // pointer to a data under construction - for view purposes
      EditObject     _target;       // edit/view target <- introduced with pedit operations
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

   A short memo on UNDEFCELL_LIB which proves to be a pain in the back...
   The decision to allow undefined structures seemed to be a good idea having in
   mind the it is legal in GDS to have references to undefined structures. The
   next argument was the introduction of the libraries. Having an undefined
   cell structure it would be stupid not to allow simple operations whith it. For
   example to define it, or simply to delete the reference to it. Right? (they say
   that the road to hell is covered with roses). When I've started the
   implementation, then I realised that UNDEFCELL_LIB shall have quite different
   behaviour from a normal library and from the target DB
   - New cells have to be generated on request - normally when a reference to them
     is added. Unlike the target DB where there is a command for this.
   - Unreferenced cells should be cleared (you don't want to see rubbish in the cell
     browser). Unlike the rest of the libraries where they simply come at the top
     of the hirerarchy if unreferenced.
   On top of the above comes the undo. Just a simple example. The last reference to
   an undefined cell had been deleted. One would think - great - we'll clean-up the
   definition. Well if you do that - the undo will crash, because the undefined cell
   reference keeps the pointer to the definition of the undefined cell. It's getting
   alsomst rediculos, because it appears that to keep the integrity of the undo stack
   you have to store also the deleted definitions of the undefined cells. The
   complications involve the hierarchy tree, the cell browser, and basic tell
   functions like addcell, removecell, group, ungroup, delete etc.
   */
   class TdtLibDir {
   public:
      typedef std::pair<std::string, TdtLibrary*> LibItem;
      typedef std::vector<LibItem*>               Catalog;
                        TdtLibDir();
                       ~TdtLibDir();
      TdtDesign*        operator ()() {return _TEDDB;}
      void              addlibrary( TdtLibrary* const, word libRef );
      TdtLibrary*       removelibrary( std::string );
      TdtLibrary*       getLib(int);
      std::string       getLibName(int);
      void              relink();
      void              reextract_hierarchy();
      int               getLastLibRefNo();
      bool              getLibCellRNP(std::string, CellDefin&, const int libID = TARGETDB_LIB) const;
      TdtDefaultCell*   getLibCellDef(std::string, const int libID = TARGETDB_LIB) const;
      CellDefin         linkcellref(std::string, int);
      CellDefin         adddefaultcell( std::string name, bool );
      void              addThisUndefCell(TdtDefaultCell*);
      bool              collect_usedlays(std::string, bool, WordList&) const;
      void              collect_usedlays(int, WordList&) const;
      void              cleanUndefLib();
      TdtDefaultCell*   displaceUndefinedCell(std::string);
      void              holdUndefinedCell(TdtDefaultCell*);
      void              deleteHeldCells();
      void              getHeldCells(CellList*);
      bool              modified() const {return (NULL == _TEDDB) ? false : _TEDDB->modified;};
      void              deleteDB() {delete _TEDDB;}
      void              setDB(TdtDesign* newdesign) {_TEDDB = newdesign;}
      const CellList&   getUndefinedCells() {return _libdirectory[UNDEFCELL_LIB]->second->cells();}
   private:
      Catalog           _libdirectory;
      TdtDesign*        _TEDDB;        // toped data base
      //! themporary storage for undefined unreferenced cell (see the comment in the class definition)
      CellList          _udurCells;
   };



   class DrcLibrary {
   public:
                     DrcLibrary(std::string name, real DBU, real UU);
      virtual       ~DrcLibrary();
      TdtDefaultCell* checkcell(std::string name);
      void           registercellread(std::string, TdtCell*);
      std::string    name()            const {return _name;}
      real           UU()              const {return _UU;}
      real           DBU()             const {return _DBU;}
   protected:

      std::string          _name;         // design/library name
      real                 _DBU;          // Size of database units in meters
      real                 _UU;           // size of user unit in DBU
      CellList             _cells;        // list of cells in the design
   };

}

#endif //TEDESIGN_H_INCLUDED
