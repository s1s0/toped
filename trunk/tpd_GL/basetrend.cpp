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
//        Created: Wed Sep 12 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Base class for all openGL renderers
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tenderer.h"
#include "viewprop.h"
#include "trend.h"

//=============================================================================
//
// class TrendRef
//
trend::TrendRef::TrendRef(std::string name, const CTM& ctm, const DBbox& obox,
                   word alphaDepth) :
   _name          ( name         ),
   _ctm           ( ctm          ),
   _alphaDepth    ( alphaDepth   )
{
   _ctm.oglForm(_translation);
   TP tp = TP(obox.p1().x(), obox.p1().y()) * _ctm;
   _obox[0] = tp.x();_obox[1] = tp.y();
   tp = TP(obox.p2().x(), obox.p1().y()) * _ctm;
   _obox[2] = tp.x();_obox[3] = tp.y();
   tp = TP(obox.p2().x(), obox.p2().y()) * _ctm;
   _obox[4] = tp.x();_obox[5] = tp.y();
   tp = TP(obox.p1().x(), obox.p2().y()) * _ctm;
   _obox[6] = tp.x();_obox[7] = tp.y();
}


trend::TrendRef::TrendRef() :
   _name       ( ""   ),
   _ctm        (      ),
   _alphaDepth ( 0    )
{
   _ctm.oglForm(_translation);
   for (word i = 0; i < 8; _obox[i++] = 0);
}

unsigned trend::TrendRef::cDataCopy(TNDR_GLDATAT* array, unsigned& pindex)
{
#ifdef TENDERER_USE_FLOATS
   for (unsigned i = 0; i <  8; i++)
      array[pindex+i] = (TNDR_GLDATAT) _obox[i];
#else
   memcpy(&(array[pindex]), _obox, sizeof(TNDR_GLDATAT) * 8);
#endif
   pindex += 8;
   return 4;
}

void trend::TrendRef::drctDrawContour()
{
   glBegin(GL_LINE_LOOP);
   for (unsigned i = 0; i < 4; i++)
      glVertex2i(_obox[2*i], _obox[2*i+1]);
   glEnd();
}

//=============================================================================
//
// class TrendTV
//
trend::TrendTV::TrendTV(TrendRef* const refCell, bool filled, bool reusable,
                   unsigned parray_offset, unsigned iarray_offset) :
   _refCell             ( refCell         ),
   _num_total_strings   ( 0u              ),
   _filled              ( filled          ),
   _reusable            ( reusable        )
{
   for (int i = fqss; i <= ftss; i++)
   {
      _alobjix[i] = 0u;
      _alindxs[i] = 0u;
   }
   for (int i = cont; i <= ncvx; i++)
   {
      _alobjvx[i] = 0u;
      _alvrtxs[i] = 0u;
   }
}

void trend::TrendTV::registerBox (TrxCnvx* cobj)
{
   unsigned allpoints = cobj->csize();
   if (_filled)
   {
      _cnvx_data.push_back(cobj);
      _alvrtxs[cnvx] += allpoints;
      _alobjvx[cnvx]++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _alvrtxs[cont] += allpoints;
      _alobjvx[cont]++;
   }
}

void trend::TrendTV::registerPoly (TrxNcvx* cobj, const TessellPoly* tchain)
{
   unsigned allpoints = cobj->csize();
   if (_filled && tchain && tchain->valid())
   {
      cobj->setTeselData(tchain);
      _ncvx_data.push_back(cobj);
      _alvrtxs[ncvx] += allpoints;
      _alobjix[ftrs] += tchain->num_ftrs();
      _alobjix[ftfs] += tchain->num_ftfs();
      _alobjix[ftss] += tchain->num_ftss();
      tchain->num_indexs(_alindxs[ftrs], _alindxs[ftfs], _alindxs[ftss]);
      _alobjvx[ncvx]++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _alvrtxs[cont] += allpoints;
      _alobjvx[cont]++;
   }
}

void trend::TrendTV::registerWire (TrxWire* cobj)
{
   unsigned allpoints = cobj->csize();
   _line_data.push_back(cobj);
   _alvrtxs[line] += cobj->lsize();
   _alobjvx[line]++;
   if ( !cobj->center_line_only() )
   {
       if (_filled)
       {
         cobj->Tesselate();
         _ncvx_data.push_back(cobj);
         _alvrtxs[ncvx] += allpoints;
         _alindxs[fqss] += allpoints;
         _alobjvx[ncvx]++;
         _alobjix[fqss]++;
       }
       else
       {
          _cont_data.push_back(cobj);
          _alobjvx[cont] ++;
          _alvrtxs[cont] += allpoints;
       }
   }
}

