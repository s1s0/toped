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


wxFrame*                         TopedMainW;
wxWindow*                        TopedCanvasW;

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::ted_cmd*         Console;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_SETINGSMENU;


//=============================================================================
telldata::ttint* tellstdfunc::CurrentLayer() {
   unsigned cl = PROPC->curLay();
   return (DEBUG_NEW telldata::ttint(cl));
}

//=============================================================================
bool tellstdfunc::waitGUInput(int input_type, telldata::operandSTACK *OPstack,
   std::string name, const CTM trans, int4b stepX, int4b stepY, word cols, word rows)
{
   // Create a temporary object in the TdtDesign (only if a DEBUG_NEW object is created, i.e. box,wire,polygon,cell etc.)
   try {DATC->mouseStart(input_type, name, trans, stepX, stepY, cols, rows);}
   catch (EXPTN) {return false;}
   // flag the prompt what type of data is expected & handle a pointer to
   // the operand stack
   Console->waitGUInput(OPstack, (console::ACTIVE_OP)input_type, trans);
   // flag the canvas that a mouse input will be required
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(input_type);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Console->threadWaits4->Wait();
   // ... and continue when the thread is woken up
   // Delete the temporary object in the TdtDesign
   DATC->mouseStop();
   // Stop the mouse stream from the canvas
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // and clean-up the state of the temporary ruler
   DATC->switch_drawruler(false);
   return Console->mouseIN_OK();
}

//=============================================================================
pointlist* tellstdfunc::t2tpoints(telldata::ttlist *pl, real DBscale) {
   pointlist *plDB = DEBUG_NEW pointlist();
   plDB->reserve(pl->size());
   telldata::ttpnt* pt;
   for (unsigned i = 0; i < pl->size(); i++) {
      pt = static_cast<telldata::ttpnt*>((pl->mlist())[i]);
      plDB->push_back(TP(pt->x(), pt->y(), DBscale));
   }
   return plDB;
}

//=============================================================================
telldata::ttlist* tellstdfunc::make_ttlaylist(laydata::SelectList* shapesel) {
   telldata::ttlist* llist = DEBUG_NEW telldata::ttlist(telldata::tn_layout);
   laydata::DataList* lslct;
   SGBitSet pntl;
   for (laydata::SelectList::const_iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++) {
      lslct = CL->second;
      // push each data reference into the TELL list
      for (laydata::DataList::const_iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++) {
         // copy the pointlist, because it will be deleted with the shapesel
         if (0 != CI->second.size()) pntl = SGBitSet(CI->second);
         else                        pntl = SGBitSet();
         llist->add(DEBUG_NEW telldata::ttlayout(CI->first, CL->first, DEBUG_NEW SGBitSet(pntl)));
      }
   }
   return llist;
}

//=============================================================================
void tellstdfunc::clean_ttlaylist(telldata::ttlist* llist) {
   // Several things to be noted here
   //  - First is kind of strange - data() method of telldata::ttlayout is defined
   // const, yet compiler doesn't complain that it is DELETED here
   //  - Second - the best place to clean-up the lists is of course inside
   // telldata::ttlayout class, however they don't know anything about laydata::TdtData
   // though with a strange error message compiler claims that destructors will
   // not be called - and I have no other choice but to belive it.
   // This of course is because of the separation of the project on modules
   // The other possibility is to convert the list to (say) AtticList and then
   // to use the corresponding destroyer, but that seem to be much more convoluted
   // This looks weird - true, but is doing the job.
   // - Don't try to delete here selp (selected ponts). It is deleted naturally
   // by the destructor of the telldata::ttlayout class it doesn't have the visibility
   // problem of laydata::TdtData
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      delete (static_cast<telldata::ttlayout*>(llist->mlist()[i])->data());
   }
}

//==============================================================================
void tellstdfunc::clean_atticlist(laydata::AtticList* nlst, bool destroy)
{
   for (laydata::AtticList::const_iterator CL = nlst->begin(); CL != nlst->end(); CL++)
   {
      if (destroy)
      {
         for (laydata::ShapeList::const_iterator DI = CL->second->begin(); DI != CL->second->end(); DI++)
            delete (*DI);
      }
      CL->second->clear();
      delete (CL->second);
   }
}

//=============================================================================
telldata::ttlist* tellstdfunc::make_ttlaylist(laydata::AtticList* shapesel) {
   telldata::ttlist* llist = DEBUG_NEW telldata::ttlist(telldata::tn_layout);
   laydata::ShapeList* lslct;
   for (laydata::AtticList::const_iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++) {
      lslct = CL->second;
      // push each data reference into the TELL list
      for (laydata::ShapeList::const_iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      //   if (sh_deleted == (*CI)->status()) - doesn't seems to need it!
            llist->add(DEBUG_NEW telldata::ttlayout(*CI, CL->first));
   }
   return llist;
}

//=============================================================================
laydata::SelectList* tellstdfunc::get_ttlaylist(telldata::ttlist* llist) {
   laydata::SelectList* shapesel = DEBUG_NEW laydata::SelectList();
   unsigned clayer;
   SGBitSet* pntl_o;
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      clayer = static_cast<telldata::ttlayout*>(llist->mlist()[i])->layer();
      if (shapesel->end() == shapesel->find(clayer))
         (*shapesel)[clayer] = DEBUG_NEW laydata::DataList();
      pntl_o = static_cast<telldata::ttlayout*>(llist->mlist()[i])->selp();

      SGBitSet pntl_n;
      if (NULL != pntl_o)  pntl_n = SGBitSet(*pntl_o);
      (*shapesel)[clayer]->push_back(laydata::SelectDataPair(
        static_cast<telldata::ttlayout*>(llist->mlist()[i])->data(), pntl_n));
   }
   return shapesel;
}

