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
#include <wx/wx.h>
#include <wx/tooltip.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <sstream>
#ifdef WIN32
   #include <crtdbg.h>
#endif
#ifdef __linux__
   #include <sys/stat.h>
   #include <sys/types.h>
   #include <unistd.h>
   #include <errno.h>
#endif
#include "toped.h"
#include "viewprop.h"
#include "datacenter.h"
#include "calbr_reader.h"

#include "tellibin.h"
#include "tpdf_db.h"
#include "tpdf_props.h"
#include "tpdf_cells.h"
#include "tpdf_edit.h"
#include "tpdf_add.h"
#include "tpdf_select.h"
#include "tllf_list.h"
#include "tpdf_get.h"
#ifdef __WXOSX_COCOA__
#include "wxcocoa_config.h"
#endif

tui::TopedFrame*                 Toped = NULL;
extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern trend::TrendCenter*       TRENDC;
extern parsercmd::cmdBLOCK*      CMDBlock;
extern parsercmd::TellPreProc*   tellPP;
extern console::toped_logfile    LogFile;
extern console::TllCmdLine*      Console;
extern Calbr::CalbrFile*         DRCData;
extern const wxEventType         wxEVT_RENDER_PARAMS;


//=============================================================================
bool TopedApp::OnInit()
{
   getLocalDirs();
   getGlobalDirs();
   _forceRenderType     = trend::rtTBD;
   _noLog               = false;
   _gui                 = true;
   wxImage::AddHandler(DEBUG_NEW wxPNGHandler);
   // Initialize Toped properties
   PROPC = DEBUG_NEW layprop::PropertyCenter();
   // Initialize Toped database
   DATC  = DEBUG_NEW DataCenter(std::string(_localDir.mb_str(wxConvUTF8)), std::string(_globalDir.mb_str(wxConvUTF8)));
   // initialize the TELL pre-processor
   tellPP = DEBUG_NEW parsercmd::TellPreProc();
   // check command line arguments
   if (!parseCmdLineArgs())
      return false;
   if (_gui)
   {
      // Get the Graphic User Interface (gui system)
      Toped = DEBUG_NEW tui::TopedFrame( wxT( "Toped" ), wxPoint(50,50), wxSize(1200,900) );
      // Check we've got the required graphic capabilities. If not - bail out.
      if (!Toped->view()->initStatus())
      {
         wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
               wxT("Toped can't obtain required GLX Visual. Check your video driver/setup please"),
               wxT("Toped"),
               wxOK | wxICON_ERROR);
         dlg1->ShowModal();
         dlg1->Destroy();
         return false;
      }
      // OK - we've got some kind of graphics we can manage - so create the rendering centre
      TRENDC = DEBUG_NEW trend::TrendCenter(true
                                            ,_forceRenderType
                                            ,Toped->view()->glRC()->useVboRendering()
                                            ,Toped->view()->glRC()->useShaders());
      // Replace the active console in the wx system with Toped console window
      console::ted_log_ctrl *logWindow = DEBUG_NEW console::ted_log_ctrl(Toped->logwin());
      delete wxLog::SetActiveTarget(logWindow);
      // Initialize the tool bars
      Toped->setIconDir(std::string(_tpdResourceDir.mb_str(wxConvFile)));
      Toped->initToolBars();
      tellstdfunc::initFuncLib(Toped, Toped->view());
   }
   else
   {
      TessellPoly::tenderTesel = gluNewTess();
      #ifndef WIN32
         gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                         (GLvoid(*)())&TessellPoly::teselBegin);
         gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                         (GLvoid(*)())&TessellPoly::teselVertex);
         gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_END_DATA,
                         (GLvoid(*)())&TessellPoly::teselEnd);
      #else
         gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                         (GLvoid(__stdcall *)())&TessellPoly::teselBegin);
         gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                         (GLvoid(__stdcall *)())&TessellPoly::teselVertex);
         gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_END_DATA,
                         (GLvoid(__stdcall *)())&TessellPoly::teselEnd);
      #endif

      DEBUG_NEW console::TllCCmdLine();
      TRENDC = DEBUG_NEW trend::TrendCenter(false);
      loadGlfFonts();
      console::ted_log_ctrl *logWindow = DEBUG_NEW console::ted_log_ctrl(NULL);
      delete wxLog::SetActiveTarget(logWindow);
      wxPrintf(wxT("Tell Compiler\n"));
   }

   getTellPathDirs();
   // Create the session log file
   if (!getLogFileName()) return false;
   // It's time to register all internal TELL functions
   // Create the main block parser block - WARNING! BlockSTACK structure MUST already exist!
   CMDBlock = DEBUG_NEW parsercmd::cmdMAIN();
   initInternalFunctions(static_cast<parsercmd::cmdMAIN*>(CMDBlock));
   // Loading of eventual plug-ins
   loadPlugIns();

   if (_gui)
   {
      // Finally show the Toped frame
      SetTopWindow(Toped);
      Toped->Show(TRUE);
      wxToolTip::SetDelay(500);
      // First thing after initialising openGL - load the shaders (eventually)
      std::string stdShaderDir(_tpdShadersDir.mb_str(wxConvFile));
      TRENDC->initShaders(stdShaderDir);
      // and then - load available layout fonts
      loadGlfFonts();
      // at this stage - the tool shall be considered fully functional
      //--------------------------------------------------------------------------
      // Put a rendering info in the log
      printLogWHeader();
   }
   return true;
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
      if (_noLog)
         LogFile.setEnabled(_noLog);
      else
         LogFile.init(std::string(_logFileName.mb_str(wxConvFile)), true);
   }
   else
   {
      // Execute the tell file from the command line
      if (_noLog)
         LogFile.setEnabled(_noLog);
      else
         LogFile.init(std::string(_logFileName.mb_str(wxConvFile)), true);
//      wxLog::AddTraceMask(wxT("thread"));
//      wxLog::AddTraceMask(wxTRACE_MemAlloc);
      defaultStartupScript();
      if ( !_inputTellFile.IsEmpty() )
         Console->parseCommand(_inputTellFile);
   }
   return wxApp::OnRun();
}

