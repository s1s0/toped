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
//          $URL$
//        Created: Mon Aug 11 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GDSII/TDT hierarchy browser, layer browser, TELL fuction
//                 definition browser
//===========================================================================
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <wx/tooltip.h>
#include "browsers.h"
#include "viewprop.h"
#include "tuidefs.h"
#include "tui.h"
#include "outbox.h"
#include "datacenter.h"
#include "../ui/activelay.xpm"
#include "../ui/lock.xpm"
#include "../ui/cellhg.xpm"
#include "../ui/cellhg_g.xpm"
#include "../ui/cellh.xpm"
#include "../ui/cellh_g.xpm"
#include "../ui/cellfg.xpm"
#include "../ui/cellfg_g.xpm"
#include "../ui/cellf.xpm"
#include "../ui/cellf_g.xpm"
#include "../ui/nolay.xpm"
#include "../ui/librarydb.xpm"
#include "../ui/targetdb.xpm"
#include "../ui/cellundef.xpm"
#include "tpdf_common.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;

//==============================================================================
//
// CellBrowser
//
//==============================================================================
browsers::CellBrowser::CellBrowser(wxWindow *parent, wxWindowID id,
                           const wxPoint& pos, const wxSize& size, long style) :
      wxTreeCtrl(parent, id, pos, size, style | wxTR_FULL_ROW_HIGHLIGHT )
{
   _hierarchy_view   = true;
   _listColor        = /**wxRED;   */   wxColor(128,128,128);
   _editColor        = /**wxYELLOW;*/ *wxBLACK;
   _corrupted        = false;
   wxColour bgnd = GetBackgroundColour();
   SetBackgroundColour(*wxLIGHT_GREY);
   Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &browsers::CellBrowser::onItemRightClick, this, tui::ID_PNL_CELLS);
//   Bind(wxEVT_RIGHT_UP             , &browsers::CellBrowser::onBlankRMouseUp , this);
   Bind(wxEVT_LEFT_DCLICK          , &browsers::CellBrowser::onLMouseDblClk  , this);
   Bind(wxEVT_MENU                 , &browsers::CellBrowser::onWxOpenCell    , this, CELLTREEOPENCELL);
   Bind(tui::wxEVT_CMD_BROWSER     , &browsers::CellBrowser::onCommand       , this);
}

void browsers::CellBrowser::initialize()
{
   DeleteAllItems();
   AddRoot(wxT("hidden_wxroot"));
   _topStructure.Unset();
   _activeStructure.Unset();
   _dbroot.Unset();
   _undefRoot.Unset();
}

void browsers::CellBrowser::showMenu(wxTreeItemId id, const wxPoint& pt)
{
   wxMenu menu;
   _rbCellID = id;
   if (!id.IsOk()) return;
   if ( id == _dbroot )
   {
      menu.Append(tui::TMCELL_NEW, wxT("New cell")); // will be catched up in toped.cpp
      menu.Append(tui::TMGDS_EXPORTL, wxT("GDS export"));
      menu.Append(tui::TMCIF_EXPORTL, wxT("CIF export"));
   }
   else if ( id == _undefRoot )
   {
      return;
   }
   else
   {
      // Check whether it's a library root
      bool libRoot = false;
      for (LibsRoot::const_iterator CLR = _libsRoot.begin(); CLR !=_libsRoot.end(); CLR++)
      {
         if (*CLR == id)
         {
            libRoot = true;break;
         }
      }
      if (libRoot)
      {
         menu.Append(tui::TMLIB_UNLOAD, wxT("Unload library"));
      }
      else
      {
         wxString RBcellname = GetItemText(id);
         switch (GetItemImage(id,wxTreeItemIcon_Normal))
         {
            case BICN_DBCELL_HIER  :
            case BICN_DBCELL_HIER_I:
            case BICN_DBCELL_FLAT  :
            case BICN_DBCELL_FLAT_I:
            {
               menu.Append(CELLTREEOPENCELL, wxT("Open " + RBcellname));
               menu.Append(tui::TMCELL_REF_B , wxT("Add reference to " + RBcellname));
               menu.Append(tui::TMCELL_AREF_B, wxT("Add array of " + RBcellname));
               wxString ost;
               ost << wxT("export ") << RBcellname << wxT(" to GDS");
               menu.Append(tui::TMGDS_EXPORTC, ost);
               ost.Clear();
               ost << wxT("export ") << RBcellname << wxT(" to CIF");
               menu.Append(tui::TMCIF_EXPORTC, ost);
               menu.Append(tui::TMCELL_REPORTLAY, wxT("Report layers used in " + RBcellname));
               break;
            }
            case BICN_LIBCELL_HIER  :
            case BICN_LIBCELL_HIER_I:
            case BICN_LIBCELL_FLAT  :
            case BICN_LIBCELL_FLAT_I:
               menu.Append(tui::TMCELL_REF_B , wxT("Add reference to " + RBcellname));
               menu.Append(tui::TMCELL_AREF_B, wxT("Add array of " + RBcellname));
               menu.Append(tui::TMCELL_REPORTLAY, wxT("Report layers used in " + RBcellname));
               break;
            case BICN_UNDEFCELL:
               menu.Append(tui::TMCELL_NEW , wxT("Define cell " + RBcellname));
               break;
            default: /*undefined cells*/ return;
         }
      }
   }
   PopupMenu(&menu, pt);
}

void browsers::CellBrowser::onWxOpenCell(wxCommandEvent& /*event*/)
{
   wxString cmd;
   cmd << wxT("opencell(\"") << GetItemText(_rbCellID) <<wxT("\");");
   TpdPost::parseCommand(cmd);
}

void browsers::CellBrowser::onItemRightClick(wxTreeEvent& event)
{
   showMenu(event.GetItem(), event.GetPoint());
}

//void browsers::CellBrowser::onBlankRMouseUp(wxMouseEvent& event)
//{
//   wxPoint pt = event.GetPosition();
//   showMenu(HitTest(pt), pt);
//}

void  browsers::CellBrowser::onLMouseDblClk(wxMouseEvent& event)
{
   int flags;
   wxPoint pt = event.GetPosition();
   wxTreeItemId id = HitTest(pt, flags);
   bool libRoot = false;
   for (LibsRoot::const_iterator CLR = _libsRoot.begin(); CLR !=_libsRoot.end(); CLR++)
   {
      if (*CLR == id)
      {
         libRoot = true;break;
      }
   }
   bool cellhit = (id != _dbroot) && (id != _undefRoot) && (!libRoot);
   if (id.IsOk() && cellhit && (flags & wxTREE_HITTEST_ONITEMLABEL))
   {
      wxString cmd;
      cmd << wxT("opencell(\"") << GetItemText(id) <<wxT("\");");
      TpdPost::parseCommand(cmd);
   }
   else
      event.Skip();
}
/*! Search for a tree item &name starting form the &parent. Returns true if the &item is found*/
bool browsers::CellBrowser::findItem(const wxString name, wxTreeItemId& item, const wxTreeItemId parent)
{
   if (!parent.IsOk()) return false;
   wxTreeItemIdValue cookie;
   wxTreeItemId child = GetFirstChild(parent,cookie);
   while (child.IsOk())
   {
      if (item.IsOk())
      {
         if (child == item) item.Unset(); // that's a child we've started from
      }
      else if (name == GetItemText(child))
      {
         item = child; return true;
      }
      if (findItem(name, item, child)) return true;
      child = GetNextChild(parent,cookie);
   }
   return false;
}

/*! Search for a tree item &name which is a child (not grand child!) of the &parent. Returns true
if such item is found*/
bool browsers::CellBrowser::findChildItem(const wxString name, wxTreeItemId& item, const wxTreeItemId parent)
{
   if (!parent.IsOk()) return false;
   wxTreeItemIdValue cookie;
   wxTreeItemId child = GetFirstChild(parent,cookie);
   while (child.IsOk())
   {
      if (item.IsOk())
      {
         if (child == item) item.Unset(); // that's a child we've started from
      }
      else if (name == GetItemText(child))
      {
         item = child; return true;
      }
      child = GetNextChild(parent,cookie);
   }
   return false;
}

void browsers::CellBrowser::copyItem(const wxTreeItemId item, const wxTreeItemId newparent, bool targetLib)
{
   wxTreeItemId newitem = AppendItem(newparent, GetItemText(item));
   int parentnormalImage = GetItemImage(newparent,wxTreeItemIcon_Normal);
   int parentGrcMask = (BICN_DBCELL_HIER_I  == parentnormalImage)
                     | (BICN_LIBCELL_HIER_I == parentnormalImage);
   int normalImage   = GetItemImage(item,wxTreeItemIcon_Normal);
   int expandedImage = GetItemImage(item,wxTreeItemIcon_Expanded);
   SetItemImage(newitem, normalImage, wxTreeItemIcon_Normal);
   SetItemImage(newitem, expandedImage, wxTreeItemIcon_Expanded);
   if (targetLib)
   {
      if (parentGrcMask)
      {
         SetItemImage(newparent, BICN_DBCELL_HIER_I, wxTreeItemIcon_Normal);
         SetItemImage(newparent, BICN_DBCELL_FLAT_I, wxTreeItemIcon_Expanded);
      }
      else
      {
         SetItemImage(newparent, BICN_DBCELL_HIER, wxTreeItemIcon_Normal);
         SetItemImage(newparent, BICN_DBCELL_FLAT, wxTreeItemIcon_Expanded);
      }
   }
   else
   {
      if (parentGrcMask)
      {
         SetItemImage(newparent, BICN_LIBCELL_HIER_I, wxTreeItemIcon_Normal);
         SetItemImage(newparent, BICN_LIBCELL_FLAT_I, wxTreeItemIcon_Expanded);
      }
      else
      {
         SetItemImage(newparent, BICN_LIBCELL_HIER, wxTreeItemIcon_Normal);
         SetItemImage(newparent, BICN_LIBCELL_FLAT, wxTreeItemIcon_Expanded);
      }
   }
   SetItemTextColour(newitem, GetItemTextColour(newparent));
   wxTreeItemIdValue cookie;
   wxTreeItemId child = GetFirstChild(item,cookie);
   while (child.IsOk())
   {
      bool childTargetLib =  (BICN_DBCELL_HIER   == normalImage)
                            |(BICN_DBCELL_HIER_I == normalImage);
      copyItem(child, newitem, childTargetLib);
      child = GetNextChild(item,cookie);
   }
}

void browsers::CellBrowser::highlightChildren(wxTreeItemId parent, wxColour clr)
{
   wxTreeItemIdValue cookie;
   if (!parent.IsOk()) return;
   SetItemTextColour(parent,clr);
   wxTreeItemId child = GetFirstChild(parent,cookie);
   while (child.IsOk())
   {
      highlightChildren(child,clr);
      child = GetNextChild(parent,cookie);
   }
}

void browsers::CellBrowser::resetData(wxString dbName)
{
   initialize();
   _dbroot = AppendItem(GetRootItem(), dbName);
   SetItemImage(_dbroot, BICN_TARGETDB, wxTreeItemIcon_Normal);
}

wxString browsers::CellBrowser::selectedCellName()
{
   wxTreeItemId selected = GetSelection();
   if (selected.IsOk())
      return GetItemText(selected);
   else
      return wxT("");
}

wxString browsers::CellBrowser::topCellName()
{
   if (_topStructure.IsOk())
      return GetItemText(_topStructure);
   else
      return wxT("");
}

wxString browsers::CellBrowser::activeCellName()
{
   if (_activeStructure.IsOk())
      return GetItemText(_activeStructure);
   else
      return wxT("");
}

