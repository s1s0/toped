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
//          $URL$
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Reader of Mentor Graphics Calibre drc errors files
//===========================================================================
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//    Description: Reader of Mentor Graphics Calibre drc errors files
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <algorithm>
#include <wx/regex.h>
#include <wx/txtstrm.h>
#include "calbr_reader.h"

//long Calbr::drcPolygon::_precision = 0;
//long Calbr::drcEdge::_precision = 0;

#ifdef WIN32
   #define tpdSTRxxxCMP _stricmp
#else
   #define tpdSTRxxxCMP strcasecmp
#endif

//=============================================================================
auxdata::DrcPoly::DrcPoly(int4b* pdata, unsigned psize, unsigned ordinal) :
   AuxData     ( sh_drc    ),
   _pdata      ( pdata     ),
   _psize      ( psize     ),
   _ordinal    ( ordinal   )
{}

auxdata::DrcPoly::~DrcPoly()
{
   delete [] _pdata;
}

DBbox auxdata::DrcPoly::overlap() const
{
   DBbox ovl(_pdata[0], _pdata[1]) ;
   for (word i = 1; i < _psize; i++)
      ovl.overlap(_pdata[2*i], _pdata[2*i+1]);
   return ovl;
}

void auxdata::DrcPoly::drawRequest(trend::TrendBase& rend) const
{
   rend.grcpoly(_pdata, _psize);
}

void auxdata::DrcPoly::drawSRequest(trend::TrendBase& rend, const SGBitSet*) const
{
   rend.poly(_pdata, _psize, NULL, NULL);
}

void auxdata::DrcPoly::info(std::ostringstream& ost, real DBU) const
{
   ost << "polygon - {";
   for (unsigned i = 0; i < _psize; i++)
   {
      TP cpnt(_pdata[2*i], _pdata[2*i+1]);
      cpnt.info(ost, DBU);
      if (i != _psize - 1) ost << " , ";
   }
   ost << "};";
}

void auxdata::DrcPoly::motionDraw(const layprop::DrawProperties&, CtmQueue& transtack,
                                 SGBitSet* plst) const
{
   CTM trans = transtack.front();
   PointVector* ptlist = DEBUG_NEW PointVector;
   ptlist->reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
   {
      ptlist->push_back( TP(_pdata[2*i], _pdata[2*i+1]) * trans);
   }
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < _psize; i++)
   {
      glVertex2i((*ptlist)[i].x(), (*ptlist)[i].y());
   }
   glEnd();
   ptlist->clear();
   delete ptlist;
}


bool auxdata::DrcPoly::pointInside(const TP pnt)const
{
   TP p0, p1;
   byte cc = 0;
   for (unsigned i = 0; i < _psize ; i++)
   {
      p0 = TP(_pdata[2 *   i             ], _pdata[2 *   i              + 1]);
      p1 = TP(_pdata[2 * ((i+1) % _psize)], _pdata[2 * ((i+1) % _psize) + 1]);
      if (((p0.y() <= pnt.y()) && (p1.y() >  pnt.y()))
        ||((p0.y() >  pnt.y()) && (p1.y() <= pnt.y())) ) {
         float tngns = (float) (pnt.y() - p0.y())/(p1.y() - p0.y());
         if (pnt.x() < p0.x() + tngns * (p1.x() - p0.x()))
            cc++;
      }
   }
   return (cc & 0x01) ? true : false;
}
//=============================================================================
auxdata::DrcSeg::DrcSeg(int4b* sdata, unsigned ssize, unsigned ordinal) :
   AuxData     ( sh_drc    ),
   _sdata      ( sdata     ),
   _ssize      ( ssize     ),
   _ordinal    ( ordinal   )
{}

auxdata::DrcSeg::~DrcSeg()
{
   delete [] _sdata;
}

DBbox auxdata::DrcSeg::overlap() const
{
   DBbox ovl(_sdata[0], _sdata[1]) ;
   ovl.overlap(_sdata[2], _sdata[3]);
   for (word i = 1; i < _ssize; i++)
   {
      ovl.overlap(_sdata[4*i  ], _sdata[4*i+1]);
      ovl.overlap(_sdata[4*i+2], _sdata[4*i+3]);
   }
   return ovl;
}

void auxdata::DrcSeg::drawRequest(trend::TrendBase&) const
{
   //TODO
}

void auxdata::DrcSeg::drawSRequest(trend::TrendBase&, const SGBitSet*) const
{
   //TODO
}

void auxdata::DrcSeg::motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const
{
   //TODO
}

void auxdata::DrcSeg::info(std::ostringstream&, real) const
{
   //TODO
}

bool auxdata::DrcSeg::pointInside(const TP)const
{
   return false;
   //TODO
}

//=============================================================================
Calbr::DrcRule::DrcRule(/*unsigned num, */const std::string& name) :
//   _num              ( num     ),
   _ruleCheckName    ( name      ),
   _curResCount      ( 0l        ),
   _origResCount     ( 0l        ),
   _timeStamp        (           )
{
   _drcData = DEBUG_NEW auxdata::QuadTreeAux();
   _tmpData = DEBUG_NEW auxdata::QTreeTmpAux(_drcData);
}

void Calbr::DrcRule::setTimeStamp(const std::string& timeStamp)
{
   _timeStamp = timeStamp;
}

void Calbr::DrcRule::setCurResCount(int curResCount)
{
   _curResCount = curResCount;
}

