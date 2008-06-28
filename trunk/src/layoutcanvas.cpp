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

#include "tpdph.h"
#include <math.h>
#include <sstream>
#include <wx/wx.h>

#if WIN32
#include <wx/image.h>
#endif

#include "layoutcanvas.h"
#include "toped.h"
#include "../tpd_DB/viewprop.h"
#include "../tpd_DB/datacenter.h"
#include "../tpd_parser/ted_prompt.h"
#include "../tpd_DB/tedat.h"
#include "../tpd_common/glf.h"

extern DataCenter*               DATC;
extern console::ted_cmd*         Console;
extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_CANVAS_CURSOR;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_MOUSE_ACCEL;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CURRENT_LAYER;

#include "../ui/crosscursor.xpm"
#include "../ui/zerocross.xpm"

//tui::CanvasStatus::CanvasStatus(){};
void tui::StatusLine::update(const int4b width, const CTM& _LayCTM)
{
   _sb_BL = TP(0,0)       * _LayCTM;
   _sb_TR = TP(width, 30) * _LayCTM;
   
   DBbox pixelbox = DBbox(TP(),TP(14,14)) * _LayCTM;
   _scaledpix = ((double)(pixelbox.p2().x()-pixelbox.p1().x()));
   _cY = TP(width-150, 17) * _LayCTM;
   _cX = TP(width-300, 17) * _LayCTM;
   _dY = TP(width-450, 17) * _LayCTM;
   _dX = TP(width-600, 17) * _LayCTM;
   _Ycoord = DBbox(TP(width - 130, 28), TP(width -   2, 2)) * _LayCTM;
   _Xcoord = DBbox(TP(width - 280, 28), TP(width - 162, 2)) * _LayCTM;
   _wcY = TP(width-120, 16) * _LayCTM;
   _wcX = TP(width-270, 16) * _LayCTM;
}

void tui::StatusLine::draw()
{
   glColor4f((GLfloat)1,(GLfloat)1,(GLfloat)1,(GLfloat)0.7);
   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
   glDisable(GL_POLYGON_STIPPLE);   //- for solid fill
   glRecti(_sb_TR.x(), _sb_TR.y(), _sb_BL.x(), _sb_BL.y());

   glColor4f(0,0,0,1);
   glPushMatrix();
   glTranslatef(_cY.x(), _cY.y(), 0);
   glScalef(_scaledpix, _scaledpix, 1);
   glfDrawSolidString("Y:");
   glPopMatrix();
   
   glPushMatrix();
   glTranslatef(_cX.x(), _cX.y(), 0);
   glScalef(_scaledpix, _scaledpix, 1);
   glfDrawSolidString("X:");
   glPopMatrix();

   glPushMatrix();
   glTranslatef(_dY.x(), _dY.y(), 0);
   glScalef(_scaledpix, _scaledpix, 1);
   glfDrawSolidString("dX:");
   glPopMatrix();
   
   glPushMatrix();
   glTranslatef(_dX.x(), _dX.y(), 0);
   glScalef(_scaledpix, _scaledpix, 1);
   glfDrawSolidString("dX:");
   glPopMatrix();
   
   update_coords(_cp);
}
//      private:
//         TP             _sb_BL;
//         TP             _sb_TR;
//         real           _scaledpix;
//   };

void tui::StatusLine::update_coords(const TP& cp)
{
   _cp = cp;
   glColor4f(0,0,0,1);
   glRecti(_Xcoord.p1().x(), _Xcoord.p1().y(), _Xcoord.p2().x(), _Xcoord.p2().y());
   glRecti(_Ycoord.p1().x(), _Ycoord.p1().y(), _Ycoord.p2().x(), _Ycoord.p2().y());

//   glScissor( _Xcoord.p1().x(), _Xcoord.p1().y(), _Xcoord.p2().x() - _Xcoord.p1().x(), _Xcoord.p2().y() - _Xcoord.p1().y() );
//   glEnable(GL_SCISSOR_TEST);
//   
   glColor4f(0,1,1,1);

   wxString wsX;
   wsX.sprintf(wxT("%6d"),_cp.x());
   wxString wsY;
   wsY.sprintf(wxT("%6d"),_cp.y());

//
//   glClear(GL_COLOR_BUFFER_BIT);
 //  
   glPushMatrix();
   glTranslatef(_wcX.x(), _wcX.y(), 0);
   glScalef(_scaledpix, _scaledpix, 1);
   glfDrawSolidString(wsX.mb_str());
   glPopMatrix();
   
   glPushMatrix();
   glTranslatef(_wcY.x(), _wcY.y(), 0);
   glScalef(_scaledpix, _scaledpix, 1);
   glfDrawSolidString(wsY.mb_str());
   glPopMatrix();
//   glDisable(GL_SCISSOR_TEST);
   
}
//=============================================================================
// class LayoutCanvas
//=============================================================================

BEGIN_EVENT_TABLE(tui::LayoutCanvas, wxGLCanvas)
   EVT_PAINT            ( tui::LayoutCanvas::OnpaintGL         )
   EVT_SIZE             ( tui::LayoutCanvas::OnresizeGL        )
   EVT_ERASE_BACKGROUND ( tui::LayoutCanvas::OnEraseBackground )
   EVT_MOTION           ( tui::LayoutCanvas::OnMouseMotion     )
   EVT_RIGHT_DOWN       ( tui::LayoutCanvas::OnMouseRightDown  )
   EVT_RIGHT_UP         ( tui::LayoutCanvas::OnMouseRightUp    )
