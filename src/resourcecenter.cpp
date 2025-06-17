
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
//        Created: Sat, 14 Jan 17:32:27 2006
//     Originator: Sergei Gaitukevich - gaitukevich@users.berlios.de
//    Description: Toped user configurable resources
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <functional>
#include "resourcecenter.h"

#include "ted_prompt.h"
#include "ttt.h"
#include "tuidefs.h"
#include "toped.h"

extern tui::TopedFrame*          Toped;
extern console::toped_logfile    LogFile;

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, std::string function) :
   _ID            ( ID        ),
   _menuItem      ( menuItem  ),
   _hotKey        ( hotKey    ),
   _function      ( function  ),
   _inserted      ( false     ),
   _changed       ( false     ),
   _method        ( NULL      )
{}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString):
   _ID            ( ID        ),
   _menuItem      ( menuItem  ),
   _hotKey        ( hotKey    ),
   _function      ( ""        ),
   _helpString    ( helpString),
   _inserted      ( false     ),
   _changed       ( false     ),
   _method        ( cbMethod  )
{}

//Build intermediate menu item (from top menu)
//Returns place where we need insert our menu item
wxMenu* tui::MenuItemHandler::buildPath(wxMenuBar *menuBar, const std::vector<std::string> &menuNames)
{
   wxMenu* menu, *menu2;
   wxMenuItem *menuItem;
   int menuID;


   //First item - Top Menu
   std::string menuString =menuNames[0];
   menuID = menuBar->FindMenu(wxString(menuString.c_str(), wxConvUTF8));
   if (wxNOT_FOUND == menuID)
      {
         // create at the left of Help menu
         menu = DEBUG_NEW wxMenu();
#ifdef __WXOSX_COCOA__
   menuBar->Append(menu, wxString(menuString.c_str(),
   wxConvUTF8));
#else
   menuBar->Insert(menuBar->GetMenuCount()-1, menu, wxString(menuString.c_str(), wxConvUTF8));
#endif

         menuID = menuBar->FindMenu(wxString(menuString.c_str(), wxConvUTF8));
      }

      menu = menuBar->GetMenu(menuID);
      //intermediate menu items - create if does not exists
      for (unsigned int j=1; j<(menuNames.size()-1); j++)
      {
         menuID=menu->FindItem(wxString(menuNames[j].c_str(), wxConvUTF8));
         menuItem = menu->FindItem(menuID, &menu2);

         if ((wxNOT_FOUND == menuID) || (menu2 == NULL))
         {
            menu2 = DEBUG_NEW wxMenu();
            menu->Append(_ID+10000 , wxString(menuNames[j].c_str(), wxConvUTF8), menu2);
            menu = menu2;
         }
         else
         {
            menu = menuItem->GetSubMenu();
         }
      }
      return menu;
}




void tui::MenuItemHandler::create(wxMenuBar *menuBar)
{
   wxMenu* menu;
   wxMenuItem *menuItem;
   std::vector<std::string> menuNames;

   menuNames = split(_menuItem, '/');



   if (menuNames.size()==0) return;

   if (menuNames.size()==1)
   {
      //Create DEBUG_NEW item in main menu
      menu = DEBUG_NEW wxMenu();
      menuBar->Insert(menuBar->GetMenuCount()-1, menu, wxString(menuNames[0].c_str(), wxConvUTF8));
      _inserted = true;
      return;
   }

   menu = buildPath(menuBar, menuNames);

   //last item
   std::string insertedString = *(menuNames.end()-1);
   if (_hotKey !="") insertedString=insertedString+"\t"+_hotKey;

   menuItem = DEBUG_NEW wxMenuItem(menu, _ID, wxString(insertedString.c_str(), wxConvUTF8), wxString(_helpString.c_str(), wxConvUTF8));

   menu->Append(menuItem);

   _inserted = true;
}

