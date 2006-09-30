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
//   This file is a part of Toped project (C) 2001-2006 Toped developers    =
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

#include "browsers.h"
#include "../tpd_DB/viewprop.h"
#include "layoutcanvas.h"
#include "../tpd_common/outbox.h"
#include "../tpd_parser/ted_prompt.h"
#include "tui.h"
#include "datacenter.h"
#include "../ui/activelay.xpm"
#include "../ui/lock.xpm"
#include "../ui/cell_normal.xpm"
#include "../ui/cell_expanded.xpm"


extern console::ted_cmd*         Console;
extern layprop::ViewProperties*  Properties;
extern DataCenter*               DATC;
extern browsers::browserTAB*     Browsers;
extern const wxEventType         wxEVT_CMD_BROWSER;

//==============================================================================
browsers::topedlay_list::topedlay_list(wxWindow *parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style) : 
                                       wxListCtrl(parent, id, pos, size, style) {
   InsertColumn(0, wxT("  No    "));
   InsertColumn(1, wxT("     Name     "));
   InsertColumn(2, wxT(" S "));
   SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
   _imageList = new wxImageList(16, 16, TRUE);
#ifdef __WXMSW__
/*@TODO : Under windows - resource loading*/
//    m_imageListNormal->Add( wxIcon(_T("icon1"), wxBITMAP_TYPE_ICO_RESOURCE) );
//
#else
   //SGREM!!! Troubles with the gdb on Linux with threads!
   // I spent a night debugging a stupid mistake with traversing the tree
   // to realize finally that the gdb is doing some funny things when 
   // stepping over next two lines. The troble comes from wxIcon constructor, 
   // that internally is calling gdk_pixmap_create_from_xpm_d
    _imageList->Add( wxIcon( activelay ) );
    _imageList->Add( wxIcon( lock      ) );
#endif
//   SetBackgroundColour(wxColour("LIGHTGREY"));
   SetImageList(_imageList,wxIMAGE_LIST_SMALL);
   _llfont_normal.SetPointSize(9);
   _llfont_bold.SetPointSize(9);
   _llfont_bold.SetWeight(wxBOLD);
}

browsers::topedlay_list::~topedlay_list() {
   delete _imageList;
   DeleteAllItems();
}


void browsers::topedlay_list::addlayer(wxString name, word layno) {
   int item = -1;
   for(;;) {
      item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
      if (item == -1) break;
      long oldno = GetItemData(item);
      if (oldno == layno) DeleteItem(item);
   }
   wxString num; num.Printf(_T("%3d"), layno);
   wxListItem row;
   row.SetMask(wxLIST_MASK_DATA | wxLIST_MASK_TEXT);
   row.SetData(layno); row.SetText(num); row.SetId(GetItemCount());
   row.SetFont(_llfont_normal);
   long inum = InsertItem(row);
   SetItem(inum, 1, name);
   // second column is reserved for active/lock icon
   row.SetColumn(2); row.SetId(inum); row.SetMask(wxLIST_MASK_IMAGE);
   row.SetImage(-1); // No icon assigned initially
   SetItem(row);
}   

void browsers::topedlay_list::defaultLayer(word newl, word oldl) {
   wxListItem info;
   // first remove the icon from the previous default layer
   long ID = FindItem(-1, oldl);
   info.SetId(ID);info.SetColumn(2);
   if ( GetItem(info) ) {
      info.SetMask(wxLIST_MASK_IMAGE);info.SetImage(-1);
      SetItem(info);
   }
   info.SetColumn(0);
   if ( GetItem(info) ) {
      info.SetFont(_llfont_normal);
      SetItem(info);
   }   
   RefreshItem(ID);
   // Now attach the Icon to the current default layer
   ID = FindItem(-1, newl);
   info.SetId(ID);info.SetColumn(2);
   if ( GetItem(info) ) {
      info.SetMask(wxLIST_MASK_IMAGE);info.SetImage(0);
      SetItem(info);
   }   
   info.SetColumn(0);
   if ( GetItem(info) ) {
      info.SetFont(_llfont_bold);
      SetItem(info);
   }
   RefreshItem(ID);
}

