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
#include <wx/regex.h>
#include "calbr_reader.h"

long Calbr::drcPolygon::_precision = 0;
long Calbr::drcEdge::_precision = 0;

#ifdef WIN32
   #define tpdSTRxxxCMP _stricmp
#else
   #define tpdSTRxxxCMP strcasecmp
#endif

void Calbr::drcEdge::addCoord(long x1, long y1, long x2, long y2)
{
//   real xx, yy;
   //???****Not using now*******
   /*wxString xstr = convert(x1, _precision);
   wxString ystr = convert(y1, _precision);
   xstr.ToDouble(&xx);
   ystr.ToDouble(&yy);

   _coords.x1 = xx;
   _coords.y1 = yy;

   xstr = convert(x2, _precision);
   ystr = convert(y2, _precision);
   xstr.ToDouble(&xx);
   ystr.ToDouble(&yy);

   _coords.x2 = xx;
   _coords.y2 = yy;*/
   //*************
   _coords.x1 = x1;
   _coords.y1 = y1;
   _coords.x2 = x2;
   _coords.y2 = y2;
}

Calbr::edge Calbr::drcEdge::getZoom() const
{
   edge ret;
   ret.x1 = std::min(_coords.x1, _coords.x2);
   ret.y1 = std::min(_coords.y1, _coords.y2);
   ret.x2 = std::max(_coords.x1, _coords.x2);
   ret.y2 = std::max(_coords.y1, _coords.y2);
   return ret;
}

void Calbr::drcEdge::addError()
{
   _render->addLine(_coords);
}


void Calbr::drcPolygon::addCoord(long x, long y)
{
   //???****Not using now*******
   /*wxString xstr = convert(x, _precision);
   wxString ystr = convert(y, _precision);

   long xx, yy;
   xstr.ToLong(&xx);
   ystr.ToLong(&yy);
*/
   TP pt(x, y);
   _coords.push_back(pt);
}

void Calbr::drcPolygon::addError()
{
   _render->addPoly(_coords);
}

Calbr::edge Calbr::drcPolygon::getZoom() const
{
   CoordsVector::const_iterator it = _coords.begin();
   long minx = (*it).x();
   long miny = (*it).y();
   long maxx = (*it).x();
   long maxy = (*it).y();
   for (CoordsVector::const_iterator it = _coords.begin(); it != _coords.end(); ++it)
   {
      minx = std::min(long((*it).x()), minx);
      miny = std::min(long((*it).y()), miny);
      maxx = std::max(long((*it).x()), maxx);
      maxy = std::max(long((*it).y()), maxy);
   }
   edge ret;
   ret.x1 = minx;
   ret.y1 = miny;
   ret.x2 = maxx;
   ret.y2 = maxy;
   return ret;
}

Calbr::drcRuleCheck::drcRuleCheck(unsigned int num, const std::string &name)
      :_num(num), _ruleCheckName(name),_borderInit(false)
{
}

Calbr::drcRuleCheck::drcRuleCheck(const drcRuleCheck& ruleCheck)
      :_borderInit(false)
{
   _num = ruleCheck._num;
   _ruleCheckName = ruleCheck._ruleCheckName;
}

Calbr::drcRuleCheck::~drcRuleCheck()
{
}

void Calbr::drcRuleCheck::setTimeStamp(const std::string &timeStamp)
{
   _timeStamp = timeStamp;
}

void Calbr::drcRuleCheck::setCurResCount(int curResCount)
{
   _curResCount = curResCount;
}

void Calbr::drcRuleCheck::setOrigResCount(int origResCount)
{
   _origResCount = origResCount;
}

void Calbr::drcRuleCheck::addDescrString(const std::string & str)
{
   _descrStrings.push_back(str);
}

void Calbr::drcRuleCheck::addPolygon(const Calbr::drcPolygon &poly)
{
   _polygons.push_back(poly);
   if (_borderInit)
   {
      edge polyBorder = poly.getZoom();
      _border.x1 = std::min(polyBorder.x1, _border.x1);
      _border.y1 = std::min(polyBorder.y1, _border.y1);
      _border.x2 = std::max(polyBorder.x2, _border.x2);
      _border.y2 = std::max(polyBorder.y2, _border.y2);
   }
   else
   {
      _border = poly.getZoom();
      _borderInit = true;
   }
}