void Calbr::DrcRule::setOrigResCount(int origResCount)
{
   _origResCount = origResCount;
}

void Calbr::DrcRule::addDescrString(const std::string& str)
{
   _descrStrings.push_back(str);
}

void Calbr::DrcRule::addResult(auxdata::AuxData* data)
{
   _tmpData->put(data);
}

void Calbr::DrcRule::parsed()
{
   _tmpData->commit();
}

//auxdata::QuadTreeAux* auxdata::DrcRule::secureLayer(const LayerDef& laydef)
//{
//   if (_layers.end() == _layers.find(laydef))
//      _layers.add(laydef, DEBUG_NEW auxdata::QuadTreeAux());
//   return _layers[laydef];
//}
//
//auxdata::QTreeTmpAux* auxdata::DrcRule::secureUnsortedLayer(const LayerDef& laydef)
//{
//   if (_tmpLayers.end() == _tmpLayers.find(laydef))
//      _tmpLayers.add(laydef, DEBUG_NEW auxdata::QTreeTmpAux(secureLayer(laydef)));
//   return _tmpLayers[laydef];
//}

Calbr::DrcRule::~DrcRule()
{
   _drcData->freeMemory();
   delete _drcData;
   delete _tmpData;
}


////=============================================================================
//void Calbr::drcEdge::addCoord(long x1, long y1, long x2, long y2)
//{
////   real xx, yy;
//   //???****Not using now*******
//   /*wxString xstr = convert(x1, _precision);
//   wxString ystr = convert(y1, _precision);
//   xstr.ToDouble(&xx);
//   ystr.ToDouble(&yy);
//
//   _coords.x1 = xx;
//   _coords.y1 = yy;
//
//   xstr = convert(x2, _precision);
//   ystr = convert(y2, _precision);
//   xstr.ToDouble(&xx);
//   ystr.ToDouble(&yy);
//
//   _coords.x2 = xx;
//   _coords.y2 = yy;*/
//   //*************
//   _coords.x1 = x1;
//   _coords.y1 = y1;
//   _coords.x2 = x2;
//   _coords.y2 = y2;
//}
//
//Calbr::edge Calbr::drcEdge::getZoom() const
//{
//   edge ret;
//   ret.x1 = std::min(_coords.x1, _coords.x2);
//   ret.y1 = std::min(_coords.y1, _coords.y2);
//   ret.x2 = std::max(_coords.x1, _coords.x2);
//   ret.y2 = std::max(_coords.y1, _coords.y2);
//   return ret;
//}
//
//void Calbr::drcEdge::addError()
//{
//   _render->addLine(_coords);
//}
//
//
//void Calbr::drcPolygon::addCoord(long x, long y)
//{
//   //???****Not using now*******
//   /*wxString xstr = convert(x, _precision);
//   wxString ystr = convert(y, _precision);
//
//   long xx, yy;
//   xstr.ToLong(&xx);
//   ystr.ToLong(&yy);
//*/
//   TP pt(x, y);
//   _coords.push_back(pt);
//}
//
//void Calbr::drcPolygon::addError()
//{
//   _render->addPoly(_coords);
//}
//
//Calbr::edge Calbr::drcPolygon::getZoom() const
//{
//   CoordsVector::const_iterator it = _coords.begin();
//   long minx = (*it).x();
//   long miny = (*it).y();
//   long maxx = (*it).x();
//   long maxy = (*it).y();
//   for (CoordsVector::const_iterator it = _coords.begin(); it != _coords.end(); ++it)
//   {
//      minx = std::min(long((*it).x()), minx);
//      miny = std::min(long((*it).y()), miny);
//      maxx = std::max(long((*it).x()), maxx);
//      maxy = std::max(long((*it).y()), maxy);
//   }
//   edge ret;
//   ret.x1 = minx;
//   ret.y1 = miny;
//   ret.x2 = maxx;
//   ret.y2 = maxy;
//   return ret;
//}
//
//Calbr::drcRuleCheck::drcRuleCheck(unsigned int num, const std::string &name) :
//   _num              (num     ),
//   _curResCount      (0l      ),
//   _origResCount     (0l      ),
//   _ruleCheckName    (name    ),
//   _borderInit       (false   )
//{
//}
//
//Calbr::drcRuleCheck::drcRuleCheck(const drcRuleCheck& ruleCheck) :
//   _num              (ruleCheck._num),
//   _curResCount      (0l      ),
//   _origResCount     (0l      ),
//   _ruleCheckName    (ruleCheck._ruleCheckName),
//   _borderInit       (false)
//{
//
//}
//
//Calbr::drcRuleCheck::~drcRuleCheck()
//{
//}
//
//void Calbr::drcRuleCheck::setTimeStamp(const std::string &timeStamp)
//{
//   _timeStamp = timeStamp;
//}
//
//void Calbr::drcRuleCheck::setCurResCount(int curResCount)
//{
//   _curResCount = curResCount;
//}
//
//void Calbr::drcRuleCheck::setOrigResCount(int origResCount)
//{
//   _origResCount = origResCount;
//}
//
//void Calbr::drcRuleCheck::addDescrString(const std::string & str)
//{
//   _descrStrings.push_back(str);
//}
//
//void Calbr::drcRuleCheck::addPolygon(const Calbr::drcPolygon &poly)
//{
//   _polygons.push_back(poly);
//   if (_borderInit)
//   {
//      edge polyBorder = poly.getZoom();
//      _border.x1 = std::min(polyBorder.x1, _border.x1);
//      _border.y1 = std::min(polyBorder.y1, _border.y1);
//      _border.x2 = std::max(polyBorder.x2, _border.x2);
//      _border.y2 = std::max(polyBorder.y2, _border.y2);
//   }
//   else
//   {
//      _border = poly.getZoom();
//      _borderInit = true;
//   }
//}
//
//void Calbr::drcRuleCheck::addEdge(const Calbr::drcEdge &theEdge)
//{
//   _edges.push_back(theEdge);
//   if (_borderInit)
//   {
//      edge polyBorder = theEdge.getZoom();
//      _border.x1 = std::min(polyBorder.x1, _border.x1);
//      _border.y1 = std::min(polyBorder.y1, _border.y1);
//      _border.x2 = std::max(polyBorder.x2, _border.x2);
//      _border.y2 = std::max(polyBorder.y2, _border.y2);
//   }
//   else
//   {
//      _border = theEdge.getZoom();
//      _borderInit = true;
//   }
//}
//
//Calbr::edge Calbr::drcRuleCheck::getZoom(long ordinal)
//{
//   edge ret;
//   for (std::vector<Calbr::drcPolygon>::const_iterator it = _polygons.begin(); it
//         != _polygons.end(); ++it)
//   {
//      if (ordinal == (*it).ordinal())
//      {
//         ret = (*it).getZoom();
//         return ret;
//      }
//   }
//
//   for (std::vector<Calbr::drcEdge>::const_iterator it = _edges.begin(); it
//         != _edges.end(); ++it)
//   {
//      if (ordinal == (*it).ordinal())
//      {
//         ret = (*it).getZoom();
//         return ret;
//      }
//   }
//   throw EXPTNdrc_reader("Can't zoom to chosen element");
//   return ret;
//}
//
//Calbr::edge Calbr::drcRuleCheck::getZoom(void)
//{
//   return _border;
//}
//
////-----------------------------------------------------------------------------
//Calbr::CalbrFile::CalbrFile(const std::string &fileName, drcRenderer *render) :
//   _calbrFile        ( NULL      ),
//   _fileName         ( fileName  ),
//   _precision        ( 0         ),
//   _curRuleCheck     ( NULL      ),
//   _ok               ( true      ),
//   _render           ( render    ),
//   _isCellNameMode   ( false     )
//{
//   readFile();
//}
//
//Calbr::CalbrFile::~CalbrFile()
//{
//   if (!_RuleChecks.empty())
//   {
//      for(RuleChecksVector::const_iterator it= _RuleChecks.begin(); it != _RuleChecks.end(); ++it)
//      {
//         if ((*it)!= NULL) delete (*it);
//      }
//      _RuleChecks.clear();
//   }
//
//   if(!_cellDRCMap.empty())
//   {
//      for(CellDRCMap::const_iterator  it= _cellDRCMap.begin(); it != _cellDRCMap.end(); ++it)
//      {
//         if ((*it).second!= NULL)
//         {
//            cellNameStruct *CNstruct = (*it).second;
//            for(RuleChecksVector::const_iterator it2= CNstruct->_RuleChecks.begin();
//               it2 != CNstruct->_RuleChecks.end(); ++it2)
//            {
//               if ((*it2)!= NULL) delete (*it2);
//            }
//            delete (*it).second;
//         }
//      }
//      _cellDRCMap.clear();
//   }
//   if (_render) delete _render;
//}
//
//void Calbr::CalbrFile::readFile()
//{
//   try
//   {
//      std::ostringstream ost;
//      std::string fname(convertString(_fileName));
//      if (!(_calbrFile = fopen(fname.c_str(),"rt"))) // open the input file
//      {
//         throw(EXPTNdrc_reader("Can't open file"));
//      }
//
//      //read header
//      char str[512];
//      if (fgets(str, 512, _calbrFile)==NULL)
//      {
//         std::string err;
//         err += "Error while reading " + fname + "\n";
//         err += "Can't read header";
//         throw(EXPTNdrc_reader(err));
//      }
//
//
//      char cellName[512];
//      if (sscanf( str, "%s %ld", cellName, &_precision) != 2)
//      {
//         std::string err;
//         err += "Error while reading " + fname + "\n";
//         err += "Can't read cell name or precision";
//         throw(EXPTNdrc_reader(err));
//      }
//      //Initialization of static member drcPolygon class
//      drcPolygon::_precision = _precision;
//      drcEdge::_precision = _precision;
//      _curCellName = cellName;
//      _topCellName = cellName;
//      unsigned int num = 1;
//
//      cellNameStruct *CNStruct = DEBUG_NEW cellNameStruct;
//      _cellDRCMap[_curCellName] = CNStruct;
//      while(parse(num))
//      {
//         //Reset CellNameMode
//         //Theoretically there is impossible to change mode from first appearance
//         //But format contains mode flag for each rule check
//         _isCellNameMode = false;
//         num++;
//      }
//
//
//      addResults();
//
//      if (_calbrFile) fclose(_calbrFile);
//
//      if(isOk())
//      {
//         if(!_RuleChecks.empty())
//         {
//            _border = (*_RuleChecks.begin())->getZoom();
//         }
//         else
//         {
//            cellNameStruct *st = _cellDRCMap.begin()->second;
//            Calbr::drcRuleCheck* rule = *(st->_RuleChecks.begin());
//            _border = rule->getZoom();
//         }
//         for (RuleChecksVector::const_iterator it = _RuleChecks.begin(); it != _RuleChecks.end(); ++it)
//         {
//            edge tempBorder = (*it)->getZoom();
//            if(tempBorder.x1 < _border.x1) _border.x1 = tempBorder.x1;
//            if(tempBorder.y1 < _border.y1) _border.y1 = tempBorder.y1;
//            if(tempBorder.x2 > _border.x2) _border.x2 = tempBorder.x2;
//            if(tempBorder.y2 > _border.y2) _border.y2 = tempBorder.y2;
//         }
//
//         for (CellDRCMap::const_iterator it = _cellDRCMap.begin(); it != _cellDRCMap.end(); ++it)
//         {
//            RuleChecksVector checks = (*it).second->_RuleChecks;
//            for (RuleChecksVector::const_iterator it2 = checks.begin(); it2 != checks.end(); ++it2)
//            {
//               edge tempBorder = (*it2)->getZoom();
//               if(tempBorder.x1 < _border.x1) _border.x1 = tempBorder.x1;
//               if(tempBorder.y1 < _border.y1) _border.y1 = tempBorder.y1;
//               if(tempBorder.x2 > _border.x2) _border.x2 = tempBorder.x2;
//               if(tempBorder.y2 > _border.y2) _border.y2 = tempBorder.y2;
//            }
//         }
//
//         _render->setCellName(_curCellName);
//      }
//   }
//   catch (EXPTNdrc_parser&)
//   {
//      _ok = false;
//      return;
//   }
//}
//
//bool Calbr::CalbrFile::parse(unsigned int num)
//{
//   std::ostringstream ost;
//   char ruleCheckName[512];
//
//   // get drc Rule Check name
//   if (fgets(ruleCheckName, 512, _calbrFile)==NULL) return false;
//
//   //Remove LF from  ruleCheckName before creating ruleCheck
//   _curRuleCheckName = std::string(ruleCheckName, strlen(ruleCheckName)-1);
//   _curRuleCheck = DEBUG_NEW Calbr::drcRuleCheck(num, _curRuleCheckName);
//   char tempStr[512];
//   char timeStamp[512];
//   long resCount, origResCount, descrStrCount;
//
//   //Get Current Results Count, Orginal Results Count and description strings count
//   if (fgets(tempStr, 512, _calbrFile)==NULL)
//   {
//      std::string err;
//      err += "Can't read  rule ";
//      err += ruleCheckName;
//      throw(EXPTNdrc_reader(err));
//   }
//   if( sscanf(tempStr, "%ld %ld %ld %[^\n]\n",  &resCount, &origResCount, &descrStrCount, timeStamp) != 4)
//   {
//      throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//   };
//
//   _curRuleCheck->setCurResCount(resCount);
//   _curRuleCheck->setOrigResCount(origResCount);
//   _curRuleCheck->setTimeStamp(timeStamp);
//
//   //Get Description Strings
//   for(long i= 0; i < descrStrCount; i++)
//   {
//      if (fgets(tempStr, 512, _calbrFile)==NULL)
//      {
//         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      }
//      _curRuleCheck->addDescrString(tempStr);
//   }
//   //Get Results
//   for(long i= 0; i < resCount; i++)
//   {
//      if (fgets(tempStr, 512, _calbrFile)==NULL)
//      {
//         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      }
//      char type;
//      long ordinal;
//      short numberOfElem;
//      if (sscanf( tempStr, "%c %ld %hd", &type, &ordinal, &numberOfElem) != 3)
//      {
//         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      };
//
//
//      switch(type)
//      {
//
//         case 'p'   :
//            {
//               drcPolygon poly(ordinal, _render);
//               if (!parsePoly(ruleCheckName ,poly, numberOfElem)) return false;
//               _curRuleCheck->addPolygon(poly);
//            }
//            break;
//
//         case 'e'   :
//            {
//               Calbr::drcEdge theEdge(ordinal, _render);
//               if (!parseEdge(ruleCheckName ,theEdge, numberOfElem)) return false;
//               _curRuleCheck->addEdge(theEdge);
//            }
//               break;
//         default   :
//            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      }
//   }
//
//   appendRuleCheckToCellName();
//   return true;
//}
//
//bool Calbr::CalbrFile::parsePoly(char* ruleCheckName, drcPolygon & poly, int numberOfElem)
//{
//   char tempStr[512];
//   std::ostringstream ost;
//
//   for(short j =0; j< numberOfElem; j++)
//   {
//      long x, y;
//      if (fgets(tempStr, 512, _calbrFile)==NULL)
//      {
//         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      }
//
//      //Check Cell Name Mode
//      if((tempStr[0]=='C') && (tempStr[1]=='N'))
//      {
//         if (_isCellNameMode) //Save _curRuleCheck in previous cellName and create copy of current _curRuleCheck
//         {
//            appendRuleCheckToCellName();
//            _curRuleCheck = DEBUG_NEW drcRuleCheck(*_curRuleCheck);
//         }
//
//         if(parseCellNameMode(tempStr))
//         {
//
//         }
//         else
//         {
//            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//         }
//         //After parsing Cell Name Mode read next string
//         if (fgets(tempStr, 512, _calbrFile)==NULL)
//         {
//            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//         }
//      }
//
//      if (sscanf( tempStr, "%ld %ld", &x, &y)!= 2)
//      {
//         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      };
//      poly.addCoord(x, y);
//}
//
//   return true;
//}
//
//bool Calbr::CalbrFile::parseEdge(char* ruleCheckName, drcEdge & edge, int numberOfElem)
//{
//   char tempStr[512];
//   std::ostringstream ost;
//
//   for(short j =0; j< numberOfElem; j++)
//   {
//      long x1, y1, x2, y2;
//      if (fgets(tempStr, 512, _calbrFile)==NULL)
//      {
//        throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      }
//
//      //Check Cell Name Mode
//      if((tempStr[0]=='C') && (tempStr[1]=='N'))
//      {
//         if (_isCellNameMode) //Save _curRuleCheck in previous cellName and create copy of current _curRuleCheck
//         {
//            appendRuleCheckToCellName();
//            _curRuleCheck = DEBUG_NEW drcRuleCheck(*_curRuleCheck);
//         }
//
//         if(parseCellNameMode( tempStr))
//         {
//            //_curRuleCheck->addCellNameStruct(CNStruct);
//         }
//         else
//         {
//            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//         }
//         //After parsing Cell Name Mode read next string
//         if (fgets(tempStr, 512, _calbrFile)==NULL)
//         {
//            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//         }
//      }
//
//      if(sscanf( tempStr, "%ld %ld %ld %ld", &x1, &y1, &x2, &y2)!=4)
//      {
//         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
//      };
//      edge.addCoord(x1, y1, x2, y2);
//   }
//
//   return true;
//}
//
//bool  Calbr::CalbrFile::parseCellNameMode(const std::string &parseString)
//{
//   cellNameStruct *CNStruct = DEBUG_NEW cellNameStruct;
//   //Check for Cell Name results
//   wxRegEx regex;
//   //Regexp: CN cellname (with 'c' or withoout 'c') number1, number2 ... number6
//   VERIFY(regex.Compile(wxT("(CN) ([$[:alnum:]_]+) (c{0,1}) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+)")));
//   wxString str=wxString(parseString.c_str(), wxConvUTF8);
//   //wxString str = wxT("CN xxx c -1 2 3 4 5 6");
//   if (regex.Matches(str))
//   {
//      std::string cellName(regex.GetMatch(str, 2).char_str());
//      std::string str2(regex.GetMatch(str, 3).char_str());
//      if (!tpdSTRxxxCMP(str2.c_str(), ""))
//      {
//         CNStruct->spaceCoords = false;
//      }
//      else
//         if (!tpdSTRxxxCMP(str2.c_str(), "c"))
//         {
//            CNStruct->spaceCoords = true;
//         }
//         else
//         {
//            return false;
//         }
//      //Save tranformation matrix
////      long number;
////      long a, b, c, d, tx, ty;
////      regex.GetMatch(str, 4).ToLong(&number);
////      a = number;
////      regex.GetMatch(str, 5).ToLong(&number);
////      b = number;
////      regex.GetMatch(str, 6).ToLong(&number);
////      c = number;
////      regex.GetMatch(str, 7).ToLong(&number);
////      d = number;
////      regex.GetMatch(str, 8).ToLong(&number);
////      tx = number;
////      regex.GetMatch(str, 9).ToLong(&number);
////      ty = number;
////      CNStruct->transfMatrix.setCTM(a, b, c, d, tx, ty); TODO - Why this is replaced by the lines below?
//      CTM dummy;
//      CNStruct->transfMatrix.setCTM(dummy.a(), dummy.b(), dummy.c(), dummy.d(), dummy.tx(), dummy.ty());
//
//      _isCellNameMode = true;
//      _curCellName = cellName;
//      CellDRCMap::iterator it = _cellDRCMap.find(cellName);
//      if(it!= _cellDRCMap.end())
//      {
//         //???Redifinition of CNStruct
//         //May be we have to add cheking here
//         //it->second = CNStruct;
//         delete CNStruct;
//      }
//      else
//      {
//         _cellDRCMap[cellName] = CNStruct;
//      }
//      return true;
//   }
//   else return false;
//}
//
//
//void   Calbr::CalbrFile::addResults()
//{
//
//   /*_render->startWriting();
//   if (!_RuleChecks.empty())
//   {
//      RuleChecksVector::const_iterator it;
//      for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
//      {
//         addRuleCheck((*it));
//      }
//   }
//   else*/
//   {
//      for (CellDRCMap::const_iterator it = _cellDRCMap.begin(); it != _cellDRCMap.end(); ++it)
//      {
//         _render->startWriting((*it).first);
//         _render->setCellName((*it).first);
//         _render->setTranformation((*it).second->transfMatrix);
//
//         RuleChecksVector checks = (*it).second->_RuleChecks;
//         for (RuleChecksVector::const_iterator it2 = checks.begin(); it2 != checks.end(); ++it2)
//         {
//            addRuleCheck((*it2));
//         }
//         _render->endWriting();
//      }
//   }
//
//   //_render->endWriting();
//   _render->hideAll();
//}
//
//void   Calbr::CalbrFile::addRuleCheck(drcRuleCheck* check)
//{
//   _render->setError(check->num());
//   std::vector <Calbr::drcPolygon>::iterator it2;
//   std::vector <Calbr::drcPolygon> *polys = check->polygons();
//   for(it2 = polys->begin(); it2 < polys->end(); ++it2)
//   {
//      (*it2).addError();
//   }
//   std::vector <Calbr::drcEdge>::iterator it2edge;
//   std::vector <Calbr::drcEdge> *edges = check->edges();
//   for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
//   {
//      (*it2edge).addError();
//   }
//}
//
//void   Calbr::CalbrFile::showError(const std::string& cell, const std::string& error, long  number)
//{
//   if(_cellDRCMap.find(cell) != _cellDRCMap.end())
//   {
//      Calbr::cellNameStruct* cellStruct =_cellDRCMap[cell];
//      RuleChecksVector* ruleCheck = &(cellStruct->_RuleChecks);
//      edge zoom;
//      RuleChecksVector::const_iterator it;
//      for(it = ruleCheck->begin(); it!= ruleCheck->end(); ++it)
//      {
//         std::string x = (*it)->ruleCheckName();
//         if((*it)->ruleCheckName() == error)
//         {
//            _curCellName = cell;
//            _render->hideAll();
//            if(_render->showError((*it)->num()))
//            {
//               try
//               {
//                  zoom = (*it)->getZoom(number);
//               }
//               catch (EXPTNdrc_reader&)
//               {
//                  return;
//               }
//               _render->zoom(zoom);
//            }
//         }
//      }
//      assert(it == ruleCheck->end());
//   }
//}
//
//
//bool   Calbr::CalbrFile::showCluster(const std::string & cell, const std::string & error)
//{
//   if (_cellDRCMap.end() == _cellDRCMap.find(cell))
//      return false;
//   Calbr::cellNameStruct* cellStruct =_cellDRCMap[cell];
//   RuleChecksVector* ruleChecks = &(cellStruct->_RuleChecks);
//
//   edge zoom;
//   RuleChecksVector::const_iterator it;
//   for(it = ruleChecks->begin(); it!= ruleChecks->end(); ++it)
//   {
////      std::string x = (*it)->ruleCheckName();
//      if((*it)->ruleCheckName() == error)
//      {
//         _render->hideAll();
//         if (_render->showError((*it)->num()))
//         {
//            try
//            {
//               zoom = (*it)->getZoom();
//            }
//            catch (EXPTNdrc_reader&)
//            {
//               return false;
//            }
//            _render->zoom(zoom);
//         }
//      }
//   }
//   assert(it == ruleChecks->end());
//   return true;
//}
//
//void   Calbr::CalbrFile::showAllErrors(void)
//{
//   _render->showAll();
//   //_render->zoom(_border);
//}
//
//void   Calbr::CalbrFile::hideAllErrors(void)
//{
//   _render->hideAll();
//}
//
//std::string Calbr::CalbrFile::explainError(word lay)
//{
//   for(CellDRCMap::const_iterator m_it = _cellDRCMap.begin(); m_it!=_cellDRCMap.end(); ++m_it)
//   {
//      RuleChecksVector *ruleChecks = &((*m_it).second->_RuleChecks);
//      for(RuleChecksVector::const_iterator it = ruleChecks->begin(); it!= ruleChecks->end(); ++it)
//      {
//         if ((*it)->num() == lay) return (*it)->ruleCheckName();
//      }
//   }
//   assert(true);
//   //dummy, to prevent compiler warnings!
//   return "";
//}
//
//bool Calbr::CalbrFile::isCellNameMode(void)
//{
//   return !_cellDRCMap.empty();
//}
//
//CTM Calbr::CalbrFile::getCTM(const std::string & cell)
//{
//   CellDRCMap::iterator it = _cellDRCMap.find(cell);
//   if (it == _cellDRCMap.end())
//   {
//      assert(true);
//   }
//   return it->second->transfMatrix;
//}
//
//void Calbr::CalbrFile::appendRuleCheckToCellName(void)
//{
//   CellDRCMap::iterator it = _cellDRCMap.find(_curCellName);
//   if(it!= _cellDRCMap.end())
//   {
//      it->second->_RuleChecks.push_back(_curRuleCheck);
//   }
//   else
//   {
//      assert(true); // Something wrong. CellNameMode is set but no such cell
//   }
//}
//
//wxString Calbr::convert(int number, long precision)
//{
//   float x   = float(number) / precision;
//   int xint = x;
//   wxString str1;
//   wxString format = wxT("%");
//   wxString xintstr, xfracstr;
//   xintstr << xint;
//   xfracstr << precision;
//   format << xintstr.Length() << wxT(".") << xfracstr.Length() << wxT("f");
//   str1.sprintf(format, x);
//   return str1;
//}
//=============================================================================


