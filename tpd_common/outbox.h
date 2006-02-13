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
//  Creation date: Mon May 10 23:13:03 BST 2004
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2004 by Svilen Krustev
//    Description: wxWidget version
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
#include <iostream>

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
      FT_FUNCTION_ADD
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
   class TELLFuncList : public wxListCtrl
   {
      public:
         TELLFuncList(wxWindow* parent, wxWindowID id = -1,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxLC_REPORT | wxLC_HRULES);
         virtual             ~TELLFuncList();
         void                 addFunc(wxString, wxString);
         void                 OnCommand(wxCommandEvent&);
         DECLARE_EVENT_TABLE();
   };

   void TellFnAdd(const std::string, const std::string);

}
   void tell_log(console::LOG_TYPE, const char* = NULL);

class  TpdTime
{
   public:
      TpdTime(time_t stdCTime) : _stdCTime(stdCTime){};
      TpdTime(std::string);
      std::string operator () ();
      time_t            stdCTime() const {return _stdCTime;}
   protected:
      void              patternNormalize(wxString& str);
      bool              getStdCTime(wxString& exp);
      time_t            _stdCTime;
};

class EXPTN {};
 
class EXPTNactive_cell : public EXPTN {
   public:
      EXPTNactive_cell();
};

class EXPTNactive_DB : public EXPTN  {
   public:
      EXPTNactive_DB();
};

class EXPTNactive_GDS : public EXPTN  {
   public:
      EXPTNactive_GDS();
};

/*class EXPTNexisting_GDS : public EXPTN  {
   public:
      EXPTNmanyTopCellsGDS();
};
*/
class EXPTNreadTDT : public EXPTN {
   public:
      EXPTNreadTDT();
};

#endif
