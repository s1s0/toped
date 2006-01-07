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
// ------------------------------------------------------------------------ =
//           $URL$
//  Creation date: Thu May  6 22:04:50 BST 2004
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <wx/sizer.h>
#include <wx/filename.h>
#include <math.h>
#include "toped.h"
#include "datacenter.h"
#include "../tpd_DB/viewprop.h"
#include "tui.h"

extern const wxEventType         wxEVT_MARKERPOSITION;
extern const wxEventType         wxEVT_CNVSSTATUSLINE;
extern const wxEventType         wxEVT_MOUSE_ACCEL;
extern const wxEventType         wxEVT_CANVAS_ZOOM;

extern DataCenter*               DATC;
extern layprop::ViewProperties*  Properties;
extern console::ted_cmd*         Console;

tui::CanvasStatus::CanvasStatus(wxWindow* parent) : wxPanel( parent, -1,
                                          wxDefaultPosition, wxDefaultSize) {
   wxFont fontX = GetFont();
   fontX.SetWeight(wxBOLD);
   SetFont(fontX);
   _abort = new wxButton(this, TBSTAT_ABORT, "Abort");
   _abort->Disable();
   SetBackgroundColour(wxColour("BLACK"));
   SetForegroundColour(wxColour("CYAN"));
   X_pos = new wxStaticText(this, -1, "0.00", wxDefaultPosition, wxSize(80,20), 
                                                   wxST_NO_AUTORESIZE /*| wxALIGN_RIGHT*/);
   Y_pos = new wxStaticText(this, -1, "0.00", wxDefaultPosition, wxSize(80,20), 
                                                   wxST_NO_AUTORESIZE /*| wxALIGN_RIGHT*/);
   _dX = new wxStaticText(this, -1, "", wxDefaultPosition, wxSize(80,20), 
                                                   wxST_NO_AUTORESIZE /*| wxALIGN_RIGHT*/);
   _dY = new wxStaticText(this, -1, "", wxDefaultPosition, wxSize(100,20), 
                                                   wxST_NO_AUTORESIZE/* | wxALIGN_RIGHT*/);
   
   _selected = new wxStaticText(this, -1, "0", wxDefaultPosition, wxSize(40,20), 
                                                   wxST_NO_AUTORESIZE | wxALIGN_LEFT);
   wxBoxSizer *thesizer = new wxBoxSizer( wxHORIZONTAL );
   thesizer->Add(_abort, 0, wxALL | wxALIGN_CENTER, 3);
   thesizer->Add(10,0,1);  
   thesizer->Add(new wxStaticText(this, -1, "Selected: "), 0, wxALIGN_CENTER, 3 );
   thesizer->Add(_selected, 0, wxALIGN_CENTER, 3 );
//   thesizer->Add(10,0,1);  
   thesizer->Add(new wxStaticText(this, -1, "dX: "), 0, wxLEFT | wxRIGHT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(_dX, 0, wxALIGN_CENTER, 3 );
   thesizer->Add(new wxStaticText(this, -1, "dY: "), 0, wxLEFT | wxRIGHT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(_dY, 0, wxALIGN_CENTER, 3 );
//   thesizer->Add(20,0,1);  
   thesizer->Add(new wxStaticText(this, -1, "X: "), 0, wxLEFT | wxRIGHT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(X_pos, 0, wxALIGN_CENTER, 3 );
   thesizer->Add(new wxStaticText(this, -1, "Y: "), 0, wxLEFT | wxRIGHT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(Y_pos, 0, wxALIGN_CENTER, 3 );
   SetSizerAndFit(thesizer);
}

tui::CanvasStatus::~CanvasStatus() {
   delete X_pos;
   delete Y_pos;
   delete _dX;
   delete _dY;
   delete _selected;
   delete _abort;
}

void tui::CanvasStatus::setXpos(wxString coordX){
   X_pos->SetLabel(coordX);
   X_pos->Refresh();
}

void tui::CanvasStatus::setYpos(wxString coordY){
   Y_pos->SetLabel(coordY);
   Y_pos->Refresh();
}

void tui::CanvasStatus::setdXpos(wxString coordX){
   _dX->SetLabel(coordX);
   _dX->Refresh();
}

void tui::CanvasStatus::setdYpos(wxString coordY){
   _dY->SetLabel(coordY);
   _dY->Refresh();
}
   
void tui::CanvasStatus::setSelected(wxString numsel) {
   _selected->SetLabel(numsel);
}
   
// The TopedFrame event table (TOPED main event table)
BEGIN_EVENT_TABLE( tui::TopedFrame, wxFrame )
   EVT_MENU( TMFILE_NEW          , tui::TopedFrame::OnNewDesign   )
   EVT_MENU( TMFILE_OPEN         , tui::TopedFrame::OnTDTRead     )
   EVT_MENU( TMFILE_INCLUDE      , tui::TopedFrame::OnTELLRead    )
   EVT_MENU( TMGDS_OPEN          , tui::TopedFrame::OnGDSRead     )
   EVT_MENU( TMGDS_TRANSLATE     , tui::TopedFrame::OnGDStranslate)
   EVT_MENU( TMGDS_IMPORT       , tui::TopedFrame::OnGDSimport)
   EVT_MENU( TMGDS_EXPORTL       , tui::TopedFrame::OnGDSexportLIB)
   EVT_MENU( TMGDS_EXPORTC       , tui::TopedFrame::OnGDSexportTOP)
   EVT_MENU( TMGDS_CLOSE         , tui::TopedFrame::OnGDSclose    )
   EVT_MENU( TMFILE_SAVE         , tui::TopedFrame::OnTDTSave     )
   EVT_MENU( TMFILE_SAVEAS       , tui::TopedFrame::OnTDTSaveAs   )
   EVT_MENU( TMFILE_EXIT         , tui::TopedFrame::OnQuit        )
   
   EVT_MENU( TMEDIT_UNDO         , tui::TopedFrame::OnUndo        )
   EVT_MENU( TMEDIT_COPY         , tui::TopedFrame::OnCopy        )
   EVT_MENU( TMEDIT_MOVE         , tui::TopedFrame::OnMove        )
   EVT_MENU( TMEDIT_DELETE       , tui::TopedFrame::OnDelete      )
   EVT_MENU( TMEDIT_ROTATE90     , tui::TopedFrame::OnRotate      )
   EVT_MENU( TMEDIT_FLIPX        , tui::TopedFrame::OnFlipX       )
   EVT_MENU( TMEDIT_FLIPY        , tui::TopedFrame::OnFlipY       )
   EVT_MENU( TMEDIT_POLYCUT      , tui::TopedFrame::OnPolyCut     )
   EVT_MENU( TMEDIT_MERGE        , tui::TopedFrame::OnMerge       )
   
   EVT_MENU( TMVIEW_ZOOMIN       , tui::TopedFrame::OnzoomIn      )
   EVT_MENU( TMVIEW_ZOOMOUT      , tui::TopedFrame::OnzoomOut     )
   EVT_MENU( TMVIEW_PANLEFT      , tui::TopedFrame::OnpanLeft     )
   EVT_MENU( TMVIEW_PANRIGHT     , tui::TopedFrame::OnpanRight    )
   EVT_MENU( TMVIEW_PANUP        , tui::TopedFrame::OnpanUp       )
   EVT_MENU( TMVIEW_PANDOWN      , tui::TopedFrame::OnpanDown     )
   EVT_MENU( TMVIEW_ZOOMALL      , tui::TopedFrame::OnZoomAll     )
   
   EVT_MENU( TMCELL_NEW          , tui::TopedFrame::OnCellNew     )
   EVT_MENU( TMCELL_OPEN         , tui::TopedFrame::OnCellOpen    )
   EVT_MENU( TMCELL_PUSH         , tui::TopedFrame::OnCellPush    )
   EVT_MENU( TMCELL_PREV         , tui::TopedFrame::OnCellPrev    )
   EVT_MENU( TMCELL_POP          , tui::TopedFrame::OnCellPop     )
   EVT_MENU( TMCELL_TOP          , tui::TopedFrame::OnCellTop     )
   EVT_MENU( TMCELL_REF_B        , tui::TopedFrame::OnCellRef_B   )
   EVT_MENU( TMCELL_REF_M        , tui::TopedFrame::OnCellRef_M   )
   EVT_MENU( TMCELL_AREF_B       , tui::TopedFrame::OnCellARef_B  )
   EVT_MENU( TMCELL_AREF_M       , tui::TopedFrame::OnCellARef_M  )
   EVT_MENU( TMCELL_GROUP        , tui::TopedFrame::OnCellGroup   )
   EVT_MENU( TMCELL_UNGROUP      , tui::TopedFrame::OnCellUngroup )
   
   EVT_MENU( TMDRAW_BOX          , tui::TopedFrame::OnDrawBox     )
   EVT_MENU( TMDRAW_POLY         , tui::TopedFrame::OnDrawPoly    )
   EVT_MENU( TMDRAW_WIRE         , tui::TopedFrame::OnDrawWire    )
   EVT_MENU( TMDRAW_TEXT         , tui::TopedFrame::OnDrawText    )
   
   EVT_MENU( TMSEL_SELECT_IN     , tui::TopedFrame::OnSelectIn    )
   EVT_MENU( TMSEL_PSELECT_IN    , tui::TopedFrame::OnPselectIn   )
   EVT_MENU( TMSEL_SELECT_ALL    , tui::TopedFrame::OnSelectAll   )
   EVT_MENU( TMSEL_UNSELECT_IN   , tui::TopedFrame::OnUnselectIn  )
   EVT_MENU( TMSEL_PUNSELECT_IN  , tui::TopedFrame::OnPunselectIn )
   EVT_MENU( TMSEL_UNSELECT_ALL  , tui::TopedFrame::OnUnselectAll )
   

   EVT_MENU( TMSET_STEP          , tui::TopedFrame::OnStep        )
   EVT_MENU( TMSET_AUTOPAN       , tui::TopedFrame::OnAutopan     )
   EVT_MENU( TMSET_GRIDDEF       , tui::TopedFrame::OnGridDefine  )
   EVT_MENU( TMSET_GRID0         , tui::TopedFrame::OnGrid0       )
   EVT_MENU( TMSET_GRID1         , tui::TopedFrame::OnGrid1       )
   EVT_MENU( TMSET_GRID2         , tui::TopedFrame::OnGrid2       )
   EVT_MENU( TMSET_CELLMARK      , tui::TopedFrame::OnCellMark    )
   EVT_MENU( TMSET_TEXTMARK      , tui::TopedFrame::OnTextMark    )
   
   EVT_MENU( TMSET_MARKER0       , tui::TopedFrame::OnMarker0     )
   EVT_MENU( TMSET_MARKER45      , tui::TopedFrame::OnMarker45    )
   EVT_MENU( TMSET_MARKER90      , tui::TopedFrame::OnMarker90    )
   
   EVT_MENU( TMHELP_ABOUTAPP     , tui::TopedFrame::OnAbout       )
   EVT_BUTTON(TBSTAT_ABORT       , tui::TopedFrame::OnAbort       )
   EVT_CLOSE(tui::TopedFrame::OnClose)
   EVT_SIZE( TopedFrame::OnSize )
   EVT_SASH_DRAGGED_RANGE(ID_WIN_BROWSERS, ID_WIN_CANVAS, tui::TopedFrame::OnSashDrag)
   EVT_TECUSTOM_COMMAND(wxEVT_MARKERPOSITION, wxID_ANY, tui::TopedFrame::OnPositionMessage)
   EVT_TECUSTOM_COMMAND(wxEVT_CNVSSTATUSLINE, wxID_ANY, tui::TopedFrame::OnUpdateStatusLine)
   EVT_TECUSTOM_COMMAND(wxEVT_MOUSE_ACCEL   , wxID_ANY, tui::TopedFrame::OnMouseAccel)
   
   
END_EVENT_TABLE()


tui::TopedFrame::TopedFrame(const wxString& title, const wxPoint& pos, 
                            const wxSize& size ) : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
   initView();
//   initToolBar();
   CreateStatusBar();
   SetStatusText( wxT( "Toped loaded..." ) );
   //Put initMenuBar() at the end because in Windows it crashes
   initMenuBar();
}

void tui::TopedFrame::OnClose(wxCloseEvent& event)
{
   if (event.CanVeto())
   {
      if (DATC->modified()) {
         wxMessageDialog dlg1(this,
                              "Save the current design before closing?\n(Cancel to continue the session)",
                              "Current design contains unsaved data",
                              wxYES_NO | wxCANCEL | wxICON_QUESTION);
         switch (dlg1.ShowModal()) {
            case wxID_YES:{
               wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, m_windowId);
               event.SetEventObject(this);
               //GetEventHandler()->ProcessEvent(event);
               OnTDTSave(event);
            }
            case wxID_NO: break;
            case wxID_CANCEL: {
               event.Veto(TRUE);
               return;
            }
         }
      }
      delete this;
   }
   else delete this;
}

tui::TopedFrame::~TopedFrame() {
   delete _laycanvas;
   delete _cmdline;
   delete _GLstatus;
   delete _browsers;
   delete mS_browsers;
   delete mS_GLstatus;
   delete mS_command;
   delete mS_log;
   delete mS_canvas;
}

void tui::TopedFrame::initMenuBar() {
   //---------------------------------------------------------------------------
   // menuBar entry fileMenu
   gdsMenu=new wxMenu();
   gdsMenu->Append(TMGDS_OPEN   , wxT("parse")  , wxT("Parse GDS file"));
   gdsMenu->Append(TMGDS_TRANSLATE , wxT("translate to library") , wxT("Import GDS structure"));
   gdsMenu->Append(TMGDS_EXPORTL, wxT("export library") , wxT("Export library to GDS"));
   gdsMenu->Append(TMGDS_EXPORTC, wxT("export cell") , wxT("Export cell to GDS"));
   gdsMenu->Append(TMGDS_CLOSE  , wxT("close")  , wxT("Clear the parsed GDS file from memory"));
   
   fileMenu=new wxMenu();
   fileMenu->Append(TMFILE_NEW   , wxT("New ...\tCTRL-N")    , wxT("Create new design"));
   fileMenu->Append(TMFILE_OPEN  , wxT("Open ...\tCTRL-O")   , wxT("Open a TDT file"));
   fileMenu->Append(TMFILE_INCLUDE, wxT("Include ...")       , wxT("Include a TELL file"));
   fileMenu->AppendSeparator();
   fileMenu->Append(TMGDS_EXPORTL, wxT("Export library to GDS") , wxT("Export library to GDS"));
   fileMenu->Append(TMGDS_IMPORT , wxT("Import GDS to library") , wxT("Import GDS structure"));
   fileMenu->Append(TMGDS_MENU   , wxT("Advanced GDS operations") , gdsMenu , wxT("Define marker movement"));
   fileMenu->AppendSeparator();
   fileMenu->Append(TMFILE_SAVE  , wxT("Save\tCTRL-S")       , wxT("Save the database"));
   fileMenu->Append(TMFILE_SAVEAS, wxT("Save as ..."), wxT("Save the database under a new name"));
   fileMenu->AppendSeparator();
   fileMenu->Append(TMFILE_EXIT  , wxT("Exit")       , wxT("Exit Toped"));
   //---------------------------------------------------------------------------
   // menuBar entry editMenu
   editMenu=new wxMenu();
   editMenu->Append(TMEDIT_UNDO  , wxT("Undo\tCTRL-Z")  , wxT("Undo last operation"));
   editMenu->AppendSeparator();
   editMenu->Append(TMEDIT_COPY  , wxT("Copy\tCTRL-C")  , wxT("Copy selected shapes"));
   editMenu->Append(TMEDIT_MOVE  , wxT("Move\tCTRL-M")  , wxT("Move selected shapes"));
   editMenu->Append(TMEDIT_DELETE, wxT("Delete\tCTRL-D"), wxT("Delete selected shapes"));
   editMenu->AppendSeparator();
   editMenu->Append(TMEDIT_ROTATE90, wxT("Rotate 90\tCTRL-9"), wxT("Rotate selected shapes on 90 deg. counter clockwise "));
   editMenu->Append(TMEDIT_FLIPX, wxT("Flip X\tCTRL-X"), wxT("Flip selected shapes towards X axis "));
   editMenu->Append(TMEDIT_FLIPY, wxT("Flip Y\tCTRL-Y"), wxT("Flip selected shapes towards X axis "));
   editMenu->Append(TMEDIT_POLYCUT, wxT("Cut with poly\tCTRL-U"), wxT("Cut selected shapes with a polygon "));
   editMenu->Append(TMEDIT_MERGE, wxT("Merge\tCTRL-G"), wxT("Merge selected shpes"));
   
   //---------------------------------------------------------------------------
   // menuBar entry viewMenu
   viewMenu=new wxMenu();
   viewMenu->AppendCheckItem(TMVIEW_VIEWTOOLBAR  , wxT("Toolbar")  , wxT("Show/Hide the tool bar"));
   viewMenu->AppendCheckItem(TMVIEW_VIEWSTATUSBAR, wxT("Statusbar"), wxT("Show/Hide the status bar"));
   viewMenu->AppendSeparator();
   viewMenu->Append(TMVIEW_ZOOMIN  , wxT("Zoom in\tF2")  , wxT("Zoom in current window"));
   viewMenu->Append(TMVIEW_ZOOMOUT , wxT("Zoom out\tF3") , wxT("Zoom out current window"));
   viewMenu->Append(TMVIEW_ZOOMALL , wxT("Zoom all\tF4") , wxT("Zoom the current cell"));
   viewMenu->AppendSeparator();
   viewMenu->Append(TMVIEW_PANLEFT , wxT("Pan left\tSHIFT-LEFT") , wxT("Move the view window left"));
   viewMenu->Append(TMVIEW_PANRIGHT, wxT("Pan right\tSHIFT-RIGHT"), wxT("Move the view window right"));
   viewMenu->Append(TMVIEW_PANUP   , wxT("Pan up\tSHIFT-UP")   , wxT("Move the view window up"));
   viewMenu->Append(TMVIEW_PANDOWN , wxT("Pan down\tSHIFT-DOWN") , wxT("Move the view window down"));
   viewMenu->AppendSeparator();
   viewMenu->Check(TMVIEW_VIEWTOOLBAR, true);
   viewMenu->Check(TMVIEW_VIEWSTATUSBAR, true);
   //---------------------------------------------------------------------------
   // menuBar entry Cell
   cellMenu=new wxMenu();
   cellMenu->Append(TMCELL_NEW      , wxT("New Cell") , wxT("Create anew cell"));
   cellMenu->Append(TMCELL_OPEN     , wxT("Open Cell") , wxT("Open existing cell for editing"));
   cellMenu->AppendSeparator();
   cellMenu->Append(TMCELL_PUSH     , wxT("Edit Push\tF9") , wxT("Edit in place"));
   cellMenu->Append(TMCELL_PREV     , wxT("Edit Previous\tCtrl-F9") , wxT("Edit in place"));
   cellMenu->Append(TMCELL_POP      , wxT("Edit Pop\tF10") , wxT("Edit in place"));
   cellMenu->Append(TMCELL_TOP      , wxT("Edit Top\tCtrl-F10") , wxT("Edit in place"));
   cellMenu->AppendSeparator();
   cellMenu->Append(TMCELL_REF_M    , wxT("Cell Reference\tCtrl-R") , wxT("Cell reference"));
   cellMenu->Append(TMCELL_AREF_M   , wxT("Array of References\tAlt-R") , wxT("Array of cell references"));
   cellMenu->AppendSeparator();
   cellMenu->Append(TMCELL_GROUP    , wxT("Group Cell") , wxT("Group selected shapes in a cell"));
   cellMenu->Append(TMCELL_UNGROUP  , wxT("Unroup Cell") , wxT("Ungroup selected cell references"));
   //---------------------------------------------------------------------------
   // menuBar entry Draw
   drawMenu=new wxMenu();
   drawMenu->Append(TMDRAW_BOX , wxT("Box\tCTRL-B")     , wxT("Create new box on the current layer"));
   drawMenu->Append(TMDRAW_POLY, wxT("Polygon\tCTRL-L") , wxT("Create new polygon on the current layer"));
   drawMenu->Append(TMDRAW_WIRE, wxT("Wire ...\tCTRL-W"), wxT("Create new wire on the current layer"));
   drawMenu->Append(TMDRAW_TEXT, wxT("Text ...\tCTRL-T"), wxT("Add text on the current layer"));
   //---------------------------------------------------------------------------
   // menuBar entry Modify
   selectMenu=new wxMenu();
   selectMenu->Append(TMSEL_SELECT_IN   , wxT("Select\tCTRL-I")        , wxT("Select objects"));
   selectMenu->Append(TMSEL_PSELECT_IN  , wxT("Part select\tCTRL-P")  , wxT("Select object edges"));
   selectMenu->Append(TMSEL_SELECT_ALL  , wxT("Select all\tCTRL-A")    , wxT("Select all objects in the current cell"));
   selectMenu->AppendSeparator();
   selectMenu->Append(TMSEL_UNSELECT_IN , wxT("Unselect\tALT-I")      , wxT("Unselect objects"));
   selectMenu->Append(TMSEL_PUNSELECT_IN, wxT("Part unselect\tALT-P"), wxT("Unselect object edges"));
   selectMenu->Append(TMSEL_UNSELECT_ALL, wxT("Unselect all\tALT-A")  , wxT("Unselect all"));
   //---------------------------------------------------------------------------
   // menuBar entry Settings
   // first the sub menu
   markerMenu=new wxMenu();
   markerMenu->AppendRadioItem(TMSET_MARKER0    , wxT("Free")      , wxT("Marker is not restricted"));
   markerMenu->AppendRadioItem(TMSET_MARKER45   , wxT("45 degrees"), wxT("Restrict shape angles to 45 deg"));
   markerMenu->AppendRadioItem(TMSET_MARKER90   , wxT("Orthogonal"), wxT("Restrict shape angles to 90 deg"));
   // now the setting menu itself
   settingsMenu=new wxMenu();
   settingsMenu->Append         (TMSET_STEP     , wxT("Step")      , wxT("Select objects"));
   settingsMenu->AppendCheckItem(TMSET_AUTOPAN  , wxT("Auto Pan")  , wxT("Automatic window move"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_GRIDDEF  , wxT("Define grid"), wxT("Define/change the grid step"));
   settingsMenu->AppendCheckItem(TMSET_GRID0    , wxT("Grid 0")    , wxT("Draw/Hide Grid 0"));
   settingsMenu->AppendCheckItem(TMSET_GRID1    , wxT("Grid 1")    , wxT("Draw/Hide Grid 1"));
   settingsMenu->AppendCheckItem(TMSET_GRID2    , wxT("Grid 2")    , wxT("Draw/Hide Grid 2"));
   settingsMenu->AppendCheckItem(TMSET_CELLMARK , wxT("Cell marks"), wxT("Draw/Hide Cell marks"));
   settingsMenu->AppendCheckItem(TMSET_TEXTMARK , wxT("Text marks"), wxT("Draw/Hide Text marks"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_MARKER   , wxT("Marker") , markerMenu , wxT("Define marker movement"));
   //---------------------------------------------------------------------------
   // menuBar entry helpMenu
   helpMenu=new wxMenu();
   helpMenu->Append(TMHELP_ABOUTAPP       , wxT("About")          , wxT("About TOPED"));
   //---------------------------------------------------------------------------
   // MENUBAR CONFIGURATION
   menuBar = new wxMenuBar();
   menuBar->Append(fileMenu      , wxT("&File"  ));
   menuBar->Append(editMenu      , wxT("&Edit"  ));
   menuBar->Append(viewMenu      , wxT("&View"  ));
   menuBar->Append(cellMenu      , wxT("&Cell"  ));
   menuBar->Append(drawMenu      , wxT("&Draw"  ));
   menuBar->Append(selectMenu    , wxT("&Select"));
   menuBar->Append(settingsMenu  , wxT("Se&ttings"));
   menuBar->Append(helpMenu      , wxT("&Help"  ));
   SetMenuBar( menuBar );
   // default menu values
   settingsMenu->Check(TMSET_CELLMARK,true);
   settingsMenu->Check(TMSET_TEXTMARK,true);
}
/*
void TopedFrame::initToolBar() {
   wxToolBar* positionBar = CreateToolBar(wxTB_DOCKABLE |  wxTB_HORIZONTAL | wxNO_BORDER);
   X_pos = new wxStaticText(positionBar, -1, "", wxDefaultPosition, 
                              wxSize(100,32), wxST_NO_AUTORESIZE);
   Y_pos = new wxStaticText(positionBar, -1, "", wxDefaultPosition, 
                              wxSize(100,32), wxST_NO_AUTORESIZE);
//wxSIMPLE_BORDER | wxALIGN_RIGHT  |
   wxFont fontX = X_pos->GetFont();
   fontX.SetPointSize(fontX.GetPointSize()*3/2);
   X_pos->SetFont(fontX);
   Y_pos->SetFont(fontX);

   X_pos->SetBackgroundColour(wxColour("WHITE"));
   Y_pos->SetBackgroundColour(wxColour("WHITE"));
   X_pos->SetLabel("0.00");
   Y_pos->SetLabel("0.00");
   X_pos->Refresh();
   Y_pos->Refresh();
   positionBar->AddControl(X_pos);
   positionBar->AddSeparator();
   positionBar->AddControl(Y_pos);
   positionBar->AddSeparator();
   positionBar->Realize();
}
*/
void tui::TopedFrame::initView() {
   // The browsers window
   mS_browsers = new wxSashLayoutWindow(this, ID_WIN_BROWSERS,
                                        wxDefaultPosition, wxDefaultSize,
                               wxSW_3D | wxCLIP_CHILDREN);
   mS_browsers->SetDefaultSize(wxSize(180, 1000));
   mS_browsers->SetOrientation(wxLAYOUT_VERTICAL);
   mS_browsers->SetAlignment(wxLAYOUT_LEFT);
   mS_browsers->SetSashVisible(wxSASH_RIGHT, TRUE);
   _browsers = new browsers::browserTAB(mS_browsers);
   //---------------------------------------------------------------------------- 
   // The Layoutcanvas toolbar window
   //---------------------------------------------------------------------------- 
   mS_GLstatus = new wxSashLayoutWindow(this, ID_WIN_GLSTATUS,
                                        wxDefaultPosition, wxDefaultSize,
                             wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
   mS_GLstatus->SetDefaultSize(wxSize(1000, 30));
   mS_GLstatus->SetOrientation(wxLAYOUT_HORIZONTAL);
   mS_GLstatus->SetAlignment(wxLAYOUT_TOP);
   mS_GLstatus->SetBackgroundColour(wxColour(255, 0, 0));
   _GLstatus = new CanvasStatus(mS_GLstatus);
   //----------------------------------------------------------------------------
   // The command window
   //----------------------------------------------------------------------------
   mS_command = new wxSashLayoutWindow(this, ID_WIN_COMMAND,
                               wxDefaultPosition, wxDefaultSize,
                               wxSW_3D | wxCLIP_CHILDREN);
   mS_command->SetDefaultSize(wxSize(1000, 30));
   mS_command->SetOrientation(wxLAYOUT_HORIZONTAL);
   mS_command->SetAlignment(wxLAYOUT_BOTTOM);
   mS_command->SetSashVisible(wxSASH_TOP, TRUE);
   _cmdline = new console::ted_cmd(mS_command);
   //----------------------------------------------------------------------------
   //the log window
   //----------------------------------------------------------------------------
   mS_log = new wxSashLayoutWindow(this, ID_WIN_LOG,
                               wxDefaultPosition, wxDefaultSize,
                               wxSW_3D | wxCLIP_CHILDREN);
   mS_log->SetDefaultSize(wxSize(1000, 150));
   mS_log->SetOrientation(wxLAYOUT_HORIZONTAL);
   mS_log->SetAlignment(wxLAYOUT_BOTTOM);
   mS_log->SetSashVisible(wxSASH_TOP, TRUE);
   _cmdlog = new console::ted_log(mS_log);
   //----------------------------------------------------------------------------
   // the openGL window
   //---------------------------------------------------------------------------- 
   mS_canvas = new wxSashLayoutWindow(this, ID_WIN_CANVAS,
                               wxDefaultPosition, wxDefaultSize,
                               wxSW_3D | wxCLIP_CHILDREN);
//   mS_canvas->SetDefaultSize(wxSize(1000, 600));
//   mS_log->SetSashVisible(wxSASH_BOTTOM, true);
//   mS_log->SetSashVisible(wxSASH_LEFT, true);
   //the canvas
#ifdef __WXMSW__
   int *gl_attrib = NULL;
#else
   int gl_attrib[20] = { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1,
            WX_GL_MIN_BLUE, 1, WX_GL_DEPTH_SIZE, 1,
            WX_GL_DOUBLEBUFFER,
#  ifdef __WXMAC__
            GL_NONE };
#  else
            None };
#  endif
#endif
   _laycanvas = new LayoutCanvas(mS_canvas, gl_attrib);
}


void tui::TopedFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) ) {
   Close(FALSE);
}

void tui::TopedFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) ) {
   wxMessageBox( wxT( "A wxWidget port of the TOPED from QT" ),
                  wxT( "About current state of wxToped" ), wxOK | wxICON_INFORMATION, this );
}


