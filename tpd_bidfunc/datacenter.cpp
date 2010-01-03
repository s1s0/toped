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

#include "tpdph.h"
#include <wx/wx.h>
#include <wx/regex.h>
#include <sstream>
#include "datacenter.h"
#include "outbox.h"
#include "tedat.h"
#include "viewprop.h"
#include "ps_out.h"
#include "tenderer.h"

// Global variables
DataCenter*                      DATC  = NULL;
extern layprop::PropertyCenter*  PROPC;
extern const wxEventType         wxEVT_CMD_BROWSER;

//-----------------------------------------------------------------------------
// class DataCenter
//-----------------------------------------------------------------------------
DataCenter::DataCenter(const std::string& localDir, const std::string& globalDir)
{
   _localDir = localDir;
   _globalDir = globalDir;
   _GDSDB = NULL; _CIFDB = NULL;_OASDB = NULL; _DRCDB = NULL;//_TEDDB = NULL;
   _bpSync = NULL;
   // initializing the static cell hierarchy tree
   laydata::TdtLibrary::initHierTreePtr();
   _tedfilename = "unnamed";
   _drawruler = false;
}

DataCenter::~DataCenter() {
   laydata::TdtLibrary::clearEntireHierTree();
   if (NULL != _GDSDB) delete _GDSDB;
   if (NULL != _CIFDB) delete _CIFDB;
   if (NULL != _OASDB) delete _OASDB;
	if (NULL != _DRCDB) delete _DRCDB;
   // _TEDLIB will be cleared automatically (not a pointer)
}
bool DataCenter::TDTcheckread(const std::string filename,
    const TpdTime& timeCreated, const TpdTime& timeSaved, bool& start_ignoring)
{
   bool retval = false;
   start_ignoring = false;
   laydata::TEDfile tempin(filename.c_str(), TEDLIB());
   if (!tempin.status()) return retval;

   std::string news = "Project created: ";
   TpdTime timec(tempin.created()); news += timec();
   tell_log(console::MT_INFO,news);
   news = "Last updated: ";
   TpdTime timeu(tempin.lastUpdated()); news += timeu();
   tell_log(console::MT_INFO,news);
   // File created time stamp must match exactly, otherwise it means
   // that we're reading not exactly the same file that is requested
   if (timeCreated != timec)
   {
      news = "time stamp \"Project created \" doesn't match";
      tell_log(console::MT_ERROR,news);
   }
   if (timeu.stdCTime() < timeSaved.stdCTime())
   {
      news = "time stamp \"Last updated \" is too old.";
      tell_log(console::MT_ERROR,news);
   }
   else if (timeu.stdCTime() > timeSaved.stdCTime())
   {
      news = "time stamp \"Last updated \" is is newer than requested.";
      news +="Some of the following commands will be ignored";
      tell_log(console::MT_WARNING,news);
      //Start ignoring
      start_ignoring = true;
      retval = true;
   }
   else
   {
      retval = true;
   }
   tempin.closeF();
   return retval;
}

bool DataCenter::TDTread(std::string filename)
{
   laydata::TEDfile tempin(filename.c_str(), &_TEDLIB);
   if (!tempin.status()) return false;

   try
   {
      tempin.read(TARGETDB_LIB);
   }
   catch (EXPTNreadTDT)
   {
      tempin.closeF();
      tempin.cleanup();
      return false;
   }
   tempin.closeF();
   _TEDLIB.deleteDB();//Erase existing data
   _tedfilename = filename;
   _neversaved = false;
   _TEDLIB.setDB(static_cast<laydata::TdtDesign*>(tempin.design()));
   _TEDLIB()->assign_properties(*PROPC);//TODO <-- This shoudn't be needed anymore having a global variable
   // Update Canvas scale
   PROPC->setUU(_TEDLIB()->UU());
   return true;
}

int DataCenter::TDTloadlib(std::string filename)
{
   laydata::TEDfile tempin(filename.c_str(), &_TEDLIB);
   if (!tempin.status()) return -1;
   int libRef = _TEDLIB.getLastLibRefNo();
   try
   {
      tempin.read(libRef);
   }
   catch (EXPTNreadTDT)
   {
      tempin.closeF();
      tempin.cleanup();
      return -1;
   }
   tempin.closeF();
   _TEDLIB.addlibrary(tempin.design(), libRef);
   // Relink everything
   _TEDLIB.relink();
   return libRef;
}

bool DataCenter::TDTunloadlib(std::string libname)
{
   // Unhook the library from the list of known DB's, get a pointer to it
   laydata::TdtLibrary* tberased = _TEDLIB.removelibrary(libname);
   if ( NULL != tberased )
   {
      // Relink everything
      _TEDLIB.relink();
      // remove tberased cells from hierarchy tree
      tberased->clearHierTree();
      // get the new hierarchy
      _TEDLIB.reextract_hierarchy();
      // after all above - remove the library
      delete tberased;
      return true;
   }
   else return false;
}


