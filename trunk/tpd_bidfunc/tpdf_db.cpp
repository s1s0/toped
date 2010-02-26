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
#include "datacenter.h"
#include "tuidefs.h"
#include "calbr_reader.h"
#include "drc_tenderer.h"
#include "viewprop.h"


extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;
extern Calbr::CalbrFile*         DRCData;

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
tellstdfunc::stdNEWDESIGNs::stdNEWDESIGNs(telldata::typeID retype, bool eor) :
      stdNEWDESIGNsd(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdNEWDESIGNs::execute()
{
   TpdTime timeCreated(time(NULL));
   OPstack.push(DEBUG_NEW telldata::ttstring(timeCreated()));
   return stdNEWDESIGNsd::execute();
}

//=============================================================================
tellstdfunc::stdNEWDESIGNsd::stdNEWDESIGNsd(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::stdNEWDESIGNsd::execute()
{
   TpdTime timeCreated(getStringValue());
   real UU = getOpValue();
   real DBU = getOpValue();
   std::string nm = getStringValue();

   DATC->newDesign(nm, timeCreated.stdCTime(), DBU, UU);
   TpdPost::addTDTtab(true, false);
   // reset UNDO buffers;
   UNDOcmdQ.clear();
   while (!UNDOPstack.empty())
   {
      delete UNDOPstack.front(); UNDOPstack.pop_front();
   }
   LogFile << "newdesign(\""<< nm            << "\" , \""
                            << DBU           << ", "
                            << UU            << ", "
                            << timeCreated() << "\");";
   LogFile.flush();
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
         laydata::TdtDesign* ATDB = DATC->lockDB(false);
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
         TpdPost::addTDTtab(true, true);
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
         laydata::TdtDesign* ATDB = DATC->lockDB(false);
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
         TpdPost::addTDTtab(true, true);
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
         laydata::TdtLibrary* LTDB = DATC->getLib(libID);
         // Gatering the used layers & update the layer definitions
         laydata::TDTHierTree* root = LTDB->hiertree()->GetFirstRoot(libID);
         do
         {
            top_cell_list.push_back(std::string(root->GetItem()->name()));
         } while (NULL != (root = root->GetNextRoot(libID)));
         updateLayerDefinitions(DATC->TEDLIB(), top_cell_list, libID);
         DATC->TEDLIB()->cleanUndefLib();
         // populating cell hierarchy browser
         TpdPost::addTDTtab(false, true);
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
      TpdPost::addTDTtab(false, true);
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
   laydata::TdtDesign* ATDB = DATC->lockDB(false);
      ATDB->tryUnselectAll();
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
      laydata::TdtDesign* ATDB = DATC->lockDB(false);
         ATDB->tryUnselectAll();
         bool stop_ignoring = false;
         if (DATC->TDTcheckwrite(timeCreated, timeSaved, stop_ignoring))
         {
            DATC->TDTwrite(DATC->tedFileName().c_str());
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
      laydata::TdtDesign* ATDB = DATC->lockDB(false);
         ATDB->tryUnselectAll();
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
         DATC->bpAddGdsTab();
         //
         GDSin::GdsInFile* AGDSDB = NULL;
         if (DATC->lockGds(AGDSDB))
         {
            GDSin::GDSHierTree* root = AGDSDB->hierTree()->GetFirstRoot(TARGETDB_LIB);
            if (root)
            {
               do
               {
                  top_cell_list.push_back(std::string(root->GetItem()->strctName()));
               } while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
            }
            //else ->it's possible to have an empty GDS file
         }
         else
         {
            // The AGDSDB mist exists here, because GDSparse returned true
            assert(false);
         }
         DATC->unlockGds(AGDSDB);
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
   // Prep: We need all used layers, and the name of the GDS DB
   std::ostringstream ost;
   std::string gdsDbName = "NonameDB";
   ExtLayers* gdsLaysAll = NULL;
   GDSin::GdsInFile* AGDSDB = NULL;
   if (DATC->lockGds(AGDSDB))
   {
      GDSin::GdsStructure *src_structure = AGDSDB->getStructure(name.c_str());
      if (src_structure)
      {
         gdsLaysAll = DEBUG_NEW ExtLayers();
         src_structure->collectLayers(*gdsLaysAll,true);
         gdsDbName = AGDSDB->libname();
      }
   }
   DATC->unlockGds(AGDSDB, true);
   //OK, here we go....
   if (NULL != gdsLaysAll)
   { // i.e. top structure is found and layers extracted
      LayerMapExt LayerExpression(gdsLaysStrList, gdsLaysAll);
      if (LayerExpression.status())
      {
         nameList top_cells;
         top_cells.push_back(name);

         try {DATC->lockDB(false);}
         catch (EXPTN)
         {  // create a default target data base if one is not already existing
            TpdTime timeCreated(time(NULL));
            createDefaultTDT(gdsDbName, timeCreated, UNDOcmdQ, UNDOPstack);
            DATC->lockDB(false);
         }
         DATC->importGDScell(top_cells, LayerExpression, recur, over);
            updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
         DATC->unlockDB();
         // populate the hierarchy browser
         TpdPost::addTDTtab(true, true);
         LogFile << LogFile.getFN() << "(\""<< name << "\"," << (*lll) << "," << LogFile._2bool(recur)
               << "," << LogFile._2bool(over) << ");"; LogFile.flush();
      }
      else
      {
         ost << "Can't execute GDS import - error in the layer map";
         tell_log(console::MT_ERROR,ost.str());
      }
   }
   else
   {
      ost << "GDS structure named \"" << name << "\" does not exists";
      tell_log(console::MT_ERROR,ost.str());
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
   ExtLayers* gdsLaysAll = DEBUG_NEW ExtLayers();
   std::string gdsDbName = "NonameDB";
   GDSin::GdsInFile* AGDSDB = NULL;
   if (DATC->lockGds(AGDSDB))
   {
      AGDSDB->collectLayers(*gdsLaysAll);
      gdsDbName = AGDSDB->libname();
   }
   DATC->unlockGds(AGDSDB, true);
   LayerMapExt LayerExpression(gdsLaysStrList, gdsLaysAll);
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
      // populate the hierarchy browser
      TpdPost::addTDTtab(true, true);
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
         LayerMapExt default_map(gdsLays, NULL);
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
      laydata::TdtCell *excell = NULL;
      laydata::TdtDesign* ATDB = DATC->lockDB(false);
         excell = static_cast<laydata::TdtCell*>(ATDB->checkCell(cellname));

         if (NULL != excell)
         {
            LayerMapExt default_map(gdsLays, NULL);

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
tellstdfunc::GDSsplit::GDSsplit(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::GDSsplit::execute()
{
   bool  recur = getBoolValue();
   std::string filename = getStringValue();
   std::string cellname = getStringValue();

   if (expandFileName(filename))
   {

      GDSin::GdsInFile* AGDSDB = NULL;
      if (DATC->lockGds(AGDSDB))
      {
         GDSin::GdsStructure *src_structure = AGDSDB->getStructure(cellname.c_str());
         std::ostringstream ost;
         if (!src_structure)
         {
            ost << "GDS structure named \"" << cellname << "\" does not exists";
            tell_log(console::MT_ERROR,ost.str());
         }
         else
         {
            GDSin::GdsSplit gdssplit(AGDSDB, filename);
            gdssplit.run(src_structure, recur);
            LogFile  << LogFile.getFN()
                     << "(\""<< cellname << "\","
                     << "\"" << filename << "\","
                     << LogFile._2bool(recur) << ");";
            LogFile.flush();
         }
      }
      DATC->unlockGds(AGDSDB, true);
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
      laydata::TdtCell *excell = NULL;
      laydata::TdtDesign* ATDB = DATC->lockDB(false);
         excell = static_cast<laydata::TdtCell*>(ATDB->checkCell(cellname));
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
   TpdPost::clearGDStab();
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
      bool success = DATC->TEDLIB()->collectUsedLays(cellname, recursive, ull);
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
   GDSin::GdsInFile* AGDSDB = NULL;
   if(DATC->lockGds(AGDSDB))
   {
      GDSin::GdsStructure *src_structure = AGDSDB->getStructure(name.c_str());
      std::ostringstream ost;
      if (!src_structure) {
         ost << "GDS structure named \"" << name << "\" does not exists";
         tell_log(console::MT_ERROR,ost.str());
      }
      else
      {
         ExtLayers gdsLayers;
         src_structure->collectLayers(gdsLayers,true);
         ost << "GDS layers found in \"" << name <<"\" { <layer_number> ; <data_type> }" << std::endl;
         for (ExtLayers::const_iterator NLI = gdsLayers.begin(); NLI != gdsLayers.end(); NLI++)
         {
            ost << "{" << NLI->first << " ; ";
            for (WordSet::const_iterator NTI = NLI->second.begin(); NTI != NLI->second.end(); NTI++)
               ost << *NTI << " ";
            ost << "}"<< std::endl;
         }
         tell_log(console::MT_INFO,ost.str());
         LogFile << LogFile.getFN() << "(\""<< name << "\");"; LogFile.flush();
      }
   }
   DATC->unlockGds(AGDSDB, true);
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
   const USMap* laymap = PROPC->getGdsLayMap();
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
      ExtLayers gdsLayers;
      DATC->gdsGetLayers(gdsLayers);
      for ( ExtLayers::const_iterator CGL = gdsLayers.begin(); CGL != gdsLayers.end(); CGL++ )
      {
         std::ostringstream dtypestr;
         dtypestr << CGL->first << ";";
         for ( WordSet::const_iterator CDT = CGL->second.begin(); CDT != CGL->second.end(); CDT++ )
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
      nameList tdtLayers;
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         drawProp->allLayers(tdtLayers);
         for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
         {
            std::ostringstream dtypestr;
            dtypestr << drawProp->getLayerNo( *CDL )<< "; 0";
            telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(drawProp->getLayerNo( *CDL ), dtypestr.str());
            theMap->add(clay);
         }
      }
      PROPC->unlockDrawProp(drawProp);
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
   PROPC->setGdsLayMap(gdsLays);

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

int tellstdfunc::CIFread::execute()
{
   std::string filename = getStringValue();
   telldata::ttlist* topcells = DEBUG_NEW telldata::ttlist(telldata::tn_string);
   if (expandFileName(filename))
   {
      switch (DATC->CIFparse(filename))
      {
         case CIFin::cfs_POK:
         {
            // add CIF tab in the browser
            DATC->bpAddCifTab();
            // Collect the top structures
            std::list<std::string> top_cell_list;
            CIFin::CifFile* ACIFDB = NULL;
            if (DATC->lockCif(ACIFDB))
            {
               CIFin::CIFHierTree* root = ACIFDB->hiertree()->GetFirstRoot(TARGETDB_LIB);
               assert(root);
               do
                  top_cell_list.push_back(std::string(root->GetItem()->name()));
               while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
            }
            else
            {
               // The ACIFDB mist exists here, because CIFparse returned cfs_POK
               assert(false);
            }
            DATC->unlockCif(ACIFDB);
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

int tellstdfunc::CIFreportlay::execute()
{

   std::string name = getStringValue();
   CIFin::CifFile* ACIFDB = NULL;
   if (DATC->lockCif(ACIFDB))
   {
      CIFin::CifStructure *src_structure = ACIFDB->getStructure(name.c_str());
      std::ostringstream ost;
      if (!src_structure)
      {
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
   }
   DATC->unlockCif(ACIFDB, true);
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
   DATC->CIFimport(top_cells, cifLays, recur, over, techno * PROPC->DBscale());
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
   DATC->CIFimport(top_cells, cifLays, recur, over, techno * PROPC->DBscale());
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
      laydata::TdtCell *excell = NULL;
      laydata::TdtDesign* ATDB = DATC->lockDB(false);
         excell = static_cast<laydata::TdtCell*>(ATDB->checkCell(cellname));
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
   TpdPost::clearCIFtab();
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
   const USMap* laymap = PROPC->getCifLayMap();
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
      nameList cifLayers;
      DATC->cifGetLayers(cifLayers);
      word laynum = 1;
      for ( nameList::const_iterator CCL = cifLayers.begin(); CCL != cifLayers.end(); CCL++ )
      {
         telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(laynum++, *CCL);
         theMap->add(clay);
      }
   }
   else
   { // generate default export CIF layer map
      nameList tdtLayers;
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         drawProp->allLayers(tdtLayers);
         for ( nameList::const_iterator CDL = tdtLayers.begin(); CDL != tdtLayers.end(); CDL++ )
         {
            std::ostringstream dtypestr;
            dtypestr << "L" << drawProp->getLayerNo( *CDL );
            telldata::tthsh* clay = DEBUG_NEW telldata::tthsh(drawProp->getLayerNo( *CDL ), dtypestr.str());
            theMap->add(clay);
         }
      }
      PROPC->unlockDrawProp(drawProp);
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
   PROPC->setCifLayMap(cifLays);

   LogFile << LogFile.getFN() << "("<< *lll  << ");"; LogFile.flush();
   delete lll;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::OASread::OASread(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::OASread::execute() {
   std::string filename = getStringValue();
   telldata::ttlist* topcells = DEBUG_NEW telldata::ttlist(telldata::tn_string);

   if (expandFileName(filename))
   {
      std::list<std::string> top_cell_list;
      if (DATC->OASParse(filename))
      {
         // add OASIS tab in the browser
         DATC->bpAddOasTab();
         //
         Oasis::OasisInFile* AOASDB = NULL;
         if (DATC->lockOas(AOASDB))
         {
            Oasis::OASHierTree* root = AOASDB->hierTree()->GetFirstRoot(TARGETDB_LIB);
            if (root)
            {
               do
               {
                  top_cell_list.push_back(std::string(root->GetItem()->name()));
               } while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
            }
            //else ->it's possible to have an empty OASIS file
         }
         else
         {
            // The AOASDB mist exists here, because OASISparse returned true
            assert(false);
         }
         DATC->unlockOas(AOASDB);
         for (std::list<std::string>::const_iterator CN = top_cell_list.begin();
                                                   CN != top_cell_list.end(); CN ++)
            topcells->add(DEBUG_NEW telldata::ttstring(*CN));
         LogFile << LogFile.getFN() << "(\""<< filename << "\");"; LogFile.flush();
      }
      else
      {
         //Error should've been already reported by the parser.
      }
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
tellstdfunc::OASimport::OASimport(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_hsh)));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttbool()));
}

int tellstdfunc::OASimport::execute()
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
   //Prep: We need all used layers, and the name of the GDS DB
   std::ostringstream ost;
   std::string oasDbName = "NonameDB";
   ExtLayers* oasLaysAll = NULL;
   Oasis::OasisInFile* AOASDB = NULL;
   if (DATC->lockOas(AOASDB))
   {
      Oasis::Cell *src_structure = AOASDB->getCell(name.c_str());
      if (src_structure)
      {
         oasLaysAll = DEBUG_NEW ExtLayers();
         src_structure->collectLayers(*oasLaysAll,true);
         oasDbName = AOASDB->getLibName();
      }
   }
   DATC->unlockOas(AOASDB, true);
   //OK, here we go....
   if (NULL != oasLaysAll)
   { // i.e. top structure is found and layers extracted
      LayerMapExt LayerExpression(gdsLaysStrList, oasLaysAll);
      if (LayerExpression.status())
      {
         nameList top_cells;
         top_cells.push_back(name);

         try {DATC->lockDB(false);}
         catch (EXPTN)
         {  // create a default target data base if one is not already existing
            TpdTime timeCreated(time(NULL));
            createDefaultTDT(oasDbName, timeCreated, UNDOcmdQ, UNDOPstack);
            DATC->lockDB(false);
         }
         DATC->importOAScell(top_cells, LayerExpression, recur, over);
            updateLayerDefinitions(DATC->TEDLIB(), top_cells, TARGETDB_LIB);
         DATC->unlockDB();
         // populate the hierarchy browser
         TpdPost::addTDTtab(true, true);
         LogFile << LogFile.getFN() << "(\""<< name << "\"," << /*(*lll) << "," <<*/ LogFile._2bool(recur)
               << "," << LogFile._2bool(over) << ");"; LogFile.flush();
      }
      else
      {
         ost << "Can't execute OASIS import - error in the layer map";
         tell_log(console::MT_ERROR,ost.str());
      }
   }
   else
   {
      ost << "OASIS structure named \"" << name << "\" does not exists";
      tell_log(console::MT_ERROR,ost.str());
   }
   delete lll;
   return EXEC_NEXT;
}
//=============================================================================
tellstdfunc::OASclose::OASclose(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::OASclose::execute() {
   TpdPost::clearOAStab();
   DATC->OASclose();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::DRCCalibreimport::DRCCalibreimport(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::DRCCalibreimport::execute()
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp, layprop::DRC))
   {
      drawProp->addLayer(DRC_LAY);
   }
   PROPC->unlockDrawProp(drawProp);
   std::string filename = getStringValue();
   if(DRCData)
   {
   }
   else
   {
      laydata::DrcLibrary* drcDesign = DATC->lockDRC();
      DRCData = DEBUG_NEW Calbr::CalbrFile(filename, new Calbr::drcTenderer(drcDesign));
      if(DRCData->isOk())
      {
         TpdPost::addDRCtab();
      }
      else
      {
         delete DRCData;
      }
      DATC->unlockDRC();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::DRCshowerror::DRCshowerror(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

int tellstdfunc::DRCshowerror::execute()
{
   long errorNumber = getWordValue();
   std::string errorName = getStringValue();
   DRCData->showError(errorName, errorNumber);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::DRCshowcluster::DRCshowcluster(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

int tellstdfunc::DRCshowcluster::execute()
{
   std::string errorName = getStringValue();
   DRCData->showCluster(errorName);
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::DRCshowallerrors::DRCshowallerrors(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
}

int tellstdfunc::DRCshowallerrors::execute()
{
	if(DRCData)
	{
	   DRCData->showAllErrors();
	}
   else
   {
      std::ostringstream ost;
      ost << "DRC database is not loaded";
      tell_log(console::MT_ERROR,ost.str());

   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::DRChideallerrors::DRChideallerrors(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
}

int tellstdfunc::DRChideallerrors::execute()
{
	if(DRCData)
	{
	   DRCData->hideAllErrors();
	}
   else
   {
      std::ostringstream ost;
      ost << "DRC database is not loaded";
      tell_log(console::MT_ERROR,ost.str());

   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::DRCexplainerror_D::DRCexplainerror_D(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

int tellstdfunc::DRCexplainerror_D::execute()
{
   // get the data from the stack
   assert(telldata::tn_pnt == OPstack.top()->get_type());
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   laydata::DrcLibrary* drcDesign = DATC->lockDRC();
      WordList selectedl = drcDesign->findSelected(p1DB);
      selectedl.unique();
      for(WordList::const_iterator it = selectedl.begin(); it!= selectedl.end(); ++it)
      {
         std::ostringstream ost;
         ost << DRCData->explainError((*it));
         tell_log(console::MT_INFO,ost.str());
      }
   DATC->unlockDRC();

   delete p1; delete p1DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::DRCexplainerror::DRCexplainerror(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype, eor)
{
}

int tellstdfunc::DRCexplainerror::execute()
{
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_point, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   assert(telldata::tn_pnt == OPstack.top()->get_type());
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   laydata::DrcLibrary* drcDesign = DATC->lockDRC();
      WordList selectedl = drcDesign->findSelected(p1DB);
      selectedl.unique();
      for(WordList::const_iterator it = selectedl.begin(); it!= selectedl.end(); ++it)
      {
         std::ostringstream ost;
         ost << DRCData->explainError((*it));
         tell_log(console::MT_INFO,ost.str());
      }
   DATC->unlockDRC();

   delete p1; delete p1DB;
   UpdateLV();
   return EXEC_NEXT;
}

