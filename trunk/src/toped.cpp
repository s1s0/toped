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
//        Created: Thu May  6 22:04:50 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Main Toped framework
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <GL/glew.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/aboutdlg.h>
#include <wx/tooltip.h>
#include <math.h>
#include "toped.h"
#include "datacenter.h"
#include "viewprop.h"
#include "tenderer.h"
#include "tui.h"
#include "../ui/toped32x32.xpm"


#ifndef WIN32
   #include "../ui/toped16x16.xpm"
#endif
#if wxCHECK_VERSION(2, 8, 0)
#define tpdfOPEN wxFD_OPEN
#define tpdfSAVE wxFD_SAVE
#else
#define tpdfOPEN wxOPEN
#define tpdfSAVE wxSAVE
#endif

extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_SETINGSMENU;
extern const wxEventType         wxEVT_MOUSE_ACCEL;
extern const wxEventType         wxEVT_CURRENT_LAYER;
extern const wxEventType         wxEVT_TOOLBARSIZE;
extern const wxEventType         wxEVT_TOOLBARDEF;
extern const wxEventType         wxEVT_TOOLBARADDITEM;
extern const wxEventType         wxEVT_TOOLBARDELETEITEM;
extern const wxEventType         wxEVT_EDITLAYER;

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern parsercmd::cmdBLOCK*      CMDBlock;

tui::CanvasStatus::CanvasStatus(wxWindow* parent, wxWindowID id ,
   const wxPoint& pos , const wxSize& size , long style)
   : wxPanel( parent, id, pos, size, style)
{
   wxFont fontX = GetFont();
   fontX.SetWeight(wxBOLD);
   fontX.SetPointSize(9);
   //fontX.SetFamily(wxFONTFAMILY_MODERN);

   SetFont(fontX);
   wxBoxSizer *thesizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   SetBackgroundColour(wxColour(wxT("LIGHT_GRAY")));
   SetForegroundColour(wxColour(wxT("BLACK")));
   X_pos = DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("0.00"), wxDefaultPosition,wxSize(120,-1),
                                                   wxST_NO_AUTORESIZE | wxALIGN_RIGHT );
   Y_pos = DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("0.00"), wxDefaultPosition, wxSize(120,-1),
                                                   wxST_NO_AUTORESIZE | wxALIGN_RIGHT );
   _dX = DEBUG_NEW wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(100,-1),
                                                   wxST_NO_AUTORESIZE | wxALIGN_RIGHT );
   _dY = DEBUG_NEW wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(100,-1),
                                                   wxST_NO_AUTORESIZE | wxALIGN_RIGHT );

   _selected = DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize(40,-1),
                                                   wxST_NO_AUTORESIZE | wxALIGN_LEFT);
   thesizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Selected: ")), 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(_selected, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(10,0,0);
   thesizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("dX: ")), 0, wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(_dX, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(10,0,0);
   thesizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("dY: ")), 0, wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(_dY, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(20,0,0);
   thesizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("X: ")), 0, wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(X_pos, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(10,0,0);
   thesizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Y: ")), 0, wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(Y_pos, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 3 );
   thesizer->Add(20,0,0);

   SetSizer( thesizer );      // use the sizer for layout
   thesizer->SetSizeHints( this );   // set size hints to honour minimum size
}

tui::CanvasStatus::~CanvasStatus() {
   delete X_pos;
   delete Y_pos;
   delete _dX;
   delete _dY;
   delete _selected;
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
//-----------------------------------------------------------------------------
// The TopedFrame event table (TOPED main event table)
BEGIN_EVENT_TABLE( tui::TopedFrame, wxFrame )
   EVT_MENU( TMFILE_NEW          , tui::TopedFrame::OnNewDesign   )
   EVT_MENU( TMFILE_OPEN         , tui::TopedFrame::OnTDTRead     )
   EVT_MENU( TMFILE_INCLUDE      , tui::TopedFrame::OnTELLRead    )
   EVT_MENU( TMLIB_LOAD          , tui::TopedFrame::OnTDTLoadLib  )
   EVT_MENU( TMLIB_UNLOAD        , tui::TopedFrame::OnTDTUnloadLib)

   EVT_MENU( TMGDS_OPEN          , tui::TopedFrame::OnGDSRead     )
   EVT_MENU( TMGDS_IMPORT        , tui::TopedFrame::OnGDSimport   )
   EVT_MENU( TMGDS_TRANSLATE     , tui::TopedFrame::OnGDStranslate)
   EVT_MENU( TMGDS_EXPORTL       , tui::TopedFrame::OnGDSexportLIB)
   EVT_MENU( TMGDS_EXPORTC       , tui::TopedFrame::OnGDSexportCELL)
   EVT_MENU( TMGDS_CLOSE         , tui::TopedFrame::OnGDSclose    )

   EVT_MENU( TMCIF_EXPORTL       , tui::TopedFrame::OnCIFexportLIB)
//   EVT_MENU( TMCIF_EXPORTC       , tui::TopedFrame::OnCIFimport   )
   EVT_MENU( TMCIF_OPEN          , tui::TopedFrame::OnCIFRead     )
   EVT_MENU( TMCIF_TRANSLATE     , tui::TopedFrame::OnCIFtranslate)
   EVT_MENU( TMCIF_EXPORTC       , tui::TopedFrame::OnCIFexportCELL)
   EVT_MENU( TMCIF_CLOSE         , tui::TopedFrame::OnCIFclose    )

   EVT_MENU( TMOAS_OPEN          , tui::TopedFrame::OnOASRead     )
   EVT_MENU( TMOAS_IMPORT        , tui::TopedFrame::OnOASimport   )
   EVT_MENU( TMOAS_TRANSLATE     , tui::TopedFrame::OnOAStranslate)
   EVT_MENU( TMOAS_EXPORTL       , tui::TopedFrame::OnOASexportLIB)
   EVT_MENU( TMOAS_EXPORTC       , tui::TopedFrame::OnOASexportCELL)
   EVT_MENU( TMOAS_CLOSE         , tui::TopedFrame::OnOASclose    )

   EVT_MENU( TMFILE_SAVE         , tui::TopedFrame::OnTDTSave     )
   EVT_MENU( TMFILE_SAVEAS       , tui::TopedFrame::OnTDTSaveAs   )
   EVT_MENU( TMPROP_SAVE         , tui::TopedFrame::OnPropSave    )
   EVT_MENU( TMFILE_EXIT         , tui::TopedFrame::OnQuit        )

   EVT_MENU( TMEDIT_UNDO         , tui::TopedFrame::OnUndo        )
   EVT_MENU( TMEDIT_COPY         , tui::TopedFrame::OnCopy        )
   EVT_MENU( TMEDIT_MOVE         , tui::TopedFrame::OnMove        )
   EVT_MENU( TMEDIT_DELETE       , tui::TopedFrame::OnDelete      )
   EVT_MENU( TMEDIT_ROTATE90     , tui::TopedFrame::OnRotate      )
   EVT_MENU( TMEDIT_FLIPX        , tui::TopedFrame::OnFlipVert    )
   EVT_MENU( TMEDIT_FLIPY        , tui::TopedFrame::OnFlipHor     )
   EVT_MENU( TMEDIT_POLYCUT      , tui::TopedFrame::OnPolyCut     )
   EVT_MENU( TMEDIT_MERGE        , tui::TopedFrame::OnMerge       )
   EVT_MENU( TMEDIT_RESIZE       , tui::TopedFrame::OnResize      )

   EVT_MENU( TMVIEW_ZOOMIN       , tui::TopedFrame::OnzoomIn      )
   EVT_MENU( TMVIEW_ZOOMOUT      , tui::TopedFrame::OnzoomOut     )
   EVT_MENU( TMVIEW_PANLEFT      , tui::TopedFrame::OnpanLeft     )
   EVT_MENU( TMVIEW_PANRIGHT     , tui::TopedFrame::OnpanRight    )
   EVT_MENU( TMVIEW_PANUP        , tui::TopedFrame::OnpanUp       )
   EVT_MENU( TMVIEW_PANDOWN      , tui::TopedFrame::OnpanDown     )
   EVT_MENU( TMVIEW_ZOOMALL      , tui::TopedFrame::OnZoomAll     )
   EVT_MENU( TMVIEW_ZOOMVISIBLE  , tui::TopedFrame::OnZoomVisible )

   EVT_MENU( TMCELL_NEW          , tui::TopedFrame::OnCellNew     )
   EVT_MENU( TMCELL_OPEN         , tui::TopedFrame::OnCellOpen    )
   EVT_MENU( TMCELL_REMOVE       , tui::TopedFrame::OnCellRemove  )
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
   EVT_MENU( TMSEL_REPORT_SLCTD  , tui::TopedFrame::OnReportSelected )

   EVT_MENU( TMSET_STEP          , tui::TopedFrame::OnStep        )
   EVT_MENU( TMSET_AUTOPAN       , tui::TopedFrame::OnAutopan     )
   EVT_MENU( TMSET_ALLPROP       , tui::TopedFrame::OnPropertySheet)
   EVT_MENU( TMSET_GRIDDEF       , tui::TopedFrame::OnGridDefine  )
   EVT_MENU( TMSET_GRID0         , tui::TopedFrame::OnGrid0       )
   EVT_MENU( TMSET_GRID1         , tui::TopedFrame::OnGrid1       )
   EVT_MENU( TMSET_GRID2         , tui::TopedFrame::OnGrid2       )
   EVT_MENU( TMSET_ZEROCROSS     , tui::TopedFrame::OnZeroCross   )

   EVT_MENU( TMSET_MARKER0       , tui::TopedFrame::OnMarker0     )
   EVT_MENU( TMSET_MARKER45      , tui::TopedFrame::OnMarker45    )
   EVT_MENU( TMSET_MARKER90      , tui::TopedFrame::OnMarker90    )
   EVT_MENU( TMSET_CURLONG       , tui::TopedFrame::OnLongCursor  )

   EVT_MENU( TMSET_HTOOLSIZE16   , tui::TopedFrame::OnHToolBarSize16   )
   EVT_MENU( TMSET_HTOOLSIZE24   , tui::TopedFrame::OnHToolBarSize24   )
   EVT_MENU( TMSET_HTOOLSIZE32   , tui::TopedFrame::OnHToolBarSize32   )
   EVT_MENU( TMSET_HTOOLSIZE48   , tui::TopedFrame::OnHToolBarSize48   )

   EVT_MENU( TMSET_VTOOLSIZE16   , tui::TopedFrame::OnVToolBarSize16   )
   EVT_MENU( TMSET_VTOOLSIZE24   , tui::TopedFrame::OnVToolBarSize24   )
   EVT_MENU( TMSET_VTOOLSIZE32   , tui::TopedFrame::OnVToolBarSize32   )
   EVT_MENU( TMSET_VTOOLSIZE48   , tui::TopedFrame::OnVToolBarSize48   )

   EVT_MENU( TMSET_UNDODEPTH     , tui::TopedFrame::OnUndoDepth   )

   EVT_MENU( TMSET_DEFLAY        , tui::TopedFrame::OnDefineLayer )

   EVT_MENU( TMSET_DEFCOLOR      , tui::TopedFrame::OnDefineColor )
   EVT_MENU( TMSET_DEFFILL       , tui::TopedFrame::OnDefineFill  )

   EVT_MENU( TMADD_RULER         , tui::TopedFrame::OnAddRuler    )
   EVT_MENU( TMCLEAR_RULERS      , tui::TopedFrame::OnClearRulers )
   EVT_MENU( TMCADENCE_CONVERT   , tui::TopedFrame::OnCadenceConvert )
   EVT_MENU( TMGET_SNAPSHOT      , tui::TopedFrame::OnTDTSnapshot )
      // EVT_MENU( TMHELP_ABOUTAPP     , tui::TopedFrame::OnAbout       )
   EVT_MENU_RANGE(TMDUMMY, TMDUMMY+TDUMMY_TOOL-1 , tui::TopedFrame::OnMenu  )
   EVT_TOOL_RANGE(TDUMMY_TOOL, TDUMMY_TOOL+1000 , tui::TopedFrame::OnMenu  )
   EVT_CLOSE(tui::TopedFrame::OnClose)
//   EVT_SIZE( TopedFrame::OnSize )
//   EVT_TECUSTOM_COMMAND(  , wxID_ANY, tui::TopedFrame::OnTopedStatus)
   EVT_TECUSTOM_COMMAND(wxEVT_CANVAS_STATUS, wxID_ANY, tui::TopedFrame::OnCanvasStatus)
   EVT_TECUSTOM_COMMAND(wxEVT_SETINGSMENU, wxID_ANY, tui::TopedFrame::OnUpdateSettingsMenu)
   EVT_TECUSTOM_COMMAND(wxEVT_MOUSE_ACCEL, wxID_ANY, tui::TopedFrame::OnMouseAccel)
   EVT_TECUSTOM_COMMAND(wxEVT_CURRENT_LAYER, wxID_ANY, tui::TopedFrame::OnCurrentLayer)
   EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_ENTER, tui::TopedFrame::OnUncapturedMouseClick)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARSIZE, wxID_ANY, tui::TopedFrame::OnToolBarSize)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARDEF,	 wxID_ANY, tui::TopedFrame::OnToolBarDefine)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARADDITEM, wxID_ANY, tui::TopedFrame::OnToolBarAddItem)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARDELETEITEM, wxID_ANY, tui::TopedFrame::OnToolBarDeleteItem)
   EVT_TECUSTOM_COMMAND(wxEVT_EDITLAYER, wxID_ANY, tui::TopedFrame::OnEditLayer )
   EVT_TEXT_MAXLEN(ID_WIN_TXT_LOG, tui::TopedFrame::OnTextLogOverflow)
