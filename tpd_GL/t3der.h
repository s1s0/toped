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

   class T3Der : public TrendBase {
   public:
                        T3Der( layprop::DrawProperties* drawprop, real UU);
      virtual          ~T3Der();
      virtual void      pushCell(std::string, const CTM&, const DBbox&, bool, bool)          {assert(false);}
      virtual void      setLayer(const LayerDef&, bool);
      virtual void      setHvrLayer(const LayerDef&)                                         {assert(false);}
      virtual void      setGrcLayer(bool, const LayerDef&)                                   {assert(false);}
      virtual bool      chunkExists(const LayerDef&, bool);

      virtual bool      collect()                                                            {assert(false);}
      virtual bool      grcCollect()                                                         {assert(false);}
      virtual bool      grdCollect(const layprop::LayoutGrid**)                              {assert(false);}
      virtual bool      rlrCollect(const layprop::RulerList&, int4b, const DBlineList&)      {assert(false);}

      virtual void      draw()                                                               {assert(false);}
      virtual void      grcDraw()                                                            {assert(false);}
      virtual void      rlrDraw()                                                            {assert(false);}
      virtual void      grdDraw()                                                            {assert(false);}
      virtual void      arefOBox(std::string, const CTM&, const DBbox&, bool)                {assert(false);}
      virtual void      text (const std::string*, const CTM&, const DBbox&, const TP&, bool) {assert(false);}

   protected:
      unsigned          _cslctd_array_offset; //! Current selected array offset

      virtual void      setLayColor(const LayerDef& layer)                                   {assert(false);}
      virtual void      setStipple()                                                         {assert(false);}
      virtual void      setLine(bool)                                                        {assert(false);}
      virtual void      cleanUp()                                                            {assert(false);}
      virtual void      grdCleanUp()                                                         {assert(false);}
      virtual void      rlrCleanUp()                                                         {assert(false);}

   private:
      void              windowVAO()                                                          {assert(false);}
   };
}

#endif  //T3DER