wxString browsers::CellBrowser::rbCellName()
{
   if (_rbCellID.IsOk())
      return GetItemText(_rbCellID);
   else
      return wxT("");
}

void browsers::CellBrowser::statusHighlight(wxString top, wxString active, wxString selected)
{
   if (_dbroot.IsOk())
   {
      if (findItem(top, _topStructure, _dbroot))
         highlightChildren(_topStructure, _editColor);
      else
         //Try to check previous name
         //that could be removed by filter
         if (findItem(_topCellNameBackup, _topStructure, _dbroot))
            highlightChildren(_topStructure, _editColor);
      if (findItem(active, _activeStructure, _dbroot))
      {
         SetItemBold(_activeStructure, true);
         EnsureVisible(_activeStructure);
      }
      else
      {
         //Try to check previous name
         //that could be removed by filter
         if (findItem(_activeCellNameBackup, _activeStructure, _dbroot))
         {
            SetItemBold(_activeStructure, true);
            EnsureVisible(_activeStructure);
         }
         else
         {
            //in flat mode if can't find active cell
            //(it is possible if cell doesn't pass cell filter)
            //just expand design name item
            if(_hierarchy_view == false)
            {            
               this->Expand(_dbroot);
            }
         }
      }
   }
   wxTreeItemId      item;
   if (findItem(selected, item, GetRootItem()))
      SelectItem(item);
}

void browsers::CellBrowser::collectInfo( bool hier)
{
   //Save previous names before Unset
   //Need for case when filter remove active cell from browser
   if(!activeCellName().IsEmpty())
      _activeCellNameBackup = activeCellName();
   if(!topCellName().IsEmpty())
      _topCellNameBackup = topCellName();

   initialize();
   _corrupted = false;
   _hierarchy_view = hier;
   if(_hierarchy_view)  updateHier();
   else                 updateFlat();
   highlightChildren(GetRootItem(), _listColor);
}

void browsers::CellBrowser::updateFlat()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   //if _cellFilter == "*" don't perform filter checking
   bool filtered = false;
   if (_cellFilter.Cmp(wxT("*"))) filtered = true; 
   if (DATC->lockTDT(dbLibDir, dbmxs_liblock))
   {
      laydata::LibCellLists *cll;
      laydata::LibCellLists::iterator curlib;
      bool rootexists = (DATC->tdtMxState() >= dbmxs_dblock);
      if (rootexists)
      {
         // design name ...
         _dbroot = AppendItem(GetRootItem(),wxString((*dbLibDir)()->name().c_str(),  wxConvUTF8));
         SetItemImage(_dbroot,BICN_TARGETDB,wxTreeItemIcon_Normal);
         // ... and the cells
         cll = dbLibDir->getCells(TARGETDB_LIB);
         for (curlib = cll->begin(); curlib != cll->end(); curlib++)
         {
            laydata::CellMap::const_iterator CL;
            for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
            {
               wxString curCell = wxString( CL->first.c_str(),  wxConvUTF8);
               if(!filtered || (filtered && ::wxMatchWild(_cellFilter, curCell, false)))
               {
                  wxTreeItemId cellitem = AppendItem(_dbroot, curCell);
                  if (CL->second->checkLayer(GRC_LAY_DEF))
                     SetItemImage(cellitem,BICN_DBCELL_FLAT_I,wxTreeItemIcon_Normal);
                  else
                     SetItemImage(cellitem,BICN_DBCELL_FLAT,wxTreeItemIcon_Normal);
               }
            }
         }
         delete cll;
         SortChildren(_dbroot);
      }

      // traverse the libraries now
      int lastLibRefNo = dbLibDir->getLastLibRefNo();
      _libsRoot.clear();
      for(int libID = 1; libID < lastLibRefNo; libID++)
      {
         // library name ...
         wxTreeItemId libroot = AppendItem(GetRootItem(),wxString(dbLibDir->getLib(libID)->name().c_str(),  wxConvUTF8));
         _libsRoot.push_back(libroot);
         SetItemImage(libroot,BICN_LIBRARYDB,wxTreeItemIcon_Normal);
         // ... and the cells
         cll = dbLibDir->getCells(libID);
         for (curlib = cll->begin(); curlib != cll->end(); curlib++)
         {
            laydata::CellMap::const_iterator CL;
            for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
            {
               wxString curCell = wxString( CL->first.c_str(),  wxConvUTF8);
               if(!filtered || (filtered && ::wxMatchWild(_cellFilter, curCell, false)))
               {
                  wxTreeItemId cellitem = AppendItem(libroot, curCell);
                  if (CL->second->checkLayer(GRC_LAY_DEF))
                     SetItemImage(cellitem,BICN_LIBCELL_FLAT_I,wxTreeItemIcon_Normal);
                  else
                     SetItemImage(cellitem,BICN_LIBCELL_FLAT,wxTreeItemIcon_Normal);
               }
            }
         }
         delete cll;
         SortChildren(libroot);
      }
      // And now - deal with the undefined cells
      if (rootexists)
      {
         cll = dbLibDir->getCells(UNDEFCELL_LIB);
         for (curlib = cll->begin(); curlib != cll->end(); curlib++)
         {
            if ((*curlib)->size() != 0)
            {
               // the type ...
               _undefRoot = AppendItem(GetRootItem(), wxString("Undefined Cells", wxConvUTF8));
               SetItemImage(_undefRoot,BICN_LIBRARYDB,wxTreeItemIcon_Normal); //@FIXME <-- HERE - one more lib icon for undefined cells!
               // ... and the cells
               laydata::CellMap::const_iterator CL;
               for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
               {
                  wxString curCell = wxString( (*CL).first.c_str(),  wxConvUTF8);
                  if(!filtered || (filtered && ::wxMatchWild(_cellFilter, curCell, false)))
                  {
                     wxTreeItemId cellitem = AppendItem(_undefRoot, curCell);
                     SetItemImage(cellitem,BICN_UNDEFCELL,wxTreeItemIcon_Normal);
                  }
               }
               SortChildren(_undefRoot);
            }
         }
         delete cll;
      }
   }
   DATC->unlockTDT(dbLibDir);
}

void browsers::CellBrowser::updateHier()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_liblock))
   {
      bool rootexists = (DATC->tdtMxState() >= dbmxs_dblock);
      laydata::TDTHierTree *tdtH = NULL;
      wxTreeItemId nroot;

      // traverse the target design - if it exists
      if (rootexists)
      {
         laydata::TdtDesign* design = (*dbLibDir)();
         // design name ...
         _dbroot = AppendItem(GetRootItem(),wxString(design->name().c_str(),  wxConvUTF8));
         SetItemImage(_dbroot,BICN_TARGETDB,wxTreeItemIcon_Normal);
         // ... and the cells
         laydata::TDTHierTree* const chTree = design->hiertree();
         if (chTree)
            tdtH = chTree->GetFirstRoot(TARGETDB_LIB);
         while (tdtH)
         {
            std::string str = tdtH->GetItem()->name();
            nroot = AppendItem(_dbroot, wxString(tdtH->GetItem()->name().c_str(), wxConvUTF8));
            collectChildren(tdtH, ALL_LIB, nroot);
            tdtH = tdtH->GetNextRoot(TARGETDB_LIB);
         }
         SortChildren(_dbroot);
      }
      // traverse the libraries now
      int lastLibRefNo = dbLibDir->getLastLibRefNo();
      _libsRoot.clear();
      for(int libID = 1; libID < lastLibRefNo; libID++)
      {
         // library name ...
         wxTreeItemId libroot = AppendItem(GetRootItem(),wxString(dbLibDir->getLib(libID)->name().c_str(),  wxConvUTF8));
         _libsRoot.push_back(libroot);
         SetItemImage(libroot,BICN_LIBRARYDB,wxTreeItemIcon_Normal);
         // ... and the cells
         tdtH = dbLibDir->getLib(libID)->hiertree()->GetFirstRoot(libID);
         while (tdtH)
         {
            nroot = AppendItem(libroot, wxString(tdtH->GetItem()->name().c_str(), wxConvUTF8));
            collectChildren(tdtH, libID, nroot);
            tdtH = tdtH->GetNextRoot(libID);
         }
         SortChildren(libroot);
      }
      // And now - deal with the undefined cells
      if (rootexists)
      {
         laydata::LibCellLists *cll = dbLibDir->getCells(UNDEFCELL_LIB);
         laydata::LibCellLists::iterator curlib;
         for (curlib = cll->begin(); curlib != cll->end(); curlib++)
         {
            if ((*curlib)->size() != 0)
            {
               // the type ...
               _undefRoot = AppendItem(GetRootItem(), wxString("Undefined Cells", wxConvUTF8));
               SetItemImage(_undefRoot,BICN_LIBRARYDB,wxTreeItemIcon_Normal); //@FIXME <-- HERE - one more lib icon for undefined cells!
               // ... and the cells
               laydata::CellMap::const_iterator CL;
               for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
               {
                  wxTreeItemId cellitem = AppendItem(_undefRoot, wxString( (*CL).first.c_str(),  wxConvUTF8));
                  SetItemImage(cellitem,BICN_UNDEFCELL,wxTreeItemIcon_Normal);
               }
               SortChildren(_undefRoot);
            }
         }
         delete cll;
      }
   }
   DATC->unlockTDT(dbLibDir);
}

void browsers::CellBrowser::collectChildren(const laydata::TDTHierTree *root,
                                           int libID, const wxTreeItemId& lroot)
{
   const laydata::TdtDefaultCell* cellItem = root->GetItem();
   const laydata::TDTHierTree* child= root->GetChild(libID);
   bool withGrc = cellItem->checkLayer(GRC_LAY_DEF);
   int rootLibID = cellItem->libID();
   if (child)
   {
      switch (rootLibID)
      {
         case TARGETDB_LIB:
            if (withGrc)
            {
               SetItemImage(lroot,BICN_DBCELL_HIER_I,wxTreeItemIcon_Normal);
               SetItemImage(lroot,BICN_DBCELL_FLAT_I,wxTreeItemIcon_Expanded);
            }
            else
            {
               SetItemImage(lroot,BICN_DBCELL_HIER,wxTreeItemIcon_Normal);
               SetItemImage(lroot,BICN_DBCELL_FLAT,wxTreeItemIcon_Expanded);
            }
            break;
         case UNDEFCELL_LIB:
            SetItemImage(lroot,BICN_UNDEFCELL,wxTreeItemIcon_Normal);
            break;
         default:
            if (withGrc)
            {
               SetItemImage(lroot,BICN_LIBCELL_HIER_I,wxTreeItemIcon_Normal);
               SetItemImage(lroot,BICN_LIBCELL_FLAT_I,wxTreeItemIcon_Expanded);
            }
            else
            {
               SetItemImage(lroot,BICN_LIBCELL_HIER,wxTreeItemIcon_Normal);
               SetItemImage(lroot,BICN_LIBCELL_FLAT,wxTreeItemIcon_Expanded);
            }
            break;
      }
   }
   else
   {
      switch (rootLibID)
      {
         case TARGETDB_LIB:
            if (withGrc)
               SetItemImage(lroot,BICN_DBCELL_FLAT_I,wxTreeItemIcon_Normal);
            else
               SetItemImage(lroot,BICN_DBCELL_FLAT,wxTreeItemIcon_Normal);
            break;
         case UNDEFCELL_LIB:
            SetItemImage(lroot,BICN_UNDEFCELL,wxTreeItemIcon_Normal);
            break;
         default:
            if (withGrc)
               SetItemImage(lroot,BICN_LIBCELL_FLAT_I,wxTreeItemIcon_Normal);
            else
               SetItemImage(lroot,BICN_LIBCELL_FLAT,wxTreeItemIcon_Normal);
            break;
      }
   }
   wxTreeItemId nroot;
   wxTreeItemId temp;
   while (child)
   {
      nroot = AppendItem(lroot, wxString(child->GetItem()->name().c_str(), wxConvUTF8));
      SortChildren(lroot);
      collectChildren(child, libID, nroot);
      child = child->GetBrother(libID);
   }
}

