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
//        Created: Mon May 10 23:12:15 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: wxWidget dependent classes for log hadling, exception
//                 classes etc.
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <time.h>
#include <string>
#include <sstream>
#include <wx/string.h>
#include <wx/regex.h>
#include <wx/filename.h>
#include "outbox.h"
#include "tuidefs.h"
#include "../ui/red_lamp.xpm"
#include "../ui/green_lamp.xpm"
#include "../ui/blue_lamp.xpm"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_STATUS      , 10000)
    DECLARE_EVENT_TYPE(wxEVT_SETINGSMENU        , 10001)
    DECLARE_EVENT_TYPE(wxEVT_CMD_BROWSER        , 10002)
    DECLARE_EVENT_TYPE(wxEVT_LOG_ERRMESSAGE     , 10003)
    DECLARE_EVENT_TYPE(wxEVT_MOUSE_ACCEL        , 10004)
    DECLARE_EVENT_TYPE(wxEVT_MOUSE_INPUT        , 10005)
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_ZOOM        , 10006)
    DECLARE_EVENT_TYPE(wxEVT_FUNC_BROWSER       , 10007)
    DECLARE_EVENT_TYPE(wxEVT_TPDSTATUS          , 10008)
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_CURSOR      , 10009)
    DECLARE_EVENT_TYPE(wxEVT_CONSOLE_PARSE      , 10010)
    DECLARE_EVENT_TYPE(wxEVT_CURRENT_LAYER      , 10011)
    DECLARE_EVENT_TYPE(wxEVT_TOOLBARSIZE        , 10012)
    DECLARE_EVENT_TYPE(wxEVT_TOOLBARDEF         , 10013)
    DECLARE_EVENT_TYPE(wxEVT_TOOLBARADDITEM     , 10014)
    DECLARE_EVENT_TYPE(wxEVT_TOOLBARDELETEITEM  , 10015)
    DECLARE_EVENT_TYPE(wxEVT_EDITLAYER          , 10016)
    DECLARE_EVENT_TYPE(wxEVT_QUITAPP            , 10017)
    DECLARE_EVENT_TYPE(wxEVT_EXECEXT            , 10018)
    DECLARE_EVENT_TYPE(wxEVT_EXECEXTDONE        , 10019)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_CANVAS_STATUS)
DEFINE_EVENT_TYPE(wxEVT_SETINGSMENU)
DEFINE_EVENT_TYPE(wxEVT_CMD_BROWSER)
DEFINE_EVENT_TYPE(wxEVT_LOG_ERRMESSAGE)
DEFINE_EVENT_TYPE(wxEVT_MOUSE_ACCEL)
DEFINE_EVENT_TYPE(wxEVT_MOUSE_INPUT)
DEFINE_EVENT_TYPE(wxEVT_CANVAS_ZOOM)
DEFINE_EVENT_TYPE(wxEVT_FUNC_BROWSER)
DEFINE_EVENT_TYPE(wxEVT_TPDSTATUS)
DEFINE_EVENT_TYPE(wxEVT_CANVAS_CURSOR)
DEFINE_EVENT_TYPE(wxEVT_CONSOLE_PARSE)
DEFINE_EVENT_TYPE(wxEVT_CURRENT_LAYER)
DEFINE_EVENT_TYPE(wxEVT_TOOLBARSIZE)
DEFINE_EVENT_TYPE(wxEVT_TOOLBARDEF)
DEFINE_EVENT_TYPE(wxEVT_TOOLBARADDITEM)
DEFINE_EVENT_TYPE(wxEVT_TOOLBARDELETEITEM)
DEFINE_EVENT_TYPE(wxEVT_EDITLAYER)
DEFINE_EVENT_TYPE(wxEVT_QUITAPP)
DEFINE_EVENT_TYPE(wxEVT_EXECEXT)
DEFINE_EVENT_TYPE(wxEVT_EXECEXTDONE)

console::TELLFuncList*  CmdList = NULL;

wxWindow*               TpdPost::_statusBar     = NULL;
wxWindow*               TpdPost::_topBrowsers   = NULL;
wxWindow*               TpdPost::_layBrowser    = NULL;
wxWindow*               TpdPost::_cllBrowser    = NULL;
wxWindow*               TpdPost::_cmdLine       = NULL;
wxWindow*               TpdPost::_tllFuncList   = NULL;
wxWindow*               TpdPost::_mainWindow    = NULL;

//==============================================================================
// The ted_log event table
BEGIN_EVENT_TABLE( console::ted_log, wxTextCtrl )
   EVT_TECUSTOM_COMMAND(wxEVT_LOG_ERRMESSAGE, -1, ted_log::OnLOGMessage)
END_EVENT_TABLE()

console::ted_log::ted_log(wxWindow *parent, wxWindowID id): wxTextCtrl( parent, id, wxT(""),
   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER|wxTE_RICH)
{
   cmd_mark = wxT("=> ");
   gui_mark = wxT(">> ");
   rply_mark = wxT("<= ");
}

