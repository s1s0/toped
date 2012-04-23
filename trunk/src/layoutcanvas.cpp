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

#include "tpdph.h"
#include <math.h>
#include <sstream>
#include <wx/wx.h>

#if WIN32
#include <wx/image.h>
#endif

#include "layoutcanvas.h"
#include "toped.h"
#include "viewprop.h"
#include "datacenter.h"
#include "ted_prompt.h"
#include "tedat.h"
#include "tenderer.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::TllCmdLine*      Console;
extern const wxEventType         wxEVT_CANVAS_STATUS;
extern const wxEventType         wxEVT_CANVAS_CURSOR;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_MOUSE_ACCEL;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CURRENT_LAYER;
//-----------------------------------------------------------------------------
// Static members
//-----------------------------------------------------------------------------
wxMutex          tui::DrawThread::_mutex;

#include "../ui/crosscursor.xpm"

//tui::CanvasStatus::CanvasStatus(){};
//void tui::StatusLine::update(const int4b width, const CTM& _LayCTM)
//{
//   _sb_BL = TP(0,0)       * _LayCTM;
//   _sb_TR = TP(width, 30) * _LayCTM;
//
//   DBbox pixelbox = DBbox(TP(),TP(14,14)) * _LayCTM;
//   _scaledpix = ((double)(pixelbox.p2().x()-pixelbox.p1().x()));
//   _cY = TP(width-150, 17) * _LayCTM;
//   _cX = TP(width-300, 17) * _LayCTM;
//   _dY = TP(width-450, 17) * _LayCTM;
//   _dX = TP(width-600, 17) * _LayCTM;
//   _Ycoord = DBbox(TP(width - 130, 28), TP(width -   2, 2)) * _LayCTM;
//   _Xcoord = DBbox(TP(width - 280, 28), TP(width - 162, 2)) * _LayCTM;
//   _wcY = TP(width-120, 16) * _LayCTM;
//   _wcX = TP(width-270, 16) * _LayCTM;
//}
//
//void tui::StatusLine::draw()
//{
//   glColor4f((GLfloat)1,(GLfloat)1,(GLfloat)1,(GLfloat)0.7);
//   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
//   glDisable(GL_POLYGON_STIPPLE);   //- for solid fill
//   glRecti(_sb_TR.x(), _sb_TR.y(), _sb_BL.x(), _sb_BL.y());
//
//   glColor4f(0,0,0,1);
//   glPushMatrix();
//   glTranslatef(_cY.x(), _cY.y(), 0);
//   glScalef(_scaledpix, _scaledpix, 1);
//   glfDrawSolidString("Y:");
//   glPopMatrix();
//
//   glPushMatrix();
//   glTranslatef(_cX.x(), _cX.y(), 0);
//   glScalef(_scaledpix, _scaledpix, 1);
//   glfDrawSolidString("X:");
//   glPopMatrix();
//
//   glPushMatrix();
//   glTranslatef(_dY.x(), _dY.y(), 0);
//   glScalef(_scaledpix, _scaledpix, 1);
//   glfDrawSolidString("dX:");
//   glPopMatrix();
//
//   glPushMatrix();
//   glTranslatef(_dX.x(), _dX.y(), 0);
//   glScalef(_scaledpix, _scaledpix, 1);
//   glfDrawSolidString("dX:");
//   glPopMatrix();
//
//   update_coords(_cp);
//}
//      private:
//         TP             _sb_BL;
//         TP             _sb_TR;
//         real           _scaledpix;
//   };

//void tui::StatusLine::update_coords(const TP& cp)
//{
//   _cp = cp;
//   glColor4f(0,0,0,1);
//   glRecti(_Xcoord.p1().x(), _Xcoord.p1().y(), _Xcoord.p2().x(), _Xcoord.p2().y());
//   glRecti(_Ycoord.p1().x(), _Ycoord.p1().y(), _Ycoord.p2().x(), _Ycoord.p2().y());
//
////   glScissor( _Xcoord.p1().x(), _Xcoord.p1().y(), _Xcoord.p2().x() - _Xcoord.p1().x(), _Xcoord.p2().y() - _Xcoord.p1().y() );
////   glEnable(GL_SCISSOR_TEST);
////
//   glColor4f(0,1,1,1);
//
//   wxString wsX;
//   wsX.sprintf(wxT("%6d"),_cp.x());
//   wxString wsY;
//   wsY.sprintf(wxT("%6d"),_cp.y());
//
////
////   glClear(GL_COLOR_BUFFER_BIT);
// //
//   glPushMatrix();
//   glTranslatef(_wcX.x(), _wcX.y(), 0);
//   glScalef(_scaledpix, _scaledpix, 1);
//   glfDrawSolidString(wsX.mb_str());
//   glPopMatrix();
//
//   glPushMatrix();
//   glTranslatef(_wcY.x(), _wcY.y(), 0);
//   glScalef(_scaledpix, _scaledpix, 1);
//   glfDrawSolidString(wsY.mb_str());
//   glPopMatrix();
////   glDisable(GL_SCISSOR_TEST);
//
//}

tui::TpdOglContext::TpdOglContext(wxGLCanvas* canvas) :
   wxGLContext               ( canvas     ),
   _oglVersion14             ( false      ),
   _oglExtMultiDrawArrays    ( false      ),
   _oglArbVertexBufferObject ( false      ),
   _vboRendering             ( false      ),
   _glewInitDone             ( false      )

{
}

