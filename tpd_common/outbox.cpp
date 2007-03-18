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
#include <time.h>
#include <string>
#include <wx/log.h>
#include <wx/regex.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include "outbox.h"

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_STATUS , 10000)
    DECLARE_EVENT_TYPE(wxEVT_SETINGSMENU   , 10001)
    DECLARE_EVENT_TYPE(wxEVT_CMD_BROWSER   , 10002)
    DECLARE_EVENT_TYPE(wxEVT_LOG_ERRMESSAGE, 10003)
    DECLARE_EVENT_TYPE(wxEVT_MOUSE_ACCEL   , 10004)
    DECLARE_EVENT_TYPE(wxEVT_MOUSE_INPUT   , 10005)
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_ZOOM   , 10006)
    DECLARE_EVENT_TYPE(wxEVT_FUNC_BROWSER  , 10007)
    DECLARE_EVENT_TYPE(wxEVT_TPDSTATUS     , 10008)
    DECLARE_EVENT_TYPE(wxEVT_CANVAS_CURSOR , 10009)
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

console::TELLFuncList*           CmdList = NULL;
//==============================================================================
// The ted_log event table
BEGIN_EVENT_TABLE( console::ted_log, wxTextCtrl )
   EVT_TECUSTOM_COMMAND(wxEVT_LOG_ERRMESSAGE, -1, ted_log::OnLOGMessage)
END_EVENT_TABLE()

console::ted_log::ted_log(wxWindow *parent): wxTextCtrl( parent, -1, wxT(""),
   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER|wxTE_RICH)
{
   cmd_mark = wxT("=> ");
   gui_mark = wxT(">> ");
   rply_mark = wxT("<= ");
}

void console::ted_log::OnLOGMessage(wxCommandEvent& evt) {
   wxColour logColour;
   long int startPos = GetLastPosition();
   switch (evt.GetInt()) {
      case    MT_INFO:
         *this << rply_mark << evt.GetString() << wxT("\n");
         logColour = *wxBLUE;
//         logColour = *wxGREEN;
         break;
      case   MT_ERROR:
         *this << rply_mark << evt.GetString() << wxT("\n");
         logColour = *wxRED;
         break;
      case MT_COMMAND:
         *this << cmd_mark << evt.GetString() << wxT("\n");
         break;
      case MT_GUIPROMPT:
         *this << gui_mark; break;
      case MT_GUIINPUT:
         *this << evt.GetString();break;
      case MT_EOL:
         *this << wxT("\n"); break;
      case MT_WARNING:
         *this << rply_mark << evt.GetString() << wxT("\n");
         logColour = *wxCYAN;
         break;
      case MT_CELLNAME:
         *this << rply_mark << wxT(" Cell ") << evt.GetString() << wxT("\n");
         break;
      case MT_DESIGNNAME:
         *this << rply_mark << wxT(" Design ") << evt.GetString() << wxT("\n");
         break;
      default:
         assert(false);
   }   
   long int endPos = GetLastPosition();
   SetStyle(startPos,endPos,wxTextAttr(logColour));
}

void console::ted_log_ctrl::DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp) {
   if (level < wxLOG_User)
      wxLogTextCtrl::DoLog(level, msg, timestamp);
   else {
      wxCommandEvent eventLOG(wxEVT_LOG_ERRMESSAGE);
      eventLOG.SetString(msg);
      eventLOG.SetInt(level);
      wxPostEvent(_tellLOGW, eventLOG);
   }
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

//==============================================================================
static int wxCALLBACK wxListCompareFunction(long item1, long item2, long sortData)
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
void console::TellFnAdd(const std::string name, void* arguments)
{
   wxCommandEvent eventFUNCTION_ADD(wxEVT_FUNC_BROWSER);
   eventFUNCTION_ADD.SetString(wxString(name.c_str(), wxConvUTF8));
   eventFUNCTION_ADD.SetClientData(arguments);
   eventFUNCTION_ADD.SetInt(FT_FUNCTION_ADD);
   wxPostEvent(CmdList, eventFUNCTION_ADD);
}

void console::TellFnSort()
{
   wxCommandEvent eventFUNCTION_ADD(wxEVT_FUNC_BROWSER);
   eventFUNCTION_ADD.SetInt(FT_FUNCTION_SORT);
   wxPostEvent(CmdList, eventFUNCTION_ADD);
}
//=============================================================================
std::string TpdTime::operator () ()
{
   tm* broken_time = localtime(&_stdCTime);
   assert(broken_time != NULL);
   char* btm = new char[256];
   strftime(btm, 256, "%d-%m-%Y %X", broken_time);
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

void TpdTime::patternNormalize(wxString& str) {
   wxRegEx regex;
   // replace tabs with spaces
   assert(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   assert(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   assert(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   assert(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before separators
   assert(regex.Compile(wxT("([[:space:]])([\\-\\:])")));
   regex.ReplaceAll(&str,wxT("\\2"));
   // remove spaces after separators
   assert(regex.Compile(wxT("([\\-\\:])([[:space:]])")));
   regex.ReplaceAll(&str,wxT("\\1"));

}

bool TpdTime::getStdCTime(wxString& exp) {
   const wxString tmpl2digits      = wxT("[[:digit:]]{2,2}");
   const wxString tmpl4digits      = wxT("[[:digit:]]{4,4}");
   const wxString tmplDate         = tmpl2digits+wxT("\\-")+tmpl2digits+wxT("\\-")+tmpl4digits;
   const wxString tmplTime         = tmpl2digits+wxT("\\:")+tmpl2digits+wxT("\\:")+tmpl2digits;
   wxRegEx src_tmpl(tmplDate+wxT("[[:space:]]")+tmplTime);
   assert(src_tmpl.IsValid());
   long conversion;
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   tm broken_time;
   // get the date
   assert(src_tmpl.Compile(tmpl2digits));
   src_tmpl.Matches(exp);
   assert(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_mday = conversion;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // get month
   src_tmpl.Matches(exp);
   assert(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_mon = conversion - 1;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // get year
   assert(src_tmpl.Compile(tmpl4digits));
   src_tmpl.Matches(exp);
   assert(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_year = conversion - 1900;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // now the time - first hour
   assert(src_tmpl.Compile(tmpl2digits));
   src_tmpl.Matches(exp);
   assert(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_hour = conversion; 
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // minutes
   src_tmpl.Matches(exp);
   assert(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_min = conversion;
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   // and seconds
   src_tmpl.Matches(exp);
   assert(src_tmpl.GetMatch(exp).ToLong(&conversion));
   broken_time.tm_sec = conversion;
   //
   broken_time.tm_isdst = -1;
   _stdCTime = mktime(&broken_time);
   return true;
}

bool expandFileName( std::string& filename)
{
   wxFileName fName(wxString(filename.c_str(), wxConvUTF8));
   fName.Normalize();
   if (fName.IsOk())
   {
      wxString dirName = fName.GetFullPath();
      if (!dirName.Matches(wxT("*$*")))
      {
         filename = fName.GetFullPath().mb_str();
         return true;
      }
   }
   return false;
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

EXPTNreadTDT::EXPTNreadTDT(std::string info) {
   std::string news = "Error parsing TDT file =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

EXPTNpolyCross::EXPTNpolyCross(std::string info) {
   std::string news = "Internal error - polygon cross =>";
   news += info;
   tell_log(console::MT_ERROR,news);
};

