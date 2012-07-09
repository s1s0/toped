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
//        Created: Wed Apr 25 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED functions that add a new shape
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tpdf_add.h"
#include "tuidefs.h"
#include "datacenter.h"
#include "viewprop.h"



extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdADDBOX::stdADDBOX(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor, parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtWnd()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdADDBOX::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* bx = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete bx;
}

void tellstdfunc::stdADDBOX::undo()
{
   TEUNDO_DEBUG("addbox(box, int) UNDO");
   telldata::TtLayout* bx = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack,true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(bx->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdADDBOX::execute()
{
   word la = getWordValue();
   secureLayer(tell2DBLayer(la));
   telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* bx = DEBUG_NEW telldata::TtLayout(tDesign->putBox(tell2DBLayer(la), p1DB, p2DB),tell2DBLayer(la));
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
      OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
      LogFile << LogFile.getFN() << "("<< *w << "," << la << ");";LogFile.flush();
   }
   delete (p1DB);
   delete (p2DB);
   delete w;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOX_D::stdADDBOX_D(telldata::typeID retype, bool eor) :
      stdADDBOX(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtWnd()));
}

int tellstdfunc::stdADDBOX_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdADDBOX::execute();
}

//=============================================================================
tellstdfunc::stdDRAWBOX::stdDRAWBOX(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdDRAWBOX::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* bx = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete bx;
}

