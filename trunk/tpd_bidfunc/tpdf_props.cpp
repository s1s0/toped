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
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "tpdf_props.h"
#include "datacenter.h"
#include "tuidefs.h"
#include "viewprop.h"
#include "ted_prompt.h"

extern parsercmd::cmdBLOCK*      CMDBlock;
extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::ted_cmd*         Console;
extern wxWindow*                 TopedCanvasW;
extern wxFrame*                  TopedMainW;
extern console::toped_logfile    LogFile;
extern layprop::FontLibrary*     fontLib;
extern const wxEventType         wxEVT_RENDER_PARAMS;
extern const wxEventType         wxEVT_CANVAS_PARAMS;

//=============================================================================
tellstdfunc::stdPROPSAVE::stdPROPSAVE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdPROPSAVE::execute()
{
   std::string fname = getStringValue();
   PROPC->saveProperties(fname);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->addLayer(name, gdsN, col, fill, sline);
      TpdPost::layer_add(name,gdsN);
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << gdsN << ",\"" <<
            col << "\",\"" << fill <<"\",\"" << sline <<"\");";LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->addLine(name, col, pattern, patscale, width);
      LogFile << LogFile.getFN() << "(\""<< name << "\" , \"" << col << "\","
            << pattern << " , " << patscale << " , " << width << ");";LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->addColor(name, colR, colG, colB, sat);
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << colR << "," <<
                          colG << "," << colB << "," << sat << ");";LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp);
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
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
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
         drawProp->addFill(name, ptrn);
         LogFile << LogFile.getFN() << "(\""<< name << "\"," << *sl << ");";
         LogFile.flush();
      }
      PROPC->unlockDrawProp(drawProp);
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

int tellstdfunc::stdGRIDDEF::execute()
{
   std::string  colname = getStringValue();
   real    step    = getOpValue();
   byte    no      = getByteValue();
   PROPC->setGrid(no,step,colname);

   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   switch (no)
   {
      case 0: eventGRIDUPD.SetId(tui::CPS_GRID0_STEP); break;
      case 1: eventGRIDUPD.SetId(tui::CPS_GRID1_STEP); break;
      case 2: eventGRIDUPD.SetId(tui::CPS_GRID2_STEP); break;
      default: assert(false);
   }
   wxString stepString;
   stepString << step;
   eventGRIDUPD.SetString(stepString);
   wxPostEvent(TopedCanvasW, eventGRIDUPD);

   LogFile << LogFile.getFN() << "(" << no << "," << step << ",\"" <<
                                              colname << "\");";LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSETPARAMETER::stdSETPARAMETER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::tthshstr()));
}

int tellstdfunc::stdSETPARAMETER::execute()
{
   telldata::tthshstr *paramSet = static_cast<telldata::tthshstr*>(OPstack.top());OPstack.pop();
   std::string paramName  = paramSet->key().value();
   std::string paramValue = paramSet->value().value();
   analyzeTopedParameters(paramName, paramValue);
   delete paramSet;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSETPARAMETERS::stdSETPARAMETERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hshstr)));
}

int tellstdfunc::stdSETPARAMETERS::execute()
{
   telldata::ttlist *paramList = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();

   for (unsigned i = 0; i < paramList->size(); i++)
   {
      telldata::tthshstr* paramSet = static_cast<telldata::tthshstr*>((paramList->mlist())[i]);
      std::string paramName  = paramSet->key().value();
      std::string paramValue = paramSet->value().value();
      analyzeTopedParameters(paramName, paramValue);
   }
   delete paramList;
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->hideLayer(layno, hide);
      DWordSet unselable;
      drawProp->allUnselectable(unselable);
      if (pl->size() > 0)
      {
         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();
            tDesign->selectFromList(get_ttlaylist(pl), unselable);
            UpdateLV(tDesign->numSelected());
         }
         else
         {
            //There are selected shapes hence DB cell should be existing and lockable
            assert(false);
         }
         DATC->unlockTDT(dbLibDir);
      }
   }
   PROPC->unlockDrawProp(drawProp);
   delete pl;
   TpdPost::layer_status(tui::BT_LAYER_HIDE, layno, hide);
}