bool DataCenter::TDTcheckwrite(const TpdTime& timeCreated, const TpdTime& timeSaved, bool& stop_ignoring)
{
   std::string news;
   stop_ignoring = false;
   // File created time stamp must match exactly, otherwise it means
   // that we're saving not exactly the same file that is requested
   if (timeCreated.stdCTime() != _TEDLIB()->created())
   {
      news = "time stamp \"Project created \" doesn't match. File save aborted";
      tell_log(console::MT_ERROR,news);
      return false;
   }
   if (_TEDLIB()->lastUpdated() < timeSaved.stdCTime())
   {
      news = "Database in memory is older than the file. File save operation ignored.";
      tell_log(console::MT_WARNING,news);
      _neversaved = false;
      return false;
   }
   else if (_TEDLIB()->lastUpdated() > timeSaved.stdCTime())
      // database in memory is newer than the file - save has to go ahead
      // ignore on recovery has to stop
      stop_ignoring = true;
   else
   {
      // database in memory is exactly the same as the file. The save
      // is going to be spared, ignore on recovery though has to stop
      stop_ignoring = true;
      return false;
   }
   return true;
}

bool DataCenter::TDTwrite(const char* filename)
{
   if (filename)  _tedfilename = filename;
   laydata::TEDfile tempin(_tedfilename, &_TEDLIB);
   _neversaved = false;
   return true;
}

void DataCenter::GDSexport(const LayerMapExt& layerMap, std::string& filename, bool x2048)
{
   GDSin::GdsExportFile gdsex(filename, NULL, layerMap, true);
   _TEDLIB()->GDSwrite(gdsex);
}

void DataCenter::GDSexport(laydata::TdtCell* cell, const LayerMapExt& layerMap, bool recur, std::string& filename, bool x2048)
{
   GDSin::GdsExportFile gdsex(filename, cell, layerMap, recur);
   _TEDLIB()->GDSwrite(gdsex);
}

bool DataCenter::GDSparse(std::string filename)
{
   bool status = true;

   GDSin::GdsInFile* AGDSDB = NULL;
   if (lockGds(AGDSDB))
   {
      std::string news = "Removing existing GDS data from memory...";
      tell_log(console::MT_WARNING,news);
      delete AGDSDB;
   }
   try
   {
#ifdef GDSCONVERT_PROFILING
      HiResTimer profTimer;
#endif
      AGDSDB = DEBUG_NEW GDSin::GdsInFile(filename);
#ifdef GDSCONVERT_PROFILING
      profTimer.report("Time elapsed for GDS parse: ");
#endif
   }
   catch (EXPTNreadGDS)
   {
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      status = false;
   }
   if (status)
      AGDSDB->hierOut();// generate the hierarchy tree of cells
   else if (NULL != AGDSDB)
   {
      delete AGDSDB;
      AGDSDB = NULL;
   }
   unlockGds(AGDSDB);
   return status;
}

void DataCenter::importGDScell(const nameList& top_names, const LayerMapExt& laymap, bool recur, bool over)
{
   GDSin::GdsInFile* AGDSDB = NULL;
   if (lockGds(AGDSDB))
   {
#ifdef GDSCONVERT_PROFILING
      HiResTimer profTimer;
#endif
      GDSin::Gds2Ted converter(AGDSDB, &_TEDLIB, laymap);
      converter.run(top_names, recur, over);
      _TEDLIB()->modified = true;
#ifdef GDSCONVERT_PROFILING
      profTimer.report("Time elapsed for GDS conversion: ");
#endif
   }
   unlockGds(AGDSDB, true);
}

void DataCenter::GDSclose()
{
   GDSin::GdsInFile* AGDSDB = NULL;
   if (lockGds(AGDSDB))
   {
      delete AGDSDB;
      AGDSDB = NULL;
   }
   unlockGds(AGDSDB);
}

void DataCenter::CIFclose()
{
   CIFin::CifFile* ACIFDB = NULL;
   if (lockCif(ACIFDB))
   {
      delete ACIFDB;
      ACIFDB = NULL;
   }
   unlockCif(ACIFDB);
}

void DataCenter::OASclose()
{
   Oasis::OasisInFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
      delete AOASDB;
      AOASDB = NULL;
   }
   unlockOas(AOASDB);
}

