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

typedef enum {
   //                        mutex     Lib   DB  cell
   dbmxs_unlocked  = -1,
   dbmxs_deadlock  =  0, //    0        ?    ?    ?
   dbmxs_liblock   =  1, //    1        1    0    0
   dbmxs_dblock    =  2, //    1        1    1    0
   dbmxs_celllock  =  3  //    1        1    1    1
} TdtMutexState;

class DataCenter {
public:
                              DataCenter(const std::string&, const std::string &);
                             ~DataCenter();
   bool                       GDSparse(std::string);
   void                       GDSclose();
   void                       CIFclose();
   void                       OASclose();
   CIFin::CifStatusType       CIFparse(std::string filename);
   bool                       cifGetLayers(nameList&);
   bool                       gdsGetLayers(ExtLayers&);
   bool                       oasGetLayers(ExtLayers&);
   bool                       OASParse(std::string);
   bool                       lockTDT(laydata::TdtLibDir*&, TdtMutexState);
   laydata::DrcLibrary*       lockDRC(void);
   bool                       lockGds(GDSin::GdsInFile*&);
   bool                       lockCif(CIFin::CifFile*&);
   bool                       lockOas(Oasis::OasisInFile*&);
   void                       unlockTDT(laydata::TdtLibDir*, bool throwexception = false);
   void                       unlockDRC();
   void                       unlockGds(GDSin::GdsInFile*&, bool throwexception = false);
   void                       unlockCif(CIFin::CifFile*&, bool throwexception = false);
   void                       unlockOas(Oasis::OasisInFile*& oasis_db, bool throwexception = false);
   void                       bpRefreshTdtTab(bool, bool);
   void                       bpAddGdsTab(bool);
   void                       bpAddCifTab(bool);
   void                       bpAddOasTab(bool);
   void                       mouseStart(int input_type, std::string, const CTM, int4b, int4b, word, word);
   void                       mousePointCancel(TP&);
   void                       mousePoint(TP p);
   void                       mouseStop();
   void                       mouseFlip();
   void                       mouseRotate();
   void                       motionDraw(const CTM&, TP, TP);
   void                       render(const CTM&);
   void                       setCmdLayer(word layno) {_curcmdlay = layno;}
   word                       curCmdLay() const       {return _curcmdlay;}
   bool                       modified() const        {return _TEDLIB.modified();};

   //------------------------------------------------------------------------------------------------
   void                       switchDrawRuler(bool st) {_drawruler = st;}
   bool                       drawRuler() {return _drawruler;}
   LayerMapExt*               secureGdsLayMap(const layprop::DrawProperties*, bool);
   LayerMapCif*               secureCifLayMap(const layprop::DrawProperties*, bool);
   std::string                globalDir(void) const     {return _globalDir;}
   TdtMutexState              tdtMxState() const {return _tdtActMxState;}
   std::string                localDir() const {return _localDir;}

protected:
   void                       openGlDraw(const CTM&);
   void                       openGlRender(const CTM&); // alternative to openGlDraw
private:
   bool                       unZip2Temp(std::string&, const std::string); //! unzip the input file in a temporary file
   bool                       unZlib2Temp(std::string&, const std::string);
   word                       _curcmdlay;    //! layer used during current drawing operation
   bool                       _drawruler;    //! draw a ruler while composing a shape interactively
   std::string                _localDir;
   std::string                _globalDir;
   laydata::TdtLibDir         _TEDLIB;       //! catalog of available TDT libraries
   laydata::DrcLibrary*       _DRCDB;        //! DRC data
   GDSin::GdsInFile*          _GDSDB;        //! GDS parsed data
   CIFin::CifFile*            _CIFDB;        //! CIF parsed data
   Oasis::OasisInFile*        _OASDB;        //! OASIS parsed data
   wxMutex                    _DBLock;       //! Main stream DB Mutex
   wxMutex                    _DRCLock;      //! DRC         DB Mutex
   wxMutex                    _GDSLock;      //! GDSII       DB Mutex
   wxMutex                    _CIFLock;      //! CIF         DB Mutex
   wxMutex                    _OASLock;      //! OASIS       DB Mutex
   wxCondition*               _bpSync;       //! Synchronization for cell browser panels
   TdtMutexState              _tdtActMxState; //! The actual (current) mutex state of the main DB
   TdtMutexState              _tdtReqMxState; //! The required mutex state of the main DB
};

//=============================================================================
//             External DB mutexes (CIF/GDS etc.) explained
//                                  or
//          How to keep the main thread alive during conversions
//=============================================================================
//
// This memo relates to the following fields of the DataCenter class:
//   GDSin::GdsInFile*    _GDSDB
//   CIFin::CifFile*      _CIFDB
//   Oasis::OasisInFile*  _OASDB
//   wxMutex              _GDSLock
//   wxMutex              _CIFLock
//   wxMutex              _OASLock
//   wxCondition*         _bpSync
// and associated methods:
//   bool lockGds(GDSin::GdsInFile*& gds_db);
//   bool lockCif(CIFin::CifFile*& cif_db);
//   bool lockOas(Oasis::OasisInFile*&);
//   void unlockGds(GDSin::GdsInFile*& gds_db, bool throwexception = false);
//   void unlockCif(CIFin::CifFile*& cif_db, bool throwexception = false);
//   void unlockOas(Oasis::OasisInFile*& oasis_db, bool throwexception = false);
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
// For more info - see:
// http://docs.wxwidgets.org/stable/wx_wxmutex.html#wxmutex
// http://docs.wxwidgets.org/stable/wx_wxcondition.html#wxcondition
// http://docs.wxwidgets.org/stable/wx_wxthread.html#wxthread
//=============================================================================

