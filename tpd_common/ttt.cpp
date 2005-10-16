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
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sat Aug 23 2003
//         Author: s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision: 1.13 $
//          $Date: 2005-10-05 23:25:12 +0100 (Wed, 05 Oct 2005) $
//        $Author: skr $
//===========================================================================

#include <math.h>
#include <sstream>
#include "ttt.h"

//-----------------------------------------------------------------------------
// class TP
//-----------------------------------------------------------------------------
TP::TP(real x, real y, real scale) {
   _x = (int4b) rint(x * scale);
   _y = (int4b) rint(y * scale);
}

void TP::roundTO(int4b step) {
   if (0 == step) step = 1;
   _x = (_x >= 0) ? (int4b) (rint((_x + (step/2)) / step) * step) :
                    (int4b) (rint((_x - (step/2)) / step) * step) ;
   _y = (_y >= 0) ? (int4b) (rint((_y + (step/2)) / step) * step) :
                    (int4b) (rint((_y - (step/2)) / step) * step) ;
}

TP TP::operator * (const CTM& op2) const {
   return TP(int4b(op2.a() * x() + op2.c() * y() + op2.tx()),
              int4b(op2.b() * x() + op2.d() * y() + op2.ty()));  
}

TP TP::operator *= (const CTM& op2) {
   int4b x_new = int4b(op2.a() * x() + op2.c() * y() + op2.tx());
   int4b y_new = int4b(op2.b() * x() + op2.d() * y() + op2.ty());
   _x = x_new; _y = y_new;
  return *this;
}

//TP TP::operator = ( const QPoint& qp) {
//   _x = qp.x(); _y = qp.y();
//   return *this;
//}

void TP::info(std::ostringstream& ost) const {
   ost << "( " << _x << " , " << _y << " )";
}

//-----------------------------------------------------------------------------
// class DBbox
//-----------------------------------------------------------------------------
DBbox::DBbox(int4b x1, int4b y1, int4b x2, int4b y2) {
   _p1 = TP(x1,y1);
   _p2 = TP(x2,y2);
}

void DBbox::overlap(const TP p) {
   // p1 is min; p2 is max
   if (_p1.x() > p.x()) _p1._x = p.x();
   if (_p2.x() < p.x()) _p2._x = p.x();
   if (_p1.y() > p.y()) _p1._y = p.y();
   if (_p2.y() < p.y()) _p2._y = p.y();
}

void DBbox::overlap(const DBbox bx) {
   if (DEFAULT_OVL_BOX == bx) return;
   if (DEFAULT_OVL_BOX == (*this)) {
      if (bx.p1().x() > bx.p2().x()) {
         _p1._x = bx.p2().x(); _p2._x = bx.p1().x(); }   
      else {
         _p1._x = bx.p1().x(); _p2._x = bx.p2().x(); }
      if (bx.p1().y() > bx.p2().y()) {
         _p1._y = bx.p2().y(); _p2._y = bx.p1().y(); }   
      else {
         _p1._y = bx.p1().y(); _p2._y = bx.p2().y(); }
   }         
   else {
      overlap(bx.p1()); overlap(bx.p2());
   }   
}

void DBbox::normalize() {
   int4b swap;
   if (_p1.x() > _p2.x()) {
      swap = _p1.x(); _p1._x = _p2.x(); _p2._x = swap;
   }
   if (_p1.y() > _p2.y()) {
      swap = _p1.y(); _p1._y = _p2.y(); _p2._y = swap;
   }
}   
   
float DBbox::cliparea(const DBbox& bx, bool calculate) {
   // returns: -1 - if bx is entirely inside this
   //           0 - if bx is entirely outside this
   //   otherwise - the AND area of the boxes
   // using Cohen-Sutherland quotation, determine the location of bx points
   // towards (this)
   char A_place;
   if       (bx.p1().x() <  _p1.x()) A_place = 0x01;
   else if (bx.p1().x() <=  _p2.x()) A_place = 0x00;
   else                              A_place = 0x02;
   if       (bx.p1().y() <  _p1.y()) A_place |= 0x04;
   else if (bx.p1().y() <=  _p2.y()) A_place |= 0x00;
   else                              A_place |= 0x08;
   if ((A_place & 0x0A) > 0) return 0.0; // outside
   char B_place;
   if       (bx.p2().x() <  _p1.x()) B_place = 0x01;
   else if (bx.p2().x() <=  _p2.x()) B_place = 0x00;
   else                              B_place = 0x02;
   if       (bx.p2().y() <  _p1.y()) B_place |= 0x04;
   else if (bx.p2().y() <=  _p2.y()) B_place |= 0x00;
   else                              B_place |= 0x08;
   if ((B_place & 0x05) > 0) return 0.0; // outside
   if ((A_place | B_place) == 0) return -1.0; //inside;
   // the boxes intersect each other, so let's find the intersection area
   if (!calculate) return 1.0;
   TP *Aprim, *Bprim;
   switch (A_place) {
      case 0x00: Aprim = new TP(bx.p1()); break;
      case 0x01: Aprim = new TP(_p1.x(), bx.p1().y());break;
      case 0x04: Aprim = new TP(bx.p1().x(), _p1.y());break;
      case 0x05: Aprim = new TP(_p1);break;
      default: assert(false);
   }
   switch (B_place) {
      case 0x00: Bprim = new TP(bx.p2());break;
      case 0x02: Bprim = new TP(_p2.x(),bx.p2().y());break;
      case 0x08: Bprim = new TP(bx.p2().x(),_p2.y());break;
      case 0x0A: Bprim = new TP(_p2);break;
      default: assert(false);
   }
   float area =  fabsf((Aprim->x() - Bprim->x()) * (Aprim->y() - Bprim->y()));
   delete Aprim; delete Bprim;
   return area;
}