CIFin::CifStatusType DataCenter::CIFparse(std::string filename)
{
   CIFin::CifFile* ACIFDB = NULL;
   if (lockCif(ACIFDB))
   {
      std::string news = "Removing existing CIF data from memory...";
      tell_log(console::MT_WARNING,news);
      delete ACIFDB;
   }
   ACIFDB = DEBUG_NEW CIFin::CifFile(filename);

   CIFin::CifStatusType status = ACIFDB->status();
   if (CIFin::cfs_POK == status)
   {
      // generate the hierarchy tree of cells
      ACIFDB->hierPrep();
      ACIFDB->hierOut();
   }
   else
   {
      if (NULL != ACIFDB)
      {
         delete ACIFDB;
         ACIFDB = NULL;
      }
   }
   unlockCif(ACIFDB);
   return status;
}

void DataCenter::CIFexport(USMap* laymap, bool verbose, std::string& filename)
{
   CIFin::CifExportFile cifex(filename, NULL, laymap, true, verbose);
   _TEDLIB()->CIFwrite(cifex);
}

void DataCenter::CIFexport(laydata::TdtCell* topcell, USMap* laymap, bool recur, bool verbose, std::string& filename)
{
   CIFin::CifExportFile cifex(filename, topcell, laymap, recur, verbose);
   _TEDLIB()->CIFwrite(cifex);
}


bool DataCenter::cifGetLayers(nameList& cifLayers)
{
   bool ret_value = false;
   CIFin::CifFile* ACIFDB = NULL;
   if (lockCif(ACIFDB))
   {
      ACIFDB->collectLayers(cifLayers);
      ret_value = true;
   }
   unlockCif(ACIFDB);
   return ret_value;
}

bool DataCenter::gdsGetLayers(ExtLayers& gdsLayers)
{
   bool ret_value = false;
   GDSin::GdsInFile* AGDSDB = NULL;
   if (lockGds(AGDSDB))
   {
      AGDSDB->collectLayers(gdsLayers);
      ret_value = true;
   }
   unlockGds(AGDSDB);
   return ret_value;
}

bool DataCenter::oasGetLayers(ExtLayers& oasLayers)
{
   bool ret_value = false;
   Oasis::OasisInFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
      AOASDB->collectLayers(oasLayers);
      ret_value = true;
   }
   unlockOas(AOASDB);
   return ret_value;
}

void DataCenter::CIFimport( const nameList& top_names, SIMap* cifLayers, bool recur, bool overwrite, real techno )
{
   // DB shold have been locked at this point (from the tell functions)
   CIFin::CifFile* ACIFDB = NULL;
   if (lockCif(ACIFDB))
   {
      CIFin::Cif2Ted converter(ACIFDB, &_TEDLIB, cifLayers, techno);
      for (nameList::const_iterator CN = top_names.begin(); CN != top_names.end(); CN++)
         converter.top_structure(*CN, recur, overwrite);
      _TEDLIB()->modified = true;
      tell_log(console::MT_INFO,"Done");
   }
   unlockCif(ACIFDB, true);
}

bool DataCenter::OASParse(std::string filename)
{
   bool status = true;

   Oasis::OasisInFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
      std::string news = "Removing existing OASIS data from memory...";
      tell_log(console::MT_WARNING,news);
      delete AOASDB;
   }
   try
   {
      AOASDB = DEBUG_NEW Oasis::OasisInFile(filename);
      status = AOASDB->status();
      if (status)
         AOASDB->readLibrary();
   }
   catch (EXPTNreadOASIS)
   {
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      status = false;
   }
   if (status)
      AOASDB->hierOut();// generate the hierarchy tree of cells
   else if (NULL != AOASDB)
   {
      delete AOASDB;
      AOASDB = NULL;
   }
   unlockOas(AOASDB);
   return status;
}

void DataCenter::importOAScell(const nameList& top_names, const LayerMapExt& laymap, bool recur, bool over)
{
   Oasis::OasisInFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
#ifdef OASCONVERT_PROFILING
      HiResTimer profTimer;
#endif
      Oasis::Oas2Ted converter(AOASDB, &_TEDLIB, laymap);
      converter.run(top_names, recur, over);
      _TEDLIB()->modified = true;
#ifdef OASCONVERT_PROFILING
      profTimer.report("Time elapsed for OASIS conversion: ");
#endif
   }
   unlockOas(AOASDB, true);
}


void DataCenter::PSexport(laydata::TdtCell* cell, std::string& filename)
{
   //Get actual time
   PSFile psex(filename);
   PROPC->drawprop().psWrite(psex);
   _TEDLIB()->PSwrite(psex, cell, PROPC->drawprop());
//   gdsex.closeFile();
}

