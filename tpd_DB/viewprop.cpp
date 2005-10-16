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
//           $URL: http://perun/tpd_svn/trunk/toped_wx/tpd_DB/viewprop.cpp $
//  Creation date: Sun Sep 29 2002
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision: 218 $
//          $Date: 2005-09-30 23:04:05 +0100 (Fri, 30 Sep 2005) $
//        $Author: skr $
//===========================================================================

#include <sstream>
#include <GL/gl.h>
#include <GL/glu.h>
#include "viewprop.h"
#include "outbox.h"

GLubyte cell_mark_bmp[30] = {0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x18, 0x18,
                             0x24, 0x48, 0x42, 0x84, 0x81, 0x02, 0x42, 0x84, 0x24, 0x48, 
                             0x18, 0x18, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00};
                                                      
GLubyte text_mark_bmp[30] = {0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x09, 0x20, 0x11, 0x10,
                             0x21, 0x08, 0x41, 0x04, 0x8F, 0xE2, 0x40, 0x04, 0x20, 0x08,
                             0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00};
                                                                                 
GLubyte array_mark_bmp[30]= {0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10,
                             0x20, 0x08, 0x50, 0x0A, 0x8F, 0xE2, 0x44, 0x44, 0x22, 0x88, 
                             0x11, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00};
//*****************************************************************************
// class LayoutGrid
//*****************************************************************************
#define _GRID_LIMIT  5    // if grid step is less than _GRID_LIMIT pixels, grid is hidden
extern const wxEventType         wxEVT_CNVSSTATUSLINE;
       wxMutex                   DBLock;


void layprop::LayoutGrid::Draw(const DrawProperties& drawprop, const real DBscale) {
   int gridstep = (int)rint(_step / DBscale);
   if (_visual && ( abs((int)(drawprop.ScrCTM().a() * gridstep)) > _GRID_LIMIT)) {
      drawprop.setGridColor(_color);
      // set first grid step to be multiply on the step
      TP bl = TP(drawprop.clipRegion().p1().x(),drawprop.clipRegion().p2().y());
      TP tr = TP(drawprop.clipRegion().p2().x(),drawprop.clipRegion().p1().y());
      int signX = (bl.x() > 0) ? 1 : -1;
      int X_is = (int)((rint(abs(bl.x()) / gridstep)) * gridstep * signX);
      int signY = (tr.y() > 0) ? 1 : -1;
      int Y_is = (int)((rint(abs(tr.y()) / gridstep)) * gridstep * signY);
      //... and finaly draw the grid
      glBegin(GL_POINTS);
      for (int i = X_is; i < tr.x()+1; i += gridstep)
         for (int j = Y_is; j < bl.y()+1; j += gridstep)
            glVertex2i(i,j);
      glEnd();
   }
}

//=============================================================================
layprop::DrawProperties::DrawProperties() : _clipRegion(0,0) {
   _blockfill = false;
   _currentop = op_none;
   _drawinglayer = 0;
   _cellmarks_hidden = false;
   _textmarks_hidden = false;
   _refstack = NULL;
}

void layprop::DrawProperties::setGridColor(std::string colname) const{
   tellRGB* gcol = _laycolors.find(colname)->second;
   if (gcol)
      glColor4ub(gcol->red(), gcol->green(), gcol->blue(), gcol->alpha());
   else // put a default gray color if color is not found
      glColor4f(0.5, 0.5, 0.5, 0.5);
}      

void layprop::DrawProperties::setCurrentColor(word layno) {
   _drawinglayer = layno;
   if (_layset.end() != _layset.find(layno)) {
      tellRGB* gcol = _laycolors[_layset[layno]->getcolor()];
      if (gcol) {
         glColor4ub(gcol->red(), gcol->green(), gcol->blue(), gcol->alpha());
         return;
      }   
   }   
   glColor4f(0.5, 0.5, 0.5, 0.5);
}

bool  layprop::DrawProperties::layerHidden(word layno) const {
   if (_layset.end() != _layset.find(layno)) {
      laySetList::const_iterator ilayset = _layset.find(layno);
      return ilayset->second->hidden();
      //return _layset[layno]->hidden(); - see the comment in getCurrentFill
   }      
   return true;
}
   