END_EVENT_TABLE()

// See the FIXME note in the bootom of browsers.cpp
//   EVT_COMMAND(wxID_ANY, wxEVT_INIT_DIALOG , tui::TopedFrame::OnDefineLayer )

tui::TopedFrame::TopedFrame(const wxString& title, const wxPoint& pos,
                            const wxSize& size ) : wxFrame((wxFrame *)NULL, ID_WIN_TOPED, title, pos, size)
{
   SetIcon(wxICON(toped16x16));
   initView();
   wxCommandEvent dummy;
   OnzoomEmpty(dummy);
   SetStatusBar(DEBUG_NEW console::TopedStatus(this));
   _resourceCenter = DEBUG_NEW ResourceCenter;
   SetStatusText( wxT( "Toped loaded..." ) );
   //Put initMenuBar() at the end because in Windows it crashes
   initMenuBar();
   wxToolTip::Enable(true);
   wxToolTip::SetDelay(3000);
   _propDialog = DEBUG_NEW tui::TopedPropertySheets(this);
   // Initialize the post system
   _tPost = DEBUG_NEW TpdPost(this);
}

void tui::TopedFrame::OnClose(wxCloseEvent& event)
{
   if (event.CanVeto())
   {
      if (DATC->modified()) {
         wxMessageDialog dlg1(this,
                              wxT("Save the current design before closing?\n(Cancel to continue the session)"),
                              wxT("Current design contains unsaved data"),
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
//   delete _laycanvas;
//   delete _cmdline;
//   delete _GLstatus;
//   delete _browsers;
    _winManager.UnInit();
   delete _propDialog;
   delete _canvas;
   delete _resourceCenter;
   delete _tPost;
//   delete _toped_status;
}

void tui::TopedFrame::initMenuBar() {
   //---------------------------------------------------------------------------
   // menuBar entry fileMenu
   /*gdsMenu=DEBUG_NEW wxMenu();
   gdsMenu->Append(TMGDS_OPEN   , wxT("parse")  , wxT("Parse GDS file"));
   gdsMenu->Append(TMGDS_TRANSLATE , wxT("translate to library") , wxT("Import GDS structure"));
   gdsMenu->Append(TMGDS_EXPORTC, wxT("export cell") , wxT("Export cell to GDS"));
   gdsMenu->Append(TMGDS_CLOSE  , wxT("close")  , wxT("Clear the parsed GDS file from memory"));
   */


   /*fileMenu=DEBUG_NEW wxMenu();
   fileMenu->Append(TMFILE_NEW   , wxT("New ...\tCTRL-N")    , wxT("Create DEBUG_NEW design"));
   fileMenu->Append(TMFILE_OPEN  , wxT("Open ...\tCTRL-O")   , wxT("Open a TDT file"));
   fileMenu->Append(TMFILE_INCLUDE, wxT("Include ...")       , wxT("Include a TELL file"));
   fileMenu->AppendSeparator();
   fileMenu->Append(TMGDS_EXPORTL, wxT("Export library to GDS") , wxT("Export library to GDS"));
   fileMenu->Append(TMGDS_IMPORT , wxT("Import GDS to library") , wxT("Import GDS structure"));
   //fileMenu->Append(TMGDS_MENU   , wxT("Advanced GDS operations") , gdsMenu , wxT("More granulated GDS related functions"));
   fileMenu->AppendSeparator();
   fileMenu->Append(TMFILE_SAVE  , wxT("Save\tCTRL-S")       , wxT("Save the database"));
   fileMenu->Append(TMFILE_SAVEAS, wxT("Save as ..."), wxT("Save the database under a DEBUG_NEW name"));
   fileMenu->AppendSeparator();
   fileMenu->Append(TMFILE_EXIT  , wxT("Exit")       , wxT("Exit Toped"));*/

   menuBar = DEBUG_NEW wxMenuBar();
   SetMenuBar( menuBar );

   _resourceCenter->appendMenu("&File/New ...",    "CTRL-N", &tui::TopedFrame::OnNewDesign,  "Create new design");
   _resourceCenter->appendMenu("&File/Open ...",   "CTRL-O", &tui::TopedFrame::OnTDTRead, "Open a TDT file" );
   _resourceCenter->appendMenu("&File/Include ...","",      &tui::TopedFrame::OnTELLRead, "Include a TELL file" );
   _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Load Library ...",  "", &tui::TopedFrame::OnTDTLoadLib, "Load a TDT library" );
   _resourceCenter->appendMenu("&File/Unload Library ...",  "", &tui::TopedFrame::OnTDTUnloadLib, "Unload a TDT library" );
   _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Export to GDS","",  &tui::TopedFrame::OnGDSexportLIB, "Export DB to GDS using default layer map");
   _resourceCenter->appendMenu("&File/Import GDS","",  &tui::TopedFrame::OnGDSimport, "Import GDS file using default layer map" );

   _resourceCenter->appendMenu("&File/More GDS .../Parse","", &tui::TopedFrame::OnGDSRead, "Parse GDS file" );
   _resourceCenter->appendMenu("&File/More GDS .../Translate","", &tui::TopedFrame::OnGDStranslate, "Import GDS structure" );
   _resourceCenter->appendMenu("&File/More GDS .../Export Cell", "", &tui::TopedFrame::OnGDSexportCELL, "Export cell to GDS" );
   _resourceCenter->appendMenu("&File/More GDS .../Close","", &tui::TopedFrame::OnGDSclose, "Clear the parsed GDS file from memory" );

   _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Export to CIF","",  &tui::TopedFrame::OnCIFexportLIB, "Export DB to CIF using the default layer map");
   _resourceCenter->appendMenu("&File/Import CIF","",  &tui::TopedFrame::OnCIFimport, "Import CIF file using default layer map" );

   _resourceCenter->appendMenu("&File/More CIF .../Parse","", &tui::TopedFrame::OnCIFRead, "Parse CIF file" );
   _resourceCenter->appendMenu("&File/More CIF .../Translate","", &tui::TopedFrame::OnCIFtranslate, "Import CIF structure" );
   _resourceCenter->appendMenu("&File/More CIF .../Export Cell", "", &tui::TopedFrame::OnCIFexportCELL, "Export cell to CIF" );
   _resourceCenter->appendMenu("&File/More CIF .../Close","", &tui::TopedFrame::OnCIFclose, "Clear the parsed CIF file from memory" );


   _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Export to Oasis","",  &tui::TopedFrame::OnOASexportLIB, "Export DB to Oasis using default layer map");
   _resourceCenter->appendMenu("&File/Import Oasis","",  &tui::TopedFrame::OnOASimport, "Import Oasis file using default layer map" );

   _resourceCenter->appendMenu("&File/More Oasis .../Parse","", &tui::TopedFrame::OnOASRead, "Parse Oasis file" );
   _resourceCenter->appendMenu("&File/More Oasis .../Translate","", &tui::TopedFrame::OnOAStranslate, "Import Oasis structure" );
   _resourceCenter->appendMenu("&File/More Oasis .../Export Cell", "", &tui::TopedFrame::OnOASexportCELL, "Export cell to Oasis" );
   _resourceCenter->appendMenu("&File/More Oasis .../Close","", &tui::TopedFrame::OnOASclose, "Clear the parsed Oasis file from memory" );


   _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Save",       "CTRL-S",  &tui::TopedFrame::OnTDTSave,  "Save the database");
   _resourceCenter->appendMenu("&File/Save as ...","",  &tui::TopedFrame::OnTDTSaveAs, "Save the database under a new name" );
   _resourceCenter->appendMenu("&File/Save properties...","",  &tui::TopedFrame::OnPropSave, "Save the layout properties" );
   _resourceCenter->appendMenuSeparator("&File");
  // _resourceCenter->appendMenu("&File/Snapshot ...","",  &tui::TopedFrame::OnTDTSnapshot, "Export screen to picture" );
  // _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Exit",        "",  &tui::TopedFrame::OnQuit, "Exit Toped" );


   //---------------------------------------------------------------------------
   // menuBar entry editMenu
   /*editMenu=DEBUG_NEW wxMenu();
   editMenu->Append(TMEDIT_UNDO  , wxT("Undo\tCTRL-Z")  , wxT("Undo last operation"));
   editMenu->AppendSeparator();
   editMenu->Append(TMEDIT_COPY  , wxT("Copy\tCTRL-C")  , wxT("Copy selected shapes"));
   editMenu->Append(TMEDIT_MOVE  , wxT("Move\tCTRL-M")  , wxT("Move selected shapes"));
   editMenu->Append(TMEDIT_DELETE, wxT("Delete\tCTRL-D"), wxT("Delete selected shapes"));
   editMenu->AppendSeparator();
   editMenu->Append(TMEDIT_ROTATE90, wxT("Rotate 90\tCTRL-9"), wxT("Rotate selected shapes on 90 deg. counter clockwise "));
   editMenu->Append(TMEDIT_FLIPX, wxT("Flip X\tCTRL-X"), wxT("Flip selected shapes towards X axis "));
   editMenu->Append(TMEDIT_FLIPY, wxT("Flip Y\tCTRL-Y"), wxT("Flip selected shapes towards Y axis "));
   editMenu->Append(TMEDIT_POLYCUT, wxT("Cut with poly\tCTRL-U"), wxT("Cut selected shapes with a polygon "));
   editMenu->Append(TMEDIT_MERGE, wxT("Merge\tCTRL-G"), wxT("Merge selected shpes"));
   */
   _resourceCenter->appendMenu("&Edit/Undo",       "CTRL-Z",  &tui::TopedFrame::OnUndo, "Undo last operation" );
   _resourceCenter->appendMenuSeparator("Edit");
   _resourceCenter->appendMenu("&Edit/Copy",       "CTRL-C",  &tui::TopedFrame::OnCopy, "Copy selected shapes" );
   _resourceCenter->appendMenu("&Edit/Move",       "CTRL-M",  &tui::TopedFrame::OnMove, "Move selected shapes" );
   _resourceCenter->appendMenu("&Edit/Delete",     "CTRL-D",  &tui::TopedFrame::OnDelete, "Delete selected shapes" );
   _resourceCenter->appendMenuSeparator("Edit");
   _resourceCenter->appendMenu("&Edit/Rotate 90",  "CTRL-9",  &tui::TopedFrame::OnRotate, "Rotate selected shapes on 90 deg. counter clockwise ");
   _resourceCenter->appendMenu("&Edit/Flip Vertical",     "CTRL-X",  &tui::TopedFrame::OnFlipVert, "Flip selected shapes towards X axis " );
   _resourceCenter->appendMenu("&Edit/Flip Horizontal",     "CTRL-Y",  &tui::TopedFrame::OnFlipHor, "Flip selected shapes towards Y axis " );
   _resourceCenter->appendMenu("&Edit/Cut with poly","CTRL-U",  &tui::TopedFrame::OnPolyCut, "Cut selected shapes with a polygon " );
   _resourceCenter->appendMenu("&Edit/Cut with box","CTRL-ALT-U", &tui::TopedFrame::OnBoxCut, "Cut selected shapes with a box " );
   _resourceCenter->appendMenu("&Edit/Merge",      "CTRL-G",  &tui::TopedFrame::OnMerge, "Merge selected shapes" );
   _resourceCenter->appendMenu("&Edit/Resize",           "",  &tui::TopedFrame::OnResize, "Blow/shrink selected shpes" );
   _resourceCenter->appendMenuSeparator("Edit");
   _resourceCenter->appendMenu("&Edit/Change Layer","",&tui::TopedFrame::OnChangeLayer, "Translate the objects to another layer" );
   _resourceCenter->appendMenu("&Edit/Change Text","",&tui::TopedFrame::OnChangeText, "Replace a text contents" );

   //---------------------------------------------------------------------------
   // menuBar entry viewMenu
   /*viewMenu=DEBUG_NEW wxMenu();
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
   viewMenu->Check(TMVIEW_VIEWSTATUSBAR, true);*/

    //???Add Toolbar & StatusBar check Item

   _resourceCenter->appendMenu("&View/Zoom in", "F2",  &tui::TopedFrame::OnzoomIn, "Zoom in current window" );
   _resourceCenter->appendMenu("&View/Zoom out","F3",  &tui::TopedFrame::OnzoomOut, "Zoom out current window" );
   _resourceCenter->appendMenu("&View/Zoom all","F4",  &tui::TopedFrame::OnZoomAll, "Zoom the current cell" );
   _resourceCenter->appendMenu("&View/Zoom visible","F5",  &tui::TopedFrame::OnZoomVisible, "Zoom visible objects of the current cell" );
   _resourceCenter->appendMenuSeparator("View");
   _resourceCenter->appendMenu("&View/Pan left",   "SHIFT-LEFT",  &tui::TopedFrame::OnpanLeft, "Move the view window left" );
   _resourceCenter->appendMenu("&View/Pan right",  "SHIFT-RIGHT", &tui::TopedFrame::OnpanRight, "Move the view window right" );
   _resourceCenter->appendMenu("&View/Pan up",     "SHIFT-UP",    &tui::TopedFrame::OnpanUp, "Move the view window up" );
   _resourceCenter->appendMenu("&View/Pan down",   "SHIFT-DOWN",  &tui::TopedFrame::OnpanDown, "Move the view window down" );
   _resourceCenter->appendMenuSeparator("View");

   //---------------------------------------------------------------------------
   // menuBar entry Cell
   /*cellMenu=DEBUG_NEW wxMenu();
   cellMenu->Append(TMCELL_NEW      , wxT("New Cell") , wxT("Create a new cell"));
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
   */
   _resourceCenter->appendMenu("&Cell/New Cell",      "",   &tui::TopedFrame::OnCellNew, "Create a DEBUG_NEW cell" );
   _resourceCenter->appendMenu("&Cell/Open Cell",     "",   &tui::TopedFrame::OnCellOpen,    "Open existing cell for editing" );
   _resourceCenter->appendMenu("&Cell/Remove Cell",   "",   &tui::TopedFrame::OnCellRemove,  "Remove existing cell" );
   _resourceCenter->appendMenuSeparator("Cell");
   _resourceCenter->appendMenu("&Cell/Edit Push",     "F9", &tui::TopedFrame::OnCellPush, "Edit in place" );
   _resourceCenter->appendMenu("&Cell/Edit Previous", "Ctrl-F9",  &tui::TopedFrame::OnCellPrev, "Edit in place" );
   _resourceCenter->appendMenu("&Cell/Edit Pop",      "F10",&tui::TopedFrame::OnCellPop, "Edit in place" );
   _resourceCenter->appendMenu("&Cell/Edit Top","Ctrl-F10", &tui::TopedFrame::OnCellTop, "Edit in place" );
   _resourceCenter->appendMenuSeparator("Cell");
   _resourceCenter->appendMenu("&Cell/Cell Reference",      "Ctrl-R",   &tui::TopedFrame::OnCellRef_M, "Cell reference" );
   _resourceCenter->appendMenu("&Cell/Array of References", "Alt-R",    &tui::TopedFrame::OnCellARef_M,  "Array of cell references");
   _resourceCenter->appendMenuSeparator("Cell");
   _resourceCenter->appendMenu("&Cell/Group Cell",    "",  &tui::TopedFrame::OnCellGroup, "Group selected shapes in a cell" );
   _resourceCenter->appendMenu("&Cell/Unroup Cell",   "",  &tui::TopedFrame::OnCellUngroup, "Ungroup selected cell references" );
   _resourceCenter->appendMenuSeparator("Cell");
   _resourceCenter->appendMenu("&Cell/Change Reference","",&tui::TopedFrame::OnChangeRef, "Replace a cell reference" );

   //---------------------------------------------------------------------------
   // menuBar entry Draw
   /*drawMenu=DEBUG_NEW wxMenu();
   drawMenu->Append(TMDRAW_BOX , wxT("Box\tCTRL-B")     , wxT("Create new box on the current layer"));
   drawMenu->Append(TMDRAW_POLY, wxT("Polygon\tCTRL-L") , wxT("Create new polygon on the current layer"));
   drawMenu->Append(TMDRAW_WIRE, wxT("Wire ...\tCTRL-W"), wxT("Create new wire on the current layer"));
   drawMenu->Append(TMDRAW_TEXT, wxT("Text ...\tCTRL-T"), wxT("Add text on the current layer"));
   */
   _resourceCenter->appendMenu("&Draw/Box",      "CTRL-B", &tui::TopedFrame::OnDrawBox, "Create new box on the current layer" );
   _resourceCenter->appendMenu("&Draw/Polygon",  "CTRL-L", &tui::TopedFrame::OnDrawPoly, "Create new polygon on the current layer" );
   _resourceCenter->appendMenu("&Draw/Wire ...", "CTRL-W", &tui::TopedFrame::OnDrawWire, "Create new wire on the current layer" );
   _resourceCenter->appendMenu("&Draw/Text ...", "CTRL-T", &tui::TopedFrame::OnDrawText, "Add text on the current layer" );


   //---------------------------------------------------------------------------
   // menuBar entry Modify
   /*selectMenu=DEBUG_NEW wxMenu();
   selectMenu->Append(TMSEL_SELECT_IN   , wxT("Select\tCTRL-I")        , wxT("Select objects"));
   selectMenu->Append(TMSEL_PSELECT_IN  , wxT("Part select\tCTRL-P")  , wxT("Select object edges"));
   selectMenu->Append(TMSEL_SELECT_ALL  , wxT("Select all\tCTRL-A")    , wxT("Select all objects in the current cell"));
   selectMenu->AppendSeparator();
   selectMenu->Append(TMSEL_UNSELECT_IN , wxT("Unselect\tALT-I")      , wxT("Unselect objects"));
   selectMenu->Append(TMSEL_PUNSELECT_IN, wxT("Part unselect\tALT-P"), wxT("Unselect object edges"));
   selectMenu->Append(TMSEL_UNSELECT_ALL, wxT("Unselect all\tALT-A")  , wxT("Unselect all"));
   */

   _resourceCenter->appendMenu("&Select/Select",      "CTRL-I", &tui::TopedFrame::OnSelectIn, "Select objects"  );
   _resourceCenter->appendMenu("&Select/Part select", "CTRL-P", &tui::TopedFrame::OnPselectIn, "Select object edges" );
   _resourceCenter->appendMenu("&Select/Select all",  "CTRL-A", &tui::TopedFrame::OnSelectAll, "Select all objects in the current cell" );
   _resourceCenter->appendMenuSeparator("Select");
   _resourceCenter->appendMenu("&Select/Unselect",    "ALT-I", &tui::TopedFrame::OnUnselectIn, "Unselect objects" );
   _resourceCenter->appendMenu("&Select/Part unselect","ALT-P", &tui::TopedFrame::OnPunselectIn, "Unselect object edges" );
   _resourceCenter->appendMenu("&Select/Unselect all", "ALT-A", &tui::TopedFrame::OnUnselectAll, "Unselect all" );
   _resourceCenter->appendMenuSeparator("Select");
   _resourceCenter->appendMenu("&Select/Report selected",   "", &tui::TopedFrame::OnReportSelected, "Report selected objects" );

   //---------------------------------------------------------------------------
   // menuBar entry Settings
   // first the sub menu
   markerMenu=DEBUG_NEW wxMenu();
   markerMenu->AppendRadioItem(TMSET_MARKER0    , wxT("Free")      , wxT("Marker is not restricted"));
   markerMenu->AppendRadioItem(TMSET_MARKER45   , wxT("45 degrees"), wxT("Restrict shape angles to 45 deg"));
   markerMenu->AppendRadioItem(TMSET_MARKER90   , wxT("Orthogonal"), wxT("Restrict shape angles to 90 deg"));

   //Toolbar Size sub menu
   toolbarHorSizeMenu = DEBUG_NEW wxMenu();
   toolbarHorSizeMenu->AppendRadioItem(TMSET_HTOOLSIZE16   ,   wxT("16x16"), wxT("Toolbar size is 16x16"));
   toolbarHorSizeMenu->AppendRadioItem(TMSET_HTOOLSIZE24   ,   wxT("24x24"), wxT("Toolbar size is 24x24"));
   toolbarHorSizeMenu->AppendRadioItem(TMSET_HTOOLSIZE32   ,   wxT("32x32"), wxT("Toolbar size is 32x32"));
   toolbarHorSizeMenu->AppendRadioItem(TMSET_HTOOLSIZE48   ,   wxT("48x48"), wxT("Toolbar size is 48x48"));

   //toolbarVertSizeMenu = DEBUG_NEW wxMenu();
   //toolbarVertSizeMenu->AppendRadioItem(TMSET_VTOOLSIZE16   ,   wxT("16x16"), wxT("Vertical toolbars size is 16x16"));
   //toolbarVertSizeMenu->AppendRadioItem(TMSET_VTOOLSIZE24   ,   wxT("24x24"), wxT("Vertical toolbars size is 24x24"));
   //toolbarVertSizeMenu->AppendRadioItem(TMSET_VTOOLSIZE32   ,   wxT("32x32"), wxT("Vertical toolbars size is 32x32"));
   //toolbarVertSizeMenu->AppendRadioItem(TMSET_VTOOLSIZE48   ,   wxT("48x48"), wxT("Vertical toolbars size is 48x48"));
   // now the setting menu itself
   settingsMenu=DEBUG_NEW wxMenu();
   settingsMenu->Append         (TMSET_STEP     , wxT("Step")      , wxT("Select objects"));
   settingsMenu->AppendCheckItem(TMSET_AUTOPAN  , wxT("Auto Pan")  , wxT("Automatic window move"));
   settingsMenu->Append         (TMSET_ALLPROP  , wxT("Properties..."), wxT("Toped Properties"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_GRIDDEF  , wxT("Define grid"), wxT("Define/change the grid step"));
   settingsMenu->AppendCheckItem(TMSET_GRID0    , wxT("Grid 0")    , wxT("Draw/Hide Grid 0"));
   settingsMenu->AppendCheckItem(TMSET_GRID1    , wxT("Grid 1")    , wxT("Draw/Hide Grid 1"));
   settingsMenu->AppendCheckItem(TMSET_GRID2    , wxT("Grid 2")    , wxT("Draw/Hide Grid 2"));
   settingsMenu->AppendSeparator();
   settingsMenu->AppendCheckItem(TMSET_ZEROCROSS, wxT("Zero Cross"), wxT("Draw/Hide Zero Cross mark"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_MARKER   , wxT("Marker") , markerMenu , wxT("Define marker movement"));
   settingsMenu->AppendCheckItem(TMSET_CURLONG  , wxT("Long cursor")  , wxT("Stretch the cursor cross"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_HTOOLSIZE   , wxT("Toolbar size") , toolbarHorSizeMenu , wxT("Define toolbars size"));
   settingsMenu->Append         (TMSET_UNDODEPTH   , wxT("Undo Depth")   , wxT("Change the depth of undo stack"));
   //settingsMenu->Append         (TMSET_HTOOLSIZE   , wxT("H. toolbar size") , toolbarHorSizeMenu , wxT("Define horizontal toolbars size"));
   //settingsMenu->Append         (TMSET_VTOOLSIZE   , wxT("V. toolbar size") , toolbarVertSizeMenu , wxT("Define vertical toolbars size"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_DEFLAY   , wxT("Define Layer") , wxT("Define a layer"));
   settingsMenu->Append         (TMSET_DEFCOLOR , wxT("Define Color") , wxT("Define a drawing color"));
   settingsMenu->Append         (TMSET_DEFFILL  , wxT("Define Fill")  , wxT("Define a drawing pattern"));
   //---------------------------------------------------------------------------
   // menuBar entry helpMenu
   /*helpMenu=DEBUG_NEW wxMenu();
   helpMenu->Append(TMHELP_ABOUTAPP       , wxT("About")          , wxT("About TOPED"));
   */
   _resourceCenter->appendMenu("&Other/Add Ruler"   , "", &tui::TopedFrame::OnAddRuler, "Add new ruler" );
   _resourceCenter->appendMenu("&Other/Clear Rulers", "", &tui::TopedFrame::OnClearRulers, "Clear all rulers" );

	_resourceCenter->appendMenuSeparator("&Other");
	_resourceCenter->appendMenu("&Other/Cadence converter ...", "", &tui::TopedFrame::OnCadenceConvert, "Convert Cadence techfiles" );

//   _resourceCenter->appendMenuSeparator("Other");

   _resourceCenter->appendMenu("&Other/Get Snapshot", "", &tui::TopedFrame::OnTDTSnapshot, "Get a snapshot of the canvas on TGA file");
   _resourceCenter->appendMenu("&Other/Load DRC results ...", "", &tui::TopedFrame::OnDRCResults, "Load DRC results");

	//   _resourceCenter->appendMenuSeparator("Other");

   _resourceCenter->appendMenu("&Help/Report Video", "", &tui::TopedFrame::OnCheckHW, "Display OpenGL & video driver information" );
   _resourceCenter->appendMenuSeparator("Help");
   _resourceCenter->appendMenu("&Help/About", "", &tui::TopedFrame::OnAbout, "About TOPED" );
   //---------------------------------------------------------------------------
   // MENUBAR CONFIGURATION
   //menuBar = DEBUG_NEW wxMenuBar();
   //menuBar->Append(fileMenu      , wxT("&File"  ));
   //menuBar->Append(editMenu      , wxT("&Edit"  ));
   //menuBar->Append(viewMenu      , wxT("&View"  ));
   //menuBar->Append(cellMenu      , wxT("&Cell"  ));
   //menuBar->Append(drawMenu      , wxT("&Draw"  ));
   //menuBar->Append(selectMenu    , wxT("&Select"));
    //menuBar->Append(helpMenu      , wxT("&Help"  ));
   //SetMenuBar( menuBar );
   // default menu values
   //settingsMenu->Check(TMSET_CELLMARK,true);
   //settingsMenu->Check(TMSET_TEXTMARK,true);
   _resourceCenter->buildMenu(menuBar);

   menuBar->Insert(menuBar->GetMenuCount()-1,settingsMenu  , wxT("Se&ttings"));

}

void tui::TopedFrame::setIconDir(const std::string& uiDir)
{
   if (_resourceCenter) _resourceCenter->setIconDir(uiDir);
}

void tui::TopedFrame::initToolBars()
{
   _resourceCenter->setDirection(wxAUI_DOCK_TOP);
   _resourceCenter->appendTool("main", "new", "new", "", "new cell", &tui::TopedFrame::OnCellNew);
   _resourceCenter->appendTool("main", "open", "open", "", "open cell", &tui::TopedFrame::OnCellOpen);
   _resourceCenter->appendTool("main", "save", "save", "", "save design", &tui::TopedFrame::OnTDTSave);

   //_resourceCenter->setToolBarSize("main", ICON_SIZE_24x24);
   _resourceCenter->setDirection(wxAUI_DOCK_TOP);

	_resourceCenter->appendTool("edit", "undo", "undo", "", "undo", &tui::TopedFrame::OnUndo);
   //_resourceCenter->appendTool("edit", "redo", "redo", "", "redo",&tui::TopedFrame::OnUndo);
   _resourceCenter->appendTool("edit", "box", "box", "", "add box",&tui::TopedFrame::OnDrawBox);
   _resourceCenter->appendTool("edit", "poly", "poly", "", "add polygon",&tui::TopedFrame::OnDrawPoly);
   _resourceCenter->appendTool("edit", "wire", "wire", "", "add wire",&tui::TopedFrame::OnDrawWire);
   _resourceCenter->appendTool("edit", "text", "text", "", "add text",&tui::TopedFrame::OnDrawText);
   _resourceCenter->appendTool("edit", "delete", "delete", "", "delete",&tui::TopedFrame::OnDelete);
   _resourceCenter->appendTool("edit", "cut_with_poly", "cut_with_poly", "", "cut",&tui::TopedFrame::OnPolyCut);
   _resourceCenter->appendTool("edit", "zoom_all", "zoom_all", "", "zoom all",&tui::TopedFrame::OnZoomAll);
   _resourceCenter->appendTool("edit", "zoom_in", "zoom_in", "", "zoom in",&tui::TopedFrame::OnzoomIn);
   _resourceCenter->appendTool("edit", "zoom_out", "zoom_out", "", "zoom out",&tui::TopedFrame::OnzoomOut);
   _resourceCenter->appendTool("edit", "ruler", "ruler", "", "add ruler",&tui::TopedFrame::OnAddRuler);
   _resourceCenter->appendTool("edit", "copy", "copy", "", "copy",&tui::TopedFrame::OnCopy);
   _resourceCenter->appendTool("edit", "move", "move", "", "move",&tui::TopedFrame::OnMove);
   _resourceCenter->appendTool("edit", "rotate", "rotate_left", "", "rotate",&tui::TopedFrame::OnRotate);
   //_resourceCenter->appendTool("edit", "rotate_right", "rotate_right.png", "", "rotate",&tui::TopedFrame::OnRotate);
   _resourceCenter->appendTool("edit", "flipvert", "flipy", "", "flip vertical",&tui::TopedFrame::OnFlipVert);
   _resourceCenter->appendTool("edit", "fliphor", "flipx", "", "flip horizontal",&tui::TopedFrame::OnFlipHor);
   _resourceCenter->appendTool("edit", "edit_push", "edit_push", "", "edit push",&tui::TopedFrame::OnCellPush);
   _resourceCenter->appendTool("edit", "edit_pop", "edit_pop", "", "edit pop",&tui::TopedFrame::OnCellPop);

   //_resourceCenter->setToolBarSize(_tuihorizontal, ICON_SIZE_16x16);
  // _resourceCenter->setToolBarSize(_tuihorizontal, ICON_SIZE_32x32);
   _status = DEBUG_NEW wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER|wxTB_HORIZONTAL);

   _GLstatus = DEBUG_NEW CanvasStatus(_status, ID_WIN_GLSTATUS ,
                                          wxDefaultPosition, wxDefaultSize,
                                          wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
   _GLstatus->SetSize(wxSize(-1, 30));

   _status->AddControl((wxControl*)_GLstatus);
   _status->Realize();

   getAuiManager()->AddPane(_status, wxAuiPaneInfo().ToolbarPane().
                           Name(wxString("Status", wxConvUTF8)).Top().Gripper().GripperTop(false).Floatable(false).
                           TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false).Row(2));

   getAuiManager()->Update();
/*   wxToolBar* positionBar = CreateToolBar(wxTB_DOCKABLE |  wxTB_HORIZONTAL | wxNO_BORDER);
   X_pos = DEBUG_NEW wxStaticText(positionBar, -1, "", wxDefaultPosition,
   id   wxSize(100,32), wxST_NO_AUTORESIZE);
   Y_pos = DEBUG_NEW wxStaticText(positionBar, -1, "", wxDefaultPosition,
                              wxSize(100,32), wxST_NO_AUTORESIZE);
//wxSIMPLE_BORDER | wxALIGN_RIGHT  |
   wxFont fontX = X_pos->GetFont();
   fontX.SetPointSize(fontX.GetPointSize()*3/2);
   X_pos->SetFont(fontX);
   Y_pos->SetFont(fontX);

   X_pos->SetBackgroundColoidur(wxColour("WHITE"));
   Y_pos->SetBackgroundColour(wxColour("WHITE"));
   X_pos->SetLabel("0.00");
   Y_pos->SetLabel("0.00");
   X_pos->Refresh();
   Y_pos->Refresh();
   positionBar->AddControl(X_pos);
   positionBar->AddSeparator();
   positionBar->AddControl(Y_pos);
   positionBar->AddSeparator();
   positionBar->Realize();*/
}

void tui::TopedFrame::initView()
{
   _winManager.SetManagedWindow(this);
   //----------------------------------------------------------------------------
   //the cell & layer browsers
   //----------------------------------------------------------------------------
   _browsers = DEBUG_NEW browsers::browserTAB(this,ID_WIN_BROWSERS,
                                        wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN);
//   _browsers->SetSize(wxSize(180, 1000));
   _browsers->SetArtProvider(DEBUG_NEW wxAuiSimpleTabArt);
   //----------------------------------------------------------------------------
   //the log & lib windows
   //----------------------------------------------------------------------------
   wxAuiNotebook* logpane = DEBUG_NEW wxAuiNotebook(this, ID_WIN_LOG_PANE,
         wxDefaultPosition, wxDefaultSize, wxNB_RIGHT|wxNO_BORDER);
         //wxAUI_NB_DEFAULT_STYLE |wxNB_RIGHT| wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
//   logpane->SetSize(wxSize(1000, 150));

   _cmdlog = DEBUG_NEW console::ted_log(logpane, ID_WIN_TXT_LOG);
   logpane->AddPage(_cmdlog, wxT("Log"));
   _cmdbrowser = DEBUG_NEW console::TELLFuncList(logpane, ID_TELL_FUNCS);
   logpane->AddPage(_cmdbrowser, wxT("Lib"));
   logpane->SetArtProvider(DEBUG_NEW wxAuiSimpleTabArt);
   //----------------------------------------------------------------------------
   // the openGL window - the canvas
   //----------------------------------------------------------------------------
   int gl_attrib[] = {
                        WX_GL_RGBA             ,
                        WX_GL_MIN_RED          , 8,
                        WX_GL_MIN_GREEN        , 8,
                        WX_GL_MIN_BLUE         , 8,
                        WX_GL_MIN_ALPHA        , 8,
                        WX_GL_MIN_ACCUM_RED    , 8,
                        WX_GL_MIN_ACCUM_GREEN  , 8,
                        WX_GL_MIN_ACCUM_BLUE   , 8,
                        WX_GL_MIN_ACCUM_ALPHA  , 8,
                        WX_GL_DOUBLEBUFFER     ,
                        GL_NONE };

   _canvas = DEBUG_NEW LayoutCanvas(this, wxDefaultPosition, wxDefaultSize, gl_attrib);

   TeselPoly::tenderTesel = gluNewTess();
#ifndef WIN32
   gluTessCallback(TeselPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                   (GLvoid(*)())&TeselPoly::teselBegin);
   gluTessCallback(TeselPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                   (GLvoid(*)())&TeselPoly::teselVertex);
   gluTessCallback(TeselPoly::tenderTesel, GLU_TESS_END_DATA,
                   (GLvoid(*)())&TeselPoly::teselEnd);
#else
   gluTessCallback(TeselPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                   (GLvoid(__stdcall *)())&TeselPoly::teselBegin);
   gluTessCallback(TeselPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                   (GLvoid(__stdcall *)())&TeselPoly::teselVertex);
   gluTessCallback(TeselPoly::tenderTesel, GLU_TESS_END_DATA,
                   (GLvoid(__stdcall *)())&TeselPoly::teselEnd);
#endif
   //----------------------------------------------------------------------------
   // The command line
   //----------------------------------------------------------------------------
   _cmdline = DEBUG_NEW console::ted_cmd(this, _canvas);
   _cmdline->SetSize(wxSize(wxSize(1000, 30)));
// _cmdline->SetWindowStyleFlag(wxSW_3D | wxCLIP_CHILDREN);

   _winManager.AddPane(_browsers,   wxAuiPaneInfo().
                                    Left().
                                    BestSize(wxSize(180,1000)).
                                    Layer(1).
                                    Caption(wxString("Project Info",wxConvUTF8)).
                                    Floatable(true).
                                    CloseButton(false).
                                    CaptionVisible(true).
                                    TopDockable(false).
                                    BottomDockable(false).
                                    LeftDockable(true).
                                    RightDockable(true)
                      );
   if (_canvas->initStatus())
      _winManager.AddPane( _canvas,    wxAuiPaneInfo().
                                       CentrePane().
                                       MaximizeButton(false).
                                       Floatable(true).
                                       CloseButton(false)
                        );
   //_winManager.AddPane(_GLstatus, wxAuiPaneInfo().Top().Floatable(false).//Fixed().
   //                               CloseButton(false).CaptionVisible(false).BestSize(wxSize(1000,30)));
   _winManager.AddPane(logpane,     wxAuiPaneInfo().
                                    Bottom().
                                    Row(1).
                                    Floatable(false).
                                    BestSize(wxSize(1000, 150)).
                                    CloseButton(false).
                                    CaptionVisible(false)
                      );

   _winManager.AddPane(_cmdline,    wxAuiPaneInfo().
                                    Bottom().
                                    Row(0).
                                    BestSize(wxSize(1000,30)).
                                    Floatable(false).
                                    CloseButton(false).
                                    CaptionVisible(false)
                      );
   Show();
   _winManager.Update();
}


void tui::TopedFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) ) {
   Close(FALSE);
}

void tui::TopedFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) ) {
    wxAboutDialogInfo info;
    info.SetName(wxT("Toped"));
    info.SetVersion(wxT("0.9.x"));
    info.SetIcon(wxIcon( toped32x32_xpm ));
    info.SetWebSite(wxT("www.toped.org.uk"));
    info.SetDescription(wxT("Open source IC layout editor"));
    info.SetCopyright(wxT("(C) 2001-2009 Toped developers"));

    wxAboutBox(info);
}

void tui::TopedFrame::OnCheckHW(wxCommandEvent&)
{
   _canvas->showInfo();
}

//void tui::TopedFrame::OnSize(wxSizeEvent& event)
//{
// event.Skip();
   //wxLayoutAlgorithm layout;
   //layout.LayoutFrame(this, mS_canvas);
//}

void tui::TopedFrame::OnCanvasStatus(wxCommandEvent& evt)
{
   switch (evt.GetInt()) {
      case CNVS_POS_X       : _GLstatus->setXpos(evt.GetString()); break;
      case CNVS_POS_Y       : _GLstatus->setYpos(evt.GetString()); break;
      case CNVS_DEL_X       : _GLstatus->setdXpos(evt.GetString()); break;
      case CNVS_DEL_Y       : _GLstatus->setdYpos(evt.GetString()); break;
      case CNVS_SELECTED    : _GLstatus->setSelected(evt.GetString());break;
      default: assert(false);
   }
}

void tui::TopedFrame::OnNewDesign(wxCommandEvent& evt) {
   if (DATC->modified()) {
      wxMessageDialog dlg1(this,
         wxT("Current design contains unsaved data"),
         wxT("Save the current design before creating a new one?"),
         wxYES_NO | wxCANCEL | wxICON_QUESTION);
      switch (dlg1.ShowModal()) {
         case wxID_YES:OnTDTSave(evt);
         case wxID_NO: break;
         case wxID_CANCEL: return;
      }
   }
   wxTextEntryDialog dlg2(this,
      wxT("Name the new design"),
      wxT("Design name:"));
   wxString dname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((dname = dlg2.GetValue()) != wxT(""))) {
      SetStatusText(wxT("Creating new file..."));
      ost << wxT("newdesign(\"") << dname << wxT("\");");
      _cmdline->parseCommand(ost);
      SetTitle(dname);
   }
   else SetStatusText(wxT("New file not created"));
}