void DataCenter::newDesign(std::string name, time_t created)
{
   if (_TEDLIB())
   {
      // Checks before closing(save?) available only when the command is launched
      // via GUI(void TopedFrame::OnNewDesign(). If the command is typed directly
      // on the command line, or parsed from file - no checks are executed.
      // In other words if we are already here we will destroy the current design
      // without much talking.
      // UNDO buffers will be reset as well in tellstdfunc::stdNEWDESIGN::execute()
      // but there is still a chance to restore everything - using the log file.
      _TEDLIB()->clearHierTree();
      _TEDLIB.deleteDB();
   }
   _TEDLIB.setDB(DEBUG_NEW laydata::TdtDesign(name, created, created));
   _TEDLIB()->assign_properties(*PROPC);//TODO <-- This shoudn't be needed anymore having a global variable
   _tedfilename = _localDir + name + ".tdt";
   _neversaved = true;
   PROPC->setUU(_TEDLIB()->UU());
}

laydata::TdtDesign*  DataCenter::lockDB(bool checkACTcell)
{
   if (_TEDLIB())
   {
      if (checkACTcell) _TEDLIB()->check_active();
      while (wxMUTEX_NO_ERROR != _DBLock.TryLock());
      return _TEDLIB();
   }
   else throw EXPTNactive_DB();
}

laydata::DrcLibrary*  DataCenter::lockDRC(void)
{
   if (!_TEDLIB()) throw EXPTNactive_DB();
   if (!_DRCDB)
   {
         _DRCDB = DEBUG_NEW laydata::DrcLibrary("drc", _TEDLIB()->DBU(), _TEDLIB()->UU());
   }
   while (wxMUTEX_NO_ERROR != _DRCLock.TryLock());
   return _DRCDB;
}

void DataCenter::unlockDB()
{
   VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
}

void DataCenter::unlockDRC()
{
   VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
}

bool DataCenter::lockGds(GDSin::GdsInFile*& gds_db)
{
   if (wxMUTEX_DEAD_LOCK == _GDSLock.Lock())
   {
      tell_log(console::MT_ERROR,"GDS Mutex deadlocked!");
      gds_db = _GDSDB;
      return false;
   }
   else
   {
      gds_db = _GDSDB;
      return (NULL != gds_db);
   }
}

void DataCenter::unlockGds(GDSin::GdsInFile*& gds_db, bool throwexception)
{
   _GDSDB = gds_db;
   VERIFY(wxMUTEX_NO_ERROR == _GDSLock.Unlock());
   if(NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == gds_db))
      throw EXPTNactive_GDS();
   gds_db = NULL;
}

bool DataCenter::lockCif(CIFin::CifFile*& cif_db)
{
   if (wxMUTEX_DEAD_LOCK == _CIFLock.Lock())
   {
      tell_log(console::MT_ERROR,"CIF Mutex deadlocked!");
      cif_db = _CIFDB;
      return false;
   }
   else
   {
      cif_db = _CIFDB;
      return (NULL != cif_db);
   }
}

void DataCenter::unlockCif(CIFin::CifFile*& cif_db, bool throwexception)
{
   _CIFDB = cif_db;
   VERIFY(wxMUTEX_NO_ERROR == _CIFLock.Unlock());
   if (NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == cif_db))
      throw EXPTNactive_CIF();
   cif_db = NULL;
}

bool DataCenter::lockOas(Oasis::OasisInFile*& oasis_db)
{
   if (wxMUTEX_DEAD_LOCK == _OASLock.Lock())
   {
      tell_log(console::MT_ERROR,"OASIS Mutex deadlocked!");
      oasis_db = _OASDB;
      return false;
   }
   else
   {
      oasis_db = _OASDB;
      return (NULL != oasis_db);
   }
}

void DataCenter::unlockOas(Oasis::OasisInFile*& oasis_db, bool throwexception)
{
   _OASDB = oasis_db;
   VERIFY(wxMUTEX_NO_ERROR == _OASLock.Unlock());
   if (NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == oasis_db))
      throw EXPTNactive_OASIS();
   oasis_db = NULL;
}

void DataCenter::bpAddGdsTab()
{
   // Lock the Mutex
   if (wxMUTEX_DEAD_LOCK == _GDSLock.Lock())
   {
      tell_log(console::MT_ERROR,"GDS Mutex deadlocked!");
      return;
   }
   // Initialize the thread condition with the locked Mutex
   _bpSync = new wxCondition(_GDSLock);
   // post a message to the main thread
   TpdPost::addGDStab();
   // Go to sleep and wait until the main thread finished
   // updating the browser panel
   _bpSync->Wait();
   // Wake-up & unlock the mutex
   VERIFY(wxMUTEX_NO_ERROR == _GDSLock.Unlock());
   // clean-up behind & prepare for the consequent use
   delete _bpSync;
   _bpSync = NULL;
}