void browsers::topedlay_list::hideLayer(word layno, bool hide) {
   long ID = FindItem(-1, layno);
   wxListItem info;
   info.SetId(ID);
   if ( GetItem(info) ) {
      info.SetTextColour((hide ? *wxLIGHT_GREY : *wxBLACK));
      SetItem(info);
      RefreshItem(ID);
   }   
}

void browsers::topedlay_list::lockLayer(word layno, bool lock) {
   long ID = FindItem(-1, layno);
   wxListItem info;
   info.SetId(ID);info.SetColumn(2);
   if ( GetItem(info) ) {
      info.SetMask(wxLIST_MASK_IMAGE);
      info.SetImage((lock ? 1 : -1));
      SetItem(info);
      RefreshItem(ID);
   }   
}

//==============================================================================
BEGIN_EVENT_TABLE(browsers::GDSbrowser, wxTreeCtrl)
   EVT_TREE_ITEM_RIGHT_CLICK( ID_TPD_CELLTREE, browsers::GDSbrowser::OnItemRightClick)
   EVT_MENU(GDSTree_ReportLay, browsers::GDSbrowser::OnGDSreportlay)
   EVT_RIGHT_UP(browsers::GDSbrowser::OnBlankRMouseUp)
END_EVENT_TABLE()
//==============================================================================
void browsers::GDSbrowser::collectInfo() {
   GDSin::GDSFile* AGDSDB = DATC->lockGDS(false);
   if (NULL == AGDSDB) return;
   AddRoot(wxString((AGDSDB->Get_libname()).c_str(), wxConvUTF8));
   if (NULL == AGDSDB->hierTree()) return; // new, empty design 
   GDSin::GDSHierTree* root = AGDSDB->hierTree()->GetFirstRoot();
   wxTreeItemId nroot;
   while (root){
      nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->Get_StrName(),wxConvUTF8));
//      SetItemTextColour(nroot,*wxLIGHT_GREY);
      collectChildren(root, nroot);
      root = root->GetNextRoot();
   }
   DATC->unlockGDS();
//   Toped->Resize();
}
      
void browsers::GDSbrowser::collectChildren(GDSin::GDSHierTree *root, wxTreeItemId& lroot) {
   GDSin::GDSHierTree* Child= root->GetChild();
   wxTreeItemId nroot;
   while (Child) {
      nroot = AppendItem(lroot, wxString(Child->GetItem()->Get_StrName(), wxConvUTF8));
//      SetItemTextColour(nroot,*wxLIGHT_GREY);
      collectChildren(Child, nroot);
      Child = Child->GetBrother();
	}
}

void browsers::GDSbrowser::OnItemRightClick(wxTreeEvent& event) {
   ShowMenu(event.GetItem(), event.GetPoint());
}

void browsers::GDSbrowser::OnBlankRMouseUp(wxMouseEvent& event) {
   wxPoint pt = event.GetPosition();
   ShowMenu(HitTest(pt), pt);
}

void browsers::GDSbrowser::ShowMenu(wxTreeItemId id, const wxPoint& pt) {
   wxMenu menu;
   RBcellID = id;
   if ( id.IsOk() && (id != GetRootItem()))   {
      wxString RBcellname = GetItemText(id);
      menu.Append(tui::TMGDS_TRANSLATE, wxT("Translate " + RBcellname));
      menu.Append(GDSTree_ReportLay, wxT("Report layers used in " + RBcellname));
   }
   else {
      menu.Append(tui::TMGDS_CLOSE, wxT("Close GDS")); // will be catched up in toped.cpp
   }
   PopupMenu(&menu, pt);
}

void browsers::GDSbrowser::OnGDSreportlay(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << wxT("report_gdslayers(\"") << GetItemText(RBcellID) <<wxT("\");");
   Console->parseCommand(ost);
}