//=============================================================================
laydata::AtticList* tellstdfunc::get_shlaylist(telldata::ttlist* llist) {
   laydata::AtticList* shapesel = DEBUG_NEW laydata::AtticList();
   unsigned clayer;
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      clayer = static_cast<telldata::ttlayout*>(llist->mlist()[i])->layer();
      if (shapesel->end() == shapesel->find(clayer))
         (*shapesel)[clayer] = DEBUG_NEW laydata::ShapeList();
      (*shapesel)[clayer]->push_back(static_cast<telldata::ttlayout*>
                                                (llist->mlist()[i])->data());
   }
   return shapesel;
}


/** Filters shapesel using the mask. Returns new list, containing copy of
unfiltered components
*/
laydata::SelectList* tellstdfunc::filter_selist(const laydata::SelectList* shapesel, word mask)
{
   laydata::SelectList* filtered = DEBUG_NEW laydata::SelectList();

   for (laydata::SelectList::const_iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++)
   {
      laydata::DataList* ssl = DEBUG_NEW laydata::DataList();
      laydata::DataList* lslct = CL->second;
      for (laydata::DataList::const_iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      {
         if (mask & (CI->first->ltype()))
         {
            SGBitSet pntl;
            if (0 != CI->second.size()) pntl = SGBitSet(CI->second);
            ssl->push_back(laydata::SelectDataPair(CI->first,pntl));
         }
      }
      if    (ssl->empty()) delete ssl;
      else                 (*filtered)[CL->first] = ssl;
   }
   return filtered;
}

//=============================================================================
laydata::AtticList* tellstdfunc::replace_str(laydata::AtticList* shapesel, std::string newstr)
{
   laydata::AtticList* newtextlist = DEBUG_NEW laydata::AtticList();
   for (laydata::AtticList::iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++)
   {
      laydata::ShapeList* lslct  = CL->second;
      laydata::ShapeList* newlst = DEBUG_NEW laydata::ShapeList();
      for (laydata::ShapeList::iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      {
         assert(laydata::_lmtext == (*CI)->ltype());
         // using build-in copy constructor
         laydata::TdtText* newtxt = DEBUG_NEW laydata::TdtText(*(static_cast<laydata::TdtText*>(*CI)));
         newtxt->replace_str(newstr);
         newlst->push_back(newtxt);
      }
      (*newtextlist)[CL->first] = newlst;
   }
   return newtextlist;
}

//=============================================================================
void tellstdfunc::UpdateLV()
{
   wxString ws;
   ws.sprintf(wxT("%d"),DATC->numselected());
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
      const WordList freshlays = PROPC->upLayers();
      for(WordList::const_iterator CUL = freshlays.begin(); CUL != freshlays.end(); CUL++)
         TpdPost::layer_add(PROPC->getLayerName(*CUL), *CUL);
      PROPC->clearUnpublishedLayers();
   }
   Console->set_canvas_invalid(true);
}

//=============================================================================
void tellstdfunc::gridON(byte No, bool status) {
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   status = PROPC->viewGrid(No, status);
   switch (No) {
      case 0: eventGRIDUPD.SetInt((status ? tui::STS_GRID0_ON : tui::STS_GRID0_OFF)); break;
      case 1: eventGRIDUPD.SetInt((status ? tui::STS_GRID1_ON : tui::STS_GRID1_OFF)); break;
      case 2: eventGRIDUPD.SetInt((status ? tui::STS_GRID2_ON : tui::STS_GRID2_OFF)); break;
      default: assert(false);
   }
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
}

//=============================================================================
void tellstdfunc::updateLayerDefinitions(laydata::TdtLibDir* LIBDIR, nameList& top_cells, int libID)
{
   // get all the layers used in the design and define them using the default definition
   WordList ull;
   for(nameList::const_iterator CTC= top_cells.begin(); CTC != top_cells.end(); CTC++)
      LIBDIR->collect_usedlays(*CTC, true, ull);
   ull.sort(); ull.unique();
//   std::unique(ull.begin(),ull.end());
   for(WordList::const_iterator CUL = ull.begin(); CUL != ull.end(); CUL++)
   {
      if (0 == *CUL) continue;
      if (PROPC->addLayer(*CUL))
         TpdPost::layer_add(PROPC->getLayerName(*CUL), *CUL);
   }
}

//=============================================================================
void tellstdfunc::initFuncLib(wxFrame* tpd, wxWindow* cnvs)
{
   TopedMainW = tpd;
   TopedCanvasW = cnvs;
}

//=============================================================================
// void tellstdfunc::makeGdsLays(ExtLayers& gdsLays)
// {
//    nameList allls;
//    DATC->allLayers(allls);
//    for (nameList::const_iterator CL = allls.begin(); CL != allls.end(); CL++)
//    {
//       WordSet data_types;
//       data_types.insert(0);
//       gdsLays[DATC->getLayerNo(*CL)] = data_types;
//    }
// }
