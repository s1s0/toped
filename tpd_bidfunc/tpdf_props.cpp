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
#include "trend.h"
#include "ted_prompt.h"

extern parsercmd::cmdBLOCK*      CMDBlock;
extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::TllCmdLine*      Console;
extern wxWindow*                 TopedCanvasW;
extern wxFrame*                  TopedMainW;
extern console::toped_logfile    LogFile;
extern trend::TrendCenter*       TRENDC;
extern const wxEventType         wxEVT_RENDER_PARAMS;
extern const wxEventType         wxEVT_CANVAS_PARAMS;
//=============================================================================
tellstdfunc::stdPROPSAVE_AUI::stdPROPSAVE_AUI(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdPROPSAVE_AUI::execute()
{
   std::string stdAuiMagicString = getStringValue();
   std::string fname = getStringValue();
   PROPC->saveProperties(fname,stdAuiMagicString);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPROPSAVE::stdPROPSAVE(telldata::typeID retype, bool eor) :
      stdPROPSAVE_AUI(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdPROPSAVE::execute()
{
   telldata::TtString *stdAuiMagicString = DEBUG_NEW telldata::TtString();
   OPstack.push(stdAuiMagicString);
   return stdPROPSAVE_AUI::execute();
}

//=============================================================================
tellstdfunc::stdLAYPROP::stdLAYPROP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayer()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdLAYPROP::execute()
{
   std::string sline = getStringValue();
   std::string fill  = getStringValue();
   std::string col   = getStringValue();
   telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>(OPstack.top());OPstack.pop();
   LayerDef laydef(tlay->value());
   std::string name  = getStringValue();
   // error message - included in the method
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->addLayer(name, laydef, col, fill, sline);
      TpdPost::layer_add(name,laydef);
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << (*tlay) << ",\"" <<
            col << "\",\"" << fill <<"\",\"" << sline <<"\");";LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete tlay;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLAYPROP_T::stdLAYPROP_T(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdLAYPROP_T::execute() {
   std::string sline = getStringValue();
   std::string fill  = getStringValue();
   std::string col   = getStringValue();
   word        layno = getWordValue();
   LayerDef laydef(layno, DEFAULT_DTYPE);
   std::string name  = getStringValue();
   tell_log(console::MT_WARNING, "The function \"layprop(string,int,string,string,string)\" is depreciated.\nPlease use \"layprop(string,layer,string,string,string)\" instead");
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->addLayer(name, laydef, col, fill, sline);
      TpdPost::layer_add(name,laydef);
      LogFile << LogFile.getFN() << "(\""<< name << "\"," << telldata::TtLayer(laydef) << ",\"" <<
            col << "\",\"" << fill <<"\",\"" << sline <<"\");";LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLINEDEF::stdLINEDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
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
      TpdPost::techEditUpdate(console::TEU_LINES);
   }
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOLORDEF::stdCOLORDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
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
      TpdPost::techEditUpdate(console::TEU_COLORS);
   }
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFILLDEF::stdFILLDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_int)));
}

int tellstdfunc::stdFILLDEF::execute() {
   telldata::TtList *sl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
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
         const telldata::memlist dataLst = sl->mlist();
         for (unsigned i = 0; i < sl->size();i++)
         {
            if (telldata::tn_uint == dataLst[i]->get_type())
            {
               telldata::TtUInt* wvar = static_cast<telldata::TtUInt*>(dataLst[i]);
               if ( MAX_BYTE_VALUE < wvar->value())
               {
                  tell_log(console::MT_ERROR,"Value out of range in a pattern definition");
                  ptrn[i] = 0x0;
               }
               else
                  ptrn[i] = wvar->value();
            }
            else if (telldata::tn_int == dataLst[i]->get_type())
            {
               telldata::TtInt* wvar = static_cast<telldata::TtInt*>(dataLst[i]);
               if ( ( wvar->value() < 0 ) || ( MAX_BYTE_VALUE < wvar->value() ) )
               {
                  tell_log(console::MT_ERROR,"Value out of range in a pattern definition");
                  ptrn[i] = 0x0;
               }
               else
                  ptrn[i] = wvar->value();
            }
         }
         // error message - included in the method
         drawProp->addFill(name, ptrn);
         LogFile << LogFile.getFN() << "(\""<< name << "\"," << *sl << ");"; LogFile.flush();
         TpdPost::techEditUpdate(console::TEU_FILLS);
      }
      PROPC->unlockDrawProp(drawProp, true);
   }
   delete sl;
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdGRIDDEF::stdGRIDDEF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
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
      default: assert(false); break;
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtHshStr()));
}

