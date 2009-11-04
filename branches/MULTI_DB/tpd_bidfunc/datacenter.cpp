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
DataCenter*                      DATC = NULL;
extern const wxEventType         wxEVT_CMD_BROWSER;

//-----------------------------------------------------------------------------
// class DataCenter
//-----------------------------------------------------------------------------
DataCenter::DataCenter(const std::string& localDir, const std::string& globalDir) 
{
   _localDir = localDir;
   _globalDir = globalDir;
	_GDSDB = NULL; _CIFDB = NULL;_OASISDB = NULL; _DRCDB = NULL;//_TEDDB = NULL;
   _bpSync = NULL;
   // initializing the static cell hierarchy tree
   laydata::tdtlibrary::initHierTreePtr();
   _tedfilename = "unnamed";
   _curlay = 1;
   _drawruler = false;
}

DataCenter::~DataCenter() {
   laydata::tdtlibrary::clearEntireHierTree();
   if (NULL != _GDSDB) delete _GDSDB;
   if (NULL != _CIFDB) delete _CIFDB;
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
   _TEDLIB.setDB(static_cast<laydata::tdtdesign*>(tempin.design()));
   _TEDLIB()->assign_properties(_properties);
   // Update Canvas scale
   _properties.setUU(_TEDLIB()->UU());
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
   laydata::tdtlibrary* tberased = _TEDLIB.removelibrary(libname);
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

void DataCenter::GDSexport(const LayerMapGds& layerMap, std::string& filename, bool x2048)
{
   GDSin::GdsExportFile gdsex(filename, NULL, layerMap, true);
   _TEDLIB()->GDSwrite(gdsex);
}

void DataCenter::GDSexport(laydata::tdtcell* cell, const LayerMapGds& layerMap, bool recur, std::string& filename, bool x2048)
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

void DataCenter::importGDScell(const nameList& top_names, const LayerMapGds& laymap, bool recur, bool over)
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

void DataCenter::CIFexport(laydata::tdtcell* topcell, USMap* laymap, bool recur, bool verbose, std::string& filename)
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

bool DataCenter::gdsGetLayers(GdsLayers& gdsLayers)
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

bool DataCenter::OasisParse(std::string filename)
{
   bool status = true;

   Oasis::OasisInFile* AOASISDB = NULL;
   if (lockOasis(AOASISDB))
   {
      std::string news = "Removing existing OASIS data from memory...";
      tell_log(console::MT_WARNING,news);
      delete AOASISDB;
   }
   try
   {
      AOASISDB = DEBUG_NEW Oasis::OasisInFile(filename);
      status = AOASISDB->status();
      if (status)
         AOASISDB->readLibrary();
   }
   catch (EXPTNreadOASIS)
   {
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      status = false;
   }
   if (status)
      AOASISDB->hierOut();// generate the hierarchy tree of cells
   else if (NULL != AOASISDB)
   {
      delete AOASISDB;
      AOASISDB = NULL;
   }
   unlockOasis(AOASISDB);
   return status;
}

void DataCenter::PSexport(laydata::tdtcell* cell, std::string& filename)
{
   //Get actual time
   PSFile psex(filename);
   _properties.drawprop().PSwrite(psex);
   _TEDLIB()->PSwrite(psex, cell, _properties.drawprop());
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
   _TEDLIB.setDB(DEBUG_NEW laydata::tdtdesign(name, created, created));
   _TEDLIB()->assign_properties(_properties);
   _tedfilename = _localDir + name + ".tdt";
   _neversaved = true;
   _properties.setUU(_TEDLIB()->UU());
}

laydata::tdtdesign*  DataCenter::lockDB(bool checkACTcell) 
{
   if (_TEDLIB()) 
   {
      if (checkACTcell) _TEDLIB()->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      return _TEDLIB();
   }
   else throw EXPTNactive_DB();
}

laydata::tdtlibrary*  DataCenter::lockDRC(void) 
{
	if (!_TEDLIB()) throw EXPTNactive_DB();
   if (!_DRCDB) 
   {
		_DRCDB = DEBUG_NEW laydata::tdtdesign("drc", 0, _TEDLIB()->DBU(), _TEDLIB()->UU());
   }
	while (wxMUTEX_NO_ERROR != DRCLock.TryLock());
   return _DRCDB;
}

void DataCenter::unlockDB() 
{
   VERIFY(wxMUTEX_NO_ERROR == DBLock.Unlock());
}

void DataCenter::unlockDRC() 
{
   VERIFY(wxMUTEX_NO_ERROR == DRCLock.Unlock());
}

bool DataCenter::lockGds(GDSin::GdsInFile*& gds_db)
{
   if (wxMUTEX_DEAD_LOCK == GDSLock.Lock())
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
   VERIFY(wxMUTEX_NO_ERROR == GDSLock.Unlock());
   if(NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == gds_db))
      throw EXPTNactive_GDS();
   gds_db = NULL;
}