int tellstdfunc::stdHIDELAYER::execute()
{
   bool        hide  = getBoolValue();
   word        layno = getWordValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (layno != drawProp->curLay())
      {
         laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();

            laydata::SelectList *listselected = tDesign->shapeSel();
            if (hide && (listselected->end() != listselected->find(layno)))
            {
               (*todslct)[layno] = DEBUG_NEW laydata::DataList(*((*listselected)[layno]));
               DWordSet unselable;
               drawProp->allUnselectable(unselable);
               tDesign->unselectFromList(copySelectList(todslct), unselable);
            }
            UpdateLV(tDesign->numSelected());
         }
         DATC->unlockTDT(dbLibDir);
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::ttint(layno));
         UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         cleanSelectList(todslct);
         drawProp->hideLayer(layno, hide);
         TpdPost::layer_status(tui::BT_LAYER_HIDE, layno, hide);
         LogFile << LogFile.getFN() << "("<< layno << "," <<
                    LogFile._2bool(hide) << ");"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"Layer above is the current. Can't be hidden");
      }
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
         drawProp->hideLayer(laynumber->value(), hide);
         TpdPost::layer_status(tui::BT_LAYER_HIDE, laynumber->value(), hide);
      }
      DWordSet unselable;
      drawProp->allUnselectable(unselable);
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         tDesign->selectFromList(get_ttlaylist(pl), unselable);
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         //There are selected shapes hence DB cell should be existing and lockable
         assert(false);
      }
      DATC->unlockTDT(dbLibDir);
   }
   delete pl; delete sl;
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdHIDELAYERS::execute()
{
   bool        hide  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   DWordSet unselable;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      telldata::ttlist* undolaylist = DEBUG_NEW telldata::ttlist(telldata::tn_int);
      laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         laydata::SelectList *listselected = tDesign->shapeSel();
         // "preliminary" pass - to collect the selected shapes in the layers, targeted
         // for locking and to issue some warning messages if appropriate
         for (unsigned i = 0; i < sl->size() ; i++)
         {
            telldata::ttint *laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
            if (LAST_EDITABLE_LAYNUM < (unsigned)laynumber->value())
            {
               std::ostringstream info;
               info << "Layer number "<< i <<" out of range ... ignored";
               tell_log(console::MT_WARNING,info.str());
            }
            else if (laynumber->value() == drawProp->curLay())
            {
               tell_log(console::MT_WARNING,"Current layer ... ignored");
            }
            else if (hide ^ drawProp->layerHidden(laynumber->value()))
            {
               if (hide && (listselected->end() != listselected->find(laynumber->value())))
               {
                  (*todslct)[laynumber->value()] = DEBUG_NEW laydata::DataList(*((*listselected)[laynumber->value()]));
               }
               TpdPost::layer_status(tui::BT_LAYER_HIDE, laynumber->value(), hide);
               undolaylist->add(DEBUG_NEW telldata::ttint(*laynumber));
            }
         }
         // Now unselect the shapes in the target layers
         drawProp->allUnselectable(unselable);
         tDesign->unselectFromList(copySelectList(todslct), unselable);
         UpdateLV(tDesign->numSelected());
      }
      DATC->unlockTDT(dbLibDir);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(undolaylist);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
      UNDOPstack.push_front(make_ttlaylist(todslct));
      cleanSelectList(todslct);
      // ... and at last - lock the layers. Here we're using the list collected for undo
      // otherwise we have to either maintain another list or to do again all the checks above
      for (unsigned i = 0; i < undolaylist->size(); i++)
      {
         telldata::ttint *laynumber = static_cast<telldata::ttint*>((undolaylist->mlist())[i]);
         drawProp->hideLayer(laynumber->value(), hide);
      }
      LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                         LogFile._2bool(hide) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp);
   delete sl;
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setCellMarksHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_CELL_MARK);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdHIDECELLMARK::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
      drawProp->setCellMarksHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_CELL_MARK);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setTextMarksHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_TEXT_MARK);
      eventGRIDUPD.SetInt((hide ? 0 : 1));
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdHIDETEXTMARK::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
      drawProp->setTextMarksHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_TEXT_MARK);
      eventGRIDUPD.SetInt((hide ? 0 : 1));
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setCellboxHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_CELL_BOX);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdHIDECELLBOND::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
      drawProp->setCellboxHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_CELL_BOX);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setTextboxHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_TEXT_BOX);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdHIDETEXTBOND::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!hide));
      drawProp->setTextboxHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_TEXT_BOX);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->lockLayer(layno, lock);
      DWordSet unselable;
      drawProp->allUnselectable(unselable);
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         tDesign->selectFromList(get_ttlaylist(pl), unselable);
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         //There are selected shapes hence DB cell should be existing and lockable
         assert(false);
      }
      DATC->unlockTDT(dbLibDir);
   }
   delete pl;
   PROPC->unlockDrawProp(drawProp);
   TpdPost::layer_status(tui::BT_LAYER_LOCK, layno, lock);
}

