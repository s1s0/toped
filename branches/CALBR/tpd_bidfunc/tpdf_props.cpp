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
//        Created: Thu Apr 19 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED property and mode related functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tpdf_props.h"

#include "../tpd_DB/datacenter.h"
#include "../tpd_common/tuidefs.h"
#include "../tpd_DB/browsers.h"

extern DataCenter*               DATC;
extern wxWindow*                 TopedCanvasW;
extern wxFrame*                  TopedMainW;
extern console::toped_logfile    LogFile;
extern const wxEventType         wxEVT_SETINGSMENU;

//=============================================================================
tellstdfunc::stdPROPSAVE::stdPROPSAVE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdPROPSAVE::execute()
{
   std::string fname = getStringValue();
   DATC->saveProperties(fname);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLAYPROP::stdLAYPROP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdLAYPROP::execute() {
   std::string sline = getStringValue();
   std::string fill  = getStringValue();
   std::string col   = getStringValue();
   word        gdsN  = getWordValue();
   std::string name  = getStringValue();
   // error message - included in the method
   DATC->addlayer(name, gdsN, col, fill, sline);
   browsers::layer_add(name,gdsN);
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << gdsN << ",\"" << 
         col << "\",\"" << fill <<"\",\"" << sline <<"\");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLINEDEF::stdLINEDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

int tellstdfunc::stdLINEDEF::execute() {
   byte width       = getByteValue();
   byte patscale    = getByteValue();
   word pattern     = getWordValue();
   std::string col  = getStringValue();
   std::string name = getStringValue();
   DATC->addline(name, col, pattern, patscale, width);
   LogFile << LogFile.getFN() << "(\""<< name << "\" , \"" << col << "\","
         << pattern << " , " << patscale << " , " << width << ");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOLORDEF::stdCOLORDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

int tellstdfunc::stdCOLORDEF::execute() {
   byte         sat  = getByteValue();
   byte         colB = getByteValue();
   byte         colG = getByteValue();
   byte         colR = getByteValue();
   std::string  name = getStringValue();
   // error message - included in the method
   DATC->addcolor(name, colR, colG, colB, sat);
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << colR << "," << 
                       colG << "," << colB << "," << sat << ");";LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFILLDEF::stdFILLDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_int)));
}

