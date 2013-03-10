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
//    Description: Canvas drawing properties
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <string>
#include "drawprop.h"
#include "tuidefs.h"

const layprop::tellRGB        layprop::DrawProperties::_dfltColor(127,127,127,127);
const layprop::LineSettings   layprop::DrawProperties::_dfltLine    ("", 0xffff, 1, 1);
const layprop::LineSettings   layprop::DrawProperties::_dfltSLine   ("", 0xffff, 1, 3);
const layprop::LineSettings   layprop::DrawProperties::_dfltCellBnd ("", 0xf18f, 1, 1);
const layprop::LineSettings   layprop::DrawProperties::_dfltCellSBnd("", 0xf18f, 1, 3);
const layprop::LineSettings   layprop::DrawProperties::_dfltTextBnd ("", 0x3030, 1, 1);
const layprop::LineSettings   layprop::DrawProperties::_dfltTextSBnd("", 0x3030, 1, 3);
const byte                    layprop::DrawProperties::_dfltFill [128] = {
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00,
   0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00
};

const layprop::LayMark        layprop::DrawProperties::_ref_mark_bmp = {
      0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10,
      0x10, 0x08, 0x26, 0x64, 0x47, 0xe2, 0x83, 0xc1,
      0x83, 0xc1, 0x47, 0xe2, 0x26, 0x64, 0x10, 0x08,
      0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80
};


const layprop::LayMark        layprop::DrawProperties::_text_mark_bmp = {
      0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10,
      0x10, 0x08, 0x2f, 0xf4, 0x4d, 0xb2, 0x81, 0x81,
      0x81, 0x81, 0x41, 0x82, 0x21, 0x84, 0x11, 0x88,
      0x09, 0x90, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80
};

const layprop::LayMark        layprop::DrawProperties::_aref_mark_bmp = {
      0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10,
      0x11, 0x88, 0x23, 0xc4, 0x46, 0x62, 0x8c, 0x31,
      0x9f, 0xf9, 0x70, 0x0e, 0x20, 0x04, 0x10, 0x08,
      0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80
};


layprop::SDLine::SDLine(const TP& p1,const TP& p2, const real UU) : _ln(p1,p2)
{
   real _A = _ln.p2().y() - _ln.p1().y();
   real _B = _ln.p1().x() - _ln.p2().x();
//   real _C = -(_A*_ln.p1().x() + _B*_ln.p1().y());
   _length = sqrt(_A*_A + _B*_B);
   std::ostringstream strdist;
   strdist << _length * UU;
   _value = strdist.str();
   _center = TP((_ln.p1().x() + _ln.p2().x()) / 2, (_ln.p1().y() + _ln.p2().y()) / 2 );
   // get the angle coefficient of the ruler and calculate the corresponding
   // functions - will be used during the drawing
   real angle_rad = atan2(_A , -_B);
   _sinus     = sin(angle_rad);
   _cosinus   = cos(angle_rad);
   real w_angle     = angle_rad * 180.0 / M_PI;
   // normalized_angle
   _angle = ((w_angle >= 90) || (w_angle < -90)) ? 180 + w_angle : w_angle;

};

//void layprop::SDLine::draw(const DBline& long_mark, const DBline& short_mark, const DBline& text_bp, const double scaledpix, const real _step) const
//{
//   // calculate the nonius ticks
//   DBlineList noni_list;
//   nonius(short_mark, long_mark, _step, noni_list);
//
//   glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, (GLfloat)0.7); // gray
//   glDisable(GL_POLYGON_STIPPLE);
//   glBegin(GL_LINES);
//   // draw the nonius ...
//   for (DBlineList::const_iterator CL = noni_list.begin(); CL != noni_list.end(); CL++)
//   {
//      glVertex2i(CL->p1().x(),CL->p1().y());
//      glVertex2i(CL->p2().x(),CL->p2().y());
//   }
//   // ... and the ruler itself
//   glVertex2i(_ln.p1().x(), _ln.p1().y());
//   glVertex2i(_ln.p2().x(), _ln.p2().y());
//   glEnd();
//
//   CTM tmtrx;
//   tmtrx.Rotate(_angle);
//   tmtrx.Translate(_center.x(), _center.y());
//   DBline central_elevation = text_bp * tmtrx;
//
//   glPushMatrix();
//   glTranslatef(central_elevation.p2().x(), central_elevation.p2().y(), 0);
//   glScalef(scaledpix, scaledpix, 1);
//   glRotatef(_angle, 0, 0, 1);
//
////   TRENDC->drawSolidString(_value);
//
//   glDisable(GL_POLYGON_SMOOTH); //- for solid fill
//   glEnable(GL_POLYGON_STIPPLE);
//   glPopMatrix();
//
//}

unsigned layprop::SDLine::nonius(const DBline& short_mark, const DBline& long_mark,
                                 const real step, DBlineList& llst) const
{
   // prepare the translation matrix for the edge point
   CTM tmtrx;
   tmtrx.Rotate(_angle);
   tmtrx.Translate(_ln.p1().x(), _ln.p1().y());
   unsigned numtics;
   for( numtics = 0 ; (numtics * step) < _length ; numtics++ )
   {
      // for each tick - get the deltas ...
      int4b deltaX = (int4b) rint(numtics * step * _cosinus);
      int4b deltaY = (int4b) rint(numtics * step * _sinus);
      // ... calculate the translation ...
      CTM pmtrx = tmtrx;
      pmtrx.Translate(deltaX, deltaY);
      // ... create a new tick and move it to its position
      if (numtics % 5)
         llst.push_back(DBline(short_mark * pmtrx));
      else
         llst.push_back(DBline(long_mark * pmtrx));
   }
   // don't forget the opposite edge point
   tmtrx.Translate(_ln.p2().x() - _ln.p1().x(), _ln.p2().y() - _ln.p1().y());
   llst.push_back(DBline(long_mark * tmtrx));
   return ++numtics;
}

//=============================================================================
/*!
 * The main troubles here are with the precision of the calculations. They are
 * getting more obvious when the points are on a long distances from each other.
 * All of the functions used make their calculations in real arithmetics, but
 * some return the result in integer. For angles close to 0 it became apparent
 * that the results doesn't quite match.
 * @param ldata
 * @param lsize
 * @param width
 * @return
 */