void tui::MenuItemHandler::recreate(wxMenuBar *menuBar)
{

   wxMenu* menu;
   wxMenuItem *menuItem;
   std::vector<std::string> menuNames;

   menuNames = split(_menuItem, '/');



   if (menuNames.size()==0) return;

   if (menuNames.size()==1)
   {
      //Create new item in main menu
      menu = DEBUG_NEW wxMenu();
      menuBar->Insert(menuBar->GetMenuCount()-1, menu, wxString(menuNames[0].c_str(), wxConvUTF8));
      _inserted = true;
      return;
   }

   menu = buildPath(menuBar, menuNames);

   //last item
   std::string insertedString = *(menuNames.end()-1);
   if (_hotKey !="") insertedString=insertedString+"\t"+_hotKey;

   int index = menu->FindItem(wxString(insertedString.c_str(), wxConvUTF8));

   menuItem = menu->FindItem(index);
   if ((index==wxNOT_FOUND)||(menuItem==NULL))
   {
      std::ostringstream ost;
      ost<<"Error redefining menu";
      tell_log(console::MT_ERROR,ost.str());
      return;
   }
#if wxCHECK_VERSION(2,9,0)
   menuItem->SetItemLabel(wxString(insertedString.c_str(), wxConvUTF8));
#else
   menuItem->SetText(wxString(insertedString.c_str(), wxConvUTF8));
#endif
   menuItem->SetHelp(wxString(_helpString.c_str(), wxConvUTF8));
   //menuItem = DEBUG_NEW wxMenuItem(menu, _ID, insertedString.c_str(), _helpString.c_str());

   //menu->Append(menuItem);

   _inserted = true;
   _changed = false;
};

void  tui::MenuItemHandler::changeInterior(std::string hotKey, std::string function, callbackMethod cbMethod, std::string helpString)
{
   _hotKey = hotKey;
   if (function!="")
   {
      _function = function;
      _method = NULL;
   }
   else
      if (cbMethod!= NULL)
         {
            _method = cbMethod;
         }
   _helpString = helpString;
   _changed = true;
}

tui::MenuItemSeparator::MenuItemSeparator(std::string menuItem)
   :MenuItemHandler(0, menuItem, "", NULL)
{
}

void tui::MenuItemSeparator::create(wxMenuBar *menuBar)
{
   wxMenu* menu;
   std::vector<std::string> menuNames;// = split
   std::string menustring;

   menustring = _menuItem;
   menuNames = split(menustring, '/');

   if (menuNames.size()==0) return;

   menu = buildPath(menuBar, menuNames);

   menu->AppendSeparator();

   _inserted = true;
}

//=============================================================================
tui::TpdResCallBack::TpdResCallBack(callbackMethod cbMethod) :
   TpdExecResource (        ),
   _cbMethod       (cbMethod)
{
   assert (NULL != cbMethod);
}
void tui::TpdResCallBack::exec()
{
   wxCommandEvent cmd_event(0);
  (Toped->*_cbMethod)(cmd_event);
}

//=============================================================================
tui::TpdResTellScript::TpdResTellScript(const wxString& tellScript) :
   TpdExecResource (          ),
   _tellScript     (tellScript)
{}

void tui::TpdResTellScript::exec()
{
   TpdPost::parseCommand(_tellScript);
}

//=============================================================================
tui::TpdResConfig::TpdResConfig(const TpdToolBar* tbRef) :
      _tbRef         ( tbRef     )
{}

void tui::TpdResConfig::exec()
{
   wxSize cSize = _tbRef->getIconSize();
   ToolBarSizeDlg dialog(_tbRef->GetParent(), wxID_ANY, wxT("ToolBar ") + _tbRef->GetName(), cSize.GetHeight());
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString tllcommand(wxT("toolbarsize(\""));
      if (dialog.setAll())
         tllcommand += wxT("*\",");
      else
         tllcommand += _tbRef->GetName() + wxT("\",");
      switch (dialog.getNewSize())
      {
          case 1: tllcommand += wxT("_iconsize24);");break;
          case 2: tllcommand += wxT("_iconsize32);");break;
          case 3: tllcommand += wxT("_iconsize48);");break;
         default: tllcommand += wxT("_iconsize16);");break;
      }
      TpdPost::parseCommand(tllcommand);
   }
}

//=============================================================================
tui::TpdResTbDrop::TpdResTbDrop(const CbCommandMap& cbCommands) :
   TpdExecResource (          ),
   _cbMethod       ( cbCommands.begin()->second.second)
{
   for (CbCommandMap::const_iterator CCI = cbCommands.begin(); CCI != cbCommands.end(); CCI++)
   {
      wxMenuItem* mItem =  new wxMenuItem(&_tbItemMenu, CCI->first, CCI->second.first);
      mItem->SetKind(wxITEM_RADIO);
      _tbItemMenu.Append(mItem);
   }
}

void tui::TpdResTbDrop::LocalMenu(wxAuiToolBarEvent& cmdEvent)
{
   if (cmdEvent.IsDropDownClicked())
   {
      wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(cmdEvent.GetEventObject());
      tb->SetToolSticky(cmdEvent.GetId(), true);

      // line up our menu with the button
      wxRect rect = tb->GetToolRect(cmdEvent.GetId());
      wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
      pt = tb->ScreenToClient(pt);

      tb->PopupMenu(&_tbItemMenu, pt);


      // make sure the button is "un-stuck"
      tb->SetToolSticky(cmdEvent.GetId(), false);
   }
   else
      cmdEvent.Skip(true);
}

