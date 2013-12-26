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
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/aboutdlg.h>
#include <wx/tooltip.h>
#include <wx/txtstrm.h>
#include <math.h>
#include "toped.h"
#include "datacenter.h"
#include "viewprop.h"
#include "tenderer.h"
#include "tui.h"
#include "techeditor.h"
#include "../ui/toped16x16.xpm"
#include "../ui/toped32x32.xpm"

#if wxCHECK_VERSION(2, 8, 0)
#define tpdfOPEN wxFD_OPEN
#define tpdfSAVE wxFD_SAVE
#else
#define tpdfOPEN wxOPEN
#define tpdfSAVE wxSAVE
#endif

extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_RENDER_PARAMS;
extern const wxEventType         wxEVT_CANVAS_PARAMS;
extern const wxEventType         wxEVT_MOUSE_ACCEL;
extern const wxEventType         wxEVT_CURRENT_LAYER;
extern const wxEventType         wxEVT_TOOLBARSIZE;
extern const wxEventType         wxEVT_TOOLBARDEF;
extern const wxEventType         wxEVT_TOOLBARADDITEM;
extern const wxEventType         wxEVT_TOOLBARDELETEITEM;
extern const wxEventType         wxEVT_EDITLAYER;
extern const wxEventType         wxEVT_EXITAPP;
extern const wxEventType         wxEVT_EXECEXT;
extern const wxEventType         wxEVT_EXECEXTPIPE;
extern const wxEventType         wxEVT_EXECEXTDONE;
extern const wxEventType         wxEVT_RELOADTELLFUNCS;
extern const wxEventType         wxEVT_CONSOLE_PARSE;

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern parsercmd::cmdBLOCK*      CMDBlock;
extern console::TllCmdLine*      Console;

tui::CanvasStatus::CanvasStatus(wxWindow* parent, wxWindowID id ,
   const wxPoint& pos , const wxSize& size , long style)
   : wxAuiToolBar( parent, id, pos, size, style)
{
   wxFont fontX = GetFont();
   fontX.SetWeight(wxBOLD);
   fontX.SetPointSize(12);
   fontX.SetFamily(wxFONTFAMILY_TELETYPE);
   SetFont(fontX);

   wxSize dSize(GetTextExtent(wxT("-999999999.999")));//2 digits more
   dSize.SetHeight(-1);

   _ctrlXPos = DEBUG_NEW wxTextCtrl( this, wxID_ANY , wxEmptyString, wxDefaultPosition, dSize, wxTE_RIGHT | wxTE_READONLY,
                           wxTextValidator(wxFILTER_NUMERIC, &_strX));
   _ctrlXPos->SetForegroundColour(*wxWHITE);  _ctrlXPos->SetBackgroundColour(*wxBLACK);

   _ctrlYPos = DEBUG_NEW wxTextCtrl( this, wxID_ANY , wxEmptyString, wxDefaultPosition, dSize, wxTE_RIGHT | wxTE_READONLY,
                           wxTextValidator(wxFILTER_NUMERIC, &_strY));
   _ctrlYPos->SetForegroundColour(*wxWHITE);  _ctrlYPos->SetBackgroundColour(*wxBLACK);

   _ctrlDX = DEBUG_NEW wxTextCtrl( this, wxID_ANY , wxEmptyString, wxDefaultPosition, dSize, wxTE_RIGHT | wxTE_READONLY,
                           wxTextValidator(wxFILTER_NUMERIC, &_strDX));
   _ctrlDX->SetForegroundColour(*wxWHITE);  _ctrlDX->SetBackgroundColour(*wxBLACK);

   _ctrlDY = DEBUG_NEW wxTextCtrl( this, wxID_ANY , wxEmptyString, wxDefaultPosition, dSize, wxTE_RIGHT | wxTE_READONLY,
                           wxTextValidator(wxFILTER_NUMERIC, &_strDY));
   _ctrlDY->SetForegroundColour(*wxWHITE);  _ctrlDY->SetBackgroundColour(*wxBLACK);

   dSize = GetTextExtent(wxT("99999999999"));// 1 digit more
   dSize.SetHeight(-1);
   _ctrlSel = DEBUG_NEW wxTextCtrl( this, wxID_ANY , wxEmptyString, wxDefaultPosition, dSize, wxTE_RIGHT | wxTE_READONLY,
                           wxTextValidator(wxFILTER_NUMERIC, &_strSel));
   _ctrlSel->SetForegroundColour(*wxWHITE);  _ctrlSel->SetBackgroundColour(*wxBLACK);

   AddControl( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE));
   AddControl(_ctrlXPos    );
   AddControl( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE));
   AddControl(_ctrlYPos    );
   AddControl( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("dX:"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE));
   AddControl(_ctrlDX      );
   AddControl( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("dY:"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE));
   AddControl(_ctrlDY      );
   AddControl( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Sel:"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE));
   AddControl(_ctrlSel     );
   Realize();
}

tui::CanvasStatus::~CanvasStatus()
{
//   delete _ctrlXPos;
//   delete _ctrlYPos;
//   delete _ctrlDX;
//   delete _ctrlDY;
//   delete _ctrlSel;
}

void tui::CanvasStatus::setXpos(wxString coordX)
{
   _strX = coordX;
   _ctrlXPos->GetValidator()->TransferToWindow();
}

void tui::CanvasStatus::setYpos(wxString coordY)
{
   _strY = coordY;
   _ctrlYPos->GetValidator()->TransferToWindow();
}

void tui::CanvasStatus::setdXpos(wxString coordX)
{
   _strDX = coordX;
   _ctrlDX->GetValidator()->TransferToWindow();
}

void tui::CanvasStatus::setdYpos(wxString coordY)
{
   _strDY = coordY;
   _ctrlDY->GetValidator()->TransferToWindow();
}