int tellstdfunc::stdLOCKLAYER::execute()
{
   bool        lock  = getBoolValue();
   word        layno = getWordValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (layno != drawProp->curLay())
      {
         laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();
            laydata::SelectList *listselected = tDesign->shapeSel();
            if (lock && (listselected->end() != listselected->find(layno)))
            {
               (*todslct)[layno] = DEBUG_NEW laydata::DataList(*((*listselected)[layno]));
               DWordSet unselable;
               drawProp->allUnselectable(unselable);
               tDesign->unselectFromList(copySelectList(todslct), unselable);
            }
            UpdateLV(tDesign->numSelected());
         }
         DATC->unlockTDT(dbLibDir);
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::ttint(layno));
         UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!lock));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         cleanSelectList(todslct);
         drawProp->lockLayer(layno, lock);
         TpdPost::layer_status(tui::BT_LAYER_LOCK, layno, lock);
         LogFile << LogFile.getFN() << "("<< layno << "," <<
                    LogFile._2bool(lock) << ");"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"Layer above is the current. Can't be locked.");
      }
   }
   PROPC->unlockDrawProp(drawProp);
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
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
         drawProp->lockLayer(laynumber->value(), lock);
         TpdPost::layer_status(tui::BT_LAYER_LOCK, laynumber->value(), lock);
      }
      DWordSet unselable;
      drawProp->allUnselectable(unselable);
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         tDesign->selectFromList(get_ttlaylist(pl), unselable);
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         //There are selected shapes hence DB cell should be existing and lockable
         assert(false);
      }
      DATC->unlockTDT(dbLibDir);
   }
   delete pl; delete sl;
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdLOCKLAYERS::execute()
{
   bool        lock  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      telldata::ttlist* undolaylist = DEBUG_NEW telldata::ttlist(telldata::tn_int);
      laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         laydata::SelectList *listselected = tDesign->shapeSel();
         // "preliminary" pass - to collect the selected shapes in the layers, targeted
         // for locking and to issue some warning messages if appropriate
         for (unsigned i = 0; i < sl->size() ; i++)
         {
            telldata::ttint *laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
            if (LAST_EDITABLE_LAYNUM < (unsigned)laynumber->value())
            {
               std::ostringstream info;
               info << "Layer number "<< i <<" out of range ... ignored";
               tell_log(console::MT_WARNING,info.str());
            }
            else if (laynumber->value() == drawProp->curLay())
            {
               tell_log(console::MT_WARNING,"Current layer ... ignored");
            }
            else if (lock ^ drawProp->layerLocked(laynumber->value()))
            {
               if (lock && (listselected->end() != listselected->find(laynumber->value())))
                  (*todslct)[laynumber->value()] = DEBUG_NEW laydata::DataList(*((*listselected)[laynumber->value()]));
               TpdPost::layer_status(tui::BT_LAYER_LOCK, laynumber->value(), lock);
               undolaylist->add(DEBUG_NEW telldata::ttint(*laynumber));
            }
         }
         // Now unselect the shapes in the target layers
         DWordSet unselable;
         drawProp->allUnselectable(unselable);
         tDesign->unselectFromList(copySelectList(todslct), unselable);
         UpdateLV(tDesign->numSelected());
      }
      DATC->unlockTDT(dbLibDir);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(undolaylist);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!lock));
      UNDOPstack.push_front(make_ttlaylist(todslct));
      cleanSelectList(todslct);
      // ... and at last - lock the layers. Here we're using the list collected for undo
      // otherwise we have to either maintain another list or to do again all the checks above
      for (unsigned i = 0; i < undolaylist->size(); i++)
      {
         telldata::ttint *laynumber = static_cast<telldata::ttint*>((undolaylist->mlist())[i]);
         drawProp->lockLayer(laynumber->value(), lock);
      }
      LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                         LogFile._2bool(lock) << ");"; LogFile.flush();
   }
   delete sl;
   PROPC->unlockDrawProp(drawProp);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdFILLLAYER::stdFILLLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdFILLLAYER::undo_cleanup() {
   getWordValue(UNDOPstack, false);
   getBoolValue(UNDOPstack, false);
}

