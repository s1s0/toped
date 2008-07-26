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
#include <wx/glcanvas.h>
#include <wx/laywin.h>
#include <wx/aui/aui.h>
#include "../tpd_parser/ted_prompt.h"
#include "layoutcanvas.h"
#include "tui.h"
#include "resourcecenter.h"
#include "../tpd_DB/browsers.h"

namespace tui {
   //-----------------------------------------------------------------------------
   class CanvasStatus : public wxPanel {
   public:
                           CanvasStatus(wxWindow*,wxWindowID id = wxID_ANY,
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = wxTAB_TRAVERSAL);
      virtual             ~CanvasStatus();
      void                 setXpos(wxString);
      void                 setYpos(wxString);
      void                 setdXpos(wxString);
      void                 setdYpos(wxString);
      void                 setSelected(wxString);
      void                 btn_abort_enable(bool state) {_abort->Enable(state);};
   private:
      wxStaticText*        X_pos;
      wxStaticText*        Y_pos;
      wxStaticText*        _dX;
      wxStaticText*        _dY;
      wxStaticText*        _selected;
      wxButton*            _abort;
   };
   //-----------------------------------------------------------------------------
   class TopedStatus : public wxStatusBar
   {
   public:
                           TopedStatus(wxWindow*);
      virtual             ~TopedStatus(){};

      // event handlers
      void OnThreadON(wxString);
      void OnThreadWait();
      void OnThreadOFF();
      void OnSize(wxSizeEvent& event);

   private:
      wxStaticBitmap*      _lamp;
      DECLARE_EVENT_TABLE();
   };

   //-----------------------------------------------------------------------------
   class TopedFrame : public wxFrame {
   public:
                              TopedFrame(const wxString& , const wxPoint& , 
                                                               const wxSize&);
      void                    OnClose(wxCloseEvent&);
      virtual                ~TopedFrame();
//      void                    OnSize(wxSizeEvent& event);
      void                    OnSashDrag(wxSashEvent& event);
      void                    OnQuit (wxCommandEvent&);
      void                    OnAbout(wxCommandEvent&);
//      wxMenuBar*              getMenuBar(void) {return GetMenuBar();}
      ResourceCenter*         resourceCenter(void) {return _resourceCenter;}
      console::ted_log*       logwin()   const {return _cmdlog;}
      LayoutCanvas*           view()     const {return _canvas;}
      console::ted_cmd*       cmdline()  const {return _cmdline;}
      browsers::browserTAB*   browsers() const {return _browsers;}
      console::TELLFuncList*  cmdlist()  const {return _cmdbrowser;}
      wxWindow*               getFrame()       {return this;}
      wxAuiManager*           getAuiManager()  {return &_winManager;}
      void                    initToolBars();
      void                    setIconDir(const std::string& uiDir);
   private:
      void                    initMenuBar();
      void                    initView();
      bool                    checkFileOverwriting(const wxString& fileName);
      console::ted_log*       _cmdlog;       // log window
      console::TELLFuncList*  _cmdbrowser;
      console::ted_cmd*       _cmdline;      // tell command input window
      //LayoutCanvas*           _laycanvas;
      CanvasStatus*           _GLstatus;
      TopedStatus*            _toped_status;
      browsers::browserTAB*   _browsers;  // TDT/GDS/layer browsers
      ResourceCenter*         _resourceCenter;
      LayoutCanvas*           _canvas;
      wxToolBar*              _status;
      wxAuiManager            _winManager;
      //Menu stuff
      wxMenuBar*              menuBar;
      wxMenu*                 fileMenu;
      wxMenu*                 editMenu;
      wxMenu*                 viewMenu;
      wxMenu*                 cellMenu;
      wxMenu*                 drawMenu;
      wxMenu*                 selectMenu;
      wxMenu*                 settingsMenu;
      wxMenu*                 markerMenu;
      wxMenu*                 gdsMenu;
      wxMenu*                 helpMenu;

      // Sash layout stuff
      //wxControl*    mS_browsers;
//      wxWindow*     mS_GLstatus;
//      wxWindow*     mS_command;
//      wxWindow*     mS_log;
      // Menu
      void   OnNewDesign(wxCommandEvent&);
      void     OnTDTRead(wxCommandEvent&);
      void  OnTDTLoadLib(wxCommandEvent&);
      void OnTDTUnloadLib(wxCommandEvent&);
      void    OnTELLRead(wxCommandEvent&);
      void     OnGDSRead(wxCommandEvent&);
      void   OnGDSimport(wxCommandEvent&);
      void   OnGDStranslate(wxCommandEvent&);
      void   OnGDSexportLIB(wxCommandEvent&);
      void   OnGDSexportCELL(wxCommandEvent&);
      void    OnGDSclose(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("gdsclose();"));}
      
