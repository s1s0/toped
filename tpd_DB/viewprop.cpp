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

#include "tpdph.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "../tpd_common/ttt.h"
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

const layprop::tellRGB        layprop::DrawProperties::_defaultColor(127,127,127,127);
const layprop::LineSettings   layprop::DrawProperties::_defaultSeline("", 0xffff, 1, 3);
const byte                    layprop::DrawProperties::_defaultFill [128] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define _GRID_LIMIT  5    // if grid step is less than _GRID_LIMIT pixels, grid is hidden

layprop::SDLine::SDLine(const TP& p1,const TP& p2, const real UU) : _ln(p1,p2)
{
   real _A = _ln.p2().y() - _ln.p1().y();
   real _B = _ln.p1().x() - _ln.p2().x();
//   real _C = -(_A*_ln.p1().x() + _B*_ln.p1().y());
   _length = sqrt(_A*_A + _B*_B);
   std::ostringstream strdist;
   strdist << _length * UU;
   _value = strdist.str();
   _center = TP((_ln.p1().x() + _ln.p2().x()) / 2, (_ln.p1().y() + _ln.p2().y()) / 2 );
   // get the angle coefficient of the ruler and calculate the corresponing
   // functions - will be used during the drawing
   real angle_rad = atan2(_A , -_B);
   _sinus     = sin(angle_rad);
   _cosinus   = cos(angle_rad);
   real w_angle     = angle_rad * 180.0 / M_PI;
   // normalized_angle
   _angle = ((w_angle >= 90) || (w_angle < -90)) ? 180 + w_angle : w_angle;

};

void layprop::SDLine::draw(const DBline& long_mark, const DBline& short_mark, const DBline& text_bp, const double scaledpix, const real _step) const
{
   // calculate the nonius ticks
   LineList noni_list;
   nonius(short_mark, long_mark, _step, noni_list);

   glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, (GLfloat)0.7); // gray
   glDisable(GL_POLYGON_STIPPLE);
//   glEnable(GL_POLYGON_SMOOTH);   //- for solid fill
   glBegin(GL_LINES);glLineWidth(2);
   // draw the nonius ...
   for (LineList::const_iterator CL = noni_list.begin(); CL != noni_list.end(); CL++)
   {
      glVertex2i(CL->p1().x(),CL->p1().y()); 
      glVertex2i(CL->p2().x(),CL->p2().y());
   }
   // ... and the ruler itself
   glVertex2i(_ln.p1().x(), _ln.p1().y());
   glVertex2i(_ln.p2().x(), _ln.p2().y());
   glEnd();
   glLineWidth(1);

   CTM tmtrx;
   tmtrx.Rotate(_angle);
   tmtrx.Translate(_center.x(), _center.y());
   DBline central_elevation = text_bp * tmtrx;

   glPushMatrix();
   glTranslatef(central_elevation.p2().x(), central_elevation.p2().y(), 0);
   glScalef(scaledpix, scaledpix, 1);
   glRotatef(_angle, 0, 0, 1);
   
   glfDrawSolidString(_value.c_str());

   glDisable(GL_POLYGON_SMOOTH); //- for solid fill
   glEnable(GL_POLYGON_STIPPLE);
   glPopMatrix();

}

