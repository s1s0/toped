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
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>
#include <sstream>
#include "gds_io.h"
#include "cif_io.h"
#include "oasis_io.h"
#include "datacenter.h"
#include "outbox.h"
#include "tedat.h"
#include "viewprop.h"
#include "trend.h"
#include "ps_out.h"
#include "basetrend.h"



#ifdef RENDER_PROFILING
   #define RENTIMER_REPORT(message) rendTimer.report(message)
   #define RENTIMER_SET             HiResTimer rendTimer

#else
   #define RENTIMER_REPORT(message)
   #define RENTIMER_SET
#endif

// Global variables
DataCenter*                      DATC  = NULL;
extern layprop::PropertyCenter*  PROPC;
//extern Calbr::CalbrFile*         DRCData;
extern trend::TrendCenter*       TRENDC;

//-----------------------------------------------------------------------------
// class DataCenter
//-----------------------------------------------------------------------------
DataCenter::DataCenter(const std::string& localDir, const std::string& globalDir) :
   _curcmdlay      ( ERR_LAY_DEF                            ),
   _drawruler      ( false                                  ),
   _localDir       ( localDir                               ),
   _globalDir      ( globalDir                              ),
   _TEDLIB         (                                        ),
   _DRCDB          ( NULL                                   ),
   _GDSDB          ( NULL                                   ),
   _CIFDB          ( NULL                                   ),
   _OASDB          ( NULL                                   ),
   _DBLock         (                                        ),
   _DRCLock        (                                        ),
   _GDSLock        (                                        ),
   _CIFLock        (                                        ),
   _OASLock        (                                        ),
   _bpSync         ( NULL                                   ),
   _tdtActMxState  ( dbmxs_unlocked                         ),
   _tdtReqMxState  ( dbmxs_unlocked                         ),
   _objectRecovery ( laydata::ValidRecovery::getInstance()  )

{
   laydata::TdtLibrary::initHierTreePtr();
}

DataCenter::~DataCenter()
{
   laydata::TdtLibrary::clearEntireHierTree();
   if (NULL != _GDSDB    ) delete _GDSDB;
   if (NULL != _CIFDB    ) delete _CIFDB;
   if (NULL != _OASDB    ) delete _OASDB;
   if (NULL != _DRCDB    ) delete _DRCDB;
   // _TEDLIB will be cleared automatically (not a pointer)
}

bool DataCenter::GDSparse(std::string filename)
{
   bool status = true;
   ForeignDbFile* AGDSDB = NULL;
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
      wxString fileNameWx(filename.c_str(),wxConvUTF8);
      AGDSDB = DEBUG_NEW GDSin::GdsInFile(fileNameWx);
#ifdef GDSCONVERT_PROFILING
      profTimer.report("Time elapsed for GDS parse: ");
#endif
   }
   catch (EXPTNreadGDS&)
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

void DataCenter::GDSclose()
{
   ForeignDbFile* AGDSDB = NULL;
   if (lockGds(AGDSDB))
   {
      delete AGDSDB;
      AGDSDB = NULL;
   }
   unlockGds(AGDSDB);
}

void DataCenter::CIFclose()
{
   ForeignDbFile* ACIFDB = NULL;
   if (lockCif(ACIFDB))
   {
      delete ACIFDB;
      ACIFDB = NULL;
   }
   unlockCif(ACIFDB);
}

void DataCenter::OASclose()
{
   ForeignDbFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
      delete AOASDB;
      AOASDB = NULL;
   }
   unlockOas(AOASDB);
}

bool DataCenter::CIFparse(std::string filename)
{
   bool status = true;
   ForeignDbFile* ACIFDB = NULL;
   if (lockCif(ACIFDB))
   {
      std::string news = "Removing existing CIF data from memory...";
      tell_log(console::MT_WARNING,news);
      delete ACIFDB;
   }
   try
   {
      ACIFDB = DEBUG_NEW CIFin::CifFile(wxString(filename.c_str(), wxConvUTF8));
   }
   catch (EXPTNcif_parser&)
   {
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      status = false;
   }
   if (status)
      // generate the hierarchy tree of cells
      ACIFDB->hierOut();
   else if (NULL != ACIFDB)
   {
      delete ACIFDB;
      ACIFDB = NULL;
   }
   unlockCif(ACIFDB);
   return status;
}

