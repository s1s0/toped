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
#include <math.h>
#include "auxdat.h"

auxdata::TdtGrcPoly::TdtGrcPoly(const PointVector& plst) :
   GrcData(          )
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

auxdata::TdtGrcPoly::TdtGrcPoly(int4b* pdata, unsigned psize) :
   GrcData  (           ),
   _pdata      ( pdata     ),
   _psize      ( psize     )
{}

auxdata::TdtGrcPoly::TdtGrcPoly(InputTdtFile* const tedfile) :
   GrcData  (           )
{
   _psize = tedfile->getWord();
   assert(_psize);
   _pdata = DEBUG_NEW int4b[_psize*2];
   TP wpnt;
   for (unsigned i = 0 ; i < _psize; i++)
   {
      wpnt = tedfile->getTP();
      _pdata[2*i  ] = wpnt.x();
      _pdata[2*i+1] = wpnt.y();
   }
}

auxdata::TdtGrcPoly::~TdtGrcPoly()
{
   delete [] _pdata;
}

DBbox auxdata::TdtGrcPoly::overlap() const
{
   DBbox ovl(_pdata[0], _pdata[1]) ;
   for (word i = 1; i < _psize; i++)
      ovl.overlap(_pdata[2*i], _pdata[2*i+1]);
   return ovl;
}

void auxdata::TdtGrcPoly::drawRequest(trend::TrendBase& rend) const
{
   rend.grcpoly(_pdata, _psize);
}

void auxdata::TdtGrcPoly::drawSRequest(trend::TrendBase& rend, const SGBitSet*) const
{
   rend.poly(_pdata, _psize, NULL, NULL);
}

void auxdata::TdtGrcPoly::info(std::ostringstream& ost, real DBU) const
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

void auxdata::TdtGrcPoly::write(OutputTdtFile* const tedfile) const
{
   tedfile->putByte(tedf_POLY);
   tedfile->putWord(_psize);
   for (unsigned i = 0; i < _psize; i++)
   {
      tedfile->put4b(_pdata[2*i]); tedfile->put4b(_pdata[2*i+1]);
   }
}

void auxdata::TdtGrcPoly::dbExport(DbExportFile& exportF) const
{
   exportF.polygon(_pdata, _psize);
}

