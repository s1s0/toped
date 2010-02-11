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

void Calbr::drcEdge::addError(/*laydata::TdtDesign* atdb, */word la)
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

void Calbr::drcPolygon::addError(/*laydata::TdtDesign* atdb, */word la)
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
	for(CoordsVector::const_iterator it = _coords.begin(); it != _coords.end(); ++it)
	{
		if ((*it).x < minx ) minx = (*it).x;
		if ((*it).y < miny ) miny = (*it).y;
		if ((*it).x > maxx ) maxx = (*it).x;
		if ((*it).y > maxy ) maxy = (*it).y;
	};
	edge ret;
	ret.x1 = minx;
	ret.y1 = miny;
	ret.x2 = maxx;
	ret.y2 = maxy;
	return ret;
}

Calbr::drcRuleCheck::drcRuleCheck(unsigned int num, const std::string &name)
      :_num(num), _ruleCheckName(name)
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

Calbr::edge Calbr::drcRuleCheck::getZoom(long ordinal)
{
	edge ret;
	for(std::vector <Calbr::drcPolygon>::const_iterator it = _polygons.begin(); it != _polygons.end(); ++it)
	{
		if (ordinal == (*it).ordinal())
		{
			ret = (*it).getZoom();
			return ret;
		}
	}

	for(std::vector <Calbr::drcEdge>::const_iterator it = _edges.begin(); it != _edges.end(); ++it)
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

//-----------------------------------------------------------------------------
Calbr::CalbrFile::CalbrFile(const std::string &fileName, drcRenderer *render)
      :_ok(true),_render(render)
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
      return;
   }


   char cellName[512];
   sscanf( str, "%s %ld", cellName, &_precision);
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
   if (_calbrFile) fclose(_calbrFile);
}

bool Calbr::CalbrFile::parse(unsigned int num)
{
   std::ostringstream ost;
   char ruleCheckName[512];

   // get drc Rule Check name
   if (fgets(ruleCheckName, 512, _calbrFile)==NULL)return false;

   //Remove LF from  ruleCheckName befor creating ruleCheck
   Calbr::drcRuleCheck *ruleCheck = DEBUG_NEW Calbr::drcRuleCheck(num, std::string(ruleCheckName, strlen(ruleCheckName)-1));
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
   sscanf(tempStr, "%ld %ld %ld %[^\n]\n",  &resCount, &origResCount, &descrStrCount, timeStamp);
   ruleCheck->setCurResCount(resCount);
   ruleCheck->setOrigResCount(origResCount);
   ruleCheck->setTimeStamp(timeStamp);

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
      ruleCheck->addDescrString(tempStr);
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
      sscanf( tempStr, "%c %ld %hd", &type, &ordinal, &numberOfElem);
      drcPolygon poly(ordinal, _render);
      switch(type)
      {
         case 'p'   :
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
               sscanf( tempStr, "%ld %ld", &x, &y);
               poly.addCoord(x, y);
            }
            ruleCheck->addPolygon(poly);
            break;

         case 'e'   :
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
               sscanf( tempStr, "%ld %ld %ld %ld", &x1, &y1, &x2, &y2);
               Calbr::drcEdge theEdge(ordinal, _render);
               theEdge.addCoord(x1, y1, x2, y2);
               ruleCheck->addEdge(theEdge);
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
   _RuleChecks.push_back(ruleCheck);
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
         (*it2).addError(0);
      }
      std::vector <Calbr::drcEdge>::iterator it2edge;
      std::vector <Calbr::drcEdge> *edges = (*it)->edges();
      for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
      {
         (*it2edge).addError(0);
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
         _render->showError((*it)->num());
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
   assert(it == _RuleChecks.end());
}

void   Calbr::CalbrFile::showAllErrors(void)
{
   _render->showAll();
}

void   Calbr::CalbrFile::hideAllErrors(void)
{
   _render->hideAll();
}

void Calbr::drcRuleCheck::addPolygon(const Calbr::drcPolygon &poly)
{
   _polygons.push_back(poly);
}

void Calbr::drcRuleCheck::addEdge(const Calbr::drcEdge &theEdge)
{
   _edges.push_back(theEdge);
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
