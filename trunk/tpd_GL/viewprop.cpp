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
#include "viewprop.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "outbox.h"

layprop::PropertyCenter*         PROPC   = NULL;

void layprop::SupplementaryData::addRuler(TP& p1, TP& p2, real UU)
{
   _rulers.push_front(SDLine(p1,p2,UU));
}

void layprop::SupplementaryData::clearRulers()
{
   _rulers.clear();
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

//=============================================================================
layprop::PropertyCenter::PropertyCenter() :
   _drawprop          ( DEBUG_NEW DrawProperties() ),
   _step              ( 1                          ),
   _autopan           ( false                      ),
   _zeroCross         ( false                      ),
   _boldOnHover       ( false                      ),
   _markerAngle       ( 0                          ),
   _layselmask        ( laydata::_lmall            ),
   _gdsLayMap         ( NULL                       ),
   _cifLayMap         ( NULL                       ),
   _oasLayMap         ( NULL                       )
{
   setUU(1);

}

void layprop::PropertyCenter::addUnpublishedLay(const LayerDef& laydef)
{
   _uplaylist.push_back(laydef);
}

const layprop::LayoutGrid* layprop::PropertyCenter::grid(byte No) const
{
   if (_grid.end() != _grid.find(No))
   {
      gridlist::const_iterator cg = _grid.find(No);
      return cg->second;
   }
   else return NULL;
}

void layprop::PropertyCenter::setGrid(byte No, real step, std::string colname)
{
   if (_grid.end() != _grid.find(No)) // if this grid No is already defined
      _grid[No]->Init(step,colname);
   else // define a new grid
      _grid[No] = DEBUG_NEW layprop::LayoutGrid(step, colname);
}

bool layprop::PropertyCenter::viewGrid(byte No, bool status) {
   if (_grid.end() != _grid.find(No))
      _grid[No]->turnover(status);
   else status = false;
   return status;
}

//void layprop::PropertyCenter::drawZeroCross(const DrawProperties* drawProp) const
//{
//   if (!_zeroCross) return;
//   glLineStipple(1,0xcccc);
//   glEnable(GL_LINE_STIPPLE);
//   glBegin(GL_LINES);
//   glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, (GLfloat)0.7); // gray
//   glVertex2i(0, drawProp->clipRegion().p1().y());
//   glVertex2i(0, drawProp->clipRegion().p2().y());
//   glVertex2i(drawProp->clipRegion().p1().x(), 0);
//   glVertex2i(drawProp->clipRegion().p2().x(), 0);
//   glEnd();
//   glDisable(GL_LINE_STIPPLE);
//}

void layprop::PropertyCenter::setUU(real UU) {
   _UU = UU;
   _DBscale = 1/UU;
};

void layprop::PropertyCenter::saveLayerMaps(FILE* prop_file) const
{
   fprintf(prop_file, "void  layerMaps() {\n");
   std::string sLayMap;
   if (NULL != _gdsLayMap)
   {
      ExtLayerMap2String(_gdsLayMap, sLayMap);
      fprintf(prop_file, "  setgdslaymap( %s );\n", sLayMap.c_str());
   }
   if (NULL != _cifLayMap)
   {
      ExtLayerMap2String(_cifLayMap, sLayMap);
      fprintf(prop_file, "  setciflaymap( %s );\n", sLayMap.c_str());
   }
   if (NULL != _oasLayMap)
   {
      ExtLayerMap2String(_oasLayMap, sLayMap);
      fprintf(prop_file, "  setoaslaymap( %s );\n", sLayMap.c_str());
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::PropertyCenter::saveScreenProps(FILE* prop_file) const
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
   fprintf(prop_file, "  shapeangle(%d);\n",_markerAngle);
   fprintf(prop_file, "}\n\n");
}


void layprop::PropertyCenter::saveProperties(std::string filename, std::string auiMagic)
{
   DrawProperties* drawProp;
   if (lockDrawProp(drawProp))
   {
      FILE * prop_file;
      std::string fname = convertString(filename);
      prop_file = fopen(fname.c_str(),"wt");
      // file header here
      drawProp->savePatterns(prop_file);
      drawProp->saveColors(prop_file);
      drawProp->saveLines(prop_file);
      drawProp->saveLayers(prop_file);
      if ((NULL != _gdsLayMap) || (NULL != _cifLayMap) || (NULL != _oasLayMap))
         saveLayerMaps(prop_file);
      saveScreenProps(prop_file);
      fprintf(prop_file, "layerSetup();");
      if ((NULL != _gdsLayMap) || (NULL != _cifLayMap))
         fprintf(prop_file, "layerMaps();");
      fprintf(prop_file, "screenSetup();\n");
      if (!auiMagic.empty())
         fprintf(prop_file, "loaduiframe(\"%s\");\n\n",auiMagic.c_str());
      else
         fprintf(prop_file,"\n");
      fclose(prop_file);
   }
   unlockDrawProp(drawProp, false);
}

void layprop::PropertyCenter::setGdsLayMap(ExpLayMap* map)
{
   if (NULL != _gdsLayMap) delete _gdsLayMap;
   _gdsLayMap = map;
}

void layprop::PropertyCenter::setCifLayMap(ExpLayMap* map)
{
   if (NULL != _cifLayMap) delete _cifLayMap;
   _cifLayMap = map;
}

void layprop::PropertyCenter::setOasLayMap(ExpLayMap* map)
{
   if (NULL != _oasLayMap) delete _oasLayMap;
   _oasLayMap = map;
}

LayerDefSet layprop::PropertyCenter::allUnselectable()
{
   LayerDefSet unselectable;
   layprop::DrawProperties* drawProp;
   if (lockDrawProp(drawProp))
   {
      drawProp->allUnselectable(unselectable);
   }
   unlockDrawProp(drawProp, false);
   return unselectable;
}

bool layprop::PropertyCenter::lockDrawProp(DrawProperties*& propDB, PropertyState state)
{
   if (wxMUTEX_DEAD_LOCK == _drawPLock.Lock())
   {
      tell_log(console::MT_ERROR,"DrawProperties Mutex deadlocked!");
      _drawprop->setState(state);
      propDB = _drawprop;
      return false;
   }
   else
   {
      _drawprop->setState(state);
      propDB = _drawprop;
      return (NULL != _drawprop);
   }
}

bool layprop::PropertyCenter::tryLockDrawProp(DrawProperties*& propDB, PropertyState state)
{
   if (wxMUTEX_NO_ERROR == _drawPLock.TryLock())
   {
      assert(NULL != _drawprop);
      _drawprop->setState(state);
      propDB = _drawprop;
      return (true);
   }
   else return false;
}

void layprop::PropertyCenter::unlockDrawProp(DrawProperties*& propDB, bool throwexception)
{
   _drawprop = propDB;
   _drawprop->setState(layprop::DB);
   VERIFY(wxMUTEX_NO_ERROR == _drawPLock.Unlock());
   if (throwexception && (NULL == propDB))
      throw EXPTNdrawProperty();
   propDB = NULL;
}

DBlineList layprop::PropertyCenter::getZCross()
{
   DBlineList zCross;
   if (_zeroCross)
   {
      DBline line1(TP(0,_drawprop->clipRegion().p1().y()), TP(0,_drawprop->clipRegion().p2().y()));
      DBline line2(TP(_drawprop->clipRegion().p1().x(),0), TP(_drawprop->clipRegion().p2().x(),0));
      zCross.push_back( line1 );
      zCross.push_back( line2 );
   }
   return zCross;
}

layprop::PropertyCenter::~PropertyCenter()
{
   for(gridlist::iterator GI = _grid.begin(); GI != _grid.end(); GI++)
      delete GI->second;
   _grid.clear();
   if (NULL != _gdsLayMap) delete _gdsLayMap;
   if (NULL != _cifLayMap) delete _cifLayMap;
   if (NULL != _oasLayMap) delete _oasLayMap;
   assert(_drawprop);
   delete _drawprop;
}


//void layprop::USMap2String(USMap* inmap, std::string& outmap)
//{
//   std::ostringstream laymapstr;
//   word recno = 0;
//   laymapstr << "{";
//   for (USMap::const_iterator CLN = inmap->begin(); CLN != inmap->end(); CLN++)
//   {
//      if (recno != 0)
//         laymapstr << ",";
//      laymapstr << "{" << CLN->first << ",\"" << CLN->second << "\"}";
//      recno++;
//   }
//   laymapstr << "}";
//   outmap = laymapstr.str();
//}

void layprop::ExtLayerMap2String(ExpLayMap* inmap, std::string& outmap)
{
   std::ostringstream laymapstr;
   word recno = 0;
   laymapstr << "{";
   for (ExpLayMap::const_iterator CLN = inmap->begin(); CLN != inmap->end(); CLN++)
   {
      if (recno != 0)
         laymapstr << ",";
      laymapstr << "{" << CLN->first << ",\"" << CLN->second << "\"}";
      recno++;
   }
   laymapstr << "}";
   outmap = laymapstr.str();
}
