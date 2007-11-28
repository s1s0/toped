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

#include "../tpd_DB/datacenter.h"
#include "../tpd_DB/browsers.h"


extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdADDBOX::stdADDBOX(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdADDBOX::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete bx;
}

void tellstdfunc::stdADDBOX::undo() {
   TEUNDO_DEBUG("addbox(box, int) UNDO");
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack,true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(bx->data(),la);
   DATC->unlockDB();
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdADDBOX::execute() {
   UNDOcmdQ.push_front(this);
   word la = getWordValue();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = DEBUG_NEW telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB),la);
   DATC->unlockDB();
   OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *w << "," << la << ");";LogFile.flush();
   delete w;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOX_D::stdADDBOX_D(telldata::typeID retype, bool eor) :
      stdADDBOX(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

int tellstdfunc::stdADDBOX_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOX::execute();
}

//=============================================================================
tellstdfunc::stdDRAWBOX::stdDRAWBOX(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdDRAWBOX::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete bx;
}

void tellstdfunc::stdDRAWBOX::undo() {
   TEUNDO_DEBUG("drawbox(int) UNDO");
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(bx->data(),la);
   DATC->unlockDB();
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdDRAWBOX::execute() {
   word     la = getWordValue();
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   UNDOcmdQ.push_front(this);
   la = DATC->curcmdlay();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = DEBUG_NEW telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();
   OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
   LogFile << "addbox("<< *w << "," << la << ");";LogFile.flush();
   delete w;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWBOX_D::stdDRAWBOX_D(telldata::typeID retype, bool eor) :
      stdDRAWBOX(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdDRAWBOX_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWBOX::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXr::stdADDBOXr(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdADDBOXr::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete bx;
}

void tellstdfunc::stdADDBOXr::undo() {
   TEUNDO_DEBUG("addbox(point, real, real, int) UNDO");
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(bx->data(),la);
   DATC->unlockDB();
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdADDBOXr::execute() {
   UNDOcmdQ.push_front(this);
   word     la = getWordValue();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
   real heigth = getOpValue();
   real width  = getOpValue();
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt  p2 = telldata::ttpnt(p1->x()+width,p1->y()+heigth);
   real DBscale = DATC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(p2.x() , p2.y() , DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = DEBUG_NEW telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();
   OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *p1 << "," << width << "," << heigth <<
                                              "," << la << ");"; LogFile.flush();
   delete p1;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOXr_D::stdADDBOXr_D(telldata::typeID retype, bool eor) :
      stdADDBOXr(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdADDBOXr_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOXr::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXp::stdADDBOXp(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdADDBOXp::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete bx;
}

void tellstdfunc::stdADDBOXp::undo() {
   TEUNDO_DEBUG("addbox(point, point, int) UNDO");
   telldata::ttlayout* bx = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(bx->data(),la);
   DATC->unlockDB();
   delete (bx);
   RefreshGL();
}

int tellstdfunc::stdADDBOXp::execute() {
   UNDOcmdQ.push_front(this);
   word     la = getWordValue();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(p2->x(), p2->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = DEBUG_NEW telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();   
   OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << "," << la << ");"; 
   LogFile.flush();
   delete p1; delete p2;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOXp_D::stdADDBOXp_D(telldata::typeID retype, bool eor) :
      stdADDBOXp(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

int tellstdfunc::stdADDBOXp_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOXp::execute();
}

//=============================================================================
tellstdfunc::stdADDPOLY::stdADDPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdADDPOLY::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* ply = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete ply;
}

void tellstdfunc::stdADDPOLY::undo() {
   TEUNDO_DEBUG("addpoly(point list, int) UNDO");
   telldata::ttlayout* ply = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(ply->data(),la);
   DATC->unlockDB();
   delete (ply);
   RefreshGL();
}

int tellstdfunc::stdADDPOLY::execute() {
   word     la = getWordValue();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() >= 3) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
      real DBscale = DATC->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* ply = DEBUG_NEW telldata::ttlayout(ATDB->addpoly(la,plst), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
      LogFile << LogFile.getFN() << "("<< *pl << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 3 points expected to create a polygon");
      OPstack.push(DEBUG_NEW telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDPOLY_D::stdADDPOLY_D(telldata::typeID retype, bool eor) :
      stdADDPOLY(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_pnt)));
}

int tellstdfunc::stdADDPOLY_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDPOLY::execute();
}

//=============================================================================
tellstdfunc::stdDRAWPOLY::stdDRAWPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdDRAWPOLY::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* ply = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete ply;
}

void tellstdfunc::stdDRAWPOLY::undo() {
   TEUNDO_DEBUG("drawpoly(int) UNDO");
   telldata::ttlayout* ply = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(ply->data(),la);
   DATC->unlockDB();
   delete (ply);
   RefreshGL();
}

int tellstdfunc::stdDRAWPOLY::execute() {
   word     la = getWordValue();
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dpoly, &OPstack)) return EXEC_ABORT;
   la = DATC->curcmdlay();
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() >= 3) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
      real DBscale = DATC->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* ply = DEBUG_NEW telldata::ttlayout(ATDB->addpoly(la,plst), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
      LogFile << "addpoly("<< *pl << "," << la << ");"; LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 3 points expected to create a polygon");
      OPstack.push(DEBUG_NEW telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWPOLY_D::stdDRAWPOLY_D(telldata::typeID retype, bool eor) :
      stdDRAWPOLY(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdDRAWPOLY_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWPOLY::execute();
}

//=============================================================================
tellstdfunc::stdADDWIRE::stdADDWIRE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdADDWIRE::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* wr = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete wr;
}

void tellstdfunc::stdADDWIRE::undo() {
   TEUNDO_DEBUG("addwire(point list, real, int) UNDO");
   telldata::ttlayout* wr = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(wr->data(),la);
   DATC->unlockDB();
   delete (wr);
   RefreshGL();
}

int tellstdfunc::stdADDWIRE::execute() {
   word     la = getWordValue();
   real      w = getOpValue();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 1) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
      real DBscale = DATC->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* wr = DEBUG_NEW telldata::ttlayout(ATDB->addwire(la,plst,
                                    static_cast<word>(rint(w * DBscale))), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(wr);UNDOPstack.push_front(wr->selfcopy());
      LogFile << LogFile.getFN() << "("<< *pl << "," << w << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 2 points expected to create a wire");
      OPstack.push(DEBUG_NEW telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDWIRE_D::stdADDWIRE_D(telldata::typeID retype, bool eor) :
      stdADDWIRE(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdADDWIRE_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDWIRE::execute();
}

//=============================================================================
tellstdfunc::stdDRAWWIRE::stdDRAWWIRE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdDRAWWIRE::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* wr = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete wr;
}

void tellstdfunc::stdDRAWWIRE::undo() {
   TEUNDO_DEBUG("drawwire(real, int) UNDO");
   telldata::ttlayout* wr = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(wr->data(),la);
   DATC->unlockDB();
   delete (wr);
   RefreshGL();
}

int tellstdfunc::stdDRAWWIRE::execute() {
   word     la = getWordValue();
   real      w = getOpValue();
   real DBscale = DATC->DBscale();
   if (!tellstdfunc::waitGUInput(static_cast<int>(rint(w * DBscale)), &OPstack)) return EXEC_ABORT;
   la = DATC->curcmdlay();
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 1) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* wr = DEBUG_NEW telldata::ttlayout(ATDB->addwire(la,plst,
                                    static_cast<word>(rint(w * DBscale))), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(wr);UNDOPstack.push_front(wr->selfcopy());
      LogFile << "addwire(" << *pl << "," << w << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 2 points expected to create a wire");
      OPstack.push(DEBUG_NEW telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWWIRE_D::stdDRAWWIRE_D(telldata::typeID retype, bool eor) :
      stdDRAWWIRE(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdDRAWWIRE_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWWIRE::execute();
}

//=============================================================================
tellstdfunc::stdADDTEXT::stdADDTEXT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

void tellstdfunc::stdADDTEXT::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete tx;
}

void tellstdfunc::stdADDTEXT::undo() {
   TEUNDO_DEBUG("addtext(string, int, point, real, bool, real) UNDO");
   telldata::ttlayout* tx = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   word la = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(tx->data(),la);
   DATC->unlockDB();
   delete (tx);
   RefreshGL();
}

int tellstdfunc::stdADDTEXT::execute() {
   // get the parameters from the operand stack
   UNDOcmdQ.push_front(this);
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::ttpnt  *rpnt  = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   word      la  = getWordValue();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(la));
   std::string text = getStringValue();
   real DBscale = DATC->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale),
                                     magn*DBscale/OPENGL_FONT_UNIT,angle,flip);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* tx = DEBUG_NEW telldata::ttlayout(ATDB->addtext(la, text, ori), la);
   DATC->unlockDB();
   OPstack.push(tx);UNDOPstack.push_front(tx->selfcopy());
   LogFile << LogFile.getFN() << "(\"" << text << "\"," << la << "," << *rpnt <<
         "," << angle << "," << LogFile._2bool(flip) << "," << magn << ");";
   LogFile.flush();
   delete rpnt;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDTEXT_D::stdADDTEXT_D(telldata::typeID retype, bool eor) :
      stdADDTEXT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdADDTEXT_D::execute() {
   real   magn   = getOpValue();
   std::string name = getStringValue();
   CTM ftrans;
   ftrans.Scale(magn,magn);
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_tbind, &OPstack, name, ftrans)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttbnd *bnd = static_cast<telldata::ttbnd*>(OPstack.top());OPstack.pop();

   OPstack.push(DEBUG_NEW telldata::ttstring(name));
   OPstack.push(CurrentLayer());
   OPstack.push(DEBUG_NEW telldata::ttpnt(bnd->p()));
   OPstack.push(DEBUG_NEW telldata::ttreal(bnd->rot()));
   OPstack.push(DEBUG_NEW telldata::ttbool(bnd->flx()));
   OPstack.push(DEBUG_NEW telldata::ttreal(bnd->sc()));
   delete bnd;
   return stdADDTEXT::execute();
}

//=============================================================================
tellstdfunc::stdCELLREF::stdCELLREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

void tellstdfunc::stdCELLREF::undo_cleanup() {
   telldata::ttlayout* cl = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete cl;
}

void tellstdfunc::stdCELLREF::undo() {
   TEUNDO_DEBUG("cellref(string, point, real, bool, real) UNDO");
   telldata::ttlayout* cl = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(cl->data(),0);
   DATC->unlockDB();   
   delete (cl);
   RefreshGL();
}

int tellstdfunc::stdCELLREF::execute() {
   UNDOcmdQ.push_front(this);
   // get the parameters from the operand stack
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::ttpnt  *rpnt  = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   real DBscale = DATC->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* cl = DEBUG_NEW telldata::ttlayout(ATDB->addcellref(name,ori), 0);
   DATC->unlockDB();
   OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," << 
                     angle << "," << LogFile._2bool(flip) << "," << magn <<");";
   LogFile.flush();
   delete rpnt;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCELLREF_D::stdCELLREF_D(telldata::typeID retype, bool eor) :
      stdCELLREF(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdCELLREF_D::execute() {
   std::string name = getStringValue();

   // check that target cell exists - otherwise tmp_draw can't onviously work.
   // there is another more extensive check when the cell is added, there the circular
   // references are checked as well 
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
   laydata::tdtcell *excell = ATDB->checkcell(name);
   DATC->unlockDB();
   if (NULL == excell)
   {
      std::string news = "Can't find cell \"";
      news += name; news += "\" ";
      tell_log(console::MT_ERROR,news);
      return EXEC_ABORT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_cbind, &OPstack, name)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttbnd *bnd = static_cast<telldata::ttbnd*>(OPstack.top());OPstack.pop();

   OPstack.push(DEBUG_NEW telldata::ttstring(name));
   OPstack.push(DEBUG_NEW telldata::ttpnt(bnd->p()));
   OPstack.push(DEBUG_NEW telldata::ttreal(bnd->rot()));
   OPstack.push(DEBUG_NEW telldata::ttbool(bnd->flx()));
   OPstack.push(DEBUG_NEW telldata::ttreal(bnd->sc()));
   delete bnd;
   return stdCELLREF::execute();
}

//=============================================================================
tellstdfunc::stdCELLAREF::stdCELLAREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

void tellstdfunc::stdCELLAREF::undo_cleanup() {
   telldata::ttlayout* cl = static_cast<telldata::ttlayout*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete cl;
}

void tellstdfunc::stdCELLAREF::undo() {
   TEUNDO_DEBUG("cellaref(string, point, real, bool, real) UNDO");
   telldata::ttlayout* cl = static_cast<telldata::ttlayout*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->destroy_this(cl->data(),0);
   DATC->unlockDB();
   delete (cl);
   RefreshGL();
}

int tellstdfunc::stdCELLAREF::execute() {
   UNDOcmdQ.push_front(this);
   // get the parameters from the operand stack
   //cellaref("boza",getpoint(),0,false,1,3,2,30,70);   
   real   stepY  = getOpValue();
   real   stepX  = getOpValue();
   word   row    = getWordValue();
   word   col    = getWordValue();
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::ttpnt  *rpnt  = 
                     static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   real DBscale = DATC->DBscale();
   int4b istepX = (int4b)rint(stepX * DBscale);
   int4b istepY = (int4b)rint(stepY * DBscale);
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   laydata::ArrayProperties arrprops(istepX,istepY,col,row);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* cl = DEBUG_NEW telldata::ttlayout(
            ATDB->addcellaref(name,ori,arrprops),0);
   DATC->unlockDB();
   OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," << 
            angle << "," << LogFile._2bool(flip) << "," << magn << "," << 
                      col << "," << row << "," << stepX << "," << stepY << ");";
   LogFile.flush();
   delete rpnt;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCELLAREF_D::stdCELLAREF_D(telldata::typeID retype, bool eor) :
      stdCELLAREF(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdCELLAREF_D::execute() {
   real        stepY = getOpValue();
   real        stepX = getOpValue();
   word        row   = getWordValue();
   word        col   = getWordValue();
   std::string name  = getStringValue();
   
   // check that target cell exists - otherwise tmp_draw can't onviously work.
   // there is another more extensive check when the cell is added, there the circular
   // references are checked as well 
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
   laydata::tdtcell *excell = ATDB->checkcell(name);
   DATC->unlockDB();
   if (NULL == excell)
   {
      std::string news = "Can't find cell \"";
      news += name; news += "\" ";
      tell_log(console::MT_ERROR,news);
      return EXEC_ABORT;
   }
   
   real DBscale = DATC->DBscale();
   int4b istepX = (int4b)rint(stepX * DBscale);
   int4b istepY = (int4b)rint(stepY * DBscale);
   
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_abind, &OPstack, name, CTM(), istepX, istepY,col,row))
      return EXEC_ABORT;
   // get the data from the stack
   telldata::ttbnd *bnd = static_cast<telldata::ttbnd*>(OPstack.top());OPstack.pop();

   OPstack.push(DEBUG_NEW telldata::ttstring(name));
   OPstack.push(DEBUG_NEW telldata::ttpnt(bnd->p()));
   OPstack.push(DEBUG_NEW telldata::ttreal(bnd->rot()));
   OPstack.push(DEBUG_NEW telldata::ttbool(bnd->flx()));
   OPstack.push(DEBUG_NEW telldata::ttreal(bnd->sc()));
   OPstack.push(DEBUG_NEW telldata::ttint(col));
   OPstack.push(DEBUG_NEW telldata::ttint(row));
   OPstack.push(DEBUG_NEW telldata::ttreal(stepX));
   OPstack.push(DEBUG_NEW telldata::ttreal(stepY));
   delete bnd;
   return stdCELLAREF::execute();
}

//=============================================================================
tellstdfunc::stdUSINGLAYER::stdUSINGLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdUSINGLAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
}

void tellstdfunc::stdUSINGLAYER::undo() {
   TEUNDO_DEBUG("usinglayer( int ) UNDO");
   word layno = getWordValue(UNDOPstack, true);
   browsers::layer_default(layno, DATC->curlay());
   DATC->defaultlayer(layno);
}

int tellstdfunc::stdUSINGLAYER::execute() {
   word layno = getWordValue();
   // Unlock and Unhide the layer(if needed)
   if (DATC->layerHidden(layno)) {
      DATC->hideLayer(layno, false);
      browsers::layer_status(browsers::BT_LAYER_HIDE, layno, false);
   }   
   if (DATC->layerLocked(layno)) {
      DATC->lockLayer(layno, false);
      browsers::layer_status(browsers::BT_LAYER_LOCK, layno, false);
   }   
   browsers::layer_default(layno, DATC->curlay());
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(DATC->curlay()));
   DATC->defaultlayer(layno);
   LogFile << LogFile.getFN() << "("<< layno << ");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUSINGLAYER_S::stdUSINGLAYER_S(telldata::typeID retype, bool eor) :
      stdUSINGLAYER(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdUSINGLAYER_S::execute() {
  std::string layname = getStringValue();
  word layno = DATC->getLayerNo(layname);
  if (layno > 0) {
    OPstack.push(DEBUG_NEW telldata::ttint(layno));
    return stdUSINGLAYER::execute();
  }
  else {// no layer with this name
    std::string news = "layer \"";
    news += layname; news += "\" is not defined";
    tell_log(console::MT_ERROR,news);
    return EXEC_NEXT;
  }
}
