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
//        Created: Thu May  6 21:56:03 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Main Toped framework
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#if !defined(TOPED_H_INCLUDED)
#define TOPED_H_INCLUDED

#include <wx/wx.h>
#include "layoutcanvas.h"
#include <wx/glcanvas.h>
#include <wx/laywin.h>
#include <wx/aui/aui.h>
#include <wx/process.h>
#include <wx/timer.h>
#include <wx/dynlib.h>
#include "ted_prompt.h"
#include "tui.h"
#include "resourcecenter.h"
#include "browsers.h"
#include "techeditor.h"
#include "trend.h"


namespace tui {
   //-----------------------------------------------------------------------------
   class CanvasStatus : public wxAuiToolBar {
   public:
                           CanvasStatus(wxWindow*,wxWindowID id = wxID_ANY,
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = wxAUI_TB_DEFAULT_STYLE);
      virtual             ~CanvasStatus();
      void                 setXpos(wxString);
      void                 setYpos(wxString);
      void                 setdXpos(wxString);
      void                 setdYpos(wxString);
      void                 setSelected(wxString);
   private:
      wxTextCtrl*          _ctrlXPos;
      wxTextCtrl*          _ctrlYPos;
      wxTextCtrl*          _ctrlDX;
      wxTextCtrl*          _ctrlDY;
      wxTextCtrl*          _ctrlSel;
      wxString             _strX;
      wxString             _strY;
      wxString             _strDX;
      wxString             _strDY;
      wxString             _strSel;
   };

   class ExternalProcess : public wxProcess {
      public:
                              ExternalProcess(wxEvtHandler* parent);
         virtual void         OnTerminate(int pid, int status);
         void                 OnTimer(wxTimerEvent& WXUNUSED(event));
//         void                 OnTextEnter(wxCommandEvent& event);
      private:
         wxTimer              _idleTimer;
         wxTextInputStream*   _tes;
         wxTextInputStream*   _tis;
         DECLARE_EVENT_TABLE();
   };
   //-----------------------------------------------------------------------------
   class TopedFrame : public wxFrame {
   public:
                              TopedFrame(const wxString& , const wxPoint& ,
                                                               const wxSize&);
      virtual                ~TopedFrame();
//      void                    OnSize(wxSizeEvent& event);
      void                    OnSashDrag(wxSashEvent& event);
      void                    OnExecExt (wxCommandEvent&);
      void                    checkExit (wxCommandEvent& WXUNUSED( event ));
      void                    OnClose(wxCloseEvent& WXUNUSED( event ));
      void                    OnExitRequest (wxCommandEvent& WXUNUSED( event ));
      void                    OnCheckHW(wxCommandEvent&);
      void                    OnAbout(wxCommandEvent& WXUNUSED( event ));
      void                    OnExecExtTextEnter(wxCommandEvent& event);
      void                    OnDefineColor(wxCommandEvent& WXUNUSED(event));
      void                    OnDefineFill(wxCommandEvent& WXUNUSED(event));
      void                    OnDefineStyle(wxCommandEvent& WXUNUSED(event));
      void                    OnDefineLayer(wxCommandEvent&);

//      wxMenuBar*              getMenuBar(void) {return GetMenuBar();}
      ResourceCenter*         resourceCenter(void) {return _resourceCenter;}
      console::ted_log*       logwin()   const {return _cmdlog;}
      LayoutCanvas*           view()     const {return _canvas;}
      console::TedCmdLine*    cmdline()  const {return _cmdline;}
      browsers::browserTAB*   browsers() const {return _browsers;}
      console::TELLFuncList*  cmdlist()  const {return _cmdbrowser;}
      wxFrame*                getFrame()       {return this;}
      wxAuiManager*           getAuiManager()  {return &_winManager;}
      void                    setIconDir(const wxString& uiDir);
      void                    setOglThread(bool val) {_canvas->setOglThread(val);}
      void                    setActiveCmd();
      void                    initToolBars();
   private:
      friend class tui::ResourceCenter;
      void                    initMenuBar();
      void                    initView();
      void                    setExitAproved()              { _exitAproved = true;}
      bool                    exitAproved() const          { return _exitAproved;}
      bool                    checkFileOverwriting(const wxString& fileName);
      void                    USMap2wxString(ExpLayMap* inmap, wxString& outmap);
      void                    SIMap2wxString(ImpLayMap* inmap, wxString& outmap);
      console::ted_log*       _cmdlog;       // log window
      console::TELLFuncList*  _cmdbrowser;
      console::TedCmdLine*    _cmdline;
      bool                    _exitAproved;
      //LayoutCanvas*           _laycanvas;
      CanvasStatus*           _GLstatus;
      browsers::browserTAB*   _browsers;  // TDT/GDS/layer browsers
      ResourceCenter*         _resourceCenter;
      LayoutCanvas*           _canvas;
//      wxToolBar*              _status;
      TpdPost*                _tPost;
      tui::TopedPropertySheets* _propDialog;
      wxAuiManager            _winManager;
      wxDialog*               _techEditor;
      //Menu stuff
      wxMenuBar*              menuBar;
//      wxMenu*                 fileMenu;
//      wxMenu*                 editMenu;
//      wxMenu*                 viewMenu;
//      wxMenu*                 cellMenu;
//      wxMenu*                 drawMenu;
//      wxMenu*                 selectMenu;
      wxMenu*                 settingsMenu;
//      wxMenu*                 markerMenu;
//      wxMenu*                 toolbarVertSizeMenu;
      wxMenu*                 toolbarHorSizeMenu;
//      wxMenu*                 gdsMenu;
//      wxMenu*                 helpMenu;
      ExternalProcess*        _extProc;

