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

#include "tpdph.h"
#include "tpdf_db.h"
#include <sstream>
#include "../tpd_DB/datacenter.h"
#include "../tpd_common/tuidefs.h"
#include "../tpd_DB/browsers.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdNEWDESIGN::stdNEWDESIGN(telldata::typeID retype, bool eor) :
      stdNEWDESIGNd(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdNEWDESIGN::execute()
{
   TpdTime timeCreated(time(NULL));
   OPstack.push(DEBUG_NEW telldata::ttstring(timeCreated()));
   return stdNEWDESIGNd::execute();
}

//=============================================================================
tellstdfunc::stdNEWDESIGNd::stdNEWDESIGNd(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdNEWDESIGNd::execute()
{
   TpdTime timeCreated(getStringValue());
   std::string nm = getStringValue();
   DATC->newDesign(nm, timeCreated.stdCTime());
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      ATDB->btreeAddMember    = &browsers::treeAddMember;
      ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
   DATC->unlockDB();
   browsers::addTDTtab();
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::TDTread::execute()
{
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      if (DATC->TDTread(filename))
      {
         laydata::tdtdesign* ATDB = DATC->lockDB(false);
            // Initialize call back functions
            ATDB->btreeAddMember    = &browsers::treeAddMember;
            ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
               // time stamps
            TpdTime timec(ATDB->created());
            TpdTime timeu(ATDB->lastUpdated());
               // Gatering the used layers & update the layer definitions
            std::list<std::string> top_cell_list;
            laydata::TDTHierTree* root = ATDB->hiertree()->GetFirstRoot(TARGETDB_LIB);
            do
            {
               top_cell_list.push_back(std::string(root->GetItem()->name()));
            } while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
            updateLayerDefinitions( DATC->TEDLIB(), top_cell_list, TARGETDB_LIB);
         DATC->unlockDB();
         // populate the hierarchy browser
         browsers::addTDTtab(true);
         //
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
      TDTread(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
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
         DATC->TDTread(filename);
         laydata::tdtdesign* ATDB = DATC->lockDB(false);
            // Initialize call back functions
            ATDB->btreeAddMember    = &browsers::treeAddMember;
            ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
            // time stamps
            TpdTime timec(ATDB->created());
            TpdTime timeu(ATDB->lastUpdated());
            // Gatering the used layers & update the layer definitions
            std::list<std::string> top_cell_list;
            laydata::TDTHierTree* root = ATDB->hiertree()->GetFirstRoot(TARGETDB_LIB);
            do
            {
               top_cell_list.push_back(std::string(root->GetItem()->name()));
            } while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
            updateLayerDefinitions(DATC->TEDLIB(), top_cell_list, TARGETDB_LIB);
         DATC->unlockDB();
         // populate the cell hierarchy browser
         browsers::addTDTtab(true);
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
tellstdfunc::TDTloadlib::TDTloadlib(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::TDTloadlib::execute()
{
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      nameList top_cell_list;
      int libID = DATC->TDTloadlib(filename);
      if (0 <= libID)
      {
         laydata::tdtlibrary* LTDB = DATC->getLib(libID);
         // Gatering the used layers & update the layer definitions
         laydata::TDTHierTree* root = LTDB->hiertree()->GetFirstRoot(libID);
         do
         {
            top_cell_list.push_back(std::string(root->GetItem()->name()));
         } while (NULL != (root = root->GetNextRoot(libID)));
         updateLayerDefinitions(DATC->TEDLIB(), top_cell_list, libID);
         DATC->TEDLIB()->cleanUndefLib();
         // populating cell hierarchy browser
         browsers::addTDTtab(true);
         LogFile << LogFile.getFN() << "(\""<< filename << "\");"; LogFile.flush();
      }
      else
      {
         std::string info = "Can't load \"" + filename + "\" as a library";
         tell_log(console::MT_ERROR,info);
      }
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//===================================================return==========================
tellstdfunc::TDTunloadlib::TDTunloadlib(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::TDTunloadlib::execute()
{
   std::string libname = getStringValue();
   
   if (DATC->TDTunloadlib(libname))
   {
      browsers::addTDTtab(true);
      LogFile << LogFile.getFN() << "(\""<< libname << "\");"; LogFile.flush();
   }
   else
   {
      std::string info = "Library \"" + libname + "\" is not loaded";
      tell_log(console::MT_ERROR,info);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::TDTsave::TDTsave(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
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
      TDTsave(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::GDSread::execute() {
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      std::list<std::string> top_cell_list;
      if (DATC->GDSparse(filename))
      {
         // add GDS tab in the browser
         browsers::addGDStab();
         //
         GDSin::GdsFile* AGDSDB = DATC->lockGDS();

            GDSin::GDSHierTree* root = AGDSDB->hierTree()->GetFirstRoot(TARGETDB_LIB);
            assert(root);
            do 
            {
               top_cell_list.push_back(std::string(root->GetItem()->name()));
            } while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
         DATC->unlockGDS();
         telldata::ttlist* topcells = DEBUG_NEW telldata::ttlist(telldata::tn_string);
         for (std::list<std::string>::const_iterator CN = top_cell_list.begin();
                                                   CN != top_cell_list.end(); CN ++)
            topcells->add(DEBUG_NEW telldata::ttstring(*CN));
         OPstack.push(topcells);
         LogFile << LogFile.getFN() << "(\""<< filename << "\");"; LogFile.flush();
      }
      else
      {
         std::string info = "File \"" + filename + "\" doesn't seem to appear a valid GDSII file";
         tell_log(console::MT_ERROR,info);
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
tellstdfunc::GDSimportT::GDSimportT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSimportT::execute()
{
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();

   // Convert layer map
   telldata::tthsh* nameh;
   USMap gdsLaysStrList;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      gdsLaysStrList[nameh->number().value()] = nameh->name().value();
   }

   GDSin::GdsFile* AGDSDB = DATC->lockGDS();
   GDSin::GdsStructure *src_structure = AGDSDB->getStructure(name.c_str());
   std::ostringstream ost;
   if (!src_structure)
   {
      ost << "GDS structure named \"" << name << "\" does not exists";
      tell_log(console::MT_ERROR,ost.str());
      DATC->unlockGDS();
   }
   else
   {
      GdsLayers gdsLaysAll;
      src_structure->collectLayers(gdsLaysAll,true);
      DATC->unlockGDS();
      GDSin::LayerMapGds LayerExpression(gdsLaysStrList, &gdsLaysAll);
      if (LayerExpression.status())
      {
         nameList top_cells;
         top_cells.push_back(name);
         DATC->lockDB(false);
         DATC->importGDScell(top_cells, LayerExpression, recur, over);
            updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
         DATC->unlockDB();
         LogFile << LogFile.getFN() << "(\""<< name << "," << (*lll) << "," << LogFile._2bool(recur)
               << "," << LogFile._2bool(over) << ");"; LogFile.flush();
      }
      else
      {
         ost << "Can't execute GDS import - error in the layer map";
         tell_log(console::MT_ERROR,ost.str());
      }
   }
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSimport::GDSimport(telldata::typeID retype, bool eor) :
      GDSimportT(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSimport::execute() 
{
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
//   std::string filename = getStringValue();

//   OPstack.push(DEBUG_NEW telldata::ttstring(filename));
   OPstack.push(DEBUG_NEW telldata::ttlist(telldata::tn_hsh));
   OPstack.push(DEBUG_NEW telldata::ttbool(recur));
   OPstack.push(DEBUG_NEW telldata::ttbool(over));

   return GDSimportT::execute();
}

//=============================================================================
tellstdfunc::GDSimportListT::GDSimportListT(telldata::typeID retype, bool eor) :
                              cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor) 
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_string)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSimportListT::execute()
{
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   nameList top_cells;
   for (unsigned i = 0; i < pl->size(); i++)
   {
      top_cells.push_back((static_cast<telldata::ttstring*>((pl->mlist())[i]))->value());
   }
   // Convert layer map
   telldata::tthsh* nameh;
   USMap gdsLaysStrList;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      gdsLaysStrList[nameh->number().value()] = nameh->name().value();
   }
   GdsLayers gdsLaysAll;
   GDSin::GdsFile* AGDSDB = DATC->lockGDS();
      AGDSDB->collectLayers(gdsLaysAll);
   DATC->unlockGDS();
   GDSin::LayerMapGds LayerExpression(gdsLaysStrList, &gdsLaysAll);
   if (LayerExpression.status())
   {
      DATC->lockDB(false);
         DATC->importGDScell(top_cells, LayerExpression, recur, over);
         updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
      DATC->unlockDB();
      // Don't refresh the tree browser here. 
      // - First - it has been updated during the conversion
      // - Second - addTDTtab is running in the same thread as the caller. It must
      // make sure that there is nothing left in the PostEvent queue in the main thread
      // which was filled-up during the conversion.
      // bottom line - don't do that, or you'll suffer ...
      // @TODO Check whether is not a good idea to skip the cell browser update
      // during GDS import. The calling addTDTtab() at the end should be safe
      //browsers::addTDTtab();
      LogFile << LogFile.getFN() << "("<< *pl << "," << *lll << "," << LogFile._2bool(recur)
            << "," << LogFile._2bool(over) << ");"; LogFile.flush();
   }
   else
   {
      std::ostringstream ost;
      ost << "Can't execute GDS import - error in the layer map";
      tell_log(console::MT_ERROR,ost.str());
   }
   delete pl;
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSimportList::GDSimportList(telldata::typeID retype, bool eor) :
      GDSimportListT(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_string)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSimportList::execute() 
{
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   // Don't pop out the last argument - we'll just have to put it back
   //telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();

   // Don't push in the first argument - it wasn't poped-up
   // OPstack.push(DEBUG_NEW telldata::ttlist(*pl));
   OPstack.push(DEBUG_NEW telldata::ttlist(telldata::tn_hsh));
   OPstack.push(DEBUG_NEW telldata::ttbool(recur));
   OPstack.push(DEBUG_NEW telldata::ttbool(over));

   return GDSimportListT::execute();
}

//=============================================================================
tellstdfunc::GDSexportLIBT::GDSexportLIBT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSexportLIBT::execute()
{
   bool x2048           = getBoolValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string filename = getStringValue();

   // Convert layer map
   USMap gdsLays;
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      gdsLays[nameh->number().value()] = nameh->name().value();
   }

   if (expandFileName(filename))
   {
      DATC->lockDB(false);
         GDSin::LayerMapGds default_map(gdsLays, NULL);
         DATC->GDSexport(default_map, filename, x2048);
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "(\""<< filename << "\", " 
              << *lll << ", " 
              << LogFile._2bool(x2048) <<");"; 
      LogFile.flush();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSexportLIB::GDSexportLIB(telldata::typeID retype, bool eor) :
      GDSexportLIBT(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSexportLIB::execute() 
{
   bool  x2048 = getBoolValue();
   //std::string filename = getStringValue();

   //OPstack.push(DEBUG_NEW telldata::ttstring(filename));
   OPstack.push(DEBUG_NEW telldata::ttlist(telldata::tn_hsh));
   OPstack.push(DEBUG_NEW telldata::ttbool(x2048));

   return GDSexportLIBT::execute();
}

//=============================================================================
tellstdfunc::GDSexportTOPT::GDSexportTOPT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSexportTOPT::execute()
{
   bool  x2048 = getBoolValue();
   std::string filename = getStringValue();
   bool  recur = getBoolValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string cellname = getStringValue();

   // Convert layer map
   USMap gdsLays;
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      gdsLays[nameh->number().value()] = nameh->name().value();
   }
   if (expandFileName(filename))
   {
      laydata::tdtcell *excell = NULL;
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         excell = ATDB->checkcell(cellname);

         if (NULL != excell)
         {
            GDSin::LayerMapGds default_map(gdsLays, NULL);

            DATC->GDSexport(excell, default_map, recur, filename, x2048);
            LogFile  << LogFile.getFN() 
                     << "(\""<< cellname << "\"," 
                     << LogFile._2bool(recur) 
                     << ",\"" << filename << "\"," 
                     << LogFile._2bool(x2048) <<");"; 
            LogFile.flush();
         }
         else
         {
            std::string message = "Cell " + cellname + " not found in the database";
            tell_log(console::MT_ERROR,message);
         }
      DATC->unlockDB();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   delete lll;
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::GDSexportTOP::GDSexportTOP(telldata::typeID retype, bool eor) :
      GDSexportTOPT(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSexportTOP::execute() 
{
   bool  x2048 = getBoolValue();
   std::string filename = getStringValue();
   bool  recur = getBoolValue();
   //std::string cellname = getStringValue();

   //OPstack.push(DEBUG_NEW telldata::ttstring(cellname));
   OPstack.push(DEBUG_NEW telldata::ttlist(telldata::tn_hsh));
   OPstack.push(DEBUG_NEW telldata::ttbool(recur));
   OPstack.push(DEBUG_NEW telldata::ttstring(filename));
   OPstack.push(DEBUG_NEW telldata::ttbool(x2048));

   return GDSexportTOPT::execute();
}

//=============================================================================
tellstdfunc::PSexportTOP::PSexportTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::GDSclose::execute() {
   browsers::clearGDStab();
   DATC->GDSclose();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTLAY::stdREPORTLAY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::stdREPORTLAY::execute() {
   bool recursive = getBoolValue();
   std::string cellname = getStringValue();
   laydata::ListOfWords ull;
   DATC->lockDB();
      bool success = DATC->TEDLIB()->collect_usedlays(cellname, recursive, ull);
   DATC->unlockDB();
   telldata::ttlist* tllull = DEBUG_NEW telldata::ttlist(telldata::tn_int);
   if (success) {
      ull.sort();ull.unique();
      std::ostringstream ost;
      ost << "used layers: {";
      for(laydata::ListOfWords::const_iterator CL = ull.begin() ; CL != ull.end();CL++ )
         ost << " " << *CL << " ";
      ost << "}";
      tell_log(console::MT_INFO, ost.str());

      for(laydata::ListOfWords::const_iterator CL = ull.begin() ; CL != ull.end();CL++ )
         tllull->add(DEBUG_NEW telldata::ttint(*CL));
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
      stdREPORTLAY(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::stdREPORTLAYc::execute() {
   bool recursive = getBoolValue();
   OPstack.push(DEBUG_NEW telldata::ttstring(""));
   OPstack.push(DEBUG_NEW telldata::ttbool(recursive));
   return stdREPORTLAY::execute();
}

//=============================================================================
tellstdfunc::GDSreportlay::GDSreportlay(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::GDSreportlay::execute()
{
   std::string name = getStringValue();
   GDSin::GdsFile* AGDSDB = DATC->lockGDS();
      GDSin::GdsStructure *src_structure = AGDSDB->getStructure(name.c_str());
      std::ostringstream ost; 
      if (!src_structure) {
         ost << "GDS structure named \"" << name << "\" does not exists";
         tell_log(console::MT_ERROR,ost.str());
      }
      else 
      {
         GdsLayers gdsLayers;
         src_structure->collectLayers(gdsLayers,true);
         ost << "GDS layers found in \"" << name <<"\" { <layer_number> ; <data_type> }" << std::endl;
         for (GdsLayers::const_iterator NLI = gdsLayers.begin(); NLI != gdsLayers.end(); NLI++)
         {
            ost << "{" << NLI->first << " ; ";
            for (WordList::const_iterator NTI = NLI->second.begin(); NTI != NLI->second.end(); NTI++)
               ost << *NTI << " ";
            ost << "}"<< std::endl;
         }
         tell_log(console::MT_INFO,ost.str());
         LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
      }
   DATC->unlockGDS();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFread::CIFread(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::CIFread::execute() {
   std::string filename = getStringValue();
   telldata::ttlist* topcells = DEBUG_NEW telldata::ttlist(telldata::tn_string);
   if (expandFileName(filename))
   {
      switch (DATC->CIFparse(filename))
      {
         case CIFin::cfs_POK:
         {
            // add CIF tab in the browser
            browsers::addCIFtab();
            // Collect the top structures
            std::list<std::string> top_cell_list;
            CIFin::CifFile* ACIFDB = DATC->lockCIF();
            CIFin::CIFHierTree* root = ACIFDB->hiertree()->GetFirstRoot(TARGETDB_LIB);
            assert(root);
            do
               top_cell_list.push_back(std::string(root->GetItem()->name()));
            while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
            DATC->unlockCIF();
            // Convert the string list to TLISTOF(telldata::tn_string)
            std::list<std::string>::const_iterator CN;
            for (CN = top_cell_list.begin(); CN != top_cell_list.end(); CN ++)
               topcells->add(DEBUG_NEW telldata::ttstring(*CN));
            // Push the top structures in the data stack
            LogFile << LogFile.getFN() << "(\""<< filename << "\");"; LogFile.flush();
            break;
         }
         case CIFin::cfs_FNF:
         {
            std::string info = "File \"" + filename + "\" not found or not readable";
            tell_log(console::MT_ERROR,info);
            break;
         }
         default:
         {
            std::string info = "File \"" + filename + "\" doesn't seem to appear a valid CIF file";
            tell_log(console::MT_ERROR,info);
         }
      }
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   OPstack.push(topcells);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::CIFgetLay::CIFgetLay(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{ }

int tellstdfunc::CIFgetLay::execute() {
   nameList cifLayers;
   telldata::ttlist* cifll = DEBUG_NEW telldata::ttlist(telldata::tn_hsh);
   if (DATC->CIFgetLay(cifLayers))
   {
      word laynum = 1;
      for (nameList::iterator NLI = cifLayers.begin(); NLI != cifLayers.end(); NLI++)
      {
         cifll->add(DEBUG_NEW telldata::tthsh(laynum++, *NLI));
      }
   }
   else
   {
      std::string info = "No CIF DB in memory. Parse first";
      tell_log(console::MT_ERROR,info);
   }
   OPstack.push(cifll);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFimportList::CIFimportList(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_string)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::CIFimportList::execute()
{
   bool  over  = getBoolValue();
   NMap* cifLays = DEBUG_NEW NMap();
   telldata::ttlist *ll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   // Convert layer map
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < ll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((ll->mlist())[i]);
      (*cifLays)[nameh->name().value()] = nameh->number().value();
   }
   // Convert top structure list
   nameList top_cells;
   for (unsigned i = 0; i < pl->size(); i++)
   {
      top_cells.push_back((static_cast<telldata::ttstring*>((pl->mlist())[i]))->value());
   }
   DATC->lockDB(false);
      DATC->CIFimport(top_cells, cifLays, over);
      updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
   DATC->unlockDB();
   // Don't refresh the tree browser here. See the comment in GDSimportAll::execute()

   LogFile << LogFile.getFN() << "(" << *pl << "," << *ll << "," << LogFile._2bool(over) << ");"; LogFile.flush();
   delete pl;
   delete ll;
   delete cifLays;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFimport::CIFimport(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::CIFimport::execute()
{
   bool  over  = getBoolValue();
   NMap* cifLays = DEBUG_NEW NMap();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   // Convert layer map
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->name().value()] = nameh->number().value();
   }
   // Convert top structure list
   nameList top_cells;
   top_cells.push_back(name.c_str());
   DATC->lockDB(false);
      DATC->CIFimport(top_cells, cifLays, over);
      updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
   DATC->unlockDB();
   // Don't refresh the tree browser here. See the comment in GDSimportAll::execute()

   LogFile << LogFile.getFN() << "(\"" << name<< "\"," << *lll << "," << LogFile._2bool(over) << ");"; LogFile.flush();
   delete lll;
   cifLays->clear();
   delete cifLays;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFexportLIB::CIFexportLIB(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::CIFexportLIB::execute()
{
   std::string filename = getStringValue();
   bool  verbose = getBoolValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   // Convert layer map
   USMap* cifLays = DEBUG_NEW USMap();
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->number().value()] = nameh->name().value();
   }
   if (expandFileName(filename))
   {
      DATC->lockDB(false);
      DATC->CIFexport(cifLays, verbose, filename);
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "(\""<< filename << "\"," << (*lll) << ");"; LogFile.flush();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   delete cifLays;
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFexportTOP::CIFexportTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::CIFexportTOP::execute()
{
   std::string filename = getStringValue();
   bool  verbose = getBoolValue();
   bool  recur = getBoolValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string cellname = getStringValue();

   // Convert layer map
   USMap* cifLays = DEBUG_NEW USMap();
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->number().value()] = nameh->name().value();
   }

   if (expandFileName(filename))
   {
      laydata::tdtcell *excell = NULL;
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         excell = ATDB->checkcell(cellname);
         if (NULL != excell)
         {
            DATC->CIFexport(excell, cifLays, recur, verbose, filename);
            LogFile << LogFile.getFN() << "(\""<< cellname << "\"," 
                                       << (*lll)            
                                       << LogFile._2bool(recur) 
                                       << LogFile._2bool(verbose) 
                                       << ",\"" << filename << "\");";
            LogFile.flush();
         }
         else
         {
            std::string message = "Cell " + cellname + " not found in the database";
            tell_log(console::MT_ERROR,message);
         }
      DATC->unlockDB();
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   cifLays->clear();
   delete cifLays;
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFclose::CIFclose(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::CIFclose::execute() {
   browsers::clearCIFtab();
   DATC->CIFclose();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   return EXEC_NEXT;
}


