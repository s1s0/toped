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
//   This file is a part of Toped project (C) 2001-2006 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Mon May 10 23:13:03 BST 2004
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
#ifndef OUTBOX_H_INCLUDED
#define OUTBOX_H_INCLUDED

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <iostream>
#include <list>

#define EVT_TECUSTOM_COMMAND(cmd, id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        cmd, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
namespace console {
   typedef enum {
      MT_INFO = wxLOG_User + 1,
      MT_ERROR,
      MT_COMMAND,
      MT_GUIPROMPT,
      MT_GUIINPUT,
      MT_WARNING,
      MT_CELLNAME,
      MT_DESIGNNAME,
      MT_EOL
   } LOG_TYPE;

   typedef enum {
      FT_FUNCTION_ADD  ,
      FT_FUNCTION_SORT
   } FUNCTION_BROWSER_TYPE;
   
   class ted_log : public wxTextCtrl  {
   public: 
                        ted_log(wxWindow *parent);
      void              OnLOGMessage(wxCommandEvent&);
   private:
      wxString          cmd_mark;
      wxString          gui_mark;
      wxString          rply_mark;
      DECLARE_EVENT_TABLE();
   };

   class ted_log_ctrl : public wxLogTextCtrl {
   public:
      ted_log_ctrl(wxTextCtrl *pTextCtrl) : wxLogTextCtrl(pTextCtrl),
                                                         _tellLOGW(pTextCtrl){};
   private:
      void         DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp);
      wxTextCtrl*  _tellLOGW;
   };

   //===========================================================================
   class TELLFuncList : public wxListView
   {
      public:
         TELLFuncList(wxWindow* parent, wxWindowID id = -1,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxLC_REPORT | wxLC_HRULES);
         virtual             ~TELLFuncList();
         void                 addFunc(wxString, void*);
         void                 OnCommand(wxCommandEvent&);
         DECLARE_EVENT_TABLE();
      protected:
         typedef std::list<std::string> ArgList;
   };

   void TellFnAdd(const std::string, void*);
   void TellFnSort();

}

   void tell_log(console::LOG_TYPE, const char* = NULL);
   void tell_log(console::LOG_TYPE, const std::string&);

class  TpdTime
{
   public:
      TpdTime(time_t stdCTime) : _stdCTime(stdCTime){};
      TpdTime(std::string);
      std::string operator () ();
      bool operator == (TpdTime& arg1) const {return _stdCTime == arg1._stdCTime;}
      bool operator != (TpdTime& arg1) const {return _stdCTime != arg1._stdCTime;}
      time_t            stdCTime() const {return _stdCTime;}
      bool              status() const {return _status;}
   protected:
      void              patternNormalize(wxString& str);
      bool              getStdCTime(wxString& exp);
      time_t            _stdCTime;
      bool              _status;
};

class EXPTN {};
 
class EXPTNactive_cell : public EXPTN
{
   public:
      EXPTNactive_cell();
};

class EXPTNactive_DB : public EXPTN
{
   public:
      EXPTNactive_DB();
};

class EXPTNactive_GDS : public EXPTN
{
   public:
      EXPTNactive_GDS();
};

class EXPTNreadTDT : public EXPTN
{
   public:
      EXPTNreadTDT(std::string);
};

class EXPTNpolyCross : public EXPTN
{
   public:
      EXPTNpolyCross(std::string);
};

bool expandFileName(std::string&);

class TopedApp : public wxApp
{
   public:
      virtual bool   OnInit();
      virtual int    OnExit();
      bool           ignoreOnRecovery() { return _ignoreOnRecovery;}
      void           set_ignoreOnRecovery(bool ior) {_ignoreOnRecovery = ior;}
   protected:
      bool           GetLogFileName();
      bool           CheckCrashLog();
      void           GetLogDir();
      void           FinishSessionLog();
      void           SaveIgnoredCrashLog();
      wxString       logFileName;
      wxString       tpdLogDir;
      bool           _ignoreOnRecovery;
};

DECLARE_APP(TopedApp)

#endif