void tui::TpdOglContext::glewContext(LayoutCanvas* canvas)
{
   canvas->SetCurrent(*this);
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
      wxString errmessage(wxT("glewInit() returns an error: "));
      std::string glewerrstr((const char*)glewGetErrorString(err));
      errmessage << wxString(glewerrstr.c_str(), wxConvUTF8);
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(canvas, errmessage, wxT("Toped"),
                    wxOK | wxICON_ERROR);
      dlg1->ShowModal();
      dlg1->Destroy();
   }
   else
   {
      _oglVersion14             = (0 != glewIsSupported("GL_VERSION_1_4"));
      _oglExtMultiDrawArrays    = (0 != glewIsSupported("GL_EXT_multi_draw_arrays"));
      _oglArbVertexBufferObject = (0 != glewIsSupported("GL_ARB_vertex_buffer_object"));
      _vboRendering = _oglVersion14 && _oglExtMultiDrawArrays && _oglArbVertexBufferObject;
      _glewInitDone             = true;
      //@TODO - to avoid the "if" in the subsequent renderer calls
      // setup the renderer - callback function
      // oGLRender = (void(__stdcall *)(const CTM&))&DataCenter::openGlRender;
   }
   TessellPoly::tenderTesel = gluNewTess();
#ifndef WIN32
   gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                   (GLvoid(*)())&TessellPoly::teselBegin);
   gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                   (GLvoid(*)())&TessellPoly::teselVertex);
   gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_END_DATA,
                   (GLvoid(*)())&TessellPoly::teselEnd);
#else
   gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_BEGIN_DATA,
                   (GLvoid(__stdcall *)())&TessellPoly::teselBegin);
   gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_VERTEX_DATA,
                   (GLvoid(__stdcall *)())&TessellPoly::teselVertex);
   gluTessCallback(TessellPoly::tenderTesel, GLU_TESS_END_DATA,
                   (GLvoid(__stdcall *)())&TessellPoly::teselEnd);
#endif
}

void tui::TpdOglContext::printStatus(bool forceBasic) const
{
   if (forceBasic)
   {
      tell_log(console::MT_INFO,"...basic rendering forced from the command line");
   }
   else if (_vboRendering)
   {
      tell_log(console::MT_INFO,"...using VBO rendering");
   }
   else
   {
      if      (!_oglVersion14)
         tell_log(console::MT_WARNING,"OpenGL version 1.4 is not supported");
      else if (!_oglArbVertexBufferObject)
         tell_log(console::MT_WARNING,"OpenGL implementation doesn't support Vertex Buffer Objects");
      else if (!_oglExtMultiDrawArrays)
         tell_log(console::MT_WARNING,"OpenGL implementation doesn't support Multi Draw Arrays");
      tell_log(console::MT_INFO,"...Using basic rendering");
   }
}

bool tui::TpdOglContext::resizeGL(int w, int h)
{
   if (_glewInitDone)
   {
      glViewport( 0, 0, (GLint)w, (GLint)h );
      return true;
   }
   else return false;
}


//=============================================================================
// class LayoutCanvas
//=============================================================================

BEGIN_EVENT_TABLE(tui::LayoutCanvas, wxGLCanvas)
   EVT_PAINT            ( tui::LayoutCanvas::OnpaintGL         )
   EVT_SIZE             ( tui::LayoutCanvas::OnresizeGL        )
   EVT_MOTION           ( tui::LayoutCanvas::OnMouseMotion     )
   EVT_RIGHT_DOWN       ( tui::LayoutCanvas::OnMouseRightDown  )
   EVT_RIGHT_UP         ( tui::LayoutCanvas::OnMouseRightUp    )
   EVT_LEFT_UP          ( tui::LayoutCanvas::OnMouseLeftUp     )
   EVT_LEFT_DCLICK      ( tui::LayoutCanvas::OnMouseLeftDClick )
   EVT_MIDDLE_UP        ( tui::LayoutCanvas::OnMouseMiddleUp   )
   EVT_MOUSEWHEEL       ( tui::LayoutCanvas::OnMouseWheel      )
   EVT_CHAR             ( tui::LayoutCanvas::OnChar            )
   EVT_ERASE_BACKGROUND ( tui::LayoutCanvas::OnEraseBackground )
   EVT_TECUSTOM_COMMAND (wxEVT_CANVAS_ZOOM  , wxID_ANY, tui::LayoutCanvas::OnZoom)
   EVT_TECUSTOM_COMMAND (wxEVT_MOUSE_INPUT  , wxID_ANY, tui::LayoutCanvas::OnMouseIN)
   EVT_TECUSTOM_COMMAND (wxEVT_CANVAS_CURSOR, wxID_ANY, tui::LayoutCanvas::OnCursorType)
   EVT_TIMER            (                     wxID_ANY, tui::LayoutCanvas::OnTimer)

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
//   EVT_LEFT_DOWN        ( tui::LayoutCanvas::OnMouseLeftDown   )

tui::LayoutCanvas::LayoutCanvas(wxWindow *parent, const wxPoint& pos,
     const wxSize& size, int* attribList):
   wxGLCanvas(parent, ID_TPD_CANVAS, attribList, pos, size, wxFULL_REPAINT_ON_RESIZE, wxT("LayoutCanvas"))
{
   // Explicitly create a new rendering context instance for this canvas.
   _glRC = DEBUG_NEW TpdOglContext(this);
//   if (!wxGLCanvas::IsDisplaySupported(attribList)) return;
#ifdef __WXGTK__
   //  Here we'll have to check that we've got what we've asked for. It is
   // quite possible that we can't get the requested GL visual. If that is the case
   // we'll have to abandon the init sequence right here, otherwise Toped will
   // crash.
#if !wxCHECK_VERSION(2,9,0)
   _xVisual = (XVisualInfo*) m_vi;
   if (NULL == _xVisual)
   {
      _crossCur = NULL;
      return;
   }
#endif
   _blinkTimer.SetOwner(this);
#endif
   _crossCur = MakeCursor(crosscursor,16, 16);
//   _crossCur = DEBUG_NEW wxCursor((const char*)crosscursor,16, 16);
   SetCursor(*_crossCur);
   _tmpWnd = false;
   _mouseInput = false;
   _rubberBand = false;
   _restrictedMove = false;
   _invalidWindow = false;
   _reperX = _reperY = _longCursor = false;
   // Running the openGL drawing in a separate thread
   // This appears to be a bad idea especially on some platforms.
   // Google it for some opinions.
   // The code is there, but I never got it running reliably if at all
   // The option stays for the sake of experiment.
   // DON'T enable it if you're not sure what you're doing!
   _oglThread = false;
   _apTrigger = 10;
   _blinkInterval = 0;
   _blinkOn = false;
   _initialised = false;
}