//==============================================================================
BEGIN_EVENT_TABLE(browsers::TDTbrowser, wxTreeCtrl)
   EVT_TREE_ITEM_RIGHT_CLICK( ID_TPD_CELLTREE, browsers::TDTbrowser::OnItemRightClick)
   EVT_RIGHT_UP(browsers::TDTbrowser::OnBlankRMouseUp)
   EVT_LEFT_DCLICK(browsers::TDTbrowser::OnLMouseDblClk)
   EVT_MENU(CellTree_OpenCell, browsers::TDTbrowser::OnWXOpenCell)
   EVT_MENU(tui::TMCELL_REPORTLAY, browsers::TDTbrowser::OnReportUsedLayers)
   EVT_TECUSTOM_COMMAND(wxEVT_CMD_BROWSER, wxID_ANY, browsers::TDTbrowser::OnCommand)
END_EVENT_TABLE()
//==============================================================================
browsers::TDTbrowser::TDTbrowser(wxWindow *parent, wxWindowID id, 
   const wxPoint& pos, const wxSize& size, long style) : 
                                       wxTreeCtrl(parent, id, pos, size,
                                       style | wxTR_FULL_ROW_HIGHLIGHT) {
   _imageList = new wxImageList(16, 16, TRUE);
#ifdef __WXMSW__
/*TODO : Under windows - resource loading*/
//    m_imageListNormal->Add( wxIcon(_T("icon1"), wxBITMAP_TYPE_ICO_RESOURCE) );
//
#else
    _imageList->Add( wxIcon( cell_normal   ) );
    _imageList->Add( wxIcon( cell_expanded ) );
#endif
   SetImageList(_imageList);
//   _llfont_bold.SetWeight(wxBOLD);
//   _llfont_normal.SetWeight(wxNORMAL);
}

void browsers::TDTbrowser::initialize() {
   DeleteAllItems();
   RBcellID.Unset(); top_structure.Unset(); active_structure.Unset();
}

void browsers::TDTbrowser::collectInfo(const wxString libname, laydata::TDTHierTree* tdtH) {
   AddRoot(libname);
   if (!tdtH) return; // new, empty design 
   laydata::TDTHierTree* root = tdtH->GetFirstRoot();
   wxTreeItemId nroot;
   while (root){
      nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->name().c_str(), wxConvUTF8));
      SetItemTextColour(nroot,*wxLIGHT_GREY);
      collectChildren(root, nroot);
      root = root->GetNextRoot();
   }
}
      
void browsers::TDTbrowser::collectChildren(laydata::TDTHierTree *root, wxTreeItemId& lroot) {
   laydata::TDTHierTree* Child= root->GetChild();
   wxTreeItemId nroot;
   while (Child) {
      SetItemImage(lroot,0,wxTreeItemIcon_Normal);
      SetItemImage(lroot,1,wxTreeItemIcon_Expanded);
      nroot = AppendItem(lroot, wxString(Child->GetItem()->name().c_str(), wxConvUTF8));
      SetItemTextColour(nroot,*wxLIGHT_GREY);
      collectChildren(Child, nroot);
      Child = Child->GetBrother();
	}
}

void browsers::TDTbrowser::OnCommand(wxCommandEvent& event) {
   switch (event.GetInt()) {
      case BT_CELL_OPEN:OnTELLopencell(event.GetString());break;
      case BT_CELL_HIGHLIGHT:OnTELLhighlightcell(event.GetString());break;
      case BT_CELL_ADD :OnTELLaddcell(event.GetString(), 
          *((wxString*)event.GetClientData()), (int)event.GetExtraLong());
          delete ((wxString*)event.GetClientData()); break;
      case BT_CELL_REMOVE:OnTELLremovecell(event.GetString(), 
          *((wxString*)event.GetClientData()), (bool)event.GetExtraLong());
          delete ((wxString*)event.GetClientData()); break;

   }   
}

void browsers::TDTbrowser::OnItemRightClick(wxTreeEvent& event) {
   ShowMenu(event.GetItem(), event.GetPoint());
}

void browsers::TDTbrowser::OnBlankRMouseUp(wxMouseEvent& event) {
   wxPoint pt = event.GetPosition();
   ShowMenu(HitTest(pt), pt);
}

