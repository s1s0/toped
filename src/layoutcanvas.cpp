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
#include "trend.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::TllCmdLine*      Console;
extern trend::ogl_logfile        OGLLogFile; // openGL call tracking log file
extern trend::TrendCenter*       TRENDC;
extern tui::TopedFrame*          Toped;

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

tui::TpdOglContext::TpdOglContext(wxGLCanvas* canvas, wxGLContextAttrs* attr) :
   wxGLContext               ( canvas,nullptr,attr),
   _oglVersion14             ( false      ),
   _oglVersion33             ( false      ),
   _oglExtMultiDrawArrays    ( false      ),
   _oglArbVertexBufferObject ( false      ),
   _useVboRendering          ( false      ),
   _useShaders               ( false      ),
   _glewInitDone             ( false      ),
   _ww                       ( 0          ),
   _wh                       ( 0          )
{
   GLint flags;
   DBGL_CALL(glGetIntegerv,GL_CONTEXT_FLAGS, &flags)

   if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
      OGLLogFile << "OpenGL debug context is available"; OGLLogFile.flush();
   }
   else {
      OGLLogFile << "OpenGL debug context is not available"; OGLLogFile.flush();
   }
}

void tui::TpdOglContext::glewContext(LayoutCanvas* canvas)
{
   canvas->SetCurrent(*this);
   GLenum err = glewInit();
   OGLLogFile << "Status: Using GLEW " << reinterpret_cast<const char *>(glewGetString(GLEW_VERSION));
   OGLLogFile.flush();
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
      _oglVersion33             = (0 != glewIsSupported("GL_VERSION_3_3"));
      _oglExtMultiDrawArrays    = (0 != glewIsSupported("GL_EXT_multi_draw_arrays"));
      _oglArbVertexBufferObject = (0 != glewIsSupported("GL_ARB_vertex_buffer_object"));
      _useVboRendering = _oglVersion14 && _oglExtMultiDrawArrays && _oglArbVertexBufferObject;
      _useShaders               = _oglVersion33;
      _glewInitDone             = true;
   }
//    wxLogDebug("OpenGL version: %s", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
   OGLLogFile << "OpenGL version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
   OGLLogFile.flush();
//    wxLogDebug("OpenGL vendor: %s", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
   OGLLogFile << "OpenGL vendor: " << reinterpret_cast<const char *>(glGetString(GL_VENDOR));
   OGLLogFile.flush();
}

void tui::TpdOglContext::printStatus() const
{
   tell_log(console::MT_INFO,"GLEW diagnostics:");
   if      (_oglVersion14)
      tell_log(console::MT_INFO,"OpenGL version 1.4 supported");
   else
      tell_log(console::MT_INFO,"OpenGL version 1.4 is not supported");
   if      (_oglVersion33)
      tell_log(console::MT_INFO,"OpenGL version 3.3 supported");
   else
      tell_log(console::MT_INFO,"OpenGL version 3.3 is not supported");
   if (_oglArbVertexBufferObject)
      tell_log(console::MT_INFO,"OpenGL implementation supports Vertex Buffer Objects");
   else
      tell_log(console::MT_INFO,"OpenGL implementation doesn't support Vertex Buffer Objects");
   if (_oglExtMultiDrawArrays)
      tell_log(console::MT_INFO,"OpenGL implementation supports Multi Draw Arrays");
   else
      tell_log(console::MT_INFO,"OpenGL implementation doesn't support Multi Draw Arrays");
}

bool tui::TpdOglContext::resizeGL(int w, int h)
{
   if (_glewInitDone && ((_ww != w) || (_wh != h)))
   {
      _ww = w; _wh = h;
      glViewport( 0, 0, (GLint)w, (GLint)h );
      return true;
   }
   else return false;
}

bool tui::TpdOglContext::initFrameBuffer()
{
   clearFrameBuffer();

   DBGL_CALL(glGenFramebuffers, 1, &_fbProps.FBO)
   DBGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, _fbProps.FBO)
   // create a color attachment texture
   DBGL_CALL(glGenTextures, 1, &_fbProps.texture)
printf("Frame buffer %2d generated\n",_fbProps.FBO);

   DBGL_CALL(glBindTexture, GL_TEXTURE_2D, _fbProps.texture)
   DBGL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGB, _ww, _wh, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr)
   DBGL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
   DBGL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fbProps.texture, 0)
   // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
   DBGL_CALL(glGenRenderbuffers, 1, &_fbProps.RBO);
   DBGL_CALL(glBindRenderbuffer,GL_RENDERBUFFER, _fbProps.RBO)
   DBGL_CALL(glRenderbufferStorage,GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _ww, _wh) // use a single renderbuffer object for both a depth AND stencil buffer.
   DBGL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _fbProps.RBO) // now actually attach it
   // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
      return false;
   }
   return true;
}

