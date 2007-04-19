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
//        Created: Fri Jan 24 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Implementation of all TELL build-in functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <math.h>
#include <sstream>
#include "tellibin.h"
#include "../tpd_parser/ted_prompt.h"
#include "../tpd_DB/tedat.h"
#include "../tpd_DB/datacenter.h"
#include "../tpd_DB/browsers.h"
#include "../tpd_DB/viewprop.h"
#include "../tpd_common/tedop.h"
#include "../tpd_common/tuidefs.h"
#include "resourcecenter.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;
extern console::ted_cmd*         Console;

extern wxFrame*                  TopedMainW;
extern wxWindow*                 TopedCanvasW;

extern const wxEventType         wxEVT_SETINGSMENU;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CANVAS_CURSOR;


//=============================================================================
int tellstdfunc::stdECHO::argsOK(argumentQ* amap) {
   return (!(amap->size() == 1));
}

nameList* tellstdfunc::stdECHO::callingConv(const telldata::typeMAP*) {
   nameList* argtypes = new nameList();
   argtypes->push_back("void");
   argtypes->push_back("<...anything...>");
   return argtypes;
}

int tellstdfunc::stdECHO::execute() {
   real DBscale = DATC->DBscale();
   telldata::tell_var *p = OPstack.top();OPstack.pop();
   std::string news;
   p->echo(news, DBscale);
   tell_log(console::MT_INFO,news);
   delete p;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdTELLSTATUS::stdTELLSTATUS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdTELLSTATUS::execute() {
   real DBscale = DATC->DBscale();
   telldata::tell_var *y;
   std::string news;
   while (OPstack.size()) {
      y = OPstack.top(); OPstack.pop();
      y->echo(news, DBscale);
      tell_log(console::MT_ERROR,news);
   }
   news = "Bottom of the operand stack reached";
   tell_log(console::MT_ERROR,news);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTSLCTD::stdREPORTSLCTD(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdREPORTSLCTD::execute() {
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->report_selected(DBscale);
   DATC->unlockDB();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTLAY::stdREPORTLAY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::stdREPORTLAY::execute() {
   bool recursive = getBoolValue();
   std::string cellname = getStringValue();
   laydata::ListOfWords ull;
   laydata::tdtdesign* ATDB = DATC->lockDB();
      bool success = ATDB->collect_usedlays(cellname, recursive, ull);
   DATC->unlockDB();
   telldata::ttlist* tllull = new telldata::ttlist(telldata::tn_int);
   if (success) {
      for(laydata::ListOfWords::const_iterator CL = ull.begin() ; CL != ull.end();CL++ )
         tllull->add(new telldata::ttint(*CL));
      ull.clear();
   }
   else {
      std::string news = "cell \"";
      news += cellname; news += "\" doesn't exists";
      tell_log(console::MT_ERROR,news);
   }
   OPstack.push(tllull);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDISTANCE::stdDISTANCE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
}

int tellstdfunc::stdDISTANCE::execute()
{
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
//   real DBscale = DATC->DBscale();

   telldata::ttpnt* p1 = NULL;
   telldata::ttpnt* p2 = NULL;
   for (unsigned i = 0; i < pl->size(); i++) {
      p1 = p2;
      p2 = static_cast<telldata::ttpnt*>((pl->mlist())[i]);
      if (NULL != p1)
      {
         TP ap1 = TP(p1->x(),p1->y(), DATC->DBscale());
         TP ap2 = TP(p2->x(),p2->y(), DATC->DBscale());
         DATC->addRuler(ap1,ap2);
/*         std::ostringstream info;
         info << "Distance {" << p1->x() << " , " << p1->y() <<"}  -  {"
                              << p2->x() << " , " << p2->y() <<"}  is ";
         tell_log(console::MT_WARNING,info.str());*/
      }
   }

   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDISTANCE_D::stdDISTANCE_D(telldata::typeID retype, bool eor) :
      stdDISTANCE(new parsercmd::argumentLIST,retype,eor)
{
}

int tellstdfunc::stdDISTANCE_D::execute()
{
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_line, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   telldata::ttlist* plst = new telldata::ttlist(telldata::tn_pnt);
   plst->add(new telldata::ttpnt(w->p1().x(), w->p1().y()));
   plst->add(new telldata::ttpnt(w->p2().x(), w->p2().y()));
   OPstack.push(plst);
   delete w;
   return stdDISTANCE::execute();

}

//=============================================================================
tellstdfunc::stdCLEARRULERS::stdCLEARRULERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdCLEARRULERS::execute()
{
   DATC->clearRulers();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLONGCURSOR::stdLONGCURSOR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::stdLONGCURSOR::execute()
{
   bool        longcur  = getBoolValue();
   
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((longcur ? tui::STS_LONG_CURSOR : tui::STS_SHORT_CURSOR));
   wxPostEvent(TopedMainW, eventGRIDUPD);

   wxCommandEvent eventCNVS(wxEVT_CANVAS_CURSOR);
   eventCNVS.SetInt((longcur ? 1 : 0));
   wxPostEvent(TopedCanvasW, eventCNVS);

   LogFile << LogFile.getFN() << "(" << LogFile._2bool(longcur) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTLAYc::stdREPORTLAYc(telldata::typeID retype, bool eor) :
      stdREPORTLAY(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::stdREPORTLAYc::execute() {
   bool recursive = getBoolValue();
   OPstack.push(new telldata::ttstring(""));
   OPstack.push(new telldata::ttbool(recursive));
   return stdREPORTLAY::execute();
}

//=============================================================================
tellstdfunc::stdUNDO::stdUNDO(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdUNDO::execute() {
   if (UNDOcmdQ.size() > 0) {
      UNDOcmdQ.front()->undo(); UNDOcmdQ.pop_front();
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   else {
      std::string news = "UNDO buffer is empty";
      tell_log(console::MT_ERROR,news);
   }   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREDRAW::stdREDRAW(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdREDRAW::execute()
{
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMWIN::stdZOOMWIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

int tellstdfunc::stdZOOMWIN::execute() {
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   DBbox* box = new DBbox(TP(p1->x(), p1->y(), DBscale), 
                          TP(p2->x(), p2->y(), DBscale));
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(TopedCanvasW, eventZOOM);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMWINb::stdZOOMWINb(telldata::typeID retype, bool eor) :
      stdZOOMWIN(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

int tellstdfunc::stdZOOMWINb::execute() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   DBbox* box = new DBbox(TP(w->p1().x(), w->p1().y(), DBscale), 
                          TP(w->p2().x(), w->p2().y(), DBscale));
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(TopedCanvasW, eventZOOM);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMALL::stdZOOMALL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype, eor)
{}

int tellstdfunc::stdZOOMALL::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      DBbox* ovl  = new DBbox(ATDB->activeoverlap());
   DATC->unlockDB();
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(ovl));
   wxPostEvent(TopedCanvasW, eventZOOM);
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::stdCELLREF::stdCELLREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
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
      telldata::ttlayout* cl = new telldata::ttlayout(ATDB->addcellref(name,ori), 0);
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
      stdCELLREF(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
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

   OPstack.push(new telldata::ttstring(name));
   OPstack.push(new telldata::ttpnt(bnd->p()));
   OPstack.push(new telldata::ttreal(bnd->rot()));
   OPstack.push(new telldata::ttbool(bnd->flx()));
   OPstack.push(new telldata::ttreal(bnd->sc()));
   delete bnd;
   return stdCELLREF::execute();
}

//=============================================================================
tellstdfunc::stdCELLAREF::stdCELLAREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
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
      telldata::ttlayout* cl = new telldata::ttlayout(
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
      stdCELLAREF(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
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

   OPstack.push(new telldata::ttstring(name));
   OPstack.push(new telldata::ttpnt(bnd->p()));
   OPstack.push(new telldata::ttreal(bnd->rot()));
   OPstack.push(new telldata::ttbool(bnd->flx()));
   OPstack.push(new telldata::ttreal(bnd->sc()));
   OPstack.push(new telldata::ttint(col));
   OPstack.push(new telldata::ttint(row));
   OPstack.push(new telldata::ttreal(stepX));
   OPstack.push(new telldata::ttreal(stepY));
   delete bnd;
   return stdCELLAREF::execute();
}

//=============================================================================
tellstdfunc::stdUSINGLAYER::stdUSINGLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   UNDOPstack.push_front(new telldata::ttint(DATC->curlay()));
   DATC->defaultlayer(layno);
   LogFile << LogFile.getFN() << "("<< layno << ");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUSINGLAYER_S::stdUSINGLAYER_S(telldata::typeID retype, bool eor) :
      stdUSINGLAYER(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::stdUSINGLAYER_S::execute() {
  std::string layname = getStringValue();
  word layno = DATC->getLayerNo(layname);
  if (layno > 0) {
    OPstack.push(new telldata::ttint(layno));
    return stdUSINGLAYER::execute();
  }
  else {// no layer with this name
    std::string news = "layer \"";
    news += layname; news += "\" is not defined";
    tell_log(console::MT_ERROR,news);
    return EXEC_NEXT;
  }
}
//=============================================================================
tellstdfunc::stdADDBOX::stdADDBOX(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   UNDOPstack.push_front(new telldata::ttint(la));
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB),la);
   DATC->unlockDB();
   OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *w << "," << la << ");";LogFile.flush();
   delete w;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOX_D::stdADDBOX_D(telldata::typeID retype, bool eor) :
      stdADDBOX(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

int tellstdfunc::stdADDBOX_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOX::execute();
}

//=============================================================================
tellstdfunc::stdDRAWBOX::stdDRAWBOX(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   UNDOPstack.push_front(new telldata::ttint(la));
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();
   OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
   LogFile << "addbox("<< *w << "," << la << ");";LogFile.flush();
   delete w;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWBOX_D::stdDRAWBOX_D(telldata::typeID retype, bool eor) :
      stdDRAWBOX(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdDRAWBOX_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWBOX::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXr::stdADDBOXr(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   UNDOPstack.push_front(new telldata::ttint(la));
   real heigth = getOpValue();
   real width  = getOpValue();
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt  p2 = telldata::ttpnt(p1->x()+width,p1->y()+heigth);
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = new TP(p2.x() , p2.y() , DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
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
      stdADDBOXr(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdADDBOXr_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOXr::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXp::stdADDBOXp(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   UNDOPstack.push_front(new telldata::ttint(la));
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = new TP(p2->x(), p2->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
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
      stdADDBOXp(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

int tellstdfunc::stdADDBOXp_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOXp::execute();
}

//=============================================================================
tellstdfunc::stdADDPOLY::stdADDPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
      UNDOPstack.push_front(new telldata::ttint(la));
      real DBscale = DATC->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* ply = new telldata::ttlayout(ATDB->addpoly(la,plst), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
      LogFile << LogFile.getFN() << "("<< *pl << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 3 points expected to create a polygon");
      OPstack.push(new telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDPOLY_D::stdADDPOLY_D(telldata::typeID retype, bool eor) :
      stdADDPOLY(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
}

int tellstdfunc::stdADDPOLY_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDPOLY::execute();
}

//=============================================================================
tellstdfunc::stdDRAWPOLY::stdDRAWPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() >= 3) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(la));
      real DBscale = DATC->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* ply = new telldata::ttlayout(ATDB->addpoly(la,plst), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
      LogFile << "addpoly("<< *pl << "," << la << ");"; LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 3 points expected to create a polygon");
      OPstack.push(new telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWPOLY_D::stdDRAWPOLY_D(telldata::typeID retype, bool eor) :
      stdDRAWPOLY(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdDRAWPOLY_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWPOLY::execute();
}

//=============================================================================
tellstdfunc::stdADDWIRE::stdADDWIRE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
      UNDOPstack.push_front(new telldata::ttint(la));
      real DBscale = DATC->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* wr = new telldata::ttlayout(ATDB->addwire(la,plst,
                                    static_cast<word>(rint(w * DBscale))), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(wr);UNDOPstack.push_front(wr->selfcopy());
      LogFile << LogFile.getFN() << "("<< *pl << "," << w << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 2 points expected to create a wire");
      OPstack.push(new telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDWIRE_D::stdADDWIRE_D(telldata::typeID retype, bool eor) :
      stdADDWIRE(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdADDWIRE_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDWIRE::execute();
}

//=============================================================================
tellstdfunc::stdDRAWWIRE::stdDRAWWIRE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
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
   if (!tellstdfunc::waitGUInput(static_cast<int>(rint(w * DBscale)), &OPstack))
      return EXEC_ABORT;
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 1) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(la));
      laydata::tdtdesign* ATDB = DATC->lockDB();
         pointlist* plst = t2tpoints(pl,DBscale);
         telldata::ttlayout* wr = new telldata::ttlayout(ATDB->addwire(la,plst,
                                    static_cast<word>(rint(w * DBscale))), la);
         delete plst;
      DATC->unlockDB();
      OPstack.push(wr);UNDOPstack.push_front(wr->selfcopy());
      LogFile << "addwire(" << *pl << "," << w << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 2 points expected to create a wire");
      OPstack.push(new telldata::ttlayout());
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWWIRE_D::stdDRAWWIRE_D(telldata::typeID retype, bool eor) :
      stdDRAWWIRE(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdDRAWWIRE_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWWIRE::execute();
}

//=============================================================================
tellstdfunc::stdADDTEXT::stdADDTEXT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
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
   UNDOPstack.push_front(new telldata::ttint(la));
   std::string text = getStringValue();
   real DBscale = DATC->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale),
                                     magn*DBscale/OPENGL_FONT_UNIT,angle,flip);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* tx = new telldata::ttlayout(ATDB->addtext(la, text, ori), la);
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
      stdADDTEXT(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
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

   OPstack.push(new telldata::ttstring(name));
   OPstack.push(CurrentLayer());
   OPstack.push(new telldata::ttpnt(bnd->p()));
   OPstack.push(new telldata::ttreal(bnd->rot()));
   OPstack.push(new telldata::ttbool(bnd->flx()));
   OPstack.push(new telldata::ttreal(bnd->sc()));
   delete bnd;
   return stdADDTEXT::execute();
}

//=============================================================================
tellstdfunc::getPOINT::getPOINT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::getPOINT::execute() {
   // Here - try a hollow lock/unlock the database just to check that it exists
   // The use of this function should be deprecated
   DATC->lockDB();
   DATC->unlockDB();
   // flag the prompt that we expect a single point & handle a pointer to
   // the operand stack
   Console->waitGUInput(&OPstack, console::op_point, CTM());
   // force the thread in wait condition until the ted_prompt has our data
   Console->threadWaits4->Wait();
   // ... and continue when the thread is woken up
   if (Console->mouseIN_OK())  return EXEC_NEXT;
   else return EXEC_RETURN;
}

//=============================================================================
tellstdfunc::getPOINTLIST::getPOINTLIST(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::getPOINTLIST::execute() {
   // Here - try a hollow lock/unlock the database just to check that it exists
   // The use of this function should be deprecated
   DATC->lockDB();
   DATC->unlockDB();
   // flag the prompt that we expect a list of points & handle a pointer to
   // the operand stack
   Console->waitGUInput(&OPstack, console::op_dpoly, CTM());
   // 
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(-1);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Console->threadWaits4->Wait();
   // ... and continue when the thread is woken up
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   if (Console->mouseIN_OK())  return EXEC_NEXT;
   else return EXEC_RETURN;
}

//=============================================================================
tellstdfunc::stdSELECT::stdSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdSELECT::undo() {
   TEUNDO_DEBUG("select(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_inBox(p1DB, p2DB);
   DATC->unlockDB();
   delete w;delete p1DB; delete p2DB;
   UpdateLV();   
}

int tellstdfunc::stdSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_inBox(p1DB, p2DB);
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSELECT_I::stdSELECT_I(telldata::typeID retype, bool eor) :
      stdSELECT(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECT_TL::stdSELECT_TL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_layout)));
}

void tellstdfunc::stdSELECT_TL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   delete pl;
}

void tellstdfunc::stdSELECT_TL::undo() {
}

int tellstdfunc::stdSELECT_TL::execute() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_fromList(get_ttlaylist(pl));
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSELECTIN::stdSELECTIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdSELECTIN::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected;
}

void tellstdfunc::stdSELECTIN::undo() {
   TEUNDO_DEBUG("select(point) UNDO");
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(selected));
   DATC->unlockDB();
   delete selected;
   UpdateLV();
}

int tellstdfunc::stdSELECTIN::execute() {
   // get the data from the stack
   assert(telldata::tn_pnt == OPstack.top()->get_type());
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(p1->x(), p1->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::atticList* selectedl = ATDB->change_select(p1DB,true);
   DATC->unlockDB();
   if (NULL != selectedl) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(selectedl));
      OPstack.push(make_ttlaylist(selectedl));
      LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
      delete selectedl;
   }   
   delete p1; delete p1DB;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTSELECT::stdPNTSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdPNTSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTSELECT::undo() {
   TEUNDO_DEBUG("pselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_inBox(p1DB, p2DB,true);
   DATC->unlockDB();   
   delete w; delete p1DB; delete p2DB;
   UpdateLV();   
}

int tellstdfunc::stdPNTSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_inBox(p1DB, p2DB,true);
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();   
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTSELECT_I::stdPNTSELECT_I(telldata::typeID retype, bool eor) :
      stdPNTSELECT(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdPNTSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   return stdPNTSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT::stdUNSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdUNSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdUNSELECT::undo() {
   TEUNDO_DEBUG("unselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_inBox(p1DB, p2DB);
   DATC->unlockDB();
   delete w; delete p1DB; delete p2DB;
   UpdateLV();   
}

int tellstdfunc::stdUNSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_inBox(p1DB, p2DB);
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECT_I::stdUNSELECT_I(telldata::typeID retype, bool eor) :
      stdUNSELECT(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT_TL::stdUNSELECT_TL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_layout)));
}

void tellstdfunc::stdUNSELECT_TL::undo() {
}

void tellstdfunc::stdUNSELECT_TL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   delete pl;
}

int tellstdfunc::stdUNSELECT_TL::execute() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(pl));
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();   
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECTIN::stdUNSELECTIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdUNSELECTIN::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected;
}

void tellstdfunc::stdUNSELECTIN::undo() {
   TEUNDO_DEBUG("unselect(point) UNDO");
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();   
   delete selected;
   UpdateLV();
}

int tellstdfunc::stdUNSELECTIN::execute() {
   // get the data from the stack
   assert(telldata::tn_pnt == OPstack.top()->get_type());
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(p1->x(), p1->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::atticList* selectedl = ATDB->change_select(p1DB,false);
   DATC->unlockDB();   
   if (NULL != selectedl) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(selectedl));
      OPstack.push(make_ttlaylist(selectedl));
      LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
      delete selectedl;
   }   
   delete p1; delete p1DB;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTUNSELECT::stdPNTUNSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdPNTUNSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTUNSELECT::undo() {
   TEUNDO_DEBUG("punselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_inBox(p1DB, p2DB,true);
   DATC->unlockDB();
   delete w; delete p1DB; delete p2DB;
   UpdateLV();   
}

int tellstdfunc::stdPNTUNSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_inBox(p1DB, p2DB,true);
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTUNSELECT_I::stdPNTUNSELECT_I(telldata::typeID retype, bool eor) :
      stdPNTUNSELECT(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdPNTUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdPNTUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECTALL::stdSELECTALL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdSELECTALL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdSELECTALL::undo() {
   TEUNDO_DEBUG("select_all() UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_all();
      ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   delete pl;
   UpdateLV();   
}

int tellstdfunc::stdSELECTALL::execute() {
   UNDOcmdQ.push_front(this);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
      ATDB->select_all();
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECTALL::stdUNSELECTALL(telldata::typeID retype, bool eor) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdUNSELECTALL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdUNSELECTALL::undo() {
   TEUNDO_DEBUG("unselect_all() UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();   
   delete (pl);
   UpdateLV();   
}

int tellstdfunc::stdUNSELECTALL::execute() {
   UNDOcmdQ.push_front(this);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
      ATDB->unselect_all();
   DATC->unlockDB();   
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDELETESEL::stdDELETESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdDELETESEL::undo_cleanup() {
   telldata::ttlist* und = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(und);
   delete und;
}

void tellstdfunc::stdDELETESEL::undo() {
   TEUNDO_DEBUG("delete() UNDO");
   telldata::ttlist* und = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->addlist(get_shlaylist(und));
      ATDB->select_fromList(get_ttlaylist(und));
   DATC->unlockDB();   
   delete (und);
   UpdateLV();
}

int tellstdfunc::stdDELETESEL::execute() {
   UNDOcmdQ.push_front(this);
   laydata::atticList* sh_delist = new laydata::atticList();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->delete_selected(sh_delist);
   DATC->unlockDB();   
   UNDOPstack.push_front(make_ttlaylist(sh_delist));
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOPYSEL::stdCOPYSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdCOPYSEL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdCOPYSEL::undo() {
   TEUNDO_DEBUG("copy(point point) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      //clean up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL); 
      ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();   
   delete (pl);
   RefreshGL();
}

int tellstdfunc::stdCOPYSEL::execute() {
   UNDOcmdQ.push_front(this);
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
      ATDB->copy_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale));
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();   
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   delete p1; delete p2;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOPYSEL_D::stdCOPYSEL_D(telldata::typeID retype, bool eor) :
      stdCOPYSEL(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdCOPYSEL_D::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_copy, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   OPstack.push(new telldata::ttpnt(w->p1().x(), w->p1().y()));
   OPstack.push(new telldata::ttpnt(w->p2().x(), w->p2().y()));
   delete w;
   return stdCOPYSEL::execute();
}

//=============================================================================
tellstdfunc::stdMOVESEL::stdMOVESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdMOVESEL::undo_cleanup() {
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(deleted);
   delete added;
   delete deleted;
   delete failed;
   delete p1;
   delete p2;
}

void tellstdfunc::stdMOVESEL::undo() {
   TEUNDO_DEBUG("move(point point) UNDO");
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(failed));
      ATDB->unselect_fromList(get_ttlaylist(added));
      laydata::selectList* fadead[3];
	  byte i;
      for (i = 0; i < 3; fadead[i++] = new laydata::selectList());
      ATDB->move_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale),fadead);
      //@TODO Here - an internal check can be done - all 3 of the fadead lists
      // MUST be empty, otherwise - god knows what's wrong!
      for (i = 0; i < 3; delete fadead[i++]);
      ATDB->select_fromList(get_ttlaylist(failed));
      // put back the replaced (deleted) shapes
      ATDB->addlist(get_shlaylist(deleted));
      // and select them
      ATDB->select_fromList(get_ttlaylist(deleted));
      // delete the added shapes
      for (word j = 0 ; j < added->mlist().size(); j++) {
         ATDB->destroy_this(static_cast<telldata::ttlayout*>(added->mlist()[j])->data(),
                           static_cast<telldata::ttlayout*>(added->mlist()[j])->layer());
      }
   DATC->unlockDB();   
   delete failed;
   delete deleted;
   delete added;
   delete p1; delete p2;
   RefreshGL();
}

int tellstdfunc::stdMOVESEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   // move_selected returns 3 select lists : Failed/Deleted/Added
   // This is because of the modify operations
   laydata::selectList* fadead[3];
   byte i;
   for (i = 0; i < 3; fadead[i++] = new laydata::selectList());
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->move_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale), fadead);
      // save for undo operations ... 
      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
      UNDOPstack.push_front(make_ttlaylist(fadead[2])); // and added
      for (i = 0; i < 3; delete fadead[i++]);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   //delete p1; delete p2; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdMOVESEL_D::stdMOVESEL_D(telldata::typeID retype, bool eor) :
      stdMOVESEL(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdMOVESEL_D::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_move, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   OPstack.push(new telldata::ttpnt(w->p1().x(), w->p1().y()));
   OPstack.push(new telldata::ttpnt(w->p2().x(), w->p2().y()));
   delete w;
   return stdMOVESEL::execute();
}

//=============================================================================
tellstdfunc::stdROTATESEL::stdROTATESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdROTATESEL::undo_cleanup() {
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   getOpValue(UNDOPstack, false);
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(deleted);
   delete added;
   delete deleted;
   delete failed;
   delete p1;
}

void tellstdfunc::stdROTATESEL::undo()
{
   TEUNDO_DEBUG("rotate(point real) UNDO");
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   real   angle  = 360 - getOpValue(UNDOPstack, true);
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(failed));
      ATDB->unselect_fromList(get_ttlaylist(added));
      laydata::selectList* fadead[3];
      byte i;
      for (i = 0; i < 3; fadead[i++] = new laydata::selectList());
      ATDB->rotate_selected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      //@TODO Here - an internal check can be done - all 3 of the fadead lists
      // MUST be empty, otherwise - god knows what's wrong!
      for (i = 0; i < 3; delete fadead[i++]);
      ATDB->select_fromList(get_ttlaylist(failed));
      // put back the replaced (deleted) shapes
      ATDB->addlist(get_shlaylist(deleted));
      // and select them
      ATDB->select_fromList(get_ttlaylist(deleted));
      // delete the added shapes
      for (word j = 0 ; j < added->mlist().size(); j++)
      {
         ATDB->destroy_this(static_cast<telldata::ttlayout*>(added->mlist()[j])->data(),
                           static_cast<telldata::ttlayout*>(added->mlist()[j])->layer());
      }
   DATC->unlockDB();
   delete failed;
   delete deleted;
   delete added;
   delete p1;
   RefreshGL();
}

int tellstdfunc::stdROTATESEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real   angle  = getOpValue();
   UNDOPstack.push_front(new telldata::ttreal(angle));
   real DBscale = DATC->DBscale();
   // rotate_selected returns 3 select lists : Failed/Deleted/Added
   // This is because of the box rotation in which case box has to be converted to polygon
   // Failed shapes here should not exist but no explicit check for this
   laydata::selectList* fadead[3];
   byte i;
   for (i = 0; i < 3; fadead[i++] = new laydata::selectList());
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->rotate_selected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      telldata::ttlist* added = make_ttlaylist(fadead[2]);
      ATDB->select_fromList(get_ttlaylist(added));
      // save for undo operations ... 
      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
      UNDOPstack.push_front(added); // and added
      for (i = 0; i < 3; delete fadead[i++]);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << "," << angle << ");"; LogFile.flush();
   //delete p1; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdROTATESEL_D::stdROTATESEL_D(telldata::typeID retype, bool eor) :
      stdROTATESEL(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdROTATESEL_D::execute()
{
   real   angle  = getOpValue();
   CTM rct;
   rct.Rotate(angle);
   OPstack.push(new telldata::ttreal(angle));
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_rotate, &OPstack, "", rct)) return EXEC_ABORT;
   return stdROTATESEL::execute();
}


//=============================================================================
tellstdfunc::stdFLIPXSEL::stdFLIPXSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdFLIPXSEL::undo_cleanup() {
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete p1;
}

void tellstdfunc::stdFLIPXSEL::undo() {
   TEUNDO_DEBUG("flipX(point) UNDO");
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), true);
   DATC->unlockDB();
   delete p1; 
   RefreshGL();
}

int tellstdfunc::stdFLIPXSEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), true);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPXSEL_D::stdFLIPXSEL_D(telldata::typeID retype, bool eor) :
      stdFLIPXSEL(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdFLIPXSEL_D::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_flipX, &OPstack)) return EXEC_ABORT;
   return stdFLIPXSEL::execute();
}

//=============================================================================
tellstdfunc::stdFLIPYSEL::stdFLIPYSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdFLIPYSEL::undo_cleanup() {
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete p1;
}

void tellstdfunc::stdFLIPYSEL::undo() {
   TEUNDO_DEBUG("flipY(point) UNDO");
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), false);
   DATC->unlockDB();
   delete p1; 
   RefreshGL();
}

int tellstdfunc::stdFLIPYSEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), false);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPYSEL_D::stdFLIPYSEL_D(telldata::typeID retype, bool eor) :
      stdFLIPYSEL(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdFLIPYSEL_D::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_flipY, &OPstack)) return EXEC_ABORT;
   return stdFLIPYSEL::execute();
}

//=============================================================================
tellstdfunc::lgcCUTPOLY::lgcCUTPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
}

