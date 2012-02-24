
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

tui::MenuItemHandler::MenuItemHandler(void)
   :_ID(0), _menuItem(""), _hotKey(""), _function(""), _method(NULL)
{
}

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


tui::ToolItem::ToolItem(int toolID, const std::string &name,
                        const std::string &bitmapName,
                        const std::string &hotKey,
                        const std::string &helpString,
                        callbackMethod cbMethod)
                        :_ID(toolID), _name(name),/*_hotKey(hotKey),*/
                        _currentSize(ICON_SIZE_16x16),
                        _function(""),
                        _helpString(helpString),
                        _method(cbMethod),
                        _ok(false)
{
   init(bitmapName);
}

tui::ToolItem::ToolItem(int toolID, const std::string &name,
                        const std::string &bitmapName,
                        const std::string &hotKey,
                        const std::string &helpString,
                        const std::string func)
                        :_ID(toolID), _name(name),/*_hotKey(hotKey),*/
                        _currentSize(ICON_SIZE_16x16),
                        _function(func),
                        _helpString(helpString),
                        _method(NULL),
                        _ok(false)
{
   init(bitmapName);
}

void tui::ToolItem::init(const std::string& bitmapName)
{
   wxImage image[ICON_SIZE_END];
   wxImage tempImage;
   std::string tempImageName;
   IconSizes isz;
   bool absentFile = false;
   for(
      isz = ICON_SIZE_16x16;
      isz < ICON_SIZE_END;
      isz=static_cast<IconSizes>(static_cast<int>(isz)+1))
   {
      char temp[100];
      sprintf(temp,"%i", IconSizesValues[isz]);
      std::string name = bitmapName;
      name.append(temp);
      name.append("x");
      name.append(temp);
      name.append(".png");
      (image[isz]).LoadFile(wxString(name.c_str(), wxConvFile),wxBITMAP_TYPE_PNG);
      _bitmapNames[isz] = name;
      if(image[isz].IsOk())
      {
         _bitmaps[isz] = wxBitmap(image[isz]);
         _ok = true;
         if(!tempImage.IsOk())
         {
            tempImage = image[isz]; //Save temp image for filling of missing icons
            tempImageName = name;
         }
      }
      else
      {
         absentFile = true;
      }
   }


   if(absentFile)
   {
      if (tempImage.IsOk())
      {
         for(
            isz = ICON_SIZE_16x16;
            isz < ICON_SIZE_END;
            isz=static_cast<IconSizes>(static_cast<int>(isz)+1))
            {
               if(!image[isz].IsOk())
               {
                  //image[isz] = tempImage.Copy();
                  image[isz].LoadFile(wxString(tempImageName.c_str(), wxConvFile),wxBITMAP_TYPE_PNG);
                  _bitmapNames[isz] = tempImageName;
                  _bitmaps[isz] = wxBitmap(image[isz]);

                  std::ostringstream ost;
                  ost<<"No icon file  "<<_bitmapNames[isz];
                  tell_log(console::MT_WARNING,ost.str());
                  ost.clear();
                  ost<<"Replaced";
                  tell_log(console::MT_WARNING,ost.str());
               }
            }
      }
      else
      {
         std::ostringstream ost;
         ost<<"No any proper file for "<<bitmapName;
         tell_log(console::MT_ERROR,ost.str());
      }
   }

}

tui::ToolItem::~ToolItem()
{
}

void tui::ToolItem::changeToolSize(IconSizes size)
{
   if(checkToolSize(size))
   {
      _currentSize = size;
   }
   else
   {
      wxString info;
      info << wxT("Wrong size for icon was chosen.");
      tell_log(console::MT_ERROR,info);
   }
}


tui::TpdToolBar::TpdToolBar(int ID, long style, IconSizes iconSize)
   :wxToolBar(Toped->getFrame(), ID, wxDefaultPosition,
      wxSize(1000, 30), wxTB_NODIVIDER|wxTB_FLAT )
{
   SetWindowStyle(style|GetWindowStyle());
   int sz = IconSizesValues[iconSize];
   SetToolBitmapSize(wxSize(sz, sz));
}

