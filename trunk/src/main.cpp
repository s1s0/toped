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
//  Creation date: Wed May  5 23:27:33 BST 2004
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
#include <wx/wx.h>
#include "toped.h"
#include "../tpd_DB/viewprop.h"
#include "tellibin.h"
#include "datacenter.h"

tui::TopedFrame*                 Toped = NULL;
layprop::ViewProperties*         Properties = NULL;
browsers::browserTAB*            Browsers = NULL;
DataCenter*                      DATC = NULL;
// from ted_prompt (console)
parsercmd::cmdBLOCK*             CMDBlock = NULL;
extern console::ted_cmd*         Console;

//-----------------------------------------------------------------------------
console::toped_logfile     LogFile;

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_MARKERPOSITION, 10000)
    DECLARE_EVENT_TYPE(wxEVT_CNVSSTATUSLINE, 10001)
    DECLARE_EVENT_TYPE(wxEVT_CMD_BROWSER   , 10002)
    DECLARE_EVENT_TYPE(wxEVT_LOG_ERRMESSAGE, 10003)
    DECLARE_EVENT_TYPE(wxEVT_MOUSE_ACCEL   , 10004)
    DECLARE_EVENT_TYPE(wxEVT_MOUSE_INPUT   , 10005)
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_ZOOM   , 10006)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_MARKERPOSITION)
DEFINE_EVENT_TYPE(wxEVT_CNVSSTATUSLINE)
DEFINE_EVENT_TYPE(wxEVT_CMD_BROWSER)
DEFINE_EVENT_TYPE(wxEVT_LOG_ERRMESSAGE) // -> to go to ted_prompt.cpp !
DEFINE_EVENT_TYPE(wxEVT_MOUSE_ACCEL)
DEFINE_EVENT_TYPE(wxEVT_MOUSE_INPUT)
DEFINE_EVENT_TYPE(wxEVT_CANVAS_ZOOM)

