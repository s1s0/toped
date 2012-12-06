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
//   This file is a part of Toped project (C) 2001-2009 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Sep 16 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL Basic renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tolder.h"
#include "viewprop.h"
#include "trend.h"

//=============================================================================
//
// class TolderTV
//
void trend::TolderTV::draw(layprop::DrawProperties* drawprop)
{
   // First - deal with openGL translation matrix
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   drawprop->adjustAlpha(_refCell->alphaDepth() - 1);
   // ... and here we go ...
   if  (_alobjvx[line] > 0)
   {// Draw the wire centre lines
      for (SliceWires::const_iterator CSH = _line_data.begin(); CSH != _line_data.end(); CSH++)
      {
         (*CSH)->drctDrawCLine();
      }
   }
   if  (_alobjvx[cnvx] > 0)
   {// Draw convex polygons
      for (SliceObjects::const_iterator CSH = _cnvx_data.begin(); CSH != _cnvx_data.end(); CSH++)
      {
         (*CSH)->drctDrawContour();
         (*CSH)->drctDrawFill();
      }
   }
   if  (_alobjvx[ncvx] > 0)
   {// Draw non-convex polygons
      for (SlicePolygons::const_iterator CSH = _ncvx_data.begin(); CSH != _ncvx_data.end(); CSH++)
      {
         (*CSH)->drctDrawContour();
         (*CSH)->drctDrawFill();
      }
   }
   if (_alobjvx[cont] > 0)
   {// Draw the remaining non-filled shapes of any kind
      for (SliceObjects::const_iterator CSH = _cont_data.begin(); CSH != _cont_data.end(); CSH++)
      {
         (*CSH)->drctDrawContour();
      }
      for (RefTxtList::const_iterator CSH = _txto_data.begin(); CSH != _txto_data.end(); CSH++)
      {
         (*CSH)->drctDrawContour();
      }
   }
   glPopMatrix();
}

void trend::TolderTV::drawTexts(layprop::DrawProperties* drawprop)
{
   glPushMatrix();
   glMultMatrixd(_refCell->translation());
   drawprop->adjustAlpha(_refCell->alphaDepth() - 1);

   for (TrendStrings::const_iterator TSTR = _text_data.begin(); TSTR != _text_data.end(); TSTR++)
   {
      real ftm[16];
      (*TSTR)->ctm().oglForm(ftm);
      glPushMatrix();
      glMultMatrixd(ftm);
      glScalef(OPENGL_FONT_UNIT, OPENGL_FONT_UNIT, 1);
      (*TSTR)->draw(_filled, drawprop);
      glPopMatrix();
   }
   glPopMatrix();
}

//=============================================================================
//
// class TolderReTV
//
void trend::TolderReTV::draw(layprop::DrawProperties* drawprop)
{
   TrendRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->draw(drawprop);
   _chunk->swapRefCells(sref_cell);
}

void trend::TolderReTV::drawTexts(layprop::DrawProperties* drawprop)
{
   TrendRef* sref_cell = _chunk->swapRefCells(_refCell);
   _chunk->drawTexts(drawprop);
   _chunk->swapRefCells(sref_cell);
}

//=============================================================================
//
// class TolderLay
//
void trend::TolderLay::newSlice(TrendRef* const ctrans, bool fill, bool reusable, unsigned)
{
   assert( 0 == total_slctdx());
   newSlice(ctrans, fill, reusable);
}

void trend::TolderLay::newSlice(TrendRef* const ctrans, bool fill, bool reusable)
{
   _cslice = DEBUG_NEW TolderTV(ctrans, fill, reusable, 2 * _num_total_points, _num_total_indexs);
}

