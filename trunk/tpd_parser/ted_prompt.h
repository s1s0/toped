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
//  Creation date: Sun Mar 17 2002
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

#ifndef TED_PROMPT_H
#define TED_PROMPT_H

#include <wx/textctrl.h>
#include <wx/thread.h>
#include <wx/string.h>
#include "tellyzer.h"
#include "../tpd_common/outbox.h"

namespace console {
   typedef std::list<std::string>   stringList;
   const wxString real_tmpl      = "[-+]?([[:digit:]]+(\\.[[:digit:]]*)?|(\\.[[:digit:]]+))";
   const wxString point_tmpl     = "\\{"+ real_tmpl+","+ real_tmpl+"\\}";
   const wxString box_tmpl       = "\\{"+point_tmpl+","+point_tmpl+"\\}";
   const wxString pointlist_tmpl = "\\{"+point_tmpl+"(,"+point_tmpl+"){1,}\\}";

   bool patternFound(const wxString templ,  wxString str);
   void patternNormalize(wxString& str);

   class miniParser {
   public:
                              miniParser(telldata::operandSTACK *,telldata::typeID);
      bool                    getGUInput(wxString expression);
      telldata::typeID        wait4type() { return _wait4type;};
   private:
      bool                    getPoint();
      bool                    getBox();
      bool                    getList();
      telldata::operandSTACK *client_stack;
      telldata::typeID         _wait4type;
      wxString                  exp;
   };

   class parse_thread : public wxThread {
   public:
      parse_thread(wxString& cmd, wxThreadKind kind=wxTHREAD_DETACHED):
                                    wxThread(kind),command(cmd){};
   protected:
      void*                   Entry();
      wxString                command;
   };


   class ted_cmd : public wxTextCtrl {
   public:
                              ted_cmd(wxWindow*);
      virtual                ~ted_cmd();
      void                    parseCommand(wxString, bool thread=true);
      void                    waitGUInput(telldata::operandSTACK*,telldata::typeID);
      void                    getGUInput(bool from_keyboard = true);
      wxCondition*            threadWaits4;
      miniParser*             puc; // parse user coordinates
      // event table handlers
      void                    getCommand(wxCommandEvent&);
      void                    getCommandA();
      void                    OnGUInput(wxCommandEvent&);
      bool                    mouseIN_OK() const {return _mouseIN_OK;};
      word                    numpoints() const {return _numpoints;}
      const char*             lastCommand() const {return _cmd_history.rbegin()->c_str();}
   private:
      void                    OnKeyUP(wxKeyEvent&);
      void                    cancelLastPoint();
      void                    mouseLB(const telldata::ttpnt& p);
      void                    mouseRB();
      word                    _numpoints;
      bool                    _mouseIN_OK;
      wxString                _guinput;
      stringList              _cmd_history;
      stringList::const_iterator _history_position;
      bool                    _thread; //flag fo detached thread
      DECLARE_EVENT_TABLE();
   };

   const int Evt_Mouse = 100;
}
#endif