void DataCenter::bpAddCifTab()
{
   // Lock the Mutex
   if (wxMUTEX_DEAD_LOCK == _CIFLock.Lock())
   {
      tell_log(console::MT_ERROR,"CIF Mutex deadlocked!");
      return;
   }
   // Initialize the thread condition with the locked Mutex
   _bpSync = new wxCondition(_CIFLock);
   // post a message to the main thread
   TpdPost::addCIFtab();
   // Go to sleep and wait until the main thread finished
   // updating the browser panel
   _bpSync->Wait();
   // Wake-up & unlock the mutex
   VERIFY(wxMUTEX_NO_ERROR == _CIFLock.Unlock());
   // clean-up behind & prepare for the consequent use
   delete _bpSync;
   _bpSync = NULL;
}

void DataCenter::bpAddOasTab()
{
   // Lock the Mutex
   if (wxMUTEX_DEAD_LOCK == _OASLock.Lock())
   {
      tell_log(console::MT_ERROR,"OASIS Mutex deadlocked!");
      return;
   }
   // initialise the thread condition with the locked Mutex
   _bpSync = new wxCondition(_OASLock);
   // post a message to the main thread
   TpdPost::addOAStab();
   // Go to sleep and wait until the main thread finished
   // updating the browser panel
   _bpSync->Wait();
   // Wake-up & uplock the mutex
   VERIFY(wxMUTEX_NO_ERROR == _OASLock.Unlock());
   // clean-up behind & prepare for the consequent use
   delete _bpSync;
   _bpSync = NULL;
}

void DataCenter::mouseStart(int input_type, std::string name, const CTM trans,
                            int4b stepX, int4b stepY, word cols, word rows)
{
   if (console::op_line == input_type) return;
   if (_TEDLIB())
   {
      _TEDLIB()->check_active();
      switch (input_type)
      {
         case console::op_dbox:   _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::TdtTmpBox()  ); break;
         case console::op_dpoly:  _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::TdtTmpPoly()) ; break;
         case console::op_cbind:
         {
            assert ("" != name);
            laydata::CellDefin strdefn;
            CTM eqm;
            VERIFY(DATC->getCellNamePair(name, strdefn));
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::TdtTmpCellRef(strdefn, eqm) );
            break;
         }
         case console::op_abind:
         {
            assert ("" != name);
            assert(0 != cols);assert(0 != rows);assert(0 != stepX);assert(0 != stepY);
            laydata::CellDefin strdefn;
            CTM eqm;
            VERIFY(DATC->getCellNamePair(name, strdefn));
            laydata::ArrayProperties arrprops(stepX, stepY, cols, rows);
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::TdtTmpCellAref(strdefn, eqm, arrprops) );
            break;
         }
         case console::op_tbind:
         {
            assert ("" != name);
            CTM eqm(trans);
            eqm.Scale(1/(PROPC->UU()*OPENGL_FONT_UNIT), 1/(PROPC->UU()*OPENGL_FONT_UNIT));
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::TdtTmpText(name, eqm) );
            break;
         }
         case console::op_rotate: _TEDLIB()->set_tmpctm( trans );
         default:
         {
            if (0  < input_type)
               _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::TdtTmpWire(input_type) );
         }
      }
   }
   else throw EXPTNactive_DB();
}

void DataCenter::mousePoint(TP p)
{
   if ((console::op_line == PROPC->currentop()) || _drawruler)
      PROPC->mousePoint(p);
   if ((NULL != _TEDLIB()) && (console::op_cbind != PROPC->currentop())
                           && (console::op_abind != PROPC->currentop())
                           && (console::op_tbind != PROPC->currentop())
                           && (console::op_line  != PROPC->currentop()) )
      _TEDLIB()->mousePoint(p);
}

void DataCenter::mousePointCancel(TP& lp)
{
   if (console::op_line == PROPC->currentop()) return;
   if (_TEDLIB())
      _TEDLIB()->mousePointCancel(lp);
}

void DataCenter::mouseStop()
{
   if (console::op_line == PROPC->currentop())
      PROPC->mouseStop();
   else if (_TEDLIB()) _TEDLIB()->mouseStop();
   else throw EXPTNactive_DB();
}

void DataCenter::mouseFlip()
{
   if (_TEDLIB()) _TEDLIB()->mouseFlip();
}

void DataCenter::mouseRotate()
{
   if (_TEDLIB()) _TEDLIB()->mouseRotate();
}

void DataCenter::render(const CTM& layCTM)
{
   if (PROPC->renderType())
      openGL_render(layCTM);
   else
      openGL_draw(layCTM);
}