bool DataCenter::cifGetLayers(NameList& cifLayers)
{
   bool ret_value = false;
   ForeignDbFile* ACIFDB = NULL;
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
   ForeignDbFile* AGDSDB = NULL;
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
   ForeignDbFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
      AOASDB->collectLayers(oasLayers);
      ret_value = true;
   }
   unlockOas(AOASDB);
   return ret_value;
}

bool DataCenter::OASParse(std::string filename)
{
   bool status = true;

   ForeignDbFile* AOASDB = NULL;
   if (lockOas(AOASDB))
   {
      std::string news = "Removing existing OASIS data from memory...";
      tell_log(console::MT_WARNING,news);
      delete AOASDB;
   }
   try
   {
      AOASDB = DEBUG_NEW Oasis::OasisInFile(wxString(filename.c_str(), wxConvUTF8));
   }
   catch (EXPTNreadOASIS&)
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


bool DataCenter::DRCparse(std::string filename)
{
   bool status = true;
   Calbr::DrcLibrary* drcDB = NULL;
   if (lockDRC(drcDB))
   {
      std::string news = "Removing existing DRC data from memory...";
      tell_log(console::MT_WARNING,news);
      delete drcDB;
      drcDB = NULL;
   }

   try
   {
      Calbr::ClbrFile(wxString(filename.c_str(), wxConvUTF8), drcDB);
   }
   catch (EXPTNdrc_reader&)
   {
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      if (NULL != drcDB)
      {
         delete drcDB;
         drcDB = NULL;
      }
      status = false;
   }
   unlockDRC(drcDB);
   return status;
}

bool DataCenter::lockDRC(Calbr::DrcLibrary*& drcDb )
{
   if (wxMUTEX_DEAD_LOCK == _DRCLock.Lock())
   {
      tell_log(console::MT_ERROR,"DRC Mutex deadlocked!");
      drcDb = _DRCDB;
      return false;
   }
   else
   {
      drcDb = _DRCDB;
      return (NULL != drcDb);
   }
}

void DataCenter::unlockDRC(Calbr::DrcLibrary* drcDb, bool throwexception)
{
   _DRCDB = drcDb;
   VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
   if(NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == drcDb))
      throw EXPTNactive_DRC();
   drcDb = NULL;
}

bool DataCenter::lockTDT(laydata::TdtLibDir*& tdt_db, TdtMutexState reqLock)
{
   assert(reqLock > dbmxs_deadlock);
   _tdtReqMxState = reqLock;
   if (wxMUTEX_DEAD_LOCK == _DBLock.Lock())
   {
      tell_log(console::MT_ERROR, "DB Mutex deadlocked!");
      tdt_db = NULL;
      _tdtActMxState = dbmxs_deadlock;
   }
   else
   {
      // OK, the mutex is locked!
      tdt_db = &_TEDLIB;
      if (_TEDLIB())
         if (_TEDLIB()->checkActiveCell())
            _tdtActMxState = dbmxs_celllock;
         else
            _tdtActMxState = dbmxs_dblock;
      else
         _tdtActMxState = dbmxs_liblock;
   }
   return (_tdtReqMxState <= _tdtActMxState);
}