void   tui::LayoutCanvas::showInfo()
{
   std::ostringstream ost1, ost2, ost3;

   const GLubyte *vendor = glGetString(GL_VENDOR);
   const GLubyte *renderer = glGetString(GL_RENDERER);
   const GLubyte *version = glGetString(GL_VERSION);

   ost1<<"Vendor:" << vendor;
   ost2<<"Renderer:" << renderer;
   ost3<<"Version:" << version;
   tell_log(console::MT_INFO,ost1.str());
   tell_log(console::MT_INFO,ost2.str());
   tell_log(console::MT_INFO,ost3.str());

#ifdef WIN32
   HDC hdc =  ::GetDC((HWND) GetHWND());
   std::ostringstream ost;

   PIXELFORMATDESCRIPTOR  pfd;
   //HDC  hdc;
   int  iPixelFormat;

   iPixelFormat = 1;


   // obtain detailed information about
   // the device context's first pixel format
   DescribePixelFormat(hdc, iPixelFormat,
        sizeof(PIXELFORMATDESCRIPTOR), &pfd);

   if((pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
   {
      tell_log(console::MT_INFO,"Program emulation of OpenGL");
      tell_log(console::MT_INFO,"Operation can be extremely slow");
   }

    // Hardware supports only part of all set of functions ( MCD-driver ).
   if((pfd.dwFlags & PFD_GENERIC_FORMAT) && (pfd.dwFlags & PFD_GENERIC_ACCELERATED))
   {
      tell_log(console::MT_INFO,"Program/hardware emulation of OpenGL");
      tell_log(console::MT_INFO,"Some operations can not be accelerated");
   }

   // Full hardware support ( ICD-driver ).
   if( !(pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
   {
      ost<<"Hardware accelerated OpenGL";
      tell_log(console::MT_INFO,ost.str());
   }
#endif
#ifdef __WXGTK__
   std::ostringstream msg;
   msg << "GLX version "<< GetGLXVersion();
   tell_log(console::MT_INFO, msg.str());
#endif
   std::ostringstream glewmsg;
   glewmsg << "Using GLEW " << glewGetString(GLEW_VERSION);
   tell_log(console::MT_INFO, glewmsg.str());
}

void tui::LayoutCanvas::snapshot(byte*& theImage, word& szW, word& szH)
{
   // The idea for this piece of code is taken from the examples given here:
   // http://www.opengl.org/sdk/docs/books/SuperBible/

   // Get the current viewport dimensions
   GLint viewPort[4];         // Viewport in pixels
   glGetIntegerv(GL_VIEWPORT, viewPort);
   szW = viewPort[2];
   szH = viewPort[3];
   // Calculate the image size in bytes
   unsigned long imageSize = szW * 3 * szH;
   // Get the memory chunk
   theImage = DEBUG_NEW byte[imageSize];

   // Now the interesting part
   glPixelStorei(GL_PACK_ALIGNMENT  , 1);
   glPixelStorei(GL_PACK_ROW_LENGTH , 0);
   glPixelStorei(GL_PACK_SKIP_ROWS  , 0);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

   // Store the state of the current read buffer
   GLenum lastBuffer;
   glGetIntegerv(GL_READ_BUFFER, (GLint *)&lastBuffer);
   // Switch the read buffer to the front one
   glReadBuffer(GL_ACCUM);
   // Get the data
   glReadPixels(0, 0, szW, szH, GL_BGR_EXT, GL_UNSIGNED_BYTE, theImage);
   // Put back the state of the current read buffer
   glReadBuffer(lastBuffer);
}


void tui::LayoutCanvas::viewshift()
{
   //@TODO screen sliding. Some rough ideas.
   int Wcl, Hcl;
   const int slide_step = 100;
   GetClientSize(&Wcl,&Hcl);
   wxPaintDC dc(this);
   glAccum(GL_RETURN, 1.0);
//   glReadBuffer(GL_FRONT);
//   glDrawBuffer(GL_BACK);
/*   glRasterPos2i (_lpBL.x(), _lpBL.y());
   glCopyPixels (_lpBL.x() + slide_step, _lpBL.y(), _lpTR.x() - _lpBL.x() -slide_step, _lpTR.y() - _lpBL.y(), GL_COLOR);*/
   glRasterPos2i (1, 1);
   glCopyPixels (slide_step, 0, Wcl - slide_step, Hcl, GL_COLOR);
   glAccum(GL_LOAD, 1.0);
   SwapBuffers();
   /*   slide = false;*/
}


void tui::LayoutCanvas::OnresizeGL(wxSizeEvent& event) {
//   // this is also necessary to update the context on some platforms
//   wxGLCanvas::OnSize(event);
//    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
   int w, h;
   GetClientSize(&w, &h);
   _lpBL = TP(0,0)  * _LayCTM;
   _lpTR = TP(w, h) * _LayCTM;
   _invalidWindow = _glRC->resizeGL(w,h);
}


void tui::LayoutCanvas::OnpaintGL(wxPaintEvent& event)
{
   if (!_initialised) return;
   if (_invalidWindow)
   {
      // _invalidWindow indicates zooming or refreshing after a tell operation.
      if (_oglThread)
      {
         tui::DrawThread *dthrd = DEBUG_NEW tui::DrawThread(this);
         wxThreadError result = dthrd->Create();
         if (wxTHREAD_NO_ERROR == result)
            dthrd->Run();
         else
            tell_log( console::MT_ERROR, "Can't execute the drawing in a separate thread");
      }
      else
      {
         _blinkTimer.Stop();
         wxPaintDC dc(this);
         glMatrixMode( GL_MODELVIEW );
         glShadeModel( GL_FLAT ); // Single color
         update_viewport();
         // CTM matrix stuff
         glLoadIdentity();
         if ((_lpBL.x()!=_lpTR.x()) && (_lpTR.y() != _lpBL.y()))
            glOrtho(_lpBL.x(),_lpTR.x(),_lpTR.y(),_lpBL.y(),-1.0,1.0);
         glClear(GL_COLOR_BUFFER_BIT);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glClear(GL_ACCUM_BUFFER_BIT);
         DATC->render(_LayCTM);
         if (0 == _blinkInterval) DATC->drawFOnly();
         glAccum(GL_LOAD, 1.0);
         _invalidWindow = false;
         if (_rubberBand) rubber_paint();
         if (_reperX || _reperY) longCursor();
         SwapBuffers();
         if (0 < _blinkInterval)
         {
            _blinkOn = false;
            _blinkTimer.Start(_blinkInterval,wxTIMER_CONTINUOUS);
         }
      }
   }
   else
   {
      wxPaintDC dc(this);
      glAccum(GL_RETURN, 1.0);
      if       (_tmpWnd)              wnd_paint();
      else if  (_rubberBand)          rubber_paint();
      else if  (PROPC->boldOnHover()) boldOnHover();
      if (_reperX || _reperY)         longCursor();
      SwapBuffers();
   }
}

void tui::LayoutCanvas::longCursor()
{
   glColor4f(1, 1, 1, .5);
   glBegin(GL_LINES);
   if (_reperX)
   {
      glVertex2i(_lpBL.x(), _ScrMark.y()) ;
      glVertex2i(_lpTR.x(), _ScrMark.y());
   }
   if (_reperY)
   {
      glVertex2i(_ScrMark.x() , _lpBL.y()) ;
      glVertex2i(_ScrMark.x() , _lpTR.y());
   }
   glEnd();
}

// void tui::LayoutCanvas::drawInterim(const TP& cp)
// {
//    wxPaintDC dc(this);
//    #ifndef __WXMOTIF__
//       if (!GetContext()) return;
//    #endif
//    SetCurrent(*_glRC);
//    _status_line.update_coords(cp);
//    SwapBuffers();
// }

void tui::LayoutCanvas::wnd_paint() {
   glColor4f((GLfloat)0.7, (GLfloat)0.7, (GLfloat)0.7, (GLfloat)0.4); // gray
   glDisable(GL_POLYGON_STIPPLE);
   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
   glRecti(_pressPoint.x(),_pressPoint.y(), _nScrMark.x(), _nScrMark.y());
   glDisable(GL_POLYGON_SMOOTH); //- for solid fill
   glEnable(GL_POLYGON_STIPPLE);
}

void tui::LayoutCanvas::rubber_paint()
{
   DATC->motionDraw(_LayCTM, _releasePoint, _nScrMark);
}

void tui::LayoutCanvas::boldOnHover()
{
   DATC->mouseHoover(_ScrMark);
}

void tui::LayoutCanvas::CursorControl(bool shift, bool ctl)
{
   // alt key forces free move
   // shift forces restricted move
   if (ctl || !(_rubberBand && (_restrictedMove || shift)))
   {
      _nScrMark = _ScrMark; _nScrMarkOld = _scrMarkOld;
      return;
   }
   _nScrMarkOld = _nScrMark;
   int sdX = _ScrMark.x() - _releasePoint.x();
   int sdY = _ScrMark.y() - _releasePoint.y();
   int dX = abs(sdX);
   int dY = abs(sdY);
   // The sign actually is the sign of the tangents. To avoid troubles with the division by zero,
   // it is easier and faster to obtain the sign like this
   int sign = (((double)sdX * (double)sdY) >= 0.0) ? 1 : -1;
   bool _45deg = (PROPC->markerAngle() == 45);
   if (dX > dY)
   {
      if (_45deg && (dX < 2*dY)) _nScrMark.setY( sign*sdX + _releasePoint.y() );
      else                       _nScrMark.setY( _releasePoint.y() );
      _nScrMark.setX(_ScrMark.x() );
   }
   else
   {
      if (_45deg && (dY < 2*dX)) _nScrMark.setX( sign*sdY + _releasePoint.x() );
      else                       _nScrMark.setX( _releasePoint.x() );
      _nScrMark.setY(_ScrMark.y() );
   }
}

void tui::LayoutCanvas::UpdateCoordWin(int coord, CVSSTATUS_TYPE postype, int dcoord, CVSSTATUS_TYPE dpostype) {
   wxString ws;
   wxCommandEvent eventPOSITION(wxEVT_CANVAS_STATUS);
   ws.sprintf(wxT("%3.2f"),coord*PROPC->UU());
   eventPOSITION.SetString(ws);
   eventPOSITION.SetInt(postype);
   wxPostEvent(this, eventPOSITION);
   if (_rubberBand) {
      ws.sprintf(wxT("%3.2f"),dcoord*PROPC->UU());
      eventPOSITION.SetString(ws);
      eventPOSITION.SetInt(dpostype);
      wxPostEvent(this, eventPOSITION);
   }
}

void tui::LayoutCanvas::EventMouseClick(int button)
{
   if (_mouseInput)
   {
      wxCommandEvent eventButtonUP(wxEVT_COMMAND_ENTER);
      telldata::TtPnt* ttp = DEBUG_NEW telldata::TtPnt(_releasePoint.x()*PROPC->UU(),
                                                 _releasePoint.y()*PROPC->UU());
      //Post an event to notify the console
      eventButtonUP.SetClientData((void*)ttp);
      eventButtonUP.SetInt(button);
      wxPostEvent(Console, eventButtonUP);
      // send the point to the current temporary object in the data base
      if (0 == button)
         DATC->mousePoint(_releasePoint);
   }
}

void tui::LayoutCanvas::PointUpdate(int nX, int nY)
{
   _scrMarkOld = _ScrMark;
   _ScrMark = TP(nX,nY) * _LayCTM;
   int4b stepDB = PROPC->stepDB();
   _ScrMark.roundTO(stepDB);

   // update movement indicators
   int deltaX = abs(_scrMarkOld.x() - _ScrMark.x());
   int deltaY = abs(_scrMarkOld.y() - _ScrMark.y());
   if (!(deltaX || deltaY)) return;
   //
   CursorControl(false, false);
   if (deltaX > 0)
      UpdateCoordWin(_ScrMark.x(), CNVS_POS_X, (_nScrMark.x() - _releasePoint.x()), CNVS_DEL_X);
   if (deltaY > 0)
      UpdateCoordWin(_ScrMark.y(), CNVS_POS_Y, (_nScrMark.y() - _releasePoint.y()), CNVS_DEL_Y);
}

void tui::LayoutCanvas::OnMouseMotion(wxMouseEvent& event)
{
   _scrMarkOld = _ScrMark;
   // get a current position
   _ScrMark = TP(event.GetX(),event.GetY()) * _LayCTM ;
   int4b stepDB = PROPC->stepDB();
   _ScrMark.roundTO(stepDB);
   if (PROPC->autopan() && _mouseInput && !_invalidWindow)
   {
      CTM LayCTMR(_LayCTM.Reversed());
      TP sp_BL     =     _lpBL * LayCTMR;
      TP sp_TR     =     _lpTR * LayCTMR;
      TP s_ScrMARK = _nScrMark * LayCTMR;
      TP nsp;
      if      (abs(s_ScrMARK.x() - sp_BL.x()) < _apTrigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_LEFT);
         OnZoom(eventZOOM);
         nsp = _ScrMark * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(sp_TR.x() - s_ScrMARK.x()) < _apTrigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_RIGHT);
         OnZoom(eventZOOM);
         nsp = _ScrMark * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(sp_BL.y() - s_ScrMARK.y()) < _apTrigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_UP);
         OnZoom(eventZOOM);
         nsp = _ScrMark * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(s_ScrMARK.y() - sp_TR.y()) < _apTrigger)
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_DOWN);
         OnZoom(eventZOOM);
         nsp = _ScrMark * _LayCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
   }
   // update movement indicators
   int deltaX = abs(_scrMarkOld.x() - _ScrMark.x());
   int deltaY = abs(_scrMarkOld.y() - _ScrMark.y());
   if (!(deltaX || deltaY)) return;
   //
   CursorControl(event.ShiftDown(), event.ControlDown());
   if (deltaX > 0)
      UpdateCoordWin(_ScrMark.x(), CNVS_POS_X, (_nScrMark.x() - _releasePoint.x()), CNVS_DEL_X);
   if (deltaY > 0)
      UpdateCoordWin(_ScrMark.y(), CNVS_POS_Y, (_nScrMark.y() - _releasePoint.y()), CNVS_DEL_Y);

//   drawInterim(_ScrMark);
   if (_tmpWnd || _mouseInput || _reperX || _reperY || PROPC->boldOnHover()) Refresh();//updateGL();
}