void DataCenter::openGL_draw(const CTM& layCTM)
{
   if (_TEDLIB())
   {
      // Don't block the drawing if the databases are
      // locked. This will block all redraw activities including UI
      // which have nothing to do with the DB. Drop a message in the log
      // and keep going!
//      if (wxMUTEX_NO_ERROR != _PROPLock.TryLock())
//      {
//         // If property DB is locked - we can't do much drawing even if the
//         // DB is not locked. In the same time there should not be an operation
//         // which holds the property DB lock for a long time. So it should be
//         // rather an exception
//         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
//         return;
//      }
      PROPC->drawGrid();
      PROPC->drawZeroCross();
      if (wxMUTEX_NO_ERROR != _DBLock.TryLock())
      {
         // If DB is locked - skip the DB drawing, but draw all the property DB stuff
         tell_log(console::MT_INFO,std::string("DB busy. Viewport redraw skipped"));
      }
      else
      {
#ifdef RENDER_PROFILING
         HiResTimer rendTimer;
#endif
         // There is no need to check for an active cell. If there isn't one
         // the function will return silently.
         _TEDLIB()->openGL_draw(PROPC->drawprop());
         if(_DRCDB)
         {
            laydata::TdtDefaultCell* dst_structure = _DRCDB->checkcell("drc");
            if (dst_structure)
            {
               dst_structure->openGL_draw(PROPC->drawprop());
            }
         }
#ifdef RENDER_PROFILING
         rendTimer.report("Total elapsed rendering time");
#endif
         VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
      }
      PROPC->drawRulers(layCTM);
//      _PROPLock.Unlock();
   }
}

void DataCenter::openGL_render(const CTM& layCTM)
{
   if (_TEDLIB())
   {
      // Don't block the drawing if the databases are
      // locked. This will block all redraw activities including UI
      // which have nothing to do with the DB. Drop a message in the log
      // and keep going!
//      if (wxMUTEX_NO_ERROR != _PROPLock.TryLock())
//      {
//         // If property DB is locked - we can't do much drawing even if the
//         // DB is not locked. In the same time there should not be an operation
//         // which holds the property DB lock for a long time. So it should be
//         // rather an exception
//         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
//         return;
//      }
      tenderer::TopRend renderer( PROPC->drawprop_ptr(), PROPC->UU());
      // render the grid
      for (byte gridNo = 0; gridNo < 3; gridNo++)
      {
         const layprop::LayoutGrid* cgrid = PROPC->grid(gridNo);
         if ((NULL !=  cgrid) && cgrid->visual())
            renderer.Grid(cgrid->step(), cgrid->color());
      }

      //       _properties.drawZeroCross(renderer);
      if (wxMUTEX_NO_ERROR != _DBLock.TryLock())
      {
         // If DB is locked - skip the DB drawing, but draw all the property DB stuff
         tell_log(console::MT_INFO,std::string("DB busy. Viewport redraw skipped"));
      }
      else
      {
#ifdef RENDER_PROFILING
         HiResTimer rendTimer;
#endif
         // Thereis no need to check for an active cell. If there isn't one
         // the function will return silently.
         _TEDLIB()->openGL_render(renderer);
         if(_DRCDB)
         {
            renderer.setState(layprop::DRC);
            laydata::TdtDefaultCell* dst_structure = _DRCDB->checkcell("drc");
            if (dst_structure)
            {
               dst_structure->openGL_render(renderer, CTM(), false, false);
            }
            renderer.setState(layprop::DB);
         }
         //_DRCDB->openGL_draw(_properties.drawprop());
#ifdef RENDER_PROFILING
         rendTimer.report("Time elapsed for data traversing: ");
#endif
         // The version with the central VBO's
         if (renderer.collect())
         {
#ifdef RENDER_PROFILING
            rendTimer.report("Time elapsed for data copying   : ");
#endif
            renderer.draw();
#ifdef RENDER_PROFILING
            rendTimer.report("    Total elapsed rendering time: ");
#endif
         }
         VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
      }
      PROPC->drawRulers(layCTM);
//      _PROPLock.Unlock();
   }
}


void DataCenter::tmp_draw(const CTM& layCTM, TP base, TP newp)
{
   if ((console::op_line == PROPC->currentop()) || _drawruler)
   {
      // ruller
      PROPC->tmp_draw(layCTM, base, newp);
   }
   if ((console::op_line != PROPC->currentop())  && (NULL !=_TEDLIB()))
   {
//      _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != _DBLock.TryLock());
      _TEDLIB()->tmp_draw(PROPC->drawprop(), base, newp);
      VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
   }
//
//   else throw EXPTNactive_DB();
}

const laydata::CellList& DataCenter::cells() {
   if (_TEDLIB()) return _TEDLIB()->cells();
   else throw EXPTNactive_DB();
};