void browsers::CellBrowser::collectChildren(const ForeignCellTree* root,
                                            const wxTreeItemId& lroot, bool hier_view)
{
   const ForeignCellTree* Child= root->GetChild(TARGETDB_LIB);
   wxTreeItemId nroot;
   wxTreeItemId temp;

   while (Child)
   {
      if (hier_view)
      {
         nroot = AppendItem(lroot, wxString(Child->GetItem()->strctName().c_str(), wxConvUTF8));
         collectChildren(Child, nroot, hier_view);
      }
      else
      {
         if (!findItem(wxString(Child->GetItem()->strctName().c_str(), wxConvUTF8), temp, GetRootItem()))
         {
            nroot = AppendItem(GetRootItem(), wxString(Child->GetItem()->strctName().c_str(), wxConvUTF8));
            collectChildren(Child, nroot, hier_view);
         }
      }
      SortChildren(lroot);
      Child = Child->GetBrother(TARGETDB_LIB);
   }
}

void browsers::CellBrowser::onCommand( wxCommandEvent& event )
{
   switch ( event.GetInt() )
   {
      case tui::BT_CELL_OPEN :{
         wxTreeItemId topItem;
         VERIFY(findItem(event.GetString(), topItem, _dbroot));
         tdtCellSpot(topItem, topItem);
         break;
      }
      case tui::BT_CELL_HIGHLIGHT: {
         wxTreeItemId actItem;
         VERIFY(findItem(event.GetString(), actItem, _dbroot));
         tdtCellSpot(_topStructure, actItem);
         break;
      }
      case tui::BT_CELL_ADD :
         onTellAddCell(event.GetString(),
                       *(static_cast<wxString*>(event.GetClientData())),
                         static_cast<int>(event.GetExtraLong()));
         delete (static_cast<wxString*>(event.GetClientData()));
         break;
      case tui::BT_CELL_REMOVE:
         onTellRemoveCell(event.GetString(),
                          *(static_cast<wxString*>(event.GetClientData())),
                            static_cast<int>(event.GetExtraLong()));
         delete (static_cast<wxString*>(event.GetClientData()));
         break;
      case tui::BT_CELL_RENAME:
         onTellRenameCell(event.GetString(),
                          *(static_cast<wxString*>(event.GetClientData())));
         delete (static_cast<wxString*>(event.GetClientData()));
         break;
      case tui::BT_NEWTDT_DB  : resetData(event.GetString()); break;
      case tui::BT_CELL_MARK_GRC:
         onTellChangeGrc(event.GetString(), 1 == event.GetExtraLong());
         break;
      default: assert(false);break;
   }
}

void browsers::CellBrowser::tdtCellSpot( const wxTreeItemId& topItem, const wxTreeItemId& actItem )
{
   //
   highlightChildren(_topStructure, _listColor);
   if (_activeStructure.IsOk()) SetItemBold(_activeStructure, false);
   if (  _topStructure.IsOk() ) SetItemBold(   _topStructure, false);
   //
   _topStructure    = topItem;
   _activeStructure = actItem;
   highlightChildren(_topStructure, _editColor);
   SetItemBold(_activeStructure, true);
   EnsureVisible(_activeStructure);
}

void browsers::CellBrowser::onTellAddCell(wxString cellname, wxString parentname, int action)
{
   wxTreeItemId item;
   switch (action)
   {
      case 0://new cell
         if (_hierarchy_view)
         {
            if (parentname.empty())
            {
               if (!_undefRoot.IsOk())
               {
                  _undefRoot = AppendItem(GetRootItem(), wxString("Undefined Cells", wxConvUTF8));
                  SetItemImage(_undefRoot,BICN_LIBRARYDB,wxTreeItemIcon_Normal); //@FIXME <-- HERE - one more lib icon for undefined cells!
               }
               item = AppendItem(_undefRoot, cellname);
               SetItemTextColour(item,GetItemTextColour(_undefRoot));
               SetItemImage(item, BICN_UNDEFCELL, wxTreeItemIcon_Normal);
               SortChildren(_undefRoot);
            }
            else
            {
               wxTreeItemId hnewparent;
               // make sure that the parent exists
               // in this case - the parent must be a library or DB
               do {
                  if (checkCorrupted(findItem(parentname, hnewparent, GetRootItem()))) return;
               } while (!isDbOrLibItem(hnewparent));
               item = AppendItem(hnewparent, cellname);
               SetItemTextColour(item,GetItemTextColour(GetRootItem()));
               SetItemImage(item,BICN_DBCELL_FLAT,wxTreeItemIcon_Normal);
               SortChildren(hnewparent);
            }
         }
         else
         {
            if (parentname.empty())
            {
               if (!_undefRoot.IsOk())
               {
                  _undefRoot = AppendItem(GetRootItem(), wxString("Undefined Cells", wxConvUTF8));
                  SetItemImage(_undefRoot,BICN_LIBRARYDB,wxTreeItemIcon_Normal); //@FIXME <-- HERE - one more lib icon for undefined cells!
               }
               item = AppendItem(_undefRoot, cellname);
               SetItemTextColour(item,GetItemTextColour(_undefRoot));
               SetItemImage(item, BICN_UNDEFCELL, wxTreeItemIcon_Normal);
               SortChildren(_undefRoot);
            }
            else
            {
               if (checkCorrupted(!findItem(cellname, item, _dbroot))) return;
               item = AppendItem(_dbroot, cellname);
               SetItemTextColour(item,GetItemTextColour(GetRootItem()));
               SetItemImage(item,BICN_DBCELL_FLAT,wxTreeItemIcon_Normal);
               SortChildren(_dbroot);
            }
         }
         break;
      case 1://first reference of existing cell
         if (_hierarchy_view)
         {
            wxTreeItemId newparent;
            if (checkCorrupted(findChildItem(cellname, item, _dbroot))) return;
            wxString cell_top_str = topCellName();
            wxString cell_act_str = activeCellName();
            wxString cell_sel_str = selectedCellName();
            while (findItem(parentname, newparent, GetRootItem()))
            {
               // in this case - the parent should not be a library
               if (isDbOrLibItem(newparent)) continue;
               copyItem(item,newparent);
               SortChildren(newparent);
            }
            DeleteChildren(item);
            Delete(item);
            _topStructure.Unset(); _activeStructure.Unset();
            statusHighlight(cell_top_str, cell_act_str, cell_sel_str);
         }
         break;
      case 2://new parent added
         if (_hierarchy_view)
         {//
            wxTreeItemId newparent;
            if (checkCorrupted(findItem(cellname, item, GetRootItem()))) return;
            while (findItem(parentname, newparent, _dbroot))
            {
               copyItem(item,newparent);
               SortChildren(newparent);
            }
         }
         break;
      case 3://first parent added for library component
         if (_hierarchy_view)
         {//
            wxTreeItemId newparent;
            bool linkFound = false;
            // check the libraries first ...
            for (LibsRoot::const_iterator CLR = _libsRoot.begin(); CLR !=_libsRoot.end(); CLR++)
            {
               if (findItem(cellname, item, *CLR))
               {
                  linkFound = true;
                  break;
               }
            }
            // ... and if it's not there - the undefined cells
            if ( (!linkFound) && findItem(cellname, item, _undefRoot) )
            {
               linkFound = true;
            }
            if (checkCorrupted(linkFound)) return;
            while (findItem(parentname, newparent, _dbroot))
            {
               copyItem(item,newparent);
               SortChildren(newparent);
            }
         }
         break;
      default: assert(false);break;
   }
}

void browsers::CellBrowser::onTellRemoveCell(wxString cellname, wxString parentname, int action)
{
   wxTreeItemId newparent;
   switch (action)
   {
      case 0:// no longer child of this parent - remove it from all parent instances
      case 1:// Lib cells not more referenced in the DB
         if (_hierarchy_view)
         {
            while (findItem(parentname, newparent, GetRootItem()))
            {
               wxTreeItemId item;
               if (checkCorrupted(findChildItem(cellname, item, newparent))) return;
               DeleteChildren(item);
               Delete(item);
            }
         }
         break;
      case 2://DB cell, which has no parents anymore
         if (_hierarchy_view)
         {
            wxTreeItemId item;
            wxString cell_top_str = topCellName();
            wxString cell_act_str = activeCellName();
            wxString cell_sel_str = selectedCellName();
            bool copied = false;
            while (findItem(parentname, newparent, GetRootItem()))
            {
               if (checkCorrupted(findChildItem(cellname, item, newparent))) return;
               // to make sure that the item will appear only once in the new parent
               // otherwise if it has been referenced in a number of parents it will
               // appear several times in the top of the tree
               if (!copied)
               {
                  copyItem(item, _dbroot);
                  copied = true;
               }
               DeleteChildren(item);
               Delete(item);
            }
            _topStructure.Unset(); _activeStructure.Unset();
            statusHighlight(cell_top_str, cell_act_str, cell_sel_str);
         }
         break;
      case 3:// we are removing the cell, not it's reference
      {
         wxTreeItemId item;
         if (checkCorrupted(findChildItem(cellname, item, _dbroot))) return;
         wxString cell_top_str = topCellName();
         wxString cell_act_str = activeCellName();
         wxString cell_sel_str = selectedCellName();
         // copy all children
         // This part is "in case". The thing is that children should have been
         // removed already, by TdtCell::removePrep
         wxTreeItemIdValue cookie;
         wxTreeItemId child = GetFirstChild(item,cookie);
         while (child.IsOk())
         {
            copyItem(child, GetRootItem());
            child = GetNextChild(item,cookie);
         }
         // finally delete the item and it's children
         DeleteChildren(item);
         Delete(item);
         _topStructure.Unset(); _activeStructure.Unset();
         statusHighlight(cell_top_str, cell_act_str, cell_sel_str);
         break;
      }
      case 4:// remove undefined cell
      {
         wxTreeItemId item;
         while (findItem(cellname, item, _undefRoot))
         {
            Delete(item);
         }
         if (!ItemHasChildren(_undefRoot))
         {
            Delete(_undefRoot);
            _undefRoot.Unset();
         }
         break;
      }
      default: assert(false);break;
   }
}

void browsers::CellBrowser::onTellRenameCell(wxString oldName, wxString newName)
{
   wxTreeItemId targetItem;
   while (findItem(oldName, targetItem, _dbroot))
   {
      SetItemText(targetItem, newName);
   }
}

void browsers::CellBrowser::onTellChangeGrc(wxString cellName, bool mark)
{
   wxTreeItemId targetItem;
   while (findItem(cellName, targetItem, _dbroot))
   {
      if (mark)
      {
         SetItemImage(targetItem,BICN_DBCELL_HIER_I,wxTreeItemIcon_Normal);
         SetItemImage(targetItem,BICN_DBCELL_FLAT_I,wxTreeItemIcon_Expanded);
      }
      else
      {
         SetItemImage(targetItem,BICN_DBCELL_HIER,wxTreeItemIcon_Normal);
         SetItemImage(targetItem,BICN_DBCELL_FLAT,wxTreeItemIcon_Expanded);
      }
   }
}

