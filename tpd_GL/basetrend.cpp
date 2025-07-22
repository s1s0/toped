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

trend::ogl_logfile     OGLLogFile;


//=============================================================================
//
// class TrendTV
//
trend::TrendTV::TrendTV(TrxCellRef* const refCell, bool filled, bool reusable,
                        unsigned /*parray_offset*/, unsigned /*iarray_offset*/) :
   _refCell             ( refCell         ),
   _num_total_strings   ( 0u              ),
   _filled              ( filled          ),
   _reusable            ( reusable        )
{
   for (int i = ITtria; i < IDX_TYPES; i++)
   {
      _iobjnum[i] = 0u;
      _indxnum[i] = 0u;
   }
   for (int i = OTcntr; i < OBJ_TYPES; i++)
   {
      _vobjnum[i] = 0u;
      _vrtxnum[i] = 0u;
   }
}

void trend::TrendTV::registerBox (TrxCnvx* cobj)
{
   unsigned allpoints = cobj->csize();
   if (_filled)
   {
      _cnvx_data.push_back(cobj);
      _vrtxnum[OTcnvx] += allpoints;
      _vobjnum[OTcnvx]++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _vrtxnum[OTcntr] += allpoints;
      _vobjnum[OTcntr]++;
   }
}

void trend::TrendTV::register3DBox(Trx3DBox* cobj)
{
   // a generic tesselation object for all boxes
   TessellPoly* tdata = DEBUG_NEW TessellPoly();
   tdata->tessellate3DBox();
   cobj->setTeselData(tdata);

   _ncvx_data.push_back(cobj);
   _vrtxnum[OTncvx] += 2 * cobj->csize();
   _iobjnum[ITtria] += tdata->num_tria();
   _iobjnum[ITtstr] += tdata->num_tstr();
   tdata->num_indexs(_indxnum[ITtria], _indxnum[ITtstr]);
   _vobjnum[OTncvx]++;
}

void trend::TrendTV::registerPoly (TrxNcvx* cobj, const TessellPoly* tchain)
{
   unsigned allpoints = cobj->csize();
   if (_filled && tchain && tchain->valid())
   {
      cobj->setTeselData(tchain);
      _ncvx_data.push_back(cobj);
      _vrtxnum[OTncvx] += allpoints;
      _iobjnum[ITtria] += tchain->num_tria();
      _iobjnum[ITtstr] += tchain->num_tstr();
      tchain->num_indexs(_indxnum[ITtria], _indxnum[ITtstr]);
      _vobjnum[OTncvx]++;
   }
   else
   {
      _cont_data.push_back(cobj);
      _vrtxnum[OTcntr] += allpoints;
      _vobjnum[OTcntr]++;
   }
}

void trend::TrendTV::register3DPoly(Trx3DPoly* cobj, const TessellPoly* tchain)
{
   TessellPoly* tdata = DEBUG_NEW TessellPoly(tchain);
   tdata->tessellate3DPoly(cobj->csize());
   cobj->setTeselData(tdata);

   _ncvx_data.push_back(cobj);
   _vrtxnum[OTncvx] += 2 * cobj->csize();
   _iobjnum[ITtria] += tdata->num_tria();
//   _alobjix[ftfs] += tdata->num_ftfs();
   _iobjnum[ITtstr] += tdata->num_tstr();
   tdata->num_indexs(_indxnum[ITtria], /*_alindxs[ftfs],*/ _indxnum[ITtstr]);
   _vobjnum[OTncvx]++;
}

void trend::TrendTV::registerWire (TrxWire* cobj)
{
   unsigned allpoints = cobj->csize();
   _line_data.push_back(cobj);
   _vrtxnum[OTline] += cobj->lsize();
   _vobjnum[OTline]++;
   if ( !cobj->center_line_only() )
   {
      if (_filled)
      {
         cobj->Tesselate();
         _ncvx_data.push_back(cobj);
         _vrtxnum[OTncvx] += allpoints;
         _iobjnum[ITtria] += cobj->tpdata()->num_tria();
         _iobjnum[ITtstr] += cobj->tpdata()->num_tstr();
         cobj->tpdata()->num_indexs(_indxnum[ITtria], _indxnum[ITtstr]);
         _vobjnum[OTncvx]++;
      }
      else
      {
         _cont_data.push_back(cobj);
         _vobjnum[OTcntr] ++;
         _vrtxnum[OTcntr] += allpoints;
      }
   }
}