//   EVT_LEFT_DOWN        ( tui::LayoutCanvas::OnMouseLeftDown   )
   EVT_LEFT_UP          ( tui::LayoutCanvas::OnMouseLeftUp     )
   EVT_LEFT_DCLICK      ( tui::LayoutCanvas::OnMouseLeftDClick )
   EVT_MIDDLE_UP        ( tui::LayoutCanvas::OnMouseMiddleUp   )
   EVT_MOUSEWHEEL       ( tui::LayoutCanvas::OnMouseWheel      )
   EVT_CHAR             ( tui::LayoutCanvas::OnChar            )
   EVT_TECUSTOM_COMMAND (wxEVT_CANVAS_ZOOM  , wxID_ANY, tui::LayoutCanvas::OnZoom)
   EVT_TECUSTOM_COMMAND (wxEVT_MOUSE_INPUT  , wxID_ANY, tui::LayoutCanvas::OnMouseIN)
   EVT_TECUSTOM_COMMAND (wxEVT_CANVAS_CURSOR, wxID_ANY, tui::LayoutCanvas::OnCursorType)

   EVT_MENU(         CM_RULER, LayoutCanvas::OnCMrulerState    )
   EVT_MENU(         CM_CHLAY, LayoutCanvas::OnCMchangeLayer   )
   EVT_MENU(      CM_CONTINUE, LayoutCanvas::OnCMcontinue      )
   EVT_MENU(         CM_ABORT, LayoutCanvas::OnCMabort         )
   EVT_MENU(   CM_CANCEL_LAST, LayoutCanvas::OnCMcancel        )
   EVT_MENU(         CM_CLOSE, LayoutCanvas::OnCMclose         )
   EVT_MENU(         CM_AGAIN, LayoutCanvas::OnRepeatLastCmd   )
   EVT_MENU(          CM_FLIP, LayoutCanvas::OnCMFlip          )
   EVT_MENU(        CM_ROTATE, LayoutCanvas::OnCMRotate        )
   EVT_MENU( TMVIEW_PANCENTER, LayoutCanvas::OnPanCenter       )
END_EVENT_TABLE()

//   EVT_MENU(      CM_CHLAY, TopedFrame::OnCurrentLayer      )

tui::LayoutCanvas::LayoutCanvas(wxWindow *parent, const wxPoint& pos, 
     const wxSize& size, int* attribList): 
 wxGLCanvas(parent, ID_TPD_CANVAS, pos, size, 0,wxT("LayoutCanvas"), attribList)
{
#ifdef __WXGTK__
   //  Here we'll have to check that we've got what we've asked for. Is is
   // quite possible that we can't get the requested GL visual. If that is the case
   // we'll have to aboandon the init sequence right here, otherwise Toped will
   // crash.
   x_visual = (XVisualInfo*) m_vi;
   if (NULL == x_visual) return;
#endif
   crossCur = MakeCursor(crosscursor,16, 16);
   //crossCur = DEBUG_NEW wxCursor((const char*)crosscursor,16, 16);
   SetCursor(*crossCur);
   tmp_wnd = false;
   mouse_input = false;
   rubber_band = false;
   restricted_move = false;
   invalid_window = false;
   reperX = reperY = long_cursor = false;
   initializeGL();
   ap_trigger = 10;
   glfInit();
   zeroMark = DEBUG_NEW wxImage(zerocross);   
}

wxImage   tui::LayoutCanvas::snapshot(void)
{
   int width, height;

   int nv[4];
   glGetIntegerv(GL_VIEWPORT, nv);
   width = nv[2];
   height = nv[3];
   void *buffer = malloc(3*(width+1)*(height+1));

   glReadBuffer(GL_BACK);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);//GL_BGRA_EXT
   wxImage image=wxImage(width, height, true);
   for (int j=0; j<(height-1); j++)
      for (int i=0; i<(width-1); i++)

      {
         unsigned char* cbuf = (unsigned char *)(buffer);
         cbuf = cbuf +3*(i+j*width);
         unsigned char r = *(cbuf+0);
         unsigned char g = *(cbuf+1);
         unsigned char b = *(cbuf+2);

         image.SetRGB(i, height-j-1, r, g, b);
      }
   //image.SetData((unsigned char*)buffer, true);

   image.SaveFile(wxT("snapshot.bmp"));


   wxImage image2;
   return image2;
}


void tui::LayoutCanvas::viewshift()
{
   //@TODO screen sliding. Some rough ideas.
   int Wcl, Hcl;
   const int slide_step = 100;
   GetClientSize(&Wcl,&Hcl);
   glReadBuffer(GL_FRONT);
   glRasterPos2i (0, 0);
   glCopyPixels (slide_step, 0, Wcl-slide_step, Hcl, GL_COLOR);
   glAccum(GL_LOAD, 1.0);
/*   slide = false;*/
}