void InitInternalFunctions(parsercmd::cmdMAIN* mblock) {
   // First the internal types
   telldata::point_type* pntype = new telldata::point_type();
   telldata::box_type*   bxtype = new telldata::box_type(pntype);
   mblock->addGlobalType("point"     , pntype);
   mblock->addGlobalType("box"       , bxtype);
   // Internal variables next - Can't think of any for the moment
//   mblock->addID("$_CW", new ttwnd(Toped->_view->lp_BL, Toped->_view->lp_TR));
   //--------------------------------------------------------------------------
   // tell functions
   //--------------------------------------------------------------------------
   mblock->addFUNC("echo"             ,(new                     tellstdfunc::stdECHO(telldata::tn_void)));
   mblock->addFUNC("status"           ,(new               tellstdfunc::stdTELLSTATUS(telldata::tn_void)));
   mblock->addFUNC("undo"             ,(new                     tellstdfunc::stdUNDO(telldata::tn_void)));
   //
   mblock->addFUNC("report_selected"  ,(new              tellstdfunc::stdREPORTSLCTD(telldata::tn_void)));
   mblock->addFUNC("report_layers"    ,(new        tellstdfunc::stdREPORTLAY(TLISTOF(telldata::tn_int))));
   mblock->addFUNC("report_layers"    ,(new       tellstdfunc::stdREPORTLAYc(TLISTOF(telldata::tn_int))));
   mblock->addFUNC("report_gdslayers" ,(new                tellstdfunc::GDSreportlay(telldata::tn_void)));
   //
   mblock->addFUNC("newdesign"        ,(new                tellstdfunc::stdNEWDESIGN(telldata::tn_void)));
   mblock->addFUNC("newcell"          ,(new                  tellstdfunc::stdNEWCELL(telldata::tn_void)));
   mblock->addFUNC("gdsread"          ,(new          tellstdfunc::GDSread(TLISTOF(telldata::tn_string))));
   mblock->addFUNC("gdsimport"        ,(new                  tellstdfunc::GDSconvert(telldata::tn_void)));
   mblock->addFUNC("gdsexport"        ,(new                tellstdfunc::GDSexportLIB(telldata::tn_void)));
   mblock->addFUNC("gdsexport"        ,(new                tellstdfunc::GDSexportTOP(telldata::tn_void)));
   mblock->addFUNC("gdsclose"         ,(new                    tellstdfunc::GDSclose(telldata::tn_void)));
   mblock->addFUNC("tdtread"          ,(new                     tellstdfunc::TDTread(telldata::tn_void)));
   mblock->addFUNC("tdtsave"          ,(new                     tellstdfunc::TDTsave(telldata::tn_void)));
   mblock->addFUNC("tdtsaveas"        ,(new                   tellstdfunc::TDTsaveas(telldata::tn_void)));
   mblock->addFUNC("opencell"         ,(new                 tellstdfunc::stdOPENCELL(telldata::tn_void)));
   mblock->addFUNC("editpush"         ,(new                 tellstdfunc::stdEDITPUSH(telldata::tn_void)));
   mblock->addFUNC("editpop"          ,(new                  tellstdfunc::stdEDITPOP(telldata::tn_void)));
   mblock->addFUNC("edittop"          ,(new                  tellstdfunc::stdEDITTOP(telldata::tn_void)));
   mblock->addFUNC("editprev"         ,(new                 tellstdfunc::stdEDITPREV(telldata::tn_void)));
   mblock->addFUNC("usinglayer"       ,(new               tellstdfunc::stdUSINGLAYER(telldata::tn_void)));
   mblock->addFUNC("usinglayer"       ,(new             tellstdfunc::stdUSINGLAYER_S(telldata::tn_void)));
   mblock->addFUNC("addbox"           ,(new                 tellstdfunc::stdADDBOX(telldata::tn_layout)));
   mblock->addFUNC("addbox"           ,(new               tellstdfunc::stdADDBOX_D(telldata::tn_layout)));
   mblock->addFUNC("addbox"           ,(new                tellstdfunc::stdADDBOXr(telldata::tn_layout)));
   mblock->addFUNC("addbox"           ,(new              tellstdfunc::stdADDBOXr_D(telldata::tn_layout)));
   mblock->addFUNC("addbox"           ,(new                tellstdfunc::stdADDBOXp(telldata::tn_layout)));
   mblock->addFUNC("addbox"           ,(new              tellstdfunc::stdADDBOXp_D(telldata::tn_layout)));
   mblock->addFUNC("addpoly"          ,(new                tellstdfunc::stdADDPOLY(telldata::tn_layout)));
   mblock->addFUNC("addpoly"          ,(new              tellstdfunc::stdADDPOLY_D(telldata::tn_layout)));
   mblock->addFUNC("addwire"          ,(new                tellstdfunc::stdADDWIRE(telldata::tn_layout)));
   mblock->addFUNC("addwire"          ,(new              tellstdfunc::stdADDWIRE_D(telldata::tn_layout)));
   mblock->addFUNC("addtext"          ,(new                tellstdfunc::stdADDTEXT(telldata::tn_layout)));
   mblock->addFUNC("cellref"          ,(new                tellstdfunc::stdCELLREF(telldata::tn_layout)));
   mblock->addFUNC("cellaref"         ,(new               tellstdfunc::stdCELLAREF(telldata::tn_layout)));
   mblock->addFUNC("select"           ,(new        tellstdfunc::stdSELECT(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("select"           ,(new      tellstdfunc::stdSELECTIN(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("select"           ,(new      tellstdfunc::stdSELECT_I(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("select"           ,(new     tellstdfunc::stdSELECT_TL(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("pselect"          ,(new     tellstdfunc::stdPNTSELECT(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("pselect"          ,(new   tellstdfunc::stdPNTSELECT_I(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("unselect"         ,(new      tellstdfunc::stdUNSELECT(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("unselect"         ,(new    tellstdfunc::stdUNSELECT_I(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("unselect"         ,(new   tellstdfunc::stdUNSELECT_TL(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("unselect"         ,(new    tellstdfunc::stdUNSELECTIN(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("punselect"        ,(new   tellstdfunc::stdPNTUNSELECT(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("punselect"        ,(new tellstdfunc::stdPNTUNSELECT_I(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("select_all"       ,(new     tellstdfunc::stdSELECTALL(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("unselect_all"     ,(new              tellstdfunc::stdUNSELECTALL(telldata::tn_void)));
   // operation on the toped data
   mblock->addFUNC("move"             ,(new                  tellstdfunc::stdMOVESEL(telldata::tn_void)));
   mblock->addFUNC("move"             ,(new                tellstdfunc::stdMOVESEL_D(telldata::tn_void)));
   mblock->addFUNC("copy"             ,(new           tellstdfunc::stdCOPYSEL(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("copy"             ,(new         tellstdfunc::stdCOPYSEL_D(TLISTOF(telldata::tn_layout))));
   mblock->addFUNC("rotate"           ,(new                tellstdfunc::stdROTATESEL(telldata::tn_void)));
   mblock->addFUNC("flipX"            ,(new                 tellstdfunc::stdFLIPXSEL(telldata::tn_void)));
   mblock->addFUNC("flipY"            ,(new                 tellstdfunc::stdFLIPYSEL(telldata::tn_void)));
   mblock->addFUNC("delete"           ,(new                tellstdfunc::stdDELETESEL(telldata::tn_void)));
   mblock->addFUNC("group"            ,(new                    tellstdfunc::stdGROUP(telldata::tn_void)));
   mblock->addFUNC("ungroup"          ,(new                  tellstdfunc::stdUNGROUP(telldata::tn_void)));
   //
   mblock->addFUNC("polycut"          ,(new                  tellstdfunc::lgcCUTPOLY(telldata::tn_void)));
   mblock->addFUNC("polycut"          ,(new                tellstdfunc::lgcCUTPOLY_I(telldata::tn_void)));
   mblock->addFUNC("merge"            ,(new                    tellstdfunc::lgcMERGE(telldata::tn_void)));
   //--------------------------------------------------------------------------
   // toped specific functons
   //--------------------------------------------------------------------------
   mblock->addFUNC("redraw"           ,(new                   tellstdfunc::stdREDRAW(telldata::tn_void)));
   mblock->addFUNC("zoom"             ,(new                  tellstdfunc::stdZOOMWIN(telldata::tn_void)));
   mblock->addFUNC("zoom"             ,(new                 tellstdfunc::stdZOOMWINb(telldata::tn_void)));
   mblock->addFUNC("zoomall"          ,(new                  tellstdfunc::stdZOOMALL(telldata::tn_void)));
   mblock->addFUNC("layprop"          ,(new                  tellstdfunc::stdLAYPROP(telldata::tn_void)));
   mblock->addFUNC("hidelayer"        ,(new                tellstdfunc::stdHIDELAYER(telldata::tn_void)));
   mblock->addFUNC("hidelayer"        ,(new               tellstdfunc::stdHIDELAYERS(telldata::tn_void)));
   mblock->addFUNC("hidecellmarks"    ,(new             tellstdfunc::stdHIDECELLMARK(telldata::tn_void)));
   mblock->addFUNC("hidetextmarks"    ,(new             tellstdfunc::stdHIDETEXTMARK(telldata::tn_void)));
   mblock->addFUNC("locklayer"        ,(new                tellstdfunc::stdLOCKLAYER(telldata::tn_void)));
   mblock->addFUNC("locklayer"        ,(new               tellstdfunc::stdLOCKLAYERS(telldata::tn_void)));
   mblock->addFUNC("definecolor"      ,(new                 tellstdfunc::stdCOLORDEF(telldata::tn_void)));
   mblock->addFUNC("definefill"       ,(new                  tellstdfunc::stdFILLDEF(telldata::tn_void)));
   mblock->addFUNC("definegrid"       ,(new                  tellstdfunc::stdGRIDDEF(telldata::tn_void)));
   mblock->addFUNC("step"             ,(new                     tellstdfunc::stdSTEP(telldata::tn_void)));
   mblock->addFUNC("grid"             ,(new                     tellstdfunc::stdGRID(telldata::tn_void)));
   mblock->addFUNC("autopan"          ,(new                  tellstdfunc::stdAUTOPAN(telldata::tn_void)));
   mblock->addFUNC("shapeangle"       ,(new               tellstdfunc::stdSHAPEANGLE(telldata::tn_void)));
   mblock->addFUNC("getpoint"         ,(new                     tellstdfunc::getPOINT(telldata::tn_pnt)));
   mblock->addFUNC("getpointlist"     ,(new        tellstdfunc::getPOINTLIST(TLISTOF(telldata::tn_pnt))));
   mblock->addFUNC("addbox"           ,(new                tellstdfunc::stdDRAWBOX(telldata::tn_layout)));
   mblock->addFUNC("addbox"           ,(new              tellstdfunc::stdDRAWBOX_D(telldata::tn_layout)));
   mblock->addFUNC("addpoly"          ,(new               tellstdfunc::stdDRAWPOLY(telldata::tn_layout)));
   mblock->addFUNC("addpoly"          ,(new             tellstdfunc::stdDRAWPOLY_D(telldata::tn_layout)));
   mblock->addFUNC("addwire"          ,(new               tellstdfunc::stdDRAWWIRE(telldata::tn_layout)));
   mblock->addFUNC("addwire"          ,(new             tellstdfunc::stdDRAWWIRE_D(telldata::tn_layout)));
}

class TopedApp : public wxApp {
public:
   virtual bool OnInit();
   virtual int  OnExit();
};

bool TopedApp::OnInit() {
   Properties = new layprop::ViewProperties();
   Toped = new tui::TopedFrame( wxT( "wx_Toped" ), wxPoint(50,50),
   wxSize(1200,900) );

   console::ted_log_ctrl *logWindow = new console::ted_log_ctrl(Toped->logwin());
   delete wxLog::SetActiveTarget(logWindow);

   Browsers = Toped->browsers();
   DATC = new DataCenter();
   // Create the main block parser block - WARNING! blockSTACK structure MUST already exist!
   CMDBlock = new parsercmd::cmdMAIN();
   InitInternalFunctions(static_cast<parsercmd::cmdMAIN*>(CMDBlock));
   Toped->Show(TRUE);
   SetTopWindow(Toped);
   LogFile.init();
//   wxLog::AddTraceMask("thread");
   if (1 < argc) {
      wxString inputfile;
      inputfile << "`include \"" << argv[1] << "\"";
      Console->parseCommand(inputfile);
   }
   return TRUE;
}

int TopedApp::OnExit() {
   delete CMDBlock;
   delete DATC;
   delete Properties;
   return 0;
}

// Starting macro
IMPLEMENT_APP(TopedApp)