Calbr::DrcCell::DrcCell(std::string name, CTM& ctm) :
   _name    ( name   ),
   _ctm     ( ctm    )
{

}

//void Calbr::DrcCell::registerRuleRead(std::string, DrcRule*)
//{
//
//}

void Calbr::DrcCell::addResult(auxdata::AuxData* data)
{
//   _tmpData->put(data); // TODO
}
//=============================================================================
Calbr::ClbrFile::ClbrFile(wxString wxfname, DrcLibrary*& drcDB) : InputDBFile (wxfname, false)
{
   std::ostringstream info;
   if (!status())
   {
      throw EXPTNdrc_reader("Failed to open the input file");
   }
   info << "Parsing \"" << fileName() << "\" using ASCII DRC grammar)";
   tell_log(console::MT_INFO,info.str());

   _textStream = DEBUG_NEW wxTextInputStream(*_inStream);

   real precision;
   std::string cellName = getTextWord();
   if (moreData()) precision = _textStream->ReadDouble();

   _drcDB = drcDB = DEBUG_NEW DrcLibrary(cellName, precision);


   CTM defCtm;
   _cCell = _drcDB->registerCellRead(cellName, defCtm);

   // Now parse the rules
//   unsigned int rcNum = 0;
   do // i.e. at least one rule is expected
   {
      DrcRule* curRule = NULL;
      std::string ruleName;
      // Get the rule name and create a drcRuleCheck object with it
      // It is considered that at this point is legal to finish the file
      // That's why getTextWord() is not used, it might throw an exception
      if (moreData(false))
      {
         wxString wxRuleName = _textStream->ReadWord();
         ruleName =  std::string(wxRuleName.mb_str(wxConvUTF8));
         if (ruleName.empty())
            break;
         else
            curRule = DEBUG_NEW DrcRule(/*rcNum++, */ruleName);
      }
      else break;
      ruleMetaData(curRule);
      ruleDrcResults(curRule);
      curRule->parsed();
      drcDB->registerRuleRead(ruleName, curRule);
   } while (true);
}

