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
//        Created: Tue Apr 17 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED build-in functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "tpdf_common.h"
#include "tuidefs.h"
#include "datacenter.h"
#include "ted_prompt.h"
#include "tedat.h"
#include "viewprop.h"


wxFrame*                         TopedMainW;
wxWindow*                        TopedCanvasW;

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::TllCmdLine*      Console;
extern console::toped_logfile    LogFile;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_CANVAS_PARAMS;


//=============================================================================
//! Make sure this function is not called when TDT mutex is locked. Otherwise
//! it will remain locked in case DrawProperties can't be locked
telldata::TtLayer* tellstdfunc::getCurrentLayer()
{
   LayerDef cl(TLL_LAY_DEF);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      cl = drawProp->curLay();
   }
   PROPC->unlockDrawProp(drawProp, true);
   return (DEBUG_NEW telldata::TtLayer(cl));
}

//=============================================================================
//! The whole idea behind this function is to ensure that the target layer is
//! defined in the property database. Otherwise the user will add a shape
//! which will be invisible.
//! Make sure this function is not called when TDT mutex is locked. Otherwise
//! it will remain locked in case DrawProperties can't be locked
void tellstdfunc::secureLayer(const LayerDef& laydef)
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      // check whether it's defined and make a default definition if it isn't
      if (drawProp->addLayer(laydef))
         TpdPost::layer_add(drawProp->getLayerName(laydef), laydef);
   }
   PROPC->unlockDrawProp(drawProp, true);
}

//=============================================================================
LayerDef tellstdfunc::secureLayer()
{
   LayerDef laydef = DATC->curCmdLay();
   secureLayer(laydef);
   return laydef;
}

//=============================================================================
bool tellstdfunc::waitGUInput(int input_type, telldata::operandSTACK *OPstack,
   std::string name, const CTM trans, int4b stepX, int4b stepY, word cols, word rows)
{
   // Create a temporary object in the TdtDesign (only if a DEBUG_NEW object is created, i.e. box,wire,polygon,cell etc.)
   try {DATC->mouseStart(input_type, name, trans, stepX, stepY, cols, rows);}
   catch (EXPTN&) {return false;}
   // flag the prompt what type of data is expected & handle a pointer to
   // the operand stack
   Console->waitGUInput(OPstack, (console::ACTIVE_OP)input_type, trans);
   // flag the canvas that a mouse input will be required
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(input_type);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Console->_threadWaits4->Wait();
   // ... and continue when the thread is woken up
   // Delete the temporary object in the TdtDesign
   DATC->mouseStop();
   // Stop the mouse stream from the canvas
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // and clean-up the state of the temporary ruler
   DATC->switchDrawRuler(false);
   return Console->mouseIN_OK();
}

//=============================================================================
PointVector* tellstdfunc::t2tpoints(telldata::TtList *pl, real DBscale) {
   PointVector *plDB = DEBUG_NEW PointVector();
   plDB->reserve(pl->size());
   telldata::TtPnt* pt;
   for (unsigned i = 0; i < pl->size(); i++) {
      pt = static_cast<telldata::TtPnt*>((pl->mlist())[i]);
      plDB->push_back(TP(pt->x(), pt->y(), DBscale));
   }
   return plDB;
}

//=============================================================================
telldata::TtList* tellstdfunc::make_ttlaylist(laydata::SelectList* shapesel) {
   telldata::TtList* llist = DEBUG_NEW telldata::TtList(telldata::tn_layout);
   laydata::DataList* lslct;
   SGBitSet pntl;
   for (laydata::SelectList::Iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++)
   {
      lslct = *CL;
      // push each data reference into the TELL list
      for (laydata::DataList::const_iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++) {
         // copy the pointlist, because it will be deleted with the shapeSel
         if (0 != CI->second.size()) pntl = SGBitSet(CI->second);
         else                        pntl = SGBitSet();
         llist->add(DEBUG_NEW telldata::TtLayout(CI->first, CL(), DEBUG_NEW SGBitSet(pntl)));
      }
   }
   return llist;
}