void browsers::TDTbrowser::OnLMouseDblClk(wxMouseEvent& event)
{
   wxPoint pt = event.GetPosition();
   wxTreeItemId id = HitTest(pt);
   if (id.IsOk() && (id != GetRootItem()))
   {
      wxString ost; 
      ost << wxT("opencell(\"") << GetItemText(id) <<wxT("\");");
      Console->parseCommand(ost);

   }
   else event.Skip();
}

void browsers::TDTbrowser::ShowMenu(wxTreeItemId id, const wxPoint& pt) {
    wxMenu menu;
    RBcellID = id;
    if ( id.IsOk() && (id != GetRootItem()))   {
      wxString RBcellname = GetItemText(id);
      menu.Append(CellTree_OpenCell, wxT("Open " + RBcellname));
      menu.Append(tui::TMCELL_REF_B , wxT("Add reference to " + RBcellname));
      menu.Append(tui::TMCELL_AREF_B, wxT("Add array of " + RBcellname));
      wxString ost;
      ost << wxT("export ") << RBcellname << wxT(" to GDS");
      menu.Append(tui::TMGDS_EXPORTC, ost);
      menu.Append(tui::TMCELL_REPORTLAY, wxT("Report layers used in " + RBcellname));
    }
    else {
      menu.Append(tui::TMCELL_NEW, wxT("New cell")); // will be catched up in toped.cpp
      menu.Append(tui::TMGDS_EXPORTL, wxT("GDS export"));
    }
    PopupMenu(&menu, pt);
}

void browsers::TDTbrowser::OnWXOpenCell(wxCommandEvent& WXUNUSED(event)) {
   active_structure = top_structure = RBcellID;
   wxString ost; 
   ost << wxT("opencell(\"") << GetItemText(RBcellID) <<wxT("\");");
   Console->parseCommand(ost);
}

void browsers::TDTbrowser::OnTELLopencell(wxString open_cell) {
   wxTreeItemId item;
   assert(findItem(open_cell, item, GetRootItem()));
   highlightChlidren(GetRootItem(), *wxLIGHT_GREY);
//   if (top_structure.IsOk())
//      SetItemFont(active_structure,_llfont_normal);
   top_structure = active_structure = item;
   highlightChlidren(top_structure, *wxBLACK);
   SetItemTextColour(active_structure,*wxBLUE);
//   SetItemFont(active_structure,_llfont_bold);
}

void browsers::TDTbrowser::OnTELLhighlightcell(wxString open_cell) {
   wxTreeItemId item;
   assert(findItem(open_cell, item, GetRootItem()));
   SetItemTextColour(active_structure,*wxBLACK);
//   SetItemFont(active_structure,_llfont_normal);
   active_structure = item;
   SetItemTextColour(active_structure,*wxBLUE);
//   SetItemFont(active_structure,_llfont_bold);
   EnsureVisible(active_structure);
}

void browsers::TDTbrowser::OnTELLaddcell(wxString cellname, wxString parentname, int action) {
   wxTreeItemId item, newparent;
   switch (action) {
      case 0: {//new cell
         wxTreeItemId item = AppendItem(GetRootItem(), cellname);
         SetItemTextColour(item,GetItemTextColour(GetRootItem()));
         break;
      }   
      case 1: {//first reference of existing cell
         assert(findItem(cellname, item, GetRootItem()));
         while (findItem(parentname, newparent, GetRootItem())) 
            copyItem(item,newparent);
         DeleteChildren(item);
         Delete(item);
         break;
      }   
      case 2: {//
         assert(findItem(cellname, item, GetRootItem()));
         while (findItem(parentname, newparent, GetRootItem())) 
            copyItem(item,newparent);
         break;
      }
      default: assert(false);
   }
}