void tui::CanvasStatus::setSelected(wxString numsel)
{
   _strSel = numsel;
   _ctrlSel->GetValidator()->TransferToWindow();
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(tui::ExternalProcess, wxProcess)
   EVT_TIMER(wxID_ANY, tui::ExternalProcess::OnTimer)
END_EVENT_TABLE()

tui::ExternalProcess::ExternalProcess(wxEvtHandler* parent) :
   wxProcess     ( parent       ),
   _idleTimer    ( this         ),
   _tes          (         NULL ),
   _tis          (         NULL )
{
   Redirect();
   _idleTimer.Start(100);
}

void tui::ExternalProcess::OnTerminate(int pid, int status)
{
   wxTextInputStream tes(*GetErrorStream());
   if (IsErrorAvailable())
   {
      wxString msg;
      msg << tes.ReadLine();
      if (!msg.IsEmpty())
         tell_log(console::MT_SHELLERROR,msg);
   }
   wxTextInputStream tis(*GetInputStream());
   while (IsInputAvailable())
   {
      wxString msg;
      msg << tis.ReadLine();
      if (!msg.IsEmpty())
         tell_log(console::MT_SHELLINFO,msg);
   }
   _idleTimer.Stop();
   //Post an event to notify the console, that the external command has exited
   wxCommandEvent eventExecExDone(wxEVT_EXECEXTDONE);
   wxPostEvent(Console, eventExecExDone);

   delete this;
}

void tui::ExternalProcess::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   wxTextInputStream tes(*GetErrorStream());
   if (IsErrorAvailable())
   {
      wxString msg;
      msg << tes.ReadLine();
      if (!msg.IsEmpty())
         tell_log(console::MT_SHELLERROR,msg);
   }
   wxTextInputStream tis(*GetInputStream());
   while (IsInputAvailable())
   {
      wxString msg;
      msg << tis.ReadLine();
      if (!msg.IsEmpty())
         tell_log(console::MT_SHELLINFO,msg);
   }
//   wxTextOutputStream tos(*GetOutputStream());
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
//   EVT_MENU( TMOAS_EXPORTL       , tui::TopedFrame::OnOASexportLIB)
//   EVT_MENU( TMOAS_EXPORTC       , tui::TopedFrame::OnOASexportCELL)
   EVT_MENU( TMOAS_CLOSE         , tui::TopedFrame::OnOASclose    )

   EVT_MENU( TMFILE_SAVE         , tui::TopedFrame::OnTDTSave     )
   EVT_MENU( TMFILE_SAVEAS       , tui::TopedFrame::OnTDTSaveAs   )
   EVT_MENU( TMPROP_SAVE         , tui::TopedFrame::OnPropSave    )
   EVT_MENU( TMFILE_EXIT         , tui::TopedFrame::checkExit     )

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

   EVT_MENU( TMVIEW_ZOOMIN       , tui::TopedFrame::OnZoomIn      )
   EVT_MENU( TMVIEW_ZOOMOUT      , tui::TopedFrame::OnZoomOut     )
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

   EVT_MENU( TMSET_ALLPROP       , tui::TopedFrame::OnPropertySheet)

   EVT_MENU( TMSET_HTOOLSIZE16   , tui::TopedFrame::OnHToolBarSize16   )
   EVT_MENU( TMSET_HTOOLSIZE24   , tui::TopedFrame::OnHToolBarSize24   )
   EVT_MENU( TMSET_HTOOLSIZE32   , tui::TopedFrame::OnHToolBarSize32   )
   EVT_MENU( TMSET_HTOOLSIZE48   , tui::TopedFrame::OnHToolBarSize48   )

   EVT_MENU( TMSET_VTOOLSIZE16   , tui::TopedFrame::OnVToolBarSize16   )
   EVT_MENU( TMSET_VTOOLSIZE24   , tui::TopedFrame::OnVToolBarSize24   )
   EVT_MENU( TMSET_VTOOLSIZE32   , tui::TopedFrame::OnVToolBarSize32   )
   EVT_MENU( TMSET_VTOOLSIZE48   , tui::TopedFrame::OnVToolBarSize48   )

   EVT_MENU( TMSET_UNDODEPTH     , tui::TopedFrame::OnUndoDepth   )

   EVT_MENU( TMSET_DEFCOLOR      , tui::TopedFrame::OnDefineColor )
   EVT_MENU( TMSET_DEFFILL       , tui::TopedFrame::OnDefineFill  )
   EVT_MENU( TMSET_DEFSTYLE      , tui::TopedFrame::OnDefineStyle )
   EVT_MENU( TMSET_TECHEDITOR    , tui::TopedFrame::OnTechEditor  )

   EVT_MENU( TMADD_RULER         , tui::TopedFrame::OnAddRuler    )
   EVT_MENU( TMCLEAR_RULERS      , tui::TopedFrame::OnClearRulers )
   EVT_MENU( TMCADENCE_CONVERT   , tui::TopedFrame::OnCadenceConvert )
   EVT_MENU( TMGET_SNAPSHOT      , tui::TopedFrame::OnTDTSnapshot )
      // EVT_MENU( TMHELP_ABOUTAPP     , tui::TopedFrame::OnAbout       )
   EVT_MENU_RANGE(TMDUMMY    , TMDUMMY    +999 , tui::TopedFrame::OnMenu   )
   EVT_TOOL_RANGE(TDUMMY_TOOL, TDUMMY_TOOL+999 , tui::TopedFrame::OnToolBar)
   EVT_ICONIZE(tui::TopedFrame::OnIconize)
   EVT_CLOSE(tui::TopedFrame::OnClose)
//   EVT_SIZE( TopedFrame::OnSize )
//   EVT_TECUSTOM_COMMAND(  , wxID_ANY, tui::TopedFrame::OnTopedStatus)
   EVT_TECUSTOM_COMMAND(wxEVT_CANVAS_STATUS, wxID_ANY, tui::TopedFrame::OnCanvasStatus)
   EVT_TECUSTOM_COMMAND(wxEVT_RENDER_PARAMS, wxID_ANY, tui::TopedFrame::OnUpdateRenderParams)
   EVT_TECUSTOM_COMMAND(wxEVT_CANVAS_PARAMS, wxID_ANY, tui::TopedFrame::OnUpdateCanvasParams)
   EVT_TECUSTOM_COMMAND(wxEVT_MOUSE_ACCEL, wxID_ANY, tui::TopedFrame::OnMouseAccel)
   EVT_TECUSTOM_COMMAND(wxEVT_CURRENT_LAYER, wxID_ANY, tui::TopedFrame::OnCurrentLayer)
   EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_ENTER, tui::TopedFrame::OnUncapturedMouseClick)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARSIZE, wxID_ANY, tui::TopedFrame::OnToolBarSize)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARDEF,  wxID_ANY, tui::TopedFrame::OnToolBarDefine)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARADDITEM, wxID_ANY, tui::TopedFrame::OnToolBarAddItem)
   EVT_TECUSTOM_COMMAND(wxEVT_TOOLBARDELETEITEM, wxID_ANY, tui::TopedFrame::OnToolBarDeleteItem)
   EVT_TECUSTOM_COMMAND(wxEVT_EDITLAYER, wxID_ANY, tui::TopedFrame::OnEditLayer )
   EVT_TEXT_MAXLEN(ID_WIN_TXT_LOG, tui::TopedFrame::OnTextLogOverflow)
   EVT_TECUSTOM_COMMAND(wxEVT_EXITAPP, wxID_ANY, tui::TopedFrame::OnExitRequest)
   EVT_TECUSTOM_COMMAND(wxEVT_EXECEXT, wxID_ANY, tui::TopedFrame::OnExecExt)
   EVT_TECUSTOM_COMMAND(wxEVT_EXECEXTPIPE, wxID_ANY, tui::TopedFrame::OnExecExtTextEnter)
   EVT_TECUSTOM_COMMAND(wxEVT_RELOADTELLFUNCS, wxID_ANY, tui::TopedFrame::onReloadTellFuncs)
   EVT_TECUSTOM_COMMAND(wxEVT_CONSOLE_PARSE, wxID_ANY, tui::TopedFrame::onParseCommand)
