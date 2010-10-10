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

#include "tpdph.h"
#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <sstream>
#if WIN32
#include <crtdbg.h>
#endif

#include "toped.h"
#include "viewprop.h"
#include "datacenter.h"
#include "calbr_reader.h"
#include "glf.h"

#include "tellibin.h"
#include "tpdf_db.h"
#include "tpdf_props.h"
#include "tpdf_cells.h"
#include "tpdf_edit.h"
#include "tpdf_add.h"
#include "tpdf_select.h"
#include "tllf_list.h"
#include "tpdf_get.h"

tui::TopedFrame*                 Toped = NULL;
extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern layprop::FontLibrary*     fontLib;
extern parsercmd::cmdBLOCK*      CMDBlock;
extern console::toped_logfile    LogFile;
extern console::ted_cmd*         Console;
extern Calbr::CalbrFile*         DRCData;
extern const wxEventType         wxEVT_SETINGSMENU;


//-----------------------------------------------------------------------------

//=============================================================================
// The top application class. All initialization and exiting code
//=============================================================================
class TopedApp : public wxApp
{
   public:
      virtual bool   OnInit();
      virtual int    OnExit();
      virtual int    OnRun();
      virtual       ~TopedApp(){};
   private:
      typedef std::list<wxDynamicLibrary*> PluginList;
      bool           getLogFileName();
      void           loadGlfFonts();
      void           loadPlugIns();
      bool           checkCrashLog();
      void           getLocalDirs();
      void           getGlobalDirs(); //get directories in TPD_GLOBAL
      void           finishSessionLog();
      void           saveIgnoredCrashLog();
      void           parseCmdLineArgs();
      void           printLogWHeader();
      void           initInternalFunctions(parsercmd::cmdMAIN* mblock);
      wxString       _logFileName;
      wxString       _tpdLogDir;
      wxString       _tpdFontDir;
      wxString       _tpdResourceDir;
      wxString       _tpdPlugInDir;
      wxString       _globalDir;
      wxString       _localDir;
      wxString       _inputTellFile;
      bool           _forceBasicRendering;
      PluginList     _plugins;
};

//=============================================================================
bool TopedApp::OnInit()
{
//   DATC = DEBUG_NEW DataCenter();
   _forceBasicRendering = false;
   wxImage::AddHandler(DEBUG_NEW wxPNGHandler);
   // Initialize Toped properties
   PROPC = DEBUG_NEW layprop::PropertyCenter();
   // Initialize Toped database
   DATC  = DEBUG_NEW DataCenter(std::string(_localDir.mb_str(wxConvFile)), std::string(_globalDir.mb_str(wxConvFile)));
   // Get the Graphic User Interface (gui system)
   Toped = DEBUG_NEW tui::TopedFrame( wxT( "Toped" ), wxPoint(50,50), wxSize(1200,900) );
   // check command line arguments
   parseCmdLineArgs();
   // Check we've got the required graphic capabilities. If not - bail out.
   if (!Toped->view()->initStatus())
   {
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
            wxT("Toped can't obtain required GLX Visual. Check your video driver/setup please"),
            wxT("Toped"),
            wxOK | wxICON_ERROR);
      dlg1->ShowModal();
      dlg1->Destroy();
      return FALSE;
   }
   // Diagnose the graphic system and return the appropriate
   // type of rendering (i.e. basic or tenderer)
   if (!_forceBasicRendering)
      PROPC->setRenderType(Toped->view()->diagnozeGL());
   // Replace the active console in the wx system with Toped console window
   console::ted_log_ctrl *logWindow = DEBUG_NEW console::ted_log_ctrl(Toped->logwin());
   delete wxLog::SetActiveTarget(logWindow);
   // Initialize the tool bars
   getLocalDirs();
   getGlobalDirs();
   Toped->setIconDir(std::string(_tpdResourceDir.mb_str(wxConvFile)));
   Toped->initToolBars();
   // Create the session log file
   if (!getLogFileName()) return FALSE;
   // It's time to register all internal TELL functions
   // Create the main block parser block - WARNING! blockSTACK structure MUST already exist!
   CMDBlock = DEBUG_NEW parsercmd::cmdMAIN();
   tellstdfunc::initFuncLib(Toped, Toped->view());
   initInternalFunctions(static_cast<parsercmd::cmdMAIN*>(CMDBlock));
   // Loading of eventual plug-ins
   loadPlugIns();
   // Finally show the Toped frame
   SetTopWindow(Toped);
   Toped->Show(TRUE);
   // First thing after initializing openGL - load available layout fonts
   loadGlfFonts();
   // at this stage - the tool shall be considered fully functional
   //--------------------------------------------------------------------------

//   //--------------------------------------------------------------------------
//   // Check that previous session ended normally
//   bool recovery_mode = false;
//   if (checkCrashLog())
//   {
//      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
//            wxT("Last session didn't exit normally. Start recovery?"),
//            wxT("Toped"),
//            wxYES_NO | wxICON_WARNING);
//      if (wxID_YES == dlg1->ShowModal())
//         recovery_mode = true;
//      else
//         tell_log(console::MT_WARNING,"Recovery rejected.");
//      dlg1->Destroy();
//      if (!recovery_mode) saveIgnoredCrashLog();
//   }
//   if (recovery_mode)
//   {
//      // Try to recover executing the commands logged during previous
//      // session
//      tell_log(console::MT_WARNING,"Starting recovery ...");
//      wxString inputfile;
//      inputfile << wxT("#include \"") << _logFileName.c_str() << wxT("\"");
//      Console->parseCommand(inputfile, false);
//      tell_log(console::MT_WARNING,"Exit recovery mode.");
//      static_cast<parsercmd::cmdMAIN*>(CMDBlock)->recoveryDone();
//      LogFile.init(std::string(_logFileName.mb_str(wxConvFile)), true);
//   }
//   else
//   {
//      // Execute the tell file from the command line
//      LogFile.init(std::string(_logFileName.mb_str(wxConvFile )));
////      wxLog::AddTraceMask(wxT("thread"));
////      wxLog::AddTraceMask(wxTRACE_MemAlloc);
//      if ( !_inputTellFile.IsEmpty() )
//         Console->parseCommand(_inputTellFile);
//
//   }
   // Put a rendering info in the log
   printLogWHeader();
   return TRUE;
}

