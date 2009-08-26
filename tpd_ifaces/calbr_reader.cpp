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
#include "../tpd_bidfunc/tpdf_common.h"
#include "../tpd_parser/ted_prompt.h"
#include "../tpd_DB/datacenter.h"
#include "../src/toped.h"
#include <sstream>

// Global variables
Calbr::CalbrFile *DRCData = NULL;
extern tui::TopedFrame*    Toped;
extern console::ted_cmd*	Console;
extern DataCenter*         DATC;
extern const wxEventType   wxEVT_CANVAS_ZOOM;

long Calbr::drcPolygon::_precision = 0;
long Calbr::drcEdge::_precision = 0;

Calbr::drcRenderer::drcRenderer():
	_ATDB(NULL)
{

}

Calbr::drcRenderer::~drcRenderer()
{
}

void Calbr::drcRenderer::drawBegin()
{
	_startDrawing = true;
	try
   {
      _ATDB = DATC->lockDB();
		_drcLayer = DATC->getLayerNo("drcResults");
		assert(_drcLayer);
   }
   catch (EXPTNactive_DB) 
	{
		tell_log(console::MT_ERROR, "No Data base loaded");
	}
}

void Calbr::drcRenderer::drawPoly(const CoordsVector	&coords)
{
	if (_startDrawing)
	{
		_startDrawing = false;
		_maxx = coords.begin()->x;
      _minx = coords.begin()->x;
      _maxy = coords.begin()->y;
      _miny = coords.begin()->y;
	}

	if (_ATDB)
	{
		real DBscale = DATC->DBscale();
		pointlist *plDB = DEBUG_NEW pointlist();
		plDB->reserve(coords.size());

		for(CoordsVector::const_iterator it = coords.begin(); it!= coords.end(); ++it)
      {
			_maxx = std::max((*it).x, _maxx);
         _maxy = std::max((*it).y, _maxy);
         _minx = std::min((*it).x, _minx);
         _miny = std::min((*it).y, _miny);
			telldata::ttpnt* pt1 = DEBUG_NEW telldata::ttpnt((*it).x, (*it).y);
			plDB->push_back(TP(pt1->x(), pt1->y(), DBscale));
			delete pt1;
      }
		_ATDB->addpoly(_drcLayer, plDB, false);
	}
}

void Calbr::drcRenderer::drawLine(const edge &edge)
{
	if (_startDrawing)
	{
		_maxx = std::max(edge.x1, edge.x2);
		_maxy = std::max(edge.y1, edge.y2);
		_minx = std::min(edge.x1, edge.x2);
		_miny = std::min(edge.y1, edge.y2);
	}
	else
	{
		_maxx = std::max(_maxx, std::max(edge.x1, edge.x2));
		_maxy = std::max(_maxy, std::max(edge.y1, edge.y2));
		_minx = std::min(_minx, std::min(edge.x1, edge.x2));
		_miny = std::min(_miny, std::min(edge.y1, edge.y2));
	}

	real DBscale = DATC->DBscale();
   //Convert drcEdge to pointlist
   pointlist *plDB = DEBUG_NEW pointlist();
   plDB->reserve(2);

   telldata::ttpnt* pt1, *pt2;

   pt1 = DEBUG_NEW telldata::ttpnt(edge.x1, edge.y1);
   pt2 = DEBUG_NEW telldata::ttpnt(edge.x2, edge.y2);

	plDB->push_back(TP(pt1->x(), pt1->y(), DBscale));
	plDB->push_back(TP(pt2->x(), pt2->y(), DBscale));

	real      w = 0.01;   //width of line

   _ATDB->addwire(_drcLayer, plDB, static_cast<word>(rint(w * DBscale)), false);

   delete pt1;
   delete pt2;
   delete plDB;
}

void Calbr::drcRenderer::drawEnd()
{
	DATC->unlockDB();
	
	real DBscale = DATC->DBscale();

	DBbox *box = DEBUG_NEW DBbox(TP(_minx, _miny, DBscale), TP(_maxx, _maxy, DBscale));
	wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
	eventZOOM.SetInt(tui::ZOOM_WINDOW);
	eventZOOM.SetClientData(static_cast<void*>(box));
	wxPostEvent(Toped->view(), eventZOOM);

	tellstdfunc::RefreshGL();
}

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

void Calbr::drcEdge::showError(laydata::tdtdesign* atdb, word la)
{
	_render->drawLine(_coords);
}


void Calbr::drcPolygon::addCoord(long x, long y)
{
   real DBscale = DATC->DBscale();

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

void Calbr::drcPolygon::showError(laydata::tdtdesign* atdb, word la)
{
	_render->drawPoly(_coords);
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
		:_ok(true),_render(new drcRenderer)
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
					Calbr::drcEdge theEdge(ordinal, _render);
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

	_render->drawBegin();
	RuleChecksVector::const_iterator it;
	for(it= _RuleChecks.begin(); it < _RuleChecks.end(); ++it)
	{
		std::vector <Calbr::drcPolygon>::iterator it2;
		std::vector <Calbr::drcPolygon> *polys = (*it)->polygons();
		for(it2 = polys->begin(); it2 < polys->end(); ++it2)
		{
			(*it2).showError(_ATDB, 0);
		}
		std::vector <Calbr::drcEdge>::iterator it2edge;
		std::vector <Calbr::drcEdge> *edges = (*it)->edges();
		for(it2edge = edges->begin(); it2edge < edges->end(); ++it2edge)
		{
			(*it2edge).showError(_ATDB, 0);
		}
	}
	_render->drawEnd();
}

void	Calbr::CalbrFile::ShowError(const std::string & error, long  number)
{
	RuleChecksVector::const_iterator it;
	for(it = _RuleChecks.begin(); it!= _RuleChecks.end(); ++it)
	{
		if((*it)->ruleCheckName() == error)
		{
			drcRuleCheck *rule = (*it);
			for(std::vector <Calbr::drcPolygon>::iterator it2poly = rule->polygons()->begin(); 
				it2poly!=  rule->polygons()->end(); ++it2poly)
			{
	         if (number == (*it2poly).ordinal())
				{
					drcPolygon *poly = &(*it2poly);

					_render->drawBegin();
						poly->showError(_ATDB, 0);
					_render->drawEnd();
            }
			}

			for(std::vector <Calbr::drcEdge>::iterator it2edge = rule->edges()->begin(); 
				it2edge!=  rule->edges()->end(); ++it2edge)
			{
            if (number == (*it2edge).ordinal())
				{
					word drcLayer = DATC->getLayerNo("drcResults");
               assert(drcLayer);
					_render->drawBegin();
						(*it2edge).showError(_ATDB, 0);
					_render->drawEnd();
				}
			}
		}
	}
	assert(it == _RuleChecks.end());
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
	wxString str1;
	wxString format = wxT("%");
	wxString xintstr, xfracstr;
	xintstr << xint;
	xfracstr << precision;
	format << xintstr.Length() << wxT(".") << xfracstr.Length() << wxT("f");
	str1.sprintf(format, x); 
	return str1;
}