int tellstdfunc::stdSETPARAMETER::execute()
{
   telldata::TtHshStr *paramSet = static_cast<telldata::TtHshStr*>(OPstack.top());OPstack.pop();
   std::string paramName  = paramSet->key().value();
   std::string paramValue = paramSet->value().value();
   analyzeTopedParameters(paramName, paramValue);
   delete paramSet;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSETPARAMETERS::stdSETPARAMETERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_hshstr)));
}

int tellstdfunc::stdSETPARAMETERS::execute()
{
   telldata::TtList *paramList = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();

   for (unsigned i = 0; i < paramList->size(); i++)
   {
      telldata::TtHshStr* paramSet = static_cast<telldata::TtHshStr*>((paramList->mlist())[i]);
      std::string paramName  = paramSet->key().value();
      std::string paramValue = paramSet->value().value();
      analyzeTopedParameters(paramName, paramValue);
   }
   delete paramList;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDELAYER::stdHIDELAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST, retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayer()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

void tellstdfunc::stdHIDELAYER::undo_cleanup()
{
   telldata::TtLayer*  tlay = TELL_UNDOOPS_CLEAN(telldata::TtLayer*);
   getBoolValue(UNDOPstack, false);
   telldata::TtList*   pl   = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete tlay;
   delete pl;
}

void tellstdfunc::stdHIDELAYER::undo() {
   TEUNDO_DEBUG("hidelayer( word , bool ) UNDO");
   telldata::TtList*     pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   bool                hide = getBoolValue(UNDOPstack,true);
   telldata::TtLayer*  tlay = TELL_UNDOOPS_UNDO(telldata::TtLayer*);
   LayerDef laydef(tlay->value());
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->hideLayer(laydef, hide);
      LayerDefSet unselable;
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
   PROPC->unlockDrawProp(drawProp, true);
   delete tlay;
   delete pl;
   TpdPost::layer_status(tui::BT_LAYER_HIDE, laydef, hide);
}

int tellstdfunc::stdHIDELAYER::execute()
{
   bool        hide  = getBoolValue();
   telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>(OPstack.top());OPstack.pop();
   LayerDef laydef(tlay->value());
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (laydef != drawProp->curLay())
      {
         laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();

            laydata::SelectList *listselected = tDesign->shapeSel();
            if (hide && (listselected->end() != listselected->find(laydef)))
            {
               todslct->add(laydef, DEBUG_NEW laydata::DataList(*((*listselected)[laydef])));
               LayerDefSet unselable;
               drawProp->allUnselectable(unselable);
               tDesign->unselectFromList(copySelectList(todslct), unselable);
            }
            UpdateLV(tDesign->numSelected());
         }
         DATC->unlockTDT(dbLibDir);
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(tlay->selfcopy());
         UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!hide));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         cleanSelectList(todslct);
         drawProp->hideLayer(laydef, hide);
         TpdPost::layer_status(tui::BT_LAYER_HIDE, laydef, hide);
         LogFile << LogFile.getFN() << "("<< *tlay << "," <<
                    LogFile._2bool(hide) << ");"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"Layer above is the current. Can't be hidden");
      }
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete tlay;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDELAYERS::stdHIDELAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{

   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_layer)));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

void tellstdfunc::stdHIDELAYERS::undo_cleanup() {
   telldata::TtList *sl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   getBoolValue(UNDOPstack, false);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete pl; delete sl;
}