void tui::TopedFrame::OnTDTRead(wxCommandEvent& evt)
{
   if (DATC->modified())
   {
      wxMessageDialog dlg1(this,
         wxT("Current design contains unsaved data"),
         wxT("Save the current design before creating a new one?"),
         wxYES_NO | wxCANCEL | wxICON_QUESTION);
      switch (dlg1.ShowModal())
      {
         case wxID_YES:OnTDTSave(evt);
         case wxID_NO: break;
         case wxID_CANCEL: return;
      }
   }
   SetStatusText(wxT("Opening file..."));
   wxFileDialog dlg2(this, wxT("Select a design to open"), wxT(""), wxT(""),
      wxT("Toped files (*.tdt)|*.tdt|All files(*.*)|*.*"),
      tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << wxT("tdtread(\"") << dlg2.GetDirectory() << wxT("/") << dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
      wxString ost1;
//      ost1 << wxT("File ") << dlg2.GetFilename() << wxT(" loaded");
//      SetStatusText(ost1);
      SetTitle(dlg2.GetFilename());
   }
   else SetStatusText(wxT("Opening aborted"));
}

void tui::TopedFrame::OnTDTLoadLib(wxCommandEvent& evt)
{
   SetStatusText(wxT("Loading library..."));
   wxFileDialog dlg2(this, wxT("Select a library to open"), wxT(""), wxT(""),
      wxT("Toped files (*.tdt)|*.tdt|All files(*.*)|*.*"),
      tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << wxT("loadlib(\"") << dlg2.GetDirectory() << wxT("/") << dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
      wxString ost1;
      SetTitle(dlg2.GetFilename());
   }
   else SetStatusText(wxT("Loading aborted"));
}