void auxdata::TdtGrcPoly::motionDraw(const layprop::DrawProperties&, CtmQueue& transtack,
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


bool auxdata::TdtGrcPoly::pointInside(const TP pnt)const
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

PointVector auxdata::TdtGrcPoly::dumpPoints() const
{
   PointVector plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back(TP(_pdata[2*i], _pdata[2*i+1]));
   return plist;
}

laydata::ShapeList* auxdata::TdtGrcPoly::getRepaired() const
{
   PointVector pData = dumpPoints();
   laydata::ValidPoly check(pData);
   if (check.acceptable())
      return check.replacements();
   else
      return NULL;
}

//==============================================================================
auxdata::TdtGrcWire::TdtGrcWire(const PointVector& plst, WireWidth width) :
   GrcData  (           ),
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

auxdata::TdtGrcWire::TdtGrcWire(int4b* pdata, unsigned psize, WireWidth width) :
   GrcData  (           ),
   _pdata      ( pdata     ),
   _psize      ( psize     ),
   _width      ( width     )
{
}

auxdata::TdtGrcWire::TdtGrcWire(InputTdtFile* const tedfile) :
   GrcData  (           )
{
   _psize = tedfile->getWord();
   assert(_psize);
   if ((0 == tedfile->revision()) && (8 > tedfile->subRevision()))
      _width = tedfile->getWord();
   else
      _width = tedfile->get4ub();
   _pdata = DEBUG_NEW int4b[_psize*2];
   TP wpnt;
   for (unsigned i = 0 ; i < _psize; i++)
   {
      wpnt = tedfile->getTP();
      _pdata[2*i  ]  = wpnt.x();
      _pdata[2*i+1]  = wpnt.y();
   }
}

auxdata::TdtGrcWire::~TdtGrcWire()
{
   delete [] _pdata;
}

DBbox auxdata::TdtGrcWire::overlap() const
{
   laydata::WireContour wcontour(_pdata, _psize, _width);
   return wcontour.getCOverlap();
}

void auxdata::TdtGrcWire::drawRequest(trend::TrendBase& rend) const
{
//   rend.fwire(_pdata, _psize, _width);
   rend.grcwire(_pdata, _psize, _width);

}

void auxdata::TdtGrcWire::drawSRequest(trend::TrendBase& rend, const SGBitSet*) const
{
   rend.wire(_pdata, _psize, _width, NULL);
}

void auxdata::TdtGrcWire::info(std::ostringstream& ost, real DBU) const
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

void auxdata::TdtGrcWire::write(OutputTdtFile* const tedfile) const
{
   tedfile->putByte(tedf_WIRE);
   tedfile->putWord(_psize);
   tedfile->put4ub(_width);
   for (word i = 0; i < _psize; i++)
   {
      tedfile->put4b(_pdata[2*i]); tedfile->put4b(_pdata[2*i+1]);
   }
}

void auxdata::TdtGrcWire::dbExport(DbExportFile& exportF) const
{
   exportF.wire(_pdata, _psize, _width);
}

void auxdata::TdtGrcWire::motionDraw(const layprop::DrawProperties& drawprop,
               CtmQueue& transtack, SGBitSet* plst) const
{
   CTM trans = transtack.front();
   PointVector ptlist;
   laydata::WireContourAux wcontour(_pdata, _psize, _width, trans);
   wcontour.getRenderingData(ptlist);
//   openGlDrawLine(const_cast<layprop::DrawProperties&>(drawprop), ptlist);
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

bool auxdata::TdtGrcWire::pointInside(const TP pnt)const
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

float auxdata::TdtGrcWire::get_distance(const TP& p1, const TP& p2, const TP& p0) const
{
   if (p1.x() == p2.x())
      // if the segment is parallel to Y axis
      if ( ((p0.y() >= p1.y()) && (p0.y() <= p2.y()))
         ||((p0.y() <= p1.y()) && (p0.y() >= p2.y())) )
         return fabsf((float)(p0.x() - p1.x()));
      else return -1;
   else if (p1.y() == p2.y())
      // if the segment is parallel to X axis
      if ( ((p0.x() >= p1.x()) && (p0.x() <= p2.x()))
         ||((p0.x() <= p1.x()) && (p0.x() >= p2.x())) )
         return fabsf((float)(p0.y() - p1.y()));
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
         return fabsf((float)(Cn / sqrt(dist)));
      else return -1;
   }
}

PointVector auxdata::TdtGrcWire::dumpPoints() const
{
   PointVector plist;
   plist.reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
      plist.push_back(TP(_pdata[2*i], _pdata[2*i+1]));
   return plist;
}

laydata::ShapeList* auxdata::TdtGrcWire::getRepaired() const
{
   PointVector pData = dumpPoints();
   laydata::ValidWire check(pData, _width);
   if (check.acceptable())
      return check.replacements();
   else return NULL;
}

//==============================================================================
auxdata::GrcCell::GrcCell(std::string name) :
   _name          ( name            ),
   _layers        (                 ),
   _cellOverlap   ( DEFAULT_OVL_BOX )
{}

auxdata::GrcCell::GrcCell(InputTdtFile* const tedfile, std::string name) :
   _name          ( name            ),
   _layers        (                 ),
   _cellOverlap   ( DEFAULT_OVL_BOX )
{
   byte      recordtype;
   while (tedf_GRCEND != (recordtype = tedfile->getByte()))
   {
      switch (recordtype)
      {
         case    tedf_LAYER:
            readTdtLay(tedfile);
            break;
         default: throw EXPTNreadTDT("LAYER record type expected");
      }
   }
   // Sort the qtrees of the new cell
   bool emptyCell = fixUnsorted();
   assert(!emptyCell);
}

auxdata::GrcCell::~GrcCell()
{
   for (LayerHolderGrc::Iterator lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      lay->freeMemory();
      delete (*lay);
   }
   _layers.clear();
}

void auxdata::GrcCell::write(OutputTdtFile* const tedfile) const
{
   for (LayerHolderGrc::Iterator wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      assert(wl.editable());
      tedfile->putByte(tedf_LAYER);
      tedfile->putLayer(wl());
      for (QuadTreeGrc::Iterator DI = wl->begin(); DI != wl->end(); DI++)
         DI->write(tedfile);
      tedfile->putByte(tedf_LAYEREND);
   }
}

void auxdata::GrcCell::dbExport(DbExportFile& exportf) const
{
   for (LayerHolderGrc::Iterator wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      assert(wl.editable());
      if ( !exportf.layerSpecification(wl()) ) continue;
      for (QuadTreeGrc::Iterator DI = wl->begin(); DI != wl->end(); DI++)
         DI->dbExport(exportf);
   }
}

void auxdata::GrcCell::collectUsedLays(LayerDefList& laylist) const
{
   for(LayerHolderGrc::Iterator CL = _layers.begin(); CL != _layers.end(); CL++)
      if (CL.editable())
         laylist.push_back(CL());
}

auxdata::QuadTreeGrc* auxdata::GrcCell::secureLayer(const LayerDef& laydef)
{
   if (_layers.end() == _layers.find(laydef))
      _layers.add(laydef, DEBUG_NEW auxdata::QuadTreeGrc());
   return _layers[laydef];
}

auxdata::QTreeTmpGrc* auxdata::GrcCell::secureUnsortedLayer(const LayerDef& laydef)
{
   if (_tmpLayers.end() == _tmpLayers.find(laydef))
      _tmpLayers.add(laydef, DEBUG_NEW auxdata::QTreeTmpGrc(secureLayer(laydef)));
   return _tmpLayers[laydef];
}

void auxdata::GrcCell::getCellOverlap()
{
   if (0 == _layers.size())
      _cellOverlap = DEFAULT_OVL_BOX;
   else
   {
      LayerHolderGrc::Iterator LCI = _layers.begin();
      _cellOverlap = LCI->overlap();
      while (++LCI != _layers.end())
         _cellOverlap.overlap(LCI->overlap());
   }
}

bool auxdata::GrcCell::fixUnsorted()//FIXME! The method result is useless! the idea is to return empty cell! (see the callers!)
{
   bool empty = (0 == _tmpLayers.size());
   if (!empty)
   {
      typedef TmpLayerMapGrc::Iterator LCI;
      for (LCI lay = _tmpLayers.begin(); lay != _tmpLayers.end(); lay++)
      {
         lay->commit();
         delete (*lay);
      }
      _tmpLayers.clear();
      getCellOverlap();
   }
   return empty;
}

void auxdata::GrcCell::openGlRender(trend::TrendBase& rend, const CTM& trans,
                                     bool selected, bool active) const
{
   // Draw figures
   typedef LayerHolderGrc::Iterator LCI;
   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
   {
      //first - to check visibility of the layer
      if (rend.layerHidden(lay())) continue;
      //second - get internal layer number:
      //       - for regular database it is equal to the TDT layer number
      //       - for DRC database it is common for all layers - DRC_LAY
      LayerDef layDef = rend.getTenderLay(lay());
      switch (layDef.num())
      {
         case REF_LAY:
         case GRC_LAY:
         case DRC_LAY: assert(false); break;
         default     :
         {
            rend.setGrcLayer(true, layDef);
            lay->openGlRender(rend, NULL);
            rend.setGrcLayer(false, layDef);
            break;
         }
      }
   }
}

DBbox auxdata::GrcCell::getVisibleOverlap(const layprop::DrawProperties& prop)
{
   DBbox vlOverlap(DEFAULT_OVL_BOX);
   for (LayerHolderGrc::Iterator LCI = _layers.begin(); LCI != _layers.end(); LCI++)
   {
//      LayerNumber  layno  = LCI->first;
//      QuadTree* cqTree = LCI->second;
      if (!prop.layerHidden(LCI()))
         vlOverlap.overlap(LCI->overlap());
   }
   return vlOverlap;
}

void auxdata::GrcCell::motionDraw(trend::TrendBase& rend/*, CtmQueue& transtack, bool active*/) const
{
   //TODO
//   typedef LayerHolder::Iterator LCI;
//   for (LCI lay = _layers.begin(); lay != _layers.end(); lay++)
//      if (!drawprop.layerHidden(lay()))
//      {
//         layprop::tellRGB theColor;
//         if (const_cast<layprop::DrawProperties&>(drawprop).setCurrentColor(lay(), theColor))
//            glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
//         for (QuadTree::DrawIterator CI = lay->begin(drawprop, transtack); CI != lay->end(); CI++)
//            CI->motionDraw(drawprop, transtack, NULL);
//      }
////      transtack.pop_front();
}

void auxdata::GrcCell::readTdtLay(InputTdtFile* const tedfile)
{
   byte      recordtype;
   GrcData*  newData;
   LayerDef  laydef(tedfile->getLayer());
   QTreeTmpGrc* tmpLayer = secureUnsortedLayer(laydef);
   while (tedf_LAYEREND != (recordtype = tedfile->getByte()))
   {
      switch (recordtype)
      {
         case     tedf_POLY: newData = DEBUG_NEW TdtGrcPoly(tedfile);break;
         case     tedf_WIRE: newData = DEBUG_NEW TdtGrcWire(tedfile);break;
         //--------------------------------------------------
         default: throw EXPTNreadTDT("Unexpected record type");
      }
      tmpLayer->put(newData);
   }
}

void auxdata::GrcCell::reportLayers(LayerDefSet& grcLays)
{
   for (LayerHolderGrc::Iterator wl = _layers.begin(); wl != _layers.end(); wl++)
   {
      grcLays.insert(wl());
   }
}

void auxdata::GrcCell::reportLayData(const LayerDef& laydef, AuxDataList& dataList)
{
   LayerHolderGrc::Iterator wl = _layers.find(laydef);
   if (_layers.end() != wl)
   {
      for (QuadTreeGrc::Iterator DI = wl->begin(); DI != wl->end(); DI++)
         dataList.push_back(*DI);
   }
}

/*!
 * Clears all the objects on layer @laydef and gathers them in the provided list @recovered.
 * Returns:
 *    1 if the cell is not empty after the operation
 *    0 if the cell is empty after the operation
 *   -1 if the layer @laydef does not contain grc data (error condition)
 */
char auxdata::GrcCell::cleanLay(const LayerDef& laydef, AuxDataList& recovered)
{
   LayerHolderGrc::Iterator wl = _layers.find(laydef);
   if (_layers.end() != wl)
   {
      // first mark all the shapes from the target layer and gather them in
      // the list provided
      for (QuadTreeGrc::Iterator DI = wl->begin(); DI != wl->end(); DI++)
      {
         DI->setStatus(sh_selected);
         recovered.push_back(*DI);
      }
      // now remove them from the quadTree
      if ( wl->deleteMarked() )
      {
         if (wl->empty())
         {
            delete *wl; _layers.erase(laydef);
         }
         else wl->validate();
      }
      if (_layers.empty()) return 0;
      else return 1;
   }
   else return -1;
}

bool auxdata::GrcCell::repairData(const LayerDef& laydef, laydata::ShapeList& newData)
{
   LayerHolderGrc::Iterator wl = _layers.find(laydef);
   if (_layers.end() != wl)
   {
      for (QuadTreeGrc::Iterator DI = wl->begin(); DI != wl->end(); DI++)
      {
         laydata::ShapeList* objReplacement = DI->getRepaired();
         if (NULL != objReplacement)
         {
            for (laydata::ShapeList::const_iterator CS = objReplacement->begin(); CS != objReplacement->end(); CS++)
               newData.push_back(*CS);
            delete objReplacement;
            DI->setStatus(sh_recovered);
         }
      }
      return true;
   }
   else return false;
}

/*!
 * Clears all objects marked as sh_recovered from the quadTree and returns them
 * in the recovered list. Returns:
 *     1 if cell is not empty and the overlap has been changed
 *     0 if the cell is not empty and overlap remains the same
 *    -1 if the cell is empty and shall be removed
 */
char auxdata::GrcCell::cleanRepaired(const LayerDef& laydef, AuxDataList& recovered)
{
   DBbox old_overlap(_cellOverlap);
   LayerHolderGrc::Iterator wl = _layers.find(laydef);
   if (_layers.end() != wl)
   {
      // gather all invalid objects which had been recovered in an AuxdataList
      for (QuadTreeGrc::Iterator DI = wl->begin(); DI != wl->end(); DI++)
      {
         if (sh_recovered == DI->status())
            recovered.push_back(*DI);
      }
      // now remove them from the quadTree
      if ( wl->deleteMarked(sh_recovered) )
      {
         if (wl->empty())
         {
            delete (*wl); _layers.erase(laydef);
         }
         else wl->validate();
      }
   }
   if (_layers.empty())                 return -1; // empty cell
   else
   {
      getCellOverlap();
      if (old_overlap == _cellOverlap)  return  0; // same overlap
      else                              return  1; // overlap changed
   }
}