int tellstdfunc::stdFILLDEF::execute() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string  name = getStringValue();
   if (sl->size() != 128) {
      tell_log(console::MT_ERROR,"Exactly 128 integers expected in a fill pattern. Ignored...");
   }
   else {
      // declare the array like this because otherwise it'll be wiped
      byte* ptrn = DEBUG_NEW byte[128];
      telldata::ttint *cmpnt;
      for (unsigned i = 0; i < 128; i++) {
         cmpnt = static_cast<telldata::ttint*>((sl->mlist())[i]);
         if (cmpnt->value() > MAX_BYTE_VALUE) {
            tell_log(console::MT_ERROR,"Value out of range in a pattern definition");
         }
         else ptrn[i] = cmpnt->value();
      }
      // error message - included in the method
      DATC->addfill(name, ptrn);
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << *sl << ");";
      LogFile.flush();
   }
   delete sl;
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdGRIDDEF::stdGRIDDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdGRIDDEF::execute() {
   std::string  colname = getStringValue();
   real    step    = getOpValue();
   byte    no      = getByteValue();
   DATC->setGrid(no,step,colname);
   LogFile << LogFile.getFN() << "(" << no << "," << step << ",\"" <<
                                              colname << "\");";LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDELAYER::stdHIDELAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST, retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdHIDELAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   getBoolValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdHIDELAYER::undo() {
   TEUNDO_DEBUG("hidelayer( word , bool ) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   bool        hide  = getBoolValue(UNDOPstack,true);
   word        layno = getWordValue(UNDOPstack,true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
   DATC->hideLayer(layno, hide);
   ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   delete pl;
   browsers::layer_status(browsers::BT_LAYER_HIDE, layno, hide);
   UpdateLV();
}

int tellstdfunc::stdHIDELAYER::execute() {
   bool        hide  = getBoolValue();
   word        layno = getWordValue();
   if (layno != DATC->curlay()) {
      laydata::tdtdesign* ATDB = DATC->lockDB();

      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(layno));
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
      laydata::selectList *listselected = ATDB->shapesel();
      laydata::selectList *todslct = DEBUG_NEW laydata::selectList();
      if (hide && (listselected->end() != listselected->find(layno)))
      {
         (*todslct)[layno] = DEBUG_NEW laydata::dataList(*((*listselected)[layno]));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         ATDB->unselect_fromList(todslct);
      }
      else
      {
         UNDOPstack.push_front(make_ttlaylist(todslct));
         delete todslct;
      }
      DATC->hideLayer(layno, hide);
      DATC->unlockDB();

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
tellstdfunc::stdHIDELAYERS::stdHIDELAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{

   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_int)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdHIDELAYERS::undo_cleanup() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   getBoolValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl; delete sl;
}

void tellstdfunc::stdHIDELAYERS::undo() {
   TEUNDO_DEBUG("hidelayer( int list , bool ) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   bool        hide  = getBoolValue(UNDOPstack,true);
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttint *laynumber;
   laydata::tdtdesign* ATDB = DATC->lockDB();
   for (unsigned i = 0; i < sl->size() ; i++) {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      DATC->hideLayer(laynumber->value(), hide);
      browsers::layer_status(browsers::BT_LAYER_HIDE, laynumber->value(), hide);
   }
   ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   delete pl; delete sl;
   UpdateLV();
}

int tellstdfunc::stdHIDELAYERS::execute()
{
   bool        hide  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   UNDOcmdQ.push_front(this);
   telldata::ttlist* undolaylist = DEBUG_NEW telldata::ttlist(telldata::tn_int);
   telldata::ttint *laynumber;
   laydata::tdtdesign* ATDB = DATC->lockDB();
   laydata::selectList *listselected = ATDB->shapesel();
   laydata::selectList *todslct = DEBUG_NEW laydata::selectList();
   // "preliminary" pass - to collect the selected shapes in the layers, targeted
   // for locking and to issue some warning messages if appropriate
   for (unsigned i = 0; i < sl->size() ; i++)
   {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      if (/*(laynumber->value() > MAX_LAYER_VALUE) ||*/ (laynumber->value() < 1))
      {
         std::ostringstream info;
         info << "Layer number "<< i <<" out of range ... ignored";
         tell_log(console::MT_WARNING,info.str());
      }
      else if (laynumber->value() == DATC->curlay())
      {
         tell_log(console::MT_WARNING,"Current layer ... ignored");
      }
      else if (hide ^ DATC->layerHidden(laynumber->value()))
      {
         if (hide && (listselected->end() != listselected->find(laynumber->value())))
            (*todslct)[laynumber->value()] = DEBUG_NEW laydata::dataList(*((*listselected)[laynumber->value()]));
         browsers::layer_status(browsers::BT_LAYER_HIDE, laynumber->value(), hide);
         undolaylist->add(DEBUG_NEW telldata::ttint(*laynumber));
      }
   }
   UNDOPstack.push_front(undolaylist);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
   UNDOPstack.push_front(make_ttlaylist(todslct));
   // Now unselect the shapes in the taret layers
   ATDB->unselect_fromList(todslct);
   // ... and at last - lock the layers. Here we're using the list collected for undo
   // otherwise we have to either maintain another list or to do agai all the checks above
   for (unsigned i = 0; i < undolaylist->size(); i++)
   {
      telldata::ttint *laynumber = static_cast<telldata::ttint*>((undolaylist->mlist())[i]);
      DATC->hideLayer(laynumber->value(), hide);
   }
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                      LogFile._2bool(hide) << ");"; LogFile.flush();
   delete sl;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDECELLMARK::stdHIDECELLMARK(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,true)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdHIDECELLMARK::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDECELLMARK::undo() {
   TEUNDO_DEBUG("hide_cellmarks( bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   DATC->setcellmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_CELLMARK_OFF : tui::STS_CELLMARK_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   RefreshGL();
}

int tellstdfunc::stdHIDECELLMARK::execute() {
   bool        hide  = getBoolValue();
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
   DATC->setcellmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_CELLMARK_OFF : tui::STS_CELLMARK_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDETEXTMARK::stdHIDETEXTMARK(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdHIDETEXTMARK::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDETEXTMARK::undo() {
   TEUNDO_DEBUG("hide_textmarks( bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   DATC->settextmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_TEXTMARK_OFF : tui::STS_TEXTMARK_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
}

int tellstdfunc::stdHIDETEXTMARK::execute() {
   bool        hide  = getBoolValue();
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
   DATC->settextmarks_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_TEXTMARK_OFF : tui::STS_TEXTMARK_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDECELLBOND::stdHIDECELLBOND(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdHIDECELLBOND::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDECELLBOND::undo() {
   TEUNDO_DEBUG("hide_cellbox( bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   DATC->setcellbox_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_CELLBOX_OFF : tui::STS_CELLBOX_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
}

int tellstdfunc::stdHIDECELLBOND::execute() {
   bool        hide  = getBoolValue();
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
   DATC->setcellbox_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_CELLBOX_OFF : tui::STS_CELLBOX_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDETEXTBOND::stdHIDETEXTBOND(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdHIDETEXTBOND::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdHIDETEXTBOND::undo() {
   TEUNDO_DEBUG("hide_textbox( bool ) UNDO");
   bool        hide  = getBoolValue(UNDOPstack,true);
   DATC->settextbox_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_TEXTBOX_OFF : tui::STS_TEXTBOX_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
}

int tellstdfunc::stdHIDETEXTBOND::execute() {
   bool        hide  = getBoolValue();
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
   DATC->settextbox_hidden(hide);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt((hide ? tui::STS_TEXTBOX_OFF : tui::STS_TEXTBOX_ON));
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOCKLAYER::stdLOCKLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdLOCKLAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   getBoolValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdLOCKLAYER::undo() {
   TEUNDO_DEBUG("locklayer( word , bool ) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   bool        lock  = getBoolValue(UNDOPstack, true);
   word        layno = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
   DATC->lockLayer(layno, lock);
   ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   delete pl;
   browsers::layer_status(browsers::BT_LAYER_LOCK, layno, lock);
   UpdateLV();
}

int tellstdfunc::stdLOCKLAYER::execute()
{
   bool        lock  = getBoolValue();
   word        layno = getWordValue();
   if (layno != DATC->curlay())
   {
      laydata::tdtdesign* ATDB = DATC->lockDB();
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(layno));
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!lock));
      laydata::selectList *listselected = ATDB->shapesel();
      laydata::selectList *todslct = DEBUG_NEW laydata::selectList();
      if (lock && (listselected->end() != listselected->find(layno)))
      {
         (*todslct)[layno] = DEBUG_NEW laydata::dataList(*((*listselected)[layno]));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         ATDB->unselect_fromList(todslct);
      }
      else
      {
         UNDOPstack.push_front(make_ttlaylist(todslct));
         delete todslct;
      }
      DATC->lockLayer(layno, lock);
      DATC->unlockDB();
      browsers::layer_status(browsers::BT_LAYER_LOCK, layno, lock);
      LogFile << LogFile.getFN() << "("<< layno << "," << 
                 LogFile._2bool(lock) << ");"; LogFile.flush();
      UpdateLV();
   }
   else
   {
      tell_log(console::MT_ERROR,"Layer above is the current. Can't be locked.");
   }   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOCKLAYERS::stdLOCKLAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{

   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_int)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdLOCKLAYERS::undo_cleanup() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   getBoolValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl; delete sl;
}

void tellstdfunc::stdLOCKLAYERS::undo() {
   TEUNDO_DEBUG("locklayer( int list , bool ) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   bool        lock  = getBoolValue(UNDOPstack,true);
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttint *laynumber;
   laydata::tdtdesign* ATDB = DATC->lockDB();
   for (unsigned i = 0; i < sl->size() ; i++) {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      DATC->lockLayer(laynumber->value(), lock);
      browsers::layer_status(browsers::BT_LAYER_LOCK, laynumber->value(), lock);
   }
   ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();
   delete pl; delete sl;
   UpdateLV();
}

int tellstdfunc::stdLOCKLAYERS::execute()
{
   bool        lock  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   UNDOcmdQ.push_front(this);
   telldata::ttlist* undolaylist = DEBUG_NEW telldata::ttlist(telldata::tn_int);
   telldata::ttint *laynumber;
   laydata::tdtdesign* ATDB = DATC->lockDB();
   laydata::selectList *listselected = ATDB->shapesel();
   laydata::selectList *todslct = DEBUG_NEW laydata::selectList();
   // "preliminary" pass - to collect the selected shapes in the layers, targeted
   // for locking and to issue some warning messages if appropriate
   for (unsigned i = 0; i < sl->size() ; i++)
   {
      laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
      if (/*(laynumber->value() > MAX_LAYER_VALUE) ||*/ (laynumber->value() < 1))
      {
         std::ostringstream info;
         info << "Layer number "<< i <<" out of range ... ignored";
         tell_log(console::MT_WARNING,info.str());
      }
      else if (laynumber->value() == DATC->curlay())
      {
         tell_log(console::MT_WARNING,"Current layer ... ignored");
      }
      else if (lock ^ DATC->layerLocked(laynumber->value()))
      {
         if (lock && (listselected->end() != listselected->find(laynumber->value())))
            (*todslct)[laynumber->value()] = DEBUG_NEW laydata::dataList(*((*listselected)[laynumber->value()]));
         browsers::layer_status(browsers::BT_LAYER_LOCK, laynumber->value(), lock);
         undolaylist->add(DEBUG_NEW telldata::ttint(*laynumber));
      }
   }
   UNDOPstack.push_front(undolaylist);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!lock));
   UNDOPstack.push_front(make_ttlaylist(todslct));
   // Now unselect the shapes in the taret layers
   ATDB->unselect_fromList(todslct);
   // ... and at last - lock the layers. Here we're using the list collected for undo
   // otherwise we have to either maintain another list or to do agai all the checks above
   for (unsigned i = 0; i < undolaylist->size(); i++)
   {
      telldata::ttint *laynumber = static_cast<telldata::ttint*>((undolaylist->mlist())[i]);
      DATC->lockLayer(laynumber->value(), lock);
   }
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                      LogFile._2bool(lock) << ");"; LogFile.flush();
   delete sl;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdGRID::stdGRID(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
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
   RefreshGL();
}

int tellstdfunc::stdGRID::execute() {
   bool  visu     = getBoolValue();
   byte    no     = getByteValue();
   if (NULL != DATC->grid(no))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(no));
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(DATC->grid_visual(no)));
      gridON(no,visu);
      LogFile << LogFile.getFN() << "(" << no << "," << LogFile._2bool(visu) << ");";
      LogFile.flush();
      RefreshGL();
   }
   else
      tell_log(console::MT_ERROR,"Grid is not defined. Use definegrid(...) first");
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSTEP::stdSTEP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

void tellstdfunc::stdSTEP::undo_cleanup() {
   getOpValue(UNDOPstack, false);
}

void tellstdfunc::stdSTEP::undo() {
   TEUNDO_DEBUG("step() UNDO");
   DATC->setstep(getOpValue(UNDOPstack,true));
}

int tellstdfunc::stdSTEP::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttreal(DATC->step()));
   //
   real    step    = getOpValue();
   DATC->setstep(step);
   LogFile << LogFile.getFN() << "(" << step << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdAUTOPAN::stdAUTOPAN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdAUTOPAN::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdAUTOPAN::undo() {
   TEUNDO_DEBUG("autopan() UNDO");
   bool autop = getBoolValue(UNDOPstack, true);
   DATC->setautopan(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt(autop ? tui::STS_AUTOPAN_ON : tui::STS_AUTOPAN_OFF);
   wxPostEvent(TopedMainW, eventGRIDUPD);

}

int tellstdfunc::stdAUTOPAN::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(DATC->autopan()));
   //
   bool autop    = getBoolValue();
   DATC->setautopan(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt(autop ? tui::STS_AUTOPAN_ON : tui::STS_AUTOPAN_OFF);
   wxPostEvent(TopedMainW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(autop) << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZEROCROSS::stdZEROCROSS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdZEROCROSS::undo_cleanup() {
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdZEROCROSS::undo() {
   TEUNDO_DEBUG("zerocross() UNDO");
   bool autop = getBoolValue(UNDOPstack, true);
   DATC->setZeroCross(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt(autop ? tui::STS_ZEROCROSS_ON : tui::STS_ZEROCROSS_OFF);
   wxPostEvent(TopedMainW, eventGRIDUPD);
   RefreshGL();
}

int tellstdfunc::stdZEROCROSS::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(DATC->autopan()));
   //
   bool zeroc    = getBoolValue();
   DATC->setZeroCross(zeroc);
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   eventGRIDUPD.SetInt(zeroc ? tui::STS_ZEROCROSS_ON : tui::STS_ZEROCROSS_OFF);
   wxPostEvent(TopedMainW, eventGRIDUPD);
   LogFile << LogFile.getFN() << "(" << LogFile._2bool(zeroc) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSHAPEANGLE::stdSHAPEANGLE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdSHAPEANGLE::undo_cleanup() {
   getByteValue(UNDOPstack,false);
}

void tellstdfunc::stdSHAPEANGLE::undo() {
   TEUNDO_DEBUG("shapeangle() UNDO");
   byte angle    = getByteValue(UNDOPstack,true);
   DATC->setmarker_angle(angle);
}

int tellstdfunc::stdSHAPEANGLE::execute() {
   byte angle    = getByteValue();
   if ((angle == 0) || (angle == 45) || (angle == 90)) {
      // prepare undo first
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(DATC->marker_angle()));
      //
      DATC->setmarker_angle(angle);
      wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
      if       (angle == 0)  eventGRIDUPD.SetInt(tui::STS_ANGLE_0);
      else if  (angle == 45) eventGRIDUPD.SetInt(tui::STS_ANGLE_45);
      else if  (angle == 90) eventGRIDUPD.SetInt(tui::STS_ANGLE_90);
      else assert(false);
      wxPostEvent(TopedMainW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << angle << ");"; LogFile.flush();
   }
   else {
      tell_log(console::MT_ERROR,"0, 45 or 90 degrees allowed only");
   }
   return EXEC_NEXT;
}