void console::ted_log::OnLOGMessage(wxCommandEvent& evt) {
   wxColour logColour;
   long int startPos = GetLastPosition();
   switch (evt.GetInt())
   {
      case    MT_INFO:
         *this << rply_mark << evt.GetString() << wxT("\n");
         logColour = *wxBLACK;
         break;
      case   MT_ERROR:
         *this << rply_mark << evt.GetString() << wxT("\n");
         logColour = *wxRED;
         break;
      case MT_COMMAND:
         *this << cmd_mark << evt.GetString() << wxT("\n");
         break;
      case MT_GUIPROMPT:
         *this << gui_mark;
         break;
      case MT_GUIINPUT:
         *this << evt.GetString();
         break;
      case MT_EOL:
         *this << wxT("\n");
         break;
      case MT_WARNING:
         *this << rply_mark << evt.GetString() << wxT("\n");
         logColour = *wxBLUE;
         break;
      case MT_CELLNAME:
         *this << rply_mark << wxT(" Cell ") << evt.GetString() << wxT("\n");
         break;
      case MT_DESIGNNAME:
         *this << rply_mark << wxT(" Design ") << evt.GetString() << wxT("\n");
         break;
      default: //wx Messages
         *this <<wxT("WX MESSAGE Level:") << evt.GetInt() << wxT(" \"") << evt.GetString() << wxT("\"\n");
         logColour = *wxLIGHT_GREY;
         break;
   }
   long int endPos = GetLastPosition();
   SetStyle(startPos,endPos,wxTextAttr(logColour));
   // Truncate the log window contents from the bottom to avoid
   wxTextPos curLogSize = GetLastPosition();
   if (curLogSize > 0x7800) // 30K
   {
      Replace(0, 0x1000, wxT("....truncated....\n"));
   }
   evt.Skip();
}

void console::ted_log_ctrl::DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp) {
   wxCommandEvent eventLOG(wxEVT_LOG_ERRMESSAGE);
   eventLOG.SetString(msg);
   eventLOG.SetInt(level);
   eventLOG.SetExtraLong(timestamp);
   wxPostEvent(_tellLOGW, eventLOG);
}

void tell_log(console::LOG_TYPE lt, const char* msg)
{
   wxLog::OnLog(lt, wxString(msg, wxConvUTF8), time(NULL));
}

void tell_log(console::LOG_TYPE lt, const std::string& msg)
{
   wxLog::OnLog(lt, wxString(msg.c_str(), wxConvUTF8), time(NULL));
}

void tell_log(console::LOG_TYPE lt, const wxString& msg)
{
   wxLog::OnLog(lt, msg, time(NULL));
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(console::TopedStatus, wxStatusBar)
   EVT_SIZE(console::TopedStatus::OnSize)
   EVT_TECUSTOM_COMMAND(wxEVT_TPDSTATUS  , wxID_ANY, console::TopedStatus::OnTopedStatus)
END_EVENT_TABLE()

console::TopedStatus::TopedStatus(wxWindow* parent) : wxStatusBar(parent, tui::ID_TPD_STATUS)
{
   const unsigned Field_Max = 3;
   static const int widths[Field_Max] = { -1, -1, 64 };

    SetFieldsCount(Field_Max);
    SetStatusWidths(Field_Max, widths);
    _dbLamp = DEBUG_NEW wxStaticBitmap(this, wxID_ANY, wxIcon(green_lamp));
    _rndrLamp = DEBUG_NEW wxStaticBitmap(this, wxID_ANY, wxIcon(green_lamp));
    _progress = NULL;
    _progressAdj = 1.0;
}

void console::TopedStatus::OnTopedStatus(wxCommandEvent& evt)
{
   switch (evt.GetInt()) {
      case console::TSTS_PRGRSBARON  : OnInitGauge(evt.GetExtraLong()); break;
      case console::TSTS_PROGRESS    : OnGaugeRun(evt.GetExtraLong()); break;
      case console::TSTS_PRGRSBAROFF : OnCloseGauge(); break;
      case console::TSTS_THREADON    : OnThreadON(evt.GetString()); break;
      case console::TSTS_THREADWAIT  : OnThreadWait(); break;
      case console::TSTS_THREADOFF   : OnThreadOFF(); break;
      case console::TSTS_RENDERON    : OnRenderON(); break;
      case console::TSTS_RENDEROFF   : OnRenderOFF(); break;
      default: assert(false);
   }
}


void console::TopedStatus::OnThreadON(wxString cmd)
{
   SetStatusText(cmd,1);
   _dbLamp->SetBitmap(wxIcon(red_lamp));
}

void console::TopedStatus::OnThreadWait()
{
//   SetStatusText(wxT("Ready..."),1);
   _dbLamp->SetBitmap(wxIcon(blue_lamp));
}

void console::TopedStatus::OnThreadOFF()
{
   SetStatusText(wxT("Ready..."),1);
   _dbLamp->SetBitmap(wxIcon(green_lamp));
}

void console::TopedStatus::OnRenderON()
{
   SetStatusText(wxT("Rendering..."),1);
   _rndrLamp->SetBitmap(wxIcon(red_lamp));
   Update();
}

void console::TopedStatus::OnRenderOFF()
{
   SetStatusText(wxT("Ready..."),1);
   _rndrLamp->SetBitmap(wxIcon(green_lamp));
   Update();
}

void console::TopedStatus::OnInitGauge(long int init_val)
{
    wxRect rect;
    GetFieldRect(1, rect);
    if (init_val > ((long) 0x7FFFFFF))
    {
      _progressAdj = (real) ((long) 0x7ffffff) / ((real)init_val);
      _progress = DEBUG_NEW wxGauge(this, wxID_ANY, ((int) 0x7ffffff), wxPoint(rect.x, rect.y), wxSize(rect.width, rect.height));
    }
    else
      _progress = DEBUG_NEW wxGauge(this, wxID_ANY, init_val, wxPoint(rect.x, rect.y), wxSize(rect.width, rect.height));
}

void console::TopedStatus::OnGaugeRun(long int position)
{
   if (NULL == _progress) return;
   int adj_position = (int)((real)position * _progressAdj);
   _progress->SetValue(adj_position);
}

void console::TopedStatus::OnCloseGauge()
{
   if (NULL != _progress)
   {
      delete (_progress);
      _progress = NULL;
      _progressAdj = 1.0;
   }
}

void console::TopedStatus::OnSize(wxSizeEvent& event)
{
   wxRect rect;
   if (NULL != _progress)
   {
      GetFieldRect(1, rect);
      _progress->SetSize(rect);
   }

   GetFieldRect(2, rect);
   wxSize size = _dbLamp->GetSize();
   _rndrLamp->Move(rect.x + rect.width / 3 - size.x / 2,
                   rect.y + (rect.height - size.y) / 2);
   _dbLamp->Move(rect.x + 2* rect.width / 3  - size.x / 2,
                 rect.y +   (rect.height - size.y) / 2);

    event.Skip();
}

//==============================================================================

TpdPost::TpdPost(wxWindow* mainWindow)
{
   _mainWindow  = mainWindow;
   _statusBar   = _mainWindow->FindWindow(tui::ID_TPD_STATUS);
   _topBrowsers = _mainWindow->FindWindow(tui::ID_WIN_BROWSERS);
   _layBrowser  = _mainWindow->FindWindow(tui::ID_PNL_LAYERS);
   _cllBrowser  = _mainWindow->FindWindow(tui::ID_PNL_CELLS);
   _cmdLine     = _mainWindow->FindWindow(tui::ID_CMD_LINE);
   _tllFuncList = _mainWindow->FindWindow(tui::ID_TELL_FUNCS);
   CmdList = static_cast<console::TELLFuncList*>(_tllFuncList);
}

void TpdPost::toped_status(console::TOPEDSTATUS_TYPE tstatus)
{
   if (NULL == _statusBar) return;
   wxCommandEvent eventSTATUSUPD(wxEVT_TPDSTATUS);
   eventSTATUSUPD.SetInt(tstatus);
   wxPostEvent(_statusBar, eventSTATUSUPD);
}

void TpdPost::toped_status(console::TOPEDSTATUS_TYPE tstatus, long int indx)
{
   if (NULL == _statusBar) return;
   wxCommandEvent eventSTATUSUPD(wxEVT_TPDSTATUS);
   eventSTATUSUPD.SetInt(tstatus);
   eventSTATUSUPD.SetExtraLong(indx);
   wxPostEvent(_statusBar, eventSTATUSUPD);
}

void TpdPost::toped_status(console::TOPEDSTATUS_TYPE tstatus, std::string sts_cmd)
{
   if (NULL == _statusBar) return;
   wxCommandEvent eventSTATUSUPD(wxEVT_TPDSTATUS);
   eventSTATUSUPD.SetInt(tstatus);
   eventSTATUSUPD.SetString(wxString(sts_cmd.c_str(), wxConvUTF8));
   wxPostEvent(_statusBar, eventSTATUSUPD);
}

void TpdPost::toped_status(console::TOPEDSTATUS_TYPE tstatus, wxString sts_cmd)
{
   if (NULL == _statusBar) return;
   wxCommandEvent eventSTATUSUPD(wxEVT_TPDSTATUS);
   eventSTATUSUPD.SetInt(tstatus);
   eventSTATUSUPD.SetString(sts_cmd);
   wxPostEvent(_statusBar, eventSTATUSUPD);
}

void TpdPost::render_status(bool on_off)
{
   if (on_off)
      static_cast<console::TopedStatus*>(_statusBar)->OnRenderON();
   else
      static_cast<console::TopedStatus*>(_statusBar)->OnRenderOFF();
}

void TpdPost::refreshTDTtab(bool targetDB, bool threadExecution)
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_ADDTDT_LIB);
   eventADDTAB.SetExtraLong(targetDB ? 1 : 0);
   if (threadExecution)
      wxPostEvent( _topBrowsers, eventADDTAB );
   else
   {
      ::wxSafeYield(_topBrowsers);
      _topBrowsers->GetEventHandler()->ProcessEvent(eventADDTAB);
   }
}

