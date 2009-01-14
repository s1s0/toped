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
//        Created: Sun Jan 11 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TENDERER_H
#define TENDERER_H

#include "drawprop.h"

class Tenderer {
   public:
                        Tenderer( layprop::DrawProperties* drawprop, real UU ) : _drawprop(drawprop), _UU(UU) {}
//                     ~Tenderer();
      void              Grid( const real, const std::string );
      void              add_data(const laydata::atticList*, const SLMap*);
//       void              add_quad(pointlist*);
//       void              add_poly(pointlist*);
//       void              add_wire(pointlist*);
//       void              add_lines(pointlist*);
//       void              add_cell_box(pointlist*);

      // temporary!
      void                       initCTMstack()                {        _drawprop->initCTMstack()        ;}
      void                       clearCTMstack()               {        _drawprop->clearCTMstack()       ;}
      void                       setCurrentColor(word layno)   {        _drawprop->setCurrentColor(layno);}
      bool                       layerHidden(word layno) const {return  _drawprop->layerHidden(layno)    ;}
      const CTM&                 ScrCTM() const                {return  _drawprop->ScrCTM()              ;}
      const CTM&                 topCTM() const                {return  _drawprop->topCTM()              ;}
      const DBbox&               clipRegion() const            {return  _drawprop->clipRegion()          ;}
      void                       pushCTM(CTM& last)            {        _drawprop->pushCTM(last)         ;}
   private:
      layprop::DrawProperties*   _drawprop;
      real                       _UU;
};


#endif //TENDERER_H