tui::ToolBarHandler::ToolBarHandler(int ID, const std::string & name, int direction)
      :_name(name),_ID(ID), _dockDirection(direction),_floating(false),_coord(-1,-1),
      _currentSize(ICON_SIZE_16x16)
{
   wxAuiPaneInfo paneInfo;

   if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
   {
      _toolBar = DEBUG_NEW TpdToolBar(ID, wxTB_VERTICAL, ICON_SIZE_16x16);
      paneInfo = Toped->getAuiManager()->GetPane(_toolBar);
      paneInfo.TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true);
   }
   else
   {
      _toolBar = DEBUG_NEW TpdToolBar(ID, wxTB_HORIZONTAL, ICON_SIZE_16x16);
      paneInfo = Toped->getAuiManager()->GetPane(_toolBar);
      paneInfo.TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
   }
   _toolBar->Hide();
   /*_toolBar->Realize();
   Toped->getAuiManager()->AddPane(_toolBar, paneInfo.ToolbarPane().
      Name(wxString(_name.c_str(), wxConvUTF8)).Direction(_dockDirection).Floatable(false));
   Toped->getAuiManager()->Update();*/
}

tui::ToolBarHandler::~ToolBarHandler()
{
   for(toolList::iterator it=_tools.begin();it!=_tools.end();it++)
   {
      delete (*it);
   }
   _tools.clear();
}

void   tui::ToolBarHandler::changeToolSize(IconSizes size)
{
   _currentSize = size;
   wxAuiPaneInfo paneInfo = Toped->getAuiManager()->GetPane(_toolBar);
   _floating = paneInfo.IsFloating();
   if(paneInfo.IsDocked())
   {
      _dockDirection = paneInfo.dock_direction;
   }
   _coord = paneInfo.floating_pos;

   Toped->getAuiManager()->DetachPane(_toolBar);
   _toolBar->Destroy();

   Toped->getAuiManager()->Update();

   if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
   {
      _toolBar = DEBUG_NEW TpdToolBar(_ID, wxTB_VERTICAL, size);
   }
   else
   {
      _toolBar = DEBUG_NEW TpdToolBar(_ID, wxTB_HORIZONTAL, size);
   }

   attachToAUI();
   for(toolList::iterator it=_tools.begin();it!=_tools.end();it++)
   {
      (*it)->changeToolSize(_currentSize);
      _toolBar->AddTool((*it)->ID(),wxT(""),(*it)->bitmap(), wxString((*it)->helpString().c_str(),wxConvUTF8));
      Toped->getAuiManager()->DetachPane(_toolBar);
      _toolBar->Realize();
      attachToAUI();
   }
}


void tui::ToolBarHandler::addTool(int ID1, const std::string &toolBarItem, const std::string &iconName,
                                    const std::string &iconFileName,
                                    const std::string hotKey,
                                    const std::string &helpString,
                                    callbackMethod cbMethod)
{
   clearTool(iconName);
   ToolItem *tool = DEBUG_NEW ToolItem(ID1, toolBarItem, iconFileName, hotKey, helpString, cbMethod);
   if (tool->isOk())
   {
      tool->changeToolSize(_currentSize);
      _tools.push_back(tool);
      _toolBar->AddTool(tool->ID(),wxT(""),tool->bitmap(), wxString(helpString.c_str(), wxConvUTF8));

      Toped->getAuiManager()->DetachPane(_toolBar);
      _toolBar->Realize();
      attachToAUI();
   }
   else
   {
      delete tool;
   }
}

void tui::ToolBarHandler::addTool(int ID1, const std::string &toolBarItem, const std::string &iconName,
                                    const std::string &iconFileName,
                                    const std::string hotKey,
                                    const std::string &helpString,
                                    const std::string &func)
{
   clearTool(iconName);
   ToolItem *tool = DEBUG_NEW ToolItem(ID1, toolBarItem, iconFileName, hotKey, helpString, func);
   if (tool->isOk())
   {
      tool->changeToolSize(_currentSize);
      _tools.push_back(tool);
      _toolBar->AddTool(tool->ID(),wxT(""),tool->bitmap(), wxString(helpString.c_str(), wxConvUTF8));

      Toped->getAuiManager()->DetachPane(_toolBar);
      _toolBar->Realize();
      attachToAUI();
   }
   else
   {
      delete tool;
   }
}