//=============================================================================
void tellstdfunc::clean_ttlaylist(telldata::TtList* llist)
{
   // Several things to be noted here
   //  - First is kind of strange - data() method of telldata::TtLayout is defined
   // const, yet compiler doesn't complain that it is DELETED here
   //  - Second - the best place to clean-up the lists is of course inside
   // telldata::TtLayout class, however they don't know anything about laydata::TdtData
   // though with a strange error message compiler claims that destructors will
   // not be called - and I have no other choice but to believe it.
   // This of course is because of the separation of the project on modules
   // The other possibility is to convert the list to (say) AtticList and then
   // to use the corresponding destroyer, but that seem to be much more convoluted
   // This looks weird - true, but is doing the job.
   // - Don't try to delete here selp (selected points). It is deleted naturally
   // by the destructor of the telldata::TtLayout class it doesn't have the visibility
   // problem of laydata::TdtData
   for (word i = 0 ; i < llist->size(); i++)
   {
      switch (llist->get_type())
      {
         case TLISTOF(telldata::tn_layout):
            delete (static_cast<telldata::TtLayout*>(llist->mlist()[i])->data());
            break;
         case TLISTOF(telldata::tn_auxilary):
            delete (static_cast<telldata::TtAuxdata*>(llist->mlist()[i])->data());
            break;
         default: assert(false); break;
      }
   }
}

//==============================================================================
void tellstdfunc::clean_atticlist(laydata::AtticList* nlst, bool destroy)
{
   if (NULL == nlst) return;
   for (laydata::AtticList::Iterator CL = nlst->begin(); CL != nlst->end(); CL++)
   {
      if (destroy)
      {
         for (laydata::ShapeList::const_iterator DI = CL->begin(); DI != CL->end(); DI++)
            delete (*DI);
      }
      CL->clear();
      delete (*CL);
   }
}

//=============================================================================
telldata::TtList* tellstdfunc::make_ttlaylist(laydata::AtticList* shapesel)
{
   telldata::TtList* llist = DEBUG_NEW telldata::TtList(telldata::tn_layout);
   laydata::ShapeList* lslct;
   for (laydata::AtticList::Iterator CL = shapesel->begin(); CL != shapesel->end(); CL++)
   {
      lslct = *CL;
      // push each data reference into the TELL list
      for (laydata::ShapeList::const_iterator CI  = lslct->begin();
                                              CI != lslct->end(); CI++)
      //   if (sh_deleted == (*CI)->status()) - doesn't seems to need it!
            llist->add(DEBUG_NEW telldata::TtLayout(*CI, CL()));
   }
   return llist;
}

telldata::TtList* tellstdfunc::make_ttlaylist(laydata::ShapeList& lslct, const LayerDef& laydef)
{
   telldata::TtList* llist = DEBUG_NEW telldata::TtList(telldata::tn_layout);
   // push each data reference into the TELL list
   for (laydata::ShapeList::const_iterator CI  = lslct.begin();
                                           CI != lslct.end(); CI++)
   //   if (sh_deleted == (*CI)->status()) - doesn't seems to need it!
      llist->add(DEBUG_NEW telldata::TtLayout(*CI, laydef));
   return llist;
}

telldata::TtList* tellstdfunc::make_ttlaylist(auxdata::AuxDataList& lslct, const LayerDef& laydef)
{
   telldata::TtList* llist = DEBUG_NEW telldata::TtList(telldata::tn_auxilary);
   // push each data reference into the TELL list
   for (auxdata::AuxDataList::const_iterator CI  = lslct.begin();
                                             CI != lslct.end(); CI++)
      llist->add(DEBUG_NEW telldata::TtAuxdata(*CI, laydef));
   return llist;
}

//=============================================================================
laydata::SelectList* tellstdfunc::get_ttlaylist(telldata::TtList* llist)
{
   laydata::SelectList* shapesel = DEBUG_NEW laydata::SelectList();
   SGBitSet* pntl_o;
   for (unsigned i = 0 ; i < llist->mlist().size(); i++)
   {
      LayerDef clayer(static_cast<telldata::TtLayout*>(llist->mlist()[i])->layer());
      if (shapesel->end() == shapesel->find(clayer))
         shapesel->add(clayer, DEBUG_NEW laydata::DataList());
      pntl_o = static_cast<telldata::TtLayout*>(llist->mlist()[i])->selp();

      SGBitSet pntl_n;
      if (NULL != pntl_o)  pntl_n = SGBitSet(*pntl_o);
      (*shapesel)[clayer]->push_back(laydata::SelectDataPair(
        static_cast<telldata::TtLayout*>(llist->mlist()[i])->data(), pntl_n));
   }
   return shapesel;
}

//=============================================================================
laydata::AtticList* tellstdfunc::get_shlaylist(telldata::TtList* llist)
{
   laydata::AtticList* shapesel = DEBUG_NEW laydata::AtticList();
   for (unsigned i = 0 ; i < llist->mlist().size(); i++)
   {
      LayerDef clayer(static_cast<telldata::TtLayout*>(llist->mlist()[i])->layer());
      if (shapesel->end() == shapesel->find(clayer))
         shapesel->add(clayer, DEBUG_NEW laydata::ShapeList());
      (*shapesel)[clayer]->push_back(static_cast<telldata::TtLayout*>
                                                (llist->mlist()[i])->data());
   }
   return shapesel;
}