bool browsers::CellBrowser::isDbOrLibItem(const wxTreeItemId item)
{
   if (_dbroot.IsOk() && (_dbroot == item)) return true;
   for (LibsRoot::const_iterator CLR = _libsRoot.begin(); CLR !=_libsRoot.end(); CLR++)
   {
      if (*CLR == item) return true;
   }
   return false;
}

bool browsers::CellBrowser::checkCorrupted(bool iresult)
{
   if (!_corrupted && !iresult)
   {
      tell_log(console::MT_ERROR, "Cell browser lost synchronization. Press Flat or Hier button to recover. Please report a bug.");
   }
   _corrupted |= (!iresult);
   return _corrupted;
}

//==============================================================================
//
// GDSCellBrowser
//
//==============================================================================
browsers::GDSCellBrowser::GDSCellBrowser(wxWindow *parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style) :
                                       CellBrowser(parent, id, pos, size, style )
{
   Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &browsers::GDSCellBrowser::onItemRightClick, this,      tui::ID_GDS_CELLTREE );
   Bind(wxEVT_MENU                 , &browsers::GDSCellBrowser::onReportlay     , this, browsers::GDSTREEREPORTLAY);
}

void browsers::GDSCellBrowser::onReportlay(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   cmd << wxT("report_gdslayers(\"") << GetItemText(_rbCellID) <<wxT("\");");
   TpdPost::parseCommand(cmd);
}

void browsers::GDSCellBrowser::showMenu(wxTreeItemId id, const wxPoint& pt)
{
   wxMenu menu;
   _rbCellID = id;
   if ( id.IsOk() && (id != GetRootItem()))   {
      wxString RBcellname = GetItemText(id);
      menu.Append(tui::TMGDS_TRANSLATE, wxT("Translate " + RBcellname));
      menu.Append(GDSTREEREPORTLAY, wxT("Report layers used in " + RBcellname));
   }
   else {
      menu.Append(tui::TMGDS_CLOSE, wxT("Close GDS")); // will be caught up in toped.cpp
   }
   PopupMenu(&menu, pt);
}

void browsers::GDSCellBrowser::collectInfo(bool hier)
{
   DeleteAllItems();
   ForeignDbFile* AGDSDB = NULL;
   if (DATC->lockGds(AGDSDB))
   {
      AddRoot(wxString((AGDSDB->libname()).c_str(), wxConvUTF8));
      if (NULL != AGDSDB->hierTree())
      {
         ForeignCellTree* root = AGDSDB->hierTree()->GetFirstRoot(TARGETDB_LIB);
         wxTreeItemId nroot;
         while (root)
         {
            nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->strctName().c_str(),wxConvUTF8));
            collectChildren(root, nroot, hier);
            root = root->GetNextRoot(TARGETDB_LIB);
         }
      }
      SortChildren(GetRootItem());
   }
   DATC->unlockGds(AGDSDB);
}

//==============================================================================
//
// CIFCellBrowser
//
//==============================================================================
browsers::CIFCellBrowser::CIFCellBrowser(wxWindow *parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style) :
      CellBrowser(parent, id, pos, size, style )
{
   Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &browsers::CIFCellBrowser::onItemRightClick, this,      tui::ID_CIF_CELLTREE );
   Bind(wxEVT_MENU                 , &browsers::CIFCellBrowser::onReportlay     , this, browsers::CIFTREEREPORTLAY);
}

void browsers::CIFCellBrowser::onReportlay(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   cmd << wxT("report_ciflayers(\"") << GetItemText(_rbCellID) <<wxT("\");");
   TpdPost::parseCommand(cmd);
}

void browsers::CIFCellBrowser::showMenu(wxTreeItemId id, const wxPoint& pt)
{
   wxMenu menu;
   _rbCellID = id;
   if ( id.IsOk() && (id != GetRootItem()) )
   {
      wxString RBcellname = GetItemText(id);
      menu.Append(tui::TMCIF_TRANSLATE, wxT("Translate " + RBcellname));
      menu.Append(CIFTREEREPORTLAY, wxT("Report layers used in " + RBcellname));
   }
   else
   {
      menu.Append(tui::TMCIF_CLOSE, wxT("Close CIF")); // will be caught up in toped.cpp
   }
   PopupMenu(&menu, pt);
}

void browsers::CIFCellBrowser::collectInfo(bool hier)
{
   DeleteAllItems();

   ForeignDbFile* ACIFDB = NULL;
   if (DATC->lockCif(ACIFDB))
   {
      AddRoot(wxString((ACIFDB->libname()).c_str(), wxConvUTF8));
      if (NULL != ACIFDB->hierTree())
      {
         ForeignCellTree* root = ACIFDB->hierTree()->GetFirstRoot(TARGETDB_LIB);
         wxTreeItemId nroot;
         while (root)
         {
            nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->strctName().c_str(),wxConvUTF8));
            collectChildren(root, nroot, hier);
            root = root->GetNextRoot(TARGETDB_LIB);
         }
      }
      SortChildren(GetRootItem());
   }
   DATC->unlockCif(ACIFDB);
}

//==============================================================================
//
// OASCellBrowser
//
//==============================================================================
browsers::OASCellBrowser::OASCellBrowser(wxWindow *parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style) :
      CellBrowser(parent, id, pos, size, style )
{
   Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &browsers::OASCellBrowser::onItemRightClick, this,      tui::ID_OAS_CELLTREE );
   Bind(wxEVT_MENU                 , &browsers::OASCellBrowser::onReportlay     , this, browsers::OASTREEREPORTLAY);
}

void browsers::OASCellBrowser::onReportlay(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   cmd << wxT("report_oasislayers(\"") << GetItemText(_rbCellID) <<wxT("\");");
   TpdPost::parseCommand(cmd);
}

void browsers::OASCellBrowser::showMenu(wxTreeItemId id, const wxPoint& pt)
{
   wxMenu menu;
   _rbCellID = id;
   if ( id.IsOk() && (id != GetRootItem()) )
   {
      wxString RBcellname = GetItemText(id);
      menu.Append(tui::TMOAS_TRANSLATE, wxT("Translate " + RBcellname));
      menu.Append(OASTREEREPORTLAY, wxT("Report layers used in " + RBcellname));
   }
   else
   {
      menu.Append(tui::TMOAS_CLOSE, wxT("Close Oasis")); // will be caught up in toped.cpp
   }
   PopupMenu(&menu, pt);
}

void browsers::OASCellBrowser::collectInfo(bool hier)
{
   DeleteAllItems();

   ForeignDbFile* AOASDB = NULL;
   if (DATC->lockOas(AOASDB))
   {
      AddRoot(wxString((AOASDB->libname()).c_str(), wxConvUTF8));
      if (NULL != AOASDB->hierTree())
      {
         ForeignCellTree* root = AOASDB->hierTree()->GetFirstRoot(TARGETDB_LIB);
         wxTreeItemId nroot;
         while (root)
         {
            nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->strctName().c_str(),wxConvUTF8));
            collectChildren(root, nroot, hier);
            root = root->GetNextRoot(TARGETDB_LIB);
         }
      }
      SortChildren(GetRootItem());
   }
   DATC->unlockOas(AOASDB);
}

//==============================================================================
//
// TDTbrowser
//
//==============================================================================
browsers::TDTbrowser::TDTbrowser(wxWindow *parent, wxWindowID id,
                              const wxPoint& pos, const wxSize& size, long style) :
      wxPanel(parent, id, pos, size)
{
   Bind(wxEVT_MENU  , &browsers::TDTbrowser::onReportUsedLayers, this, tui::TMCELL_REPORTLAY);
   Bind(wxEVT_BUTTON, &browsers::TDTbrowser::onHierView        , this, tui::BT_CELLS_HIER   );
   Bind(wxEVT_BUTTON, &browsers::TDTbrowser::onFlatView        , this, tui::BT_CELLS_FLAT   );
   Bind(wxEVT_TEXT  , &browsers::TDTbrowser::onFlatView        , this, tui::ID_CELL_FILTER  );

   _hierarchy_view = true;
   wxBoxSizer *thesizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   wxBoxSizer *sizer1   = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   _hierButton = DEBUG_NEW wxButton( this, tui::BT_CELLS_HIER, wxT("Hier") );
   _flatButton = DEBUG_NEW wxButton( this, tui::BT_CELLS_FLAT, wxT("Flat") );
   _cellFilter = DEBUG_NEW wxTextCtrl(this, tui::ID_CELL_FILTER);
   //Set bold font for _hierButton
   wxFont font = _hierButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _hierButton->SetFont(font);

   sizer1->Add(_hierButton, 1, wxEXPAND|wxBOTTOM, 3);
   sizer1->Add(_flatButton, 1, wxEXPAND|wxBOTTOM, 3);
   _cellBrowser = DEBUG_NEW CellBrowser(this, tui::ID_PNL_CELLS, pos, size , style | wxTR_HIDE_ROOT | wxTR_NO_BUTTONS | wxTR_LINES_AT_ROOT);
   thesizer->Add(_cellFilter, 0, wxEXPAND|wxTOP, 3);
   thesizer->Add(_cellBrowser, 1, wxEXPAND | wxBOTTOM);
   thesizer->Add(sizer1, 0, wxEXPAND | wxALL);

   _imageList = DEBUG_NEW wxImageList(16, 16, TRUE);
   _imageList->Add( wxIcon( cellhg    ) ); // BICN_LIBCELL_HIER
   _imageList->Add( wxIcon( cellhg_g  ) ); // BICN_LIBCELL_HIER_I
   _imageList->Add( wxIcon( cellh     ) ); // BICN_DBCELL_HIER
   _imageList->Add( wxIcon( cellh_g   ) ); // BICN_DBCELL_HIER_I
   _imageList->Add( wxIcon( cellfg    ) ); // BICN_LIBCELL_FLAT
   _imageList->Add( wxIcon( cellfg_g  ) ); // BICN_LIBCELL_FLAT_I
   _imageList->Add( wxIcon( cellf     ) ); // BICN_DBCELL_FLAT
   _imageList->Add( wxIcon( cellf_g   ) ); // BICN_DBCELL_FLAT_I
   _imageList->Add( wxIcon( librarydb ) ); // BICN_LIBRARYDB
   _imageList->Add( wxIcon( targetdb  ) ); // BICN_TARGETDB
   _imageList->Add( wxIcon( cellundef ) ); // BICN_UNDEFCELL

   wxString filterState = wxT("*");
   _cellFilter->ChangeValue(filterState);
   _cellBrowser->setCellFilter(filterState);
   _cellFilter->Show(false); //hide cell filter for initial hierarchy view
   _cellBrowser->SetImageList(_imageList);
   SetSizerAndFit(thesizer);
   thesizer->SetSizeHints( this );
}

void browsers::TDTbrowser::onFlatView( wxCommandEvent& /*event*/ )
{
   _hierarchy_view = false;
   _cellFilter->Show(true); //show cell filter for flat view
   wxString filterState = _cellFilter->GetValue();
   _cellBrowser->setCellFilter(filterState); //update filters
   wxString cell_top_str = _cellBrowser->topCellName();
   wxString cell_act_str = _cellBrowser->activeCellName();
   wxString cell_sel_str = _cellBrowser->selectedCellName();
   _cellBrowser->collectInfo(_hierarchy_view);
   _cellBrowser->statusHighlight(cell_top_str, cell_act_str, cell_sel_str);

   //Set normal font for  _hierButton
   wxFont font = _hierButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_NORMAL);
   _hierButton->SetFont(font);
   //Set bold font for _flatButton;
   font = _flatButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _flatButton->SetFont(font);

   Show();
   (this->GetSizer())->Layout();

}