void tui::TopedFrame::OnSashDrag(wxSashEvent& event) {
   if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
      return;
   switch (event.GetId()) {
//      case ID_WIN_GLSTATUS:
//         mS_GLstatus->SetDefaultSize(wxSize(1000, event.GetDragRect().height));
//         break;
      case ID_WIN_BROWSERS:
         mS_browsers->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
         break;
      case ID_WIN_LOG:
         mS_log->SetDefaultSize(wxSize(1000, event.GetDragRect().height));
         break;
   }
   wxLayoutAlgorithm layout;
   layout.LayoutFrame(this, mS_canvas);
}

void tui::TopedFrame::OnSize(wxSizeEvent& WXUNUSED(event)) {
   wxLayoutAlgorithm layout;
   layout.LayoutFrame(this, mS_canvas);
}

void tui::TopedFrame::OnPositionMessage(wxCommandEvent& evt) {
   switch ((POSITION_TYPE)evt.GetInt()) {
      case POS_X: _GLstatus->setXpos(evt.GetString()); break;
      case POS_Y: _GLstatus->setYpos(evt.GetString()); break;
      case DEL_X: _GLstatus->setdXpos(evt.GetString()); break;
      case DEL_Y: _GLstatus->setdYpos(evt.GetString()); break;
      default: assert(false);
   }   
}

