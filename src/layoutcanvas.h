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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Wed Dec 26 2001
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Canvas control
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef LAYOUTCANVAS_H
#define LAYOUTCANVAS_H

#include <GL/glew.h>
#include <wx/glcanvas.h>
#include <wx/cursor.h>
#include <wx/image.h>
#include <string>
#include "ttt.h"
#include "tuidefs.h"

namespace tui {
   //=============================================================================
   //class StatusLine
   //{
   //   public:
   //                 StatusLine():_Xcoord(TP()),_Ycoord(TP()) {};
   //   void          update(const int4b, const CTM&);
   //   void          draw();
   //   void          update_coords(const TP&);
   //   private:
   //      TP             _sb_BL;
   //      TP             _sb_TR;
   //      double         _scaledpix;
   //      TP             _cX;
   //      TP             _cY;
   //      TP             _dX;
   //      TP             _dY;
   //      TP             _wcX;
   //      TP             _wcY;
   //      DBbox          _Xcoord;
   //      DBbox          _Ycoord;
   //      TP             _cp;
   //};
   class LayoutCanvas;
   //=============================================================================
   class TpdOglContext : public wxGLContext {
   public:
                     TpdOglContext(wxGLCanvas*, wxGLContextAttrs*);
      void           glewContext(LayoutCanvas*);
      void           printStatus() const;
      bool           resizeGL(int, int);
      bool           useVboRendering() const          { return _useVboRendering;             }
      bool           useShaders() const               { return _useShaders;                  }
   private:
      bool           _oglVersion14;             //! OpenGL version >= 1.4 detected
      bool           _oglVersion33;             //! OpenGL version >= 3.3 detected
      bool           _oglExtMultiDrawArrays;    //! GL_EXT_multi_draw_arrays feature is supported
      bool           _oglArbVertexBufferObject; //! GL_ARB_vertex_buffer_object feature is supported
      bool           _useVboRendering;
      bool           _useShaders;
      bool           _glewInitDone;
      int            _ww;
      int            _wh;
  };
  
   //=============================================================================
   class LayoutCanvas : public wxGLCanvas  {
   public:
                     LayoutCanvas(wxWindow *parent, const wxPoint&,
                                                const wxSize& , wxGLAttributes attr);
      friend class DrawThread;
      virtual       ~LayoutCanvas();
      void           snapshot(byte*&, word&, word&);
      void           showInfo();
      void           setOglThread(bool /*val*/) {_oglThread = true;}
      TpdOglContext* glRC() { return _glRC;}
      void           glewContext() {_glRC->glewContext(this);setScrCTM(DEFAULT_ZOOM_BOX);_initialised = true;}

      bool           initStatus() {
#ifdef __WXGTK__
   #if !wxCHECK_VERSION(2,9,0)
         return (NULL != _xVisual);
   #else
         return (nullptr != _glRC);
   #endif
#else
         return (nullptr != _glRC);
#endif
      }
      void           setBlinkInterval(word bi)   {_blinkInterval = bi;}
   protected:
      void           OnpaintGL(wxPaintEvent& event);
      void           OnresizeGL(wxSizeEvent& event);
      void           OnEraseBackground(wxEraseEvent&) {/* this prevents flickering !*/};
      void           OnMouseMotion(wxMouseEvent&);
      void           OnMouseRightUp(wxMouseEvent& WXUNUSED(event));
      void           OnMouseRightDown(wxMouseEvent& WXUNUSED(event));
      void           OnMouseLeftUp(wxMouseEvent& WXUNUSED(event));
      void           OnMouseMiddleUp(wxMouseEvent&);
      void           OnMouseLeftDown(wxMouseEvent& WXUNUSED(event));
      void           OnMouseLeftDClick(wxMouseEvent&);
      void           OnMouseWheel(wxMouseEvent&);
      void           OnChar(wxKeyEvent&);
      void           OnZoom(wxCommandEvent&);
      void           OnMouseIN(wxCommandEvent&);
      void           OnCMcontinue(wxCommandEvent& WXUNUSED(event));
      void           OnCMabort(wxCommandEvent& WXUNUSED(event));
      void           OnCMchangeLayer(wxCommandEvent& WXUNUSED(event));
      void           OnCMrulerState(wxCommandEvent& WXUNUSED(event));
      void           OnCMcancel(wxCommandEvent& WXUNUSED(event));
      void           OnCMclose(wxCommandEvent& WXUNUSED(event));
      void           OnRepeatLastCmd(wxCommandEvent& WXUNUSED(event));
      void           OnCMFlip(wxCommandEvent&);
      void           OnCMRotate(wxCommandEvent&);
      void           OnCursorType(wxCommandEvent&);
      void           OnDrcCollect(wxCommandEvent&);
      void           OnPanCenter(wxCommandEvent&);
      void           OnTimer(wxTimerEvent& WXUNUSED(event));