void browsers::TDTbrowser::OnTELLremovecell(wxString cellname, wxString parentname, bool orphan)
{
   wxTreeItemId newparent;
   if (orphan)
   {
      wxTreeItemId item;
      findItem(cellname, item, GetRootItem());
      copyItem(item,GetRootItem());
      item = wxTreeItemId();
      assert(findItem(parentname, newparent, GetRootItem()));
      assert(findItem(cellname, item, newparent));
      DeleteChildren(item);
      Delete(item);
   }
   else if (wxT("") == parentname)
   {// no parent => we are removing the cell, not it's reference
      wxTreeItemId item;
      assert(findItem(cellname, item, GetRootItem()));
      // copy all children
      // This part is "in case". The thing is that children should have been
      // removed already, by tdtcell::removePrep
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
   }
   else 
      while (findItem(parentname, newparent, GetRootItem()))
      {
         wxTreeItemId item;
         assert(findItem(cellname, item, newparent));
         DeleteChildren(item);
         Delete(item);
      }
}

void browsers::TDTbrowser::OnReportUsedLayers(wxCommandEvent& WXUNUSED(event)) {
   wxString cmd;
   cmd << wxT("report_layers(\"") << selectedCellname() << wxT("\" , true);");
   Console->parseCommand(cmd);
}


void browsers::TDTbrowser::highlightChlidren(wxTreeItemId parent, wxColour clr) {
   wxTreeItemIdValue cookie;
   SetItemTextColour(parent,clr);
   wxTreeItemId child = GetFirstChild(parent,cookie);
   while (child.IsOk()) {
      highlightChlidren(child,clr);
      child = GetNextChild(parent,cookie);
   }   
}

bool browsers::TDTbrowser::findItem(const wxString name, wxTreeItemId& item, const wxTreeItemId parent) {
   wxTreeItemIdValue cookie;
   wxTreeItemId child = GetFirstChild(parent,cookie);
   while (child.IsOk()) {
      if (item.IsOk()) {
         if (child == item) item = wxTreeItemId();
      }
      else if (name == GetItemText(child)) {
         item = child; return true;
      }   
      if (findItem(name, item, child)) return true;
      child = GetNextChild(parent,cookie);
   }
   return false;
}   

void browsers::TDTbrowser::copyItem(const wxTreeItemId item, const wxTreeItemId newparent) {
   wxTreeItemId newitem = AppendItem(newparent, GetItemText(item));
   SetItemImage(newitem, GetItemImage(item,wxTreeItemIcon_Normal), wxTreeItemIcon_Normal);
   SetItemImage(newitem, GetItemImage(item,wxTreeItemIcon_Expanded), wxTreeItemIcon_Expanded);
   SetItemImage(newparent,0,wxTreeItemIcon_Normal);
   SetItemImage(newparent,1,wxTreeItemIcon_Expanded);
   SetItemTextColour(newitem,GetItemTextColour(newparent));
   wxTreeItemIdValue cookie;
   wxTreeItemId child = GetFirstChild(item,cookie);
   while (child.IsOk()) {
      copyItem(child, newitem);
      child = GetNextChild(item,cookie);
   }
}

browsers::TDTbrowser::~TDTbrowser()
{
   delete _imageList;
   DeleteAllItems();
}
//==============================================================================
BEGIN_EVENT_TABLE(browsers::browserTAB, wxNotebook)
   EVT_TECUSTOM_COMMAND(wxEVT_CMD_BROWSER, wxID_ANY, browsers::browserTAB::OnCommand)
END_EVENT_TABLE()
//==============================================================================
browsers::browserTAB::browserTAB(wxWindow *parent, wxWindowID id,const 
   wxPoint& pos, const wxSize& size, long style) : 
                                 wxNotebook(parent, id, pos, size, style) {
   _TDTstruct = new TDTbrowser(this, ID_TPD_CELLTREE);
   AddPage(_TDTstruct, wxT("Cells"));
   _TDTlayers = new layerbrowser(this, ID_TPD_LAYERS);
   AddPage(_TDTlayers, wxT("Layers"));
   _GDSstruct = NULL;
}

browsers::browserTAB::~browserTAB() {
//   It appears that wx is calling automatically the destructors of the
//   child windows, so no need to call them here
//   delete _TDTstruct; _TDTstruct = NULL;
//   delete _TDTlayers; _TDTlayers = NULL;
}