      void     OnCIFRead(wxCommandEvent&);
      void   OnCIFtranslate(wxCommandEvent&);
      void    OnCIFclose(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("cifclose();"));}
      void     OnTDTSave(wxCommandEvent&);
      void   OnTDTSaveAs(wxCommandEvent&);
      void   OnPropSave(wxCommandEvent&);
      void OnTDTSnapshot(wxCommandEvent&);
      void        OnCopy(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("copy();"));}
      void        OnMove(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("move();"));}
      void      OnDelete(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("delete();"));}
      void      OnRotate(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("rotate(90);"));}
      void       OnFlipX(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("flipX();"));}
      void       OnFlipY(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("flipY();"));}
      void     OnPolyCut(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("polycut();"));}
      void      OnBoxCut(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("boxcut();"));}
      void       OnMerge(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("merge();"));}
      void      OnResize(wxCommandEvent& WXUNUSED(event));
      void  OnChangeText(wxCommandEvent& WXUNUSED(event));

      void     OnCellNew(wxCommandEvent&);
      void    OnCellOpen(wxCommandEvent&);
      void    OnCellRemove(wxCommandEvent&);
      void    OnCellPush(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("editpush(getpoint());"));}
      void     OnCellPop(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("editpop();"));}
      void     OnCellTop(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("edittop();"));}
      void    OnCellPrev(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("editprev();"));}
      void   OnCellRef_B(wxCommandEvent&);
      void   OnCellRef_M(wxCommandEvent&);
      void  OnCellARef_B(wxCommandEvent&);
      void   OnCellARef_M(wxCommandEvent&);
      void   OnCellGroup(wxCommandEvent&);
      void OnCellUngroup(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("ungroup();"));}
      void   OnChangeRef(wxCommandEvent& WXUNUSED(event));

      void        OnUndo(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("undo();"));}
      void      OnzoomIn(wxCommandEvent& WXUNUSED(event));
      void     OnzoomOut(wxCommandEvent& WXUNUSED(event));
      void   OnzoomEmpty(wxCommandEvent& WXUNUSED(event));
      void     OnpanLeft(wxCommandEvent& WXUNUSED(event));
      void    OnpanRight(wxCommandEvent& WXUNUSED(event));
      void       OnpanUp(wxCommandEvent& WXUNUSED(event));
      void     OnpanDown(wxCommandEvent& WXUNUSED(event));
      void     OnZoomAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("zoomall();"));}
      void     OnDrawBox(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("addbox();"));}
      void    OnDrawPoly(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("addpoly();"));}
      void    OnDrawWire(wxCommandEvent&);
      void    OnDrawText(wxCommandEvent&);
      void    OnSelectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("select();"));}
      void  OnUnselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("unselect();"));}
      void   OnPselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("pselect();"));}
      void OnPunselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("punselect();"));}
      void OnUnselectAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("unselect_all();"));}
      void OnReportSelected(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("report_selected();"));}
      void   OnSelectAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("select_all();"));}
      //
      void        OnStep(wxCommandEvent&);
      void     OnAutopan(wxCommandEvent&);
      void       OnGrid0(wxCommandEvent&);
      void       OnGrid1(wxCommandEvent&);
      void       OnGrid2(wxCommandEvent&);
      void  OnLongCursor(wxCommandEvent&);
      void    OnCellMark(wxCommandEvent&);
      void    OnCellBox(wxCommandEvent&);
      void    OnTextMark(wxCommandEvent&);
      void    OnTextBox(wxCommandEvent&);
      void     OnMarker0(wxCommandEvent&);
      void    OnMarker45(wxCommandEvent&);
      void    OnMarker90(wxCommandEvent&);
      void OnDefineLayer(wxCommandEvent&);
      void OnDefineColor(wxCommandEvent&);
      void  OnDefineFill(wxCommandEvent&);
      void    OnMenu(wxCommandEvent&);

      void  OnGridDefine(wxCommandEvent&);
      void  OnAddRuler(wxCommandEvent& WXUNUSED(event))    {_cmdline->parseCommand(wxT("addruler();") );}
      void  OnClearRulers(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("clearrulers();") );}
      void  OnChangeLayer(wxCommandEvent& WXUNUSED(event));
      void  OnCurrentLayer(wxCommandEvent& WXUNUSED(event));
      //
      void               OnAbort(wxCommandEvent&);
      void  OnUpdateSettingsMenu(wxCommandEvent&);
      void          OnMouseAccel(wxCommandEvent&);
      void        OnCanvasStatus(wxCommandEvent&);
      void         OnTopedStatus(wxCommandEvent&);
      // additional
      void    CellRef(wxString);
      void   CellARef(wxString);
      // The declaration of the associated event table
      void OnUncapturedMouseClick(wxCommandEvent&);
      DECLARE_EVENT_TABLE();
   };

}
#endif