void trend::TrendTV::register3DWire(Trx3DWire* cobj)
{
   cobj->Tesselate();
//   unsigned allpoints = cobj->csize();
   _ncvx_data.push_back(cobj);
   
   _vrtxnum[OTncvx] += 2 * cobj->csize();
   _iobjnum[ITtria] += cobj->tpdata()->num_tria();
   _iobjnum[ITtstr] += cobj->tpdata()->num_tstr();
   cobj->tpdata()->num_indexs(_indxnum[ITtria], _indxnum[ITtstr]);
   _vobjnum[OTncvx]++;
}


void trend::TrendTV::registerText (TrxText* cobj, TrxTextOvlBox* oobj)
{
   _text_data.push_back(cobj);
   _num_total_strings++;
   if (NULL != oobj)
   {
      _txto_data.push_back(oobj);
      _vrtxnum[OTcntr] += 4;
      _vobjnum[OTcntr]++;
   }
}

unsigned trend::TrendTV::num_total_points()
{
   return ( _vrtxnum[OTcntr] +
            _vrtxnum[OTline] +
            _vrtxnum[OTcnvx] +
            _vrtxnum[OTncvx]
          );
}

unsigned trend::TrendTV::num_total_indexs()
{
   return ( _indxnum[ITtria] +
            _indxnum[ITtstr]
          );
}

trend::TrxCellRef* trend::TrendTV::swapRefCells(TrxCellRef* newRefCell)
{
   TrxCellRef* the_swap = _refCell;
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

void trend::TrendTV::DEBUGprintOGLdata(const unsigned start, GLuint **_firstix, GLsizei **_sizesix, unsigned int *index_array, TPVX &point_array, unsigned int *size_index)
{
   unsigned i = start;
   for (auto boza : point_array)
   {
      printf("%3i ->X: %f; Y: %f\n", i++, boza.x, boza.y);
   }
   
   for (i = 0; i < size_index[ITtria]; i++)
   {
      unsigned findex = _firstix[ITtria][i]/sizeof(unsigned);
      printf("Triangle  index %d -> Offset: %d ; Size: %d\n", i, findex, _sizesix[ITtria][i]);
      printf("       Indexes:");
      for (GLsizei j = 0; j < _sizesix[ITtria][i]; j++)
         printf(" %d", index_array[findex+j]);
      printf("\n");
   }
   
   for (i = 0; i < size_index[ITtstr]; i++)
   {
      unsigned findex = _firstix[ITtstr][i]/sizeof(unsigned);
      printf("TriStrips index %d -> Offset: %d ; Size: %d\n", i, findex, _sizesix[ITtstr][i]);
      printf("       Indexes:");
      for (GLsizei j = 0; j < _sizesix[ITtstr][i]; j++)
         printf(" %d", index_array[findex+j]);
      printf("\n");
   }
}

//=============================================================================
//
// class TrendLay
//
trend::TrendLay::TrendLay(bool rend3D):
   _cslice               (        NULL ),
   _num_total_points     (          0u ),
   _num_total_indexs     (          0u ),
   _num_total_slctdx     (          0u ),
   _num_total_strings    (          0u ),
   _rend3D               ( rend3D      )
{
   for (int i = STlstr; i < SLCT_TYPES; i++)
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
   if (_rend3D)
      _cslice->register3DBox(DEBUG_NEW Trx3DBox(pdata));
   else
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
   if (_rend3D)
      _cslice->register3DPoly(DEBUG_NEW Trx3DPoly(pdata, psize), tpoly);
   else
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
   if (_rend3D)
      _cslice->register3DWire(DEBUG_NEW Trx3DWire(pdata, psize, width));
   else
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
      _asindxs[STlnes] += sobj->ssize();
      _asobjix[STlnes]++;
   }
   else
   {
      _asindxs[STllps] += sobj->csize();
      _asobjix[STllps]++;
   }
}