void tui::TopedFrame::OnNewDesign(wxCommandEvent& evt) {
   if (DATC->modified()) {
      wxMessageDialog dlg1(this,
         "Current design contains unsaved data",
         "Save the current design before creating a new one?",
         wxYES_NO | wxCANCEL | wxICON_QUESTION);
      switch (dlg1.ShowModal()) {
         case wxID_YES:OnTDTSave(evt);
         case wxID_NO: break;
         case wxID_CANCEL: return;
      }
   }
   wxTextEntryDialog dlg2(this,
      "Name the new design",
      "Design name:");
   wxString dname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((dname = dlg2.GetValue()) != "")) {
      SetStatusText("Creating new file...");
      ost << "newdesign(\"" << dname << "\");";
      _cmdline->parseCommand(ost);
      SetTitle(dname);
   }
   else SetStatusText("New file not created");
}

void tui::TopedFrame::OnTDTRead(wxCommandEvent& evt) {
   if (DATC->modified()) {
      wxMessageDialog dlg1(this,
         "Current design contains unsaved data",
         "Save the current design before creating a new one?",
         wxYES_NO | wxCANCEL | wxICON_QUESTION);
      switch (dlg1.ShowModal()) {
         case wxID_YES:OnTDTSave(evt);
         case wxID_NO: break;
         case wxID_CANCEL: return;
      }
   }
   SetStatusText("Opening file...");
   wxFileDialog dlg2(this, "Select a file", "", "", 
      "Toped files |*.tdt",
      wxOPEN);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << "tdtread(\"" << dlg2.GetDirectory() << "/" << dlg2.GetFilename() << "\");";
      _cmdline->parseCommand(ost);
      SetStatusText("File "+dlg2.GetFilename()+" loaded");
      SetTitle(dlg2.GetFilename());
   }
   else SetStatusText("Opening aborted");
}

