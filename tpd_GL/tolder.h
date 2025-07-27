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
#ifndef TOLDER_H
#define TOLDER_H

#include <GL/glew.h>
#include "basetrend.h"

namespace trend {

   class TolderTV : public TrendTV {
      public:
                           TolderTV(TrxCellRef* const refCell, bool filled, bool reusable,
                                          unsigned parray_offset, unsigned iarray_offset) :
                                 TrendTV(refCell, filled, reusable, parray_offset, iarray_offset) {}
         virtual          ~TolderTV();
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
         virtual void      registerBox   (TrxCnvx*);
         virtual void      registerPoly  (TrxNcvx*, const TessellPoly*);
         virtual void      registerWire  (TrxWire*);
         virtual void      registerText  (TrxText*, TrxTextOvlBox*);
         virtual void      collect(TPVX&, unsigned int*)                {assert(false);}
      protected:
      // collected data lists
         SliceObjects      _cont_data; //! Contour data
         SliceWires        _line_data; //! Line data

         TrendStrings      _text_data; //! Text (strings)
         RefTxtList        _txto_data; //! Text overlapping boxes
   };

   class TolderReTV : public TrendReTV {
      public:
                           TolderReTV(TrendTV* const chunk, TrxCellRef* const refCell):
                              TrendReTV(chunk, refCell) {}
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
   };

   class TolderLay : public TrendLay {
      public:
                           TolderLay() : TrendLay() {}
         virtual          ~TolderLay() {}
         virtual void      newSlice(TrxCellRef* const, bool, bool /*, bool, unsigned*/);
         virtual void      newSlice(TrxCellRef* const, bool, bool, unsigned slctd_array_offset);
         virtual bool      chunkExists(TrxCellRef* const, bool);
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawSelected();
         virtual void      drawTexts(layprop::DrawProperties*);
   };

   class TolderRefLay : public TrendRefLay {
      public:
                           TolderRefLay() : TrendRefLay() {}
         virtual          ~TolderRefLay() {}
         virtual void      draw(layprop::DrawProperties*);
      protected:
         virtual void      setLine(layprop::DrawProperties*, bool);
   };

   class TolderMarks : public TrendMarks {
      public:
                           TolderMarks() : TrendMarks() {}
         virtual void      draw(layprop::DrawProperties*);
   };

   class Tolder : public TrendBase {
      public:
                           Tolder( layprop::DrawProperties* drawprop, real UU );
         virtual          ~Tolder();
         virtual void      grdDraw();
         virtual void      setLayer(const LayerDef&, bool);
         virtual void      setHvrLayer(const LayerDef&);
         virtual void      setGrcLayer(bool, const LayerDef&);
         virtual bool      chunkExists(const LayerDef&, bool);
         virtual bool      collect();
         virtual bool      grcCollect();
         virtual bool      grdCollect(const layprop::LayoutGrid**);
         virtual bool      rlrCollect(const layprop::RulerList&, int4b,const DBlineList&);
         virtual void      draw();
         virtual void      grcDraw();
         virtual void      rlrDraw();

         virtual void      pushCell(std::string, const CTM&, const DBbox&, bool, bool);
         virtual void      arefOBox(std::string, const CTM&, const DBbox&, bool);
         virtual void      text (const std::string*, const CTM&, const DBbox&, const TP&, bool);
   
   protected:
         virtual void      cleanUp();
         virtual void      grcCleanUp();
         virtual void      grdCleanUp();
         virtual void      rlrCleanUp();
         virtual void      setLayColor(const LayerDef& layer);
         virtual void      setStipple();
         virtual void      setLine(bool);
         DBlineList        _noniList;        //!All ruler lines including Vernier ticks.
         TrendRefLay*      _refLayer;        //!All cell references with visible overlapping boxes
         unsigned          _cslctd_array_offset; //! Current selected array offset
       //
         RefBoxList        _hiddenRefBoxes;  //!Those cRefBox objects which didn't ended in the TrendRefLay structures
         TrendMarks*       _marks;           //!All kinds of object marks
         VGrids            _grid_props;      //! The properties of all visual grids
         unsigned          _num_grid_points; //! Number of all points in all grids
         TrendStrings      _rulerTexts;      //!The labels on all rulers
   };

}

#endif //TOLDER_H