void tui::TpdResTbDrop::setCurrent(callbackMethod cbMethod)
{
   _cbMethod = cbMethod;
}

void tui::TpdResTbDrop::exec()
{
   wxCommandEvent cmd_event(0);
  (Toped->*_cbMethod)(cmd_event);
}

//=============================================================================
tui::TpdDropCallBack::TpdDropCallBack(callbackMethod cbMethod, TpdResTbDrop* dropRef) :
   TpdResCallBack ( cbMethod ),
   _dropRef       ( dropRef  )
{
}

void tui::TpdDropCallBack::exec()
{
   wxCommandEvent cmd_event(0);
  (Toped->*_cbMethod)(cmd_event);
  _dropRef->setCurrent(_cbMethod);
}


//=============================================================================
tui::TpdToolBar::TpdToolBar(int ID, wxWindow* parent, const wxString& tbName, TpdExecResourceMap* execReourceRef, word iconSize) :
   wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW),
   _iconSize       (iconSize,iconSize ),
   _execResourceRef(execReourceRef    )
{
   SetToolBitmapSize(_iconSize);
   SetName(tbName);

   wxAuiToolBarItemArray prepend_items;
   wxAuiToolBarItemArray append_items;
   wxAuiToolBarItem item;
   item.SetKind(wxITEM_SEPARATOR);
   append_items.Add(item);
   item.SetKind(wxITEM_NORMAL);
   item.SetId(ID);
   item.SetLabel(wxT("Size..."));
   append_items.Add(item);

   (*_execResourceRef)[ID] = DEBUG_NEW TpdResConfig(this);
   SetCustomOverflowItems(prepend_items, append_items);

}

wxAuiToolBarItem* tui::TpdToolBar::addToolItem(int ID, const wxArtID& artID, const wxString& label, callbackMethod cbMethod)
{
   (*_execResourceRef)[ID] = DEBUG_NEW TpdResCallBack(cbMethod);
   _itemIdList.push_back(ID);
   return AddTool(ID, artID , wxArtProvider::GetBitmap(artID , wxART_TOOLBAR, _iconSize), label);
}

void tui::TpdToolBar::addToolItem(int ID, const wxArtID& artID, const CbCommandMap& cbCommands)
{
   TpdResTbDrop* ddtb = DEBUG_NEW TpdResTbDrop(cbCommands);
   Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &TpdResTbDrop::LocalMenu, ddtb, ID);
   _itemIdList.push_back(ID);
   (*_execResourceRef)[ID] = ddtb;
   for (CbCommandMap::const_iterator CCI = cbCommands.begin(); CCI != cbCommands.end(); CCI++)
   {
      (*_execResourceRef)[CCI->first] = DEBUG_NEW TpdDropCallBack(CCI->second.second, ddtb);
   }
   AddTool(ID, artID , wxArtProvider::GetBitmap(artID , wxART_TOOLBAR, _iconSize), cbCommands.begin()->second.first);
   SetToolDropDown(ID, true);
}

wxAuiToolBarItem* tui::TpdToolBar::addToolItem(int ID, const wxString& label, const wxString& tllCommand)
{
   (*_execResourceRef)[ID] = DEBUG_NEW TpdResTellScript(tllCommand);
   _itemIdList.push_back(ID);
   wxArtID artID(label);
   artID.UpperCase();
   artID = wxT("tpdART_") + artID;
   return AddTool(ID, artID , wxArtProvider::GetBitmap(artID , wxART_TOOLBAR, _iconSize), label);
}

bool tui::TpdToolBar::deleteToolItem(const wxString& label)
{
   for (std::list<int>::iterator CTI = _itemIdList.begin(); CTI != _itemIdList.end(); CTI++)
   {
      wxAuiToolBarItem* ctbItem = FindTool(*CTI);
      wxArtID artID(label);
      artID.UpperCase();
      artID = wxT("tpdART_") + artID;
      if (artID == ctbItem->GetLabel())
      {
         _execResourceRef->erase(*CTI);
         _itemIdList.erase(CTI);
         return DeleteTool(*CTI);
      }
   }
   return false;
}

