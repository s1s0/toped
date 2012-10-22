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
//        Created: Sun Oct 14 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL shader renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TOSHADER_H
#define TOSHADER_H

#include <GL/glew.h>
#include "tenderer.h"

namespace trend {
   class ToshaderTV : public TenderTV {
      public:
                           ToshaderTV(TrendRef* const, bool, bool, unsigned, unsigned);
         virtual          ~ToshaderTV() {};
         virtual void      draw(layprop::DrawProperties*);
         void              setCtm(layprop::DrawProperties*);
         //virtual void      drawTexts(layprop::DrawProperties*);
   };

   class ToshaderReTV : public TenderReTV {
      public:
                           ToshaderReTV(TrendTV* const chunk, TrendRef* const refCell):
                              TenderReTV(chunk, refCell) {}
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
   };

   class ToshaderLay : public TenderLay {
      public:
                           ToshaderLay();
         virtual          ~ToshaderLay();
         virtual void      newSlice(TrendRef* const, bool, bool /*, bool, unsigned*/);
         virtual void      newSlice(TrendRef* const, bool, bool, unsigned slctd_array_offset);
         virtual bool      chunkExists(TrendRef* const, bool);
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawSelected();
         virtual void      drawTexts(layprop::DrawProperties*);
   };

   class ToshaderRefLay : public TenderRefLay {
      public:
                           ToshaderRefLay();
         virtual          ~ToshaderRefLay();
         virtual void      draw(layprop::DrawProperties*);
   };

   class Toshader : public Tenderer {
      public:
                           Toshader( layprop::DrawProperties*, real UU);
         virtual          ~Toshader();
         virtual void      grid( const real, const std::string );
         virtual void      zeroCross();
         virtual void      setLayer(const LayerDef&, bool);
         virtual void      setHvrLayer(const LayerDef&);
         virtual void      setGrcLayer(bool, const LayerDef&);
         virtual bool      chunkExists(const LayerDef&, bool);
         virtual void      draw();
         virtual void      grcDraw();
         //virtual void      cleanUp();
         //virtual void      grcCleanUp();
      protected:
         virtual void      setColor(const LayerDef& layer);
         virtual void      setStipple();
   };

}

#endif // TOSHADER_H