END_EVENT_TABLE()

tui::TopedFrame::TopedFrame(const wxString& title, const wxPoint& pos,
                            const wxSize& size ) : wxFrame((wxFrame *)NULL, ID_WIN_TOPED, title, pos, size),_exitAproved(false)
{
   SetIcon(wxIcon( toped16x16_xpm ));
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
   _techEditor = NULL;
   // Initialize the post system
   _tPost = DEBUG_NEW TpdPost(this);
}


tui::TopedFrame::~TopedFrame() {
//   delete _laycanvas;
//   delete Console;
//   delete _GLstatus;
//   delete _browsers;
    _winManager.UnInit();
   delete _propDialog;
   delete _canvas;
   delete _resourceCenter;
   delete _tPost;
   delete _cmdline;
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
//   _resourceCenter->appendMenu("&File/Export to Oasis","",  &tui::TopedFrame::OnOASexportLIB, "Export DB to Oasis using default layer map");
   _resourceCenter->appendMenu("&File/Import Oasis","",  &tui::TopedFrame::OnOASimport, "Import Oasis file using default layer map" );

   _resourceCenter->appendMenu("&File/More Oasis .../Parse","", &tui::TopedFrame::OnOASRead, "Parse Oasis file" );
   _resourceCenter->appendMenu("&File/More Oasis .../Translate","", &tui::TopedFrame::OnOAStranslate, "Import Oasis structure" );
//   _resourceCenter->appendMenu("&File/More Oasis .../Export Cell", "", &tui::TopedFrame::OnOASexportCELL, "Export cell to Oasis" );
   _resourceCenter->appendMenu("&File/More Oasis .../Close","", &tui::TopedFrame::OnOASclose, "Clear the parsed Oasis file from memory" );


   _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Save",       "CTRL-S",  &tui::TopedFrame::OnTDTSave,  "Save the database");
   _resourceCenter->appendMenu("&File/Save as ...","",  &tui::TopedFrame::OnTDTSaveAs, "Save the database under a new name" );
   _resourceCenter->appendMenu("&File/Save properties...","",  &tui::TopedFrame::OnPropSave, "Save the layout properties" );
   _resourceCenter->appendMenuSeparator("&File");
  // _resourceCenter->appendMenu("&File/Snapshot ...","",  &tui::TopedFrame::OnTDTSnapshot, "Export screen to picture" );
  // _resourceCenter->appendMenuSeparator("&File");
   _resourceCenter->appendMenu("&File/Exit",        "",  &tui::TopedFrame::checkExit, "Exit Toped" );


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

   _resourceCenter->appendMenu("&View/Zoom in", "F2",  &tui::TopedFrame::OnZoomIn, "Zoom in current window" );
   _resourceCenter->appendMenu("&View/Zoom out","F3",  &tui::TopedFrame::OnZoomOut, "Zoom out current window" );
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
   settingsMenu->Append         (TMSET_ALLPROP  , wxT("Properties..."), wxT("Toped Properties"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_HTOOLSIZE   , wxT("Toolbar size") , toolbarHorSizeMenu , wxT("Define toolbars size"));
   settingsMenu->Append         (TMSET_UNDODEPTH   , wxT("Undo Depth")   , wxT("Change the depth of undo stack"));
   //settingsMenu->Append         (TMSET_HTOOLSIZE   , wxT("H. toolbar size") , toolbarHorSizeMenu , wxT("Define horizontal toolbars size"));
   //settingsMenu->Append         (TMSET_VTOOLSIZE   , wxT("V. toolbar size") , toolbarVertSizeMenu , wxT("Define vertical toolbars size"));
   settingsMenu->AppendSeparator();
   settingsMenu->Append         (TMSET_DEFCOLOR , wxT("Define Color") , wxT("Define a drawing color"));
   settingsMenu->Append         (TMSET_DEFFILL  , wxT("Define Fill")  , wxT("Define a drawing pattern"));
   settingsMenu->Append         (TMSET_DEFSTYLE , wxT("Define Style") , wxT("Define a style of lines"));
   settingsMenu->Append         (TMSET_TECHEDITOR, wxT("Technology Editor")  , wxT("Define a technology"));
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

void tui::TopedFrame::setIconDir(const wxString& uiDir)
{
   if (_resourceCenter) _resourceCenter->setIconDir(uiDir);
}

void  tui::TopedFrame::setActiveCmd()
{
   _cmdline->getWidget()->SetFocus();
}

void tui::TopedFrame::initToolBars()
{
   wxAuiToolBar* tbA = _resourceCenter->initToolBarA(this);
   wxAuiToolBar* tbB = _resourceCenter->initToolBarB(this);
   wxAuiToolBar* tbC = _resourceCenter->initToolBarC(this);
   _GLstatus = DEBUG_NEW CanvasStatus(this, ID_WIN_GLSTATUS , wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_TEXT);

   getAuiManager()->AddPane(tbA, wxAuiPaneInfo()
                                .ToolbarPane()
                                .Top()
                                .Floatable()
                                .Row(1)
                                .Position(1)
                           );
   getAuiManager()->AddPane(tbB, wxAuiPaneInfo()
                                .ToolbarPane()
                                .Top()
                                .Floatable()
                                .Row(1)
                                .Position(2)
                           );
   getAuiManager()->AddPane(tbC, wxAuiPaneInfo()
                                .ToolbarPane()
                                .Top()
                                .Floatable()
                                .Row(1)
                                .Position(3)
                           );
   getAuiManager()->AddPane(_GLstatus, wxAuiPaneInfo()
                                .ToolbarPane()
                                .Top()
                                .Floatable()
                                .Gripper()
                                .Name(wxT("Status"))
                                .GripperTop(false)
                                .TopDockable(true)
                                .BottomDockable(true)
                                .LeftDockable(false)
                                .RightDockable(false)
                                .Row(2)
                            );
   getAuiManager()->Update();
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

   wxFileName helpFile(wxGetApp().globalDir());
//   rcFile.AppendDir(wxT("tll"));
   helpFile.SetFullName(wxT("funchelp.txt"));
   helpFile.Normalize();
   assert(helpFile.IsOk());
   console::HelpObject *helpObject = DEBUG_NEW console::HelpObject(helpFile/*wxGetApp().globalDir()+ wxT("funchelp.txt")*/);
   _cmdbrowser->setHelpObject(helpObject);
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

   //----------------------------------------------------------------------------
   // The command line
   //----------------------------------------------------------------------------
   wxTextCtrl* cmdlineW = DEBUG_NEW wxTextCtrl( this, tui::ID_CMD_LINE, wxT(""),
                                                wxDefaultPosition,
                                                wxSize(1000, 30),
                                                wxTE_PROCESS_ENTER | wxNO_BORDER );
   _cmdline = DEBUG_NEW console::TedCmdLine(_canvas, cmdlineW);

// cmdlineW->SetWindowStyleFlag(wxSW_3D | wxCLIP_CHILDREN);

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

   _winManager.AddPane(cmdlineW,    wxAuiPaneInfo().
                                    Bottom().
                                    Row(0).
                                    BestSize(wxSize(1000,30)).
                                    Floatable(false).
                                    CloseButton(false).
                                    CaptionVisible(false)
                      );
   Show();
   _winManager.Update();
   // At this stage the initial openGL context is considered valid - so we shall be
   // able to initialize glew
   _canvas->glewContext();
}


void tui::TopedFrame::OnExecExt( wxCommandEvent& event )
{
   wxString extCmd = event.GetString();
//   wxShell(extCmd);
//   ExternalProcess* extProc = DEBUG_NEW ExternalProcess(this);
   _extProc = DEBUG_NEW ExternalProcess(this);

//   Connect(-1, wxEVT_EXECEXTPIPE,
//           (wxObjectEventFunction) (wxEventFunction)
//           (wxCommandEventFunction)&ExternalProcess::OnTextEnter);
//

   int returnCode = wxExecute(extCmd, wxEXEC_ASYNC, _extProc);
   if ( 0 == returnCode )
   {
      //Post an event to notify the console, that the external command has exited
      wxCommandEvent eventExecExDone(wxEVT_EXECEXTDONE);
      wxPostEvent(Console, eventExecExDone);
   }
}

void tui::TopedFrame::OnExecExtTextEnter(wxCommandEvent& event)
{
   wxTextOutputStream tos(*(_extProc->GetOutputStream()));
   tos << event.GetString();
   _extProc->CloseOutput();
}

void tui::TopedFrame::checkExit( wxCommandEvent& WXUNUSED( event ) )
{
   if (DATC->modified())
   {
      wxMessageDialog dlg1(this,
                           wxT("Save the current design before closing?\n(Cancel to continue the session)"),
                           wxT("Current design contains unsaved data"),
                           wxYES_NO | wxCANCEL | wxICON_QUESTION);
      switch (dlg1.ShowModal())
      {
         case wxID_YES:{
            wxCommandEvent sevent(wxEVT_CLOSE_WINDOW, m_windowId);
            sevent.SetEventObject(this);
            //GetEventHandler()->ProcessEvent(event);
            OnTDTSave(sevent);
            setExitAproved(); 
         }
         case wxID_NO:  
            Console->stopParserThread();
            setExitAproved();            
         case wxID_CANCEL: return;
      }
   }
   else
   {
      Console->stopParserThread();
      setExitAproved();
   }
}

void tui::TopedFrame::OnExitRequest( wxCommandEvent&  event )
{
  switch (event.GetInt())
   {
      case 0 : Close(FALSE); break; // Exit the application
      case 1 : checkExit(event) ; break; // Make pre-exit checks
      default: assert(false); break;
   }
}

void tui::TopedFrame::OnClose(wxCloseEvent& WXUNUSED( event ))
{
   wxCommandEvent evt;
   //Next string is added because this method is calling by two ways
   //1. Directly using Close system button (need to call checkExit())
   //2. Indirectly when parse thread is close in TopedFrame::checkExit 
   //   by Console->stopParserThread(); (checkExit() already called)
   if (! exitAproved()) checkExit(evt);
   if (exitAproved())
   {
      wxMilliSleep(300);
      Destroy();
   }
}

void tui::TopedFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) ) {
    wxAboutDialogInfo info;
    info.SetName(wxT("Toped"));
    info.SetVersion(wxT("0.9.x"));
    info.SetIcon(wxIcon( toped32x32_xpm ));
    info.SetWebSite(wxT("www.toped.org.uk"));
    info.SetDescription(wxT("Open source IC layout editor"));
    info.SetCopyright(wxT("(C) 2001-2012 Toped developers"));

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
      default: assert(false); break;
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
      Console->parseCommand(ost);
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
   wxString dbfext;
   dbfext << wxT("Toped files(*.tdt;*.gz;*.zip)|*.tdt;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a design to open"), wxT(""), wxT(""),
                     dbfext, tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      wxString filename = dlg2.GetPath();
      wxString ost;
      ost << wxT("tdtread(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
      wxString ost1;
//      ost1 << wxT("File ") << dlg2.GetPath() << wxT(" loaded");
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
      wxString filename = dlg2.GetPath();
      wxString ost;
      ost << wxT("loadlib(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
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
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost;
      ost << wxT("unloadlib(\"") << dlg->get_selected() << wxT("\");");
      Console->parseCommand(ost);
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
      wxString filename = dlg2.GetPath();
      wxString ost;
      ost << wxT("#include \"") << filename << wxT("\";");
      Console->parseCommand(ost);
//      SetStatusText(dlg2.GetFilename() + wxT(" parsed"));
   }
   else SetStatusText(wxT("include aborted"));
}

void tui::TopedFrame::OnGDSRead(wxCommandEvent& WXUNUSED(event))
{
   wxString dbfext;
   dbfext << wxT("Stream files(*.gds;*.sf;*.gz;*.zip)|*.gds;*.sf;*.GDS;*.SF;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""), dbfext,
      tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing GDS file..."));
      wxString filename = dlg2.GetPath();
      wxString ost;
      ost << wxT("gdsread(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
//      wxString ost1;
//      ost1 << wxT("Stream ") << dlg2.GetPath() << wxT(" loaded");
//      SetStatusText(ost1);
   }
   else SetStatusText(wxT("Parsing aborted"));
}

void tui::TopedFrame::OnTDTSave(wxCommandEvent&  callingEvent)
{
   std::string  tedFileName;
   bool         tedNeverSaved = false;
   bool         tedDBExists = false;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      tedFileName = dbLibDir->tedFileName();
      tedNeverSaved = dbLibDir->neverSaved();
      tedDBExists = true;
   }
   DATC->unlockTDT(dbLibDir);
   if (!tedDBExists)
   {
      wxMessageDialog dlg1(this,
            wxT("No database in memory. Nothing to save"),
            wxT("Toped"),
            wxOK | wxICON_WARNING);
      dlg1.ShowModal();
      return;
   }
   //
   wxString wxfilename(tedFileName.c_str(), wxConvUTF8);
   wxFileName datafile( wxfilename );
   assert(datafile.IsOk());
   SetStatusText(wxT("Saving file..."));
   wxString ost;
   ost << wxT("tdtsave();");
   bool threadExecution = (wxEVT_CLOSE_WINDOW != callingEvent.GetEventType());
   if (datafile.FileExists() && tedNeverSaved)
   {
      wxMessageDialog dlg1(this,
         wxT("File ") + wxfilename + wxT(" already exists. Overwrite ?"),
         wxT("Toped"),
         wxYES_NO | wxICON_QUESTION);
      switch (dlg1.ShowModal()) {
         case wxID_YES:Console->parseCommand(ost, threadExecution); //Overwrite;
         case wxID_NO: return;
      }
   }
   else Console->parseCommand(ost, threadExecution);
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
      ost << wxT("tdtsaveas(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
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
      ost << wxT("propsave(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
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
   wxString filename = dlg2.GetPath();
   if(!checkFileOverwriting(filename)) return;

   // Note the pragmas below. It won't create a proper targa file without them!
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

void tui::TopedFrame::OnCopy(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("copy();"));
}

void tui::TopedFrame::OnMove(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("move();"));
}

void tui::TopedFrame::OnDelete(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("delete();"));
}

void tui::TopedFrame::OnRotate(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("rotate(90);"));
}