void tellstdfunc::stdHIDELAYERS::undo() {
   TEUNDO_DEBUG("hidelayer( int list , bool ) UNDO");
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   bool        hide  = getBoolValue(UNDOPstack,true);
   telldata::TtList *sl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtLayer *tlay;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         tlay = static_cast<telldata::TtLayer*>((sl->mlist())[i]);
         drawProp->hideLayer(tlay->value(), hide);
         TpdPost::layer_status(tui::BT_LAYER_HIDE, tlay->value(), hide);
      }
      LayerDefSet unselable;
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
   PROPC->unlockDrawProp(drawProp, true);
}

int tellstdfunc::stdHIDELAYERS::execute()
{
   bool        hide  = getBoolValue();
   telldata::TtList *sl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   LayerDefSet unselable;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      telldata::TtList* undolaylist = DEBUG_NEW telldata::TtList(telldata::tn_layer);
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
            telldata::TtLayer *tlay = static_cast<telldata::TtLayer*>((sl->mlist())[i]);
            LayerDef laydef(tlay->value());
            if (!laydef.editable())
            {
               std::ostringstream info;
               info << "Layer number "<< laydef <<" out of range ... ignored";
               tell_log(console::MT_WARNING,info.str());
            }
            else if (laydef == drawProp->curLay())
            {
               tell_log(console::MT_WARNING,"Current layer ... ignored");
            }
            else if (hide ^ drawProp->layerHidden(laydef))
            {
               if (hide && (listselected->end() != listselected->find(laydef)))
               {
                  todslct->add(laydef, DEBUG_NEW laydata::DataList(*((*listselected)[laydef])));
               }
               TpdPost::layer_status(tui::BT_LAYER_HIDE, laydef, hide);
               undolaylist->add(DEBUG_NEW telldata::TtLayer(laydef));
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
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!hide));
      UNDOPstack.push_front(make_ttlaylist(todslct));
      cleanSelectList(todslct);
      // ... and at last - lock the layers. Here we're using the list collected for undo
      // otherwise we have to either maintain another list or to do again all the checks above
      for (unsigned i = 0; i < undolaylist->size(); i++)
      {
         telldata::TtLayer *tlay = static_cast<telldata::TtLayer*>((undolaylist->mlist())[i]);
         drawProp->hideLayer(tlay->value(), hide);
      }
      LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                         LogFile._2bool(hide) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete sl;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDECELLMARK::stdHIDECELLMARK(telldata::typeID retype, bool /*eor*/) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,true)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
}

int tellstdfunc::stdHIDECELLMARK::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!hide));
      drawProp->setCellMarksHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_CELL_MARK);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDETEXTMARK::stdHIDETEXTMARK(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
}

int tellstdfunc::stdHIDETEXTMARK::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!hide));
      drawProp->setTextMarksHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_TEXT_MARK);
      eventGRIDUPD.SetInt((hide ? 0 : 1));
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDECELLBOND::stdHIDECELLBOND(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
}

int tellstdfunc::stdHIDECELLBOND::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!hide));
      drawProp->setCellboxHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_CELL_BOX);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdHIDETEXTBOND::stdHIDETEXTBOND(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
}

int tellstdfunc::stdHIDETEXTBOND::execute() {
   bool        hide  = getBoolValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!hide));
      drawProp->setTextboxHidden(hide);
      wxCommandEvent eventGRIDUPD(wxEVT_RENDER_PARAMS);
      eventGRIDUPD.SetId(tui::RPS_TEXT_BOX);
      eventGRIDUPD.SetInt(hide ? 0 : 1);
      wxPostEvent(TopedCanvasW, eventGRIDUPD);
      LogFile << LogFile.getFN() << "(" << LogFile._2bool(hide) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOCKLAYER::stdLOCKLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayer()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

void tellstdfunc::stdLOCKLAYER::undo_cleanup()
{
   telldata::TtLayer*  tlay = TELL_UNDOOPS_CLEAN(telldata::TtLayer*);
   getBoolValue(UNDOPstack, false);
   telldata::TtList*   pl   = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete tlay;
   delete pl;
}

