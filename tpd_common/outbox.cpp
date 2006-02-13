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
//  Creation date: Mon May 10 23:12:15 BST 2004
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
#include <string>
#include <wx/log.h>
#include <wx/regex.h>
#include "outbox.h"

extern const wxEventType      wxEVT_LOG_ERRMESSAGE;
extern const wxEventType      wxEVT_FUNC_BROWSER;
extern console::TELLFuncList* CmdList;
//==============================================================================
// The ted_log event table
BEGIN_EVENT_TABLE( console::ted_log, wxTextCtrl )
   EVT_TECUSTOM_COMMAND(wxEVT_LOG_ERRMESSAGE, -1, ted_log::OnLOGMessage)
END_EVENT_TABLE()

console::ted_log::ted_log(wxWindow *parent): wxTextCtrl( parent, -1, "",
   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER) {
   cmd_mark = "=> ";
   gui_mark = ">> ";
   rply_mark = "<= ";
}

void console::ted_log::OnLOGMessage(wxCommandEvent& evt) {
   wxColour logColour;
   long int startPos = GetLastPosition();
   switch (evt.GetInt()) {
      case    MT_INFO:
         *this << rply_mark << evt.GetString() <<"\n";
         logColour = *wxBLUE;
//         logColour = *wxGREEN;
         break;
      case   MT_ERROR:
         *this << rply_mark << evt.GetString() <<"\n";
         logColour = *wxRED;
         break;
      case MT_COMMAND:
         *this << cmd_mark << evt.GetString() <<"\n";
         break;
      case MT_GUIPROMPT:
         *this << gui_mark; break;
      case MT_GUIINPUT:
         *this << evt.GetString();break;
      case MT_EOL:
         *this << "\n"; break;
      case MT_WARNING:
         *this << rply_mark << evt.GetString() <<"\n";
         logColour = *wxCYAN;
         break;
      case MT_CELLNAME:
         *this << rply_mark << " Cell " << evt.GetString() <<"\n";
         break;
      case MT_DESIGNNAME:
         *this << rply_mark << " Design " << evt.GetString() <<"\n";
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

void tell_log(console::LOG_TYPE lt, const char* msg) {
   wxLog::OnLog(lt, msg, time(NULL));
}

//==============================================================================
BEGIN_EVENT_TABLE( console::TELLFuncList, wxListCtrl )
   EVT_TECUSTOM_COMMAND(wxEVT_FUNC_BROWSER, -1, TELLFuncList::OnCommand)
END_EVENT_TABLE()

console::TELLFuncList::TELLFuncList(wxWindow *parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style) :
      wxListCtrl(parent, id, pos, size, style)
{
   InsertColumn(0, "type");
   InsertColumn(1, "name");
   InsertColumn(2, "arguments");
   SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
   SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
}


console::TELLFuncList::~TELLFuncList()
{
   DeleteAllItems();
}

void console::TELLFuncList::addFunc(wxString name, wxString arguments)
{
   wxListItem row;
#ifndef WIN32
   //???Under windows this string leads to crash toped with 
   //Debug: C:\wxWindows\src\msw\listctrl.cpp(1531):
   //assert "m_count==ListView_GetItemCount(GetHwnd())" failed: m_count should match ListView_GetItemCount 
   row.SetColumn(1);
#endif
   row.SetMask(wxLIST_MASK_TEXT);
   row.SetText(name.c_str());
   row.SetId(GetItemCount());

   long inum = InsertItem(row);
  
   SetColumnWidth(1, wxLIST_AUTOSIZE);

   row.SetColumn(2);
   row.SetMask(wxLIST_MASK_TEXT);
   row.SetText(arguments.c_str());
   SetItem(row);
   SetColumnWidth(2, wxLIST_AUTOSIZE);
}

void console::TELLFuncList::OnCommand(wxCommandEvent& event)
{
   int command = event.GetInt();
   switch (command) {
      case console::FT_FUNCTION_ADD:
      {
         wxString *args = (wxString*)event.GetClientData();
         FT_FUNCTION_ADD:addFunc(event.GetString(), *args);
         delete args;
         break;
      }
   default: assert(false);
   }
}

//=============================================================================
void console::TellFnAdd(const std::string name, const std::string arguments)
{
//   int* bt = new int(console::FT_FUNCTION_ADD);
   wxCommandEvent eventFUNCTION_ADD(wxEVT_FUNC_BROWSER);
   eventFUNCTION_ADD.SetString(name.c_str());
   wxString* args = new wxString(arguments.c_str());
   eventFUNCTION_ADD.SetClientData(args);
   eventFUNCTION_ADD.SetInt(FT_FUNCTION_ADD);
   wxPostEvent(CmdList, eventFUNCTION_ADD);
}

//=============================================================================
std::string TpdTime::operator () ()
{
   tm* broken_time = localtime(&_stdCTime);
   assert(broken_time != NULL);
   char* btm = new char[256];
   strftime(btm, 256, "%d-%m-%Y %T", broken_time);
   std::string info = btm;
   delete btm;
   return info;
}

TpdTime::TpdTime(std::string str_time)
{
   wxString wxstr_time(str_time.c_str());
   patternNormalize(wxstr_time);
   getStdCTime(wxstr_time);
}

void TpdTime::patternNormalize(wxString& str) {
   wxRegEx regex;
   // replace tabs with spaces
   assert(regex.Compile("\\t"));
   regex.ReplaceAll(&str," ");
   // remove continious spaces
   assert(regex.Compile("[[:space:]]{2,}"));
   regex.ReplaceAll(&str,"");
   //remove leading spaces
   assert(regex.Compile("^[[:space:]]"));
   regex.ReplaceAll(&str,"");
   // remove trailing spaces
   assert(regex.Compile("[[:space:]]$"));
   regex.ReplaceAll(&str,"");
   //remove spaces before separators
   assert(regex.Compile("([[:space:]])([\\-\\:])"));
   regex.ReplaceAll(&str,"\\2");
   // remove spaces after separators
   assert(regex.Compile("([\\-\\:])([[:space:]])"));
   regex.ReplaceAll(&str,"\\1");

}

bool TpdTime::getStdCTime(wxString& exp) {
   const wxString tmpl2digits      = "[[:digit:]]{2,2}";
   const wxString tmpl4digits      = "[[:digit:]]{4,4}";
   const wxString tmplDate         = tmpl2digits+"\\-"+tmpl2digits+"\\-"+tmpl4digits;
   const wxString tmplTime         = tmpl2digits+"\\:"+tmpl2digits+"\\:"+tmpl2digits;
   wxRegEx src_tmpl(tmplDate+"[[:space:]]"+tmplTime);
   assert(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   tm broken_time;
   // get the date
   assert(src_tmpl.Compile(tmpl2digits));
   src_tmpl.Matches(exp);
   broken_time.tm_mday = atoi(src_tmpl.GetMatch(exp).c_str());
   src_tmpl.ReplaceFirst(&exp,"");
   // get month
   src_tmpl.Matches(exp);
   broken_time.tm_mon = atoi(src_tmpl.GetMatch(exp).c_str()) - 1;
   src_tmpl.ReplaceFirst(&exp,"");
   // get year
   assert(src_tmpl.Compile(tmpl4digits));
   src_tmpl.Matches(exp);
   broken_time.tm_year = atoi(src_tmpl.GetMatch(exp).c_str()) - 1900;
   src_tmpl.ReplaceFirst(&exp,"");
   // now the time - first hour
   assert(src_tmpl.Compile(tmpl2digits));
   src_tmpl.Matches(exp);
   broken_time.tm_hour = atoi(src_tmpl.GetMatch(exp).c_str());
   src_tmpl.ReplaceFirst(&exp,"");
   // minutes
   src_tmpl.Matches(exp);
   broken_time.tm_min = atoi(src_tmpl.GetMatch(exp).c_str());
   src_tmpl.ReplaceFirst(&exp,"");
   // and seconds
   src_tmpl.Matches(exp);
   broken_time.tm_sec = atoi(src_tmpl.GetMatch(exp).c_str());
   //
   _stdCTime = mktime(&broken_time);
   return true;
}

//=============================================================================
EXPTNactive_cell::EXPTNactive_cell() {
   std::string news = "No active cell. Use opencell(\"<name>\") to select one";
   tell_log(console::MT_ERROR,news.c_str());
};

EXPTNactive_DB::EXPTNactive_DB() {
   std::string news = "No active database. Create or load one";
   tell_log(console::MT_ERROR,news.c_str());
};

EXPTNactive_GDS::EXPTNactive_GDS() {
   std::string news = "No GDS structure in memory. Parse first";
   tell_log(console::MT_ERROR,news.c_str());
};

/*EXPTNmanyTopCellsGDS::EXPTNmanyTopCellsGDS() {
   std::string news = "There are more then one top cells into GDS structure.\n Use advanced operations with this GDS";
   tell_log(console::MT_ERROR,news.c_str());
};
*/
EXPTNreadTDT::EXPTNreadTDT() {
   std::string news = "Error in TDT file";
   tell_log(console::MT_ERROR,news.c_str());
};
