#ifndef RESOURCE_CENTER_INCLUDED
#define RESOURCE_CENTER_INCLUDED

#include <string>
#include <vector>
#include <wx/wx.h>

#include "layoutcanvas.h"

namespace tui 
{
   class MenuItemHandler
   {
   public:
      MenuItemHandler(void):_ID(0), _menuItem(""), _hotKey(""), _function("") {_inserted = false;};
      MenuItemHandler(int ID, std::string menuItem, std::string hotKey, std::string function)
         :_ID(ID), _menuItem(menuItem), _hotKey(hotKey), _function(function) {_inserted = false;};

      std::string    menuItem(void) { return _menuItem;};
      std::string    hotKey(void)   { return _hotKey;};
      std::string    function(void) { return _function;};
      bool           inserted(void)       { return _inserted; }
      void           setInserted(bool ins){_inserted = ins;}
      int            ID(void)             {return _ID;};

    private:
      int         _ID; 
      std::string _menuItem; 
      std::string _hotKey;
      std::string _function;
      bool        _inserted;
   };


   class ResourceCenter
   {
   public:
      ResourceCenter(void): _menuCount(0) {};
      ~ResourceCenter(void) {};
      //Using for build of complete menu
      void buildMenu(wxMenuBar *menuBar);
      //Insert new menu item
      //!IMPORTANT after appending call buildMenu()
      void appendMenu(std::string menuItem, std::string hotKey, std::string function);
      
      void executeMenu(int ID);
      
   private:
      std::vector <MenuItemHandler> _menus;
      int _menuCount; //quantity of menu items;


   };
}
#endif //RESOURCE_CENTER_INCLUDED