void tellstdfunc::stdFILLLAYER::undo() {
   TEUNDO_DEBUG("filllayer( word , bool ) UNDO");
   bool        fill  = getBoolValue(UNDOPstack, true);
   word        layno = getWordValue(UNDOPstack, true);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->fillLayer(layno, fill);
      TpdPost::layer_status(tui::BT_LAYER_FILL, layno, fill);
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdFILLLAYER::execute()
{
   bool        fill  = getBoolValue();
   word        layno = getWordValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(layno));
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!fill));
      drawProp->fillLayer(layno, fill);
      TpdPost::layer_status(tui::BT_LAYER_FILL, layno, fill);
      LogFile << LogFile.getFN() << "("<< layno << "," <<
                 LogFile._2bool(fill) << ");"; LogFile.flush();
      RefreshGL();
   }
   PROPC->unlockDrawProp(drawProp);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdFILLLAYERS::stdFILLLAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_int)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

void tellstdfunc::stdFILLLAYERS::undo_cleanup() {
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   getBoolValue(UNDOPstack, false);
   delete sl;
}

void tellstdfunc::stdFILLLAYERS::undo() {
   TEUNDO_DEBUG("filllayer( int list , bool ) UNDO");
   bool        fill  = getBoolValue(UNDOPstack, true);
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         telldata::ttint* laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
         word lay = laynumber->value();
         drawProp->fillLayer(lay, fill);
         TpdPost::layer_status(tui::BT_LAYER_FILL, lay, fill);
      }
      RefreshGL();
   }
   delete sl;
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdFILLLAYERS::execute()
{
   bool        fill  = getBoolValue();
   telldata::ttlist *sl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         telldata::ttint* laynumber = static_cast<telldata::ttint*>((sl->mlist())[i]);
         word lay = laynumber->value();
         drawProp->fillLayer(lay, fill);
         TpdPost::layer_status(tui::BT_LAYER_FILL, lay, fill);
      }
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(sl);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(!fill));
      LogFile << LogFile.getFN() << "("<< *sl << "," <<
                 LogFile._2bool(fill) << ");"; LogFile.flush();
      RefreshGL();
   }
   delete sl;
   PROPC->unlockDrawProp(drawProp);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSAVELAYSTAT::stdSAVELAYSTAT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdSAVELAYSTAT::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
}