void tui::LayoutCanvas::initializeGL() {
   // Create a couple of common callback functions
#ifndef WIN32
   gluTessCallback(laydata::tdtdata::tessellObj, GLU_TESS_BEGIN,
                                   (GLvoid(*)())&glBegin);
   gluTessCallback(laydata::tdtdata::tessellObj, GLU_TESS_VERTEX,
                                   (GLvoid(*)())&laydata::tdtdata::polyVertex);
   gluTessCallback(laydata::tdtdata::tessellObj, GLU_TESS_END,
                                                &glEnd);
#else 
   gluTessCallback(laydata::tdtdata::tessellObj, GLU_TESS_BEGIN,
                                   (GLvoid(__stdcall *)())&glBegin);
   gluTessCallback(laydata::tdtdata::tessellObj, GLU_TESS_VERTEX,
                                   (GLvoid(__stdcall *)())&laydata::tdtdata::polyVertex);
   gluTessCallback(laydata::tdtdata::tessellObj, GLU_TESS_END,
                                                &glEnd);
#endif
   //@TODO Check somewhere that RGBA mode is available!?
   //@TODO The next call needs to be fitted in some kind of GL descructor
   // gluDeleteTess(tessellObj);

   //@NOTE: With the Mesa library updates (first noticed in ver. 6.5) - most of the
   // gl* functions are starting with ASSERT_OUTSIDE_BEGIN_END(...) which in turn
   // will produce a segmentation. Other openGL implementations are more polite,
   // but I don't know the result of those calls will be.
   // So... don't try to put gl stuff here, unless you're sure that it is not
   // causing troubles and that's the proper place for it.
}

void tui::LayoutCanvas::OnresizeGL(wxSizeEvent& event) {
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);
   #ifndef __WXMOTIF__
      if (GetContext())
   #endif
    {
      SetCurrent();
      glViewport( 0, 0, (GLint)w, (GLint)h );
    }
    lp_BL = TP(0,0)  * _LayCTM;
    lp_TR = TP(w, h) * _LayCTM;
   invalid_window = true;
}