void tui::TopedFrame::OnFlipVert(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("flip(_vertical);"));
}

void tui::TopedFrame::OnFlipHor(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("flip(_horizontal);"));
}

void tui::TopedFrame::OnPolyCut(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("polycut();"));
}

void tui::TopedFrame::OnBoxCut(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("boxcut();"));
}

void tui::TopedFrame::OnMerge(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("merge();"));
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
      Console->parseCommand(ost);
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
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << wxT("opencell(\"") << dlg->get_selectedcell() << wxT("\");");
      Console->parseCommand(ost);
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
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
      ost << wxT("removecell(\"") << dlg->get_selectedcell() << wxT("\");");
      Console->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnCellPush(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("editpush(getpoint());"));
}

void tui::TopedFrame::OnCellPop(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("editpop();"));
}

void tui::TopedFrame::OnCellTop(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("edittop();"));
}

void tui::TopedFrame::OnCellPrev(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("editprev();"));
}


void tui::TopedFrame::OnGDStranslate(wxCommandEvent& WXUNUSED(event)) {
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      tui::GetGDSimport* dlg = NULL;
      ExpLayMap* laymap;
      try {
         dlg = DEBUG_NEW tui::GetGDSimport(this, -1, wxT("Import GDS structure"), pos,
                                             _browsers->tdtSelectedGdsName(), drawProp);
      }
      catch (EXPTN&) {delete dlg;return;}
      if ( dlg->ShowModal() == wxID_OK )
      {
         laymap = dlg->getGdsLayerMap();
         ExpLayMap* laymap2save = NULL;
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
   PROPC->unlockDrawProp(drawProp, false);
   if (success)
      Console->parseCommand(ost);


}

void tui::TopedFrame::OnGDSimport(wxCommandEvent& WXUNUSED(event))
{
   wxString dbfext;
   dbfext << wxT("Stream files(*.gds;*.sf;*.gz;*.zip)|*.gds;*.sf;*.GDS;*.SF;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""), dbfext,
                         tpdfOPEN);
   if (wxID_OK != dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing aborted")); return;
   }
   SetStatusText(wxT("Importing GDS file..."));
   wxString filename = dlg2.GetPath();
   wxString ost_int;
   ost_int << wxT("gdsread(\"") << filename << wxT("\")");
   wxString ost;
   ost << wxT("gdsimport(") << ost_int << wxT(", getgdslaymap(true), true, false );gdsclose();");
   Console->parseCommand(ost);
//   SetStatusText(wxT("Stream ")+dlg2.GetPath()+wxT(" imported"));
}

void tui::TopedFrame::OnCIFimport(wxCommandEvent& WXUNUSED(event))
{
   wxString dbfext;
   dbfext << wxT("Caltech files(*.cif;*.gz;*.zip)|*.cif;*.CIF;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""), dbfext, tpdfOPEN);
   if (wxID_OK != dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing aborted")); return;
   }
   SetStatusText(wxT("Importing CIF file..."));
   wxString filename = dlg2.GetPath();
   wxString ost_int;
   ost_int << wxT("cifread(\"") << filename << wxT("\")");
   wxString ost;
   ost << wxT("cifimport(") << ost_int << wxT(", getciflaymap(true), true, false, 0.0 );cifclose();");
   Console->parseCommand(ost);
//   SetStatusText(wxT("Stream ")+dlg2.GetPath()+wxT(" imported"));
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
      ost << wxT("gdsexport(getgdslaymap(false), \"") << filename << wxT("\", false);");
      Console->parseCommand(ost);
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
      ost << wxT("cifexport(getciflaymap(false), \"") << filename << wxT("\", false);");
      Console->parseCommand(ost);
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
      tui::GetGDSexport* dlg = NULL;
      try {
         dlg = DEBUG_NEW tui::GetGDSexport(this, -1, wxT("GDS export cell"), pos,
                                           _browsers->tdtSelectedCellName(), drawProp);
      }
      catch (EXPTN&) {delete dlg;return;}
      wxString cellname;
      bool recur;
      ExpLayMap* laymap;
      ExpLayMap* laymap2save = NULL;
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
               << filename
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
   PROPC->unlockDrawProp(drawProp, false);
   if (success)
      Console->parseCommand(ost);
}

void tui::TopedFrame::OnGDSclose(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("gdsclose();"));
}

void tui::TopedFrame::OnCIFRead(wxCommandEvent& WXUNUSED(event))
{
   wxString dbfext;
   dbfext << wxT("Caltech files(*.cif;*.gz;*.zip)|*.cif;*.CIF;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""), dbfext, tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal()) {
      SetStatusText(wxT("Parsing CIF file..."));
      wxString filename = dlg2.GetPath();
      wxString ost;
      ost << wxT("cifread(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
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
      tui::GetCIFimport* dlg = NULL;
      try {
         dlg = DEBUG_NEW tui::GetCIFimport(this, -1, wxT("Import CIF structure"), pos,
                                           _browsers->tdtSelectedCifName(), drawProp);
      }
      catch (EXPTN&) {delete dlg;return;}
      if ( dlg->ShowModal() == wxID_OK )
      {
         // get the layer map first
         ImpLayMap* laymap = dlg->getCifLayerMap(drawProp);
         ExpLayMap* laymap2save = NULL;
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
   PROPC->unlockDrawProp(drawProp, false);
   if (success) Console->parseCommand(ost);
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
      tui::GetCIFexport* dlg = NULL;
      try {
         dlg = DEBUG_NEW tui::GetCIFexport(this, -1, wxT("CIF export cell"), pos,
                                           _browsers->tdtSelectedCellName(), drawProp);
      }
      catch (EXPTN&) {delete dlg;return;}
      wxString cellname;
      bool recur;
      bool sverbose;
      ExpLayMap* laymap;
      ExpLayMap* laymap2save = NULL;
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
               <<  filename << wxT("\", ")
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
   PROPC->unlockDrawProp(drawProp, false);
   if (success)
   {
      Console->parseCommand(ost);
//      SetStatusText(wxT("Design exported to: ")+dlg2.GetFilename());
   }
}

void tui::TopedFrame::OnCIFclose(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("cifclose();"));
}

void tui::TopedFrame::OnOASRead(wxCommandEvent& WXUNUSED(event))
{
   wxString dbfext;
   dbfext << wxT("Oasis files(*.oas;*.gz;*.zip)|*.oas;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""), dbfext, tpdfOPEN);
   if (wxID_OK == dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing Oasis file..."));
      wxString filename = dlg2.GetPath();
      wxString ost;
      ost << wxT("oasisread(\"") << filename << wxT("\");");
      Console->parseCommand(ost);
   }
   else SetStatusText(wxT("Parsing aborted"));
}

void tui::TopedFrame::OnOASimport(wxCommandEvent& WXUNUSED(event))
{
   wxString dbfext;
   dbfext << wxT("Oasis files(*.oas;*.gz;*.zip)|*.oas;*.gz;*.zip;*.GZ;*.ZIP")
          << wxT("|All files(*.*)|*.*");
   wxFileDialog dlg2(this, wxT("Select a file"), wxT(""), wxT(""), dbfext, tpdfOPEN);
   if (wxID_OK != dlg2.ShowModal())
   {
      SetStatusText(wxT("Parsing aborted")); return;
   }
   SetStatusText(wxT("Importing Oasis file..."));
   wxString filename = dlg2.GetPath();
   wxString ost_int;
   ost_int << wxT("oasisread(\"") << filename << wxT("\")");
   wxString ost;
   ost << wxT("oasisimport(") << ost_int << wxT(", getoasislaymap(true), true, false );oasisclose();");
   Console->parseCommand(ost);
//   SetStatusText(wxT("Stream ")+dlg2.GetPath()+wxT(" imported"));
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
      tui::GetOASimport* dlg = NULL;
      ExpLayMap* laymap;
      try {
         dlg = DEBUG_NEW tui::GetOASimport(this, -1, wxT("Import OASIS structure"), pos,
                                             _browsers->tdtSelectedOasName(), drawProp);
      }
      catch (EXPTN&) {delete dlg;return;}
      if ( dlg->ShowModal() == wxID_OK )
      {
         laymap = dlg->getOasLayerMap();
         ExpLayMap* laymap2save = NULL;
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
   PROPC->unlockDrawProp(drawProp,false);
   if (success)
      Console->parseCommand(ost);
}

//void tui::TopedFrame::OnOASexportLIB(wxCommandEvent& WXUNUSED(event))
//{
//   //@TODO
//}

//void tui::TopedFrame::OnOASexportCELL(wxCommandEvent& WXUNUSED(event))
//{
//   //@TODO
//}

void tui::TopedFrame::OnOASclose(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("oasisclose();"));
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
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost;
/*      ost << wxT("cellref(\"") << dlg->get_selectedcell() <<wxT("\",getpoint(),")
          <<                     dlg->get_angle() << wxT(",")
          << (dlg->get_flip() ? wxT("true") : wxT("false")) << wxT(",")
          <<                                wxT("1")  << wxT(");");*/
      ost << wxT("cellref(\"") << dlg->get_selectedcell() << wxT("\");");
      Console->parseCommand(ost);
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
   catch (EXPTN&) {delete dlg; return;}
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
      Console->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnCellGroup(wxCommandEvent& WXUNUSED(event)) {
   // Here - try a hollow lock/unlock just to check that it exists
   try
   {
      laydata::TdtLibDir* dbLibDir = NULL;
      DATC->lockTDT(dbLibDir, dbmxs_celllock);
      DATC->unlockTDT(dbLibDir, true);
   }
   catch (EXPTN&) {return;}
   //
   wxTextEntryDialog dlg2(this,
      wxT("Cell name:"),
      wxT("Create new cell from selected components"));
   wxString cname, ost;
   if ((wxID_OK == dlg2.ShowModal()) && ((cname = dlg2.GetValue()) != wxT(""))) {
      SetStatusText(wxT("Grouping in a new cell..."));
      ost << wxT("group(\"") << cname << wxT("\");");
      Console->parseCommand(ost);
   }
   else SetStatusText(wxT("Groupping canceled"));
}

void tui::TopedFrame::OnCellUngroup(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("ungroup();"));
}

void tui::TopedFrame::OnZoomVisible(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("zoomvisible();"));
}

void tui::TopedFrame::OnDrawBox(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("addbox();"));
}

void tui::TopedFrame::OnDrawPoly(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("addpoly();"));
}

void tui::TopedFrame::OnSelectIn(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("select();"));
}