bool DataCenter::lockCif(CIFin::CifFile*& cif_db)
{
   if (wxMUTEX_DEAD_LOCK == CIFLock.Lock())
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
   VERIFY(wxMUTEX_NO_ERROR == CIFLock.Unlock());
   if (NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == cif_db))
      throw EXPTNactive_CIF();
   cif_db = NULL;
}

bool DataCenter::lockOasis(Oasis::OasisInFile*& oasis_db)
{
   if (wxMUTEX_DEAD_LOCK == OASISLock.Lock())
   {
      tell_log(console::MT_ERROR,"OASIS Mutex deadlocked!");
      oasis_db = _OASISDB;
      return false;
   }
   else
   {
      oasis_db = _OASISDB;
      return (NULL != oasis_db);
   }
}

void DataCenter::unlockOasis(Oasis::OasisInFile*& oasis_db, bool throwexception)
{
   _OASISDB = oasis_db;
   VERIFY(wxMUTEX_NO_ERROR == OASISLock.Unlock());
   if (NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == oasis_db))
      throw EXPTNactive_OASIS();
   oasis_db = NULL;
}

void DataCenter::bpAddGdsTab()
{
   // Lock the Mutex
   if (wxMUTEX_DEAD_LOCK == GDSLock.Lock())
   {
      tell_log(console::MT_ERROR,"GDS Mutex deadlocked!");
      return;
   }
   // initialise the thread condition with the locked Mutex
   _bpSync = new wxCondition(GDSLock);
   // post a message to the main thread
   TpdPost::addGDStab();
   // Go to sleep and wait until the main thread finished
   // updating the browser panel
   _bpSync->Wait();
   // Wake-up & uplock the mutex
   VERIFY(wxMUTEX_NO_ERROR == GDSLock.Unlock());
   // clean-up behind & prepare for the consequent use
   delete _bpSync;
   _bpSync = NULL;
}

void DataCenter::bpAddCifTab()
{
   // Lock the Mutex
   if (wxMUTEX_DEAD_LOCK == CIFLock.Lock())
   {
      tell_log(console::MT_ERROR,"CIF Mutex deadlocked!");
      return;
   }
   // initialise the thread condition with the locked Mutex
   _bpSync = new wxCondition(CIFLock);
   // post a message to the main thread
   TpdPost::addCIFtab();
   // Go to sleep and wait untill the main thread finished
   // updating the browser panel
   _bpSync->Wait();
   // Wake-up & uplock the mutex
   VERIFY(wxMUTEX_NO_ERROR == CIFLock.Unlock());
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
         case console::op_dbox:   _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttmpbox()  ); break;
         case console::op_dpoly:  _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttmppoly()) ; break;
         case console::op_cbind:
         {
            assert ("" != name);
            laydata::CellDefin strdefn;
            CTM eqm;
            VERIFY(DATC->getCellNamePair(name, strdefn));
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttmpcellref(strdefn, eqm) );
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
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttmpcellaref(strdefn, eqm, arrprops) );
            break;
         }
         case console::op_tbind:
         {
            assert ("" != name);
            CTM eqm(trans);
            eqm.Scale(1/(UU()*OPENGL_FONT_UNIT), 1/(UU()*OPENGL_FONT_UNIT));
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttmptext(name, eqm) );
            break;
         }
         case console::op_rotate: _TEDLIB()->set_tmpctm( trans );
         default:
         {
            if (0  < input_type)
               _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttmpwire(input_type) );
         }
      }
   }
   else throw EXPTNactive_DB();
}