void browsers::TDTbrowser::onHierView( wxCommandEvent& /*event*/ )
{
   _hierarchy_view = true;
   _cellFilter->Show(false); //hide cell filter for hierarchy view
   wxString cell_top_str = _cellBrowser->topCellName();
   wxString cell_act_str = _cellBrowser->activeCellName();
   wxString cell_sel_str = _cellBrowser->selectedCellName();
   _cellBrowser->collectInfo(_hierarchy_view);
   _cellBrowser->statusHighlight(cell_top_str, cell_act_str, cell_sel_str);

   //Set bold  font for  _hierButton
   wxFont font = _hierButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _hierButton->SetFont(font);
   //Set normal  font for _flatButton;
   font = _flatButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_NORMAL);
   _flatButton->SetFont(font);

   Show();
   (this->GetSizer())->Layout();
}

void browsers::TDTbrowser::onReportUsedLayers(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   cmd << wxT("report_layers(\"") << selectedCellName() << wxT("\" , true);");
   TpdPost::parseCommand(cmd);
}

wxString browsers::TDTbrowser::selectedCellName() const
{
   return _cellBrowser->selectedCellName();
}

void browsers::TDTbrowser::refreshData(bool keepAct)
{
   wxString selectedcn(_cellBrowser->selectedCellName());
   wxString topcn(_cellBrowser->topCellName());
   wxString activecn(_cellBrowser->activeCellName());
   _cellBrowser->collectInfo(_hierarchy_view);
   if (keepAct)
   {
      _cellBrowser->statusHighlight(topcn, activecn, selectedcn);
   }
}

browsers::TDTbrowser::~TDTbrowser()
{
   _imageList->RemoveAll();
   delete _imageList;
   _cellBrowser->DeleteAllItems();
   delete _cellBrowser;
}

//==============================================================================
//
// XdbBrowser (External Data base browser)
//
//==============================================================================
browsers::XdbBrowser::XdbBrowser(   wxWindow *parent,
                                    wxWindowID id,
                                    const wxPoint& pos ,
                                    const wxSize& size ,
                                    long style ):
      wxPanel(parent, id, pos, size, style)
{
   Bind(wxEVT_BUTTON, &browsers::XdbBrowser::onHierView, this, tui::BT_CELLS_HIER2);
   Bind(wxEVT_BUTTON, &browsers::XdbBrowser::onFlatView, this, tui::BT_CELLS_FLAT2);

   _hierarchy_view = true;
   switch (id)
   {
      case tui::ID_GDS_CELLTREE:
         _cellBrowser = DEBUG_NEW GDSCellBrowser(this, tui::ID_GDS_CELLTREE, pos, size, style);
         break;
      case tui::ID_CIF_CELLTREE:
         _cellBrowser = DEBUG_NEW CIFCellBrowser(this, tui::ID_CIF_CELLTREE, pos, size, style);
         break;
      case tui::ID_OAS_CELLTREE:
         _cellBrowser = DEBUG_NEW OASCellBrowser(this, tui::ID_OAS_CELLTREE, pos, size, style);
         break;
      default: assert(false);break;
   }

   wxBoxSizer *thesizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );

   wxBoxSizer *sizer1 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   _hierButton = DEBUG_NEW wxButton( this, tui::BT_CELLS_HIER2, wxT("Hier") );
   //Set bold font for _hierButton
   wxFont font = _hierButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _hierButton->SetFont(font);

   _flatButton = DEBUG_NEW wxButton( this, tui::BT_CELLS_FLAT2, wxT("Flat") );

   sizer1->Add(_hierButton, 1, wxEXPAND|wxBOTTOM, 3);
   sizer1->Add(_flatButton, 1, wxEXPAND|wxBOTTOM, 3);

   thesizer->Add(_cellBrowser, 1, wxEXPAND | wxBOTTOM);
   thesizer->Add(sizer1, 0, wxEXPAND | wxALL);

   SetSizerAndFit(thesizer);
   thesizer->SetSizeHints( this );
}

void browsers::XdbBrowser::onFlatView(wxCommandEvent& /*event*/)
{
   if (!_hierarchy_view) return;
   _hierarchy_view = false;

   wxString cell_sel_str = _cellBrowser->selectedCellName();
   _cellBrowser->collectInfo(_hierarchy_view);
   _cellBrowser->statusHighlight(wxT(""), wxT(""), cell_sel_str);

   //Set normal font for  _hierButton
   wxFont font = _flatButton->GetFont();
   _hierButton->SetFont(font);
   //Set bold font for _flatButton;
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _flatButton->SetFont(font);
}

void browsers::XdbBrowser::onHierView(wxCommandEvent& /*event*/)
{
   if (_hierarchy_view) return;
   _hierarchy_view = true;

   wxString cell_sel_str = _cellBrowser->selectedCellName();
   _cellBrowser->collectInfo(_hierarchy_view);
   _cellBrowser->statusHighlight(wxT(""), wxT(""), cell_sel_str);

   //Set normal  font for _flatButton;
   wxFont font = _hierButton->GetFont();
   _flatButton->SetFont(font);
   //Set bold font for _hierButton
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _hierButton->SetFont(font);
}

//==============================================================================
//
// browserTAB
//
//==============================================================================
browsers::browserTAB::browserTAB(wxWindow *parent, wxWindowID id,const wxPoint& pos, const wxSize& size, long style) :
   wxAuiNotebook( parent, id, pos, size, style),
   _gdsStruct   ( NULL ),
   _cifStruct   ( NULL ),
   _oasStruct   ( NULL ),
   _drcStruct   ( NULL )
//   _gdsPageIndex(    0 ),
//   _cifPageIndex(    0 )
{
   Bind(tui::wxEVT_CMD_BROWSER, &browsers::browserTAB::onCommand, this);
   _tdtStruct = DEBUG_NEW TDTbrowser(this, tui::ID_TPD_CELLTREE);
   AddPage(_tdtStruct, wxT("Cells"));
   _layers = DEBUG_NEW LayerBrowser(this,  tui::ID_TPD_LAYERS);
   AddPage(_layers, wxT("Layers"));
}

browsers::browserTAB::~browserTAB()
{
//   It appears that wx is calling automatically the destructors of the
//   child windows, so no need to call them here
//   delete _tdtStruct; _tdtStruct = NULL;
//   delete _TDTlayers; _TDTlayers = NULL;
}

wxString browsers::browserTAB::tdtSelectedGdsName() const
{
   if (NULL != _gdsStruct)
      return _gdsStruct->selectedCellName();
   else return wxT("");
}

wxString browsers::browserTAB::tdtSelectedCifName() const
{
   if (NULL != _cifStruct)
      return _cifStruct->selectedCellName();
   else
      return wxT("");
}

wxString browsers::browserTAB::tdtSelectedOasName() const
{
   if (NULL != _oasStruct)
      return _oasStruct->selectedCellName();
   else return wxT("");
}

void browsers::browserTAB::onCommand(wxCommandEvent& event)
{
   int command = event.GetInt();
   switch (command)
   {
      case tui::BT_ADDTDT_LIB   : onTellRefreshTdtLib(1 == event.GetExtraLong());break;
      case tui::BT_ADDGDS_TAB   : onTellAddGdsTab();break;
      case tui::BT_CLEARGDS_TAB : onTellClearGdsTab(); break;
      case tui::BT_ADDCIF_TAB   : onTellAddCifTab();break;
      case tui::BT_CLEARCIF_TAB : onTellClearCifTab(); break;
      case tui::BT_ADDOAS_TAB   : onTellAddOasTab();break;
      case tui::BT_CLEAROAS_TAB : onTellClearOasTab(); break;
      case tui::BT_ADDDRC_TAB   : onTellAddDRCTab();break;
      case tui::BT_CLEARDRC_TAB : onTellClearDRCTab(); break;
      default: event.Skip();break;
   }
}

void browsers::browserTAB::onTellRefreshTdtLib(bool targetDB)
{
   _tdtStruct->refreshData(!targetDB);
}

void browsers::browserTAB::onTellAddGdsTab()
{
   if (!_gdsStruct)
   {
      _gdsStruct = DEBUG_NEW XdbBrowser(this, tui::ID_GDS_CELLTREE);
      AddPage(_gdsStruct, wxT("GDS"), true);
   }
   // don't bother to clean-up existing DB. It's done in the function called
   _gdsStruct->collectInfo();
}

void browsers::browserTAB::onTellClearGdsTab()
{
   if (_gdsStruct)
   {
      int _gdsPageIndex = GetPageIndex(_gdsStruct);
      assert(wxNOT_FOUND != _gdsPageIndex);
      _gdsStruct->deleteAllItems();
      DeletePage(_gdsPageIndex);
      _gdsStruct = NULL;
   }
}

void browsers::browserTAB::onTellAddCifTab()
{
   if (NULL == _cifStruct)
   {
      _cifStruct = DEBUG_NEW XdbBrowser(this, tui::ID_CIF_CELLTREE);
      AddPage(_cifStruct, wxT("CIF"), true);
   }
   // don't bother to clean-up existing DB. It's done in the function called
   _cifStruct->collectInfo();
}

void browsers::browserTAB::onTellClearCifTab()
{
   if (_cifStruct)
   {
      int _cifPageIndex = GetPageIndex(_cifStruct);
      assert(wxNOT_FOUND != _cifPageIndex);
      _cifStruct->deleteAllItems();
      DeletePage(_cifPageIndex);
      _cifStruct = NULL;
   }
}

void browsers::browserTAB::onTellAddOasTab()
{
   if (NULL == _oasStruct)
   {
      _oasStruct = DEBUG_NEW XdbBrowser(this, tui::ID_OAS_CELLTREE);
      AddPage(_oasStruct, wxT("Oasis"), true);
   }
   // don't bother to clean-up existing DB. It's done in the function called
   _oasStruct->collectInfo();
}

void browsers::browserTAB::onTellClearOasTab()
{
   if (_oasStruct)
   {
      int _oasPageIndex = GetPageIndex(_oasStruct);
      assert(wxNOT_FOUND != _oasPageIndex);
      _oasStruct->deleteAllItems();
      DeletePage(_oasPageIndex);
      _oasStruct = NULL;
   }
}

void browsers::browserTAB::onTellAddDRCTab()
{
   if (NULL == _drcStruct)
   {
      _drcStruct = DEBUG_NEW DRCBrowser(this, tui::ID_DRC_CELLTREE);
      AddPage(_drcStruct, wxT("DRC results"), true);
   }
}

void browsers::browserTAB::onTellClearDRCTab()
{
   if (_drcStruct)
   {
     int _drcPageIndex = GetPageIndex(_drcStruct);
      assert(wxNOT_FOUND != _drcPageIndex);
      _drcStruct->deleteAllItems();
      DeletePage(_drcPageIndex);
      _drcStruct = NULL;
   }
}

//====================================================================
browsers::LayerInfo::LayerInfo(const LayerInfo& lay) :
   _name   (lay._name  ),
   _laydef (lay._laydef)
{}

browsers::LayerInfo::LayerInfo(const std::string &name, const LayerDef& laydef) :
   _name   (name  ),
   _laydef (laydef)
{}