wxString browsers::browserTAB::TDTSelectedGDSName() const {
   if (NULL != _GDSstruct)
      return _GDSstruct->selectedCellname();
   else return wxT("");
}

void browsers::browserTAB::OnCommand(wxCommandEvent& event) 
{
   int command = event.GetInt();
   switch (command) 
   {
      case BT_ADDTDT_TAB:OnTELLaddTDTtab(event.GetString(), 
                            (laydata::TDTHierTree*)event.GetClientData());break;
      case BT_ADDGDS_TAB:OnTELLaddGDStab();break;
      case BT_CLEARGDS_TAB:OnTELLclearGDStab(); break;
   }
}

void browsers::browserTAB::OnTELLaddTDTtab(const wxString libname, laydata::TDTHierTree* tdtH) {
   _TDTstruct->initialize();
   _TDTstruct->collectInfo(libname, tdtH);
}

void browsers::browserTAB::OnTELLaddGDStab() {
   if (!_GDSstruct) {
      _GDSstruct = new GDSbrowser(this, ID_GDS_CELLTREE);
      AddPage(_GDSstruct, wxT("GDS"));
   }
   else _GDSstruct->DeleteAllItems();
   _GDSstruct->collectInfo();
}

void browsers::browserTAB::OnTELLclearGDStab() {
   if (_GDSstruct) {
      _GDSstruct->DeleteAllItems();
      DeletePage(2);
      _GDSstruct = NULL;
   }
}

//==============================================================================
void browsers::layer_status(BROWSER_EVT_TYPE btype, const word layno, const bool status) {
   int* bt = new int(btype);
   wxCommandEvent eventLAYER_STATUS(wxEVT_CMD_BROWSER);
   eventLAYER_STATUS.SetExtraLong(layno);
   eventLAYER_STATUS.SetInt(status);
   eventLAYER_STATUS.SetClientData((void*) bt);
   wxPostEvent(Browsers->TDTlayers(), eventLAYER_STATUS);
}

void browsers::layer_add(const std::string name, const word layno) {
   int* bt = new int(BT_LAYER_ADD);
   wxCommandEvent eventLAYER_ADD(wxEVT_CMD_BROWSER);
   eventLAYER_ADD.SetExtraLong(layno);
   eventLAYER_ADD.SetString(wxString(name.c_str(), wxConvUTF8));
   eventLAYER_ADD.SetClientData((void*) bt);
   wxPostEvent(Browsers->TDTlayers(), eventLAYER_ADD);
}

void browsers::layer_default(const word newlay, const word oldlay) {
   int* bt = new int(BT_LAYER_DEFAULT);
   wxCommandEvent eventLAYER_DEF(wxEVT_CMD_BROWSER);
   eventLAYER_DEF.SetExtraLong(newlay);
   eventLAYER_DEF.SetInt(oldlay);
   eventLAYER_DEF.SetClientData((void*) bt);
   wxPostEvent(Browsers->TDTlayers(), eventLAYER_DEF);
}

void browsers::addTDTtab(std::string libname, laydata::TDTHierTree* tdtH) {
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(BT_ADDTDT_TAB);
   eventADDTAB.SetClientData((void*) tdtH);
   eventADDTAB.SetString(wxString(libname.c_str(), wxConvUTF8));
   wxPostEvent(Browsers, eventADDTAB);
}

void browsers::addGDStab() {
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(BT_ADDGDS_TAB);
   wxPostEvent(Browsers, eventADDTAB);
}

void browsers::clearGDStab() {
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(BT_CLEARGDS_TAB);
   wxPostEvent(Browsers, eventADDTAB);
}

void browsers::celltree_open(const std::string cname) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_OPEN);
   eventCELLTREE.SetString(wxString(cname.c_str(), wxConvUTF8));
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

void browsers::celltree_highlight(const std::string cname) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_HIGHLIGHT);
   eventCELLTREE.SetString(wxString(cname.c_str(), wxConvUTF8));
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

