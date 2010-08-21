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
//#include <sys/resource.h>
#include "ttt.h"

#define EVT_TECUSTOM_COMMAND(cmd, id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        cmd, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

//#define RENDER_PROFILING
//#define GDSCONVERT_PROFILING

#ifdef RENDER_PROFILING
#define TIME_PROFILING
#endif
#ifdef GDSCONVERT_PROFILING
#define TIME_PROFILING
#endif

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
      TSTS_THREADON     ,
      TSTS_THREADWAIT   ,
      TSTS_THREADOFF    ,
      TSTS_RENDERON     ,
      TSTS_RENDEROFF    ,
      TSTS_PRGRSBARON   ,
      TSTS_PROGRESS     ,
      TSTS_PRGRSBAROFF
   }TOPEDSTATUS_TYPE;

   typedef enum {
      FT_FUNCTION_ADD  ,
      FT_FUNCTION_SORT
   } FUNCTION_BROWSER_TYPE;

/* Maybe the only place to describe the parameter dictating the behaviour during user input:
   op_dwire  >  0 -> wire where input type is the width. List of points expected.
                     Rubber band wire will be shown.
   op_dbox   =  0 -> box. Two points expected, A rubberband box will be shown
   op_dpoly  = -1 -> polygon. List of points expected. Rubber band polygon will
                     be shown
   op_move   = -2 -> move. Two points expected. Selected and partially selected
                     objects will be moved on the screen with the marker
   op_copy   = -3 -> copy. Two points expected. Fully selected shapes will be
                     moved on the screen with the marker.
   op_flipX  = -4 -> flipX. One point expected only. The eventual new location of the
                     fully selected shapes will be shown
   op_flipY  = -5 -> flipY. One point expected only. The eventual new location of the
                     fully selected shapes will be shown
   op_rotate = -6 -> rotate. One point expected. The eventual new location of the
                     fully selected shapes will be shown
   op_line   = -7 -> line. Two points expected. Used for rulers. It is dynamically
                     rolled
   op_point  = -8 -> point. One point expected. No graphical effects
   op_cbind  = -9 -> cell bind. One point expected. The possible reference location is following
                     the cursor position
   op_cbind  =-10 -> cell array bind. One point expected. The possible reference location is following
                     the cursor position
   op_tbind  =-11 -> text bind. One point expected. The possible text location is following
                     the cursor position
*/
   typedef enum
   {
      op_none     = -12 ,
      op_tbind          ,
      op_abind          ,
      op_cbind          ,
      op_point          ,
      op_line           ,
      op_rotate         ,
      op_flipY          ,
      op_flipX          ,
      op_copy           ,
      op_move           ,
      op_dpoly          ,
      op_dbox           ,
      op_dwire
   } ACTIVE_OP;

   //===========================================================================
   class ted_log : public wxTextCtrl  {
   public:
                        ted_log(wxWindow *parent, wxWindowID);
      void              OnLOGMessage(wxCommandEvent&);
   private:
      wxString          cmd_mark;
      wxString          gui_mark;
      wxString          rply_mark;
      DECLARE_EVENT_TABLE();
   };

   //===========================================================================
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

   //===========================================================================
   class TopedStatus : public wxStatusBar {
   public:
                           TopedStatus(wxWindow*);
      virtual             ~TopedStatus(){};

      // event handlers
      void                 OnTopedStatus(wxCommandEvent& evt);
      void                 OnThreadON(wxString);
      void                 OnThreadWait();
      void                 OnThreadOFF();
      void                 OnRenderON();
      void                 OnRenderOFF();
      void                 OnSize(wxSizeEvent& event);
      void                 OnInitGauge(long int);
      void                 OnGaugeRun(long int);
      void                 OnCloseGauge();
   private:
      wxStaticBitmap*      _dbLamp;
      wxStaticBitmap*      _rndrLamp;
      wxGauge*             _progress;
      real                 _progressAdj;
      DECLARE_EVENT_TABLE();
   };
}

//===========================================================================
void tell_log(console::LOG_TYPE, const char* = NULL);
void tell_log(console::LOG_TYPE, const std::string&);
void tell_log(console::LOG_TYPE, const wxString&);

