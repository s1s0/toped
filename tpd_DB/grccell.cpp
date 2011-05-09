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
//           $URL$
//        Created: Sat Mar 26 13:48:39 GMT 2011
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Handling of objects failing Graphical Rules Checks
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "grccell.h"

auxdata::TdtErrPoly::TdtErrPoly(const PointVector& plst) :
   TdtErrData(          )
{
   _psize = plst.size();
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   unsigned index = 0;
   for (unsigned i = 0; i < _psize; i++)
   {
      _pdata[index++] = plst[i].x();
      _pdata[index++] = plst[i].y();
   }
}

auxdata::TdtErrPoly::TdtErrPoly(int4b* pdata, unsigned psize) :
   TdtErrData  (           ),
   _pdata      ( pdata     ),
   _psize      ( psize     )
{}

auxdata::TdtErrPoly::~TdtErrPoly()
{
   delete [] _pdata;
}

DBbox auxdata::TdtErrPoly::overlap() const
{
   DBbox ovl(_pdata[0], _pdata[1]) ;
   for (word i = 1; i < _psize; i++)
      ovl.overlap(_pdata[2*i], _pdata[2*i+1]);
   return ovl;
}

void auxdata::TdtErrPoly::openGlPrecalc(layprop::DrawProperties& drawprop, PointVector& ptlist) const
{
   // translate the points using the current CTM
   ptlist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
   {
      ptlist.push_back(TP(_pdata[2*i], _pdata[2*i+1]) * drawprop.topCtm());
   }
}

void auxdata::TdtErrPoly::openGlDrawLine(layprop::DrawProperties&, const PointVector& ptlist) const
{
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < ptlist.size(); i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void auxdata::TdtErrPoly::openGlDrawFill(layprop::DrawProperties&, const PointVector&) const
{
   // Not filled!
}

void auxdata::TdtErrPoly::openGlDrawSel(const PointVector& ptlist, const SGBitSet*) const
{
   assert(0 != ptlist.size());
   if (sh_selected == status())
   {
      glBegin(GL_LINE_LOOP);
      for (unsigned i = 0; i < ptlist.size(); i++)
         glVertex2i(ptlist[i].x(), ptlist[i].y());
      glEnd();
   }
}

void auxdata::TdtErrPoly::drawRequest(tenderer::TopRend& rend) const
{
//   rend.fpoly(_pdata, _psize);
   rend.poly(_pdata, _psize, NULL );
}

void auxdata::TdtErrPoly::drawSRequest(tenderer::TopRend& rend, const SGBitSet*) const
{
   rend.poly(_pdata, _psize, NULL, NULL);
}

void auxdata::TdtErrPoly::info(std::ostringstream& ost, real DBU) const
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

bool auxdata::TdtErrPoly::pointInside(const TP pnt)const
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

PointVector auxdata::TdtErrPoly::dumpPoints() const
{
   PointVector plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back(TP(_pdata[2*i], _pdata[2*i+1]));
   return plist;
}


//==============================================================================
auxdata::TdtErrWire::TdtErrWire(const PointVector& plst, WireWidth width) :
   TdtErrData  (           ),
   _width      (width      )
{
   _psize = plst.size();
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   for (unsigned i = 0; i < _psize; i++)
   {
      _pdata[2*i  ] = plst[i].x();
      _pdata[2*i+1] = plst[i].y();
   }
}

auxdata::TdtErrWire::TdtErrWire(int4b* pdata, unsigned psize, WireWidth width) :
   TdtErrData  (           ),
   _pdata      ( pdata     ),
   _psize      ( psize     ),
   _width      ( width     )
{

}

auxdata::TdtErrWire::~TdtErrWire()
{
   delete [] _pdata;
}

DBbox auxdata::TdtErrWire::overlap() const
{
   laydata::WireContour wcontour(_pdata, _psize, _width);
   return wcontour.getCOverlap();
}

void auxdata::TdtErrWire::openGlPrecalc(layprop::DrawProperties& drawprop, PointVector& ptlist) const
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP((int4b)_width, (int4b)_width));
   bool center_line_only = !wsquare.visible(drawprop.topCtm() * drawprop.scrCtm(), drawprop.visualLimit());
   if (center_line_only)
   {
      ptlist.reserve(_psize+1);
      ptlist.push_back(TP(_psize, 0));
      for (unsigned i = 0; i < _psize; i++)
         ptlist.push_back(TP( _pdata[2*i], _pdata[2*i+1] ) * drawprop.topCtm());
   }
   else
   {
      laydata::WireContourAux wcontour(_pdata, _psize, _width, drawprop.topCtm());
      wcontour.getRenderingData(ptlist);
   }
}

