
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

extern const wxEventType         wxEVT_TOOLBARSIZE;
extern const wxEventType         wxEVT_TOOLBARDEF;
extern const wxEventType         wxEVT_TOOLBARADDITEM;
extern const wxEventType         wxEVT_TOOLBARDELETEITEM;

tui::MenuItemHandler::MenuItemHandler(void) :
   _ID            ( 0      ),
   _menuItem      ( ""     ),
   _hotKey        ( ""     ),
   _function      ( ""     ),
   _inserted      ( false  ),
   _changed       ( false  ),
   _method        ( NULL   )
{}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, std::string function)
   :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(function), _method(NULL)
{
   _inserted   =  false;
   _changed    =  false;
}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod)
   :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(""), _method(cbMethod)
{
   _inserted   =  false;
   _changed    =  false;
}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString)
   :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(""), _helpString(helpString), _method(cbMethod)
{
      _inserted   =  false;
      _changed    =  false;
}

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



tui::MenuItem::MenuItem(void)
   :MenuItemHandler()
{
}

tui::MenuItemSeparator::MenuItemSeparator(void)
   :MenuItemHandler()
{
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

//tui::ToolItem::ToolItem(int toolID, const wxString& name,
//                        const wxString& bitmapName,
//                        const wxString& hotKey,
//                        const wxString& helpString,
//                        callbackMethod cbMethod) :
//   _ID            ( toolID          ),
//   _name          ( name            ),
//   /*_hotKey(hotKey),*/
//   _function      ( ""              ),
//   _helpString    ( helpString      ),
//   _ok            ( false           ),
//   _method        ( cbMethod        ),
//   _currentSize   ( ICON_SIZE_16x16 )
//{
//   init(bitmapName);
//}
//
//tui::ToolItem::ToolItem(int toolID, const wxString& name,
//                        const wxString& bitmapName,
//                        const wxString& hotKey,
//                        const wxString& helpString,
//                        const wxString func) :
//   _ID               ( toolID          ),
//   _name             ( name            ),
//   /*_hotKey(hotKey),*/
//   _function         ( func            ),
//   _helpString       ( helpString      ),
//   _ok               ( false           ),
//   _method           ( NULL            ),
//   _currentSize      ( ICON_SIZE_16x16 )
//{
//   init(bitmapName);
//}
//
//void tui::ToolItem::init(const wxString& bitmapName)
//{
//   wxImage image[ICON_SIZE_END];
//   wxImage tempImage;
//   wxString tempImageName;
//   bool fileMissing = false;
//   for(IconSizes isz = ICON_SIZE_16x16;  isz < ICON_SIZE_END;  isz=static_cast<IconSizes>(static_cast<int>(isz)+1))
//   {
//      wxString sizeDirName;
//      sizeDirName.Printf("%ix%i", IconSizesValues[isz], IconSizesValues[isz]);
//      wxFileName fName(bitmapName);
//      fName.AppendDir(sizeDirName);
//      fName.SetExt(wxT("png"));
//      (image[isz]).LoadFile(fName.GetFullPath(),wxBITMAP_TYPE_PNG);
//      _bitmapNames[isz] = fName.GetFullName().mb_str(wxConvFile);
//      if(image[isz].IsOk())
//      {
//         _bitmaps[isz] = wxBitmap(image[isz]);
//         _ok = true;
//         if(!tempImage.IsOk())
//         {
//            tempImage = image[isz]; //Save temp image for filling of missing icons
//            tempImageName = fName.GetFullName();
//         }
//      }
//      else
//      {
//         fileMissing = true;
//      }
//   }
//
//
//   if(fileMissing)
//   {
//      if (tempImage.IsOk())
//      {
//         for(IconSizes isz = ICON_SIZE_16x16; isz < ICON_SIZE_END; isz=static_cast<IconSizes>(static_cast<int>(isz)+1))
//         {
//            if(!image[isz].IsOk())
//            {
//               //image[isz] = tempImage.Copy();
//               image[isz].LoadFile(tempImageName, wxBITMAP_TYPE_PNG);
//               _bitmapNames[isz] = tempImageName;
//               _bitmaps[isz] = wxBitmap(image[isz]);
//
//               std::ostringstream ost;
//               ost<<"No icon file  "<<_bitmapNames[isz];
//               tell_log(console::MT_WARNING,ost.str());
//               ost.clear();
//               ost<<"Replaced";
//               tell_log(console::MT_WARNING,ost.str());
//            }
//         }
//      }
//      else
//      {
//         std::ostringstream ost;
//         ost<<"No any proper file for "<<bitmapName;
//         tell_log(console::MT_ERROR,ost.str());
//      }
//   }
//}
//
//
//void tui::ToolItem::changeToolSize(IconSizes size)
//{
//   if(checkToolSize(size))
//   {
//      _currentSize = size;
//   }
//   else
//   {
//      wxString info;
//      info << wxT("Wrong size for icon was chosen.");
//      tell_log(console::MT_ERROR,info);
//   }
//}
//
////=============================================================================
//tui::TpdToolBar::TpdToolBar(int ID, long style, IconSizes iconSize) :
//   wxToolBar(Toped->getFrame(), ID, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER|wxTB_FLAT )
//{
//   SetWindowStyle(style|GetWindowStyle());
//   int sz = IconSizesValues[iconSize];
//   SetToolBitmapSize(wxSize(sz, sz));
//}
//
//tui::ToolBarHandler::ToolBarHandler(int ID, const wxString& name, int direction) :
//   _ID            ( ID                ),
//   _name          ( name              ),
//   _dockDirection ( direction         ),
//   _floating      ( false             ),
//   _coord         ( wxDefaultPosition ),
//   _currentSize   ( ICON_SIZE_16x16   )
//{
////   wxAuiPaneInfo paneInfo;
//
//   if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
//   {
//      _toolBar = DEBUG_NEW TpdToolBar(ID, wxTB_VERTICAL, ICON_SIZE_16x16);
////      paneInfo = Toped->getAuiManager()->GetPane(_toolBar);
////      paneInfo.TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true);
//   }
//   else
//   {
//      _toolBar = DEBUG_NEW TpdToolBar(ID, wxTB_HORIZONTAL, ICON_SIZE_16x16);
////      paneInfo = Toped->getAuiManager()->GetPane(_toolBar);
////      paneInfo.TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
//   }
//   _toolBar->Hide();
//   /*_toolBar->Realize();
//   Toped->getAuiManager()->AddPane(_toolBar, paneInfo.ToolbarPane().
//      Name(wxString(_name.c_str(), wxConvUTF8)).Direction(_dockDirection).Floatable(false));
//   Toped->getAuiManager()->Update();*/
//}
//
//tui::ToolBarHandler::~ToolBarHandler()
//{
//   for(ToolList::iterator it=_tools.begin();it!=_tools.end();it++)
//   {
//      delete (*it);
//   }
//   _tools.clear();
//}
//
//void   tui::ToolBarHandler::changeToolSize(IconSizes size)
//{
//   _currentSize = size;
//   wxAuiPaneInfo paneInfo = Toped->getAuiManager()->GetPane(_toolBar);
//   _floating = paneInfo.IsFloating();
//   if(paneInfo.IsDocked())
//   {
//      _dockDirection = paneInfo.dock_direction;
//   }
//   _coord = paneInfo.floating_pos;
//
//   Toped->getAuiManager()->DetachPane(_toolBar);
//   _toolBar->Destroy();
//
//   Toped->getAuiManager()->Update();
//
//   if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
//   {
//      _toolBar = DEBUG_NEW TpdToolBar(_ID, wxTB_VERTICAL, size);
//   }
//   else
//   {
//      _toolBar = DEBUG_NEW TpdToolBar(_ID, wxTB_HORIZONTAL, size);
//   }
//
//   attachToAUI();
//   for(ToolList::iterator it=_tools.begin();it!=_tools.end();it++)
//   {
//      (*it)->changeToolSize(_currentSize);
//      _toolBar->AddTool((*it)->ID(),wxT(""),(*it)->bitmap(), (*it)->helpString());
//      Toped->getAuiManager()->DetachPane(_toolBar);
//      _toolBar->Realize();
//      attachToAUI();
//   }
//}
//
//
//void tui::ToolBarHandler::addTool(int ID1, const wxString &toolBarItem, const wxString &iconName,
//                                    const wxString &iconFileName,
//                                    const wxString hotKey,
//                                    const wxString &helpString,
//                                    callbackMethod cbMethod)
//{
//   clearTool(iconName);
//   ToolItem *tool = DEBUG_NEW ToolItem(ID1, toolBarItem, iconFileName, hotKey, helpString, cbMethod);
//   if (tool->isOk())
//   {
//      tool->changeToolSize(_currentSize);
//      _tools.push_back(tool);
//      _toolBar->AddTool(tool->ID(),wxT(""),tool->bitmap(), helpString);
//
//      Toped->getAuiManager()->DetachPane(_toolBar);
//      _toolBar->Realize();
//      attachToAUI();
//   }
//   else
//   {
//      delete tool;
//   }
//}
//
//void tui::ToolBarHandler::addTool(int ID1, const wxString &toolBarItem, const wxString &iconName,
//                                    const wxString &iconFileName,
//                                    const wxString hotKey,
//                                    const wxString &helpString,
//                                    const wxString &func)
//{
//   clearTool(iconName);
//   ToolItem *tool = DEBUG_NEW ToolItem(ID1, toolBarItem, iconFileName, hotKey, helpString, func);
//   if (tool->isOk())
//   {
//      tool->changeToolSize(_currentSize);
//      _tools.push_back(tool);
//      _toolBar->AddTool(tool->ID(),wxT(""),tool->bitmap(), helpString );
//
//      Toped->getAuiManager()->DetachPane(_toolBar);
//      _toolBar->Realize();
//      attachToAUI();
//   }
//   else
//   {
//      delete tool;
//   }
//}
//
//void   tui::ToolBarHandler::deleteTool(const wxString& toolBarItem)
//{
//   ToolList::iterator it;
//   for(it = _tools.begin(); it != _tools.end(); it++)
//   {
//      if ((*it)->name() == toolBarItem)
//      {
//         std::ostringstream ost;
//         ost<<"Tool item " + toolBarItem + " is deleted";
//         tell_log(console::MT_WARNING,ost.str());
//         _toolBar->DeleteTool((*it)->ID());
//         delete (*it);
//         _tools.erase(it);
//         break;
//      }
//   }
//   Toped->getAuiManager()->DetachPane(_toolBar);
//   _toolBar->Realize();
//   attachToAUI();
//}
//
//void   tui::ToolBarHandler::clearTool(const wxString &iconName)
//{
//   ToolList::iterator it;
//   for(it = _tools.begin(); it != _tools.end(); it++)
//   {
//      if ((*it)->name() == iconName)
//      {
//         std::ostringstream ost;
//         ost<<"Tool item "+iconName+" is redefined";
//         tell_log(console::MT_WARNING,ost.str());
//            delete (*it);
//         _tools.erase(it);
//         break;
//      }
//   }
//}
//
//void tui::ToolBarHandler::attachToAUI(void)
//{
//   wxAuiPaneInfo pane;
//   pane=wxAuiPaneInfo().ToolbarPane().Name(_name).Direction(_dockDirection).Gripper().GripperTop(false).Floatable(false).
//         TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
//   if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
//   {
//      pane.GripperTop(true).TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true);
//   }
//   else
//   {
//      pane.GripperTop(false).TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
//   }
//
//   if(_floating)
//   {
//      pane.Float().FloatingPosition(_coord);
//   }
//   Toped->getAuiManager()->AddPane(_toolBar,pane);
//   Toped->getAuiManager()->Update();
//}
//
//
//void   tui::ToolBarHandler::execute(int ID1)
//{
//   for(ToolList::iterator it = _tools.begin();it!=_tools.end(); it++)
//   {
//      if (((*it)->ID())==ID1)
//      {
//         //Priority - user defined function
//         if (!((*it)->function()).empty())
//         {
//            TpdPost::parseCommand((*it)->function());
//            return;
//         }
//         else if ((*it)->method()!=NULL)
//         {
//            callbackMethod cbMethod;
//            wxCommandEvent cmd_event(0);
//            cbMethod = (*it)->method();
//            (Toped->* cbMethod)(cmd_event);
//            return;
//         }
//      }
//   }
//}

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
tui::TpdToolBarWrap::TpdToolBarWrap(wxWindow* parent, const wxSize& iconSize, const wxString& tbName, TpdExecResourceMap* execReourceRef) :
   wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW),
   _iconSize (iconSize),
   _execResourceRef(execReourceRef)
{
   SetToolBitmapSize(_iconSize);
   SetName(tbName);

   wxAuiToolBarItemArray prepend_items;
   wxAuiToolBarItemArray append_items;
   wxAuiToolBarItem item;
//   item.SetKind(wxITEM_SEPARATOR);
//   append_items.Add(item);
   item.SetKind(wxITEM_NORMAL);
   item.SetId(wxID_ANY);
   item.SetLabel(_("Customize..."));
   append_items.Add(item);

   SetCustomOverflowItems(prepend_items, append_items);

}