void   tui::ToolBarHandler::deleteTool(const std::string &toolBarItem)
{
   toolList::iterator it;
   for(it = _tools.begin(); it != _tools.end(); it++)
   {
      if ((*it)->name() == toolBarItem)
      {
         std::ostringstream ost;
         ost<<"Tool item "+toolBarItem+" is deleted";
         tell_log(console::MT_WARNING,ost.str());
         _toolBar->DeleteTool((*it)->ID());
         delete (*it);
         _tools.erase(it);
         break;
      }
   }
   Toped->getAuiManager()->DetachPane(_toolBar);
   _toolBar->Realize();
   attachToAUI();
}

void   tui::ToolBarHandler::clearTool(const std::string &iconName)
{
   toolList::iterator it;
   for(it = _tools.begin(); it != _tools.end(); it++)
   {
      if ((*it)->name() == iconName)
      {
         std::ostringstream ost;
         ost<<"Tool item "+iconName+" is redefined";
         tell_log(console::MT_WARNING,ost.str());
            delete (*it);
         _tools.erase(it);
         break;
      }
   }
}

void tui::ToolBarHandler::attachToAUI(void)
{
   wxAuiPaneInfo pane;
   pane=wxAuiPaneInfo().ToolbarPane().Name(wxString(_name.c_str(), wxConvUTF8)).Direction(_dockDirection).Gripper().GripperTop(false).Floatable(false).
         TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
   if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
   {
      pane.GripperTop(true).TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true);
   }
   else
   {
      pane.GripperTop(false).TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
   }

   if(_floating)
   {
      pane.Float().FloatingPosition(_coord);
   }
   Toped->getAuiManager()->AddPane(_toolBar,pane);
   Toped->getAuiManager()->Update();
}


void   tui::ToolBarHandler::execute(int ID1)
{
   for(toolList::iterator it = _tools.begin();it!=_tools.end(); it++)
   {
      if (((*it)->ID())==ID1)
      {
         //Priority - user defined function
         if (!((*it)->function()).empty())
         {
            TpdPost::parseCommand(wxString(((*it)->function()).c_str(), wxConvUTF8));
            return;
         }
         else
            if ((*it)->method()!=NULL)
            {
               callbackMethod cbMethod;
               wxCommandEvent cmd_event(0);
               cbMethod = (*it)->method();
               (Toped->* cbMethod)(cmd_event);
               return;
            }
      }
   }
}

tui::ResourceCenter::ResourceCenter(void):
                              _menuCount(0), _toolCount(0), _direction(wxAUI_DOCK_TOP)
{
}