void tui::TpdOglContext::windowVAO(const ANIVX4& wndCoords, const ANIVX4& texCoords)
{
   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
   const std::vector<glm::vec4> quadVertices= {
                // positions   // texCoords
      glm::vec4 (wndCoords[0], texCoords[0])
     ,glm::vec4 (wndCoords[1], texCoords[1])
     ,glm::vec4 (wndCoords[2], texCoords[2])
     ,glm::vec4 (wndCoords[3], texCoords[3])
   };
   // screen quad VAO
   DBGL_CALL(glGenVertexArrays, 1, &_fbProps.quadVAO)
   DBGL_CALL(glGenBuffers, 1, &_fbProps.quadVBO)
   DBGL_CALL(glBindVertexArray,_fbProps.quadVAO)
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _fbProps.quadVBO)
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER, byteSize(quadVertices), &quadVertices[0], GL_STATIC_DRAW)
   DBGL_CALL(glEnableVertexAttribArray, 0)
   DBGL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0)
   DBGL_CALL(glEnableVertexAttribArray, 1)
   DBGL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)))
}

void tui::TpdOglContext::drawFrameBuffer(const ANIVX4& wndCoords, const ANIVX4& texCoords)
{
   windowVAO(wndCoords, texCoords);
   DBGL_CALL(glBindFramebuffer,GL_FRAMEBUFFER, 0)
   DBGL_CALL(glDisable,GL_DEPTH_TEST) // disable depth test so screen-space quad isn't discarded due to depth test.
   // clear all relevant buffers
   DBGL_CALL(glClearColor, 0.0f, 0.0f, 0.0f, 0.0f) // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
   DBGL_CALL(glClear, GL_COLOR_BUFFER_BIT)

   TRENDC->setGlslProg(trend::glslp_FB);
   DBGL_CALL(glBindVertexArray, _fbProps.quadVAO)
   DBGL_CALL(glBindTexture, GL_TEXTURE_2D, _fbProps.texture)   // use the color attachment texture as the texture of the quad plane
   DBGL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4)
}

void tui::TpdOglContext::animateFrameBuffer(const ANIVX4& wndCoords, const ANIVX4& texCoords)
{
//   float K = float(size)/100.0f;
   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
   std::vector<glm::vec4> quadVertices= {
      //          positions     texture coordinates
      glm::vec4 (wndCoords[0], texCoords[0])
     ,glm::vec4 (wndCoords[1], texCoords[1])
     ,glm::vec4 (wndCoords[2], texCoords[2])
     ,glm::vec4 (wndCoords[3], texCoords[3])
   };
   
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _fbProps.quadVBO)
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER, byteSize(quadVertices), &quadVertices[0], GL_DYNAMIC_DRAW)
   
   DBGL_CALL(glClear, GL_COLOR_BUFFER_BIT)
   DBGL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4)
}

void tui::TpdOglContext::clearFrameBuffer()
{
   DBGL_CALL(glDeleteVertexArrays   , 1, &_fbProps.quadVAO  )
   DBGL_CALL(glDeleteBuffers        , 1, &_fbProps.quadVBO  )
   DBGL_CALL(glDeleteTextures       , 1, &_fbProps.texture  )
   DBGL_CALL(glDeleteRenderbuffers  , 1, &_fbProps.RBO      )
   DBGL_CALL(glDeleteFramebuffers   , 1, &_fbProps.FBO      )
   _fbProps = {0,0,0,0,0};
}


//=============================================================================
tui::AnimationData::AnimationData() :
   wxEvtHandler()
  ,_animationTimer  ( this, CPS_ANIMATION_TIMER)
  ,_counter         (   0                      )
  ,_stepBL          ( 0.0f, 0.0f               )
  ,_stepTR          ( 0.0f, 0.0f               )
  ,_wndCoords       ( { TPX(-1.0f, -1.0f)
                       ,TPX( 1.0f, -1.0f)
                       ,TPX(-1.0f,  1.0f)
                       ,TPX( 1.0f,  1.0f) }    )
  ,_texCoords       ( { TPX( 0.0f,  0.0f)
                       ,TPX( 1.0f,  0.0f)
                       ,TPX( 0.0f,  1.0f)
                       ,TPX( 1.0f,  1.0f) }    )
  ,_event           ( wxID_NONE                )
{
}

void tui::AnimationData::setAnimation(const int event, const DBbox& nw, const DBbox& ow)
{
   Bind(wxEVT_TIMER       ,&tui::LayoutCanvas::OnAnimationTimer  , Toped->view() , CPS_ANIMATION_TIMER);
   _counter    = _allSteps;
   _event      = event;

   DBbox newWin = nw; newWin.normalize();
   DBbox oldWin = ow; oldWin.normalize();

   switch (_event)
   {
      case ZOOM_WINDOW :
      case ZOOM_WINDOWM:
      case ZOOM_IN     :
      case ZOOM_OUT    : zooming(newWin, oldWin); break;
      case ZOOM_LEFT   :
      case ZOOM_RIGHT  :
      case ZOOM_UP     :
      case ZOOM_DOWN   : rolling(); break;
      case ZOOM_EMPTY  : /* nothing to do*/ break;
      case ZOOM_REFRESH: /* nothing to do*/ break;
      default: assert(false); break;
   }
   _animationTimer.Start(_timeInterval);
}