bool trend::TolderLay::chunkExists(TrendRef* const ctrans, bool filled)
{
   ReusableTTVMap::iterator achunk;
   if (filled)
   {
      if (_reusableFData.end() == ( achunk =_reusableFData.find(ctrans->name()) ) )
         return false;
   }
   else
   {
      if (_reusableCData.end() == ( achunk =_reusableCData.find(ctrans->name()) ) )
         return false;
   }
   _reLayData.push_back(DEBUG_NEW TolderReTV(achunk->second, ctrans));
   return true;
}

void trend::TolderLay::draw(layprop::DrawProperties* drawprop)
{
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      (*TLAY)->draw(drawprop);
   }
   for (TrendReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
   {
      (*TLAY)->draw(drawprop);
   }
}

void trend::TolderLay::drawSelected()
{
   for (SliceSelected::const_iterator SSL = _slct_data.begin(); SSL != _slct_data.end(); SSL++)
   {
      (*SSL)->drctDrawSlctd();
   }

}

void trend::TolderLay::drawTexts(layprop::DrawProperties* drawprop)
{
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
   {
      (*TLAY)->drawTexts(drawprop);
   }
   for (TrendReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
   {
      (*TLAY)->drawTexts(drawprop);
   }
}

//=============================================================================
//
// class TolderRefLay
//

void trend::TolderRefLay::draw(layprop::DrawProperties* drawprop)
{
   layprop::tellRGB theColor;
   if (drawprop->setCurrentColor(REF_LAY_DEF, theColor))
      glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());
   //drawprop->setCurrentColor(REF_LAY_DEF);
   drawprop->setLineProps(false);

   for (RefBoxList::const_iterator CSH = _cellRefBoxes.begin(); CSH != _cellRefBoxes.end(); CSH++)
   {
      if (1 < (*CSH)->alphaDepth())
      {
         (*CSH)->drctDrawContour();
      }
   }
   drawprop->setLineProps(true);
   for (RefBoxList::const_iterator CSH = _cellSRefBoxes.begin(); CSH != _cellSRefBoxes.end(); CSH++)
   {
      (*CSH)->drctDrawContour();
   }
}

//=============================================================================
//
// class Tolder
//
trend::Tolder::Tolder( layprop::DrawProperties* drawprop, real UU ) :
    TrendBase            (drawprop, UU)
{
   _refLayer = DEBUG_NEW TolderRefLay();
}


void trend::Tolder::grid( const real step, const std::string color)
{
   int gridstep = (int)rint(step / _UU);
   if ( abs((int)(_drawprop->scrCtm().a() * gridstep)) > GRID_LIMIT)
   {
      _drawprop->setGridColor(color);
      // set first grid step to be multiply on the step
      TP bl = TP(_drawprop->clipRegion().p1().x(),_drawprop->clipRegion().p2().y());
      TP tr = TP(_drawprop->clipRegion().p2().x(),_drawprop->clipRegion().p1().y());
      int signX = (bl.x() > 0) ? 1 : -1;
      int X_is = (int)((rint(abs(bl.x()) / gridstep)) * gridstep * signX);
      int signY = (tr.y() > 0) ? 1 : -1;
      int Y_is = (int)((rint(abs(tr.y()) / gridstep)) * gridstep * signY);

      //... and finaly draw the grid
      glBegin(GL_POINTS);
      for (int i = X_is; i < tr.x()+1; i += gridstep)
         for (int j = Y_is; j < bl.y()+1; j += gridstep)
            glVertex2i(i,j);
      glEnd();
   }
}

void trend::Tolder::zeroCross()
{
   glLineStipple(1,0xcccc);
   glEnable(GL_LINE_STIPPLE);
   glBegin(GL_LINES);
   glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, (GLfloat)0.7); // gray
   glVertex2i(0, _drawprop->clipRegion().p1().y());
   glVertex2i(0, _drawprop->clipRegion().p2().y());
   glVertex2i(_drawprop->clipRegion().p1().x(), 0);
   glVertex2i(_drawprop->clipRegion().p2().x(), 0);
   glEnd();
   glDisable(GL_LINE_STIPPLE);
}

