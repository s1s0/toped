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
//        Created: Thu Jul 24 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL 3D renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef T3DER
#define T3DER

#include "toshader.h"
namespace trend {

   class T3Der : public Toshader {
   public:
                        T3Der( layprop::DrawProperties*, real UU);
      virtual          ~T3Der();
      virtual void      setLayer(const LayerDef&, bool);
      virtual void      setHvrLayer(const LayerDef&);
      virtual void      setGrcLayer(bool, const LayerDef&);
      virtual bool      chunkExists(const LayerDef&, bool);
      virtual void      draw();
      virtual void      grcDraw();
      virtual void      rlrDraw();
      virtual void      grdDraw();
   protected:
      virtual void      setLayColor(const LayerDef& layer);
      virtual void      setStipple();
      virtual void      setLine(bool);
   private:
      void              windowVAO();
   };
}

#endif  //T3DER