void tellstdfunc::stdDRAWBOX::undo()
{
   TEUNDO_DEBUG("drawbox(int) UNDO");
   telldata::TtLayout* bx = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(bx->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdDRAWBOX::execute()
{
   LayerDef laydef(tell2DBLayer(getWordValue()), DEFAULT_LAY_DATATYPE);
   DATC->setCmdLayer(laydef);
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   // get the (final) target layer
   laydef = secureLayer();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* bx = DEBUG_NEW telldata::TtLayout(tDesign->addBox(laydef, p1DB, p2DB), laydef);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(laydef.num()));
      OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
      LogFile << "addbox("<< *w << "," << telldata::TtLayer(laydef) << ");";LogFile.flush();
   }
   delete p1DB;
   delete p2DB;
   delete w;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWBOX_D::stdDRAWBOX_D(telldata::typeID retype, bool eor) :
      stdDRAWBOX(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdDRAWBOX_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdDRAWBOX::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXr::stdADDBOXr(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor, parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdADDBOXr::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* bx = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete bx;
}

void tellstdfunc::stdADDBOXr::undo()
{
   TEUNDO_DEBUG("addbox(point, real, real, int) UNDO");
   telldata::TtLayout* bx = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(bx->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdADDBOXr::execute()
{
   word     la = getWordValue();
   secureLayer(tell2DBLayer(la));
   real heigth = getOpValue();
   real width  = getOpValue();
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt  p2 = telldata::TtPnt(p1->x()+width,p1->y()+heigth);
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(p2.x() , p2.y() , DBscale);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* bx = DEBUG_NEW telldata::TtLayout(tDesign->putBox(tell2DBLayer(la), p1DB, p2DB), tell2DBLayer(la));
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
      OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
      LogFile << LogFile.getFN() << "("<< *p1 << "," << width << "," << heigth <<
                                                 "," << la << ");"; LogFile.flush();
   }
   delete (p1DB);
   delete (p2DB);
   delete p1;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOXr_D::stdADDBOXr_D(telldata::typeID retype, bool eor) :
      stdADDBOXr(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor, parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdADDBOXr_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdADDBOXr::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXp::stdADDBOXp(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor, parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdADDBOXp::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* bx = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete bx;
}

void tellstdfunc::stdADDBOXp::undo()
{
   TEUNDO_DEBUG("addbox(point, point, int) UNDO");
   telldata::TtLayout* bx = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(bx->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdADDBOXp::execute()
{
   word     la = getWordValue();
   secureLayer(tell2DBLayer(la));
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt *p2 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(p2->x(), p2->y(), DBscale);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* bx = DEBUG_NEW telldata::TtLayout(tDesign->putBox(tell2DBLayer(la), p1DB, p2DB), tell2DBLayer(la));
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
      OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
      LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << "," << la << ");";
      LogFile.flush();
   }
   delete p1; delete p2;
   delete (p1DB);
   delete (p2DB);
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOXp_D::stdADDBOXp_D(telldata::typeID retype, bool eor) :
      stdADDBOXp(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

int tellstdfunc::stdADDBOXp_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdADDBOXp::execute();
}

//=============================================================================
tellstdfunc::stdADDPOLY::stdADDPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_pnt)));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdADDPOLY::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* ply = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete ply;
}

void tellstdfunc::stdADDPOLY::undo()
{
   TEUNDO_DEBUG("addpoly(point list, int) UNDO");
   telldata::TtLayout* ply = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(ply->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (ply);
   RefreshGL();
}

int tellstdfunc::stdADDPOLY::execute()
{
   word     la = getWordValue();
   telldata::TtList *pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   if (pl->size() >= 3)
   {
      secureLayer(tell2DBLayer(la));
      real DBscale = PROPC->DBscale();
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         PointVector* plst = t2tpoints(pl,DBscale);
         telldata::TtLayout* ply = DEBUG_NEW telldata::TtLayout(tDesign->putPoly(tell2DBLayer(la),plst), tell2DBLayer(la));
         delete plst;
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
         OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
         LogFile << LogFile.getFN() << "("<< *pl << "," << la << ");";
         LogFile.flush();
      }
      DATC->unlockTDT(dbLibDir, true);
   }
   else
   {
      tell_log(console::MT_ERROR,"At least 3 points expected to create a polygon");
      OPstack.push(DEBUG_NEW telldata::TtLayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDPOLY_D::stdADDPOLY_D(telldata::typeID retype, bool eor) :
      stdADDPOLY(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_pnt)));
}

int tellstdfunc::stdADDPOLY_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdADDPOLY::execute();
}

//=============================================================================
tellstdfunc::stdDRAWPOLY::stdDRAWPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdDRAWPOLY::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* ply = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete ply;
}

void tellstdfunc::stdDRAWPOLY::undo()
{
   TEUNDO_DEBUG("drawpoly(int) UNDO");
   telldata::TtLayout* ply = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(ply->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (ply);
   RefreshGL();
}

int tellstdfunc::stdDRAWPOLY::execute()
{
   LayerDef laydef(tell2DBLayer(getWordValue()), DEFAULT_LAY_DATATYPE);
   DATC->setCmdLayer(laydef);
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dpoly, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::TtList *pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   // get the (final) target layer
   laydef = secureLayer();
   if (pl->size() >= 3)
   {
      real DBscale = PROPC->DBscale();
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         PointVector* plst = t2tpoints(pl,DBscale);
         telldata::TtLayout* ply = DEBUG_NEW telldata::TtLayout(tDesign->addPoly(laydef,plst), laydef);
         delete plst;
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(laydef.num()));
         OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
         LogFile << "addpoly("<< *pl << "," << telldata::TtLayer(laydef) << ");"; LogFile.flush();
      }
      DATC->unlockTDT(dbLibDir, true);
   }
   else
   {
      tell_log(console::MT_ERROR,"At least 3 points expected to create a polygon");
      OPstack.push(DEBUG_NEW telldata::TtLayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWPOLY_D::stdDRAWPOLY_D(telldata::typeID retype, bool eor) :
      stdDRAWPOLY(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdDRAWPOLY_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdDRAWPOLY::execute();
}

//=============================================================================
tellstdfunc::stdADDWIRE::stdADDWIRE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_pnt)));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdADDWIRE::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* wr = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete wr;
}

void tellstdfunc::stdADDWIRE::undo()
{
   TEUNDO_DEBUG("addwire(point list, real, int) UNDO");
   telldata::TtLayout* wr = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(wr->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (wr);
   RefreshGL();
}

int tellstdfunc::stdADDWIRE::execute()
{
   word     la = getWordValue();
   real      w = getOpValue();
   telldata::TtList *pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   if (pl->size() > 1) {
      secureLayer(tell2DBLayer(la));
      real DBscale = PROPC->DBscale();
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         PointVector* plst = t2tpoints(pl,DBscale);
         telldata::TtLayout* wr = DEBUG_NEW telldata::TtLayout(tDesign->putWire(tell2DBLayer(la),plst,
                                    static_cast<WireWidth>(rint(w * DBscale))), tell2DBLayer(la));
         delete plst;
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
         OPstack.push(wr);UNDOPstack.push_front(wr->selfcopy());
         LogFile << LogFile.getFN() << "("<< *pl << "," << w << "," << la << ");";
         LogFile.flush();
      }
      DATC->unlockTDT(dbLibDir, true);
   }
   else
   {
      tell_log(console::MT_ERROR,"At least 2 points expected to create a wire");
      OPstack.push(DEBUG_NEW telldata::TtLayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDWIRE_D::stdADDWIRE_D(telldata::typeID retype, bool eor) :
      stdADDWIRE(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_pnt)));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdADDWIRE_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdADDWIRE::execute();
}