void tellstdfunc::stdSAVELAYSTAT::undo() {
   TEUNDO_DEBUG("savelaystat( string ) UNDO");
   std::string sname  = getStringValue(UNDOPstack, true);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      VERIFY(drawProp->deleteLaysetStatus(sname));
      TpdPost::layers_state(sname, false);
   }
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdSAVELAYSTAT::execute()
{
   std::string   sname  = getStringValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttstring(sname));
      if (!drawProp->saveLaysetStatus(sname))
      {
         std::stringstream info;
         info << "Layer set \"" << sname << "\" was redefined";
         tell_log(console::MT_INFO, info.str());
      }
      else
      {
         TpdPost::layers_state(sname, true);
      }
      LogFile << LogFile.getFN() << "(\""<< sname << "\");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOADLAYSTAT::stdLOADLAYSTAT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdLOADLAYSTAT::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->popBackLayerStatus();
   }
   PROPC->unlockDrawProp(drawProp);
   delete pl;
}

void tellstdfunc::stdLOADLAYSTAT::undo() {
   TEUNDO_DEBUG("loadlaystat( string ) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   std::string sname  = getStringValue(UNDOPstack, true);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->popLayerStatus();
      DWordSet unselable;
      drawProp->allUnselectable(unselable);
      laydata::TdtLibDir* dbLibDir = NULL;
      if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
      {
         laydata::TdtDesign* tDesign = (*dbLibDir)();
         tDesign->selectFromList(get_ttlaylist(pl), unselable);
         UpdateLV(tDesign->numSelected());
      }
      else
      {
         //There are selected shapes hence DB cell should be existing and lockable
         assert(false);
      }
      DATC->unlockTDT(dbLibDir);
   }
   delete pl;
   PROPC->unlockDrawProp(drawProp);
}

int tellstdfunc::stdLOADLAYSTAT::execute()
{
   std::string   sname  = getStringValue();
   WordSet hidel, lockl, filll;
   unsigned activel = 0;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (drawProp->getLaysetStatus(sname, hidel, lockl, filll, activel))
      {
         // the list containing all deselected shapes
         laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
         WordSet hll(hidel); // combined locked and hidden layers
         hll.insert(lockl.begin(), lockl.end());

         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();
            laydata::SelectList *listselected = tDesign->shapeSel();
            // first thing is to pick-up the selected shapes of the layers which
            // will be locked or hidden
            for (WordSet::const_iterator CL = hll.begin(); CL != hll.end(); CL++)
            {
               if (listselected->end() != listselected->find(*CL))
                  (*todslct)[*CL] = DEBUG_NEW laydata::DataList(*((*listselected)[*CL]));
            }
            // Now unselect the shapes in the target layers
            DWordSet unselable;
            drawProp->allUnselectable(unselable);
            tDesign->unselectFromList(copySelectList(todslct), unselable);
            UpdateLV(tDesign->numSelected());
         }
         DATC->unlockTDT(dbLibDir);
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::ttstring(sname));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         cleanSelectList(todslct);
         drawProp->pushLayerStatus();
         drawProp->loadLaysetStatus(sname);
         LogFile << LogFile.getFN() << "(\""<< sname << "\");"; LogFile.flush();
      }
      else
      {
         std::stringstream info;
         info << "Layer set \"" << sname << "\" is not defined";
         tell_log(console::MT_ERROR, info.str());
      }
   }
   PROPC->unlockDrawProp(drawProp);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDELLAYSTAT::stdDELLAYSTAT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdDELLAYSTAT::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::ttlist* undohidel = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* undolockl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* undofilll = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete undofilll;
   delete undolockl;
   delete undohidel;
}