void tui::LayoutCanvas::OnMouseRightDown(wxMouseEvent& WXUNUSED(event)) {
   _pressPoint = _ScrMark;
   _tmpWnd = true;
}

void tui::LayoutCanvas::OnMouseRightUp(wxMouseEvent& WXUNUSED(event))
{
   _tmpWnd = false;
   int4b stepDB = PROPC->stepDB();
   if ((abs(_pressPoint.x() - _ScrMark.x())  > stepDB) ||
       (abs(_pressPoint.y() - _ScrMark.y())  > stepDB))   {
      // if dragging ...
      wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
      eventZOOM.SetInt(ZOOM_WINDOWM);
      OnZoom(eventZOOM);
   }
   else
   {
   // Context menu here
      wxMenu menu;
      if ( NULL != Console->_puc)
      {
         console::ACTIVE_OP currentOp = console::op_none;
         layprop::DrawProperties* drawProp;
         if (PROPC->lockDrawProp(drawProp))
         {
            currentOp = drawProp->currentOp();
         }
         PROPC->unlockDrawProp(drawProp, false);
         switch (currentOp)
         {
            case console::op_dbox:
               if (Console->numpoints() > 0)
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               menu.Append(   CM_CHLAY, wxT("Change Layer"));
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case console::op_dpoly:
               if (Console->numpoints() >= 3)
                  menu.Append(CM_CLOSE, wxT("Close polygon"));
               if (Console->numpoints() > 1)
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel last point"));
               else if (Console->numpoints() > 0)
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               if (DATC->drawRuler())
                  menu.Append(   CM_RULER, wxT("Ruler Off"));
               else
                  menu.Append(   CM_RULER, wxT("Ruler On"));
               menu.Append(   CM_CHLAY, wxT("Change Layer"));
               menu.Append(CM_CONTINUE, wxT("Continue"));
               menu.Append(   CM_ABORT, wxT("Abort"));
               break;
            case console::op_dwire:
               if (Console->numpoints() > 1)
                  menu.Append(CM_CLOSE, wxT("Finish wire"));
               if (Console->numpoints() > 1)
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel last point"));
               else if (Console->numpoints() > 0)
                  menu.Append(CM_CANCEL_LAST, wxT("Cancel first point"));
               if (DATC->drawRuler())
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
               break;
         }
      }
      else
      { // no user input expected
         unsigned numSelected = 0;
         bool skipContextMenu = false;
         laydata::TdtLibDir* dbLibDir = NULL;
         if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
         {
            laydata::TdtDesign* tDesign = (*dbLibDir)();
            numSelected = tDesign->numSelected();
         }
         else
         {
            skipContextMenu = true;
         }
         DATC->unlockTDT(dbLibDir);
         if (skipContextMenu) return;
         if (Console->cmdHistoryExists())
         {
            menu.Append(   CM_AGAIN, wxString(Console->lastCommand(), wxConvUTF8));
            menu.Append(TMEDIT_UNDO, wxT("undo"));
            menu.AppendSeparator();
         }
         if (numSelected > 0)
         {
            menu.Append(       TMEDIT_MOVE, wxT("move"  ));
            menu.Append(       TMEDIT_COPY, wxT("copy"  ));
            menu.Append(   TMEDIT_ROTATE90, wxT("rotate"));
            menu.Append(      TMEDIT_FLIPX, wxT("flip vertical" ));
            menu.Append(      TMEDIT_FLIPY, wxT("flip horizontal" ));
            menu.Append(TMSEL_REPORT_SLCTD, wxT("report selected"));
         }
         else
         {
            menu.Append(     TMDRAW_BOX, wxT("box"   ));
            menu.Append(    TMDRAW_POLY, wxT("poly"  ));
            menu.Append(    TMDRAW_WIRE, wxT("wire"  ));
            menu.Append(    TMDRAW_TEXT, wxT("text"  ));
         }
         menu.AppendSeparator();
         menu.Append( TMSEL_SELECT_IN, wxT("select"     ));
         menu.Append(TMSEL_PSELECT_IN, wxT("part select"));
         if (numSelected > 0)
         {
            menu.Append( TMSEL_UNSELECT_IN, wxT("unselect"     ));
            menu.Append(TMSEL_PUNSELECT_IN, wxT("part unselect"));
         }
      }
      TP s_ScrMARK = _ScrMark * _LayCTM.Reversed();
      PopupMenu(&menu, wxPoint(s_ScrMARK.x(), s_ScrMARK.y()));
   }
}