void tui::TopedFrame::OnUnselectIn(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("unselect();"));
}

void tui::TopedFrame::OnPselectIn(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("pselect();"));
}

void tui::TopedFrame::OnPunselectIn(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("punselect();"));
}

void tui::TopedFrame::OnUnselectAll(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("unselect_all();"));
}

void tui::TopedFrame::OnReportSelected(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("report_selected();"));
}

void tui::TopedFrame::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("select_all();"));
}

void tui::TopedFrame::OnDrawWire(wxCommandEvent& WXUNUSED(event)) {
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Wire width"), pos, PROPC->step() ,3, 2);
   }
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost; ost << wxT("addwire(")<<dlg->value()<<wxT(");");
      Console->parseCommand(ost);
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
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost; ost << wxT("addtext(\"")
                        << dlg->get_text()                      << wxT("\",")
                        << dlg->get_size()                      << wxT(");");
      Console->parseCommand(ost);
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
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK ) {
      wxString ost; ost << wxT("resize(")<<dlg->value()<<wxT(");");
      Console->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnUndoDepth(wxCommandEvent& WXUNUSED(event))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getSize* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getSize(this, -1, wxT("Undo depth"), pos, 1, 0, CMDBlock->undoDepth());
   }
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      unsigned long newValue;
      if (dlg->value().ToULong(&newValue))
      {
         wxString ost;
         ost << wxT("setparams( {\"UNDO_DEPTH\", \"")
             << newValue
             << wxT("\"});");
         Console->parseCommand(ost);
      }
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
      Console->parseCommand(ost);
   }
}

