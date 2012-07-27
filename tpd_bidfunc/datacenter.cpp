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
#include "ps_out.h"
#include "tenderer.h"
#include "calbr_reader.h"

// Global variables
DataCenter*                      DATC  = NULL;
extern layprop::PropertyCenter*  PROPC;
extern Calbr::CalbrFile*         DRCData;

//-----------------------------------------------------------------------------
// class DataCenter
//-----------------------------------------------------------------------------
DataCenter::DataCenter(const std::string& localDir, const std::string& globalDir) :
   _curcmdlay      ( ERR_LAY, DEFAULT_LAY_DATATYPE          ),
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
   _cRenderer      ( NULL                                   ),
   _objectRecovery ( laydata::ValidRecovery::getInstance()  )

{
   laydata::TdtLibrary::initHierTreePtr();
}

DataCenter::~DataCenter()
{
   if (NULL != _cRenderer) delete _cRenderer;
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

laydata::DrcLibrary*  DataCenter::lockDRC(void)
{
   if (!_TEDLIB()) throw EXPTNactive_DB();
   if (!_DRCDB)
   {
         _DRCDB = DEBUG_NEW laydata::DrcLibrary("drc");
   }
   while (wxMUTEX_NO_ERROR != _DRCLock.TryLock());
   return _DRCDB;
}

void DataCenter::unlockDRC()
{
   VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
}

void DataCenter::deleteDRC(void)
{
   if (_DRCDB)
   {
         delete _DRCDB;
         _DRCDB = NULL;
   }
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
         case dbmxs_dblock   :
            switch (_tdtReqMxState)
            {
               case dbmxs_celllock :
                  _tdtActMxState = _tdtReqMxState = dbmxs_unlocked;
                  if(NULL != _bpSync) _bpSync->Signal();
                  throw EXPTNactive_cell();
               default: break;
            }
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

void DataCenter::render(const CTM& layCTM)
{
   if (PROPC->renderType())
      openGlRender(layCTM);
   else
      openGlDraw(layCTM);
}

void DataCenter::drawFOnly()
{
   if (PROPC->renderType())
   {
      if (NULL != _cRenderer) _cRenderer->grcDraw();
   }
}

void DataCenter::mouseHoover(TP& position)
{
   if (_TEDLIB())
   {
      LayerDefSet unselectable = PROPC->allUnselectable();
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
         {
            _TEDLIB()->mouseHoover(position, *drawProp, unselectable);
            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
         }
      }
      PROPC->unlockDrawProp(drawProp, false);
   }
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


void DataCenter::openGlDraw(const CTM& layCTM)
{
   if (_TEDLIB())
   {
      // Don't block the drawing if the databases are
      // locked. This will block all redraw activities including UI
      // which have nothing to do with the DB. Drop a message in the log
      // and keep going!
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         PROPC->drawGrid(drawProp);
         PROPC->drawZeroCross(drawProp);
         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
         {
//            TpdPost::toped_status(console::TSTS_RENDERON);
            TpdPost::render_status(true);
            #ifdef RENDER_PROFILING
               HiResTimer rendTimer;
            #endif
            // There is no need to check for an active cell. If there isn't one
            // the function will return silently.
            _TEDLIB()->openGlDraw(*drawProp);
            // Draw DRC data (if any)
            // TODO! clean-up the DRC stuff here - see the comment below in the
            // openGlRender method
            if(_DRCDB)
            {
               if (wxMUTEX_NO_ERROR == _DRCLock.TryLock())
               {
                  std::string cell = DRCData->cellName();
                  _DRCDB->openGlDraw(*drawProp, cell);
                  VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
               }
            }
            #ifdef RENDER_PROFILING
               rendTimer.report("Total elapsed rendering time");
            #endif
            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
//            TpdPost::toped_status(console::TSTS_RENDEROFF);
            TpdPost::render_status(false);
         }
         else
         {
            // If DB is locked - skip the DB drawing, but draw all the property DB stuff
            tell_log(console::MT_INFO,std::string("DB busy. Viewport redraw skipped"));
         }
         PROPC->drawRulers(layCTM);
      }
      else
      {
         // If property DB is locked - we can't do much drawing even if the
         // DB is not locked. In the same time there should not be an operation
         // which holds the property DB lock for a long time. So it should be
         // rather an exception
         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
      }
      PROPC->unlockDrawProp(drawProp, false);
   }
}

void DataCenter::openGlRender(const CTM& layCTM)
{
   if (_TEDLIB())
   {
      // Don't block the drawing if the databases are
      // locked. This will block all redraw activities including UI
      // which have nothing to do with the DB. Drop a message in the log
      // and keep going!
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         if (NULL != _cRenderer)
         {
//            _cRenderer->cleanUp();
            _cRenderer->grcCleanUp();
            delete _cRenderer;
         }
         _cRenderer = new tenderer::TopRend( drawProp, PROPC->UU() );
         // render the grid
         for (byte gridNo = 0; gridNo < 3; gridNo++)
         {
            const layprop::LayoutGrid* cgrid = PROPC->grid(gridNo);
            if ((NULL !=  cgrid) && cgrid->visual())
               _cRenderer->Grid(cgrid->step(), cgrid->color());
         }
         //_properties.drawZeroCross(renderer);
         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
         {
//            TpdPost::toped_status(console::TSTS_RENDERON);
            TpdPost::render_status(true);
            #ifdef RENDER_PROFILING
            HiResTimer rendTimer;
            #endif
            // There is no need to check for an active cell. If there isn't one
            // the function will return silently.
            _TEDLIB()->openGlRender(*_cRenderer);
            // Draw DRC data (if any)
            // TODO! clean-up the DRC stuff here - lock/unlock and more importantly
            // DrcLibrary <-> CalibrFile i.e. _DRCDB <-> DRCData. The end of the line shall be
            // the removal of the DRCData object
            if(_DRCDB)
            {
               if (wxMUTEX_NO_ERROR == _DRCLock.TryLock())
               {
                  std::string cellName = DRCData->cellName();
                  CTM cellCTM = DRCData->getCTM(cellName);
                  _DRCDB->openGlRender(*_cRenderer, cellName, cellCTM);
                  VERIFY(wxMUTEX_NO_ERROR == _DRCLock.Unlock());
               }
            }
            #ifdef RENDER_PROFILING
            rendTimer.report("Time elapsed for data traversing: ");
            #endif
            // The version with the central VBO's
            if (_cRenderer->collect())
            {
               #ifdef RENDER_PROFILING
                  rendTimer.report("Time elapsed for data copying   : ");
               #endif
               _cRenderer->draw();
               #ifdef RENDER_PROFILING
                  rendTimer.report("    Total elapsed rendering time: ");
               #endif
               _cRenderer->cleanUp();
            }
            if (_cRenderer->grcCollect())
            {
//               _cRenderer->grcDraw();
//               _cRenderer->grcCleanUp();
            }
            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
//            TpdPost::toped_status(console::TSTS_RENDEROFF);
            TpdPost::render_status(false);
         }
         else
         {
            // If DB is locked - skip the DB drawing, but draw all the property DB stuff
            tell_log(console::MT_INFO,std::string("DB busy. Viewport redraw skipped"));
         }
         PROPC->drawRulers(layCTM);
      }
      else
      {
         // If property DB is locked - we can't do much drawing even if the
         // DB is not locked. In the same time there should not be an operation
         // which holds the property DB lock for a long time. So it should be
         // rather an exception
         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
         return;
      }
      PROPC->unlockDrawProp(drawProp, false);
   }
}


