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
//        Created: Sun Apr 18 10:59:37 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: The top class in the layout database
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "ttt.h"
#include "outbox.h"
#include "tedesign.h"
#include "tedat.h"
#include "viewprop.h"
#include "tenderer.h"

extern layprop::PropertyCenter*  PROPC;

//! the stack of all previously edited (opened) cells
laydata::EditCellStack      laydata::EditObject::_editstack;

// initializing the static variables
laydata::TDTHierTree* laydata::TdtLibrary::_hiertree = NULL;

//-----------------------------------------------------------------------------
// class TdtLibrary
//-----------------------------------------------------------------------------
laydata::TdtLibrary::TdtLibrary(std::string name, real DBU, real UU, int libID,
                                time_t created, time_t lastUpdated) :
   _name         ( name        ),
   _libID        ( libID       ),
   _DBU          ( DBU         ),
   _UU           ( UU          ),
   _created      ( created     ),
   _lastUpdated  ( lastUpdated )

{}

//void laydata::TdtLibrary::unloadprep(laydata::TdtLibDir* libdir)
//{
//   laydata::CellList::const_iterator wc;
//   for (wc = _cells.begin(); wc != _cells.end(); wc++)
//     if (!wc->second->orphan())
//      {
////         laydata::TdtDefaultCell* boza = new laydata::TdtDefaultCell(wc->first, 0, false);
//         _hiertree->replaceChild(wc->second, libdir->addDefaultCell(wc->first), _hiertree);
//      }
//}

void laydata::TdtLibrary::relink(TdtLibDir* libdir)
{
   laydata::CellMap::iterator wc;
   bool need_validation = false;
   for (wc = _cells.begin(); wc != _cells.end(); wc++)
   {
      assert(wc->second);
      need_validation |= wc->second->relink(libdir);
   }
   if (need_validation)
      do {} while(validateCells());
}

laydata::TdtLibrary::~TdtLibrary()
{
   clearLib();
}

void laydata::TdtLibrary::clearHierTree()
{
   // get rid of the hierarchy tree
   const TDTHierTree* var1 = _hiertree;
   _hiertree = NULL;
   TDTHierTree* lastValid = NULL;
   while (var1)
   {
      const TDTHierTree* var2 = var1->GetLast();
      if (var1->itemRefdIn(_libID))
      {
         if (NULL != lastValid)
            lastValid->relink(var1);
         delete var1;
      }
      else
      {
         if (NULL != lastValid)
            lastValid = const_cast<TDTHierTree*>(var1);
         else
            _hiertree = lastValid = const_cast<TDTHierTree*>(var1);
      }
      var1 = var2;
   }
}

void laydata::TdtLibrary::clearEntireHierTree()
{
   // get rid of the hierarchy tree
   const TDTHierTree* var1 = _hiertree;
   while (var1)
   {
      const TDTHierTree* var2 = var1->GetLast();
      delete var1; var1 = var2;
   }
   _hiertree = NULL;
}

void laydata::TdtLibrary::read(InputTdtFile* const tedfile)
{
   std::string cellname;
   while (tedf_CELL == tedfile->getByte())
   {
      cellname = tedfile->getString();
      tell_log(console::MT_CELLNAME, cellname);
      registerCellRead(cellname, DEBUG_NEW TdtCell(tedfile, cellname, _libID));
   }
   recreateHierarchy(tedfile->TEDLIB());
   tell_log(console::MT_INFO, "Done");
}

void laydata::TdtLibrary::registerCellRead(std::string cellname, TdtCell* cell) {
   if (_cells.end() != _cells.find(cellname))
   {
   // There are several possibilities here:
   // 1. Cell has been referenced before the definition takes place
   // 2. The same case 1, but the reason is circular reference.
   // 3. Cell is defined more than once
   // Case 3 seems to be just theoretical and we should abort the reading
   // and retun with error in this case.
   // Case 2 is really dangerous and once again theoretically we need to
   // break the circularity. This might happen however once the whole file
   // is parced
   // Case 1 is quite OK, although, the write sequence should use the
   // cell structure tree and start the writing from the leaf cells. At the
   // moment writing is in kind of alphabetical order and case 1 is more
   // than possible. In the future it might me appropriate to issue a warning
   // for possible circular reference.
      if (NULL == _cells[cellname]) {
         // case 1 or case 2 -> can't be distiguised in this moment
         //_cells[cellname] = cell;
         // cell has been referenced already, so it's not an orphan
         cell->setOrphan(false);
      }
      else {
         //@FIXME case 3 -> parsing should be stopped !
      }
   }
   _cells[cellname] = cell;
}

void laydata::TdtLibrary::dbExport(DbExportFile& exportF)
{
   TpdTime timelu(_lastUpdated);
   exportF.libraryStart(name(), timelu, DBU(), UU());
   //
   if (NULL == exportF.topcell())
   {
      laydata::TDTHierTree* root_cell = _hiertree->GetFirstRoot(TARGETDB_LIB);
      while (root_cell)
      {
         _cells[root_cell->GetItem()->name()]->dbExport(exportF, _cells, root_cell);
         root_cell = root_cell->GetNextRoot(TARGETDB_LIB);
      }
   }
   else
   {
      laydata::TDTHierTree* root_cell = _hiertree->GetMember(exportF.topcell());
      exportF.topcell()->dbExport(exportF, _cells, root_cell);
   }
   exportF.libraryFinish();
}

laydata::TdtDefaultCell* laydata::TdtLibrary::checkCell(std::string name, bool undeflib)
{
   if ((!undeflib && (UNDEFCELL_LIB == _libID)) || (_cells.end() == _cells.find(name)))
      return NULL;
   else return _cells[name];
}

void laydata::TdtLibrary::recreateHierarchy(const laydata::TdtLibDir* libdir)
{
   if (TARGETDB_LIB == _libID)
   {
      clearHierTree();
   }
   // here - run the hierarchy extraction on orphans only
   for (laydata::CellMap::const_iterator wc = _cells.begin();
                                          wc != _cells.end(); wc++) {
      if (wc->second && wc->second->orphan())
         _hiertree = wc->second->hierOut(_hiertree, NULL, &_cells, libdir);
   }
}

laydata::CellDefin laydata::TdtLibrary::getCellNamePair(std::string name) const
{
   CellMap::const_iterator striter = _cells.find(name);
   if (_cells.end() == striter)
      return NULL;
   else
      return striter->second;
}


laydata::CellDefin laydata::TdtLibrary::secureDefaultCell(std::string name, bool updateHier)
{
   assert(UNDEFCELL_LIB == _libID);
   if (_cells.end() == _cells.find(name))
   {
      TdtDefaultCell* newcell = DEBUG_NEW TdtDefaultCell(name, UNDEFCELL_LIB, true);
      _cells[name] = newcell;
      if (updateHier)
         _hiertree = DEBUG_NEW TDTHierTree(newcell, NULL, _hiertree);
   }
   return _cells[name];
}

void laydata::TdtLibrary::addThisUndefCell(laydata::TdtDefaultCell* thecell)
{
   assert(UNDEFCELL_LIB == _libID);
   assert(_cells.end() == _cells.find(thecell->name()));
   _cells[thecell->name()] = thecell;
   _hiertree = DEBUG_NEW TDTHierTree(thecell, NULL, _hiertree);
}


bool laydata::TdtLibrary::validateCells()
{
   bool invalidParents = false;
   laydata::CellMap::const_iterator wc;
   for (wc = _cells.begin(); wc != _cells.end(); wc++)
   {
      if (NULL != wc->second)
         invalidParents |= static_cast<TdtCell*>(wc->second)->validateCells(this);
   }
   return invalidParents;
}

void laydata::TdtLibrary::clearLib()
{
   laydata::CellMap::const_iterator wc;
   for (wc = _cells.begin(); wc != _cells.end(); wc++)
      delete wc->second;
   _cells.clear();
}

