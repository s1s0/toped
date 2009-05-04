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
//        Created: Mon Aug 11 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GDSII/TDT hierarchy browser, layer browser, TELL fuction
//                 definition browser
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
#include "calbr_reader.h"
#include "../tpd_common/outbox.h"
#include "../tpd_parser/ted_prompt.h"
#include <sstream>

// Global variables
Calbr::CalbrFile *DRCData = NULL;

extern console::ted_cmd*	Console;
long Calbr::drcPolygon::_precision = 0;
long Calbr::drcEdge::_precision = 0;

void Calbr::drcEdge::addCoord(long x1, long y1, long x2, long y2)
{
	Calbr::edge crd;
	crd.x1 = x1;
	crd.y1 = y1;
	crd.x2 = x2;
	crd.y2 = y2;
	_coords.push_back(crd);
}

void Calbr::drcEdge::showError(void)
{
	wxString ost;
	ost << wxT("addpoly({");
	EdgesVector::iterator it;
	for(it = _coords.begin(); it < _coords.end(); ++it)
	{
		if (it != _coords.begin())
		{
			ost<<wxT(",");
		}


		ost << wxT("{") << convert((*it).x1, _precision) << wxT(",") << convert((*it).y1, _precision) 
			<< convert((*it).x2, _precision) << wxT(",") << convert((*it).y2, _precision) << wxT("}");;

	}
	ost<<wxT("});");
	Console->parseCommand(ost);
}


void Calbr::drcPolygon::addCoord(long x, long y)
{
	Calbr::coord crd;
	crd.x = x;
	crd.y = y;
	_coords.push_back(crd);
}

void Calbr::drcPolygon::showError(void)
{
	wxString ost;
	ost << wxT("addpoly({");
	CoordsVector::iterator it;
	//CoordsVector *poly1 = (*it).coords();
	for(it = _coords.begin(); it < _coords.end(); ++it)
	{
		if (it != _coords.begin())
		{
			ost<<wxT(",");
		}
		wxString xstr = convert((*it).x, _precision); 
		wxString ystr = convert((*it).y, _precision); 

		ost << wxT("{") << convert((*it).x, _precision) << wxT(",") << convert((*it).y, _precision) << wxT("}");;

	}
	ost<<wxT("});");
	Console->parseCommand(ost);
}

Calbr::drcRuleCheck::drcRuleCheck(const std::string &name)
		:_ruleCheckName(name)
{
}

void	Calbr::drcRuleCheck::setTimeStamp(const std::string &timeStamp)
{
	_timeStamp = timeStamp;
}

void	Calbr::drcRuleCheck::setCurResCount(int curResCount)
{
	_curResCount = curResCount;
}

void	Calbr::drcRuleCheck::setOrigResCount(int origResCount)
{
	_origResCount = origResCount;
}
	
void	Calbr::drcRuleCheck::addDescrString(const std::string & str)
{
	_descrStrings.push_back(str);
}


//-----------------------------------------------------------------------------
Calbr::CalbrFile::CalbrFile(const std::string &fileName)
		:_ok(true)
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
	sscanf( str, "%s %ud", cellName, &_precision);
	//initialisation of static member drcPolygon class
	drcPolygon::_precision = _precision;
	_cellName = cellName;
	while(parse())
	{

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
	fclose(_calbrFile);
}