void tui::AnimationData::rolling()
{
   float nBLx    = -1.0f;
   float nBLy    = -1.0f;
   float nTRx    =  1.0f;
   float nTRy    =  1.0f;

   float tBLx    =  0.0f;
   float tBLy    =  0.0f;
   float tTRx    =  1.0f;
   float tTRy    =  1.0f;

   float stepBLx =  0.0f;
   float stepBLy =  0.0f;
   float stepTRx =  0.0f;
   float stepTRy =  0.0f;
   
   switch (_event)
   {
      case ZOOM_DOWN     :
         nTRy =  0.0f; tBLy = 0.5f; stepBLy = stepTRy = 1.0f/(float)_allSteps;
         break;
      case ZOOM_UP     :
         nBLy =  0.0f; tTRy = 0.5f; stepBLy = stepTRy = -1.0f/(float)_allSteps;
         break;
      case ZOOM_LEFT   :
         nTRx =  0.0f; tBLx = 0.5f; stepBLx = stepTRx = 1.0f/(float)_allSteps;
         break;
      case ZOOM_RIGHT  :
         nBLx =  0.0f; tTRx = 0.5f; stepBLx = stepTRx = -1.0f/(float)_allSteps;
         break;

      default: assert(false); break;
   }
   _wndCoords = {TPX(nBLx,nBLy), TPX(nTRx,nBLy), TPX(nBLx,nTRy), TPX(nTRx, nTRy)};
   _texCoords = {TPX(tBLx,tBLy), TPX(tTRx,tBLy), TPX(tBLx,tTRy), TPX(tTRx, tTRy)};
   _stepBL = {stepBLx, stepBLy};
   _stepTR = {stepTRx, stepTRy};
}

void tui::AnimationData::zooming(const DBbox& nw, const DBbox& ow)
{
   // figure-out whether the new window overlaps the old window, or vice versa
   DBbox big, small;
   float scK; // scale factor
   float shK; // shift factor
   if       ( ow.inside(nw.p1())  &&  ow.inside(nw.p2()) )
   {
      _event = ZOOM_IN ; big = ow; small = nw; scK = 2.0f; shK = -1.0f;
   }
   else if  ( nw.inside(ow.p1())  &&  nw.inside(ow.p2()) )
   {
      _event = ZOOM_OUT; big = nw; small = ow; scK = 1.0f; shK = 0.0f;
   }
   else return; // not a zoom operation?

   float scaleX = scK / ((float)big.p2().x() - (float)big.p1().x());
   float scaleY = scK / ((float)big.p2().y() - (float)big.p1().y());
   
   float shiftX = -(scaleX * (float)big.p1().x()) + shK;
   float shiftY = -(scaleY * (float)big.p1().y()) + shK;
   
   // new window coordinates converted to ±1 window
   float nBLx = scaleX * (float)small.p1().x() + shiftX;
   float nBLy = scaleY * (float)small.p1().y() + shiftY;
   float nTRx = scaleX * (float)small.p2().x() + shiftX;
   float nTRy = scaleY * (float)small.p2().y() + shiftY;
   
   float stepBLx = ( shK  - nBLx) / _allSteps;
   float stepBLy = ( shK  - nBLy) / _allSteps;
   float stepTRx = ( 1.0f - nTRx) / _allSteps;
   float stepTRy = ( 1.0f - nTRy) / _allSteps;
   //   printf("BL Step=> X: %10f; Y: %10f ||  TR step=> X: %10f; Y: %10f\n", stepBLx, stepBLy, stepTRx, stepTRy);
   _stepBL = {stepBLx, stepBLy};
   _stepTR = {stepTRx, stepTRy};

   if      (ZOOM_IN == _event)
      // on zoomIn - manupulate the screen coordinates
      _wndCoords = {TPX(nBLx,nBLy), TPX(nTRx,nBLy), TPX(nBLx,nTRy), TPX(nTRx, nTRy)};
   else if (ZOOM_OUT == _event)
      // on zoomOut - manipulate texture coordinates
      _texCoords = {TPX(nBLx,nBLy), TPX(nTRx,nBLy), TPX(nBLx,nTRy), TPX(nTRx, nTRy)};
   else assert(false);
}

void tui::AnimationData::stepDown()
{
   if (0 > (_counter--))
   {
      _animationTimer.Stop();
      Unbind(wxEVT_TIMER  ,&tui::LayoutCanvas::OnAnimationTimer  , Toped->view() , CPS_ANIMATION_TIMER);
      _stepBL      = { 0.0f, 0.0f        };
      _stepTR      = { 0.0f, 0.0f        };
      _wndCoords   = { TPX(-1.0f, -1.0f)
                      ,TPX( 1.0f, -1.0f)
                      ,TPX(-1.0f,  1.0f)
                      ,TPX( 1.0f,  1.0f) };
      _texCoords   = { TPX( 0.0f,  0.0f)
                      ,TPX( 1.0f,  0.0f)
                      ,TPX( 0.0f,  1.0f)
                      ,TPX( 1.0f,  1.0f) };
      _event       =  wxID_NONE;
   }
   else
   {
      float nBLx = _wndCoords[0].x;
      float nBLy = _wndCoords[0].y;
      float nTRx = _wndCoords[3].x;
      float nTRy = _wndCoords[3].y;

      float tBLx = _texCoords[0].x;
      float tBLy = _texCoords[0].y;
      float tTRx = _texCoords[3].x;
      float tTRy = _texCoords[3].y;
      switch (_event)
      {
         case ZOOM_IN     :
            // magnify - i.e. manipulate the window coordinates
            nBLx += _stepBL.x;
            nBLy += _stepBL.y;
            nTRx += _stepTR.x;
            nTRy += _stepTR.y;
            break;
         case ZOOM_OUT    :
            tBLx += _stepBL.x;
            tBLy += _stepBL.y;
            tTRx += _stepTR.x;
            tTRy += _stepTR.y;
            break;
         case ZOOM_DOWN   :
            nTRy += _stepTR.y;
            tBLy -= _stepBL.y/2.0f;
            break;
         case ZOOM_UP     :
            nBLy +=  _stepBL.y;
            tTRy -=  _stepTR.y/2.0f;
            break;
         case ZOOM_LEFT   :
            nTRx += _stepTR.x;
            tBLx -= _stepBL.x/2.0f;
            break;
         case ZOOM_RIGHT  :
            nBLx +=  _stepBL.x;
            tTRx -=  _stepTR.x/2.0f;
            break;
//         default: assert(false);
      }
      _wndCoords = {TPX(nBLx,nBLy), TPX(nTRx,nBLy), TPX(nBLx,nTRy), TPX(nTRx, nTRy)};
      _texCoords = {TPX(tBLx,tBLy), TPX(tTRx,tBLy), TPX(tBLx,tTRy), TPX(tTRx, tTRy)};
   }
}