void trend::Tolder::setLayer(const LayerDef& laydef, bool has_selected)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with REF_LAY by accident
   assert(REF_LAY_DEF != laydef);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(laydef))
   {
      _clayer = _data[laydef];
   }
   else
   {
      _clayer = DEBUG_NEW TolderLay();
      _data.add(laydef, _clayer);
   }
   if (has_selected)
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false, _cslctd_array_offset);
   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), false);
}

void trend::Tolder::setHvrLayer(const LayerDef& laydef)
{
   if (REF_LAY_DEF != laydef)
   {
      _clayer = DEBUG_NEW TolderLay();
      _data.add(laydef, _clayer);
      _clayer->newSlice(_cellStack.top(), false, false, 0 /*_cslctd_array_offset*/);
   }
}

void trend::Tolder::setGrcLayer(bool setEData, const LayerDef& laydef)
{
   if (setEData)
   {
      assert(_grcLayer == NULL);
      if (_grcData.end() != _grcData.find(laydef))
      {
         _grcLayer = _grcData[laydef];
      }
      else
      {
         _grcLayer = DEBUG_NEW TolderLay();
         _grcData.add(laydef, _grcLayer);
      }
      _grcLayer->newSlice(_cellStack.top(), false, false);
   }
   else
   {
      assert(_grcLayer != NULL);
      // post process the current layer
      _grcLayer->ppSlice();
      _grcLayer = NULL;
//      _cslctd_array_offset += _elayer->total_slctdx();
   }
}

bool trend::Tolder::chunkExists(const LayerDef& laydef, bool has_selected)
{
   // Reference layer is processed differently (pushCell), so make sure
   // that we haven't got here with REF_LAY by accident
   assert(REF_LAY_DEF != laydef);
   if (NULL != _clayer)
   { // post process the current layer
      _clayer->ppSlice();
      _cslctd_array_offset += _clayer->total_slctdx();
   }
   if (_data.end() != _data.find(laydef))
   {
      _clayer = _data[laydef];
      if (_clayer->chunkExists(_cellStack.top(), _drawprop->layerFilled(laydef) ) ) return true;
   }
   else
   {
      _clayer = DEBUG_NEW TolderLay();
      _data.add(laydef, _clayer);
   }
   if (has_selected)
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true, _cslctd_array_offset);
   else
      _clayer->newSlice(_cellStack.top(), _drawprop->layerFilled(laydef), true);
   return false;
}

bool trend::Tolder::collect()
{
   // First filter-out the layers that doesn't have any objects on them and
   // post process the last slices in the layers
   //
   DataLay::Iterator CCLAY = _data.begin();
   unsigned num_total_buffers = 0;
   unsigned num_total_slctdx = 0; // Initialise the number of total selected indexes
   unsigned num_total_strings = 0;
   while (CCLAY != _data.end())
   {
      CCLAY->ppSlice();
      num_total_strings += CCLAY->total_strings();
      if ((0 == CCLAY->total_points()) && (0 == CCLAY->total_strings()))
      {
         delete (*CCLAY);
         // Note! Careful here with the map iteration and erasing! Erase method
         // of map<> template doesn't return an iterator (unlike the list<>).
         // Despite the temptation to assume that the iterator will be valid after
         // the erase, it must be clear that erasing will invalidate the iterator.
         // If this is implemented more trivially using "for" cycle the code shall
         // crash, although it seems to work on certain platforms. Only seems -
         // it doesn't always crash, but it iterates in a weird way.
         // The implementation below seems to be the cleanest way to do this,
         // although it relies on my understanding of the way "++" operator should
         // be implemented
         _data.erase(CCLAY++());
      }
      else if (0 != CCLAY->total_points())
      {
         num_total_slctdx += CCLAY->total_slctdx();
         num_total_buffers++;
//         if (0 < CCLAY->total_indexs())
//            num_total_buffers++;
         ++CCLAY;
      }
      else
         ++CCLAY;
   }
   _clayer = NULL;
   if (0 < _refLayer->total_points())  num_total_buffers++; // reference boxes
   if (0 < num_total_slctdx      )     num_total_buffers++;  // selected
   // Check whether we have to continue after traversing
   if (0 == num_total_buffers)
   {
      if (0 == num_total_strings)  return false;
      else                         return true;
   }
   //
   // that's about it, there is nothing to collect down the hierarchy in
   // this implementation
   return true;
}

