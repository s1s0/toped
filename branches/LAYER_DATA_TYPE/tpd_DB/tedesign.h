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
      friend class TdtLibDir;
      friend class ::InputTdtFile;
                        TdtLibrary(std::string, real, real, int, time_t, time_t);
      virtual          ~TdtLibrary();
      virtual void      read(InputTdtFile* const);
      void              dbExport(DbExportFile&);
      void              psWrite(PSFile&, const TdtCell*, const layprop::DrawProperties&);
      TdtDefaultCell*   checkCell(std::string name, bool undeflib = false);
      void              recreateHierarchy(const laydata::TdtLibDir* );
      void              registerCellRead(std::string, TdtCell*);
      CellDefin         getCellNamePair(std::string name) const;
      CellDefin         secureDefaultCell(std::string name, bool);
      void              addThisUndefCell(laydata::TdtDefaultCell*);
      void              relink(TdtLibDir*);
      void              clearLib();
      void              cleanUnreferenced();
      void              collectUsedLays(LayerDefList&) const;
      void              dbHierAdd(const TdtDefaultCell*, const TdtDefaultCell*);
      void              dbHierAddParent(const TdtDefaultCell*, const TdtDefaultCell*);
      void              dbHierRemoveParent(TdtDefaultCell*, const TdtDefaultCell*, laydata::TdtLibDir*);
      void              dbHierRemoveRoot(const TdtDefaultCell*);
      bool              dbHierCheckAncestors(const TdtDefaultCell*, const TdtDefaultCell*);
      //
      void              clearHierTree();
      static void       clearEntireHierTree();
      static void       initHierTreePtr() {_hiertree = NULL;}
      std::string       name()            const {return _name;}
      real              UU()              const {return _UU;}
      real              DBU()             const {return _DBU;}
      const CellMap&    cells()           const {return _cells;}
      TDTHierTree*      hiertree()        const {return _hiertree;}
      int               libID()           const {return _libID;}
      time_t            created()         const {return _created;}
      time_t            lastUpdated()     const {return _lastUpdated;}
      //
   protected:
      bool                 validateCells();
      TdtDefaultCell*      displaceCell(const std::string&);
      std::string          _name;         // design/library name
      int                  _libID;        // library ID
      real                 _DBU;          // Size of database units in meters
      real                 _UU;           // size of user unit in DBU
      CellMap              _cells;        // list of cells in the design
      auxdata::GrcCellMap  _grcCells;     // List of cells with geometry errors
                                          //
      static TDTHierTree*  _hiertree;     //
      time_t               _created;
      time_t               _lastUpdated;
   };

   class TdtDesign : public TdtLibrary {
   public:
                     TdtDesign(std::string, time_t, time_t, real DBU, real UU);
      virtual       ~TdtDesign();
      void           read(InputTdtFile* const);
      void           write(OutputTdtFile* const tedfile);
      int            readLibrary(OutputTdtFile* const);
      TdtCell*       addCell(std::string name, laydata::TdtLibDir*);
      void           addThisCell(laydata::TdtCell* strdefn, laydata::TdtLibDir*);
      TdtCell*       removeTopCell(std::string&, laydata::AtticList*, laydata::TdtLibDir*);
      void           renameCell(TdtDefaultCell*, std::string);
      void           removeRefdCell(std::string&, CellDefList&, laydata::AtticList*, laydata::TdtLibDir*);
      TdtData*       addBox(const LayerDef&, TP*, TP*);
      TdtData*       putBox(const LayerDef&, TP*, TP*);
      TdtData*       addPoly(const LayerDef&, PointVector*);
      TdtData*       putPoly(const LayerDef&, PointVector*);
      TdtData*       addWire(const LayerDef&, PointVector*, WireWidth);
      TdtData*       putWire(const LayerDef&, PointVector*, WireWidth);
      TdtData*       addText(const LayerDef&, std::string& text, CTM& ori);
      TdtData*       putText(const LayerDef&, std::string&, CTM&);
      TdtData*       addCellRef(laydata::CellDefin strdefn, CTM& ori);
      TdtData*       addCellARef(std::string&, CTM&, ArrayProps&);
      void           addList(AtticList*, TdtCell* tCell = NULL);
      void           addList(LayerNumber, ShapeList&);
      TdtCell*       openCell(std::string name);
      bool           editPush(const TP&, const DWordSet&);
      bool           editPrev(const bool undo = false);
      bool           editPop();
      bool           editTop();
      void           openGlDraw(layprop::DrawProperties&);
      void           openGlRender(tenderer::TopRend&);
      void           tmpDraw(const layprop::DrawProperties&, TP, TP);
      void           setTmpData(TdtTmpData* tmpdata) {_tmpdata = tmpdata;}
      void           setTmpCtm(CTM tmpctm)        {_tmpctm  = tmpctm; }
      void           mousePoint(TP p);
      void           mousePointCancel(TP&);
      void           mouseStop();
      void           mouseFlip();
      void           mouseRotate();
      void           copySelected( TP p1, TP p2);
      void           moveSelected( TP p1, TP p2, SelectList**);
      void           rotateSelected( TP p, real angle, SelectList**);
      void           flipSelected( TP p, bool Xaxis);
      void           deleteSelected(laydata::AtticList*, laydata::TdtLibDir*);
      void           destroyThis(TdtData*, const LayerDef&, laydata::TdtLibDir* );
      bool           groupSelected(std::string name, laydata::TdtLibDir*);
      ShapeList*     ungroupPrep(laydata::TdtLibDir*);
      AtticList*     ungroupThis(ShapeList*);
      bool           cutPoly(PointVector& pl, AtticList** dasao);
      bool           merge(AtticList** dasao);
      bool           stretch(int bfactor, AtticList** dasao);
      unsigned int   numSelected() const;
      DBbox          activeOverlap();
      DBbox          getVisibleOverlap(layprop::DrawProperties&);
      void           transferLayer(const LayerDef&);
      void           transferLayer(laydata::SelectList*, const LayerDef&);
      AtticList*     changeRef(ShapeList*, std::string);
      //
      void           collectParentCells(std::string&, CellDefList&);
      bool           checkActiveCell();
      bool           checkValidRef(std::string);
      void           fixUnsorted();
      void           fixReferenceOverlap(DBbox&, TdtCell* targetCell = NULL);
      void           setModified();
      void           storeViewPort(const DBbox& vp)  {_target.storeViewPort(vp);}
      DBbox*         getLastViewPort() const  { return _target.getLastViewPort();}
      TdtCell*       targetECell()            {assert(_target.checkEdit()); return _target.edit();}
      std::string    activeCellName()  const {return _target.name();}


      void           selectFromList(SelectList* ss, const DWordSet& unselable)
                                            {_target.edit()->selectFromList(ss, unselable);}
      void           unselectFromList(SelectList* ss, const DWordSet& unselable)
                                            {_target.edit()->unselectFromList(ss, unselable);}
      void           selectInBox(TP*, TP*, const DWordSet&, word layselmask, bool);
      void           unselectInBox(TP*, TP*, const DWordSet&, bool);
      AtticList*     changeSelect(TP*, const DWordSet&, bool);
      void           mouseHoover(TP&, layprop::DrawProperties&, const DWordSet&);
      void           unselectAll()    const {_target.edit()->unselectAll(false);}
      void           selectAll(const DWordSet& unselable, word layselmask) const
                                             {       _target.edit()->selectAll(unselable, layselmask);}
      void           tryUnselectAll()const;
      SelectList*    shapeSel()        const {return _target.edit()->shapeSel();}
      SelectList*    copySeList()     const {return _target.edit()->copySeList();}
      bool           modified() const       {return _modified;}
      //
   private:
      EditObject     _target;       //! edit/view target - introduced with pedit operations
      CTM            _tmpctm;
      TdtTmpData*    _tmpdata;      //! pointer to a data under construction - for view purposes
      bool           _modified;
   };

   /*! Library directory or Directory of libraries.
    *  This object contains pointers to all loaded libraries. Current database
    *  is a special case. The other special case is the library of undefined
    *  cells. It is always defined and always located in the 0 slot of the
    *  Catalog. Undefined cell library is not accessible outside of the scope of
    *  this class
    *
    *  Current database is accessible using the class functor. The class is
    *  using following library ID definitions:
    *  - ALL_LIB
    *  - TARGETDB_LIB
    *  - UNDEFCELL_LIB
    *
    *  A short memo on UNDEFCELL_LIB which proves to be a pain in the back...
    *  The decision to allow undefined structures seemed to be a good idea
    *  having in mind the it is legal in GDS to have references to undefined
    *  structures. The next argument was the introduction of the libraries.
    *  Having an undefined cell structure it would be stupid not to allow simple
    *  operations with it. For example to define it, or simply to delete the
    *  reference to it. Right? (they say that the road to hell is covered with
    *  roses). When I've started the implementation, then I realized that
    *  UNDEFCELL_LIB shall have quite different behavior from a normal library
    *  and from the target DB
    *   - New cells have to be generated on request - normally when a reference
    *     to them is added. Unlike the target DB where there is a command for
    *     this.
    *   - Unreferenced cells should be cleared (you don't want to see rubbish in
    *     the cell browser). Unlike the rest of the libraries where they simply
    *     come at the top of the hierarchy if unreferenced.
    *
    *  On top of the above comes the undo. Just a simple example. The last
    *  reference to an undefined cell had been deleted. One would think - great -
    *  we'll clean-up the definition. Well if you do that - the undo will crash,
    *  because the undefined cell reference keeps the pointer to the definition
    *  of the undefined cell. It's getting almost ridiculous, because it appears
    *  that to keep the integrity of the undo stack you have to store also the
    *  deleted definitions of the undefined cells. The complications involve the
    *  hierarchy tree, the cell browser, and basic tell functions like addCell,
    *  removeCell, group, ungroup, delete etc.
    */
   class TdtLibDir {
   public:
      typedef std::pair<std::string, TdtLibrary*> LibItem;
      typedef std::vector<LibItem*>               LibCatalog;
                        TdtLibDir();
                       ~TdtLibDir();
      TdtDesign*        operator ()() {return _TEDDB;}
      int               loadLib(std::string);
      bool              unloadLib(std::string);
      TdtLibrary*       getLib(int);
      std::string       getLibName(int);
      void              newDesign(std::string, std::string, time_t, real, real);
      bool              readDesign(std::string);
      void              writeDesign(const char* filename = NULL);
      bool              TDTcheckwrite(const TpdTime&, const TpdTime&, bool&);
      bool              TDTcheckread(const std::string filename, const TpdTime&, const TpdTime&, bool&);
      void              relink();
      void              reextractHierarchy();
      int               getLastLibRefNo();
      bool              getCellNamePair(std::string, laydata::CellDefin&);
      bool              getLibCellRNP(std::string, CellDefin&, const int libID = TARGETDB_LIB) const;
      TdtDefaultCell*   getLibCellDef(std::string, const int libID = TARGETDB_LIB) const;
      CellDefin         linkCellRef(std::string, int);
      CellDefin         addDefaultCell( std::string name, bool );
      void              addThisUndefCell(TdtDefaultCell*);
      bool              collectUsedLays(std::string, bool, LayerDefList&) const;
      void              collectUsedLays(int, LayerDefList&) const;
      void              cleanUndefLib();
      TdtDefaultCell*   displaceUndefinedCell(std::string);
      void              holdUndefinedCell(TdtDefaultCell*);
      void              deleteHeldCells();
      void              getHeldCells(CellMap*);
      LibCellLists*     getCells(int libID);
      bool              modified() const {return (NULL == _TEDDB) ? false : _TEDDB->modified();}
      std::string       tedFileName()    {return _tedFileName;}
      bool              neverSaved()     {return _neverSaved;}
   private:
      std::string       _tedFileName;
      bool              _neverSaved;
      void              addLibrary(TdtLibrary* const lib, word libRef);
      TdtLibrary*       removeLibrary( std::string );
      LibCatalog        _libdirectory;
      TdtDesign*        _TEDDB;        // toped data base
      //! Temporary storage for undefined unreferenced cell (see the comment in the class definition)
      CellMap           _udurCells;
   };

   class DrcLibrary {
   public:
                           DrcLibrary(std::string name);
      virtual              ~DrcLibrary();
      TdtDefaultCell*      checkCell(std::string name);
      void                 registerCellRead(std::string, TdtCell*);
      WordList             findSelected(const std::string &cell, TP*); //use for DRCexplainerror
      void                 openGlDraw(layprop::DrawProperties&, std::string);
      void                 openGlRender(tenderer::TopRend&, std::string, CTM&);
      std::string          name()            const {return _name;}
   protected:

      std::string          _name;         // design/library name
      CellMap              _cells;        // list of cells in the design
   };

}

#endif //TEDESIGN_H_INCLUDED