//=============================================================================
// class LayoutCanvas
//=============================================================================
tui::LayoutCanvas::LayoutCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size, wxGLAttributes attr):
   wxGLCanvas(parent, attr, ID_TPD_CANVAS, pos, size, wxFULL_REPAINT_ON_RESIZE, wxT("LayoutCanvas"))
  ,_whRatio         (     1 )
  ,_apTrigger       ( 10    )
  ,_tmpWnd          ( false )
  ,_invalidWindow   ( false )
  ,_mouseInput      ( false )
  ,_rubberBand      ( false )
  ,_restrictedMove  ( false )
  ,_reperX          ( false )
  ,_reperY          ( false )
  ,_longCursor      ( false )
  ,_wndAnimation    ( false )
  ,_rend3D          ( false )
  ,_oglThread       ( false )
  ,_blinkInterval   ( 0     )
  ,_blinkOn         ( false )
  ,_initialised     ( false )
#ifdef __WXGTK__
  ,_xVisual         ( NULL  )
#endif
{
   Bind(wxEVT_PAINT              ,&tui::LayoutCanvas::OnpaintGL         , this);
   Bind(wxEVT_SIZE               ,&tui::LayoutCanvas::OnresizeGL        , this);
   Bind(wxEVT_MOTION             ,&tui::LayoutCanvas::OnMouseMotion     , this);
   Bind(wxEVT_RIGHT_DOWN         ,&tui::LayoutCanvas::OnMouseRightDown  , this);
   Bind(wxEVT_RIGHT_UP           ,&tui::LayoutCanvas::OnMouseRightUp    , this);
   Bind(wxEVT_LEFT_UP            ,&tui::LayoutCanvas::OnMouseLeftUp     , this);
   Bind(wxEVT_LEFT_DCLICK        ,&tui::LayoutCanvas::OnMouseLeftDClick , this);
   Bind(wxEVT_MIDDLE_UP          ,&tui::LayoutCanvas::OnMouseMiddleUp   , this);
   Bind(wxEVT_MOUSEWHEEL         ,&tui::LayoutCanvas::OnMouseWheel      , this);
   Bind(wxEVT_CHAR               ,&tui::LayoutCanvas::OnChar            , this);
   Bind(wxEVT_ERASE_BACKGROUND   ,&tui::LayoutCanvas::OnEraseBackground , this);
   Bind(wxEVT_TIMER              ,&tui::LayoutCanvas::OnTimer           , this);
   Bind(tui::wxEVT_CANVAS_ZOOM   ,&tui::LayoutCanvas::OnZoom            , this);
   Bind(tui::wxEVT_MOUSE_INPUT   ,&tui::LayoutCanvas::OnMouseIN         , this);
   Bind(tui::wxEVT_CANVAS_CURSOR ,&tui::LayoutCanvas::OnCursorType      , this);
   Bind(tui::wxEVT_ANIMATE_ZOOM  ,&tui::LayoutCanvas::OnWndAnimation    , this);
   Bind(tui::wxEVT_REND3D        ,&tui::LayoutCanvas::OnRend3D          , this);
   Bind(tui::wxEVT_DRCDRAWPREP   ,&tui::LayoutCanvas::OnDrcCollect      , this);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMrulerState         ,this ,          CM_RULER);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMchangeLayer        ,this ,          CM_CHLAY);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMcontinue           ,this ,       CM_CONTINUE);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMabort              ,this ,          CM_ABORT);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMcancel             ,this ,    CM_CANCEL_LAST);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMclose              ,this ,          CM_CLOSE);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnRepeatLastCmd        ,this ,          CM_AGAIN);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMFlip               ,this ,           CM_FLIP);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnCMRotate             ,this ,         CM_ROTATE);
   Bind(wxEVT_MENU               ,&LayoutCanvas::OnPanCenter            ,this ,  TMVIEW_PANCENTER);