//=============================================================================
auxdata::AuxDataList* tellstdfunc::get_auxdatalist(telldata::TtList* llist, LayerDef& laydef)
{
   auxdata::AuxDataList* shapesel = DEBUG_NEW auxdata::AuxDataList();
   bool layerCheck = false;
   for (unsigned i = 0 ; i < llist->mlist().size(); i++)
   {
      if (layerCheck)
      {
         assert(laydef ==  static_cast<telldata::TtAuxdata*>(llist->mlist()[i])->layer());
      }
      else
      {
         laydef = static_cast<telldata::TtAuxdata*>(llist->mlist()[i])->layer();
         layerCheck = true;
      }
      shapesel->push_back(static_cast<telldata::TtAuxdata*>
                                                (llist->mlist()[i])->data());
   }
   return shapesel;
}

//=============================================================================
laydata::DataList* tellstdfunc::copyDataList(const laydata::DataList* dlist)
{
   laydata::DataList* clist = DEBUG_NEW laydata::DataList();
   for (laydata::DataList::const_iterator CDI = dlist->begin(); CDI != dlist->end(); CDI++)
   {
      clist->push_back(laydata::SelectDataPair(CDI->first, CDI->second));
   }
   return clist;
}

//=============================================================================
laydata::SelectList* tellstdfunc::copySelectList(const laydata::SelectList* dlist)
{
   laydata::SelectList* clist = DEBUG_NEW laydata::SelectList();
   for (laydata::SelectList::Iterator CDI = dlist->begin(); CDI != dlist->end(); CDI++)
   {
      clist->add(CDI(), copyDataList(*CDI));
   }
   return clist;
}

//=============================================================================
void tellstdfunc::cleanSelectList(laydata::SelectList* dlist)
{
   for (laydata::SelectList::Iterator CDI = dlist->begin(); CDI != dlist->end(); CDI++)
   {
      delete *CDI;
   }
   delete dlist;
}

void tellstdfunc::cleanFadeadList(laydata::SelectList** fadead)
{
   for (byte i = 0; i < 3; i++)
   {
      for (laydata::SelectList::Iterator CI = fadead[i]->begin(); CI != fadead[i]->end(); CI++)
      {
         laydata::DataList* sshape = *CI;
         if (1 == i) // deleted list only
         {
            for (laydata::DataList::iterator CCI = sshape->begin(); CCI  != sshape->end(); CCI++)
            {
               if (0 != CCI->second.size()) CCI->second.clear();
            }
         }
         delete sshape;
      }
      delete fadead[i];
   }
}

