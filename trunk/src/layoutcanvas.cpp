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

#include <math.h>
#include <wx/wx.h>

#if WIN32
#include <wx/image.h>
#endif

#include "layoutcanvas.h"
#include "../tpd_DB/viewprop.h"
#include "datacenter.h"
#include "../tpd_parser/ted_prompt.h"
#include "../tpd_DB/tedat.h"

extern layprop::ViewProperties*  Properties;
extern DataCenter*               DATC;
extern console::ted_cmd*         Console;
extern const wxEventType         wxEVT_MARKERPOSITION;
extern const wxEventType         wxEVT_MOUSE_ACCEL;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CNVSSTATUSLINE;
extern const wxEventType         wxEVT_CANVAS_ZOOM;


#include "../ui/crosscursor.xpm"

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
   EVT_CHAR             ( tui::LayoutCanvas::OnChar)
   EVT_TECUSTOM_COMMAND (wxEVT_CANVAS_ZOOM, wxID_ANY, tui::LayoutCanvas::OnZoom)
   EVT_TECUSTOM_COMMAND (wxEVT_MOUSE_INPUT, wxID_ANY, tui::LayoutCanvas::OnMouseIN)

   EVT_MENU(   CM_CONTINUE, LayoutCanvas::OnCMcontinue      )
   EVT_MENU(      CM_ABORT, LayoutCanvas::OnCMabort         )
   EVT_MENU(CM_CANCEL_LAST, LayoutCanvas::OnCMcancel        )
   EVT_MENU(      CM_CLOSE, LayoutCanvas::OnCMclose         )
   EVT_MENU(      CM_AGAIN, LayoutCanvas::OnRepeatLastCmd   )
END_EVENT_TABLE()