void tui::TopedFrame::OnTDTUnloadLib(wxCommandEvent& evt)
{
   wxString libName(_browsers->tdtSelectedCellName());

   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getLibList* dlg = NULL;
   try
   {
      dlg = DEBUG_NEW tui::getLibList(this, -1, wxT("Close Library"), pos, libName);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost;
      ost << wxT("unloadlib(\"") << dlg->get_selected() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnTELLRead(wxCommandEvent& evt)
{
   SetStatusText(wxT("Including command file..."));
   wxFileDialog dlg2(this, wxT("Select a script to run"), wxT(""), wxT(""),
      wxT("Tell files(*.tll)|*.tll|All files(*.*)|*.*"),
      tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << wxT("#include \"") << dlg2.GetDirectory() << wxT("/") << dlg2.GetFilename() << wxT("\";");
      _cmdline->parseCommand(ost);
//      SetStatusText(dlg2.GetFilename() + wxT(" parsed"));
   }
   else SetStatusText(wxT("include aborted"));
}

void tui::TopedFrame::OnGDSRead(wxCommandEvent& WXUNUSED(event))
{
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""),
                     wxT("Stream files(*.gds;*.sf)|*.gds;*.sf;*.GDS;*.SF|All files(*.*)|*.*"),
      tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing GDS file..."));
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << wxT("gdsread(\"") << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
//      wxString ost1;
//      ost1 << wxT("Stream ") << dlg2.GetFilename() << wxT(" loaded");
//      SetStatusText(ost1);
   }
   else SetStatusText(wxT("Parsing aborted"));
}