//=============================================================================
tellstdfunc::stdDRAWWIRE::stdDRAWWIRE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdDRAWWIRE::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* wr = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete wr;
}

void tellstdfunc::stdDRAWWIRE::undo()
{
   TEUNDO_DEBUG("drawwire(real, int) UNDO");
   telldata::TtLayout* wr = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(wr->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (wr);
   RefreshGL();
}

int tellstdfunc::stdDRAWWIRE::execute()
{
   LayerDef laydef(tell2DBLayer(getWordValue()), DEFAULT_LAY_DATATYPE);
   real      w = getOpValue();
   DATC->setCmdLayer(laydef);
   real DBscale = PROPC->DBscale();
   if (!tellstdfunc::waitGUInput(static_cast<int>(rint(w * DBscale)), &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::TtList *pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   // get the (final) target layer
   laydef = secureLayer();
   if (pl->size() > 1)
   {
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         PointVector* plst = t2tpoints(pl,DBscale);
         telldata::TtLayout* wr = DEBUG_NEW telldata::TtLayout(tDesign->addWire(laydef, plst,
                                    static_cast<WireWidth>(rint(w * DBscale))), laydef);
         delete plst;
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(laydef.num()));
         OPstack.push(wr);UNDOPstack.push_front(wr->selfcopy());
         LogFile << "addwire(" << *pl << "," << w << "," << telldata::TtLayer(laydef) << ");";
         LogFile.flush();
      }
      DATC->unlockTDT(dbLibDir, true);
   }
   else {
      tell_log(console::MT_ERROR,"At least 2 points expected to create a wire");
      OPstack.push(DEBUG_NEW telldata::TtLayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWWIRE_D::stdDRAWWIRE_D(telldata::typeID retype, bool eor) :
      stdDRAWWIRE(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdDRAWWIRE_D::execute()
{
   OPstack.push(getCurrentLayer());
   return stdDRAWWIRE::execute();
}

//=============================================================================
tellstdfunc::stdADDTEXT::stdADDTEXT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

void tellstdfunc::stdADDTEXT::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* tx = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete tx;
}

void tellstdfunc::stdADDTEXT::undo()
{
   TEUNDO_DEBUG("addtext(string, int, point, real, bool, real) UNDO");
   telldata::TtLayout* tx = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(tx->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (tx);
   RefreshGL();
}

int tellstdfunc::stdADDTEXT::execute()
{
   // get the parameters from the operand stack
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::TtPnt  *rpnt  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   word      la  = getWordValue();
   std::string text = getStringValue();
   if ("" == text)
   {
      tell_log(console::MT_ERROR,"Empty string. Operation ignored");
      return EXEC_ABORT;
   }
   if (0.0 == magn)
   {
      tell_log(console::MT_ERROR,"Text with size 0. Operation ignored");
      return EXEC_ABORT;
   }
   secureLayer(tell2DBLayer(la));
   real DBscale = PROPC->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale),
                                     magn*DBscale/OPENGL_FONT_UNIT,angle,flip);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* tx = DEBUG_NEW telldata::TtLayout(tDesign->putText(tell2DBLayer(la), text, ori), tell2DBLayer(la));
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
      OPstack.push(tx);UNDOPstack.push_front(tx->selfcopy());
      LogFile << LogFile.getFN() << "(\"" << text << "\"," << la << "," << *rpnt <<
            "," << angle << "," << LogFile._2bool(flip) << "," << magn << ");";
      LogFile.flush();
   }
   delete rpnt;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWTEXT::stdDRAWTEXT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

void tellstdfunc::stdDRAWTEXT::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::TtLayout* tx = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete tx;
}

void tellstdfunc::stdDRAWTEXT::undo()
{
   TEUNDO_DEBUG("addtext(string, int, point, real, bool, real) UNDO");
   telldata::TtLayout* tx = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   word la = getWordValue(UNDOPstack, true);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(tx->data(),tell2DBLayer(la), dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (tx);
   RefreshGL();
}

int tellstdfunc::stdDRAWTEXT::execute()
{
   real   omagn     = getOpValue();
   std::string text = getStringValue();
   CTM ftrans;
   ftrans.Scale(omagn,omagn);
   if ("" == text)
   {
      tell_log(console::MT_ERROR,"Empty string. Operation ignored");
      return EXEC_ABORT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_tbind, &OPstack, text, ftrans)) return EXEC_ABORT;
   // get the data from the stack
   telldata::TtBnd *bnd = static_cast<telldata::TtBnd*>(OPstack.top());OPstack.pop();
   //--------------------------------------------------------------------------
   real              magn   = bnd->sc().value();
   bool              flip   = bnd->flx().value();
   real              angle  = bnd->rot().value();
   telldata::TtPnt   rpnt   = bnd->p();
   word              la     = getCurrentLayer()->value();
   delete bnd;

   if (0.0 == magn)
   {
      tell_log(console::MT_ERROR,"Text with size 0. Operation ignored");
      return EXEC_ABORT;
   }
   secureLayer(tell2DBLayer(la));
   real DBscale = PROPC->DBscale();
   CTM ori(TP(rpnt.x(), rpnt.y(), DBscale), magn*DBscale/OPENGL_FONT_UNIT,angle,flip);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* tx = DEBUG_NEW telldata::TtLayout(tDesign->addText(tell2DBLayer(la), text, ori), tell2DBLayer(la));
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(la));
      OPstack.push(tx);UNDOPstack.push_front(tx->selfcopy());
      LogFile << LogFile.getFN() << "(\"" << text << "\"," << la << "," << rpnt <<
            "," << angle << "," << LogFile._2bool(flip) << "," << magn << ");";
      LogFile.flush();
   }
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;

   //   return stdADDTEXT::execute();
}