void tui::TopedFrame::OnHToolBarSize16(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_horizontal, _iconsize16);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize24(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_horizontal, _iconsize24);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize32(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_horizontal, _iconsize32);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnHToolBarSize48(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_horizontal, _iconsize48);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize16(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_vertical, _iconsize16);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize24(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_vertical, _iconsize24);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize32(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_vertical, _iconsize32);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnVToolBarSize48(wxCommandEvent& WXUNUSED(event))
{
   wxString ost;
   ost << wxT("toolbarsize(_vertical, _iconsize48);");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnPropertySheet(wxCommandEvent& WXUNUSED(event))
{
   _propDialog->Show();
}

void tui::TopedFrame::OnEditLayer(wxCommandEvent& evt)
{
   LayerDef* laydef = static_cast<LayerDef*>(evt.GetClientData());
   _techEditor = DEBUG_NEW TechEditorDialog(this,ID_TECH_EDITOR, *laydef);
   TpdPost::SetTechEditWindow(_techEditor);
   _techEditor->ShowModal();
   delete _techEditor;
   delete laydef;
   _techEditor = NULL;
   TpdPost::SetTechEditWindow(_techEditor);
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
      tui::DefineColor dlg(this, -1, wxT("Color Definitions"), pos, drawprop);
      if ( dlg.ShowModal() == wxID_OK )
      {
         const tui::DefineColor::ColorLMap colors = dlg.allColors();
         for(tui::DefineColor::ColorLMap::const_iterator CC = colors.begin() ; CC != colors.end(); CC++)
         {
            if (CC->second.first)
            {
               const layprop::tellRGB coldef = CC->second.second;
               ost   << wxT("definecolor(\"") << wxString(CC->first.c_str(), wxConvUTF8)
                     << wxT("\" , ")      << static_cast <int>(coldef.red())
                     << wxT(" , ")        << static_cast <int>(coldef.green())
                     << wxT(" , ")        << static_cast <int>(coldef.blue())
                     << wxT(" , ")        << static_cast <int>(coldef.alpha())
                     << wxT(");");
            }
         }
         success = true;
      }
   }
   PROPC->unlockDrawProp(drawprop, false);
   if (success) Console->parseCommand(ost);
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
      tui::DefineFill dlg(this, -1, wxT("Fill Definition"), pos, drawprop);
      if ( dlg.ShowModal() == wxID_OK )
      {
         tui::DefineFill::FillLMap patterns = dlg.allPatterns();
         for(tui::DefineFill::FillLMap::const_iterator CC = patterns.begin() ; CC != patterns.end(); CC++)
         {
            if (CC->second.first)
            {
               ost   << wxT("definefill(\"") << wxString(CC->first.c_str(), wxConvUTF8)
                     << wxT("\" , {");
               ost << static_cast <unsigned>(CC->second.second[0]);
               for (byte i = 1; i < 128; i++)
                  ost  << wxT(",") << static_cast <unsigned>(CC->second.second[i]);
               ost   << wxT("});");
               success = true;
            }
         }
      }
   }
   PROPC->unlockDrawProp(drawprop, false);
   if (success) Console->parseCommand(ost);
}