unsigned layprop::SDLine::nonius(const DBline& short_mark, const DBline& long_mark,
                                 const real step, LineList& llst) const
{
   // prepare the translation matrix for the edge point
   CTM tmtrx;
   tmtrx.Rotate(_angle);
   tmtrx.Translate(_ln.p1().x(), _ln.p1().y());
   unsigned numtics;
   for( numtics = 0 ; numtics * step < _length ; numtics++ )
   {
      // for each tick - get the deltas ...
      int4b deltaX = (int4b) rint(numtics * step * _cosinus);
      int4b deltaY = (int4b) rint(numtics * step * _sinus);
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
   tmtrx.Translate(_ln.p2().x() - _ln.p1().x(), _ln.p2().y() - _ln.p1().y());
   llst.push_back(DBline(long_mark * tmtrx));
   return ++numtics;
}

void layprop::SupplementaryData::addRuler(TP& p1, TP& p2, real UU)
{
   _rulers.push_front(SDLine(p1,p2,UU));
}

void layprop::SupplementaryData::clearRulers()
{
   _rulers.clear();
}

void layprop::SupplementaryData::drawRulers(const CTM& LayCTM, real step)
{
   DBline long_mark, short_mark, text_bp;
   double scaledpix;
   getConsts(LayCTM, long_mark, short_mark, text_bp, scaledpix);
   for(ruler_collection::const_iterator RA = _rulers.begin(); RA != _rulers.end(); RA++)
      RA->draw(long_mark, short_mark, text_bp, scaledpix, step);
}

void layprop::SupplementaryData::tmp_draw(const TP& base, const TP& newp, real UU, const CTM& LayCTM, const real _step)
{
   if (_tmp_base)
   {
      DBline long_mark, short_mark, text_bp;
      double scaledpix;
      getConsts(LayCTM, long_mark, short_mark, text_bp, scaledpix);
      SDLine* tmp_ruler = DEBUG_NEW SDLine(base, newp, UU);
      tmp_ruler->draw(long_mark, short_mark, text_bp, scaledpix, _step);
   }
}

void layprop::SupplementaryData::getConsts(const CTM& LayCTM, DBline& long_mark, DBline& short_mark, DBline& text_bp, double& scaledpix)
{
   // Side ticks (segments) of the rulers has to be with constant size. The next lines
   // are generating a segment with the size 7/3 screen pixels centered in
   // the {0,0} point of the canvas (logical coords)
   // the coeffitients 1e3/1e-3 are picked ramdomly attempting to reduce the
   // error
   DBline tick_sample = DBline(TP(0,0),TP(0,7,1e3)) * LayCTM;
   double tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   long_mark = DBline(TP(0,-tick_size, 1e-3),TP(0,tick_size, 1e-3));
   
   tick_sample = DBline(TP(0,0),TP(0,3,1e3)) * LayCTM;
   tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   short_mark = DBline(TP(0,-tick_size, 1e-3),TP(0,tick_size, 1e-3));
   
   tick_sample = DBline(TP(0,0),TP(0,20,1e3)) * LayCTM;
   tick_size = ((double)(tick_sample.p1().y()-tick_sample.p2().y()));
   text_bp = DBline(TP(0,0),TP(0,tick_size, 1e-3));
   
   // now prepare to draw the size
   DBbox pixelbox = DBbox(TP(),TP(15,15)) * LayCTM;
   scaledpix = ((double)(pixelbox.p2().x()-pixelbox.p1().x()));
   
}
void layprop::SupplementaryData::mousePoint(const TP& bp)
{
   if (!_tmp_base)
      _tmp_base = DEBUG_NEW TP(bp);
}

void layprop::SupplementaryData::mouseStop()
{
   if (NULL == _tmp_base)
   {
      delete _tmp_base;
      _tmp_base = NULL;
   }
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
   _currentop = console::op_none;
   _drawinglayer = 0;
   _cellmarks_hidden = true;
   _textmarks_hidden = true;
   _cellbox_hidden = true;
   _textbox_hidden = false;
   _refstack = NULL;
}

void layprop::DrawProperties::setGridColor(std::string colname) const{
   tellRGB* gcol = _laycolors.find(colname)->second;
   if (gcol)
      glColor4ub(gcol->red(), gcol->green(), gcol->blue(), gcol->alpha());
   else // put a default gray color if color is not found
      glColor4ub(_defaultColor.red(), _defaultColor.green(), _defaultColor.blue(), _defaultColor.alpha());
}      

void layprop::DrawProperties::setCurrentColor(word layno)
{
   _drawinglayer = layno;
   if (_layset.end() != _layset.find(layno))
   {
      if (_laycolors.end() != _laycolors.find(_layset[layno]->color()))
      {
         tellRGB* gcol = _laycolors[_layset[layno]->color()];
         if (gcol) {
            glColor4ub(gcol->red(), gcol->green(), gcol->blue(), gcol->alpha());
            return;
         }
      } 
   }
      glColor4ub(_defaultColor.red(), _defaultColor.green(), _defaultColor.blue(), _defaultColor.alpha());
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

bool layprop::DrawProperties::getCurrentFill() const
{
   if (0 == _drawinglayer)
      return true;
   if ((_layset.end() != _layset.find(_drawinglayer)) && !_blockfill)
   {
      // The 3 lines below are doing effectively
      // byte* ifill = _layfill[_layset[_drawinglayer]->getfill]
      // but the problem is const stuff (discards qualifiers)
      // They are ugly, but don't know how to do it better
      // Similar for layerLocked and layerHidden
      laySetList::const_iterator ilayset = _layset.find(_drawinglayer);
      fillMAP::const_iterator ifillset;
      if (_layfill.end() == (ifillset = _layfill.find(ilayset->second->fill())))
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

bool layprop::DrawProperties::getCurrentBoundary() const
{
   if (0 == _drawinglayer)
      return !_cellbox_hidden;
   else
      return !_textbox_hidden;
}

void layprop::DrawProperties::setLineProps(bool selected) const
{
   laySetList::const_iterator ilayset;
   if (selected)
   {
      ilayset = _layset.find(_drawinglayer);
      lineMAP::const_iterator ilineset = _lineset.find(ilayset->second->sline());
      const LineSettings* seline;
      if (_lineset.end() != ilineset)
         seline = ilineset->second;
      else
         seline = &_defaultSeline;
      std::string colorname = seline->color();
      colorMAP::const_iterator gcol;
      if (("" != colorname) && (_laycolors.end() != (gcol = _laycolors.find(colorname))))
         glColor4ub(gcol->second->red(), gcol->second->green(), gcol->second->blue(), gcol->second->alpha());
      glLineWidth(seline->width());glEnable(GL_LINE_SMOOTH);glEnable(GL_LINE_STIPPLE);
      glLineStipple(seline->patscale(),seline->pattern());
      return;
   }
   if (_layset.end() != (ilayset = _layset.find(_drawinglayer)))
   {
      colorMAP::const_iterator gcol = _laycolors.find(ilayset->second->color());
      if (gcol != _laycolors.end())
         glColor4ub(gcol->second->red(), gcol->second->green(), gcol->second->blue(), gcol->second->alpha());
   }
   else
      glColor4ub(_defaultColor.red(), _defaultColor.green(), _defaultColor.blue(), _defaultColor.alpha());

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
                         glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
                         the_mark = cell_mark_bmp;
                         break;
                      }
      case array_mark:if (_cellmarks_hidden) return;
                      else
                      {
                         glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
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

word layprop::DrawProperties::getLayerNo(std::string name) const {
  for (laySetList::const_iterator CL = _layset.begin(); CL != _layset.end(); CL++) {
    if (name == CL->second->name()) return CL->first;
  }
  return 0;
}

std::string layprop::DrawProperties::getLayerName(word layno) const
{
   laySetList::const_iterator CL = _layset.find(layno);
   if (_layset.end() != CL)
   {
      return CL->second->name();
   }
   else return "";
}

std::string layprop::DrawProperties::getColorName(word layno) const
{
   laySetList::const_iterator CL = _layset.find(layno);
   if (_layset.end() != CL)
   {
      return CL->second->color();
   }
   else return "";
}

std::string layprop::DrawProperties::getFillName(word layno) const
{
   laySetList::const_iterator CL = _layset.find(layno);
   if (_layset.end() != CL)
   {
      return CL->second->fill();
   }
   else return "";
}

std::string layprop::DrawProperties::getLineName(word layno) const
{
   laySetList::const_iterator CL = _layset.find(layno);
   if (_layset.end() != CL)
   {
      return CL->second->sline();
   }
   else return "";
}

const layprop::LineSettings* layprop::DrawProperties::getLine(word layno) const
{
   laySetList::const_iterator layer_set = _layset.find(layno);
   if (_layset.end() == layer_set) return &_defaultSeline;
   lineMAP::const_iterator line = _lineset.find(layer_set->second->sline());
   if (_lineset.end() == line) return &_defaultSeline;
   return line->second;
// All the stuff above is equivalent to 
//   return _layfill[_layset[layno]->sline()];
// but is safer and preserves constness
}

const layprop::LineSettings* layprop::DrawProperties::getLine(std::string line_name) const
{
   lineMAP::const_iterator line = _lineset.find(line_name);
   if (_lineset.end() == line) return &_defaultSeline;
   return line->second;
// All the stuff above is equivalent to 
//   return _layfill[_layset[layno]->sline()];
// but is safer and preserves constness
}

const byte* layprop::DrawProperties::getFill(word layno) const
{
   laySetList::const_iterator layer_set = _layset.find(layno);
   if (_layset.end() == layer_set) return &_defaultFill[0];
   fillMAP::const_iterator fill_set = _layfill.find(layer_set->second->fill());
   if (_layfill.end() == fill_set) return &_defaultFill[0];
   return fill_set->second;
// All the stuff above is equivalent to 
//   return _layfill[_layset[layno]->fill()];
// but is safer and preserves constness
}

const byte* layprop::DrawProperties::getFill(std::string fill_name) const
{
   fillMAP::const_iterator fill_set = _layfill.find(fill_name);
   if (_layfill.end() == fill_set) return &_defaultFill[0];
   return fill_set->second;
// All the stuff above is equivalent to 
//   return _layfill[fill_name];
// but is safer and preserves constness
}

const layprop::tellRGB& layprop::DrawProperties::getColor(word layno) const
{
   laySetList::const_iterator layer_set = _layset.find(layno);
   if (_layset.end() == layer_set) return _defaultColor;
   colorMAP::const_iterator col_set = _laycolors.find(layer_set->second->color());
   if (_laycolors.end() == col_set) return _defaultColor;
   return *(col_set->second);
// All the stuff above is equivalent to 
//   return _laycolors[_layset[layno]->color()];
// but is safer and preserves constness
}

const layprop::tellRGB& layprop::DrawProperties::getColor(std::string color_name) const
{
   colorMAP::const_iterator col_set = _laycolors.find(color_name);
   if (_laycolors.end() == col_set) return _defaultColor;
   return *(col_set->second);
// All the stuff above is equivalent to 
//   return _laycolors[color_name];
// but is safer and preserves constness
}

void layprop::DrawProperties::savePatterns(FILE* prop_file) const
{
   fillMAP::const_iterator CI;
   fprintf(prop_file, "void  fillSetup() {\n");
   for( CI = _layfill.begin(); CI != _layfill.end(); CI++)
   {
      fprintf(prop_file, "   int list %s = {\n", CI->first.c_str());
      byte* patdef = CI->second;
      for (byte i = 0; i < 16; i++)
      {
         fprintf(prop_file, "      ");
         for (byte j = 0; j < 8; j++)
         {
            if (127 == i*8+j)
               fprintf(prop_file, "0x%02x  "  , patdef[127]);
            else
               fprintf(prop_file, "0x%02x ,", patdef[i*8+j]);
         }
         fprintf(prop_file, "\n");
      }
      fprintf(prop_file, "   };\n\n");
   }
   for( CI = _layfill.begin(); CI != _layfill.end(); CI++)
   {
      fprintf(prop_file, "   definefill(\"%s\", %s );\n", CI->first.c_str(), CI->first.c_str());
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveColors(FILE* prop_file) const
{
   colorMAP::const_iterator CI;
   fprintf(prop_file, "void  colorSetup() {\n");
   for( CI = _laycolors.begin(); CI != _laycolors.end(); CI++)
   {
      tellRGB* the_color = CI->second;
      fprintf(prop_file, "   definecolor(\"%s\", %3d, %3d, %3d, %3d);\n",
            CI->first.c_str() ,
            the_color->red()  ,
            the_color->green(),
            the_color->blue() ,
            the_color->alpha()
      );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLayers(FILE* prop_file) const
{
   laySetList::const_iterator CI;
   fprintf(prop_file, "void  layerSetup() {\n");
   fprintf(prop_file, "   colorSetup(); fillSetup(); lineSetup();\n");
   for( CI = _layset.begin(); CI != _layset.end(); CI++)
   {
      if (0 == CI->first) continue;
      LayerSettings* the_layer = CI->second;
      fprintf(prop_file, "   layprop(\"%s\", %d , \"%s\", \"%s\", \"%s\");\n",
            the_layer->name().c_str()  ,
            CI->first                  ,
            the_layer->color().c_str() ,
            the_layer->fill().c_str()  ,
            the_layer->sline().c_str()  );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLines(FILE* prop_file) const
{
   lineMAP::const_iterator CI;
   fprintf(prop_file, "void  lineSetup() {\n");
   for( CI = _lineset.begin(); CI != _lineset.end(); CI++)
   {
      LineSettings* the_line = CI->second;
      fprintf(prop_file, "   defineline(\"%s\", \"%s\", 0x%04x , %d, %d);\n",
            CI->first.c_str()         ,
            the_line->color().c_str() ,
            the_line->pattern()       ,
            the_line->patscale()      ,
            the_line->width()            );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::PSwrite(PSFile& psf) const
{
   for(colorMAP::const_iterator CI = _laycolors.begin(); CI != _laycolors.end(); CI++)
   {
      tellRGB* the_color = CI->second;
      psf.defineColor( CI->first.c_str() , the_color->red(),
                       the_color->green(), the_color->blue() );
   }

   for(fillMAP::const_iterator CI = _layfill.begin(); CI != _layfill.end(); CI++)
      psf.defineFill( CI->first.c_str() , CI->second);
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
   _step = 1;
   setUU(1);
   _marker_angle = 0;
   _autopan = false;
   _layselmask = laydata::_lmall;
}

bool layprop::ViewProperties::selectable(word layno) const {
   return (!_drawprop.layerHidden(layno) && !_drawprop.layerLocked(layno));
}

bool layprop::ViewProperties::addlayer(std::string name, word layno, std::string col,
                                       std::string fill, std::string sline)
{
   if ((col != "") && (_drawprop._laycolors.end() == _drawprop._laycolors.find(col)))
   {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if ((fill != "") && (_drawprop._layfill.end() == _drawprop._layfill.find(fill)))
   {
      std::ostringstream ost;
      ost << "Warning! Fill \""<<fill<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   if ((sline != "") && (_drawprop._lineset.end() == _drawprop._lineset.find(sline)))
   {
      std::ostringstream ost;
      ost << "Warning! Line \""<<sline<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   bool new_layer = true;
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
   {
      new_layer = false;
      delete _drawprop._layset[layno];
      std::ostringstream ost;
      ost << "Warning! Layer "<<layno<<" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }   
   _drawprop._layset[layno] = DEBUG_NEW LayerSettings(name,col,fill,sline);
   return new_layer;
}

bool layprop::ViewProperties::addlayer(std::string name, word layno)
{
   if (_drawprop._layset.end() == _drawprop._layset.find(layno))
   {
      _drawprop._layset[layno] = DEBUG_NEW LayerSettings(name,"","","");
      return true;
   }
   return false;
}

void layprop::ViewProperties::addUnpublishedLay(word layno)
{
   _uplaylist.push_back(layno);
}


void layprop::ViewProperties::addline(std::string name, std::string col, word pattern,
                                       byte patscale, byte width) {
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
   _drawprop._lineset[name] = DEBUG_NEW LineSettings(col,pattern,patscale,width);
}

void layprop::ViewProperties::addcolor(std::string name, byte R, byte G, byte B, byte A) {
   if (_drawprop._laycolors.end() != _drawprop._laycolors.find(name)) {
      delete _drawprop._laycolors[name];
      std::ostringstream ost;
      ost << "Warning! Color \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   tellRGB* col = DEBUG_NEW tellRGB(R,G,B,A);
   _drawprop._laycolors[name] = col;
}

void layprop::ViewProperties::addfill(std::string name, byte* ptrn) {
   if (_drawprop._layfill.end() != _drawprop._layfill.find(name)) {
      delete [] _drawprop._layfill[name];
      std::ostringstream ost;
      ost << "Warning! Fill \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   _drawprop._layfill[name] = ptrn;
}

void  layprop::ViewProperties::hideLayer(word layno, bool hide) {
   // No error messages here, because of possible range use
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->_hidden = hide;
}

void  layprop::ViewProperties::lockLayer(word layno, bool lock) {
   // No error messages here, because of possible range use
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->_locked = lock;
}

const layprop::LayoutGrid* layprop::ViewProperties::grid(byte No) const {
   if (_grid.end() != _grid.find(No)) {
      gridlist::const_iterator cg = _grid.find(No);
      return cg->second;
   }
   else return NULL;
}

void layprop::ViewProperties::setGrid(byte No, real step, std::string colname) {
   if (_grid.end() != _grid.find(No)) // if this grid No is already defined
      _grid[No]->Init(step,colname);
   else // define a new grid
      _grid[No] = DEBUG_NEW layprop::LayoutGrid(step, colname);
}

bool layprop::ViewProperties::viewGrid(byte No, bool status) {
   if (_grid.end() != _grid.find(No))
      _grid[No]->turnover(status);
   else status = false;
   return status;
}

void layprop::ViewProperties::drawGrid() const{
   typedef gridlist::const_iterator CI;
   for(CI p = _grid.begin(); p != _grid.end(); p++)
      p->second->Draw(_drawprop, _UU);
}

void layprop::ViewProperties::all_colors(nameList& colist) const
{
   for( colorMAP::const_iterator CI = _drawprop._laycolors.begin(); CI != _drawprop._laycolors.end(); CI++)
      colist.push_back(CI->first);
}

void layprop::ViewProperties::all_fills(nameList& filist) const
{
   for( fillMAP::const_iterator CI = _drawprop._layfill.begin(); CI != _drawprop._layfill.end(); CI++)
      filist.push_back(CI->first);
}

void layprop::ViewProperties::all_lines(nameList& linelist) const
{
   for( lineMAP::const_iterator CI = _drawprop._lineset.begin(); CI != _drawprop._lineset.end(); CI++)
      linelist.push_back(CI->first);
}

void layprop::ViewProperties::setUU(real UU) {
   _UU = UU;
   _DBscale = 1/UU;
};

void layprop::ViewProperties::saveScreenProps(FILE* prop_file) const
{
   fprintf(prop_file, "void  screenSetup() {\n");
   gridlist::const_iterator GLS;
   if (_grid.end() != (GLS = _grid.find(0)))
   {
      fprintf(prop_file, "  definegrid(0, %f , \"%s\");\n", GLS->second->step(), GLS->second->color().c_str());
      fprintf(prop_file, "  grid(0,%s);\n",GLS->second->visual() ? "true" : "false");
   }
   if (_grid.end() != (GLS = _grid.find(1)))
   {
      fprintf(prop_file, "  definegrid(1, %f , \"%s\");\n", GLS->second->step(), GLS->second->color().c_str());
      fprintf(prop_file, "  grid(1,%s);\n",GLS->second->visual() ? "true" : "false");
   }
   if (_grid.end() != (GLS = _grid.find(2)))
   {
      fprintf(prop_file, "  definegrid(2, %f , \"%s\");\n", GLS->second->step(), GLS->second->color().c_str());
      fprintf(prop_file, "  grid(2,%s);\n",GLS->second->visual() ? "true" : "false");
   }
   fprintf(prop_file, "  step(%f);\n",_step);
   fprintf(prop_file, "  autopan(%s);\n",_autopan ? "true" : "false");
   fprintf(prop_file, "  shapeangle(%d);\n",_marker_angle);
   fprintf(prop_file, "}\n\n");
}


void layprop::ViewProperties::saveProperties(std::string filename) const
{
   FILE * prop_file;
   prop_file = fopen(filename.c_str(),"wt");
   // file header here
   _drawprop.savePatterns(prop_file);
   _drawprop.saveColors(prop_file);
   _drawprop.saveLines(prop_file);
   _drawprop.saveLayers(prop_file);
   saveScreenProps(prop_file);
   fprintf(prop_file, "layerSetup();screenSetup();\n\n");
   fclose(prop_file);
}

layprop::ViewProperties::~ViewProperties() {
   for(gridlist::iterator GI = _grid.begin(); GI != _grid.end(); GI++)
      delete GI->second;
   _grid.clear();
}