void tui::TopedFrame::OnTDTSave(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << wxT("tdtsave();");
   SetStatusText(wxT("Saving file..."));
   wxString wxfilename(DATC->tedFileName().c_str(), wxConvFile);
   wxFileName datafile( wxfilename );
   assert(datafile.IsOk());
   if (datafile.FileExists() && DATC->neverSaved()) {
      wxMessageDialog dlg1(this,
         wxT("File ") + wxfilename + wxT(" already exists. Overwrite ?"),
         wxT("Toped"),
         wxYES_NO | wxICON_QUESTION);
      switch (dlg1.ShowModal()) {
         case wxID_YES:_cmdline->parseCommand(ost); //Overwrite;
         case wxID_NO: return;
      }
   }
   else _cmdline->parseCommand(ost);;
//   SetStatusText(wxT("Design saved"));
}

void tui::TopedFrame::OnTDTSaveAs(wxCommandEvent& WXUNUSED(event)) {
   SetStatusText(wxT("Saving database under new filename..."));
   wxFileDialog dlg2(this, wxT("Save a design in a file"), wxT(""), wxT(""),
      wxT("Toped files |*.tdt"),
      tpdfSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetPath();
      if(!checkFileOverwriting(filename))
      {
         SetStatusText(wxT("Saving aborted"));
         return;
      }

      wxString ost;
      ost << wxT("tdtsaveas(\"") << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
//      SetStatusText(wxT("Design saved in file: ")+dlg2.GetFilename());
   }
   else SetStatusText(wxT("Saving aborted"));
}

void tui::TopedFrame::OnPropSave(wxCommandEvent& WXUNUSED(event))
{
   SetStatusText(wxT("Saving layout properties ..."));
   wxFileDialog dlg2(this, wxT("Save properties"), wxT(""), wxT(""),
      wxT("TELL files |*.tll"),
      tpdfSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetPath();
      if(!checkFileOverwriting(filename))
      {
         SetStatusText(wxT("Saving aborted"));
         return;
      }

      wxString ost;
      ost << wxT("propsave(\"") << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
//      SetStatusText(wxT("Design saved in file: ")+dlg2.GetFilename());
   }
   else SetStatusText(wxT("Saving aborted"));
}