void tui::TpdToolBar::changeIconSize(const wxSize& iconSize)
{
   _iconSize = iconSize;
   SetToolBitmapSize(_iconSize);
   for (std::list<int>::iterator CTI = _itemIdList.begin(); CTI != _itemIdList.end(); CTI++)
   {
      wxAuiToolBarItem* ctbItem = FindTool(*CTI);
      if (NULL == ctbItem) break;
      wxString label = ctbItem->GetLabel();
      ctbItem->SetBitmap(wxArtProvider::GetBitmap(label , wxART_TOOLBAR, _iconSize));
   }
   Realize();
}
//=============================================================================

tui::ResourceCenter::ResourceCenter(wxWindow* wxParent):
   _wxParent         ( wxParent        ),
   _menuCount        ( 0               ),
   _nextToolId       ( TDUMMY_TOOL     )
{}

tui::ResourceCenter::~ResourceCenter(void)
{
   for(ItemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      delete (*mItem);
   }
   _menus.clear();

   for(TpdExecResourceMap::const_iterator CI = _execResources.begin(); CI != _execResources.end(); CI++)
      delete CI->second;
   _execResources.clear();
   _allToolBars.clear(); //Don't delete resources, they will be deleted by the wxAuiManager
}

void tui::ResourceCenter::buildMenu(wxMenuBar *menuBar)
{
   wxMenu* menu;
   int menuID;
   std::vector<std::string> menuNames;// = split
   std::string menustring;

   //Try to create Help menu
   menuID = menuBar->FindMenu(wxT("&Help"));
   if (wxNOT_FOUND == menuID)
   {
      // create Help menu on most right
      menu = DEBUG_NEW wxMenu();
      menuBar->Insert(menuBar->GetMenuCount(), menu, wxT("&Help"));
   }

   //looks all saved menus
   for(ItemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      //only for unshown menus
      if ((*mItem)->inserted()== false)
      {
         (*mItem)->create(menuBar);
      }

      //only for changed menus
      if ((*mItem)->changed()== true)
      {
         (*mItem)->recreate(menuBar);
      }

   }
}

bool tui::ResourceCenter::checkExistence(const tui::MenuItemHandler &item)
{
   bool ret=false;
   std::ostringstream ost;

   std::string curStr, itemStr;
   itemStr = item.menuItem();

   //convert menuItem into lowercase and remove char '&'
   itemStr = simplify(itemStr, '&');

   std::string itemHotKey = simplify(item.hotKey(), '-');
   itemHotKey = simplify(itemHotKey, '+');

   for(ItemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      curStr = (*mItem)->menuItem();

      //convert menuItem into lowercase and remove char '&'
      curStr = simplify(curStr, '&');

      if (itemStr.compare(curStr)==0)
      {
         ost<<"Menu item redefining\n";
         tell_log(console::MT_WARNING,ost.str());

         (*mItem)->changeInterior(item.hotKey(), item.function(), item.method(), item.helpString());

         ret = true;
         continue;
      }

      if (item.hotKey()!="")
      {

         std::string mItemHotKey = simplify((*mItem)->hotKey(), '-');
         mItemHotKey = simplify(mItemHotKey, '+');

         if (mItemHotKey.compare(itemHotKey)==0)
         {
            ost<<"Hot key redefined for menu item "+(*mItem)->menuItem();
            tell_log(console::MT_WARNING,ost.str());
            (*mItem)->changeInterior("");//, "", item.method, item.helpString);
            (*mItem)->setChanged(true);
         }
      }
   }
   return ret;
}

//produce lowercase string and exclude unwanted character
std::string tui::ResourceCenter::simplify(std::string str, char ch)
{
   std::transform(str.begin(), str.end(), str.begin(), tolower);
   std::string::iterator curIter;
//   curIter = std::find_if(str.begin(), str.end(), std::bind2nd(std::equal_to<char>(), ch));
   curIter = std::find_if(str.begin(), str.end(), std::bind(std::equal_to<char>(), std::placeholders::_1, ch));
   if (curIter != str.end())
   {
      str.erase(curIter);
   }

   return str;
}

void tui::ResourceCenter::appendMenu(const std::string &menuItem, const std::string &hotKey, const std::string &function)
{
   int ID = TMDUMMY + _menuCount;

   //Set first character in top menu into uppercase
   //it need for simplicity
   std::string str = menuItem;
   str[0] = toupper(str[0]);

   MenuItemHandler* mItem = DEBUG_NEW MenuItemHandler(ID, str, hotKey, function);
   if (!checkExistence(*mItem))
   {
      _menus.push_back(mItem);
      _menuCount++;
   }

}

