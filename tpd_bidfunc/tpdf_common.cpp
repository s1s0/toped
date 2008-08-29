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
#include <wx/wx.h>
#include <wx/regex.h>
#include "tpdf_common.h"
#include "../tpd_DB/datacenter.h"
#include "../tpd_parser/ted_prompt.h"
#include "../tpd_common/tuidefs.h"
#include "../tpd_DB/browsers.h"


wxFrame*                         TopedMainW;
wxWindow*                        TopedCanvasW;

extern DataCenter*               DATC;
extern console::ted_cmd*         Console;

extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_SETINGSMENU;


//=============================================================================
tellstdfunc::LayerMapGds::LayerMapGds(GDSin::NumStrMap& inlist)
{
//   typedef std::map<word, GdsLayers>      TdtGdsMap;
//   typedef std::map<word, std::string>    NumStrMap;
//GDSin::TdtGdsMap*    _theMap;
   for (GDSin::NumStrMap::const_iterator CE = inlist.begin(); CE != inlist.end(); CE++)
   {
      WordList layList;
      WordList dTypeList;
      parseLayTypeString(CE->second, layList, dTypeList);
   }
}

bool tellstdfunc::LayerMapGds::parseLayTypeString(std::string str, WordList& llst, WordList& dtlst)
{
   const wxString tmplLayNumbers    = wxT("[[:digit:]\\,\\-]*");
   const wxString tmplTypeNumbers   = wxT("[[:digit:]\\,\\-]*|\\*");

   wxString exp(str.c_str(), wxConvUTF8);
   patternNormalize(exp);

   wxRegEx src_tmpl(tmplLayNumbers+wxT("\\;")+tmplTypeNumbers); VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp))
   {
//      std::string news = "Can't make sence from the string \"" + std::string(exp.c_str()) + "\"";
//      tell_log(console::MT_ERROR,news);
      return false;
   }
   //separate the layer expression from data type expression
   src_tmpl.Compile(tmplLayNumbers+wxT("\\;")); VERIFY(src_tmpl.IsValid());
   src_tmpl.Matches(exp);
   wxString lay_exp = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   wxString type_exp = exp;
   // we need to remove the ';' separator that left in the lay_exp
   src_tmpl.Compile(wxT("\\;")); VERIFY(src_tmpl.IsValid());
   src_tmpl.Matches(exp);
   src_tmpl.ReplaceFirst(&lay_exp,wxT(""));

   getList(  lay_exp , llst);
   getList( type_exp , dtlst);
   return true;
}

void tellstdfunc::LayerMapGds::patternNormalize(wxString& str)
{
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before separators
   VERIFY(regex.Compile(wxT("([[:space:]])([\\-\\;\\,])")));
   regex.ReplaceAll(&str,wxT("\\2"));
   // remove spaces after separators
   VERIFY(regex.Compile(wxT("([\\-\\;\\,])([[:space:]])")));
   regex.ReplaceAll(&str,wxT("\\1"));

}

void tellstdfunc::LayerMapGds::getList(wxString& exp, WordList& data)
{
   wxRegEx number_tmpl(wxT("[[:digit:]]*"));
   wxRegEx separ_tmpl(wxT("\\,\\-{1,1}"));
   unsigned long conversion;
   bool last_was_separator = true;
   char separator = ',';
   VERIFY(number_tmpl.IsValid());
   VERIFY(separ_tmpl.IsValid());

   do
   {
      if (last_was_separator)
      {
         number_tmpl.Matches(exp);
         number_tmpl.GetMatch(exp).ToULong(&conversion);
         number_tmpl.ReplaceFirst(&exp,wxT(""));
         if (',' == separator)
            data.push_back((word)conversion);
         else
         {
            for (word numi = data.back(); numi <= conversion; numi++)
               data.push_back(numi);
         }
      }
      else
      {
         separ_tmpl.Matches(exp);
         if (wxT("-") == separ_tmpl.GetMatch(exp))
            separator = '-';
         else
            separator = ',';
         separ_tmpl.ReplaceFirst(&exp,wxT(""));
      }
      last_was_separator = !last_was_separator;
   } while (!exp.IsEmpty());

}
//=============================================================================
telldata::ttint* tellstdfunc::CurrentLayer() {
   word cl = 0;
   cl = DATC->curlay();
   return (DEBUG_NEW telldata::ttint(cl));
}