void tui::TopedFrame::OnTELLRead(wxCommandEvent& evt) {
   SetStatusText("Including command file...");
   wxFileDialog dlg2(this, "Select a file", "", "", 
      "Tell files |*.tll",
      wxOPEN);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << "`include \"" << dlg2.GetDirectory() << "/" << dlg2.GetFilename() << "\";";
      _cmdline->parseCommand(ost);
      SetStatusText(dlg2.GetFilename() + " parsed");
   }
   else SetStatusText("include aborted");
}

void tui::TopedFrame::OnGDSRead(wxCommandEvent& WXUNUSED(event)) {
   wxFileDialog dlg2(this, "Select a file", "", "", 
      "Stream files |*.sf;*.gds",
      wxOPEN);
   if (wxID_OK == dlg2.ShowModal()) {
      SetStatusText("Parsing GDS file...");
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << "gdsread(\"" << dlg2.GetDirectory() << "/" <<dlg2.GetFilename() << "\");";
      _cmdline->parseCommand(ost, true);
      SetStatusText("Stream "+dlg2.GetFilename()+" loded");
   }
   else SetStatusText("Parsing aborted");
}

void tui::TopedFrame::OnTDTSave(wxCommandEvent& WXUNUSED(event)) {
   wxString ost; 
   ost << "tdtsave();";
   SetStatusText("Saving file...");
   wxFileName datafile(wxString(DATC->tedfilename().c_str()));
   assert(datafile.IsOk());
   if (datafile.FileExists() && DATC->neversaved()) {
      wxMessageDialog dlg1(this,
         "File "+ wxString(DATC->tedfilename().c_str()) + " already exists. Overwrite ?",
         "Toped",
         wxYES_NO | wxICON_QUESTION);
      switch (dlg1.ShowModal()) {
         case wxID_YES:_cmdline->parseCommand(ost); //Overwrite;
         case wxID_NO: return;
      }
   }
   else _cmdline->parseCommand(ost);;
   SetStatusText("Design saved");
}