tui::ResourceCenter::~ResourceCenter(void)
{
   for(itemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      delete (*mItem);
   }
   _menus.clear();

   for(toolBarList::iterator tItem=_toolBars.begin(); tItem!=_toolBars.end(); tItem++)
   {
      delete (*tItem);
   }
   _toolBars.clear();
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
   for(itemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
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

   for(itemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
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

   for(itemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
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
      //???Proceed here toolbars
   }
   for(toolBarList::iterator it=_toolBars.begin(); it!=_toolBars.end(); it++)
   {
      (*it)->execute(ID1);
   }
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

void tui::ResourceCenter::setDirection(int direction)
{
   if((direction == wxAUI_DOCK_TOP) || (direction == wxAUI_DOCK_LEFT) ||
      (direction == wxAUI_DOCK_BOTTOM) || (direction == wxAUI_DOCK_RIGHT))
      _direction = direction;
}

void tui::ResourceCenter::setToolBarSize(bool direction, IconSizes size)
{
   if(checkToolSize(size))
   {
      toolBarList::const_iterator it;
      for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
      {
         int dir = (*it)->direction();
         if(_tuihorizontal==direction)
         {
            if((wxAUI_DOCK_TOP == dir) || (wxAUI_DOCK_BOTTOM == dir))
               (*it)->changeToolSize(size);
         }
         else //_tuivertical==direction
         {
            if((wxAUI_DOCK_LEFT == dir) || (wxAUI_DOCK_RIGHT == dir))
               (*it)->changeToolSize(size);
         }
      }
   }
}

void tui::ResourceCenter::defineToolBar(const std::string &toolBarName)
{
   int ID;
   //find toolbar
   std::string str = toolBarName;
   std::transform(str.begin(), str.end(), str.begin(), tolower);
   toolBarList::const_iterator it;
   for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
   {
      if ((*it)->name()==str)
      {
         std::ostringstream ost;
         ost<<"definetoolbar: already defined";
         tell_log(console::MT_WARNING,ost.str());
         break;
      }
   }

   if (it==_toolBars.end())
   {
      ID = TDUMMY_TOOL + _toolCount;
      //Create new toolbar
      ToolBarHandler* toolBar = DEBUG_NEW ToolBarHandler(ID, str, _direction);
      _toolCount++;
      _toolBars.push_back(toolBar);

   }

}

void tui::ResourceCenter::appendTool(const std::string &toolBarName, const std::string &toolBarItem,
                     const  std::string &iconName, const std::string &hotKey, const std::string &helpString,
                     callbackMethod cbMethod)
{
   int ID;
   //set correct filename for toolBarItem
   std::string fullIconName = _IconDir+iconName;
   //increase counter of toolItems
   ID = TDUMMY_TOOL + _toolCount;
   _toolCount++;

   ToolBarHandler* toolBar = proceedTool(toolBarName, toolBarItem);
   toolBar->addTool(ID, toolBarItem, toolBarItem, fullIconName, hotKey, helpString, cbMethod);
}

void tui::ResourceCenter::appendTool(const std::string &toolBarName, const std::string &toolBarItem,
                     const std::string &iconName,
                     const std::string &hotKey,
                     const std::string &helpString,
                     std::string func)
{
   int ID;
   //set correct filename for toolBarItem
   std::string fullIconName = _IconDir+iconName;
   //increase counter of toolItems
   ID = TDUMMY_TOOL + _toolCount;
   _toolCount++;

   ToolBarHandler* toolBar = proceedTool(toolBarName, toolBarItem);
   toolBar->addTool(ID, toolBarItem, toolBarItem, fullIconName, hotKey, helpString, func);
}


void tui::ResourceCenter::deleteTool(const std::string &toolBarName, const std::string &toolBarItem)
{
   //find toolbar
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
   //if toolbar doesn't exist, create it
   if (it==_toolBars.end())
   {
      std::ostringstream ost;
      ost<<"toolbardeleteitem: there is no such toolbar";
      tell_log(console::MT_WARNING,ost.str());
      return;
   }
   else
   {
      (*it)->deleteTool(toolBarItem);
   }
}

tui::ToolBarHandler* tui::ResourceCenter::proceedTool(const std::string &toolBarName, const std::string &toolBarItem)
{
   int ID;
   ToolBarHandler* toolBar;

   //find toolbar
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
   //if toolbar doesn't exist, create it
   if (it==_toolBars.end())
   {
      ID = TDUMMY_TOOL + _toolCount;
      //Create new toolbar
      toolBar = DEBUG_NEW ToolBarHandler(ID, str, _direction);
      _toolCount++;
      _toolBars.push_back(toolBar);
   }
   else
   {
      toolBar = *it;
   }

   return toolBar;
}


bool tui::checkToolSize(IconSizes size)
{
   IconSizes isz;
   for(
      isz = ICON_SIZE_16x16;
      isz < ICON_SIZE_END;
      isz=static_cast<IconSizes>(static_cast<int>(isz)+1))
   {
      if(size == isz) break;
   }
   if(ICON_SIZE_END == size)
      return false;
   else return true;
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