//=============================================================================
// How to synchronize between the cell browsers and corresponding DB in memory
//                                  or
//                 Cell browsers messages/events explained
//=============================================================================
//
// This memo relates to the following fields of the DataCenter class:
//   wxMutex              _DBLock
//   wxMutex              _GDSLock
//   wxMutex              _CIFLock
//   wxMutex              _OASLock
//   wxCondition*         _bpSync;
// and associated methods:
//   bool lockTDT(laydata::TdtLibDir*&, TdtMutexState);
//   void unlockTDT(laydata::TdtLibDir*, bool throwexception = false);
//   void bpRefreshTdtTab(bool, bool);
//   void bpAddGdsTab(bool);
//   void bpAddCifTab(bool);
//   void bpAddOasTab(bool);
//
//-----------------------------------------------------------------------------
//   The cell browser panel is updated due to a message posted from the parser
//   thread to the main thread which contains the user event wxEVT_CMD_BROWSER.
//   There are two types of events:
//   - self contained events - they carry all information needed to update the
//     cell browser in the event message - for example BT_CELL_ADD
//   - refresh events (BT_ADDTDT_LIB) - they imply that the the cell browser
//     must obtain the new DB hierarchy by itself. This can happen only if the
//     entire DB hierarchy three is parsed from the main thread. This kind of
//     event occur usually after DB read or DB import.
//   The processing of the self contained events is trivial and TpdPost class
//   defines a number of convenience static methods which are widely accessible.
//   The troubles come with refresh events. Here is why.
//
//   - to ensure thread safe message exchange, wxPostMessage must be used for
//     all messages. This mechanism doesn't define the time when the event will
//     be processed. In the multithread environment this means that the cell
//     browser will traverse the DB at any moment after the message was sent
//     which in turn means that meanwhile the DB could have been changed or
//     updated. Here is a simple example:
//       (1)    newdesign("ddd"); // generates refresh event BT_ADDTDT_LIB
//       (2)    newcell("cell1"); // generates self contained event BT_CELL_ADD
//       (3)    opencell("cell1");  //
//     By the time when BT_ADDTDT_LIB event is processed in the main thread,
//     the parser thread could have had the DB updated with the "cell1". In
//     result the "cell1" will appear twice in the cell browser due to the
//     BT_CELL_ADD event generated by the second line in the example code, and
//     this is in the best case.
//     The example above is by far not the worst case. Another case could be:
//       (1)    newdesign("ddd"); // generates refresh event BT_ADDTDT_LIB
//       (2)    newcell("cell1"); // generates self contained event BT_CELL_ADD
//       (3)    newdesign("fff"); // generates refresh event BT_ADDTDT_LIB
//     Here "cell1" might appear as a cell in the new design "fff". Again it's
//     a matter of time until the cell browser crashes.
//     Very similar (although less obvious) is the situation with the external
//     interfaces. Here is the example:
//       (1) gdsread(...);   // generates refresh event
//       (2) gdsimport(...); // generates refresh event
//       (3) gdsclose();     // generates self contained event
//     The GDS cell browser will easily crash
//
//     Obviously we need some kind of thread synchronization for refresh events.
//     This is implemented in the bp*() methods. They put the parser thread in
//     sleep until the corresponding _*Lock mutex is not locked/unlocked by the
//     main thread, and the latter will happen when the cell browser is
//     processing the BT_ADDTDT_LIB/BT_ADD*_TAB (see the comments inside the
//     methods for more info)
//
//   - This is not the end of it however. Another trouble comes when Toped
//     executes in a single thread (recovery). In all cases PostEvent puts all
//     the events in an event queue and they are executed by the wx event queue
//     "later". PostEvent does not block the execution. It copies the event in
//     the event queue and returns immediately.
//     When we have a mix between self contained and refresh events this "later"
//     generates the same problem as before even though the messages will be
//     processed in strict succession. In this case, it's not possible to
//     resolve the issue using threads synchronization as above, because we do
//     control only a single thread. It appears that PostEvent in this case
//     can not be used. When a refresh event occurs in this situation, the
//     event queue must be flushed and then the new event must be processed
//     immediately using ProcessEvent. Here is the example code
//         ::wxSafeYield(_topBrowsers);
//         _topBrowsers->GetEventHandler()->ProcessEvent(eventADDTAB);
//     Note! - this is safe only in a single thread context.
//     The code above guarantees that the events will be processed immediately.
//     It blocks the execution.
//
//     To address all the above:
//     1. For refresh events in the TDT or external DB - always use the
//        corresponding bp*Tab(...) method. Never use directly TpdPost::refreshTDTtab(...)
//     2. For self contained events - the corresponding TpdPost methods
//        shall be used
//     3. bpRefreshTdtTab(...) must be called with _DBLock mutex locked
//     4. All other methods - bpAdd*Tab(...) - which serve the external DBs
//        shall be called with their corresponding mutexes unlocked.
//

#endif