void auxdata::TdtErrWire::openGlDrawLine(layprop::DrawProperties&, const PointVector& ptlist) const
{
   if (0 == ptlist.size()) return;
   word lsize = ptlist[0].x();
   word csize = ptlist[0].y();
   // the central line
   if (0 == lsize) return;
   glBegin(GL_LINE_STRIP);
   for (word i = 0; i < lsize; i++)
      glVertex2i(ptlist[i+1].x(), ptlist[i+1].y());
   glEnd();
   // the contour
   if (0 == csize) return;
   glBegin(GL_LINE_LOOP);
   for (word i = lsize; i <= lsize + csize; i++)
      glVertex2i(ptlist[i].x(), ptlist[i].y());
   glEnd();
}

void auxdata::TdtErrWire::openGlDrawFill(layprop::DrawProperties&, const PointVector&) const
{
   // Never filled
}

void auxdata::TdtErrWire::openGlDrawSel(const PointVector& ptlist, const SGBitSet* pslist) const
{
   if (0 == ptlist.size()) return;
   word lsize = ptlist[0].x();
   word csize = ptlist[0].y();
   if (0 == lsize) return;
   if (sh_selected == status())
   {
      glBegin(GL_LINE_STRIP);
      for (word i = 0; i < lsize; i++)
         glVertex2i(ptlist[i+1].x(), ptlist[i+1].y());
      glEnd();
   }
   else if (sh_partsel == status())
   {
      assert(pslist);
      glBegin(GL_LINES);
      for (unsigned i = 0; i < _psize-1; i++)
      {
         if (pslist->check(i) && pslist->check((i+1)%_psize))
         {
            glVertex2i(ptlist[i+1].x(), ptlist[i+1].y());
            glVertex2i(ptlist[(i+1)%_psize + 1].x(), ptlist[(i+1)%_psize + 1].y());
         }
      }
      if (csize > 0)
      {
         if (pslist->check(0))
         {// if only the first is selected
            glVertex2i(ptlist[lsize+csize/2].x(), ptlist[lsize+csize/2].y());
            glVertex2i(ptlist[lsize+csize/2+1].x(), ptlist[lsize+csize/2 + 1].y());
         }
         if (pslist->check(_psize-1))
         {// if only the last is selected
            glVertex2i(ptlist[lsize+1].x(), ptlist[lsize+1].y());
            glVertex2i(ptlist[lsize+csize].x(), ptlist[lsize+csize].y());
         }
      }
      glEnd();
   }
}

void auxdata::TdtErrWire::drawRequest(tenderer::TopRend& rend) const
{
//   rend.fwire(_pdata, _psize, _width);
   rend.wire(_pdata, _psize, _width);

}

void auxdata::TdtErrWire::drawSRequest(tenderer::TopRend& rend, const SGBitSet*) const
{
   rend.wire(_pdata, _psize, _width, NULL);
}

void auxdata::TdtErrWire::info(std::ostringstream& ost, real DBU) const
{
   ost << "wire " << _width/DBU << " - {";
   for (unsigned i = 0; i < _psize; i++)
   {
      TP cpnt(_pdata[2*i], _pdata[2*i+1]);
      cpnt.info(ost, DBU);
      if (i != _psize - 1) ost << " , ";
   }
   ost << "};";
}

bool auxdata::TdtErrWire::pointInside(const TP pnt)const
{
   TP p0, p1;
   for (unsigned i = 0; i < _psize - 1 ; i++)
   {
      p0 = TP(_pdata[2* i   ], _pdata[2* i   +1]);
      p1 = TP(_pdata[2*(i+1)], _pdata[2*(i+1)+1]);
      float distance = get_distance(p0,p1,pnt);
      if ((distance >= 0) && (distance <= _width/2))
         return true;
   }
   return false;
}

float auxdata::TdtErrWire::get_distance(const TP& p1, const TP& p2, const TP& p0) const
{
   if (p1.x() == p2.x())
      // if the segment is parallel to Y axis
      if ( ((p0.y() >= p1.y()) && (p0.y() <= p2.y()))
         ||((p0.y() <= p1.y()) && (p0.y() >= p2.y())) )
         return fabsf(p0.x() - p1.x());
      else return -1;
   else if (p1.y() == p2.y())
      // if the segment is parallel to X axis
      if ( ((p0.x() >= p1.x()) && (p0.x() <= p2.x()))
         ||((p0.x() <= p1.x()) && (p0.x() >= p2.x())) )
         return fabsf(p0.y() - p1.y());
      else return -1;
   else {
      // segment is not parallel to any axis
      float A = p2.y() - p1.y();
      float B = p1.x() - p2.x();
      float C = - (p1.y() * B) - (p1.x() * A);
      float dist = A*A + B*B;
      float Cn = A*p0.x() + B*p0.y() + C;
      float X = p0.x() - (A / dist) * Cn;
      float Y = p0.y() - (B / dist) * Cn;
      // now check that the new coordinate is on the p1-p2 line
      if ((((Y >= p1.y()) && (Y <= p2.y()))||((Y <= p1.y()) && (Y >= p2.y()))) &&
          (((X >= p1.x()) && (X <= p2.x()))||((X <= p1.x()) && (X >= p2.x())))   )
         return fabsf(Cn / sqrt(dist));
      else return -1;
   }
}