void tui::ResourceCenter::appendMenu(const std::string &menuItem, const std::string &hotKey, callbackMethod cbMethod, const std::string &helpString)
{
   int ID = TMDUMMY + _menuCount;

   //Set first character in top menu into uppercase
   //it need for simplicity
   std::string str = menuItem;
   str[0] = toupper(str[0]);

   MenuItemHandler* mItem= DEBUG_NEW MenuItemHandler(ID, str, hotKey, cbMethod, helpString);
   if (!checkExistence(*mItem))
   {
      _menus.push_back(mItem);
      _menuCount++;
   }

};

void tui::ResourceCenter::appendMenuSeparator(const std::string &menuItem)
{
   MenuItemHandler* mItem= DEBUG_NEW MenuItemSeparator(menuItem);
   _menus.push_back(mItem);
}

void tui::ResourceCenter::executeMenu(int ID1)
{
   for(ItemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      if (((*mItem)->ID())==ID1)
      {
         //Priority - user defined function
         if (!((*mItem)->function()).empty())
         {
            TpdPost::parseCommand(wxString(((*mItem)->function()).c_str(), wxConvUTF8));
            return;
         }
         else
         {
            if ((*mItem)->method()!=NULL)
            {
               callbackMethod cbMethod;
               wxCommandEvent cmd_event(0);
               cbMethod = (*mItem)->method();
               (Toped->* cbMethod)(cmd_event);
               return;
            }
         }
      }
   }
}

void tui::ResourceCenter::executeToolBar(int ID1)
{
   assert(_execResources.end() != _execResources.find(ID1));
   _execResources[ID1]->exec();
}

void tui::ResourceCenter::setToolBarSize(const wxString& tbName, IconSizes size)
{
   if(size < ICON_SIZE_END)
      for (TpdToolBarList::iterator CT = _allToolBars.begin(); CT != _allToolBars.end(); CT++)
         if ((*CT)->GetName().Matches(tbName))
            (*CT)->changeIconSize(wxSize(IconSizesValues[size],IconSizesValues[size]));
}

wxAuiToolBar* tui::ResourceCenter::initToolBarA(word iconSize)
{
   CbCommandMap cbMap;
   TpdToolBar* tbar = DEBUG_NEW TpdToolBar(_nextToolId++, _wxParent, tpdPane_TB_A, &_execResources, iconSize);

   cbMap[_nextToolId++]= CbCommandPair(wxT("new_design")    ,&tui::TopedFrame::OnNewDesign );
   cbMap[_nextToolId++]= CbCommandPair(wxT("new_cell")      ,&tui::TopedFrame::OnCellNew);
   tbar->addToolItem(_nextToolId++, wxART_NEW      , cbMap);
   cbMap.clear();
   cbMap[_nextToolId++]= CbCommandPair(wxT("open_design")    ,&tui::TopedFrame::OnTDTRead );
   cbMap[_nextToolId++]= CbCommandPair(wxT("open_cell")      ,&tui::TopedFrame::OnCellOpen);
   tbar->addToolItem(_nextToolId++, wxART_FILE_OPEN      , cbMap);
   cbMap.clear();
   cbMap[_nextToolId++]= CbCommandPair(wxT("save")           ,&tui::TopedFrame::OnTDTSave );
   cbMap[_nextToolId++]= CbCommandPair(wxT("save_as")        ,&tui::TopedFrame::OnTDTSaveAs);
   cbMap[_nextToolId++]= CbCommandPair(wxT("save_properties"),&tui::TopedFrame::OnPropSave);
   tbar->addToolItem(_nextToolId++, wxART_FILE_SAVE      , cbMap);
   _allToolBars.push_back(tbar);
   tbar->Realize();
   return tbar;
}