void tellstdfunc::stdLOCKLAYER::undo()
{
   TEUNDO_DEBUG("locklayer( word , bool ) UNDO");
   telldata::TtList*   pl   = TELL_UNDOOPS_UNDO(telldata::TtList*);
   bool        lock  = getBoolValue(UNDOPstack, true);
   telldata::TtLayer*  tlay = TELL_UNDOOPS_UNDO(telldata::TtLayer*);
   LayerDef laydef(tlay->value());
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->lockLayer(laydef, lock);
      LayerDefSet unselable;
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
   delete tlay;
   PROPC->unlockDrawProp(drawProp, true);
   TpdPost::layer_status(tui::BT_LAYER_LOCK, laydef, lock);
}

int tellstdfunc::stdLOCKLAYER::execute()
{
   bool        lock  = getBoolValue();
   telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>(OPstack.top());OPstack.pop();
   LayerDef laydef(tlay->value());
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (laydef != drawProp->curLay())
      {
         laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();
            laydata::SelectList *listselected = tDesign->shapeSel();
            if (lock && (listselected->end() != listselected->find(laydef)))
            {
               todslct->add(laydef, DEBUG_NEW laydata::DataList(*((*listselected)[laydef])));
               LayerDefSet unselable;
               drawProp->allUnselectable(unselable);
               tDesign->unselectFromList(copySelectList(todslct), unselable);
            }
            UpdateLV(tDesign->numSelected());
         }
         DATC->unlockTDT(dbLibDir);
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(tlay->selfcopy());
         UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!lock));
         UNDOPstack.push_front(make_ttlaylist(todslct));
         cleanSelectList(todslct);
         drawProp->lockLayer(laydef, lock);
         TpdPost::layer_status(tui::BT_LAYER_LOCK, laydef, lock);
         LogFile << LogFile.getFN() << "("<< *tlay << "," <<
                    LogFile._2bool(lock) << ");"; LogFile.flush();
      }
      else
      {
         tell_log(console::MT_ERROR,"Layer above is the current. Can't be locked.");
      }
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete tlay;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOCKLAYERS::stdLOCKLAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{

   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_layer)));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

void tellstdfunc::stdLOCKLAYERS::undo_cleanup() {
   telldata::TtList *sl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   getBoolValue(UNDOPstack, false);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete pl; delete sl;
}

void tellstdfunc::stdLOCKLAYERS::undo() {
   TEUNDO_DEBUG("locklayer( int list , bool ) UNDO");
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   bool        lock  = getBoolValue(UNDOPstack,true);
   telldata::TtList *sl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtLayer *tlay;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         tlay = static_cast<telldata::TtLayer*>((sl->mlist())[i]);
         drawProp->lockLayer(tlay->value(), lock);
         TpdPost::layer_status(tui::BT_LAYER_LOCK, tlay->value(), lock);
      }
      LayerDefSet unselable;
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
   PROPC->unlockDrawProp(drawProp, true);
}