void TopedApp::reloadInternalFunctions()
{
   CMDBlock = CMDBlock->rootBlock();
   CMDBlock->cleaner(true);
   delete CMDBlock;
   CMDBlock = DEBUG_NEW parsercmd::cmdMAIN();
   initInternalFunctions(static_cast<parsercmd::cmdMAIN*>(CMDBlock));
   for (PluginList::const_iterator CP = _plugins.begin(); CP != _plugins.end(); CP++)
   {
      ModuleFunction piInitFunc = (ModuleFunction)(*CP)->GetSymbol(wxT("initPluginFunctions"));
      assert(NULL != piInitFunc);
      piInitFunc(static_cast<parsercmd::cmdMAIN*>(CMDBlock));
   }
}

//=============================================================================
int TopedApp::OnExit()
{
   if (DRCData) delete DRCData;
   delete CMDBlock;
   delete PROPC;
   delete TRENDC;
   delete DATC;
   // unload the eventual plug-ins
   for (PluginList::const_iterator CP = _plugins.begin(); CP != _plugins.end(); CP++)
      delete (*CP);
   delete tellPP;

#ifdef DB_MEMORY_TRACE
   MemTrack::TrackDumpBlocks();
#endif

   if (!_noLog) finishSessionLog();
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
   if (fontDirectory.IsOpened())
   {
      wxString curFN;
      if (fontDirectory.GetFirst(&curFN, wxT("*.glf"), wxDIR_FILES))
      {
         do
         {
            std::string ffname(_tpdFontDir.mb_str(wxConvFile));
            ffname += curFN.mb_str(wxConvFile);
            TRENDC->loadLayoutFont(ffname);
         } while (fontDirectory.GetNext(&curFN));
      }
   }
   if (0 == TRENDC->numFonts())
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
      wxCommandEvent eventLoadFont(wxEVT_RENDER_PARAMS);
      eventLoadFont.SetId(tui::RPS_SLCT_FONT);
      eventLoadFont.SetString(wxT("Arial Normal 1"));
      wxPostEvent(Toped, eventLoadFont);
      TRENDC->selectFont("Arial Normal 1");
   }
}

