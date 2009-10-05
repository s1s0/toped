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
#include "ttt.h"
#include <GL/glew.h>
#include "viewprop.h"
#include "outbox.h"
#include "tenderer.h"

extern layprop::FontLibrary* fontLib;

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
   glBegin(GL_LINES);
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

   CTM tmtrx;
   tmtrx.Rotate(_angle);
   tmtrx.Translate(_center.x(), _center.y());
   DBline central_elevation = text_bp * tmtrx;

   glPushMatrix();
   glTranslatef(central_elevation.p2().x(), central_elevation.p2().y(), 0);
   glScalef(scaledpix, scaledpix, 1);
   glRotatef(_angle, 0, 0, 1);

   assert(NULL != fontLib);
   fontLib->drawSolidString(_value);

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
   for( numtics = 0 ; (numtics * step) < _length ; numtics++ )
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
      delete tmp_ruler;
   }
}

void layprop::SupplementaryData::getConsts(const CTM& LayCTM, DBline& long_mark, DBline& short_mark, DBline& text_bp, double& scaledpix)
{
   // Side ticks (segments) of the rulers has to be with constant size. The next lines
   // are generating a segment with the size 7/3 screen pixels centered in
   // the {0,0} point of the canvas (logical coords)
   // the coeffitients 1e3/1e-3 are picked ramdomly attempting to reduce the
   // error
   const double ico = 1e3;
   const double dco = 1/ico;
   DBline tick_sample = DBline(TP(0,0),TP(0,7,ico)) * LayCTM;
   double tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   long_mark = DBline(TP(0,-tick_size, dco),TP(0,tick_size, dco));
   
   tick_sample = DBline(TP(0,0),TP(0,3,ico)) * LayCTM;
   tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   short_mark = DBline(TP(0,-tick_size, dco),TP(0,tick_size, dco));
   
   tick_sample = DBline(TP(0,0),TP(0,20,ico)) * LayCTM;
   tick_size = ((double)(tick_sample.p1().y()-tick_sample.p2().y()));
   text_bp = DBline(TP(0,0),TP(0,tick_size, dco));
   
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
void layprop::LayoutGrid::Draw(const DrawProperties& drawprop, const real DBscale)
{
   int gridstep = (int)rint(_step / DBscale);
   if (_visual && ( abs((int)(drawprop.ScrCTM().a() * gridstep)) > GRID_LIMIT))
   {
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
layprop::ViewProperties::ViewProperties()
{
   _step = 1;
   setUU(1);
   _marker_angle = 0;
   _autopan = false;
   _layselmask = laydata::_lmall;
   _gdsLayMap = NULL;
   _cifLayMap = NULL;
   _zeroCross = false;
}

bool layprop::ViewProperties::selectable(unsigned layno) const {
   return (!_drawprop.layerHidden(layno) && !_drawprop.layerLocked(layno));
}

bool layprop::ViewProperties::addlayer(std::string name, unsigned layno, std::string col,
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

bool layprop::ViewProperties::addlayer(std::string name, unsigned layno)
{
   if (_drawprop._layset.end() == _drawprop._layset.find(layno))
   {
      _drawprop._layset[layno] = DEBUG_NEW LayerSettings(name,"","","");
      return true;
   }
   return false;
}

bool layprop::ViewProperties::addlayer( unsigned layno )
{
   if (_drawprop._layset.end() == _drawprop._layset.find(layno))
   {
      std::ostringstream lname;
      lname << "_UNDEF" << layno;
      _drawprop._layset[layno] = DEBUG_NEW LayerSettings(lname.str(),"","","");
      return true;
   }
   return false;
}

unsigned layprop::ViewProperties::addlayer(std::string name)
{
   unsigned layno = 1;
   laySetList::reverse_iterator lastLayNo = _drawprop._layset.rbegin();
   if (_drawprop._layset.rend() != lastLayNo)
      layno = lastLayNo->first;
   while (!addlayer(name, layno)) {layno++;}
   return layno;
}

bool layprop::ViewProperties::isLayerExist(word layno)
{
	bool b = (_drawprop._layset.end() != _drawprop._layset.find(layno));
	return b;
}

bool layprop::ViewProperties::isLayerExist(std::string layname)
{
	for(laySetList::const_iterator it = _drawprop._layset.begin(); it != _drawprop._layset.end(); ++it)
	{
		if((*it).second->name() == layname) return true;
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

void  layprop::ViewProperties::hideLayer(unsigned layno, bool hide) {
   // No error messages here, because of possible range use
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->_hidden = hide;
}

void  layprop::ViewProperties::lockLayer(unsigned layno, bool lock) {
   // No error messages here, because of possible range use
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->_locked = lock;
}
const WordList layprop::ViewProperties::getLockedLayers(void)
{
	//drawprop._layset
	WordList lockedLayers;
	laySetList::const_iterator it;
	for(  it = _drawprop._layset.begin(); it != _drawprop._layset.end(); ++it)
	{
		if((*it).second->locked()) lockedLayers.push_back((*it).first);
	}
	return lockedLayers;
}

void  layprop::ViewProperties::fillLayer(unsigned layno, bool fill) {
   // No error messages here, because of possible range use
   if (_drawprop._layset.end() != _drawprop._layset.find(layno))
      _drawprop._layset[layno]->fillLayer(fill);
}

const WordList layprop::ViewProperties::getAllLayers(void)
{
	//drawprop._layset
	WordList listLayers;
	laySetList::const_iterator it;
	for(  it = _drawprop._layset.begin(); it != _drawprop._layset.end(); ++it)
	{
		listLayers.push_back((*it).first);
	}
	return listLayers;
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

void layprop::ViewProperties::drawGrid() const
{
   typedef gridlist::const_iterator CI;
   for(CI p = _grid.begin(); p != _grid.end(); p++)
      p->second->Draw(_drawprop, _UU);
}

// void layprop::ViewProperties::drawGrid(TopRend& rend) const
// {
//    typedef gridlist::const_iterator CI;
//    for(CI p = _grid.begin(); p != _grid.end(); p++)
//       if (p->second->visual())
//          rend.Grid(p->second->step(), p->second->color());
// }

void layprop::ViewProperties::drawZeroCross() const
{
   if (!_zeroCross) return;
   glLineStipple(1,0xcccc);
   glEnable(GL_LINE_STIPPLE);
   glBegin(GL_LINES);
   glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, (GLfloat)0.7); // gray
   glVertex2i(0, _drawprop.clipRegion().p1().y());
   glVertex2i(0, _drawprop.clipRegion().p2().y()); 
   glVertex2i(_drawprop.clipRegion().p1().x(), 0);
   glVertex2i(_drawprop.clipRegion().p2().x(), 0); 
   glEnd();
   glDisable(GL_LINE_STIPPLE);
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

void layprop::ViewProperties::saveLayerMaps(FILE* prop_file) const
{
   fprintf(prop_file, "void  layerMaps() {\n");
   if (NULL != _gdsLayMap)
   {
      std::string sGdsLayMap;
      USMap2String(_gdsLayMap, sGdsLayMap);
      fprintf(prop_file, "  setgdslaymap( %s );\n", sGdsLayMap.c_str());
   }
   if (NULL != _cifLayMap)
   {
      std::string sCifLayMap;
      USMap2String(_cifLayMap, sCifLayMap);
      fprintf(prop_file, "  setciflaymap( %s );\n", sCifLayMap.c_str());
   }
   fprintf(prop_file, "}\n\n");
}

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
   fprintf(prop_file, "  zerocross(%s);\n",_zeroCross ? "true" : "false");
   fprintf(prop_file, "  shapeangle(%d);\n",_marker_angle);
   fprintf(prop_file, "}\n\n");
}


void layprop::ViewProperties::saveProperties(std::string filename) const
{
   FILE * prop_file;
   std::string fname = convertString(filename);
   prop_file = fopen(fname.c_str(),"wt");
   // file header here
   _drawprop.savePatterns(prop_file);
   _drawprop.saveColors(prop_file);
   _drawprop.saveLines(prop_file);
   _drawprop.saveLayers(prop_file);
   if ((NULL != _gdsLayMap) || (NULL != _cifLayMap))
      saveLayerMaps(prop_file);
   saveScreenProps(prop_file);
   fprintf(prop_file, "layerSetup();");
   if ((NULL != _gdsLayMap) || (NULL != _cifLayMap))
      fprintf(prop_file, "layerMaps();");
   fprintf(prop_file, "screenSetup();\n\n");
   fclose(prop_file);
}

void layprop::ViewProperties::setGdsLayMap(USMap* map)
{
   if (NULL != _gdsLayMap) delete _gdsLayMap;
   _gdsLayMap = map;
}

void layprop::ViewProperties::setCifLayMap(USMap* map)
{
   if (NULL != _cifLayMap) delete _cifLayMap;
   _cifLayMap = map;
}

layprop::ViewProperties::~ViewProperties() 
{
   for(gridlist::iterator GI = _grid.begin(); GI != _grid.end(); GI++)
      delete GI->second;
   _grid.clear();
   if (NULL != _gdsLayMap) delete _gdsLayMap;
   if (NULL != _cifLayMap) delete _cifLayMap;
}


void layprop::USMap2String(USMap* inmap, std::string& outmap)
{
   std::ostringstream laymapstr;
   word recno = 0;
   laymapstr << "{";
   for (USMap::const_iterator CLN = inmap->begin(); CLN != inmap->end(); CLN++)
   {
      if (recno != 0)
         laymapstr << ",";
      laymapstr << "{" << CLN->first << ",\"" << CLN->second << "\"}";
      recno++;
   }
   laymapstr << "}";
   outmap = laymapstr.str();
}