void tui::TopedFrame::OnDefineStyle(wxCommandEvent& WXUNUSED(event))
{
   bool success = false;
   wxString ost;
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   layprop::DrawProperties* drawprop;
   if (PROPC->lockDrawProp(drawprop))
   {
      tui::DefineLineStyle dlg(this, -1, wxT("Style Definition"), pos, drawprop);
      if ( dlg.ShowModal() == wxID_OK )
      {
         const tui::LineStyleMap styles = dlg.allStyles();
         for(tui::LineStyleMap::const_iterator CC = styles.begin() ; CC != styles.end(); CC++)
         {
            tui::LineStyleRecord styleDef = CC->second;
            if (styleDef.modified)
            {
               ost   << wxT("defineline(\"") << wxString(CC->first.c_str(), wxConvUTF8)
                     << wxT("\" , \"\" , ")      << styleDef.pattern
                     << wxT(" , ")        << static_cast<unsigned int>(styleDef.pscale)
                     << wxT(" , ")        << static_cast<unsigned int>(styleDef.width)
                     << wxT(");");
            }
         }
         success = true;
      }
   }
   PROPC->unlockDrawProp(drawprop, false);
   if (success) Console->parseCommand(ost);
}


void tui::TopedFrame::OnTechEditor(wxCommandEvent& WXUNUSED(event))
{
   _techEditor = DEBUG_NEW TechEditorDialog(this,ID_TECH_EDITOR, ERR_LAY_DEF);
   TpdPost::SetTechEditWindow(_techEditor);
   _techEditor->ShowModal();
   delete _techEditor;
   _techEditor = NULL;
   TpdPost::SetTechEditWindow(_techEditor);
}

void tui::TopedFrame::OnChangeRef( wxCommandEvent& WXUNUSED( event ))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::getCellRef* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::getCellRef(this, -1, wxT("Change Cell Reference"), pos, wxT(""));
   }
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost;
      ost << wxT("changeref(\"") << dlg->get_selectedcell() << wxT("\");");
      Console->parseCommand(ost);
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
      Console->parseCommand(ost);
   }
}

void tui::TopedFrame::OnChangeLayer( wxCommandEvent& WXUNUSED( event ))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::DefaultLayer* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::DefaultLayer(this, wxID_ANY, wxT("Transfer to layer"), pos, false);
   }
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      wxString ost; ost << wxT("changelayer({") << dlg->value().num() << wxT(",") << dlg->value().typ() << wxT("});");
      Console->parseCommand(ost);
   }
   delete dlg;
}

