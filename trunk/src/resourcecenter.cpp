#include <sstream>
#include "resourcecenter.h"

#include "../tpd_parser/ted_prompt.h"
#include "../tpd_common/ttt.h"

extern console::ted_cmd*         Console;


void tui::ResourceCenter::buildMenu(wxMenuBar *menuBar)
{
   wxMenu* menu, *menu2;
   wxMenuItem *menuItem;
   int menuID;
   std::vector<std::string> menuNames;// = split
   MenuItemHandler *mItem;
   int id;
   std::string menustring;
   //looks all saved menus
   for(std::vector <MenuItemHandler>::iterator it=_menus.begin(); it!=_menus.end(); it++)
   {
      mItem = it;
      //only for unshown menus
      if (mItem->inserted()== false)
      {
         id = mItem->ID();
         menustring = mItem->menuItem();
         menuNames = split(menustring, '/');
         
         if (menuNames.size()==0) continue;

         if (menuNames.size()==1)
         {
            //Create new item in main menu
            menu = new wxMenu();
            menuBar->Insert(menuBar->GetMenuCount()-1, menu, menuNames[0].c_str());
            mItem->setInserted(true);
            continue;
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
         for (int j=1; j<(menuNames.size()-1); j++)
         {
            menuID=menu->FindItem(menuNames[j].c_str());
            menuItem = menu->FindItem(menuID, &menu2);
            if ((wxNOT_FOUND == menuID) || (menu2 == NULL))
            {
               menu2 = new wxMenu();
               menu->Append(TMDUMMY + _menuCount, menuNames[j].c_str(), menu2);
               _menuCount++;
               menu = menu2;
            }
            else
            {
               menu = menu2;
            }
         }
         //last item
         menuItem = new wxMenuItem(menu, mItem->ID(), (*(menuNames.end()-1)).c_str());
         _menuCount++;
         menu->Append(menuItem);
         //menu->Append(mItem->ID(), "", menu2);
   
         mItem->setInserted(true);
      }
   }
   //menu->Append(TMDUMMY, "dd");
}

void tui::ResourceCenter::appendMenu(std::string menuItem, std::string hotKey, std::string function)
{
   int ID = TMDUMMY + _menuCount;

   //???Here need to find already existed menus and hot-keys


   MenuItemHandler mItem(ID, menuItem, hotKey, function);
   _menus.push_back(mItem);
   _menuCount++;
}


void tui::ResourceCenter::executeMenu(int ID1)
{
   MenuItemHandler *mItem;
   for(std::vector <MenuItemHandler>::iterator it=_menus.begin(); it!=_menus.end(); it++)
   {
      mItem = it;
      if ((mItem->ID())==ID1) 
      Console->parseCommand((mItem->function()).c_str());
   }
}