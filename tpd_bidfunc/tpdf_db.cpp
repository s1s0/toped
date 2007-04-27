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
//    Description: Definition of all TOPED database functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdf_db.h"
#include <sstream>
#include "../tpd_DB/datacenter.h"
#include "../tpd_common/tuidefs.h"
#include "../tpd_DB/browsers.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdNEWDESIGN::stdNEWDESIGN(telldata::typeID retype, bool eor) :
      stdNEWDESIGNd(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::stdNEWDESIGN::execute()
{
   TpdTime timeCreated(time(NULL));
   OPstack.push(new telldata::ttstring(timeCreated()));
   return stdNEWDESIGNd::execute();
}

//=============================================================================
tellstdfunc::stdNEWDESIGNd::stdNEWDESIGNd(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::stdNEWDESIGNd::execute()
{
   TpdTime timeCreated(getStringValue());
   std::string nm = getStringValue();
   DATC->newDesign(nm, timeCreated.stdCTime());
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      ATDB->btreeAddMember    = &browsers::treeAddMember;
      ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
      browsers::addTDTtab(nm, ATDB->hiertree());
   DATC->unlockDB();
   // reset UNDO buffers;
   UNDOcmdQ.clear();
   while (!UNDOPstack.empty()) {
      delete UNDOPstack.front(); UNDOPstack.pop_front();
   }
   LogFile << LogFile.getFN() << "(\""<< nm << "\" , \"" << timeCreated() <<
         "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTread::TDTread(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::TDTread::execute()
{
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      nameList top_cell_list;
      if (DATC->TDTread(filename))
      {
         laydata::tdtdesign* ATDB = DATC->lockDB(false);
            ATDB->btreeAddMember    = &browsers::treeAddMember;
            ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
            browsers::addTDTtab(ATDB->name(), ATDB->hiertree());
            laydata::TDTHierTree* root = ATDB->hiertree()->GetFirstRoot();
            do
            {
               top_cell_list.push_back(std::string(root->GetItem()->name()));
            } while (NULL != (root = root->GetNextRoot()));
            TpdTime timec(ATDB->created());
            TpdTime timeu(ATDB->lastUpdated());
            updateLayerDefinitions(ATDB, top_cell_list);
         DATC->unlockDB();
         LogFile << LogFile.getFN() << "(\""<< filename << "\",\"" <<  timec() <<
               "\",\"" <<  timeu() << "\");"; LogFile.flush();
         // reset UNDO buffers;
         UNDOcmdQ.clear();
         while (!UNDOPstack.empty()) {
            delete UNDOPstack.front(); UNDOPstack.pop_front();
         }
      }
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTreadIFF::TDTreadIFF(telldata::typeID retype, bool eor) :
      TDTread(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::TDTreadIFF::execute()
{
   TpdTime timeSaved(getStringValue());
   TpdTime timeCreated(getStringValue());
   std::string filename = getStringValue();
   if (!(timeSaved.status() && timeCreated.status()))
   {
      tell_log(console::MT_ERROR,"Bad time format in read command");
   }
   else if (expandFileName(filename))
   {
      bool start_ignoring = false;
      if (DATC->TDTcheckread(filename, timeCreated, timeSaved, start_ignoring))
      {
         std::list<std::string> top_cell_list;
         DATC->TDTread(filename);
         laydata::tdtdesign* ATDB = DATC->lockDB(false);
         ATDB->btreeAddMember    = &browsers::treeAddMember;
         ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
         browsers::addTDTtab(ATDB->name(), ATDB->hiertree());
         laydata::TDTHierTree* root = ATDB->hiertree()->GetFirstRoot();
         do
         {
            top_cell_list.push_back(std::string(root->GetItem()->name()));
         } while (NULL != (root = root->GetNextRoot()));
         TpdTime timec(ATDB->created());
         TpdTime timeu(ATDB->lastUpdated());
         updateLayerDefinitions(ATDB, top_cell_list);
         DATC->unlockDB();
         LogFile << LogFile.getFN() << "(\""<< filename << "\",\"" <<  timec() <<
               "\",\"" <<  timeu() << "\");"; LogFile.flush();
         // reset UNDO buffers;
         UNDOcmdQ.clear();
         while (!UNDOPstack.empty()) {
            delete UNDOPstack.front(); UNDOPstack.pop_front();
         }
      }
      if (start_ignoring) set_ignoreOnRecovery(true);
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTsave::TDTsave(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::TDTsave::execute() {
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_all();
   DATC->unlockDB();
   DATC->TDTwrite();
   ATDB = DATC->lockDB(false);
      TpdTime timec(ATDB->created());
      TpdTime timeu(ATDB->lastUpdated());
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "(\"" <<  timec() << "\" , \"" << 
         timeu() << "\");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTsaveIFF::TDTsaveIFF(telldata::typeID retype, bool eor) :
      TDTsave(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::TDTsaveIFF::execute() {
   TpdTime timeSaved(getStringValue());
   TpdTime timeCreated(getStringValue());
   if (!(timeSaved.status() && timeCreated.status()))
   {
      tell_log(console::MT_ERROR,"Bad time format in read command");
   }
   else
   {
      laydata::tdtdesign* ATDB = DATC->lockDB();
         ATDB->unselect_all();
      DATC->unlockDB();
      bool stop_ignoring = false;
      if (DATC->TDTcheckwrite(timeCreated, timeSaved, stop_ignoring))
      {
         DATC->TDTwrite(DATC->tedfilename().c_str());
         ATDB = DATC->lockDB(false);
            TpdTime timec(ATDB->created());
            TpdTime timeu(ATDB->lastUpdated());
         DATC->unlockDB();
         LogFile << LogFile.getFN() << "(\"" <<  timec() << "\" , \"" <<
               timeu() << "\");"; LogFile.flush();
      }
      if (stop_ignoring) set_ignoreOnRecovery(false); 
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTsaveas::TDTsaveas(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::TDTsaveas::execute() {
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      laydata::tdtdesign* ATDB = DATC->lockDB();
         ATDB->unselect_all();
      DATC->unlockDB();
      DATC->TDTwrite(filename.c_str());
      ATDB = DATC->lockDB(false);
         TpdTime timec(ATDB->created());
         TpdTime timeu(ATDB->lastUpdated());
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "(\""<< filename << "\" , \"" << timec() <<
            "\" , \"" << timeu() << "\");"; LogFile.flush();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSread::GDSread(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSread::execute() {
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      std::list<std::string> top_cell_list;
      DATC->GDSparse(filename);
      // add GDS tab in the browser
      browsers::addGDStab();
      //
      GDSin::GDSFile* AGDSDB = DATC->lockGDS();

         GDSin::GDSHierTree* root = AGDSDB->hiertree()->GetFirstRoot();
         do 
         {
            top_cell_list.push_back(std::string(root->GetItem()->Get_StrName()));
         } while (NULL != (root = root->GetNextRoot()));
      DATC->unlockGDS();
      telldata::ttlist* topcells = new telldata::ttlist(telldata::tn_string);
      for (std::list<std::string>::const_iterator CN = top_cell_list.begin();
                                                CN != top_cell_list.end(); CN ++)
         topcells->add(new telldata::ttstring(*CN));
      OPstack.push(topcells);
      LogFile << LogFile.getFN() << "(\""<< filename << "\");"; LogFile.flush();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSconvert::GDSconvert(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor) 
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));

}

int tellstdfunc::GDSconvert::execute() 
{
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   std::string name = getStringValue();
   nameList top_cells;
   top_cells.push_back(name.c_str());
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      DATC->importGDScell(top_cells, recur, over);
      updateLayerDefinitions(ATDB, top_cells);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "(\""<< name << "\"," << LogFile._2bool(recur) 
         << "," << LogFile._2bool(over) << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSconvertAll::GDSconvertAll(telldata::typeID retype, bool eor) :
                              cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor) 
{
   arguments->push_back(new argumentTYPE("", new telldata::ttlist(telldata::tn_string)));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
   arguments->push_back(new argumentTYPE("", new telldata::ttbool()));
}

int tellstdfunc::GDSconvertAll::execute() 
{
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   nameList top_cells;
   for (unsigned i = 0; i < pl->size(); i++)
   {
      top_cells.push_back((static_cast<telldata::ttstring*>((pl->mlist())[i]))->value());
   }
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      DATC->importGDScell(top_cells, recur, over);
      browsers::addTDTtab(ATDB->name(), ATDB->hiertree());
      updateLayerDefinitions(ATDB, top_cells);
   DATC->unlockDB();
   
   LogFile << LogFile.getFN() << "(\""<< *pl << "\"," << LogFile._2bool(recur)
         << "," << LogFile._2bool(over) << ");"; LogFile.flush();
   delete pl;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSexportLIB::GDSexportLIB(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::GDSexportLIB::execute()
{
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      DATC->lockDB(false);
         DATC->GDSexport(filename);
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "(\""<< filename << ");"; LogFile.flush();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSexportTOP::GDSexportTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
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
   if (expandFileName(filename))
   {
      laydata::tdtcell *excell = NULL;
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         excell = ATDB->checkcell(cellname);
         if (NULL != excell)
            DATC->GDSexport(excell, recur, filename);
      DATC->unlockDB();
      if (NULL != excell)
      {
         LogFile << LogFile.getFN() << "(\""<< cellname << "\"," 
               << LogFile._2bool(recur) << ",\"" << filename << "\");";
         LogFile.flush();
      }
      else
      {
         std::string message = "Cell " + cellname + " not found in the database";
         tell_log(console::MT_ERROR,message);
      }
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
      
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::PSexportTOP::PSexportTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
   arguments->push_back(new argumentTYPE("", new telldata::ttstring()));
}

int tellstdfunc::PSexportTOP::execute()
{
   std::string filename = getStringValue();
   std::string cellname = getStringValue();
   if (expandFileName(filename))
   {
      laydata::tdtcell *excell = NULL;
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         excell = ATDB->checkcell(cellname);
         if (NULL != excell)
            DATC->PSexport(excell, filename);
      DATC->unlockDB();
      if (NULL != excell)
      {
         LogFile << LogFile.getFN() << "(\""<< cellname << "\"," 
                                    << ",\"" << filename << "\");";
         LogFile.flush();
      }
      else
      {
         std::string message = "Cell " + cellname + " not found in the database";
         tell_log(console::MT_ERROR,message);
      }
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSclose::GDSclose(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(new parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::GDSclose::execute() {
   browsers::clearGDStab();
   DATC->GDSclose();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
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