void DataCenter::unlockTDT(laydata::TdtLibDir* tdt_db, bool throwexception)
{
//   _TEDLIB = tdt_db;
   assert(_tdtActMxState > dbmxs_unlocked);
   VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
   tdt_db = NULL;
   //TODO! In all cases - throw exception if we've got to the deadlocked state
//   if (dbmxs_deadlock == _tdtActMxState) throw EXPTNmutex_DB();
   if (throwexception)
      switch (_tdtActMxState)
      {
         case dbmxs_liblock  :
            switch (_tdtReqMxState)
            {
               case dbmxs_dblock:
               case dbmxs_celllock :
                  _tdtActMxState = _tdtReqMxState = dbmxs_unlocked;
                  if(NULL != _bpSync) _bpSync->Signal();
                  throw EXPTNactive_DB();
               default: break;
            }
            break;
         case dbmxs_dblock   :
            switch (_tdtReqMxState)
            {
               case dbmxs_celllock :
                  _tdtActMxState = _tdtReqMxState = dbmxs_unlocked;
                  if(NULL != _bpSync) _bpSync->Signal();
                  throw EXPTNactive_cell();
               default: break;
            }
            break;
         default             : break;
      }
   _tdtActMxState = _tdtReqMxState = dbmxs_unlocked;
   if(NULL != _bpSync) _bpSync->Signal();
}

bool DataCenter::lockGds(ForeignDbFile*& gds_db)
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

void DataCenter::unlockGds(ForeignDbFile*& gds_db, bool throwexception)
{
   _GDSDB = gds_db;
   VERIFY(wxMUTEX_NO_ERROR == _GDSLock.Unlock());
   if(NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == gds_db))
      throw EXPTNactive_GDS();
   gds_db = NULL;
}

bool DataCenter::lockCif(ForeignDbFile*& cif_db)
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

void DataCenter::unlockCif(ForeignDbFile*& cif_db, bool throwexception)
{
   _CIFDB = cif_db;
   VERIFY(wxMUTEX_NO_ERROR == _CIFLock.Unlock());
   if (NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == cif_db))
      throw EXPTNactive_CIF();
   cif_db = NULL;
}

bool DataCenter::lockOas(ForeignDbFile*& oasis_db)
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

void DataCenter::unlockOas(ForeignDbFile*& oasis_db, bool throwexception)
{
   _OASDB = oasis_db;
   VERIFY(wxMUTEX_NO_ERROR == _OASLock.Unlock());
   if (NULL != _bpSync)
      _bpSync->Signal();
   else if (throwexception && (NULL == oasis_db))
      throw EXPTNactive_OASIS();
   oasis_db = NULL;
}

bool DataCenter::checkActiveCell()
{
   if (_TEDLIB() && _TEDLIB()->checkActiveCell())
      return true;
   else
      return false;
}

void DataCenter::bpRefreshTdtTab(bool targetDB, bool threadExecution)
{
   // This function MUST be called from a locked DB state. It will assert otherwise.
   assert(_tdtActMxState > dbmxs_deadlock);
   if (threadExecution)
   {
      assert(NULL == _bpSync);
      TdtMutexState saveMutexState = _tdtActMxState;
      TdtMutexState saveReqMxState = _tdtReqMxState;
      // Initialize the thread condition with the locked Mutex
      _bpSync = DEBUG_NEW wxCondition(_DBLock);
      // post a message to the main thread
      TpdPost::refreshTDTtab(targetDB, threadExecution);
      // Go to sleep and wait until the main thread finished
      // updating the browser panel
      //
      // NOTE! The function below will release the lock of the mutex associated with
      // it - i.e. in this case it will release the _GDSlock and will put the thread in
      // sleep until Signal or broadcast is called
      _bpSync->Wait();
      // When the thread is woken-up, the function above will lock the mutex again and
      // THEN will return here.
      //
      // Wake-up & unlock the mutex
      _tdtActMxState = saveMutexState;
      _tdtReqMxState = saveReqMxState;
      // clean-up behind & prepare for the consequent use
      delete _bpSync;
      _bpSync = NULL;
   }
   else
   {
      TdtMutexState saveMutexState = _tdtActMxState;
      _DBLock.Unlock();
      TpdPost::refreshTDTtab(targetDB, threadExecution);
      _DBLock.Lock();
      _tdtActMxState = saveMutexState;
   }
}