bool  layprop::DrawProperties::layerLocked(word layno) const {
   if (_layset.end() != _layset.find(layno)) {
      laySetList::const_iterator ilayset = _layset.find(layno);
      return ilayset->second->locked();
      //return _layset[layno]->locked(); - see the comment in getCurrentFill
   }   
   return true;
}

bool layprop::DrawProperties::getCurrentFill() const {
   if ((_layset.end() != _layset.find(_drawinglayer)) && !_blockfill) {
      // The 3 lines below are doing effectively
      // byte* ifill = _layfill[_layset[_drawinglayer]->getfill]
      // but the problem is const stuff (discards qualifiers)
      // They are ugly, but don't know how to do it better
      // Similar fo layerLocked and layerHidden
      laySetList::const_iterator ilayset = _layset.find(_drawinglayer);
      fillMAP::const_iterator ifillset;
      if (_layfill.end() == (ifillset = _layfill.find(ilayset->second->getfill())))
         return false;
      byte* ifill = ifillset->second;
      glEnable(GL_POLYGON_STIPPLE);
//      glEnable(GL_POLYGON_SMOOTH); //- for solid fill
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glPolygonStipple(ifill);
      return true;
   }   
   else return false;
}

void  layprop::DrawProperties::blockfill(laydata::cellrefstack* refstack) {
   _blockfill = true;
   _refstack = refstack;
}

void  layprop::DrawProperties::unblockfill() {
   _blockfill = false;
   _refstack = NULL;
}

void  layprop::DrawProperties::pushref(const laydata::tdtcellref* cref) {
   assert(cref);
   if (_refstack) {
      if (_refstack->empty()) _blockfill = true;
      _refstack->push_front(cref);
   }
}

byte layprop::DrawProperties::popref(const laydata::tdtcellref* cref) {
   assert(cref);
   if (_refstack && !_refstack->empty()) {
      if (_refstack->front() == cref) {
         _refstack->pop_front();
         if (_refstack->empty()) {
            _blockfill = false;
            return 2;
         }   
         else return 1;
      }
   }
   return 0;
}

void layprop::DrawProperties::draw_reference_marks(const TP& p0, const binding_marks mark_type) const {
   GLubyte* the_mark;
   switch (mark_type) {
      case  cell_mark:if (_cellmarks_hidden) return;
                      else the_mark = cell_mark_bmp;break;
      case array_mark:if (_cellmarks_hidden) return;
                      else the_mark = array_mark_bmp;break;
      case  text_mark:if (_textmarks_hidden) return;
                      else the_mark = text_mark_bmp;break;
      default: assert(false);
   }
   glColor4f(1.0, 1.0, 1.0, 0.8);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glRasterPos2i(p0.x(),p0.y());
   glBitmap(16,16,7,7,0,0, the_mark);
}

word layprop::DrawProperties::getlayerNo(std::string name) const {
  for (laySetList::const_iterator CL = _layset.begin(); CL != _layset.end(); CL++) {
    if (name == CL->second->name()) return CL->first;
  }
  return 0;
}

layprop::DrawProperties::~DrawProperties() {
   for (laySetList::iterator LSI = _layset.begin(); LSI != _layset.end(); LSI++)
      delete LSI->second;
   for (colorMAP::iterator CMI = _laycolors.begin(); CMI != _laycolors.end(); CMI++)
      delete CMI->second;
   for (fillMAP::iterator FMI = _layfill.begin(); FMI != _layfill.end(); FMI++)
      delete [] FMI->second;
//   if (NULL != _refstack) delete _refstack; -> deleted in editobject
}

//=============================================================================
layprop::ViewProperties::ViewProperties() {
   addlayer(std::string("_$teLayer_cell"),0,"","");
   _step = 1;_curlay = 1;
   setUU(1);
   _marker_angle = 0;
   _autopan = false;
}

bool layprop::ViewProperties::selectable(word layno) const {
   return (!_drawprop.layerHidden(layno) && !_drawprop.layerLocked(layno));
}