      // Sash layout stuff
      //wxControl*    mS_browsers;
//      wxWindow*     mS_GLstatus;
//      wxWindow*     mS_command;
//      wxWindow*     mS_log;
      // Menu
      void            OnNewDesign(wxCommandEvent&);
      void              OnTDTRead(wxCommandEvent&);
      void           OnTDTLoadLib(wxCommandEvent&);
      void         OnTDTUnloadLib(wxCommandEvent&);
      void             OnTELLRead(wxCommandEvent&);
      void              OnGDSRead(wxCommandEvent& WXUNUSED(event));
      void            OnGDSimport(wxCommandEvent& WXUNUSED(event));
      void         OnGDStranslate(wxCommandEvent& WXUNUSED(event));
      void         OnGDSexportLIB(wxCommandEvent& WXUNUSED(event));
      void        OnGDSexportCELL(wxCommandEvent& WXUNUSED(event));
      void             OnGDSclose(wxCommandEvent& WXUNUSED(event));

      void         OnCIFexportLIB(wxCommandEvent& WXUNUSED(event));
      void            OnCIFimport(wxCommandEvent& WXUNUSED(event));
      void              OnCIFRead(wxCommandEvent& WXUNUSED(event));
      void         OnCIFtranslate(wxCommandEvent& WXUNUSED(event));
      void        OnCIFexportCELL(wxCommandEvent& WXUNUSED(event));
      void             OnCIFclose(wxCommandEvent& WXUNUSED(event));

      void              OnOASRead(wxCommandEvent& WXUNUSED(event));
      void            OnOASimport(wxCommandEvent& WXUNUSED(event));
      void         OnOAStranslate(wxCommandEvent& WXUNUSED(event));
//    void         OnOASexportLIB(wxCommandEvent& WXUNUSED(event));
//    void        OnOASexportCELL(wxCommandEvent& WXUNUSED(event));
      void             OnOASclose(wxCommandEvent& WXUNUSED(event));