void laydata::TdtLibrary::cleanUnreferenced()
{
   laydata::CellMap::iterator wc = _cells.begin();
   while (wc != _cells.end())
   {
      const TDTHierTree* hcell = _hiertree->GetMember(wc->second);
      if ((NULL != hcell) && (NULL == hcell->Getparent()))
      {
         _hiertree->removeRootItem(wc->second, _hiertree);
         delete wc->second;
         laydata::CellMap::iterator wcd = wc++;
         _cells.erase(wcd);
      }
      else wc++;
   }
}

/*! removes a cell /cell_name from this library. Updates the hierarchy tree. The
cell is not deleted, but returned by the function.
Cell must not be referenced otherwise the _hiertree will assert. Primary usage
is to clear a default cell from the library of undefined cells when a new cell
is created in the target DB with the same name.
There is no another valid usage, so the function will assert if called for
libraries other than UNDEFCELL_LIB.
*/
laydata::TdtDefaultCell* laydata::TdtLibrary::displaceCell(const std::string& cell_name)
{
   assert(UNDEFCELL_LIB == _libID); // see the function comment above!
   laydata::CellMap::iterator wc = _cells.find(cell_name);
   if (_cells.end() == wc) return NULL;
   laydata::TdtDefaultCell* celldef = wc->second;
   _hiertree->removeRootItem(celldef, _hiertree);
   _cells.erase(wc);
   return celldef;
}

void laydata::TdtLibrary::collectUsedLays(LayerDefList& laylist) const
{
   for (CellMap::const_iterator CC = _cells.begin(); CC != _cells.end(); CC++)
   {
      CC->second->collectUsedLays(NULL, false,laylist);
   }
   laylist.sort();
   laylist.unique();
//   if ( (0 < laylist.size()) && (0 == laylist.front()) )
//      laylist.pop_front();
}

void laydata::TdtLibrary::dbHierAdd(const laydata::TdtDefaultCell* comp, const laydata::TdtDefaultCell* prnt)
{
   assert(comp);
   _hiertree = DEBUG_NEW TDTHierTree(comp, prnt, _hiertree);
   switch (comp->libID())
   {
      case TARGETDB_LIB:
      {
         std::string prnt_name = (NULL == prnt) ? _name : prnt->name();
         TpdPost::treeAddMember(comp->name().c_str(), prnt_name.c_str(), 0);
         break;
      }
      case UNDEFCELL_LIB:
      {
         std::string prnt_name = "";
         TpdPost::treeAddMember(comp->name().c_str(), prnt_name.c_str(), 0);
         break;
      }
      default: assert(false); break;
   }
}

void laydata::TdtLibrary::dbHierAddParent(const laydata::TdtDefaultCell* comp, const laydata::TdtDefaultCell* prnt)
{
   assert(comp); assert(prnt);
   int res = _hiertree->addParent(comp, prnt, _hiertree);
   if (res > 0)
      TpdPost::treeAddMember(comp->name().c_str(), prnt->name().c_str(), res);
}

void laydata::TdtLibrary::dbHierRemoveParent(TdtDefaultCell* comp, const TdtDefaultCell* prnt, laydata::TdtLibDir* libdir)
{
   assert(comp); assert(prnt);
   int res = _hiertree->removeParent(comp, prnt, _hiertree);
   if ((1 == res) && (UNDEFCELL_LIB == comp->libID()))
   {
      // if that cell was undefined - remove it from the library of undefined cells
      laydata::TdtDefaultCell* libcellX = libdir->displaceUndefinedCell(comp->name());
      assert(comp == libcellX);
      TpdPost::treeRemoveMember(comp->name().c_str(), prnt->name().c_str(), res);
      TpdPost::treeRemoveMember(comp->name().c_str(), prnt->name().c_str(), 4);
      libdir->holdUndefinedCell(libcellX);
   }
   else if ( 3 != res)
   {
      TpdPost::treeRemoveMember(comp->name().c_str(), prnt->name().c_str(), res);
      comp->setOrphan(res>0);
   }
}

void laydata::TdtLibrary::dbHierRemoveRoot(const TdtDefaultCell* comp)
{
   assert(comp);
   _hiertree->removeRootItem(comp, _hiertree);
   TpdPost::treeRemoveMember(comp->name().c_str(), NULL, 3);
}

bool laydata::TdtLibrary::dbHierCheckAncestors(const TdtDefaultCell* comp, const TdtDefaultCell* child)
{
   assert(comp); assert(child);
   return _hiertree->checkAncestors(comp, child, _hiertree);
}

//-----------------------------------------------------------------------------
// class TdtLibDir
//-----------------------------------------------------------------------------
laydata::TdtLibDir::TdtLibDir()
{
   time_t timeNow = time(NULL);
   // create the default library of unknown cells
   TdtLibrary* undeflib = DEBUG_NEW TdtLibrary("__UNDEFINED__", 1e-9, 1e-3, UNDEFCELL_LIB, timeNow, timeNow);
   _libdirectory.insert( _libdirectory.end(), DEBUG_NEW LibItem("__UNDEFINED__", undeflib) );
   // toped data base
   _TEDDB = NULL;
   // default name of the target DB
   _tedFileName = "unnamed";
   // marker whether the target DB has been saved ever
   _neverSaved = true;
}

laydata::TdtLibDir::~TdtLibDir()
{
   for (word i = 0; i < _libdirectory.size(); i++)
   {
      delete _libdirectory[i]->second;
      delete _libdirectory[i];
   }
   if (NULL != _TEDDB) delete _TEDDB;
}

int laydata::TdtLibDir::getLastLibRefNo()
{
   return _libdirectory.size();
}

void laydata::TdtLibDir::addLibrary(TdtLibrary* const lib, word libRef)
{
   assert(libRef == _libdirectory.size());
   _libdirectory.insert( _libdirectory.end(), DEBUG_NEW LibItem(lib->name(), lib) );
}

int laydata::TdtLibDir::loadLib(std::string filename)
{
   InputTdtFile tempin(wxString(filename.c_str(), wxConvUTF8), this);
   if (!tempin.status()) return -1;
   int libRef = getLastLibRefNo();
   try
   {
      tempin.read(libRef);
   }
   catch (EXPTNreadTDT&)
   {
      tempin.closeStream();
      tempin.cleanup();
      return -1;
   }
   tempin.closeStream();
   addLibrary(tempin.design(), libRef);
   relink();// Re-link everything
   return libRef;
}

bool laydata::TdtLibDir::unloadLib(std::string libname)
{
   // Unhook the library from the list of known DB's, get a pointer to it
   laydata::TdtLibrary* tberased = removeLibrary(libname);
   if ( NULL != tberased )
   {
      // Relink everything
      relink();
      // remove tberased cells from hierarchy tree
      tberased->clearHierTree();
      // get the new hierarchy
      reextractHierarchy();
      // after all above - remove the library
      delete tberased;
      return true;
   }
   else return false;
}

laydata::TdtLibrary* laydata::TdtLibDir::removeLibrary(std::string libname)
{
   TdtLibrary* tberased = NULL;
   for (LibCatalog::iterator LDI = _libdirectory.begin(); LDI != _libdirectory.end(); LDI++)
   {
      if (libname == (*LDI)->first)
      {
         tberased = (*LDI)->second;
         _libdirectory.erase(LDI);
         break;
      }
   }
   return tberased;
}

laydata::TdtLibrary* laydata::TdtLibDir::getLib(int libID)
{
   assert(libID); // make sure that nobody asks for the default library
   assert(libID <= (int)_libdirectory.size());
   return _libdirectory[libID]->second;
}

std::string laydata::TdtLibDir::getLibName(int libID)
{
   assert(libID); // make sure that nobody asks for the default library
   assert(libID <= (int)_libdirectory.size());
   return _libdirectory[libID]->first;
}