void trend::TrendTV::registerText (TrxText* cobj, TrxTextOvlBox* oobj)
{
   _text_data.push_back(cobj);
   _num_total_strings++;
   if (NULL != oobj)
   {
      _txto_data.push_back(oobj);
      _alvrtxs[cont] += 4;
      _alobjvx[cont]++;
   }
}

unsigned trend::TrendTV::num_total_points()
{
   return ( _alvrtxs[cont] +
            _alvrtxs[line] +
            _alvrtxs[cnvx] +
            _alvrtxs[ncvx]
          );
}

unsigned trend::TrendTV::num_total_indexs()
{
   return ( _alindxs[fqss] +
            _alindxs[ftrs] +
            _alindxs[ftfs] +
            _alindxs[ftss]
          );
}

trend::TrendRef* trend::TrendTV::swapRefCells(TrendRef* newRefCell)
{
   TrendRef* the_swap = _refCell;
   _refCell = newRefCell;
   return the_swap;
}

void trend::TrendTV::setAlpha(layprop::DrawProperties* drawprop)
{
   layprop::tellRGB tellColor;
   if (drawprop->getAlpha(_refCell->alphaDepth() - 1, tellColor))
   {
      glColor4ub(tellColor.red(), tellColor.green(), tellColor.blue(), tellColor.alpha());
   }
}

trend::TrendTV::~TrendTV()
{
   for (SliceWires::const_iterator CSO = _line_data.begin(); CSO != _line_data.end(); CSO++)
      if ((*CSO)->center_line_only()) delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cnvx_data.begin(); CSO != _cnvx_data.end(); CSO++)
      delete (*CSO);
   for (SliceObjects::const_iterator CSO = _cont_data.begin(); CSO != _cont_data.end(); CSO++)
      delete (*CSO);
   for (SlicePolygons::const_iterator CSO = _ncvx_data.begin(); CSO != _ncvx_data.end(); CSO++)
      delete (*CSO);
   for (TrendStrings::const_iterator CSO = _text_data.begin(); CSO != _text_data.end(); CSO++)
      delete (*CSO);
   for (RefTxtList::const_iterator CSO = _txto_data.begin(); CSO != _txto_data.end(); CSO++)
      delete (*CSO);
   // Don't delete  _tmatrix. It's only a reference to it here
}

//=============================================================================
//
// class TrendLay
//
trend::TrendLay::TrendLay():
   _cslice               (        NULL ),
   _num_total_points     (          0u ),
   _num_total_indexs     (          0u ),
   _num_total_slctdx     (          0u ),
   _num_total_strings    (          0u )
{
   for (int i = lstr; i <= lnes; i++)
   {
      _asindxs[i] = 0u;
      _asobjix[i] = 0u;
   }
}

/** Add the current slice object (_cslice) to the list of slices _layData but
only if it's not empty. Also track the total number of vertexes in the layer
*/
void trend::TrendLay::ppSlice()
{
   if (NULL != _cslice)
   {
      unsigned num_points  = _cslice->num_total_points();
      unsigned num_strings = _cslice->num_total_strings();
      if ((num_points > 0) || (num_strings > 0))
      {
         _layData.push_back(_cslice);
         _num_total_points  += num_points;
         _num_total_strings += num_strings;
         _num_total_indexs  += _cslice->num_total_indexs();
         if (_cslice->reusable())
         {
            if (_cslice->filled())
            {
               assert(_reusableFData.end() == _reusableFData.find(_cslice->cellName()));
               _reusableFData[_cslice->cellName()] = _cslice;
            }
            else
            {
               assert(_reusableCData.end() == _reusableCData.find(_cslice->cellName()));
               _reusableCData[_cslice->cellName()] = _cslice;
            }
         }
      }
      else
         delete _cslice;
      _cslice = NULL;
   }
}

void trend::TrendLay::box  (const int4b* pdata)
{
   _cslice->registerBox(DEBUG_NEW TrxBox(pdata));
}