void tui::TopedFrame::OnTDTSnapshot(wxCommandEvent& WXUNUSED(event))
{
   wxFileDialog dlg2(this, wxT("Save a canvas snapshot"), wxT(""), wxT(""),
                     wxT("Targa files |*.tga"),
                         tpdfSAVE);
   if (wxID_OK != dlg2.ShowModal()) return;
   wxString filename;
   filename << dlg2.GetDirectory() << wxT("/") << dlg2.GetFilename();
//   wxString fname = dlg2.GetPath();
   if(!checkFileOverwriting(filename)) return;

   // Note the pragmas below. It won't create a proper targa file whithout them!
   // See the note in LayoutCanvas::snapshot(...)
#pragma pack(push,1)
   typedef struct {
      byte   identsize;              // Size of ID field that follows header (0)
      byte   colorMapType;           // 0 = None, 1 = paletted
      byte   imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
      word   colorMapStart;          // First colour map entry
      word   colorMapLength;         // Number of colors
      byte   colorMapBits;           // bits per palette entry
      word   xstart;                 // image x origin
      word   ystart;                 // image y origin
      word   width;                  // width in pixels
      word   height;                 // height in pixels
      byte   bits;                   // bits per pixel (8 16, 24, 32)
      byte   descriptor;             // image descriptor
   } TargaHeader;
#pragma pack(pop)

   byte* theImage = NULL;
   word szH, szW;
   _canvas->snapshot(theImage, szW, szH);
   unsigned long imgSize = 3 * szH * szW;

   // Initialize the Targa header
   TargaHeader tHdr = {0, 0, 2, 0, 0, 0, 0, 0, szW, szH, 24, 0 };

   FILE* tFile = fopen(filename.mb_str(wxConvUTF8) , "wb");
   if(NULL != theImage)
   {
      //Save first the targa header and then - the data
      fwrite(&tHdr, sizeof(TargaHeader), 1, tFile);
      fwrite(theImage, imgSize, 1, tFile);
   }
	fclose(tFile);
   delete[] theImage;
}

void tui::TopedFrame::OnCellNew(wxCommandEvent& cevent)
{
   wxString defcellname = wxEmptyString;
   if (0 != cevent.GetId())
      defcellname = _browsers->tdtSelectedCellName();
   wxTextEntryDialog dlg2(this,
      wxT("Cell name:"),
      wxT("Create new cell"),
      defcellname);
   wxString cname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((cname = dlg2.GetValue()) != wxT(""))) {
      SetStatusText(wxT("Creating new cell..."));
      ost << wxT("newcell(\"") << cname << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   else SetStatusText(wxT("New cell not created"));
}

void tui::TopedFrame::OnCellOpen(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellOpen* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getCellOpen(this, -1, wxT("Cell Open"), pos, wxT(""));
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << wxT("opencell(\"") << dlg->get_selectedcell() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnCellRemove(wxCommandEvent&)
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellOpen* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getCellOpen(this, -1, wxT("Cell Remove"), pos, wxT(""));
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << wxT("removecell(\"") << dlg->get_selectedcell() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnGDStranslate(wxCommandEvent& WXUNUSED(event)) {
   bool success;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      tui::getGDSimport* dlg = NULL;
      USMap* laymap;
      try {
         dlg = DEBUG_NEW tui::getGDSimport(this, -1, wxT("Import GDS structure"), pos,
                                             _browsers->tdtSelectedGdsName(), drawProp);
      }
      catch (EXPTN) {delete dlg;return;}
      if ( dlg->ShowModal() == wxID_OK )
      {
         laymap = dlg->getGdsLayerMap();
         USMap* laymap2save = NULL;
         if (dlg->getSaveMap()) laymap2save = dlg->getFullGdsLayerMap();

         wxString wxlaymap, wxlaymap2save;
         USMap2wxString(laymap      , wxlaymap     );
         if (NULL != laymap2save)
            USMap2wxString(laymap2save , wxlaymap2save);
         ost << wxT("gdsimport(\"") << dlg->get_selectedcell() << wxT("\" , ")
             << wxlaymap << wxT(", ")
             << (dlg->get_recursive() ? wxT("true") : wxT("false"))   << wxT(" , ")
             << (dlg->get_overwrite() ? wxT("true") : wxT("false"))   <<wxT(");");
         if (NULL != laymap2save)
            ost << wxT("setgdslaymap(")
                << wxlaymap2save << wxT(");");
         success = true;
         delete laymap;
         if (NULL != laymap2save) delete laymap2save;
      }
      delete dlg;
   }
   PROPC->unlockDrawProp(drawProp);
   if (success)
      _cmdline->parseCommand(ost);


}

void tui::TopedFrame::OnGDSimport(wxCommandEvent& WXUNUSED(event))
{
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""),
                     wxT("Stream files(*.gds;*.sf)|*.gds;*.sf;*.GDS;*.SF|All files(*.*)|*.*"),
                         tpdfOPEN);
   if (wxID_OK != dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing aborted")); return;
   }
   SetStatusText(wxT("Importing GDS file..."));
   wxString filename = dlg2.GetFilename();
   wxString ost_int;
   ost_int << wxT("gdsread(\"") << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename() << wxT("\")");
   wxString ost;
   ost << wxT("gdsimport(") << ost_int << wxT(", getgdslaymap(true), true, false );gdsclose();");
   _cmdline->parseCommand(ost);
//   SetStatusText(wxT("Stream ")+dlg2.GetFilename()+wxT(" imported"));
}

void tui::TopedFrame::OnCIFimport(wxCommandEvent& WXUNUSED(event))
{
   // Here - try a hollow lock/unlock the database just to check that it exists
   try {DATC->lockDB(false);}
   catch (EXPTN) {return;}
   DATC->unlockDB();
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""),
                     wxT("Caltech files(*.cif)|*.cif;*.CIF|All files(*.*)|*.*"),
                     tpdfOPEN);
   if (wxID_OK != dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing aborted")); return;
   }
   SetStatusText(wxT("Importing CIF file..."));
   wxString filename = dlg2.GetFilename();
   wxString ost_int;
   ost_int << wxT("cifread(\"") << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename() << wxT("\")");
   wxString ost;
   ost << wxT("cifimport(") << ost_int << wxT(", getciflaymap(true), true, false, 0.0 );cifclose();");
   _cmdline->parseCommand(ost);
//   SetStatusText(wxT("Stream ")+dlg2.GetFilename()+wxT(" imported"));
}

void tui::TopedFrame::OnGDSexportLIB(wxCommandEvent& WXUNUSED(event)) {
   SetStatusText(wxT("Exporting database to GDS file..."));
   wxFileDialog dlg2(this, wxT("Export design to GDS file"), wxT(""), wxT(""),
      wxT("GDS files |*.gds;*.sf"),
      tpdfSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetPath();
      if(!checkFileOverwriting(filename))
      {
         SetStatusText(wxT("GDS export aborted"));
         return;
      }
      wxString ost;
      ost << wxT("gdsexport(getgdslaymap(false), \"")
          << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename()
          << wxT("\", false);");
      _cmdline->parseCommand(ost);
//      SetStatusText(wxT("Design exported to: ")+dlg2.GetFilename());
   }
   else SetStatusText(wxT("GDS export aborted"));
}

void tui::TopedFrame::OnCIFexportLIB(wxCommandEvent& WXUNUSED(event))
{
   SetStatusText(wxT("Exporting database to CIF file..."));
   wxFileDialog dlg2(this, wxT("Export design to CIF file"), wxT(""), wxT(""),
                     wxT("Caltech files(*.cif)|*.cif;*.CIF"),
                         tpdfSAVE);
   if (wxID_OK == dlg2.ShowModal()) {
      wxString filename = dlg2.GetPath();
      if(!checkFileOverwriting(filename))
      {
         SetStatusText(wxT("CIF export aborted"));
         return;
      }
      wxString ost;
      ost << wxT("cifexport(getciflaymap(false), \"")
            << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename()
            << wxT("\", false);");
      _cmdline->parseCommand(ost);
//      SetStatusText(wxT("Design exported to: ")+dlg2.GetFilename());
   }
   else SetStatusText(wxT("CIF export aborted"));
}

void tui::TopedFrame::OnGDSexportCELL(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   SetStatusText(wxT("Exporting a cell to GDS file..."));
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      tui::getGDSexport* dlg = NULL;
      try {
         dlg = DEBUG_NEW tui::getGDSexport(this, -1, wxT("GDS export cell"), pos,
                                           _browsers->tdtSelectedCellName(), drawProp);
      }
      catch (EXPTN) {delete dlg;return;}
      wxString cellname;
      bool recur;
      USMap* laymap;
      USMap* laymap2save = NULL;
      if ( dlg->ShowModal() == wxID_OK ) {
         cellname = dlg->get_selectedcell();
         recur = dlg->get_recursive();
         laymap = dlg->getGdsLayerMap();
         if (dlg->getSaveMap()) laymap2save = dlg->getFullGdsLayerMap();
         delete dlg;
      }
      else {delete dlg;return;}
      wxString oststr;
      oststr <<wxT("Exporting ") << cellname << wxT(" to GDS file");
      wxString fullCellName;
      fullCellName << cellname << wxT(".gds");
      wxFileDialog dlg2(this, oststr , wxT(""), fullCellName,
         wxT("GDS files |*.gds;*.sf"),
         tpdfSAVE);
      if (wxID_OK == dlg2.ShowModal()) {
         wxString filename = dlg2.GetPath();
         if(!checkFileOverwriting(filename))
            SetStatusText(wxT("GDS export aborted"));
         else
         {
            wxString wxlaymap, wxlaymap2save;
            USMap2wxString(laymap      , wxlaymap     );
            if (NULL != laymap2save)
               USMap2wxString(laymap2save , wxlaymap2save);
            ost << wxT("gdsexport(\"")
               << cellname.c_str() << wxT("\" , ")
               << (recur ? wxT("true") : wxT("false")) << wxT(",")
               << wxlaymap << wxT(", \"")
               << (dlg2.GetDirectory()).c_str() << wxT("/") <<(dlg2.GetFilename()).c_str()
               << wxT("\", false);");
            if (NULL != laymap2save)
               ost << wxT("setgdslaymap(")
                     << wxlaymap2save << wxT(");");
            success = true;
         }
   //      SetStatusText(wxT("Design exported to: ")+dlg2.GetFilename());
      }
      else SetStatusText(wxT("GDS export aborted"));
      delete laymap;
      if (NULL != laymap2save) delete laymap2save;
   }
   PROPC->unlockDrawProp(drawProp);
   if (success)
      _cmdline->parseCommand(ost);
}


void tui::TopedFrame::OnCIFRead(wxCommandEvent& WXUNUSED(event))
{
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""),
                     wxT("Caltech files(*.cif)|*.cif;*.CIF|All files(*.*)|*.*"),
                         tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal()) {
      SetStatusText(wxT("Parsing CIF file..."));
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << wxT("cifread(\"") << dlg2.GetDirectory() << wxT("/") <<dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   else SetStatusText(wxT("Parsing aborted"));
}