void tui::TpdToolBarWrap::addToolItem(int ID, const wxString& label, const wxArtID& artID, callbackMethod cbMethod)
{
   (*_execResourceRef)[ID] = DEBUG_NEW TpdResCallBack(cbMethod);
   _itemIdList.push_back(ID);
   AddTool(ID, artID , wxArtProvider::GetBitmap(artID , wxART_TOOLBAR, _iconSize), label);

}

void tui::TpdToolBarWrap::changeIconSize(const wxSize& iconSize)
{
   _iconSize = iconSize;
   SetToolBitmapSize(_iconSize);
   for (std::list<int>::iterator CTI = _itemIdList.begin(); CTI != _itemIdList.end(); CTI++)
   {
      wxAuiToolBarItem* ctbItem = FindTool(*CTI);
      wxString label = ctbItem->GetLabel();
      ctbItem->SetBitmap(wxArtProvider::GetBitmap(label , wxART_TOOLBAR, _iconSize));
   }
   Realize();
   //      wxAuiToolBar* tbar = static_cast<wxAuiToolBar*>(Toped->FindWindow(tpdTB_A));
   //      for(unsigned int CTID = 0; CTID < tbar->GetToolCount();CTID++)
   //      {
   //         wxAuiToolBarItem* ctbItem = tbar->FindTool(TDUMMY_TOOL + CTID);
   //         wxString label = ctbItem->GetLabel();
   //         ctbItem->SetBitmap(wxArtProvider::GetBitmap(label , wxART_TOOLBAR, wxSize(32,32)/*_curTBSize*/));
   //      }
   //      tbar->SetToolBitmapSize(wxSize(32,32));
   //      tbar->Realize();
   //      Toped->getAuiManager()->Update();

}
//=============================================================================
//   private:
//      std::list<int>    _itemIdList;
//      wxSize            _iconSize;