void trend::TrendLay::box  (const TP& p1, const CTM& rmm)
{
   _cslice->registerBox(DEBUG_NEW TrxTBox(p1, rmm));
}

void trend::TrendLay::box (const int4b* pdata, const SGBitSet* ss)
{
   TrxSBox* sobj = DEBUG_NEW TrxSBox(pdata, ss);
   registerSBox(sobj);
   _cslice->registerBox(sobj);
}

void trend::TrendLay::box (const int4b* pdata, const SGBitSet* ss, const CTM& rmm)
{
   TrxSBox* sobj = DEBUG_NEW TrxSMBox(pdata, ss, rmm);
   registerSBox(sobj);
   _cslice->registerBox(sobj);
}

void trend::TrendLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly)
{
   _cslice->registerPoly(DEBUG_NEW TrxNcvx(pdata, psize), tpoly);
}

void trend::TrendLay::poly (const PointVector& pdata, const CTM& rmm)
{
   _cslice->registerPoly(DEBUG_NEW TrxTNcvx(pdata, rmm), NULL);
}

void trend::TrendLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss)
{
   TrxSNcvx* sobj = DEBUG_NEW TrxSNcvx(pdata, psize, ss);
   registerSPoly(sobj);
   _cslice->registerPoly(sobj, tpoly);
}

void trend::TrendLay::poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss, const CTM& rmm)
{
   TrxSNcvx* sobj = DEBUG_NEW TrxSMNcvx(pdata, psize, ss, rmm);
   registerSPoly(sobj);
   _cslice->registerPoly(sobj, tpoly);
}

void trend::TrendLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool center_only)
{
   _cslice->registerWire(DEBUG_NEW TrxWire(pdata, psize, width, center_only));
}

void trend::TrendLay::wire (const PointVector& pdata, WireWidth width, bool center_only, const CTM& rmm)
{
   _cslice->registerWire(DEBUG_NEW TrxTWire(pdata, width, center_only, rmm));

}

void trend::TrendLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool center_only, const SGBitSet* ss)
{
   TrxSWire* sobj = DEBUG_NEW TrxSWire(pdata, psize, width, center_only, ss);
   registerSWire(sobj);
   _cslice->registerWire(sobj);
}

void trend::TrendLay::wire (int4b* pdata, unsigned psize, WireWidth width, bool center_only, const SGBitSet* ss, const CTM& rmm)
{
   TrxSWire* sobj = DEBUG_NEW TrxSMWire(pdata, psize, width, center_only, ss, rmm);
   registerSWire(sobj);
   _cslice->registerWire(sobj);
}

void trend::TrendLay::text (const std::string* txt, const CTM& ftmtrx, const DBbox* ovl, const TP& cor, bool sel)
{
   // Make sure that selected shapes don't come unexpected
   TrxTextOvlBox* cobj = NULL;
   if (sel)
   {
      assert(ovl);
      TrxTextSOvlBox* sobj = DEBUG_NEW TrxTextSOvlBox((*ovl) , ftmtrx);
      registerSOBox(sobj);
      cobj = sobj;
   }
   else if (ovl)
   {
      cobj = DEBUG_NEW TrxTextOvlBox((*ovl) , ftmtrx);
   }

   CTM ftm(ftmtrx.a(), ftmtrx.b(), ftmtrx.c(), ftmtrx.d(), 0, 0);
   ftm.Translate(cor * ftmtrx);
   _cslice->registerText(DEBUG_NEW TrxText(txt, ftm), cobj);
}


void trend::TrendLay::registerSBox (TrxSBox* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[lnes] += sobj->ssize();
      _asobjix[lnes]++;
   }
   else
   {
      _asindxs[llps] += sobj->csize();
      _asobjix[llps]++;
   }
}

void trend::TrendLay::registerSOBox (TrxTextSOvlBox* sobj)
{
   _slct_data.push_back(sobj);
   _asindxs[llps] += 4;
   _asobjix[llps]++;
}


void trend::TrendLay::registerSPoly (TrxSNcvx* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[lnes] += sobj->ssize();
      _asobjix[lnes]++;
   }
   else
   {
      _asindxs[llps] += sobj->csize();
      _asobjix[llps]++;
   }
}

void trend::TrendLay::registerSWire (TrxSWire* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[lnes] += sobj->ssize();
      _asobjix[lnes]++;
   }
   else
   {
      _asindxs[lstr] += sobj->lsize();
      _asobjix[lstr]++;
   }
}