void Calbr::drcRuleCheck::addEdge(const Calbr::drcEdge &theEdge)
{
   _edges.push_back(theEdge);
   if (_borderInit)
   {
      edge polyBorder = theEdge.getZoom();
      _border.x1 = std::min(polyBorder.x1, _border.x1);
      _border.y1 = std::min(polyBorder.y1, _border.y1);
      _border.x2 = std::max(polyBorder.x2, _border.x2);
      _border.y2 = std::max(polyBorder.y2, _border.y2);
   }
   else
   {
      _border = theEdge.getZoom();
      _borderInit = true;
   }
}

Calbr::edge Calbr::drcRuleCheck::getZoom(long ordinal)
{
   edge ret;
   for (std::vector<Calbr::drcPolygon>::const_iterator it = _polygons.begin(); it
         != _polygons.end(); ++it)
   {
      if (ordinal == (*it).ordinal())
      {
         ret = (*it).getZoom();
         return ret;
      }
   }

   for (std::vector<Calbr::drcEdge>::const_iterator it = _edges.begin(); it
         != _edges.end(); ++it)
   {
      if (ordinal == (*it).ordinal())
      {
         ret = (*it).getZoom();
         return ret;
      }
   }
   throw EXPTNdrc_reader("Can't zoom to chosen element");
   return ret;
}

Calbr::edge Calbr::drcRuleCheck::getZoom(void)
{
   return _border;
}

//-----------------------------------------------------------------------------
Calbr::CalbrFile::CalbrFile(const std::string &fileName, drcRenderer *render)
      :_ok(true), _render(render),_isCellNameMode(false)
{
   _fileName = fileName;
}

Calbr::CalbrFile::~CalbrFile()
{
   if (!_RuleChecks.empty())
   {
      for(RuleChecksVector::const_iterator it= _RuleChecks.begin(); it != _RuleChecks.end(); ++it)
      {
         if ((*it)!= NULL) delete (*it);
      }
      _RuleChecks.clear();
   }

   if(!_cellDRCMap.empty())
   {
      for(CellDRCMap::const_iterator  it= _cellDRCMap.begin(); it != _cellDRCMap.end(); ++it)
      {
         if ((*it).second!= NULL)
         {
            cellNameStruct *CNstruct = (*it).second;
            for(RuleChecksVector::const_iterator it2= CNstruct->_RuleChecks.begin(); 
               it2 != CNstruct->_RuleChecks.end(); ++it2)
            {
               if ((*it2)!= NULL) delete (*it2);
            }
            delete (*it).second;
         }
      }
      _cellDRCMap.clear();
   }
   if (_render) delete _render;
}

void Calbr::CalbrFile::readFile()
{
   try
   {
      std::ostringstream ost;
      std::string fname(convertString(_fileName));
      if (!(_calbrFile = fopen(fname.c_str(),"rt"))) // open the input file
      {
         throw(EXPTNdrc_reader("Can't open file"));
      }

      //read header
      char str[512];
      if (fgets(str, 512, _calbrFile)==NULL)
      {
         std::string err;
         err += "Problem of reading file " + fname + "\n";
         err += "Can't read header";
         throw(EXPTNdrc_reader(err));
      }


      char cellName[512];
      if (sscanf( str, "%s %ld", cellName, &_precision) != 2)
      {
         std::string err;
         err += "Problem of reading file " + fname + "\n";
         err += "Can't read cell name or precision";
         throw(EXPTNdrc_reader(err));
      }
      //Initialization of static member drcPolygon class
      drcPolygon::_precision = _precision;
      drcEdge::_precision = _precision;
      _curCellName = cellName;
      _topCellName = cellName;
      unsigned int num = 1;

      cellNameStruct *CNStruct = DEBUG_NEW cellNameStruct;
      _cellDRCMap[_curCellName] = CNStruct;
      while(parse(num))
      {
         //Reset CellNameMode
         //Theoretically there is impossible to change mode from first appearance
         //But format contains mode flag for each rule check
         _isCellNameMode = false;
         num++;
      }


      addResults();

      if (_calbrFile) fclose(_calbrFile);

      if(isOk())
      {
         if(!_RuleChecks.empty())
         {
            _border = (*_RuleChecks.begin())->getZoom();
         }
         else
         {
            cellNameStruct *st = _cellDRCMap.begin()->second;
            Calbr::drcRuleCheck* rule = *(st->_RuleChecks.begin());
            _border = rule->getZoom();
         }
         for (RuleChecksVector::const_iterator it = _RuleChecks.begin(); it != _RuleChecks.end(); ++it)
         {
            edge tempBorder = (*it)->getZoom();
            if(tempBorder.x1 < _border.x1) _border.x1 = tempBorder.x1;
            if(tempBorder.y1 < _border.y1) _border.y1 = tempBorder.y1;
            if(tempBorder.x2 > _border.x2) _border.x2 = tempBorder.x2;
            if(tempBorder.y2 > _border.y2) _border.y2 = tempBorder.y2;
         }

         for (CellDRCMap::const_iterator it = _cellDRCMap.begin(); it != _cellDRCMap.end(); ++it)
         {
            RuleChecksVector checks = (*it).second->_RuleChecks;
            for (RuleChecksVector::const_iterator it2 = checks.begin(); it2 != checks.end(); ++it2)
            {  
               edge tempBorder = (*it2)->getZoom();
               if(tempBorder.x1 < _border.x1) _border.x1 = tempBorder.x1;
               if(tempBorder.y1 < _border.y1) _border.y1 = tempBorder.y1;
               if(tempBorder.x2 > _border.x2) _border.x2 = tempBorder.x2;
               if(tempBorder.y2 > _border.y2) _border.y2 = tempBorder.y2;
            }
         }

         _render->setCellName(_curCellName);
      }
   }
   catch (EXPTNdrc_parser&)
   {
      _ok = false;
      return;
   }
}