PointVector auxdata::TdtErrWire::dumpPoints() const
{
   PointVector plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back(TP(_pdata[2*i], _pdata[2*i+1]));
   return plist;
}

//==============================================================================
auxdata::GrcCell::GrcCell(std::string name) :
   _name          ( name            ),
   _layers        (                 ),
   _cellOverlap   ( DEFAULT_OVL_BOX ),
   _shapesel      (                 )
{}

auxdata::GrcCell::~GrcCell()
{
   for (LayerList::iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      lay->second->freeMemory();
      delete lay->second;
   }
   _layers.clear();
}

void auxdata::GrcCell::collectUsedLays(WordList& laylist) const
{
   for(LayerList::const_iterator CL = _layers.begin(); CL != _layers.end(); CL++)
      if (LAST_EDITABLE_LAYNUM > CL->first)
         laylist.push_back(CL->first);
}

auxdata::QuadTree* auxdata::GrcCell::secureLayer(unsigned layno)
{
   if (_layers.end() == _layers.find(layno))
      _layers[layno] = DEBUG_NEW auxdata::QuadTree();
   return _layers[layno];
}

auxdata::QTreeTmp* auxdata::GrcCell::secureUnsortedLayer(unsigned layno)
{
   if (_tmpLayers.end() == _tmpLayers.find(layno))
      _tmpLayers[layno] = DEBUG_NEW auxdata::QTreeTmp(secureLayer(layno));
   return _tmpLayers[layno];
}

void auxdata::GrcCell::getCellOverlap()
{
   if (0 == _layers.size())
      _cellOverlap = DEFAULT_OVL_BOX;
   else
   {
      LayerList::const_iterator LCI = _layers.begin();
      _cellOverlap = LCI->second->overlap();
      while (++LCI != _layers.end())
         _cellOverlap.overlap(LCI->second->overlap());
   }
}

bool auxdata::GrcCell::fixUnsorted()
{
   bool empty = (0 == _tmpLayers.size());
   if (!empty)
   {
      typedef TmpLayerMap::const_iterator LCI;
      for (LCI lay = _tmpLayers.begin(); lay != _tmpLayers.end(); lay++)
      {
         lay->second->commit();
         delete lay->second;
      }
      _tmpLayers.clear();
      getCellOverlap();
   }
   return empty;
}

void auxdata::GrcCell::openGlDraw(layprop::DrawProperties& drawprop, bool active) const
{
   // Draw figures
   typedef LayerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      unsigned curlayno = drawprop.getTenderLay(lay->first);
      if (!drawprop.layerHidden(curlayno)) drawprop.setCurrentColor(curlayno);
      else continue;
      // fancy like this (dlist iterator) , because a simple
      // _shapesel[curlayno] complains about loosing qualifiers (const)
      SelectList::const_iterator dlst;
      bool fill = drawprop.setCurrentFill(false);// honor block_fill state)
      if ((active) && (_shapesel.end() != (dlst = _shapesel.find(curlayno))))
         lay->second->openGlDraw(drawprop,dlst->second, fill);
      else
         lay->second->openGlDraw(drawprop, NULL, fill);
   }
}

void auxdata::GrcCell::openGlRender(tenderer::TopRend& rend, const CTM& trans,
                                     bool selected, bool active) const
{
//   rend.setBlinkData(true);
   //   rend.pushCell(_name, trans, _cellOverlap, active, selected);

   // Draw figures
   typedef LayerList::const_iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      //first - to check visibility of the layer
      if (rend.layerHidden(lay->first)) continue;
      //second - get internal layer number:
      //       - for regular database it is equal to the TDT layer number
      //       - for DRC database it is common for all layers - DRC_LAY
      unsigned curlayno = rend.getTenderLay(lay->first);
      // retrieve the selected objects (if they exists)
      SelectList::const_iterator dlsti;
      const DataList* dlist;
      if (active && (_shapesel.end() != (dlsti = _shapesel.find(curlayno))))
         dlist = dlsti->second;
      else
         dlist = NULL;
      switch (curlayno)
      {
         case REF_LAY:
         case GRC_LAY:
         case DRC_LAY: assert(false); break;
         default     :
         {
            rend.setLayer(curlayno, (NULL != dlist));
            lay->second->openGlRender(rend, dlist);
         }
      }
   }
//   rend.setBlinkData(false);
   //   rend.popCell();
}

DBbox auxdata::GrcCell::getVisibleOverlap(const layprop::DrawProperties& prop)
{
   DBbox vlOverlap(DEFAULT_OVL_BOX);
   for (LayerList::const_iterator LCI = _layers.begin(); LCI != _layers.end(); LCI++)
   {
      unsigned  layno  = LCI->first;
      QuadTree* cqTree = LCI->second;
      if (!prop.layerHidden(layno))
         vlOverlap.overlap(cqTree->overlap());
   }
   return vlOverlap;
}