bool DataCenter::getCellNamePair(std::string name, laydata::CellDefin& strdefn)
{
   laydata::TdtDesign* ATDB = lockDB();
   if (ATDB->checkcell(name))
   {
      strdefn = ATDB->getcellnamepair(name);
      unlockDB();
      return true;
   }
   unlockDB();
   // search the cell in the libraries because it's not in the DB
   return _TEDLIB.getLibCellRNP(name, strdefn);
}


LayerMapCif* DataCenter::secureCifLayMap(const layprop::DrawProperties* drawProp, bool import)
{
   const USMap* savedMap = PROPC->getCifLayMap();
   if (NULL != savedMap) return DEBUG_NEW LayerMapCif(*savedMap);
   USMap* theMap = DEBUG_NEW USMap();
   if (import)
   {// Generate the default CIF layer map for import
      nameList cifLayers;
      cifGetLayers(cifLayers);
      word laynum = 1;
      for ( nameList::const_iterator CCL = cifLayers.begin(); CCL != cifLayers.end(); CCL++ )
         (*theMap)[laynum] = *CCL;
   }
   else
   {// Generate the default CIF layer map for export
      lockDB(false);
      nameList tdtLayers;
      drawProp->allLayers(tdtLayers);
      for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
      {
         std::ostringstream ciflayname;
         unsigned layno = PROPC->getLayerNo( *CDL );
         ciflayname << "L" << layno;
         (*theMap)[layno] = ciflayname.str();
      }
      unlockDB();
   }
   return DEBUG_NEW LayerMapCif(*theMap);
}

LayerMapExt* DataCenter::secureGdsLayMap(const layprop::DrawProperties* drawProp, bool import)
{
   const USMap* savedMap = PROPC->getGdsLayMap();
   LayerMapExt* theGdsMap;
   if (NULL == savedMap)
   {
      USMap theMap;
      if (import)
      { // generate default import GDS layer map
         ExtLayers* gdsLayers = DEBUG_NEW ExtLayers();
         gdsGetLayers(*gdsLayers);
         for ( ExtLayers::const_iterator CGL = gdsLayers->begin(); CGL != gdsLayers->end(); CGL++ )
         {
            std::ostringstream dtypestr;
            dtypestr << CGL->first << ";";
            for ( WordSet::const_iterator CDT = CGL->second.begin(); CDT != CGL->second.end(); CDT++ )
            {
               if ( CDT != CGL->second.begin() ) dtypestr << ", ";
               dtypestr << *CDT;
            }
            theMap[CGL->first] = dtypestr.str();
         }
         theGdsMap = DEBUG_NEW LayerMapExt(theMap, gdsLayers);
      }
      else
      { // generate default export GDS layer map
         lockDB(false);
         nameList tdtLayers;
         drawProp->allLayers(tdtLayers);
         for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
         {
            std::ostringstream dtypestr;
            dtypestr << PROPC->getLayerNo( *CDL )<< "; 0";
            theMap[PROPC->getLayerNo( *CDL )] = dtypestr.str();
         }
         DATC->unlockDB();
         theGdsMap = DEBUG_NEW LayerMapExt(theMap, NULL);
      }
   }
   else
   {
      if (import)
      {
         ExtLayers* gdsLayers = DEBUG_NEW ExtLayers();
         gdsGetLayers(*gdsLayers);
         theGdsMap = DEBUG_NEW LayerMapExt(*savedMap, gdsLayers);
      }
      else
         theGdsMap = DEBUG_NEW LayerMapExt(*savedMap, NULL);
   }
   return theGdsMap;
}

laydata::LibCellLists* DataCenter::getCells(int libID)
{
   laydata::LibCellLists* all_cells = DEBUG_NEW laydata::LibCellLists();
   if (libID == ALL_LIB)
   {
      if (NULL != _TEDLIB())
         all_cells->push_back(&(_TEDLIB()->cells()));
      for (int i = 1; i < _TEDLIB.getLastLibRefNo(); i++)
         all_cells->push_back(&(_TEDLIB.getLib(i)->cells()));
   }
   else if ( (libID == TARGETDB_LIB) && (NULL != _TEDLIB()) )
      all_cells->push_back(&(_TEDLIB()->cells()));
   else if (libID == UNDEFCELL_LIB)
      all_cells->push_back(&(_TEDLIB.getUndefinedCells()));
   else if (libID < _TEDLIB.getLastLibRefNo())
      all_cells->push_back(&(_TEDLIB.getLib(libID)->cells()));
   return all_cells;
}

void DataCenter::updateVisibleOverlap()
{
   lockDB();
   _TEDLIB()->updateVisibleOverlap(PROPC->drawprop());
   unlockDB();
}

