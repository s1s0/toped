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
#ifndef RESOURCE_CENTER_INCLUDED
#define RESOURCE_CENTER_INCLUDED

#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/laywin.h>
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include "tpdf_common.h"
#include "tui.h"

/*WARNING!!!
   Current version ResourceCenter can use callback functions only
   for TopedFrame class. Using all other classes include TopedFrame children
   is VERY dangerous.
   In future replace "callbackMethod" type to boost::function, winni::closure
   or similar library
*/

// This below is how the main stream IDs are defined
//#define tpdART_NEW_BOX         wxART_MAKE_ART_ID(tpdART_NEW_BOX)
// Prefer to avoid using preprocessor where possible - so
const wxArtID tpdART_NEW_BOX         = wxT("tpdART_NEW_BOX")     ;
const wxArtID tpdART_NEW_POLY        = wxT("tpdART_NEW_POLY")    ;
const wxArtID tpdART_NEW_WIRE        = wxT("tpdART_NEW_WIRE")    ;
const wxArtID tpdART_NEW_TEXT        = wxT("tpdART_NEW_TEXT")    ;
const wxArtID tpdART_COPY            = wxT("tpdART_COPY")        ;
const wxArtID tpdART_MOVE            = wxT("tpdART_MOVE")        ;
const wxArtID tpdART_SELECT          = wxT("tpdART_SELECT")      ;
const wxArtID tpdART_UNSELECT        = wxT("tpdART_UNSELECT")    ;
const wxArtID tpdART_DELETE          = wxT("tpdART_DELETE")      ;
const wxArtID tpdART_FLIP_HORI       = wxT("tpdART_FLIP_HORI")   ;
const wxArtID tpdART_FLIP_VERT       = wxT("tpdART_FLIP_VERT")   ;
const wxArtID tpdART_ROTATE_CCW      = wxT("tpdART_ROTATE_CCW")  ;
const wxArtID tpdART_ROTATE_CW       = wxT("tpdART_ROTATE_CW")   ;
const wxArtID tpdART_GROUP           = wxT("tpdART_GROUP")       ;
const wxArtID tpdART_UNGROUP         = wxT("tpdART_UNGROUP")     ;
const wxArtID tpdART_CUT_BOX         = wxT("tpdART_CUT_BOX")     ;
const wxArtID tpdART_MERGE           = wxT("tpdART_MERGE")       ;
const wxArtID tpdART_RULER           = wxT("tpdART_RULER")       ;
const wxArtID tpdART_UNDO            = wxT("tpdART_UNDO")        ;
const wxArtID tpdART_ZOOM_IN         = wxT("tpdART_ZOOM_IN")     ;
const wxArtID tpdART_ZOOM_OUT        = wxT("tpdART_ZOOM_OUT")    ;
const wxArtID tpdART_ZOOM_ALL        = wxT("tpdART_ZOOM_ALL")    ;
//const wxArtID tpdART_NEW
//const wxArtID tpdART_OPEN
//const wxArtID tpdART_SAVE
//const wxArtID tpdART_EDIT_POP
//const wxArtID tpdART_EDIT_PUSH
//const wxArtID tpdART_REDO

// Names of all AUI panes. Required for restoring of the frame state -
// (Tell function loaduiframe).
const wxString tpdPane_TB_A          = wxT("_AUIP_TB_A")           ;// Tool Box A
const wxString tpdPane_TB_B          = wxT("_AUIP_TB_B")           ;// Tool Box B
const wxString tpdPane_TB_C          = wxT("_AUIP_TB_C")           ;// Tool Box C
const wxString tpdPane_Status        = wxT("_AUIP_TB_STATUS")      ;// Tool Box Status
const wxString tpdPane_Browsers      = wxT("_AUIP_BROWSERS")       ;// Browsers
const wxString tpdPane_Canvas        = wxT("_AUIP_CANVAS")         ;// Canvas
const wxString tpdPane_Log           = wxT("_AUIP_LOG")            ;// Log
const wxString tpdPane_CmdLine       = wxT("_AUIP_CMDLINE")        ;// Command Line


namespace tui
{
   const word _tuihorizontal    = 0x0000;
   const word _tuivertical      = 0x0001;

   class TopedFrame;
   typedef void (TopedFrame::*callbackMethod)(wxCommandEvent&);

   //=================================
   //      Everything about menu
   //=================================
   class MenuItemHandler {
   public:
                     MenuItemHandler(void);
                     MenuItemHandler(int ID, std::string menuItem, std::string hotKey, std::string function);
                     MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod);
                     MenuItemHandler(int ID, std::string menuItem, std::string hotKey, callbackMethod cbMethod, std::string helpString);
      virtual       ~MenuItemHandler() {};
      std::string    menuItem(void) const    { return _menuItem;}
      std::string    hotKey(void)   const    { return _hotKey;}
      std::string    function(void) const    { return _function;}
      std::string    helpString(void) const  { return _helpString;}
      callbackMethod method(void)   const    { return _method;}
      void           changeInterior(std::string hotKey, std::string function="", callbackMethod cbMethod=NULL, std::string helpString="");
      bool           inserted(void) const    { return _inserted; }
      bool           changed(void)  const    { return _changed; }
      int            ID(void)       const    { return _ID;}