void tui::LayoutCanvas::OnMouseLeftDown(wxMouseEvent& WXUNUSED(event)) {
//   _pressPoint = _ScrMark;
//   mouseIN(true);
}

void tui::LayoutCanvas::OnMouseLeftUp(wxMouseEvent& WXUNUSED(event)) {
//   if ((abs(_pressPoint.x() - _ScrMark.x())  > step) or
//       (abs(_pressPoint.y() - _ScrMark.y())  > step))   {
//      // if dragging ...
//      mouseIN(false);
////      zoom(_pressPoint.x(),_pressPoint.y(),_ScrMark.x(),_ScrMark.y());
//   }
//   else {
      _releasePoint = _nScrMark;
      if (_mouseInput)  _rubberBand = true;
      EventMouseClick(0);
//   }
}

void tui::LayoutCanvas::OnMouseLeftDClick(wxMouseEvent& event)
{
   wxString ws;
   wxCommandEvent eventMOUSEACCEL(wxEVT_MOUSE_ACCEL);
   ws.sprintf(wxT("{%3.2f,%3.2f}"),_ScrMark.x()*PROPC->UU(), _ScrMark.y()*PROPC->UU());
   eventMOUSEACCEL.SetString(ws);
   eventMOUSEACCEL.SetInt(event.ShiftDown() ? 0 : 1);
   wxPostEvent(this, eventMOUSEACCEL);
}