bool Calbr::ClbrFile::moreData(bool throwexception)
{
   if (_inStream->Eof())
   {
      if (throwexception)
         throw EXPTNdrc_reader("Unexpected End of File");
      else
         return false;
   }
   return true;
}

std::string Calbr::ClbrFile::getTextWord()
{
   if (moreData())
   {
      wxString wxWord = _textStream->ReadWord();
      return std::string(wxWord.mb_str(wxConvUTF8));
   }
   return "";
}

std::string Calbr::ClbrFile::getTextLine()
{
   if (moreData())
   {
      wxString wxWord = _textStream->ReadLine();
      return std::string(wxWord.mb_str(wxConvUTF8));
   }
   return "";
}

void Calbr::ClbrFile::ruleMetaData(DrcRule* curRule)
{
   assert(curRule);
   if (moreData()) curRule->setCurResCount(_textStream->Read32());
   if (moreData()) curRule->setOrigResCount(_textStream->Read32());
   unsigned int numInfoLines;
   if (moreData()) numInfoLines = _textStream->Read32();
   curRule->setTimeStamp(getTextLine());

   for (unsigned int i = 0; i < numInfoLines; i++)
   {
      curRule->addDescrString(getTextLine());
   }
}

void Calbr::ClbrFile::ruleDrcResults(DrcRule* curRule)
{
   assert(curRule);
   do
   {
      int nextChar = _inStream->GetC();
      if (wxEOF == nextChar) return /*end of file*/;
      auxdata::AuxData* cError = NULL;
      switch ((char) nextChar)
      {
         case 'p' : cError = drcPoly(); break;
         case 'e' : cError = drcEdge(); break;
         case '\n':
         case '\r':
         case ' ' : continue; break;
           default: _inStream->Ungetch((char) nextChar);return /*new rule follows*/;
      }
      if (NULL!= cError)
      {
         curRule->addResult(cError);
         _cCell->addResult(cError);
      }
   } while (true);
}

