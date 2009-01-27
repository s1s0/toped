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
#include "tui.h"

/*WARNING!!!
   Current version ResourceCenter can use callback functions only
   for TopedFrame class. Using all other classes include TopedFrame children
   is VERY dangerous.
   In future replace "callbackMethod" type to boost::function, winni::closure
   or similar library
*/
namespace tui 
{
	const word _tuihorizontal	= 0x0000;
   const word _tuivertical		= 0x0001;

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
		ToolItem(int toolID, const std::string &name,
					const std::string &bitmapName,
					const std::string &hotKey, 
					const std::string &helpString, callbackMethod cbMethod);
		ToolItem(int toolID, const std::string &name,
					const std::string &bitmapName,
					const std::string &hotKey, 
					const std::string &helpString, 
					const std::string func);

//		void addIcon(const std::string &bitmapName, int size); 
		//std::string  hotKey(void)		const		{ return _hotKey;};
      std::string    function(void)		const    { return _function;};
		std::string    name(void)			const    { return _name;};
      std::string    helpString(void)	const		{ return _helpString;};
		bool				isOk(void)			const		{ return _ok;};
      callbackMethod method(void)		const    { return _method;};

		virtual ~ToolItem();
		wxBitmap	bitmap(void)	const {return _bitmaps[_currentSize];};
		int		ID(void)			const {return _ID;};
		void		changeToolSize(IconSizes size);
	private:
		void				init(const std::string& bitmapName);
		int				_ID;
		std::string		_name;
		wxBitmap			_bitmaps[ICON_SIZE_END];
		std::string		_bitmapNames[ICON_SIZE_END];
		IconSizes		_currentSize;
		
		//std::string	_hotKey;
      std::string		_function;	
      std::string		_helpString;
      callbackMethod	_method;
		bool				_ok;
	};

	class TpdToolBar:public wxToolBar
	{
	public:
		TpdToolBar(int ID, long style, IconSizes iconSize);
	private:
		//DECLARE_EVENT_TABLE();
	};

	class ToolBarHandler
	{
	public:
		ToolBarHandler(int ID, const std::string& name, int direction);
		virtual ~ToolBarHandler();

		void				addTool(int ID1, const std::string &toolBarItem, const std::string &iconName,
										const std::string &iconFileName, const std::string hotKey, 
										const std::string &helpString, callbackMethod cbMethod);
		void				addTool(int ID1, const std::string &toolBarItem, const std::string &iconName,
										const std::string &iconFileName, const std::string hotKey, 
										const std::string &helpString, const std::string &func);
		void				deleteTool(const std::string &toolBarItem);
		void				execute(int ID1);
		int				direction() const {return _dockDirection;};
		std::string		name() const {return _name;};
		void				changeToolSize(IconSizes size);
	private:
		void				attachToAUI(void);
		//Check tool and delete if exist
		void				clearTool(const std::string &iconName);
		std::string					_name;
		int							_ID;
		TpdToolBar*					_toolBar;
		toolList						_tools;
		int							_dockDirection;
		bool							_floating;
		wxPoint						_coord;
		IconSizes					_currentSize;
	};

	bool checkToolSize(IconSizes size);

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
		void setDirection(int direction);
		void defineToolBar(const std::string &toolBarName);
		void appendTool(const std::string &toolBarName, const std::string &toolBarItem,
							const std::string &iconName,
							const std::string &hotKey, 
							const std::string &helpString,
							callbackMethod cbMethod);
		void appendTool(const std::string &toolBarName, const std::string &toolBarItem,
							const std::string &iconName,
							const std::string &hotKey, 
							const std::string &helpString,
							std::string func);
		void deleteTool(const std::string &toolBarName, const std::string &toolBarItem);
		/*Don’t call setToolBarSize immediately!!! 
		It leads to nonsynchronized internal state of object and Setting Menu.
		Better to use toolbarsize TELL-function.*/
		void setToolBarSize(bool direction, IconSizes size);
   private:
      //produce lowercase string and exclude unwanted character
      std::string simplify(std::string str, char ch);
		//Function to avoid copy-paste in appendTool functions
		ToolBarHandler* proceedTool(const std::string &toolBarName, const std::string &toolBarItem);

      itemList				_menus;
		toolBarList			_toolBars;
      int					_menuCount; //number of menu items
		int					_toolCount; //number of tool items
		std::string			_IconDir;	//directory that contains
		int					_direction;
   };



}

namespace tellstdfunc {
	class StringMapClientData: public wxClientData
	{
	public:
		StringMapClientData():_key(),_value() {};
		StringMapClientData(const std::string &key, const std::string &value):
			_key(key), _value(value) {};
		void SetData(const std::string &key, const std::string &value) {_key = key; _value = value;};
		const std::string GetKey() const {return _key;};
		const std::string GetValue() const {return _value;};
	private:
		std::string _key;
		std::string _value;
	};

   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;
   
   TELL_STDCMD_CLASSA(stdADDMENU       );  //
	TELL_STDCMD_CLASSA(stdTOOLBARSIZE       );  //
	TELL_STDCMD_CLASSA(stdDEFINETOOLBAR  );  //
	TELL_STDCMD_CLASSA(stdTOOLBARADDITEM  );  //
	TELL_STDCMD_CLASSB(stdTOOLBARADDITEM_S     , stdTOOLBARADDITEM     );  //
	TELL_STDCMD_CLASSA(stdTOOLBARDELETEITEM  );  //
}

#endif //RESOURCE_CENTER_INCLUDED