void tui::LayoutCanvas::OnpaintGL(wxPaintEvent& event) {
    wxPaintDC dc(this);
   #ifndef __WXMOTIF__
      if (!GetContext()) return;
   #endif
   SetCurrent();
   // invalid_window indicates zooming.
   // event.GetEventType() == event.GetId() should means that database is updated
   // In both cases - the entire window is redrawn
   if ((invalid_window) || (event.GetEventType() == event.GetId()))
   {
      glMatrixMode( GL_MODELVIEW );
      glShadeModel( GL_FLAT ); // Single color
      update_viewport();
      //@TODO !! Check somewhere that RGBA mode is available!?
      // CTM matrix stuff
      glLoadIdentity();
      glOrtho(lp_BL.x(),lp_TR.x(),lp_TR.y(),lp_BL.y(),-1.0,1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glClear(GL_ACCUM_BUFFER_BIT);
      drawZeroMark();
      DATC->openGL_draw(_LayCTM);    // draw data
      glAccum(GL_LOAD, 1.0);
      invalid_window = false;
      if (rubber_band) rubber_paint();
   }
   else
   {
      glAccum(GL_RETURN, 1.0);
      if       (tmp_wnd)         wnd_paint();
      else if  (rubber_band)     rubber_paint();
   }
   // deal with the long cursor
   if (reperX || reperY)
   {
      glColor4f(1, 1, 1, .5);
      glBegin(GL_LINES);
      if (reperX)
      {
         glVertex2i(lp_BL.x(), ScrMARK.y()) ;
         glVertex2i(lp_TR.x(), ScrMARK.y());
      }
      if (reperY)
      {
         glVertex2i(ScrMARK.x() , lp_BL.y()) ;
         glVertex2i(ScrMARK.x() , lp_TR.y());
      }
      glEnd();
   }

   SwapBuffers();
}

void tui::LayoutCanvas::drawZeroMark()
{
   
//   glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
//   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glRasterPos2i(0,0);
//   glBitmap(32,32,16,16,0,0, zeroMark);
   glDrawPixels(32,32, GL_RGB, GL_UNSIGNED_BYTE, zeroMark->GetData());
}

// void tui::LayoutCanva+s::drawInterim(const TP& cp)
// {
//    wxPaintDC dc(this);
//    #ifndef __WXMOTIF__
//       if (!GetContext()) return;
//    #endif
//    SetCurrent();
//    _status_line.update_coords(cp);
//    SwapBuffers();
// }

void tui::LayoutCanvas::wnd_paint() {
   glColor4f((GLfloat)0.7, (GLfloat)0.7, (GLfloat)0.7, (GLfloat)0.4); // gray
   glDisable(GL_POLYGON_STIPPLE);
   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
   glRecti(presspoint.x(),presspoint.y(), n_ScrMARK.x(), n_ScrMARK.y());
   glDisable(GL_POLYGON_SMOOTH); //- for solid fill
   glEnable(GL_POLYGON_STIPPLE);
}

void tui::LayoutCanvas::rubber_paint()
{
   DATC->tmp_draw(_LayCTM, releasepoint, n_ScrMARK);
}

void tui::LayoutCanvas::CursorControl(bool shift, bool ctl) {
   // alt key forces free move
   // shift forces restricted move
   if (ctl || !(rubber_band && (restricted_move || shift))) {
      n_ScrMARK = ScrMARK; n_ScrMARKold = ScrMARKold;
      return;
   }   
   n_ScrMARKold = n_ScrMARK;
   int sdX = ScrMARK.x() - releasepoint.x();
   int sdY = ScrMARK.y() - releasepoint.y();
   int dX = abs(sdX);
   int dY = abs(sdY);
   // The sign actually is the sign of the tangens. To avoud troubles with the division by zero,
   // it is easier and faster to obtain the sign like this
   int sign = ((sdX * sdY) >= 0) ? 1 : -1;
   bool _45deg = (DATC->marker_angle() == 45);
   if (dX > dY) {
      if (_45deg && (dX < 2*dY)) n_ScrMARK.setY( sign*sdX + releasepoint.y() );
      else                       n_ScrMARK.setY( releasepoint.y() );
      n_ScrMARK.setX(ScrMARK.x() );
   }   
   else {
      if (_45deg && (dY < 2*dX)) n_ScrMARK.setX( sign*sdY + releasepoint.x() );
      else                      n_ScrMARK.setX( releasepoint.x() );
      n_ScrMARK.setY(ScrMARK.y() );
   }
}

void tui::LayoutCanvas::UpdateCoordWin(int coord, CVSSTATUS_TYPE postype, int dcoord, CVSSTATUS_TYPE dpostype) {
   wxString ws;
   wxCommandEvent eventPOSITION(wxEVT_CANVAS_STATUS);
   ws.sprintf(wxT("%3.2f"),coord*DATC->UU());
   eventPOSITION.SetString(ws);
   eventPOSITION.SetInt(postype);
   wxPostEvent(this, eventPOSITION);
   if (rubber_band) {
      ws.sprintf(wxT("%3.2f"),dcoord*DATC->UU());
      eventPOSITION.SetString(ws);
      eventPOSITION.SetInt(dpostype);
      wxPostEvent(this, eventPOSITION);
   }
}

void tui::LayoutCanvas::EventMouseClick(int button) {
   wxCommandEvent eventButtonUP(wxEVT_COMMAND_ENTER);

   telldata::ttpnt* ttp = DEBUG_NEW telldata::ttpnt(releasepoint.x()*DATC->UU(),
                                              releasepoint.y()*DATC->UU());
   //Post an event to notify the console
   eventButtonUP.SetClientData((void*)ttp);
   eventButtonUP.SetInt(button);
   wxPostEvent(Console, eventButtonUP);
   // send the point to the current temporary object in the data base
   if (0 == button) {
      DATC->mousePoint(releasepoint);
   }
}

void tui::LayoutCanvas::PointUpdate(int nX, int nY)
{
   ScrMARKold = ScrMARK;
   ScrMARK = TP(nX,nY) * _LayCTM;
   int4b stepDB = DATC->stepDB();
   ScrMARK.roundTO(stepDB);
   
   // update movement indicators
   int deltaX = abs(ScrMARKold.x() - ScrMARK.x());
   int deltaY = abs(ScrMARKold.y() - ScrMARK.y());
   if (!(deltaX || deltaY)) return;
   //
   CursorControl(false, false);
   if (deltaX > 0) 
      UpdateCoordWin(ScrMARK.x(), CNVS_POS_X, (n_ScrMARK.x() - releasepoint.x()), CNVS_DEL_X);
   if (deltaY > 0) 
      UpdateCoordWin(ScrMARK.y(), CNVS_POS_Y, (n_ScrMARK.y() - releasepoint.y()), CNVS_DEL_Y);
}

void tui::LayoutCanvas::OnMouseMotion(wxMouseEvent& event)
{
   ScrMARKold = ScrMARK;
   // get a current position
   ScrMARK = TP(event.GetX(),event.GetY()) * _LayCTM ;
   int4b stepDB = DATC->stepDB();
   ScrMARK.roundTO(stepDB);
   if (DATC->autopan() && mouse_input && !invalid_window)
   {
      CTM LayCTMR(_LayCTM.Reversed());
      TP sp_BL     =     lp_BL * LayCTMR;
      TP sp_TR     =     lp_TR * LayCTMR;
      TP s_ScrMARK = n_ScrMARK * LayCTMR;
      TP nsp;
      if      (abs(s_ScrMARK.x() - sp_BL.x()) < ap_trigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_LEFT);
         OnZoom(eventZOOM);
         nsp = ScrMARK * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(sp_TR.x() - s_ScrMARK.x()) < ap_trigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_RIGHT);
         OnZoom(eventZOOM);
         nsp = ScrMARK * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(sp_BL.y() - s_ScrMARK.y()) < ap_trigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_UP);
         OnZoom(eventZOOM);
         nsp = ScrMARK * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(s_ScrMARK.y() - sp_TR.y()) < ap_trigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_DOWN);
         OnZoom(eventZOOM);
         nsp = ScrMARK * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
   }
   // update movement indicators
   static int deltaX = abs(ScrMARKold.x() - ScrMARK.x());
   static int deltaY = abs(ScrMARKold.y() - ScrMARK.y());
   if (!(deltaX || deltaY)) return;
   //
   CursorControl(event.ShiftDown(), event.ControlDown());
   if (deltaX > 0) 
      UpdateCoordWin(ScrMARK.x(), CNVS_POS_X, (n_ScrMARK.x() - releasepoint.x()), CNVS_DEL_X);
   if (deltaY > 0) 
      UpdateCoordWin(ScrMARK.y(), CNVS_POS_Y, (n_ScrMARK.y() - releasepoint.y()), CNVS_DEL_Y);