unsigned trend::TrendLay::total_slctdx()
{
   return ( _asindxs[lstr] +
            _asindxs[llps] +
            _asindxs[lnes]
          );
}

trend::TrendLay::~TrendLay()
{
   for (TrendTVList::const_iterator TLAY = _layData.begin(); TLAY != _layData.end(); TLAY++)
      delete (*TLAY);
   for (TrendReTVList::const_iterator TLAY = _reLayData.begin(); TLAY != _reLayData.end(); TLAY++)
      delete (*TLAY);
// not required?!
//   for (SliceSelected::const_iterator TLAY = _slct_data.begin(); TLAY != _slct_data.end(); TLAY++)
//      delete (*TLAY);
}


//=============================================================================
//
// class TrendRefLay
//
trend::TrendRefLay::TrendRefLay() :
   _alvrtxs    (      0u),
   _alobjvx    (      0u),
   _asindxs    (      0u),
   _asobjix    (      0u)
   {
}

void trend::TrendRefLay::addCellOBox(TrendRef* cRefBox, word alphaDepth, bool selected)
{
   if (selected)
   {
      _cellSRefBoxes.push_back(cRefBox);
//      assert(2 == alphaDepth); <-- @TODO (Alpha depth dependent rendering). Why this is hit during edit in place?
      _asindxs += 4;
      _asobjix++;
   }
   else
   {
      _cellRefBoxes.push_back(cRefBox);
      if (1 < alphaDepth)
      {
         // The meaning of this condition is to prevent rendering of the overlapping box of the
         // top visible cell. Only the top visible cell has alphaDepth parameter equals to 1
         _alvrtxs += 4;
         _alobjvx++;
      }
   }
}

unsigned trend::TrendRefLay::total_points()
{
   return (_alvrtxs + _asindxs);
}

unsigned trend::TrendRefLay::total_indexes()
{
   return (_alobjvx + _asobjix);
}

trend::TrendRefLay::~TrendRefLay()
{
   for (RefBoxList::const_iterator CSH = _cellRefBoxes.begin(); CSH != _cellRefBoxes.end(); CSH++)
      delete (*CSH);
   for (RefBoxList::const_iterator CSH = _cellSRefBoxes.begin(); CSH != _cellSRefBoxes.end(); CSH++)
      delete (*CSH);
}

//=============================================================================
//
// class TrendBase
//
trend::TrendBase::TrendBase( layprop::DrawProperties* drawprop, real UU ) :
   _drawprop           ( drawprop  ),
   _UU                 (        UU ),
   _clayer             (      NULL ),
   _grcLayer           (      NULL ),
   _refLayer           (      NULL ),
   _cslctd_array_offset(        0u ),
   _activeCS           (      NULL ),
   _dovCorrection      (         0 ),
   _rmm                (      NULL )
{
   // Initialize the cell (CTM) stack
   _cellStack.push(DEBUG_NEW TrendRef());
}

void trend::TrendBase::setRmm(const CTM& mm)
{
   _rmm = DEBUG_NEW CTM(mm.Reversed());
}

void trend::TrendBase::pushCell(std::string cname, const CTM& trans, const DBbox& overlap, bool active, bool selected)
{
   TrendRef* cRefBox = DEBUG_NEW TrendRef(cname,
                                          trans * _cellStack.top()->ctm(),
                                          overlap,
                                          _cellStack.size()
                                         );
   if (selected || (!_drawprop->isCellBoxHidden()))
      _refLayer->addCellOBox(cRefBox, _cellStack.size(), selected);
   else
      // This list is to keep track of the hidden cRefBox - so we can clean
      // them up. Don't get confused - we need cRefBox during the collecting
      // and drawing phase so we can't really delete them here or after they're
      // poped-up from _cellStack. The confusion is coming from the "duality"
      // of the TrendRef - once as a cell reference with CTM, view depth etc.
      // and then as a placeholder of the overlapping reference box
      _hiddenRefBoxes.push_back(cRefBox);

   _cellStack.push(cRefBox);
   if (active)
   {
      assert(NULL == _activeCS);
      _activeCS = cRefBox;
   }
}

void trend::TrendBase::grcpoly(int4b* pdata, unsigned psize)
{
   assert(_grcLayer);
   _grcLayer->poly(pdata, psize, NULL);
}

