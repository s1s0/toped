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
//        Created: Wed May  5 23:27:33 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Top file in the project
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#include <wx/wx.h>
#include <wx/filefn.h>
#include <wx/filename.h>

#if WIN32 
#include <crtdbg.h>
#endif

#include "toped.h"
#include "../tpd_DB/viewprop.h"
#include "tellibin.h"
#include "datacenter.h"
#include "../tpd_common/glf.h"

tui::TopedFrame*                 Toped = NULL;
browsers::browserTAB*            Browsers = NULL;
DataCenter*                      DATC = NULL;
// from ted_prompt (console)
extern parsercmd::cmdBLOCK*      CMDBlock;
extern console::toped_logfile    LogFile;
extern console::ted_cmd*         Console;
extern console::TELLFuncList*    CmdList;
//console::TELLFuncList*           CmdLst = NULL;

//-----------------------------------------------------------------------------

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
   mblock->addFUNC("echo"             ,(new                     tellstdfunc::stdECHO(telldata::tn_void, true)));
   mblock->addFUNC("status"           ,(new               tellstdfunc::stdTELLSTATUS(telldata::tn_void, true)));
   mblock->addFUNC("undo"             ,(new                     tellstdfunc::stdUNDO(telldata::tn_void,false)));
   //
   mblock->addFUNC("report_selected"  ,(new              tellstdfunc::stdREPORTSLCTD(telldata::tn_void,false)));
   mblock->addFUNC("report_layers"    ,(new        tellstdfunc::stdREPORTLAY(TLISTOF(telldata::tn_int),false)));
   mblock->addFUNC("report_layers"    ,(new       tellstdfunc::stdREPORTLAYc(TLISTOF(telldata::tn_int),false)));
   mblock->addFUNC("report_gdslayers" ,(new                tellstdfunc::GDSreportlay(telldata::tn_void, true)));
   //
   mblock->addFUNC("newdesign"        ,(new                tellstdfunc::stdNEWDESIGN(telldata::tn_void, true)));
   mblock->addFUNC("newdesign"        ,(new               tellstdfunc::stdNEWDESIGNd(telldata::tn_void, true)));
   mblock->addFUNC("newcell"          ,(new                  tellstdfunc::stdNEWCELL(telldata::tn_void,false)));
   mblock->addFUNC("removecell"       ,(new               tellstdfunc::stdREMOVECELL(telldata::tn_void,false)));
   mblock->addFUNC("gdsread"          ,(new          tellstdfunc::GDSread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("gdsimport"        ,(new               tellstdfunc::GDSconvertAll(telldata::tn_void, true)));
   mblock->addFUNC("gdsimport"        ,(new                  tellstdfunc::GDSconvert(telldata::tn_void, true)));
   mblock->addFUNC("gdsexport"        ,(new                tellstdfunc::GDSexportLIB(telldata::tn_void,false)));
   mblock->addFUNC("gdsexport"        ,(new                tellstdfunc::GDSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("gdsclose"         ,(new                    tellstdfunc::GDSclose(telldata::tn_void, true)));
   mblock->addFUNC("psexport"         ,(new                 tellstdfunc::PSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("tdtread"          ,(new                     tellstdfunc::TDTread(telldata::tn_void, true)));
   mblock->addFUNC("tdtread"          ,(new                  tellstdfunc::TDTreadIFF(telldata::tn_void, true)));
   mblock->addFUNC("tdtsave"          ,(new                     tellstdfunc::TDTsave(telldata::tn_void, true)));
   mblock->addFUNC("tdtsave"          ,(new                  tellstdfunc::TDTsaveIFF(telldata::tn_void, true)));
   mblock->addFUNC("tdtsaveas"        ,(new                   tellstdfunc::TDTsaveas(telldata::tn_void, true)));
   mblock->addFUNC("opencell"         ,(new                 tellstdfunc::stdOPENCELL(telldata::tn_void, true)));
   mblock->addFUNC("editpush"         ,(new                 tellstdfunc::stdEDITPUSH(telldata::tn_void, true)));
   mblock->addFUNC("editpop"          ,(new                  tellstdfunc::stdEDITPOP(telldata::tn_void, true)));
   mblock->addFUNC("edittop"          ,(new                  tellstdfunc::stdEDITTOP(telldata::tn_void, true)));
   mblock->addFUNC("editprev"         ,(new                 tellstdfunc::stdEDITPREV(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(new               tellstdfunc::stdUSINGLAYER(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(new             tellstdfunc::stdUSINGLAYER_S(telldata::tn_void, true)));
   mblock->addFUNC("addbox"           ,(new                 tellstdfunc::stdADDBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(new               tellstdfunc::stdADDBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(new                tellstdfunc::stdADDBOXr(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(new              tellstdfunc::stdADDBOXr_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(new                tellstdfunc::stdADDBOXp(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(new              tellstdfunc::stdADDBOXp_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(new                tellstdfunc::stdADDPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(new              tellstdfunc::stdADDPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(new                tellstdfunc::stdADDWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(new              tellstdfunc::stdADDWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(new                tellstdfunc::stdADDTEXT(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(new                tellstdfunc::stdCELLREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(new               tellstdfunc::stdCELLAREF(telldata::tn_layout,false)));
   mblock->addFUNC("select"           ,(new        tellstdfunc::stdSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(new      tellstdfunc::stdSELECTIN(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(new      tellstdfunc::stdSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(new     tellstdfunc::stdSELECT_TL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("pselect"          ,(new     tellstdfunc::stdPNTSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("pselect"          ,(new   tellstdfunc::stdPNTSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(new      tellstdfunc::stdUNSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(new    tellstdfunc::stdUNSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(new   tellstdfunc::stdUNSELECT_TL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(new    tellstdfunc::stdUNSELECTIN(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("punselect"        ,(new   tellstdfunc::stdPNTUNSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("punselect"        ,(new tellstdfunc::stdPNTUNSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select_all"       ,(new     tellstdfunc::stdSELECTALL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect_all"     ,(new              tellstdfunc::stdUNSELECTALL(telldata::tn_void,false)));
   // operation on the toped data
   mblock->addFUNC("move"             ,(new                  tellstdfunc::stdMOVESEL(telldata::tn_void,false)));
   mblock->addFUNC("move"             ,(new                tellstdfunc::stdMOVESEL_D(telldata::tn_void,false)));
   mblock->addFUNC("copy"             ,(new       tellstdfunc::stdCOPYSEL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("copy"             ,(new     tellstdfunc::stdCOPYSEL_D(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("rotate"           ,(new                tellstdfunc::stdROTATESEL(telldata::tn_void,false)));
   mblock->addFUNC("rotate"           ,(new              tellstdfunc::stdROTATESEL_D(telldata::tn_void,false)));
   mblock->addFUNC("flipX"            ,(new                 tellstdfunc::stdFLIPXSEL(telldata::tn_void,false)));
   mblock->addFUNC("flipX"            ,(new               tellstdfunc::stdFLIPXSEL_D(telldata::tn_void,false)));
   mblock->addFUNC("flipY"            ,(new                 tellstdfunc::stdFLIPYSEL(telldata::tn_void,false)));
   mblock->addFUNC("flipY"            ,(new               tellstdfunc::stdFLIPYSEL_D(telldata::tn_void,false)));
   mblock->addFUNC("delete"           ,(new                tellstdfunc::stdDELETESEL(telldata::tn_void,false)));
   mblock->addFUNC("group"            ,(new                    tellstdfunc::stdGROUP(telldata::tn_void,false)));
   mblock->addFUNC("ungroup"          ,(new                  tellstdfunc::stdUNGROUP(telldata::tn_void,false)));
   //
   mblock->addFUNC("polycut"          ,(new                  tellstdfunc::lgcCUTPOLY(telldata::tn_void,false)));
   mblock->addFUNC("polycut"          ,(new                tellstdfunc::lgcCUTPOLY_I(telldata::tn_void,false)));
   mblock->addFUNC("merge"            ,(new                    tellstdfunc::lgcMERGE(telldata::tn_void,false)));
   //--------------------------------------------------------------------------
   // toped specific functons
   //--------------------------------------------------------------------------
   mblock->addFUNC("redraw"           ,(new                   tellstdfunc::stdREDRAW(telldata::tn_void, true)));
   mblock->addFUNC("addruler"         ,(new                 tellstdfunc::stdDISTANCE(telldata::tn_void, true)));
   mblock->addFUNC("addruler"         ,(new               tellstdfunc::stdDISTANCE_D(telldata::tn_void, true)));
   mblock->addFUNC("clearrulers"      ,(new              tellstdfunc::stdCLEARRULERS(telldata::tn_void, true)));
   mblock->addFUNC("zoom"             ,(new                  tellstdfunc::stdZOOMWIN(telldata::tn_void, true)));
   mblock->addFUNC("zoom"             ,(new                 tellstdfunc::stdZOOMWINb(telldata::tn_void, true)));
   mblock->addFUNC("zoomall"          ,(new                  tellstdfunc::stdZOOMALL(telldata::tn_void, true)));
   mblock->addFUNC("layprop"          ,(new                  tellstdfunc::stdLAYPROP(telldata::tn_void, true)));
   mblock->addFUNC("hidelayer"        ,(new                tellstdfunc::stdHIDELAYER(telldata::tn_void, true)));
   mblock->addFUNC("hidelayer"        ,(new               tellstdfunc::stdHIDELAYERS(telldata::tn_void, true)));
   mblock->addFUNC("hidecellmarks"    ,(new             tellstdfunc::stdHIDECELLMARK(telldata::tn_void, true)));
   mblock->addFUNC("hidetextmarks"    ,(new             tellstdfunc::stdHIDETEXTMARK(telldata::tn_void, true)));
   mblock->addFUNC("locklayer"        ,(new                tellstdfunc::stdLOCKLAYER(telldata::tn_void, true)));
   mblock->addFUNC("locklayer"        ,(new               tellstdfunc::stdLOCKLAYERS(telldata::tn_void, true)));
   mblock->addFUNC("definecolor"      ,(new                 tellstdfunc::stdCOLORDEF(telldata::tn_void, true)));
   mblock->addFUNC("definefill"       ,(new                  tellstdfunc::stdFILLDEF(telldata::tn_void, true)));
   mblock->addFUNC("defineline"       ,(new                  tellstdfunc::stdLINEDEF(telldata::tn_void, true)));
   mblock->addFUNC("definegrid"       ,(new                  tellstdfunc::stdGRIDDEF(telldata::tn_void, true)));
   mblock->addFUNC("step"             ,(new                     tellstdfunc::stdSTEP(telldata::tn_void, true)));
   mblock->addFUNC("grid"             ,(new                     tellstdfunc::stdGRID(telldata::tn_void, true)));
   mblock->addFUNC("autopan"          ,(new                  tellstdfunc::stdAUTOPAN(telldata::tn_void, true)));
   mblock->addFUNC("shapeangle"       ,(new               tellstdfunc::stdSHAPEANGLE(telldata::tn_void, true)));
   mblock->addFUNC("getpoint"         ,(new                    tellstdfunc::getPOINT(telldata::tn_pnt ,false)));
   mblock->addFUNC("getpointlist"     ,(new        tellstdfunc::getPOINTLIST(TLISTOF(telldata::tn_pnt),false)));
   mblock->addFUNC("addbox"           ,(new                tellstdfunc::stdDRAWBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(new              tellstdfunc::stdDRAWBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(new               tellstdfunc::stdDRAWPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(new             tellstdfunc::stdDRAWPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(new               tellstdfunc::stdDRAWWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(new             tellstdfunc::stdDRAWWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("propsave"         ,(new                 tellstdfunc::stdPROPSAVE(telldata::tn_void, true)));
   mblock->addFUNC("addmenu"          ,(new                  tellstdfunc::stdADDMENU(telldata::tn_void, true)));
   
   console::TellFnSort();
}

class TopedApp : public wxApp
{
   public:
      virtual bool   OnInit();
      virtual int    OnExit();
      virtual       ~TopedApp(){};
//      bool           ignoreOnRecovery() { return _ignoreOnRecovery;}
//      void           set_ignoreOnRecovery(bool ior) {_ignoreOnRecovery = ior;}
   protected:
      bool           GetLogFileName();
      bool           CheckCrashLog();
      void           GetLogDir();
      void           FinishSessionLog();
      void           SaveIgnoredCrashLog();
      wxString       logFileName;
      wxString       tpdLogDir;
//      bool           _ignoreOnRecovery;
};


void TopedApp::GetLogDir()
{
   wxFileName* logDIR = new wxFileName(wxT("$TPD_LOCAL/"));
   logDIR->Normalize();
   wxString dirName = logDIR->GetPath();
   wxString info;
   bool undefined = dirName.Matches(wxT("*$TPD_LOCAL*"));
   if (!undefined)
   {
      logDIR->AppendDir(wxT("log"));
      logDIR->Normalize();
   }
   if (logDIR->IsOk())
   {
      bool exist = logDIR->DirExists();
      if (!exist)
      {
         if (undefined)
            info = wxT("Environment variable $TPD_LOCAL is not defined");
         else
         {
            info << wxT("Directory ") << logDIR->GetFullPath() << wxT(" doesn't exists");
         }
         info << wxT(". Log file will be created in the current directory \"");
         info << wxGetCwd() << wxT("\"");
         tell_log(console::MT_WARNING,info);
         tpdLogDir = wxT(".");
      }
      else
         tpdLogDir = logDIR->GetFullPath();
   }
   else
   {
      info = wxT("Can't evaluate properly \"$TPD_LOCAL\" env. variable");
      info << wxT(". Log file will be created in the current directory \"");
      tell_log(console::MT_WARNING,info);
      tpdLogDir = wxT(".");
   }
   delete logDIR;
}

bool TopedApp::GetLogFileName()
{
   bool status = false;
   wxString fullName;
   fullName << tpdLogDir << wxT("toped_session.log");
   wxFileName* logFN = new wxFileName(fullName);
   logFN->Normalize();
   if (logFN->IsOk())
   {
      logFileName = logFN->GetFullPath();
      status =  true;
   }
   else status = false;
   delete logFN;
   return status;
}

bool TopedApp::CheckCrashLog()
{
   if (wxFileExists(logFileName))
   {
      tell_log(console::MT_WARNING,"Previous session didn't exit normally.");
      return true;
   }
   else return false;
}

void TopedApp::SaveIgnoredCrashLog()
{
   time_t timeNow = time(NULL);
   tm* broken_time = localtime(&timeNow);
   char* btm = new char[256];
   strftime(btm, 256, "_%y%m%d_%H%M%S", broken_time);
   wxString fullName;
   fullName << tpdLogDir + wxT("/crash") + wxString(btm, wxConvUTF8) + wxT(".log");
   wxFileName* lFN = new wxFileName(fullName.c_str());
   delete [] btm;
   lFN->Normalize();
   assert(lFN->IsOk());
   wxRenameFile(logFileName.c_str(), lFN->GetFullPath());
   delete lFN;
}

void TopedApp::FinishSessionLog()
{
   LogFile.close();
   wxString fullName;
   fullName << tpdLogDir << wxT("/tpd_previous.log");
   wxFileName* lFN = new wxFileName(fullName.c_str());
   lFN->Normalize();
   assert(lFN->IsOk());
   wxRenameFile(logFileName.c_str(), lFN->GetFullPath());
   delete lFN;
}

bool TopedApp::OnInit() {
//Memory leakages check for Windows
/*  #ifdef _DEBUG
  int tmpDbgFlag;

  HANDLE hLogFile=CreateFile("log.txt",GENERIC_WRITE,FILE_SHARE_WRITE,
    NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE|_CRTDBG_MODE_WNDW|_CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_WARN,_CRTDBG_MODE_FILE|_CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_FILE|_CRTDBG_MODE_WNDW|_CRTDBG_MODE_DEBUG);

  _CrtSetReportFile(_CRT_ASSERT,hLogFile);
  _CrtSetReportFile(_CRT_WARN,hLogFile);
  _CrtSetReportFile(_CRT_ERROR,hLogFile);


  tmpDbgFlag=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpDbgFlag|=_CRTDBG_ALLOC_MEM_DF;
  tmpDbgFlag|=_CRTDBG_DELAY_FREE_MEM_DF;
  tmpDbgFlag|=_CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpDbgFlag);
  //_CrtSetBreakAlloc(5919);
#endif*/
   DATC = new DataCenter();
   Toped = new tui::TopedFrame( wxT( "wx_Toped" ), wxPoint(50,50), wxSize(1200,900) );

   console::ted_log_ctrl *logWindow = new console::ted_log_ctrl(Toped->logwin());
   delete wxLog::SetActiveTarget(logWindow);

   Browsers = Toped->browsers();
   CmdList = Toped->cmdlist();
   // Create the main block parser block - WARNING! blockSTACK structure MUST already exist!
   CMDBlock = new parsercmd::cmdMAIN();
   InitInternalFunctions(static_cast<parsercmd::cmdMAIN*>(CMDBlock));

   SetTopWindow(Toped);
   Toped->Show(TRUE);

   wxString fontFile = wxT("$TPD_LOCAL/fonts/arial1.glf");
   wxFileName fontFN(fontFile);
   fontFN.Normalize();
   if (!(fontFN.IsOk() && (-1 != glfLoadFont(fontFN.GetFullPath().mb_str()))))
   {
      wxMessageDialog* dlg1 = new  wxMessageDialog(Toped,
            wxT("Font library \"$TPD_LOCAL/fonts/arial1.glf\" not found or corrupted. \n Toped will be unstable.\n Continue?"),
            wxT("Toped"),
            wxYES_NO | wxICON_WARNING);
      if (wxID_NO == dlg1->ShowModal())
         return false;
      dlg1->Destroy();
      std::string info("Font library \"$TPD_LOCAL/fonts/arial1.glf\" is not loaded. All text objects will not be properly processed");
      tell_log(console::MT_ERROR,info);
   }
   //

   GetLogDir();
   if (!GetLogFileName()) return FALSE;
   bool recovery_mode = false;
   if (CheckCrashLog())
   {
      wxMessageDialog* dlg1 = new  wxMessageDialog(Toped,
            wxT("Last session didn't exit normally. Start recovery?\n\n WARNING! Recovery mode is experimental.\nMake sure that you've backed-up your database before proceeding"),
            wxT("Toped"),
            wxYES_NO | wxICON_WARNING);
      if (wxID_YES == dlg1->ShowModal())
         recovery_mode = true;
      else
         tell_log(console::MT_WARNING,"Recovery rejected.");
      dlg1->Destroy();
      if (!recovery_mode) SaveIgnoredCrashLog();
   }
   if (recovery_mode)
   {
      tell_log(console::MT_WARNING,"Starting recovery ...");
      wxString inputfile;
      inputfile << wxT("#include \"") << logFileName.c_str() << wxT("\"");
      Console->parseCommand(inputfile, false);
      tell_log(console::MT_WARNING,"Previous session recovered.");
      static_cast<parsercmd::cmdMAIN*>(CMDBlock)->recoveryDone();
      LogFile.init(std::string(logFileName.mb_str()), true);
   }
   else
   {
      LogFile.init(std::string(logFileName.mb_str()));
      //   wxLog::AddTraceMask("thread");
      if (1 < argc) 
      {
         wxString inputfile;
         for (int i=1; i<argc; i++)
         {
            inputfile.Clear();
            inputfile << wxT("#include \"") << argv[i] << wxT("\"");
            Console->parseCommand(inputfile);
         }
      }
   }
   return TRUE;
}

int TopedApp::OnExit() {
   delete CMDBlock; 
   delete DATC;
   FinishSessionLog();
   glfClose();
   return wxApp::OnExit();
}

// Starting macro
IMPLEMENT_APP(TopedApp)
//DECLARE_APP(TopedApp)