tui::LayoutCanvas::LayoutCanvas(wxWindow *parent, int* attribList): wxGLCanvas(parent,
   ID_TPD_CANVAS, wxDefaultPosition, wxDefaultSize, 0,wxT("LayoutCanvas"), attribList){

   crossCur = MakeCursor(crosscursor,16, 16);
   //crossCur = new wxCursor((const char*)crosscursor,16, 16);
   SetCursor(*crossCur);
   tmp_wnd = false;
   mouse_input = false;
   rubber_band = false;
   restricted_move = false;
   invalid_window = false;
   reperX = false;
   reperY = false;
   initializeGL();
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(ZOOM_EMPTY);
   OnZoom(eventZOOM);
   ap_trigger = 10;
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

void tui::LayoutCanvas::initializeGL() {
   // OpenGL clear to black
//   if (format().rgba()) glClearColor(0.0,0.0,0.0,0.0);
//   else                 glClearIndex(0);
//   glClearColor(0.5,0.5,0.5,0.5);
//    object = makeObject();		// Generate display lists
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
   // The next call needs to be fitted in some kind of GL descructor
   // gluDeleteTess(tessellObj);
   
   // SG REMARK !! Strange -> in wx port glPixelStorei is kind of ignored
   // in this place.
   // it looks like glGetError() fixes the problem. It does not seems to make 
   // any sence - it obviously has something to do with the openGL initialization
   // when invoked by wxGLCanvas
   // The effect is that bitmaps (select/cell/text marks) are not displayed
   // correctly, because default alignment 4 is used
//   GLenum error = glGetError();
   glShadeModel( GL_FLAT ); // Single color
//   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//   error = glGetError();
//   int boza;
//   glGetIntegerv(GL_UNPACK_ALIGNMENT, &boza);
//   boza++;
   glClear(GL_ACCUM_BUFFER_BIT);
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
//   glMatrixMode( GL_PROJECTION );
//   glLoadIdentity();
//   glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 15.0 );
   glMatrixMode( GL_MODELVIEW );
   glClear(GL_ACCUM_BUFFER_BIT);
   invalid_window = true;
}


void tui::LayoutCanvas::OnpaintGL(wxPaintEvent&) {
    wxPaintDC dc(this);
   #ifndef __WXMOTIF__
      if (!GetContext()) return;
   #endif
   SetCurrent();
   update_viewport();
   //@TODO !! Check somewhere that RGBA mode is available!?
   // CTM matrix stuff
   glLoadIdentity();
   glOrtho(lp_BL.x(),lp_TR.x(),lp_TR.y(),lp_BL.y(),-1.0,1.0);
   if (invalid_window || !(tmp_wnd || rubber_band)) {
      // invalid_window indicates zooming. If that is false and the rest two 
      // variables are not set, means that the system request repaint 
      // In both cases - the entire window is redrawn
      glClear(GL_COLOR_BUFFER_BIT);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glClear(GL_ACCUM_BUFFER_BIT);
      Properties->drawGrid();
      DATC->openGL_draw(Properties->drawprop());    // draw data
      glAccum(GL_LOAD, 1.0);
      if (rubber_band) rubber_paint();
      invalid_window = false;
   }
   else if (tmp_wnd) 
      // zooming using the mouse
      wnd_paint();
   else if (n_ScrMARKold != n_ScrMARK) 
      // the only reason to get to this point remains rubber_band == true
      // so the paint will be invoked only if we have something new to show on 
      //the screen
      rubber_paint();
   SwapBuffers();
}

void tui::LayoutCanvas::wnd_paint() {
   glAccum(GL_RETURN, 1.0);
   glColor4f(0.7, 0.7, 0.7, 0.4); // gray
   glDisable(GL_POLYGON_STIPPLE);
   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
   glRecti(presspoint.x(),presspoint.y(), n_ScrMARK.x(), n_ScrMARK.y());
   glDisable(GL_POLYGON_SMOOTH); //- for solid fill
   glEnable(GL_POLYGON_STIPPLE);
}

void tui::LayoutCanvas::rubber_paint() {
   glAccum(GL_RETURN, 1.0);
   DATC->tmp_draw(Properties->drawprop(), releasepoint, n_ScrMARK);
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
   bool _45deg = (Properties->marker_angle() == 45);
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

void tui::LayoutCanvas::UpdateCoordWin(int coord, POSITION_TYPE postype, int dcoord, POSITION_TYPE dpostype) {
   wxString ws;
   wxCommandEvent eventPOSITION(wxEVT_MARKERPOSITION);
   ws.sprintf(wxT("%3.2f"),coord*Properties->UU());
   eventPOSITION.SetString(ws);
   eventPOSITION.SetInt(postype);
   wxPostEvent(this, eventPOSITION);
   if (rubber_band) {
      ws.sprintf(wxT("%3.2f"),dcoord*Properties->UU());
      eventPOSITION.SetString(ws);
      eventPOSITION.SetInt(dpostype);
      wxPostEvent(this, eventPOSITION);
   }
}

void tui::LayoutCanvas::EventMouseClick(int button) {
   wxCommandEvent eventButtonUP(wxEVT_COMMAND_ENTER);

   telldata::ttpnt* ttp = new telldata::ttpnt(releasepoint.x()*Properties->UU(),
                                              releasepoint.y()*Properties->UU());
   //Post an event to notify the console
   eventButtonUP.SetClientData((void*)ttp);
   eventButtonUP.SetInt(button);
   wxPostEvent(Console, eventButtonUP);
   // send the point to the current temporary object in the data base
   if (0 == button) {
      DATC->mousePoint(releasepoint);
   }
}

void tui::LayoutCanvas::OnMouseMotion(wxMouseEvent& event) {
   ScrMARKold = ScrMARK;
   // get a current position
   ScrMARK = TP(event.GetX(),event.GetY()) * _LayCTM ;
   int4b stepDB = Properties->stepDB();
   ScrMARK.roundTO(stepDB);
   if (Properties->autopan() && mouse_input && !invalid_window) {
      CTM LayCTMR(_LayCTM.Reversed());
      TP sp_BL     =     lp_BL * LayCTMR;
      TP sp_TR     =     lp_TR * LayCTMR;
      TP s_ScrMARK = n_ScrMARK * LayCTMR;
      TP nsp;
      if      (abs(s_ScrMARK.x() - sp_BL.x()) < ap_trigger)  {
               wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
               eventZOOM.SetInt(ZOOM_LEFT);
               OnZoom(eventZOOM);
               nsp = ScrMARK * _LayCTM.Reversed();
               WarpPointer(nsp.x(),nsp.y());return;
            }
      else  if(abs(sp_TR.x() - s_ScrMARK.x()) < ap_trigger) {
               wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
               eventZOOM.SetInt(ZOOM_RIGHT);
               OnZoom(eventZOOM);
               nsp = ScrMARK * _LayCTM.Reversed();
               WarpPointer(nsp.x(),nsp.y());return;
            }   
      else  if(abs(sp_BL.y() - s_ScrMARK.y()) < ap_trigger) {
               wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
               eventZOOM.SetInt(ZOOM_UP);
               OnZoom(eventZOOM);
               nsp = ScrMARK * _LayCTM.Reversed();
               WarpPointer(nsp.x(),nsp.y());return;
            }   
      else  if(abs(s_ScrMARK.y() - sp_TR.y()) < ap_trigger) {
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
//   if (event.LeftIsDown() && !mouse_input) {
//      presspoint = ScrMARKold;
//      mouseIN(true);rubber_band = true;
//   }   
//   if (mouse_input && event.LeftIsDown() && !rubber_band)  rubber_band = true;
   //
   CursorControl(event.ShiftDown(), event.ControlDown());
   if (deltaX > 0) 
      UpdateCoordWin(ScrMARK.x(), POS_X, (n_ScrMARK.x() - releasepoint.x()), DEL_X);
   if (deltaY > 0) 
      UpdateCoordWin(ScrMARK.y(), POS_Y, (n_ScrMARK.y() - releasepoint.y()), DEL_Y);
   if ((tmp_wnd || mouse_input)) Refresh();//updateGL();
}
      
void tui::LayoutCanvas::OnMouseRightDown(wxMouseEvent& WXUNUSED(event)) {
   presspoint = ScrMARK;
   tmp_wnd = true;
}   

void tui::LayoutCanvas::OnMouseRightUp(wxMouseEvent& WXUNUSED(event)) {
   tmp_wnd = false;
   int4b stepDB = Properties->stepDB();
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
         switch (Properties->drawprop().currentop()) {
            case layprop::op_dbox:
               if (Console->numpoints() > 0) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               }
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case layprop::op_dpoly:
               if (Console->numpoints() >= 3) {
                  menu.Append(CM_CLOSE, wxT("Close polygon"));
               }
               if (Console->numpoints() > 1) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel last point"));
               }
               else if (Console->numpoints() > 0) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               }
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case layprop::op_dwire:
               if (Console->numpoints() > 1) {
                  menu.Append(CM_CLOSE, wxT("Finish wire"));
               }
               if (Console->numpoints() > 1) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel last point"));
               }
               else if (Console->numpoints() > 0) {
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               }
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
//            case layprop::op_move:
//            case layprop::op_copy:
            default:
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
         }
      }
      else { // no user input expected
         menu.Append(   CM_AGAIN, wxString(Console->lastCommand(), wxConvUTF8));
         menu.Append(TMEDIT_UNDO, wxT("undo"));
         menu.AppendSeparator();
         if (DATC->numselected() > 0) {
            menu.Append(    TMEDIT_MOVE, wxT("move"  ));
            menu.Append(    TMEDIT_COPY, wxT("copy"  ));
            menu.Append(TMEDIT_ROTATE90, wxT("rotate"));
            menu.Append(   TMEDIT_FLIPX, wxT("flipX" ));
            menu.Append(   TMEDIT_FLIPY, wxT("flipY" ));
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

void tui::LayoutCanvas::OnMouseLeftDClick(wxMouseEvent& event) {
   wxString ws;
   wxCommandEvent eventMOUSEACCEL(wxEVT_MOUSE_ACCEL);
   ws.sprintf(wxT("{%3.2f,%3.2f}"),ScrMARK.x()*Properties->UU(), ScrMARK.y()*Properties->UU());
   eventMOUSEACCEL.SetString(ws);
   eventMOUSEACCEL.SetInt(event.ShiftDown() ? 0 : 1);
   wxPostEvent(this, eventMOUSEACCEL);
}

void tui::LayoutCanvas::OnMouseMiddleUp(wxMouseEvent& event) {
   TP s_ScrMARK = ScrMARK * _LayCTM.Reversed();
   wxMenu menu;
   menu.Append(1000, wxT("Menu Item 1"));
   menu.Append(1001, wxT("Menu Item 2"));
   PopupMenu(&menu, wxPoint(s_ScrMARK.x(), s_ScrMARK.y()));
}

void tui::LayoutCanvas::OnChar(wxKeyEvent& event)
{
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   switch(event.GetKeyCode())
   {
   case WXK_LEFT:
   case '4':
      eventZOOM.SetInt(ZOOM_LEFT);
      OnZoom(eventZOOM);
      break;
   case WXK_RIGHT:
   case '6':
      eventZOOM.SetInt(ZOOM_RIGHT);
      OnZoom(eventZOOM);
      break;
   case WXK_UP:
   case '8':
      eventZOOM.SetInt(ZOOM_UP);
      OnZoom(eventZOOM);
      break;
   case WXK_DOWN:
   case '2':
      eventZOOM.SetInt(ZOOM_DOWN);
      OnZoom(eventZOOM);
      break;
   case '+':
      eventZOOM.SetInt(ZOOM_IN);
      OnZoom(eventZOOM);
      break;
   case '-':
      eventZOOM.SetInt(ZOOM_OUT);
      OnZoom(eventZOOM);
      break;
   default:
      event.Skip();
      return;
   }
}

void tui::LayoutCanvas::OnZoom(wxCommandEvent& evt) {
   DBbox* box = NULL;
   switch (evt.GetInt()) {
   case ZOOM_WINDOW : box = static_cast<DBbox*>(evt.GetClientData());break;
   case ZOOM_WINDOWM: box = new DBbox(presspoint.x(),presspoint.y(),
                                            ScrMARK.x(),ScrMARK.y());break;
   case ZOOM_IN     : box = new DBbox((3*lp_BL.x() + lp_TR.x())/4, //in
                            (3*lp_BL.y() + lp_TR.y())/4,
                            (3*lp_TR.x() + lp_BL.x())/4, 
                            (3*lp_TR.y() + lp_BL.y())/4); break;
   case ZOOM_OUT    : box = new DBbox((5*lp_BL.x() - lp_TR.x())/4, //out
                            (5*lp_BL.y() - lp_TR.y())/4,
                            (5*lp_TR.x() - lp_BL.x())/4, 
                            (5*lp_TR.y() - lp_BL.y())/4); break;
   case ZOOM_LEFT   : box = new DBbox((  lp_TR.x() + lp_BL.x())/2, //left
                               lp_BL.y()               ,
                            (3*lp_BL.x() - lp_TR.x())/2, 
                               lp_TR.y()               ); break;
   case ZOOM_RIGHT  : box = new DBbox((3*lp_TR.x() - lp_BL.x())/2, // right
                               lp_BL.y()               ,
                            (  lp_TR.x() + lp_BL.x())/2, 
                               lp_TR.y()               ); break;
   case ZOOM_UP     : box = new DBbox(   lp_BL.x()               , // up
                            (3*lp_BL.y() - lp_TR.y())/2,
                               lp_TR.x()               ,
                            (  lp_TR.y() + lp_BL.y())/2); break;
   case ZOOM_DOWN   : box = new DBbox(   lp_BL.x()               , // down
                            (  lp_TR.y() + lp_BL.y())/2,
                               lp_TR.x()               ,
                            (3*lp_TR.y() - lp_BL.y())/2); break;
   case ZOOM_EMPTY  : box = new DBbox(-10,-10,90,90); break;
   default: assert(false);
   }
   int W, H;
   GetClientSize(&W,&H);
   double w = abs(box->p1().x() - box->p2().x());
   double h = abs(box->p1().y() - box->p2().y());
   double sc = (W/H < w/h) ? w/W : h/H;
   double tx = ((box->p1().x() + box->p2().x()) - W*sc) / 2;
   double ty = ((box->p1().y() + box->p2().y()) - H*sc) / 2;
   _LayCTM.setCTM( sc, 0.0, 0.0, sc, tx, ty);
   _LayCTM.FlipX((box->p1().y() + box->p2().y())/2);  // flip Y coord towards the center
   Properties->setScrCTM(_LayCTM.Reversed());
   invalid_window = true;
   delete box;
   Refresh();
}

void tui::LayoutCanvas::update_viewport() {
   int W, H;
   GetClientSize(&W,&H);
   lp_BL = TP(0,0)  * _LayCTM;
   lp_TR = TP(W, H) * _LayCTM;
   Properties->setClipRegion(DBbox(lp_BL.x(),lp_TR.y(), lp_TR.x(), lp_BL.y()));
   glClearColor(0,0,0,0);
}

void tui::LayoutCanvas::OnMouseIN(wxCommandEvent& evt)
{
   wxCommandEvent eventABORTEN(wxEVT_CNVSSTATUSLINE);
   if (1 == evt.GetExtraLong())
   { // start mouse input
      mouse_input = true;
      Properties->setCurrentOp(evt.GetInt());
      //restricted_move will be true for wire and polygon
      restricted_move = (Properties->marker_angle() != 0) && 
                              ((evt.GetInt() > 0) || (evt.GetInt() == -1));
      eventABORTEN.SetInt(STS_ABORTENABLE);
      reperX = (-4 == evt.GetInt());
      reperY = (-5 == evt.GetInt());
      if (reperX || reperY || (-6 == evt.GetInt()))
         rubber_band = true;
   }
   else
   { // stop mouse input
      mouse_input = false;
      rubber_band = false;
      restricted_move = false;
      reperX = false;
      reperY = false;
      Properties->setCurrentOp(layprop::op_none);
      wxCommandEvent eventPOSITION(wxEVT_MARKERPOSITION);
      eventPOSITION.SetString(wxT(""));
      eventPOSITION.SetInt(DEL_Y);
      wxPostEvent(this, eventPOSITION);
      eventPOSITION.SetInt(DEL_X);
      wxPostEvent(this, eventPOSITION);
      eventABORTEN.SetInt(STS_ABORTDISABLE);
   }
   wxPostEvent(this, eventABORTEN);
}

void tui::LayoutCanvas::OnCMcontinue(wxCommandEvent& WXUNUSED(event)) {
// keep going ... This function is not doing anything really
   return;
}

void tui::LayoutCanvas::OnCMabort(wxCommandEvent& WXUNUSED(event)) {
   wxCommandEvent eventButtonUP(wxEVT_COMMAND_ENTER);
   eventButtonUP.SetClientData((void*)NULL);
   eventButtonUP.SetInt(-1);
   wxPostEvent(Console, eventButtonUP);
   Refresh();
}

void tui::LayoutCanvas::OnCMcancel(wxCommandEvent& WXUNUSED(event)) {
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

void tui::LayoutCanvas::OnCMclose(wxCommandEvent& WXUNUSED(event)) {
   releasepoint = ScrMARK;
   EventMouseClick(2);
}

void tui::LayoutCanvas::OnRepeatLastCmd(wxCommandEvent& WXUNUSED(event)){
   Console->parseCommand(wxString(Console->lastCommand(), wxConvUTF8));
}
tui::LayoutCanvas::~LayoutCanvas(){
   delete crossCur;
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
   
   unsigned char *bits = new unsigned char [imagebitcount];
   unsigned char *maskBits = new unsigned char [imagebitcount];

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

   wxColour* col_black = new wxColour(  0,   0,   0);
   wxColour* col_white = new wxColour(255, 255, 255);

   pCursor = new wxCursor((const char *)bits, w, h,
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
   pCursor = new wxCursor( Image );
#endif

   return pCursor;
}