void DataCenter::bpAddGdsTab(bool threadExecution)
{
   if (threadExecution)
   {
      // Lock the Mutex
      if (wxMUTEX_DEAD_LOCK == _GDSLock.Lock())
      {
         tell_log(console::MT_ERROR,"GDS Mutex deadlocked!");
         return;
      }
      // Initialize the thread condition with the locked Mutex
      _bpSync = DEBUG_NEW wxCondition(_GDSLock);
      // post a message to the main thread
      TpdPost::addGDStab(threadExecution);
      // Go to sleep and wait until the main thread finished
      // updating the browser panel
      //
      // NOTE! The function below will release the lock of the mutex associated with
      // it - i.e. in this case it will release the _GDSlock and will put the thread in
      // sleep until Signal or broadcast is called
      _bpSync->Wait();
      // When the thread is woken-up, the function above will lock the mutex again and
      // THEN will return here.
      //
      // Wake-up & unlock the mutex
      VERIFY(wxMUTEX_NO_ERROR == _GDSLock.Unlock());
      // clean-up behind & prepare for the consequent use
      delete _bpSync;
      _bpSync = NULL;
   }
   else
   {
      TpdPost::addGDStab(threadExecution);
   }
}

void DataCenter::bpAddCifTab(bool threadExecution)
{
   if (threadExecution)
   {
      // Lock the Mutex
      if (wxMUTEX_DEAD_LOCK == _CIFLock.Lock())
      {
         tell_log(console::MT_ERROR,"CIF Mutex deadlocked!");
         return;
      }
      // Initialize the thread condition with the locked Mutex
      _bpSync = DEBUG_NEW wxCondition(_CIFLock);
      // post a message to the main thread
      TpdPost::addCIFtab(threadExecution);
      // Go to sleep and wait until the main thread finished
      // updating the browser panel
      _bpSync->Wait();
      // Wake-up & unlock the mutex
      VERIFY(wxMUTEX_NO_ERROR == _CIFLock.Unlock());
      // clean-up behind & prepare for the consequent use
      delete _bpSync;
      _bpSync = NULL;
   }
   else
   {
      TpdPost::addCIFtab(threadExecution);
   }
}

void DataCenter::bpAddOasTab(bool threadExecution )
{
   if (threadExecution)
   {
      // Lock the Mutex
      if (wxMUTEX_DEAD_LOCK == _OASLock.Lock())
      {
         tell_log(console::MT_ERROR,"OASIS Mutex deadlocked!");
         return;
      }
      // initialise the thread condition with the locked Mutex
      _bpSync = DEBUG_NEW wxCondition(_OASLock);
      // post a message to the main thread
      TpdPost::addOAStab(threadExecution);
      // Go to sleep and wait until the main thread finished
      // updating the browser panel
      _bpSync->Wait();
      // Wake-up & uplock the mutex
      VERIFY(wxMUTEX_NO_ERROR == _OASLock.Unlock());
      // clean-up behind & prepare for the consequent use
      delete _bpSync;
      _bpSync = NULL;
   }
   else
   {
      TpdPost::addOAStab(threadExecution);
   }
}

void DataCenter::bpAddDrcTab(bool threadExecution )
{
   if (threadExecution)
   {
      // Lock the Mutex
      if (wxMUTEX_DEAD_LOCK == _DRCLock.Lock())
      {
         tell_log(console::MT_ERROR,"DRC Mutex deadlocked!");
         return;
      }
      // initialise the thread condition with the locked Mutex
      _bpSync = DEBUG_NEW wxCondition(_DRCLock);
      // post a message to the main thread
      TpdPost::addDRCtab(threadExecution);
      // Go to sleep and wait until the main thread finished
      // updating the browser panel
      _bpSync->Wait();
      // Wake-up & uplock the mutex
      VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
      // clean-up behind & prepare for the consequent use
      delete _bpSync;
      _bpSync = NULL;
   }
   else
   {
      TpdPost::addOAStab(threadExecution);
   }
}