//=============================================================================
void TopedApp::defaultStartupScript()
{
   wxFileName rcFile(_globalDir);
   rcFile.AppendDir(wxT("tll"));
   rcFile.SetFullName(wxT(".topedrc"));
   rcFile.Normalize();
   assert(rcFile.IsOk());
   if (rcFile.FileExists())
   {
      wxString tellCommand;
      tellCommand << wxT("#include \"") << rcFile.GetFullPath() << wxT("\"");
      Console->parseCommand(tellCommand, false);
   }
   else
   {
      rcFile.AssignDir(_localDir);
      rcFile.AppendDir(wxT("tll"));
      rcFile.SetFullName(wxT(".topedrc"));
      rcFile.Normalize();
      assert(rcFile.IsOk());
      if (rcFile.FileExists())
      {
         wxString tellCommand;
         tellCommand << wxT("#include \"") << rcFile.GetFullPath() << wxT("\"");
         Console->parseCommand(tellCommand, false);
      }
      else
      {
         tell_log(console::MT_WARNING,"File \".topedrc\" not found. Consider loading a technology file.");
      }
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
            do
            {
#ifdef __linux__
               // Handling the eventual symbolic links manually. Mainstream wx
               // doesn't seem to have a anything about it yet although there are
               // some reports:
               // http://trac.wxwidgets.org/ticket/1993
               // http://trac.wxwidgets.org/ticket/4619
               // http://groups.google.com/group/wx-users/browse_thread/thread/7a2146856543d06d/713af78928657f66?hl=en&lnk=gst&q=symbolic+link#713af78928657f66
               wxString pinCandName = _tpdPlugInDir + curFN;
               std::string fnstr(pinCandName.mb_str(wxConvUTF8));
               struct stat stbuf;
               if (-1 == stat(fnstr.c_str(), &stbuf))
               {
                  printf( "Can't get a file \"%s\" stat: %s\n", fnstr.c_str(), strerror( errno ) );
                  printf( "Plug-in load operation is likely to fail\n");
               }
               else
               {
                  if (stbuf.st_mode & S_IFLNK)
                  {
                     //File is a symbolic link - it has to be resolved
                     char buf[1024];
                     ssize_t len = readlink(fnstr.c_str(), buf, sizeof(buf)-1);
                     if (-1 == len)
                     {
                        printf( "Can't read a link \"%s\": %s\n", fnstr.c_str(), strerror( errno ) );
                        printf( "Plug-in load operation is likely to fail\n");
                     }
                     else
                     {
                        //OK, the link resolved - replace the original curFN
                        // returned from GetFirst/GetNext
                        buf[len] = '\0';
                        curFN = wxString(buf, wxConvUTF8);
                     }
                  }
                  // else - the file is not a symbolic link - so
                  // there is nothing to do
               }
#endif
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
                  errMessage += wxT("\" dynamically\n");
                  errMessage += wxT("\" Check the OS & Toped consoles for further details");
                  wxMessageBox(errMessage);
                  delete plugin;
               }

            } while (plugDir.GetNext(&curFN));
         }
      }
   }
}