wxAuiToolBar* tui::ResourceCenter::initToolBarB(word iconSize)
{
   CbCommandMap cbMap;
   TpdToolBar* tbar = DEBUG_NEW TpdToolBar(_nextToolId++, _wxParent, tpdPane_TB_B, &_execResources, iconSize);

   tbar->addToolItem(_nextToolId++, tpdART_UNDO        , wxT("undo")     ,&tui::TopedFrame::OnUndo       );
   tbar->AddSeparator();
   tbar->addToolItem(_nextToolId++, tpdART_NEW_BOX     , wxT("box")      ,&tui::TopedFrame::OnDrawBox    );
   tbar->addToolItem(_nextToolId++, tpdART_NEW_POLY    , wxT("poly")     ,&tui::TopedFrame::OnDrawPoly   );
   tbar->addToolItem(_nextToolId++, tpdART_NEW_WIRE    , wxT("wire")     ,&tui::TopedFrame::OnDrawWire   );
   tbar->addToolItem(_nextToolId++, tpdART_NEW_TEXT    , wxT("text")     ,&tui::TopedFrame::OnDrawText   );
   tbar->AddSeparator();
   cbMap[_nextToolId++]= CbCommandPair(wxT("select")        ,&tui::TopedFrame::OnSelectIn );
   cbMap[_nextToolId++]= CbCommandPair(wxT("pselect")       ,&tui::TopedFrame::OnPselectIn);
   cbMap[_nextToolId++]= CbCommandPair(wxT("select_all")    ,&tui::TopedFrame::OnSelectAll);
   tbar->addToolItem(_nextToolId++, tpdART_SELECT      , cbMap);
   cbMap.clear();
   cbMap[_nextToolId++]= CbCommandPair(wxT("unselect")      ,&tui::TopedFrame::OnUnselectIn );
   cbMap[_nextToolId++]= CbCommandPair(wxT("punselect")     ,&tui::TopedFrame::OnPunselectIn);
   cbMap[_nextToolId++]= CbCommandPair(wxT("unselect_all")  ,&tui::TopedFrame::OnUnselectAll);
   tbar->addToolItem(_nextToolId++, tpdART_UNSELECT    , cbMap);
   tbar->addToolItem(_nextToolId++, tpdART_MOVE        , wxT("move")     ,&tui::TopedFrame::OnMove       );
   tbar->addToolItem(_nextToolId++, tpdART_COPY        , wxT("copy")     ,&tui::TopedFrame::OnCopy       );
   tbar->addToolItem(_nextToolId++, tpdART_DELETE      , wxT("delete")   ,&tui::TopedFrame::OnDelete     );
   tbar->AddSeparator();
   cbMap.clear();
   cbMap[_nextToolId++]= CbCommandPair(wxT("rotate cw")      ,&tui::TopedFrame::OnRotate270 );
   cbMap[_nextToolId++]= CbCommandPair(wxT("rotate ccw")     ,&tui::TopedFrame::OnRotate90  );
   tbar->addToolItem(_nextToolId++, tpdART_ROTATE_CW      , cbMap);
   tbar->addToolItem(_nextToolId++, tpdART_FLIP_HORI   , wxT("flip_hori"),&tui::TopedFrame::OnFlipHor    );
   tbar->addToolItem(_nextToolId++, tpdART_FLIP_VERT   , wxT("flip_vert"),&tui::TopedFrame::OnFlipVert   );
   tbar->AddSeparator();
   tbar->addToolItem(_nextToolId++, tpdART_GROUP       , wxT("group")    ,&tui::TopedFrame::OnCellGroup  );
   tbar->addToolItem(_nextToolId++, tpdART_UNGROUP     , wxT("ungroup")  ,&tui::TopedFrame::OnCellUngroup);

//   _resourceCenter->appendTool(wxT("edit"), wxT("cut_box"      ), wxT("cut_box"      ), wxT(""), wxT("cut with box"   ), &tui::TopedFrame::OnBoxCut       );
////   _resourceCenter->appendTool(wxT("edit"), wxT("edit_push"    ), wxT("edit_push"    ), wxT(""), wxT("edit push"      ), &tui::TopedFrame::OnCellPush     );
////   _resourceCenter->appendTool(wxT("edit"), wxT("edit_pop"     ), wxT("edit_pop"     ), wxT(""), wxT("edit pop"       ), &tui::TopedFrame::OnCellPop      );
//
   _allToolBars.push_back(tbar);
   tbar->Realize();
   return tbar;
}

wxAuiToolBar* tui::ResourceCenter::initToolBarC(word iconSize)
{
   TpdToolBar* tbar = DEBUG_NEW TpdToolBar(_nextToolId++, _wxParent, tpdPane_TB_C, &_execResources, iconSize);

   tbar->addToolItem(_nextToolId++, tpdART_ZOOM_IN     , wxT("zoom_in")  ,&tui::TopedFrame::OnZoomIn     );
   tbar->addToolItem(_nextToolId++, tpdART_ZOOM_OUT    , wxT("zoom_out") ,&tui::TopedFrame::OnZoomOut    );
   tbar->addToolItem(_nextToolId++, tpdART_ZOOM_ALL    , wxT("zoom_all") ,&tui::TopedFrame::OnZoomAll    );
   tbar->addToolItem(_nextToolId++, tpdART_RULER       , wxT("add ruler"),&tui::TopedFrame::OnAddRuler   );

   _allToolBars.push_back(tbar);
   tbar->Realize();
   return tbar;
}