      void           viewshift();
   private:
      TpdOglContext* _glRC;
      void           cursorControl(bool, bool);
      void           pointUpdate(int nX, int nY);
      void           updateViewport();
      void           wndPaint();
      void           drawOGLBuffer();
      void           boldOnHover();
      void           drawZeroMark();
      void           updateCoordWin(int coord, CVSSTATUS_TYPE postype, int dcoord, CVSSTATUS_TYPE dpostype);
      void           eventMouseClick(int button);
      void           setScrCTM(const DBbox&);
      DBbox*         zoomIn();
      DBbox*         zoomOut();
      DBbox*         zoomLeft();
      DBbox*         zoomRight();
      DBbox*         zoomUp();
      DBbox*         zoomDown();
//      void           drawInterim(const TP&);
      CTM            _layCTM;        //! Layout translation matrix
      TP             _scrMark;       //! Current marker position in DB units
      TP             _scrMarkOld;    //! Old marker position  in DB units
      TP             _nScrMark;      //! Normalized marker position in DB units
      TP             _nScrMarkOld;   //! Normalized Old marker position  in DB units
      TP             _lpBL;          //! bottom left corner of the current visual window
      TP             _lpTR;          //! top right corner of the current visual window
      double         _whRatio;       //! width/height ratio of the screen as reported by the openGL
      word           _apTrigger;     //! autopan trigger limit
      TP             _pressPoint;    //! store the location where a mouse button has been pressed
      TP             _releasePoint;  //! store the location where a mouse button has been released
      bool           _tmpWnd;
      bool           _invalidWindow; //! Indicates canvas needs repainting due to a change of the zoom
      bool           _mouseInput;    //! Indicates that a mouse input is expected
      bool           _rubberBand;    //! Indicates that moving or changing objects must be drawn
      bool           _restrictedMove;//! when mouse controlled input
      wxCursor*      _crossCur;
      //
      bool           _reperX;        //! Draw a cursor line across the window parallel to the X axis
      bool           _reperY;        //! Draw a cursor line across the window parallel to the Y axis
      bool           _longCursor;    //! Stretch the cursor across the entire canvas
      bool           _oglThread;     //! Run the openGL drawing in a separate thread
      word           _blinkInterval; //!
      wxTimer        _blinkTimer;    //! To implement the flashing images
      bool           _blinkOn;
      bool           _initialised;   //!
#ifdef __WXGTK__
      XVisualInfo*   _xVisual;       //
#endif
//      StatusLine     _status_line;
   };

   class DrawThread : public wxThread
   {
   public:
                               DrawThread( LayoutCanvas* canvas, wxThreadKind kind=wxTHREAD_DETACHED ):
                                 wxThread(kind), _canvas(canvas) {};
   protected:
      void*                    Entry();
      tui::LayoutCanvas*       _canvas;
      static wxMutex           _mutex;
   };

   wxCursor* MakeCursor(const char * pXpm[36],  int HotX, int HotY );
}
#endif