      void              OnTDTSave(wxCommandEvent& WXUNUSED(event));
      void            OnTDTSaveAs(wxCommandEvent& WXUNUSED(event));
      void             OnPropSave(wxCommandEvent& WXUNUSED(event));
      void          OnTDTSnapshot(wxCommandEvent& WXUNUSED(event));
      void                 OnCopy(wxCommandEvent& WXUNUSED(event));
      void                 OnMove(wxCommandEvent& WXUNUSED(event));
      void               OnDelete(wxCommandEvent& WXUNUSED(event));
      void               OnRotate(wxCommandEvent& WXUNUSED(event));
      void             OnFlipVert(wxCommandEvent& WXUNUSED(event));
      void              OnFlipHor(wxCommandEvent& WXUNUSED(event));
      void              OnPolyCut(wxCommandEvent& WXUNUSED(event));
      void               OnBoxCut(wxCommandEvent& WXUNUSED(event));
      void                OnMerge(wxCommandEvent& WXUNUSED(event));
      void               OnResize(wxCommandEvent& WXUNUSED(event));
      void           OnChangeText(wxCommandEvent& WXUNUSED(event));

      void              OnCellNew(wxCommandEvent&);
      void             OnCellOpen(wxCommandEvent& WXUNUSED(event));
      void           OnCellRemove(wxCommandEvent&);
      void             OnCellPush(wxCommandEvent& WXUNUSED(event));
      void              OnCellPop(wxCommandEvent& WXUNUSED(event));
      void              OnCellTop(wxCommandEvent& WXUNUSED(event));
      void             OnCellPrev(wxCommandEvent& WXUNUSED(event));
      void            OnCellRef_B(wxCommandEvent& WXUNUSED(event));
      void            OnCellRef_M(wxCommandEvent& WXUNUSED(event));
      void           OnCellARef_B(wxCommandEvent& WXUNUSED(event));
      void           OnCellARef_M(wxCommandEvent& WXUNUSED(event));
      void            OnCellGroup(wxCommandEvent& WXUNUSED(event));
      void          OnCellUngroup(wxCommandEvent& WXUNUSED(event));
      void            OnChangeRef(wxCommandEvent& WXUNUSED(event));

      void                 OnUndo(wxCommandEvent& WXUNUSED(event));
      void               OnZoomIn(wxCommandEvent& WXUNUSED(event));
      void              OnZoomOut(wxCommandEvent& WXUNUSED(event));
      void            OnzoomEmpty(wxCommandEvent& WXUNUSED(event));
      void              OnpanLeft(wxCommandEvent& WXUNUSED(event));
      void             OnpanRight(wxCommandEvent& WXUNUSED(event));
      void                OnpanUp(wxCommandEvent& WXUNUSED(event));
      void              OnpanDown(wxCommandEvent& WXUNUSED(event));
      void              OnZoomAll(wxCommandEvent& WXUNUSED(event));
      void          OnZoomVisible(wxCommandEvent& WXUNUSED(event));
      void              OnDrawBox(wxCommandEvent& WXUNUSED(event));
      void             OnDrawPoly(wxCommandEvent& WXUNUSED(event));
      void             OnDrawWire(wxCommandEvent& WXUNUSED(event));
      void             OnDrawText(wxCommandEvent& WXUNUSED(event));
      void             OnSelectIn(wxCommandEvent& WXUNUSED(event));
      void           OnUnselectIn(wxCommandEvent& WXUNUSED(event));
      void            OnPselectIn(wxCommandEvent& WXUNUSED(event));
      void          OnPunselectIn(wxCommandEvent& WXUNUSED(event));
      void          OnUnselectAll(wxCommandEvent& WXUNUSED(event));
      void       OnReportSelected(wxCommandEvent& WXUNUSED(event));
      void            OnSelectAll(wxCommandEvent& WXUNUSED(event));
      //
      void        OnPropertySheet(wxCommandEvent& WXUNUSED(event));
      void       OnHToolBarSize16(wxCommandEvent& WXUNUSED(event));
      void       OnHToolBarSize24(wxCommandEvent& WXUNUSED(event));
      void       OnHToolBarSize32(wxCommandEvent& WXUNUSED(event));
      void       OnHToolBarSize48(wxCommandEvent& WXUNUSED(event));
      void            OnUndoDepth(wxCommandEvent& WXUNUSED(event));
      void       OnVToolBarSize16(wxCommandEvent& WXUNUSED(event));
      void       OnVToolBarSize24(wxCommandEvent& WXUNUSED(event));
      void       OnVToolBarSize32(wxCommandEvent& WXUNUSED(event));
      void       OnVToolBarSize48(wxCommandEvent& WXUNUSED(event));