wxAuiToolBar* tui::ResourceCenter::appendTools(const wxString& tbName, const WxStringPairList* clientData)
{

   TpdToolBar* tbar = secureToolBar(tbName);
   for (WxStringPairList::const_iterator CDP = clientData->begin();CDP != clientData->end(); CDP++)
   {
      tbar->addToolItem(_nextToolId++, CDP->first, CDP->second);
   }
   tbar->Realize();

   return tbar;
}

bool tui::ResourceCenter::deleteTool(const wxString& tbName, const wxString& item)
{
   TpdToolBarList::const_iterator CTB;
   TpdToolBar* tbar = NULL;
   for (CTB = _allToolBars.begin(); CTB != _allToolBars.end(); CTB++)
   {
      if (tbName == (*CTB)->GetName())
      {
         tbar = *CTB; break;
      }
   }
   if (NULL == tbar)
   {
      wxString msg;
      msg << wxT("Toolbar \"") << tbName << wxT("\" doesn't exist. Can't delete an item.");
      tell_log(console::MT_ERROR, msg);
   }
   else
   {
      if (tbar->deleteToolItem(item))
      {
         tbar->Realize();
         return true;
      }
      else
      {
         wxString msg;
         msg << wxT("Item \"") << item   << wxT("\" not find in toolbar \"")
                               << tbName << wxT("\". Nothing to delete");
         tell_log(console::MT_ERROR, msg);
      }
   }
   return false;
}

tui::TpdToolBar* tui::ResourceCenter::secureToolBar(const wxString &tbName)
{
   // first - try and find an existing toolbar with this name
   TpdToolBarList::const_iterator CTB;
   TpdToolBar* tbar = NULL;
   for (CTB = _allToolBars.begin(); CTB != _allToolBars.end(); CTB++)
   {
      if (tbName == (*CTB)->GetName())
      {
         tbar = *CTB; break;
      }
   }
   if (NULL == tbar)
   {
      tbar = DEBUG_NEW TpdToolBar(_nextToolId++, _wxParent, tbName, &_execResources);
      _allToolBars.push_back(tbar);
   }
   return tbar;

}

//=============================================================================
tui::TpdArtProvider::TpdArtProvider(const wxString& rootDir)
{
   for (IconSizes CI = ICON_SIZE_16x16; CI < ICON_SIZE_END; CI++)
   {
      _rootDir[CI].AssignDir(rootDir);
      wxString sizeDirName;
      sizeDirName.Printf("%ix%i", IconSizesValues[CI], IconSizesValues[CI]);
      _rootDir[CI].AppendDir(sizeDirName);
      _rootDir[CI].Normalize(wxPATH_NORM_ENV_VARS|wxPATH_NORM_DOTS|wxPATH_NORM_TILDE|wxPATH_NORM_ABSOLUTE );
      if (!_rootDir[CI].DirExists())
      {
         std::ostringstream ost;
         ost<<"Directory with "
            << IconSizesValues[CI]
            <<"x"
            << IconSizesValues[CI]
            <<" icon images not found. Expected at "
            << _rootDir[CI].GetFullPath();
         tell_log(console::MT_ERROR,ost.str());
      }
   }
}

wxBitmap tui::TpdArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size)
{
   if (client == wxART_TOOLBAR)
   {
      IconSizes CI;
      for (CI = ICON_SIZE_16x16; CI < ICON_SIZE_END; CI++)
         if (size == wxSize(IconSizesValues[CI],IconSizesValues[CI]))
            break;
      if (CI == ICON_SIZE_END) return wxBitmap(48,48);
      //
      wxString bname(id);
      bname.Remove(0,7).LowerCase();
      wxFileName fName(_rootDir[CI]);
      fName.SetName(bname);
      fName.SetExt(wxT("png"));
      fName.Normalize(wxPATH_NORM_ENV_VARS|wxPATH_NORM_DOTS|wxPATH_NORM_TILDE|wxPATH_NORM_ABSOLUTE );
      if (fName.IsOk() && fName.FileExists())
      {
         wxBitmap tmp(fName.GetFullPath(), wxBITMAP_TYPE_PNG);
         if (tmp.IsOk())
            return tmp;
         else
            return wxBitmap(IconSizesValues[CI],IconSizesValues[CI]);
      }
      else
      {
         std::ostringstream ost;
         ost<<"Can't open image file \""
            << fName.GetFullPath()
            << "\". Generating a random image ...";
         tell_log(console::MT_ERROR,ost.str());
         return wxBitmap(IconSizesValues[CI],IconSizesValues[CI]);
      }
   }
   return wxBitmap(48,48);
}