//   Bind(EVT_MENU               ,&TopedFrame::OnCurrentLayer           ,this ,          CM_CHLAY);
//   Bind(wxEVT_LEFT_DOWN        ,&tui::LayoutCanvas::OnMouseLeftDown   ,this );


   // required Context attributes
    wxGLContextAttrs ctxAttrs;
   ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(4, 4).DebugCtx().EndList();

   // Explicitly create a new rendering context instance for this canvas.
   _glRC = DEBUG_NEW TpdOglContext(this,&ctxAttrs);
    if (!_glRC->IsOK()) {
//        wxMessageBox("OpenGL 3.3 capable driver is not available. Toped graphic engine will not work properly",
//                     "OpenGL version error", wxOK | wxICON_INFORMATION, this);
        delete (_glRC);
        _glRC = nullptr;
        _crossCur = NULL;
        return;
    }
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
#endif
   _blinkTimer.SetOwner(this);
   _crossCur = MakeCursor(crosscursor, 16, 16);
   SetCursor(*_crossCur);
   // Running the openGL drawing in a separate thread - _oglThread
   // This appears to be a bad idea especially on some platforms.
   // Google it for some opinions.
   // The code is there, but I never got it running reliably if at all
   // The option stays for the sake of experiment.
   // DON'T enable it if you're not sure what you're doing!
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
   _glRC->printStatus();
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


void tui::LayoutCanvas::OnresizeGL(wxSizeEvent& /*event*/) {
//   // this is also necessary to update the context on some platforms
//   wxGLCanvas::OnSize(event);
//    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
   int w, h;
   GetClientSize(&w, &h);
   _lpBL = TP(0,0)  * _layCTM;
   _lpTR = TP(w, h) * _layCTM;
   _invalidWindow |= _glRC->resizeGL(w,h);
}

void tui::LayoutCanvas::OnpaintGL(wxPaintEvent& /*event*/)
{
   if (!_initialised) return;
   if (_invalidWindow)
   { // _invalidWindow indicates zooming or refreshing after a tell operation.
      if (_oglThread)
      {
         tui::DrawThread *dthrd = DEBUG_NEW tui::DrawThread(this);
         wxThreadError result = dthrd->Create();
         if (wxTHREAD_NO_ERROR == result)
            dthrd->Run();
         else
            tell_log( console::MT_ERROR, "Can't execute the drawing in a separate thread");
      }
      else if (TRENDC->shaderAvailable())
      {
         _blinkTimer.Stop();
         wxPaintDC dc(this);
         SetCurrent(*_glRC);
        
         GLuint VertexArrayID;
         DBGL_CALL(glGenVertexArrays, 1, &VertexArrayID)
         DBGL_CALL(glBindVertexArray, VertexArrayID)
         updateViewport();

         _glRC->initFrameBuffer();

         if (_rend3D)
            DATC->render3D();
         else
         {  DATC->renderOGLBuffer();
            if (0 == _blinkInterval) DATC->grcDraw();
         }
         DBGL_CALL(glBindVertexArray, 0);
         DBGL_CALL(glDeleteVertexArrays, 1, &VertexArrayID)
         _invalidWindow = false;
         
         drawOGLBuffer();
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
      SetCurrent(*_glRC);

      if (_animation.active())
         _glRC->animateFrameBuffer(_animation.wndCoords(), _animation.texCoords());
      else
         drawOGLBuffer();
      if       (_tmpWnd)              wndPaint();
      SwapBuffers();
   }
}

void tui::LayoutCanvas::wndPaint() 
{
   DATC->zoomDraw(_pressPoint, _nScrMark);
}

void tui::LayoutCanvas::drawOGLBuffer()
{
   _glRC->drawFrameBuffer(_animation.wndCoords(), _animation.texCoords());
   DBlineList repers;
   if (_reperX)
   {
      DBline rX(TP(_lpBL.x(), _scrMark.y()), TP(_lpTR.x(), _scrMark.y()));
      repers.push_back( rX );
   }
   if (_reperY)
   {
      DBline rY(TP(_scrMark.x() , _lpBL.y()), TP(_scrMark.x() , _lpTR.y()));
      repers.push_back( rY );
   }
   if (_rubberBand || _reperX || _reperY)
      DATC->motionDraw(_layCTM, _releasePoint, _nScrMark, _rubberBand, repers);
   
   if (!_rubberBand && PROPC->boldOnHover())
      boldOnHover();
}

void tui::LayoutCanvas::boldOnHover()
{
   DATC->mouseHooverDraw(_scrMark);
}

void tui::LayoutCanvas::cursorControl(bool shift, bool ctl)
{
   // alt key forces free move
   // shift forces restricted move
   if (ctl || !(_rubberBand && (_restrictedMove || shift)))
   {
      _nScrMark = _scrMark; _nScrMarkOld = _scrMarkOld;
      return;
   }
   _nScrMarkOld = _nScrMark;
   int sdX = _scrMark.x() - _releasePoint.x();
   int sdY = _scrMark.y() - _releasePoint.y();
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
      _nScrMark.setX(_scrMark.x() );
   }
   else
   {
      if (_45deg && (dY < 2*dX)) _nScrMark.setX( sign*sdY + _releasePoint.x() );
      else                       _nScrMark.setX( _releasePoint.x() );
      _nScrMark.setY(_scrMark.y() );
   }
}