//=============================================================================
bool TopedApp::parseCmdLineArgs()
{

  /* FIXME: it seems the amount of command line options starts to grow
     for the future it could be useful to change to the mighty
     getopt_long() parser from unistd.h
  */ 
   bool runTheTool = true;
   if (1 < argc)
   {
      int curarNum = 1;
      while (curarNum < argc)
      {
         wxString curar(argv[curarNum++]);
         if      (wxT("-ogl_thread") == curar) Toped->setOglThread(true);
         else if (wxT("-ogl_safe")   == curar)
         {
            std::cout << "Obsolete command line option \"-ogl_safe\". "
                      << "Use -render <type> instead"<< std::endl ;
            runTheTool = false;
         }
         else if (wxT("-render")      == curar)
         {
            wxString forceRenderType(argv[curarNum++]);
            if      (wxT("basic")  == forceRenderType) _forceRenderType = trend::rtTolder;
            else if (wxT("vbo")    == forceRenderType) _forceRenderType = trend::rtTenderer;
            else if (wxT("shader") == forceRenderType) _forceRenderType = trend::rtToshader;
            else {
               std::cout << "  -render <type> : One of \"basic\", \"vbo\" or \"shader\" expected" << std::endl;
               runTheTool = false;
            }
         }
         else if (wxT("-nolog")      == curar) _noLog               = true;
         else if (wxT("-nogui")      == curar) _gui                 = false;
         else if (0 == curar.Find(wxT("-I")))
         {
            // Store the include paths in a temporary location until we have a
            // valid console
            _tllIncludePath.Add(curar.Remove(0,2));
//            Console->addTllIncludePath(curar.Remove(0,2));
         }
         else if (0 == curar.Find(wxT("-D")))
         {
            wxString defOnly = curar.Remove(0,2);
            tellPP->cmdlDefine( std::string(defOnly.mb_str(wxConvUTF8)) );
         }
         else if (wxT("-help") == curar)
         { //no idea how useful this could be for Windows users
            std::cout << "Usage: toped {options}* [tll-file]" << std::endl ;
            std::cout << "Command line options:" << std::endl ;
            std::cout << "  -ogl_thread: " << std::endl ;
//            std::cout << "  -ogl_safe  : " << std::endl ;
            std::cout << "  -render <type> : enforce openGL render type. One of \"basic\", \"vbo\" or \"shader\"" << std::endl;
            std::cout << "  -nolog         : logging will be suppressed" << std::endl;
            std::cout << "  -nogui         : GUI will not be started. Useful for TLL parsing" << std::endl ;
            std::cout << "  -I<path>       : includes additional search paths for TLL files" << std::endl ;
            std::cout << "  -D<macro>      : equivalent to #define <macro> " << std::endl ;
            std::cout << "  -help          : This help message " << std::endl ;
            runTheTool = false;
         }
         else if (!(0 == curar.Find('-')))
         {
            _inputTellFile.Clear();
            _inputTellFile << wxT("#include \"") << curar << wxT("\"");
         }
         else
         {
            std::string invalid_argument(curar.mb_str(wxConvUTF8));
            std::cout << "Unknown command line option \"" << invalid_argument <<"\"" << std::endl ;
            runTheTool = false;
            //I think it is more convenient to get a clear indication if something is wrong
            //for Windows users other solutions have to be found
         }
      }
   }
   return runTheTool;
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

   // Check log directory
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
#ifdef __WXOSX_COCOA__
       _globalDir = wxGetCwd() << "/" << MAC_BUNDLE_NAME << ".app/Contents/MacOs/";
#else
       _globalDir = wxT("./");
#endif
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
   // Check shaders directory
   wxFileName shadderFolder(_globalDir);
   shadderFolder.AppendDir(wxT("shaders"));
   shadderFolder.Normalize();
   if (shadderFolder.DirExists())
      _tpdShadersDir = shadderFolder.GetFullPath();
   else
      // Don't generate a noise about shaders directory.
      _tpdShadersDir = wxT("");
}

void TopedApp::getTellPathDirs()
{
   for (unsigned i = 0; i < _tllIncludePath.size(); i++)
      Console->addTllIncludePath(_tllIncludePath[i]);
   Console->addTllEnvList(wxT("TLL_INCLUDE_PATH"));
   wxString tllDefaultIncPath;
   tllDefaultIncPath << _globalDir << wxT("/tll/");
   Console->addTllIncludePath(tllDefaultIncPath);
}

