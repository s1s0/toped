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
//        Created: Sat Jan 10 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TDT I/O and database access control
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef DATA_HANDLER_INCLUDED
#define DATA_HANDLER_INCLUDED
#include "tedesign.h"
#include "cif_io.h"
#include "gds_io.h"
#include "oasis_io.h"
#include "viewprop.h"

class DataCenter {
public:
                              DataCenter(const std::string&, const std::string &);
                             ~DataCenter(); 
   bool                       GDSparse(std::string);
   void                       GDSexport(const LayerMapGds&, std::string&, bool);
   void                       GDSexport(laydata::tdtcell*, const LayerMapGds&, bool, std::string&, bool);
   void                       importGDScell(const nameList&, const LayerMapGds&, bool recur, bool over);
   void                       GDSclose();
   void                       CIFclose();
   void                       OASclose();
   CIFin::CifStatusType       CIFparse(std::string filename);
   void                       CIFexport(USMap*, bool, std::string&);
   void                       CIFexport(laydata::tdtcell*, USMap*, bool, bool, std::string&);
   bool                       cifGetLayers(nameList&);
   bool                       gdsGetLayers(ExtLayers&);
   bool                       oasGetLayers(ExtLayers&);
   void                       CIFimport(const nameList&, SIMap*, bool, bool, real);
   bool                       OASParse(std::string);
   void                       importOAScell(const nameList&, const LayerMapGds&, bool recur, bool over);
   void                       PSexport(laydata::tdtcell*, std::string&);
   bool                       TDTread(std::string);
   int                        TDTloadlib(std::string);
   bool                       TDTunloadlib(std::string);
   bool                       TDTwrite(const char* filename = NULL);
   bool                       TDTcheckwrite(const TpdTime&, const TpdTime&, bool&); 
   bool                       TDTcheckread(const std::string, const TpdTime&, const TpdTime&, bool&); 
   void                       newDesign(std::string, time_t);
   laydata::tdtdesign*        lockDB(bool checkACTcell = true);
   bool                       lockGds(GDSin::GdsInFile*&);
   bool                       lockCif(CIFin::CifFile*&);
   bool                       lockOas(Oasis::OasisInFile*&);
   void                       bpAddGdsTab();
   void                       bpAddCifTab();
   void                       bpAddOasTab();
   laydata::tdtlibrary*       getLib(int libID) {return _TEDLIB.getLib(libID);}
   int                        getLastLibRefNo() {return _TEDLIB.getLastLibRefNo();}
   bool                       getCellNamePair(std::string name, laydata::CellDefin& strdefn);
   void                       unlockDB();
   void                       unlockGds(GDSin::GdsInFile*&, bool throwexception = false);
   void                       unlockCif(CIFin::CifFile*&, bool throwexception = false);
   void                       unlockOas(Oasis::OasisInFile*& oasis_db, bool throwexception = false);
   void                       mouseStart(int input_type, std::string, const CTM, int4b, int4b, word, word);
   void                       mousePointCancel(TP&);
   void                       mousePoint(TP p);
   void                       mouseStop();
   void                       mouseFlip();
   void                       mouseRotate();
   void                       tmp_draw(const CTM&, TP, TP);
   void                       render(const CTM&);
   const laydata::cellList&   cells();
   laydata::tdtlibdir*        TEDLIB() {return &_TEDLIB;}
   laydata::LibCellLists*     getCells(int libID);
   unsigned int               numselected()           {return (NULL != _TEDLIB()) ? _TEDLIB()->numselected() : 0 ;}
   void                       defaultlayer(word layno){_curlay = layno;}
   void                       setcmdlayer(word layno) {_curcmdlay = layno;}
   word                       curlay() const          {return _curlay;}
   word                       curcmdlay() const       {return _curcmdlay;}
   std::string                tedfilename() const     {return _tedfilename;};
   bool                       neversaved()  const     {return _neversaved;}; 
   bool                       modified() const        {return _TEDLIB.modified();};