int tellstdfunc::stdLOCKLAYERS::execute()
{
   bool        lock  = getBoolValue();
   telldata::TtList *sl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      telldata::TtList* undolaylist = DEBUG_NEW telldata::TtList(telldata::tn_layer);
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
            telldata::TtLayer *tlay = static_cast<telldata::TtLayer*>((sl->mlist())[i]);
            LayerDef laydef(tlay->value());
            if (!laydef.editable())
            {
               std::ostringstream info;
               info << "Layer number "<< laydef <<" out of range ... ignored";
               tell_log(console::MT_WARNING,info.str());
            }
            else if (laydef == drawProp->curLay())
            {
               tell_log(console::MT_WARNING,"Current layer ... ignored");
            }
            else if (lock ^ drawProp->layerLocked(laydef))
            {
               if (lock && (listselected->end() != listselected->find(laydef)))
                  todslct->add(laydef, DEBUG_NEW laydata::DataList(*((*listselected)[laydef])));
               TpdPost::layer_status(tui::BT_LAYER_LOCK, laydef, lock);
               undolaylist->add(DEBUG_NEW telldata::TtLayer(laydef));
            }
         }
         // Now unselect the shapes in the target layers
         LayerDefSet unselable;
         drawProp->allUnselectable(unselable);
         tDesign->unselectFromList(copySelectList(todslct), unselable);
         UpdateLV(tDesign->numSelected());
      }
      DATC->unlockTDT(dbLibDir);
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(undolaylist);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!lock));
      UNDOPstack.push_front(make_ttlaylist(todslct));
      cleanSelectList(todslct);
      // ... and at last - lock the layers. Here we're using the list collected for undo
      // otherwise we have to either maintain another list or to do again all the checks above
      for (unsigned i = 0; i < undolaylist->size(); i++)
      {
         telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>((undolaylist->mlist())[i]);
         drawProp->lockLayer(tlay->value(), lock);
      }
      LogFile << LogFile.getFN() << "("<< *sl << "," <<
                                         LogFile._2bool(lock) << ");"; LogFile.flush();
   }
   delete sl;
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdFILLLAYER::stdFILLLAYER(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtLayer()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

void tellstdfunc::stdFILLLAYER::undo_cleanup() {
   telldata::TtLayer*  tlay = TELL_UNDOOPS_CLEAN(telldata::TtLayer*);
   getBoolValue(UNDOPstack, false);
   delete tlay;
}

void tellstdfunc::stdFILLLAYER::undo() {
   TEUNDO_DEBUG("filllayer( word , bool ) UNDO");
   bool        fill  = getBoolValue(UNDOPstack, true);
   telldata::TtLayer*  tlay = TELL_UNDOOPS_UNDO(telldata::TtLayer*);
   LayerDef laydef(tlay->value());
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->fillLayer(laydef, fill);
      TpdPost::layer_status(tui::BT_LAYER_FILL, laydef, fill);
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete tlay;
   RefreshGL();
}

int tellstdfunc::stdFILLLAYER::execute()
{
   bool        fill  = getBoolValue();
   telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>(OPstack.top());OPstack.pop();
   LayerDef laydef(tlay->value());
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(tlay->selfcopy());
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!fill));
      drawProp->fillLayer(laydef, fill);
      TpdPost::layer_status(tui::BT_LAYER_FILL, laydef, fill);
      LogFile << LogFile.getFN() << "("<< *tlay << "," <<
                 LogFile._2bool(fill) << ");"; LogFile.flush();
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete tlay;
   RefreshGL();
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdFILLLAYERS::stdFILLLAYERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_layer)));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

void tellstdfunc::stdFILLLAYERS::undo_cleanup() {
   telldata::TtList* sl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   getBoolValue(UNDOPstack, false);
   delete sl;
}

void tellstdfunc::stdFILLLAYERS::undo() {
   TEUNDO_DEBUG("filllayer( int list , bool ) UNDO");
   bool        fill  = getBoolValue(UNDOPstack, true);
   telldata::TtList *sl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>((sl->mlist())[i]);
         drawProp->fillLayer(tlay->value(), fill);
         TpdPost::layer_status(tui::BT_LAYER_FILL, tlay->value(), fill);
      }
   }
   delete sl;
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
}

int tellstdfunc::stdFILLLAYERS::execute()
{
   bool        fill  = getBoolValue();
   telldata::TtList *sl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for (unsigned i = 0; i < sl->size() ; i++)
      {
         telldata::TtLayer* tlay = static_cast<telldata::TtLayer*>((sl->mlist())[i]);
         drawProp->fillLayer(tlay->value(), fill);
         TpdPost::layer_status(tui::BT_LAYER_FILL, tlay->value(), fill);
      }
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(sl);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(!fill));
      LogFile << LogFile.getFN() << "("<< *sl << "," <<
                 LogFile._2bool(fill) << ");"; LogFile.flush();
   }
   delete sl;
   PROPC->unlockDrawProp(drawProp, true);
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSAVELAYSTAT::stdSAVELAYSTAT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
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
   PROPC->unlockDrawProp(drawProp, true);
}

int tellstdfunc::stdSAVELAYSTAT::execute()
{
   std::string   sname  = getStringValue();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(DEBUG_NEW telldata::TtString(sname));
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
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLOADLAYSTAT::stdLOADLAYSTAT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdLOADLAYSTAT::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::TtList* pl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->popBackLayerStatus();
   }
   PROPC->unlockDrawProp(drawProp, true);
   delete pl;
}