void DataCenter::mousePoint(TP p)
{
   if ((console::op_line == currentop()) || _drawruler)
      _properties.mousePoint(p);
   if ((NULL != _TEDLIB()) && (console::op_cbind != currentop())
                           && (console::op_abind != currentop())
                           && (console::op_tbind != currentop()) 
                           && (console::op_line  != currentop()) )
      _TEDLIB()->mousePoint(p);
}

void DataCenter::mousePointCancel(TP& lp)
{
   if (console::op_line == currentop()) return;
   if (_TEDLIB())
      _TEDLIB()->mousePointCancel(lp);
}

void DataCenter::mouseStop()
{
   if (console::op_line == currentop())
      _properties.mouseStop();
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
   if (_properties.renderType())
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
      if (wxMUTEX_NO_ERROR != PROPLock.TryLock())
      {
         // If property DB is locked - we can't do much drawing even if the
         // DB is not locked. In the same time there should not be an operation
         // which holds the property DB lock for a long time. So it should be
         // rather an exception
         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
         return;
      }
      _properties.drawGrid();
      _properties.drawZeroCross();
      if (wxMUTEX_NO_ERROR != DBLock.TryLock())
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
         _TEDLIB()->openGL_draw(_properties.drawprop());
			if(_DRCDB)
			{
				laydata::tdtdefaultcell* dst_structure = _DRCDB->checkcell("drc");
				if (dst_structure)
				{
					dst_structure->openGL_draw(_properties.drawprop());
				}
			}
#ifdef RENDER_PROFILING
         rendTimer.report("Total elapsed rendering time");
#endif
         VERIFY(wxMUTEX_NO_ERROR == DBLock.Unlock());
      }
      _properties.drawRulers(layCTM);
      PROPLock.Unlock();
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
      if (wxMUTEX_NO_ERROR != PROPLock.TryLock())
      {
         // If property DB is locked - we can't do much drawing even if the
         // DB is not locked. In the same time there should not be an operation
         // which holds the property DB lock for a long time. So it should be
         // rather an exception
         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
         return;
      }
      tenderer::TopRend renderer( _properties.drawprop_ptr(), _properties.UU());
      // render the grid
      for (byte gridNo = 0; gridNo < 3; gridNo++)
      {
         const layprop::LayoutGrid* cgrid = _properties.grid(gridNo);
         if ((NULL !=  cgrid) && cgrid->visual())
            renderer.Grid(cgrid->step(), cgrid->color());
      }

      //       _properties.drawZeroCross(renderer);
      if (wxMUTEX_NO_ERROR != DBLock.TryLock())
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
				laydata::tdtdefaultcell* dst_structure = _DRCDB->checkcell("drc");
				if (dst_structure)
				{
					dst_structure->openGL_render(renderer, CTM(), false, false);
				}
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
         VERIFY(wxMUTEX_NO_ERROR == DBLock.Unlock());
      }
      _properties.drawRulers(layCTM);
      PROPLock.Unlock();
   }
}


void DataCenter::tmp_draw(const CTM& layCTM, TP base, TP newp) 
{
   if ((console::op_line == currentop()) || _drawruler)
   {
      // ruller
      while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
      _properties.tmp_draw(layCTM, base, newp);
      PROPLock.Unlock();
   }
   if ((console::op_line != currentop())  && (NULL !=_TEDLIB()))
   {
//      _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      _TEDLIB()->tmp_draw(_properties.drawprop(), base, newp);
      VERIFY(wxMUTEX_NO_ERROR == DBLock.Unlock());
   }
// 
//   else throw EXPTNactive_DB();      
}

const laydata::cellList& DataCenter::cells() {
   if (_TEDLIB()) return _TEDLIB()->cells();
   else throw EXPTNactive_DB();
};


bool DataCenter::addlayer(std::string name, unsigned layno, std::string col,
                                       std::string fill, std::string sline)
{
   bool status;
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.addlayer(name, layno, col, fill, sline);
   PROPLock.Unlock();
   return status;
}

bool DataCenter::addlayer(std::string name, unsigned layno)
{
   bool status;
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.addlayer(name, layno);
   PROPLock.Unlock();
   return status;
}

bool DataCenter::addlayer(unsigned layno)
{
   bool status;
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.addlayer(layno);
   PROPLock.Unlock();
   return status;
}

