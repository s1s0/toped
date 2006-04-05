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
// ------------------------------------------------------------------------ =
//          $URL$
//        Created: Mon Aug 11 2003
//         Author: s_krustev@yahoo.com
//      Copyright: (C) 2003 by Svilen Krustev
//    Description: Tell function definition browser, GDSII hierarchy browser
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
   InsertColumn(0, "  No    ");
   InsertColumn(1, "     Name     ");
   InsertColumn(2, " S ");
   SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
   _imageList = new wxImageList(16, 16, TRUE);
#ifdef __WXMSW__
/*TODO : Under windows - resource loading*/
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
   AddRoot((AGDSDB->Get_libname()).c_str());
   if (NULL == AGDSDB->hierTree()) return; // new, empty design 
   GDSin::GDSHierTree* root = AGDSDB->hierTree()->GetFirstRoot();
   wxTreeItemId nroot;
   while (root){
      nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->Get_StrName()));
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
      nroot = AppendItem(lroot, wxString(Child->GetItem()->Get_StrName()));
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
   ost << "report_gdslayers(\"" << GetItemText(RBcellID) <<"\");";
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
      nroot = AppendItem(GetRootItem(), wxString(root->GetItem()->name().c_str()));
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
      nroot = AppendItem(lroot, wxString(Child->GetItem()->name().c_str()));
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
      ost << "opencell(\"" << GetItemText(id) <<"\");";
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
      menu.Append(tui::TMGDS_EXPORTC, wxT("export " + RBcellname + " to GDS"));
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
   ost << "opencell(\"" << GetItemText(RBcellID) <<"\");";
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
   else if ("" == parentname)
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
   cmd << "report_layers(\"" << selectedCellname() << "\" , true);";
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
   AddPage(_TDTstruct, "Cells");
   _TDTlayers = new layerbrowser(this, ID_TPD_LAYERS);
   AddPage(_TDTlayers, "Layers");
   _layers = new LayerBrowser2(this, ID_TPD_LAYERS);
   AddPage(_layers, "L");
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
      AddPage(_GDSstruct, "GDS");
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

void browsers::layer_add(const std::string name, const word layno, const std::string col, const std::string fill) 
{
   //int* bt = new int(BT_LAYER_ADD);
   
   int bt = BT_LAYER_ADD;
   wxCommandEvent eventLAYER_ADD(wxEVT_CMD_BROWSER);
   eventLAYER_ADD.SetExtraLong(layno);
   eventLAYER_ADD.SetString(name.c_str());
   
   eventLAYER_ADD.SetInt(bt);
   wxPostEvent(Browsers->TDTlayers(), eventLAYER_ADD);

   wxCommandEvent eventLAYER_ADD2(wxEVT_CMD_BROWSER);
   LayerInfo *layer = new LayerInfo(name, layno, col, fill);
   bt = BT_LAYER_ADD;
   eventLAYER_ADD2.SetClientData((void*) layer);
   eventLAYER_ADD2.SetInt(bt);
   
   wxPostEvent(Browsers->layers(), eventLAYER_ADD2);
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
   eventADDTAB.SetString(wxString(libname.c_str()));
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
   eventCELLTREE.SetString(wxString(cname.c_str()));
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

void browsers::celltree_highlight(const std::string cname) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_HIGHLIGHT);
   eventCELLTREE.SetString(wxString(cname.c_str()));
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

void browsers::treeAddMember(const char* cell, const char* parent, int action) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_ADD);
   eventCELLTREE.SetString(wxString(cell));
   eventCELLTREE.SetExtraLong(action);
   wxString* prnt = new wxString(parent);
   eventCELLTREE.SetClientData((void*) prnt);
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}

void browsers::treeRemoveMember(const char* cell, const char* parent, bool orphan) {
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(BT_CELL_REMOVE);
   eventCELLTREE.SetString(wxString(cell));
   eventCELLTREE.SetExtraLong(orphan);
   wxString* prnt = new wxString(parent);
   eventCELLTREE.SetClientData((void*) prnt);
   wxPostEvent(Browsers->TDTstruct(), eventCELLTREE);
}


browsers::LayerInfo::LayerInfo(const std::string &name, const word layno, const std::string &col, const std::string &fill)
{
   _name    = name;
   _layno   = layno;
   _col     = col;
   _fill    = fill;
};

BEGIN_EVENT_TABLE(browsers::LayerButton, wxBitmapButton)
   //EVT_COMMAND_RANGE(12000,  12100, wxEVT_COMMAND_BUTTON_CLICKED, LayerButton::OnClick)
   EVT_LEFT_DOWN(LayerButton::OnClick)
END_EVENT_TABLE()