bool Calbr::CalbrFile::parse(unsigned int num)
{
   std::ostringstream ost;
   char ruleCheckName[512];

   // get drc Rule Check name
   if (fgets(ruleCheckName, 512, _calbrFile)==NULL) return false;

   //Remove LF from  ruleCheckName before creating ruleCheck
   _curRuleCheckName = std::string(ruleCheckName, strlen(ruleCheckName)-1);
   _curRuleCheck = DEBUG_NEW Calbr::drcRuleCheck(num, _curRuleCheckName);
   char tempStr[512];
   char timeStamp[512];
   long resCount, origResCount, descrStrCount;

   //Get Current Results Count, Orginal Results Count and description strings count
   if (fgets(tempStr, 512, _calbrFile)==NULL)
   {
      std::string err;
      err += "Can't read  rule ";
      err += ruleCheckName;
      throw(EXPTNdrc_reader(err));
   }
   if( sscanf(tempStr, "%ld %ld %ld %[^\n]\n",  &resCount, &origResCount, &descrStrCount, timeStamp) != 4)
   {
      throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
   };

   _curRuleCheck->setCurResCount(resCount);
   _curRuleCheck->setOrigResCount(origResCount);
   _curRuleCheck->setTimeStamp(timeStamp);

   //Get Description Strings
   for(long i= 0; i < descrStrCount; i++)
   {
      if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      }
      _curRuleCheck->addDescrString(tempStr);
   }
   //Get Results
   for(long i= 0; i < resCount; i++)
   {
      if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      }
      char type;
      long ordinal;
      short numberOfElem;
      if (sscanf( tempStr, "%c %ld %hd", &type, &ordinal, &numberOfElem) != 3)
      {
         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      };
               
      
      switch(type)
      {

         case 'p'   :
            {
               drcPolygon poly(ordinal, _render);
               if (!parsePoly(ruleCheckName ,poly, numberOfElem)) return false;
               _curRuleCheck->addPolygon(poly);
            }
            break;

         case 'e'   :
            {
               Calbr::drcEdge theEdge(ordinal, _render);
               if (!parseEdge(ruleCheckName ,theEdge, numberOfElem)) return false;
               _curRuleCheck->addEdge(theEdge);
            }
               break;
         default   :
            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      }
   }

   appendRuleCheckToCellName();
   return true;
}

bool Calbr::CalbrFile::parsePoly(char* ruleCheckName, drcPolygon & poly, int numberOfElem)
{
   char tempStr[512];
   std::ostringstream ost;

   for(short j =0; j< numberOfElem; j++)
   {
      long x, y;
      if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      }

      //Check Cell Name Mode
      if((tempStr[0]=='C') && (tempStr[1]=='N'))
      {
         if (_isCellNameMode) //Save _curRuleCheck in previous cellName and create copy of current _curRuleCheck
         {
            appendRuleCheckToCellName();
            _curRuleCheck = DEBUG_NEW drcRuleCheck(*_curRuleCheck);
         }

         if(parseCellNameMode(tempStr))
         {
            
         }
         else
         {
            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
         }
         //After parsing Cell Name Mode read next string 
         if (fgets(tempStr, 512, _calbrFile)==NULL)
         {
            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
         }
      }

      if (sscanf( tempStr, "%ld %ld", &x, &y)!= 2)
      {
         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      };
      poly.addCoord(x, y);
}

   return true;
}