//=============================================================================
void TopedApp::printLogWHeader()
{
   TRENDC->reportRenderer(_forceRenderType);
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
   telldata::TPointType*   pntype      = DEBUG_NEW telldata::TPointType();
   telldata::TBoxType*     bxtype      = DEBUG_NEW telldata::TBoxType(pntype);
   telldata::TBindType*    bndtype     = DEBUG_NEW telldata::TBindType(pntype);
   telldata::TLayerType*   laytype     = DEBUG_NEW telldata::TLayerType();
   telldata::TLMapType*    lmaptype    = DEBUG_NEW telldata::TLMapType(laytype);
   telldata::THshStrType*  hshstrtype  = DEBUG_NEW telldata::THshStrType();

   mblock->addGlobalType("point"     , pntype);
   mblock->addGlobalType("box"       , bxtype);
   mblock->addGlobalType("bind"      , bndtype);
   mblock->addGlobalType("layer"     , laytype);
   mblock->addGlobalType("lmap"      , lmaptype);
   mblock->addGlobalType("strmap"    , hshstrtype);
   //-----------------------------------------------------------------------------------------------------------
   // Internal variables
   //-----------------------------------------------------------------------------------------------------------
   // layout type masks
   mblock->addconstID("_lmbox"   , DEBUG_NEW telldata::TtInt( laydata::_lmbox  ), true);
   mblock->addconstID("_lmpoly"  , DEBUG_NEW telldata::TtInt( laydata::_lmpoly ), true);
   mblock->addconstID("_lmwire"  , DEBUG_NEW telldata::TtInt( laydata::_lmwire ), true);
   mblock->addconstID("_lmtext"  , DEBUG_NEW telldata::TtInt( laydata::_lmtext ), true);
   mblock->addconstID("_lmref"   , DEBUG_NEW telldata::TtInt( laydata::_lmref  ), true);
   mblock->addconstID("_lmaref"  , DEBUG_NEW telldata::TtInt( laydata::_lmaref ), true);
   mblock->addconstID("_lmpref"  , DEBUG_NEW telldata::TtInt( laydata::_lmpref ), true);
   mblock->addconstID("_lmapref" , DEBUG_NEW telldata::TtInt( laydata::_lmapref), true);
   // Toolbar properties
   mblock->addconstID("_horizontal", DEBUG_NEW telldata::TtInt( tui::_tuihorizontal), true);
   mblock->addconstID("_vertical"  , DEBUG_NEW telldata::TtInt( tui::_tuivertical),   true);
   mblock->addconstID("_iconsize16", DEBUG_NEW telldata::TtInt( tui::ICON_SIZE_16x16),true);
   mblock->addconstID("_iconsize24", DEBUG_NEW telldata::TtInt( tui::ICON_SIZE_24x24),true);
   mblock->addconstID("_iconsize32", DEBUG_NEW telldata::TtInt( tui::ICON_SIZE_32x32),true);
   mblock->addconstID("_iconsize48", DEBUG_NEW telldata::TtInt( tui::ICON_SIZE_48x48),true);
   // Renderer properties

   // Internal functions, not user accessible (parser is using them at its discretion)
   mblock->addIntFUNC("$sort_db"      ,(DEBUG_NEW             tellstdfunc::intrnlSORT_DB(telldata::tn_void, false )));
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
   mblock->addFUNC("printf"           ,(DEBUG_NEW                   tellstdfunc::stdPRINTF(telldata::tn_void, true)));
   mblock->addFUNC("sprintf"          ,(DEBUG_NEW                tellstdfunc::stdSPRINTF(telldata::tn_string, true)));
   mblock->addFUNC("status"           ,(DEBUG_NEW               tellstdfunc::stdTELLSTATUS(telldata::tn_void, true)));
   mblock->addFUNC("undo"             ,(DEBUG_NEW                     tellstdfunc::stdUNDO(telldata::tn_void,false)));
   //
   mblock->addFUNC("report_selected"  ,(DEBUG_NEW              tellstdfunc::stdREPORTSLCTD(telldata::tn_void,true )));
   mblock->addFUNC("report_layers"    ,(DEBUG_NEW      tellstdfunc::stdREPORTLAY(TLISTOF(telldata::tn_layer),true )));
   mblock->addFUNC("report_layers"    ,(DEBUG_NEW     tellstdfunc::stdREPORTLAYc(TLISTOF(telldata::tn_layer),true )));
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
   mblock->addFUNC("getciflaymap"     ,(DEBUG_NEW     tellstdfunc::CIFgetlaymap(TLISTOF(telldata::tn_laymap), true)));
   mblock->addFUNC("setciflaymap"     ,(DEBUG_NEW                tellstdfunc::CIFsetlaymap(telldata::tn_void, true)));
   mblock->addFUNC("clearciflaymap"   ,(DEBUG_NEW              tellstdfunc::CIFclearlaymap(telldata::tn_void, true)));
   mblock->addFUNC("gdsread"          ,(DEBUG_NEW          tellstdfunc::GDSread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("gdsimport"        ,(DEBUG_NEW               tellstdfunc::GDSimportList(telldata::tn_void, true)));
   mblock->addFUNC("gdsimport"        ,(DEBUG_NEW                   tellstdfunc::GDSimport(telldata::tn_void, true)));
   mblock->addFUNC("gdsexport"        ,(DEBUG_NEW                tellstdfunc::GDSexportLIB(telldata::tn_void,false)));
   mblock->addFUNC("gdsexport"        ,(DEBUG_NEW                tellstdfunc::GDSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("gdssplit"         ,(DEBUG_NEW                    tellstdfunc::GDSsplit(telldata::tn_void,false)));
   mblock->addFUNC("gdsclose"         ,(DEBUG_NEW                    tellstdfunc::GDSclose(telldata::tn_void, true)));
   mblock->addFUNC("getgdslaymap"     ,(DEBUG_NEW     tellstdfunc::GDSgetlaymap(TLISTOF(telldata::tn_laymap), true)));
   mblock->addFUNC("setgdslaymap"     ,(DEBUG_NEW                tellstdfunc::GDSsetlaymap(telldata::tn_void, true)));
   mblock->addFUNC("cleargdslaymap"   ,(DEBUG_NEW              tellstdfunc::GDSclearlaymap(telldata::tn_void, true)));
   mblock->addFUNC("oasisread"        ,(DEBUG_NEW          tellstdfunc::OASread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("oasisimport"      ,(DEBUG_NEW               tellstdfunc::OASimportList(telldata::tn_void, true)));
   mblock->addFUNC("oasisimport"      ,(DEBUG_NEW                   tellstdfunc::OASimport(telldata::tn_void, true)));
   mblock->addFUNC("oasisclose"       ,(DEBUG_NEW                    tellstdfunc::OASclose(telldata::tn_void, true)));
   mblock->addFUNC("getoasislaymap"   ,(DEBUG_NEW     tellstdfunc::OASgetlaymap(TLISTOF(telldata::tn_laymap), true)));
   mblock->addFUNC("setoasislaymap"   ,(DEBUG_NEW     tellstdfunc::OASsetlaymap(TLISTOF(telldata::tn_laymap), true)));
   mblock->addFUNC("clearoasislaymap" ,(DEBUG_NEW              tellstdfunc::OASclearlaymap(telldata::tn_void, true)));
   mblock->addFUNC("drccalibreimport" ,(DEBUG_NEW            tellstdfunc::DRCCalibreimport(telldata::tn_void, true)));
   mblock->addFUNC("drcshowerror"     ,(DEBUG_NEW                tellstdfunc::DRCshowerror(telldata::tn_void, true)));
   mblock->addFUNC("drcshowcluster"   ,(DEBUG_NEW              tellstdfunc::DRCshowcluster(telldata::tn_void, true)));
   mblock->addFUNC("drcshowallerrors" ,(DEBUG_NEW            tellstdfunc::DRCshowallerrors(telldata::tn_void, true)));
   mblock->addFUNC("drchideallerrors" ,(DEBUG_NEW            tellstdfunc::DRChideallerrors(telldata::tn_void, true)));
   mblock->addFUNC("drcexplainerror"  ,(DEBUG_NEW           tellstdfunc::DRCexplainerror_D(telldata::tn_void, true)));
   mblock->addFUNC("drcexplainerror"  ,(DEBUG_NEW             tellstdfunc::DRCexplainerror(telldata::tn_void, true)));
   mblock->addFUNC("grcgetcells"      ,(DEBUG_NEW      tellstdfunc::grcGETCELLS(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("grcgetlayers"     ,(DEBUG_NEW      tellstdfunc::grcGETLAYERS(TLISTOF(telldata::tn_layer), true)));
   mblock->addFUNC("grcgetdata"       ,(DEBUG_NEW     tellstdfunc::grcGETDATA(TLISTOF(telldata::tn_auxilary), true)));
   mblock->addFUNC("grcrecoverdata"   ,(DEBUG_NEW               tellstdfunc::grcREPAIRDATA(telldata::tn_void, true)));
   mblock->addFUNC("grccleanlayer"    ,(DEBUG_NEW              tellstdfunc::grcCLEANALAYER(telldata::tn_void, true)));
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
   mblock->addFUNC("usinglayer"       ,(DEBUG_NEW             tellstdfunc::stdUSINGLAYER_T(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(DEBUG_NEW             tellstdfunc::stdUSINGLAYER_S(telldata::tn_void, true)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                 tellstdfunc::stdADDBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW               tellstdfunc::stdADDBOX_T(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW               tellstdfunc::stdADDBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdADDBOXr(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXr_T(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXr_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdADDBOXp(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXp_T(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXp_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW                tellstdfunc::stdADDPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW              tellstdfunc::stdADDPOLY_T(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW              tellstdfunc::stdADDPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW                tellstdfunc::stdADDWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW              tellstdfunc::stdADDWIRE_T(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW              tellstdfunc::stdADDWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(DEBUG_NEW                tellstdfunc::stdADDTEXT(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(DEBUG_NEW               tellstdfunc::stdDRAWTEXT(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(DEBUG_NEW                tellstdfunc::stdCELLREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(DEBUG_NEW              tellstdfunc::stdCELLREF_D(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW               tellstdfunc::stdCELLAREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW              tellstdfunc::stdCELLAREFO(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW            tellstdfunc::stdCELLAREFO_D(telldata::tn_layout,false)));
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
   mblock->addFUNC("changelayer"      ,(DEBUG_NEW              tellstdfunc::stdCHANGELAY_T(telldata::tn_void,false)));
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
   mblock->addFUNC("layprop"          ,(DEBUG_NEW                tellstdfunc::stdLAYPROP_T(telldata::tn_void, true)));
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
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdDRAWBOX_T(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdDRAWBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW               tellstdfunc::stdDRAWPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW             tellstdfunc::stdDRAWPOLY_T(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW             tellstdfunc::stdDRAWPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW               tellstdfunc::stdDRAWWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW             tellstdfunc::stdDRAWWIRE_T(telldata::tn_layout,false)));
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
   mblock->addFUNC("exec"             ,(DEBUG_NEW                     tellstdfunc::stdEXEC(telldata::tn_void, true)));
   mblock->addFUNC("exit"             ,(DEBUG_NEW                     tellstdfunc::stdEXIT(telldata::tn_void,false)));

   TpdPost::tellFnSort();
}
// Starting macro
IMPLEMENT_APP(TopedApp)