auxdata::DrcPoly* Calbr::ClbrFile::drcPoly()
{
   unsigned int ordinal;
   unsigned int numVrtx;
   if (moreData()) ordinal = _textStream->Read32();
   if (moreData()) numVrtx = _textStream->Read32();
   int4b* pdata = DEBUG_NEW int4b [2*numVrtx];
   while (!checkCNnP()) {};
   for (unsigned int vrtx = 0; vrtx < 2*numVrtx; vrtx++)
   {
      if (moreData()) pdata[vrtx] = _textStream->Read32();
   }
   return DEBUG_NEW auxdata::DrcPoly(pdata, numVrtx, ordinal);
}

auxdata::DrcSeg* Calbr::ClbrFile::drcEdge()
{
   unsigned int ordinal;
   unsigned int numEdgs;
   if (moreData()) ordinal = _textStream->Read32();
   if (moreData()) numEdgs = _textStream->Read32();

   while (!checkCNnP()) {};
   int4b* pdata = DEBUG_NEW int4b [numEdgs * 4];
   for (unsigned int vrtx = 0; vrtx < numEdgs * 4; vrtx++)
   {
      if (moreData()) pdata[vrtx] = _textStream->Read32();
   }
   return DEBUG_NEW auxdata::DrcSeg(pdata, numEdgs, ordinal);
}