laydata::WireContour::WireContour(const int4b* ldata, unsigned lsize, WireWidth width) :
   _ldata(ldata), _lsize(lsize), _width(width)
{
   endPnts(0,1, true);
   for (unsigned i = 1; i < _lsize - 1; i++)
   {
      switch (chkCollinear(i-1, i, i+1))
      {
         case 0: {// points not in one line
            int angle1 = xangle(i  , i-1);
            int angle2 = xangle(i  , i+1);
            int ang = abs(angle1 - angle2);
            if (0 == ang)
               colPnts(i-1,  i, i+1 );
            else if (180 == ang)
               mdlPnts(i-1,  i, i+1 );
            if ((ang < 90) || (ang > 270))
               mdlAcutePnts(i-1,  i, i+1, angle1, angle2); // acute angle
            else
               mdlPnts(i-1,  i, i+1 );
            break;
         }
         case 1: //i-1 and i coincide
            endPnts( i, i+1, true); break;
         case 2: // i and i+1 coincide
            endPnts(i-1,  i,false); break;
         case 3: // collinear points
            colPnts(i-1,  i, i+1 ); break;
         case 4: // 3 points in one line with i2 in the middle
            mdlPnts(i-1,  i, i+1 ); break;
         case 5: // 3 coinciding points
                                    break;
         default: assert(false); break;
      }
   }
   endPnts(_lsize -2, _lsize -1, false);
}

/*!
 * Dumps the generated wire contour in the @plist vector. For optimal performance the
 * vector object shall be properly allocated using something like reserve(csize())
 * before calling this method. The method will cope with @plist vectors which already
 * contain some data. It will just add the contour at the end of the @plist.
 */
void laydata::WireContour::getVectorData(PointVector& plist)
{
   for (PointList::const_iterator CP = _cdata.begin(); CP != _cdata.end(); CP++)
   {
      plist.push_back(*CP);
   }
}

/*!
 * Dumps the generated wire contour in the @contour array. The array must be allocated
 * before calling this function. The size of the array can be taken from the function
 * csize()
 */
void laydata::WireContour::getArrayData(int4b* contour)
{
   word index = 0;
   for (PointList::const_iterator CP = _cdata.begin(); CP != _cdata.end(); CP++)
   {
      contour[index++] = CP->x();
      contour[index++] = CP->y();
   }
}

DBbox laydata::WireContour::getCOverlap()
{
   PointList::const_iterator CP = _cdata.begin();
   DBbox ovl(*CP);
   while (CP != _cdata.end())
   {
      ovl.overlap(*CP); CP++;
   }
   return ovl;
}

void laydata::WireContour::mdlPnts(word i1, word i2, word i3)
{
   double    w = _width/2;
   i1 *= 2; i2 *= 2; i3 *= 2;
   double  x32 = _ldata[i3  ] - _ldata[i2  ];
   double  x21 = _ldata[i2  ] - _ldata[i1  ];
   double  y32 = _ldata[i3+1] - _ldata[i2+1];
   double  y21 = _ldata[i2+1] - _ldata[i1+1];
   double   L1 = sqrt(x21*x21 + y21*y21); //the length of segment 1
   double   L2 = sqrt(x32*x32 + y32*y32); //the length of segment 2
   double denom = x32 * y21 - x21 * y32;
   if ((0 == denom) || (0 == L1) || (0 == L2)) return;
   // the corrections
   double xcorr = w * ((x32 * L1 - x21 * L2) / denom);
   double ycorr = w * ((y21 * L2 - y32 * L1) / denom);
   _cdata.push_front(TP((int4b) rint(_ldata[i2  ] - xcorr),(int4b) rint(_ldata[i2+1] + ycorr)));
   _cdata.push_back (TP((int4b) rint(_ldata[i2  ] + xcorr),(int4b) rint(_ldata[i2+1] - ycorr)));
}

