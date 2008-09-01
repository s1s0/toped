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

#include "../tpd_parser/ted_prompt.h"
#include "../tpd_common/ttt.h"
#include "../tpd_common/tuidefs.h"
#include "../src/toped.h"

extern console::ted_cmd*         Console;
extern tui::TopedFrame*          Toped;


extern const wxEventType         wxEVT_TOOLBARSIZE;


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
         menuBar->Insert(menuBar->GetMenuCount()-1, menu, wxString(menuString.c_str(), wxConvUTF8));
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
   menuItem->SetText(wxString(insertedString.c_str(), wxConvUTF8));
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
                        currentSize(ICON_SIZE_16x16), _helpString(helpString), _method(cbMethod)
{
	wxImage image(wxString(bitmapName.c_str(), wxConvFile),wxBITMAP_TYPE_PNG);
	for( 
	IconSizes isz = ICON_SIZE_16x16; 
	isz < ICON_SIZE_END; 
	isz=static_cast<IconSizes>(static_cast<int>(isz)+1))
	{
		_bitmapNames[isz] = bitmapName;
		_bitmaps[isz] = wxBitmap(image);
	}
}

void tui::ToolItem::addIcon(const std::string &bitmapName, int size)
{
	_bitmapNames[size] = bitmapName;
	wxImage image(wxString(_bitmapNames[size].c_str(), wxConvFile),wxBITMAP_TYPE_PNG);
	_bitmaps[size] = wxBitmap(image);
}


tui::ToolItem::~ToolItem()
{
}

void tui::ToolItem::changeToolSize(IconSizes size)
{
	if(checkToolSize(size))
	{
		currentSize = size;
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
		:_name(name),_ID(ID), _dockDirection(direction)
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
	
	_toolBar->Realize();
	Toped->getAuiManager()->AddPane(_toolBar, paneInfo.ToolbarPane().
		Name(wxString(_name.c_str(), wxConvUTF8)).Direction(_dockDirection).Floatable(false));
	Toped->getAuiManager()->Update();
}

tui::ToolBarHandler::~ToolBarHandler()
{
	for(toolList::iterator it=_tools.begin();it!=_tools.end();it++)
	{
		delete (*it);
	}
	_tools.clear();
}

void	tui::ToolBarHandler::changeToolSize(IconSizes size)
{
	//------------------------------------
	Toped->getAuiManager()->DetachPane(_toolBar);
	delete _toolBar;

	Toped->getAuiManager()->Update();

	for(toolList::iterator it=_tools.begin();it!=_tools.end();it++)
	{
		(*it)->changeToolSize(size);
	}

	wxAuiPaneInfo paneInfo;
	
	if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
	{
		_toolBar = DEBUG_NEW TpdToolBar(_ID, wxTB_VERTICAL, size);
	}
	else
	{
		_toolBar = DEBUG_NEW TpdToolBar(_ID, wxTB_HORIZONTAL, size);
	}

	for(toolList::iterator it=_tools.begin();it!=_tools.end();it++)
	{
		_toolBar->AddTool((*it)->ID(),wxT(""),(*it)->bitmap(), wxString((*it)->helpString().c_str(),wxConvUTF8));
	}
	_toolBar->Realize();

	attachToAUI();
	Toped->getAuiManager()->Update();
}


void tui::ToolBarHandler::addTool(int ID1, const std::string &toolBarItem, const std::string &iconName,
												const std::string &iconFileName,IconSizes size,
												const std::string hotKey,
												const std::string &helpString,
												callbackMethod cbMethod)
{
	toolList::const_iterator it;
	for(it = _tools.begin(); it != _tools.end(); it++)
	{
		if ((*it)->name() == iconName)
		{
			(*it)->addIcon(iconFileName, size);
			break;
		}
	}
	if (it == _tools.end()) 
	{
		ToolItem *tool = DEBUG_NEW ToolItem(ID1, toolBarItem, iconFileName, hotKey, helpString, cbMethod);

		_tools.push_back(tool);
		_toolBar->AddTool(tool->ID(),wxT(""),tool->bitmap(), wxString(helpString.c_str(), wxConvUTF8));

		Toped->getAuiManager()->DetachPane(_toolBar);
		_toolBar->Realize();
		attachToAUI();
	}
}

void tui::ToolBarHandler::attachToAUI(void)
{
		if((_dockDirection==wxAUI_DOCK_LEFT)||(_dockDirection==wxAUI_DOCK_RIGHT))
		{
			Toped->getAuiManager()->AddPane(_toolBar, wxAuiPaneInfo().ToolbarPane().
			Name(wxString(_name.c_str(), wxConvUTF8)).Direction(_dockDirection).Gripper().GripperTop(true).Floatable(false).
			TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true));
		}
		else
		{
			Toped->getAuiManager()->AddPane(_toolBar, wxAuiPaneInfo().ToolbarPane().
			Name(wxString(_name.c_str(), wxConvUTF8)).Direction(_dockDirection).Gripper().GripperTop(false).Floatable(false).
			TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false));
		}

		Toped->getAuiManager()->Update();
}


void	tui::ToolBarHandler::execute(int ID1)
{
	for(toolList::iterator it = _tools.begin();it!=_tools.end(); it++)
	{
	   if (((*it)->ID())==ID1) 
      {
         //Priority - user defined function
        /* if (!((*it)->function()).empty())
         {
            Console->parseCommand(wxString(((*mItem)->function()).c_str(), wxConvUTF8));
				return;
         }
         else*/
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
            Console->parseCommand(wxString(((*mItem)->function()).c_str(), wxConvUTF8));
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

void tui::ResourceCenter::setToolBarSize(const std::string &toolBarName, IconSizes size)
{
	if(checkToolSize(size))
	{
		/*toolBarList::const_iterator it;
		for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
		{
			if ((*it)->name()==toolBarName)
			{
				(*it)->changeToolSize(size);
			}
			else
			{
				wxString info;
				info << wxT("Unknown name of toolbar");
				tell_log(console::MT_WARNING,info);
			}*/
		toolBarList::const_iterator it;
		for(it=_toolBars.begin(); it!=_toolBars.end(); it++)
		{
			(*it)->changeToolSize(size);
		}

	}

	/*}
	else
	{
		wxString info;
		info << wxT("Wrong size for icon was chosen.");
      tell_log(console::MT_ERROR,info);
	}*/
}

void tui::ResourceCenter::appendTool(const std::string &toolBarName, const std::string &toolBarItem,
							const  std::string &iconName, IconSizes size,
							const std::string &hotKey, const std::string &helpString,
							callbackMethod cbMethod)
{
	int ID; 
	ToolBarHandler* toolBar;
//   wxBitmap bitmap = wxIcon(wxString(iconName.c_str(), wxConvUTF8), wxBITMAP_TYPE_ICO_RESOURCE, 24, 24);

	//set correct filename for toolBarItem
	std::string fullIconName = _IconDir+iconName;


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
	//increase counter of toolItems
	ID = TDUMMY_TOOL + _toolCount;
	_toolCount++;

	toolBar->addTool(ID, toolBarItem, toolBarItem, fullIconName, size, hotKey, helpString, cbMethod); 
	//toolBar->addTool(tool);

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
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
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
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

int tellstdfunc::stdTOOLBARSIZE::execute()
{
   int size = getWordValue();
	tui::IconSizes sz = static_cast<tui::IconSizes>(size);
	if(checkToolSize) 
	{
		wxCommandEvent eventToolBarSize(wxEVT_TOOLBARSIZE);
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