bool Calbr::ClbrFile::checkCNnP()
{
   int nextChar;
   std::string cnLine;
   if (moreData()) nextChar = _inStream->GetC();
   if (isdigit(nextChar) || ('-' == (char) nextChar))
   {
      _inStream->Ungetch((char) nextChar);
      return true;
   }
   else if ('C' != (char) nextChar)
   {
      _inStream->Ungetch((char) nextChar);// - should be a property!
   }
   else
   {
      if (moreData()) nextChar = _inStream->GetC();
      if ('N' != (char) nextChar)
      {
         _inStream->Ungetch((char) nextChar);// - should be a property!
      }
      else
      {
         readCN();
         return true;
      }
   }
   std::string property = getTextLine();// TODO parse a property
   return false;
}

void Calbr::ClbrFile::readCN()
{
   wxString wxCellName;
   if (moreData()) wxCellName = _textStream->ReadWord();
   std::string cellName =  std::string(wxCellName.mb_str(wxConvUTF8));
   if (moreData(false))
   {
      int nextChar;
      do
      {
         nextChar = _inStream->GetC(); //_textStream->GetChar
      } while (isspace(nextChar));
      if ('c' == (char) nextChar)
      {// local CTM
         int ctmraw[6];
         for (unsigned int vrtx = 0; vrtx < 6; vrtx++)
         {
            if (moreData()) ctmraw[vrtx] = _textStream->Read32();
         }
         CTM cCtm(ctmraw[0], ctmraw[1], ctmraw[2], ctmraw[3], ctmraw[4], ctmraw[5]);
         _cCell = _drcDB->registerCellRead(cellName, cCtm);
      }
      else
         throw EXPTNdrc_reader("Expected valid CN (Cell Name) record");
   }
}