void trend::TrendLay::registerSOBox (TrxTextSOvlBox* sobj)
{
   _slct_data.push_back(sobj);
   _asindxs[STllps] += 4;
   _asobjix[STllps]++;
}


void trend::TrendLay::registerSPoly (TrxSNcvx* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[STlnes] += sobj->ssize();
      _asobjix[STlnes]++;
   }
   else
   {
      _asindxs[STllps] += sobj->csize();
      _asobjix[STllps]++;
   }
}

void trend::TrendLay::registerSWire (TrxSWire* sobj)
{
   _slct_data.push_back(sobj);
   if ( sobj->partSelected() )
   {
      _asindxs[STlnes] += sobj->ssize();
      _asobjix[STlnes]++;
   }
   else
   {
      _asindxs[STlstr] += sobj->lsize();
      _asobjix[STlstr]++;
   }
}

unsigned trend::TrendLay::total_slctdx()
{
   return ( _asindxs[STlstr] +
            _asindxs[STllps] +
            _asindxs[STlnes]
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

void trend::TrendRefLay::addCellOBox(TrxCellRef* cRefBox, word alphaDepth, bool selected)
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
// class TrendMarks
//

unsigned trend::TrendMarks::total_points()
{
   return static_cast<unsigned>(( _refMarks.size()
                                + _textMarks.size()
                                + _arefMarks.size()
                               ));
}
//=============================================================================
//
// class TrendBase
//
trend::TrendBase::TrendBase( layprop::DrawProperties* drawprop, real UU ) :
   _drawprop             ( drawprop  ),
   _UU                   (        UU ),
   _clayer               (      NULL ),
   _grcLayer             (      NULL ),
   _refLayer             (      NULL ),
   _cslctd_array_offset  (        0u ),
   _activeCS             (      NULL ),
   _dovCorrection        (         0 ),
   _marks                (      NULL ),
   _rmm                  (      NULL ),
   _num_grid_points      (        0u ),
   _rend3D               (      false)

{
   // Initialize the cell (CTM) stack
   _cellStack.push(DEBUG_NEW TrxCellRef());

}

void trend::TrendBase::setRmm(const CTM& mm)
{
   _rmm = DEBUG_NEW CTM(mm.Reversed());
}

void trend::TrendBase::pushCell(std::string cname, const CTM& trans, const DBbox& overlap, bool active, bool selected)
{
   TrxCellRef* cRefBox = DEBUG_NEW TrxCellRef(cname,
                                          trans * _cellStack.top()->ctm(),
                                          overlap,
                                          _cellStack.size()
                                         );
   if (selected || (!_drawprop->cellBoxHidden()) || !_rend3D)
      _refLayer->addCellOBox(cRefBox, _cellStack.size(), selected);
   else
      // This list is to keep track of the hidden cRefBox - so we can clean
      // them up. Don't get confused - we need cRefBox during the collecting
      // and drawing phase so we can't really delete them here or after they're
      // poped-up from _cellStack. The confusion is coming from the "duality"
      // of the TrxCellRef - once as a cell reference with CTM, view depth etc.
      // and then as a placeholder of the overlapping reference box
      _hiddenRefBoxes.push_back(cRefBox);

   _cellStack.push(cRefBox);
   if (active)
   {
      assert(NULL == _activeCS);
      _activeCS = cRefBox;
   }
   else if (!(_drawprop->cellMarksHidden() && _rend3D))
   {
      _marks->addRefMark(overlap.p1(), _cellStack.top()->ctm());
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
   if (!_drawprop->cellMarksHidden())
   {
      _marks->addARefMark(overlap.p1(), trans * _cellStack.top()->ctm());
   }

   if (selected || (!_drawprop->cellBoxHidden()))
   {
      TrxCellRef* cRefBox = DEBUG_NEW TrxCellRef(cname,
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
   else if (_drawprop->textBoxHidden())
      _clayer->text(txt, ftmtrx, NULL, cor, false);
   else
      _clayer->text(txt, ftmtrx, &ovl, cor, false);
   if (!_drawprop->textMarksHidden())
   {
      _marks->addTextMark(ovl.p1(),ftmtrx*_cellStack.top()->ctm());
   }
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
//   if (1 == _cellStack.size())
//   {
//      delete (_cellStack.top()); _cellStack.pop();
//   }
//   else
//   {
//      static int bozaInt;
//      std::stringstream boza;
//      boza << "Empty Stack " << bozaInt++ ;
//      tell_log(console::MT_WARNING, boza.str());
//   }
   assert(1 == _cellStack.size());
   delete (_cellStack.top()); _cellStack.pop();
   for (RefBoxList::const_iterator CSH = _hiddenRefBoxes.begin(); CSH != _hiddenRefBoxes.end(); CSH++)
      delete (*CSH);
   _hiddenRefBoxes.clear();
   _activeCS = NULL;
}

void trend::TrendBase::grcCleanUp()
{
   for (DataLay::Iterator CLAY = _grcData.begin(); CLAY != _grcData.end(); CLAY++)
   {
      delete (*CLAY);
   }
   _grcData.clear();
}

void trend::TrendBase::grdCleanUp()
{
   for (VGrids::const_iterator CG = _grid_props.begin(); CG != _grid_props.end(); CG++)
   {
      delete (*CG);
   }
   _grid_props.clear();
}

void trend::TrendBase::rlrCleanUp()
{
   for (TrendStrings::const_iterator TS = _rulerTexts.begin(); TS != _rulerTexts.end(); TS++)
   {
      delete (*TS);
   }
   _rulerTexts.clear();
}

void trend::TrendBase::genRulerMarks(const CTM& LayCTM, DBline& long_mark, DBline& short_mark, DBline& text_bp, double& scaledpix)
{
   // Side ticks (segments) of the rulers has to be with constant size. The next
   // lines are generating a segment with the size 7/3 screen pixels centred in
   // the {0,0} point of the canvas (logical coordinates)
   // The coefficients 1e3/1e-3 are picked arbitrary in an attempt to reduce the
   // error
   const double ico = 1e3;
   const double dco = 1/ico;
   DBline tick_sample = DBline(TP(0,0),TP(0,7,ico)) * LayCTM;
   double tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   long_mark = DBline(TP(0,-tick_size, dco),TP(0,tick_size, dco));

   tick_sample = DBline(TP(0,0),TP(0,3,ico)) * LayCTM;
   tick_size = ((double)(tick_sample.p2().y()-tick_sample.p1().y()));
   short_mark = DBline(TP(0,-tick_size, dco),TP(0,tick_size, dco));

   tick_sample = DBline(TP(0,0),TP(0,20,ico)) * LayCTM;
   tick_size = ((double)(tick_sample.p1().y()-tick_sample.p2().y()));
   text_bp = DBline(TP(0,0),TP(0,tick_size, dco));

   // now prepare to draw the size
   DBbox pixelbox = DBbox(TP(),TP(15,15)) * LayCTM;
   scaledpix = ((double)(pixelbox.p2().x()-pixelbox.p1().x()));
}

trend::TrendBase::~TrendBase()
{
   if (_refLayer) delete _refLayer;
   if (_marks)    delete _marks;
   if (_rmm)      delete _rmm;

}

trend::TrendGridC::TrendGridC(TP bl, TP tr, int step, std::string color) :
   _bl    ( bl     ),
   _tr    ( tr     ),
   _step  ( step   ),
   _color ( color  ),
   _X     (      0 ),
   _Y     (      0 ),
   _asize (      0 )
{
   calculate();
}

void trend::TrendGridC::calculate()
{
   int signX = (_bl.x() > 0) ? 1 : -1;
       _X    = (int)((rint(abs(_bl.x()) / _step)) * _step * signX);
   int signY = (_tr.y() > 0) ? 1 : -1;
       _Y = (int)((rint(abs(_tr.y()) / _step)) * _step * signY);

   _asize = ( (floor((_tr.x() - _X + 1) / _step)+1) * (floor((_bl.y() - _Y + 1) / _step)+1) );
}

unsigned trend::TrendGridC::dump(TNDR_GLDATAT* parray, unsigned index)
{
   for (int i = _X; i < _tr.x()+1; i += _step)
   {
      for (int j = _Y; j < _bl.y()+1; j += _step)
      {
         parray[index++] = i;
         parray[index++] = j;
      }
   }
   return index/2;
}

void trend::ogl_logfile::init(const std::string logFN)
{
   /*char *locale=*/setlocale(LC_ALL, "");
   _file.open(logFN.c_str(), std::ios::out);
   TpdTime timec(time(NULL));
   _file << "//   Session started: " << timec() << std::endl;
   setlocale(LC_ALL, "English");
   _enabled = true;
}

trend::ogl_logfile& trend::ogl_logfile::flush()
{
   if (_enabled) _file << std::endl;
   return *this;
}

void trend::ogl_logfile::close()
{
   if (_enabled) _file.close();

}

trend::ogl_logfile& trend::ogl_logfile::operator<< (const std::string& info)
{
   if (_enabled) _file << info ;
   return *this;
}

trend::ogl_logfile& trend::ogl_logfile::operator<< (const unsigned int _i)
{
   if (_enabled) _file << _i ;
   return *this;
}


void trend::checkOGLError(std::string loc)
{
   std::ostringstream ost;
   GLenum ogle;
   while (GL_NO_ERROR != (ogle=glGetError()))
   {
      ost << "OpenGL Error: \"" ;
      std::string errString;
      switch (ogle) {
         case GL_INVALID_ENUM     : ost << "Unacceptable value for an enumerated argument";     break;
         case GL_INVALID_VALUE    : ost << "Numeric argument is out of range";                  break;
         case GL_INVALID_OPERATION: ost << "The operation is not allowed in the current state"; break;
         case GL_STACK_OVERFLOW   : ost << "Stack overflow";                                    break;
         case GL_STACK_UNDERFLOW  : ost << "Stack underflow";                                   break;
         case GL_OUT_OF_MEMORY    : ost << "Out of Memory";                                     break;
         case GL_TABLE_TOO_LARGE  : ost << "Tablesize too big";                                 break;
         default                  : ost << "Unknown error value reported:" << ogle;             break;
      }
      ost << "\" during " << loc;
      tell_log(console::MT_ERROR,ost.str());
   }
}

void trend::reportOGLStatus(std::string loc)
{
//   std::ostringstream ost;
   GLenum ogle;
   if (GL_NO_ERROR != (ogle=glGetError()))
   {
      OGLLogFile << loc;
      switch (ogle) {
         case GL_INVALID_ENUM     : OGLLogFile << "Unacceptable value for an enumerated argument";     break;
         case GL_INVALID_VALUE    : OGLLogFile << "Numeric argument is out of range";                  break;
         case GL_INVALID_OPERATION: OGLLogFile << "The operation is not allowed in the current state"; break;
         case GL_STACK_OVERFLOW   : OGLLogFile << "Stack overflow";                                    break;
         case GL_STACK_UNDERFLOW  : OGLLogFile << "Stack underflow";                                   break;
         case GL_OUT_OF_MEMORY    : OGLLogFile << "Out of Memory";                                     break;
         case GL_TABLE_TOO_LARGE  : OGLLogFile << "Tablesize too big";                                 break;
         default                  : OGLLogFile << "Unknown error value reported:" << ogle;             break;
      }
      OGLLogFile.flush();
   }
//   else {
//      OGLLogFile << loc << " OK";
//      OGLLogFile.flush();
//   }
}

//void trend::dumpOGLArrayFloat(const GLfloat* cindex_array, unsigned size)
//{
//   OGLLogFile << "DATA:";
//   OGLLogFile.flush();
//   for(unsigned i = 0; i < 2*size; i += 2)
//   {
//      char buffer[100];
//      std::sprintf(buffer, "(%f,%f) ", cindex_array[i], cindex_array[i+1]);
//      OGLLogFile << buffer;
//   }
//   OGLLogFile.flush();
//}
//
//void trend::dumpOGLArrayUint(const unsigned* cindex_array, unsigned size)
//{
//   OGLLogFile << "INDEX:";
//   OGLLogFile.flush();
//   for(unsigned i = 0; i < size; i++)
//   {
//      char buffer[100];
//      std::sprintf(buffer, "%d ", cindex_array[i]);
//      OGLLogFile << buffer;
//   }
//   OGLLogFile.flush();
//}

TFPTR_DrawElementsOffset tpd_glDrawElements = (TFPTR_DrawElementsOffset)glDrawElements;