void tui::LayoutCanvas::updateCoordWin(int coord, CVSSTATUS_TYPE postype, int dcoord, CVSSTATUS_TYPE dpostype) {
   wxString ws;
   wxCommandEvent eventPOSITION(tui::wxEVT_CANVAS_STATUS);
   ws.sprintf(wxT("%9.3f"),coord*PROPC->UU());
   eventPOSITION.SetString(ws);
   eventPOSITION.SetInt(postype);
   wxPostEvent(this, eventPOSITION);
   if (_rubberBand) {
      ws.sprintf(wxT("%9.3f"),dcoord*PROPC->UU());
      eventPOSITION.SetString(ws);
      eventPOSITION.SetInt(dpostype);
      wxPostEvent(this, eventPOSITION);
   }
}

void tui::LayoutCanvas::eventMouseClick(int button)
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

void tui::LayoutCanvas::pointUpdate(int nX, int nY)
{
   _scrMarkOld = _scrMark;
   _scrMark = TP(nX,nY) * _layCTM;
   int4b stepDB = PROPC->stepDB();
   _scrMark.roundTO(stepDB);

   // update movement indicators
   int deltaX = abs(_scrMarkOld.x() - _scrMark.x());
   int deltaY = abs(_scrMarkOld.y() - _scrMark.y());
   if (!(deltaX || deltaY)) return;
   //
   cursorControl(false, false);
   if (deltaX > 0)
      updateCoordWin(_scrMark.x(), CNVS_POS_X, (_nScrMark.x() - _releasePoint.x()), CNVS_DEL_X);
   if (deltaY > 0)
      updateCoordWin(_scrMark.y(), CNVS_POS_Y, (_nScrMark.y() - _releasePoint.y()), CNVS_DEL_Y);
}

void tui::LayoutCanvas::OnMouseMotion(wxMouseEvent& event)
{
   _scrMarkOld = _scrMark;
   // get a current position
   _scrMark = TP(event.GetX(),event.GetY()) * _layCTM ;
   int4b stepDB = PROPC->stepDB();
   _scrMark.roundTO(stepDB);
   if (PROPC->autopan() && _mouseInput && !_invalidWindow)
   {
      CTM LayCTMR(_layCTM.Reversed());
      TP sp_BL     =     _lpBL * LayCTMR;
      TP sp_TR     =     _lpTR * LayCTMR;
      TP s_ScrMARK = _nScrMark * LayCTMR;
      TP nsp;
      if      (abs(s_ScrMARK.x() - sp_BL.x()) < _apTrigger)
      {// move view window left
         wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_LEFT);
         OnZoom(eventZOOM);
         nsp = _scrMark * _layCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(sp_TR.x() - s_ScrMARK.x()) < _apTrigger)
      {// move view window right
         wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_RIGHT);
         OnZoom(eventZOOM);
         nsp = _scrMark * _layCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(sp_BL.y() - s_ScrMARK.y()) < _apTrigger)
      {// move view window up
         wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_UP);
         OnZoom(eventZOOM);
         nsp = _scrMark * _layCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
      else  if(abs(s_ScrMARK.y() - sp_TR.y()) < _apTrigger)
      {// move view window down
         wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(ZOOM_DOWN);
         OnZoom(eventZOOM);
         nsp = _scrMark * _layCTM.Reversed();
         WarpPointer(nsp.x(),nsp.y());return;
      }
   }
   // update movement indicators
   int deltaX = abs(_scrMarkOld.x() - _scrMark.x());
   int deltaY = abs(_scrMarkOld.y() - _scrMark.y());
   if (!(deltaX || deltaY)) return;
   //
   cursorControl(event.ShiftDown(), event.ControlDown());
   if (deltaX > 0)
      updateCoordWin(_scrMark.x(), CNVS_POS_X, (_nScrMark.x() - _releasePoint.x()), CNVS_DEL_X);
   if (deltaY > 0)
      updateCoordWin(_scrMark.y(), CNVS_POS_Y, (_nScrMark.y() - _releasePoint.y()), CNVS_DEL_Y);

//   drawInterim(_ScrMark);
   if (_tmpWnd || _mouseInput || _reperX || _reperY || PROPC->boldOnHover())
      Refresh();
}

void tui::LayoutCanvas::OnMouseRightDown(wxMouseEvent& WXUNUSED(event)) {
   _pressPoint = _scrMark;
   _tmpWnd = true;
}

