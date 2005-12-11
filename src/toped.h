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
//  Creation date: Thu May  6 21:56:03 BST 2004
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

#if !defined(TOPED_H_INCLUDED)
#define TOPED_H_INCLUDED

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/laywin.h>
 
#include "../tpd_parser/ted_prompt.h"
#include "layoutcanvas.h"
#include "tui.h"
#include "browsers.h"

namespace tui {
   //-----------------------------------------------------------------------------
   class CanvasStatus : public wxPanel {
   public:
           CanvasStatus(wxWindow*);
          ~CanvasStatus();
      void setXpos(wxString);
      void setYpos(wxString);
      void setdXpos(wxString);
      void setdYpos(wxString);
      void setSelected(wxString);
      void btn_abort_enable(bool state) {_abort->Enable(state);};
   private:
      wxStaticText*        X_pos;
      wxStaticText*        Y_pos;
      wxStaticText*        _dX;
      wxStaticText*        _dY;
      wxStaticText*        _selected;
      wxButton*            _abort;
   };

   //-----------------------------------------------------------------------------
   class TopedFrame : public wxFrame {
   public:
                              TopedFrame(const wxString& , const wxPoint& , 
                                                               const wxSize&);
                             ~TopedFrame();
      void                    OnSize(wxSizeEvent& event);
      void                    OnSashDrag(wxSashEvent& event);
      void                    OnPositionMessage(wxCommandEvent&);
      void                    OnQuit (wxCommandEvent&);
      void                    OnAbout(wxCommandEvent&);

      console::ted_log*       logwin()   const {return _cmdlog;};
      LayoutCanvas*           view()     const {return _laycanvas;};
      console::ted_cmd*       cmdline()  const {return _cmdline;};
      browsers::browserTAB*   browsers() const {return _browsers;};
   private:
      void                    initMenuBar();
      void                    initToolBar();
      void                    initView();
      console::ted_log*       _cmdlog;       // log window
      console::ted_cmd*       _cmdline;      // tell command input window
      LayoutCanvas*           _laycanvas;
      CanvasStatus*           _GLstatus;
      browsers::browserTAB*   _browsers;  // tell definitions browser
      //Menu stuff
      wxMenuBar*              menuBar;
      wxMenu*                 fileMenu;
      wxMenu*                 editMenu;
      wxMenu*                 viewMenu;
      wxMenu*                 cellMenu;
      wxMenu*                 drawMenu;
      wxMenu*                 selectMenu;
      wxMenu*                 settingsMenu;
      wxMenu*                 markerMenu;
      wxMenu*                 gdsMenu;
      wxMenu*                 helpMenu;
      // Sash layout stuff
      wxSashLayoutWindow*     mS_browsers;
      wxSashLayoutWindow*     mS_GLstatus;
      wxSashLayoutWindow*     mS_command;
      wxSashLayoutWindow*     mS_log;
      wxSashLayoutWindow*     mS_canvas;
      // Menu 
      void   OnNewDesign(wxCommandEvent&);
      void     OnTDTRead(wxCommandEvent&);
      void    OnTELLRead(wxCommandEvent&);
      void     OnGDSRead(wxCommandEvent&);
      void   OnGDSimport(wxCommandEvent&);
      void   OnGDSexportLIB(wxCommandEvent&);
      void   OnGDSexportTOP(wxCommandEvent&);
      void    OnGDSclose(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("gdsclose();");};
      void     OnTDTSave(wxCommandEvent&);
      void   OnTDTSaveAs(wxCommandEvent&);
      void        OnCopy(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("copy();");};
      void        OnMove(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("move();");};
      void      OnDelete(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("delete();");};
      void      OnRotate(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("rotate(getpoint(),90);");};
      void       OnFlipX(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("flipX(getpoint());");};
      void       OnFlipY(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("flipY(getpoint());");};
      void     OnPolyCut(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("polycut();");};
      void       OnMerge(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("merge();");};

      void     OnCellNew(wxCommandEvent&);
      void    OnCellOpen(wxCommandEvent&);
      void    OnCellPush(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("editpush(getpoint());");};
      void     OnCellPop(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("editpop();");};
      void     OnCellTop(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("edittop();");};
      void    OnCellPrev(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("editprev();");};
      void   OnCellRef_B(wxCommandEvent&);
      void   OnCellRef_M(wxCommandEvent&);
      void  OnCellARef_B(wxCommandEvent&);
      void   OnCellARef_M(wxCommandEvent&);
      void   OnCellGroup(wxCommandEvent&);
      void OnCellUngroup(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("ungroup();");};

      void        OnUndo(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("undo();");};
      void      OnzoomIn(wxCommandEvent& WXUNUSED(event));
      void     OnzoomOut(wxCommandEvent& WXUNUSED(event));
      void     OnpanLeft(wxCommandEvent& WXUNUSED(event));
      void    OnpanRight(wxCommandEvent& WXUNUSED(event));
      void       OnpanUp(wxCommandEvent& WXUNUSED(event));
      void     OnpanDown(wxCommandEvent& WXUNUSED(event));
      void     OnZoomAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("zoomall();");};
      void     OnDrawBox(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("addbox();");};
      void    OnDrawPoly(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("addpoly();");};
      void    OnDrawWire(wxCommandEvent&);
      void    OnDrawText(wxCommandEvent&);
      void    OnSelectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("select();");};
      void  OnUnselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("unselect();");};
      void   OnPselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("pselect();");};
      void OnPunselectIn(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("punselect();");};
      void OnUnselectAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("unselect_all();");};
      void   OnSelectAll(wxCommandEvent& WXUNUSED(event)) {_cmdline->parseCommand("select_all();");};
      //
      void        OnStep(wxCommandEvent&);
      void     OnAutopan(wxCommandEvent&);
      void       OnGrid0(wxCommandEvent&);
      void       OnGrid1(wxCommandEvent&);
      void       OnGrid2(wxCommandEvent&);
      void    OnCellMark(wxCommandEvent&);
      void    OnTextMark(wxCommandEvent&);
      void     OnMarker0(wxCommandEvent&);
      void    OnMarker45(wxCommandEvent&);
      void    OnMarker90(wxCommandEvent&);

      void  OnGridDefine(wxCommandEvent&);
      //
      void       OnAbort(wxCommandEvent&);
      void OnUpdateStatusLine(wxCommandEvent&);// {_GLstatus->setSelected(evt.GetString());};
      void  OnMouseAccel(wxCommandEvent&);
      // additional
      void    CellRef(wxString);
      void   CellARef(wxString);
      // The declaration of the associated event table
      DECLARE_EVENT_TABLE();
   };
}

#endif