browsers::LayerButton::LayerButton(wxWindow* parent, wxWindowID id,  const wxPoint& pos , const wxSize& size, long style , const wxValidator& validator , const wxString& name, LayerInfo *layer)
{
   
   _layer = layer;
   _selected = false;

   wxMemoryDC DC;
   wxBrush *brush;
   std::string caption;
   
   picture = new wxBitmap(198, 28, -1);
   selectedPicture = new wxBitmap(198, 28, -1);

   byte *ifill=Properties->drawprop().getFill(layer->layno());
   layprop::tellRGB *col = Properties->drawprop().getColor(layer->layno());
   wxColour color(col->red(), col->green(), col->blue());

   if(ifill!=NULL)
   {
     
      wxBitmap *stipplebrush = new wxBitmap((char  *)ifill, 32, 32, 1);

      wxImage image;
      image = stipplebrush->ConvertToImage();
      int w = image.GetWidth();
      int h = image.GetHeight();

      //Change white color for current one
      for (int i=0; i<w; i++)
         for (int j=0; j<h; j++)
         {
            if((image.GetRed(i,j)==255)&& (image.GetGreen(i,j)==255) && (image.GetBlue(i,j)==255))
            {
               image.SetRGB(i, j, col->red(), col->green(), col->blue());
            }
         }

      delete stipplebrush;

      //Recreate bitmap with new color
      stipplebrush = new wxBitmap(image, 1);

      brush = new wxBrush(	*stipplebrush);
      
   }
   else
   {
      brush = new wxBrush(*wxBLACK_BRUSH);
   }

   DC.SetBrush(*brush);
       
   wxPen *pen = new wxPen();
           
            
   if (col!=NULL)
   {
      pen->SetColour(color);
   }
            
   //***Draw main picture***         
   DC.SelectObject(*picture);
   DC.SetBackground(*wxBLACK_BRUSH);
   DC.SetPen(*pen);
   DC.SetTextForeground(*wxWHITE);
   
   DC.Clear();
   DC.DrawRectangle(1, 1, 49, 49);
   caption = layer->name()+ "   sv_";
   DC.DrawText(caption.c_str(), 60, 0);
   DC.SelectObject(wxNullBitmap);


   //***Draw picture for selected mode***         
   DC.SelectObject(*selectedPicture);
   DC.SetBackground(*wxWHITE_BRUSH);
   DC.SetPen(*pen);
   DC.SetTextForeground(*wxBLACK);
   
   DC.Clear();
   DC.DrawRectangle(1, 1, 49, 49);
   caption = layer->name()+ "   sv_";
   DC.DrawText(layer->name().c_str(), 60, 0);
   DC.SelectObject(wxNullBitmap);


   Create(parent, id, *picture, pos, size, style, validator, name);
}

browsers::LayerButton::~LayerButton()
{
   delete picture;
   delete selectedPicture;
   delete _layer;
}

void browsers::LayerButton::OnClick(wxMouseEvent &event)
{

   wxString cmd;
   cmd << "usinglayer("<<_layer->layno()<<");";
   Console->parseCommand(cmd);

   if (!_selected)
   {
      select();
   
      int bt = BT_LAYER_SELECT;
      wxCommandEvent eventLAYER_SELECT(wxEVT_CMD_BROWSER);
   
      eventLAYER_SELECT.SetExtraLong(_layer->layno());
   
      eventLAYER_SELECT.SetInt(bt);
      wxPostEvent(Browsers->layers(), eventLAYER_SELECT);
      
   }
   
}

void browsers::LayerButton::select(void)
{
   _selected = true;
   SetBitmapLabel(*selectedPicture);
}

void browsers::LayerButton::unselect(void)
{
   _selected = false;
   SetBitmapLabel(*picture);
}

//====================================================================
BEGIN_EVENT_TABLE(browsers::LayerBrowser2, wxPanel)
   //EVT_BUTTON(BT_LAYER_NEW, browsers::layerBrowser2::OnNewLayer)
   //EVT_BUTTON(BT_LAYER_EDIT, browsers::layerbrowser2::OnEditLayer)
   //EVT_BUTTON(BT_LAYER_DO, browsers::layerbrowser2::OnXXXSelected)
   //EVT_BUTTON(BT_LAYER_SELECTWILD,browsers::layerbrowser2::OnSelectWild)
   //EVT_LIST_ITEM_ACTIVATED(ID_TPD_LAYERS, browsers::layerbrowser2::OnActiveLayer)
   //EVT_LIST_ITEM_RIGHT_CLICK(ID_TPD_LAYERS,browsers::layerbrowser2::OnShowHideLayer)
   EVT_TECUSTOM_COMMAND(wxEVT_CMD_BROWSER, wxID_ANY, browsers::LayerBrowser2::OnCommand)
END_EVENT_TABLE()
//====================================================================


browsers::LayerBrowser2::LayerBrowser2(wxWindow* parent, wxWindowID id) 
   :wxPanel(parent, id) 
{
   _buttonCount = 0;
}