void tellstdfunc::stdDELLAYSTAT::undo() {
   TEUNDO_DEBUG("deletelaystat( string ) UNDO");
   // get the layer lists from the undo stack ...
   word activel = getWordValue(UNDOPstack, true);
   telldata::ttlist* undofilll = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* undolockl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* undohidel = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   // ...get the layer set name from the undo stack ...
   std::string sname  = getStringValue(UNDOPstack, true);
   // ...convert the layer lists
   WordSet filll;
   for (unsigned i = 0; i < undofilll->size() ; i++)
      filll.insert(filll.begin(), (static_cast<telldata::ttint*>((undofilll->mlist())[i]))->value());
   WordSet lockl;
   for (unsigned i = 0; i < undolockl->size() ; i++)
      lockl.insert(lockl.begin(), (static_cast<telldata::ttint*>((undolockl->mlist())[i]))->value());
   WordSet hidel;
   for (unsigned i = 0; i < undohidel->size() ; i++)
      hidel.insert(hidel.begin(), (static_cast<telldata::ttint*>((undohidel->mlist())[i]))->value());
   // ... restore the layer set ...
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->saveLaysetStatus(sname, hidel, lockl, filll, activel);
      TpdPost::layers_state(sname, true);
   }
   PROPC->unlockDrawProp(drawProp);
   // ... and finally - clean-up
   delete undofilll;
   delete undolockl;
   delete undohidel;
}

int tellstdfunc::stdDELLAYSTAT::execute()
{
   std::string   sname  = getStringValue();
   WordSet hidel, lockl, filll;
   unsigned activel = 0;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (drawProp->getLaysetStatus(sname, hidel, lockl, filll, activel))
      {
         VERIFY(drawProp->deleteLaysetStatus(sname));
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::ttstring(sname));
         // Push the layer lists in tell form for undo
         telldata::ttlist* undohidel = DEBUG_NEW telldata::ttlist(telldata::tn_int);
         for (WordSet::const_iterator CL = hidel.begin(); CL != hidel.end(); CL++)
            undohidel->add(DEBUG_NEW telldata::ttint(*CL));
         UNDOPstack.push_front(undohidel);
         telldata::ttlist* undolockl = DEBUG_NEW telldata::ttlist(telldata::tn_int);
         for (WordSet::const_iterator CL = lockl.begin(); CL != lockl.end(); CL++)
            undolockl->add(DEBUG_NEW telldata::ttint(*CL));
         UNDOPstack.push_front(undolockl);
         telldata::ttlist* undofilll = DEBUG_NEW telldata::ttlist(telldata::tn_int);
         for (WordSet::const_iterator CL = filll.begin(); CL != filll.end(); CL++)
            undofilll->add(DEBUG_NEW telldata::ttint(*CL));
         UNDOPstack.push_front(undofilll);
         UNDOPstack.push_front(DEBUG_NEW telldata::ttint(activel));
         TpdPost::layers_state(sname, false);
         LogFile << LogFile.getFN() << "(\""<< sname << "\");"; LogFile.flush();
      }
      else
      {
         std::stringstream info;
         info << "Layer set \"" << sname << "\" is not defined";
         tell_log(console::MT_ERROR, info.str());
      }
   }
   PROPC->unlockDrawProp(drawProp);
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
   if (NULL != PROPC->grid(no))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(no));
      UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(PROPC->gridVisual(no)));
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
   real    step    = getOpValue(UNDOPstack,true);
   PROPC->setStep(step);

   wxString stepStr;
   stepStr << step;
   wxCommandEvent eventMARKERSTEP(wxEVT_CANVAS_PARAMS);
   eventMARKERSTEP.SetId(tui::CPS_MARKER_STEP);
   eventMARKERSTEP.SetString(stepStr);
   wxPostEvent(TopedMainW, eventMARKERSTEP);
}

int tellstdfunc::stdSTEP::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttreal(PROPC->step()));
   //
   real    step    = getOpValue();
   PROPC->setStep(step);

   wxString stepStr;
   stepStr << step;
   wxCommandEvent eventMARKERSTEP(wxEVT_CANVAS_PARAMS);
   eventMARKERSTEP.SetId(tui::CPS_MARKER_STEP);
   eventMARKERSTEP.SetString(stepStr);
   wxPostEvent(TopedMainW, eventMARKERSTEP);

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
   PROPC->setAutoPan(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   eventGRIDUPD.SetId(tui::CPS_AUTOPAN);
   eventGRIDUPD.SetInt(autop ? 1 : 0);
   wxPostEvent(TopedMainW, eventGRIDUPD);

}