      void            OnEditLayer(wxCommandEvent&);
      void           OnTechEditor(wxCommandEvent& WXUNUSED(event));
      void                 OnMenu(wxCommandEvent&);
      void              OnToolBar(wxCommandEvent&);

      void           OnGridDefine(wxCommandEvent& WXUNUSED(event));
      void             OnAddRuler(wxCommandEvent& WXUNUSED(event));
      void          OnClearRulers(wxCommandEvent& WXUNUSED(event));
      void       OnCadenceConvert(wxCommandEvent& WXUNUSED(event));
      void      OnTextLogOverflow(wxCommandEvent& WXUNUSED(event));
      void          OnChangeLayer(wxCommandEvent& WXUNUSED(event));
      void         OnCurrentLayer(wxCommandEvent& WXUNUSED(event));
      //
      void   OnUpdateRenderParams(wxCommandEvent&);
      void   OnUpdateCanvasParams(wxCommandEvent&);
      void           OnMouseAccel(wxCommandEvent&);
      void         OnCanvasStatus(wxCommandEvent&);
      // additional
      void CellRef               (wxString);
      void CellARef              (wxString);
      void OnUncapturedMouseClick(wxCommandEvent&);
      void OnToolBarSize         (wxCommandEvent&);
      void OnToolBarDefine       (wxCommandEvent&);
      void OnToolBarAddItem      (wxCommandEvent&);
      void OnToolBarDeleteItem   (wxCommandEvent&);
      void OnDRCResults          (wxCommandEvent& WXUNUSED(evt));
      void OnIconize             (wxIconizeEvent&);
      void onReloadTellFuncs     (wxCommandEvent& WXUNUSED(evt));
      void onParseCommand        (wxCommandEvent& evt);
      // The declaration of the associated event table
      DECLARE_EVENT_TABLE();
   };
}

//=============================================================================
// The top application class. All initialization and exiting code
//=============================================================================
class TopedApp : public wxApp
{
   public:
      virtual bool         OnInit();
      virtual int          OnExit();
      virtual int          OnRun();
              void         reloadInternalFunctions();
           wxString        localDir() {return _localDir;};    
           wxString        globalDir() {return _globalDir;}
      virtual             ~TopedApp(){};
   private:
      typedef std::list<wxDynamicLibrary*> PluginList;
      typedef void (*ModuleFunction)(parsercmd::cmdMAIN*);
      bool                 getLogFileName();
      void                 loadGlfFonts();
      void                 defaultStartupScript();
      void                 loadPlugIns();
      bool                 checkCrashLog();
      void                 getLocalDirs();    //! Get directories in TPD_LOCAL
      void                 getGlobalDirs();   //! Get directories in TPD_GLOBAL
      void                 getTellPathDirs(); //! Check directories in TLL_INCLUDE_PATH
      void                 finishSessionLog();
      void                 saveIgnoredCrashLog();
      bool                 parseCmdLineArgs();
      void                 printLogWHeader();
      void                 initInternalFunctions(parsercmd::cmdMAIN* mblock);
      wxString             _logFileName;
      wxString             _tpdLogDir;
      wxString             _tpdFontDir;
      wxString             _tpdResourceDir;
      wxString             _tpdPlugInDir;
      wxString             _tpdShadersDir;
      wxString             _globalDir;
      wxString             _localDir;
      wxString             _inputTellFile;
      trend::RenderType    _forceRenderType;
      bool                 _noLog;     //! Don't create a log file
      bool                 _gui;       //! Run graphics (as opposed to a command line mode)
      PluginList           _plugins;
      wxPathList           _tllIncludePath;

};

DECLARE_APP(TopedApp)

#endif