//=============================================================================
tellstdfunc::stdCELLREF::stdCELLREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor,parsercmd::sdbrUNSORTED)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

void tellstdfunc::stdCELLREF::undo_cleanup()
{
   telldata::TtLayout* cl = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete cl;
}

void tellstdfunc::stdCELLREF::undo()
{
   TEUNDO_DEBUG("cellref(string, point, real, bool, real) UNDO");
   telldata::TtLayout* cl = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(cl->data(),REF_LAY, dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (cl);
   RefreshGL();
}

int tellstdfunc::stdCELLREF::execute()
{
   // get the parameters from the operand stack
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::TtPnt  *rpnt  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   real DBscale = PROPC->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   //
   bool cellFound = false;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      // check that target cell exists - otherwise tmpDraw can't obviously work.
      // there is another more extensive check when the cell is added, there the circular
      // references are checked as well
      laydata::CellDefin strdefn;
      cellFound = dbLibDir->getCellNamePair(name, strdefn);
      if (cellFound)
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         telldata::TtLayout* cl = DEBUG_NEW telldata::TtLayout(tDesign->addCellRef(strdefn,ori), REF_LAY);
         UNDOcmdQ.push_front(this);
         OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
         LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," <<
                           angle << "," << LogFile._2bool(flip) << "," << magn <<");";
         LogFile.flush();
      }
   }
   DATC->unlockTDT(dbLibDir, true);
   delete rpnt;
   if (cellFound)
   {
      RefreshGL();
      return EXEC_NEXT;
   }
   else
   {
      std::string news = "Cell \"";
      news += name; news += "\" is not defined";
      tell_log(console::MT_ERROR,news);
      return EXEC_ABORT;
   }
}