bool Calbr::CalbrFile::parse()
{
	std::ostringstream ost;
	char ruleCheckName[512];

	// get drc Rule Check name
	if (fgets(ruleCheckName, 512, _calbrFile)==NULL)return false;


	Calbr::drcRuleCheck *ruleCheck = DEBUG_NEW Calbr::drcRuleCheck(ruleCheckName);
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
			return false;
		}
		ruleCheck->addDescrString(tempStr);
	}
	//Get Results

	for(long i= 0; i < resCount; i++)
	{
		drcPolygon poly;
		if (fgets(tempStr, 512, _calbrFile)==NULL) 		
		{
			_ok = false;
			ost << "Can't parse  rule " << ruleCheckName;
			tell_log(console::MT_ERROR,ost.str());
			return false;
		}
		char type;
		long ordinal;
		short numberOfElem;
		sscanf( tempStr, "%c %ld %hd", &type, &ordinal, &numberOfElem);
		switch(type)
		{
			case 'p'	: 

				for(short j =0; j< numberOfElem; j++)
				{
					long x, y;
					if (fgets(tempStr, 512, _calbrFile)==NULL) 
					{
						_ok = false;
						ost << "Can't parse  rule " << ruleCheckName;
						tell_log(console::MT_ERROR,ost.str());
						return false;
					}
					sscanf( tempStr, "%ld %ld", &x, &y);
					poly.addCoord(x, y);
				}
				ruleCheck->addPolygon(poly);
				break;
						
			case 'e'	: 
				long x1, y1, x2, y2;
				if (fgets(tempStr, 512, _calbrFile)==NULL) 
				{
					_ok = false;
					ost << "Can't parse  rule " << ruleCheckName;
					tell_log(console::MT_ERROR,ost.str());
					return false;
				}
				sscanf( tempStr, "%ld %ld %ld %ld", &x1, &y1, &x2, &y2);
				Calbr::edge theEdge;
				theEdge.x1 = x1;
				theEdge.y1 = y1;
				theEdge.x2 = x2;
				theEdge.y2 = y2;
				ruleCheck->addEdge(theEdge);
				break;
			default	:
				_ok = false;
				ost << "Can't parse  rule " << ruleCheckName;
				tell_log(console::MT_ERROR,ost.str());
				return false;
		}
	}
	_RuleChecks.push_back(ruleCheck);
	return true;

}

void	Calbr::CalbrFile::ShowResults()
{
	RuleChecksVector::const_iterator it;
	for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
	{
		std::vector <Calbr::drcPolygon>::iterator it2;
		std::vector <Calbr::drcPolygon> *polys = (*it)->polygons();
		for(it2 = polys->begin(); it2 < polys->end(); ++it2)
		{
			(*it2).showError();
		}
		std::vector <Calbr::edge>::iterator it2edge;
		std::vector <Calbr::edge> *edges = (*it)->edges();
		for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
		{
			wxString ost;
			ost << wxT("addpoly({");
			long x1int	= (*it2edge).x1 % _precision;
			long x1frac = (*it2edge).x1 - x1int*_precision;
			long y1int	= (*it2edge).y1 % _precision;
			long y1frac	= (*it2edge).y1 - y1int*_precision;
			long x2int	= (*it2edge).x2 % _precision;
			long x2frac = (*it2edge).x2 - x2int*_precision;
			long y2int	= (*it2edge).y2 % _precision;
			long y2frac = (*it2edge).y2 - y2int*_precision;
			
			ost << convert((*it2edge).x1, _precision) << wxT(",") 
				 << convert((*it2edge).y1, _precision) << wxT(",") 
				 << convert((*it2edge).x2, _precision) << wxT(",") 
				 << convert((*it2edge).y2, _precision) << ost<<wxT("}, 0.1)");
			Console->parseCommand(ost);
		}
	}

}



void Calbr::drcRuleCheck::addPolygon(const Calbr::drcPolygon &poly)
{
	_polygons.push_back(poly);
}

void Calbr::drcRuleCheck::addEdge(const Calbr::edge &theEdge)
{
	_edges.push_back(theEdge);
}

wxString Calbr::convert(int number, long precision)
{
	float x	= float(number) / precision;
	int xint = x;
	float xfrac = x - xint;

	wxString str1;
	wxString format = wxT("%");
	wxString xintstr, xfracstr;
	xintstr << xint;
	xfracstr << precision;
	format << xintstr.Length() << wxT(".") << xfracstr.Length() << wxT("f");
	str1.sprintf(format, x); 
	return str1;
}