void tui::LayoutCanvas::OnMouseRightUp(wxMouseEvent& WXUNUSED(event))
{
   _tmpWnd = false;
   int4b stepDB = PROPC->stepDB();
   if ((abs(_pressPoint.x() - _scrMark.x())  > stepDB) ||
       (abs(_pressPoint.y() - _scrMark.y())  > stepDB))   {
      // if dragging ...
      wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
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
      TP s_ScrMARK = _scrMark * _layCTM.Reversed();
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
      eventMouseClick(0);
//   }
}

void tui::LayoutCanvas::OnMouseLeftDClick(wxMouseEvent& event)
{
   wxString ws;
   wxCommandEvent eventMOUSEACCEL(tui::wxEVT_MOUSE_ACCEL);
   ws.sprintf(wxT("{%3.2f,%3.2f}"),_scrMark.x()*PROPC->UU(), _scrMark.y()*PROPC->UU());
   eventMOUSEACCEL.SetString(ws);
   eventMOUSEACCEL.SetInt(event.ShiftDown() ? 0 : 1);
   wxPostEvent(this, eventMOUSEACCEL);
}

void tui::LayoutCanvas::OnMouseMiddleUp(wxMouseEvent& /*event*/)
{
   if (DATC->checkActiveCell())
   {
      TP s_ScrMARK = _scrMark * _layCTM.Reversed();
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
   wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
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
      tmpmtrx.Translate(markerpos * _layCTM - markerpos * _layCTM * tmpmtrx);
      DBbox* box = DEBUG_NEW DBbox( _lpBL, _lpTR );
      (*box) = (*box) * tmpmtrx;
      eventZOOM.SetInt(tui::ZOOM_WINDOW);
      eventZOOM.SetClientData(static_cast<void*>(box));
   }
   OnZoom(eventZOOM);
   pointUpdate(event.GetX(), event.GetY());
}

void tui::LayoutCanvas::OnChar(wxKeyEvent& event)
{
   if (_rend3D)
   {
      wxCommandEvent eventCAMOVE(tui::wxEVT_CANVAS_ZOOM);
      switch(event.GetKeyCode())
      {
         case 'm':eventCAMOVE.SetInt(R3D_CAM_XPLUS ); break;
         case 'n':eventCAMOVE.SetInt(R3D_CAM_XMINUS); break;
         case 'j':eventCAMOVE.SetInt(R3D_CAM_YPLUS ); break;
         case 'h':eventCAMOVE.SetInt(R3D_CAM_YMINUS); break;
         case 'u':eventCAMOVE.SetInt(R3D_CAM_ZPLUS ); break;
         case 'y':eventCAMOVE.SetInt(R3D_CAM_ZMINUS); break;
         case WXK_ESCAPE:((TopedFrame*)this->GetParent())->setActiveCmd();return;
         default : event.Skip(); return;
      }
      OnCameraMove(eventCAMOVE);
   }
   else
   {
      wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
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
      pointUpdate(event.GetX(), event.GetY());
   }
}

void tui::LayoutCanvas::setScrCTM(const DBbox& box)
{
   int Wcl, Hcl;
   GetClientSize(&Wcl,&Hcl);
   // To prevent a loss of precision in the following lines - don't use
   // integer variables (Wcl & Hcl) directly
   double W = Wcl;
   double H = Hcl;
   double w = fabs((double)box.p1().x() - (double)box.p2().x());
   double h = fabs((double)box.p1().y() - (double)box.p2().y());
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
//   sc = (0 == sc) ? 1.0 : sc;
   double tx = (((double)box.p1().x() + (double)box.p2().x()) - W*sc) / 2;
   double ty = (((double)box.p1().y() + (double)box.p2().y()) - H*sc) / 2;
   _layCTM.setCTM( sc, 0.0, 0.0, sc, tx, ty);
   _layCTM.FlipX(((double)box.p1().y() + (double)box.p2().y())/2);  // flip Y coord towards the center
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setScrCTM(_layCTM.Reversed());
   }
   PROPC->unlockDrawProp(drawProp, false);
}

void tui::LayoutCanvas::OnZoom(wxCommandEvent& evt) {
   DBbox* box = NULL;
   switch (evt.GetInt())
   {
      case ZOOM_WINDOW : box = static_cast<DBbox*>(evt.GetClientData());break;
      case ZOOM_WINDOWM: box = DEBUG_NEW DBbox(_pressPoint.x(),_pressPoint.y(),
                                             _scrMark.x(),_scrMark.y());break;
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
   setScrCTM(*box);
   _invalidWindow = true;
   if (_wndAnimation)
      _animation.setAnimation(evt.GetInt(), *box, DBbox(_lpBL, _lpTR));
   delete box;
   Refresh();
}

void tui::LayoutCanvas::updateViewport()
{
   int W, H;
   _glRC->getWSize(W, H);
   _lpBL = TP(0,0)  * _layCTM;
   _lpTR = TP(W, H) * _layCTM;
//   _status_line.update(W, _LayCTM);
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->setClipRegion(DBbox(_lpBL.x(),_lpTR.y(), _lpTR.x(), _lpBL.y()));
   }
   PROPC->unlockDrawProp(drawProp, false);
//   DBGL_CALL(glClearColor,0,0,0,0)
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
         wxCommandEvent eventPOSITION(tui::wxEVT_CANVAS_STATUS);
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
   CTM tmpmtrx;
   TP center((_lpTR.x() + _lpBL.x())/2, (_lpTR.y() + _lpBL.y())/2);
   tmpmtrx.Translate(_scrMark - center);
   DBbox* box = DEBUG_NEW DBbox( _lpBL, _lpTR );
   (*box) = (*box) * tmpmtrx;
   wxCommandEvent eventZOOM(tui::wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   OnZoom(eventZOOM);
}

void tui::LayoutCanvas::OnCursorType(wxCommandEvent& event)
{
   _longCursor = (1 == event.GetInt());
   _reperX = _reperY = _longCursor;
}

void tui::LayoutCanvas::OnWndAnimation(wxCommandEvent& event)
{
   _wndAnimation = (1 == event.GetInt());
}

void tui::LayoutCanvas::OnRend3D(wxCommandEvent& event)
{
   _rend3D = (1 == event.GetInt());
   if (_rend3D)
   {
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         drawProp->resetCamera();
      }
      PROPC->unlockDrawProp(drawProp, false);
   }
}