/** Filters shapeSel using the mask. Returns new list, containing copy of
unfiltered components
*/
laydata::SelectList* tellstdfunc::filter_selist(const laydata::SelectList* shapesel, word mask)
{
   laydata::SelectList* filtered = DEBUG_NEW laydata::SelectList();

   for (laydata::SelectList::Iterator CL = shapesel->begin(); CL != shapesel->end(); CL++)
   {
      laydata::DataList* ssl = DEBUG_NEW laydata::DataList();
      laydata::DataList* lslct = *CL;
      for (laydata::DataList::const_iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      {
         if (mask & (CI->first->lType()))
         {
            SGBitSet pntl;
            if (0 != CI->second.size()) pntl = SGBitSet(CI->second);
            ssl->push_back(laydata::SelectDataPair(CI->first,pntl));
         }
      }
      if    (ssl->empty()) delete ssl;
      else                 filtered->add(CL(), ssl);
   }
   return filtered;
}

//=============================================================================
laydata::AtticList* tellstdfunc::replace_str(laydata::AtticList* shapesel, std::string newstr)
{
   laydata::AtticList* newtextlist = DEBUG_NEW laydata::AtticList();
   for (laydata::AtticList::Iterator CL = shapesel->begin(); CL != shapesel->end(); CL++)
   {
      laydata::ShapeList* lslct  = *CL;
      laydata::ShapeList* newlst = DEBUG_NEW laydata::ShapeList();
      for (laydata::ShapeList::iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      {
         assert(laydata::_lmtext == (*CI)->lType());
         // using build-in copy constructor
         laydata::TdtText* newtxt = DEBUG_NEW laydata::TdtText(*(static_cast<laydata::TdtText*>(*CI)));
         newtxt->replaceStr(newstr);
         newlst->push_back(newtxt);
      }
      newtextlist->add(CL(), newlst);
   }
   return newtextlist;
}

//=============================================================================
void tellstdfunc::UpdateLV(unsigned int numSel)
{
   wxString ws;
   ws.sprintf(wxT("%d"),numSel);
   wxCommandEvent eventUPDATESEL(wxEVT_CANVAS_STATUS);
   eventUPDATESEL.SetInt(tui::CNVS_SELECTED);
   eventUPDATESEL.SetString(ws);
   wxPostEvent(TopedCanvasW, eventUPDATESEL);
   RefreshGL();
}

//=============================================================================
void tellstdfunc::RefreshGL()
{
   if (!PROPC->upLayers().empty())
   {
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         const LayerDefList freshlays = PROPC->upLayers();
         for(LayerDefList::const_iterator CUL = freshlays.begin(); CUL != freshlays.end(); CUL++)
            TpdPost::layer_add(drawProp->getLayerName(*CUL), *CUL);
         PROPC->clearUnpublishedLayers();
      }
      PROPC->unlockDrawProp(drawProp, true);
   }
   Console->set_canvas_invalid(true);
}

//=============================================================================
void tellstdfunc::gridON(byte No, bool status)
{
   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   status = PROPC->viewGrid(No, status);
   switch (No)
   {
      case 0: eventGRIDUPD.SetId(tui::CPS_GRID0_ON); break;
      case 1: eventGRIDUPD.SetId(tui::CPS_GRID1_ON); break;
      case 2: eventGRIDUPD.SetId(tui::CPS_GRID2_ON); break;
      default: assert(false); break;
   }
   eventGRIDUPD.SetInt(status ? 1 : 0);
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
}

//=============================================================================
void tellstdfunc::updateLayerDefinitions(laydata::TdtLibDir* LIBDIR, NameList& top_cells, int /*libID*/)
{
   // get all the layers used in the design and define them using the default definition
   LayerDefList ull;
   for(NameList::const_iterator CTC= top_cells.begin(); CTC != top_cells.end(); CTC++)
      LIBDIR->collectUsedLays(*CTC, true, ull);
   ull.sort(); ull.unique();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      for(LayerDefList::const_iterator CUL = ull.begin(); CUL != ull.end(); CUL++)
      {
         if (REF_LAY_DEF == *CUL) continue;
         if (drawProp->addLayer(*CUL))
            TpdPost::layer_add(drawProp->getLayerName(*CUL), *CUL);
      }
   }
   PROPC->unlockDrawProp(drawProp, false);
}

//=============================================================================
void tellstdfunc::initFuncLib(wxFrame* tpd, wxWindow* cnvs)
{
   TopedMainW = tpd;
   TopedCanvasW = cnvs;
}

//=============================================================================
//bool tellstdfunc::secureLayDef(LayerNumber layno)
//{
//   bool success = true;
//   layprop::DrawProperties* drawProp;
//   if (PROPC->lockDrawProp(drawProp))
//   {
//      if (layno != REF_LAY)
//      {
//         if (drawProp->addLayer(layno))
//            PROPC->addUnpublishedLay(layno);
//      }
//      else
//      {
//         // TODO -add message here
//         success = false;
//      }
//   }
//   PROPC->unlockDrawProp(drawProp, false);
//   return success;
//}

//=============================================================================
void tellstdfunc::createDefaultTDT(std::string dbname,
      laydata::TdtLibDir* dbLibDir, TpdTime& timeCreated, bool threadExecution,
      parsercmd::UndoQUEUE& undstack, telldata::UNDOPerandQUEUE& undopstack)
{
   dbLibDir->newDesign(dbname, DATC->localDir(), timeCreated.stdCTime(), DEFAULT_DBU, DEFAULT_UU);
   dbLibDir->cleanUndefLib();
   DATC->bpRefreshTdtTab(true, threadExecution);
//   TpdPost::resetTDTtab(dbname);
   // reset UNDO buffers;
   undstack.clear();
   while (!undopstack.empty())
   {
      delete undopstack.front(); undopstack.pop_front();
   }
   LogFile << "newdesign(\""<< dbname << "\" , \"" << timeCreated() <<
         "\");"; LogFile.flush();
}

//=============================================================================
// void tellstdfunc::makeGdsLays(ExtLayers& gdsLays)
// {
//    NameList allls;
//    DATC->allLayers(allls);
//    for (NameList::const_iterator CL = allls.begin(); CL != allls.end(); CL++)
//    {
//       WordSet data_types;
//       data_types.insert(0);
//       gdsLays[DATC->getLayerNo(*CL)] = data_types;
//    }
// }