//====================================================================
browsers::LayerButton::LayerButton(wxWindow* parent, wxWindowID id,  const wxPoint& pos ,
                                   const wxSize& size, long style , const wxValidator& /*validator*/ ,
                                   const wxString& name, LayerInfo* layer):wxPanel()
{
   Bind(wxEVT_LEFT_DOWN  ,&LayerButton::onLeftClick  , this);
   Bind(wxEVT_MIDDLE_DOWN,&LayerButton::onMiddleClick, this);
   Bind(wxEVT_RIGHT_DOWN ,&LayerButton::onRightClick , this);
   Bind(wxEVT_PAINT      ,&LayerButton::onPaint      , this);

   _layer   = DEBUG_NEW LayerInfo(*layer);
   _selected= false;
   _hidden  = false;
   _locked  = false;
   _picture = NULL;
   layprop::tellRGB col(0,0,0,0);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      _filled  = drawProp->layerFilled(_layer->laydef());
      const byte* ifill = drawProp->getFill(_layer->laydef());
      col   = drawProp->getColor(_layer->laydef());
      _brush = tui::makeBrush(ifill, col);
   }
   PROPC->unlockDrawProp(drawProp, false);
   wxColour color(col.red(), col.green(), col.blue());

   _pen = DEBUG_NEW wxPen();

   _pen->SetColour(color);
   _brush->SetColour(color);

   Create(parent, id,  pos, size, style, name);
   GetClientSize(&_buttonWidth, &_buttonHeight);
   //***Draw main picture***
   preparePicture();

   wxString caption;
   caption << wxT("{") << _layer->laydef().num()
           << wxT(",") << _layer->laydef().typ()
           << wxT("}") << wxString(_layer->name().c_str(),wxConvUTF8);
   SetToolTip(caption);

}

void browsers::LayerButton::preparePicture()
{
   const int clearence = 2;
   int sizeX, sizeY;
   GetParent()->GetClientSize(&sizeX, &sizeY);
   _buttonWidth = sizeX;
   SetSize(_buttonWidth, _buttonHeight);

   wxMemoryDC DC;
   wxFont font(10,wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
   DC.SetFont(font);
   //pict.SetWidth(_buttonWidth);
   if (_picture)  delete _picture;
   _picture = DEBUG_NEW wxBitmap(_buttonWidth-16, _buttonHeight, -1);
   DC.SelectObject(*_picture);


   DC.SetPen(*_pen);
   DC.SetBackground(*wxBLACK);
   DC.SetTextForeground(*wxWHITE);

   DC.Clear();
   int curw = clearence;

//   char temp[100];
//   sprintf(temp, "%3i", _layer->laydef().num());
//   wxString layno(temp, wxConvUTF8);
//   int hno,wno;
//   DC.GetTextExtent(layno, &wno, &hno);
//   DC.DrawText(layno, 0, int((_buttonHeight - hno)/2));
//   curw += wno + clearence;

   if (_selected)
   {
      DC.SetBrush(*wxWHITE_BRUSH);
      DC.SetTextForeground(*wxBLACK);
   }
   else
   {
      DC.SetBrush(*wxBLACK_BRUSH);
      DC.SetTextForeground(*wxWHITE);
   }
   const wxString dummy= _T("WWWWWWWWWW");
   wxString caption(_layer->name().c_str(),wxConvUTF8);
   if (caption.Len()>10)
   {
      caption[7]=wxT('.');
      caption[8]=wxT('.');
      caption[9]=wxT('.');
   }
   int hna,wna;
   DC.GetTextExtent(dummy, &wna, &hna);
   DC.DrawRectangle(curw, clearence, wna, _buttonHeight - 2*clearence);
   curw += clearence;
   DC.DrawText(caption, curw, int(_buttonHeight/2 - hna/2));
   curw += wna;

   if (_filled)
   {
      DC.SetBrush(*_brush);
   }
   else
   {
      DC.SetBrush(wxNullBrush);
      DC.SetBrush(*wxBLACK_BRUSH);
      DC.SetTextForeground(*wxWHITE);
   }
   DC.DrawRectangle(curw, clearence, _buttonWidth-curw-16, _buttonHeight-2*clearence);

   DC.SelectObject(wxNullBitmap);
   Refresh();
}

void browsers::LayerButton::onPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.DrawBitmap(*_picture, 0, 0, false);
   if (_selected)
   {
      dc.DrawIcon(wxIcon(activelay),_buttonWidth-16,15);
   }
   else
   {
      if (_locked) dc.DrawIcon(wxIcon(lock)     ,_buttonWidth-16,15);
      if (_hidden) dc.DrawIcon(wxIcon(nolay_xpm),_buttonWidth-16,0 );
   }
}

void browsers::LayerButton::onLeftClick(wxMouseEvent &event)
{
   wxString cmd;
   if (event.ShiftDown())
   {//Hide layer
      cmd << wxT("hidelayer({") << _layer->laydef().num() << wxT(", ") << _layer->laydef().typ() << wxT("}, ");
      if (_hidden) cmd << wxT("false") << wxT(");");
      else cmd << wxT("true") << wxT(");");
   }
   else if (event.ControlDown())
   {// Lock Layer
      cmd << wxT("locklayer({") << _layer->laydef().num() << wxT(", ") << _layer->laydef().typ() << wxT("}, ");
      if (_locked) cmd << wxT("false") << wxT(");");
      else cmd << wxT("true") << wxT(");");
   }
   else
   {//Select layer
      cmd << wxT("usinglayer({") << _layer->laydef().num() << wxT(", ") << _layer->laydef().typ() << wxT("});");
   }
   TpdPost::parseCommand(cmd);
}

void browsers::LayerButton::onMiddleClick(wxMouseEvent& /*event*/)
{
   wxString cmd;
   cmd << wxT("filllayer({") << _layer->laydef().num() << wxT(", ") << _layer->laydef().typ() << wxT("}, ");
   if (_filled) cmd << wxT("false") << wxT(");");
      else cmd << wxT("true") << wxT(");");
   TpdPost::parseCommand(cmd);
}

void browsers::LayerButton::onRightClick(wxMouseEvent& /*event*/)
{
   wxMenu menu(GetToolTip()->GetTip());
//   wxString boza = ;
//   menu.SetTitle(boza);
   menu.Append(LAYERCURRENTEDIT, wxT("Edit layer...")); //if selected call LayerButton::OnEditLayer tui::TMLAY_EDIT
   PopupMenu(&menu);
}

void  browsers::LayerButton::OnEditLayer(wxCommandEvent&)
{
   wxCommandEvent eventEditLayer(tui::wxEVT_EDITLAYER);
   LayerDef* laydef = DEBUG_NEW LayerDef(_layer->laydef());
   eventEditLayer.SetClientData(static_cast<void*>(laydef));
   // This is supposed to work according to the wx documentation, but compiler
   // says ... not a member of wxWindow ...
   //ProcessWindowEvent(eventEditLayer);
   GetEventHandler()->ProcessEvent(eventEditLayer);
}


void browsers::LayerButton::hideLayer(bool hide)
{
   _hidden = hide;
   preparePicture();
}

void browsers::LayerButton::lockLayer(bool locklay)
{
   _locked = locklay;
   preparePicture();
}

void browsers::LayerButton::fillLayer(bool fill)
{
   _filled = fill;
   preparePicture();
}

void browsers::LayerButton::selectLayer(bool slct)
{
   _selected = slct;
   preparePicture();
}

browsers::LayerButton::~LayerButton()
{
   delete _picture;
   delete _brush;
   delete _pen;
   delete _layer;
}

//====================================================================
//
//  Layer Panel
//
//====================================================================
browsers::LayerPanel::LayerPanel(wxWindow* parent, wxWindowID id,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style , const wxString& name)
                              :wxScrolledWindow(parent, id, pos, size, style, name)
{
   _buttonCount = 0;
   Bind(tui::wxEVT_CMD_BROWSER, &browsers::LayerPanel::onCommand, this);
   Bind(wxEVT_PAINT           , &browsers::LayerPanel::onPaint  , this);
}

browsers::LayerPanel::~LayerPanel()
{
}

browsers::LayerButton* browsers::LayerPanel::checkDefined(const LayerDef& key)
{
   if (_buttonMap.end() == _buttonMap.find(key)) return NULL;
   return _buttonMap[key];
}

void browsers::LayerPanel::onCommand(wxCommandEvent& event)
{
   int command = event.GetInt();
   LayerButton* wbutton;
   switch (command)
   {
      case tui::BT_LAYER_DEFAULT:
      {
         std::pair<LayerDef, LayerDef>* laydefs = static_cast<std::pair<LayerDef, LayerDef>*> (event.GetClientData());
         if ((wbutton = checkDefined(laydefs->second))) wbutton->selectLayer(false);
         if ((wbutton = checkDefined(laydefs->first ))) wbutton->selectLayer(true);
         delete (laydefs);
         break;
      }
      case    tui::BT_LAYER_HIDE:
      {
         LayerDef *laydef = static_cast<LayerDef*>(event.GetClientData());
         bool status = (1 == event.GetExtraLong());
         if ((wbutton = checkDefined(*laydef))) wbutton->hideLayer(status);
         delete (laydef);
         break;
      }
      case    tui::BT_LAYER_LOCK:
      {
         //_layerlist->lockLayer((word)event.GetExtraLong(),event.IsChecked());
         LayerDef *laydef = static_cast<LayerDef*>(event.GetClientData());
         bool status = (1 == event.GetExtraLong());
         if ((wbutton = checkDefined(*laydef))) wbutton->lockLayer(status);
         delete (laydef);
         break;
      }
      case    tui::BT_LAYER_FILL:
      {
         LayerDef *laydef = static_cast<LayerDef*>(event.GetClientData());
         bool status = (1 == event.GetExtraLong());
         if ((wbutton = checkDefined(*laydef))) wbutton->fillLayer(status);
         delete (laydef);
         break;
      }
      case     tui::BT_LAYER_ADD:
      {
         LayerDef *laydef = static_cast<LayerDef*>(event.GetClientData());
         std::string name = std::string(event.GetString().mb_str(wxConvFile ));
         LayerInfo layer(name, *laydef);
         addButton(layer);
         delete (laydef);
         break;
      }
      default: event.Skip();break;
   }
}

void  browsers::LayerPanel::addButton(LayerInfo& layer)
{
   LayerButton* wbutton;
   int szx, szy;
   if ((wbutton = checkDefined( layer.laydef() )))
   {
      //Button already exists, replace it
      //layerButton = DEBUG_NEW LayerButton(this, tui::TMDUMMY_LAYER+_buttonCount, wxPoint (0, _buttonCount*30), wxSize(200, 30),
      //wxBU_AUTODRAW, wxDefaultValidator, _T("TTT"), layer);
      int x, y;
      int ID;
      wbutton->GetPosition(&x, &y);
      wbutton->GetSize(&szx, &szy);
      ID = wbutton->GetId();
      LayerButton* layerButton = DEBUG_NEW LayerButton(this, ID, wxPoint (x, y), wxSize(szx, szy),
         wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("button"), &layer);
      _buttonMap[layer.laydef()] = layerButton;
      delete wbutton;
   }
   else
   {
      //Button doesn't exist, create new button
      GetClientSize(&szx, &szy);
      LayerButton* layerButton = DEBUG_NEW LayerButton(this, tui::TMDUMMY_LAYER+_buttonCount,
                                          wxPoint (0, _buttonCount*buttonHeight), wxSize(szx, buttonHeight),
                                          wxBU_AUTODRAW, wxDefaultValidator, _T("button"), &layer);
      _buttonMap[layer.laydef()] = layerButton;
      _buttonCount++;
      this->SetScrollbars(0, buttonHeight, 0, _buttonCount);
      //Reorder buttons
      int number = 0;
      for(LayerButtonMap::iterator it=_buttonMap.begin() ;it!=_buttonMap.end(); ++it, ++number)
      {
         LayerButton* tempButton = (*it).second;
         wxPoint point = wxPoint(0, number*buttonHeight);
         tempButton->Move(point);
      }
   }
}