int DBbox::clipbox(DBbox& bx) {
   // returns: -1 - if bx is entirely inside this
   //           0 - if bx is entirely outside this
   //   otherwise - the AND area of the boxes
   // using Cohen-Sutherland quotation, determine the location of bx points
   // towards (this)
   char A_place;
   if       (bx.p1().x() <  _p1.x()) A_place = 0x01;
   else if (bx.p1().x() <=  _p2.x()) A_place = 0x00;
   else                              A_place = 0x02;
   if       (bx.p1().y() <  _p1.y()) A_place |= 0x04;
   else if (bx.p1().y() <=  _p2.y()) A_place |= 0x00;
   else                              A_place |= 0x08;
   if ((A_place & 0x0A) > 0) return 0; // outside
   char B_place;
   if       (bx.p2().x() <  _p1.x()) B_place = 0x01;
   else if (bx.p2().x() <=  _p2.x()) B_place = 0x00;
   else                              B_place = 0x02;
   if       (bx.p2().y() <  _p1.y()) B_place |= 0x04;
   else if (bx.p2().y() <=  _p2.y()) B_place |= 0x00;
   else                              B_place |= 0x08;
   if ((B_place & 0x05) > 0) return 0; // outside
   if ((A_place | B_place) == 0) return -1; //inside;
   // the boxes intersect each other, so let's find the intersection area
   TP *Aprim, *Bprim;
   switch (A_place) {
      case 0x00: Aprim = new TP(bx.p1()); break;
      case 0x01: Aprim = new TP(_p1.x(), bx.p1().y());break;
      case 0x04: Aprim = new TP(bx.p1().x(), _p1.y());break;
      case 0x05: Aprim = new TP(_p1);break;
      default: assert(false);
   }
   switch (B_place) {
      case 0x00: Bprim = new TP(bx.p2());break;
      case 0x02: Bprim = new TP(_p2.x(),bx.p2().y());break;
      case 0x08: Bprim = new TP(bx.p2().x(),_p2.y());break;
      case 0x0A: Bprim = new TP(_p2);break;
      default: assert(false);
   }
   bx = DBbox(*Aprim, *Bprim);
   delete Aprim; delete Bprim;
   return 1;
}

bool DBbox::inside(const TP& pnt) {
   // using Cohen-Sutherland quotation, determine the location of pnt
   // towards (this)
   char A_place;
   if       (pnt.x() <  _p1.x()) A_place = 0x01;
   else if (pnt.x() <=  _p2.x()) A_place = 0x00;
   else                          A_place = 0x02;
   if       (pnt.y() <  _p1.y()) A_place |= 0x04;
   else if (pnt.y() <=  _p2.y()) A_place |= 0x00;
   else                          A_place |= 0x08;
   if (A_place != 0) return false; // outside
   else              return true;
}

float DBbox::area() {
   return fabsf((_p1.x() - _p2.x()) * (_p1.y() - _p2.y()));
}

DBbox DBbox::getcorner(byte corner) {
   switch(corner) {
      case 0: // NW
   return DBbox(_p1.x(), static_cast<int4b>(rint((_p2.y() + _p1.y()) / 2)),
                 static_cast<int4b>(rint((_p2.x() + _p1.x()) / 2)),_p2.y());
      case 1: // NE           
   return DBbox(static_cast<int4b>(rint((_p2.x() + _p1.x()) / 2)), 
                 static_cast<int4b>(rint((_p2.y() + _p1.y()) / 2)),
                 _p2.x(),_p2.y());
      case 2: // SE
   return DBbox(static_cast<int4b>(rint((_p2.x() + _p1.x()) / 2)), _p1.y(),
                 _p2.x(), static_cast<int4b>(rint((_p2.y() + _p1.y()) / 2)));
      case 3: // SW
   return DBbox(_p1.x(),_p1.y(),
                 static_cast<int4b>(rint((_p2.x() + _p1.x()) / 2)), 
                 static_cast<int4b>(rint((_p2.y() + _p1.y()) / 2)));
   }              
}
   