browsers::LayerBrowser2::~LayerBrowser2() 
{
}

void browsers::LayerBrowser2::OnCommand(wxCommandEvent& event)
{
   int command = event.GetInt();
   
   switch (command) 
   {

     // case BT_LAYER_DEFAULT:_layerlist->defaultLayer((word)event.GetExtraLong(), (word)event.GetInt());break;
     // case    BT_LAYER_HIDE:_layerlist->hideLayer((word)event.GetExtraLong(),event.IsChecked());break;
     // case    BT_LAYER_LOCK:_layerlist->lockLayer((word)event.GetExtraLong(),event.IsChecked());break;
      case     BT_LAYER_SELECT:
         {
            word layno = event.GetExtraLong();
            _selectedButton->unselect();
            _selectedButton = _buttonMap[layno];
         
            break;
         }

      case     BT_LAYER_ADD:
         {
            LayerInfo* layer = (LayerInfo*)event.GetClientData();

            LayerButton *layerButton;

            layerButtonMap::iterator it;
            it = _buttonMap.find(layer->layno());
            if (it!= _buttonMap.end())
            {
               LayerButton *tempButton = it->second;
               
               //layerButton = new LayerButton(this, tui::TMDUMMY_LAYER+_buttonCount, wxPoint (0, _buttonCount*30), wxSize(200, 30),
               //wxBU_AUTODRAW, wxDefaultValidator, _T("TTT"), layer);
               int x, y;
               int szx, szy;
               int ID;
               tempButton->GetPosition(&x, &y);
               tempButton->GetSize(&szx, &szy);
               ID = tempButton->GetId();
               layerButton = new LayerButton(this, ID, wxPoint (x, y), wxSize(szx, szy),
               wxBU_AUTODRAW, wxDefaultValidator, _T("TTT"), layer);
               _buttonMap[layer->layno()] = layerButton;
               delete tempButton;

            }
            else
            {
               int szx, szy;
               GetSize(&szx, &szy);
               layerButton = new LayerButton(this, tui::TMDUMMY_LAYER+_buttonCount, wxPoint (0, _buttonCount*30), wxSize(szx, 30),
               wxBU_AUTODRAW, wxDefaultValidator, _T("TTT"), layer);
               _buttonMap[layer->layno()] = layerButton;
               _buttonCount++;              
            }
            
            _selectedButton = (_buttonMap.begin())->second;
            _selectedButton->select();
            break;
         }
   }
   //delete layer;
}

void browsers::LayerBrowser2::OnNewLayer(wxCommandEvent& WXUNUSED(event)) 
{

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
   sizer1->Add(new wxButton( this, BT_LAYER_DO, "Selected" ), 1, wxEXPAND, 3);
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
   sizer3->Add(new wxButton( this, BT_LAYER_SELECTWILD, "Select" ), 1, wxEXPAND, 3);
   sizer3->Add(action_wild, 1, wxEXPAND, 3);
   thesizer->Add(sizer3, 0, wxEXPAND | wxALL);
   //
   wxBoxSizer *sizer2 = new wxBoxSizer( wxHORIZONTAL );
   sizer2->Add(new wxButton( this, BT_LAYER_NEW, "New" ), 1, wxEXPAND, 3);
   sizer2->Add(new wxButton( this, BT_LAYER_EDIT, "Edit"), 1, wxEXPAND, 3);
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
      cmd << "usinglayer(" << info.GetText() << ");";
      Console->parseCommand(cmd);
   }
}

void browsers::layerbrowser::OnCommand(wxCommandEvent& event)
{
   //int* command = (int*)event.GetClientData();
   int command = event.GetInt();
   switch (command) {
      case BT_LAYER_DEFAULT:_layerlist->defaultLayer((word)event.GetExtraLong(), (word)event.GetInt());break;
      case    BT_LAYER_HIDE:_layerlist->hideLayer((word)event.GetExtraLong(),event.IsChecked());break;
      case    BT_LAYER_LOCK:_layerlist->lockLayer((word)event.GetExtraLong(),event.IsChecked());break;
      case     BT_LAYER_ADD:_layerlist->addlayer(event.GetString(),(word)event.GetExtraLong());break;
   }
   //delete command;
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
   std::string tl_command;
   std::string tl_option;
   switch (action_select->GetSelection()) {
       case 0: tl_command = "hidelayer"; tl_option = "true"  ; break;
       case 1: tl_command = "hidelayer"; tl_option = "false" ; break;
       case 2: tl_command = "locklayer"; tl_option = "true"  ; break;
       case 3: tl_command = "locklayer"; tl_option = "false" ; break;
      default: assert(false);
   }
   wxString cmd;
   cmd << tl_command.c_str() << "({";
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
      cmd << " " << info.GetText() << ",";
   }
   cmd.RemoveLast(); cmd << "}, " << tl_option.c_str() << ");";
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