void laydata::WireContour::endPnts(word i1, word i2, bool first)
{
   double     w = _width/2;
   i1 *= 2; i2 *= 2;
   double denom = first ? (_ldata[i2  ] - _ldata[i1  ]) : (_ldata[i1  ] - _ldata[i2  ]);
   double   nom = first ? (_ldata[i2+1] - _ldata[i1+1]) : (_ldata[i1+1] - _ldata[i2+1]);
   double xcorr, ycorr; // the corrections
   if ((0 == nom) && (0 == denom)) return;
   double signX = (  nom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   double signY = (denom > 0) ? (first ? 1.0 : -1.0) : (first ? -1.0 : 1.0);
   if      (0 == denom) // vertical
   {
      xcorr =signX * w ; ycorr = 0        ;
   }
   else if (0 == nom  )// horizontal |----|
   {
      xcorr = 0        ; ycorr = signY * w;
   }
   else
   {
      double sl   = nom / denom;
      double sqsl = signY*sqrt( sl*sl + 1);
      xcorr = rint(w * (sl / sqsl));
      ycorr = rint(w * ( 1 / sqsl));
   }
   word it = first ? i1 : i2;
   _cdata.push_front(TP((int4b) rint(_ldata[it  ] - xcorr),(int4b) rint(_ldata[it+1] + ycorr)));
   _cdata.push_back (TP((int4b) rint(_ldata[it  ] + xcorr),(int4b) rint(_ldata[it+1] - ycorr)));
}

byte laydata::WireContour::chkCollinear(word i1, word i2, word i3)
{
   if ( 0 != orientation(i1, i2, i3)) return 0; // points not in one line
   float lambda1 = getLambda  (i3, i2, i1);
   float lambda2 = getLambda  (i1, i2, i3);
   if ((_ldata[2*i1] == _ldata[2*i3]) && (_ldata[2*i1+1] == _ldata[2*i3+1]))
      return 3;
   if ((0.0 == lambda1) && (0.0 == lambda2)) return 5; // 3 coinciding points
   if ((0.0 <  lambda1) || (0.0 <  lambda2))
      return 3; // colinear points
   if (0.0 == lambda1) return 1; //i2 and i3 coincide
   if (0.0 == lambda2) return 2; //i2 and i1 coincide
   return 4; // 3 points in one line sequenced with i2 in the middle
}

void laydata::WireContour::colPnts(word i1, word i2, word i3)
{
   TP extPnt = mdlCPnt(i1, i2);
   // Now - this is cheating! We're altering temporary one the central line
   // points and the reason is - to use the existing methods which deal with
   // point indexes
   TP swap(_ldata[2*i2], _ldata[2*i2 + 1]);
   const_cast<int4b*>(_ldata)[2*i2  ] = extPnt.x();
   const_cast<int4b*>(_ldata)[2*i2+1] = extPnt.y();
   endPnts(i1, i2,false);
   endPnts(i2 ,i3,true );
   const_cast<int4b*>(_ldata)[2*i2  ] = swap.x();
   const_cast<int4b*>(_ldata)[2*i2+1] = swap.y();
}

TP laydata::WireContour::mdlCPnt(word i1, word i2)
{
   i1 *= 2; i2 *= 2;
   double    w = _width / 2;
   double   x21 = _ldata[i2]   - _ldata[i1]  ;
   double   y21 = _ldata[i2+1] - _ldata[i1+1];
   double    L1 = sqrt(x21*x21 + y21*y21); //the length of the segment
   assert(L1 != 0.0);
   double xcorr = (w * x21)  / L1;
   double ycorr = (w * y21)  / L1;
   return TP((int4b) rint(_ldata[i2] + xcorr), (int4b) rint(_ldata[i2+1] + ycorr));
}


void laydata::WireContour::mdlAcutePnts(word i1, word i2, word i3, int angle1, int angle2)
{
   mdlPnts(i1,  i2, i3 );
   //
   i1 *= 2; i2 *= 2; i3 *= 2;
   int ysign = ((angle1 - angle2) >  0) && ((angle1 -angle2) < 90 )?  1 : - 1;
   CTM mtrx1;//
   mtrx1.Rotate(angle1);
   mtrx1.Translate(_ldata[i2], _ldata[i2+1]);
   TP p1 ( -((int4b)_width/2),  ysign * ((int4b)_width/2));
//   TP p1a(  ((int4b)_width/2), -ysign * ((int4b)_width/2));
   p1 *= mtrx1;
//   p1a*= mtrx1;

   CTM mtrx2;
   mtrx2.Rotate(angle2);
   mtrx2.Translate(_ldata[i2], _ldata[i2+1]);
   TP p2 ( -((int4b)_width/2), -ysign * ((int4b)_width/2));
//   TP p2a(  ((int4b)_width/2),  ysign * ((int4b)_width/2));
   p2 *= mtrx2;
//   p2a*= mtrx2;

   TP pi = _cdata.front(); _cdata.pop_front();
   TP pe = _cdata.back();  _cdata.pop_back();
   if (-1 == ysign)
   {
      _cdata.push_front(p1);
      _cdata.push_front(p2);
//      _cdata.push_back (p1a);
//      _cdata.push_back (p2a);
      _cdata.push_back (pe);
      _cdata.push_back (pe);
   }
   else
   {
      _cdata.push_front(pi);
      _cdata.push_front(pi);
//      _cdata.push_front(p1a);
//      _cdata.push_front(p2a);
      _cdata.push_back (p1);
      _cdata.push_back (p2);
   }
}

int laydata::WireContour::orientation(word i1, word i2, word i3)
{
   i1 *= 2; i2 *= 2; i3 *=2;
   // twice the "oriented" area of the enclosed triangle
   real area = (real(_ldata[i1]) - real(_ldata[i3])) * (real(_ldata[i2+1]) - real(_ldata[i3+1]))
             - (real(_ldata[i2]) - real(_ldata[i3])) * (real(_ldata[i1+1]) - real(_ldata[i3+1]));
   if (0 == area) return 0;
   else
      return (area > 0) ? 1 : -1;
}

double laydata::WireContour::getLambda(word i1, word i2, word ii)
{
   i1 *= 2; i2 *= 2; ii *=2;
   double denomX = _ldata[i2  ] - _ldata[ii  ];
   double denomY = _ldata[i2+1] - _ldata[ii+1];
   double lambda;
   if      (0.0 != denomX) lambda = double(_ldata[ii  ] - _ldata[i1  ]) / denomX;
   else if (0.0 != denomY) lambda = double(_ldata[ii+1] - _ldata[i1+1]) / denomY;
   // point coincides with the lp vertex of the segment
   else lambda = 0;
   return lambda;
}

//-----------------------------------------------------------------------------
/*! Returns the angle between the line and the X axis
*/
int laydata::WireContour::xangle(word i1, word i2)
{
   i1 *= 2; i2 *= 2;
   const long double Pi = 3.1415926535897932384626433832795;
   if (_ldata[i1] == _ldata[i2])
   { //vertical line
      assert(_ldata[i1+1] != _ldata[i2+1]); // make sure both points do not coincide
      if   (_ldata[i2+1] > _ldata[i1+1]) return  90;
      else                               return -90;
   }
   else if (_ldata[i1+1] == _ldata[i2+1])
   { // horizontal line
      if (_ldata[i2]    > _ldata[i1]   ) return 0;
      else                               return 180;
   }
   else
      return (int)rint(180*atan2(double(_ldata[i2+1] - _ldata[i1+1]),
                                 double(_ldata[i2]   - _ldata[i1]  ) ) /Pi);
}

//=============================================================================
/*!
 * Takes the original wire central line @parray, makes the appropriate transformations
 * using the @translation and stores the resulting wire in _ldata. Then it creates the
 * WireContour object and initializes it with the transformed data.
 */
laydata::WireContourAux::WireContourAux(const int4b* parray, unsigned lsize, const WireWidth width, const CTM& translation)
{
   _ldata = DEBUG_NEW int[2 * lsize];
   for (unsigned i = 0; i < lsize; i++)
   {
      TP cpoint(parray[2*i], parray[2*i+1]);
      cpoint *= translation;
      _ldata[2*i  ] = cpoint.x();
      _ldata[2*i+1] = cpoint.y();
   }
   DBbox wadjust(TP(), TP(width,width));
   wadjust = wadjust * translation;
   WireWidth adjwidth = abs(wadjust.p1().x() - wadjust.p2().x());
   _wcObject = DEBUG_NEW laydata::WireContour(_ldata, lsize, adjwidth);
}

/*!
 * Accelerates the WireContour usage with PointVector input data. Converts the @plist
 * into array format and stores the result in _ldata. Then creates the
 * WireContour object and initializes it with the _ldata array.
 */
laydata::WireContourAux::WireContourAux(const PointVector& plist, const WireWidth width)
{
   word psize = plist.size();
   _ldata = DEBUG_NEW int[2 * psize];
   for (unsigned i = 0; i < psize; i++)
   {
      _ldata[2*i  ] = plist[i].x();
      _ldata[2*i+1] = plist[i].y();
   }
   _wcObject = DEBUG_NEW laydata::WireContour(_ldata, psize, width);
}

laydata::WireContourAux::WireContourAux(const PointVector& plist, const WireWidth width, const TP extraP)
{
   unsigned psize = plist.size() + 1;
   _ldata = DEBUG_NEW int[2 * psize];
   for (unsigned i = 0; i < (unsigned)(psize - 1); i++)
   {
      _ldata[2*i  ] = plist[i].x();
      _ldata[2*i+1] = plist[i].y();
   }
   _ldata[2*psize - 2] = extraP.x();
   _ldata[2*psize - 1] = extraP.y();

   _wcObject = DEBUG_NEW laydata::WireContour(_ldata, psize, width);
}

/*!
 * Dumps the wire central line and the contour generated by _wxObject in @plist
 * vector in a format which can be used directly by the methods of the basic
 * renderer. The @plist must be empty.
 * The format is: plist[0].x() returns the number of the central line points;
 * plist[0].y() returns the number of the wire contour points. The central
 * line points start from plist[1]. The contour points - follow.
 */
void laydata::WireContourAux::getRenderingData(PointVector& plist)
{
   assert(_wcObject);
   assert(0 == plist.size());
   word lsize = _wcObject->lsize();
   word csize = _wcObject->csize();
   plist.reserve(lsize + csize + 1);
   plist.push_back(TP(lsize, csize));
   for (int i = 0; i < lsize; i++)
      plist.push_back(TP(_ldata[2*i], _ldata[2*i+1]));
   _wcObject->getVectorData(plist);
}

void laydata::WireContourAux::getVectorLData(PointVector& plist)
{
   assert(_wcObject);
   assert(0 == plist.size());
   word lsize = _wcObject->lsize();
   plist.reserve(lsize);
   for (int i = 0; i < lsize; i++)
      plist.push_back(TP(_ldata[2*i], _ldata[2*i+1]));
}

void laydata::WireContourAux::getVectorCData(PointVector& plist)
{
   assert(_wcObject);
   assert(0 == plist.size());
   plist.reserve(_wcObject->csize());
   _wcObject->getVectorData(plist);
}

void laydata::WireContourAux::getArrayLData(int4b* parr)
{
   assert(_wcObject);
   memcpy(parr, _ldata, sizeof(int4b)* 2 * _wcObject->lsize());
}

void laydata::WireContourAux::getArrayCData(int4b* parr)
{
   assert(_wcObject);
   _wcObject->getArrayData(parr);

}

laydata::WireContourAux::~WireContourAux()
{
   delete _wcObject;
   delete [] _ldata;
}

//=============================================================================
layprop::DrawProperties::DrawProperties() :
   _curlay                ( TLL_LAY_DEF         ),
   _clipRegion            ( 0,0                 ),
   _visualLimit           ( 40                  ),
   _cellDepthAlphaEbb     ( 0                   ),
   _cellDepthView         ( 0                   ),
   _cellMarksHidden       ( true                ),
   _cellBoxHidden         ( true                ),
   _textMarksHidden       ( true                ),
   _textBoxHidden         ( true                ),
   _adjustTextOrientation ( false               ),
   _currentOp             ( console::op_none    ),
   _blockFill             ( false               ),
   _refStack              ( NULL                ),
   _drawingLayer          ( TLL_LAY_DEF         ),
   _propertyState         ( DB                  )
{
}

bool layprop::DrawProperties::addLayer( const LayerDef& laydef )
{
   std::ostringstream lname;
   switch (_propertyState)
   {
      case DB : if (_laySetDb.end() != _laySetDb.find(laydef)) return false;
                lname << "_UNDEF" << laydef.num() << "_" << laydef.typ();
                _laySetDb.add(laydef, DEBUG_NEW LayerSettings(lname.str(),"","",""));
                return true;
      case DRC: if (_laySetDrc.end() != _laySetDrc.find(laydef)) return false;
                lname << "_DRC" << laydef.num() << "_" << laydef.typ();
                _laySetDrc.add(laydef, DEBUG_NEW LayerSettings(lname.str(),"","",""));
                return true;
      default: assert(false);break;
   }
   return true; // dummy statement to prevent compilation warnings
}

bool layprop::DrawProperties::addLayer(std::string name, const LayerDef& laydef, std::string col,
                                       std::string fill, std::string sline)
{
   if ((col != "") && (_layColors.end() == _layColors.find(col)))
   {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if ((fill != "") && (_layFill.end() == _layFill.find(fill)))
   {
      std::ostringstream ost;
      ost << "Warning! Fill \""<<fill<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   if ((sline != "") && (_lineSet.end() == _lineSet.find(sline)))
   {
      std::ostringstream ost;
      ost << "Warning! Line \""<<sline<<"\" is not defined";
      tell_log(console::MT_WARNING, ost.str());
   }
   bool new_layer = true;
   switch(_propertyState)
   {
      case DB:
         if (_laySetDb.end() != _laySetDb.find(laydef))
         {
            new_layer = false;
            delete _laySetDb[laydef];
            _laySetDb.erase(laydef);
            std::ostringstream ost;
            ost << "Warning! Layer "<<laydef<<" redefined";
            tell_log(console::MT_WARNING, ost.str());
         }
         _laySetDb.add(laydef, DEBUG_NEW LayerSettings(name,col,fill,sline));
         break;
      case DRC: assert(false); break; //User can't call DRC database directly
      default: assert(false); break;
   }
   return new_layer;
}

bool layprop::DrawProperties::addLayer(std::string name, const LayerDef& laydef)
{
   switch(_propertyState)
   {
      case DB:
         if (_laySetDb.end() != _laySetDb.find(laydef)) return false;
         _laySetDb.add(laydef, DEBUG_NEW LayerSettings(name,"","",""));
         return true;
      case DRC:
         if (_laySetDrc.end() != _laySetDrc.find(laydef)) return false;
         _laySetDrc.add(laydef, DEBUG_NEW LayerSettings(name,"","",""));
         return true;
      default: assert(false); break;
   }
   return false; // dummy statement to prevent compilation warnings
}

LayerDef layprop::DrawProperties::addLayer(std::string name)
{
   LayerDef laydef(TLL_LAY_DEF);
   // get the last layer layDef;
   for (LaySetList::Iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
      laydef = CL();
   laydef++;
   assert(addLayer(name, laydef));
   return laydef;
}

void layprop::DrawProperties::addLine(std::string name, std::string col, word pattern,
                                       byte patscale, byte width) {
   if ((col != "") && (_layColors.end() == _layColors.find(col))) {
      std::ostringstream ost;
      ost << "Warning! Color \""<<col<<"\" is not defined";
      tell_log(console::MT_WARNING,ost.str());
   }
   if (_lineSet.end() != _lineSet.find(name)) {
      delete _lineSet[name];
      std::ostringstream ost;
      ost << "Warning! Line "<< name <<" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   _lineSet[name] = DEBUG_NEW LineSettings(col,pattern,patscale,width);
}

void layprop::DrawProperties::addColor(std::string name, byte R, byte G, byte B, byte A) {
   if (_layColors.end() != _layColors.find(name)) {
      delete _layColors[name];
      std::ostringstream ost;
      ost << "Warning! Color \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   tellRGB* col = DEBUG_NEW tellRGB(R,G,B,A);
   _layColors[name] = col;
}

void layprop::DrawProperties::addFill(std::string name, byte* ptrn) {
   if (_layFill.end() != _layFill.find(name)) {
      delete [] _layFill[name];
      std::ostringstream ost;
      ost << "Warning! Fill \""<<name<<"\" redefined";
      tell_log(console::MT_WARNING, ost.str());
   }
   _layFill[name] = ptrn;
}


bool layprop::DrawProperties::setCurrentColor(const LayerDef& laydef, layprop::tellRGB& theColor)
{
   if (_drawingLayer == laydef)
      return false;
   else
   {
      _drawingLayer = laydef;
      theColor = getColor(_drawingLayer);
      return true;
   }
}

const byte* layprop::DrawProperties::getCurrentFill() const
{
   assert((REF_LAY_DEF != _drawingLayer) && 
          (GRC_LAY_DEF != _drawingLayer)    );
   // Retrive the layer settings
   const LayerSettings* ilayset = findLayerSettings(_drawingLayer);
   if (NULL != ilayset) 
   {
      if(ilayset->filled())
      { // layer is filled
         FillMap::const_iterator ifillset = _layFill.find(ilayset->fill());
         if (_layFill.end() == ifillset)
            // no stipple defined - will use default fill
            return _dfltFill;
         else
            return ifillset->second;
      }
      else return NULL;
   }
   else return NULL;
}

bool layprop::DrawProperties::layerFilled(const LayerDef& laydef) const
{
   assert(REF_LAY_DEF != laydef);
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if ((NULL != ilayset) && !_blockFill)
   {
      if(ilayset->filled()) return true;
      else return false;
   }
   else return false;
}

bool layprop::DrawProperties::getAlpha(word factor, layprop::tellRGB& theColor)
{
   theColor = getColor(_drawingLayer);
   word resultingEbb = factor * _cellDepthAlphaEbb;
   if (0 < factor)
   {
      byte alpha = (resultingEbb > (word)theColor.alpha()) ? 0 : theColor.alpha() - resultingEbb;
      theColor.setAlpha(alpha);
      return true;
   }
   return false;
}

bool  layprop::DrawProperties::layerHidden(const LayerDef& laydef) const
{
   if ((REF_LAY_DEF == laydef) || (GRC_LAY_DEF == laydef)) return false;
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL != ilayset) return ilayset->hidden();
   else return true;
}

bool  layprop::DrawProperties::layerLocked(const LayerDef& laydef) const
{
   if (REF_LAY_DEF == laydef) return false;
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL != ilayset) return ilayset->locked();
   else return true;
}

bool layprop::DrawProperties::selectable(const LayerDef& laydef) const
{
   return (!layerHidden(laydef) && !layerLocked(laydef));
}

void layprop::DrawProperties::getCurrentLine(layprop::LineSettings& lineSet, bool selected) const
{
   if (REF_LAY_DEF == _drawingLayer)
   {
      if (selected)
         lineSet = _dfltCellSBnd;
      else
         lineSet = _dfltCellBnd;
   }
   else
   {
      if (selected)
      {
         const layprop::LineSettings* theLine = getLine(_drawingLayer);
         LineSettings dummy(theLine->color(),theLine->pattern(), theLine->patscale(), theLine->width());
         lineSet = dummy;
      }
      else
      {
         lineSet = _dfltLine;
      }
   }
}

void layprop::DrawProperties::initDrawRefStack(laydata::CellRefStack* refStack)
{
   _refStack = refStack;
   _blockFill = (NULL != _refStack);
}

void layprop::DrawProperties::clearDrawRefStack()
{
   _refStack = NULL;
   _blockFill = false;
}

void  layprop::DrawProperties::postCheckCRS(const laydata::TdtCellRef* cref)
{
   assert(cref);
   if (_refStack)
   {
      if (_refStack->empty()) _blockFill = true;
      _refStack->push_front(cref);
   }
}

layprop::CellRefChainType layprop::DrawProperties::preCheckCRS(const laydata::TdtCellRef* cref)
{
   assert(cref);
   if (_refStack)
      if (!_refStack->empty())
         if (_refStack->front() == cref )
         {
            _refStack->pop_front();
            if (_refStack->empty())
            {
               _blockFill = false;
               return crc_ACTIVE;
            }
            else return crc_PREACTIVE;
         }
         else return crc_VIEW;
      else return crc_POSTACTIVE;
   else return crc_VIEW;
}

//void layprop::DrawProperties::drawReferenceMarks(const TP& p0, const binding_marks mark_type) const
//{
//   GLubyte* the_mark;
//   switch (mark_type)
//   {
//      case  cell_mark:if (_cellMarksHidden) return;
//      else
//      {
//         glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
//         the_mark = cell_mark_bmp;
//         break;
//      }
//      case array_mark:if (_cellMarksHidden) return;
//      else
//      {
//         glColor4f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.8);
//         the_mark = array_mark_bmp;
//         break;
//      }
//      case  text_mark:if (_textMarksHidden) return;
//      else the_mark = text_mark_bmp;break;
//      default: assert(false); break;
//   }
//   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//   glRasterPos2i(p0.x(),p0.y());
//   glBitmap(16,16,7,7,0,0, the_mark);
//}

LayerDef layprop::DrawProperties::getLayerNo(std::string name) const
{
   for (LaySetList::Iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
   {
      if (name == CL->name()) return CL();
   }
   return ERR_LAY_DEF;
}

LayerDefList layprop::DrawProperties::getAllLayers() const
{
   LayerDefList listLayers;
   for( LaySetList::Iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
      listLayers.push_back(CL());
   return listLayers;
}

void layprop::DrawProperties::allUnselectable(LayerDefSet& layset)
{
   for( LaySetList::Iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
   {
      if (CL->hidden() || CL->locked())
         layset.insert(CL());
   }
   layset.insert(GRC_LAY_DEF);
}

void layprop::DrawProperties::allInvisible(LayerDefSet& layset)
{
   for( LaySetList::Iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
   {
      if (CL->hidden())
         layset.insert(CL());
   }
}
//WordList layprop::PropertyCenter::getLockedLayers() const
//{
//   WordList lockedLayers;
//   for( LaySetList::const_iterator = getCurSetList().begin(); it != getCurSetList().end(); it++)
//      if(it->second->locked()) lockedLayers.push_back(it->first);
//   return lockedLayers;
//}

std::string layprop::DrawProperties::getLayerName(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL != ilayset) return ilayset->name();
   else return "";
}

std::string layprop::DrawProperties::getColorName(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL != ilayset)
   {
      return ilayset->color();
   }
   else return "";
}

std::string layprop::DrawProperties::getFillName(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL != ilayset)
   {
      return ilayset->fill();
   }
   else return "";
}

std::string layprop::DrawProperties::getLineName(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL != ilayset)
   {
      return ilayset->sline();
   }
   else return "";
}

LayerDef layprop::DrawProperties::getTenderLay(const LayerDef& laydef) const
{
//   if (REF_LAY == layno) return layno;//no references yet in the DRC DB
   switch (_propertyState)
   {
      case DB: return laydef;
      case DRC: return DRC_LAY_DEF;
      default: assert(false); break;
   }
   return laydef; // dummy, to prevent warnings
}

void layprop::DrawProperties::allLayers(NameList& alllays) const
{
   for (LaySetList::Iterator CL = getCurSetList().begin(); CL != getCurSetList().end(); CL++)
      if (REF_LAY_DEF != CL()) alllays.push_back(CL->name());
}

void layprop::DrawProperties::allColors(NameList& colist) const
{
   for( ColorMap::const_iterator CI = _layColors.begin(); CI != _layColors.end(); CI++)
      colist.push_back(CI->first);
}

void layprop::DrawProperties::allFills(NameList& filist) const
{
   for( FillMap::const_iterator CI = _layFill.begin(); CI != _layFill.end(); CI++)
      filist.push_back(CI->first);
}

void layprop::DrawProperties::allLines(NameList& linelist) const
{
   for( LineMap::const_iterator CI = _lineSet.begin(); CI != _lineSet.end(); CI++)
      linelist.push_back(CI->first);
}

const layprop::LineSettings* layprop::DrawProperties::getLine(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL == ilayset) return &_dfltSLine;
   LineMap::const_iterator line = _lineSet.find(ilayset->sline());
   if (_lineSet.end() == line) return &_dfltSLine;
   return line->second;
// All the stuff above is equivalent to
//   return _layFill[_layset[layno]->sline()];
// but is safer and preserves constness
}

const layprop::LineSettings* layprop::DrawProperties::getLine(std::string line_name) const
{
   LineMap::const_iterator line = _lineSet.find(line_name);
   if (_lineSet.end() == line) return &_dfltSLine;
   return line->second;
// All the stuff above is equivalent to
//   return _layFill[_layset[layno]->sline()];
// but is safer and preserves constness
}

const byte* layprop::DrawProperties::getFill(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL == ilayset) return &_dfltFill[0];
   FillMap::const_iterator fill_set = _layFill.find(ilayset->fill());
   if (_layFill.end() == fill_set) return &_dfltFill[0];
   return fill_set->second;
// All the stuff above is equivalent to
//   return _layFill[_layset[layno]->fill()];
// but is safer and preserves constness
}

const byte* layprop::DrawProperties::getFill(std::string fill_name) const
{
   FillMap::const_iterator fill_set = _layFill.find(fill_name);
   if (_layFill.end() == fill_set) return &_dfltFill[0];
   return fill_set->second;
// All the stuff above is equivalent to
//   return _layFill[fill_name];
// but is safer and preserves constness
}

const layprop::tellRGB& layprop::DrawProperties::getColor(const LayerDef& laydef) const
{
   const LayerSettings* ilayset = findLayerSettings(laydef);
   if (NULL == ilayset) return _dfltColor;
   ColorMap::const_iterator col_set = _layColors.find(ilayset->color());
   if (_layColors.end() == col_set) return _dfltColor;
   return *(col_set->second);
// All the stuff above is equivalent to
//   return _layColors[_layset[layno]->color()];
// but is safer and preserves constness
}

const layprop::tellRGB& layprop::DrawProperties::getColor(std::string color_name) const
{
   ColorMap::const_iterator col_set = _layColors.find(color_name);
   if (_layColors.end() == col_set) return _dfltColor;
   return *(col_set->second);
// All the stuff above is equivalent to
//   return _layColors[color_name];
// but is safer and preserves constness
}

void layprop::DrawProperties::savePatterns(FILE* prop_file) const
{
   FillMap::const_iterator CI;
   fprintf(prop_file, "void  fillSetup() {\n");
   for( CI = _layFill.begin(); CI != _layFill.end(); CI++)
   {
      fprintf(prop_file, "   int list _%s = {\n", CI->first.c_str());
      byte* patdef = CI->second;
      for (byte i = 0; i < 16; i++)
      {
         fprintf(prop_file, "      ");
         for (byte j = 0; j < 8; j++)
         {
            if (127 == i*8+j)
               fprintf(prop_file, "0x%02x  "  , patdef[127]);
            else
               fprintf(prop_file, "0x%02x ,", patdef[i*8+j]);
         }
         fprintf(prop_file, "\n");
      }
      fprintf(prop_file, "   };\n\n");
   }
   for( CI = _layFill.begin(); CI != _layFill.end(); CI++)
   {
      fprintf(prop_file, "   definefill(\"%s\", _%s );\n", CI->first.c_str(), CI->first.c_str());
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveColors(FILE* prop_file) const
{
   ColorMap::const_iterator CI;
   fprintf(prop_file, "void  colorSetup() {\n");
   for( CI = _layColors.begin(); CI != _layColors.end(); CI++)
   {
      tellRGB* the_color = CI->second;
      fprintf(prop_file, "   definecolor(\"%s\", %3d, %3d, %3d, %3d);\n",
              CI->first.c_str() ,
              the_color->red()  ,
              the_color->green(),
              the_color->blue() ,
              the_color->alpha()
             );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLayers(FILE* prop_file) const
{
   fprintf(prop_file, "void  layerSetup() {\n");
   fprintf(prop_file, "   colorSetup(); fillSetup(); lineSetup();\n");
   for( LaySetList::Iterator CI = getCurSetList().begin(); CI != getCurSetList().end(); CI++ )
   {
      if (REF_LAY_DEF == CI()) continue;
      std::stringstream wstr;
      wstr << "   layprop(\"" << CI->name()  <<
              "\", "          << CI()        <<
              ", \""          << CI->color() <<
              "\", \""        << CI->fill()  <<
              "\", \""        << CI->sline() <<
              "\");"          << std::endl;
      fprintf(prop_file, "%s", wstr.str().c_str());
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLines(FILE* prop_file) const
{
   LineMap::const_iterator CI;
   fprintf(prop_file, "void  lineSetup() {\n");
   for( CI = _lineSet.begin(); CI != _lineSet.end(); CI++)
   {
      LineSettings* the_line = CI->second;
      fprintf(prop_file, "   defineline(\"%s\", \"%s\", 0x%04x , %d, %d);\n",
              CI->first.c_str()         ,
              the_line->color().c_str() ,
              the_line->pattern()       ,
              the_line->patscale()      ,
              the_line->width()            );
   }
   fprintf(prop_file, "}\n\n");
}

void layprop::DrawProperties::saveLayState(FILE* prop_file) const
{
   LayStateMap::const_iterator CS;
   fprintf(prop_file, "void  layerState() {\n");
   for (CS = _layStateMap.begin(); CS != _layStateMap.end(); CS++)
   {
      LayStateList the_state(CS->second);
      //TODO In order to save a layer status we need something like:
      // locklayers({<all>}, false);
      // hidelayers({<all>}, false);
      // filllayers({<all>}, false);
      // locklayers({<listed>}, true);
      // hidelayers({<listed>}, true);
      // filllayers({<listed>}, true);
      // usinglayer(<layno>);
      // savelaystatus(<name>);
      // where <all>     - All defined layers
      //       <listed>  - The LayStateList members which have the corresponding property
      //                   set to true
      // This method is not called at the moment!
      fprintf(prop_file, "   savelaystatus(\"%s\");\n",
               CS->first.c_str());
   }
   fprintf(prop_file, "}\n\n");

}

const layprop::LayerSettings*  layprop::DrawProperties::findLayerSettings(const LayerDef& laydef) const
{
   LaySetList::Iterator ilayset;
   switch (_propertyState)
   {
      case DB : ilayset = _laySetDb.find(laydef) ; if (_laySetDb.end()  == ilayset) return NULL; break;
      case DRC: ilayset = _laySetDrc.find(laydef); if (_laySetDrc.end() == ilayset) return NULL; break;
      default: assert(false);break;
   }
   return *ilayset;
}

const layprop::LaySetList& layprop::DrawProperties::getCurSetList() const
{
   switch (_propertyState)
   {
      case DB : return _laySetDb;
      case DRC: return _laySetDrc;
      default: assert(false);break;
   }
   return _laySetDb; // dummy, to prevent warnings
}


void  layprop::DrawProperties::hideLayer(const LayerDef& laydef, bool hide)
{
   // No error messages here, because of possible range use
   LayerSettings* ilayset = const_cast<LayerSettings*>(findLayerSettings(laydef));
   if (NULL != ilayset)
      ilayset->_hidden = hide;
}

void  layprop::DrawProperties::lockLayer(const LayerDef& laydef, bool lock)
{
   // No error messages here, because of possible range use
   LayerSettings* ilayset = const_cast<LayerSettings*>(findLayerSettings(laydef));
   if (NULL != ilayset)
      ilayset->_locked = lock;
}

void  layprop::DrawProperties::fillLayer(const LayerDef& laydef, bool fill)
{
   // No error messages here, because of possible range use
   LayerSettings* ilayset = const_cast<LayerSettings*>(findLayerSettings(laydef));
   if (NULL != ilayset)
      ilayset->fillLayer(fill);
}

layprop::DrawProperties::~DrawProperties() {
   //clear all databases
   setState(layprop::DRC);
   for (LaySetList::Iterator LSI = getCurSetList().begin(); LSI != getCurSetList().end(); LSI++)
      delete (*LSI);
   setState(layprop::DB);
   for (LaySetList::Iterator LSI = getCurSetList().begin(); LSI != getCurSetList().end(); LSI++)
      delete (*LSI);

   for (ColorMap::iterator CMI = _layColors.begin(); CMI != _layColors.end(); CMI++)
      delete CMI->second;
   for (FillMap::iterator FMI = _layFill.begin(); FMI != _layFill.end(); FMI++)
      delete [] FMI->second;
   for (LineMap::iterator LMI = _lineSet.begin(); LMI != _lineSet.end(); LMI++)
      delete LMI->second;
//   if (NULL != _refStack) delete _refStack; -> deleted in EditObject
}

/*! Shall be called by the execute method of loadlaystatus TELL function.
 * Stores the current state of the defined layers in a _layStateHistory
 * WARNING! This function is only for undo purposes. Should not be used
 * to store/change/delete the layer state
 */
void layprop::DrawProperties::pushLayerStatus()
{
   _layStateHistory.push_front(LayStateList(_curlay, std::list<LayerState>()));
   LayStateList& clist = _layStateHistory.front();
   for (LaySetList::Iterator CL = _laySetDb.begin(); CL != _laySetDb.end(); CL++)
   {
      clist.second.push_back(LayerState(CL(), *(*CL)));
   }
//   clist.first = _curlay.num();
}

/*! Shall be called by the undo method of loadlaystatus TELL function.
 * Restores the loch/hide/fill state of the defined layers in a _laySetDb
 * WARNING! This function is only for undo purposes. Should not be used
 * to store/change/delete the layer state
 */
void layprop::DrawProperties::popLayerStatus()
{
   LayStateList& clist = _layStateHistory.front();
   for (std::list<LayerState>::const_iterator CL = clist.second.begin(); CL != clist.second.end(); CL++)
   {
      LaySetList::Iterator clay;
      if (_laySetDb.end() != (clay = _laySetDb.find(CL->layDef())))
      {
         clay->_filled = CL->filled();
         TpdPost::layer_status(tui::BT_LAYER_FILL, CL->layDef(), CL->filled());
         clay->_hidden = CL->hidden();
         TpdPost::layer_status(tui::BT_LAYER_HIDE, CL->layDef(), CL->hidden());
         clay->_locked = CL->locked();
         TpdPost::layer_status(tui::BT_LAYER_LOCK, CL->layDef(), CL->locked());
      }
   }
   TpdPost::layer_default(clist.first, _curlay);
   _curlay = clist.first;
   _layStateHistory.pop_front();
}

/*!
 * Removes the oldest saved state in the _layStateHistory. Should be called
 * by undo_cleanup methods of the related tell functions.
 * WARNING! This function is only for undo purposes. Should not be used
 * to store/change/delete the layer state
 */
void layprop::DrawProperties::popBackLayerStatus()
{
   _layStateHistory.pop_back();
}

bool layprop::DrawProperties::saveLaysetStatus(const std::string& sname)
{
   LayStateList clist(_curlay, std::list<LayerState>() );
   bool status = true;
   for (LaySetList::Iterator CL = _laySetDb.begin(); CL != _laySetDb.end(); CL++)
   {
      clist.second.push_back(LayerState(CL(), *(*CL)));
   }
//   clist.first = _curlay;
   if (_layStateMap.end() != _layStateMap.find(sname)) status = false;
   _layStateMap.insert(std::pair<std::string, LayStateList>(sname, clist));
   return status;
}

bool layprop::DrawProperties::saveLaysetStatus(const std::string& sname, const LayerDefSet& hidel,
      const LayerDefSet& lockl, const LayerDefSet& filll, const LayerDef& alaydef)
{
   LayStateList clist(alaydef, std::list<LayerState>());
   bool status = true;
   for (LaySetList::Iterator CL = _laySetDb.begin(); CL != _laySetDb.end(); CL++)
   {
      bool hiden  = (hidel.end() != hidel.find(CL()));
      bool locked = (lockl.end() != lockl.find(CL()));
      bool filled = (filll.end() != filll.find(CL()));
      clist.second.push_back(LayerState(CL(), hiden, locked, filled));
   }
//   clist.first = alaydef;
   if (_layStateMap.end() == _layStateMap.find(sname)) status = false;
//   _layStateMap[sname] = clist;
   _layStateMap.insert(std::pair<std::string, LayStateList>(sname, clist));

   return status;
}

bool layprop::DrawProperties::loadLaysetStatus(const std::string& sname)
{
   LayStateMap::iterator CLS = _layStateMap.find(sname);
   if (_layStateMap.end() == CLS) return false;
   LayStateList clist(CLS->second);
   for (std::list<LayerState>::const_iterator CL = clist.second.begin(); CL != clist.second.end(); CL++)
   {
      LaySetList::Iterator clay;
      if (_laySetDb.end() != (clay = _laySetDb.find(CL->layDef())))
      {
         clay->_filled = CL->filled();
         TpdPost::layer_status(tui::BT_LAYER_FILL, CL->layDef(), CL->filled());
         clay->_hidden = CL->hidden();
         TpdPost::layer_status(tui::BT_LAYER_HIDE, CL->layDef(), CL->hidden());
         clay->_locked = CL->locked();
         TpdPost::layer_status(tui::BT_LAYER_LOCK, CL->layDef(), CL->locked());
      }
   }
   TpdPost::layer_default(clist.first, _curlay);
   _curlay = clist.first;
   return true;
}

bool layprop::DrawProperties::deleteLaysetStatus(const std::string& sname)
{
   if (_layStateMap.end() == _layStateMap.find(sname)) return false;
   _layStateMap.erase(sname);
   return true;
}

bool layprop::DrawProperties::getLaysetStatus(const std::string& sname, LayerDefSet& hidel,
                                              LayerDefSet& lockl, LayerDefSet& filll, LayerDef& activelaydef)
{
   LayStateMap::iterator CLS = _layStateMap.find(sname);
   if (_layStateMap.end() == CLS) return false;
   LayStateList clist(CLS->second);
   for (std::list<LayerState>::const_iterator CL = clist.second.begin(); CL != clist.second.end(); CL++)
   {
      if (CL->hidden()) hidel.insert(hidel.begin(),CL->layDef());
      if (CL->locked()) lockl.insert(lockl.begin(),CL->layDef());
      if (CL->filled()) filll.insert(filll.begin(),CL->layDef());
   }
   activelaydef = clist.first;
   return true;
}