void tellstdfunc::lgcCUTPOLY::undo_cleanup() {
   telldata::ttlist* pl4 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl3 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl2 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl1; delete pl2;
   delete pl3; delete pl4;
}

void tellstdfunc::lgcCUTPOLY::undo() {
   TEUNDO_DEBUG("cutpoly() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // now unselect all
      ATDB->unselect_all();
      // get the list of cut-offs
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL);
      delete pl;
      // now get the list of cuts ...
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL);
      delete pl;
      // now get the list of deleted shapes
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // put them back
      ATDB->addlist(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // ... and restore the selection
      ATDB->select_fromList(get_ttlaylist(pl)); 
   DATC->unlockDB();
   delete pl;
   UpdateLV();   
}

int tellstdfunc::lgcCUTPOLY::execute() {
   if (DATC->numselected() == 0) 
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
   else {
      // get the data from the stack
      telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
      real DBscale = DATC->DBscale();
      pointlist *plist = t2tpoints(pl,DBscale);
      laydata::valid_poly check(*plist);
      delete plist;
      if (!check.valid()) {
         tell_log(console::MT_ERROR, "Invalid cutting polygon encountered");
      }
      else {
         //cutpoly returns 3 Attic lists -> Delete/AddSelect/AddOnly,  
         // create and initialize them here
         laydata::atticList* dasao[3];
         for (byte i = 0; i < 3; dasao[i++] = new laydata::atticList());
         laydata::tdtdesign* ATDB = DATC->lockDB();
            if (ATDB->cutpoly(check.get_validated() ,dasao)) {
               // push the command for undo
               UNDOcmdQ.push_front(this);
               UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
               // unselect everything
               ATDB->unselect_all();

               telldata::ttlist* shdeleted = make_ttlaylist(dasao[0]);
               // select the shapes to delete & delete them ...
               ATDB->select_fromList(get_ttlaylist(shdeleted));
               laydata::atticList* sh_delist = new laydata::atticList();
               ATDB->delete_selected(sh_delist);
               // ... not forgetting to save them in the undo data stack for undo
               UNDOPstack.push_front(make_ttlaylist(sh_delist));
               // clean-up the delete attic list
               delete sh_delist; delete shdeleted;
               // add the result of the cut...
               telldata::ttlist* shaddselect = make_ttlaylist(dasao[1]);
               telldata::ttlist* shaddonly = make_ttlaylist(dasao[2]);
               ATDB->addlist(dasao[1]);
               UNDOPstack.push_front(shaddselect);
               // ... the cut-offs ....
               ATDB->addlist(dasao[2]);
               UNDOPstack.push_front(shaddonly);
               // and finally select the_cut
               ATDB->select_fromList(get_ttlaylist(shaddselect));
               LogFile << "polycut("<< *pl << ");"; LogFile.flush();
               delete dasao[0];
               // delete dasao[1]; delete dasao[2]; - deleted by ATDB->addlist
            }
         DATC->unlockDB();
      }
      delete pl;
      UpdateLV();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY_I::lgcCUTPOLY_I(telldata::typeID retype, bool eor) :
      lgcCUTPOLY(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::lgcCUTPOLY_I::execute() {
   if (DATC->numselected() == 0) {
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dpoly, &OPstack)) return EXEC_ABORT;
   return lgcCUTPOLY::execute();
}

//=============================================================================
tellstdfunc::lgcMERGE::lgcMERGE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::lgcMERGE::undo_cleanup() {
   telldata::ttlist* pl3 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl2 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl1; delete pl2;
   delete pl3;
}

void tellstdfunc::lgcMERGE::undo() {
   TEUNDO_DEBUG("merge() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // now unselect all
      ATDB->unselect_all();
      // get the shapes resulted from the merge operation
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL);
      delete pl;
      // now get the list of deleted shapes
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // put them back
      ATDB->addlist(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // ... and restore the selection
      ATDB->select_fromList(get_ttlaylist(pl)); 
   DATC->unlockDB();   
   delete pl;
   UpdateLV();   
}

int tellstdfunc::lgcMERGE::execute() {
   if (DATC->numselected() == 0) {
      tell_log(console::MT_ERROR,"No shapes selected. Nothing to cut");
   }
   else {
      //merge returns 2 Attic lists -> Delete/AddMerged
      // create and initialize them here
      laydata::atticList* dasao[2];
	  byte i;
      for (i = 0; i < 2; dasao[i++] = new laydata::atticList());
      // create a list of currently selected shapes
      laydata::tdtdesign* ATDB = DATC->lockDB();
         telldata::ttlist* listselected = make_ttlaylist(ATDB->shapesel());
         if (ATDB->merge(dasao)) {
/*-!-*/     DATC->unlockDB();
            // push the command for undo
            UNDOcmdQ.push_front(this);
            // save the list of originally selected shapes
            UNDOPstack.push_front(listselected);
            // save the list of deleted shapes
            UNDOPstack.push_front(make_ttlaylist(dasao[0]));
            // add the result of the merge...
            UNDOPstack.push_front(make_ttlaylist(dasao[1]));
            LogFile << "merge( );"; LogFile.flush();
         }
         else {
/*-!-*/     DATC->unlockDB();
            delete listselected;
         }   
      // clean-up the lists
      for (i = 0; i < 2; delete dasao[i++]);
   }
   UpdateLV();
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::GDSreportlay::GDSreportlay(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSreportlay::execute() {
   std::string name = getStringValue();
   GDSin::GDSFile* AGDSDB = DATC->lockGDS();
      GDSin::GDSstructure *src_structure = AGDSDB->GetStructure(name.c_str());
      std::ostringstream ost; 
      if (!src_structure) {
         ost << "GDS structure named \"" << name << "\" does not exists";
         tell_log(console::MT_ERROR,ost.str());
      }
      else 
      {
         ost << "GDS layers found in \"" << name <<"\": ";
         for(int i = 0 ; i < GDS_MAX_LAYER ; i++)
            if (src_structure->Get_Allay(i)) ost << i << " ";
         tell_log(console::MT_INFO,ost.str());
         LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
      }
   DATC->unlockGDS();
   //   DATC->reportGDSlay(name.c_str());
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDMENU::stdADDMENU(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdADDMENU::undo_cleanup() 
{
   //@TODO
/*???Not implemented yet*/
}

void tellstdfunc::stdADDMENU::undo() 
{
   //@TODO
/*???Not implemented yet*/
}

int tellstdfunc::stdADDMENU::execute() 
{
   std::string function = getStringValue();
   std::string hotKey   = getStringValue();
   std::string menu     = getStringValue();

   wxMenuBar *menuBar      = TopedMainW->GetMenuBar();
//   tui::ResourceCenter *resourceCenter = Toped->getResourceCenter();
//   resourceCenter->appendMenu(menu, hotKey, function);
//   resourceCenter->buildMenu(menuBar);

   return EXEC_NEXT;
}
   
//=============================================================================
tellstdfunc::stdCHANGELAY::stdCHANGELAY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
}

void tellstdfunc::stdCHANGELAY::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdCHANGELAY::undo()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   word src = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->transferLayer(get_ttlaylist(pl), src);
   DATC->unlockDB();
   delete pl;
   RefreshGL();
}

int tellstdfunc::stdCHANGELAY::execute()
{
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::selectList *listselected = ATDB->shapesel();
   DATC->unlockDB();
   if (listselected->empty())
   {
      std::ostringstream ost;
      ost << "No objects selected";
      tell_log(console::MT_ERROR,ost.str());
   }
   else
   {
      // prepare undo stacks
      UNDOcmdQ.push_front(this);
      word target = getWordValue();
      UNDOPstack.push_front(new telldata::ttint(target));
      UNDOPstack.push_front(make_ttlaylist(listselected));
      ATDB = DATC->lockDB();
         ATDB->transferLayer(target);
      DATC->unlockDB();
      RefreshGL();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCHANGEREF::stdCHANGEREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdCHANGEREF::undo_cleanup()
{
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
   delete pl1;
}

void tellstdfunc::stdCHANGEREF::undo()
{
   TEUNDO_DEBUG("ungroup() CHANGEREF");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // first save the list of all currently selected components
      laydata::selectList *savelist = ATDB->copy_selist();
      // now unselect all
      ATDB->unselect_all();
      // get the list of new references from the UNDO stack
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL);
      // now get the list of the old cell ref's from the UNDO stack
      telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // and add them to the target cell
      ATDB->addlist(get_shlaylist(pl1)); 
      // select the restored cell refs
      ATDB->select_fromList(get_ttlaylist(pl1)); 
      // now restore selection
      ATDB->select_fromList(savelist);
   DATC->unlockDB();
   // finally - clean-up behind
   delete pl;
   delete pl1;
   UpdateLV();
}

int tellstdfunc::stdCHANGEREF::execute()
{
   std::string newref = getStringValue();
   laydata::shapeList* cells4u = NULL;
   laydata::tdtdesign* ATDB = DATC->lockDB();
      bool refok = ATDB->checkValidRef(newref);
      if (refok)
         cells4u = ATDB->ungroup_prep();
   DATC->unlockDB();
   if (refok)
   {
      if (cells4u->empty())
      {
         tell_log(console::MT_ERROR,"No cell references selected");
         delete cells4u;
      }
      else
      {
         ATDB = DATC->lockDB();
            laydata::atticList* undol2 = ATDB->changeref(cells4u, newref);
         DATC->unlockDB();
         assert(NULL != undol2);
         UNDOcmdQ.push_front(this);
         // Push the list of the cells to be ungroupped first
         laydata::atticList undol;
         undol[0] = cells4u;
         UNDOPstack.push_front(make_ttlaylist(&undol));
         UNDOPstack.push_front(make_ttlaylist(undol2));
         delete cells4u;
         delete undol2;
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
         RefreshGL();
      }
   }
   return EXEC_NEXT;
}

/*
UNDO/REDO operation - some preliminary thoughts
   Where to fit it. Two major possibilities here:
      - generating string commands and using the parser
      - Maintaining UNDO command stack and UNDO operand stack
   1. At this moment seems that the first way has two major drawback - it will 
   mess around with the threads. Undo will be a command that will have to 
   reinvoke the parser to parse another command. The other thing that seems 
   to be potentially hazardous is the maintanance of the operand stack - it 
   seems impossible  to execute properly undo/redo if it is included somewhere
   in a function, script, file, preproc.definition or god knows where else in 
   the language.
   2. Separate UNDO command/operand stack. This solution seems much more robust
   (and of course more complicated to implement). At least it is free of the 
   troubles described above (though have others maybe...)
   The table below is trying to clarify operations over normal and UNDO operand
   stacks and when they are executed.
   ============================================================================
                    |    filled during        |         emptied
   ============================================================================
    CMD_STACK       |     parsing             |        execution
   ----------------------------------------------------------------------------
    UNDO CMD_STACK  | execution of UNDOable   | undo execution  OR reaching
                    |      command            |  the limit of UNDO CMD_STACK
   ----------------------------------------------------------------------------
    OP_STACK        |     execution           |        execution
   ----------------------------------------------------------------------------
    UNDO OP_STACK   | execution of UNDOable   |  undo execution  OR reaching
                    |      command            |  the limit of UNDO CMD_STACK
   ----------------------------------------------------------------------------
   What is needed for implementation...
    Additional method (UNDO) in the cmdSTDFUNC. Thus undo will be executed 
    in a similar way as normal commands, but one command at a time - when 
    undo() command is invoked - and calling undo() method instead of execute()
    This has a nasty drawback - all internal commands has to be implemented 
    separately and initialy this will swallow a lot of time. It seems to be
    flexible though and will require minimum amount of additional toped functions.
       

=========================================================================================
|   TELL command   |   tellibin class  |  Docu  |   LOG  |  UNDO  |        Note         |
=========================================================================================
|  "echo"          |  stdECHO          |        |    -   |   -    |                     |
|  "status"        |  stdTELLSTATUS    |        |    -   |   -    |                     |
|  "newdesign"     |  stdNEWDESIGN     |   OK   |    x   |   x*   | if not initial      |
|  "newcell"       |  stdNEWCELL       |   OK   |    x   |   x*   | if not initial      |
|  "gdsread"       |  GDSread          |   OK   |    x   |   -    |                     |
|  "importcell"    |  GDSconvert       |   OK   |    x   |   -*   | issue a warning     |
|  "tdtread"       |  TDTread          |   OK   |    x   |   -*   | issue a warning     |
|  "tdtsave"       |  TDTsave          |   OK   |    x   |   -    |                     |
|  "tdtsaveas"     |  TDTsaveas        |   OK   |    x   |   -    |                     |
|  "opencell"      |  stdOPENCELL      |   OK   |    x   |   x*   | if not initial      |
|  "usinglayer"    |  stdUSINGLAYER    |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdADDBOX        |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdADDBOX_D      |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdADDBOXr       |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdADDBOXr_D     |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdADDBOXp       |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdADDBOXp_D     |   OK   |    x   |   x    |                     |
|  "addbox"        |  stdDRAWBOX       |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addbox"        |  stdDRAWBOX_D     |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addpoly"       |  stdADDPOLY       |   OK   |    x   |   x    |                     |
|  "addpoly"       |  stdADDPOLY_D     |   OK   |    x   |   x    |                     |
|  "addpoly"       |  stdDRAWPOLY      |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addpoly"       |  stdDRAWPOLY_D    |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addwire"       |  stdADDWIRE       |   OK   |    x   |   x    |                     |
|  "addwire"       |  stdADDWIRE_D     |   OK   |    x   |   x    |                     |
|  "addwire"       |  stdDRAWWIRE      |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addwire"       |  stdDRAWWIRE_D    |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addtext"       |  stdADDTEXT       |   OK   |    x   |   x    |                     |
|  "cellref"       |  stdCELLREF       |   OK   |    x   |   x    |                     |
|  "cellaref"      |  stdCELLAREF      |   OK   |    x   |   x    |                     |
|  "select"        |  stdSELECT        |   OK   |    x   |   x    |                     |
|  "select"        |  stdSELECT_I      |   OK   |    x   |   x    |                     |
|  "select"        |  stdSELECT_TL     |   OK   |    -*  |   x    |  see note 3         |
|  "pselect"       |  stdPNTSELECT     |   OK   |    x   |   x    |                     |
|  "pselect"       |  stdPNTSELECT_I   |   OK   |    x   |   x    |                     |
|  "unselect"      |  stdUNSELECT      |   OK   |    x   |   x    |                     |
|  "unselect"      |  stdUNSELECT_I    |   OK   |    x   |   x    |                     |
|  "unselect"      |  stdUNSELECT_TL   |   OK   |    -*  |   x    |  see note 3         |
|  "punselect"     |  stdPNTUNSELECT   |   OK   |    x   |   x    |                     |
|  "punselect"     |  stdPNTUNSELECT_I |   OK   |    x   |   x    |                     |
|  "select_all"    |  stdSELECTALL     |   OK   |    x   |   x    |                     |
|  "unselect_all"  |  stdUNSELECTALL   |   OK   |    x   |   x    |                     |
-----------------------------------------------------------------------------------------
|      operation on the toped data     |        |        |        |                     |
-----------------------------------------------------------------------------------------
|  "move"          |  stdMOVESEL       |   OK   |    x   |   x    |                     |
|  "move"          |  stdMOVESEL_D     |   OK   |    x   |   x    |                     |
|  "copy"          |  stdCOPYSEL       |   OK   |    x   |   x    |                     |
|  "copy"          |  stdCOPYSEL_D     |   OK   |    x   |   x    |                     |
|  "delete"        |  stdDELETESEL     |   OK   |    x   |   x    |                     |
|  "group"         |  stdGROUP         |   OK   |    x   |   x    |                     |
|  "ungroup"       |  stdUNGROUP       |   OK   |    x   |   x    |                     |
-----------------------------------------------------------------------------------------
|        toped specific functons       |        |        |        |                     |
-----------------------------------------------------------------------------------------
|  "getCW"         |  stdGETCW         |   OK   |    -   |   -    |                     |
|  "zoom"          |  stdZOOMWIN       |   OK   |    -   |   -    |  see note 1         |
|  "zoom"          |  stdZOOMWINb      |   OK   |    -   |   -    |  see note 1         |
|  "zoomall"       |  stdZOOMALL       |   OK   |    -   |   -    |  see note 1         |
|  "layprop"       |  stdLAYPROP       |   OK   |    x   |   x*   |  if redefined only  |
|  "hidelayer"     |  stdHIDELAYER     |   OK   |    x   |   x    |                     |
|  "locklayer"     |  stdLOCKLAYER     |   OK   |    x   |   x    |                     |
|  "definecolor"   |  stdCOLORDEF      |   OK   |    x   |   x*   |  if redefined only  |
|  "definefill"    |  stdFILLDEF       |   OK   |    x   |   x*   |  if redefined only  |
|  "definegrid"    |  stdGRIDDEF       |   OK   |    x   |   x*   |  if redefined only  |
|  "step"          |  stdSTEP          |   OK   |    x   |   x*   |  if redefined only  |
|  "grid"          |  stdGRID          |   OK   |    x   |   x    |                     |
|  "getpoint"      |  getPOINT         |   OK   |    -   |   -    |  see note 2         |
|  "getpointlist"  |  getPOINTLIST     |   OK   |    -   |   -    |                     |
=========================================================================================
   Note 1: For zooming a separate stack of available views can be maintained.
   Note 2: For getpoint operation, a local undo should be maintained. Not here,
           but in the tedprompt maybe.
   Note 3: It is a real trouble how to log select/unselect from list! Have no
           idea really!!! Something really to think hard...
*/