void tui::LayoutCanvas::OnMouseMiddleUp(wxMouseEvent& event)
{
   if (DATC->checkActiveCell())
   {
      TP s_ScrMARK = _ScrMark * _LayCTM.Reversed();
      wxMenu menu;
      menu.Append(TMVIEW_ZOOMALL     , wxT("Zoom All"));
      menu.Append(TMVIEW_ZOOMVISIBLE , wxT("Zoom Visible"));
      menu.Append(TMVIEW_PANCENTER   , wxT("Pan Center"));
      PopupMenu(&menu, wxPoint(s_ScrMARK.x(), s_ScrMARK.y()));
   }
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
      DBbox* box = DEBUG_NEW DBbox( _lpBL, _lpTR );
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
      case WXK_ESCAPE:((TopedFrame*)this->GetParent())->setActiveCmd();return;
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
      case ZOOM_WINDOWM: box = DEBUG_NEW DBbox(_pressPoint.x(),_pressPoint.y(),
                                             _ScrMark.x(),_ScrMark.y());break;
      case ZOOM_IN     : box = zoomIn()   ; break;
      case ZOOM_OUT    : box = zoomOut()  ; break;
      case ZOOM_LEFT   : box = zoomLeft() ; break;
      case ZOOM_RIGHT  : box = zoomRight(); break;
      case ZOOM_UP     : box = zoomUp()   ; break;
      case ZOOM_DOWN   : box = zoomDown() ; break;
      case ZOOM_EMPTY  : box = DEBUG_NEW DBbox(DEFAULT_OVL_BOX);
                        break;
      case ZOOM_REFRESH: _invalidWindow = true; Refresh(); return;
      default: assert(false); break;
   }
   int Wcl, Hcl;
   GetClientSize(&Wcl,&Hcl);
   // To prevent a loss of precision in the following lines - don't use
   // integer variables (Wcl & Hcl) directly
   double W = Wcl;
   double H = Hcl;
   double w = fabs((double)box->p1().x() - (double)box->p2().x());
   double h = fabs((double)box->p1().y() - (double)box->p2().y());
   if (w > (double) MAX_INT4B)
   {
      tell_log(console::MT_WARNING, "Can't zoom any further");
      w = MAX_INT4B;
   }
   if (h > (double) MAX_INT4B)
   {
      tell_log(console::MT_WARNING, "Can't zoom any further");
      h = MAX_INT4B;
   }
   double sc =  ((W/H < w/h) ? w/W : h/H);
   double tx = (((double)box->p1().x() + (double)box->p2().x()) - W*sc) / 2;
   double ty = (((double)box->p1().y() + (double)box->p2().y()) - H*sc) / 2;
   _LayCTM.setCTM( sc, 0.0, 0.0, sc, tx, ty);
   _LayCTM.FlipX(((double)box->p1().y() + (double)box->p2().y())/2);  // flip Y coord towards the center
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setScrCTM(_LayCTM.Reversed());
   }
   PROPC->unlockDrawProp(drawProp, false);
   delete box;
   _invalidWindow = true;
   Refresh();
}