//tenderer::TopRend* DataCenter::openGlRenderIt(const CTM& layCTM)
//{
//   tenderer::TopRend* renderer = NULL;
//   if (_TEDLIB())
//   {
//      // Don't block the drawing if the databases are
//      // locked. This will block all redraw activities including UI
//      // which have nothing to do with the DB. Drop a message in the log
//      // and keep going!
//      layprop::DrawProperties* drawProp;
//      if (PROPC->lockDrawProp(drawProp))
//      {
//         renderer = DEBUG_NEW tenderer::TopRend( drawProp, PROPC->UU() );
//         // render the grid
//         for (byte gridNo = 0; gridNo < 3; gridNo++)
//         {
//            const layprop::LayoutGrid* cgrid = PROPC->grid(gridNo);
//            if ((NULL !=  cgrid) && cgrid->visual())
//               renderer->Grid(cgrid->step(), cgrid->color());
//         }
//         //       _properties.drawZeroCross(renderer);
//         if (wxMUTEX_NO_ERROR == _DBLock.TryLock())
//         {
//            TpdPost::render_status(true);
//            // There is no need to check for an active cell. If there isn't one
//            // the function will return silently.
//            _TEDLIB()->openGlRender(*renderer);
//            // Draw DRC data (if any)
//            //_DRCDB->openGlDraw(_properties.drawprop());
//            //TODO the block below should get into the line above
//
//            renderer->collect();
//            VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
//            TpdPost::render_status(false);
//         }
//         else
//         {
//            // If DB is locked - skip the DB drawing, but draw all the property DB stuff
//            tell_log(console::MT_INFO,std::string("DB busy. Viewport redraw skipped"));
//         }
//      }
//      else
//      {
//         // If property DB is locked - we can't do much drawing even if the
//         // DB is not locked. In the same time there should not be an operation
//         // which holds the property DB lock for a long time. So it should be
//         // rather an exception
//         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
//      }
//      PROPC->unlockDrawProp(drawProp);
//   }
//   return renderer;
//}

void DataCenter::motionDraw(const CTM& layCTM, TP base, TP newp)
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      console::ACTIVE_OP currentOp = drawProp->currentOp();
      if ((console::op_line == currentOp) || _drawruler)
      {
         // ruller
         PROPC->tmp_draw(layCTM, base, newp);
      }
      if ((console::op_line != currentOp)  && (NULL !=_TEDLIB()))
      {
         while (wxMUTEX_NO_ERROR != _DBLock.TryLock());
         _TEDLIB()->tmpDraw(*drawProp, base, newp);
         VERIFY(wxMUTEX_NO_ERROR == _DBLock.Unlock());
      }
   }
   PROPC->unlockDrawProp(drawProp, false);
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
      ExpLayMap theMap;
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
         NameList tdtLayers;
         drawProp->allLayers(tdtLayers);
         for ( NameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
         {
            std::ostringstream dtypestr;
            dtypestr << drawProp->getLayerNo( *CDL );
            theMap[drawProp->getLayerNo( *CDL )] = dtypestr.str();
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

