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
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Interlayer between CalbrFile and Toped database
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include "drc_tenderer.h"
#include "datacenter.h"
#include "tuidefs.h"
#include "tpdf_common.h"
#include "viewprop.h"
#include "tedat_ext.h"

#include <sstream>

// Global variables
Calbr::CalbrFile                *DRCData = NULL;
extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern wxWindow*                 TopedCanvasW;


extern const wxEventType         wxEVT_CANVAS_ZOOM;

Calbr::drcTenderer::drcTenderer(laydata::DrcLibrary* library)
{
   _ATDB = library;
   _ctm.Initialize();
}

Calbr::drcTenderer::~drcTenderer()
{
}

void Calbr::drcTenderer::setError(unsigned int numError)
{
   _numError = numError;
}

void Calbr::drcTenderer::startWriting(const std::string &cell)
{
   _startDrawing = true;
   _cell = cell;
   _DRCCell = DEBUG_NEW laydata::TdtCell(_cell);
//   PROPC->setState(layprop::DB);
}

void Calbr::drcTenderer::addPoly(const CoordsVector   &coords)
{
   if (_startDrawing)
   {
      _startDrawing = false;
      _max = TP(coords.begin()->x(), coords.begin()->y())*_ctm;
      _min = TP(coords.begin()->x(), coords.begin()->y())*_ctm;
   }

   if (_ATDB)
   {
//      real DBscale = 1000;
      PointVector plDB;
      plDB.reserve(coords.size());

      for(CoordsVector::const_iterator it = coords.begin(); it!= coords.end(); ++it)
      {
         TP tempPoint = (*it)*_ctm;
         _max.setX(std::max(tempPoint.x(), _max.x()));
         _max.setY(std::max(tempPoint.y(), _max.y()));
         _min.setX(std::min(tempPoint.x(), _min.x()));
         _min.setY(std::min(tempPoint.y(), _min.y()));
         plDB.push_back(tempPoint);
         //plDB.push_back(TP(tempPoint.x(), tempPoint.y(), DBscale));
      }
      laydata::QTreeTmp* dwl = _DRCCell->secureUnsortedLayer(_numError);
      PROPC->addUnpublishedLay(_numError);

      laydata::TdtPolyEXT *shape = DEBUG_NEW laydata::TdtPolyEXT(plDB);
      shape->setLong(_numError);
      shape->transfer(_ctm);
      dwl->put(shape);
   }
}

void Calbr::drcTenderer::addLine(const edge &edge)
{  
   TP tempPoint1 = TP(edge.x1, edge.y1)*_ctm;
   TP tempPoint2 = TP(edge.x2, edge.y2)*_ctm;
   if (_startDrawing)
   {
      _max = TP(edge.x1, edge.y1);
      _min = TP(edge.x1, edge.y1);
   }
   else
   {
      long maxx, maxy, minx, miny;
      maxx = std::max(_max.x(), std::max(tempPoint1.x(), tempPoint2.x()));
      maxy = std::max(_max.y(), std::max(tempPoint1.y(), tempPoint2.y()));
      minx = std::min(_min.x(), std::min(tempPoint1.x(), tempPoint2.x()));
      miny = std::min(_min.y(), std::min(tempPoint1.y(), tempPoint2.y()));
      _max = TP(maxx, maxy);
      _min = TP(minx, miny);
   }

   real DBscale = 1000 ;
   //Convert drcEdge to PointVector
   PointVector plDB;
   plDB.reserve(2);

   plDB.push_back(tempPoint1);
   plDB.push_back(tempPoint2);
   //plDB.push_back(TP( tempPoint1.x(), tempPoint1.y(), DBscale));
   //plDB.push_back(TP( tempPoint2.x(), tempPoint2.y(), DBscale));

   real      w = 0.01;   //width of line
   word      width = static_cast<word>(rint(w * DBscale));

   laydata::QTreeTmp* dwl = _DRCCell->secureUnsortedLayer(_numError);
   PROPC->addUnpublishedLay(_numError);


   /*laydata::ValidWire check(plDB, width);

   if (!check.valid())
   {
      std::ostringstream ost;
      ost << "Wire check fails - {" << check.failType() << " }";
      tell_log(console::MT_ERROR, ost.str());
   }
   else plDB = check.getValidated();
*/
   laydata::TdtWireEXT *shape = DEBUG_NEW laydata::TdtWireEXT(plDB, width);
   shape->setLong(_numError);
   shape->transfer(_ctm);
   dwl->put(shape);
}

void Calbr::drcTenderer::showAll(void)
{
   if(checkCellName())
   {
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp, layprop::DRC))
      {
         LayerTMPList lays = drawProp->getAllLayers();
         for(LayerTMPList::const_iterator it = lays.begin(); it != lays.end(); ++it)
            drawProp->hideLayer((*it), false);
      }
      PROPC->unlockDrawProp(drawProp, true);
      edge zoomEdge;
      zoomEdge.x1 = _min.x();
      zoomEdge.y1 = _min.y();
      zoomEdge.x2 = _max.x();
      zoomEdge.y2 = _max.y();
      zoom(zoomEdge);
      tellstdfunc::RefreshGL();
   }
   else
   {
      std::ostringstream ost;
      ost << "Wrong cell, expected:" << "\n" << _cellName;
      tell_log(console::MT_ERROR, ost.str());
   }
}

void Calbr::drcTenderer::hideAll(void)
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp, layprop::DRC))
   {
      LayerTMPList lays = drawProp->getAllLayers();
      for(LayerTMPList::const_iterator it = lays.begin(); it != lays.end(); ++it)
         drawProp->hideLayer((*it), true);
   }
   PROPC->unlockDrawProp(drawProp, true);
   tellstdfunc::RefreshGL();
}

bool Calbr::drcTenderer::showError(unsigned int numError)
{
   if(checkCellName())
   {
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp, layprop::DRC))
      {
         drawProp->hideLayer(numError, false);
      }
      PROPC->unlockDrawProp(drawProp, true);
      tellstdfunc::RefreshGL();
      return true;
   }
   else
   {
      std::ostringstream ost;
      ost << "Wrong cell, expected:" << "\n" << _cellName;
      tell_log(console::MT_ERROR, ost.str());
      return false;
   }
}

void Calbr::drcTenderer::zoom(const edge &edge)
{
//   real DBscale = PROPC->DBscale();
   TP zoomPoint1 = TP(edge.x1, edge.y1);
   TP zoomPoint2 = TP(edge.x2, edge.y2);
   DBbox* box = DEBUG_NEW DBbox(zoomPoint1,
                                zoomPoint2);
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(TopedCanvasW, eventZOOM);
}


void Calbr::drcTenderer::endWriting()
{
   _DRCCell->fixUnsorted();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp, layprop::DRC))
   {
      if (!PROPC->upLayers().empty())
      {
         const LayerTMPList freshlays = PROPC->upLayers();
         for(LayerTMPList::const_iterator CUL = freshlays.begin(); CUL != freshlays.end(); CUL++)
            drawProp->addLayer((*CUL));
         PROPC->clearUnpublishedLayers();
      }
   }
   PROPC->unlockDrawProp(drawProp, true);
   _ATDB->registerCellRead(_cell, _DRCCell);
}

bool Calbr::drcTenderer::checkCellName()
{
   std::string activeCell;
   laydata::TdtLibDir *libDir;
   DATC->lockTDT(libDir, dbmxs_liblock);
      laydata::TdtDesign *design = (*libDir)();
      activeCell = design->activeCellName();
   DATC->unlockTDT(libDir);
    //bool ret = _cellName==activeCell;
    return true;
}
