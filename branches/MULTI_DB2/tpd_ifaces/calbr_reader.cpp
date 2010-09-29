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

void Calbr::drcEdge::addCoord(long x1, long y1, long x2, long y2)
{
   real xx, yy;
   wxString xstr = convert(x1, _precision);
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
   _coords.y2 = yy;
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
   wxString xstr = convert(x, _precision);
   wxString ystr = convert(y, _precision);

   real xx, yy;
   xstr.ToDouble(&xx);
   ystr.ToDouble(&yy);

   Calbr::coord pt;
   pt.x = xx;
   pt.y = yy;
   _coords.push_back(pt);
}

void Calbr::drcPolygon::addError()
{
   _render->addPoly(_coords);
}

Calbr::edge Calbr::drcPolygon::getZoom() const
{
   CoordsVector::const_iterator it = _coords.begin();
   real minx = (*it).x;
   real miny = (*it).y;
   real maxx = (*it).x;
   real maxy = (*it).y;
   for (CoordsVector::const_iterator it = _coords.begin(); it != _coords.end(); ++it)
   {
      minx = std::min((*it).x, minx);
      miny = std::min((*it).y, miny);
      maxx = std::max((*it).x, maxx);
      maxy = std::max((*it).y, maxy);
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

Calbr::drcRuleCheck::~drcRuleCheck()
{
	if (_CNStruct) delete _CNStruct;
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

void Calbr::drcRuleCheck::addCellNameStruct(Calbr::cellNameStruct *cnStruct)
{
	_CNStruct = cnStruct;
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
      :_ok(true), _render(render)
{
   std::ostringstream ost;
   _fileName = fileName;
   std::string fname(convertString(_fileName));
   if (!(_calbrFile = fopen(fname.c_str(),"rt"))) // open the input file
   {
      _ok = false;
      ost << "Can't open file " << fname;
      tell_log(console::MT_ERROR,ost.str());
      return;
   }

   //read header
   char str[512];
   if (fgets(str, 512, _calbrFile)==NULL)
   {
      _ok = false;
      ost << "Problem of reading file " << fname;
      tell_log(console::MT_ERROR,ost.str());
      ost.str("");
      ost<<"Can't read header";
      tell_log(console::MT_ERROR,ost.str());
      return;
   }


   char cellName[512];
   if (sscanf( str, "%s %ld", cellName, &_precision) != 2)
   {
      _ok = false;
      ost << "Problem of reading file " << fname;
      tell_log(console::MT_ERROR,ost.str());
      ost.str("");
      ost<<"Can't read cell name or precision";
      tell_log(console::MT_ERROR,ost.str());
      return;

   }
   //Initialization of static member drcPolygon class
   drcPolygon::_precision = _precision;
   drcEdge::_precision = _precision;
   _cellName = cellName;
	unsigned int num = 1;
   while(parse(num))
   {
		num++;
   }
   addResults();

   if (_calbrFile) fclose(_calbrFile);

   if(isOk())
   {
	   _border = (*_RuleChecks.begin())->getZoom();
		for (RuleChecksVector::const_iterator it = _RuleChecks.begin(); it != _RuleChecks.end(); ++it)
		{
			edge tempBorder = (*it)->getZoom();
			if(tempBorder.x1 < _border.x1) _border.x1 = tempBorder.x1;
			if(tempBorder.y1 < _border.y1) _border.y1 = tempBorder.y1;
			if(tempBorder.x2 > _border.x2) _border.x2 = tempBorder.x2;
			if(tempBorder.y2 > _border.y2) _border.y2 = tempBorder.y2;
		}
		_render->setCellName(_cellName);
   }
}

Calbr::CalbrFile::~CalbrFile()
{
   RuleChecksVector::const_iterator it;
   if (!_RuleChecks.empty())
   {
      for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
      {
         if ((*it)!= NULL) delete (*it);
      }
      _RuleChecks.clear();
   }

   if (_render) delete _render;
}

bool Calbr::CalbrFile::parse(unsigned int num)
{
   std::ostringstream ost;
   char ruleCheckName[512];

   // get drc Rule Check name
   if (fgets(ruleCheckName, 512, _calbrFile)==NULL) return false;

   //Remove LF from  ruleCheckName before creating ruleCheck
   _curRuleCheck = DEBUG_NEW Calbr::drcRuleCheck(num, std::string(ruleCheckName, strlen(ruleCheckName)-1));
   char tempStr[512];
   char timeStamp[512];
   long resCount, origResCount, descrStrCount;

   //Get Current Results Count, Orginal Results Count and description strings count
   if (fgets(tempStr, 512, _calbrFile)==NULL)
   {
      _ok = false;
      ost << "Can't read  rule " << ruleCheckName;
      tell_log(console::MT_ERROR,ost.str());
      return false;
   }
   if( sscanf(tempStr, "%ld %ld %ld %[^\n]\n",  &resCount, &origResCount, &descrStrCount, timeStamp) != 4)
   {
      _ok = false;
      ost << "Can't parse  rule " << ruleCheckName;
      tell_log(console::MT_ERROR,ost.str());
      ost.str("");
      ost<<"string: " <<tempStr;
      return false;
   };
   _curRuleCheck->setCurResCount(resCount);
   _curRuleCheck->setOrigResCount(origResCount);
   _curRuleCheck->setTimeStamp(timeStamp);

   //Get Description Strings
   for(long i= 0; i < descrStrCount; i++)
   {
      if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
      }
      _curRuleCheck->addDescrString(tempStr);
   }
   //Get Results
   for(long i= 0; i < resCount; i++)
   {
      if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
      }
      char type;
      long ordinal;
      short numberOfElem;
      if (sscanf( tempStr, "%c %ld %hd", &type, &ordinal, &numberOfElem) != 3)
      {
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;

      };
					
		
		/*if (fgets(tempStr, 512, _calbrFile)==NULL)
      {
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
      }*/

      drcPolygon poly(ordinal, _render);
      switch(type)
      {
         case 'p'   :
            if (!parsePoly(ruleCheckName ,poly, numberOfElem)) return false;
            _curRuleCheck->addPolygon(poly);
            break;

         case 'e'   :
            {
               Calbr::drcEdge theEdge(ordinal, _render);
               if (!parseEdge(ruleCheckName ,theEdge, numberOfElem)) return false;
               _curRuleCheck->addEdge(theEdge);
            }
               break;
         default   :
            _ok = false;
            ost << "Can't parse  rule " << ruleCheckName;
            tell_log(console::MT_ERROR,ost.str());
            ost.str("");
            ost<<"string: " <<tempStr;
            tell_log(console::MT_ERROR,ost.str());
            return false;
      }
   }
   _RuleChecks.push_back(_curRuleCheck);
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
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
      }

		//Check Cell Name Mode
      if((tempStr[0]=='C') && (tempStr[1]=='N'))
      {
			cellNameStruct *CNStruct = DEBUG_NEW cellNameStruct;
			if(parseCellNameMode(CNStruct, tempStr))
			{
				_curRuleCheck->addCellNameStruct(CNStruct);
			}
			else
			{
				_ok = false;
				ost << "Can't parse  rule " << ruleCheckName;
				tell_log(console::MT_ERROR,ost.str());
				ost.str("");
				ost<<"string: " <<tempStr;
				tell_log(console::MT_ERROR,ost.str());
				return false;
			}
			//After parsing Cell Name Mode read next string 
			if (fgets(tempStr, 512, _calbrFile)==NULL)
			{
				_ok = false;
				ost << "Can't parse  rule " << ruleCheckName;
				tell_log(console::MT_ERROR,ost.str());
				ost.str("");
				ost<<"string: " <<tempStr;
				tell_log(console::MT_ERROR,ost.str());
				return false;
			}
      }

      if (sscanf( tempStr, "%ld %ld", &x, &y)!= 2)
      {
			_ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
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
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
      }

		//Check Cell Name Mode
      if((tempStr[0]=='C') && (tempStr[1]=='N'))
      {
			cellNameStruct *CNStruct = DEBUG_NEW cellNameStruct;
			if(parseCellNameMode(CNStruct, tempStr))
			{
				_curRuleCheck->addCellNameStruct(CNStruct);
			}
			else
			{
				_ok = false;
				ost << "Can't parse  rule " << ruleCheckName;
				tell_log(console::MT_ERROR,ost.str());
				ost.str("");
				ost<<"string: " <<tempStr;
				tell_log(console::MT_ERROR,ost.str());
				return false;
			}
			//After parsing Cell Name Mode read next string 
			if (fgets(tempStr, 512, _calbrFile)==NULL)
			{
				_ok = false;
				ost << "Can't parse  rule " << ruleCheckName;
				tell_log(console::MT_ERROR,ost.str());
				ost.str("");
				ost<<"string: " <<tempStr;
				tell_log(console::MT_ERROR,ost.str());
				return false;
			}
      }

      if(sscanf( tempStr, "%ld %ld %ld %ld", &x1, &y1, &x2, &y2)!=4)
      {
         _ok = false;
         ost << "Can't parse  rule " << ruleCheckName;
         tell_log(console::MT_ERROR,ost.str());
         ost.str("");
         ost<<"string: " <<tempStr;
         tell_log(console::MT_ERROR,ost.str());
         return false;
      };
      edge.addCoord(x1, y1, x2, y2);
   }

   return true;
}