//=============================================================================
bool tellstdfunc::waitGUInput(int input_type, telldata::operandSTACK *OPstack,
   std::string name, const CTM trans, int4b stepX, int4b stepY, word cols, word rows)
{
   // Create a temporary object in the tdtdesign (only if a DEBUG_NEW object is created, i.e. box,wire,polygon,cell etc.)
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
   // Delete the temporary object in the tdtdesign
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
telldata::ttlist* tellstdfunc::make_ttlaylist(laydata::selectList* shapesel) {
   telldata::ttlist* llist = DEBUG_NEW telldata::ttlist(telldata::tn_layout);
   laydata::dataList* lslct;
   SGBitSet pntl;
   for (laydata::selectList::const_iterator CL = shapesel->begin(); 
                                            CL != shapesel->end(); CL++) {
      lslct = CL->second;
      // push each data reference into the TELL list
      for (laydata::dataList::const_iterator CI = lslct->begin(); 
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
   // telldata::ttlayout class, however they don't know anything about laydata::tdtdata
   // though with a strange error message compiler claims that destructors will 
   // not be called - and I have no other choice but to belive it.
   // This of course is because of the separation of the project on modules
   // The other possibility is to convert the list to (say) atticList and then
   // to use the corresponding destroyer, but that seem to be much more convoluted
   // This looks weird - true, but is doing the job.
   // - Don't try to delete here selp (selected ponts). It is deleted naturally
   // by the destructor of the telldata::ttlayout class it doesn't have the visibility
   // problem of laydata::tdtdata
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      delete (static_cast<telldata::ttlayout*>(llist->mlist()[i])->data());
   }
}

//==============================================================================
void tellstdfunc::clean_atticlist(laydata::atticList* nlst, bool destroy)
{
   for (laydata::atticList::const_iterator CL = nlst->begin(); CL != nlst->end(); CL++) 
   {
      if (destroy)
      {
         for (laydata::shapeList::const_iterator DI = CL->second->begin(); DI != CL->second->end(); DI++) 
            delete (*DI);
      }
      CL->second->clear();
      delete (CL->second);
   }
}

//=============================================================================
telldata::ttlist* tellstdfunc::make_ttlaylist(laydata::atticList* shapesel) {
   telldata::ttlist* llist = DEBUG_NEW telldata::ttlist(telldata::tn_layout);
   laydata::shapeList* lslct;
   for (laydata::atticList::const_iterator CL = shapesel->begin(); 
                                            CL != shapesel->end(); CL++) {
      lslct = CL->second;
      // push each data reference into the TELL list
      for (laydata::shapeList::const_iterator CI = lslct->begin(); 
                                             CI != lslct->end(); CI++)
      //   if (sh_deleted == (*CI)->status()) - doesn't seems to need it!
            llist->add(DEBUG_NEW telldata::ttlayout(*CI, CL->first));
   }
   return llist;
}

//=============================================================================
laydata::selectList* tellstdfunc::get_ttlaylist(telldata::ttlist* llist) {
   laydata::selectList* shapesel = DEBUG_NEW laydata::selectList();
   word clayer;
   SGBitSet* pntl_o;
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      clayer = static_cast<telldata::ttlayout*>(llist->mlist()[i])->layer();
      if (shapesel->end() == shapesel->find(clayer))
         (*shapesel)[clayer] = DEBUG_NEW laydata::dataList();
      pntl_o = static_cast<telldata::ttlayout*>(llist->mlist()[i])->selp();

      SGBitSet pntl_n;
      if (NULL != pntl_o)  pntl_n = SGBitSet(*pntl_o);
      (*shapesel)[clayer]->push_back(laydata::selectDataPair(
        static_cast<telldata::ttlayout*>(llist->mlist()[i])->data(), pntl_n));
   }
   return shapesel;
}

//=============================================================================
laydata::atticList* tellstdfunc::get_shlaylist(telldata::ttlist* llist) {
   laydata::atticList* shapesel = DEBUG_NEW laydata::atticList();
   word clayer;
   for (word i = 0 ; i < llist->mlist().size(); i++) {
      clayer = static_cast<telldata::ttlayout*>(llist->mlist()[i])->layer();
      if (shapesel->end() == shapesel->find(clayer))
         (*shapesel)[clayer] = DEBUG_NEW laydata::shapeList();
      (*shapesel)[clayer]->push_back(static_cast<telldata::ttlayout*>
                                                (llist->mlist()[i])->data());
   }
   return shapesel;
}


/** Filters shapesel using the mask. Returns new list, containing copy of
unfiltered components
*/
laydata::selectList* tellstdfunc::filter_selist(const laydata::selectList* shapesel, word mask)
{
   laydata::selectList* filtered = DEBUG_NEW laydata::selectList();

   for (laydata::selectList::const_iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++)
   {
      laydata::dataList* ssl = DEBUG_NEW laydata::dataList();
      laydata::dataList* lslct = CL->second;
      for (laydata::dataList::const_iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      {
         if (mask & CI->first->ltype())
         {
            SGBitSet pntl;
            if (0 != CI->second.size()) pntl = SGBitSet(CI->second);
            ssl->push_back(laydata::selectDataPair(CI->first,pntl));
         }
      }
      if    (ssl->empty()) delete ssl;
      else                 (*filtered)[CL->first] = ssl;
   }
   return filtered;
}

//=============================================================================
laydata::atticList* tellstdfunc::replace_str(laydata::atticList* shapesel, std::string newstr)
{
   laydata::atticList* newtextlist = DEBUG_NEW laydata::atticList();
   for (laydata::atticList::iterator CL = shapesel->begin();
                                            CL != shapesel->end(); CL++)
   {
      laydata::shapeList* lslct  = CL->second;
      laydata::shapeList* newlst = DEBUG_NEW laydata::shapeList();
      for (laydata::shapeList::iterator CI = lslct->begin();
                                             CI != lslct->end(); CI++)
      {
         assert(laydata::_lmtext == (*CI)->ltype());
         // using build-in copy constructor
         laydata::tdttext* newtxt = DEBUG_NEW laydata::tdttext(*(static_cast<laydata::tdttext*>(*CI)));
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
   if (!DATC->upLayers().empty())
   {
      const laydata::ListOfWords freshlays = DATC->upLayers();
      for(laydata::ListOfWords::const_iterator CUL = freshlays.begin(); CUL != freshlays.end(); CUL++)
         browsers::layer_add(UNDEFLAYNAME, *CUL);
      DATC->clearUnpublishedLayers();
   }
   Console->set_canvas_invalid(true);
}

//=============================================================================
void tellstdfunc::gridON(byte No, bool status) {
   wxCommandEvent eventGRIDUPD(wxEVT_SETINGSMENU);
   status = DATC->viewGrid(No, status);
   switch (No) {
      case 0: eventGRIDUPD.SetInt((status ? tui::STS_GRID0_ON : tui::STS_GRID0_OFF)); break;
      case 1: eventGRIDUPD.SetInt((status ? tui::STS_GRID1_ON : tui::STS_GRID1_OFF)); break;
      case 2: eventGRIDUPD.SetInt((status ? tui::STS_GRID2_ON : tui::STS_GRID2_OFF)); break;
      default: assert(false);
   }
   wxPostEvent(TopedCanvasW, eventGRIDUPD);
}

//=============================================================================
void tellstdfunc::updateLayerDefinitions(laydata::tdtlibdir* LIBDIR, nameList& top_cells, int libID)
{
   // get all the layers used in the design and define them using the default definition
   laydata::ListOfWords ull;
   for(nameList::const_iterator CTC= top_cells.begin(); CTC != top_cells.end(); CTC++)
      LIBDIR->collect_usedlays(*CTC, true, ull);
   ull.sort(); ull.unique();
//   std::unique(ull.begin(),ull.end());
   for(laydata::ListOfWords::const_iterator CUL = ull.begin(); CUL != ull.end(); CUL++)
   {
      if (0 == *CUL) continue;
      if (DATC->addlayer(UNDEFLAYNAME, *CUL))
         browsers::layer_add(UNDEFLAYNAME, *CUL);
   }
}

//=============================================================================
void tellstdfunc::initFuncLib(wxFrame* tpd, wxWindow* cnvs)
{
   TopedMainW = tpd;
   TopedCanvasW = cnvs;
}