//=============================================================================

tui::ResourceCenter::ResourceCenter(void):
   _menuCount        ( 0               ),
//   _toolCount        ( 0               ),
   _nextToolId       ( TDUMMY_TOOL     ),
//   _direction        ( wxAUI_DOCK_TOP  ),
   _curTBSize        (16,16            )
{}

tui::ResourceCenter::~ResourceCenter(void)
{
   for(ItemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      delete (*mItem);
   }
   _menus.clear();

//   for(ToolBarList::iterator tItem=_toolBars.begin(); tItem!=_toolBars.end(); tItem++)
//   {
//      delete (*tItem);
//   }
//   _toolBars.clear();

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
   curIter = std::find_if(str.begin(), str.end(), std::bind2nd(std::equal_to<char>(), ch));
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

   MenuItemHandler* mItem = DEBUG_NEW MenuItem(ID, str, hotKey, function);
   if (!checkExistence(*mItem))
   {
      _menus.push_back(mItem);
      _menuCount++;
   }

}

void tui::ResourceCenter::appendMenu(const std::string &menuItem, const std::string &hotKey, callbackMethod cbMethod)
{
   int ID = TMDUMMY + _menuCount;

   //Set first character in top menu into uppercase
   //it need for simplicity
   std::string str = menuItem;
   str[0] = toupper(str[0]);


   MenuItemHandler* mItem= DEBUG_NEW MenuItem(ID, str, hotKey, cbMethod);
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

   MenuItemHandler* mItem= DEBUG_NEW MenuItem(ID, str, hotKey, cbMethod, helpString);
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

/*void tui::ResourceCenter::appendTool(const std::string toolBarName, const std::string &toolBarItem,
                                     const std::string &bitmapFileName,
                                     const std::string &hotKey, callbackMethod cbMethod, int direction)
{
   int ID;
   std::string str = toolBarName;

   std::transform(str.begin(), str.end(), str.begin(), tolower);
   toolBarList::const_iterator it;
   for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
   {
      if ((*it)->name()==str)
      {
         break;
      }
   }

   if (it==_toolBars.end())
   {
      ID = TDUMMY_TOOL + _toolCount;
      //Create new toolbar
      ToolBarHandler* toolBar = DEBUG_NEW ToolBarHandler(ID, str, direction);
      _toolCount++;

      ID = TDUMMY_TOOL + _toolCount;
      ToolItem *tool = DEBUG_NEW ToolItem(ID, toolBarItem, bitmapFileName, hotKey, cbMethod);
      _toolCount++;
      toolBar->addTool(tool);
   }

}*/

//void tui::ResourceCenter::setDirection(int direction)
//{
//   assert(  ( direction == wxAUI_DOCK_TOP    )
//          ||( direction == wxAUI_DOCK_LEFT   )
//          ||( direction == wxAUI_DOCK_BOTTOM )
//          ||( direction == wxAUI_DOCK_RIGHT  ));
//   _direction = direction;
//}

void tui::ResourceCenter::setToolBarSize(IconSizes size)
{
   if(checkToolSize(size))
   {
      for (TpdToolBarList::iterator CT = _allToolBars.begin(); CT != _allToolBars.end(); CT++)
      {
         (*CT)->changeIconSize(wxSize(IconSizesValues[size],IconSizesValues[size]));
      }
   }
}

//void tui::ResourceCenter::defineToolBar(const wxString &toolBarName)
//{
//   int ID;
//   //find toolbar
//   wxString str = toolBarName;
//   str.UpperCase();
//   ToolBarList::const_iterator it;
//   for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
//   {
//      if ((*it)->name()==str)
//      {
//         std::ostringstream ost;
//         ost<<"definetoolbar: already defined";
//         tell_log(console::MT_WARNING,ost.str());
//         break;
//      }
//   }
//
//   if (it==_toolBars.end())
//   {
//      ID = TDUMMY_TOOL + _toolCount;
//      //Create new toolbar
//      ToolBarHandler* toolBar = DEBUG_NEW ToolBarHandler(ID, str, _direction);
//      _toolCount++;
//      _toolBars.push_back(toolBar);
//   }
//}
//
//void tui::ResourceCenter::appendTool(const wxString &toolBarName, const wxString &toolBarItem,
//                     const  wxString &iconName, const wxString &hotKey, const wxString &helpString,
//                     callbackMethod cbMethod)
//{
//   //set correct filename for toolBarItem
//   wxString fullIconName = _IconDir+iconName;
//   //increase counter of toolItems
//   int ID = TDUMMY_TOOL + _toolCount++;
//
//   ToolBarHandler* toolBar = secureToolBar(toolBarName);
//   toolBar->addTool(ID, toolBarItem, toolBarItem, fullIconName, hotKey, helpString, cbMethod);
//}
//
//void tui::ResourceCenter::appendTool(const wxString &toolBarName, const wxString &toolBarItem,
//                     const wxString &iconName,
//                     const wxString &hotKey,
//                     const wxString &helpString,
//                     wxString func)
//{
//   //set correct filename for toolBarItem
//   wxString fullIconName = _IconDir+iconName;
//   //increase counter of toolItems
//   int ID = TDUMMY_TOOL + _toolCount++;
//
//   ToolBarHandler* toolBar = secureToolBar(toolBarName);
//   toolBar->addTool(ID, toolBarItem, toolBarItem, fullIconName, hotKey, helpString, func);
//}
//
//void tui::ResourceCenter::deleteTool(const wxString& toolBarName, const wxString& toolBarItem)
//{
//   //find toolbar
//   wxString str = toolBarName;
////   std::transform(str.begin(), str.end(), str.begin(), tolower);
//   str.LowerCase();
//   ToolBarList::const_iterator it;
//   for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
//   {
//      if ((*it)->name()==str)
//      {
//         break;
//      }
//   }
//   //if toolbar doesn't exist, create it
//   if (it==_toolBars.end())
//   {
//      std::ostringstream ost;
//      ost<<"toolbardeleteitem: there is no such toolbar";
//      tell_log(console::MT_WARNING,ost.str());
//      return;
//   }
//   else
//   {
//      (*it)->deleteTool(toolBarItem);
//   }
//}
//
//tui::ToolBarHandler* tui::ResourceCenter::secureToolBar(const wxString &toolBarName)
//{
//   int ID;
//   ToolBarHandler* toolBar = NULL;
//
//   //find toolbar
//   wxString str = toolBarName;
//   str.LowerCase();
//   ToolBarList::const_iterator it;
//   for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
//   {
//      if ((*it)->name()==str)
//      {
//         toolBar = *it;
//         break;
//      }
//   }
//   //if toolbar doesn't exist, create it
//   if (NULL == toolBar)
//   {
//      ID = TDUMMY_TOOL + _toolCount;
//      //Create new toolbar
//      toolBar = DEBUG_NEW ToolBarHandler(ID, str, _direction);
//      _toolCount++;
//      _toolBars.push_back(toolBar);
//   }
//
//   return toolBar;
//}

wxAuiToolBar* tui::ResourceCenter::initToolBarA(wxWindow* parent)
{
   TpdToolBarWrap* tbar = DEBUG_NEW TpdToolBarWrap(parent, _curTBSize, tpdPane_TB_A, &_execResources);
   tbar->addToolItem(_nextToolId++, wxT("new_cell") , wxART_NEW      , &tui::TopedFrame::OnCellNew       );
   tbar->addToolItem(_nextToolId++, wxT("open_cell"), wxART_FILE_OPEN, &tui::TopedFrame::OnCellOpen      );
   tbar->addToolItem(_nextToolId++, wxT("save")     , wxART_FILE_SAVE, &tui::TopedFrame::OnTDTSave       );
   _allToolBars.push_back(tbar);
   return tbar;
}

wxAuiToolBar* tui::ResourceCenter::initToolBarB(wxWindow* parent)
{
   TpdToolBarWrap* tbar = DEBUG_NEW TpdToolBarWrap(parent, _curTBSize, tpdPane_TB_B, &_execResources);

   tbar->addToolItem(_nextToolId++, wxT("undo")     , tpdART_UNDO        ,&tui::TopedFrame::OnUndo       );
   tbar->AddSeparator();
   tbar->addToolItem(_nextToolId++, wxT("box")      , tpdART_NEW_BOX     ,&tui::TopedFrame::OnDrawBox    );
   tbar->addToolItem(_nextToolId++, wxT("poly")     , tpdART_NEW_POLY    ,&tui::TopedFrame::OnDrawPoly   );
   tbar->addToolItem(_nextToolId++, wxT("wire")     , tpdART_NEW_WIRE    ,&tui::TopedFrame::OnDrawWire   );
   tbar->addToolItem(_nextToolId++, wxT("text")     , tpdART_NEW_TEXT    ,&tui::TopedFrame::OnDrawText   );
   tbar->AddSeparator();
   tbar->addToolItem(_nextToolId++, wxT("select")    , tpdART_SELECT     ,&tui::TopedFrame::OnSelectIn   );
   tbar->addToolItem(_nextToolId++, wxT("unselect")  , tpdART_UNSELECT   ,&tui::TopedFrame::OnUnselectIn );
   tbar->addToolItem(_nextToolId++, wxT("move")      , tpdART_MOVE       ,&tui::TopedFrame::OnMove       );
   tbar->addToolItem(_nextToolId++, wxT("copy")      , tpdART_COPY       ,&tui::TopedFrame::OnCopy       );
   tbar->addToolItem(_nextToolId++, wxT("delete")    , tpdART_DELETE     ,&tui::TopedFrame::OnDelete     );
   tbar->AddSeparator();
   tbar->addToolItem(_nextToolId++, wxT("rotate")    , tpdART_ROTATE_CW  ,&tui::TopedFrame::OnRotate     );
   tbar->addToolItem(_nextToolId++, wxT("flip_hori") , tpdART_FLIP_HORI  ,&tui::TopedFrame::OnFlipHor    );
   tbar->addToolItem(_nextToolId++, wxT("flip_vert") , tpdART_FLIP_VERT  ,&tui::TopedFrame::OnFlipVert   );
   tbar->AddSeparator();
   tbar->addToolItem(_nextToolId++, wxT("group")     , tpdART_GROUP      ,&tui::TopedFrame::OnCellGroup  );
   tbar->addToolItem(_nextToolId++, wxT("ungroup")   , tpdART_UNGROUP    ,&tui::TopedFrame::OnCellUngroup);

//   _resourceCenter->appendTool(wxT("edit"), wxT("cut_box"      ), wxT("cut_box"      ), wxT(""), wxT("cut with box"   ), &tui::TopedFrame::OnBoxCut       );
////   _resourceCenter->appendTool(wxT("edit"), wxT("edit_push"    ), wxT("edit_push"    ), wxT(""), wxT("edit push"      ), &tui::TopedFrame::OnCellPush     );
////   _resourceCenter->appendTool(wxT("edit"), wxT("edit_pop"     ), wxT("edit_pop"     ), wxT(""), wxT("edit pop"       ), &tui::TopedFrame::OnCellPop      );
//
   _allToolBars.push_back(tbar);
   return tbar;
}

wxAuiToolBar* tui::ResourceCenter::initToolBarC(wxWindow* parent)
{
   TpdToolBarWrap* tbar = DEBUG_NEW TpdToolBarWrap(parent, _curTBSize, tpdPane_TB_B, &_execResources);

   tbar->addToolItem(_nextToolId++, wxT("zoom_in")    , tpdART_ZOOM_IN   ,&tui::TopedFrame::OnZoomIn     );
   tbar->addToolItem(_nextToolId++, wxT("zoom_out")   , tpdART_ZOOM_OUT  ,&tui::TopedFrame::OnZoomOut    );
   tbar->addToolItem(_nextToolId++, wxT("zoom_all")   , tpdART_ZOOM_ALL  ,&tui::TopedFrame::OnZoomAll    );
   tbar->addToolItem(_nextToolId++, wxT("add ruler")  , tpdART_RULER     ,&tui::TopedFrame::OnAddRuler   );

   _allToolBars.push_back(tbar);
   return tbar;
}

wxAuiToolBar* tui::ResourceCenter::initToolBar(wxWindow*, const wxString&)
{
   return NULL; //TODO
}

//void tui::ResourceCenter::addToolBarItem(wxAuiToolBar* tbar, const wxString& label, const wxArtID& artID, callbackMethod cbMethod)
//{
//   _execResources[_nextToolId] = DEBUG_NEW TpdResCallBack(cbMethod);
//   tbar->AddTool(_nextToolId++, artID , wxArtProvider::GetBitmap(artID , wxART_TOOLBAR, _curTBSize), label);
//}

//=============================================================================
bool tui::checkToolSize(IconSizes size)
{
   for(IconSizes isz = ICON_SIZE_16x16; isz < ICON_SIZE_END; isz++)
   {
      if(size == isz) break;
   }
   if(ICON_SIZE_END == size) return false;
   else return true;
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
      _rootDir[CI].Normalize();
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
      fName.Normalize();
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
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtInt()));
}

