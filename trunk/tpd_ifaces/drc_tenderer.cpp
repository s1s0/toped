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
//           $URL: https://toped.googlecode.com/svn/trunk/tpd_ifaces/drc_tenderer.cpp $
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Interlayer between CalbrFile and Toped database
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 1204 $
//          $Date: 2009-08-26 23:20:02 +0800 (б№, 26 рту 2009) $
//        $Author: gaitukevich.s $
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include "drc_tenderer.h"
#include "../tpd_bidfunc/tpdf_common.h"
#include "../tpd_parser/ted_prompt.h"
#include "../src/toped.h"

// Global variables
Calbr::CalbrFile *DRCData = NULL;
extern tui::TopedFrame*    Toped;
extern console::ted_cmd*	Console;
extern DataCenter*         DATC;
extern const wxEventType   wxEVT_CANVAS_ZOOM;

Calbr::drcTenderer::drcTenderer():
	_ATDB(NULL)
{

}

Calbr::drcTenderer::~drcTenderer()
{
}

void Calbr::drcTenderer::drawBegin()
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

void Calbr::drcTenderer::drawPoly(const CoordsVector	&coords)
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

void Calbr::drcTenderer::drawLine(const edge &edge)
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

void Calbr::drcTenderer::drawEnd()
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