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
//   This file is a part of Toped project (C) 2001-2007 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL: https://toped.googlecode.com/svn/trunk/tpd_ifaces/drc_tenderer.h $
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Interlayer between CalbrFile and Toped database
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 1204 $
//          $Date: 2009-08-26 23:20:02 +0800 (��, 26 ��� 2009) $
//        $Author: gaitukevich.s $
//===========================================================================
//      Comments :
//===========================================================================

#if !defined(DRC_TENDERER_H_INCLUDED)
#define DRC_TENDERER_H_INCLUDED

#include "calbr_reader.h"

namespace Calbr
{
class drcTenderer: public drcRenderer
{
   public:
   drcTenderer();
   ~drcTenderer();
   void drawBegin();
   void drawPoly(const CoordsVector   &coords);
   void drawLine(const edge &edge);
   void drawEnd();
private:
   laydata::tdtdesign*  _ATDB;
   unsigned             _drcLayer;
   double               _maxx;
   double               _maxy;
   double               _minx;
   double               _miny;
   bool                 _startDrawing; //use for initial setting of _minx, maxy etc
};

}

#endif //DRC_TENDERER_H_INCLUDED