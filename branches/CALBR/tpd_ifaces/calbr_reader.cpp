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
#include "calbr_reader.h"
#include "../tpd_common/outbox.h"
#include "../tpd_bidfunc/tpdf_common.h"
#include "../tpd_parser/ted_prompt.h"
#include "../tpd_DB/datacenter.h"
#include <sstream>

// Global variables
Calbr::CalbrFile *DRCData = NULL;

extern console::ted_cmd*	Console;
extern DataCenter*         DATC;

long Calbr::drcPolygon::_precision = 0;
long Calbr::drcEdge::_precision = 0;

void Calbr::drcEdge::addCoord(long x1, long y1, long x2, long y2)
{
	_coords.x1 = x1;
	_coords.y1 = y1;
	_coords.x2 = x2;
	_coords.y2 = y2;
}

void Calbr::drcEdge::showError(laydata::tdtdesign* atdb)
{
   real DBscale = DATC->DBscale();
   //Convert drcEdge to pointlist
   pointlist *plDB = DEBUG_NEW pointlist();
   plDB->reserve(2);

   telldata::ttpnt* pt1, *pt2;

   real xx, yy;
  // DATC->unlockDB();

   wxString xstr = convert(_coords.x1, _precision);
   wxString ystr = convert(_coords.y1, _precision);
   xstr.ToDouble(&xx);
   ystr.ToDouble(&yy);
   pt1 = DEBUG_NEW telldata::ttpnt(xx, yy);

   xstr = convert(_coords.x2, _precision);
   ystr = convert(_coords.y2, _precision);
   xstr.ToDouble(&xx);
   ystr.ToDouble(&yy);
   pt2 = DEBUG_NEW telldata::ttpnt(xx, yy);

   pt2 = DEBUG_NEW telldata::ttpnt(xx, yy);
   plDB->push_back(TP(pt1->x(), pt1->y(), DBscale));
   plDB->push_back(TP(pt2->x(), pt2->y(), DBscale));
   //ATDB->addpoly(1,plDB);
   real      w = 0.01;
//       telldata::ttlayout* wr = DEBUG_NEW telldata::ttlayout(ATDB->addwire(1,plDB,
//                                    static_cast<word>(rint(w * DBscale))), 1);
  // laydata::tdtdesign* ATDB = DATC->lockDB();
   atdb->addwire(1,plDB, static_cast<word>(rint(w * DBscale)));
   //DATC->unlockDB();
   delete pt1;
   delete pt2;
   delete plDB;
}


void Calbr::drcPolygon::addCoord(long x, long y)
{
	Calbr::coord crd;
	crd.x = x;
	crd.y = y;
	_coords.push_back(crd);
}

void Calbr::drcPolygon::showError(laydata::tdtdesign* atdb)
{

   real DBscale = DATC->DBscale();
   //Convert drcPolygon to pointlist
   pointlist *plDB = DEBUG_NEW pointlist();
   plDB->reserve(_coords.size());

   telldata::ttpnt* pt;
   for (unsigned i = 0; i < _coords.size(); i++)
   {
      wxString xstr = convert(_coords[i].x, _precision);
      wxString ystr = convert(_coords[i].y, _precision);
      real xx, yy;
      xstr.ToDouble(&xx);
      ystr.ToDouble(&yy);
      //pt = DEBUG_NEW telldata::ttpnt(_coords[i].x, (real)_coords[i].y);//((pl->mlist())[i]);
      pt = DEBUG_NEW telldata::ttpnt(xx, yy);
      plDB->push_back(TP(pt->x(), pt->y(), DBscale));
   }

   //laydata::tdtdesign* ATDB = DATC->lockDB();
   atdb->addpoly(1,plDB);
   //DATC->unlockDB();
   delete plDB;
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
	drcEdge::_precision = _precision;
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
	if (_calbrFile) fclose(_calbrFile);
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
		drcPolygon poly;
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
						
			case 'e'	: 
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
					Calbr::drcEdge theEdge;
					theEdge.addCoord(x1, y1, x2, y2);
					ruleCheck->addEdge(theEdge);
				}
				break;
			default	:
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

void	Calbr::CalbrFile::ShowResults()
{
	_ATDB = DATC->lockDB();

		RuleChecksVector::const_iterator it;
		for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
		{
			std::vector <Calbr::drcPolygon>::iterator it2;
			std::vector <Calbr::drcPolygon> *polys = (*it)->polygons();
			for(it2 = polys->begin(); it2 < polys->end(); ++it2)
			{
				(*it2).showError(_ATDB);
			}
			std::vector <Calbr::drcEdge>::iterator it2edge;
			std::vector <Calbr::drcEdge> *edges = (*it)->edges();
			for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
			{
				(*it2edge).showError(_ATDB);
			}
	DATC->unlockDB();
		/*for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
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
		}*/
	}
	tellstdfunc::RefreshGL();

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
	float x	= float(number) / precision;
	int xint = x;
//	float xfrac = x - xint;

	wxString str1;
	wxString format = wxT("%");
	wxString xintstr, xfracstr;
	xintstr << xint;
	xfracstr << precision;
	format << xintstr.Length() << wxT(".") << xfracstr.Length() << wxT("f");
	str1.sprintf(format, x); 
	return str1;
}