//   drawInterim(ScrMARK);
   if (tmp_wnd || mouse_input || reperX || reperY) Refresh();//updateGL();
}
      
void tui::LayoutCanvas::OnMouseRightDown(wxMouseEvent& WXUNUSED(event)) {
   presspoint = ScrMARK;
   tmp_wnd = true;
}   

void tui::LayoutCanvas::OnMouseRightUp(wxMouseEvent& WXUNUSED(event)) {
   tmp_wnd = false;
   int4b stepDB = DATC->stepDB();
   if ((abs(presspoint.x() - ScrMARK.x())  > stepDB) ||
       (abs(presspoint.y() - ScrMARK.y())  > stepDB))   {
      // if dragging ...
      wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
      eventZOOM.SetInt(ZOOM_WINDOWM);
      OnZoom(eventZOOM);
   }
   else {
   // Context menu here
      wxMenu menu;
      if ( NULL != Console->puc) {
         switch (DATC->currentop()) {
            case console::op_dbox:
               if (Console->numpoints() > 0) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               }
               menu.Append(   CM_CHLAY, wxT("Change Layer"));
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case console::op_dpoly:
               if (Console->numpoints() >= 3) {
                  menu.Append(CM_CLOSE, wxT("Close polygon"));
               }
               if (Console->numpoints() > 1) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel last point"));
               }
               else if (Console->numpoints() > 0) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               }
               if (DATC->drawruler())
                  menu.Append(   CM_RULER, wxT("Ruler Off"));
               else
                  menu.Append(   CM_RULER, wxT("Ruler On"));
               menu.Append(   CM_CHLAY, wxT("Change Layer"));
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case console::op_dwire:
               if (Console->numpoints() > 1) {
                  menu.Append(CM_CLOSE, wxT("Finish wire"));
               }
               if (Console->numpoints() > 1) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel last point"));
               }
               else if (Console->numpoints() > 0) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               }
               if (DATC->drawruler())
                  menu.Append(   CM_RULER, wxT("Ruler Off"));
               else
                  menu.Append(   CM_RULER, wxT("Ruler On"));
               menu.Append(   CM_CHLAY, wxT("Change Layer"));
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case console::op_cbind:
            case console::op_abind:
            case console::op_tbind:
               menu.Append(CM_ROTATE, wxT("Rotate"));
               menu.Append(CM_FLIP, wxT("Flip"));
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
//            case console::op_copy:
            default:
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
         }
      }
      else { // no user input expected
         if (Console->cmdHistoryExists())
         {
            menu.Append(   CM_AGAIN, wxString(Console->lastCommand(), wxConvUTF8));
            menu.Append(TMEDIT_UNDO, wxT("undo"));
            menu.AppendSeparator();
         }
         if (DATC->numselected() > 0) {
            menu.Append(       TMEDIT_MOVE, wxT("move"  ));
            menu.Append(       TMEDIT_COPY, wxT("copy"  ));
            menu.Append(   TMEDIT_ROTATE90, wxT("rotate"));
            menu.Append(      TMEDIT_FLIPX, wxT("flipX" ));
            menu.Append(      TMEDIT_FLIPY, wxT("flipY" ));
            menu.Append(TMSEL_REPORT_SLCTD, wxT("report selected"));
         }
         else {
            menu.Append(     TMDRAW_BOX, wxT("box"   ));
            menu.Append(    TMDRAW_POLY, wxT("poly"  ));
            menu.Append(    TMDRAW_WIRE, wxT("wire"  ));
            menu.Append(    TMDRAW_TEXT, wxT("text"  ));
         }
         menu.AppendSeparator();
         menu.Append( TMSEL_SELECT_IN, wxT("select"     ));
         menu.Append(TMSEL_PSELECT_IN, wxT("part select"));
         if (DATC->numselected() > 0) {
            menu.Append( TMSEL_UNSELECT_IN, wxT("unselect"     ));
            menu.Append(TMSEL_PUNSELECT_IN, wxT("part unselect"));
         }
      }
      TP s_ScrMARK = ScrMARK * _LayCTM.Reversed();
      PopupMenu(&menu, wxPoint(s_ScrMARK.x(), s_ScrMARK.y()));
   }
}

void tui::LayoutCanvas::OnMouseLeftDown(wxMouseEvent& WXUNUSED(event)) {
//   presspoint = ScrMARK;
//   mouseIN(true);
}   

void tui::LayoutCanvas::OnMouseLeftUp(wxMouseEvent& WXUNUSED(event)) {
//   if ((abs(presspoint.x() - ScrMARK.x())  > step) or
//       (abs(presspoint.y() - ScrMARK.y())  > step))   {
//      // if dragging ...
//      mouseIN(false);
////      zoom(presspoint.x(),presspoint.y(),ScrMARK.x(),ScrMARK.y());
//   }
//   else {
      releasepoint = n_ScrMARK;
      if (mouse_input)  rubber_band = true;
      EventMouseClick(0);
//   }   
}

