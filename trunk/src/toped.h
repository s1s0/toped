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
#include "ted_prompt.h"
#include "layoutcanvas.h"
#include "tui.h"
#include "resourcecenter.h"
#include "browsers.h"


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
   private:
      wxStaticText*        X_pos;
      wxStaticText*        Y_pos;
      wxStaticText*        _dX;
      wxStaticText*        _dY;
      wxStaticText*        _selected;
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
      void                    OnQuit (wxCommandEvent& WXUNUSED( event ));
      void                    OnCheckHW(wxCommandEvent&);
      void                    OnAbout(wxCommandEvent& WXUNUSED( event ));
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
      void                    setOglThread(bool val) {_canvas->setOglThread(val);}
   private:
      void                    initMenuBar();
      void                    initView();
      bool                    checkFileOverwriting(const wxString& fileName);
      void                    USMap2wxString(USMap* inmap, wxString& outmap);
      void                    SIMap2wxString(SIMap* inmap, wxString& outmap);
      console::ted_log*       _cmdlog;       // log window
      console::TELLFuncList*  _cmdbrowser;
      console::ted_cmd*       _cmdline;      // tell command input window
      //LayoutCanvas*           _laycanvas;
      CanvasStatus*           _GLstatus;
      browsers::browserTAB*   _browsers;  // TDT/GDS/layer browsers
      ResourceCenter*         _resourceCenter;
      LayoutCanvas*           _canvas;
      wxToolBar*              _status;
      TpdPost*                _tPost;
      tui::TopedPropertySheets* _propDialog;
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
      wxMenu*                 toolbarVertSizeMenu;
      wxMenu*                 toolbarHorSizeMenu;
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
      void     OnGDSRead(wxCommandEvent& WXUNUSED(event));
      void   OnGDSimport(wxCommandEvent& WXUNUSED(event));
      void   OnGDStranslate(wxCommandEvent& WXUNUSED(event));
      void   OnGDSexportLIB(wxCommandEvent& WXUNUSED(event));
      void   OnGDSexportCELL(wxCommandEvent& WXUNUSED(event));
      void    OnGDSclose(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("gdsclose();"));}

      void     OnCIFexportLIB(wxCommandEvent& WXUNUSED(event));
      void     OnCIFimport(wxCommandEvent& WXUNUSED(event));
      void     OnCIFRead(wxCommandEvent& WXUNUSED(event));
      void   OnCIFtranslate(wxCommandEvent& WXUNUSED(event));
      void   OnCIFexportCELL(wxCommandEvent& WXUNUSED(event));
      void    OnCIFclose(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("cifclose();"));}

      void     OnOASRead(wxCommandEvent& WXUNUSED(event));
      void   OnOASimport(wxCommandEvent& WXUNUSED(event));
      void   OnOAStranslate(wxCommandEvent& WXUNUSED(event));