void DataCenter::mouseStart(int input_type, std::string name, const CTM trans,
                            int4b stepX, int4b stepY, word cols, word rows)
{
   if (console::op_line == input_type) return;

   laydata::TdtLibDir* dbLibDir = NULL;
   if (lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      switch (input_type)
      {
         case console::op_dbox:   tDesign->setTmpData( DEBUG_NEW laydata::TdtTmpBox()  ); break;
         case console::op_dpoly:  tDesign->setTmpData( DEBUG_NEW laydata::TdtTmpPoly()) ; break;
         case console::op_cbind:
         {
            assert ("" != name);
            laydata::CellDefin strdefn;
            CTM eqm;
            VERIFY(dbLibDir->getCellNamePair(name, strdefn));
            tDesign->setTmpData( DEBUG_NEW laydata::TdtTmpCellRef(strdefn, eqm) );
            break;
         }
         case console::op_abind:
         {
            assert ("" != name);
            assert(0 != cols);assert(0 != rows);assert(0 != stepX);assert(0 != stepY);
            laydata::CellDefin strdefn;
            CTM eqm;
            VERIFY(dbLibDir->getCellNamePair(name, strdefn));
            TP cDispl(stepX,     0);
            TP rDispl(    0, stepY);
            laydata::ArrayProps arrprops(cDispl, rDispl, cols, rows);
            tDesign->setTmpData( DEBUG_NEW laydata::TdtTmpCellAref(strdefn, eqm, arrprops) );
            break;
         }
         case console::op_tbind:
         {
            assert ("" != name);
            CTM eqm(trans);
            eqm.Scale(1/(PROPC->UU()*OPENGL_FONT_UNIT), 1/(PROPC->UU()*OPENGL_FONT_UNIT));
            tDesign->setTmpData( DEBUG_NEW laydata::TdtTmpText(name, eqm) );
            break;
         }
         case console::op_rotate: tDesign->setTmpCtm( trans ); break;
         default:
         {
            if (0  < input_type)
               tDesign->setTmpData( DEBUG_NEW laydata::TdtTmpWire(input_type) );
            break;
         }
      }
   }
   unlockTDT(dbLibDir, true);
}

void DataCenter::mousePoint(TP p)
{
   console::ACTIVE_OP currentOp = console::op_none;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      currentOp = drawProp->currentOp();
   }
   PROPC->unlockDrawProp(drawProp, false);
   if ((console::op_line == currentOp) || _drawruler)
      PROPC->mousePoint(p);
   if ((console::op_cbind != currentOp) &&
       (console::op_abind != currentOp) &&
       (console::op_tbind != currentOp) &&
       (console::op_line  != currentOp) &&
       (console::op_none  != currentOp)    )
   {
      laydata::TdtLibDir* dbLibDir = NULL;
      if (lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         tDesign->mousePoint(p);
      }
      else
      {
         // How we've got here?!?
         assert(false);
      }
      unlockTDT(dbLibDir);
   }
}

void DataCenter::mousePointCancel(TP& lp)
{
   console::ACTIVE_OP currentOp = console::op_none;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      currentOp = drawProp->currentOp();
   }
   PROPC->unlockDrawProp(drawProp, false);
   if (console::op_line == currentOp) return;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->mousePointCancel(lp);
   }
   else
   {
      // How we've got here?!?
      assert(false);
   }
   unlockTDT(dbLibDir);
}

void DataCenter::mouseStop()
{
   console::ACTIVE_OP currentOp = console::op_none;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      currentOp = drawProp->currentOp();
   }
   PROPC->unlockDrawProp(drawProp, false);
   if (console::op_line == currentOp)
      PROPC->mouseStop();
   else
   {
      laydata::TdtLibDir* dbLibDir = NULL;
      if (lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         tDesign->mouseStop();
      }
      else
      {
         // How we've got here?!?
         assert(false);
      }
      unlockTDT(dbLibDir);
   }
}

void DataCenter::mouseFlip()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->mouseFlip();
   }
   else
   {
      // How we've got here?!?
      assert(false);
   }
   unlockTDT(dbLibDir);
}

void DataCenter::mouseRotate()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->mouseRotate();
   }
   else
   {
      // How we've got here?!?
      assert(false);
   }
   unlockTDT(dbLibDir);
}

