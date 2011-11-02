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
//        Created: Sun Mar 17 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Toped prompt
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
#include "outbox.h"

namespace console {

   typedef std::list<std::string>   stringList;
   const wxString real_tmpl      = wxT("[-+]?([[:digit:]]+(\\.[[:digit:]]*)?|(\\.[[:digit:]]+))");
   const wxString bool_tmpl      = wxT("true|false");
   const wxString point_tmpl     = wxT("\\{")+ real_tmpl+wxT(",")+ real_tmpl+wxT("\\}");
   const wxString box_tmpl       = wxT("\\{")+point_tmpl+wxT(",")+point_tmpl+wxT("\\}");
   const wxString bind_tmpl      = wxT("\\{")+point_tmpl+wxT(",")+real_tmpl+wxT(",")+bool_tmpl+wxT(",")+real_tmpl+wxT("\\}");
   const wxString pointlist_tmpl = wxT("\\{")+point_tmpl+wxT("(,")+point_tmpl+wxT("){1,}\\}");

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
      bool                    getBind();
      telldata::operandSTACK *client_stack;
      telldata::typeID         _wait4type;
      wxString                  exp;
   };

   class parse_thread : public wxThread
   {
   public:
                              parse_thread(wxWindow* canvas_wnd, wxThreadKind kind=wxTHREAD_DETACHED);
      void                    setCommand(const wxString& str) {_command = str;}
      wxCondition*            mutexCondition()                { return _mutexCondition;}
      wxMutex                 _mutex;
   protected:
      void*                   Entry();
      void                    OnExit();
      wxString                _command;
      wxWindow*               _canvas_wnd;
      wxCondition*            _mutexCondition;
   };

   class TllCmdLine : public wxEvtHandler {
   public:
                              TllCmdLine(wxWindow *canvas);
      virtual                ~TllCmdLine();
      void                    parseCommand(wxString, bool thread=true);
      void                    stopParserThread();
      void                    getCommand(bool);
      bool                    findTellFile(const char*, std::string&);
      virtual void            waitGUInput(telldata::operandSTACK*,console::ACTIVE_OP, const CTM&) = 0;
      virtual void            getGUInput(bool from_keyboard) = 0;
      virtual void            waitExternal(wxString) = 0;
      bool                    mouseIN_OK() const            {return _mouseIN_OK;};
      word                    numpoints() const             {return _numpoints;}
      const char*             lastCommand() const           {return _cmd_history.rbegin()->c_str();}
      void                    set_canvas_invalid(bool val)  { _canvas_invalid = val;}
      bool                    canvas_invalid()              {return _canvas_invalid;}
      void                    setExitRequest(bool val)      { _exitRequested = val;}
      void                    setExitAproved()              { _exitAproved = true;}
      bool                    exitRequested() const         { return _exitRequested;}
      bool                    exitAproved() const           { return _exitAproved;}
      bool                    cmdHistoryExists() const      {return !_cmd_history.empty();}
      void                    addTllIncludePath(wxString path){ _tllIncludePath.Add(path);}
      void                    addTllEnvList(wxString pvar)  { _tllIncludePath.AddEnvList(pvar);}
      wxCondition*            _threadWaits4;
      miniParser*             _puc; // parse user coordinates
   protected:
      virtual wxString        getString() = 0;
      virtual void            setString(const wxString&) = 0;
      virtual void            clearString() = 0;
      void                    runTellCommand(const wxString&);
      void                    mouseLB(const telldata::TtPnt& p);
      void                    mouseRB();
      void                    cancelLastPoint();
      word                    _numpoints;
      CTM                     _translation;
      CTM                     _initrans;
      wxString                _guinput;
      stringList              _cmd_history;
      stringList::const_iterator _history_position;
      bool                    _execExternal;
      bool                    _mouseIN_OK;
   private:
      void                    spawnTellThread(/*wxString*/);
      void                    onGetCommand(wxCommandEvent& WXUNUSED(event));
      wxWindow*               _canvas;
      bool                    _canvas_invalid;
      bool                    _exitRequested;
	  bool                    _exitAproved;
      wxPathList              _tllIncludePath;
      parse_thread*           _tellThread;
   };

   class TedCmdLine: public TllCmdLine {
   public:
                              TedCmdLine(wxWindow*, wxTextCtrl*);
      virtual void            waitGUInput(telldata::operandSTACK*,console::ACTIVE_OP, const CTM&);
      virtual void            getGUInput(bool from_keyboard);
      virtual void            waitExternal(wxString);
      void                    onParseCommand(wxCommandEvent&);
      void                    onGetCommand(wxCommandEvent& WXUNUSED(event));
      void                    onKeyUP(wxKeyEvent&);
      wxTextCtrl*             getWidget()                 {return _cmdLineWnd;}
   protected:
      virtual wxString        getString();
      virtual void            setString(const wxString&);
      virtual void            clearString();
   private:
      void                    onGUInput(wxCommandEvent&);
      void                    onExternalDone(wxCommandEvent&);
      wxTextCtrl*             _cmdLineWnd;
   };

   class TllCCmdLine: public TllCmdLine {
   public:
                              TllCCmdLine();
      virtual void            waitGUInput(telldata::operandSTACK*,console::ACTIVE_OP, const CTM&);
      virtual void            getGUInput(bool from_keyboard);
      virtual void            waitExternal(wxString);
   protected:
      virtual wxString        getString();
      virtual void            setString(const wxString&);
      virtual void            clearString();
   private:
      wxString                _currentCommand;
   };

}
#endif