//=============================================================================
int TopedApp::OnRun()
{
   //--------------------------------------------------------------------------
   // Check that previous session ended normally
   bool recovery_mode = false;
   if (checkCrashLog())
   {
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
            wxT("Last session didn't exit normally. Start recovery?"),
            wxT("Toped"),
            wxYES_NO | wxICON_WARNING);
      if (wxID_YES == dlg1->ShowModal())
         recovery_mode = true;
      else
         tell_log(console::MT_WARNING,"Recovery rejected.");
      dlg1->Destroy();
      if (!recovery_mode) saveIgnoredCrashLog();
   }
   if (recovery_mode)
   {
      // Try to recover executing the commands logged during previous
      // session
      tell_log(console::MT_WARNING,"Starting recovery ...");
      wxString inputfile;
      inputfile << wxT("#include \"") << _logFileName.c_str() << wxT("\"");
      Console->parseCommand(inputfile, false);
      tell_log(console::MT_WARNING,"Exit recovery mode.");
      static_cast<parsercmd::cmdMAIN*>(CMDBlock)->recoveryDone();
      LogFile.init(std::string(_logFileName.mb_str(wxConvFile)), true);
   }
   else
   {
      // Execute the tell file from the command line
      LogFile.init(std::string(_logFileName.mb_str(wxConvFile )));
//      wxLog::AddTraceMask(wxT("thread"));
//      wxLog::AddTraceMask(wxTRACE_MemAlloc);
      if ( !_inputTellFile.IsEmpty() )
         Console->parseCommand(_inputTellFile);

   }
   return wxApp::OnRun();
}
//=============================================================================
int TopedApp::OnExit()
{
   if (DRCData)
   {
      delete DRCData;
   }
   delete CMDBlock;
   delete PROPC;
   delete DATC;
   // unload the eventual plug-ins
   for (PluginList::const_iterator CP = _plugins.begin(); CP != _plugins.end(); CP++)
      delete (*CP);

#ifdef DB_MEMORY_TRACE
   MemTrack::TrackDumpBlocks();
#endif

   finishSessionLog();
   return wxApp::OnExit();
}

//=============================================================================
bool TopedApp::getLogFileName()
{
   bool status = false;
   wxString fullName;
   fullName << _tpdLogDir << wxT("toped_session.log");
   wxFileName* logFN = DEBUG_NEW wxFileName(fullName);
   logFN->Normalize();
   if (logFN->IsOk())
   {
      _logFileName = logFN->GetFullPath();
      status =  true;
   }
   else status = false;
   delete logFN;
   return status;
}

//=============================================================================
void TopedApp::loadGlfFonts()
{
   wxDir fontDirectory(_tpdFontDir);
   fontLib = DEBUG_NEW layprop::FontLibrary(PROPC->renderType());
   if (fontDirectory.IsOpened())
   {
      wxString curFN;
      if (fontDirectory.GetFirst(&curFN, wxT("*.glf"), wxDIR_FILES))
      {
         do
         {
            std::string ffname(_tpdFontDir.mb_str(wxConvFile));
            ffname += curFN.mb_str(wxConvFile);
            if (fontLib->LoadLayoutFont(ffname))
            {
               wxCommandEvent eventLoadFont(wxEVT_SETINGSMENU);
               eventLoadFont.SetId(tui::STS_LDFONT);
               eventLoadFont.SetString(wxString(fontLib->getActiveFontName().c_str(), wxConvUTF8));
               wxPostEvent(Toped, eventLoadFont);
            }
         } while (fontDirectory.GetNext(&curFN));
      }
   }
   if (0 == fontLib->numFonts())
   {
      wxString errmsg;
      errmsg << wxT("Can't load layout fonts.\n")
             << wxT("Check/fix the installation and/or TPD_GLOBAL env. variable\n")
             << wxT("Text objects will not be visualized.\n");
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
            errmsg,
            wxT("Toped"),
            wxOK | wxICON_ERROR);
      dlg1->ShowModal();
      dlg1->Destroy();
   }
   else
   {
      wxCommandEvent eventLoadFont(wxEVT_SETINGSMENU);
      eventLoadFont.SetId(tui::STS_SLCTFONT);
      eventLoadFont.SetString(wxT("Arial Normal 1"));
      wxPostEvent(Toped, eventLoadFont);
      fontLib->selectFont("Arial Normal 1");
   }
}

//=============================================================================
void TopedApp::loadPlugIns()
{
   if (! _tpdPlugInDir.IsEmpty())
   {
      wxDir plugDir(_tpdPlugInDir);
      if (plugDir.IsOpened())
      {
         wxString curFN;
         if (plugDir.GetFirst(&curFN, wxT("*.so"), wxDIR_FILES))
         {
            typedef void (*ModuleFunction)(parsercmd::cmdMAIN*);
            do
            {
               wxDynamicLibrary* plugin = DEBUG_NEW wxDynamicLibrary();
               if (plugin->Load(curFN))
               {
                  ModuleFunction piInitFunc = (ModuleFunction)plugin->GetSymbol(wxT("initPluginFunctions"));
                  if (piInitFunc)
                  {
                     piInitFunc(static_cast<parsercmd::cmdMAIN*>(CMDBlock));
                     _plugins.push_back(plugin);
                  }
                  else
                  {
                     wxString errMessage(wxT("File \""));
                     errMessage += curFN;
                     errMessage += wxT("\" doesn't look like a Toped plug-in");
                     wxMessageBox(errMessage);
                     delete plugin;
                  }
               }
               else
               {
                  wxString errMessage(wxT("Troubles when trying to load \""));
                  errMessage += curFN;
                  errMessage += wxT("\" dynamically");
                  wxMessageBox(errMessage);
                  delete plugin;
               }

            } while (plugDir.GetNext(&curFN));
         }
      }
   }
}