void tui::LayoutCanvas::update_viewport()
{
   int W, H;
   GetClientSize(&W,&H);
   _lpBL = TP(0,0)  * _LayCTM;
   _lpTR = TP(W, H) * _LayCTM;
//   _status_line.update(W, _LayCTM);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setClipRegion(DBbox(_lpBL.x(),_lpTR.y(), _lpTR.x(), _lpBL.y()));
   }
   PROPC->unlockDrawProp(drawProp, false);
   glClearColor(0,0,0,0);
}

void tui::LayoutCanvas::OnMouseIN(wxCommandEvent& evt)
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      if (1 == evt.GetExtraLong())
      { // start mouse input
         _mouseInput = true;
         console::ACTIVE_OP actop;
         if (evt.GetInt() > 0) actop = console::op_dwire;
         else                  actop = (console::ACTIVE_OP)evt.GetInt();
         drawProp->setCurrentOp(actop);
         //_restrictedMove will be true for wire and polygon
         _restrictedMove = (PROPC->markerAngle() != 0) &&
               ((actop > 0) || (actop == console::op_dpoly));
         _reperX = (console::op_flipX == actop) || (_longCursor && (console::op_flipY != actop));
         _reperY = (console::op_flipY == actop) || (_longCursor && (console::op_flipX != actop));
         if (  (console::op_flipX  == actop)
             ||(console::op_flipY  == actop)
             ||(console::op_rotate == actop)
             ||(console::op_cbind  == actop)
             ||(console::op_abind  == actop)
             ||(console::op_tbind  == actop) )
            _rubberBand = true;
         _releasePoint = TP(0,0);
      }
      else
      { // stop mouse input
         _mouseInput = false;
         _rubberBand = false;
         _restrictedMove = false;
         _reperX = _longCursor;
         _reperY = _longCursor;
         drawProp->setCurrentOp(console::op_none);
         wxCommandEvent eventPOSITION(wxEVT_CANVAS_STATUS);
         eventPOSITION.SetString(wxT(""));
         eventPOSITION.SetInt(CNVS_DEL_Y);
         wxPostEvent(this, eventPOSITION);
         eventPOSITION.SetInt(CNVS_DEL_X);
         wxPostEvent(this, eventPOSITION);
      }
   }
   PROPC->unlockDrawProp(drawProp, false);
}


void tui::LayoutCanvas::OnPanCenter(wxCommandEvent&)
{
  //viewshift();
   CTM tmpmtrx;
   TP center((_lpTR.x() + _lpBL.x())/2, (_lpTR.y() + _lpBL.y())/2);
   tmpmtrx.Translate(_ScrMark - center);
   DBbox* box = DEBUG_NEW DBbox( _lpBL, _lpTR );
   (*box) = (*box) * tmpmtrx;
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   OnZoom(eventZOOM);
}

void tui::LayoutCanvas::OnCursorType(wxCommandEvent& event)
{
   _longCursor = (1 == event.GetInt());
   _reperX = _reperY = _longCursor;
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
   DATC->switchDrawRuler(!DATC->drawRuler());
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
      DATC->mousePointCancel(_releasePoint);
   }
}

void tui::LayoutCanvas::OnCMclose(wxCommandEvent& WXUNUSED(event))
{
   _releasePoint = _ScrMark;
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

void tui::LayoutCanvas::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   wxPaintDC dc(this);
   if (_blinkOn)
   {
      glAccum(GL_RETURN, 1.0);
      if       (_tmpWnd)              wnd_paint();
      else if  (_rubberBand)          rubber_paint();
      else if  (PROPC->boldOnHover()) boldOnHover();
      if (_reperX || _reperY)         longCursor();
   }
   else
   {
      DATC->drawFOnly();
   }
   SwapBuffers();
   _blinkOn = !_blinkOn;
}