      void           setInserted(bool ins)   {_inserted = ins;};
      void           setChanged(bool  chang) {_changed = chang;};

      virtual void   create(wxMenuBar *menuBar);//Create menu item into this wxMenuBar
      virtual void   recreate(wxMenuBar *menuBar);//Change menu item into this wxMenuBar used for already existed items
    protected:
      wxMenu*        buildPath(wxMenuBar *menuBar, const std::vector<std::string> &menuNames);

      int            _ID         ;
      std::string    _menuItem   ;
      std::string    _hotKey     ;
      std::string    _function   ;
      std::string    _helpString ;
      bool           _inserted   ;
      bool           _changed    ;
      callbackMethod _method     ;
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

      virtual       ~MenuItem() {};
   };

   class MenuItemSeparator:public MenuItemHandler
   {
   public:
                     MenuItemSeparator(void);
                     MenuItemSeparator(std::string menuItem);
      virtual       ~MenuItemSeparator() {};

      virtual void   create(wxMenuBar *menuBar);
   };


//   //=================================
//   //      Everything about toolbar
//   //=================================
//   class ToolItem {
//   public:
//                     ToolItem(int toolID, const wxString& name,
//                              const wxString& bitmapName,
//                              const wxString& hotKey,
//                              const wxString& helpString, callbackMethod cbMethod);
//                     ToolItem(int toolID, const wxString& name,
//                              const wxString& bitmapName,
//                              const wxString& hotKey,
//                              const wxString& helpString,
//                              const wxString  func);
//      virtual      ~ToolItem() {}
//
////      void addIcon(const std::string &bitmapName, int size);
//      void           changeToolSize(IconSizes size);
//      int            ID(void)const                { return _ID;                     }
//      wxString       name(void)const              { return _name;                   }
//      wxBitmap       bitmap(void)const            { return _bitmaps[_currentSize];  }
//      wxString       function(void)const          { return _function;               }
//      wxString       helpString(void)const        { return _helpString;             }
//      bool           isOk(void)const              { return _ok;                     }
//      callbackMethod method(void)const            { return _method;                 }
//      //std::string  hotKey(void)      const      { return _hotKey;};
//   private:
//      void           init(const wxString& bitmapName);
//      int            _ID;
//      wxString       _name;
//      wxBitmap       _bitmaps[ICON_SIZE_END];
//      wxString       _bitmapNames[ICON_SIZE_END];
//      wxString       _function;
//      wxString       _helpString;
//      bool           _ok;
//      callbackMethod _method;
//      IconSizes      _currentSize;
//      //std::string   _hotKey;
//   };
//
//   class TpdToolBar:public wxToolBar {
//   public:
//      TpdToolBar(int ID, long style, IconSizes iconSize);
////   private:
//      //DECLARE_EVENT_TABLE();
//   };
//
//   class ToolBarHandler {
//   public:
//                      ToolBarHandler(int ID, const wxString& name, int direction);
//      virtual        ~ToolBarHandler();
//
//      void            addTool(int ID1, const wxString &toolBarItem, const wxString &iconName,
//                              const wxString &iconFileName, const wxString hotKey,
//                              const wxString &helpString, callbackMethod cbMethod);
//      void            addTool(int ID1, const wxString &toolBarItem, const wxString &iconName,
//                              const wxString &iconFileName, const wxString hotKey,
//                              const wxString &helpString, const wxString &func);
//      void            deleteTool(const wxString& toolBarItem);
//      void            execute(int ID1);
//      void            changeToolSize(IconSizes size);
//      wxString        name() const        {return _name;}
//      int             direction() const   {return _dockDirection;}
//   private:
//      typedef std::vector <ToolItem*>        ToolList;
//      void            attachToAUI(void);
//      void            clearTool(const wxString& iconName);//!Check tool and delete if exist
//      int             _ID           ;
//      wxString        _name         ;
//      TpdToolBar*     _toolBar      ;
//      ToolList        _tools        ;
//      int             _dockDirection;
//      bool            _floating     ;
//      wxPoint         _coord        ;
//      IconSizes       _currentSize  ;
//   };

   bool checkToolSize(IconSizes size);

   //=============================================================================

   class TpdExecResource {
   public:
                        TpdExecResource() {}
      virtual          ~TpdExecResource() {}
      virtual void      exec() {assert(false);}  //Can't be pure virtual, because is used as a type below
   };

   typedef std::map<int, TpdExecResource*> TpdExecResourceMap;

   class TpdResCallBack : public TpdExecResource {
   public:
                        TpdResCallBack(callbackMethod cbMethod);
      virtual          ~TpdResCallBack() {}
      virtual void      exec();
   private:
      callbackMethod    _cbMethod;
   };

   class TpdResTellScript : public TpdExecResource {
   public:
                        TpdResTellScript(const wxString&);
      virtual          ~TpdResTellScript() {}
   private:
      wxString          _tellScript;
   };

   class TpdToolBarWrap : public wxAuiToolBar {
   public:
                        TpdToolBarWrap(wxWindow*, const wxSize&, const wxString&, TpdExecResourceMap*);
      void              addToolItem(int ID, const wxString&, const wxArtID&, callbackMethod);
      void              changeIconSize(const wxSize&);
   private:
      std::list<int>    _itemIdList;
      wxSize            _iconSize;
      TpdExecResourceMap* _execResourceRef;// Don't delete! It's just a reference here!
   };
   typedef std::list<TpdToolBarWrap*> TpdToolBarList;
   class TpdArtProvider : public wxArtProvider {
      public:
                       TpdArtProvider(const wxString&);
   protected:
      virtual wxBitmap CreateBitmap(const wxArtID&, const wxArtClient&, const wxSize&);
   private:
      wxFileName       _rootDir[ICON_SIZE_END];//! icon root directories
   };

   //=============================================================================
   //      Resourcecenter is responsible
   //      for handling all of ui actions.
   //      Currently only menu and tool bars
   //=============================================================================
   class ResourceCenter {
   public:
      ResourceCenter(void);
      ~ResourceCenter(void);
//      void setIconDir(const wxString& dir) {_IconDir = dir;}
      //Using for build of complete menu
      void buildMenu(wxMenuBar *menuBar);
      //Insert new menu item
      //!IMPORTANT after appending call buildMenu()
      void appendMenu(const std::string &menuItem, const std::string &hotKey, const std::string &function);
      void appendMenu(const std::string &menuItem, const std::string &hotKey, callbackMethod cbMethod);
      void appendMenu(const std::string &menuItem, const std::string &hotKey, callbackMethod cbMethod, const std::string &helpString);
      void appendMenuSeparator(const std::string &menuItem);
      void executeMenu(int ID);
      void executeToolBar(int ID);
      bool checkExistence(const tui::MenuItemHandler & item);

      /*void appendTool(const std::string toolBarName, const std::string &toolBarItem,
                     const std::string &bitmapFileName,
                     const std::string &hotKey, callbackMethod cbMethod, int direction);*/
//      void setDirection(int direction);
//      void defineToolBar(const wxString &toolBarName);
//      void appendTool(const wxString &toolBarName, const wxString &toolBarItem,
//                     const wxString &iconName,
//                     const wxString &hotKey,
//                     const wxString &helpString,
//                     callbackMethod cbMethod);
//      void appendTool(const wxString &toolBarName, const wxString &toolBarItem,
//                     const wxString &iconName,
//                     const wxString &hotKey,
//                     const wxString &helpString,
//                     wxString func);
//      void deleteTool(const wxString& toolBarName, const wxString& toolBarItem);
      /*Don't call setToolBarSize immediately!!!
      It leads to nonsynchronized internal state of object and Setting Menu.
      Better to use toolbarsize TELL-function.*/
      void setToolBarSize(IconSizes size);
      wxAuiToolBar* initToolBarA(wxWindow*);
      wxAuiToolBar* initToolBarB(wxWindow*);
      wxAuiToolBar* initToolBarC(wxWindow*);
      wxAuiToolBar* initToolBar(wxWindow*, const wxString&);

   private:
      typedef std::vector <MenuItemHandler*> ItemList;
//      typedef std::vector <ToolBarHandler*>  ToolBarList;

      //produce lowercase string and exclude unwanted character
      std::string simplify(std::string str, char ch);
      //Function to avoid copy-paste in appendTool functions
//      ToolBarHandler* secureToolBar(const wxString &toolBarName);

      ItemList          _menus      ;
//      ToolBarList       _toolBars   ;
      int               _menuCount  ; //number of menu items
//      int               _toolCount  ; //number of tool items // TODO clean-up
      int               _nextToolId ; // The ID of the next Tool
//      wxString          _IconDir    ; //directory that contains
//      int               _direction  ;
      wxSize            _curTBSize  ;

      TpdExecResourceMap _execResources;
      TpdToolBarList    _allToolBars;
//      void              addToolBarItem(wxAuiToolBar*, const wxString&, const wxArtID&, callbackMethod);

   };

}
//=============================================================================

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
   using parsercmd::ArgumentLIST;
   using parsercmd::ArgumentTYPE;

   TELL_STDCMD_CLASSA(stdADDMENU            );  //
   TELL_STDCMD_CLASSA(stdTOOLBARSIZE        );  //
   TELL_STDCMD_CLASSA(stdDEFINETOOLBAR      );  //
   TELL_STDCMD_CLASSA(stdTOOLBARADDITEM     );  //
   TELL_STDCMD_CLASSB(stdTOOLBARADDITEM_S     , stdTOOLBARADDITEM     );  //
   TELL_STDCMD_CLASSA(stdTOOLBARDELETEITEM  );  //
   TELL_STDCMD_CLASSA(stdUIFRAME_LOAD       );
}

#endif //RESOURCE_CENTER_INCLUDED