class TpdPost {
   public:
                  TpdPost(wxWindow*);
      static void toped_status(console::TOPEDSTATUS_TYPE);
      static void toped_status(console::TOPEDSTATUS_TYPE, long int);
      static void toped_status(console::TOPEDSTATUS_TYPE, std::string);
      static void toped_status(console::TOPEDSTATUS_TYPE, wxString);
      static void render_status(bool);
      static void refreshTDTtab(bool, bool);
      static void addGDStab(bool);
      static void addCIFtab(bool);
      static void addOAStab(bool);
      static void addDRCtab();
      static void clearGDStab();
      static void clearCIFtab();
      static void clearOAStab();
      static void clearDRCtab();
      static void layer_status(int, const word, const bool);
      static void layer_add(const std::string, const word);
      static void layer_default(const word, const word);
      static void layers_state(const std::string& name, bool add);
      static void resetTDTtab(const std::string);
      static void celltree_open(const std::string);
      static void celltree_highlight(const std::string);
      static void treeAddMember(const char*, const char*, int action = 0);
      static void treeRemoveMember(const char*, const char*, int orphan);
      static void parseCommand(const wxString);
      static void tellFnAdd(const std::string, void*);
      static void tellFnSort();
      static void quitApp(bool);

   private:
      static wxWindow* _statusBar;
      static wxWindow* _topBrowsers;
      static wxWindow* _layBrowser;
      static wxWindow* _cllBrowser;
      static wxWindow* _cmdLine;
      static wxWindow* _tllFuncList;
      static wxWindow* _mainWindow;
};

//===========================================================================
class  TpdTime
{
   public:
      TpdTime() : _stdCTime(0), _status(false) {};
      TpdTime(time_t stdCTime) : _stdCTime(stdCTime), _status(true){};
      TpdTime(tm&);
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

class EXPTNactive_CIF : public EXPTN
{
   public:
      EXPTNactive_CIF();
};

class EXPTNactive_OASIS : public EXPTN
{
   public:
      EXPTNactive_OASIS();
};

class EXPTNreadTDT : public EXPTN
{
   public:
      EXPTNreadTDT(std::string);
};

class EXPTNreadGDS : public EXPTN
{
   public:
      EXPTNreadGDS(std::string);
};

class EXPTNreadOASIS : public EXPTN
{
   public:
      EXPTNreadOASIS(std::string);
};

class EXPTNpolyCross : public EXPTN
{
   public:
      EXPTNpolyCross(std::string);
};

class EXPTNtell_parser : public EXPTN
{
   public:
      EXPTNtell_parser(std::string);
};

class EXPTNcif_parser : public EXPTN
{
   public:
      EXPTNcif_parser(std::string);
};

class EXPTNdrc_reader : public EXPTN
{
   public:
      EXPTNdrc_reader(std::string);
};

bool        expandFileName(std::string&);
//std::string getFileNameOnly(std::string);
//Convert string from UTF8 to wxConvFile
std::string convertString(const std::string &str);
int wxCALLBACK wxListCompareFunction(long, long, long);


/** The LayerMapExt is used for GDS/OASIS - TDT layer correspondence in both directions.
      - If the class is constructed with ExtLayers == NULL, then _import will be
        set to false and only the getExtLayType() method should be used.
      - If the class is constructed with ExtLayers != NULL, then _import will be
        set to true and only the getTdtLay() method should be used

   To ensure this policy some asserts are in place. Don't remove them!
 */
class LayerMapExt {
   public:
                           LayerMapExt(const USMap&, ExtLayers*);
                          ~LayerMapExt();
      bool                 getTdtLay(word&, word, word) const;
      bool                 getExtLayType(word&, word&, word) const;
      bool                 status() {return _status;}
      USMap*               updateMap(USMap*, bool);
   private:
      typedef std::map< word, word  >     GdtTdtMap;
      typedef std::map< word, GdtTdtMap>  GlMap;
      bool                 parseLayTypeString(wxString, word);
      void                 patternNormalize(wxString&);
      void                 getList(wxString, WordList&);
      bool                 separateQuickLists(wxString, wxString&, wxString&);
      USMap*               generateAMap();
      GlMap                _theMap;
      bool                 _status;
      bool                 _import;
      ExtLayers*           _alist; // all available GDS layers with their data types
};

class LayerMapCif {
   public:
                           LayerMapCif(const USMap&);
      bool                 getTdtLay(word&, std::string);
      bool                 getCifLay(std::string&, word);
      USMap*               updateMap(USMap*);
      USMap*               updateMap(SIMap*);
   private:
      USMap                _theEmap;
      SIMap                _theImap;
};

#ifdef TIME_PROFILING
class HiResTimer {
   public:
      HiResTimer();
      void           report(std::string);
   private:
      timeval        _start_time;
      timeval        _end_time;
#ifdef WIN32
      // System frequency of timer for Windows.
      LARGE_INTEGER  _freq;
      LARGE_INTEGER  _inittime;
#endif
};
#endif //TIME_PROFILING

//class OSResourceMeter {
//public:
//                        OSResourceMeter();
//   void                 report(std::string);
//private:
//   struct rusage        _start_res;
//   struct rusage        _end_res;
//};
#endif