void tui::TopedFrame::OnCIFtranslate(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      tui::getCIFimport* dlg = NULL;
      try {
         dlg = DEBUG_NEW tui::getCIFimport(this, -1, wxT("Import CIF structure"), pos,
                                           _browsers->tdtSelectedCifName(), drawProp);
      }
      catch (EXPTN) {delete dlg;return;}
      if ( dlg->ShowModal() == wxID_OK )
      {
         // get the layer map first
         SIMap* laymap = dlg->getCifLayerMap(drawProp);
         USMap* laymap2save = NULL;
         if (dlg->getSaveMap()) laymap2save = dlg->getFullCifLayerMap(drawProp);
         wxString wxlaymap, wxlaymap2save;
         SIMap2wxString(laymap      , wxlaymap     );
         if (NULL != laymap2save)
            USMap2wxString(laymap2save , wxlaymap2save);
         double techno;
         if (!dlg->getTechno().ToDouble(&techno))
            techno = 0.0;
         ost << wxT("cifimport(\"") << dlg->getSelectedCell() << wxT("\" , ") << wxlaymap << wxT(",")
             << (dlg->getRecursive() ? wxT("true") : wxT("false")) << wxT(",")
             << (dlg->getOverwrite() ? wxT("true") : wxT("false")) << wxT(",")
             << techno << wxT(");");
         if (NULL != laymap2save)
            ost << wxT("setciflaymap(")
                << wxlaymap2save << wxT(");");
         success = true;
         delete laymap;
         if (NULL != laymap2save) delete laymap2save;
      }
      delete dlg;
   }
   PROPC->unlockDrawProp(drawProp);
   if (success) _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnCIFexportCELL(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   SetStatusText(wxT("Exporting a cell to CIF file..."));
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      tui::getCIFexport* dlg = NULL;
      try {
         dlg = DEBUG_NEW tui::getCIFexport(this, -1, wxT("CIF export cell"), pos,
                                           _browsers->tdtSelectedCellName(), drawProp);
      }
      catch (EXPTN) {delete dlg;return;}
      wxString cellname;
      bool recur;
      bool sverbose;
      USMap* laymap;
      USMap* laymap2save = NULL;
      if ( dlg->ShowModal() == wxID_OK ) {
         cellname = dlg->get_selectedcell();
         recur    = dlg->get_recursive();
         sverbose = dlg->get_slang();
         laymap   = dlg->getCifLayerMap();
         if (dlg->getSaveMap()) laymap2save = dlg->getFullCifLayerMap();
         delete dlg;
      }
      else {delete dlg;return;}
      wxString oststr;
      oststr <<wxT("Exporting ") << cellname << wxT(" to CIF file");
      wxString fullCellName;
      fullCellName << cellname << wxT(".cif");
      wxFileDialog dlg2(this, oststr , wxT(""), fullCellName,
         wxT("CIF files |*.cif;*.CIF"),
         tpdfSAVE);
      if (wxID_OK == dlg2.ShowModal())
      {
         wxString filename = dlg2.GetPath();
         if(!checkFileOverwriting(filename))
            SetStatusText(wxT("CIF export aborted"));
         else
         {
            wxString wxlaymap, wxlaymap2save;
            USMap2wxString(laymap      , wxlaymap     );
            if (NULL != laymap2save)
               USMap2wxString(laymap2save , wxlaymap2save);
            ost << wxT("cifexport(\"")
               << cellname << wxT("\", ")
               << (recur ? wxT("true") : wxT("false")) << wxT(", ")
               << wxlaymap << wxT(", \"")
               <<  dlg2.GetDirectory().c_str() << wxT("/") << dlg2.GetFilename().c_str() << wxT("\", ")
               << (sverbose ? wxT("true") : wxT("false")) << wxT(" );");
            if (NULL != laymap2save)
               ost << wxT("setciflaymap(")
                   << wxlaymap2save << wxT(");");
            success = true;
         }
      }
      else SetStatusText(wxT("CIF export aborted"));
      delete laymap;
      if (NULL != laymap2save) delete laymap2save;
   }
   PROPC->unlockDrawProp(drawProp);
   if (success)
   {
      _cmdline->parseCommand(ost);
//      SetStatusText(wxT("Design exported to: ")+dlg2.GetFilename());
   }

}

void tui::TopedFrame::OnOASRead(wxCommandEvent& WXUNUSED(event))
{
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""),
                     wxT("Oasis files(*.oas)|*.oas;*.OAS|All files(*.*)|*.*"),
                     tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing Oasis file..."));
      wxString filename = dlg2.GetFilename();
      wxString ost;
      ost << wxT("oasisread(\"") << dlg2.GetDirectory() << wxT("/") << dlg2.GetFilename() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   else SetStatusText(wxT("Parsing aborted"));
}

void tui::TopedFrame::OnOASimport(wxCommandEvent& WXUNUSED(event))
{
   //@TODO
}

void tui::TopedFrame::OnOAStranslate(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      tui::getOASimport* dlg = NULL;
      USMap* laymap;
      try {
         dlg = DEBUG_NEW tui::getOASimport(this, -1, wxT("Import OASIS structure"), pos,
                                             _browsers->tdtSelectedOasName(), drawProp);
      }
      catch (EXPTN) {delete dlg;return;}
      if ( dlg->ShowModal() == wxID_OK )
      {
         laymap = dlg->getOasLayerMap();
         USMap* laymap2save = NULL;
         if (dlg->getSaveMap()) laymap2save = dlg->getFullOasLayerMap();

         wxString wxlaymap, wxlaymap2save;
         USMap2wxString(laymap      , wxlaymap     );
         if (NULL != laymap2save)
            USMap2wxString(laymap2save , wxlaymap2save);
         ost << wxT("oasisimport(\"") << dlg->get_selectedcell() << wxT("\" , ")
             << wxlaymap << wxT(", ")
             << (dlg->get_recursive() ? wxT("true") : wxT("false"))   << wxT(" , ")
             << (dlg->get_overwrite() ? wxT("true") : wxT("false"))   <<wxT(");");
         if (NULL != laymap2save)
            ost << wxT("setoasislaymap(")
                << wxlaymap2save << wxT(");");
         success = true;
         delete laymap;
         if (NULL != laymap2save) delete laymap2save;
      }
      delete dlg;
   }
   PROPC->unlockDrawProp(drawProp);
   if (success)
      _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnOASexportLIB(wxCommandEvent& WXUNUSED(event))
{
   //@TODO
}

void tui::TopedFrame::OnOASexportCELL(wxCommandEvent& WXUNUSED(event))
{
   //@TODO
}


void tui::TopedFrame::OnCellRef_B(wxCommandEvent& WXUNUSED(event)) {
   CellRef(_browsers->tdtSelectedCellName());
}

void tui::TopedFrame::OnCellRef_M(wxCommandEvent& WXUNUSED(event)) {
   CellRef(wxT(""));
}

void tui::TopedFrame::OnCellARef_B(wxCommandEvent& WXUNUSED(event)) {
   CellARef(_browsers->tdtSelectedCellName());
}

void tui::TopedFrame::OnCellARef_M(wxCommandEvent& WXUNUSED(event)) {
   CellARef(wxT(""));
}

void tui::TopedFrame::CellRef(wxString clname) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellRef* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getCellRef(this, -1, wxT("Cell Reference"), pos, clname);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
/*      ost << wxT("cellref(\"") << dlg->get_selectedcell() <<wxT("\",getpoint(),")
          <<                     dlg->get_angle() << wxT(",")
          << (dlg->get_flip() ? wxT("true") : wxT("false")) << wxT(",")
          <<                                wxT("1")  << wxT(");");*/
      ost << wxT("cellref(\"") << dlg->get_selectedcell() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::CellARef(wxString clname) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellARef* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getCellARef(this, -1, wxT("Array of References"), pos, clname);
   }
   catch (EXPTN) {delete dlg; return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
/*      ost << wxT("cellaref(\"") << dlg->get_selectedcell() <<wxT("\",getpoint(),")
          <<                     dlg->get_angle() << wxT(",")
          << (dlg->get_flip() ? wxT("true") : wxT("false")) << wxT(",")
          <<                                 wxT("1")  << wxT(",")
          <<                       dlg->get_col() << wxT(",")
          <<                       dlg->get_row() << wxT(",")
          <<                     dlg->get_stepX() << wxT(",")
          <<                     dlg->get_stepY() << wxT(");");*/
      ost << wxT("cellaref(\"") << dlg->get_selectedcell() << wxT("\",")
          <<                       dlg->get_col() << wxT(",")
          <<                       dlg->get_row() << wxT(",")
          <<                     dlg->get_stepX() << wxT(",")
          <<                     dlg->get_stepY() << wxT(");");
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
      wxT("Cell name:"),
      wxT("Create new cell from selected components"));
   wxString cname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((cname = dlg2.GetValue()) != wxT(""))) {
      SetStatusText(wxT("Grouping in a new cell..."));
      ost << wxT("group(\"") << cname << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   else SetStatusText(wxT("Groupping canceled"));
}

void tui::TopedFrame::OnDrawWire(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Wire width"), pos, PROPC->step() ,3, 2);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost; ost << wxT("addwire(")<<dlg->value()<<wxT(");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnDrawText(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getTextdlg* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getTextdlg(this, -1, wxT("Add text"), pos);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost; ost << wxT("addtext(\"")
                        << dlg->get_text()                      << wxT("\",")
                        << dlg->get_size()                      << wxT(");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnResize(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Resize by"), pos, PROPC->step() ,3, 1);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost; ost << wxT("resize(")<<dlg->value()<<wxT(");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnStep(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getStep dlg(this, -1, wxT("Step size"), pos, PROPC->step());
   if ( dlg.ShowModal() == wxID_OK ) {
      wxString ost; ost << wxT("step(")<<dlg.value()<<wxT(");");
      _cmdline->parseCommand(ost);
   }
}

void tui::TopedFrame::OnUndoDepth(wxCommandEvent& WXUNUSED(event))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Undo depth"), pos, 1, 0, CMDBlock->undoDepth());
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      unsigned long newValue;
      if (dlg->value().ToULong(&newValue))
         CMDBlock->setUndoDepth(static_cast<word>(newValue));
   }
   delete dlg;
}


void tui::TopedFrame::OnGridDefine(wxCommandEvent& WXUNUSED(event)) {
   real grid[3];
   const layprop::LayoutGrid *gr  = NULL;
   for (byte i = 0; i < 3; i++)
      if (NULL != (gr = PROPC->grid(i))) grid[i] = gr->step();
      else                                    grid[i] = 0.0;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getGrid dlg(this, -1, wxT("Grid size"), pos, grid[0], grid[1], grid[2]);
   if ( dlg.ShowModal() == wxID_OK ) {
      wxString ost;
      ost << wxT("definegrid(0,")<<dlg.grid0()<<wxT(",\"white\");");
      ost << wxT("definegrid(1,")<<dlg.grid1()<<wxT(",\"white\");");
      ost << wxT("definegrid(2,")<<dlg.grid2()<<wxT(",\"white\");");
      _cmdline->parseCommand(ost);
   }
}