//=============================================================================
tellstdfunc::stdCELLREF_D::stdCELLREF_D(telldata::typeID retype, bool eor) :
      stdCELLREF(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdCELLREF_D::execute()
{
   std::string name = getStringValue();
   bool cellFound = false;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      // check that target cell exists - otherwise tmpDraw can't obviously work.
      // there is another more extensive check when the cell is added, there the circular
      // references are checked as well
      laydata::CellDefin strdefn;
      cellFound = dbLibDir->getCellNamePair(name, strdefn);
   }
   DATC->unlockTDT(dbLibDir, true);

   if (cellFound)
   {
      // stop the thread and wait for input from the GUI
      if (!tellstdfunc::waitGUInput(console::op_cbind, &OPstack, name)) return EXEC_ABORT;
      // get the data from the stack
      telldata::TtBnd *bnd = static_cast<telldata::TtBnd*>(OPstack.top());OPstack.pop();

      OPstack.push(DEBUG_NEW telldata::TtString(name));
      OPstack.push(DEBUG_NEW telldata::TtPnt(bnd->p()));
      OPstack.push(DEBUG_NEW telldata::TtReal(bnd->rot()));
      OPstack.push(DEBUG_NEW telldata::TtBool(bnd->flx()));
      OPstack.push(DEBUG_NEW telldata::TtReal(bnd->sc()));
      delete bnd;
      return stdCELLREF::execute();
   }
   else
   {
      std::string news = "Can't find cell \"";
      news += name; news += "\" ";
      tell_log(console::MT_ERROR,news);
      return EXEC_ABORT;
   }
}


//=============================================================================
tellstdfunc::stdCELLAREF::stdCELLAREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

void tellstdfunc::stdCELLAREF::undo_cleanup()
{
   telldata::TtLayout* cl = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete cl;
}

void tellstdfunc::stdCELLAREF::undo()
{
   TEUNDO_DEBUG("cellaref(string, point, real, bool, real) UNDO");
   telldata::TtLayout* cl = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(cl->data(),REF_LAY, dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (cl);
   RefreshGL();
}

int tellstdfunc::stdCELLAREF::execute()
{
   // get the parameters from the operand stack
   //cellaref("boza",getpoint(),0,false,1,3,2,{30,0},{0,70});
   telldata::TtPnt  *stepTPY  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt  *stepTPX  = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   word   row    = getWordValue();
   word   col    = getWordValue();
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::TtPnt  *rpnt  =
                     static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   real DBscale = PROPC->DBscale();
   TP cDispl(stepTPX->x(), stepTPX->y(), DBscale);
   TP rDispl(stepTPY->x(), stepTPY->y(), DBscale);
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   laydata::ArrayProps arrprops(cDispl, rDispl, col, row);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* cl = DEBUG_NEW telldata::TtLayout(
            tDesign->addCellARef(name,ori,arrprops),REF_LAY);
      UNDOcmdQ.push_front(this);
      OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," <<
               angle << "," << LogFile._2bool(flip) << "," << magn << "," <<
                         col << "," << row << "," << *stepTPX << "," << *stepTPY << ");";
      LogFile.flush();
   }
   delete stepTPY;
   delete stepTPX;
   delete rpnt;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCELLAREFO::stdCELLAREFO(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

void tellstdfunc::stdCELLAREFO::undo_cleanup()
{
   telldata::TtLayout* cl = TELL_UNDOOPS_CLEAN(telldata::TtLayout*);
   delete cl;
}

void tellstdfunc::stdCELLAREFO::undo()
{
   TEUNDO_DEBUG("cellaref(string, point, real, bool, real) UNDO");
   telldata::TtLayout* cl = TELL_UNDOOPS_UNDO(telldata::TtLayout*);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->destroyThis(cl->data(),REF_LAY, dbLibDir);
   }
   DATC->unlockTDT(dbLibDir, true);
   delete (cl);
   RefreshGL();
}

int tellstdfunc::stdCELLAREFO::execute()
{
   // get the parameters from the operand stack
   //cellaref("boza",getpoint(),0,false,1,3,2,30,70);
   real   stepY  = getOpValue();
   real   stepX  = getOpValue();
   word   row    = getWordValue();
   word   col    = getWordValue();
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::TtPnt  *rpnt  =
                     static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   real DBscale = PROPC->DBscale();
   TP cDispl(stepX, 0    , DBscale);
   TP rDispl(    0, stepY, DBscale);
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   laydata::ArrayProps arrprops(cDispl,rDispl,col,row);
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      telldata::TtLayout* cl = DEBUG_NEW telldata::TtLayout(
            tDesign->addCellARef(name,ori,arrprops),REF_LAY);
      UNDOcmdQ.push_front(this);
      OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," <<
               angle << "," << LogFile._2bool(flip) << "," << magn << "," <<
                         col << "," << row << "," << stepX << "," << stepY << ");";
      LogFile.flush();
   }
   delete rpnt;
   DATC->unlockTDT(dbLibDir, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCELLAREFO_D::stdCELLAREFO_D(telldata::typeID retype, bool eor) :
      stdCELLAREFO(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
}

int tellstdfunc::stdCELLAREFO_D::execute()
{
   real        stepY = getOpValue();
   real        stepX = getOpValue();
   word        row   = getWordValue();
   word        col   = getWordValue();
   std::string name  = getStringValue();

   // check that target cell exists - otherwise tmpDraw can't obviously work.
   // there is another more extensive check when the cell is added, there the circular
   // references are checked as well
   laydata::TdtCell *excell = NULL;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      excell = static_cast<laydata::TdtCell*>(tDesign->checkCell(name));
   }
   DATC->unlockTDT(dbLibDir, true);
   if (NULL == excell)
   {
      std::string news = "Can't find cell \"";
      news += name; news += "\" ";
      tell_log(console::MT_ERROR,news);
      return EXEC_ABORT;
   }

   real DBscale = PROPC->DBscale();
   int4b istepX = (int4b)rint(stepX * DBscale);
   int4b istepY = (int4b)rint(stepY * DBscale);

   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_abind, &OPstack, name, CTM(), istepX, istepY,col,row))
      return EXEC_ABORT;
   // get the data from the stack
   telldata::TtBnd *bnd = static_cast<telldata::TtBnd*>(OPstack.top());OPstack.pop();

   OPstack.push(DEBUG_NEW telldata::TtString(name));
   OPstack.push(DEBUG_NEW telldata::TtPnt(bnd->p()));
   OPstack.push(DEBUG_NEW telldata::TtReal(bnd->rot()));
   OPstack.push(DEBUG_NEW telldata::TtBool(bnd->flx()));
   OPstack.push(DEBUG_NEW telldata::TtReal(bnd->sc()));
   OPstack.push(DEBUG_NEW telldata::TtInt(col));
   OPstack.push(DEBUG_NEW telldata::TtInt(row));
   OPstack.push(DEBUG_NEW telldata::TtReal(stepX));
   OPstack.push(DEBUG_NEW telldata::TtReal(stepY));
   delete bnd;
   return stdCELLAREFO::execute();
}