   //------------------------------------------------------------------------------------------------
   bool                       addlayer(std::string, unsigned, std::string, std::string, std::string);
   bool                       addlayer(std::string, unsigned);
   bool                       addlayer(unsigned layno);
   unsigned                   addlayer(std::string);
   bool                       isLayerExist(word);
   bool                       isLayerExist(std::string);
   void                       addline(std::string, std::string, word, byte, byte);
   void                       addcolor(std::string, byte, byte, byte, byte);
   void                       addfill(std::string, byte*);
   void                       hideLayer(word, bool);
   void                       lockLayer(word, bool);
   void                       fillLayer(word, bool);
   void                       setcellmarks_hidden(bool);
   void                       settextmarks_hidden(bool);
   void                       setcellbox_hidden(bool);
   void                       settextbox_hidden(bool);
   void                       setGrid(byte, real, std::string);
   bool                       viewGrid(byte, bool);
   void                       addRuler(TP&, TP&);
   void                       clearRulers();
   void                       switch_drawruler(bool st) {_drawruler = st;}
   bool                       drawruler() {return _drawruler;}
   LayerMapGds*               secureGdsLayMap(bool);
   LayerMapCif*               secureCifLayMap(bool);
   bool                       autopan() const         {return _properties.autopan();}
   bool                       zeroCross() const       {return _properties.zeroCross();}
   const real                 step() const            {return _properties.step();}
   const layprop::LayoutGrid* grid(byte gn) const     {return _properties.grid(gn);}
   const int4b                stepDB() const          {return _properties.stepDB();}
   const real                 UU() const              {return _properties.UU();}
   const real                 DBscale() const         {return _properties.DBscale();}
   unsigned                   getLayerNo(std::string name) const
                                                      {return _properties.getLayerNo(name);}
   std::string                getLayerName(word layno) const
                                                      {return _properties.getLayerName(layno);}
   byte                       marker_angle() const    {return _properties.marker_angle();}
   bool                       layerHidden(word layno) {return _properties.drawprop().layerHidden(layno);}
   bool                       layerLocked(word layno) {return _properties.drawprop().layerLocked(layno);}
   const WordList             getAllLayers(void)      {return _properties.getAllLayers();};
   const WordList             getLockedLayers(void)   {return _properties.getLockedLayers();};
   bool                       grid_visual(word no)    {return grid(no)->visual();}
   void                       setautopan(bool status) {_properties.setautopan(status);}
   void                       setZeroCross(bool status) {_properties.setZeroCross(status);}
   void                       setmarker_angle(byte angle)
                                                      {_properties.setmarker_angle(angle);}
   void                       setstep(real st)        {_properties.setstep(st);}
   void                       setClipRegion(DBbox clipR)
                                                      {_properties.setClipRegion(clipR);}
   void                       setScrCTM(CTM ScrCTM)   {_properties.setScrCTM(ScrCTM);}
   void                       setCurrentOp(console::ACTIVE_OP op)
                                                      {_properties.setCurrentOp(op);}
   const console::ACTIVE_OP   currentop() const       {return _properties.currentop();}
   void                       all_layers(nameList& laylist) const {_properties.all_layers(laylist);}
   void                       all_colors(nameList& colist)  const {_properties.all_colors(colist); }
   void                       all_fills(nameList& filist)   const {_properties.all_fills(filist);  }
   void                       all_lines(nameList& linelist) const {_properties.all_lines(linelist);}
   bool                       isFilled(unsigned layno) {return _properties.drawprop().isFilled(layno);}
   const byte*                getFill(word layno) {return _properties.drawprop().getFill(layno);}
   const byte*                getFill(std::string fill_name) {return _properties.drawprop().getFill(fill_name);}
   const layprop::tellRGB&    getColor(word layno) {return _properties.drawprop().getColor(layno);}
   const layprop::tellRGB&    getColor(std::string color_name) {return _properties.drawprop().getColor(color_name);}
   const layprop::LineSettings* getLine(word layno) {return _properties.drawprop().getLine(layno);}
   const layprop::LineSettings* getLine(std::string line_name) {return _properties.drawprop().getLine(line_name);}
   const std::string          getColorName(word layno) {return _properties.drawprop().getColorName(layno);}
   const std::string          getFillName(word layno) {return _properties.drawprop().getFillName(layno);}
   const std::string          getLineName(word layno) {return _properties.drawprop().getLineName(layno);}
   const WordList             upLayers() {return _properties.upLayers();}
   void                       clearUnpublishedLayers() {_properties.clearUnpublishedLayers();}
   const word                 layselmask() {return _properties.layselmask();}
   void                       setlayselmask(word lsm) {_properties.setlayselmask(lsm);}
   void                       setGdsLayMap(USMap* map)   {_properties.setGdsLayMap(map);}
   void                       setCifLayMap(USMap* map)   {_properties.setCifLayMap(map);}
   const USMap*               getGdsLayMap() const       {return _properties.getGdsLayMap();}
   const USMap*               getCifLayMap() const       {return _properties.getCifLayMap();}
   void                       saveProperties(std::string fname)
                                                      {_properties.saveProperties(fname);}
   std::string                globalDir(void) const
                                                      {return _globalDir;}
   void                       loadLayoutFonts(std::string ffn, bool vbo)
                                                      {_properties.loadLayoutFonts(ffn, vbo);}

protected:
   std::string                _tedfilename;
   bool                       _neversaved;
   void                       openGL_draw(const CTM&);
   void                       openGL_render(const CTM&); // alternative to openGL_draw
private:
   word                       _curlay;       // current drawing layer
   word                       _curcmdlay;    // layer used during current drawing operation
   bool                       _drawruler;    // draw a ruler while coposing a shape interactively
   std::string                _localDir;
   std::string                _globalDir;
   laydata::tdtlibdir         _TEDLIB;       // catalog of available TDT libraries
   GDSin::GdsInFile*          _GDSDB;        // GDS parsed data
   CIFin::CifFile*            _CIFDB;        // CIF parsed data
   Oasis::OasisInFile*        _OASDB;        // OASIS parsed data
   layprop::ViewProperties    _properties;   // properties data base
   wxMutex                    DBLock;
   wxMutex                    GDSLock;
   wxMutex                    CIFLock;
   wxMutex                    OASLock;
   wxMutex                    PROPLock;
   wxCondition*               _bpSync;       // Synchroniosation for cell browser panels

};