Calbr::ClbrFile::~ClbrFile()
{
   delete _textStream;
   // delete all CIF structures
//   CifStructure* local = _first;
//   CifStructure* local4d;
//   while (NULL != local)
//   {
//      local4d = local;
//      local = local->last();
//      delete local4d;
//   }
//   delete _default;
}

//=============================================================================
Calbr::DrcLibrary::DrcLibrary(std::string name, real precision) :
   _name          ( name      ),
   _precision     ( precision )
{}

Calbr::DrcLibrary::~DrcLibrary()
{
//   RuleMap::const_iterator wc;
   for (RuleMap::const_iterator wc = _rules.begin(); wc != _rules.end(); wc++)
      delete wc->second;
   _rules.clear();
   for (CellMap::const_iterator wc = _cells.begin(); wc != _cells.end(); wc++)
      delete wc->second;
   _cells.clear();
}

void Calbr::DrcLibrary::registerRuleRead(std::string rulename, DrcRule* rule)
{
   if (_rules.end() != _rules.find(rulename))
   {
      assert(false); // is that legal?
   }
   _rules[rulename] = rule;
}

Calbr::DrcCell* Calbr::DrcLibrary::registerCellRead(std::string cellname, CTM& cCtm)
{
   DrcCell* cCell;
   if (_cells.end() != _cells.find(cellname))
   {
      cCell = _cells[cellname];
//      assert(cCtm == cCell->ctm());
   }
   else
   {
      cCell = DEBUG_NEW DrcCell(cellname, cCtm);
      _cells[cellname] = cCell;
   }
   return cCell;
}