bool trend::Tolder::grcCollect()
{
   // First filter-out the layers that doesn't have any objects on them,
   // post process the last slices in the layers and also gather the number
   // of required virtual buffers
   //
   unsigned num_total_buffers = 0;

   DataLay::Iterator CCLAY = _grcData.begin();
   while (CCLAY != _grcData.end())
   {
      CCLAY->ppSlice();
      if (0 == CCLAY->total_points())
      {
         delete (*CCLAY);
         _grcData.erase(CCLAY++());
      }
      else if (0 != CCLAY->total_points())
      {
         num_total_buffers++;
         if (0 < CCLAY->total_indexs())
            num_total_buffers++;
         ++CCLAY;
      }
      else
         ++CCLAY;
   }
   // Check whether we have to continue after traversing
   if (0 == num_total_buffers) return false;
   return true;
}

void trend::Tolder::setColor(const LayerDef& layer)
{
   layprop::tellRGB theColor;
   if (_drawprop->setCurrentColor(layer, theColor))
      glColor4ub(theColor.red(), theColor.green(), theColor.blue(), theColor.alpha());

}

void trend::Tolder::setStipple()
{
   const byte* tellStipple = _drawprop->getCurrentFill();
   if (NULL == tellStipple)
   {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }
   else
   {
      byte FlipStillple [128];
      for (word i = 0; i < 32; i++)
         for (word j = 0; j < 4; j++)
            FlipStillple[(31-i)*4 + j] = tellStipple[i*4 + j];
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_POLYGON_STIPPLE);
      glPolygonStipple(FlipStillple);
   }
}

void trend::Tolder::setLine(bool selected)
{
   layprop::LineSettings curLine;
   _drawprop->getCurrentLine(curLine, selected);
   glLineWidth(curLine.width());
   if (0xffff == curLine.pattern())
   {
      glDisable(GL_LINE_STIPPLE);
      /*glDisable(GL_LINE_SMOOTH);*/
   }
   else
   {
      glEnable(GL_LINE_STIPPLE);
      /*glEnable(GL_LINE_SMOOTH);*/
      glLineStipple(curLine.patscale(),curLine.pattern());
   }
}

void trend::Tolder::draw()
{
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {// for every layer
      setColor(CLAY());
      setStipple();
      if (0 != CLAY->total_slctdx())
      {// redraw selected contours only
         _drawprop->setLineProps(true);
         glPushMatrix();
         glMultMatrixd(_activeCS->translation());
         CLAY->drawSelected();
         glPopMatrix();
      }
      _drawprop->setLineProps(false);
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
      // draw texts
      if (0 != CLAY->total_strings())
      {
         CLAY->drawTexts(_drawprop);
      }
   }
   // draw reference boxes
   if (0 < _refLayer->total_points())   _refLayer->draw(_drawprop);
   checkOGLError("draw");
}

void trend::Tolder::grcDraw()
{
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {// for every layer
      setColor(CLAY());
      setStipple();
      _drawprop->setLineProps(false);
      // draw everything
      if (0 != CLAY->total_points())
         CLAY->draw(_drawprop);
   }
   checkOGLError("grcDraw");
}

void trend::Tolder::cleanUp()
{
   TrendBase::cleanUp();
}

void trend::Tolder::grcCleanUp()
{
   TrendBase::grcCleanUp();
}

trend::Tolder::~Tolder()
{
   delete _refLayer;
}

