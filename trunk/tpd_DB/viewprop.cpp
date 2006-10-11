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
//        Created: Sun Sep 29 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Canvas visual properties
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#include "../tpd_common/ttt.h"
#include <sstream>
#include <GL/gl.h>
#include <GL/glu.h>
#include "viewprop.h"
#include "../tpd_common/outbox.h"
#include "../tpd_common/glf.h"

GLubyte cell_mark_bmp[30] = {0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x18, 0x18,
                             0x24, 0x48, 0x42, 0x84, 0x81, 0x02, 0x42, 0x84, 0x24, 0x48, 
                             0x18, 0x18, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00};
                                                      
GLubyte text_mark_bmp[30] = {0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x09, 0x20, 0x11, 0x10,
                             0x21, 0x08, 0x41, 0x04, 0x8F, 0xE2, 0x40, 0x04, 0x20, 0x08,
                             0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00};
                                                                                 
GLubyte array_mark_bmp[30]= {0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10,
                             0x20, 0x08, 0x50, 0x0A, 0x8F, 0xE2, 0x44, 0x44, 0x22, 0x88, 
                             0x11, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00};

layprop::ViewProperties*         Properties = NULL;

#define _GRID_LIMIT  5    // if grid step is less than _GRID_LIMIT pixels, grid is hidden
extern const wxEventType         wxEVT_CNVSSTATUSLINE;
       wxMutex                   DBLock;

layprop::SDLine::SDLine(const TP& p1,const TP& p2) : _ln(p1,p2)
{
   _A = _ln.p2().y() - _ln.p1().y();
   _B = _ln.p1().x() - _ln.p2().x();
   _C = -(_A*_ln.p1().x() + _B*_ln.p1().y());
   _length = sqrt(_A*_A + _B*_B);
   std::ostringstream strdist;
   strdist << _length * Properties->UU();
   _value = strdist.str();
   _center = TP((_ln.p1().x() + _ln.p2().x()) / 2, (_ln.p1().y() + _ln.p2().y()) / 2 );
};

void layprop::SDLine::draw(const CTM& LayCTM, const real _step) const
{
   // Side ticks (segments) has to be with constant size. The next 3 lines
   // are generating a segment with the size 15 screen pixels centered in
   // the {0,0} point of the canvas (logical coords)
   // the coeffitients 1e3/1e-3 are picked ramdomly in a try to reduce the
   // error
   DBline tick_sample = DBline(TP(0,0),TP(0,7,1e3)) * LayCTM;
   double tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   DBline long_mark(TP(0,-tick_size, 1e-3),TP(0,tick_size, 1e-3));
   tick_sample = DBline(TP(0,0),TP(0,3,1e3)) * LayCTM;
   tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   DBline short_mark(TP(0,-tick_size, 1e-3),TP(0,tick_size, 1e-3));

   LineList noni_list;
   nonius(short_mark, long_mark, _step, noni_list);

   glColor4f(1, 1, 1, 0.7); // gray
   glDisable(GL_POLYGON_STIPPLE);
   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
   glBegin(GL_LINE);glLineWidth(2);
   for (LineList::const_iterator CL = noni_list.begin(); CL != noni_list.end(); CL++)
   {
      glVertex2i(CL->p1().x(),CL->p1().y()); 
      glVertex2i(CL->p2().x(),CL->p2().y());
   }
   glVertex2i(_ln.p1().x(), _ln.p1().y());
   glVertex2i(_ln.p2().x(), _ln.p2().y());
   glEnd();
   glLineWidth(1);
   DBbox pixelbox = DBbox(TP(),TP(15,15)) * LayCTM;
   double scaledpix = ((double)(pixelbox.p2().x()-pixelbox.p1().x()));

   glColor4f(1, 1, 1, 0.7); // gray
   glPushMatrix();
   glTranslatef(_center.x(), _center.y(), 0);
   glScalef(scaledpix, scaledpix, 1);
   glRotatef(atan2(-_A , _B)* 180.0 / M_PI, 0,0,1);

   glfDrawSolidString(_value.c_str());

   glDisable(GL_POLYGON_SMOOTH); //- for solid fill
   glEnable(GL_POLYGON_STIPPLE);
   glPopMatrix();

//    double x_dist, y_dist;
//    if (_ln.p1().x() == _ln.p2().x())
//       x_dist = _ln.p1().x();
//    else
//       x_dist = _ln.p1().x() - (_step * 1000 * _B) / sqrt(_A*_A + _B*_B);
// 
//    if (_ln.p1().y() == _ln.p2().y())
//       y_dist = _ln.p1().y();
//    else
//       y_dist = _ln.p1().y() + (_step * 1000 * _A) / sqrt(_A*_A + _B*_B);
// 
//    TP one_step(x_dist,y_dist,1);
//   CTM cm;
//   cm.Rotate(90, _ln.p1());
//   TP segl1 = one_step * cm;
//   cm.Rotate(180, _ln.p1());
//   TP segl2 = one_step * cm;

//   CTM cm2;
//   cm2.Translate(_ln.p2().x() - _ln.p1().x(), _ln.p2().y() - _ln.p1().y());
//   TP segl3 = segl1 * cm2;
//   TP segl4 = segl2 * cm2;
//      glVertex2i(segl1.x(), segl1.y());
//      glVertex2i(segl2.x(), segl2.y());
//      glVertex2i(segl4.x(), segl4.y());
//      glVertex2i(segl3.x(), segl3.y());

}