int tellstdfunc::stdAUTOPAN::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(PROPC->autopan()));
   //
   bool autop    = getBoolValue();
   PROPC->setAutoPan(autop);
   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   eventGRIDUPD.SetId(tui::CPS_AUTOPAN);
   eventGRIDUPD.SetInt(autop ? 1 : 0);
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
   bool zeroc = getBoolValue(UNDOPstack, true);
   PROPC->setZeroCross(zeroc);
   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   eventGRIDUPD.SetId(tui::CPS_ZERO_CROSS);
   eventGRIDUPD.SetInt(zeroc ? 1 : 0);
   wxPostEvent(TopedMainW, eventGRIDUPD);
   RefreshGL();
}

int tellstdfunc::stdZEROCROSS::execute() {
   // prepare undo first
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(DEBUG_NEW telldata::ttbool(PROPC->autopan()));
   //
   bool zeroc    = getBoolValue();
   PROPC->setZeroCross(zeroc);
   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   eventGRIDUPD.SetId(tui::CPS_ZERO_CROSS);
   eventGRIDUPD.SetInt(zeroc ? 1 : 0);
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
   PROPC->setMarkerAngle(angle);
   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   eventGRIDUPD.SetId(tui::CPS_MARKER_MOTION);
   eventGRIDUPD.SetInt(angle);
   wxPostEvent(TopedMainW, eventGRIDUPD);
}

int tellstdfunc::stdSHAPEANGLE::execute()
{
   byte angle    = getByteValue();
   if ((angle == 0) || (angle == 45) || (angle == 90))
   {
      // prepare undo first
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(PROPC->markerAngle()));
      //
      PROPC->setMarkerAngle(angle);
      wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
      eventGRIDUPD.SetId(tui::CPS_MARKER_MOTION);
      eventGRIDUPD.SetInt(angle);
      wxPostEvent(TopedMainW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << angle << ");"; LogFile.flush();
   }
   else
   {
      tell_log(console::MT_ERROR,"0, 45 or 90 degrees allowed only");
   }
   return EXEC_NEXT;
}


template <class T>
   bool from_string(T& t, const std::string& s,
                                std::ios_base& (*f)(std::ios_base&))
   {
      std::istringstream iss(s);
      return !(iss >> f >> t).fail();
   }

