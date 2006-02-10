#ifndef RESOURCE_CENTER_INCLUDED
#define RESOURCE_CENTER_INCLUDED

#include <string>
#include <vector>
#include <wx/wx.h>

#include "layoutcanvas.h"


/*WARNING!!!
   Current version ResourceCenter can use callback functions only
   for TopedFrame class. Using all other classes include TopedFrame children
   is VERY dangerous.
   In future replace "callbackMethod" type to boost::function, winni::closure
   or similar library
*/
namespace tui 
{
   //forward declarations
   class TopedFrame;
   class MenuItemHandler;

   typedef void (TopedFrame::*callbackMethod)(wxCommandEvent&);
   typedef std::vector <MenuItemHandler*> itemList;


   class MenuItemHandler
   {
   public:
      MenuItemHandler(void);
      MenuItemHandler(int ID, std::string menuItem, std::string hotKey, std::string function);
      MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod);
      MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString);
      virtual ~MenuItemHandler() {};
      std::string    menuItem(void) { return _menuItem;};
      std::string    hotKey(void)   { return _hotKey;};
      std::string    function(void) { return _function;};
      callbackMethod method(void)   { return _method;};
      bool           inserted(void)       { return _inserted; }
      void           setInserted(bool ins){_inserted = ins;}
      int            ID(void)             {return _ID;};
      
      virtual void create(wxMenuBar *menuBar);
    protected:
      int         _ID; 
      std::string _menuItem; 
      std::string _hotKey;
      std::string _function;
      std::string _helpString;
      bool        _inserted;
      callbackMethod _method;
   };


   class MenuItem:public MenuItemHandler
   {
   public:
      MenuItem(void);
      MenuItem(int ID, std::string menuItem, std::string hotKey, std::string function)
         :MenuItemHandler(ID, menuItem, hotKey, function) {};

      MenuItem(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod)
         :MenuItemHandler(ID, menuItem, hotKey, cbMethod) {};

      MenuItem(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString)
         :MenuItemHandler(ID, menuItem, hotKey, cbMethod, helpString) {};
      virtual ~MenuItem() {};
   };

   class MenuItemSeparator:public MenuItemHandler
   {
   public:
      MenuItemSeparator(void);
      MenuItemSeparator(std::string menuItem);
      virtual ~MenuItemSeparator() {};
      
      virtual void create(wxMenuBar *menuBar);
   };



   class ResourceCenter
   {
   public:
      ResourceCenter(void);
      ~ResourceCenter(void);
      //Using for build of complete menu
      void buildMenu(wxMenuBar *menuBar);
      //Insert new menu item
      //!IMPORTANT after appending call buildMenu()
      void appendMenu(std::string menuItem, std::string hotKey, std::string function);
      void appendMenu(std::string menuItem, std::string hotKey, callbackMethod cbMethod);
      void appendMenu(std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString);
      void appendMenuSeparator(std::string menuItem);
      void executeMenu(int ID);
      
   private:
      itemList _menus;
      int _menuCount; //quantity of menu items;
      

   };
}
#endif //RESOURCE_CENTER_INCLUDED