void tui::LayoutCanvas::OnMouseLeftDClick(wxMouseEvent& event)
{
   wxString ws;
   wxCommandEvent eventMOUSEACCEL(wxEVT_MOUSE_ACCEL);
   ws.sprintf(wxT("{%3.2f,%3.2f}"),ScrMARK.x()*DATC->UU(), ScrMARK.y()*DATC->UU());
   eventMOUSEACCEL.SetString(ws);
   eventMOUSEACCEL.SetInt(event.ShiftDown() ? 0 : 1);
   wxPostEvent(this, eventMOUSEACCEL);
}

void tui::LayoutCanvas::OnMouseMiddleUp(wxMouseEvent& event)
{
   TP s_ScrMARK = ScrMARK * _LayCTM.Reversed();
   wxMenu menu;
   menu.Append(TMVIEW_ZOOMALL  , wxT("Zoom All"));
   menu.Append(TMVIEW_PANCENTER, wxT("Pan Center"));
   PopupMenu(&menu, wxPoint(s_ScrMARK.x(), s_ScrMARK.y()));
}

void tui::LayoutCanvas::OnMouseWheel(wxMouseEvent& event)
{
   int delta    = event.GetWheelDelta();
   int fulldist = event.GetWheelRotation();
   double scroll = fulldist / delta;
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   if (event.ShiftDown())
   {
      if      ( 1 <= scroll) eventZOOM.SetInt(ZOOM_UP);
      else if (-1 >= scroll) eventZOOM.SetInt(ZOOM_DOWN);
   }
   else if (event.ControlDown())
   {
      if      ( 1 <= scroll) eventZOOM.SetInt(ZOOM_RIGHT);
      else if (-1 >= scroll) eventZOOM.SetInt(ZOOM_LEFT);
   }
   else
   {
      const double scalefactor = event.AltDown() ? 0.8 : 0.5;
      CTM tmpmtrx;
      TP markerpos(event.GetX(), event.GetY());
      if      ( 1 <= scroll)
         tmpmtrx.Scale(scalefactor,scalefactor);
      else if (-1 >= scroll)
         tmpmtrx.Scale(1/scalefactor,1/scalefactor);
      tmpmtrx.Translate(markerpos * _LayCTM - markerpos * _LayCTM * tmpmtrx);
      DBbox* box = DEBUG_NEW DBbox( lp_BL, lp_TR );
      (*box) = (*box) * tmpmtrx;
      eventZOOM.SetInt(tui::ZOOM_WINDOW);
      eventZOOM.SetClientData(static_cast<void*>(box));
   }
   OnZoom(eventZOOM);
   PointUpdate(event.GetX(), event.GetY());
}

void tui::LayoutCanvas::OnChar(wxKeyEvent& event)
{
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   switch(event.GetKeyCode())
   {
      case WXK_LEFT : eventZOOM.SetInt(ZOOM_LEFT ); break;
      case WXK_RIGHT: eventZOOM.SetInt(ZOOM_RIGHT); break;
      case WXK_UP   : eventZOOM.SetInt(ZOOM_UP   ); break;
      case WXK_DOWN : eventZOOM.SetInt(ZOOM_DOWN ); break;
      case '+'      : eventZOOM.SetInt(ZOOM_IN   ); break;
      case '-'      : eventZOOM.SetInt(ZOOM_OUT  ); break;
            default : event.Skip(); return;
   }
   OnZoom(eventZOOM);
   PointUpdate(event.GetX(), event.GetY());
}