void laydata::TdtLibDir::newDesign(std::string name, std::string dir, time_t created, real DBU, real UU)
{
   if (NULL != _TEDDB)
   {
      // Checks before closing(save?) available only when the command is launched
      // via GUI(void TopedFrame::OnNewDesign(). If the command is typed directly
      // on the command line, or parsed from file - no checks are executed.
      // In other words if we are already here we will destroy the current design
      // without much talking.
      // UNDO buffers will be reset as well in tellstdfunc::stdNEWDESIGN::execute()
      // but there is still a chance to restore everything - using the log file.
      _TEDDB->clearHierTree();
      delete _TEDDB;
   }
   _TEDDB = DEBUG_NEW laydata::TdtDesign(name, created, created, DBU, UU);
   _tedFileName = dir + name + ".tdt";
   _neverSaved = true;
   PROPC->setUU(_TEDDB->UU());
}

bool laydata::TdtLibDir::readDesign(std::string filename)
{
   InputTdtFile tempin(wxString(filename.c_str(), wxConvUTF8), this);
   if (!tempin.status()) return false;

   try
   {
      tempin.read(TARGETDB_LIB);
   }
   catch (EXPTNreadTDT&)
   {
      tempin.closeStream();
      tempin.cleanup();
      return false;
   }
   tempin.closeStream();
   delete _TEDDB;//Erase existing data
   _tedFileName = filename;
   _neverSaved = false;
   _TEDDB = static_cast<laydata::TdtDesign*>(tempin.design());
   // Update Canvas scale
   PROPC->setUU(_TEDDB->UU());
   return true;
}

void laydata::TdtLibDir::writeDesign(const char* filename)
{
   if (filename)  _tedFileName = filename;
   OutputTdtFile tempin(_tedFileName, this);
   _neverSaved = false;
}

bool laydata::TdtLibDir::TDTcheckwrite(const TpdTime& timeCreated, const TpdTime& timeSaved, bool& stop_ignoring)
{
   if (NULL == _TEDDB) return false;
   std::string news;
   stop_ignoring = false;
   // File created time stamp must match exactly, otherwise it means
   // that we're saving not exactly the same file that is requested
   if (timeCreated.stdCTime() != _TEDDB->created())
   {
      news = "time stamp \"Project created \" doesn't match. File save aborted";
      tell_log(console::MT_ERROR,news);
      return false;
   }
   if (_TEDDB->lastUpdated() < timeSaved.stdCTime())
   {
      news = "Database in memory is older than the file. File save operation ignored.";
      tell_log(console::MT_WARNING,news);
      _neverSaved = false;
      return false;
   }
   else if (_TEDDB->lastUpdated() > timeSaved.stdCTime())
      // database in memory is newer than the file - save has to go ahead
      // ignore on recovery has to stop
      stop_ignoring = true;
   else
   {
      // database in memory is exactly the same as the file. The save
      // is going to be spared, ignore on recovery though has to stop
      stop_ignoring = true;
      return false;
   }
   return true;
}

bool laydata::TdtLibDir::TDTcheckread(const std::string filename,
    const TpdTime& timeCreated, const TpdTime& timeSaved, bool& start_ignoring)
{
   bool retval = false;
   start_ignoring = false;
   InputTdtFile tempin(wxString(filename.c_str(), wxConvUTF8), this);
   if (!tempin.status()) return retval;

   std::string news = "Project created: ";
   TpdTime timec(tempin.created()); news += timec();
   tell_log(console::MT_INFO,news);
   news = "Last updated: ";
   TpdTime timeu(tempin.lastUpdated()); news += timeu();
   tell_log(console::MT_INFO,news);
   // File created time stamp must match exactly, otherwise it means
   // that we're reading not exactly the same file that is requested
   if (timeCreated != timec)
   {
      news = "time stamp \"Project created \" doesn't match";
      tell_log(console::MT_ERROR,news);
   }
   if (timeu.stdCTime() < timeSaved.stdCTime())
   {
      news = "time stamp \"Last updated \" is too old.";
      tell_log(console::MT_ERROR,news);
   }
   else if (timeu.stdCTime() > timeSaved.stdCTime())
   {
      news = "time stamp \"Last updated \" is is newer than requested.";
      news +="Some of the following commands will be ignored";
      tell_log(console::MT_WARNING,news);
      //Start ignoring
      start_ignoring = true;
      retval = true;
   }
   else
   {
      retval = true;
   }
   tempin.closeStream();
   return retval;
}

void laydata::TdtLibDir::relink()
{
   // relink starting from the back of the library queue
   for (int i = _libdirectory.size() - 2; i > 0 ; i--)
   {
      _libdirectory[i]->second->relink(this);
   }
   // finally - relink the active database
   if (NULL !=_TEDDB)
      _TEDDB->relink(this);
}

void laydata::TdtLibDir::reextractHierarchy()
{
   // parse starting from the back of the library queue
   for (int i = _libdirectory.size() - 2; i > 0 ; i--)
   {
      _libdirectory[i]->second->recreateHierarchy(this);
   }
   // finally - relink the active database
   if (NULL !=_TEDDB)
      _TEDDB->recreateHierarchy(this);
}

bool laydata::TdtLibDir::getCellNamePair(std::string name, laydata::CellDefin& strdefn)
{
   if ((NULL != _TEDDB) && (_TEDDB->checkCell(name)))
   {
      strdefn = _TEDDB->getCellNamePair(name);
      return true;
   }
   // search the cell in the libraries because it's not in the DB
   return getLibCellRNP(name, strdefn);
}

/*! Searches the library directory for a cell \a name. Returns true and updates \a strdefn if
the cell is found. Returns false otherwise. The method never searches in the UNDEFCELL_LIB and
TARGETDB_LIB. It starts searching from the library after \a libID .
*/
bool laydata::TdtLibDir::getLibCellRNP(std::string name, laydata::CellDefin& strdefn, const int libID) const
{
   // start searching form the first library after the current
   word first2search = (TARGETDB_LIB == libID) ? 1 : libID + 1;
   for (word i = first2search; i < _libdirectory.size(); i++)
   {
      if (NULL != _libdirectory[i]->second->checkCell(name))
      {
         strdefn = _libdirectory[i]->second->getCellNamePair(name);
         return true;
      }
   }
   return false;
}

/*! Searches the library directory for a cell \a name. Returns the cell definition if
the cell is found. Returns NULL otherwise. The method never searches in the TARGETDB_LIB.
It starts searching from the library after \a libID . If the cell is not found in the
libraries it also checks the UNDEFCELL_LIB
 */
laydata::TdtDefaultCell* laydata::TdtLibDir::getLibCellDef(std::string name, const int libID) const
{
   // start searching from the first library after the current
   word first2search = (TARGETDB_LIB == libID) ? 1 : libID + 1;
   for (word i = first2search; i < _libdirectory.size(); i++)
   {
      if (NULL != _libdirectory[i]->second->checkCell(name))
      {
         return _libdirectory[i]->second->getCellNamePair(name);
      }
   }
   // not in the libraries - must be in the defaultlib
   if (NULL != _libdirectory[UNDEFCELL_LIB]->second->checkCell(name, true))
   {
      return _libdirectory[UNDEFCELL_LIB]->second->getCellNamePair(name);
   }
   return NULL;
}

laydata::CellDefin laydata::TdtLibDir::addDefaultCell( std::string name, bool updateHier )
{
   laydata::TdtLibrary* undeflib = _libdirectory[UNDEFCELL_LIB]->second;
   return undeflib->secureDefaultCell(name, updateHier);
}

void laydata::TdtLibDir::addThisUndefCell(TdtDefaultCell* rcell)
{
   laydata::TdtLibrary* undeflib = _libdirectory[UNDEFCELL_LIB]->second;
   undeflib->addThisUndefCell(rcell);
}