void tellstdfunc::stdLOADLAYSTAT::undo() {
   TEUNDO_DEBUG("loadlaystat( string ) UNDO");
   telldata::TtList* pl = TELL_UNDOOPS_UNDO(telldata::TtList*);
   std::string sname  = getStringValue(UNDOPstack, true);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->popLayerStatus();
      LayerDefSet unselable;
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
   PROPC->unlockDrawProp(drawProp, true);
}

int tellstdfunc::stdLOADLAYSTAT::execute()
{
   std::string   sname  = getStringValue();
   LayerDefSet hidel, lockl, filll;
   LayerDef activel(TLL_LAY_DEF);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (drawProp->getLaysetStatus(sname, hidel, lockl, filll, activel))
      {
         // the list containing all deselected shapes
         laydata::SelectList *todslct = DEBUG_NEW laydata::SelectList();
         LayerDefSet hll(hidel); // combined locked and hidden layers
         hll.insert(lockl.begin(), lockl.end());

         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();
            laydata::SelectList *listselected = tDesign->shapeSel();
            // first thing is to pick-up the selected shapes of the layers which
            // will be locked or hidden
            for (LayerDefSet::const_iterator CL = hll.begin(); CL != hll.end(); CL++)
            {
               if (listselected->end() != listselected->find(*CL))
                  todslct->add(*CL, DEBUG_NEW laydata::DataList(*((*listselected)[*CL])));
            }
            // Now unselect the shapes in the target layers
            LayerDefSet unselable;
            drawProp->allUnselectable(unselable);
            tDesign->unselectFromList(copySelectList(todslct), unselable);
            UpdateLV(tDesign->numSelected());
         }
         DATC->unlockTDT(dbLibDir);
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtString(sname));
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
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDELLAYSTAT::stdDELLAYSTAT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

void tellstdfunc::stdDELLAYSTAT::undo_cleanup()
{
   getStringValue(UNDOPstack, false);
   telldata::TtList* undohidel = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* undolockl = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   telldata::TtList* undofilll = TELL_UNDOOPS_CLEAN(telldata::TtList*);
   delete undofilll;
   delete undolockl;
   delete undohidel;
}

void tellstdfunc::stdDELLAYSTAT::undo() {
   TEUNDO_DEBUG("deletelaystat( string ) UNDO");
   // get the layer lists from the undo stack ...
   telldata::TtLayer* activetlay = TELL_UNDOOPS_UNDO(telldata::TtLayer*);
   telldata::TtList* undofilll   = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtList* undolockl   = TELL_UNDOOPS_UNDO(telldata::TtList*);
   telldata::TtList* undohidel   = TELL_UNDOOPS_UNDO(telldata::TtList*);
   // ...get the layer set name from the undo stack ...
   std::string sname  = getStringValue(UNDOPstack, true);
   // ...convert the layer lists
   LayerDefSet filll;
   for (unsigned i = 0; i < undofilll->size() ; i++)
      filll.insert(filll.begin(), (static_cast<telldata::TtLayer*>((undofilll->mlist())[i]))->value());
   LayerDefSet lockl;
   for (unsigned i = 0; i < undolockl->size() ; i++)
      lockl.insert(lockl.begin(), (static_cast<telldata::TtLayer*>((undolockl->mlist())[i]))->value());
   LayerDefSet hidel;
   for (unsigned i = 0; i < undohidel->size() ; i++)
      hidel.insert(hidel.begin(), (static_cast<telldata::TtLayer*>((undohidel->mlist())[i]))->value());
   // ... restore the layer set ...
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->saveLaysetStatus(sname, hidel, lockl, filll, activetlay->value());
      TpdPost::layers_state(sname, true);
   }
   PROPC->unlockDrawProp(drawProp, true);
   // ... and finally - clean-up
   delete activetlay;
   delete undofilll;
   delete undolockl;
   delete undohidel;
}