void DataCenter::setRecoverPoly(bool rcv)
{
   assert(NULL != _objectRecovery);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (lockTDT(dbLibDir, dbmxs_celllock))
   {
      _objectRecovery->setPolyRecovery(rcv);
   }
   unlockTDT(dbLibDir, true);
}

void DataCenter::setRecoverWire(bool rcv)
{
   assert(NULL != _objectRecovery);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (lockTDT(dbLibDir, dbmxs_celllock))
   {
      _objectRecovery->setWireRecovery(rcv);
   }
   unlockTDT(dbLibDir, true);
}

void DataCenter::render()
{
   if (_TEDLIB())
   {
      trend::TrendBase* cRenderer = TRENDC->makeCRenderer();
      if (NULL != cRenderer)
      {
         // grid
         const layprop::LayoutGrid* allGrids[3] = {PROPC->grid(0),PROPC->grid(1),PROPC->grid(2)};
         if (cRenderer->grdCollect(allGrids))
         {
            cRenderer->grdDraw();
            cRenderer->grdCleanUp();
         }
         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
         {
            TpdPost::render_status(true);
            RENTIMER_SET;
            // There is no need to check for an active cell. If there isn't one
            // the function will return silently.
            _TEDLIB()->openGlRender(*cRenderer);
//            // Draw DRC data (if any)
//            // TODO! clean-up the DRC stuff here - lock/unlock and more importantly
//            // DrcLibrary <-> CalibrFile i.e. _DRCDB <-> DRCData. The end of the line shall be
//            // the removal of the DRCData object
//            if(_DRCDB)
//            {
//               if (wxMUTEX_NO_ERROR == _DRCLock.TryLock())
//               {
//                  std::string cellName = DRCData->cellName();
//                  CTM cellCTM = DRCData->getCTM(cellName);
//                  _DRCDB->openGlRender(*cRenderer, cellName, cellCTM);
//                  VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
//               }
//            }
            RENTIMER_REPORT("Time elapsed for data traversing: ");
            if (cRenderer->collect())
            {
               RENTIMER_REPORT("Time elapsed for data copying   : ");
               cRenderer->draw();
               RENTIMER_REPORT("    Total elapsed rendering time: ");
            }
            cRenderer->grcCollect();
            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
            TpdPost::render_status(false);
         }
         else
         {
            // If DB is locked - skip the DB drawing, but draw all the property DB stuff
            tell_log(console::MT_INFO,std::string("DB busy. Viewport redraw skipped"));
            cRenderer->cleanUp();
         }
         // rulers & zero cross
         layprop::RulerList const rulers = PROPC->getAllRulers();
         DBlineList const zCross = PROPC->getZCross();
         if (cRenderer->rlrCollect(rulers, PROPC->stepDB(),zCross))
         {
            cRenderer->rlrDraw();
            cRenderer->rlrCleanUp();
         }
         TRENDC->releaseCRenderer();
      }
   }
}


void DataCenter::motionDraw(const CTM& layCTM, TP base, TP newp, bool rubber, const DBlineList repers)
{
   console::ACTIVE_OP currentOp;
   trend::TrendBase* mRenderer = TRENDC->makeMRenderer(currentOp);
   if (NULL != mRenderer)
   {
      layprop::RulerList rulers;
      if (rubber && ((console::op_line == currentOp) || _drawruler))
      {
         // ruller
         rulers.push_back(layprop::SDLine(base, newp, PROPC->UU()));
      }
      if (mRenderer->rlrCollect(rulers,PROPC->stepDB(), repers))
      {
         mRenderer->rlrDraw();
         mRenderer->rlrCleanUp();
      }
      if (rubber && (console::op_line != currentOp)  && (NULL !=_TEDLIB()))
      {
         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
         {
            _TEDLIB()->motionDraw(*mRenderer, base, newp);
            if (mRenderer->collect())
               mRenderer->draw();
            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
         }
      }
      mRenderer->cleanUp();
      TRENDC->destroyMRenderer();
   }
}