bool Calbr::CalbrFile::parseEdge(char* ruleCheckName, drcEdge & edge, int numberOfElem)
{
   char tempStr[512];
   std::ostringstream ost;

   for(short j =0; j< numberOfElem; j++)
   {
      long x1, y1, x2, y2;
      if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
        throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      }

      //Check Cell Name Mode
      if((tempStr[0]=='C') && (tempStr[1]=='N'))
      {
         if (_isCellNameMode) //Save _curRuleCheck in previous cellName and create copy of current _curRuleCheck
         {
            appendRuleCheckToCellName();
            _curRuleCheck = DEBUG_NEW drcRuleCheck(*_curRuleCheck);
         }

         if(parseCellNameMode( tempStr))
         {
            //_curRuleCheck->addCellNameStruct(CNStruct);
         }
         else
         {
            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
         }
         //After parsing Cell Name Mode read next string 
         if (fgets(tempStr, 512, _calbrFile)==NULL)
         {
            throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
         }
      }

      if(sscanf( tempStr, "%ld %ld %ld %ld", &x1, &y1, &x2, &y2)!=4)
      {
         throw(EXPTNdrc_parser(drc_parse, ruleCheckName, tempStr));
      };
      edge.addCoord(x1, y1, x2, y2);
   }

   return true;
}

bool  Calbr::CalbrFile::parseCellNameMode(const std::string &parseString)
{
   cellNameStruct *CNStruct = DEBUG_NEW cellNameStruct;
   //Check for Cell Name results
   wxRegEx regex;
   //Regexp: CN cellname (with 'c' or withoout 'c') number1, number2 ... number6
   VERIFY(regex.Compile(wxT("(CN) ([$[:alnum:]_]+) (c{0,1}) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+) (-{0,1}[[:digit:]]+)")));
   wxString str=wxString(parseString.c_str(), wxConvUTF8);
   //wxString str = wxT("CN xxx c -1 2 3 4 5 6");
   if (regex.Matches(str))
   {
      std::string cellName(regex.GetMatch(str, 2).char_str());
      std::string str2(regex.GetMatch(str, 3).char_str());
      if (!tpdSTRxxxCMP(str2.c_str(), ""))
      {
         CNStruct->spaceCoords = false;
      }
      else
         if (!tpdSTRxxxCMP(str2.c_str(), "c"))
         {
            CNStruct->spaceCoords = true;
         }
         else
         {
            return false;
         }
      //Save tranformation matrix 
      long number;
      long a, b, c, d, tx, ty;
      regex.GetMatch(str, 4).ToLong(&number);
      a = number;
      regex.GetMatch(str, 5).ToLong(&number);
      b = number;
      regex.GetMatch(str, 6).ToLong(&number);
      c = number;
      regex.GetMatch(str, 7).ToLong(&number);
      d = number;
      regex.GetMatch(str, 8).ToLong(&number);
      tx = number;
      regex.GetMatch(str, 9).ToLong(&number);
      ty = number;
      CTM dummy;
//      CNStruct->transfMatrix.setCTM(a, b, c, d, tx, ty);
      CNStruct->transfMatrix.setCTM(dummy.a(), dummy.b(), dummy.c(), dummy.d(), dummy.tx(), dummy.ty());

      _isCellNameMode = true;
      _curCellName = cellName;
      CellDRCMap::iterator it = _cellDRCMap.find(cellName);
      if(it!= _cellDRCMap.end())
      {
         //???Redifinition of CNStruct 
         //May be we have to add cheking here
         //it->second = CNStruct;
         delete CNStruct;
      }
      else
      {
         _cellDRCMap[cellName] = CNStruct;
      }
      return true;
   }
   else return false;
}


void   Calbr::CalbrFile::addResults()
{

   /*_render->startWriting();
   if (!_RuleChecks.empty())
   {
      RuleChecksVector::const_iterator it;
      for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
      {
         addRuleCheck((*it));
      }
   }
   else*/
   {
      for (CellDRCMap::const_iterator it = _cellDRCMap.begin(); it != _cellDRCMap.end(); ++it)
      {
         _render->startWriting((*it).first);
         _render->setCellName((*it).first);
         _render->setTranformation((*it).second->transfMatrix);

         RuleChecksVector checks = (*it).second->_RuleChecks;
         for (RuleChecksVector::const_iterator it2 = checks.begin(); it2 != checks.end(); ++it2)
         {  
            addRuleCheck((*it2));
         }
         _render->endWriting();
      }
   }

   //_render->endWriting();
   _render->hideAll();
}