//=============================================================================
//             External DB mutexes (CIF/GDS etc.) explained
//                                  or
//          How to keep the main thread alive during conversions
//=============================================================================
//
// This memo relates to the following fields of the DataCenter class:
//   GDSin::GdsInFile* _GDSDB
//   CIFin::CifFile* _CIFDB
//   wxMutex         GDSLock
//   wxMutex         CIFLock
//   wxCondition*    _bpSync;
// and associated methods:
//   bool lockGds(GDSin::GdsInFile*& gds_db);
//   bool lockCif(CIFin::CifFile*& cif_db);
//   void unlockGds(GDSin::GdsInFile*& gds_db, bool throwexception = false);
//   void unlockCif(CIFin::CifFile*& cif_db, bool throwexception = false);
//   void bpAddGdsTab();
//   void bpAddCifTab();
//
//-----------------------------------------------------------------------------
//   An external database is normally processed in the secondary (parser)
//   thread, but there is also a cell browser panel which should be kept
//   updated so that the user can see the structure of the DB and can request
//   partial conversions. The cell browser panel is updated due to a message
//   posted from the parser thread to the main one which contains the user
//   event wxEVT_CMD_BROWSER. The browser traverses the DB from the main
//   thread - so we need the corresponding mutex which shall guarantee the
//   integrity. Some of the operations (the conversions) require an existing
//   DB in the memory. Below is the required set of operations:
//   - in order to work with the DB we need to obtain a valid pointer to it
//     and lock the corresponding mutex.
//   - check whether a DB exists in memory and in some cases throw an
//     exception if it doesn't.
//   - when operation with the DB is done, unlock the mutex and invalidate
//     the used pointer.
//   - post a message to the main thread to get the browser panel updated
//   We have to keep in mind also the environment in which those operations
//   are executed to asses the complexity of the task. Normally we would
//   have to use those DBs with the main DB (import/export) and the latter
//   has its own mutex. Here is the list of the possible troubles:
//   - Work with the DB when the mutex is not locked. This leads to
//     intermittent crashes/hangs which are very very hard to diagnose. Nothing
//     really helps in such a situation, maybe only meditation...
//   - Deadlocks - this kind of situation in any of the mutexes will almost
//     certainly stall one of the threads and Toped will be as good as crashed.
//   - Long waiting time to lock a mutex from the main thread. This will
//     stall the main thread which means that the message queue will not
//     be processed, the window will not be refreshed. All this creates
//     a feeling that the whole program hangs which often annoys the user.
//   - Message queue (between the threads) is out of sync. For example -
//     the main thread is trying to create a GDS browser tab while the parser
//     thread already converted the DB and deleted it. This is possibly the
//     smallest of all the evils, but just if the browsers are coded in a
//     highly defensive manner.
//
// To address all the above some rules must be obeyed. Also - every DB has
// 3 corresponding methods which must be explicitly used when an external
// database is to be processed.
// 1. NEVER use the _GDSDB/_CIFDB fields directly. Use the corresponding
//    lock method to obtain the pointer to them and unlock method to
//    invalidate it.
// 2. If the DB is not in the memory the lock method will return false
//    and the pointer to the DB (the function parameter) will be set to NULL.
//    Do not check _GDSDB/_CIFDB directly even for existence(NULL or not).
//    (See p.1)
// 3. When done with the DB, call unlock methods and use the same variable
//    which was used in the corresponding lock call as a parameter. The unlock
//    will release the mutex and will set the variable to NULL, so that it can
//    not be used after the unlock (Toped will crash if this is attempted, but
//    that's easy to diagnose and fix).
// 4. Even when the DB is about to be created (new) do not assign to the
//    _GDSDB/_CIFDB directly. Call the lock, assign to the obtained pointer,
//    and then return the new pointer to the unlock. Same is valid when the
//    DB is about to be destroyed (delete).
// 5. To prevent deadlocks the exceptions will be thrown eventually by the
//    unlock, not by the lock.
// 6. Use the pointer obtained by the lock method only if the method itself
//    returns true.
//
// Here is the code template to be followed:
//
//   GDSin::GdsInFile* AGDSDB = NULL;
//   if (lockGds(AGDSDB))
//   { // DB exists, AGDSDB contains a valid pointer to it
//     //
//     // ... processing here
//     //
//     //    you can even do:
//     // delete AGDSDB; AGDSDB = NULL;
//     //    or
//     // AGDSDB = new GDSin::GdsInFile(...);
//     //    or a combination of the above
//   }
//   else
//   { //    DB doesn't exists, AGDSDB = NULL
//     // or deadlock has been flagged. AGDSDB is unknown
//     // in any case - do not use AGDSDB by any means
//   }
//   // when done unlock must be called with the same variable
//   // which was used when the corresponding lock was called.
//   unlockGds(AGDSDB);
//   // or alternatively call
//   // unlockGds(AGDSDB, true)
//   // which will throw EXPTNactive_GDS exception if AGDSDB == NULL
//   // AGDSDB is invalid (NULL) from this point on
//
// And the last but not least! bpAdd???Tab() must be used from the
// parser thread to update the browser. The method will put the parser
// thread in sleep until the main thread finishes with the DB. This will
// prevent the main thread from eventual long waits to lock the mutex which
// in turn will keep the main window active and the message queue in sync.
// See the comments in the bpAdd???Tab() function
//
// For more info - see:
// http://docs.wxwidgets.org/stable/wx_wxmutex.html#wxmutex
// http://docs.wxwidgets.org/stable/wx_wxcondition.html#wxcondition
// http://docs.wxwidgets.org/stable/wx_wxthread.html#wxthread
//=============================================================================

void initDBLib(const std::string&, const std::string&);

#endif