bool  Calbr::CalbrFile::parseCellNameMode(cellNameStruct *CNStruct, const std::string &parseString)
{
	//Check for Cell Name results
	wxRegEx regex;
	//Regexp: CN cellname (with 'c' or withoout 'c') number1, number2 ... number6
	VERIFY(regex.Compile(wxT("(CN) ([[:alnum:]_]+) (c{0,1}) ([[:digit:]]+) ([[:digit:]]+) ([[:digit:]]+) ([[:digit:]]+) ([[:digit:]]+) ([[:digit:]]+)")));
	wxString str=wxString(parseString.c_str(), wxConvUTF8);
	//wxString str = wxT("CN xxx c 1 2 3 4 5 6");

	if (regex.Matches(str))
	{
		CNStruct->cellName = regex.GetMatch(str, 2).char_str();
		std::string str2 = regex.GetMatch(str, 3).char_str();
		if (!stricmp(str2.c_str(), "")) 
		{
			CNStruct->spaceCoords = false;
		}
		else
			if (!stricmp(str2.c_str(), "c")) 
			{
				CNStruct->spaceCoords = true;
			}
			else
			{
				return false;
			}
		//Save tranformation matrix 
		long number;
		regex.GetMatch(str, 4).ToLong(&number);
		CNStruct->a[0][0] = number;
		regex.GetMatch(str, 5).ToLong(&number);
		CNStruct->a[0][1] = number;
		regex.GetMatch(str, 6).ToLong(&number);
		CNStruct->a[0][2] = number;
		regex.GetMatch(str, 7).ToLong(&number);
		CNStruct->a[1][0] = number;
		regex.GetMatch(str, 8).ToLong(&number);
		CNStruct->a[1][1] = number;
		regex.GetMatch(str, 9).ToLong(&number);
		CNStruct->a[1][2] = number;
	}
	return true;
}