void TpdPost::addGDStab(bool threadExecution)
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_ADDGDS_TAB);
   if (threadExecution)
      wxPostEvent(_topBrowsers, eventADDTAB);
   else
   {
      ::wxSafeYield(_topBrowsers);
      _topBrowsers->GetEventHandler()->ProcessEvent(eventADDTAB);
   }
}

void TpdPost::addCIFtab(bool threadExecution)
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_ADDCIF_TAB);
   if (threadExecution)
      wxPostEvent(_topBrowsers, eventADDTAB);
   else
   {
      ::wxSafeYield(_topBrowsers);
      _topBrowsers->GetEventHandler()->ProcessEvent(eventADDTAB);
   }
}

void TpdPost::addOAStab(bool threadExecution)
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_ADDOAS_TAB);
   if (threadExecution)
      wxPostEvent(_topBrowsers, eventADDTAB);
   else
   {
      ::wxSafeYield(_topBrowsers);
      _topBrowsers->GetEventHandler()->ProcessEvent(eventADDTAB);
   }
}

void TpdPost::addDRCtab()
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_ADDDRC_TAB);
   wxPostEvent(_topBrowsers, eventADDTAB);
}

void TpdPost::clearGDStab()
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_CLEARGDS_TAB);
   wxPostEvent(_topBrowsers, eventADDTAB);
}

void TpdPost::clearCIFtab()
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_CLEARCIF_TAB);
   wxPostEvent(_topBrowsers, eventADDTAB);
}

void TpdPost::clearOAStab()
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_CLEAROAS_TAB);
   wxPostEvent(_topBrowsers, eventADDTAB);
}

void TpdPost::clearDRCtab()
{
   assert(_topBrowsers);
   wxCommandEvent eventADDTAB(wxEVT_CMD_BROWSER);
   eventADDTAB.SetInt(tui::BT_CLEARDRC_TAB);
   wxPostEvent(_topBrowsers, eventADDTAB);
}

void TpdPost::layer_status(int btype, const word layno, const bool status)
{
   assert(_layBrowser);
   wxCommandEvent eventLAYER_STATUS(wxEVT_CMD_BROWSER);
   eventLAYER_STATUS.SetExtraLong(status);
   eventLAYER_STATUS.SetInt(btype);
   word *laynotemp = DEBUG_NEW word(layno);
   eventLAYER_STATUS.SetClientData(static_cast<void*> (laynotemp));
   wxPostEvent(_layBrowser, eventLAYER_STATUS);
}