int tellstdfunc::stdTOOLBARSIZE::execute()
{
   int         size   = getWordValue();
   int      direction= getWordValue();
   tui::IconSizes sz = static_cast<tui::IconSizes>(size);
   if(tui::checkToolSize(sz))
   {
      wxCommandEvent eventToolBarSize(wxEVT_TOOLBARSIZE);
      eventToolBarSize.SetExtraLong(direction);
      eventToolBarSize.SetInt(size);
      wxPostEvent(Toped, eventToolBarSize);
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
tellstdfunc::stdDEFINETOOLBAR::stdDEFINETOOLBAR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdDEFINETOOLBAR::execute()
{
   std::string   toolbarname   = getStringValue();
   wxString str = wxString(toolbarname.c_str(), wxConvUTF8);
   wxCommandEvent eventToolBarDef(wxEVT_TOOLBARDEF);
   eventToolBarDef.SetString(str);
   wxPostEvent(Toped, eventToolBarDef);

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

   telldata::TtHshStr* iconCmdMap;
   for (unsigned i = 0; i < iconCmdMapList->size(); i++)
   {
      iconCmdMap = static_cast<telldata::TtHshStr*>((iconCmdMapList->mlist())[i]);
      std::string toolName = iconCmdMap->key().value();
      std::string tellCommand = iconCmdMap->value().value();

      //WARNING!!! Object data must destroyed in event reciever
      StringMapClientData *data = DEBUG_NEW StringMapClientData(toolName, tellCommand);

      wxString str = wxString(toolbarName.c_str(), wxConvUTF8);
      wxCommandEvent eventToolBarDef(wxEVT_TOOLBARADDITEM);
      eventToolBarDef.SetClientObject(data);
      eventToolBarDef.SetString(str);
      wxPostEvent(Toped, eventToolBarDef);

   }
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
   std::string   itemname   = getStringValue();
   wxString utfitemname = wxString(itemname.c_str(), wxConvUTF8);
   std::string   toolbarname   = getStringValue();
   wxString utftoolbarname = wxString(toolbarname.c_str(), wxConvUTF8);

   wxCommandEvent eventToolBarDelItem(wxEVT_TOOLBARDELETEITEM);
   eventToolBarDelItem.SetString(utftoolbarname);
   wxStringClientData *data = DEBUG_NEW wxStringClientData(utfitemname);
   eventToolBarDelItem.SetClientObject(data);
   wxPostEvent(Toped, eventToolBarDelItem);

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