//      void   OnOASexportLIB(wxCommandEvent& WXUNUSED(event));
//      void   OnOASexportCELL(wxCommandEvent& WXUNUSED(event));
      void    OnOASclose(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("oasisclose();"));}

      void     OnTDTSave(wxCommandEvent& WXUNUSED(event));
      void   OnTDTSaveAs(wxCommandEvent& WXUNUSED(event));
      void   OnPropSave(wxCommandEvent& WXUNUSED(event));
      void OnTDTSnapshot(wxCommandEvent& WXUNUSED(event));
      void        OnCopy(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("copy();"));}
      void        OnMove(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("move();"));}
      void      OnDelete(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("delete();"));}
      void      OnRotate(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("rotate(90);"));}
      void    OnFlipVert(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("flip(_vertical);"));}
      void     OnFlipHor(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("flip(_horizontal);"));}
      void     OnPolyCut(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("polycut();"));}
      void      OnBoxCut(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("boxcut();"));}
      void       OnMerge(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("merge();"));}
      void      OnResize(wxCommandEvent& WXUNUSED(event));
      void  OnChangeText(wxCommandEvent& WXUNUSED(event));

      void     OnCellNew(wxCommandEvent&);
      void    OnCellOpen(wxCommandEvent& WXUNUSED(event));
      void    OnCellRemove(wxCommandEvent&);
      void    OnCellPush(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("editpush(getpoint());"));}
      void     OnCellPop(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("editpop();"));}
      void     OnCellTop(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("edittop();"));}
      void    OnCellPrev(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("editprev();"));}
      void   OnCellRef_B(wxCommandEvent& WXUNUSED(event));
      void   OnCellRef_M(wxCommandEvent& WXUNUSED(event));
      void  OnCellARef_B(wxCommandEvent& WXUNUSED(event));
      void  OnCellARef_M(wxCommandEvent& WXUNUSED(event));
      void   OnCellGroup(wxCommandEvent& WXUNUSED(event));
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
      void OnZoomVisible(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("zoomvisible();"));}
      void     OnDrawBox(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("addbox();"));}
      void    OnDrawPoly(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("addpoly();"));}
      void    OnDrawWire(wxCommandEvent& WXUNUSED(event));
      void    OnDrawText(wxCommandEvent& WXUNUSED(event));
      void    OnSelectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("select();"));}
      void  OnUnselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("unselect();"));}
      void   OnPselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("pselect();"));}
      void OnPunselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("punselect();"));}
      void OnUnselectAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("unselect_all();"));}
      void OnReportSelected(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("report_selected();"));}
      void   OnSelectAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("select_all();"));}
      //
      void        OnStep(wxCommandEvent& WXUNUSED(event));
      void     OnAutopan(wxCommandEvent& WXUNUSED(event));
      void OnPropertySheet(wxCommandEvent& WXUNUSED(event));
      void   OnZeroCross(wxCommandEvent& WXUNUSED(event));
      void       OnGrid0(wxCommandEvent& WXUNUSED(event));
      void       OnGrid1(wxCommandEvent& WXUNUSED(event));
      void       OnGrid2(wxCommandEvent& WXUNUSED(event));
      void  OnLongCursor(wxCommandEvent& WXUNUSED(event));
      void     OnMarker0(wxCommandEvent& WXUNUSED(event));
      void    OnMarker45(wxCommandEvent& WXUNUSED(event));
      void    OnMarker90(wxCommandEvent& WXUNUSED(event));
      void OnHToolBarSize16(wxCommandEvent& WXUNUSED(event));
      void OnHToolBarSize24(wxCommandEvent& WXUNUSED(event));
      void OnHToolBarSize32(wxCommandEvent& WXUNUSED(event));
      void OnHToolBarSize48(wxCommandEvent& WXUNUSED(event));
      void   OnUndoDepth(wxCommandEvent& WXUNUSED(event));
      void OnVToolBarSize16(wxCommandEvent& WXUNUSED(event));
      void OnVToolBarSize24(wxCommandEvent& WXUNUSED(event));
      void OnVToolBarSize32(wxCommandEvent& WXUNUSED(event));
      void OnVToolBarSize48(wxCommandEvent& WXUNUSED(event));


      void OnEditLayer(wxCommandEvent&);
      void OnDefineLayer(wxCommandEvent&);
      void OnDefineColor(wxCommandEvent& WXUNUSED(event));
      void  OnDefineFill(wxCommandEvent& WXUNUSED(event));
      void    OnMenu(wxCommandEvent&);

      void  OnGridDefine(wxCommandEvent& WXUNUSED(event));
      void  OnAddRuler(wxCommandEvent& WXUNUSED(event))    {_cmdline->parseCommand(wxT("addruler();") );}
      void  OnClearRulers(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand(wxT("clearrulers();") );}
      void  OnCadenceConvert(wxCommandEvent& WXUNUSED(event));
      void  OnTextLogOverflow(wxCommandEvent& WXUNUSED(event));
      void  OnChangeLayer(wxCommandEvent& WXUNUSED(event));
      void  OnCurrentLayer(wxCommandEvent& WXUNUSED(event));
      //
      void  OnUpdateSettingsMenu(wxCommandEvent&);
      void          OnMouseAccel(wxCommandEvent&);
      void        OnCanvasStatus(wxCommandEvent&);
      // additional
      void    CellRef(wxString);
      void   CellARef(wxString);
      // The declaration of the associated event table
      void OnUncapturedMouseClick(wxCommandEvent&);
      void          OnToolBarSize(wxCommandEvent&);
      void        OnToolBarDefine (wxCommandEvent& evt);
      void       OnToolBarAddItem (wxCommandEvent& evt);
      void     OnToolBarDeleteItem(wxCommandEvent& evt);
      void     editLayerDlg(word layno, const layprop::DrawProperties*);
      void     OnDRCResults(wxCommandEvent& evt);
      DECLARE_EVENT_TABLE();
   };

}
#endif