void  browsers::LayerPanel::onPaint(wxPaintEvent& evt)
{
   for(LayerButtonMap::const_iterator it = _buttonMap.begin(); it!=_buttonMap.end();++it)
      it->second->preparePicture();
   evt.Skip();
}

wxString browsers::LayerPanel::getAllSelected()
{
   if (_buttonMap.empty()) return wxEmptyString;
   wxString layers = wxT("{");
   for(LayerButtonMap::iterator it = _buttonMap.begin(); it != _buttonMap.end(); it++)
   {
      LayerDef laydef((it->second)->getLayDef());
      layers << wxT(" {") <<  laydef.num() << wxT(",") <<  laydef.typ() << wxT("} ,");
   }
   layers.RemoveLast();
   layers << wxT("}");
   return layers;
}

//====================================================================
//
// Layer Browser
//
//====================================================================
browsers::LayerBrowser::LayerBrowser(wxWindow* parent, wxWindowID id)
   :wxPanel(parent, id, wxDefaultPosition, wxDefaultSize),
   _layerPanel(NULL),
   _thesizer(NULL)
{
   Bind(wxEVT_BUTTON           , &browsers::LayerBrowser::onShowAll   , this, tui::BT_LAYER_SHOW_ALL  );
   Bind(wxEVT_BUTTON           , &browsers::LayerBrowser::onHideAll   , this, tui::BT_LAYER_HIDE_ALL  );
   Bind(wxEVT_BUTTON           , &browsers::LayerBrowser::onLockAll   , this, tui::BT_LAYER_LOCK_ALL  );
   Bind(wxEVT_BUTTON           , &browsers::LayerBrowser::onUnlockAll , this, tui::BT_LAYER_UNLOCK_ALL);
   Bind(wxEVT_BUTTON           , &browsers::LayerBrowser::onSaveState , this, tui::BT_LAYER_SAVE_ST   );
   Bind(wxEVT_BUTTON           , &browsers::LayerBrowser::onLoadState , this, tui::BT_LAYER_LOAD_ST   );
   Bind(tui::wxEVT_CMD_BROWSER , &browsers::LayerBrowser::onCommand   , this);

   _thesizer = DEBUG_NEW wxBoxSizer(wxVERTICAL);

   wxBoxSizer* sizer1 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
   sizer1->Add(DEBUG_NEW wxButton(this, tui::BT_LAYER_SHOW_ALL, wxT("Show All")), 1, wxTOP|wxEXPAND, 3);
   sizer1->Add(DEBUG_NEW wxButton(this, tui::BT_LAYER_HIDE_ALL, wxT("Hide All")), 1, wxTOP|wxEXPAND, 3);
   _thesizer->Add(sizer1, 0, wxTOP|wxEXPAND);

   wxBoxSizer* sizer2 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
   sizer2->Add(DEBUG_NEW wxButton(this, tui::BT_LAYER_LOCK_ALL, wxT("Lock All")), 1, wxTOP, 3);
   sizer2->Add(DEBUG_NEW wxButton(this, tui::BT_LAYER_UNLOCK_ALL, wxT("Unlock All")), 1, wxTOP, 3);
   _thesizer->Add(sizer2, 0, wxTOP|wxEXPAND);

   _layerPanel = DEBUG_NEW LayerPanel(this, tui::ID_PNL_LAYERS, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
   _thesizer->Add(_layerPanel, 3, wxEXPAND|wxALL);

   wxBoxSizer* sizer3t = DEBUG_NEW wxBoxSizer(wxVERTICAL);
   wxBoxSizer* sizer3 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
   sizer3->Add(DEBUG_NEW wxButton(this, tui::BT_LAYER_SAVE_ST, wxT("Save State")), 1, wxTOP|wxEXPAND, 3);
   sizer3->Add(DEBUG_NEW wxButton(this, tui::BT_LAYER_LOAD_ST, wxT("Load State")), 1, wxTOP|wxEXPAND, 3);
   sizer3t->Add(sizer3, 1, wxTOP|wxEXPAND);
   sizer3t->Add(DEBUG_NEW wxComboBox(this, tui::BT_LAYER_STATES), 0, wxTOP|wxEXPAND);
   _thesizer->Add(sizer3t, 0, wxBOTTOM|wxEXPAND);
   SetSizerAndFit(_thesizer);
   _thesizer->SetSizeHints( this );
}

browsers::LayerBrowser::~LayerBrowser()
{
}


void browsers::LayerBrowser::onShowAll(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   wxString layers=getAllSelected();
   if (layers != wxEmptyString)
   {
      cmd << wxT("hidelayer(") << getAllSelected() << wxT(", false);");
      TpdPost::parseCommand(cmd);
   }
}

void browsers::LayerBrowser::onHideAll(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   wxString layers=getAllSelected();
   if (layers != wxEmptyString)
   {
      cmd << wxT("hidelayer(") << getAllSelected() << wxT(", true);");
      TpdPost::parseCommand(cmd);
   }
}

void browsers::LayerBrowser::onLockAll(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   wxString layers=getAllSelected();
   if (layers != wxEmptyString)
   {
      cmd << wxT("locklayer(") << getAllSelected() << wxT(", true);");
      TpdPost::parseCommand(cmd);
   }

}

void browsers::LayerBrowser::onUnlockAll(wxCommandEvent& WXUNUSED(event))
{
   wxString cmd;
   wxString layers=getAllSelected();
   if (layers != wxEmptyString)
   {
      cmd << wxT("locklayer(") << getAllSelected() << wxT(", false);");
      TpdPost::parseCommand(cmd);
   }
}

void browsers::LayerBrowser::onSaveState(wxCommandEvent& WXUNUSED(event))
{
   wxComboBox* layerState = static_cast<wxComboBox*>(FindWindow(tui::BT_LAYER_STATES));
   wxString stateName = layerState->GetValue();
   if (!stateName.IsEmpty())
   {
      wxString cmd;
//      layerState->Append(stateName);
      cmd << wxT("savelaystatus(\"") << stateName << wxT("\");");
      TpdPost::parseCommand(cmd);
   }
   else
   {
      std::ostringstream ost;
      ost<<"Name of Layers State is not specified";
      tell_log(console::MT_ERROR,ost.str());
      return;
   }
}

void browsers::LayerBrowser::onLoadState(wxCommandEvent& WXUNUSED(event))
{
   wxComboBox* layerState = static_cast<wxComboBox*>(FindWindow(tui::BT_LAYER_STATES));
   wxString stateName = layerState->GetValue();
   if (!stateName.IsEmpty())
   {
      wxString cmd;
      cmd << wxT("restorelaystatus(\"") << stateName << wxT("\");");
      TpdPost::parseCommand(cmd);
   }
   else
   {
      std::ostringstream ost;
      ost<<"Name of Layers State is not specified";
      tell_log(console::MT_ERROR,ost.str());
      return;
   }
}

void browsers::LayerBrowser::onCommand(wxCommandEvent& event)
{
   int command = event.GetInt();
   switch (command)
   {
      case tui::BT_LAYSTATE_SAVE:
      {
         wxString stateName = event.GetString();
         wxComboBox* layerState = static_cast<wxComboBox*>(FindWindow(tui::BT_LAYER_STATES));
         layerState->Append(stateName);
         break;
      }
      case tui::BT_LAYSTATE_DELETE:
      {
         wxString stateName = event.GetString();
         wxComboBox* layerState = static_cast<wxComboBox*>(FindWindow(tui::BT_LAYER_STATES));
         layerState->Delete(layerState->FindString(stateName, true));
         break;
      }
//      default: event.Skip();
      default: assert(false);break;
   }
}

wxString browsers::LayerBrowser::getAllSelected()
{
   return _layerPanel->getAllSelected();
}

//====================================================================
browsers::ErrorBrowser::ErrorBrowser(wxWindow* parent, wxWindowID id,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style):
                     wxTreeCtrl(parent, id, pos, size, style |/*wxTR_HIDE_ROOT|*/ wxTR_FULL_ROW_HIGHLIGHT )
{
   Bind(wxEVT_LEFT_DCLICK          ,&browsers::ErrorBrowser::onLMouseDblClk  ,this );
//   Bind(wxEVT_RIGHT_UP             ,&browsers::ErrorBrowser::onBlankRMouseUp ,this );
   Bind(wxEVT_TREE_ITEM_RIGHT_CLICK,&browsers::ErrorBrowser::onItemRightClick,this, tui::ID_PNL_DRC        );
   Bind(wxEVT_MENU                 ,&browsers::ErrorBrowser::onOpenCell      ,this, tui::TMDRC_OPEN_CELL   );
   Bind(wxEVT_MENU                 ,&browsers::ErrorBrowser::onShowError     ,this, tui::TMDRC_SHOW_ERR    );
   Bind(wxEVT_MENU                 ,&browsers::ErrorBrowser::onShowCluster   ,this, tui::TMDRC_SHOW_CLUSTER);
}


void browsers::ErrorBrowser::onLMouseDblClk(wxMouseEvent& event)
{
   wxString cmd; 
   int flags;
   wxPoint pt = event.GetPosition();
   wxTreeItemId id = HitTest(pt, flags);
   if (id.IsOk() &&(flags & wxTREE_HITTEST_ONITEMLABEL))
   {
      if (ItemHasChildren(id))
      {
         if(IsExpanded(id)) Collapse(id); else Expand(id);
      }
      else
      {
         wxString numstr = GetItemText(id);
         long number;
         numstr.ToLong(&number);

         wxTreeItemId parent = GetItemParent(id);
         wxString wxError = GetItemText(parent);
         std::string error(wxError.mb_str(wxConvUTF8));
         //Check if Cell Name Mode
         wxTreeItemId parent2 = GetItemParent(parent);
         //Check if parent2 is cell name
          DRCItemData *data = static_cast<DRCItemData *>(GetItemData(parent2));
         if (data->getType() == ITEM_CELL)
         {
            wxString wxCell = GetItemText(parent2);
            std::string cell(wxCell.mb_str(wxConvUTF8));
            if(!checkCellName(cell))
            {
               cmd << wxT("opencell(\"") << wxCell <<wxT("\");");
            }
         }

         cmd << wxT("drcshowerror(\"") <<  wxError << wxT("\", ") << numstr << wxT("  );");
         TpdPost::parseCommand(cmd);
      }
   }
   else
      event.Skip();
}

void browsers::ErrorBrowser::onItemRightClick(wxTreeEvent& event)
{
   showMenu(event.GetItem(), event.GetPoint());
}

//void browsers::ErrorBrowser::onBlankRMouseUp(wxMouseEvent& event)
//{
//   wxPoint pt = event.GetPosition();
//   showMenu(HitTest(pt), pt);
//}


void browsers::ErrorBrowser::onOpenCell(wxCommandEvent& /*event*/)
{
   wxString cmd;
   cmd << wxT("opencell(\"") << GetItemText(_rbCellID) <<wxT("\");");
   TpdPost::parseCommand(cmd);
}


void browsers::ErrorBrowser::onShowError(wxCommandEvent& /*event*/)
{
   wxString cmd;
   wxTreeItemId parent = GetItemParent(_rbCellID);
   wxTreeItemId parent2 = GetItemParent(parent);
   //Check if parent2 is cell name
   DRCItemData *data = static_cast<DRCItemData *>(GetItemData(parent2));
   if (data->getType() == ITEM_CELL)
   {
     wxString wxCell = GetItemText(parent2);
      std::string cell(wxCell.mb_str(wxConvUTF8));
      if(!checkCellName(cell))
      {
         cmd << wxT("opencell(\"") << wxCell <<wxT("\");");
      }
   }

   cmd << wxT("drcshowerror(\"") << _cluster << wxT("\", ") << GetItemText(_rbCellID) <<wxT(");");
   TpdPost::parseCommand(cmd);
}

void browsers::ErrorBrowser::onShowCluster(wxCommandEvent& /*event*/)
{
   wxString cmd;

   wxTreeItemId parent = GetItemParent(_rbCellID);
   //Check if parent is cell name
   DRCItemData *data = static_cast<DRCItemData *>(GetItemData(parent));
   if (data->getType() == ITEM_CELL)
   {
     wxString wxCell = GetItemText(parent);
      std::string cell(wxCell.mb_str(wxConvUTF8));
      if(!checkCellName(cell))
      {
         cmd << wxT("opencell(\"") << wxCell <<wxT("\");");
      }
   }

   cmd << wxT("drcshowcluster(\"") << _cluster <<wxT("\");");
   TpdPost::parseCommand(cmd);
}

void browsers::ErrorBrowser::showMenu(wxTreeItemId id, const wxPoint& pt)
{
   wxMenu menu;
   _rbCellID = id;
   if (!id.IsOk()) return;
   DRCItemData *data = static_cast<DRCItemData *>(GetItemData(id));
   if (data != NULL)
   {
      int type=data->getType();
      switch(type)
      {
      case ITEM_CELL:
         menu.Append(tui::TMDRC_OPEN_CELL, wxT("Open Cell"));
         break;
      case ITEM_ERR:
        menu.Append(tui::TMDRC_SHOW_CLUSTER, wxT("Show cluster"));
         _cluster = GetItemText(id);
         break;
      case ITEM_ERR_NUM:
         menu.Append(tui::TMDRC_SHOW_ERR, wxT("Show Error"));
         _cluster = GetItemText(GetItemParent(id));
         break;
      default:
         assert(true);break;
      }
   }
   PopupMenu(&menu, pt);
}

bool browsers::ErrorBrowser::checkCellName(const std::string &str)
{
   laydata::TdtLibDir *libDir;
   std::string activeCell;
   DATC->lockTDT(libDir, dbmxs_liblock);
      laydata::TdtDesign *design = (*libDir)();
      activeCell = design->activeCellName();
   DATC->unlockTDT(libDir);
   bool ret = (0 != activeCell.compare(str));
   return !ret;
}

//====================================================================
browsers::DRCBrowser::DRCBrowser(wxWindow* parent, wxWindowID id)
   :wxPanel(parent, id, wxDefaultPosition, wxDefaultSize)
{
   Bind(wxEVT_BUTTON, &browsers::DRCBrowser::onShowAll       , this, tui::BT_DRC_SHOW_ALL  );
   Bind(wxEVT_BUTTON, &browsers::DRCBrowser::onHideAll       , this, tui::BT_DRC_HIDE_ALL  );
   Bind(wxEVT_BUTTON, &browsers::DRCBrowser::onRulesHierarchy, this, tui::BT_DRC_RULES_HIER);
   Bind(wxEVT_BUTTON, &browsers::DRCBrowser::onCellsHierarchy, this, tui::BT_DRC_CELLS_HIER);

   wxBoxSizer *thesizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   wxBoxSizer *sizer1   = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   wxBoxSizer *sizer2   = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   _showAllButton = DEBUG_NEW wxButton( this, tui::BT_DRC_SHOW_ALL, wxT("Show All") );
   _hideAllButton = DEBUG_NEW wxButton( this, tui::BT_DRC_HIDE_ALL, wxT("Hide All") );
   //Set bold font for _hierButton

   sizer1->Add(_showAllButton, 1, wxEXPAND|wxBOTTOM, 3);
   sizer1->Add(_hideAllButton, 1, wxEXPAND|wxBOTTOM, 3);
   _errorBrowser = DEBUG_NEW ErrorBrowser(this, tui::ID_PNL_DRC);
   thesizer->Add(sizer1, 0, wxEXPAND | wxALL);
   thesizer->Add(_errorBrowser, 1, wxEXPAND | wxBOTTOM);
//   _explainButton = DEBUG_NEW wxButton( this, tui::BT_DRC_EXPLAIN, wxT("Explain error") );

//   wxBoxSizer *sizer2   = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   _rulesButton = DEBUG_NEW wxButton( this, tui::BT_DRC_RULES_HIER, wxT("Rules") );
   _cellsButton = DEBUG_NEW wxButton( this, tui::BT_DRC_CELLS_HIER, wxT("Cells") );
   wxFont font = _rulesButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _rulesButton->SetFont(font);

   sizer2->Add(_rulesButton, 1, wxEXPAND|wxBOTTOM, 3);
   sizer2->Add(_cellsButton, 1, wxEXPAND|wxBOTTOM, 3);

//   sizer2->Add(_explainButton, 1, wxEXPAND|wxBOTTOM, 3);
   thesizer->Add(sizer2, 0, wxEXPAND | wxALL);

   SetSizerAndFit(thesizer);

   showRuleHierarchy();

}

void browsers::DRCBrowser::deleteAllItems(void)
{
   _errorBrowser->DeleteAllItems();
}

void   browsers::DRCBrowser::onShowAll(wxCommandEvent& /*event*/)
{
   wxString cmd;
//   wxString cell=wxString(DRCData->topCellName().c_str(),  wxConvUTF8);
//   cmd << wxT("opencell(\"") << cell <<wxT("\");");
   cmd << wxT("drcshowallerrors();");
   TpdPost::parseCommand(cmd);
}

void   browsers::DRCBrowser::onHideAll(wxCommandEvent& /*eveint*/)
{
   wxString cmd;
   cmd << wxT("drchideallerrors();");
   TpdPost::parseCommand(cmd);
 }

//void   browsers::DRCBrowser::onExplainError(wxCommandEvent& evt)
//{
//   wxString cmd;
//   cmd << wxT("drcexplainerror();");
//   TpdPost::parseCommand(cmd);
// }

void   browsers::DRCBrowser::onRulesHierarchy(wxCommandEvent& /*event*/)
{
   deleteAllItems();
   showRuleHierarchy();
   //Set normal font for  _rulesButton
   wxFont font = _rulesButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _rulesButton->SetFont(font);
   //Set bold font for _cellsButton;
   font = _cellsButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_NORMAL);
   _cellsButton->SetFont(font);

   Show();
   (this->GetSizer())->Layout();
}