void tui::LayoutCanvas::OnZoom(wxCommandEvent& evt) {
   DBbox* box = NULL;
   switch (evt.GetInt())
   {
      case ZOOM_WINDOW : box = static_cast<DBbox*>(evt.GetClientData());break;
      case ZOOM_WINDOWM: box = DEBUG_NEW DBbox(presspoint.x(),presspoint.y(),
                                             ScrMARK.x(),ScrMARK.y());break;
      case ZOOM_IN     : box = DEBUG_NEW DBbox( (3*lp_BL.x() + lp_TR.x())/4, //in
                                                (3*lp_BL.y() + lp_TR.y())/4,
                                                (3*lp_TR.x() + lp_BL.x())/4,
                                                (3*lp_TR.y() + lp_BL.y())/4);
                        break;
      case ZOOM_OUT    : box = DEBUG_NEW DBbox( (5*lp_BL.x() - lp_TR.x())/4, //out
                                                (5*lp_BL.y() - lp_TR.y())/4,
                                                (5*lp_TR.x() - lp_BL.x())/4,
                                                (5*lp_TR.y() - lp_BL.y())/4);
                        break;
      case ZOOM_LEFT   : box = DEBUG_NEW DBbox( (  lp_TR.x() + lp_BL.x())/2, //left
                                                   lp_BL.y()               ,
                                                (3*lp_BL.x() - lp_TR.x())/2,
                                                   lp_TR.y()               );
                        break;
      case ZOOM_RIGHT  : box = DEBUG_NEW DBbox( (3*lp_TR.x() - lp_BL.x())/2, // right
                                                   lp_BL.y()               ,
                                                (  lp_TR.x() + lp_BL.x())/2,
                                                   lp_TR.y()               );
                        break;
      case ZOOM_UP     : box = DEBUG_NEW DBbox(    lp_BL.x()               , // up
                                                (3*lp_BL.y() - lp_TR.y())/2,
                                                   lp_TR.x()               ,
                                                (  lp_TR.y() + lp_BL.y())/2);
                        break;
      case ZOOM_DOWN   : box = DEBUG_NEW DBbox(    lp_BL.x()               , // down
                                                (  lp_TR.y() + lp_BL.y())/2,
                                                   lp_TR.x()               ,
                                                (3*lp_TR.y() - lp_BL.y())/2);
                        break;
      case ZOOM_EMPTY  : box = DEBUG_NEW DBbox(-10,-10,90,90);
                        break;
      default: assert(false);
   }
   int Wcl, Hcl;
   GetClientSize(&Wcl,&Hcl);
   // To prevent a loss of precision in the following lines - don't use
   // integer variables (Wcl & Hcl) directly
   double W = Wcl;
   double H = Hcl;
   double w = abs(box->p1().x() - box->p2().x());
   double h = abs(box->p1().y() - box->p2().y());
   double sc =  ((W/H < w/h) ? w/W : h/H);
   double tx = ((box->p1().x() + box->p2().x()) - W*sc) / 2;
   double ty = ((box->p1().y() + box->p2().y()) - H*sc) / 2;
   _LayCTM.setCTM( sc, 0.0, 0.0, sc, tx, ty);
   _LayCTM.FlipX((box->p1().y() + box->p2().y())/2);  // flip Y coord towards the center
   DATC->setScrCTM(_LayCTM.Reversed());
   invalid_window = true;
   delete box;
   Refresh();
}

void tui::LayoutCanvas::update_viewport() {
   int W, H;
   GetClientSize(&W,&H);
   lp_BL = TP(0,0)  * _LayCTM;
   lp_TR = TP(W, H) * _LayCTM;
//   _status_line.update(W, _LayCTM);
   DATC->setClipRegion(DBbox(lp_BL.x(),lp_TR.y(), lp_TR.x(), lp_BL.y()));
   glClearColor(0,0,0,0);
}

void tui::LayoutCanvas::OnMouseIN(wxCommandEvent& evt)
{
   wxCommandEvent eventABORTEN(wxEVT_CANVAS_STATUS);
   if (1 == evt.GetExtraLong())
   { // start mouse input
      mouse_input = true;
      console::ACTIVE_OP actop;
      if (evt.GetInt() > 0) actop = console::op_dwire;
      else                  actop = (console::ACTIVE_OP)evt.GetInt();
      DATC->setCurrentOp(actop);
      //restricted_move will be true for wire and polygon
      restricted_move = (DATC->marker_angle() != 0) &&
            ((actop > 0) || (actop == console::op_dpoly));
      eventABORTEN.SetInt(CNVS_ABORTENABLE);
      reperX = (console::op_flipX == actop) || (long_cursor && (console::op_flipY != actop));
      reperY = (console::op_flipY == actop) || (long_cursor && (console::op_flipX != actop));
      if (  (console::op_flipX  == actop)
          ||(console::op_flipY  == actop)
          ||(console::op_rotate == actop)
          ||(console::op_cbind  == actop)
          ||(console::op_abind  == actop)
          ||(console::op_tbind  == actop) )
         rubber_band = true;
      releasepoint = TP(0,0);
   }
   else
   { // stop mouse input
      mouse_input = false;
      rubber_band = false;
      restricted_move = false;
      reperX = long_cursor;
      reperY = long_cursor;
      DATC->setCurrentOp(console::op_none);
      wxCommandEvent eventPOSITION(wxEVT_CANVAS_STATUS);
      eventPOSITION.SetString(wxT(""));
      eventPOSITION.SetInt(CNVS_DEL_Y);
      wxPostEvent(this, eventPOSITION);
      eventPOSITION.SetInt(CNVS_DEL_X);
      wxPostEvent(this, eventPOSITION);
      eventABORTEN.SetInt(CNVS_ABORTDISABLE);
   }
   wxPostEvent(this, eventABORTEN);
}


void tui::LayoutCanvas::OnPanCenter(wxCommandEvent&)
{
   CTM tmpmtrx;
   TP center((lp_TR.x() + lp_BL.x())/2, (lp_TR.y() + lp_BL.y())/2);
   tmpmtrx.Translate(ScrMARK - center);
   DBbox* box = DEBUG_NEW DBbox( lp_BL, lp_TR );
   (*box) = (*box) * tmpmtrx;
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   OnZoom(eventZOOM);
}

void tui::LayoutCanvas::OnCursorType(wxCommandEvent& event)
{
   long_cursor = (1 == event.GetInt());
   reperX = reperY = long_cursor;
}

void tui::LayoutCanvas::OnCMcontinue(wxCommandEvent& WXUNUSED(event))
{
// keep going ... This function is not doing anything really
   return;
}