//=============================================================================
tellstdfunc::stdADDMENU::stdADDMENU(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdADDMENU::execute()
{
   std::string function = getStringValue();
   std::string hotKey   = getStringValue();
   std::string menu     = getStringValue();

   wxMenuBar *menuBar   = Toped->GetMenuBar();
   tui::ResourceCenter *resourceCenter = Toped->resourceCenter();
   resourceCenter->appendMenu(menu, hotKey, function);
   resourceCenter->buildMenu(menuBar);

   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdTOOLBARSIZE::stdTOOLBARSIZE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

int tellstdfunc::stdTOOLBARSIZE::execute()
{
   int         size        = getWordValue();
   std::string toolbarName = getStringValue();
   if (size < tui::ICON_SIZE_END)
   {
      wxCommandEvent eventToolBarSize(console::wxEVT_TOOLBARSIZE);
      eventToolBarSize.SetString(wxString(toolbarName.c_str(), wxConvUTF8));
      eventToolBarSize.SetInt(size);
      wxPostEvent(Toped, eventToolBarSize);
      LogFile << LogFile.getFN() << "(\""<< toolbarName << "\"," << size << ");";LogFile.flush();
   }
   else
   {
      std::ostringstream ost;
      ost<<"toolbarsize: wrong size value";
      tell_log(console::MT_ERROR,ost.str());
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdTOOLBARADDITEM::stdTOOLBARADDITEM(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_hshstr)));
}

int tellstdfunc::stdTOOLBARADDITEM::execute()
{
   telldata::TtList *iconCmdMapList = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();
   std::string toolbarName = getStringValue();

   wxString wxtbName = wxString(toolbarName.c_str(), wxConvUTF8);
   WxStringPairList* clientData = DEBUG_NEW WxStringPairList(); //Note! shall be destroyed in the event receiver
   for (unsigned i = 0; i < iconCmdMapList->size(); i++)
   {
      telldata::TtHshStr* iconCmdMap = static_cast<telldata::TtHshStr*>((iconCmdMapList->mlist())[i]);
      clientData->push_back(WxStringPair(wxString(iconCmdMap->key().value().c_str()  , wxConvUTF8),
                                         wxString(iconCmdMap->value().value().c_str(), wxConvUTF8)));
   }
   LogFile << LogFile.getFN() << "(\""<< toolbarName << "\"," << *iconCmdMapList << ");";LogFile.flush();
   delete iconCmdMapList;

   wxCommandEvent eventToolBarDef(console::wxEVT_TOOLBARADDITEM);
   eventToolBarDef.SetString(wxtbName);
   eventToolBarDef.SetClientData(clientData);
   wxPostEvent(Toped, eventToolBarDef);

   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdTOOLBARADDITEM_S::stdTOOLBARADDITEM_S(telldata::typeID retype, bool eor) :
      stdTOOLBARADDITEM(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtHshStr()));
}

int tellstdfunc::stdTOOLBARADDITEM_S::execute()
{
   telldata::TtHshStr* iconCmdMap = static_cast<telldata::TtHshStr*>(OPstack.top());OPstack.pop();
   telldata::TtList *iconCmdMapList = DEBUG_NEW telldata::TtList(telldata::tn_hshstr);
   iconCmdMapList->add(iconCmdMap);
   OPstack.push(iconCmdMapList);

   return stdTOOLBARADDITEM::execute();
}

//=============================================================================
tellstdfunc::stdTOOLBARDELETEITEM::stdTOOLBARDELETEITEM(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdTOOLBARDELETEITEM::execute()
{
   std::string   itemName      = getStringValue();
   std::string   toolbarName   = getStringValue();

   //Note! shall be destroyed in the event receiver
   WxStringPair* clientData  = DEBUG_NEW WxStringPair(wxString(toolbarName.c_str(), wxConvUTF8),
                                                      wxString(itemName.c_str()   , wxConvUTF8));

   wxCommandEvent eventToolBarDelItem(console::wxEVT_TOOLBARDELETEITEM);
   eventToolBarDelItem.SetClientData(clientData);
   wxPostEvent(Toped, eventToolBarDelItem);

   LogFile << LogFile.getFN() << "(\""<< toolbarName << "\",\"" << itemName << "\");";LogFile.flush();

   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUIFRAME_LOAD::stdUIFRAME_LOAD(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdUIFRAME_LOAD::execute()
{
   std::string stdAuiMagicString = getStringValue();
   wxString    wxsAuiMagicString(stdAuiMagicString.c_str(), wxConvUTF8);
   TpdPost::restoreAuiState(wxsAuiMagicString);
   return EXEC_NEXT;
}