void browsers::treeAddMember(const char* cell, const char* parent, int action) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_ADD);
   eventCELLTREE.SetString(wxString(cell, wxConvUTF8));
   eventCELLTREE.SetExtraLong(action);
   wxString* prnt = new wxString(parent, wxConvUTF8);
   eventCELLTREE.SetClientData((void*) prnt);
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

void browsers::treeRemoveMember(const char* cell, const char* parent, bool orphan) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_REMOVE);
   eventCELLTREE.SetString(wxString(cell, wxConvUTF8));
   eventCELLTREE.SetExtraLong(orphan);
   wxString* prnt = new wxString(parent, wxConvUTF8);
   eventCELLTREE.SetClientData((void*) prnt);
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

//====================================================================
BEGIN_EVENT_TABLE(browsers::layerbrowser, wxPanel)
   EVT_BUTTON(BT_LAYER_NEW, browsers::layerbrowser::OnNewLayer)
   EVT_BUTTON(BT_LAYER_EDIT, browsers::layerbrowser::OnEditLayer)
   EVT_BUTTON(BT_LAYER_DO, browsers::layerbrowser::OnXXXSelected)
   EVT_BUTTON(BT_LAYER_SELECTWILD,browsers::layerbrowser::OnSelectWild)
   EVT_LIST_ITEM_ACTIVATED(ID_TPD_LAYERS, browsers::layerbrowser::OnActiveLayer)
   EVT_LIST_ITEM_RIGHT_CLICK(ID_TPD_LAYERS,browsers::layerbrowser::OnShowHideLayer)
   EVT_TECUSTOM_COMMAND(wxEVT_CMD_BROWSER, wxID_ANY, browsers::layerbrowser::OnCommand)
END_EVENT_TABLE()
//====================================================================
browsers::layerbrowser::layerbrowser(wxWindow* parent, wxWindowID id) :
                                                               wxPanel(parent, id) {
   wxBoxSizer *thesizer = new wxBoxSizer( wxVERTICAL );
   //   
   wxString action[] = { _T("Hide"), _T("View"),  _T("Lock"),  _T("Unlock") };
   wxBoxSizer *sizer1 = new wxBoxSizer( wxHORIZONTAL );
   action_select = new wxChoice(this, BT_LAYER_ACTION, wxDefaultPosition,
                                                       wxDefaultSize, 4, action);
   action_select->SetSelection(0);
   sizer1->Add(action_select, 1, wxEXPAND, 3);
   sizer1->Add(new wxButton( this, BT_LAYER_DO, wxT("Selected") ), 1, wxEXPAND, 3);
   thesizer->Add(sizer1, 0, wxEXPAND | wxALL);
   //
   _layerlist = new topedlay_list(this, ID_TPD_LAYERS);
   thesizer->Add(_layerlist,1, wxEXPAND | wxALL | wxALIGN_TOP ,3);
   //
   wxString actionwild[] = { _T("All"), _T("None")};
   wxBoxSizer *sizer3 = new wxBoxSizer( wxHORIZONTAL );
   action_wild = new wxChoice(this, BT_LAYER_ACTIONWILD, wxDefaultPosition,
                                                         wxDefaultSize, 2, actionwild);
   action_wild->SetSelection(0);
   sizer3->Add(new wxButton( this, BT_LAYER_SELECTWILD, wxT("Select") ), 1, wxEXPAND, 3);
   sizer3->Add(action_wild, 1, wxEXPAND, 3);
   thesizer->Add(sizer3, 0, wxEXPAND | wxALL);
   //
   wxBoxSizer *sizer2 = new wxBoxSizer( wxHORIZONTAL );
   sizer2->Add(new wxButton( this, BT_LAYER_NEW, wxT("New") ), 1, wxEXPAND, 3);
   sizer2->Add(new wxButton( this, BT_LAYER_EDIT, wxT("Edit")), 1, wxEXPAND, 3);
   thesizer->Add(sizer2, 0, wxEXPAND | wxALL);
   //
   SetSizerAndFit(thesizer);
   thesizer->SetSizeHints( this );
   //
}

browsers::layerbrowser::~layerbrowser()
{
   delete _layerlist;
}