void layprop::ViewProperties::addlayer(std::string name, word layno, std::string col,
                                                             std::string fill) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if ((col != "") && (_drawprop._laycolors.end() == _drawprop._laycolors.find(col))) {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_ERROR,ost.str().c_str());
   }
   if ((fill != "") && (_drawprop._layfill.end() == _drawprop._layfill.find(fill))) {
      std::ostringstream ost;
      ost << "Warning! Fill \""<<fill<<"\" is not defined";
      tell_log(console::MT_ERROR, ost.str().c_str());
   }
   if (_drawprop._layset.end() != _drawprop._layset.find(layno)) {
      delete _drawprop._layset[layno];
      std::ostringstream ost;
      ost << "Warning! Layer "<<layno<<" redefined";
      tell_log(console::MT_ERROR, ost.str().c_str());
   }   
   _drawprop._layset[layno] = new LayerSettings(name,col,fill);
   DBLock.Unlock();
}

void layprop::ViewProperties::addcolor(std::string name, byte R, byte G, byte B, byte A) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_drawprop._laycolors.end() != _drawprop._laycolors.find(name)) {
      delete _drawprop._laycolors[name];
      std::ostringstream ost;
      ost << "Warning! Color \""<<name<<"\" redefined";
      tell_log(console::MT_ERROR, ost.str().c_str());
   }
   tellRGB* col = new tellRGB(R,G,B,A);
   _drawprop._laycolors[name] = col;
   DBLock.Unlock();
}

void layprop::ViewProperties::addfill(std::string name, byte* ptrn) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_drawprop._layfill.end() != _drawprop._layfill.find(name)) {
      delete [] _drawprop._layfill[name];
      std::ostringstream ost;
      ost << "Warning! Fill \""<<name<<"\" redefined";
      tell_log(console::MT_ERROR, ost.str().c_str());
   }
   _drawprop._layfill[name] = ptrn;
   DBLock.Unlock();
}

void  layprop::ViewProperties::hideLayer(word layno, bool hide) {
   // No error messages here, because of possible range use
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->_hidden = hide;
   DBLock.Unlock();
}

void  layprop::ViewProperties::lockLayer(word layno, bool lock) {
   // No error messages here, because of possible range use
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->_locked = lock;
   DBLock.Unlock();
}

void layprop::ViewProperties::setcellmarks_hidden(bool hide) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      _drawprop._cellmarks_hidden = hide;
   DBLock.Unlock();
}

void layprop::ViewProperties::settextmarks_hidden(bool hide) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      _drawprop._textmarks_hidden = hide;
   DBLock.Unlock();
}

const layprop::LayoutGrid* layprop::ViewProperties::grid(byte No) const {
   if (_grid.end() != _grid.find(No)) {
      gridlist::const_iterator cg = _grid.find(No);
      return cg->second;
   }
   else return NULL;   
}

void layprop::ViewProperties::setGrid(byte No, real step, std::string colname) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_grid.end() != _grid.find(No)) // if this grid No is already defined
      _grid[No]->Init(step,colname);
   else // define a new grid
      _grid[No] = new layprop::LayoutGrid(step, colname);
   DBLock.Unlock();
}

bool layprop::ViewProperties::viewGrid(byte No, bool status) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_grid.end() != _grid.find(No)) 
      _grid[No]->turnover(status);
   else status = false;
   DBLock.Unlock();
   return status;
}

void layprop::ViewProperties::setCurrentOp(int actop) {
   if       (-2 == actop)  _drawprop._currentop = op_move;
   else if  (-3 == actop)  _drawprop._currentop = op_copy;
   else                    _drawprop._currentop = op_none;
}

void layprop::ViewProperties::drawGrid() const{
   typedef gridlist::const_iterator CI;
   for(CI p = _grid.begin(); p != _grid.end(); p++)
      p->second->Draw(_drawprop, _UU);
}

void layprop::ViewProperties::setUU(real UU) {
   _UU = UU;
   _DBscale = 1/UU;
};

layprop::ViewProperties::~ViewProperties() {
   for(gridlist::iterator GI = _grid.begin(); GI != _grid.end(); GI++)
      delete GI->second;
   _grid.clear();
}