void tui::TopedFrame::OnGrid0(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << wxT("grid(0,") << (settingsMenu->IsChecked(TMSET_GRID0) ? wxT("true") : wxT("false")) << wxT(");");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnGrid1(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << wxT("grid(1,") << (settingsMenu->IsChecked(TMSET_GRID1) ? wxT("true") : wxT("false")) << wxT(");");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnGrid2(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << wxT("grid(2,") << (settingsMenu->IsChecked(TMSET_GRID2) ? wxT("true") : wxT("false")) << wxT(");");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnLongCursor(wxCommandEvent& WXUNUSED(event)){
  wxString ost;
  ost << wxT("longcursor(") << (settingsMenu->IsChecked(TMSET_CURLONG) ? wxT("true") : wxT("false")) <<
  wxT(");");
  _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize16(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(horizontal, _iconsize16);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize24(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(horizontal, _iconsize24);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize32(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(horizontal, _iconsize32);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize48(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(horizontal, _iconsize48);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize16(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(vertical, _iconsize16);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize24(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(vertical, _iconsize24);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize32(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(vertical, _iconsize32);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize48(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(vertical, _iconsize48);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnAutopan(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << wxT("autopan(")<< (settingsMenu->IsChecked(TMSET_AUTOPAN) ? wxT("true") : wxT("false")) << wxT(");");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnPropertySheet(wxCommandEvent& WXUNUSED(event))
{
   _propDialog->Show();
}

void tui::TopedFrame::OnZeroCross(wxCommandEvent& WXUNUSED(event)){
   wxString ost;
   ost << wxT("zerocross(")<< (settingsMenu->IsChecked(TMSET_ZEROCROSS) ? wxT("true") : wxT("false")) << wxT(");");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnMarker0(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << wxT("shapeangle(0);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnMarker45(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << wxT("shapeangle(45);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnMarker90(wxCommandEvent& WXUNUSED(event)) {
   wxString ost;
   ost << wxT("shapeangle(90);");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnDefineLayer(wxCommandEvent& event)
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      word layno = drawProp->curLay();
      editLayerDlg(layno, drawProp);
   }
   PROPC->unlockDrawProp(drawProp);
}

void tui::TopedFrame::OnEditLayer(wxCommandEvent& evt)
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
	word layno = evt.GetInt();
	editLayerDlg(layno, drawProp);
   }
   PROPC->unlockDrawProp(drawProp);
}

void tui::TopedFrame::editLayerDlg(word layno, const layprop::DrawProperties* drawprop)
{
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::defineLayer dlg(this, -1, wxT("Define Layer"), pos, layno, drawprop);
   if ( dlg.ShowModal() == wxID_OK )
   {
      ost      << wxT("layprop(\"") << dlg.layname()
               << wxT("\" , ")      << dlg.layno()
               << wxT(" , \"")      << dlg.color()
               << wxT("\" , \"")    << dlg.fill()
               << wxT("\" , \"")    << dlg.line()
               << wxT("\");");
      success = true;
   }
   if (success) _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnDefineColor(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawprop;
   if (PROPC->lockDrawProp(drawprop))
   {
      tui::defineColor dlg(this, -1, wxT("Color Definitions"), pos, drawprop);
      if ( dlg.ShowModal() == wxID_OK )
      {
         const layprop::ColorMap colors = dlg.allColors();
         for(layprop::ColorMap::const_iterator CC = colors.begin() ; CC != colors.end(); CC++)
         {
            layprop::tellRGB* coldef = CC->second;
            ost   << wxT("definecolor(\"") << wxString(CC->first.c_str(), wxConvUTF8)
                  << wxT("\" , ")      << coldef->red()
                  << wxT(" , ")        << coldef->green()
                  << wxT(" , ")        << coldef->blue()
                  << wxT(" , ")        << coldef->alpha()
                  << wxT(");");
         }
         success = true;
      }
   }
   PROPC->unlockDrawProp(drawprop);
   if (success) _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnDefineFill(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawprop;
   if (PROPC->lockDrawProp(drawprop))
   {
      tui::defineFill dlg(this, -1, wxT("Fill Definition"), pos, drawprop);
      if ( dlg.ShowModal() == wxID_OK )
      {
         layprop::FillMap patterns = dlg.allPatterns();
         for(layprop::FillMap::const_iterator CC = patterns.begin() ; CC != patterns.end(); CC++)
         {
            byte* patdef = CC->second;
            ost   << wxT("definefill(\"") << wxString(CC->first.c_str(), wxConvUTF8)
                  << wxT("\" , {");
            ost << patdef[0];
            for (byte i = 1; i < 128; i++)
               ost  << wxT(",") << patdef[i];
            ost   << wxT("});");
            success = true;
         }
      }
   }
   PROPC->unlockDrawProp(drawprop);
   if (success) _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnChangeRef( wxCommandEvent& WXUNUSED( event ))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellRef* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getCellRef(this, -1, wxT("Change Cell Reference"), pos, wxT(""));
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost;
      ost << wxT("changeref(\"") << dlg->get_selectedcell() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnChangeText( wxCommandEvent& WXUNUSED( event ))
{
   wxTextEntryDialog dlg(this, wxT("New string:"), wxT("Change text string"));
   wxString cname, ost;
   if ((wxID_OK == dlg.ShowModal()) && ((cname = dlg.GetValue()) != wxT("")))
   {
      SetStatusText(wxT("Change text string ..."));
      wxString ost;
      ost << wxT("changestr(\"") << dlg.GetValue() << wxT("\");");
      _cmdline->parseCommand(ost);
   }
}

void tui::TopedFrame::OnChangeLayer( wxCommandEvent& WXUNUSED( event ))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Transfer to layer"), pos, 1, 0, 2);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost; ost << wxT("changelayer(")<<dlg->value()<<wxT(");");
      _cmdline->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnCurrentLayer( wxCommandEvent& WXUNUSED( event ))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Change current layer"), pos, 1, 0, 2);
   }
   catch (EXPTN) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      unsigned long vlu;
      dlg->value().ToULong(&vlu);
      DATC->setCmdLayer((word)vlu);
   }
   delete dlg;
}

void tui::TopedFrame::OnMenu(wxCommandEvent& event)
{
   _resourceCenter->executeMenu(event.GetId());
}

void tui::TopedFrame::OnUpdateSettingsMenu(wxCommandEvent& evt)
{
   switch (evt.GetId())
   {
      case STS_GRID0          : settingsMenu->Check(TMSET_GRID0       , evt.GetInt() );break;
      case STS_GRID1          : settingsMenu->Check(TMSET_GRID1       , evt.GetInt() );break;
      case STS_GRID2          : settingsMenu->Check(TMSET_GRID2       , evt.GetInt() );break;
      case STS_AUTOPAN        : settingsMenu->Check(TMSET_AUTOPAN     , evt.GetInt() );break;
      case STS_ZEROCROSS      : settingsMenu->Check(TMSET_ZEROCROSS   , evt.GetInt() );break;
      case STS_CURSOR         : settingsMenu->Check(TMSET_CURLONG     , evt.GetInt() );break;
      case STS_ANGLE          :
         switch (evt.GetInt())
         {
            case  0: settingsMenu->Check(TMSET_MARKER0     , true );break;
            case 45: settingsMenu->Check(TMSET_MARKER45    , true );break;
            case 90: settingsMenu->Check(TMSET_MARKER90    , true );break;
            default: assert(false);
         }
         break;
      default: _propDialog->updateRenderSheet(evt);break;
   }
};

void  tui::TopedFrame::OnMouseAccel(wxCommandEvent& evt) {
   wxString ost;
   if (0 == evt.GetInt())
      ost << wxT("unselect(");
   else if (1 == evt.GetInt())
      ost << wxT("select(");
   else return;
   ost << evt.GetString() << wxT(");");
   _cmdline->parseCommand(ost);
}

void tui::TopedFrame::OnzoomIn(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_IN);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnzoomOut(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_OUT);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnzoomEmpty(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_EMPTY);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnpanLeft(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_LEFT);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnpanRight(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_RIGHT);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnpanUp(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_UP);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnpanDown(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_DOWN);
   wxPostEvent(_canvas, eventZOOM);
}

bool tui::TopedFrame::checkFileOverwriting(const wxString& fileName)
{
   bool ret=true;
   wxFileName checkedFileName(fileName);
   assert(checkedFileName.IsOk());
   if (checkedFileName.FileExists())
   {
      wxMessageDialog dlg1(this,
      wxT("File ") + fileName + wxT(" already exists. Overwrite ?"),
      wxT("Toped"),
      wxYES_NO | wxICON_QUESTION);
      if (wxID_NO==dlg1.ShowModal()) ret = false; else ret = true;
   }
   return ret;
}

void tui::TopedFrame::OnUncapturedMouseClick(wxCommandEvent& evt)
{
   telldata::ttpnt* p = static_cast<telldata::ttpnt*>(evt.GetClientData());
   delete p;
}

void tui::TopedFrame::OnToolBarSize(wxCommandEvent& evt)
{
   tui::IconSizes sz = static_cast<tui::IconSizes>(evt.GetInt());
   bool direction = static_cast<bool>(evt.GetExtraLong());

   _resourceCenter->setToolBarSize(direction, sz);
   // update the menu state
   if (tui::_tuihorizontal == direction)
      switch (sz)
      {
         case  ICON_SIZE_16x16: settingsMenu->Check(TMSET_HTOOLSIZE16 , true );break;
         case  ICON_SIZE_24x24: settingsMenu->Check(TMSET_HTOOLSIZE24 , true );break;
         case  ICON_SIZE_32x32: settingsMenu->Check(TMSET_HTOOLSIZE32 , true );break;
         case  ICON_SIZE_48x48: settingsMenu->Check(TMSET_HTOOLSIZE48 , true );break;
         default: assert(false);
      }
   else
      switch (sz)
      {
         case  ICON_SIZE_16x16: settingsMenu->Check(TMSET_VTOOLSIZE16 , true );break;
         case  ICON_SIZE_24x24: settingsMenu->Check(TMSET_VTOOLSIZE24 , true );break;
         case  ICON_SIZE_32x32: settingsMenu->Check(TMSET_VTOOLSIZE32 , true );break;
         case  ICON_SIZE_48x48: settingsMenu->Check(TMSET_VTOOLSIZE48 , true );break;
         default: assert(false);
      }
}

void tui::TopedFrame::OnToolBarDefine(wxCommandEvent& evt)
{
	std::string toolBarBame(evt.GetString().mb_str(wxConvUTF8));
	_resourceCenter->defineToolBar(toolBarBame);
}

void tui::TopedFrame::OnToolBarAddItem(wxCommandEvent& evt)
{
	std::string toolBarName(evt.GetString().mb_str(wxConvUTF8));
	tellstdfunc::StringMapClientData* map = static_cast<tellstdfunc::StringMapClientData*>(evt.GetClientObject());
	std::string toolName = map->GetKey();
	std::string toolFunc = map->GetValue();

	_resourceCenter->appendTool(toolBarName, toolName, toolName,  "", "", toolFunc);
	delete map;
}

void tui::TopedFrame::OnToolBarDeleteItem(wxCommandEvent& evt)
{
	std::string toolBarName(evt.GetString().mb_str(wxConvUTF8));
	wxStringClientData *data= static_cast<wxStringClientData*>(evt.GetClientObject());
	wxString str = data->GetData();
	std::string toolName(str.mb_str(wxConvUTF8));

	_resourceCenter->deleteTool(toolBarName, toolName);
}

void	tui::TopedFrame::OnDRCResults(wxCommandEvent& evt)
{
	wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
	wxFileDialog dlg(this, wxT("Select Calibre DRC Results to open"), wxT(""), wxT(""),
      wxT("DRC file (*.results)|*.results|All files(*.*)|*.*"),
      tpdfOPEN);
    if ( dlg.ShowModal() == wxID_OK )
   {
		wxString ost;
		ost << wxT("drccalibreimport(\"") << dlg.GetPath()<< wxT("\");");//\"D:/toped/drc3/drc3.drc.results\");");
		_cmdline->parseCommand(ost);
   }

	//wxString ost;
   //ost << wxT("drccalibreimport(\"D:/toped/drc3/drc3.drc.results\");");
	//_cmdline->parseCommand(ost);
}

void  tui::TopedFrame::OnCadenceConvert(wxCommandEvent& WXUNUSED(event))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::cadenceConvert dlg(this, -1, wxT("Cadence Converter"), pos);
   if ( dlg.ShowModal() == wxID_OK )
   {
   }
}

void tui::TopedFrame::OnTextLogOverflow(wxCommandEvent& WXUNUSED(event))
{
   //@TODO! I can't get this message. In the same time it seems that
   // wx is getting stuck when bombarded with messages from the
   // tell thread (GDS parse/import case). This is observed on
   // windows. Trincating the text log contents doesn't really help
//   int boza;
//   boza++;
//   int lalal = 2* boza;
}

void tui::TopedFrame::USMap2wxString(USMap* inmap, wxString& outmap)
{
   std::string soutmap;
   layprop::USMap2String(inmap, soutmap);
   outmap = wxString(soutmap.c_str(), wxConvUTF8);
}

void tui::TopedFrame::SIMap2wxString(SIMap* inmap, wxString& outmap)
{
   std::ostringstream laymapstr;
   word recno = 0;
   laymapstr << "{";
   for (SIMap::const_iterator CLN = inmap->begin(); CLN != inmap->end(); CLN++)
   {
      if (recno != 0)
         laymapstr << ",";
      laymapstr << "{" << CLN->second << ",\"" << CLN->first << "\"}";
      recno++;
   }
   laymapstr << "}";
   outmap = wxString(laymapstr.str().c_str(), wxConvUTF8);
}
