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
      std::string    menuItem(void) const    { return _menuItem;};
      std::string    hotKey(void)   const    { return _hotKey;};
      std::string    function(void) const    { return _function;};
      std::string    helpString(void) const  { return _helpString;};
      callbackMethod method(void)   const    { return _method;};
      void           changeInterior(std::string hotKey, std::string function="", callbackMethod cbMethod=NULL, std::string helpString="");
      bool           inserted(void) const    { return _inserted; }
      bool           changed(void)  const    { return _changed; }
      int            ID(void)       const    { return _ID;} ;
      
      void           setInserted(bool ins)   {_inserted = ins;};
      void           setChanged(bool  chang) {_changed = chang;};
   
      //Create menu item into this wxMenuBar 
      virtual void create(wxMenuBar *menuBar);
      //Change menu item into this wxMenuBar 
      //used for already existed items
      virtual void recreate(wxMenuBar *menuBar);
    protected:
      wxMenu* buildPath(wxMenuBar *menuBar, const std::vector<std::string> &menuNames);
      
      int         _ID; 
      std::string _menuItem; 
      std::string _hotKey;
      std::string _function;
      std::string _helpString;
      bool        _inserted;
      bool        _changed;
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
      void appendMenu(const std::string &menuItem, const std::string &hotKey, const std::string &function);
      void appendMenu(const std::string &menuItem, const std::string &hotKey, callbackMethod cbMethod);
      void appendMenu(const std::string &menuItem, const std::string &hotKey, callbackMethod cbMethod, const std::string &helpString);
      void appendMenuSeparator(const std::string &menuItem);
      void executeMenu(int ID);
      bool checkExistence(const tui::MenuItemHandler & item);
   private:
      //produce lowercase string and exclude unwanted character
      std::string simplify(std::string str, char ch);


      itemList _menus;
      int _menuCount; //quantity of menu items;
      

   };
}
#endif //RESOURCE_CENTER_INCLUDED