unsigned layprop::SDLine::nonius(const DBline& short_mark, const DBline& long_mark,
                                 real step, LineList& llst) const
{
   // get the angle coefficient of the ruler and calculate the corresponing
   // functions - will be used below
   real angle_rad = atan2(-_A , _B);
   real sinus     = sin(angle_rad);
   real cosinus   = cos(angle_rad);
   real angle     = angle_rad * 180.0 / M_PI;
   //step - converted to DBU
   step *= Properties->DBscale();
   // prepare the translation matrix for the edge point
   CTM tmtrx;
   tmtrx.Rotate(angle);
   tmtrx.Translate(_ln.p2().x(), _ln.p2().y());
   unsigned numtics;
   for( numtics = 0 ; numtics * step < _length ; numtics++ )
   {
      // for each tick - get the deltas ...
      int4b deltaX = (int4b) rint(numtics * step * cosinus);
      int4b deltaY = (int4b) rint(numtics * step * sinus);
      // ... calculate the translation ...
      CTM pmtrx = tmtrx;
      pmtrx.Translate(deltaX, deltaY);
      // ... create a new tick and move it to its position
      if (numtics % 5)
         llst.push_back(DBline(short_mark * pmtrx));
      else
         llst.push_back(DBline(long_mark * pmtrx));
   }
   // don't forget the opposite edge point
   tmtrx.Translate(_ln.p1().x() - _ln.p2().x(), _ln.p1().y() - _ln.p2().y());
   llst.push_back(DBline(long_mark * tmtrx));
   return ++numtics;
}

void layprop::SupplementaryData::addRuler(TP& p1, TP& p2)
{
   _rulers.push_front(SDLine(p1,p2));
}

void layprop::SupplementaryData::clearRulers()
{
   _rulers.clear();
}

void layprop::SupplementaryData::drawRulers(CTM& LayCTM, real step)
{
   for(ruler_collection::const_iterator RA = _rulers.begin(); RA != _rulers.end(); RA++)
      RA->draw(LayCTM, step);
}

//*****************************************************************************
// class LayoutGrid
//*****************************************************************************
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
   _cellmarks_hidden = true;
   _textmarks_hidden = true;
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

bool  layprop::DrawProperties::layerHidden(word layno) const
{
   if (0 == layno) return false;
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
   if (0 == _drawinglayer)
      return true;
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
//      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glPolygonStipple(ifill);
      return true;
   }   
   else return false;
}