//=============================================================================
void TopedApp::parseCmdLineArgs()
{
   if (1 < argc)
   {
      for (int i=1; i<argc; i++)
      {
         wxString curar(argv[i]);
         if (wxT("-ogl_thread") == curar) Toped->setOglThread(true);
         else if (wxT("-ogl_safe") == curar) _forceBasicRendering = true;
         else if (!(0 == curar.Find('-')))
         {
            _inputTellFile.Clear();
            _inputTellFile << wxT("#include \"") << curar << wxT("\"");
         }
         else
         {
            std::string invalid_argument(curar.mb_str(wxConvUTF8));
            std::cout << "Unknown command line option \"" << invalid_argument <<"\". Ignored" << std::endl ;
         }
      }
   }
}

//=============================================================================
bool TopedApp::checkCrashLog()
{
   if (wxFileExists(_logFileName))
   {
      tell_log(console::MT_WARNING,"Previous session didn't exit normally.");
      return true;
   }
   else return false;
}

//=============================================================================
void TopedApp::getLocalDirs()
{
   wxString info;
   if (!wxGetEnv(wxT("TPD_LOCAL"), &_localDir))
   {
      tell_log(console::MT_WARNING,"Environment variable $TPD_LOCAL is not defined. Trying to use current directory");
      _localDir = wxT("./");
   }
   else
      _localDir << wxT("/");

   // Check fonts directory
   wxFileName logFolder(_localDir);
   logFolder.AppendDir(wxT("log"));
   logFolder.Normalize();
   if (logFolder.DirExists())
      _tpdLogDir = logFolder.GetFullPath();
   else
   {
      info = wxT("Directory \"");
      info << logFolder.GetFullPath() << wxT("\" doesn't exists.");
      info << wxT(" Log file will be created in the current directory");
      _tpdLogDir = wxT("./");
      tell_log(console::MT_WARNING,info);
   }
}

//=============================================================================
void TopedApp::getGlobalDirs()
{
   wxString info;
   if (!wxGetEnv(wxT("TPD_GLOBAL"), &_globalDir))
   {
      tell_log(console::MT_WARNING,"Environment variable $TPD_GLOBAL is not defined. Trying to use current directory");
      _globalDir = wxT("./");
   }
   else
      _globalDir << wxT("/");
   // Check fonts directory
   wxFileName fontsFolder(_globalDir);
   fontsFolder.AppendDir(wxT("fonts"));
   fontsFolder.Normalize();
   if (fontsFolder.DirExists())
      _tpdFontDir = fontsFolder.GetFullPath();
   else
   {
      info = wxT("Directory \"");
      info << fontsFolder.GetFullPath() << wxT("\" doesn't exists.");
      info << wxT(" Looking for fonts in the current directory \"");
      tell_log(console::MT_WARNING,info);
      _tpdFontDir = wxT("./");
   }
   // Check resource directory
   wxFileName iconsFolder(_globalDir);
   iconsFolder.AppendDir(wxT("icons"));
   iconsFolder.Normalize();
   if (iconsFolder.DirExists())
      _tpdResourceDir = iconsFolder.GetFullPath();
   else
   {
      info = wxT("Directory \"");
      info << iconsFolder.GetFullPath() << wxT("\" doesn't exists.");
      info << wxT(" Looking for icons in the current directory \"");
      tell_log(console::MT_WARNING,info);
      _tpdResourceDir = wxT("./");
   }
   // Check plug-ins directory
   wxFileName plugFolder(_globalDir);
   plugFolder.AppendDir(wxT("plugins"));
   plugFolder.Normalize();
   if (plugFolder.DirExists())
      _tpdPlugInDir = plugFolder.GetFullPath();
   else
      // Don't generate a noise about plug-in directory.
      _tpdPlugInDir = wxT("");
}

//=============================================================================
void TopedApp::printLogWHeader()
{
   if (PROPC->renderType())
   {
      tell_log(console::MT_INFO,"...using VBO rendering");
   }
   else if (_forceBasicRendering)
   {
      tell_log(console::MT_INFO,"...basic rendering forced from the command line");
   }
   else
   {
      if      (!Toped->view()->oglVersion14())
         tell_log(console::MT_WARNING,"OpenGL version 1.4 is not supported");
      else if (!Toped->view()->oglArbVertexBufferObject())
         tell_log(console::MT_WARNING,"OpenGL implementation doesn't support Vertex Buffer Objects");
      else if (!Toped->view()->oglExtMultiDrawArrays())
         tell_log(console::MT_WARNING,"OpenGL implementation doesn't support Multi Draw Arrays");
      tell_log(console::MT_INFO,"...Using basic rendering");
   }
   tell_log(console::MT_INFO,"Toped loaded.");
   tell_log(console::MT_WARNING,"Please submit your feedback to feedback@toped.org.uk");
}

//=============================================================================
void TopedApp::finishSessionLog()
{
   LogFile.close();
   wxString fullName;
   fullName << _tpdLogDir << wxT("/tpd_previous.log");
   wxFileName* lFN = DEBUG_NEW wxFileName(fullName.c_str());
   lFN->Normalize();
   assert(lFN->IsOk());
   wxRenameFile(_logFileName.c_str(), lFN->GetFullPath());
   delete lFN;
}