void tui::LayoutCanvas::OnDrcCollect(wxCommandEvent& event)
{
   int collectType = event.GetInt();
   wxString name = event.GetString();
   DATC->drcCollect(collectType, std::string(name.mb_str(wxConvUTF8)));
}

void tui::LayoutCanvas::OnCMcontinue(wxCommandEvent& WXUNUSED(event))
{
// keep going ... This function is not doing anything really
   return;
}

void tui::LayoutCanvas::OnCMchangeLayer(wxCommandEvent& WXUNUSED(event))
{
   // post an event to the toped.cpp
   wxCommandEvent eventCurLay(tui::wxEVT_CURRENT_LAYER);
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
   _releasePoint = _scrMark;
   eventMouseClick(2);
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
   wxClientDC dc(this);
   if (_blinkOn)
   {
      glAccum(GL_RETURN, 1.0);
      if       (_tmpWnd)              wndPaint();
      drawOGLBuffer();
      if (!_rubberBand && PROPC->boldOnHover()) boldOnHover();
   }
   else
   {
#ifdef _WIN32
      glAccum(GL_RETURN, 1.0);
#endif
      DATC->grcDraw();
   }
   SwapBuffers();
   _blinkOn = !_blinkOn;
}

void tui::LayoutCanvas::OnAnimationTimer(wxTimerEvent& WXUNUSED(event))
{
   _animation.stepDown();
   Refresh(true);
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
      return DEBUG_NEW DBbox( static_cast<int4b>(blX),
                              static_cast<int4b>(blY),
                              static_cast<int4b>(trX),
                              static_cast<int4b>(trY));
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

   return DEBUG_NEW DBbox( static_cast<int4b>(blX),
                           static_cast<int4b>(blY),
                           static_cast<int4b>(trX),
                           static_cast<int4b>(trY));
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
   return DEBUG_NEW DBbox( static_cast<int4b>(trX), _lpBL.y(),
                           static_cast<int4b>(blX), _lpTR.y());
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
   return DEBUG_NEW DBbox( static_cast<int4b>(trX) ,_lpBL.y(),
                           static_cast<int4b>(blX) , _lpTR.y());
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
   return DEBUG_NEW DBbox(_lpBL.x(), static_cast<int4b>(trY),
                          _lpTR.x(), static_cast<int4b>(blY));
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
   return DEBUG_NEW DBbox(_lpBL.x(), static_cast<int4b>(trY),
                          _lpTR.x(), static_cast<int4b>(blY));
}


void tui::LayoutCanvas::OnCameraMove(wxCommandEvent& evt) {
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      drawProp->moveCameraLocation(evt.GetInt());
   }
   PROPC->unlockDrawProp(drawProp, false);
   _invalidWindow = true;
   Refresh();
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
//      wxClientDC dc(_canvas);
//      int W, H;
//      GetClientSize(&W,&H);

//      DBGL_CALL(glMatrixMode, GL_MODELVIEW )
//      DBGL_CALL(glShadeModel, GL_FLAT ) // Single color
      _canvas->updateViewport();
      // CTM matrix stuff
//      DBGL_CALL0(glLoadIdentity)
//      DBGL_CALL(glOrtho,_canvas->_lpBL.x(),_canvas->_lpTR.x(),_canvas->_lpTR.y(),_canvas->_lpBL.y(),-1.0,1.0)
      DBGL_CALL(glClear,GL_COLOR_BUFFER_BIT)
      DBGL_CALL(glEnable,GL_BLEND)
      DBGL_CALL(glBlendFunc,GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
//      DBGL_CALL(glClear,GL_ACCUM_BUFFER_BIT)
      DATC->renderOGLBuffer();    // draw data
//      DBGL_CALL(glAccum,GL_LOAD, 1.0)
      _canvas->_invalidWindow = false;
      _canvas->drawOGLBuffer();
//      if (_canvas->_rubberBand) _canvas->rubberPaint();
//      if (_canvas->_reperX || _canvas->_reperY) _canvas->longCursor();
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

wxCursor* tui::MakeCursor( const char * pXpm[36],  int HotX, int HotY )
{
   wxCursor * pCursor;
   const int HotAdjust =0;

   wxImage Image = wxImage(wxBitmap(pXpm).ConvertToImage());
   Image.SetMaskColour(255,0,0);
   Image.SetMask();// Enable mask.

   Image.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, HotX-HotAdjust );
   Image.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, HotY-HotAdjust );
   pCursor = DEBUG_NEW wxCursor( Image );

   return pCursor;
}

//void  GLAPIENTRY tui::debugMessage(GLenum source,
//                       GLenum type,
//                       GLuint id,
//                       GLenum severity,
//                       GLsizei length,
//                       const GLchar *message,
//                       const void *userParam)
//{
//   wxLogDebug("Blah blah");
//}