void layprop::DrawProperties::setLineProps(bool selected) const
{
   laySetList::const_iterator ilayset;
   if (selected)
   {
      ilayset = _layset.find(_drawinglayer);
      lineMAP::const_iterator ilineset = _lineset.find(ilayset->second->sline());
      if (_lineset.end() != ilineset)
      {
         std::string colorname = ilineset->second->color();
         colorMAP::const_iterator gcol;
         if (("" != colorname) && (_laycolors.end() != (gcol = _laycolors.find(colorname))))
            glColor4ub(gcol->second->red(), gcol->second->green(), gcol->second->blue(), gcol->second->alpha());
         glLineWidth(ilineset->second->width());glEnable(GL_LINE_SMOOTH);glEnable(GL_LINE_STIPPLE);
         glLineStipple(ilineset->second->patscale(),ilineset->second->pattern());
         return;
      }
   }
   if (_layset.end() != (ilayset = _layset.find(_drawinglayer)))
   {
      colorMAP::const_iterator gcol = _laycolors.find(ilayset->second->getcolor());
      if (gcol != _laycolors.end())
         glColor4ub(gcol->second->red(), gcol->second->green(), gcol->second->blue(), gcol->second->alpha());
   }
   else glColor4f(0.5, 0.5, 0.5, 0.5);
   glLineWidth(1);glDisable(GL_LINE_SMOOTH);glDisable(GL_LINE_STIPPLE);
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

void layprop::DrawProperties::draw_reference_marks(const TP& p0, const binding_marks mark_type) const
{
   GLubyte* the_mark;
   switch (mark_type) {
      case  cell_mark:if (_cellmarks_hidden) return;
                      else
                      {
                         glColor4f(1.0, 1.0, 1.0, 0.8);
                         the_mark = cell_mark_bmp;
                         break;
                      }
      case array_mark:if (_cellmarks_hidden) return;
                      else
                      {
                         glColor4f(1.0, 1.0, 1.0, 0.8);
                         the_mark = array_mark_bmp;
                         break;
                      }
      case  text_mark:if (_textmarks_hidden) return;
                      else the_mark = text_mark_bmp;break;
      default: assert(false);
   }
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
   addlayer(std::string("_$teLayer_cell"),0,"","","");
   _step = 1;_curlay = 1;
   setUU(1);
   _marker_angle = 0;
   _autopan = false;
}

bool layprop::ViewProperties::selectable(word layno) const {
   return (!_drawprop.layerHidden(layno) && !_drawprop.layerLocked(layno));
}

void layprop::ViewProperties::addlayer(std::string name, word layno, std::string col,
                                       std::string fill, std::string sline) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if ((col != "") && (_drawprop._laycolors.end() == _drawprop._laycolors.find(col))) {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if ((fill != "") && (_drawprop._layfill.end() == _drawprop._layfill.find(fill))) {
      std::ostringstream ost;
      ost << "Warning! Fill \""<<fill<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   if ((sline != "") && (_drawprop._lineset.end() == _drawprop._lineset.find(sline))) {
      std::ostringstream ost;
      ost << "Warning! Line \""<<sline<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   if (_drawprop._layset.end() != _drawprop._layset.find(layno)) {
      delete _drawprop._layset[layno];
      std::ostringstream ost;
      ost << "Warning! Layer "<<layno<<" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }   
   _drawprop._layset[layno] = new LayerSettings(name,col,fill,sline);
   DBLock.Unlock();
}

void layprop::ViewProperties::addline(std::string name, std::string col, word pattern,
                                       byte patscale, byte width) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if ((col != "") && (_drawprop._laycolors.end() == _drawprop._laycolors.find(col))) {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if (_drawprop._lineset.end() != _drawprop._lineset.find(name)) {
      delete _drawprop._lineset[name];
      std::ostringstream ost;
      ost << "Warning! Line "<< name <<" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   _drawprop._lineset[name] = new LineSettings(col,pattern,patscale,width);
   DBLock.Unlock();
}

void layprop::ViewProperties::addcolor(std::string name, byte R, byte G, byte B, byte A) {
   while (wxMUTEX_NO_ERROR != DBLock.TryLock());
   if (_drawprop._laycolors.end() != _drawprop._laycolors.find(name)) {
      delete _drawprop._laycolors[name];
      std::ostringstream ost;
      ost << "Warning! Color \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
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
      tell_log(console::MT_WARNING, ost.str());
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
   if (actop > 0) _drawprop._currentop = op_dwire;
   else
      switch (actop) {
         case  0: _drawprop._currentop = op_dbox  ;break;
         case -1: _drawprop._currentop = op_dpoly ;break;
         case -2: _drawprop._currentop = op_move  ;break;
         case -3: _drawprop._currentop = op_copy  ;break;
         case -4: _drawprop._currentop = op_flipX ;break;
         case -5: _drawprop._currentop = op_flipY ;break;
         case -6: _drawprop._currentop = op_rotate;break;
         default: _drawprop._currentop = op_none;
      }
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