void   Calbr::CalbrFile::addResults()
{

   _render->startWriting();
   RuleChecksVector::const_iterator it;
   for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
   {
      _render->setError((*it)->num());
      std::vector <Calbr::drcPolygon>::iterator it2;
      std::vector <Calbr::drcPolygon> *polys = (*it)->polygons();
      for(it2 = polys->begin(); it2 < polys->end(); ++it2)
      {
         (*it2).addError();
      }
      std::vector <Calbr::drcEdge>::iterator it2edge;
      std::vector <Calbr::drcEdge> *edges = (*it)->edges();
      for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
      {
         (*it2edge).addError();
      }
   }
   _render->endWriting();
   _render->hideAll();
}

void   Calbr::CalbrFile::showError(const std::string & error, long  number)
{
   edge zoom;
   RuleChecksVector::const_iterator it;
   for(it = _RuleChecks.begin(); it!= _RuleChecks.end(); ++it)
   {
      std::string x = (*it)->ruleCheckName();
      if((*it)->ruleCheckName() == error)
      {
         _render->hideAll();
         if(_render->showError((*it)->num()))
         {
            try
            {
               zoom = (*it)->getZoom(number);
            }
            catch (EXPTNdrc_reader)
            {
               return;
            }
            _render->zoom(zoom);
         }
      }
   }
   assert(it == _RuleChecks.end());
}


void   Calbr::CalbrFile::showCluster(const std::string & error)
{
   edge zoom;
   RuleChecksVector::const_iterator it;
   for(it = _RuleChecks.begin(); it!= _RuleChecks.end(); ++it)
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
            catch (EXPTNdrc_reader)
            {
               return;
            }
            _render->zoom(zoom);
         }
      }
   }
   assert(it == _RuleChecks.end());
}

void   Calbr::CalbrFile::showAllErrors(void)
{
   _render->showAll();
   _render->zoom(_border);
}

void   Calbr::CalbrFile::hideAllErrors(void)
{
   _render->hideAll();
}

std::string Calbr::CalbrFile::explainError(word lay)
{
   for(RuleChecksVector::const_iterator it = _RuleChecks.begin(); it!= _RuleChecks.end(); ++it)
   {
      if ((*it)->num() == lay) return (*it)->ruleCheckName();
   }
   assert(true);
   //dummy, to prevent compiler warnings!
   return "";
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