void tui::TopedFrame::OnCurrentLayer( wxCommandEvent& WXUNUSED( event ))
{
   wxRect wnd = GetRect();
   wxPoint pos(wnd.x+wnd.width/2-100,wnd.y+wnd.height/2-50);
   tui::DefaultLayer* dlg = NULL;
   try {
      dlg = DEBUG_NEW tui::DefaultLayer(this, wxID_ANY, wxT("Change current layer"), pos, true);
   }
   catch (EXPTN&) {delete dlg;return;}
   if ( dlg->ShowModal() == wxID_OK )
   {
      LayerDef laydef = dlg->value();
      if (ERR_LAY_DEF != laydef)
         DATC->setCmdLayer(dlg->value());
   }
   delete dlg;
}

void tui::TopedFrame::OnMenu(wxCommandEvent& event)
{
   _resourceCenter->executeMenu(event.GetId());
}

void tui::TopedFrame::OnToolBar(wxCommandEvent& event)
{
   _resourceCenter->executeToolBar(event.GetId());
}

void tui::TopedFrame::OnUpdateRenderParams(wxCommandEvent& evt)
{
   _propDialog->updateRenderSheet(evt);
};

void tui::TopedFrame::OnUpdateCanvasParams(wxCommandEvent& evt)
{
   _propDialog->updateCanvasSheet(evt);
}

void  tui::TopedFrame::OnMouseAccel(wxCommandEvent& evt) {
   wxString ost;
   if (0 == evt.GetInt())
      ost << wxT("unselect(");
   else if (1 == evt.GetInt())
      ost << wxT("select(");
   else return;
   ost << evt.GetString() << wxT(");");
   Console->parseCommand(ost);
}

void tui::TopedFrame::OnZoomAll(wxCommandEvent& WXUNUSED(event)) {
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DBbox* ovl  = DEBUG_NEW DBbox(tDesign->activeOverlap());
      wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
      eventZOOM.SetInt(tui::ZOOM_WINDOW);
      eventZOOM.SetClientData(static_cast<void*>(ovl));
      wxPostEvent(_canvas, eventZOOM);
   }
   DATC->unlockTDT(dbLibDir, false);
}

void tui::TopedFrame::OnUndo(wxCommandEvent& WXUNUSED(event)) {Console->parseCommand(wxT("undo();"));}

void tui::TopedFrame::OnZoomIn(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_IN);
   wxPostEvent(_canvas, eventZOOM);
}

void tui::TopedFrame::OnZoomOut(wxCommandEvent& WXUNUSED(event)) {
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
   telldata::TtPnt* p = static_cast<telldata::TtPnt*>(evt.GetClientData());
   delete p;
}

void tui::TopedFrame::OnToolBarSize(wxCommandEvent& evt)
{
   tui::IconSizes sz = static_cast<tui::IconSizes>(evt.GetInt());
   bool direction = (0l != evt.GetExtraLong());

   _resourceCenter->setToolBarSize(direction, sz);
   // update the menu state
   if (tui::_tuihorizontal == direction)
      switch (sz)
      {
         case  ICON_SIZE_16x16: settingsMenu->Check(TMSET_HTOOLSIZE16 , true );break;
         case  ICON_SIZE_24x24: settingsMenu->Check(TMSET_HTOOLSIZE24 , true );break;
         case  ICON_SIZE_32x32: settingsMenu->Check(TMSET_HTOOLSIZE32 , true );break;
         case  ICON_SIZE_48x48: settingsMenu->Check(TMSET_HTOOLSIZE48 , true );break;
         default: assert(false); break;
      }
   else
      switch (sz)
      {
         case  ICON_SIZE_16x16: settingsMenu->Check(TMSET_VTOOLSIZE16 , true );break;
         case  ICON_SIZE_24x24: settingsMenu->Check(TMSET_VTOOLSIZE24 , true );break;
         case  ICON_SIZE_32x32: settingsMenu->Check(TMSET_VTOOLSIZE32 , true );break;
         case  ICON_SIZE_48x48: settingsMenu->Check(TMSET_VTOOLSIZE48 , true );break;
         default: assert(false); break;
      }
}

void tui::TopedFrame::OnToolBarDefine(wxCommandEvent& evt)
{
   wxString toolBarName(evt.GetString());
   _resourceCenter->defineToolBar(toolBarName);
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
   wxString toolBarName(evt.GetString());
   wxStringClientData *data= static_cast<wxStringClientData*>(evt.GetClientObject());
   wxString toolName = data->GetData();
//   std::string toolName(str.mb_str(wxConvUTF8));

   _resourceCenter->deleteTool(toolBarName, toolName);
}

void   tui::TopedFrame::OnDRCResults(wxCommandEvent& WXUNUSED(evt))
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
      Console->parseCommand(ost);
   }

   //wxString ost;
   //ost << wxT("drccalibreimport(\"D:/toped/drc3/drc3.drc.results\");");
   //Console->parseCommand(ost);
}

void tui::TopedFrame::OnIconize(wxIconizeEvent& evt)
{
#if wxCHECK_VERSION(2,9,0)
   if (!evt.IsIconized())
#else
   if (!evt.Iconized())
#endif
   {
      wxCommandEvent eventREFRESH(wxEVT_CANVAS_ZOOM);
      eventREFRESH.SetInt(ZOOM_REFRESH);
      wxPostEvent(_canvas, eventREFRESH);
   }
}

void tui::TopedFrame::OnAddRuler(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("addruler();") );
}

void tui::TopedFrame::OnClearRulers(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxT("clearrulers();") );
}

void tui::TopedFrame::OnCadenceConvert(wxCommandEvent& WXUNUSED(event))
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
   // windows. Truncating the text log contents doesn't really help
//   int boza;
//   boza++;
//   int lalal = 2* boza;
}

void tui::TopedFrame::onReloadTellFuncs(wxCommandEvent& WXUNUSED(evt))
{
   wxGetApp().reloadInternalFunctions();
}

void tui::TopedFrame::onParseCommand(wxCommandEvent& evt)
{
   _cmdline->onParseCommand(evt);
}

void tui::TopedFrame::USMap2wxString(ExpLayMap* inmap, wxString& outmap)
{
   std::string soutmap;
   layprop::ExtLayerMap2String(inmap, soutmap);
   outmap = wxString(soutmap.c_str(), wxConvUTF8);
}

void tui::TopedFrame::SIMap2wxString(ImpLayMap* inmap, wxString& outmap)
{
   std::ostringstream laymapstr;
   word recno = 0;
   laymapstr << "{";
   for (ImpLayMap::const_iterator CLN = inmap->begin(); CLN != inmap->end(); CLN++)
   {
      if (recno != 0)
         laymapstr << ",";
      laymapstr << "{" << CLN->second << ",\"" << CLN->first << "\"}";
      recno++;
   }
   laymapstr << "}";
   outmap = wxString(laymapstr.str().c_str(), wxConvUTF8);
}