DBbox DBbox::operator * (const CTM& op2) const {
   TP np1(int4b(op2.a() * _p1.x() + op2.c() * _p1.y() + op2.tx()),
          int4b(op2.b() * _p1.x() + op2.d() * _p1.y() + op2.ty()));
   TP np2(int4b(op2.a() * _p2.x() + op2.c() * _p2.y() + op2.tx()),
          int4b(op2.b() * _p2.x() + op2.d() * _p2.y() + op2.ty()));
   return DBbox(np1, np2);       
}

DBbox DBbox::operator = (const DBbox& bx) {
   _p1 = bx.p1();
   _p2 = bx.p2();
   return *this;       
}

bool  DBbox::operator == (const DBbox& bx) const {
   return ((_p1 == bx.p1()) && (_p2 == bx.p2()));
}

bool  DBbox::operator != (const DBbox& bx) const {
   return (!((_p1 == bx.p1()) && (_p2 == bx.p2())));
}

//-----------------------------------------------------------------------------
// class SGBitSet
//-----------------------------------------------------------------------------
SGBitSet::SGBitSet(word  bit_length) {
   _size = bit_length;
   word nb = _size / 8;
   _packet = new byte[nb+1];
   for (word i = 0; i <= nb; i++) _packet[i] = 0;
}     

SGBitSet::SGBitSet(SGBitSet*  bs) {
   _size = bs->size();
   word nb = _size / 8;
   _packet = new byte[nb+1];
   for (word i = 0; i <= nb; i++) _packet[i] = bs->_packet[i];
}     

void SGBitSet::set(word  bit) {
   assert(bit <= _size);
   _packet[bit / 8] |= (0x01 << (bit % 8));
}     

void SGBitSet::setall() {
   assert(_size);
   for(word i = 0; i < _size / 8; i++)
      _packet[i] = 0xFF;
   _packet[(_size / 8)] = (0xFF >> (8 - (_size % 8)));
}     

void SGBitSet::reset(word  bit) {
   assert(bit <= _size);
   _packet[bit / 8] &= ~(0x01 << (bit % 8));
}     

void SGBitSet::check_neighbours_set(bool wire) {
   word size;
   if (wire) 
      if (_size > 2) size = _size - 2;
      else return;
   else size = _size;
   for (word indx = 0; indx < size; indx++)
      if ((!(_packet[((indx  ) % _size) / 8] & (0x01 << ((indx  ) % _size) % 8))) &&
            (_packet[((indx+1) % _size) / 8] & (0x01 << ((indx+1) % _size) % 8)) &&
          (!(_packet[((indx+2) % _size) / 8] & (0x01 << ((indx+2) % _size) % 8))) )
          _packet[(((indx+1) % _size) / 8)] &= ~(0x01 << ((indx+1) % _size)% 8); //reset
}

bool SGBitSet::check(word  bit) const {
   assert(bit <= _size);
   return (_packet[bit / 8] & (0x01 << (bit % 8)));
}     

bool SGBitSet::isallclear() const {
   assert(_size);
   for(word i = 0; i <= _size / 8; i++)
      if (0x00 != _packet[i]) return false;
   return true;   
}     

bool SGBitSet::isallset() const {
   assert(_size);
   for(word i = 0; i < _size / 8; i++)
      if (0xFF != _packet[i]) return false;
   if (_packet[(_size / 8)] != (0xFF << (8 - (_size % 8)))) return false;
   return true;   
}     

SGBitSet::~SGBitSet() {
   delete [] _packet;
}         


//-----------------------------------------------------------------------------
// class CTM
//-----------------------------------------------------------------------------
CTM::CTM(TP dp, real scale, real rotation,bool reflX) {
   Initialize();
   if (reflX)          FlipX();
   if (0 != rotation)  Rotate(rotation);
   if (1 != scale   ) Scale(scale, scale);
   Translate((real) dp.x(),(real) dp.y());
}

CTM CTM::Rotate(real alfa) {// alfa - in degrees
   const long double Pi = 3.1415926535897932384626433832795;
   long double alfaG = (Pi/180)*alfa;// translate in radians
   return (*this *= CTM(cos(alfaG),sin(alfaG),-sin(alfaG),cos(alfaG),0,0));
}

CTM CTM::Reversed() const {
   real denom = (a() * d()) - (b() * c());
   return CTM(                 d()/denom,                   -b()/denom,// 0.0,
                               -c()/denom,                    a()/denom,// 0.0,
              (ty()*c() - tx()*d())/denom , (tx()*b() - ty()*a())/denom );// 1.0 );
}

CTM CTM::operator * (const CTM op2) const{
   CTM res;
   res._a  = a()  * op2.a() + b()  * op2.c();
   res._b  = a()  * op2.b() + b()  * op2.d();
   res._c  = c()  * op2.a() + d()  * op2.c();
   res._d  = c()  * op2.b() + d()  * op2.d();
   res._tx = tx() * op2.a() + ty() * op2.c() + op2.tx();
   res._ty = tx() * op2.b() + ty() * op2.d() + op2.ty();
   return res;
}

CTM CTM::operator = (const CTM op2) {
   _a = op2.a();_b = op2.b();_c = op2.c();_d = op2.d();
   _tx = op2.tx();_ty = op2.ty();
   return *this;
}