//=============================================================================
tellstdfunc::stdUSINGLAYER::stdUSINGLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

void tellstdfunc::stdUSINGLAYER::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
}

void tellstdfunc::stdUSINGLAYER::undo()
{
   TEUNDO_DEBUG("usinglayer( int ) UNDO");
   word layno = getWordValue(UNDOPstack, true);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      TpdPost::layer_default(tell2DBLayer(layno), drawProp->curLay());
      drawProp->defaultLayer(tell2DBLayer(layno));
   }
   PROPC->unlockDrawProp(drawProp, true);
}

int tellstdfunc::stdUSINGLAYER::execute()
{
   word layno = getWordValue();
   // Unlock and Unhide the layer(if needed)
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (drawProp->layerHidden(tell2DBLayer(layno)))
      {
         drawProp->hideLayer(tell2DBLayer(layno), false);
         TpdPost::layer_status(tui::BT_LAYER_HIDE, layno, false);
      }
      if (drawProp->layerLocked(tell2DBLayer(layno)))
      {
         drawProp->lockLayer(tell2DBLayer(layno), false);
         TpdPost::layer_status(tui::BT_LAYER_LOCK, layno, false);
      }
      TpdPost::layer_default(layno, drawProp->curLay());
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(drawProp->curLay()));
      drawProp->defaultLayer(tell2DBLayer(layno));
      LogFile << LogFile.getFN() << "("<< layno << ");";LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp,true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUSINGLAYER_S::stdUSINGLAYER_S(telldata::typeID retype, bool eor) :
      stdUSINGLAYER(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdUSINGLAYER_S::execute()
{
  std::string layname = getStringValue();
  layprop::DrawProperties* drawProp;
  unsigned layno = ERR_LAY;
  if (PROPC->lockDrawProp(drawProp))
  {
     layno = drawProp->getLayerNo(layname);
  }
  PROPC->unlockDrawProp(drawProp, true);
  if (ERR_LAY != layno)
  {
    OPstack.push(DEBUG_NEW telldata::TtInt(layno));
    return stdUSINGLAYER::execute();
  }
  else
  {// no layer with this name
    std::string news = "layer \"";
    news += layname; news += "\" is not defined";
    tell_log(console::MT_ERROR,news);
    return EXEC_ABORT;
  }
}
