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
         virtual          ~TolderTV() {}
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
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
         virtual void      zeroCross();
         virtual void      setLayer(const LayerDef&, bool);
         virtual void      setHvrLayer(const LayerDef&);
         virtual void      setGrcLayer(bool, const LayerDef&);
         virtual bool      chunkExists(const LayerDef&, bool);
         virtual bool      collect();
         virtual bool      grcCollect();
         virtual bool      grdCollect(const layprop::LayoutGrid**);
         virtual bool      rlrCollect(const layprop::RulerList&, int4b);
         virtual void      draw();
         virtual void      grcDraw();
         virtual void      cleanUp();
         virtual void      grcCleanUp();
         virtual void      rlrDraw();
      protected:
         virtual void      setLayColor(const LayerDef& layer);
         virtual void      setStipple();
         virtual void      setLine(bool);
         DBlineList        _noniList;        //!All ruler lines including Vernier ticks.
   };

}

#endif //TOLDER_H