void TpdPost::layer_add(const std::string name, const word layno)
{
   assert(_layBrowser);
   wxCommandEvent eventLAYER_ADD(wxEVT_CMD_BROWSER);
   word *laynotemp = DEBUG_NEW word(layno);
   eventLAYER_ADD.SetClientData(static_cast<void*> (laynotemp));
   eventLAYER_ADD.SetString(wxString(name.c_str(), wxConvUTF8));
   eventLAYER_ADD.SetInt(tui::BT_LAYER_ADD);
   wxPostEvent(_layBrowser, eventLAYER_ADD);
}

void TpdPost::layer_default(const word newlay, const word oldlay)
{
   assert(_layBrowser);
   wxCommandEvent eventLAYER_DEF(wxEVT_CMD_BROWSER);
   eventLAYER_DEF.SetExtraLong(newlay);
   word *laynotemp = DEBUG_NEW word(oldlay);
   eventLAYER_DEF.SetClientData(static_cast<void*> (laynotemp));
   eventLAYER_DEF.SetInt(tui::BT_LAYER_DEFAULT);
   wxPostEvent(_layBrowser, eventLAYER_DEF);
}

void TpdPost::layers_state(const std::string& name, bool add)
{
   assert(_layBrowser);
   wxCommandEvent eventLAYERS_STATE(wxEVT_CMD_BROWSER);
   eventLAYERS_STATE.SetString(wxString(name.c_str(), wxConvUTF8));
   if (add)
      eventLAYERS_STATE.SetInt(tui::BT_LAYSTATE_SAVE);
   else
      eventLAYERS_STATE.SetInt(tui::BT_LAYSTATE_DELETE);
   wxPostEvent(_layBrowser, eventLAYERS_STATE);
}

void TpdPost::resetTDTtab(const std::string dbName)
{
   assert(_topBrowsers);
   wxCommandEvent resetTDTTab(wxEVT_CMD_BROWSER);
   resetTDTTab.SetInt(tui::BT_NEWTDT_DB);
   resetTDTTab.SetString(wxString(dbName.c_str(), wxConvUTF8));
   wxPostEvent( _cllBrowser, resetTDTTab );
}

void TpdPost::celltree_open(const std::string cname)
{
   assert(_cllBrowser);
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(tui::BT_CELL_OPEN);
   eventCELLTREE.SetString(wxString(cname.c_str(), wxConvUTF8));
   wxPostEvent(_cllBrowser, eventCELLTREE);
}

void TpdPost::celltree_highlight(const std::string cname)
{
   assert(_cllBrowser);
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(tui::BT_CELL_HIGHLIGHT);
   eventCELLTREE.SetString(wxString(cname.c_str(), wxConvUTF8));
   wxPostEvent(_cllBrowser, eventCELLTREE);
}

void TpdPost::treeAddMember(const char* cell, const char* parent, int action)
{
   assert(_cllBrowser);
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(tui::BT_CELL_ADD);
   eventCELLTREE.SetString(wxString(cell, wxConvUTF8));
   eventCELLTREE.SetExtraLong(action);
   wxString* prnt = DEBUG_NEW wxString(parent, wxConvUTF8);
   eventCELLTREE.SetClientData(static_cast<void*> (prnt));
   wxPostEvent(_cllBrowser, eventCELLTREE);
}

void TpdPost::treeRemoveMember(const char* cell, const char* parent, int action)
{
   assert(_cllBrowser);
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(tui::BT_CELL_REMOVE);
   eventCELLTREE.SetString(wxString(cell, wxConvUTF8));
   eventCELLTREE.SetExtraLong(action);
   wxString* prnt = DEBUG_NEW wxString(parent, wxConvUTF8);
   eventCELLTREE.SetClientData(static_cast<void*> (prnt));
   wxPostEvent(_cllBrowser, eventCELLTREE);
}

void TpdPost::treeRenameMember(const char* oldName, const char* newName)
{
   assert(_cllBrowser);
   wxCommandEvent eventCELLTREE(wxEVT_CMD_BROWSER);
   eventCELLTREE.SetInt(tui::BT_CELL_RENAME);
   eventCELLTREE.SetString(wxString(oldName, wxConvUTF8));
   wxString* prnt = DEBUG_NEW wxString(newName, wxConvUTF8);
   eventCELLTREE.SetClientData(static_cast<void*> (prnt));
   wxPostEvent(_cllBrowser, eventCELLTREE);
}

void TpdPost::parseCommand(const wxString cmd)
{
   assert(_cmdLine);
   wxCommandEvent eventPARSE(wxEVT_CONSOLE_PARSE);
   eventPARSE.SetString(cmd);
   wxPostEvent(_cmdLine, eventPARSE);
}

void TpdPost::tellFnAdd(const std::string name, void* arguments)
{
   wxCommandEvent eventFUNCTION_ADD(wxEVT_FUNC_BROWSER);
   eventFUNCTION_ADD.SetString(wxString(name.c_str(), wxConvUTF8));
   eventFUNCTION_ADD.SetClientData(arguments);
   eventFUNCTION_ADD.SetInt(console::FT_FUNCTION_ADD);
   wxPostEvent(_tllFuncList, eventFUNCTION_ADD);
}

void TpdPost::tellFnSort()
{
   wxCommandEvent eventFUNCTION_ADD(wxEVT_FUNC_BROWSER);
   eventFUNCTION_ADD.SetInt(console::FT_FUNCTION_SORT);
   wxPostEvent(_tllFuncList, eventFUNCTION_ADD);
}

void TpdPost::execExt(const wxString extCmd)
{
   wxCommandEvent eventEXEXEXT(wxEVT_EXECEXT);
   eventEXEXEXT.SetString(extCmd);
   wxPostEvent(_mainWindow, eventEXEXEXT);
}