bool laydata::TdtLibDir::collectUsedLays(std::string cellname, bool recursive, LayerDefList& laylist) const
{
   TdtCell* topcell = NULL;
   if (NULL != _TEDDB) topcell = static_cast<laydata::TdtCell*>(_TEDDB->checkCell(cellname));
   if (NULL != topcell)
   {
      topcell->collectUsedLays(this, recursive, laylist);
      return true;
   }
   else
   {
      laydata::CellDefin strdefn;
      if (getLibCellRNP(cellname, strdefn))
      {
         static_cast<laydata::TdtCell*>(strdefn)->collectUsedLays(this, recursive, laylist);
         return true;
      }
      else return false;
   }
}

void laydata::TdtLibDir::collectUsedLays( int libID, LayerDefList& laylist) const
{
   assert(UNDEFCELL_LIB != libID);
   laydata::TdtLibrary* curlib = (TARGETDB_LIB == libID) ? _TEDDB : _libdirectory[libID]->second;
   if (NULL != curlib) curlib->collectUsedLays(laylist);
}

/*! Used to link the cell references with their definitions. This function is called to relink
databases (libraries) already loaded in memory. A function with completely the same functionality
and name is defined in the TEDfile. That one is used to link cell references during tdt parsing
phase
*/
laydata::CellDefin laydata::TdtLibDir::linkCellRef(std::string cellname, int libID)
{
   assert(UNDEFCELL_LIB != libID);
   laydata::TdtLibrary* curlib = (TARGETDB_LIB == libID) ? _TEDDB : _libdirectory[libID]->second;
   CellMap::const_iterator striter = curlib->_cells.find(cellname);
   laydata::CellDefin strdefn = NULL;
   // link the cells instances with their definitions
   if (curlib->_cells.end() == striter)
   {
      // search the cell in the rest of the libraries because it's not in the current
      if (!getLibCellRNP(cellname, strdefn, libID))
      {
         // not found! make a default cell
         strdefn = addDefaultCell(cellname, true);
      }
   }
   else
   {
      strdefn = striter->second;
   }
   // Mark that the cell definition is referenced, i.e. it is not the top
   // of the tree (orphan flag in the TdtCell)
   assert(strdefn);
   strdefn->setOrphan(false);
   return strdefn;
}

void  laydata::TdtLibDir::cleanUndefLib()
{
   _libdirectory[UNDEFCELL_LIB]->second->cleanUnreferenced();
}

laydata::TdtDefaultCell* laydata::TdtLibDir::displaceUndefinedCell(std::string cell_name)
{
   return _libdirectory[UNDEFCELL_LIB]->second->displaceCell(cell_name);
}

/*! Ensures a temporary storage of an undefined cell which has been unlinked
(unreferenced).
*/
void laydata::TdtLibDir::holdUndefinedCell(TdtDefaultCell* udefrcell)
{
   // Make sure - it's a default definition of a missing cell ...
   assert(UNDEFCELL_LIB == udefrcell->libID());
   // .. and it's not already in the themporary storage
   assert(_udurCells.end() == _udurCells.find(udefrcell->name()));
   // store a pointer to the definition
   _udurCells[udefrcell->name()] = udefrcell;
}

void laydata::TdtLibDir::deleteHeldCells()
{
   for (CellMap::const_iterator CUDU = _udurCells.begin(); CUDU != _udurCells.end(); CUDU++)
   {
      delete CUDU->second;
   }
   _udurCells.clear();
}

void laydata::TdtLibDir::getHeldCells(CellMap* copyList)
{
   for (CellMap::const_iterator CUDU = _udurCells.begin(); CUDU != _udurCells.end(); CUDU++)
   {
      (*copyList)[CUDU->first] = CUDU->second;
   }
   _udurCells.clear();
}

laydata::LibCellLists* laydata::TdtLibDir::getCells(int libID)
{
   laydata::LibCellLists* all_cells = DEBUG_NEW laydata::LibCellLists();
   if (libID == ALL_LIB)
   {
      if (NULL != _TEDDB)
         all_cells->push_back(&(_TEDDB->cells()));
      for (int i = 1; i < getLastLibRefNo(); i++)
         all_cells->push_back(&(getLib(i)->cells()));
   }
   else if ( (libID == TARGETDB_LIB) && (NULL != _TEDDB) )
      all_cells->push_back(&(_TEDDB->cells()));
   else if (libID == UNDEFCELL_LIB)
      all_cells->push_back(&(_libdirectory[UNDEFCELL_LIB]->second->cells()));
   else if (libID < getLastLibRefNo())
      all_cells->push_back(&(getLib(libID)->cells()));
   return all_cells;
}


//-----------------------------------------------------------------------------
// class TdtDesign
//-----------------------------------------------------------------------------
laydata::TdtDesign::TdtDesign(std::string name, time_t created, time_t lastUpdated,
           real DBU, real UU)  :
   laydata::TdtDesign::TdtLibrary(name, DBU, UU, TARGETDB_LIB, created, lastUpdated),
   _tmpdata       ( NULL        ),
   _modified      ( false       )
{}

void laydata::TdtDesign::read(InputTdtFile* const tedfile)
{
   TdtLibrary::read(tedfile);
   _tmpdata = NULL;
   _modified = false;
}

// !!! Do not forget that the indexing[] operations over std::map can alter the structure !!!
// use find for searching.
laydata::TdtCell* laydata::TdtDesign::addCell(std::string name, laydata::TdtLibDir* libdir)
{
   if (_cells.end() != _cells.find(name)) return NULL; // cell already exists in the target library
   laydata::TdtDefaultCell* libcell = libdir->getLibCellDef(name);
   setModified();
   TdtCell* ncl = DEBUG_NEW TdtCell(name);
   _cells[name] = ncl;
   _hiertree = DEBUG_NEW TDTHierTree(ncl, NULL, _hiertree);
   if (NULL == libcell)
   {// Library cell with this name doesn't exists
      TpdPost::treeAddMember(_hiertree->GetItem()->name().c_str(), _name.c_str(), 0);
   }
   else
   {// Library cell with this name exists. the new cell should replace it in all
    // of its references
      TpdPost::treeAddMember(_hiertree->GetItem()->name().c_str(), _name.c_str(), 0);
      libdir->relink();
      libdir->deleteHeldCells(); // clean-up the unreferenced undefined cells
   }
   return ncl;
}

/*! A ready created structure of TdtCell type is added to the cell list in the
TARGETLIB_DB. Function will assert if a structure with this name is already
listed in the _cells. Used in removeCell undo
*/
void laydata::TdtDesign::addThisCell(laydata::TdtCell* strdefn, laydata::TdtLibDir* libdir)
{
   // Make sure cell with this name doesn't exists already in the target library
   std::string cname = strdefn->name();
   assert(_cells.end() == _cells.find(cname));
   setModified();
   // Check whether structure with this name exists in the libraries or among the
   // referenced, but undefined cells
   laydata::TdtDefaultCell* libcell = libdir->getLibCellDef(cname);
   _cells[cname] = strdefn;
   _hiertree = DEBUG_NEW TDTHierTree(strdefn, NULL, _hiertree);
   if (NULL == libcell)
   {// Library cell with this name doesn't exists
      TpdPost::treeAddMember(cname.c_str(), _name.c_str(), 0);
   }
   else
   {// Library cell with this name exists. the new cell should replace it in all
    // of its references
      TpdPost::treeAddMember(_hiertree->GetItem()->name().c_str(), _name.c_str(), 0);
      libdir->relink();
   }
}

/*! Removes unreferenced cell and returns its contents if required
*/
laydata::TdtCell* laydata::TdtDesign::removeTopCell(std::string& name, laydata::AtticList* fsel, laydata::TdtLibDir* libdir)
{
   assert(NULL == _hiertree->GetMember(_cells[name])->Getparent());

   setModified();
   // get the cell by name
   TdtCell* remcl = static_cast<laydata::TdtCell*>(_cells[name]);
   //empty the contents of the removed cell and return it in AtticList
   remcl->fullSelect();
   // validation is not required here, because the cell is not supposed to be
   // referenced if this method is used
   remcl->deleteSelected(fsel, libdir);
   // update the _hiertree
   dbHierRemoveRoot(remcl);
   // remove the cell from the list of all design cells
   _cells.erase(_cells.find(name));
   // finally - return the empty cell. Don't delete it. Will be used for undo!
   return remcl;
}

