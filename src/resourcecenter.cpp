#include <sstream>
#include "resourcecenter.h"

#include "../tpd_parser/ted_prompt.h"
#include "../tpd_common/ttt.h"

extern console::ted_cmd*         Console;
extern tui::TopedFrame*          Toped;


tui::MenuItemHandler::MenuItemHandler(void)
   :_ID(0), _menuItem(""), _hotKey(""), _function(""), _method(NULL)
{
}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, std::string function)
   :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(function), _method(NULL) 
{
   _inserted = false;
}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod)
   :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(""), _method(cbMethod) 
{
   _inserted = false;
}

tui::MenuItemHandler::MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString)
   :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(""), _method(cbMethod), _helpString(helpString) 
{
      _inserted = false;
}

void tui::MenuItemHandler::create(wxMenuBar *menuBar)
{
   wxMenu* menu, *menu2;
   wxMenuItem *menuItem;
   int menuID;
   std::vector<std::string> menuNames;

   int id;
   std::string menustring;
   
   id = _ID;
   menustring = _menuItem;
   menuNames = split(menustring, '/');
         
   if (menuNames.size()==0) return;

   if (menuNames.size()==1)
   {
      //Create new item in main menu
      menu = new wxMenu();
      menuBar->Insert(menuBar->GetMenuCount()-1, menu, menuNames[0].c_str());
      _inserted = true;
      return;
   }
         
   //First item - Top Menu 
   std::string menuString =menuNames[0];
   menuID = menuBar->FindMenu(menuString.c_str());
   if (wxNOT_FOUND == menuID)
      {
         // create at the left of Help menu
         menu = new wxMenu();
         menuBar->Insert(menuBar->GetMenuCount()-1, menu, menuString.c_str());
         menuID = menuBar->FindMenu(menuString.c_str());
      }
      
      menu = menuBar->GetMenu(menuID);
      //intermediate menu items - create if does not exists
      for (unsigned int j=1; j<(menuNames.size()-1); j++)
      {
         menuID=menu->FindItem(menuNames[j].c_str());
         menuItem = menu->FindItem(menuID, &menu2);
            
         if ((wxNOT_FOUND == menuID) || (menu2 == NULL))
         {
            menu2 = new wxMenu();
            menu->Append(_ID+10000 , menuNames[j].c_str(), menu2);
            menu = menu2;
         }
         else
         {
            menu = menuItem->GetSubMenu();
         }
      }
      
      //last item
      menuItem = new wxMenuItem(menu, _ID, (*(menuNames.end()-1)).c_str(), _helpString.c_str());
         
      menu->Append(menuItem);
            
      _inserted = true;
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
   wxMenu* menu, *menu2;
   wxMenuItem *menuItem;
   int menuID;
   std::vector<std::string> menuNames;// = split
   std::string menustring;

   menustring = _menuItem;
   menuNames = split(menustring, '/');
         
   if (menuNames.size()==0) return;

   //First item - Top Menu 
   std::string menuString =menuNames[0];
   menuID = menuBar->FindMenu(menuString.c_str());
   if (wxNOT_FOUND == menuID)
   {
      // create at the left of Help menu
      menu = new wxMenu();
      menuBar->Insert(menuBar->GetMenuCount()-1, menu, menuString.c_str());
      menuID = menuBar->FindMenu(menuString.c_str());
   }
   
   menu = menuBar->GetMenu(menuID);
   //intermediate menu items - create if does not exists
   for (unsigned int j=1; j<(menuNames.size()); j++)
   {
      menuID=menu->FindItem(menuNames[j].c_str());
      menuItem = menu->FindItem(menuID, &menu2);
      if ((wxNOT_FOUND == menuID) || (menu2 == NULL))
      {
         menu2 = new wxMenu();
         menu->Append(TMDUMMY /*+ _menuCount*/, menuNames[j].c_str(), menu2);
         menu = menu2;
      }
      else
      {
         menu = menuItem->GetSubMenu();// menu2;
      }
   }
      
   menu->AppendSeparator();
            
   _inserted = true;
}

tui::ResourceCenter::ResourceCenter(void):_menuCount(0) 
{
}

tui::ResourceCenter::~ResourceCenter(void) 
{
   for(itemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      delete (*mItem);
   }
   _menus.clear();
}

void tui::ResourceCenter::buildMenu(wxMenuBar *menuBar)
{
   wxMenu* menu;
   int menuID;
   std::vector<std::string> menuNames;// = split
   std::string menustring;

   //Try to create Help menu
   menuID = menuBar->FindMenu("&Help");
   if (wxNOT_FOUND == menuID)
   {
      // create Help menu on most right
      menu = new wxMenu();
      menuBar->Insert(menuBar->GetMenuCount(), menu, "&Help");
   }

   //looks all saved menus
   for(itemList::iterator mItem=_menus.begin(); mItem!=_menus.end(); mItem++)
   {
      //only for unshown menus
      if ((*mItem)->inserted()== false)
      {
         (*mItem)->create(menuBar);
      }
   }
}

void tui::ResourceCenter::appendMenu(std::string menuItem, std::string hotKey, std::string function)
{
   int ID = TMDUMMY + _menuCount;

   //???Here need to find already existed menus and hot-keys


   MenuItemHandler* mItem = new MenuItem(ID, menuItem, hotKey, function);
   _menus.push_back(mItem);
   _menuCount++;
}

void tui::ResourceCenter::appendMenu(std::string menuItem, std::string hotKey, callbackMethod cbMethod)
{
   int ID = TMDUMMY + _menuCount;

   //???Here need to find already existed menus and hot-keys

   MenuItemHandler* mItem= new MenuItem(ID, menuItem, hotKey, cbMethod);
   _menus.push_back(mItem);
   _menuCount++;
}

void tui::ResourceCenter::appendMenu(std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString)
{
   int ID = TMDUMMY + _menuCount;

   //???Here need to find already existed menus and hot-keys

   MenuItemHandler* mItem= new MenuItem(ID, menuItem, hotKey, cbMethod, helpString);
   _menus.push_back(mItem);
   _menuCount++;
   
};

void tui::ResourceCenter::appendMenuSeparator(std::string menuItem)
{
   MenuItemHandler* mItem= new MenuItemSeparator(menuItem);
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
            Console->parseCommand(((*mItem)->function()).c_str());
         }
         else
         {
            if ((*mItem)->method()!=NULL)
            {
               callbackMethod cbMethod;
               wxCommandEvent event(0);
               cbMethod = (*mItem)->method();
               (Toped->* cbMethod)(event);
            }
         }
      }
   }
}