//void initDBLib(const std::string &localDir, const std::string &globalDir)
//{
//   DATC  = DEBUG_NEW DataCenter(localDir, globalDir);
//   PROPC = DEBUG_NEW layprop::PropertyCenter();
//}



//bool DataCenter::addLayer(std::string name, unsigned layno, std::string col,
//                                       std::string fill, std::string sline)
//{
//   bool status;
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   status = _properties.addLayer(name, layno, col, fill, sline);
//   _PROPLock.Unlock();
//   return status;
//}
//
//bool DataCenter::addLayer(std::string name, unsigned layno)
//{
//   bool status;
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   status = _properties.addLayer(name, layno);
//   _PROPLock.Unlock();
//   return status;
//}
//
//bool DataCenter::addLayer(unsigned layno)
//{
//   bool status;
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   status = _properties.addLayer(layno);
//   _PROPLock.Unlock();
//   return status;
//}
//
//unsigned DataCenter::addLayer(std::string name)
//{
//   unsigned layno;
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   layno = _properties.addLayer(name);
//   _PROPLock.Unlock();
//   return layno;
//}
//bool  DataCenter::isLayerExist(word layno)
//{
//   return _properties.isLayerExist(layno);
//}

//bool  DataCenter::isLayerExist(std::string layname)
//{
//   return _properties.isLayerExist(layname);
//}
//void DataCenter::addLine(std::string name, std::string col, word pattern,
//                                      byte patscale, byte width)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.addLine(name, col, pattern, patscale, width);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::addColor(std::string name, byte R, byte G, byte B, byte A)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.addColor(name, R, G, B, A);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::addFill(std::string name, byte *ptrn)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.addFill(name, ptrn);
//   _PROPLock.Unlock();
//}

//void DataCenter::hideLayer(word layno, bool hide)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.hideLayer(layno, hide);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::lockLayer(word layno, bool lock)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.lockLayer(layno, lock);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::fillLayer(word layno, bool fill)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.fillLayer(layno, fill);
//   _PROPLock.Unlock();
//}

//void DataCenter::pushLayerStatus()
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.pushLayerStatus();
//   _PROPLock.Unlock();
//}
//
//void DataCenter::popLayerStatus()
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.popLayerStatus();
//   _PROPLock.Unlock();
//}
//
//void DataCenter::popBackLayerStatus()
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.popBackLayerStatus();
//   _PROPLock.Unlock();
//}
//
//bool DataCenter::saveLaysetStatus(const std::string& sname)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   bool stat = _properties.saveLaysetStatus(sname);
//   _PROPLock.Unlock();
//   return stat;
//}
//
//bool DataCenter::saveLaysetStatus(const std::string& sname, const WordSet& hl, const WordSet& ll, const WordSet& fl, unsigned al)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   bool stat = _properties.saveLaysetStatus(sname, hl, ll, fl, al);
//   _PROPLock.Unlock();
//   return stat;
//}
//
//bool DataCenter::loadLaysetStatus(const std::string& sname)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   bool stat = _properties.loadLaysetStatus(sname);
//   _PROPLock.Unlock();
//   return stat;
//}
//
//bool DataCenter::deleteLaysetStatus(const std::string& sname)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   bool stat = _properties.deleteLaysetStatus(sname);
//   _PROPLock.Unlock();
//   return stat;
//}
//
//bool DataCenter::getLaysetStatus(const std::string& sname, WordSet& hl, WordSet& ll, WordSet& fl, unsigned al)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   bool stat = _properties.getLaysetStatus(sname, hl, ll, fl, al);
//   _PROPLock.Unlock();
//   return stat;
//}

//void DataCenter::setCellMarksHidden(bool hide)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.setCellMarksHidden(hide);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::setTextMarksHidden(bool hide)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.setTextMarksHidden(hide);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::setCellboxHidden(bool hide)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.setCellboxHidden(hide);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::setTextboxHidden(bool hide)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.setTextboxHidden(hide);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::setGrid(byte No, real step, std::string colname)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.setGrid(No, step, colname);
//   _PROPLock.Unlock();
//}
//
//bool DataCenter::viewGrid(byte No, bool status)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   status = _properties.viewGrid(No, status);
//   _PROPLock.Unlock();
//   return status;
//}
//
//void DataCenter::addRuler(TP& p1, TP& p2)
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.addRuler(p1, p2);
//   _PROPLock.Unlock();
//}
//
//void DataCenter::clearRulers()
//{
//   while (wxMUTEX_NO_ERROR != _PROPLock.TryLock());
//   _properties.clearRulers();
//   _PROPLock.Unlock();
//}
//