void browsers::DRCBrowser::showRuleHierarchy()
{
   clbr::DrcLibrary* drcDB = NULL;
   if (DATC->lockDRC(drcDB))
   {
      _errorBrowser->AddRoot(wxString(drcDB->name().c_str(), wxConvUTF8), -1, -1, DEBUG_NEW DRCItemData(ITEM_ROOT));

      const clbr::RuleNameMap* ruleMap = drcDB->rules();
      for (clbr::RuleNameMap::const_iterator CIT = ruleMap->begin(); CIT != ruleMap->end(); CIT++)
      {
//         wxTreeItemId cellId = addCellCheck(_errorBrowser->GetRootItem(), CIT->first);
         wxTreeItemId cellId = _errorBrowser->AppendItem(_errorBrowser->GetRootItem(), wxString(CIT->first.c_str(), wxConvUTF8), -1, -1, DEBUG_NEW DRCItemData(ITEM_ERR));
         const NameList* cells = CIT->second;
         for (NameList::const_iterator CIC = cells->begin(); CIC != cells->end(); CIC++)
         {
//            addRuleCheck(cellId, CIC->first, CIC->second);
            _errorBrowser->AppendItem(cellId, wxString(CIC->c_str(), wxConvUTF8), -1, -1, DEBUG_NEW DRCItemData(ITEM_CELL));
         }
         delete cells;
      }
      delete ruleMap;
   }
   DATC->unlockDRC(drcDB);
}

void browsers::DRCBrowser::onCellsHierarchy(wxCommandEvent& /*event*/)
{
   deleteAllItems();
   showCellHierarchy();
   //Set normal font for  _rulesButton
   wxFont font = _rulesButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_NORMAL);
   _rulesButton->SetFont(font);
   //Set bold font for _cellsButton;
   font = _cellsButton->GetFont();
   font.SetWeight(wxFONTWEIGHT_BOLD);
   _cellsButton->SetFont(font);

   Show();
   (this->GetSizer())->Layout();
}

void  browsers::DRCBrowser::showCellHierarchy()
{
   clbr::DrcLibrary* drcDB = NULL;
   if (DATC->lockDRC(drcDB))
   {
      _errorBrowser->AddRoot(wxString(drcDB->name().c_str(), wxConvUTF8), -1, -1, DEBUG_NEW DRCItemData(ITEM_ROOT));
      const clbr::CellMap* cells = drcDB->cells();
      for(clbr::CellMap::const_iterator CIT = cells->begin();CIT != cells->end(); ++CIT)
      {

         wxTreeItemId cellId = addCellCheck(_errorBrowser->GetRootItem(), CIT->first );

         const clbr::RuleMap* rules = CIT->second->rules();
         for(clbr::RuleMap::const_iterator RIT = rules->begin();RIT != rules->end(); ++RIT)
         {
            addRuleCheck(cellId, RIT->first, RIT->second);
         }
      }
   }
   DATC->unlockDRC(drcDB);
}

void  browsers::DRCBrowser::addRuleCheck( const wxTreeItemId &rootId, std::string name, clbr::DrcRule* /*check*/)
{
//   std::string name = check->ruleCheckName();
   /*wxTreeItemId  id = */_errorBrowser->AppendItem(rootId, wxString(name.c_str(), wxConvUTF8), -1, -1, DEBUG_NEW DRCItemData(ITEM_ERR));
//   std::vector <Calbr::drcPolygon>::iterator it2;
//   std::vector <Calbr::drcPolygon> *polys = check->polygons();
//
//   //Save polygons
////   long sz = polys->size();
//   for(std::vector <Calbr::drcPolygon>::const_iterator it = polys->begin(); it != polys->end(); ++it)
//   {
//      wxString str;
//      str << it->ordinal();
//      _errorBrowser->AppendItem(id, str, -1, -1, DEBUG_NEW DRCItemData(ITEM_ERR_NUM));
//   }
//
//   //Save Edges
//   std::vector <Calbr::drcEdge> *edges = check->edges();
////   sz = edges->size();
//   for(std::vector <Calbr::drcEdge>::const_iterator it = edges->begin(); it != edges->end(); ++it)
//   {
//      wxString str;
//      str << it->ordinal();
//      _errorBrowser->AppendItem(id, str, -1, -1, DEBUG_NEW DRCItemData(ITEM_ERR_NUM));
//   }
}


wxTreeItemId browsers::DRCBrowser::addCellCheck( const wxTreeItemId &rootId, std::string name/*, Calbr::DrcCell* check*/)
{
   return _errorBrowser->AppendItem(rootId, wxString(name.c_str(), wxConvUTF8), -1, -1, DEBUG_NEW DRCItemData(ITEM_CELL));
}


browsers::DRCBrowser::~DRCBrowser()
{
}