int tellstdfunc::stdDELLAYSTAT::execute()
{
   std::string   sname  = getStringValue();
   LayerDefSet hidel, lockl, filll;
   LayerDef activel(TLL_LAY_DEF);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (drawProp->getLaysetStatus(sname, hidel, lockl, filll, activel))
      {
         VERIFY(drawProp->deleteLaysetStatus(sname));
         UNDOcmdQ.push_front(this);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtString(sname));
         // Push the layer lists in tell form for undo
         telldata::TtList* undohidel = DEBUG_NEW telldata::TtList(telldata::tn_layer);
         for (LayerDefSet::const_iterator CL = hidel.begin(); CL != hidel.end(); CL++)
            undohidel->add(DEBUG_NEW telldata::TtLayer(*CL));
         UNDOPstack.push_front(undohidel);
         telldata::TtList* undolockl = DEBUG_NEW telldata::TtList(telldata::tn_layer);
         for (LayerDefSet::const_iterator CL = lockl.begin(); CL != lockl.end(); CL++)
            undolockl->add(DEBUG_NEW telldata::TtLayer(*CL));
         UNDOPstack.push_front(undolockl);
         telldata::TtList* undofilll = DEBUG_NEW telldata::TtList(telldata::tn_layer);
         for (LayerDefSet::const_iterator CL = filll.begin(); CL != filll.end(); CL++)
            undofilll->add(DEBUG_NEW telldata::TtLayer(*CL));
         UNDOPstack.push_front(undofilll);
         UNDOPstack.push_front(DEBUG_NEW telldata::TtLayer(activel));
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
   PROPC->unlockDrawProp(drawProp, true);
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::stdGRID::stdGRID(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(no));
      UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(PROPC->gridVisual(no)));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtReal()));
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
   UNDOPstack.push_front(DEBUG_NEW telldata::TtReal(PROPC->step()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
   UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(PROPC->autopan()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
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
   UNDOPstack.push_front(DEBUG_NEW telldata::TtBool(PROPC->autopan()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
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
      UNDOPstack.push_front(DEBUG_NEW telldata::TtInt(PROPC->markerAngle()));
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
         PROPC->unlockDrawProp(drawProp, true);
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
         PROPC->unlockDrawProp(drawProp, true);
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
         PROPC->unlockDrawProp(drawProp, true);
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
         PROPC->unlockDrawProp(drawProp, true);
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
         PROPC->unlockDrawProp(drawProp, true);
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
      if (TRENDC->selectFont(value))
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
         // objects shall call TRENDC->getStringBounds(...)
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

   else if ("GRC_BLINK_PERIOD" == name)
   {
      word val;
      if ((from_string<word>(val, value, std::dec)) && (val <= 10))
      {
         // send an event to update the property dialog
         wxCommandEvent event(wxEVT_RENDER_PARAMS);
         event.SetId(tui::RPS_GRC_PERIOD);
         event.SetInt(val);
         wxPostEvent(TopedMainW, event);
         Console->set_canvas_invalid(true);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected value is between 0 and 10";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   else if ("RECOVER_POLY" == name)
   {//setparams({"RECOVER_POLY", "true"});
      bool val;
      if (from_string<bool>(val, value, std::boolalpha))
      {
         DATC->setRecoverPoly(val);
         // send an event to update the property dialog
         wxCommandEvent eventTextOri(wxEVT_CANVAS_PARAMS);
         eventTextOri.SetId(tui::CPS_RECOVER_POLY);
         eventTextOri.SetInt(val?1:0);
         wxPostEvent(TopedMainW, eventTextOri);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected \"true\" or \"false\"";
         tell_log(console::MT_ERROR,info.str());
      }
   }

   else if ("RECOVER_WIRE" == name)
   {//setparams({"RECOVER_POLY", "true"});
      bool val;
      if (from_string<bool>(val, value, std::boolalpha))
      {
         DATC->setRecoverWire(val);
         // send an event to update the property dialog
         wxCommandEvent eventTextOri(wxEVT_CANVAS_PARAMS);
         eventTextOri.SetId(tui::CPS_RECOVER_WIRE);
         eventTextOri.SetInt(val?1:0);
         wxPostEvent(TopedMainW, eventTextOri);
      }
      else
      {
         std::ostringstream info;
         info << "Invalid \""<< name <<"\" value. Expected \"true\" or \"false\"";
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