void tui::LayoutCanvas::OnCMchangeLayer(wxCommandEvent& WXUNUSED(event))
{
   // post an event to the toped.cpp
   wxCommandEvent eventCurLay(wxEVT_CURRENT_LAYER);
   wxPostEvent(this, eventCurLay);
}

void tui::LayoutCanvas::OnCMrulerState(wxCommandEvent& WXUNUSED(event))
{
   DATC->switch_drawruler(!DATC->drawruler());
}

void tui::LayoutCanvas::OnCMabort(wxCommandEvent& WXUNUSED(event))
{
   wxCommandEvent eventButtonUP(wxEVT_COMMAND_ENTER);
   eventButtonUP.SetClientData((void*)NULL);
   eventButtonUP.SetInt(-1);
   wxPostEvent(Console, eventButtonUP);
   Refresh();
}

void tui::LayoutCanvas::OnCMcancel(wxCommandEvent& WXUNUSED(event))
{
   //Post an event to notify the console
   if (Console->numpoints() > 0) {
      wxCommandEvent eventCancelLast(wxEVT_COMMAND_ENTER);
      eventCancelLast.SetInt(-2);
      wxPostEvent(Console, eventCancelLast);
      // remove the point from the current temporary object in the data base
      DATC->mousePointCancel(releasepoint);
      rubber_paint();
   }
}

void tui::LayoutCanvas::OnCMclose(wxCommandEvent& WXUNUSED(event))
{
   releasepoint = ScrMARK;
   EventMouseClick(2);
}

void tui::LayoutCanvas::OnRepeatLastCmd(wxCommandEvent& WXUNUSED(event))
{
   Console->parseCommand(wxString(Console->lastCommand(), wxConvUTF8));
}

void tui::LayoutCanvas::OnCMFlip(wxCommandEvent&)
{
   //Post an event to notify the console
   wxCommandEvent eventCancelLast(wxEVT_COMMAND_ENTER);
   eventCancelLast.SetInt(-4);
   wxPostEvent(Console, eventCancelLast);
   DATC->mouseFlip();
}

void tui::LayoutCanvas::OnCMRotate(wxCommandEvent&)
{
   //Post an event to notify the console
   wxCommandEvent eventCancelLast(wxEVT_COMMAND_ENTER);
   eventCancelLast.SetInt(-3);
   wxPostEvent(Console, eventCancelLast);
   DATC->mouseRotate();
}

tui::LayoutCanvas::~LayoutCanvas(){
   delete crossCur;
   delete zeroMark;
//   delete (laydata::tdtdata::tessellObj);
}

// Code below taken from the internet after nasty troubles with the cursor
// initialization in GTK
//http://cvs.sourceforge.net/viewcvs.py/audacity/audacity-src/src/TrackPanel.cpp?rev=1.218
wxCursor* tui::MakeCursor( const char * pXpm[36],  int HotX, int HotY ) {
   wxCursor * pCursor;
   const int HotAdjust =0;

   wxImage Image = wxImage(wxBitmap(pXpm).ConvertToImage());   
   Image.SetMaskColour(255,0,0);
   Image.SetMask();// Enable mask.

#ifdef __WXGTK__
   //
   // Kludge: the wxCursor Image constructor is broken in wxGTK.
   // This code, based loosely on the broken code from the wxGTK source,
   // works around the problem by constructing a 1-bit bitmap and
   // calling the other custom cursor constructor.
   //
   // -DMM
   //

   unsigned char *rgbBits = Image.GetData();
   int w = Image.GetWidth() ;
   int h = Image.GetHeight();
   int imagebitcount = (w*h)/8;
   
   unsigned char *bits = DEBUG_NEW unsigned char [imagebitcount];
   unsigned char *maskBits = DEBUG_NEW unsigned char [imagebitcount];

   int i, j, i8;
   unsigned char cMask;
   for (i=0; i<imagebitcount; i++) {
      bits[i] = 0;
      i8 = i * 8;

      cMask = 1;
      for (j=0; j<8; j++) {
         if (rgbBits[(i8+j)*3+2] < 127)
            bits[i] = bits[i] | cMask;
         cMask = cMask * 2;
      }
   }

   for (i=0; i<imagebitcount; i++) {
      maskBits[i] = 0x0;
      i8 = i * 8;

      cMask = 1;
      for (j=0; j<8; j++) {
         if (rgbBits[(i8+j)*3] < 127 || rgbBits[(i8+j)*3+1] > 127)
            maskBits[i] = maskBits[i] | cMask;
         cMask = cMask * 2;
      }
   }

   wxColour* col_black = DEBUG_NEW wxColour(  0,   0,   0);
   wxColour* col_white = DEBUG_NEW wxColour(255, 255, 255);

   pCursor = DEBUG_NEW wxCursor((const char *)bits, w, h,
                          HotX-HotAdjust, HotY-HotAdjust,
                          (const char *)maskBits,
                          col_black,
                          col_white);

   delete [] bits;
   delete [] maskBits;
   delete col_black;
   delete col_white;
#else 
   Image.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, HotX-HotAdjust );
   Image.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, HotY-HotAdjust );
   pCursor = DEBUG_NEW wxCursor( Image );
#endif

   return pCursor;
}