void tellstdfunc::analyzeTopedParameters(std::string name, std::string value)
{
   if      ("UNDO_DEPTH"      == name)
   {
      word val;
      if(from_string<word>(val, value, std::dec))
         CMDBlock->setUndoDepth(val);
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected value is between 0 and 65535";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   else if ("MIN_VISUAL_AREA" == name)
   {
      word val;
      if ((from_string<word>(val, value, std::dec)) && (val < 256))
      {
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            drawProp->setVisualLimit(val);
         }
         PROPC->unlockDrawProp(drawProp);
         // send an event to update the property dialog
         wxCommandEvent eventTextOri(wxEVT_RENDER_PARAMS);
         eventTextOri.SetId(tui::RPS_VISI_LIMIT);
         eventTextOri.SetInt(val);
         wxPostEvent(TopedMainW, eventTextOri);
         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected value is between 0 and 255";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   else if ("ADJUST_TEXT_ORIENTATION" == name)
   {//setparams({"ADJUST_TEXT_ORIENTATION", "true"});
      bool val;
      if (from_string<bool>(val, value, std::boolalpha))
      {
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            drawProp->setAdjustTextOrientation(val);
         }
         PROPC->unlockDrawProp(drawProp);
         // send an event to update the property dialog
         wxCommandEvent eventTextOri(wxEVT_RENDER_PARAMS);
         eventTextOri.SetId(tui::RPS_TEXT_ORI);
         eventTextOri.SetInt(val?1:0);
         wxPostEvent(TopedMainW, eventTextOri);
         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected \"true\" or \"false\"";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   else if ("CELL_DEPTH_ALPHA_EBB" == name)
   {
      word val;
      if ((from_string<word>(val, value, std::dec)) && (val <= 80))
      {
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            drawProp->setCellDepthAlphaEbb(val);
         }
         PROPC->unlockDrawProp(drawProp);
         // send an event to update the property dialog
         wxCommandEvent event(wxEVT_RENDER_PARAMS);
         event.SetId(tui::RPS_CELL_DAB);
         event.SetInt(val);
         wxPostEvent(TopedMainW, event);
         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected value is between 0 and 80";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   else if ("CELL_VIEW_DEPTH" == name)
   {
      word val;
      std::transform(value.begin(), value.end(), // source
                     value.begin(),              // destination
                     ::tolower                 );// operation
      if ("all" == value)
      {
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            drawProp->setCellDepthView(0);
         }
         PROPC->unlockDrawProp(drawProp);
         // send an event to update the property dialog
         wxCommandEvent event(wxEVT_RENDER_PARAMS);
         event.SetId(tui::RPS_CELL_DOV);
         event.SetInt(0);
         wxPostEvent(TopedMainW, event);
         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
      }
      else if ((from_string<word>(val, value, std::dec)) && (val <= 8))
      {
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            drawProp->setCellDepthView(val);
         }
         PROPC->unlockDrawProp(drawProp);
         // send an event to update the property dialog
         wxCommandEvent event(wxEVT_RENDER_PARAMS);
         event.SetId(tui::RPS_CELL_DOV);
         event.SetInt(val);
         wxPostEvent(TopedMainW, event);

         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected \"all\" keyword or value between 1 and 8";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   else if ("SELECT_TEXT_FONT" == name)
   {
      if (fontLib->selectFont(value))
      {
         wxCommandEvent eventLoadFont(wxEVT_RENDER_PARAMS);
         eventLoadFont.SetId(tui::RPS_SLCT_FONT);
         eventLoadFont.SetString(wxString(value.c_str(), wxConvUTF8));
         wxPostEvent(TopedMainW, eventLoadFont);
         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
         //TODO The trouble with font changing on the fly is that the
         // overlapping boxes of the existing texts shall be reevaluated.
         // This means that the database shall be traversed and all text
         // objects shall call fontLib->getStringBounds(...)
      }
      else
      {
         std::ostringstream info;
         info << "Font \"" << value << "\" not loaded";
         tell_log(console::MT_ERROR, info.str());
      }
   }
   else if ("HIGHLIGHT_ON_HOVER" == name)
   {//setparams({"HIGHLIGHT_ON_HOVER", "true"});
      bool val;
      if (from_string<bool>(val, value, std::boolalpha))
      {
         PROPC->setHighlightOnHover(val);
         // send an event to update the property dialog
         wxCommandEvent eventTextOri(wxEVT_CANVAS_PARAMS);
         eventTextOri.SetId(tui::CPS_BOLD_ON_HOVER);
         eventTextOri.SetInt(val?1:0);
         wxPostEvent(TopedMainW, eventTextOri);
         // Request a redraw at the thread exit
         Console->set_canvas_invalid(true);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected \"true\" or \"false\"";
         tell_log(console::MT_ERROR,info.str());
      }
   }

   else if ("GRC_BLINK_FREQ" == name)
   {
      word val;
      if ((from_string<word>(val, value, std::dec)) && (val <= 10))
      {
//         layprop::DrawProperties* drawProp;
//         if (PROPC->lockDrawProp(drawProp))
//         {
//            drawProp->setCellDepthAlphaEbb(val);
//         }
//         PROPC->unlockDrawProp(drawProp);
         // send an event to update the property dialog
         wxCommandEvent event(wxEVT_RENDER_PARAMS);
         event.SetId(tui::RPS_GRC_FREQ);
         event.SetInt(val);
         wxPostEvent(TopedMainW, event);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected value is between 0 and 10";
         tell_log(console::MT_ERROR,info.str());
      }
   }


   else
   {
      std::ostringstream info;
      info << "Unknown parameter name \""<< name <<"\"";
      tell_log(console::MT_ERROR,info.str());
   }
}