void   Calbr::CalbrFile::addRuleCheck(drcRuleCheck* check)
{
   _render->setError(check->num());
   std::vector <Calbr::drcPolygon>::iterator it2;
   std::vector <Calbr::drcPolygon> *polys = check->polygons();
   for(it2 = polys->begin(); it2 < polys->end(); ++it2)
   {
      (*it2).addError();
   }
   std::vector <Calbr::drcEdge>::iterator it2edge;
   std::vector <Calbr::drcEdge> *edges = check->edges();
   for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
   {
      (*it2edge).addError();
   }
}

void   Calbr::CalbrFile::showError(const std::string& cell, const std::string& error, long  number)
{
   Calbr::cellNameStruct* cellStruct =_cellDRCMap[cell];
   RuleChecksVector* ruleCheck = &(cellStruct->_RuleChecks);
   edge zoom;
   RuleChecksVector::const_iterator it;
   for(it = ruleCheck->begin(); it!= ruleCheck->end(); ++it)
   {
      std::string x = (*it)->ruleCheckName();
      if((*it)->ruleCheckName() == error)
      {
         _curCellName = cell;
         _render->hideAll();
         if(_render->showError((*it)->num()))
         {
            try
            {
               zoom = (*it)->getZoom(number);
            }
            catch (EXPTNdrc_reader&)
            {
               return;
            }
            _render->zoom(zoom);
         }
      }
   }
   assert(it == ruleCheck->end());
}


void   Calbr::CalbrFile::showCluster(const std::string & cell, const std::string & error)
{
   Calbr::cellNameStruct* cellStruct =_cellDRCMap[cell];
   RuleChecksVector* ruleChecks = &(cellStruct->_RuleChecks);

   edge zoom;
   RuleChecksVector::const_iterator it;
   for(it = ruleChecks->begin(); it!= ruleChecks->end(); ++it)
   {
      std::string x = (*it)->ruleCheckName();
      if((*it)->ruleCheckName() == error)
      {
         _render->hideAll();
         if (_render->showError((*it)->num()))
         {
            try
            {
               zoom = (*it)->getZoom();
            }
            catch (EXPTNdrc_reader&)
            {
               return;
            }
            _render->zoom(zoom);
         }
      }
   }
   assert(it == ruleChecks->end());
}

void   Calbr::CalbrFile::showAllErrors(void)
{
   _render->showAll();
   //_render->zoom(_border);
}

void   Calbr::CalbrFile::hideAllErrors(void)
{
   _render->hideAll();
}

std::string Calbr::CalbrFile::explainError(word lay)
{
   for(CellDRCMap::const_iterator m_it = _cellDRCMap.begin(); m_it!=_cellDRCMap.end(); ++m_it)
   {
      RuleChecksVector *ruleChecks = &((*m_it).second->_RuleChecks);
      for(RuleChecksVector::const_iterator it = ruleChecks->begin(); it!= ruleChecks->end(); ++it)
      {
         if ((*it)->num() == lay) return (*it)->ruleCheckName();
      }
   }
   assert(true);
   //dummy, to prevent compiler warnings!
   return "";
}

bool Calbr::CalbrFile::isCellNameMode(void)
{
   return !_cellDRCMap.empty();
}

CTM Calbr::CalbrFile::getCTM(const std::string & cell)
{
   CellDRCMap::iterator it = _cellDRCMap.find(cell);
   if (it == _cellDRCMap.end())
   {
      assert(true);
   }
   return it->second->transfMatrix;
}

void Calbr::CalbrFile::appendRuleCheckToCellName(void)
{
   CellDRCMap::iterator it = _cellDRCMap.find(_curCellName);
   if(it!= _cellDRCMap.end())
   {
      it->second->_RuleChecks.push_back(_curRuleCheck);
   }
   else
   {
      assert(true); // Something wrong. CellNameMode is set but no such cell
   }
}

wxString Calbr::convert(int number, long precision)
{
   float x   = float(number) / precision;
   int xint = x;
   wxString str1;
   wxString format = wxT("%");
   wxString xintstr, xfracstr;
   xintstr << xint;
   xfracstr << precision;
   format << xintstr.Length() << wxT(".") << xfracstr.Length() << wxT("f");
   str1.sprintf(format, x);
   return str1;
}