//=============================================================================
void TopedApp::saveIgnoredCrashLog()
{
   time_t timeNow = time(NULL);
   tm* broken_time = localtime(&timeNow);
   char* btm = DEBUG_NEW char[256];
   strftime(btm, 256, "_%y%m%d_%H%M%S", broken_time);
   wxString fullName;
   fullName << _tpdLogDir + wxT("/crash") + wxString(btm, wxConvUTF8) + wxT(".log");
   wxFileName* lFN = DEBUG_NEW wxFileName(fullName.c_str());
   delete [] btm;
   lFN->Normalize();
   assert(lFN->IsOk());
   wxRenameFile(_logFileName.c_str(), lFN->GetFullPath());
   delete lFN;
}

void TopedApp::initInternalFunctions(parsercmd::cmdMAIN* mblock)
{
   //-----------------------------------------------------------------------------------------------------------
   // First the internal types
   //-----------------------------------------------------------------------------------------------------------
   telldata::point_type*   pntype      = DEBUG_NEW telldata::point_type();
   telldata::box_type*     bxtype      = DEBUG_NEW telldata::box_type(pntype);
   telldata::bnd_type*     bndtype     = DEBUG_NEW telldata::bnd_type(pntype);
   telldata::hsh_type*     hshtype     = DEBUG_NEW telldata::hsh_type();
   telldata::hshstr_type*  hshstrtype  = DEBUG_NEW telldata::hshstr_type();

   mblock->addGlobalType("point"     , pntype);
   mblock->addGlobalType("box"       , bxtype);
   mblock->addGlobalType("bind"      , bndtype);
   mblock->addGlobalType("lmap"      , hshtype);
   mblock->addGlobalType("strmap"    , hshstrtype);
   //-----------------------------------------------------------------------------------------------------------
   // Internal variables
   //-----------------------------------------------------------------------------------------------------------
   // layout type masks
   mblock->addconstID("_lmbox"   , DEBUG_NEW telldata::ttint( laydata::_lmbox  ), true);
   mblock->addconstID("_lmpoly"  , DEBUG_NEW telldata::ttint( laydata::_lmpoly ), true);
   mblock->addconstID("_lmwire"  , DEBUG_NEW telldata::ttint( laydata::_lmwire ), true);
   mblock->addconstID("_lmtext"  , DEBUG_NEW telldata::ttint( laydata::_lmtext ), true);
   mblock->addconstID("_lmref"   , DEBUG_NEW telldata::ttint( laydata::_lmref  ), true);
   mblock->addconstID("_lmaref"  , DEBUG_NEW telldata::ttint( laydata::_lmaref ), true);
   mblock->addconstID("_lmpref"  , DEBUG_NEW telldata::ttint( laydata::_lmpref ), true);
   mblock->addconstID("_lmapref" , DEBUG_NEW telldata::ttint( laydata::_lmapref), true);
   // Toolbar properties
   mblock->addconstID("_horizontal", DEBUG_NEW telldata::ttint( tui::_tuihorizontal), true);
   mblock->addconstID("_vertical"  , DEBUG_NEW telldata::ttint( tui::_tuivertical),   true);
   mblock->addconstID("_iconsize16", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_16x16),true);
   mblock->addconstID("_iconsize24", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_24x24),true);
   mblock->addconstID("_iconsize32", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_32x32),true);
   mblock->addconstID("_iconsize48", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_48x48),true);
   // Renderer properties

   //-----------------------------------------------------------------------------------------------------------
   // tell build-in functions                                                                              execute on recovery
   //             TELL function name                      Implementation class               return type  (when ignoreOnRecovery
   //                                                                                                      is active see cmdFUNCCALL::execute()
   //-----------------------------------------------------------------------------------------------------------
   mblock->addFUNC("length"           ,(DEBUG_NEW                   tellstdfunc::lstLENGTH(telldata::tn_int, true )));
   mblock->addFUNC("pointdump"        ,(DEBUG_NEW       tellstdfunc::lytPOINTDUMP(TLISTOF(telldata::tn_pnt), true )));
   mblock->addFUNC("typeof"           ,(DEBUG_NEW                   tellstdfunc::lytTYPEOF(telldata::tn_int, true )));
   mblock->addFUNC("abs"              ,(DEBUG_NEW                     tellstdfunc::stdABS(telldata::tn_real, true )));
   mblock->addFUNC("sin"              ,(DEBUG_NEW                     tellstdfunc::stdSIN(telldata::tn_real, true )));
   mblock->addFUNC("cos"              ,(DEBUG_NEW                     tellstdfunc::stdCOS(telldata::tn_real, true )));
   mblock->addFUNC("tan"              ,(DEBUG_NEW                     tellstdfunc::stdTAN(telldata::tn_real, true )));
   mblock->addFUNC("asin"             ,(DEBUG_NEW                    tellstdfunc::stdASIN(telldata::tn_real, true )));
   mblock->addFUNC("acos"             ,(DEBUG_NEW                    tellstdfunc::stdACOS(telldata::tn_real, true )));
   mblock->addFUNC("atan"             ,(DEBUG_NEW                    tellstdfunc::stdATAN(telldata::tn_real, true )));
   mblock->addFUNC("sinh"             ,(DEBUG_NEW                    tellstdfunc::stdSINH(telldata::tn_real, true )));
   mblock->addFUNC("cosh"             ,(DEBUG_NEW                    tellstdfunc::stdCOSH(telldata::tn_real, true )));
   mblock->addFUNC("tanh"             ,(DEBUG_NEW                    tellstdfunc::stdTANH(telldata::tn_real, true )));
   mblock->addFUNC("asinh"            ,(DEBUG_NEW                   tellstdfunc::stdASINH(telldata::tn_real, true )));
   mblock->addFUNC("acosh"            ,(DEBUG_NEW                   tellstdfunc::stdACOSH(telldata::tn_real, true )));
   mblock->addFUNC("atanh"            ,(DEBUG_NEW                   tellstdfunc::stdATANH(telldata::tn_real, true )));
   mblock->addFUNC("round"            ,(DEBUG_NEW                    tellstdfunc::stdROUND(telldata::tn_int, true )));
   mblock->addFUNC("ceil"             ,(DEBUG_NEW                     tellstdfunc::stdCEIL(telldata::tn_int, true )));
   mblock->addFUNC("floor"            ,(DEBUG_NEW                    tellstdfunc::stdFLOOR(telldata::tn_int, true )));
   mblock->addFUNC("fmod"             ,(DEBUG_NEW                 tellstdfunc::stdFMODULO(telldata::tn_real, true )));
   mblock->addFUNC("sqrt"             ,(DEBUG_NEW                    tellstdfunc::stdSQRT(telldata::tn_real, true )));
   mblock->addFUNC("pow"              ,(DEBUG_NEW                     tellstdfunc::stdPOW(telldata::tn_real, true )));
   mblock->addFUNC("exp"              ,(DEBUG_NEW                     tellstdfunc::stdEXP(telldata::tn_real, true )));
   mblock->addFUNC("log"              ,(DEBUG_NEW                     tellstdfunc::stdLOG(telldata::tn_real, true )));
   mblock->addFUNC("log10"            ,(DEBUG_NEW                   tellstdfunc::stdLOG10(telldata::tn_real, true )));
   mblock->addFUNC("getlaytype"       ,(DEBUG_NEW               tellstdfunc::stdGETLAYTYPE(telldata::tn_int, true )));
   mblock->addFUNC("getlaytext"       ,(DEBUG_NEW         tellstdfunc::stdGETLAYTEXTSTR(telldata::tn_string, true )));
   mblock->addFUNC("getlayref"        ,(DEBUG_NEW          tellstdfunc::stdGETLAYREFSTR(telldata::tn_string, true )));
   //-----------------------------------------------------------------------------------------------------------
   // toped build-in functions
   //-----------------------------------------------------------------------------------------------------------
   mblock->addFUNC("echo"             ,(DEBUG_NEW                     tellstdfunc::stdECHO(telldata::tn_void, true)));
   mblock->addFUNC("status"           ,(DEBUG_NEW               tellstdfunc::stdTELLSTATUS(telldata::tn_void, true)));
   mblock->addFUNC("undo"             ,(DEBUG_NEW                     tellstdfunc::stdUNDO(telldata::tn_void,false)));
   //
   mblock->addFUNC("report_selected"  ,(DEBUG_NEW              tellstdfunc::stdREPORTSLCTD(telldata::tn_void,true )));
   mblock->addFUNC("report_layers"    ,(DEBUG_NEW        tellstdfunc::stdREPORTLAY(TLISTOF(telldata::tn_int),true )));
   mblock->addFUNC("report_layers"    ,(DEBUG_NEW       tellstdfunc::stdREPORTLAYc(TLISTOF(telldata::tn_int),true )));
   mblock->addFUNC("report_gdslayers" ,(DEBUG_NEW                tellstdfunc::GDSreportlay(telldata::tn_void,true )));
   mblock->addFUNC("report_ciflayers" ,(DEBUG_NEW                tellstdfunc::CIFreportlay(telldata::tn_void,true )));
   mblock->addFUNC("report_oasislayers",(DEBUG_NEW               tellstdfunc::OASreportlay(telldata::tn_void,true )));
   //
   mblock->addFUNC("newdesign"        ,(DEBUG_NEW                tellstdfunc::stdNEWDESIGN(telldata::tn_void, true)));
   mblock->addFUNC("newdesign"        ,(DEBUG_NEW               tellstdfunc::stdNEWDESIGNd(telldata::tn_void, true)));
   mblock->addFUNC("newdesign"        ,(DEBUG_NEW               tellstdfunc::stdNEWDESIGNs(telldata::tn_void, true)));
   mblock->addFUNC("newdesign"        ,(DEBUG_NEW              tellstdfunc::stdNEWDESIGNsd(telldata::tn_void, true)));
   mblock->addFUNC("newcell"          ,(DEBUG_NEW                  tellstdfunc::stdNEWCELL(telldata::tn_void,false)));
   mblock->addFUNC("removecell"       ,(DEBUG_NEW               tellstdfunc::stdREMOVECELL(telldata::tn_void,false)));