void laydata::TdtDesign::renameCell(TdtDefaultCell* targetCell, std::string newName)
{
   assert(NULL != targetCell);
   std::string oldName = targetCell->name();
   if (!targetCell->orphan())
   {
      for (CellMap::iterator CS = _cells.begin(); CS != _cells.end(); CS++)
      {
         if (CS->first != oldName)
            CS->second->renameChild(oldName,newName);
      }
   }
   _cells.erase(oldName);
   _cells[newName] = targetCell;
   targetCell->setName(newName);
   TpdPost::treeRenameMember(oldName.c_str(), newName.c_str());
}

void laydata::TdtDesign::removeRefdCell(std::string& name, CellDefList& pcells, laydata::AtticList* fsel, laydata::TdtLibDir* libdir)
{
   setModified();
   // get the cell by name
   TdtCell* remcl = static_cast<laydata::TdtCell*>(_cells[name]);
   // We need a replacement cell for the references
   laydata::CellDefin strdefn;
   // search the cell in the libraries first
   if (!libdir->getLibCellRNP(name, strdefn, TARGETDB_LIB))
   {
      // not found! make a default cell
      strdefn = libdir->addDefaultCell(name, false);
      // ... and add it to the hierarchy tree
      dbHierAdd(strdefn, NULL);
   }
   // now for every parent cell - relink all the references to cell "name"
   for (laydata::CellDefList::const_iterator CPS = pcells.begin(); CPS != pcells.end(); CPS++)
   {
      (*CPS)->relinkThis(name, strdefn, libdir);
   }
   // validate the cells
   do {} while(validateCells());
   // OK, now when the references are sorted, proceed with the cell itself
   dbHierRemoveRoot(remcl);
   // remove the cell from the list of all design cells
   _cells.erase(_cells.find(name));
   //empty the contents of the removed cell and return it in AtticList
   remcl->fullSelect();
   remcl->deleteSelected(fsel, libdir);
   // now - delete the cell. Cell is already empty
   delete remcl;
}

/*! Get a list of references to all cells in the TARGETDB_LIB instantiating cell "name"
from TERGETDB_LIB - i.e. all parent cells of cell "name" belonging to the TARGETDB_LIB
 */
void laydata::TdtDesign::collectParentCells(std::string& cname, CellDefList& parrentCells)
{
   laydata::CellMap::const_iterator ccellIter = _cells.find(cname);
   if (_cells.end() == ccellIter) return;
   laydata::TdtDefaultCell* tcell = ccellIter->second;
   TDTHierTree* ccl = _hiertree->GetMember(tcell);
   while(ccl)
   {
      if (ccl->Getparent())
      {
         std::string pname = ccl->Getparent()->GetItem()->name();
         assert(_cells.end() != _cells.find(pname));
         parrentCells.push_back( static_cast<laydata::TdtCell*>(_cells[pname]) );
      }
      ccl = ccl->GetNextMember(tcell);
   }
}

