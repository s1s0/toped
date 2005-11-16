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
//  Creation date: Wed Dec 26 2001
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

#ifndef LAYOUTCANVAS_H
#define LAYOUTCANVAS_H

#include <string>
#include <wx/glcanvas.h>
#include <wx/cursor.h>
#include "../tpd_common/ttt.h"

//==============================================================================   
// WX window ID's
//==============================================================================   
#define ID_WIN_BROWSERS    100
#define ID_WIN_GLSTATUS    101
#define ID_WIN_COMMAND     102
#define ID_WIN_LOG         103
#define ID_WIN_CANVAS      104
#define ID_TPD_CANVAS      105
#define ID_TPD_LAYERS      106
#define ID_TPD_CELLTREE    107
#define ID_GDS_CELLTREE    108
//-----------------------------------------------------------------------------

namespace tui {
   
   typedef enum  {
      ZOOM_WINDOW  = 0    ,
      ZOOM_WINDOWM        ,
      ZOOM_IN             ,
      ZOOM_OUT            ,
      ZOOM_LEFT           ,
      ZOOM_RIGHT          ,
      ZOOM_UP             ,
      ZOOM_DOWN           ,
      ZOOM_EMPTY          ,
   } ZOOM_TYPE;
   
   typedef enum {
      POS_X = 0x01,
      POS_Y = 0x02,
      DEL_X = 0x10,
      DEL_Y = 0x20,
   } POSITION_TYPE;
   
   typedef enum {
      STS_SELECTED      ,
      STS_ABORTENABLE   ,
      STS_ABORTDISABLE  ,
      STS_GRID0_ON      ,
      STS_GRID0_OFF     ,
      STS_GRID1_ON      ,
      STS_GRID1_OFF     ,
      STS_GRID2_ON      ,
      STS_GRID2_OFF     ,
      STS_CELLMARK_ON   ,
      STS_CELLMARK_OFF  ,
      STS_TEXTMARK_ON   ,
      STS_TEXTMARK_OFF  ,
      STS_AUTOPAN_ON    ,
      STS_AUTOPAN_OFF   ,
      STS_ANGLE_0       ,
      STS_ANGLE_45      ,
      STS_ANGLE_90
   } STATUSLINE_TYPE;
   
   //=============================================================================
   class LayoutCanvas : public wxGLCanvas  {
   public: 
                     LayoutCanvas(wxWindow *parent, int* attribList);
                    ~LayoutCanvas();
   protected:
      void           OnpaintGL(wxPaintEvent& event);
      void           OnresizeGL(wxSizeEvent& event);
      void           OnEraseBackground(wxEraseEvent&) {};
      void           OnMouseMotion(wxMouseEvent&);
      void           OnMouseRightUp(wxMouseEvent&);
      void           OnMouseRightDown(wxMouseEvent&);
      void           OnMouseLeftUp(wxMouseEvent&);
      void           OnMouseLeftDown(wxMouseEvent&);
      void           OnMouseLeftDClick(wxMouseEvent&);
      void           OnZoom(wxCommandEvent&);
      void           OnMouseIN(wxCommandEvent&);
      void           initializeGL();
   private:
      void           CursorControl(bool, bool);
      CTM            _LayCTM;      // Layout translation matrix
      TP             ScrMARK;      // Current marker position in DB units
      TP             ScrMARKold;   // Old marker position  in DB units
      TP             n_ScrMARK;    // Normalized marker position in DB units
      TP             n_ScrMARKold; // Normalized Old marker position  in DB units
      TP             lp_BL;        // bottom left corner of the current visual window
      TP             lp_TR;        // top right corner of the current visual window
      double         WH_ratio;     // width/height ratio of the screen as reported by the openGL 
      TP             presspoint;   // store the location where a mouse button has been pressed
      TP             releasepoint; // store the location where a mouse button has been released
      void           update_viewport();
      bool           tmp_wnd;
      bool           invalid_window; // Indicates canvas needs repainting due to a change of the zoom
      bool           mouse_input;    // Indicates that a mouse input is expected
      bool           rubber_band;    // Indicates that moving or changing objects must be drawn
      bool           restricted_move;// when mouse cotrolled input
      wxCursor*      crossCur;
      //
      void           wnd_paint();
      void           rubber_paint();
      void           UpdateCoordWin(int coord, POSITION_TYPE postype, int dcoord, POSITION_TYPE dpostype);
      void           EventMouseClick(int button);
      DECLARE_EVENT_TABLE();
   };

   wxCursor* MakeCursor(const char * pXpm[36],  int HotX, int HotY );
}   
#endif
