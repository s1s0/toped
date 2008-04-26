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
#ifndef RESOURCE_CENTER_INCLUDED
#define RESOURCE_CENTER_INCLUDED

#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/laywin.h>

#include "../tpd_bidfunc/tpdf_common.h"

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
	class ToolBarHandler;
	class ToolItem;

   typedef void (TopedFrame::*callbackMethod)(wxCommandEvent&);
   typedef std::vector <MenuItemHandler*> itemList;
	typedef std::vector <ToolBarHandler*> toolBarList;
	typedef std::vector <ToolItem*> toolList;

//=================================
//		Everything about menu
//=================================
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


//=================================
//		Everything about toolbar
//=================================
	class ToolItem
	{
	public:
		/*ToolItem(int toolID, const std::string &name,
							const std::string &bitmapFileName,
							const std::string &hotKey, callbackMethod cbMethod);*/
		ToolItem(int toolID, const std::string &name,
					const std::string &bitmapName,
					const std::string &hotKey, 
					const std::string &helpString, callbackMethod cbMethod);
		//std::string    hotKey(void)   const    { return _hotKey;};
      std::string    function(void) const    { return _function;};
      std::string    helpString(void) const  { return _helpString;};
      callbackMethod method(void)   const    { return _method;};

		virtual ~ToolItem();
		wxBitmap	bitmap(void)	const {return _bitmap;};
		int		ID(void)			const {return _ID;};
		void		changeToolSize(const wxSize& size);
	private:
		int				_ID;
		wxBitmap			_bitmap;
		std::string		_bitmapName;
		//std::string	_hotKey;
      std::string		_function;
      std::string		_helpString;
      callbackMethod	_method;


	};

	class ToolBarHandler:public wxToolBar
	{
	public:
		ToolBarHandler(int ID, std::string name, int direction);
		virtual ~ToolBarHandler();

//		void				addTool(ToolItem *tool);
		void				addTool(int ID1, const std::string &toolBarItem, const std::string iconName, 
										const std::string hotKey, const std::string &helpString, callbackMethod cbMethod);
		void				execute(int ID1);

		std::string		name() const {return _name;};
		void				changeToolSize(const wxSize& size);
	private:
		void				OnSize(wxSizeEvent& event);
		void				OnPaint(wxPaintEvent&event);
		std::string					_name;
		int							_ID;
		//wxToolBar*					_toolBar;
		toolList						_tools;
		int							_dockDirection;
		DECLARE_EVENT_TABLE();
	};


//=================================
//		Resourcecenter is resposible 
//		for handle of all ui action
//		currently only menu and toolbar
//=================================
   class ResourceCenter
   {
   public:
		ResourceCenter(void);
      ~ResourceCenter(void);
		void setIconDir(const std::string& dir) {_IconDir = dir;};
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

		/*void appendTool(const std::string toolBarName, const std::string &toolBarItem,
							const std::string &bitmapFileName,
							const std::string &hotKey, callbackMethod cbMethod, int direction);*/
		void appendTool(const std::string toolBarName, const std::string &toolBarItem,
							const std::string &iconName,
							const std::string &hotKey, 
							const std::string &helpString,
							callbackMethod cbMethod, int direction);
   private:
      //produce lowercase string and exclude unwanted character
      std::string simplify(std::string str, char ch);


      itemList				_menus;
		toolBarList			_toolBars;
      int					_menuCount; //number of menu items
		int					_toolCount; //number of tool items
		std::string			_IconDir;	//directory that contains

   };

}

namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;
   
   TELL_STDCMD_CLASSA(stdADDMENU       );  //
}

#endif //RESOURCE_CENTER_INCLUDED