//   mblock->addFUNC("removerefdcell"   ,(DEBUG_NEW           tellstdfunc::stdREMOVEREFDCELL(telldata::tn_void,false)));
   mblock->addFUNC("renamecell"       ,(DEBUG_NEW               tellstdfunc::stdRENAMECELL(telldata::tn_void,false)));
   mblock->addFUNC("cifread"          ,(DEBUG_NEW          tellstdfunc::CIFread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("cifimport"        ,(DEBUG_NEW               tellstdfunc::CIFimportList(telldata::tn_void, true)));
   mblock->addFUNC("cifimport"        ,(DEBUG_NEW                   tellstdfunc::CIFimport(telldata::tn_void, true)));
   mblock->addFUNC("cifexport"        ,(DEBUG_NEW                tellstdfunc::CIFexportLIB(telldata::tn_void,false)));
   mblock->addFUNC("cifexport"        ,(DEBUG_NEW                tellstdfunc::CIFexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("cifclose"         ,(DEBUG_NEW                    tellstdfunc::CIFclose(telldata::tn_void, true)));
   mblock->addFUNC("getciflaymap"     ,(DEBUG_NEW        tellstdfunc::CIFgetlaymap(TLISTOF(telldata::tn_hsh), true)));
   mblock->addFUNC("setciflaymap"     ,(DEBUG_NEW                tellstdfunc::CIFsetlaymap(telldata::tn_void, true)));
   mblock->addFUNC("gdsread"          ,(DEBUG_NEW          tellstdfunc::GDSread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("gdsimport"        ,(DEBUG_NEW               tellstdfunc::GDSimportList(telldata::tn_void, true)));
   mblock->addFUNC("gdsimport"        ,(DEBUG_NEW                   tellstdfunc::GDSimport(telldata::tn_void, true)));
   mblock->addFUNC("gdsexport"        ,(DEBUG_NEW                tellstdfunc::GDSexportLIB(telldata::tn_void,false)));
   mblock->addFUNC("gdsexport"        ,(DEBUG_NEW                tellstdfunc::GDSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("gdssplit"         ,(DEBUG_NEW                    tellstdfunc::GDSsplit(telldata::tn_void,false)));
   mblock->addFUNC("gdsclose"         ,(DEBUG_NEW                    tellstdfunc::GDSclose(telldata::tn_void, true)));
   mblock->addFUNC("getgdslaymap"     ,(DEBUG_NEW        tellstdfunc::GDSgetlaymap(TLISTOF(telldata::tn_hsh), true)));
   mblock->addFUNC("setgdslaymap"     ,(DEBUG_NEW                tellstdfunc::GDSsetlaymap(telldata::tn_void, true)));
   mblock->addFUNC("oasisread"        ,(DEBUG_NEW          tellstdfunc::OASread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("oasisimport"      ,(DEBUG_NEW               tellstdfunc::OASimportList(telldata::tn_void, true)));
   mblock->addFUNC("oasisimport"      ,(DEBUG_NEW                   tellstdfunc::OASimport(telldata::tn_void, true)));
   mblock->addFUNC("oasisclose"       ,(DEBUG_NEW                    tellstdfunc::OASclose(telldata::tn_void, true)));
   mblock->addFUNC("getoasislaymap"   ,(DEBUG_NEW        tellstdfunc::OASgetlaymap(TLISTOF(telldata::tn_hsh), true)));
   mblock->addFUNC("setoasislaymap"   ,(DEBUG_NEW        tellstdfunc::OASsetlaymap(TLISTOF(telldata::tn_hsh), true)));
   mblock->addFUNC("drccalibreimport" ,(DEBUG_NEW            tellstdfunc::DRCCalibreimport(telldata::tn_void, true)));
   mblock->addFUNC("drcshowerror"     ,(DEBUG_NEW                tellstdfunc::DRCshowerror(telldata::tn_void, true)));
   mblock->addFUNC("drcshowcluster"   ,(DEBUG_NEW              tellstdfunc::DRCshowcluster(telldata::tn_void, true)));
   mblock->addFUNC("drcshowallerrors" ,(DEBUG_NEW            tellstdfunc::DRCshowallerrors(telldata::tn_void, true)));
   mblock->addFUNC("drchideallerrors" ,(DEBUG_NEW            tellstdfunc::DRChideallerrors(telldata::tn_void, true)));
   mblock->addFUNC("drcexplainerror"  ,(DEBUG_NEW           tellstdfunc::DRCexplainerror_D(telldata::tn_void, true)));
   mblock->addFUNC("drcexplainerror"  ,(DEBUG_NEW             tellstdfunc::DRCexplainerror(telldata::tn_void, true)));
   mblock->addFUNC("psexport"         ,(DEBUG_NEW                 tellstdfunc::PSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("tdtread"          ,(DEBUG_NEW                     tellstdfunc::TDTread(telldata::tn_void, true)));
   mblock->addFUNC("tdtread"          ,(DEBUG_NEW                  tellstdfunc::TDTreadIFF(telldata::tn_void, true)));
   mblock->addFUNC("loadlib"          ,(DEBUG_NEW                  tellstdfunc::TDTloadlib(telldata::tn_void, true)));
   mblock->addFUNC("unloadlib"        ,(DEBUG_NEW                tellstdfunc::TDTunloadlib(telldata::tn_void, true)));
   mblock->addFUNC("tdtsave"          ,(DEBUG_NEW                     tellstdfunc::TDTsave(telldata::tn_void, true)));
   mblock->addFUNC("tdtsave"          ,(DEBUG_NEW                  tellstdfunc::TDTsaveIFF(telldata::tn_void, true)));
   mblock->addFUNC("tdtsaveas"        ,(DEBUG_NEW                   tellstdfunc::TDTsaveas(telldata::tn_void, true)));
   mblock->addFUNC("opencell"         ,(DEBUG_NEW                 tellstdfunc::stdOPENCELL(telldata::tn_void, true)));
   mblock->addFUNC("editpush"         ,(DEBUG_NEW                 tellstdfunc::stdEDITPUSH(telldata::tn_void, true)));
   mblock->addFUNC("editpop"          ,(DEBUG_NEW                  tellstdfunc::stdEDITPOP(telldata::tn_void, true)));
   mblock->addFUNC("edittop"          ,(DEBUG_NEW                  tellstdfunc::stdEDITTOP(telldata::tn_void, true)));
   mblock->addFUNC("editprev"         ,(DEBUG_NEW                 tellstdfunc::stdEDITPREV(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(DEBUG_NEW               tellstdfunc::stdUSINGLAYER(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(DEBUG_NEW             tellstdfunc::stdUSINGLAYER_S(telldata::tn_void, true)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                 tellstdfunc::stdADDBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW               tellstdfunc::stdADDBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdADDBOXr(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXr_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdADDBOXp(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXp_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW                tellstdfunc::stdADDPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW              tellstdfunc::stdADDPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW                tellstdfunc::stdADDWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW              tellstdfunc::stdADDWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(DEBUG_NEW                tellstdfunc::stdADDTEXT(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(DEBUG_NEW              tellstdfunc::stdADDTEXT_D(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(DEBUG_NEW                tellstdfunc::stdCELLREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(DEBUG_NEW              tellstdfunc::stdCELLREF_D(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW               tellstdfunc::stdCELLAREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW             tellstdfunc::stdCELLAREF_D(telldata::tn_layout,false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW        tellstdfunc::stdSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW      tellstdfunc::stdSELECTIN(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW      tellstdfunc::stdSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW     tellstdfunc::stdSELECT_TL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("pselect"          ,(DEBUG_NEW     tellstdfunc::stdPNTSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("pselect"          ,(DEBUG_NEW   tellstdfunc::stdPNTSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW      tellstdfunc::stdUNSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW    tellstdfunc::stdUNSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW   tellstdfunc::stdUNSELECT_TL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW    tellstdfunc::stdUNSELECTIN(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("punselect"        ,(DEBUG_NEW   tellstdfunc::stdPNTUNSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("punselect"        ,(DEBUG_NEW tellstdfunc::stdPNTUNSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select_all"       ,(DEBUG_NEW     tellstdfunc::stdSELECTALL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect_all"     ,(DEBUG_NEW              tellstdfunc::stdUNSELECTALL(telldata::tn_void,false)));
   mblock->addFUNC("selectmask"       ,(DEBUG_NEW             tellstdfunc::stdSETSELECTMASK(telldata::tn_int,false)));
   // operation on the toped data
   mblock->addFUNC("move"             ,(DEBUG_NEW                  tellstdfunc::stdMOVESEL(telldata::tn_void,false)));
   mblock->addFUNC("move"             ,(DEBUG_NEW                tellstdfunc::stdMOVESEL_D(telldata::tn_void,false)));
   mblock->addFUNC("copy"             ,(DEBUG_NEW       tellstdfunc::stdCOPYSEL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("copy"             ,(DEBUG_NEW     tellstdfunc::stdCOPYSEL_D(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("rotate"           ,(DEBUG_NEW                tellstdfunc::stdROTATESEL(telldata::tn_void,false)));
   mblock->addFUNC("rotate"           ,(DEBUG_NEW              tellstdfunc::stdROTATESEL_D(telldata::tn_void,false)));
   //************Deprecated************************
   mblock->addFUNC("flipX"            ,(DEBUG_NEW                 tellstdfunc::stdFLIPXSEL(telldata::tn_void,false)));
   mblock->addFUNC("flipX"            ,(DEBUG_NEW               tellstdfunc::stdFLIPXSEL_D(telldata::tn_void,false)));
   mblock->addFUNC("flipY"            ,(DEBUG_NEW                 tellstdfunc::stdFLIPYSEL(telldata::tn_void,false)));
   mblock->addFUNC("flipY"            ,(DEBUG_NEW               tellstdfunc::stdFLIPYSEL_D(telldata::tn_void,false)));
   //**********************************************
   mblock->addFUNC("flip"             ,(DEBUG_NEW                  tellstdfunc::stdFLIPSEL(telldata::tn_void,false)));
   mblock->addFUNC("flip"             ,(DEBUG_NEW                tellstdfunc::stdFLIPSEL_D(telldata::tn_void,false)));
   mblock->addFUNC("delete"           ,(DEBUG_NEW                tellstdfunc::stdDELETESEL(telldata::tn_void,false)));
   mblock->addFUNC("group"            ,(DEBUG_NEW                    tellstdfunc::stdGROUP(telldata::tn_void,false)));
   mblock->addFUNC("ungroup"          ,(DEBUG_NEW                  tellstdfunc::stdUNGROUP(telldata::tn_void,false)));
   // logical operations
   mblock->addFUNC("polycut"          ,(DEBUG_NEW                  tellstdfunc::lgcCUTPOLY(telldata::tn_void,false)));
   mblock->addFUNC("polycut"          ,(DEBUG_NEW                tellstdfunc::lgcCUTPOLY_I(telldata::tn_void,false)));
   mblock->addFUNC("boxcut"           ,(DEBUG_NEW                 tellstdfunc::lgcCUTBOX_I(telldata::tn_void,false)));
   mblock->addFUNC("merge"            ,(DEBUG_NEW                    tellstdfunc::lgcMERGE(telldata::tn_void,false)));
   mblock->addFUNC("resize"           ,(DEBUG_NEW                  tellstdfunc::lgcSTRETCH(telldata::tn_void,false)));
   // layer/reference operations
   mblock->addFUNC("changelayer"      ,(DEBUG_NEW                tellstdfunc::stdCHANGELAY(telldata::tn_void,false)));
   mblock->addFUNC("changeref"        ,(DEBUG_NEW                tellstdfunc::stdCHANGEREF(telldata::tn_void,false)));
   mblock->addFUNC("changestr"        ,(DEBUG_NEW             tellstdfunc::stdCHANGESTRING(telldata::tn_void,false)));
   //-----------------------------------------------------------------------------------------------------------
   // toped specific functons
   //-----------------------------------------------------------------------------------------------------------
   mblock->addFUNC("redraw"           ,(DEBUG_NEW                   tellstdfunc::stdREDRAW(telldata::tn_void, true)));
   mblock->addFUNC("addruler"         ,(DEBUG_NEW                 tellstdfunc::stdDISTANCE(telldata::tn_void, true)));
   mblock->addFUNC("addruler"         ,(DEBUG_NEW               tellstdfunc::stdDISTANCE_D(telldata::tn_void, true)));
   mblock->addFUNC("clearrulers"      ,(DEBUG_NEW              tellstdfunc::stdCLEARRULERS(telldata::tn_void, true)));
   mblock->addFUNC("longcursor"       ,(DEBUG_NEW               tellstdfunc::stdLONGCURSOR(telldata::tn_void, true)));
   mblock->addFUNC("zoom"             ,(DEBUG_NEW                  tellstdfunc::stdZOOMWIN(telldata::tn_void, true)));
   mblock->addFUNC("zoom"             ,(DEBUG_NEW                 tellstdfunc::stdZOOMWINb(telldata::tn_void, true)));
   mblock->addFUNC("zoomall"          ,(DEBUG_NEW                  tellstdfunc::stdZOOMALL(telldata::tn_void, true)));
   mblock->addFUNC("zoomvisible"      ,(DEBUG_NEW              tellstdfunc::stdZOOMVISIBLE(telldata::tn_void, true)));
   mblock->addFUNC("layprop"          ,(DEBUG_NEW                  tellstdfunc::stdLAYPROP(telldata::tn_void, true)));
   mblock->addFUNC("hidelayer"        ,(DEBUG_NEW                tellstdfunc::stdHIDELAYER(telldata::tn_void, true)));
   mblock->addFUNC("hidelayer"        ,(DEBUG_NEW               tellstdfunc::stdHIDELAYERS(telldata::tn_void, true)));
   mblock->addFUNC("hidecellmarks"    ,(DEBUG_NEW             tellstdfunc::stdHIDECELLMARK(telldata::tn_void, true)));
   mblock->addFUNC("hidecellbox"      ,(DEBUG_NEW             tellstdfunc::stdHIDECELLBOND(telldata::tn_void, true)));
   mblock->addFUNC("hidetextmarks"    ,(DEBUG_NEW             tellstdfunc::stdHIDETEXTMARK(telldata::tn_void, true)));
   mblock->addFUNC("hidetextbox"      ,(DEBUG_NEW             tellstdfunc::stdHIDETEXTBOND(telldata::tn_void, true)));
   mblock->addFUNC("locklayer"        ,(DEBUG_NEW                tellstdfunc::stdLOCKLAYER(telldata::tn_void, true)));
   mblock->addFUNC("locklayer"        ,(DEBUG_NEW               tellstdfunc::stdLOCKLAYERS(telldata::tn_void, true)));
   mblock->addFUNC("filllayer"        ,(DEBUG_NEW                tellstdfunc::stdFILLLAYER(telldata::tn_void, true)));
   mblock->addFUNC("filllayer"        ,(DEBUG_NEW               tellstdfunc::stdFILLLAYERS(telldata::tn_void, true)));
   mblock->addFUNC("savelaystatus"    ,(DEBUG_NEW              tellstdfunc::stdSAVELAYSTAT(telldata::tn_void, true)));
   mblock->addFUNC("restorelaystatus" ,(DEBUG_NEW              tellstdfunc::stdLOADLAYSTAT(telldata::tn_void, true)));
   mblock->addFUNC("deletelaystatus"  ,(DEBUG_NEW               tellstdfunc::stdDELLAYSTAT(telldata::tn_void, true)));
   mblock->addFUNC("definecolor"      ,(DEBUG_NEW                 tellstdfunc::stdCOLORDEF(telldata::tn_void, true)));
   mblock->addFUNC("definefill"       ,(DEBUG_NEW                  tellstdfunc::stdFILLDEF(telldata::tn_void, true)));
   mblock->addFUNC("defineline"       ,(DEBUG_NEW                  tellstdfunc::stdLINEDEF(telldata::tn_void, true)));
   mblock->addFUNC("definegrid"       ,(DEBUG_NEW                  tellstdfunc::stdGRIDDEF(telldata::tn_void, true)));
   mblock->addFUNC("step"             ,(DEBUG_NEW                     tellstdfunc::stdSTEP(telldata::tn_void, true)));
   mblock->addFUNC("grid"             ,(DEBUG_NEW                     tellstdfunc::stdGRID(telldata::tn_void, true)));
   mblock->addFUNC("autopan"          ,(DEBUG_NEW                  tellstdfunc::stdAUTOPAN(telldata::tn_void, true)));
   mblock->addFUNC("zerocross"        ,(DEBUG_NEW                tellstdfunc::stdZEROCROSS(telldata::tn_void, true)));
   mblock->addFUNC("shapeangle"       ,(DEBUG_NEW               tellstdfunc::stdSHAPEANGLE(telldata::tn_void, true)));
   mblock->addFUNC("getpoint"         ,(DEBUG_NEW                    tellstdfunc::getPOINT(telldata::tn_pnt ,false)));
   mblock->addFUNC("getpointlist"     ,(DEBUG_NEW        tellstdfunc::getPOINTLIST(TLISTOF(telldata::tn_pnt),false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdDRAWBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdDRAWBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW               tellstdfunc::stdDRAWPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW             tellstdfunc::stdDRAWPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW               tellstdfunc::stdDRAWWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW             tellstdfunc::stdDRAWWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("propsave"         ,(DEBUG_NEW                 tellstdfunc::stdPROPSAVE(telldata::tn_void, true)));
   mblock->addFUNC("addmenu"          ,(DEBUG_NEW                  tellstdfunc::stdADDMENU(telldata::tn_void, true)));
   mblock->addFUNC("toolbarsize"      ,(DEBUG_NEW              tellstdfunc::stdTOOLBARSIZE(telldata::tn_void, true)));
   mblock->addFUNC("definetoolbar"    ,(DEBUG_NEW            tellstdfunc::stdDEFINETOOLBAR(telldata::tn_void, true)));
   mblock->addFUNC("toolbaradditem"   ,(DEBUG_NEW           tellstdfunc::stdTOOLBARADDITEM(telldata::tn_void, true)));
   mblock->addFUNC("toolbaradditem"   ,(DEBUG_NEW         tellstdfunc::stdTOOLBARADDITEM_S(telldata::tn_void, true)));
   mblock->addFUNC("toolbardeleteitem",(DEBUG_NEW        tellstdfunc::stdTOOLBARDELETEITEM(telldata::tn_void, true)));
   mblock->addFUNC("setparams"        ,(DEBUG_NEW            tellstdfunc::stdSETPARAMETERS(telldata::tn_void, true)));
   mblock->addFUNC("setparams"        ,(DEBUG_NEW             tellstdfunc::stdSETPARAMETER(telldata::tn_void, true)));
   mblock->addFUNC("exit"             ,(DEBUG_NEW                     tellstdfunc::stdEXIT(telldata::tn_void,false)));

   TpdPost::tellFnSort();
}
// Starting macro
IMPLEMENT_APP(TopedApp)
//DECLARE_APP(TopedApp)