laydata::TdtData* laydata::TdtDesign::addBox(const LayerDef& laydef, TP* p1, TP* p2 )
{
   DBbox old_overlap(_target.edit()->cellOverlap());
   QuadTree *actlay = _target.edit()->secureLayer(laydef);
   setModified();
   TP np1((*p1) * _target.rARTM());
   TP np2((*p2) * _target.rARTM());
   laydata::TdtBox *newshape = DEBUG_NEW TdtBox(np1,np2);
   actlay->add(newshape);
   fixReferenceOverlap(old_overlap);
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::putBox(const LayerDef& laydef, TP* p1, TP* p2 )
{
   QTreeTmp *actlay = _target.edit()->secureUnsortedLayer(laydef);
   setModified();
   TP np1((*p1) * _target.rARTM());
   TP np2((*p2) * _target.rARTM());
   laydata::TdtData* newshape = DEBUG_NEW TdtBox(np1,np2);
   actlay->put(newshape);
   return newshape;
}

void laydata::TdtDesign::fixUnsorted()
{
   TdtCell* editTarget = _target.edit();
   DBbox old_overlap(editTarget->cellOverlap());
   editTarget->fixUnsorted();
   fixReferenceOverlap(old_overlap, editTarget);
}

void laydata::TdtDesign::fixReferenceOverlap(DBbox& old_overlap, TdtCell* targetCell)
{
   if (NULL == targetCell)
   {
      if (_target.edit()->overlapChanged(old_overlap, this))
         do {} while(validateCells());
   }
   else
   {
      if (targetCell->overlapChanged(old_overlap, this))
         do {} while(validateCells());
   }
}


void laydata::TdtDesign::setModified()
{
   _modified = true;
   _lastUpdated = time(NULL);
}

laydata::TdtData* laydata::TdtDesign::addPoly(const LayerDef& laydef, PointVector* pl)
{
   laydata::TdtData *newshape = NULL;
   for(PointVector::iterator PL = pl->begin(); PL != pl->end(); PL++)
      (*PL) *= _target.rARTM();

   laydata::ValidPoly check(*pl);
   if (check.acceptable())
   {
      laydata::ShapeList* newShapes = check.replacements();
      if (!newShapes->empty())
      {
         DBbox old_overlap(_target.edit()->cellOverlap());
         QuadTree *actlay = _target.edit()->secureLayer(laydef);
         setModified();
         for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
         {
            actlay->add(*CS);
            newshape = *CS;
         }
         fixReferenceOverlap(old_overlap);
      }
      delete newShapes;
   }
   else
   {
      std::ostringstream ost;
      ost << "Validation check fails - " << check.failType();
      tell_log(console::MT_ERROR, ost.str());
   }
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::putPoly(const LayerDef& laydef, PointVector* pl)
{
   laydata::TdtData *newshape = NULL;
   for(PointVector::iterator PL = pl->begin(); PL != pl->end(); PL++)
      (*PL) *= _target.rARTM();

   laydata::ValidPoly check(*pl);
   if (check.acceptable())
   {
      laydata::ShapeList* newShapes = check.replacements();
      if (!newShapes->empty())
      {
         QTreeTmp *actlay = _target.edit()->secureUnsortedLayer(laydef);
         setModified();
         for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
         {
            actlay->put(*CS);
            newshape = *CS;
         }
         newShapes->clear();
      }
      delete newShapes;
   }
   else
   {
      std::ostringstream ost;
      ost << "Validation check fails - " << check.failType();
      tell_log(console::MT_ERROR, ost.str());
   }
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::addWire(const LayerDef& laydef, PointVector* pl, WireWidth w)
{
   laydata::TdtData *newshape = NULL;
   for(PointVector::iterator PL = pl->begin(); PL != pl->end(); PL++)
      (*PL) *= _target.rARTM();

   laydata::ValidWire check(*pl, w);
   if (check.acceptable())
   {
      laydata::ShapeList* newShapes = check.replacements();
      if (!newShapes->empty())
      {
         DBbox old_overlap(_target.edit()->cellOverlap());
         QuadTree *actlay = _target.edit()->secureLayer(laydef);
         setModified();
         for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
         {
            actlay->add(*CS);
            newshape = *CS;
         }
         newShapes->clear();
         fixReferenceOverlap(old_overlap);
      }
      delete newShapes;
   }
   else
   {
      std::ostringstream ost;
      ost << "Validation check fails - " << check.failType();
      tell_log(console::MT_ERROR, ost.str());
   }
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::putWire(const LayerDef& laydef, PointVector* pl, WireWidth w)
{
   laydata::TdtData *newshape = NULL;
   for(PointVector::iterator PL = pl->begin(); PL != pl->end(); PL++)
      (*PL) *= _target.rARTM();

   laydata::ValidWire check(*pl,w);
   if (check.acceptable())
   {
      laydata::ShapeList* newShapes = check.replacements();
      if (!newShapes->empty())
      {
         QTreeTmp *actlay = _target.edit()->secureUnsortedLayer(laydef);
         setModified();
         for (laydata::ShapeList::const_iterator CS = newShapes->begin(); CS != newShapes->end(); CS++)
         {
            actlay->put(*CS);
            newshape = *CS;
         }
         newShapes->clear();
      }
      delete newShapes;
   }
   else
   {
      std::ostringstream ost;
      ost << "Validation check fails - " << check.failType();
      tell_log(console::MT_ERROR, ost.str());
   }
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::addText(const LayerDef& laydef, std::string& text, CTM& ori)
{
   DBbox old_overlap(_target.edit()->cellOverlap());
   QuadTree *actlay = _target.edit()->secureLayer(laydef);
   setModified();
   ori *= _target.rARTM();
   laydata::TdtText *newshape = DEBUG_NEW TdtText(text,ori);
   actlay->add(newshape);

   fixReferenceOverlap(old_overlap);
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::putText(const LayerDef& laydef, std::string& text, CTM& ori)
{
   QTreeTmp *actlay = _target.edit()->secureUnsortedLayer(laydef);
   setModified();
   ori *= _target.rARTM();
   laydata::TdtData* newshape = DEBUG_NEW TdtText(text,ori);
   actlay->put(newshape);
   return newshape;
}

laydata::TdtData* laydata::TdtDesign::addCellRef(laydata::CellDefin strdefn, CTM& ori)
{
   setModified();
   ori *= _target.rARTM();
   DBbox old_overlap(_target.edit()->cellOverlap());
   TdtData* ncrf = _target.edit()->addCellRef(this, strdefn, ori);
   if (NULL == ncrf)
   {
     tell_log(console::MT_ERROR, "Circular reference is forbidden");
   }
   else
   {
      fixReferenceOverlap(old_overlap);
   }
   return ncrf;
}

laydata::TdtData* laydata::TdtDesign::addCellARef(std::string& name, CTM& ori,
                             ArrayProps& arrprops)
{
   if (checkCell(name))
   {
      laydata::CellDefin strdefn = getCellNamePair(name);
      setModified();
      ori *= _target.rARTM();
      DBbox old_overlap(_target.edit()->cellOverlap());
      TdtData* ncrf = _target.edit()->addCellARef(this, strdefn, ori, arrprops);
      if (NULL == ncrf)
      {
        tell_log(console::MT_ERROR, "Circular reference is forbidden");
      }
      else
      {
         fixReferenceOverlap(old_overlap);
      }
      return ncrf;
   }
   else
   {
      std::string news = "Cell \"";
      news += name; news += "\" is not defined";
      tell_log(console::MT_ERROR,news);
      return NULL;
   }
}

void laydata::TdtDesign::addList(AtticList* nlst, TdtCell* tCell)
{
   TdtCell* targetCell = (NULL == tCell) ? _target.edit() : tCell;
   DBbox old_overlap(targetCell->cellOverlap());
   targetCell->addList(this, nlst);
   fixReferenceOverlap(old_overlap, targetCell);
}

void laydata::TdtDesign::addList(const LayerDef& laydef, ShapeList& newShapes)
{
   if (!newShapes.empty())
   {
      DBbox old_overlap(_target.edit()->cellOverlap());
      QuadTree *actlay = _target.edit()->secureLayer(laydef);
      setModified();
      for (laydata::ShapeList::const_iterator CS = newShapes.begin(); CS != newShapes.end(); CS++)
         actlay->add(*CS);
      fixReferenceOverlap(old_overlap);
   }
}

laydata::TdtCell* laydata::TdtDesign::openCell(std::string name)
{
   if (_cells.end() != _cells.find(name))
   {
      laydata::TdtDefaultCell* tcell = _cells[name];
      if (tcell && (UNDEFCELL_LIB != tcell->libID()))
      {
         _target.setcell(static_cast<TdtCell*>(tcell));
         return _target.edit();
      }
   }
   return NULL; // Active cell has not changed if name is not found
}

bool laydata::TdtDesign::editPush(const TP& pnt, const LayerDefSet& unselable)
{
   if (_target.checkEdit()) {//
      CtmStack transtack;
      transtack.push(CTM());
      laydata::CellRefStack* crstack = DEBUG_NEW laydata::CellRefStack();
      TdtCell* oldtvcell = _target.view();
      // Find the new active reference
      laydata::TdtCellRef *new_activeref =
                      oldtvcell->getCellOver(pnt,transtack,crstack, unselable);
      if (new_activeref) {
         // Set the new active reference and the chain to it
         _target.push(new_activeref,oldtvcell,crstack,transtack.top());
         return true;
      }
      else
         delete crstack;
   }
   return false;
}

bool laydata::TdtDesign::editPrev(const bool undo)
{
   return _target.previous(undo);
}

bool laydata::TdtDesign::editPop()
{
   return _target.pop();
}

bool laydata::TdtDesign::editTop() {
   return _target.top();
}

void laydata::TdtDesign::openGlRender(trend::TrendBase& rend)
{
   if (_target.checkEdit())
   {
      const CTM unity;
      rend.initDrawRefStack(_target.pEditChain());
      _target.view()->openGlRender(rend, unity, false, _target.isCell());
      rend.clearDrawRefStack();
   }
}


void laydata::TdtDesign::write(OutputTdtFile* const tedfile) {
   tedfile->putByte(tedf_DESIGN);
   tedfile->putString(_name);
   tedfile->putReal(_DBU);
   tedfile->putReal(_UU);
   //
   laydata::TDTHierTree* root = _hiertree->GetFirstRoot(TARGETDB_LIB);
   while (root) {
      _cells[root->GetItem()->name()]->write(tedfile, _cells, root);
      root = root->GetNextRoot(TARGETDB_LIB);
   }
   tedfile->putByte(tedf_DESIGNEND);
   _modified = false;
}

void laydata::TdtDesign::tmpDraw(const layprop::DrawProperties& drawprop,
                                          TP base, TP newp) {
   CtmQueue tmp_stack;
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   if (_tmpdata)
   {
      glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.7);
      tmp_stack.push_front(CTM(newp - base,1,0,false));
      _tmpdata->draw(drawprop, tmp_stack);
   }
   else if ((drawprop.currentOp() != console::op_none) && _target.checkEdit())
   {
      if ((console::op_copy == drawprop.currentOp()) || (console::op_move == drawprop.currentOp()))
      {
         base *= _target.rARTM();
         newp *= _target.rARTM();
         tmp_stack.push_front(CTM(_target.ARTM()));
         tmp_stack.push_front(CTM(newp - base,1,0,false)*_target.ARTM());
      }
      else if ((console::op_flipX == drawprop.currentOp()) || (console::op_flipY == drawprop.currentOp()))
      {
         CTM newpos = _target.ARTM();
         tmp_stack.push_front(newpos);
         if (console::op_flipX == drawprop.currentOp())
            newpos.FlipX(newp.y());
         else
            newpos.FlipY(newp.x());
         tmp_stack.push_front(newpos);
      }
      else if (console::op_rotate == drawprop.currentOp())
      {
         CTM newpos = _target.ARTM();
         tmp_stack.push_front(_target.ARTM());
         newpos.Translate(-newp.x(),-newp.y());
         newpos *= _tmpctm;
         newpos.Translate(newp.x(),newp.y());
         tmp_stack.push_front(newpos);
      }
      _target.edit()->motionDraw(drawprop, tmp_stack, true);
      tmp_stack.clear();
   }
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void laydata::TdtDesign::mousePoint(TP p)
{
   if (_tmpdata) _tmpdata->addpoint(p);
}

void laydata::TdtDesign::mousePointCancel(TP& lp)
{
   if (_tmpdata) _tmpdata->rmpoint(lp);
}

void laydata::TdtDesign::mouseStop()
{
   if (_tmpdata) delete _tmpdata;
   _tmpdata = NULL;
}

void laydata::TdtDesign::mouseFlip()
{
   if (_tmpdata) _tmpdata->objFlip();
}

void laydata::TdtDesign::mouseRotate()
{
   if (_tmpdata) _tmpdata->objRotate();
}

void laydata::TdtDesign::selectInBox(TP* p1, TP* p2, const LayerDefSet& unselable, word layselmask, bool pntsel)
{
   if (_target.checkEdit())
   {
      DBbox select_in((*p1)*_target.rARTM(), (*p2)*_target.rARTM());
      select_in.normalize();
      _target.edit()->selectInBox(select_in, unselable, layselmask, pntsel);
   }
}

laydata::AtticList* laydata::TdtDesign::changeSelect(TP* p1, const LayerDefSet& unselable, bool select)
{
   if (_target.checkEdit()) {
      TP selp = (*p1) * _target.rARTM();
      return _target.edit()->changeSelect(selp, select ? sh_selected:sh_active, unselable);
   }
   else return NULL;
}


void laydata::TdtDesign::mouseHoover(TP& position, trend::TrendBase& rend, const LayerDefSet& unselable)
{
   if (_target.checkEdit())
   {
      TP selp = position * _target.rARTM();
      rend.initDrawRefStack(_target.pEditChain());
      _target.edit()->mouseHoover(selp, rend, unselable);
      rend.clearDrawRefStack();
   }
}

void laydata::TdtDesign::unselectInBox(TP* p1, TP* p2, const LayerDefSet& unselable, bool pntsel) {
   if (_target.checkEdit()) {
      DBbox unselect_in((*p1)*_target.rARTM(), (*p2)*_target.rARTM());
      unselect_in.normalize();
      _target.edit()->unselectInBox(unselect_in, pntsel, unselable);
   }
}

void laydata::TdtDesign::copySelected( TP p1, TP p2)
{
   CTM trans;
   DBbox old_overlap(_target.edit()->cellOverlap());
   p1 *= _target.rARTM();
   p2 *= _target.rARTM();
   int4b dX = p2.x() - p1.x();
   int4b dY = p2.y() - p1.y();
   trans.Translate(dX,dY);
   _target.edit()->copySelected(trans);
   fixReferenceOverlap(old_overlap);
}

void laydata::TdtDesign::moveSelected( TP p1, TP p2, SelectList** fadead)
{
   CTM trans;
   DBbox old_overlap(_target.edit()->cellOverlap());
   p1 *= _target.rARTM();
   p2 *= _target.rARTM();
   int4b dX = p2.x() - p1.x();
   int4b dY = p2.y() - p1.y();
   trans.Translate(dX,dY);
   _target.edit()->moveSelected(trans, fadead);
   fixReferenceOverlap(old_overlap);
}

bool laydata::TdtDesign::cutPoly(PointVector& pl, AtticList** dasao)
{
   for (PointVector::iterator CP = pl.begin(); CP != pl.end(); CP ++)
      (*CP) *= _target.rARTM();
   setModified();
   return _target.edit()->cutPolySelected(pl,dasao);
}

bool laydata::TdtDesign::merge(AtticList** dasao)
{
   setModified();
   return _target.edit()->mergeSelected(dasao);
}

bool laydata::TdtDesign::stretch(int bfactor, AtticList** dasao)
{
   setModified();
   return _target.edit()->stretchSelected(bfactor, dasao);
}

void laydata::TdtDesign::rotateSelected( TP p, real angle, SelectList** fadead)
{
   // Things to remember...
   // To deal with edit in place, you have to :
   // - get the current translation matrix of the active cell
   // - apply the transformation - in this case rotation around a point
   //   is done in 3 steps - translate to 0, rotate, translate back
   // - multiply with the reversed CTM to get back in the current
   //   coordinates
   // If just rotation is applied (second step) operation can not deal
   // with the cases when the referenced cell is flipped
   CTM trans = _target.ARTM();
   DBbox old_overlap(_target.edit()->cellOverlap());
   trans.Translate(-p.x(),-p.y());
   trans.Rotate(angle);
   trans.Translate(p.x(),p.y());
   trans *= _target.rARTM();
   _target.edit()->rotateSelected(trans, fadead);
//   _target.edit()->transferSelected(this, trans)
   fixReferenceOverlap(old_overlap);
}

void laydata::TdtDesign::flipSelected( TP p, bool Xaxis) {
   // Here - same principle as for rotateSelected
   // Otherwise flip toggles between X and Y depending on the angle
   // of rotation of the active cell reference
   CTM trans = _target.ARTM();
   DBbox old_overlap(_target.edit()->cellOverlap());
   if (Xaxis)  trans.FlipX(p.y());
   else        trans.FlipY(p.x());
   trans *= _target.rARTM();
   _target.edit()->transferSelected(trans);
   fixReferenceOverlap(old_overlap);
}

void laydata::TdtDesign::deleteSelected(laydata::AtticList* fsel,
                                         laydata::TdtLibDir* libdir)
{
   DBbox old_overlap(_target.edit()->cellOverlap());
   _target.edit()->deleteSelected(fsel, libdir);
   fixReferenceOverlap(old_overlap);
}

void laydata::TdtDesign::destroyThis(TdtData* ds, const LayerDef& laydef, laydata::TdtLibDir* libdir)
{
   DBbox old_overlap(_target.edit()->cellOverlap());
   _target.edit()->destroyThis(libdir, ds, laydef);
   fixReferenceOverlap(old_overlap);
}

bool laydata::TdtDesign::groupSelected(std::string name, laydata::TdtLibDir* libdir)
{
   // first check that the cell with this name does not exist already
   if (_cells.end() != _cells.find(name)) {
      tell_log(console::MT_ERROR, "Cell with this name already exists. Group impossible");
      return false;
   }
   //unlink the fully selected shapes from the QuadTree of the current cell
   AtticList* TBgroup = _target.edit()->groupPrep(libdir);
   if (TBgroup->empty())
   {
      tell_log(console::MT_WARNING, "Nothing to group");
      delete TBgroup; return false;
   }
   //Create a new cell
   TdtCell* newcell = addCell(name, libdir);
   assert(newcell);
   //Get the selected shapes from the current cell and add them to the new cell
   for(AtticList::Iterator CL = TBgroup->begin();  CL != TBgroup->end(); CL++)
   {
      ShapeList* lslct = *CL;
      QTreeTmp* wl = newcell->secureUnsortedLayer(CL());
      // There is no point here to ensure that the layer definition exists.
      // We are just transferring shapes from one structure to another.
      // securelaydef( CL->first );
      for(ShapeList::const_iterator CI = lslct->begin();
                                                     CI != lslct->end(); CI++)
      {
         wl->put(*CI);
         if (REF_LAY_DEF == CL()) newcell->addChild(this,
                                    static_cast<TdtCellRef*>(*CI)->structure());
      }
      lslct->clear();
      delete (lslct);
   }
   TBgroup->clear();delete TBgroup;
   newcell->fixUnsorted();
   //reference the new cell into the current one.
   TdtData* ref = _target.edit()->addCellRef(this,
                                    getCellNamePair(name),CTM(TP(0,0),1,0,false));
   //select the new cell
   ref->setStatus(sh_selected);
   _target.edit()->selectThis(ref,REF_LAY_DEF);
   //Just for the records. No shapes are moved to the Attic during this operation
   //Undo is possible simply by ungrouping the new cell
   return true;
}

laydata::ShapeList* laydata::TdtDesign::ungroupPrep(laydata::TdtLibDir* libdir)
{
   //unlink the selected ref/aref's from the QuadTree of the current cell
   return _target.edit()->ungroupPrep(libdir);
}

laydata::AtticList* laydata::TdtDesign::ungroupThis(laydata::ShapeList* cells4u)
{
   laydata::AtticList* shapeUngr = DEBUG_NEW laydata::AtticList();
   for (ShapeList::const_iterator CC = cells4u->begin();
                                                     CC != cells4u->end(); CC++)
      static_cast<TdtCellRef*>(*CC)->ungroup(this, _target.edit(), shapeUngr);
   // cell and parent validation should not be required here, because
   // the initial and final overlap of the cell should not have been
   // changed by this operation. That's not true for the layers though.
   // Bottom line - validate only the layers
   _target.edit()->fixUnsorted();
   return shapeUngr;
}

bool laydata::TdtDesign::checkValidRef(std::string newref)
{
   if ( _cells.end() == _cells.find(newref) )
   {
      std::string news = "Cell \"";
      news += newref; news += "\" is not defined";
      tell_log(console::MT_ERROR,news);
      return false;
   }
   laydata::TdtDefaultCell* child = _cells[newref];
   if (_hiertree->checkAncestors(_target.edit(), child, _hiertree))
   {
      tell_log(console::MT_ERROR, "Circular reference is forbidden.");
      return false;
   }
   return true;
}

laydata::AtticList* laydata::TdtDesign::changeRef(ShapeList* cells4u, std::string newref)
{
   assert(checkCell(newref));
   assert((!cells4u->empty()));
   laydata::ShapeList* cellsUngr = DEBUG_NEW laydata::ShapeList();
   laydata::CellDefin strdefn = getCellNamePair(newref);
   DBbox old_overlap(_target.edit()->cellOverlap());

   for (ShapeList::const_iterator CC = cells4u->begin(); CC != cells4u->end(); CC++)
   {
      CTM ori = static_cast<TdtCellRef*>(*CC)->translation();
      ArrayProps arrayprops = static_cast<TdtCellRef*>(*CC)->arrayProps();
      TdtData* ncrf;
      if (arrayprops.valid())
         ncrf = _target.edit()->addCellARef(this, strdefn, ori, arrayprops);
      else
         ncrf = _target.edit()->addCellRef(this, strdefn, ori);
      assert(NULL != ncrf);
      ncrf->setStatus(sh_selected);
      _target.edit()->selectThis(ncrf,REF_LAY_DEF);
      cellsUngr->push_back(ncrf);
   }
   laydata::AtticList* shapeUngr = DEBUG_NEW laydata::AtticList();
   shapeUngr->add(REF_LAY_DEF, cellsUngr);
   fixReferenceOverlap(old_overlap);
   return shapeUngr;
}

unsigned int laydata::TdtDesign::numSelected() const
{
   if (_target.checkEdit()) return _target.edit()->numSelected();
   else return 0;
}

DBbox laydata::TdtDesign::activeOverlap()
{
   DBbox ovl = _target.overlap();
   if (ovl == DEFAULT_OVL_BOX) ovl = DEFAULT_ZOOM_BOX;
   return ovl;
}

DBbox laydata::TdtDesign::getVisibleOverlap(layprop::DrawProperties& prop)
{
   DBbox ovl = _target.view()->getVisibleOverlap(prop);
   if (ovl == DEFAULT_OVL_BOX) return activeOverlap();
   else return ovl;
}

void laydata::TdtDesign::tryUnselectAll() const {
   if (NULL != _target.edit())
      _target.edit()->unselectAll();
}

void laydata::TdtDesign::transferLayer(const LayerDef& laydef)
{
   _target.edit()->transferLayer(laydef);
}

void laydata::TdtDesign::transferLayer(laydata::SelectList* slst, const LayerDef& laydef)
{
   _target.edit()->transferLayer(slst, laydef);
}

laydata::TdtDesign::~TdtDesign()
{
   _target.reset();
   //clean-up the _target stack
   for( EditCellStack::iterator CECS = _target._editstack.begin();
                                CECS != _target._editstack.end(); CECS++)
      delete (*CECS);
   _target._editstack.clear();
}
//
//

laydata::DrcLibrary::DrcLibrary(std::string name/*, real DBU, real UU*/) :
   _name(name){}

laydata::DrcLibrary::~DrcLibrary()
{
   laydata::CellMap::const_iterator wc;
   for (wc = _cells.begin(); wc != _cells.end(); wc++)
      delete wc->second;
   _cells.clear();
}

void laydata::DrcLibrary::registerCellRead(std::string cellname, TdtCell* cell) {
   if (_cells.end() != _cells.find(cellname))
   {
   // There are several possibilities here:
   // 1. Cell has been referenced before the definition takes place
   // 2. The same case 1, but the reason is circular reference.
   // 3. Cell is defined more than once
   // Case 3 seems to be just theoretical and we should abort the reading
   // and return with error in this case.
   // Case 2 is really dangerous and once again theoretically we need to
   // break the circularity. This might happen however once the whole file
   // is parsed
   // Case 1 is quite OK, although, the write sequence should use the
   // cell structure tree and start the writing from the leaf cells. At the
   // moment writing is in kind of alphabetical order and case 1 is more
   // than possible. In the future it might me appropriate to issue a warning
   // for possible circular reference.
      if (NULL == _cells[cellname]) {
         // case 1 or case 2 -> can't be distinguished in this moment
         //_cells[cellname] = cell;
         // cell has been referenced already, so it's not an orphan
         cell->setOrphan(false);
      }
      else {
         //@FIXME case 3 -> parsing should be stopped !
      }
   }
   _cells[cellname] = cell;
}

WordList laydata::DrcLibrary::findSelected(const std::string &cell, TP* p1)
{
   //TdtDefaultCell* cell = checkCell("drc");
   TdtCell* theCell = dynamic_cast<TdtCell*>(checkCell(cell));
   TP selp;
   WordList errorList;
   laydata::AtticList* shapes;
   laydata::ShapeList *shapeList;
   TdtData *shape;
   if (theCell)
   {

      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp, layprop::DRC))
      {
         selp = (*p1)*CTM().Reversed(); //Take identity matrix
         //??? Add here Error List construction
         shapes = theCell->findSelected(selp);
         for(laydata::AtticList::Iterator it = shapes->begin(); it != shapes->end(); ++it)
         {
            word error;
            shapeList = *it;
            for (laydata::ShapeList::const_iterator it2 = shapeList->begin(); it2 != shapeList->end(); ++it2)
            {
               shape = dynamic_cast<TdtData*> (*it2);
               error = shape->getLong();
               errorList.push_back(error);
            }
         }
      }
      PROPC->unlockDrawProp(drawProp, true);
      errorList.unique();
      if(shapes != NULL)
      {
         for(laydata::AtticList::Iterator it = shapes->begin(); it != shapes->end(); ++it)
         {
            shapeList = *it;
            delete shapeList;
         }

         delete shapes;
      }
      return errorList;
   }
   else
      return errorList;
}

laydata::TdtDefaultCell* laydata::DrcLibrary::checkCell(std::string name)
{
   if (_cells.end() == _cells.find(name))
      return NULL;
   else return _cells[name];
}

//void laydata::DrcLibrary::openGlDraw(layprop::DrawProperties& drawProp, std::string cell)
//{
//   drawProp.setState(layprop::DRC);
//   laydata::TdtDefaultCell* dst_structure = checkCell(cell);
//   if (dst_structure)
//   {
//      drawProp.initCtmStack();
////    drawProp->initDrawRefStack(NULL); // no references yet in the DRC DB
//      dst_structure->openGlDraw(drawProp);
////    drawProp->clearCtmStack();
//      drawProp.clearDrawRefStack();
//   }
//   drawProp.setState(layprop::DB);
//}

void laydata::DrcLibrary::openGlRender(trend::TrendBase& renderer, std::string cell, CTM& cctm)
{
   renderer.setState(layprop::DRC);
   laydata::TdtDefaultCell* dst_structure = checkCell(cell);
   if (dst_structure)
   {
      dst_structure->openGlRender(renderer, cctm, false, false);
   }
   renderer.setState(layprop::DB);
}