void DataCenter::mouseHooverDraw(TP& position)
{
   if (_TEDLIB())
   {
      LayerDefSet unselectable = PROPC->allUnselectable();
      trend::TrendBase* cRenderer = TRENDC->makeHRenderer();
      if (NULL != cRenderer)
      {
         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
         {
            _TEDLIB()->mouseHoover(position, *cRenderer, unselectable);
            if (cRenderer->collect())
            {
               cRenderer->draw();
            }
            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
         }
         cRenderer->cleanUp();
         TRENDC->destroyHRenderer();
      }
   }
}

void DataCenter::zoomDraw(const TP& base, const TP& newp)
{
   trend::TrendBase* zRenderer = TRENDC->makeZRenderer();
   if (NULL != zRenderer)
   {
      zRenderer->setRmm(CTM(base - newp,1,0,false));
      zRenderer->setLayer(ZOM_LAY_DEF, false);
      zRenderer->boxt(base);
      if (zRenderer->collect())
      {
         zRenderer->draw();
      }
      zRenderer->cleanUp();
      TRENDC->destroyZRenderer();
   }
}

void DataCenter::grcDraw()
{
   trend::TrendBase* cRenderer = TRENDC->getCRenderer();
   if (NULL != cRenderer)
   {
      cRenderer->grcDraw();
      TRENDC->releaseCRenderer();
   }
}

LayerMapCif* DataCenter::secureCifLayMap(const layprop::DrawProperties* drawProp, bool import)
{
   const ExpLayMap* savedMap = PROPC->getCifLayMap();
   if (NULL != savedMap) return DEBUG_NEW LayerMapCif(*savedMap);
   ExpLayMap theMap;
   if (import)
   {// Generate the default CIF layer map for import
      NameList cifLayers;
      cifGetLayers(cifLayers);
      LayerDef laydef(TLL_LAY_DEF);
      for ( NameList::const_iterator CCL = cifLayers.begin(); CCL != cifLayers.end(); CCL++ )
         theMap[laydef++] = *CCL;
   }
   else
   {// Generate the default CIF layer map for export
      NameList tdtLayers;
      drawProp->allLayers(tdtLayers);
      for ( NameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
      {
         std::ostringstream ciflayname;
         LayerDef laydef = drawProp->getLayerNo( *CDL );
         ciflayname << "L" << laydef;
         theMap[laydef] = ciflayname.str();
      }
   }
   return DEBUG_NEW LayerMapCif(theMap);
}

LayerMapExt* DataCenter::secureGdsLayMap(const layprop::DrawProperties* drawProp, bool import)
{
   const ExpLayMap* savedMap = PROPC->getGdsLayMap();
   LayerMapExt* theGdsMap;
   if (NULL == savedMap)
   {
      ExpLayMap theMap; // std::map<LayerDef, std::string>
      if (import)
      { // generate default import GDS layer map
         ExtLayers* gdsLayers = DEBUG_NEW ExtLayers(); //std::map<word, WordSet>
         gdsGetLayers(*gdsLayers);
         for ( ExtLayers::const_iterator CGL = gdsLayers->begin(); CGL != gdsLayers->end(); CGL++ )
         {
            std::ostringstream laynumstr;
            laynumstr << CGL->first << ";";
            for ( WordSet::const_iterator CDT = CGL->second.begin(); CDT != CGL->second.end(); CDT++ )
            {
               std::ostringstream dtypestr;
               dtypestr << laynumstr.str() << *CDT;
               theMap[LayerDef(CGL->first,*CDT)] = dtypestr.str();
            }
         }
         theGdsMap = DEBUG_NEW LayerMapExt(theMap, gdsLayers);
      }
      else
      { // generate default export GDS layer map
         NameList tdtLayers;
         drawProp->allLayers(tdtLayers);
         for ( NameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
         {
            LayerDef laydef(drawProp->getLayerNo( *CDL ));
            theMap[laydef] = laydef.toQList();
         }
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