void TpdPost::quitApp(bool threadExecution)
{
   wxCommandEvent eventQUITAPP(wxEVT_QUITAPP);
   //   eventQUITAPP.SetInt(???);
   if (threadExecution)
   {
      wxPostEvent(_mainWindow, eventQUITAPP);
   }
   else
   {
      ::wxSafeYield(_mainWindow);
      _mainWindow->GetEventHandler()->ProcessEvent(eventQUITAPP);
   }
}


//==============================================================================
//
// The existence of this function is ridiculous.
// It is required to keep the list of defined TELL functions (internal&external)
// sorted. Required by wxListCtrl::SortItems as a callback function
// The trouble is that there is no clean way in C++ to make a call back to a
// method. There are much simpler ways to achieve the same functionality, but
// wx boys didn't use them in this particular case.
// So the function MUST be global.
// Having the function defined as global means that it's loosing the connection
// with the data in the control (class) which means that it requires a global
// pointer to to the control itself. It's complete rubbish!
//
// Must be reported to wx lads!
//
int wxCALLBACK wxListCompareFunction(long item1, long item2, long sortData)
{
   wxListItem li1, li2;
   li1.SetMask(wxLIST_MASK_TEXT);
   li1.SetColumn(1);
   li1.SetId(CmdList->FindItem(-1, item1));
   CmdList->GetItem(li1);
   li2.SetMask(wxLIST_MASK_TEXT);
   li2.SetColumn(1);
   li2.SetId(CmdList->FindItem(-1, item2));
   CmdList->GetItem(li2);
   wxString s1 = li1.GetText();
   wxString s2 = li2.GetText();
   return s1.CompareTo(s2.c_str());
}

BEGIN_EVENT_TABLE( console::TELLFuncList, wxListCtrl )
   EVT_TECUSTOM_COMMAND(wxEVT_FUNC_BROWSER, -1, TELLFuncList::OnCommand)
END_EVENT_TABLE()

console::TELLFuncList::TELLFuncList(wxWindow *parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style) :
      wxListView(parent, id, pos, size, style)
{
   InsertColumn(0, wxT("type"));
   InsertColumn(1, wxT("name"));
   InsertColumn(2, wxT("arguments"));
   SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
}


console::TELLFuncList::~TELLFuncList()
{
   DeleteAllItems();
}

void console::TELLFuncList::addFunc(wxString name, void* arguments)
{
   ArgList* arglist = static_cast<ArgList*>(arguments);
   wxListItem row;
   row.SetMask(wxLIST_MASK_DATA | wxLIST_MASK_TEXT);
   row.SetId(GetItemCount());
   row.SetData(GetItemCount());
   row.SetText( wxString((arglist->front()).c_str(), wxConvUTF8));arglist->pop_front();
   InsertItem(row);
   SetColumnWidth(0, wxLIST_AUTOSIZE);
   //
   row.SetColumn(1);
   row.SetMask(wxLIST_MASK_TEXT);
   row.SetText(name.c_str());
   SetItem(row);
   SetColumnWidth(1, wxLIST_AUTOSIZE);
   //
   wxString strlist(wxT("( "));
   while (!arglist->empty())
   {
      strlist << wxString(arglist->front().c_str(), wxConvUTF8);arglist->pop_front();
      if (arglist->size()) strlist << wxT(" , ");
   }
   delete arglist;
   strlist << wxT(" )");
   //
   row.SetColumn(2);
   row.SetMask(wxLIST_MASK_TEXT);
   row.SetText(strlist);
   SetItem(row);
   SetColumnWidth(2, wxLIST_AUTOSIZE);
}

void console::TELLFuncList::OnCommand(wxCommandEvent& event)
{
   int command = event.GetInt();
   switch (command)
   {
      case console::FT_FUNCTION_ADD:addFunc(event.GetString(), event.GetClientData()); break;
      case console::FT_FUNCTION_SORT:SortItems(wxListCompareFunction,0); break;
      default: assert(false);
   }
}

//=============================================================================
std::string TpdTime::operator () ()
{
   tm* broken_time = localtime(&_stdCTime);
   VERIFY(broken_time != NULL);
   char* btm = DEBUG_NEW char[256];
   strftime(btm, 256, "%d-%m-%Y %I:%M:%S %p", broken_time);
   std::string info = btm;
   delete [] btm;
   return info;
}

TpdTime::TpdTime(std::string str_time)
{
   wxString wxstr_time(str_time.c_str(), wxConvUTF8);
   patternNormalize(wxstr_time);
   _status = getStdCTime(wxstr_time);
}

TpdTime::TpdTime(tm& brokenTime)
{
   _stdCTime = mktime(&brokenTime);
   _status = (_stdCTime >= 0);
}

void TpdTime::patternNormalize(wxString& str) {
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before separators
   VERIFY(regex.Compile(wxT("([[:space:]])([\\-\\:])")));
   regex.ReplaceAll(&str,wxT("\\2"));
   // remove spaces after separators
   VERIFY(regex.Compile(wxT("([\\-\\:])([[:space:]])")));
   regex.ReplaceAll(&str,wxT("\\1"));

}