WordList Calbr::DrcLibrary::findSelected(const std::string &cell, TP* p1)
{//TODO
   //TdtDefaultCell* cell = checkCell("drc");
//   auxdata::DrcRule* theCell = checkCell(cell);
//   TP selp;
   WordList errorList;
//   laydata::AtticList* shapes = NULL;
//   laydata::ShapeList *shapeList;
//   TdtData *shape;
//   if (theCell)
//   {
//
//      layprop::DrawProperties* drawProp;
//      if (PROPC->lockDrawProp(drawProp, layprop::prsDRC))
//      {
//         selp = (*p1)*CTM().Reversed(); //Take identity matrix
//         //??? Add here Error List construction
//         shapes = theCell->findSelected(selp);
//         for(laydata::AtticList::Iterator it = shapes->begin(); it != shapes->end(); ++it)
//         {
//            word error;
//            shapeList = *it;
//            for (laydata::ShapeList::const_iterator it2 = shapeList->begin(); it2 != shapeList->end(); ++it2)
//            {
//               shape = dynamic_cast<TdtData*> (*it2);
//               error = shape->getLong();
//               errorList.push_back(error);
//            }
//         }
//      }
//      PROPC->unlockDrawProp(drawProp, true);
//      errorList.unique();
//      if(shapes != NULL)
//      {
//         for(laydata::AtticList::Iterator it = shapes->begin(); it != shapes->end(); ++it)
//         {
//            shapeList = *it;
//            delete shapeList;
//         }
//
//         delete shapes;
//      }
//      return errorList;
//   }
//   else
      return errorList;
}

Calbr::DrcRule* Calbr::DrcLibrary::checkCell(std::string name)
{
   if (_rules.end() == _rules.find(name))
      return NULL;
   else return _rules[name];
}

//void laydata::DrcLibrary::openGlDraw(layprop::DrawProperties& drawProp, std::string cell)
//{
//   drawProp.setState(layprop::DRC);
//   laydata::TdtDefaultCell* dst_structure = checkCell(cell);
//   if (dst_structure)
//   {
//      drawProp.initCtmStack();
////    drawProp->initDrawRefStack(NULL); // no references yet in the DRC DB
//      dst_structure->openGlDraw(drawProp);
////    drawProp->clearCtmStack();
//      drawProp.clearDrawRefStack();
//   }
//   drawProp.setState(layprop::DB);
//}

void Calbr::DrcLibrary::openGlRender(trend::TrendBase& renderer, std::string cell, CTM& cctm)
{
   renderer.setState(layprop::prsDRC);
   DrcRule* dst_structure = checkCell(cell);
   if (dst_structure)
   {
//      dst_structure->openGlRender(renderer, cctm, false, false); TODO
   }
   renderer.setState(layprop::prsDB);
}