void tui::TopedFrame::OnTDTSaveAs(wxCommandEvent& WXUNUSED(event)) {
   SetStatusText("Saving database under new filename...");
   wxFileDialog dlg2(this, "Save a design in a file", "", "",
      "Toped files |*.tdt",
      wxSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << "tdtsaveas(\"" << dlg2.GetDirectory() << "/" <<dlg2.GetFilename() << "\");";
      _cmdline->parseCommand(ost);
      SetStatusText("Design saved in file: "+dlg2.GetFilename());
   }   
   else SetStatusText("Saving aborted");
}

void tui::TopedFrame::OnCellNew(wxCommandEvent& WXUNUSED(event)) {
   wxTextEntryDialog dlg2(this,
      "Cell name:",
      "Create new cell");
   wxString cname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((cname = dlg2.GetValue()) != "")) {
      SetStatusText("Creating new cell...");
      ost << "newcell(\"" << cname << "\");";
      _cmdline->parseCommand(ost);
   }
   else SetStatusText("New cell not created");
}

void tui::TopedFrame::OnCellOpen(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellOpen* dlg = NULL;
   try {
      dlg = new tui::getCellOpen(this, -1, "Cell Open", pos, "");
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << "opencell(\"" << dlg->get_selectedcell() << "\");";
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnGDStranslate(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getGDSimport* dlg = NULL;
   try {
      dlg = new tui::getGDSimport(this, -1, "Import GDS structure", pos,
                                          _browsers->TDTSelectedGDSName());
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << "gdsimport(\"" << dlg->get_selectedcell() << "\" , "
          << (dlg->get_recursive() ? "true" : "false")   << " , "
          << (dlg->get_overwrite() ? "true" : "false")   <<");";
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnGDSimport(wxCommandEvent& WXUNUSED(event) evt)
{
   OnGDSRead(evt);

  // OnGDStranslate(evt);
   wxString topCellName;

   wxString ost;
   try
   {
      topCellName = _browsers->TDTGDSTopCellName();
   }
   catch (EXPTN) 
   {
      ost<<"There are more then one top cells into GDS structure\n"
         <<"Use advanced operations menu with this GDS";
      wxMessageBox(ost);
      OnGDSclose(evt);
      return;
   }
   
   ost << "gdsimport(\"" << topCellName << "\" , "
          <<  "true"    << " , "
          <<  "true"    <<");";
      _cmdline->parseCommand(ost);
   OnGDSclose(evt);
}

void tui::TopedFrame::OnGDSexportLIB(wxCommandEvent& WXUNUSED(event)) {
   SetStatusText("Exporting database to GDS file...");
   wxFileDialog dlg2(this, "Export design to GDS file", "", "",
      "GDS files |*.sf;*.gds",
      wxSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << "gdsexport(\"" << dlg2.GetDirectory() << "/" <<dlg2.GetFilename() << "\");";
      _cmdline->parseCommand(ost);
      SetStatusText("Design exported to: "+dlg2.GetFilename());
   }
   else SetStatusText("GDS export aborted");
}

void tui::TopedFrame::OnGDSexportTOP(wxCommandEvent& WXUNUSED(event)) {
   SetStatusText("Exporting a cell to GDS file...");
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getGDSexport* dlg = NULL;
   try {
      dlg = new tui::getGDSexport(this, -1, "GDS export cell", pos, _browsers->TDTSelectedCellName());
   }
   catch (EXPTN) {delete dlg;return;}
   std::string cellname;
   bool recur;
   if ( dlg->ShowModal() == wxID_OK ) {
      cellname = dlg->get_selectedcell();
      recur = dlg->get_recursive();
      delete dlg;
   }
   else {delete dlg;return;}
   std::string str = std::string("Exporting ")+cellname+" to GDS file";
   std::string fullCellName = cellname+".gds";
   wxFileDialog dlg2(this, wxT(str.c_str()), "", wxT(fullCellName.c_str()), 
      wxT("GDS files |*.sf;*.gds"),
      wxSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << "gdsexport(\"" << cellname.c_str() << "\" , " <<
                        (recur ? "true" : "false") << ",\"" <<
                        (dlg2.GetDirectory()).c_str() << "/" <<(dlg2.GetFilename()).c_str() << "\");";
      _cmdline->parseCommand(ost);
      SetStatusText("Design exported to: "+dlg2.GetFilename());
   }
   else SetStatusText("GDS export aborted");
}

void tui::TopedFrame::OnCellRef_B(wxCommandEvent& WXUNUSED(event)) {
   CellRef(_browsers->TDTSelectedCellName());
}

void tui::TopedFrame::OnCellRef_M(wxCommandEvent& WXUNUSED(event)) {
   CellRef("");
}

void tui::TopedFrame::OnCellARef_B(wxCommandEvent& WXUNUSED(event)) {
   CellARef(_browsers->TDTSelectedCellName());
}

void tui::TopedFrame::OnCellARef_M(wxCommandEvent& WXUNUSED(event)) {
   CellARef("");
}

void tui::TopedFrame::CellRef(wxString clname) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellRef* dlg = NULL;
   try {
      dlg = new tui::getCellRef(this, -1, "Cell Reference", pos, clname);
   }
   catch (EXPTN) {delete dlg;return;}   
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << "cellref(\"" << dlg->get_selectedcell() <<"\",getpoint()," 
          <<                     dlg->get_angle() << "," 
          << (dlg->get_flip() ? "true" : "false") << ","
          <<                                "1"  << ");";
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::CellARef(wxString clname) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellARef* dlg = NULL;
   try {
      dlg = new tui::getCellARef(this, -1, "Array of References", pos, clname);
   }
   catch (EXPTN) {delete dlg; return;}   
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << "cellaref(\"" << dlg->get_selectedcell() <<"\",getpoint()," 
          <<                     dlg->get_angle() << "," 
          << (dlg->get_flip() ? "true" : "false") << ","
          <<                                 "1"  << ","
          <<                       dlg->get_col() << "," 
          <<                       dlg->get_row() << "," 
          <<                     dlg->get_stepX() << "," 
          <<                     dlg->get_stepY() << ");";
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnCellGroup(wxCommandEvent& WXUNUSED(event)) {
   // Here - try a hollow lock/unlock the database just to check that it exists
   try {DATC->lockDB();}
   catch (EXPTN) {return;}   
   DATC->unlockDB();
   //
   wxTextEntryDialog dlg2(this,
      "Cell name:",
      "Create new cell from selected components");
   wxString cname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((cname = dlg2.GetValue()) != "")) {
      SetStatusText("Grouping in a new cell...");
      ost << "group(\"" << cname << "\");";
      _cmdline->parseCommand(ost);
   }
   else SetStatusText("Groupping canceled");
}

void tui::TopedFrame::OnDrawWire(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = new tui::getSize(this, -1, "Wire width", pos);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost; ost << "addwire("<<dlg->value()<<");";
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnDrawText(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);   
   tui::getTextdlg* dlg = NULL;
   try {
      dlg = new tui::getTextdlg(this, -1, "Add text", pos);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost; ost << "addtext(\""
                        << dlg->get_text()                      << "\","
                        << Properties->curlay()                 << ","
                        << "getpoint(),"
                        << dlg->get_angle()                     << ","
                        << (dlg->get_flip() ? "true" : "false") << ","
                        << dlg->get_size()                      << ");";
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnStep(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getStep dlg(this, -1, "Step size", pos, Properties->step());
   if ( dlg.ShowModal() == wxID_OK ) {
      wxString ost; ost << "step("<<dlg.value()<<");";
      _cmdline->parseCommand(ost);
   }
}

void tui::TopedFrame::OnGridDefine(wxCommandEvent& WXUNUSED(event)) {
   real grid[3];
   const layprop::LayoutGrid *gr  = NULL;
   for (byte i = 0; i < 3; i++)
      if (NULL != (gr = Properties->grid(i))) grid[i] = gr->step();
      else                                    grid[i] = 0.0;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getGrid dlg(this, -1, "Grid size", pos, grid[0], grid[1], grid[2]);
   if ( dlg.ShowModal() == wxID_OK ) {
      wxString ost; 
      ost << "definegrid(0,"<<dlg.grid0()<<",\"white\");";
      ost << "definegrid(1,"<<dlg.grid1()<<",\"white\");";
      ost << "definegrid(2,"<<dlg.grid2()<<",\"white\");";
      _cmdline->parseCommand(ost);
   }
}

void tui::TopedFrame::OnGrid0(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << "grid(0," << (settingsMenu->IsChecked(TMSET_GRID0) ? "true" : "false") << ");";
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnGrid1(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << "grid(1," << (settingsMenu->IsChecked(TMSET_GRID1) ? "true" : "false") << ");";
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnGrid2(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << "grid(2," << (settingsMenu->IsChecked(TMSET_GRID2) ? "true" : "false") << ");";
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnCellMark(wxCommandEvent& WXUNUSED(event)){
  wxString ost;
  ost << "hidecellmarks(" << (settingsMenu->IsChecked(TMSET_CELLMARK) ? "false" : "true") << ");";
  _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnTextMark(wxCommandEvent& WXUNUSED(event)){
  wxString ost;
  ost << "hidetextmarks(" << (settingsMenu->IsChecked(TMSET_TEXTMARK) ? "false" : "true") << ");";
  _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnAutopan(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << "autopan("<< (settingsMenu->IsChecked(TMSET_AUTOPAN) ? "true" : "false") << ");";
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnMarker0(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << "shapeangle(0);";
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnMarker45(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << "shapeangle(45);";
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnMarker90(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << "shapeangle(90);";
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnAbort(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventButtonUP(wxEVT_COMMAND_ENTER);
   eventButtonUP.SetClientData((void*)NULL);
   eventButtonUP.SetInt(-1);
   wxPostEvent(Console, eventButtonUP);
}

void tui::TopedFrame::OnUpdateStatusLine(wxCommandEvent& evt) {
   switch (evt.GetInt()) {
      case STS_SELECTED    : _GLstatus->setSelected(evt.GetString());break;
      case STS_ABORTENABLE : _GLstatus->btn_abort_enable(true);break;
      case STS_ABORTDISABLE: _GLstatus->btn_abort_enable(false);break;
      case STS_GRID0_ON    : settingsMenu->Check(TMSET_GRID0,true );break;
      case STS_GRID0_OFF   : settingsMenu->Check(TMSET_GRID0,false);break;
      case STS_GRID1_ON    : settingsMenu->Check(TMSET_GRID1,true );break;
      case STS_GRID1_OFF   : settingsMenu->Check(TMSET_GRID1,false);break;
      case STS_GRID2_ON    : settingsMenu->Check(TMSET_GRID2,true );break;
      case STS_GRID2_OFF   : settingsMenu->Check(TMSET_GRID2,false);break;
      case STS_CELLMARK_OFF: settingsMenu->Check(TMSET_CELLMARK,false);break;
      case STS_CELLMARK_ON : settingsMenu->Check(TMSET_CELLMARK,true);break;
      case STS_TEXTMARK_OFF: settingsMenu->Check(TMSET_TEXTMARK,false);break;
      case STS_TEXTMARK_ON : settingsMenu->Check(TMSET_TEXTMARK,true);break;
      case STS_AUTOPAN_ON  : settingsMenu->Check(TMSET_AUTOPAN,true );break;
      case STS_AUTOPAN_OFF : settingsMenu->Check(TMSET_AUTOPAN,false);break;
      case STS_ANGLE_0     : settingsMenu->Check(TMSET_MARKER0  ,true );break;
      case STS_ANGLE_45    : settingsMenu->Check(TMSET_MARKER45 ,true );break;
      case STS_ANGLE_90    : settingsMenu->Check(TMSET_MARKER90 ,true );break;
                    default: assert(false);
   }
};

void  tui::TopedFrame::OnMouseAccel(wxCommandEvent& evt) {
   wxString ost;
   if (0 == evt.GetInt())
      ost << "unselect(";
   else if (1 == evt.GetInt())
      ost << "select(";
   else return;
   ost << evt.GetString() << ");";   
   _cmdline->parseCommand(ost);   
}

void tui::TopedFrame::OnzoomIn(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_IN);
   wxPostEvent(_laycanvas, eventZOOM);
}

void tui::TopedFrame::OnzoomOut(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_OUT);
   wxPostEvent(_laycanvas, eventZOOM);
}

void tui::TopedFrame::OnpanLeft(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_LEFT);
   wxPostEvent(_laycanvas, eventZOOM);
}

void tui::TopedFrame::OnpanRight(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_RIGHT);
   wxPostEvent(_laycanvas, eventZOOM);
}

void tui::TopedFrame::OnpanUp(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_UP);
   wxPostEvent(_laycanvas, eventZOOM);
}

void tui::TopedFrame::OnpanDown(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_DOWN);
   wxPostEvent(_laycanvas, eventZOOM);
}