DBbox* tui::LayoutCanvas::zoomIn()
{
   // The idea here is not to produce a zero box - i.e. to
   // keep the viewing area > 0;
   int8b blX = lround( (3.0*(double)_lpBL.x() + (double)_lpTR.x())/4.0 );
   int8b blY = lround( (3.0*(double)_lpBL.y() + (double)_lpTR.y())/4.0 );
   int8b trX = lround( (3.0*(double)_lpTR.x() + (double)_lpBL.x())/4.0 );
   int8b trY = lround( (3.0*(double)_lpTR.y() + (double)_lpBL.y())/4.0 );
   if ((blX != trX) && (blY != trY))
      return DEBUG_NEW DBbox( blX, blY, trX, trY);
   else // i.e. the resulting box is too small - so return the existing one
   {
      tell_log(console::MT_WARNING, "Can't zoom any further");
      return DEBUG_NEW DBbox( _lpBL, _lpTR);
   }
}

DBbox* tui::LayoutCanvas::zoomOut()
{
   // In this operation we should avoid loss of precision and
   // producing a box which is bigger than the canvas size
   int8b blX = lround( (5.0*(double)_lpBL.x() - (double)_lpTR.x())/4.0 );
   int8b blY = lround( (5.0*(double)_lpBL.y() - (double)_lpTR.y())/4.0 );
   int8b trX = lround( (5.0*(double)_lpTR.x() - (double)_lpBL.x())/4.0 );
   int8b trY = lround( (5.0*(double)_lpTR.y() - (double)_lpBL.y())/4.0 );
   blX = (blX > MAX_INT4B) ? MAX_INT4B :
         (blX < MIN_INT4B) ? MIN_INT4B : blX;
   blY = (blY > MAX_INT4B) ? MAX_INT4B :
         (blY < MIN_INT4B) ? MIN_INT4B : blY;
   trX = (trX > MAX_INT4B) ? MAX_INT4B :
         (trX < MIN_INT4B) ? MIN_INT4B : trX;
   trY = (trY > MAX_INT4B) ? MAX_INT4B :
         (trY < MIN_INT4B) ? MIN_INT4B : trY;

   return DEBUG_NEW DBbox( blX, blY, trX, trY);
}

DBbox* tui::LayoutCanvas::zoomLeft()
{
   // keep the left boundary within the canvas
   int8b trX = lround( (     (double)_lpBL.x() + (double)_lpTR.x())/2.0 );
   int8b blX = lround( ( 3.0*(double)_lpBL.x() - (double)_lpTR.x())/2.0 );
   if (blX < MIN_INT4B)
   {
      trX -= blX - MIN_INT4B;
      blX = MIN_INT4B;
      tell_log(console::MT_WARNING, "Canvas boundary reached");
   }
   return DEBUG_NEW DBbox( trX ,_lpBL.y(), blX, _lpTR.y());
}

DBbox* tui::LayoutCanvas::zoomRight()
{
   // keep the right boundary within the canvas
   int8b trX = lround( ( 3.0*(double)_lpTR.x() - (double)_lpBL.x())/2.0 );
   int8b blX = lround( (     (double)_lpTR.x() + (double)_lpBL.x())/2.0 );
   if (trX > MAX_INT4B)
   {
      blX -= trX - MAX_INT4B;
      trX  = MAX_INT4B;
      tell_log(console::MT_WARNING, "Canvas boundary reached");
   }
   return DEBUG_NEW DBbox( trX ,_lpBL.y(), blX, _lpTR.y());
}

DBbox* tui::LayoutCanvas::zoomUp()
{
   // keep the bottom boundary within the canvas
   int8b trY = lround( ( 3.0*(double)_lpBL.y() - (double)_lpTR.y())/2.0 );
   int8b blY = lround( (     (double)_lpBL.y() + (double)_lpTR.y())/2.0 );
   if (trY > MAX_INT4B)
   {
      blY -= trY - MAX_INT4B;
      trY  = MAX_INT4B;
      tell_log(console::MT_WARNING, "Canvas boundary reached");
   }
   return DEBUG_NEW DBbox(_lpBL.x(), trY, _lpTR.x(), blY);
}

DBbox* tui::LayoutCanvas::zoomDown()
{
   // keep the top boundary within the canvas
   int8b trY = lround( (     (double)_lpTR.y() + (double)_lpBL.y())/2.0 );
   int8b blY = lround( ( 3.0*(double)_lpTR.y() - (double)_lpBL.y())/2.0 );
   if (blY < MIN_INT4B)
   {
      trY -= blY - MIN_INT4B;
      blY  = MIN_INT4B;
      tell_log(console::MT_WARNING, "Canvas boundary reached");
   }
   return DEBUG_NEW DBbox(_lpBL.x(), trY, _lpTR.x(), blY);
}

tui::LayoutCanvas::~LayoutCanvas()
{
   delete _glRC;
   if (NULL != _crossCur) delete _crossCur;
}

void* tui::DrawThread::Entry(/*wxGLContext* glRC*/)
{
   if (wxMUTEX_NO_ERROR == _mutex.TryLock())
   {
      wxClientDC dc(_canvas);
      glMatrixMode( GL_MODELVIEW );
      glShadeModel( GL_FLAT ); // Single color
      _canvas->update_viewport();
      // CTM matrix stuff
      glLoadIdentity();
      glOrtho(_canvas->_lpBL.x(),_canvas->_lpTR.x(),_canvas->_lpTR.y(),_canvas->_lpBL.y(),-1.0,1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glClear(GL_ACCUM_BUFFER_BIT);
      DATC->render(_canvas->_LayCTM);    // draw data
      glAccum(GL_LOAD, 1.0);
      _canvas->_invalidWindow = false;
      if (_canvas->_rubberBand) _canvas->rubber_paint();
      if (_canvas->_reperX || _canvas->_reperY) _canvas->longCursor();
      _canvas->SwapBuffers();
      _mutex.Unlock();
   }
   else
   {
      tell_log( console::MT_ERROR, "GL drawing skipped. Mutex busy.");
      return NULL;
   }
   return NULL;
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