void trend::TrendBase::wire (int4b* pdata, unsigned psize, WireWidth width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * scrCTM(), visualLimit());
   _clayer->wire(pdata, psize, width, center_line_only);
}

void trend::TrendBase::wiret(const PointVector& pdata, WireWidth width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * scrCTM(), visualLimit());
   _clayer->wire(pdata, width, center_line_only, *_rmm);
}

void trend::TrendBase::wire (int4b* pdata, unsigned psize, WireWidth width, const SGBitSet* psel)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * scrCTM(), visualLimit());
   _clayer->wire(pdata, psize, width, center_line_only,psel);
}

void trend::TrendBase::wirem(int4b* pdata, unsigned psize, WireWidth width, const SGBitSet* psel)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * scrCTM(), visualLimit());
   _clayer->wire(pdata, psize, width, center_line_only,psel,*_rmm);
}

void trend::TrendBase::grcwire (int4b* pdata, unsigned psize, WireWidth width)
{
   // first check whether to draw only the center line
   DBbox wsquare = DBbox(TP(0,0),TP(width,width));
   bool center_line_only = !wsquare.visible(topCTM() * scrCTM(), visualLimit());
   _grcLayer->wire(pdata, psize, width, center_line_only);
}

void trend::TrendBase::arefOBox(std::string cname, const CTM& trans, const DBbox& overlap, bool selected)
{
   if (selected || (!_drawprop->isCellBoxHidden()))
   {
      TrendRef* cRefBox = DEBUG_NEW TrendRef(cname,
                                               trans * _cellStack.top()->ctm(),
                                               overlap,
                                               _cellStack.size()
                                              );
      _refLayer->addCellOBox(cRefBox, _cellStack.size(), selected);
   }
}

void trend::TrendBase::text (const std::string* txt, const CTM& ftmtrx, const DBbox& ovl, const TP& cor, bool sel)
{
   if (sel)
      _clayer->text(txt, ftmtrx, &ovl, cor, true);
   else if (_drawprop->isTextBoxHidden())
      _clayer->text(txt, ftmtrx, NULL, cor, false);
   else
      _clayer->text(txt, ftmtrx, &ovl, cor, false);
}

void trend::TrendBase::textt(const std::string* txt, const CTM& ftmtrx, const TP& cor)
{
   _clayer->text(txt, ftmtrx*(*_rmm), NULL, cor, false);
}

bool trend::TrendBase::preCheckCRS(const laydata::TdtCellRef* ref, layprop::CellRefChainType& crchain)
{
   crchain = _drawprop->preCheckCRS(ref);
   byte dovLimit = _drawprop->cellDepthView();
   if (0 == dovLimit) return true;
   switch (crchain)
   {
      case layprop::crc_VIEW:
         return (_cellStack.size() <= _drawprop->cellDepthView());
      case layprop::crc_POSTACTIVE:
         return ((_cellStack.size() - _dovCorrection) < _drawprop->cellDepthView());
      case layprop::crc_ACTIVE:
         _dovCorrection = _cellStack.size(); return true;
      default: return true;
   }
   return true;// Dummy statement - to prevent compiler warnings
}

void trend::TrendBase::cleanUp()
{
   for (DataLay::Iterator CLAY = _data.begin(); CLAY != _data.end(); CLAY++)
   {
      delete (*CLAY);
   }
   _data.clear();
   assert(1 == _cellStack.size());
   delete (_cellStack.top()); _cellStack.pop();
   for (RefBoxList::const_iterator CSH = _hiddenRefBoxes.begin(); CSH != _hiddenRefBoxes.end(); CSH++)
      delete (*CSH);
   _activeCS = NULL;
}

void trend::TrendBase::grcCleanUp()
{
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {
      delete (*CLAY);
   }
}

trend::TrendBase::~TrendBase()
{
   if (_refLayer) delete _refLayer;
   if (_rmm)      delete _rmm;
}

void trend::checkOGLError(std::string loc)
{
   std::ostringstream ost;
   GLenum ogle;
   while ((ogle=glGetError()) != GL_NO_ERROR)
   {
      ost << "OpenGL Error: \"" << gluErrorString(ogle)
          << "\" during " << loc;
      tell_log(console::MT_ERROR,ost.str());
   }
}