bool TpdTime::getStdCTime(wxString& exp) {
   const wxString tmpl2digits      = wxT("[[:digit:]]{1,2}");
   const wxString tmpl4digits      = wxT("[[:digit:]]{4,4}");
   const wxString tmplDate         = tmpl2digits+wxT("\\-")+tmpl2digits+wxT("\\-")+tmpl4digits;
   const wxString tmplTime         = tmpl2digits+wxT("\\:")+tmpl2digits+wxT("\\:")+tmpl2digits;
   const wxString tmplAmPm         = wxT("[AP]M");
   wxRegEx src_tmpl(tmplDate+wxT("[[:space:]]")+tmplTime + wxT("[[:space:]]") + tmplAmPm);
   VERIFY(src_tmpl.IsValid());
   long conversion;
   // search the entire pattern
   if (!src_tmpl.Matches(exp))
   {
      std::string news = "Can't recognise the time format. Recovery will be unreliable ";
      tell_log(console::MT_ERROR,news);
      _stdCTime = 0;
      return false;
   }
   tm broken_time;
   // get the date
   VERIFY(src_tmpl.Compile(tmpl2digits));
   src_tmpl.Matches(exp);
   src_tmpl.GetMatch(exp).ToLong(&conversion);
   VERIFY(conversion);
   broken_time.tm_mday = conversion;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // get month
   src_tmpl.Matches(exp);
   VERIFY(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_mon = conversion - 1;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // get year
   VERIFY(src_tmpl.Compile(tmpl4digits));
   src_tmpl.Matches(exp);
   VERIFY(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_year = conversion - 1900;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // now the time - first hour
   VERIFY(src_tmpl.Compile(tmpl2digits));
   src_tmpl.Matches(exp);
   VERIFY(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_hour = conversion;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // minutes
   src_tmpl.Matches(exp);
   VERIFY(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_min = conversion;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // and seconds
   src_tmpl.Matches(exp);
   VERIFY(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_sec = conversion;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   //AM-PM
   VERIFY(src_tmpl.Compile(tmplAmPm));
   if (src_tmpl.Matches(exp))
   {
      wxString ampm = src_tmpl.GetMatch(exp);
      assert(0 != ampm.Len());
      if ( wxT("PM") == ampm )
         broken_time.tm_hour += 12;
      src_tmpl.ReplaceFirst(&exp,wxT(""));
   }
   //
   broken_time.tm_isdst = -1;
   _stdCTime = mktime(&broken_time);
   return true;
}

bool expandFileName( std::string& filename)
{
   wxFileName fName(wxString(filename.c_str(), wxConvFile ));
   fName.Normalize();
   if (fName.IsOk())
   {
      wxString dirName = fName.GetFullPath();
      if (!dirName.Matches(wxT("*$*")))
      {
         filename = fName.GetFullPath().mb_str(wxConvFile );
         return true;
      }
   }
   return false;
}

//std::string getFileNameOnly( std::string filename )
//{
//   wxFileName fName(wxString(filename.c_str(), wxConvUTF8));
//   fName.Normalize();
//   assert (fName.IsOk());
//   {
//      wxString name = fName.GetName();
//      return std::string(name.mb_str(wxConvFile ));
//   }
//}

//Convert string from UTF8 to wxConvFile
std::string convertString(const std::string &str)
{
   wxString wxstr(str.c_str(), wxConvUTF8 );
   std::string retstr( wxstr.mb_str(wxConvFile ) );
   return retstr;
}

//=============================================================================
EXPTNactive_cell::EXPTNactive_cell() {
   std::string news = "No active cell. Use opencell(\"<name>\") to select one";
   tell_log(console::MT_ERROR,news);
};

EXPTNactive_DB::EXPTNactive_DB() {
   std::string news = "No active database. Create or load one";
   tell_log(console::MT_ERROR,news);
};

EXPTNactive_GDS::EXPTNactive_GDS() {
   std::string news = "No GDS structure in memory. Parse first";
   tell_log(console::MT_ERROR,news);
};

EXPTNactive_CIF::EXPTNactive_CIF() {
   std::string news = "No CIF structure in memory. Parse first";
   tell_log(console::MT_ERROR,news);
};

EXPTNactive_OASIS::EXPTNactive_OASIS() {
   std::string news = "No OASIS structure in memory. Parse first";
   tell_log(console::MT_ERROR,news);
};

EXPTNreadTDT::EXPTNreadTDT(std::string info) {
   std::string news = "Error parsing TDT file =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNreadGDS::EXPTNreadGDS(std::string info) {
   std::string news = "Error parsing GDS file =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNreadOASIS::EXPTNreadOASIS(std::string info) {
   std::string news = "Error parsing OASIS file =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNpolyCross::EXPTNpolyCross(std::string info) {
   std::string news = "Internal error - polygon cross =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNtell_parser::EXPTNtell_parser(std::string info) {
   std::string news = "TELL parser fatal error =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNcif_parser::EXPTNcif_parser(std::string info) {
   std::string news = "CIF parser fatal error =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNdrc_reader::EXPTNdrc_reader(std::string info)
{
   std::string news = "Error in drc reader =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};



EXPTNdrc_parser::EXPTNdrc_parser(std::string info)
{
   std::string news = "Error in drc parser =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNdrc_parser::EXPTNdrc_parser(err_message type, std::string info, std::string wrongStr)
{
   if (type == drc_std)
   {
      std::string news = "Error in drc parser =>";
      news += info;
      news += "\n";
      news += wrongStr;
      tell_log(console::MT_ERROR,news);
   }
   else
   {
      std::string news = "Can't parse  rule\n";
      news += info;
      news += "\n";
      news += "string:\n";
      news += wrongStr;
      tell_log(console::MT_ERROR,news);
   }
};

//=============================================================================
LayerMapExt::LayerMapExt(const USMap& inlist, ExtLayers* alist)
   : _theMap(), _status(true), _alist(alist)
{
   _import = (NULL != _alist);
   for (USMap::const_iterator CE = inlist.begin(); CE != inlist.end(); CE++)
   {
      wxString exp(CE->second.c_str(), wxConvUTF8);
      patternNormalize(exp);
      _status &= parseLayTypeString(exp, CE->first);
   }
}

LayerMapExt::~LayerMapExt()
{
   if (NULL != _alist) delete _alist;
}

bool LayerMapExt::parseLayTypeString(wxString exp, word tdtLay)
{
   wxString lay_exp, type_exp;
   if (!separateQuickLists(exp, lay_exp, type_exp)) return false;


   WordList llst;
   // get the listed layers
   getList(  lay_exp , llst);

   if (NULL != _alist)
   {// GDS to TDT conversion
      // ... for every listed layer
      for ( WordList::const_iterator CL = llst.begin(); CL != llst.end(); CL++ )
      {
         // if the layer is listed among the GDS used layers
         if ( _alist->end() != _alist->find(*CL) )
         {
            if (wxT('*') == type_exp)
            { // for all data types
            // get all GDS data types for the current layer and copy them
            // to the map
               for ( WordSet::const_iterator CT = (*_alist)[*CL].begin(); CT != (*_alist)[*CL].end(); CT++)
                  _theMap[*CL].insert(std::make_pair(*CT, tdtLay));
            }
            else
            {// for particular (listed) data type
               WordList dtlst;
               getList( type_exp , dtlst);
               for ( WordList::const_iterator CT = dtlst.begin(); CT != dtlst.end(); CT++)
                  _theMap[*CL].insert(std::make_pair(*CT, tdtLay));
            }
         }
         // else ignore the mapped GDS layer
      }
   }
   else
   {// TDT to GDS conversion
      if (1 < llst.size())
      {
         wxString wxmsg;
         wxmsg << wxT("Can't export to multiple layers. Expression \"")
               << lay_exp << wxT("\"")
               << wxT(" is coerced to a single layer ")
               << llst.front();
         std::string msg(wxmsg.mb_str(wxConvUTF8));
         tell_log(console::MT_ERROR,msg);
      }
      if (wxT('*') == type_exp)
      { // for all data types - same as data type = 0
         _theMap[tdtLay].insert(std::make_pair(0, llst.front()));
      }
      else
      {// for particular (listed) data type
         WordList dtlst;
         getList( type_exp , dtlst);
         if (1 < dtlst.size())
         {
            wxString wxmsg;
            wxmsg << wxT("Can't export to multiple types. Expression \"")
                  << type_exp << wxT("\"")
                  << wxT(" for layer ") << tdtLay
                  << wxT(" is coerced to a single data type ")
                  << dtlst.front();
            std::string msg(wxmsg.mb_str(wxConvUTF8));
            tell_log(console::MT_ERROR,msg);
         }
         _theMap[tdtLay].insert(std::make_pair(dtlst.front(), llst.front()));
      }
   }

   return true;
}

bool LayerMapExt::separateQuickLists(wxString exp, wxString& lay_exp, wxString& type_exp)
{
   const wxString tmplLayNumbers    = wxT("[[:digit:]\\,\\-]*");
   const wxString tmplTypeNumbers   = wxT("[[:digit:]\\,\\-]*|\\*");


   wxRegEx src_tmpl(tmplLayNumbers+wxT("\\;")+tmplTypeNumbers); VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp))
   {
      wxString wxmsg;
      wxmsg << wxT("Can't make sense from the string \"") << exp << wxT("\"");
      std::string msg(wxmsg.mb_str(wxConvUTF8));
      tell_log(console::MT_ERROR,msg);
      return false;
   }
   //separate the layer expression from data type expression
   src_tmpl.Compile(tmplLayNumbers+wxT("\\;")); VERIFY(src_tmpl.IsValid());
   src_tmpl.Matches(exp);
   lay_exp = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   type_exp = exp;
   // we need to remove the ';' separator that left in the lay_exp
   src_tmpl.Compile(wxT("\\;")); VERIFY(src_tmpl.IsValid());
   src_tmpl.Matches(exp);
   src_tmpl.ReplaceFirst(&lay_exp,wxT(""));
   return true;
}

void LayerMapExt::patternNormalize(wxString& str)
{
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continuous spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before separators
   VERIFY(regex.Compile(wxT("([[:space:]])([\\-\\;\\,])")));
   regex.ReplaceAll(&str,wxT("\\2"));
   // remove spaces after separators
   VERIFY(regex.Compile(wxT("([\\-\\;\\,])([[:space:]])")));
   regex.ReplaceAll(&str,wxT("\\1"));

}

void LayerMapExt::getList(wxString exp, WordList& data)
{
   wxRegEx number_tmpl(wxT("[[:digit:]]*"));
   wxRegEx separ_tmpl(wxT("[\\,\\-]{1,1}"));
   unsigned long conversion;
   bool last_was_separator = true;
   char separator = ',';
   VERIFY(number_tmpl.IsValid());
   VERIFY(separ_tmpl.IsValid());

   do
   {
      if (last_was_separator)
      {
         number_tmpl.Matches(exp);
         number_tmpl.GetMatch(exp).ToULong(&conversion);
         number_tmpl.ReplaceFirst(&exp,wxT(""));
         if (',' == separator)
            data.push_back((word)conversion);
         else
         {
            for (word numi = data.back() + 1; numi <= conversion; numi++)
               data.push_back(numi);
         }
      }
      else
      {
         separ_tmpl.Matches(exp);
         if      (wxT("-") == separ_tmpl.GetMatch(exp))
            separator = '-';
         else if (wxT(",") == separ_tmpl.GetMatch(exp))
            separator = ',';
         else assert(false);
         separ_tmpl.ReplaceFirst(&exp,wxT(""));
      }
      last_was_separator = !last_was_separator;
   } while (!exp.IsEmpty());

}

bool LayerMapExt::getTdtLay(word& tdtlay, word gdslay, word gdstype) const
{
   assert(_import); // If you hit this - see the comment in the class declaration
   // All that this function is doing is:
   // tdtlay = _theMap[gdslay][gdstype]
   // A number of protections are in place though as well as const_cast
   tdtlay = gdslay; // the default value
   if (_theMap.end()       == _theMap.find(gdslay)       ) return false;
   GlMap::const_iterator glmap = _theMap.find(gdslay);
   if (glmap->second.end() == glmap->second.find(gdstype)) return false;
   GdtTdtMap::const_iterator tltype = glmap->second.find(gdstype);
   tdtlay = tltype->second;
   return true;
}

bool LayerMapExt::getExtLayType(word& gdslay, word& gdstype, word tdtlay) const
{
   assert(!_import); // If you hit this - see the comment in the class declaration
   gdslay  = tdtlay; // the default value
   gdstype = 0;
   if (_theMap.end()       == _theMap.find(tdtlay)       ) return false;
   GlMap::const_iterator glmap = _theMap.find(tdtlay);
   if (1 != glmap->second.size())   return false;
   gdstype =  glmap->second.begin()->first;
   gdslay =   glmap->second.begin()->second;
   return true;
}

USMap* LayerMapExt::generateAMap()
{
   USMap* wMap = new USMap();
   if (_import)
   {
      for (GlMap::const_iterator CTL = _theMap.begin(); CTL != _theMap.end(); CTL++)
      {
         for (GdtTdtMap::const_iterator CGT = CTL->second.begin(); CGT != CTL->second.end(); CGT++)
         {
            std::ostringstream lay_type;
            lay_type << CTL->first << ";" << CGT->first;
            (*wMap)[CGT->second] = lay_type.str();
         }
      }
   }
   else
   {
      for (GlMap::const_iterator CTL = _theMap.begin(); CTL != _theMap.end(); CTL++)
      {
         for (GdtTdtMap::const_iterator CGT = CTL->second.begin(); CGT != CTL->second.end(); CGT++)
         {
            std::ostringstream lay_type;
            lay_type << CGT->second << ";" << CGT->first;
            (*wMap)[CTL->first] = lay_type.str();
         }
      }
   }
   return wMap;
}

USMap* LayerMapExt::updateMap(USMap* update, bool import)
{
   assert(_import == import);
   // first generate the output from the current map
   USMap* wMap = generateAMap();
   for (USMap::const_iterator CE = update->begin(); CE != update->end(); CE++)
   {
      // the idea behind this parsing is simply to check the syntax and
      // to protect ourself from saving rubbish
      wxString exp(CE->second.c_str(), wxConvUTF8);
      patternNormalize(exp);
      wxString lay_exp;
      wxString type_exp;
      if (separateQuickLists(exp, lay_exp, type_exp))
      {
         (*wMap)[CE->first] = CE->second;
      }
      else
      {
         wxString wxmsg;
         wxmsg << wxT("Can't make sence from the input string for layer ") << CE->first;
         std::string msg(wxmsg.mb_str(wxConvUTF8));
         tell_log(console::MT_ERROR,msg);
      }
   }
   return wMap;
}

//=============================================================================
LayerMapCif::LayerMapCif(const USMap& inMap)
{
   for (USMap::const_iterator CI = inMap.begin(); CI != inMap.end(); CI++ )
   {
      _theImap[CI->second] = CI->first;
      _theEmap[CI->first] = CI->second;
   }
}

bool LayerMapCif::getTdtLay(word& tdtLay, std::string cifLay)
{
   if (_theImap.end() != _theImap.find(cifLay))
   {
      tdtLay = _theImap[cifLay];
      return true;
   }
   return false;
}

bool LayerMapCif::getCifLay(std::string& cifLay, word tdtLay)
{
   if (_theEmap.end() != _theEmap.find(tdtLay))
   {
      cifLay = _theEmap[tdtLay];
      return true;
   }
   return false;
}

USMap* LayerMapCif::updateMap(USMap* update)
{
   for (USMap::const_iterator CMI = update->begin(); CMI != update->end(); CMI++)
   {
      _theEmap[CMI->first] = CMI->second;
   }

   for (USMap::const_iterator CME = _theEmap.begin(); CME != _theEmap.end(); CME++)
   {
      _theImap[CME->second] = CME->first;
   }
   return DEBUG_NEW USMap(_theEmap);
}

USMap* LayerMapCif::updateMap(SIMap* update)
{
   for (SIMap::const_iterator CMI = update->begin(); CMI != update->end(); CMI++)
   {
      _theImap[CMI->first] = CMI->second;
   }

   for (SIMap::const_iterator CME = _theImap.begin(); CME != _theImap.end(); CME++)
   {
      _theEmap[CME->second] = CME->first;
   }
   return DEBUG_NEW USMap(_theEmap);
}

//=============================================================================
//
// class HiResTimer (Profiling timer for debugging purposes)
//
#ifdef TIME_PROFILING
HiResTimer::HiResTimer()
{
#ifdef WIN32
   // Get system frequency (number of ticks per second) of timer
   if (!QueryPerformanceFrequency(&_freq) || !QueryPerformanceCounter(&_inittime))
   {
      tell_log(console::MT_INFO,"Problem with timer");
   }
#else
   gettimeofday(&_start_time, NULL);
#endif
}

void HiResTimer::report(std::string message)
{
   char time_message[256];
#ifdef WIN32
   LARGE_INTEGER curtime;
   if (!QueryPerformanceCounter(&curtime))
      return ;
  // Convert number of ticks to milliseconds
   int millisec = (curtime.QuadPart-_inittime.QuadPart) / (_freq.QuadPart / 1000);
   int sec = millisec / 1000;
   millisec = millisec - sec * 1000;
   sprintf (time_message, "%s:   %i sec. %06i msec.",message.c_str(), sec, millisec);

#else

   gettimeofday(&_end_time, NULL);
   timeval result;
   result.tv_sec = _end_time.tv_sec - _start_time.tv_sec;
   result.tv_usec = _end_time.tv_usec - _start_time.tv_usec;
   if (result.tv_usec < 0)
   {
      result.tv_sec -= 1;
      result.tv_usec += 1000000;
   }
   sprintf (time_message, "%s:   %li sec. %06li msec.",message.c_str(), result.tv_sec, result.tv_usec);
#endif
   tell_log(console::MT_INFO,time_message);
}
#endif //TIME_PROFILING