unsigned DataCenter::addlayer(std::string name)
{
   unsigned layno;
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   layno = _properties.addlayer(name);
   PROPLock.Unlock();
   return layno;
}

bool  DataCenter::isLayerExist(word layno)
{
   return _properties.isLayerExist(layno);
}

bool  DataCenter::isLayerExist(std::string layname)
{
   return _properties.isLayerExist(layname);
}

void DataCenter::addline(std::string name, std::string col, word pattern,
                                      byte patscale, byte width)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addline(name, col, pattern, patscale, width);
   PROPLock.Unlock();
}

void DataCenter::addcolor(std::string name, byte R, byte G, byte B, byte A)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addcolor(name, R, G, B, A);
   PROPLock.Unlock();
}

void DataCenter::addfill(std::string name, byte *ptrn)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addfill(name, ptrn);
   PROPLock.Unlock();
}

void DataCenter::hideLayer(word layno, bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.hideLayer(layno, hide);
   PROPLock.Unlock();
}

void DataCenter::lockLayer(word layno, bool lock)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.lockLayer(layno, lock);
   PROPLock.Unlock();
}

void DataCenter::fillLayer(word layno, bool fill)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.fillLayer(layno, fill);
   PROPLock.Unlock();
}

void DataCenter::setcellmarks_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.setcellmarks_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::settextmarks_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.settextmarks_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::setcellbox_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.setcellbox_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::settextbox_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.settextbox_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::setGrid(byte No, real step, std::string colname)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.setGrid(No, step, colname);
   PROPLock.Unlock();
}

bool DataCenter::viewGrid(byte No, bool status)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.viewGrid(No, status);
   PROPLock.Unlock();
   return status;
}

void DataCenter::addRuler(TP& p1, TP& p2)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addRuler(p1, p2);
   PROPLock.Unlock();
}

void DataCenter::clearRulers()
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.clearRulers();
   PROPLock.Unlock();
}

bool DataCenter::getCellNamePair(std::string name, laydata::CellDefin& strdefn) 
{
   laydata::tdtdesign* ATDB = lockDB();
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


LayerMapCif* DataCenter::secureCifLayMap(bool import)
{
   const USMap* savedMap = _properties.getCifLayMap();
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
      all_layers(tdtLayers);
      for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
      {
         std::ostringstream ciflayname;
         unsigned layno = getLayerNo( *CDL );
         ciflayname << "L" << layno;
         (*theMap)[layno] = ciflayname.str();
      }
      unlockDB();
   }
   return DEBUG_NEW LayerMapCif(*theMap);
}

LayerMapGds* DataCenter::secureGdsLayMap(bool import)
{
   const USMap* savedMap = _properties.getGdsLayMap();
   LayerMapGds* theGdsMap;
   if (NULL == savedMap)
   {
      USMap theMap;
      if (import)
      { // generate default import GDS layer map
         GdsLayers* gdsLayers = DEBUG_NEW GdsLayers();
         gdsGetLayers(*gdsLayers);
         for ( GdsLayers::const_iterator CGL = gdsLayers->begin(); CGL != gdsLayers->end(); CGL++ )
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
         theGdsMap = DEBUG_NEW LayerMapGds(theMap, gdsLayers);
      }
      else
      { // generate default export GDS layer map
         lockDB(false);
         nameList tdtLayers;
         all_layers(tdtLayers);
         for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
         {
            std::ostringstream dtypestr;
            dtypestr << DATC->getLayerNo( *CDL )<< "; 0";
            theMap[DATC->getLayerNo( *CDL )] = dtypestr.str();
         }
         DATC->unlockDB();
         theGdsMap = DEBUG_NEW LayerMapGds(theMap, NULL);
      }
   }
   else
   {
      if (import)
      {
         GdsLayers* gdsLayers = DEBUG_NEW GdsLayers();
         gdsGetLayers(*gdsLayers);
         theGdsMap = DEBUG_NEW LayerMapGds(*savedMap, gdsLayers);
      }
      else
         theGdsMap = DEBUG_NEW LayerMapGds(*savedMap, NULL);
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

void initDBLib(const std::string &localDir, const std::string &globalDir)
{
   DATC = DEBUG_NEW DataCenter(localDir, globalDir);
}