void browsers::layerbrowser::OnActiveLayer(wxListEvent& event)
{
   wxListItem info;
   info.SetId(event.GetIndex()); info.SetMask(wxLIST_MASK_TEXT);
   if (_layerlist->GetItem(info) ) {
//      word layno = (word)info.GetData();
      wxString cmd;
      cmd << wxT("usinglayer(") << info.GetText() << wxT(");");
      Console->parseCommand(cmd);
   }
}

void browsers::layerbrowser::OnCommand(wxCommandEvent& event)
{
   int* command = (int*)event.GetClientData();
   switch (*command) {
      case BT_LAYER_DEFAULT:_layerlist->defaultLayer((word)event.GetExtraLong(), (word)event.GetInt());break;
      case    BT_LAYER_HIDE:_layerlist->hideLayer((word)event.GetExtraLong(),event.IsChecked());break;
      case    BT_LAYER_LOCK:_layerlist->lockLayer((word)event.GetExtraLong(),event.IsChecked());break;
      case     BT_LAYER_ADD:_layerlist->addlayer(event.GetString(),(word)event.GetExtraLong());break;
   }
   delete command;
}

void browsers::layerbrowser::OnNewLayer(wxCommandEvent& WXUNUSED(event)) {
}

void  browsers::layerbrowser::OnEditLayer(wxCommandEvent& WXUNUSED(event)) {
}

void  browsers::layerbrowser::OnSelectWild(wxCommandEvent& WXUNUSED(event))
{
   long actionmask = (0 == action_wild->GetSelection()) ?
                              wxLIST_STATE_SELECTED : ~wxLIST_STATE_SELECTED;
   //swipe all items
   wxListItem info;
   long item = -1;
   for ( ;; ) {
      item = _layerlist->GetNextItem(item, wxLIST_NEXT_ALL,
                                     wxLIST_STATE_DONTCARE);
      if ( item == -1 )  break;
      _layerlist->SetItemState(item, actionmask, wxLIST_STATE_SELECTED);
   }
}

void  browsers::layerbrowser::OnXXXSelected(wxCommandEvent& WXUNUSED(event))
{
   wxString tl_command;
   wxString tl_option;
   switch (action_select->GetSelection()) {
      case 0: tl_command = wxT("hidelayer"); tl_option = wxT("true")  ; break;
      case 1: tl_command = wxT("hidelayer"); tl_option = wxT("false") ; break;
      case 2: tl_command = wxT("locklayer"); tl_option = wxT("true")  ; break;
      case 3: tl_command = wxT("locklayer"); tl_option = wxT("false") ; break;
      default: assert(false);
   }
   wxString cmd;
   cmd << tl_command << wxT("({");
   //swipe the selected items
   wxListItem info;
   long item = -1;
   for ( ;; ) {
      item = _layerlist->GetNextItem(item, wxLIST_NEXT_ALL,
                                     wxLIST_STATE_SELECTED);
      if ( item == -1 )  break;
     // this item is selected - 
      info.SetId(item); info.SetMask(wxLIST_MASK_TEXT);
      _layerlist->GetItem(info);
      cmd << wxT(" ") << info.GetText() << wxT(",");
   }
   cmd.RemoveLast(); cmd << wxT("}, ") << tl_option << wxT(");");
   Console->parseCommand(cmd);
}


void browsers::layerbrowser::OnShowHideLayer(wxListEvent& event)
{
/*   wxListItem info;
   info.SetId(event.GetIndex()); info.SetMask(wxLIST_MASK_TEXT);
   if (_layerlist->GetItem(info) )
   {
      bool on_off;
   // for some reason - colours can't be compared properly
   // what is m_refData ?? in wxColour?
   wxColour itemcol = info.GetTextColour();
      wxColour black(*wxBLACK);
      if (itemcol == black) on_off = false;
      else on_off = true;
      wxString cmd;
      cmd << "hidelayer(" << info.GetText() << " , " <<
            (on_off ? "true" : "false") <<");";
      Console->parseCommand(cmd);
   }
*/
}
