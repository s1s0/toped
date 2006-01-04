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
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Fri Jan 24 2003
//         Author: s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
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
#include "layoutcanvas.h"
#include "datacenter.h"
#include "browsers.h"
#include "../tpd_DB/viewprop.h"
#include "../tpd_common/tedop.h"
#include "toped.h"

//#define TEUNDO_DEBUG_ON
#ifdef TEUNDO_DEBUG_ON
#define TEUNDO_DEBUG(a)  tell_log(console::MT_INFO,a);
#else
#define TEUNDO_DEBUG(a) 
#endif

extern layprop::ViewProperties*  Properties;
extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;
extern tui::TopedFrame*          Toped;


extern const wxEventType         wxEVT_CNVSSTATUSLINE;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_MOUSE_INPUT;


//=============================================================================
int tellstdfunc::stdECHO::argsOK(argumentQ* amap) {
   return (!(amap->size() == 1));
}

std::string tellstdfunc::stdECHO::callingConv() {
   return "(<...anything...>)";
}

int tellstdfunc::stdECHO::execute() {
   telldata::tell_var *p = OPstack.top();OPstack.pop();
   std::string news;
   p->echo(news);
   tell_log(console::MT_INFO,news.c_str());
   delete p;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdTELLSTATUS::stdTELLSTATUS(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdTELLSTATUS::execute() {
   telldata::tell_var *y;
   std::string news;
   while (OPstack.size()) {
      y = OPstack.top(); OPstack.pop();
      y->echo(news);
      tell_log(console::MT_ERROR,news.c_str());
   }
   news = "Bottom of the operand stack reached";
   tell_log(console::MT_ERROR,news.c_str());
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTSLCTD::stdREPORTSLCTD(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdREPORTSLCTD::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->report_selected();
   DATC->unlockDB();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTLAY::stdREPORTLAY(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::stdREPORTLAY::execute() {
   bool recursive = getBoolValue();
   std::string cellname = getStringValue();
   laydata::usedlayList ull;
   laydata::tdtdesign* ATDB = DATC->lockDB();
      bool success = ATDB->collect_usedlays(cellname, recursive, ull);
   DATC->unlockDB();
   telldata::ttlist* tllull = new telldata::ttlist(telldata::tn_int);
   if (success) {
      for(laydata::usedlayList::const_iterator CL = ull.begin() ; CL != ull.end();CL++ )
         tllull->add(new telldata::ttint(*CL));
      ull.clear();
   }
   else {
      std::string news = "cell \"";
      news += cellname; news += "\" doesn't exists";
      tell_log(console::MT_ERROR,news.c_str());
   }
   OPstack.push(tllull);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTLAYc::stdREPORTLAYc(telldata::typeID retype) :
                              stdREPORTLAY(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::stdREPORTLAYc::execute() {
   bool recursive = getBoolValue();
   OPstack.push(new telldata::ttstring(""));
   OPstack.push(new telldata::ttbool(recursive));
   return stdREPORTLAY::execute();
}

//=============================================================================
tellstdfunc::stdUNDO::stdUNDO(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdUNDO::execute() {
   if (UNDOcmdQ.size() > 0) {
      UNDOcmdQ.front()->undo(); UNDOcmdQ.pop_front();
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   else {
      std::string news = "UNDO buffer is empty";
      tell_log(console::MT_ERROR,news.c_str());
   }   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREDRAW::stdREDRAW(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdREDRAW::execute() {
   wxPaintEvent upde(wxEVT_PAINT);
   wxPostEvent(Toped->view(), upde);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMWIN::stdZOOMWIN(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

int tellstdfunc::stdZOOMWIN::execute() {
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   DBbox* box = new DBbox(TP(p1->x(), p1->y(), DBscale), 
                          TP(p2->x(), p2->y(), DBscale));
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(Toped->view(), eventZOOM);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMWINb::stdZOOMWINb(telldata::typeID retype) :
                                stdZOOMWIN(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

int tellstdfunc::stdZOOMWINb::execute() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   DBbox* box = new DBbox(TP(w->p1().x(), w->p1().y(), DBscale), 
                          TP(w->p2().x(), w->p2().y(), DBscale));
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(Toped->view(), eventZOOM);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMALL::stdZOOMALL(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdZOOMALL::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      DBbox* ovl  = new DBbox(ATDB->activeoverlap());
   DATC->unlockDB();
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(ovl));
   wxPostEvent(Toped->view(), eventZOOM);
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::stdLAYPROP::stdLAYPROP(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdLAYPROP::undo_cleanup() {
}

void tellstdfunc::stdLAYPROP::undo() {
}

int tellstdfunc::stdLAYPROP::execute() {
   std::string fill = getStringValue();
   std::string col  = getStringValue();
   word        gdsN = getWordValue();
   std::string name = getStringValue();
   // error message - included in the method
   Properties->addlayer(name, gdsN, col, fill);
   browsers::layer_add(name,gdsN);
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << gdsN << ",\"" << 
                               col << "\",\"" << fill <<"\");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOLORDEF::stdCOLORDEF(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
}

void tellstdfunc::stdCOLORDEF::undo_cleanup() {
}

void tellstdfunc::stdCOLORDEF::undo() {
}

int tellstdfunc::stdCOLORDEF::execute() {
   byte         sat  = getByteValue();
   byte         colB = getByteValue();
   byte         colG = getByteValue();
   byte         colR = getByteValue();
   std::string  name = getStringValue();
   // error message - included in the method
   Properties->addcolor(name, colR, colG, colB, sat);
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << colR << "," << 
                       colG << "," << colB << "," << sat << ");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFILLDEF::stdFILLDEF(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_int)));
}

void tellstdfunc::stdFILLDEF::undo_cleanup() {
}

void tellstdfunc::stdFILLDEF::undo() {
}

int tellstdfunc::stdFILLDEF::execute() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string  name = getStringValue();
   if (sl->size() != 128) {
      tell_log(console::MT_ERROR,"Exactly 128 integers expected in a fill pattern. Ignored...");
   }
   else {
      // declare the array like this because otherwise it'll be wiped
      byte* ptrn = new byte[128];
      telldata::ttint *cmpnt;
      for (unsigned i = 0; i < 128; i++) {
         cmpnt = static_cast<telldata::ttint*>((sl->mlist())[i]);
         if (cmpnt->value() > MAX_BYTE_VALUE) {
            tell_log(console::MT_ERROR,"Value out of range in a pattern definition");
         }
         else ptrn[i] = cmpnt->value();
      }
      // error message - included in the method
      Properties->addfill(name, ptrn);
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << *sl << ");";
      LogFile.flush();
   }
   delete sl;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDELAYER::stdHIDELAYER(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdHIDELAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDELAYER::undo() {
   TEUNDO_DEBUG("hidelayer( word , bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   word        layno = getWordValue(UNDOPstack,true);
   Properties->hideLayer(layno, hide);
   browsers::layer_status(browsers::BT_LAYER_HIDE, layno, hide);
}

int tellstdfunc::stdHIDELAYER::execute() {
   bool        hide  = getBoolValue();
   word        layno = getWordValue();
   if (layno != Properties->curlay()) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(layno));
      UNDOPstack.push_front(new telldata::ttbool(!hide));
      Properties->hideLayer(layno, hide);
      browsers::layer_status(browsers::BT_LAYER_HIDE, layno, hide);
      LogFile << LogFile.getFN() << "("<< layno << "," << 
                 LogFile._2bool(hide) << ");"; LogFile.flush();
      UpdateLV();
   }
   else {
      tell_log(console::MT_ERROR,"Layer above is the current. Can't be hidden");
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDECELLMARK::stdHIDECELLMARK(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdHIDECELLMARK::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDECELLMARK::undo() {
   TEUNDO_DEBUG("hide_cellmarks( bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   Properties->setcellmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   eventGRIDUPD.SetInt((hide ? tui::STS_CELLMARK_OFF : tui::STS_CELLMARK_ON));
   wxPostEvent(Toped->view(), eventGRIDUPD);
   UpdateLV();
}

int tellstdfunc::stdHIDECELLMARK::execute() {
   bool        hide  = getBoolValue();
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(new telldata::ttbool(!hide));
   Properties->setcellmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   eventGRIDUPD.SetInt((hide ? tui::STS_CELLMARK_OFF : tui::STS_CELLMARK_ON));
   wxPostEvent(Toped->view(), eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDETEXTMARK::stdHIDETEXTMARK(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdHIDETEXTMARK::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDETEXTMARK::undo() {
   TEUNDO_DEBUG("hide_textmarks( bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   Properties->settextmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   eventGRIDUPD.SetInt((hide ? tui::STS_TEXTMARK_OFF : tui::STS_TEXTMARK_ON));
   wxPostEvent(Toped->view(), eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   UpdateLV();
}

int tellstdfunc::stdHIDETEXTMARK::execute() {
   bool        hide  = getBoolValue();
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(new telldata::ttbool(!hide));
   Properties->settextmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   eventGRIDUPD.SetInt((hide ? tui::STS_TEXTMARK_OFF : tui::STS_TEXTMARK_ON));
   wxPostEvent(Toped->view(), eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDELAYERS::stdHIDELAYERS(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {

   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_int)));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdHIDELAYERS::undo_cleanup() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   getBoolValue(UNDOPstack, false);
   delete sl;
}

void tellstdfunc::stdHIDELAYERS::undo() {
   TEUNDO_DEBUG("hidelayer( int list , bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttint *laynumber;
   for (unsigned i = 0; i < sl->size() ; i++) {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      Properties->hideLayer(laynumber->value(), hide);
      browsers::layer_status(browsers::BT_LAYER_HIDE, laynumber->value(), hide);
   }
   delete sl;
}

int tellstdfunc::stdHIDELAYERS::execute() {
   bool        hide  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   UNDOcmdQ.push_front(this);
   telldata::ttlist* undolaylist = new telldata::ttlist(telldata::tn_int);
   telldata::ttint *laynumber;
   for (unsigned i = 0; i < sl->size() ; i++) {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      if (/*(laynumber->value() > MAX_LAYER_VALUE) ||*/ (laynumber->value() < 1)) {
         std::ostringstream info;
         info << "Layer number "<< i <<" out of range ... ignored";
         tell_log(console::MT_WARNING,info.str().c_str());
      }
      else if (laynumber->value() == Properties->curlay()) {
         tell_log(console::MT_WARNING,"Current layer ... ignored");
      }
      else {
         Properties->hideLayer(laynumber->value(), hide);
         browsers::layer_status(browsers::BT_LAYER_HIDE, laynumber->value(), hide);
         undolaylist->add(new telldata::ttint(*laynumber));
      }
   }
   UNDOPstack.push_front(undolaylist);
   UNDOPstack.push_front(new telldata::ttbool(!hide));
   LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                      LogFile._2bool(hide) << ");"; LogFile.flush();
   delete sl;
   UpdateLV();
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::stdLOCKLAYER::stdLOCKLAYER(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {

   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdLOCKLAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdLOCKLAYER::undo() {
   TEUNDO_DEBUG("locklayer( word , bool ) UNDO");
   bool        lock  = getBoolValue(UNDOPstack, true);
   word        layno = getWordValue(UNDOPstack, true);
   Properties->lockLayer(layno, lock);
   browsers::layer_status(browsers::BT_LAYER_LOCK, layno, lock);
}

int tellstdfunc::stdLOCKLAYER::execute() {
   bool        lock  = getBoolValue();
   word        layno = getWordValue();
   if (layno != Properties->curlay()) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(layno));
      UNDOPstack.push_front(new telldata::ttbool(!lock));
      Properties->lockLayer(layno, lock);
      browsers::layer_status(browsers::BT_LAYER_LOCK, layno, lock);
      LogFile << LogFile.getFN() << "("<< layno << "," << 
                 LogFile._2bool(lock) << ");"; LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"Layer above is the current. Can't be locked.");
   }   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOCKLAYERS::stdLOCKLAYERS(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {

   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_int)));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdLOCKLAYERS::undo_cleanup() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   getBoolValue(UNDOPstack, false);
   delete sl;
}

void tellstdfunc::stdLOCKLAYERS::undo() {
   TEUNDO_DEBUG("locklayer( int list , bool ) UNDO");
   bool        lock  = getBoolValue(UNDOPstack,true);
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttint *laynumber;
   for (unsigned i = 0; i < sl->size() ; i++) {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      Properties->lockLayer(laynumber->value(), lock);
      browsers::layer_status(browsers::BT_LAYER_LOCK, laynumber->value(), lock);
   }
   delete sl;
}

int tellstdfunc::stdLOCKLAYERS::execute() {
   bool        lock  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   UNDOcmdQ.push_front(this);
   telldata::ttlist* undolaylist = new telldata::ttlist(telldata::tn_int);
   telldata::ttint *laynumber;
   for (int4b i = 0; i < sl->size() ; i++) {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      if (/*(laynumber->value() > MAX_LAYER_VALUE) ||*/ (laynumber->value() < 1)) {
         std::ostringstream info;
         info << "Layer number "<< i <<" out of range ... ignored";
         tell_log(console::MT_WARNING,info.str().c_str());
      }
      else if (laynumber->value() == Properties->curlay()) {
         tell_log(console::MT_WARNING,"Current layer ... ignored");
      }
      else {
         Properties->lockLayer(laynumber->value(), lock);
         browsers::layer_status(browsers::BT_LAYER_LOCK, laynumber->value(), lock);
         undolaylist->add(new telldata::ttint(*laynumber));
      }
   }
   UNDOPstack.push_front(undolaylist);
   UNDOPstack.push_front(new telldata::ttbool(!lock));
   LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                      LogFile._2bool(lock) << ");"; LogFile.flush();
   delete sl;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdNEWDESIGN::stdNEWDESIGN(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::stdNEWDESIGN::execute() {
   std::string nm = getStringValue();
   DATC->newDesign(nm);
   // reset UNDO buffers;
   UNDOcmdQ.clear();
   while (!UNDOPstack.empty()) {
      delete UNDOPstack.front(); UNDOPstack.pop_front();
   }   
   LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdNEWCELL::stdNEWCELL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdNEWCELL::undo_cleanup() {
}

void tellstdfunc::stdNEWCELL::undo() {
}

int tellstdfunc::stdNEWCELL::execute() {
   std::string nm = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      if (!ATDB->addcell(nm)) {
         std::string news = "Cell \"";
         news += nm; news += "\" already exists";
         tell_log(console::MT_ERROR,news.c_str());
      }
      else {
         LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
      }
   DATC->unlockDB();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdOPENCELL::stdOPENCELL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdOPENCELL::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdOPENCELL::undo() {
   TEUNDO_DEBUG("opencell( string ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      assert(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
      DBbox* ovl  = new DBbox(ATDB->activeoverlap());
   DATC->unlockDB();
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(ovl));
   wxPostEvent(Toped->view(), eventZOOM);
}

int tellstdfunc::stdOPENCELL::execute() {
   std::string nm = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      std::string oldnm = ATDB->activecellname();
      telldata::ttlist* selected = NULL;
      if ("" != oldnm)  selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->opencell(nm)) {
         if (oldnm != "") {
            UNDOcmdQ.push_front(this);
            UNDOPstack.push_front(selected);
         }
         DBbox* ovl  = new DBbox(ATDB->activeoverlap());
/*-!-*/  DATC->unlockDB();
         if (*ovl == DEFAULT_OVL_BOX) *ovl = DEFAULT_ZOOM_BOX;
         browsers::celltree_open(nm);
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(tui::ZOOM_WINDOW);
         eventZOOM.SetClientData(static_cast<void*>(ovl));
         wxPostEvent(Toped->view(), eventZOOM);
         LogFile << LogFile.getFN() << "(\""<< nm << "\");"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         std::string news = "cell \"";
         news += nm; news += "\" doesn't exists";
         tell_log(console::MT_ERROR,news.c_str());
         if (selected) delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPUSH::stdEDITPUSH(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdEDITPUSH::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITPUSH::undo() {
   TEUNDO_DEBUG("editpush( point ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      assert(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITPUSH::execute() {
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   TP p1DB = TP(p1->x(), p1->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->editpush(p1DB)) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"No cell reference found on this location");
         delete selected;
      }
   delete p1;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPOP::stdEDITPOP(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {

}

void tellstdfunc::stdEDITPOP::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITPOP::undo() {
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      assert(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();   
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITPOP::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
   telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->editpop()) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"Already on the top level of the curent hierarchy");
         delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITPREV::stdEDITPREV(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {

}

void tellstdfunc::stdEDITPREV::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITPREV::undo() {
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      assert(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITPREV::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->editprev(false)) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"This is the first cell open during this session");
         delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEDITTOP::stdEDITTOP(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {

}

void tellstdfunc::stdEDITTOP::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected; //ttlist does not have active destructor
}

void tellstdfunc::stdEDITTOP::undo() {
   TEUNDO_DEBUG("editpop( ) UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      assert(ATDB->editprev(true));
      browsers::celltree_open(ATDB->activecellname());
      telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      ATDB->select_fromList(get_ttlaylist(selected));
   DATC->unlockDB();   
   delete selected;
   RefreshGL();
}

int tellstdfunc::stdEDITTOP::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlist* selected = make_ttlaylist(ATDB->shapesel());
      if (ATDB->edittop()) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(selected);
         std::string name = ATDB->activecellname();
/*-!-*/  DATC->unlockDB();
         browsers::celltree_highlight(name);
         RefreshGL();
         LogFile << LogFile.getFN() << "();"; LogFile.flush();
      }
      else {
/*-!-*/  DATC->unlockDB();
         tell_log(console::MT_ERROR,"Already on the top level of the curent hierarchy");
         delete selected;
      }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCELLREF::stdCELLREF(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdCELLREF::execute() {
   UNDOcmdQ.push_front(this);
   // get the parameters from the operand stack
   real   magn   = getOpValue();
   bool   flip   = getBoolValue();
   real   angle  = getOpValue();
   telldata::ttpnt  *rpnt  = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   real DBscale = Properties->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* cl = new telldata::ttlayout(ATDB->addcellref(name,ori), 0);
   DATC->unlockDB();
   OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," << 
                     angle << "," << LogFile._2bool(flip) << "," << magn <<");";
   LogFile.flush();
   delete rpnt;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCELLAREF::stdCELLAREF(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
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
   real DBscale = Properties->DBscale();
   stepX = rint(stepX * DBscale);
   stepY = rint(stepY * DBscale);
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), magn,angle,flip);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* cl = new telldata::ttlayout(
            ATDB->addcellaref(name,ori,(int4b)stepX,(int4b)stepY,col,row),0);
   DATC->unlockDB();
   OPstack.push(cl); UNDOPstack.push_front(cl->selfcopy());
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << *rpnt << "," << 
            angle << "," << LogFile._2bool(flip) << "," << magn << "," << 
                      col << "," << row << "," << stepX << "," << stepY << ");";
   LogFile.flush();
   delete rpnt;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUSINGLAYER::stdUSINGLAYER(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
}

void tellstdfunc::stdUSINGLAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
}

void tellstdfunc::stdUSINGLAYER::undo() {
   TEUNDO_DEBUG("usinglayer( int ) UNDO");
   word layno = getWordValue(UNDOPstack, true);
   browsers::layer_default(layno, Properties->curlay());
   Properties->defaultlayer(layno);
}

int tellstdfunc::stdUSINGLAYER::execute() {
   word layno = getWordValue();
   // Unlock and Unhide the layer(if needed)
   if (Properties->drawprop().layerHidden(layno)) {
      Properties->hideLayer(layno, false);
      browsers::layer_status(browsers::BT_LAYER_HIDE, layno, false);
   }   
   if (Properties->drawprop().layerLocked(layno)) {
      Properties->lockLayer(layno, false);
      browsers::layer_status(browsers::BT_LAYER_LOCK, layno, false);
   }   
   browsers::layer_default(layno, Properties->curlay());
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(new telldata::ttint(Properties->curlay()));
   Properties->defaultlayer(layno);
   LogFile << LogFile.getFN() << "("<< layno << ");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUSINGLAYER_S::stdUSINGLAYER_S(telldata::typeID retype) :
                            stdUSINGLAYER(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::stdUSINGLAYER_S::execute() {
  std::string layname = getStringValue();
  word layno = Properties->getlayerNo(layname);
  if (layno > 0) {
    OPstack.push(new telldata::ttint(layno));
    return stdUSINGLAYER::execute();
  }
  else {// no layer with this name
    std::string news = "layer \"";
    news += layname; news += "\" is not defined";
    tell_log(console::MT_ERROR,news.c_str());
    return EXEC_NEXT;
  }
}
//=============================================================================
tellstdfunc::stdADDBOX::stdADDBOX(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdADDBOX::execute() {
   UNDOcmdQ.push_front(this);
   word la = getWordValue();
   UNDOPstack.push_front(new telldata::ttint(la));
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB),la);
   DATC->unlockDB();
   OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *w << "," << la << ");";LogFile.flush();
   delete w;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOX_D::stdADDBOX_D(telldata::typeID retype) :
                                stdADDBOX(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

int tellstdfunc::stdADDBOX_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOX::execute();
}

//=============================================================================
tellstdfunc::stdDRAWBOX::stdDRAWBOX(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdDRAWBOX::execute() {
   UNDOcmdQ.push_front(this);
   word     la = getWordValue();
   UNDOPstack.push_front(new telldata::ttint(la));
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(0, &OPstack)) return EXEC_RETURN;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   TP* p1DB = new TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = new TP(w->p2().x(), w->p2().y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();
   OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
   LogFile << "addbox("<< *w << "," << la << ");";LogFile.flush();
   delete w;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWBOX_D::stdDRAWBOX_D(telldata::typeID retype) :
                               stdDRAWBOX(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdDRAWBOX_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWBOX::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXr::stdADDBOXr(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdADDBOXr::execute() {
   UNDOcmdQ.push_front(this);
   word     la = getWordValue();
   UNDOPstack.push_front(new telldata::ttint(la));
   real heigth = getOpValue();
   real width  = getOpValue();
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt  p2 = telldata::ttpnt(p1->x()+width,p1->y()+heigth);
   real DBscale = Properties->DBscale();
   TP* p1DB = new TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = new TP(p2.x() , p2.y() , DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();
   OPstack.push(bx);UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *p1 << "," << width << "," << heigth <<
                                              "," << la << ");"; LogFile.flush();
   delete p1;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOXr_D::stdADDBOXr_D(telldata::typeID retype) :
                               stdADDBOXr(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdADDBOXr_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOXr::execute();
}

//=============================================================================
tellstdfunc::stdADDBOXp::stdADDBOXp(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdADDBOXp::execute() {
   UNDOcmdQ.push_front(this);
   word     la = getWordValue();
   UNDOPstack.push_front(new telldata::ttint(la));
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   TP* p1DB = new TP(p1->x(), p1->y(), DBscale);
   TP* p2DB = new TP(p2->x(), p2->y(), DBscale);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* bx = new telldata::ttlayout(ATDB->addbox(la, p1DB, p2DB), la);
   DATC->unlockDB();   
   OPstack.push(bx); UNDOPstack.push_front(bx->selfcopy());
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << "," << la << ");"; 
   LogFile.flush();
   delete p1; delete p2;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDBOXp_D::stdADDBOXp_D(telldata::typeID retype) :
                               stdADDBOXp(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

int tellstdfunc::stdADDBOXp_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDBOXp::execute();
}

//=============================================================================
tellstdfunc::stdADDPOLY::stdADDPOLY(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdADDPOLY::execute() {
   word     la = getWordValue();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 3) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(la));
      real DBscale = Properties->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         telldata::ttlayout* ply = new telldata::ttlayout(ATDB->addpoly(la,t2tpoints(pl,DBscale)), la);
      DATC->unlockDB();
      OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
      LogFile << LogFile.getFN() << "("<< *pl << "," << la << ");"; 
      LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 4 points expected to create a polygon");
      OPstack.push(new telldata::ttlayout());
   }
   delete pl;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDPOLY_D::stdADDPOLY_D(telldata::typeID retype) :
                               stdADDPOLY(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
}

int tellstdfunc::stdADDPOLY_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDPOLY::execute();
}

//=============================================================================
tellstdfunc::stdDRAWPOLY::stdDRAWPOLY(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdDRAWPOLY::execute() {
   word     la = getWordValue();
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(-1, &OPstack)) return EXEC_RETURN;
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 3) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(la));
      real DBscale = Properties->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         telldata::ttlayout* ply = new telldata::ttlayout(ATDB->addpoly(la,t2tpoints(pl,DBscale)), la);
      DATC->unlockDB();
      OPstack.push(ply); UNDOPstack.push_front(ply->selfcopy());
      LogFile << "addpoly("<< *pl << "," << la << ");"; LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"At least 4 points expected to create a polygon");
      OPstack.push(new telldata::ttlayout());
   }
   delete pl;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWPOLY_D::stdDRAWPOLY_D(telldata::typeID retype) :
                              stdDRAWPOLY(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdDRAWPOLY_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWPOLY::execute();
}

//=============================================================================
tellstdfunc::stdADDWIRE::stdADDWIRE(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdADDWIRE::execute() {
   word     la = getWordValue();
   real      w = getOpValue();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 1) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(la));
      real DBscale = Properties->DBscale();
      laydata::tdtdesign* ATDB = DATC->lockDB();
         telldata::ttlayout* wr = new telldata::ttlayout(ATDB->addwire(la,t2tpoints(pl,DBscale),
                                    static_cast<word>(rint(w * DBscale))), la);
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
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdADDWIRE_D::stdADDWIRE_D(telldata::typeID retype) :
                               stdADDWIRE(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_pnt)));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdADDWIRE_D::execute() {
   OPstack.push(CurrentLayer());
   return stdADDWIRE::execute();
}

//=============================================================================
tellstdfunc::stdDRAWWIRE::stdDRAWWIRE(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdDRAWWIRE::execute() {
   word     la = getWordValue();
   real      w = getOpValue();
   real DBscale = Properties->DBscale();
   if (!tellstdfunc::waitGUInput(static_cast<int>(rint(w * DBscale)), &OPstack))
      return EXEC_RETURN;
   // get the data from the stack
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   if (pl->size() > 1) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(la));
      laydata::tdtdesign* ATDB = DATC->lockDB();
         telldata::ttlayout* wr = new telldata::ttlayout(ATDB->addwire(la,t2tpoints(pl,DBscale),
                                    static_cast<word>(rint(w * DBscale))), la);
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
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDRAWWIRE_D::stdDRAWWIRE_D(telldata::typeID retype) :
                              stdDRAWWIRE(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

int tellstdfunc::stdDRAWWIRE_D::execute() {
   OPstack.push(CurrentLayer());
   return stdDRAWWIRE::execute();
}

//=============================================================================
tellstdfunc::stdADDTEXT::stdADDTEXT(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();
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
   real DBscale = Properties->DBscale();
   CTM ori(TP(rpnt->x(), rpnt->y(), DBscale), 
                                     magn*DBscale/OPENGL_FONT_UNIT,angle,flip);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      telldata::ttlayout* tx = new telldata::ttlayout(ATDB->addtext(la, text, ori), la);
   DATC->unlockDB();
   OPstack.push(tx);UNDOPstack.push_front(tx->selfcopy());
   LogFile << LogFile.getFN() << "(\"" << text << "\"," << la << "," << *rpnt <<
           "," << angle << "," << flip << "," << magn << ");";LogFile.flush();
   delete rpnt;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGRIDDEF::stdGRIDDEF(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdGRIDDEF::undo_cleanup() {
}

void tellstdfunc::stdGRIDDEF::undo() {
}

int tellstdfunc::stdGRIDDEF::execute() {
   std::string  colname = getStringValue();
   real    step    = getOpValue();
   byte    no      = getByteValue();
   Properties->setGrid(no,step,colname);
   UpdateLV();   
   LogFile << LogFile.getFN() << "(" << no << "," << step << ",\"" << 
                                              colname << "\");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGRID::stdGRID(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdGRID::undo_cleanup() {
   getByteValue(UNDOPstack, false);
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdGRID::undo() {
   TEUNDO_DEBUG("grid( int bool ) UNDO");
   bool  visu     = getBoolValue(UNDOPstack, true);
   byte    no     = getByteValue(UNDOPstack, true);
   gridON(no,visu);
   UpdateLV();   
}

int tellstdfunc::stdGRID::execute() {
   UNDOcmdQ.push_front(this);
   bool  visu     = getBoolValue();
   byte    no     = getByteValue();
   UNDOPstack.push_front(new telldata::ttint(no));
   UNDOPstack.push_front(new telldata::ttbool(Properties->grid(no)->visual()));
   gridON(no,visu);
   UpdateLV();   
   LogFile << LogFile.getFN() << "(" << no << "," << LogFile._2bool(visu) << ");";
   LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSTEP::stdSTEP(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

void tellstdfunc::stdSTEP::undo_cleanup() {
   getOpValue(UNDOPstack, false);
}

void tellstdfunc::stdSTEP::undo() {
   TEUNDO_DEBUG("step() UNDO");
   Properties->setstep(getOpValue(UNDOPstack,true));
}

int tellstdfunc::stdSTEP::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(new telldata::ttreal(Properties->step()));
   //
   real    step    = getOpValue();
   Properties->setstep(step);
   LogFile << LogFile.getFN() << "(" << step << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdAUTOPAN::stdAUTOPAN(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

void tellstdfunc::stdAUTOPAN::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdAUTOPAN::undo() {
   TEUNDO_DEBUG("autopan() UNDO");
   bool autop = getBoolValue(UNDOPstack, true);
   Properties->setautopan(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   eventGRIDUPD.SetInt(autop ? tui::STS_AUTOPAN_ON : tui::STS_AUTOPAN_OFF);
   wxPostEvent(Toped, eventGRIDUPD);

}

int tellstdfunc::stdAUTOPAN::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(new telldata::ttbool(Properties->autopan()));
   //
   bool autop    = getBoolValue();
   Properties->setautopan(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   eventGRIDUPD.SetInt(autop ? tui::STS_AUTOPAN_ON : tui::STS_AUTOPAN_OFF);
   wxPostEvent(Toped, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << autop << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSHAPEANGLE::stdSHAPEANGLE(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttint()));
}

void tellstdfunc::stdSHAPEANGLE::undo_cleanup() {
   getByteValue(UNDOPstack,false);
}

void tellstdfunc::stdSHAPEANGLE::undo() {
   TEUNDO_DEBUG("shapeangle() UNDO");
   byte angle    = getByteValue(UNDOPstack,true);
   Properties->setmarker_angle(angle);
}

int tellstdfunc::stdSHAPEANGLE::execute() {
   byte angle    = getByteValue();
   if ((angle == 0) || (angle == 45) || (angle == 90)) {
      // prepare undo first
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(new telldata::ttint(Properties->marker_angle()));
      //
      Properties->setmarker_angle(angle);
      wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
      if       (angle == 0)  eventGRIDUPD.SetInt(tui::STS_ANGLE_0);
      else if  (angle == 45) eventGRIDUPD.SetInt(tui::STS_ANGLE_45);
      else if  (angle == 90) eventGRIDUPD.SetInt(tui::STS_ANGLE_90);
      else assert(false);
      wxPostEvent(Toped, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << angle << ");"; LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"0, 45 or 90 degrees allowed only");
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::getPOINT::getPOINT(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::getPOINT::execute() {
   // Here - try a hollow lock/unlock the database just to check that it exists
   // The use of this function should be deprecated
   DATC->lockDB();
   DATC->unlockDB();
   // flag the prompt that we expect a single point & handle a pointer to
   // the operand stack
   Toped->cmdline()->waitGUInput(&OPstack, telldata::tn_pnt);
   // force the thread in wait condition until the ted_prompt has our data
   Toped->cmdline()->threadWaits4->Wait();
   // ... and continue when the thread is woken up
   if (Toped->cmdline()->mouseIN_OK())  return EXEC_NEXT;
   else return EXEC_RETURN;
}

//=============================================================================
tellstdfunc::getPOINTLIST::getPOINTLIST(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::getPOINTLIST::execute() {
   // Here - try a hollow lock/unlock the database just to check that it exists
   // The use of this function should be deprecated
   DATC->lockDB();
   DATC->unlockDB();
   // flag the prompt that we expect a list of points & handle a pointer to
   // the operand stack
   Toped->cmdline()->waitGUInput(&OPstack, TLISTOF(telldata::tn_pnt));
   // 
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(0);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(Toped->view(), eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Toped->cmdline()->threadWaits4->Wait();
   // ... and continue when the thread is woken up
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(Toped->view(), eventMOUSEIN);
   if (Toped->cmdline()->mouseIN_OK())  return EXEC_NEXT;
   else return EXEC_RETURN;
}

//=============================================================================
tellstdfunc::stdSELECT::stdSELECT(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdSELECT::undo() {
   TEUNDO_DEBUG("select(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = Properties->DBscale();
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
   real DBscale = Properties->DBscale();
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
tellstdfunc::stdSELECT_I::stdSELECT_I(telldata::typeID retype) :
                                stdSELECT(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(0, &OPstack)) return EXEC_RETURN;
   else return stdSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECT_TL::stdSELECT_TL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
tellstdfunc::stdSELECTIN::stdSELECTIN(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   real DBscale = Properties->DBscale();
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
tellstdfunc::stdPNTSELECT::stdPNTSELECT(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdPNTSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTSELECT::undo() {
   TEUNDO_DEBUG("pselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = Properties->DBscale();
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
   real DBscale = Properties->DBscale();
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
tellstdfunc::stdPNTSELECT_I::stdPNTSELECT_I(telldata::typeID retype) :
                             stdPNTSELECT(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdPNTSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(0, &OPstack)) return EXEC_RETURN;
   return stdPNTSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT::stdUNSELECT(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdUNSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdUNSELECT::undo() {
   TEUNDO_DEBUG("unselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = Properties->DBscale();
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
   real DBscale = Properties->DBscale();
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
tellstdfunc::stdUNSELECT_I::stdUNSELECT_I(telldata::typeID retype) :
                              stdUNSELECT(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(0, &OPstack)) return EXEC_RETURN;
   else return stdUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT_TL::stdUNSELECT_TL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
tellstdfunc::stdUNSELECTIN::stdUNSELECTIN(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   real DBscale = Properties->DBscale();
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
tellstdfunc::stdPNTUNSELECT::stdPNTUNSELECT(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttwnd()));
}

void tellstdfunc::stdPNTUNSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTUNSELECT::undo() {
   TEUNDO_DEBUG("punselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = Properties->DBscale();
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
   real DBscale = Properties->DBscale();
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
tellstdfunc::stdPNTUNSELECT_I::stdPNTUNSELECT_I(telldata::typeID retype) :
                           stdPNTUNSELECT(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdPNTUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(0, &OPstack)) return EXEC_RETURN;
   else return stdPNTUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECTALL::stdSELECTALL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

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
tellstdfunc::stdUNSELECTALL::stdUNSELECTALL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

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
tellstdfunc::stdDELETESEL::stdDELETESEL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

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
tellstdfunc::stdCOPYSEL::stdCOPYSEL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   UpdateLV();   
}

int tellstdfunc::stdCOPYSEL::execute() {
   UNDOcmdQ.push_front(this);
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
      ATDB->copy_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale));
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();   
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   delete p1; delete p2;
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOPYSEL_D::stdCOPYSEL_D(telldata::typeID retype) :
                               stdCOPYSEL(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdCOPYSEL_D::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(-3, &OPstack)) return EXEC_RETURN;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   OPstack.push(new telldata::ttpnt(w->p1().x(), w->p1().y()));
   OPstack.push(new telldata::ttpnt(w->p2().x(), w->p2().y()));
   delete w;
   return stdCOPYSEL::execute();
}

//=============================================================================
tellstdfunc::stdMOVESEL::stdMOVESEL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
   
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(failed));
      ATDB->unselect_fromList(get_ttlaylist(added));
      laydata::selectList* fadead[3];
	  byte i;
      for (i = 0; i < 3; fadead[i++] = new laydata::selectList());
      ATDB->move_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale),fadead);
      //SGREM Here - an internal check can be done - all 3 of the fadead lists
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
   UpdateLV();   
}

int tellstdfunc::stdMOVESEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
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
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdMOVESEL_D::stdMOVESEL_D(telldata::typeID retype) :
                               stdMOVESEL(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::stdMOVESEL_D::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(-2, &OPstack)) return EXEC_RETURN;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   OPstack.push(new telldata::ttpnt(w->p1().x(), w->p1().y()));
   OPstack.push(new telldata::ttpnt(w->p2().x(), w->p2().y()));
   delete w;
   return stdMOVESEL::execute();
}

//=============================================================================
tellstdfunc::stdROTATESEL::stdROTATESEL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
   arguments->push_back(new argumentTYPE("", new telldata::ttreal()));
}

void tellstdfunc::stdROTATESEL::undo_cleanup() {
   getOpValue(UNDOPstack, false);
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
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
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real   angle  = 360 - getOpValue(UNDOPstack, true);
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(failed));
      ATDB->unselect_fromList(get_ttlaylist(added));
      laydata::selectList* fadead[3];
      byte i;
      for (i = 0; i < 3; fadead[i++] = new laydata::selectList());
      ATDB->rotate_selected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      //SGREM Here - an internal check can be done - all 3 of the fadead lists
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
   UpdateLV();
}

int tellstdfunc::stdROTATESEL::execute() {
   UNDOcmdQ.push_front(this);
   real   angle  = getOpValue();
   UNDOPstack.push_front(new telldata::ttreal(angle));
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
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
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPXSEL::stdFLIPXSEL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdFLIPXSEL::undo_cleanup() {
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete p1;
}

void tellstdfunc::stdFLIPXSEL::undo() {
   TEUNDO_DEBUG("flipX(point) UNDO");
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), true);
   DATC->unlockDB();
   delete p1; 
   UpdateLV();   
}

int tellstdfunc::stdFLIPXSEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), true);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPYSEL::stdFLIPYSEL(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttpnt()));
}

void tellstdfunc::stdFLIPYSEL::undo_cleanup() {
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete p1;
}

void tellstdfunc::stdFLIPYSEL::undo() {
   TEUNDO_DEBUG("flipY(point) UNDO");
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), false);
   DATC->unlockDB();
   delete p1; 
   UpdateLV();   
}

int tellstdfunc::stdFLIPYSEL::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = Properties->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), false);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGROUP::stdGROUP(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

void tellstdfunc::stdGROUP::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdGROUP::undo() {
   TEUNDO_DEBUG("group(string) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->select_fromList(get_ttlaylist(pl));
      ATDB->ungroup_this(ATDB->ungroup_prep());
   DATC->unlockDB();
   delete pl;
//   SGREM HERE!!! Delete the cell (not just the reference!) Wait for ATDB->remove_cell()
   UpdateLV();   
}

int tellstdfunc::stdGROUP::execute() {
   std::string name = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      if (ATDB->group_selected(name)) {
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
/*-!-*/  DATC->unlockDB();
         LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
         UpdateLV();   
      }
      else
/*-!-*/  DATC->unlockDB();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNGROUP::stdUNGROUP(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

void tellstdfunc::stdUNGROUP::undo_cleanup() {
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
   delete pl1;
}

void tellstdfunc::stdUNGROUP::undo() {
   TEUNDO_DEBUG("ungroup() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // first save the list of all currently selected components
      laydata::selectList *savelist = ATDB->copy_selist();
      // now unselect all
      ATDB->unselect_all();
      // get the list of shapes produced by the ungroup from the UNDO stack
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL);
      // now get the list of the ungroupped cell ref's from the UNDO stack
      telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // and add them to the target cell
      ATDB->addlist(get_shlaylist(pl1)); 
      // select the restored cell refs
      ATDB->select_fromList(get_ttlaylist(pl1)); 
      // now restore selection
      ATDB->select_fromList(savelist);
      // and add the list of restored cells to the selection
      ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   // finally - clean-up behind
   delete pl;
   delete pl1;
   UpdateLV();   
}
   
int tellstdfunc::stdUNGROUP::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::shapeList* cells4u = ATDB->ungroup_prep();
   DATC->unlockDB();
   if (cells4u->empty()) {
      tell_log(console::MT_ERROR,"Nothing to ungroup");
      delete cells4u;
   }
   else {
      laydata::atticList undol;
      UNDOcmdQ.push_front(this);
      // Push the list of the cells to be ungroupped first
      undol[0] = cells4u;
      UNDOPstack.push_front(make_ttlaylist(&undol));
      ATDB = DATC->lockDB();
         // and then ungroup and push the list of the shapes produced in 
         //result of the ungroup
         laydata::atticList* undol2 = ATDB->ungroup_this(cells4u);
      DATC->unlockDB();
      UNDOPstack.push_front(make_ttlaylist(undol2));
      delete cells4u;
      for(laydata::atticList::iterator CL = undol2->begin(); CL != undol2->end(); CL++)
         delete CL->second;
      delete undol2;
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
      UpdateLV();
   }   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY::lgcCUTPOLY(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
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
      real DBscale = Properties->DBscale();
      pointlist plist = t2tpoints(pl,DBscale);
      laydata::valid_poly check(plist);
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
               // select the shapes to delete & delete them ...
               ATDB->select_fromList(get_ttlaylist(make_ttlaylist(dasao[0])));
               laydata::atticList* sh_delist = new laydata::atticList();
               ATDB->delete_selected(sh_delist);
               // ... not forgetting to save them in the undo data stack for undo
               UNDOPstack.push_front(make_ttlaylist(sh_delist));
               // clean-up the delete attic list
               delete dasao[0];
               // add the result of the cut...
               ATDB->addlist(dasao[1]);
               UNDOPstack.push_front(make_ttlaylist(dasao[1]));
               // ... the cut-offs ....
               ATDB->addlist(dasao[2]);
               UNDOPstack.push_front(make_ttlaylist(dasao[2]));
               // and finally select the_cut
               ATDB->select_fromList(get_ttlaylist(make_ttlaylist(dasao[1])));
               LogFile << "polycut("<< *pl << ");"; LogFile.flush();
            }
         DATC->unlockDB();
      }
      delete pl;
      UpdateLV();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY_I::lgcCUTPOLY_I(telldata::typeID retype) :
                               lgcCUTPOLY(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::lgcCUTPOLY_I::execute() {
   if (DATC->numselected() == 0) {
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(-1, &OPstack)) return EXEC_RETURN;
   return lgcCUTPOLY::execute();
}

//=============================================================================
tellstdfunc::lgcMERGE::lgcMERGE(telldata::typeID retype) :
                               cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

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
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
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
tellstdfunc::GDSread::GDSread(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSread::execute() {
   std::string name = getStringValue();
   std::list<std::string> top_cell_list;
   DATC->GDSparse(name, top_cell_list);
   telldata::ttlist* topcells = new telldata::ttlist(telldata::tn_string);
   for (std::list<std::string>::const_iterator CN = top_cell_list.begin();
                                              CN != top_cell_list.end(); CN ++)
      topcells->add(new telldata::ttstring(*CN));
   OPstack.push(topcells);
   LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTread::TDTread(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::TDTread::execute() {
   std::string name = getStringValue();
   if (DATC->TDTread(name)) {
      time_t ftime = DATC->tedtimestamp();
      std::string time(ctime(&ftime));
      time.erase(time.length()-1,1); // remove the trailing \n
      LogFile << LogFile.getFN() << "(\""<< name << "\",\"" <<  time <<
                                                        "\");"; LogFile.flush();
      // reset UNDO buffers;
      UNDOcmdQ.clear();
      while (!UNDOPstack.empty()) {
         delete UNDOPstack.front(); UNDOPstack.pop_front();
      }   
   }                                                           
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTsaveas::TDTsaveas(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::TDTsaveas::execute() {
   std::string name = getStringValue();
   DATC->TDTwrite(name.c_str());
   LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTsave::TDTsave(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::TDTsave::execute() {
   DATC->TDTwrite();
   time_t ftime = DATC->tedtimestamp();
   std::string time(ctime(&ftime));
   time.erase(time.length()-1,1); // remove the trailing \n
   LogFile << LogFile.getFN() << "(\"" <<  time << "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSconvert::GDSconvert(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::GDSconvert::execute() {
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   std::string name = getStringValue();
   DATC->lockDB(false);
      DATC->importGDScell(name.c_str(), recur, over);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << recur << "," << 
                                                  over << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSexportLIB::GDSexportLIB(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSexportLIB::execute()
{
   std::string filename = getStringValue();
   DATC->lockDB(false);
      DATC->GDSexport(filename);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "(\""<< filename << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSexportTOP::GDSexportTOP(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSexportTOP::execute()
{
   std::string filename = getStringValue();
   bool  recur = getBoolValue();
   std::string cellname = getStringValue();
   laydata::tdtcell *excell = NULL;
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      excell = ATDB->checkcell(cellname);
      if (NULL != excell)
         DATC->GDSexport(excell, recur, filename);
   DATC->unlockDB();
   if (NULL != excell)
   {
      LogFile << LogFile.getFN() << "(\""<< cellname << "\"," << recur << ",\"" << filename << "\");";
      LogFile.flush();
   }
   else
   {
      std::string message = "Cell " + cellname + " not found in the database";
      tell_log(console::MT_ERROR,message.c_str());
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSreportlay::GDSreportlay(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSreportlay::execute() {
   std::string name = getStringValue();
   DATC->reportGDSlay(name.c_str());
   LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSclose::GDSclose(telldata::typeID retype) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype) {
}

int tellstdfunc::GDSclose::execute() {
   DATC->GDSclose();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
telldata::ttint* tellstdfunc::CurrentLayer() {
   word cl = 0;
   cl = Properties->curlay();
   return (new telldata::ttint(cl));
}


/* Maybe the only place to describe the input_type parameter:
   input_type > 0 - wire where input type is the width. List of points expected.
                    Rubber band wire will be shown.
   input_type = 0 - box. Two points expected, A rubberband box will be shown
   input_type = -1 - polygon. List of points expected. Rubber band polygon will 
                     be shown
   input_type = -2 - move. Two points expected. Selected and partially selected 
                     objects will be moved on the screen with the marker
   input type = -3 - copy. Two points expected. Fully selected shapes will be
                     moved on the screen with the marker.
*/
bool tellstdfunc::waitGUInput(int input_type, telldata::operandSTACK *OPstack) {
   // Create a temporary object in the tdtdesign
   try {DATC->mouseStart(input_type);}
   catch (EXPTN) {return false;}   
   // flag the prompt that we expect a list of points & handle a pointer to
   // the operand stack
   if ((input_type == 0) || (input_type < -1))
        Toped->cmdline()->waitGUInput(OPstack, telldata::tn_box);
   else Toped->cmdline()->waitGUInput(OPstack, TLISTOF(telldata::tn_pnt));
   // flag the canvas that a mouse input will be required
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(input_type);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(Toped->view(), eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Toped->cmdline()->threadWaits4->Wait();
   // ... and continue when the thread is woken up
   // Delete the temporary object in the tdtdesign
   DATC->mouseStop();
   // Stop the mouse stream from the canvas
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(Toped->view(), eventMOUSEIN);
   return Toped->cmdline()->mouseIN_OK();
}

pointlist& tellstdfunc::t2tpoints(telldata::ttlist *pl, real DBscale) {
   pointlist *plDB = new pointlist();
   plDB->reserve(pl->size());
   telldata::ttpnt* pt;
   for (unsigned i = 0; i < pl->size(); i++) {
      pt = static_cast<telldata::ttpnt*>((pl->mlist())[i]);
      plDB->push_back(TP(pt->x(), pt->y(), DBscale));
   }
   return *plDB;
}   

telldata::ttlist* tellstdfunc::make_ttlaylist(laydata::selectList* shapesel) {
   telldata::ttlist* llist = new telldata::ttlist(telldata::tn_layout);
   laydata::dataList* lslct;
   SGBitSet* pntl;
   for (laydata::selectList::const_iterator CL = shapesel->begin(); 
                                            CL != shapesel->end(); CL++) {
      lslct = CL->second;
      // push each data reference into the TELL list
      for (laydata::dataList::const_iterator CI = lslct->begin(); 
                                             CI != lslct->end(); CI++) {
         // copy the pointlist, because it will be deleted with the shapesel
         if (CI->second) pntl = new SGBitSet(CI->second);
         else           pntl = NULL;
         llist->add(new telldata::ttlayout(CI->first, CL->first, pntl));
      }   
   }
   return llist;
}

void tellstdfunc::clean_ttlaylist(telldata::ttlist* llist) {
   // Two things to be noted here
   //  - First is kind of strange - data() method of telldata::ttlayout is defined
   // const, yet compiler doesn't complain that it is DELETED here
   //  - Second - the best place to clean-up the lists is of course inside
   // telldata::ttlayout class, however they don't know shit about laydata::tdtdata
   // though with a strange error message compiler claims that destructors will 
   // not be called - and I have no other choise except to belive it.
   // This if course is because of the separation of the project on modules
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      delete (static_cast<telldata::ttlayout*>(llist->mlist()[i])->data());
   }
}

telldata::ttlist* tellstdfunc::make_ttlaylist(laydata::atticList* shapesel) {
   telldata::ttlist* llist = new telldata::ttlist(telldata::tn_layout);
   laydata::shapeList* lslct;
   for (laydata::atticList::const_iterator CL = shapesel->begin(); 
                                            CL != shapesel->end(); CL++) {
      lslct = CL->second;
      // push each data reference into the TELL list
      for (laydata::shapeList::const_iterator CI = lslct->begin(); 
                                             CI != lslct->end(); CI++)
      //   if (sh_deleted == (*CI)->status()) - doesn't seems to need it!
            llist->add(new telldata::ttlayout(*CI, CL->first));
   }
   return llist;
}

laydata::selectList* tellstdfunc::get_ttlaylist(telldata::ttlist* llist) {
   laydata::selectList* shapesel = new laydata::selectList();
   word clayer;
   SGBitSet *pntl, *pntl_o;
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      clayer = static_cast<telldata::ttlayout*>(llist->mlist()[i])->layer();
      if (shapesel->end() == shapesel->find(clayer))
         (*shapesel)[clayer] = new laydata::dataList();
      pntl_o = static_cast<telldata::ttlayout*>(llist->mlist()[i])->selp();
      // copy the pointlist, to preserve the TELL variable contents when
      // Toped select list is deleted
      if (pntl_o) pntl = new SGBitSet(pntl_o);
      else        pntl = NULL;
      (*shapesel)[clayer]->push_back(laydata::selectDataPair(
        static_cast<telldata::ttlayout*>(llist->mlist()[i])->data(), pntl));
   }
   return shapesel;
}

laydata::atticList* tellstdfunc::get_shlaylist(telldata::ttlist* llist) {
   laydata::atticList* shapesel = new laydata::atticList();
   word clayer;
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      clayer = static_cast<telldata::ttlayout*>(llist->mlist()[i])->layer();
      if (shapesel->end() == shapesel->find(clayer))
         (*shapesel)[clayer] = new laydata::shapeList();
      (*shapesel)[clayer]->push_back(static_cast<telldata::ttlayout*>
                                                (llist->mlist()[i])->data());
   }
   return shapesel;
}

void tellstdfunc::UpdateLV() {
   wxString ws;
   ws.sprintf("%d",DATC->numselected());
   wxCommandEvent eventUPDATESEL(wxEVT_CNVSSTATUSLINE);
   eventUPDATESEL.SetInt(tui::STS_SELECTED);
   eventUPDATESEL.SetString(ws);
   wxPostEvent(Toped->view(), eventUPDATESEL);
   RefreshGL();
}

void tellstdfunc::RefreshGL() {
   wxPaintEvent upde(wxEVT_PAINT);
   wxPostEvent(Toped->view(), upde);
}

void tellstdfunc::gridON(byte No, bool status) {
   wxCommandEvent eventGRIDUPD(wxEVT_CNVSSTATUSLINE);
   status = Properties->viewGrid(No, status);
   switch (No) {
      case 0: eventGRIDUPD.SetInt((status ? tui::STS_GRID0_ON : tui::STS_GRID0_OFF)); break;
      case 1: eventGRIDUPD.SetInt((status ? tui::STS_GRID1_ON : tui::STS_GRID1_OFF)); break;
      case 2: eventGRIDUPD.SetInt((status ? tui::STS_GRID2_ON : tui::STS_GRID2_OFF)); break;
      default: assert(false);
   }
   wxPostEvent(Toped->view(), eventGRIDUPD);
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
