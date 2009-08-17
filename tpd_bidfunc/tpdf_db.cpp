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
   createDefaultTDT(nm, timeCreated, UNDOcmdQ, UNDOPstack);
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
         browsers::addTDTtab(true, true);
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
         browsers::addTDTtab(true, true);
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
         browsers::addTDTtab(false, true);
         // Clean-up eventual remainings in the themporary storage of the undefined cells
         DATC->TEDLIB()->deleteHeldCells();
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
      browsers::addTDTtab(false, true);
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

int tellstdfunc::TDTsave::execute()
{
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      ATDB->try_unselect_all();
      DATC->TDTwrite();
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
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         ATDB->try_unselect_all();
         bool stop_ignoring = false;
         if (DATC->TDTcheckwrite(timeCreated, timeSaved, stop_ignoring))
         {
            DATC->TDTwrite(DATC->tedfilename().c_str());
            TpdTime timec(ATDB->created());
            TpdTime timeu(ATDB->lastUpdated());
            LogFile << LogFile.getFN() << "(\"" <<  timec() << "\" , \"" <<
                  timeu() << "\");"; LogFile.flush();
         }
      DATC->unlockDB();
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

int tellstdfunc::TDTsaveas::execute()
{
   std::string filename = getStringValue();
   if (expandFileName(filename))
   {
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         ATDB->try_unselect_all();
         DATC->TDTwrite(filename.c_str());
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
   telldata::ttlist* topcells = DEBUG_NEW telldata::ttlist(telldata::tn_string);

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
               top_cell_list.push_back(std::string(root->GetItem()->strctName()));
            } while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
         DATC->unlockGDS();
         for (std::list<std::string>::const_iterator CN = top_cell_list.begin();
                                                   CN != top_cell_list.end(); CN ++)
            topcells->add(DEBUG_NEW telldata::ttstring(*CN));
         LogFile << LogFile.getFN() << "(\""<< filename << "\");"; LogFile.flush();
      }
//      else
//      {
//         Error should've been already reported by the parser. 
//      }
   }
   else
   {
      std::string info = "Filename \"" + filename + "\" can't be expanded properly";
      tell_log(console::MT_ERROR,info);
   }
   // Make sure you always return what you have to return - a list in this case even if
   // it's empty. Otherwise the following tell function will crash, because it can't
   // retrieve from the operand stack the required number of parameters. Empty list
   // is still a list and everybody should deal with them
   OPstack.push(topcells);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSimport::GDSimport(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSimport::execute()
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
      gdsLaysStrList[nameh->key().value()] = nameh->value().value();
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
      GdsLayers* gdsLaysAll = DEBUG_NEW GdsLayers();
      src_structure->collectLayers(*gdsLaysAll,true);
      std::string gdsDbName = AGDSDB->libname();
      DATC->unlockGDS();
      LayerMapGds LayerExpression(gdsLaysStrList, gdsLaysAll);
      if (LayerExpression.status())
      {
         nameList top_cells;
         top_cells.push_back(name);

         try {DATC->lockDB(false);}
         catch (EXPTN) 
         {
            // create a default target data base if one is not already existing
            TpdTime timeCreated(time(NULL));
            createDefaultTDT(gdsDbName, timeCreated, UNDOcmdQ, UNDOPstack);
            DATC->lockDB(false);
         }
         DATC->importGDScell(top_cells, LayerExpression, recur, over);
            updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
         DATC->unlockDB();
         LogFile << LogFile.getFN() << "(\""<< name << "\"," << (*lll) << "," << LogFile._2bool(recur)
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
tellstdfunc::GDSimportList::GDSimportList(telldata::typeID retype, bool eor) :
                              cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor) 
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_string)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSimportList::execute()
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
      gdsLaysStrList[nameh->key().value()] = nameh->value().value();
   }
   GdsLayers* gdsLaysAll = DEBUG_NEW GdsLayers();
   GDSin::GdsFile* AGDSDB = DATC->lockGDS();
      AGDSDB->collectLayers(*gdsLaysAll);
      std::string gdsDbName = AGDSDB->libname();
   DATC->unlockGDS();
   LayerMapGds LayerExpression(gdsLaysStrList, gdsLaysAll);
   if (LayerExpression.status())
   {
      try {DATC->lockDB(false);}
      catch (EXPTN) 
      {
         // create a default target data base if one is not already existing
         TpdTime timeCreated(time(NULL));
         createDefaultTDT(gdsDbName, timeCreated, UNDOcmdQ, UNDOPstack);
         DATC->lockDB(false);
      }
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
tellstdfunc::GDSexportLIB::GDSexportLIB(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSexportLIB::execute()
{
   bool x2048           = getBoolValue();
   std::string filename = getStringValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();

   // Convert layer map
   USMap gdsLays;
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      gdsLays[nameh->key().value()] = nameh->value().value();
   }

   if (expandFileName(filename))
   {
      DATC->lockDB(false);
         LayerMapGds default_map(gdsLays, NULL);
         DATC->GDSexport(default_map, filename, x2048);
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "( "
              << *lll << ", "
              << "\""<< filename << "\", "
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
tellstdfunc::GDSexportTOP::GDSexportTOP(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSexportTOP::execute()
{
   bool  x2048 = getBoolValue();
   std::string filename = getStringValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   bool  recur = getBoolValue();
   std::string cellname = getStringValue();

   // Convert layer map
   USMap gdsLays;
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      gdsLays[nameh->key().value()] = nameh->value().value();
   }
   if (expandFileName(filename))
   {
      laydata::tdtcell *excell = NULL;
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         excell = static_cast<laydata::tdtcell*>(ATDB->checkcell(cellname));

         if (NULL != excell)
         {
            LayerMapGds default_map(gdsLays, NULL);

            DATC->GDSexport(excell, default_map, recur, filename, x2048);
            LogFile  << LogFile.getFN() 
                     << "(\""<< cellname << "\"," 
                     << LogFile._2bool(recur) << ", "
                     << *lll << ", "
                     << "\"" << filename << "\","
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
         excell = static_cast<laydata::tdtcell*>(ATDB->checkcell(cellname));
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
   WordList ull;
   DATC->lockDB(false);
      bool success = DATC->TEDLIB()->collect_usedlays(cellname, recursive, ull);
   DATC->unlockDB();
   telldata::ttlist* tllull = DEBUG_NEW telldata::ttlist(telldata::tn_int);
   if (success) {
      ull.sort();ull.unique();
      std::ostringstream ost;
      ost << "used layers: {";
      for(WordList::const_iterator CL = ull.begin() ; CL != ull.end();CL++ )
         ost << " " << *CL << " ";
      ost << "}";
      tell_log(console::MT_INFO, ost.str());

      for(WordList::const_iterator CL = ull.begin() ; CL != ull.end();CL++ )
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
tellstdfunc::GDSgetlaymap::GDSgetlaymap(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSgetlaymap::execute()
{
   bool import = getBoolValue();
   telldata::ttlist* theMap = DEBUG_NEW telldata::ttlist(telldata::tn_hsh);
   const USMap* laymap = DATC->getGdsLayMap();
   if (NULL != laymap)
   {
      for (USMap::const_iterator CI = laymap->begin(); CI != laymap->end(); CI++)
      {
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(CI->first, CI->second);
         theMap->add(clay);
      }
   }
   else if (import)
   { // generate default import GDS layer map
      DATC->lockGDS();
      GdsLayers gdsLayers;
      DATC->gdsGetLayers(gdsLayers);
      DATC->unlockGDS();
      for ( GdsLayers::const_iterator CGL = gdsLayers.begin(); CGL != gdsLayers.end(); CGL++ )
      {
         std::ostringstream dtypestr;
         dtypestr << CGL->first << ";";
         for ( WordList::const_iterator CDT = CGL->second.begin(); CDT != CGL->second.end(); CDT++ )
         {
            if ( CDT != CGL->second.begin() ) dtypestr << ", ";
            dtypestr << *CDT;
         }
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(CGL->first, dtypestr.str());
         theMap->add(clay);
      }
   }
   else
   { // generate default export GDS layer map
      DATC->lockDB(false);
      nameList tdtLayers;
      DATC->all_layers(tdtLayers);
      for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
      {
         std::ostringstream dtypestr;
         dtypestr << DATC->getLayerNo( *CDL )<< "; 0";
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(DATC->getLayerNo( *CDL ), dtypestr.str());
         theMap->add(clay);
      }
      DATC->unlockDB();
   }
   OPstack.push(theMap);
   LogFile << LogFile.getFN() << "("<< LogFile._2bool(import)  << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::GDSsetlaymap::GDSsetlaymap(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
}

int tellstdfunc::GDSsetlaymap::execute()
{
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();

   // Convert layer map
   USMap* gdsLays = DEBUG_NEW USMap();
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*gdsLays)[nameh->key().value()] = nameh->value().value();
   }
   DATC->setGdsLayMap(gdsLays);

   LogFile << LogFile.getFN() << "("<< *lll  << ");"; LogFile.flush();
   delete lll;
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
tellstdfunc::CIFreportlay::CIFreportlay(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::CIFreportlay::execute() {

   std::string name = getStringValue();
   CIFin::CifFile* ACIFDB = DATC->lockCIF();
   CIFin::CifStructure *src_structure = ACIFDB->getStructure(name.c_str());
   std::ostringstream ost;
   if (!src_structure) {
      ost << "CIF structure named \"" << name << "\" does not exists";
      tell_log(console::MT_ERROR,ost.str());
   }
   else
   {
      nameList cifLayers;
      src_structure->collectLayers(cifLayers,true);
      ost << "CIF layers found in \"" << name <<"\"" << std::endl;
      for (nameList::iterator NLI = cifLayers.begin(); NLI != cifLayers.end(); NLI++)
         ost << *NLI << std::endl;
      tell_log(console::MT_INFO,ost.str());
      LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
   }
   DATC->unlockCIF();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFimportList::CIFimportList(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_string)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::CIFimportList::execute()
{
   real techno = getOpValue();
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   SIMap* cifLays = DEBUG_NEW SIMap();
   telldata::ttlist *ll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   // Convert layer map
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < ll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((ll->mlist())[i]);
      (*cifLays)[nameh->value().value()] = nameh->key().value();
   }
   // Convert top structure list
   nameList top_cells;
   for (unsigned i = 0; i < pl->size(); i++)
   {
      top_cells.push_back((static_cast<telldata::ttstring*>((pl->mlist())[i]))->value());
   }
   try {DATC->lockDB(false);}
   catch (EXPTN)
   {
      // create a default target data base if one is not already existing
      TpdTime timeCreated(time(NULL));
      createDefaultTDT("CIF_default", timeCreated, UNDOcmdQ, UNDOPstack);
      DATC->lockDB(false);
   }
   DATC->CIFimport(top_cells, cifLays, recur, over, techno * DATC->DBscale());
   updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
   DATC->unlockDB();
   // Don't refresh the tree browser here. See the comment in GDSimportAll::execute()

   LogFile << LogFile.getFN() << "(" << *pl << ","
           << *ll                   << ","
           << LogFile._2bool(recur) << ","
           << LogFile._2bool(over ) << ","
           << techno                << ");";

   LogFile.flush();
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
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::CIFimport::execute()
{
   real techno = getOpValue();
   bool  over  = getBoolValue();
   bool  recur = getBoolValue();
   SIMap* cifLays = DEBUG_NEW SIMap();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   std::string name = getStringValue();
   // Convert layer map
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->value().value()] = nameh->key().value();
   }
   // Convert top structure list
   nameList top_cells;
   top_cells.push_back(name.c_str());
   try {DATC->lockDB(false);}
   catch (EXPTN)
   {
      // create a default target data base if one is not already existing
      TpdTime timeCreated(time(NULL));
      createDefaultTDT("CIF_default", timeCreated, UNDOcmdQ, UNDOPstack);
      DATC->lockDB(false);
   }
   DATC->CIFimport(top_cells, cifLays, recur, over, techno * DATC->DBscale());
   updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
   DATC->unlockDB();
   // Don't refresh the tree browser here. See the comment in GDSimportAll::execute()

   LogFile << LogFile.getFN() << "(\"" << name<< "\","
           << *lll                  << ","
           << LogFile._2bool(recur) << ","
           << LogFile._2bool(over)  << ","
           << techno                << ");";
   LogFile.flush();
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
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::CIFexportLIB::execute()
{
   bool  verbose = getBoolValue();
   std::string filename = getStringValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   // Convert layer map
   USMap* cifLays = DEBUG_NEW USMap();
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->key().value()] = nameh->value().value();
   }
   if (expandFileName(filename))
   {
      DATC->lockDB(false);
      DATC->CIFexport(cifLays, verbose, filename);
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "( "
              << (*lll) << ", \""
              << filename << "\", "
              << LogFile._2bool(verbose)
              << " );"; LogFile.flush();
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
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::CIFexportTOP::execute()
{
   bool  verbose = getBoolValue();
   std::string filename = getStringValue();
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   bool  recur = getBoolValue();
   std::string cellname = getStringValue();

   // Convert layer map
   USMap* cifLays = DEBUG_NEW USMap();
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->key().value()] = nameh->value().value();
   }

   if (expandFileName(filename))
   {
      laydata::tdtcell *excell = NULL;
      laydata::tdtdesign* ATDB = DATC->lockDB(false);
         excell = static_cast<laydata::tdtcell*>(ATDB->checkcell(cellname));
         if (NULL != excell)
         {
            DATC->CIFexport(excell, cifLays, recur, verbose, filename);
            LogFile << LogFile.getFN() << "( \""
                    << cellname << "\", " 
                    << LogFile._2bool(recur) << ", "
                    << (*lll) << ", \""
                    << filename << "\", "
                    << LogFile._2bool(verbose) << ");";
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


//=============================================================================
tellstdfunc::CIFgetlaymap::CIFgetlaymap(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::CIFgetlaymap::execute()
{
   bool import = getBoolValue();
   telldata::ttlist* theMap = DEBUG_NEW telldata::ttlist(telldata::tn_hsh);
   const USMap* laymap = DATC->getCifLayMap();
   if (NULL != laymap)
   {
      for (USMap::const_iterator CI = laymap->begin(); CI != laymap->end(); CI++)
      {
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(CI->first, CI->second);
         theMap->add(clay);
      }
   }
   else if (import)
   { // generate default import CIF layer map
      DATC->lockCIF();
      nameList cifLayers;
      DATC->CIFgetLay(cifLayers);
      DATC->unlockCIF();
      word laynum = 1;
      for ( nameList::const_iterator CCL = cifLayers.begin(); CCL != cifLayers.end(); CCL++ )
      {
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(laynum++, *CCL);
         theMap->add(clay);
      }
   }
   else
   { // generate default export CIF layer map
      DATC->lockDB(false);
      nameList tdtLayers;
      DATC->all_layers(tdtLayers);
      for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
      {
         std::ostringstream dtypestr;
         dtypestr << "L" << DATC->getLayerNo( *CDL );
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(DATC->getLayerNo( *CDL ), dtypestr.str());
         theMap->add(clay);
      }
      DATC->unlockDB();
   }
   OPstack.push(theMap);
   LogFile << LogFile.getFN() << "("<< LogFile._2bool(import)  << ");"; LogFile.flush();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::CIFsetlaymap::CIFsetlaymap(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
}

int tellstdfunc::CIFsetlaymap::execute()
{
   telldata::ttlist *lll = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();

   // Convert layer map
   USMap* cifLays = DEBUG_NEW USMap();
   telldata::tthsh* nameh;
   for (unsigned i = 0; i < lll->size(); i++)
   {
      nameh = static_cast<telldata::tthsh*>((lll->mlist())[i]);
      (*cifLays)[nameh->key().value()] = nameh->value().value();
   }
   DATC->setCifLayMap(cifLays);

   LogFile << LogFile.getFN() << "("<< *lll  << ");"; LogFile.flush();
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
void tellstdfunc::createDefaultTDT(std::string dbname, TpdTime& timeCreated,
                                   parsercmd::undoQUEUE& undstack, telldata::UNDOPerandQUEUE& undopstack)
{
   DATC->newDesign(dbname, timeCreated.stdCTime());
   browsers::addTDTtab(true, false);
   // reset UNDO buffers;
   undstack.clear();
   while (!undopstack.empty())
   {
      delete undopstack.front(); undopstack.pop_front();
   }
   LogFile << "newdesign(\""<< dbname << "\" , \"" << timeCreated() <<
         "\");"; LogFile.flush();
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
   ATDB->btreeAddMember    = &browsers::treeAddMember;
   ATDB->btreeRemoveMember = &browsers::treeRemoveMember;
   DATC->unlockDB();
}


