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
//           $URL: http://perun/tpd_svn/trunk/toped_wx/tpd_common/outbox.cpp $
//  Creation date: Mon May 10 23:12:15 BST 2004
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision: 172 $
//          $Date: 2005-06-29 01:12:09 +0100 (Wed, 29 Jun 2005) $
//        $Author: skr $
//===========================================================================
#include <string>
#include <wx/log.h>
#include "outbox.h"
//const modificator is not accepted from the compiler for some reason.
//Actually the problem is with the extern declarations in other files
//const wxEventType wxEVT_LOG_ERRMESSAGE = wxNewEventType();
extern const wxEventType      wxEVT_LOG_ERRMESSAGE;

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
      default:assert(false);
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
