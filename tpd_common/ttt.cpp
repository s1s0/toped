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
//        Created: Sat Aug 23 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Basic common Toped types&classes
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <math.h>
#include <sstream>
#include <algorithm>
#include <functional>
#include "ttt.h"


//-----------------------------------------------------------------------------
// class LayerDef
//-----------------------------------------------------------------------------
bool LayerDef::operator==(const LayerDef& cmp) const
{
   return ((_num == cmp._num) && (_typ == cmp._typ));
}

bool LayerDef::operator!=(const LayerDef& cmp) const
{
   return ((_num != cmp._num) || (_typ != cmp._typ));
}

const LayerDef LayerDef::operator++(int)/*Postfix*/
{
   LayerDef current(*this); _num++; return current;
}

bool LayerDef::operator< (const LayerDef& cmp) const
{
   return (_num == cmp._num) ? (_typ < cmp._typ) : (_num < cmp._num);
}

void LayerDef::toGds(word& lay, word& typ) const
{
   assert(MAX_WORD_VALUE >= _num);
   assert(MAX_WORD_VALUE >= _typ);
   lay = (word)_num;
   typ = (word)_typ;
}

std::string LayerDef::toQList() const
{
   std::ostringstream result;
   result << _num << ";" << _typ;
   return result.str();
}

bool LayerDef::editable() const
{
   return (!(_num > LAST_EDITABLE_LAYNUM));
}

std::ostream& operator <<(std::ostream& os, const LayerDef& obj)
{
   os << "{ " << obj.num() << ", " << obj.typ() << "}";
   return os;
}

//-----------------------------------------------------------------------------
// class TP
//-----------------------------------------------------------------------------
TP::TP(real x, real y, real scale)
{
   _x = (int4b) rint(x * scale);
   _y = (int4b) rint(y * scale);
}

void TP::roundTO(int4b step)
{
   if (0 == step) step = 1;
   _x = (_x >= 0) ? (int4b) (rint((_x + (step/2)) / step) * step) :
                    (int4b) (rint((_x - (step/2)) / step) * step) ;
   _y = (_y >= 0) ? (int4b) (rint((_y + (step/2)) / step) * step) :
                    (int4b) (rint((_y - (step/2)) / step) * step) ;
}

TP TP::operator * (const CTM& op2) const
{
   int8b Xlongtmp = lround(op2.a() * (real)x() + op2.c() * (real)y() + op2.tx());
   int8b Ylongtmp = lround(op2.b() * (real)x() + op2.d() * (real)y() + op2.ty());
   int4b Xtmp = (Xlongtmp > MAX_INT4B) ? MAX_INT4B :
                (Xlongtmp < MIN_INT4B) ? MIN_INT4B : static_cast<int4b>(Xlongtmp);
   int4b Ytmp = (Ylongtmp > MAX_INT4B) ? MAX_INT4B :
                (Ylongtmp < MIN_INT4B) ? MIN_INT4B : static_cast<int4b>(Ylongtmp);
   return TP(Xtmp, Ytmp);
}

TP TP::operator *= (const CTM& op2)
{
   int8b Xlongtmp = lround(op2.a() * (real)x() + op2.c() * (real)y() + op2.tx());
   int8b Ylongtmp = lround(op2.b() * (real)x() + op2.d() * (real)y() + op2.ty());
   _x = (Xlongtmp > MAX_INT4B) ? MAX_INT4B :
        (Xlongtmp < MIN_INT4B) ? MIN_INT4B : static_cast<int4b>(Xlongtmp);
   _y = (Ylongtmp > MAX_INT4B) ? MAX_INT4B :
        (Ylongtmp < MIN_INT4B) ? MIN_INT4B : static_cast<int4b>(Ylongtmp);
  return *this;
}

TP TP::operator *= (const real factor)
{
   _x = (int4b) rint((real)_x * factor);
   _y = (int4b) rint((real)_y * factor);
   return *this;
}

TP TP::operator /= (const real factor)
{
   _x = (int4b) rint((real)_x / factor);
   _y = (int4b) rint((real)_y / factor);
   return *this;
}

//TP TP::operator = ( const QPoint& qp) {
//   _x = qp.x(); _y = qp.y();
//   return *this;
//}

void TP::info(std::ostringstream& ost, real DBU) const {
   ost << "{ " << _x/DBU << " , " << _y/DBU << " }";
}

//-----------------------------------------------------------------------------
// class DBbox
//-----------------------------------------------------------------------------
void DBbox::overlap(const TP& p)
{
   // p1 is min; p2 is max
   if (_p1.x() > p.x()) _p1._x = p.x();
   if (_p2.x() < p.x()) _p2._x = p.x();
   if (_p1.y() > p.y()) _p1._y = p.y();
   if (_p2.y() < p.y()) _p2._y = p.y();
}

void DBbox::overlap(int4b x, int4b y)
{
   // p1 is min; p2 is max
   if (_p1.x() > x) _p1._x = x;
   if (_p2.x() < x) _p2._x = x;
   if (_p1.y() > y) _p1._y = y;
   if (_p2.y() < y) _p2._y = y;
}

void DBbox::overlap(const DBbox& bx)
{
   if (DEFAULT_OVL_BOX == bx) return;
   if (DEFAULT_OVL_BOX == (*this))
   {
      if (bx.p1().x() > bx.p2().x())
      {
         _p1._x = bx.p2().x(); _p2._x = bx.p1().x();
      }
      else
      {
         _p1._x = bx.p1().x(); _p2._x = bx.p2().x();
      }
      if (bx.p1().y() > bx.p2().y())
      {
         _p1._y = bx.p2().y(); _p2._y = bx.p1().y();
      }
      else
      {
         _p1._y = bx.p1().y(); _p2._y = bx.p2().y();
      }
   }
   else
   {
      overlap(bx.p1()); overlap(bx.p2());
   }
}

DBbox DBbox::overlap(const CTM& op2)  const
{
   DBbox result(_p1 * op2);
   result.overlap(TP(_p2.x(), _p1.y()) * op2);
   result.overlap(  (      _p2       ) * op2);
   result.overlap(TP(_p1.x(), _p2.y()) * op2);
   return result;
}

void DBbox::normalize()
{
   int4b swap;
   if (_p1.x() > _p2.x())
   {
      swap = _p1.x(); _p1._x = _p2.x(); _p2._x = swap;
   }
   if (_p1.y() > _p2.y())
   {
      swap = _p1.y(); _p1._y = _p2.y(); _p2._y = swap;
   }
}

int8b DBbox::cliparea(const DBbox& bx, bool calculate) const
{
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
      case 0x00: Aprim = DEBUG_NEW TP(bx.p1()); break;
      case 0x01: Aprim = DEBUG_NEW TP(_p1.x(), bx.p1().y());break;
      case 0x04: Aprim = DEBUG_NEW TP(bx.p1().x(), _p1.y());break;
      case 0x05: Aprim = DEBUG_NEW TP(_p1);break;
      default: assert(false);break;
   }
   switch (B_place) {
      case 0x00: Bprim = DEBUG_NEW TP(bx.p2());break;
      case 0x02: Bprim = DEBUG_NEW TP(_p2.x(),bx.p2().y());break;
      case 0x08: Bprim = DEBUG_NEW TP(bx.p2().x(),_p2.y());break;
      case 0x0A: Bprim = DEBUG_NEW TP(_p2);break;
      default: assert(false);break;
   }
   int8b area =  llabs(((int8b)Aprim->x() - (int8b)Bprim->x()) *
                       ((int8b)Aprim->y() - (int8b)Bprim->y())   );
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
      case 0x00: Aprim = DEBUG_NEW TP(bx.p1()); break;
      case 0x01: Aprim = DEBUG_NEW TP(_p1.x(), bx.p1().y());break;
      case 0x04: Aprim = DEBUG_NEW TP(bx.p1().x(), _p1.y());break;
      case 0x05: Aprim = DEBUG_NEW TP(_p1);break;
      default: assert(false);break;
   }
   switch (B_place) {
      case 0x00: Bprim = DEBUG_NEW TP(bx.p2());break;
      case 0x02: Bprim = DEBUG_NEW TP(_p2.x(),bx.p2().y());break;
      case 0x08: Bprim = DEBUG_NEW TP(bx.p2().x(),_p2.y());break;
      case 0x0A: Bprim = DEBUG_NEW TP(_p2);break;
      default: assert(false);break;
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

int8b DBbox::boxarea() const
{
   return llabs(((int8b)_p2.x() - (int8b)_p1.x()) *
                ((int8b)_p2.y() - (int8b)_p1.y())   );
}

bool DBbox::visible(const CTM& tmtrx, int8b visualLimit) const
{
   PointVector ptlist;
   ptlist.reserve(4);

   ptlist.push_back(               (_p1) * tmtrx);
   ptlist.push_back(TP(_p2.x(), _p1.y()) * tmtrx);
   ptlist.push_back(               (_p2) * tmtrx);
   ptlist.push_back(TP(_p1.x(), _p2.y()) * tmtrx);

   if (llabs(polyarea(ptlist)) >= visualLimit) return true;
   else                                        return false;
}

DBbox DBbox::getcorner(QuadIdentificators corner)
{
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
     default: assert(false);break;
   }
   return DBbox(TP()); // dummy, to prevent warnings
}

DBbox DBbox::operator * (const CTM& op2) const
{
   TP np1 = _p1 * op2;
   TP np2 = _p2 * op2;
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
// class DBline
//-----------------------------------------------------------------------------
DBline DBline::operator *  (const CTM& op2) const
{
   TP np1(int4b(op2.a() * _p1.x() + op2.c() * _p1.y() + op2.tx()),
          int4b(op2.b() * _p1.x() + op2.d() * _p1.y() + op2.ty()));
   TP np2(int4b(op2.a() * _p2.x() + op2.c() * _p2.y() + op2.tx()),
          int4b(op2.b() * _p2.x() + op2.d() * _p2.y() + op2.ty()));
   return DBline(np1, np2);
}

DBline DBline::operator  =  (const DBline& ln)
{
   _p1 = ln.p1();
   _p2 = ln.p2();
   return *this;
}

//-----------------------------------------------------------------------------
// class PSegment
//-----------------------------------------------------------------------------
PSegment::PSegment(TP p1,TP p2)
{
   _A = p2.y() - p1.y();
   _B = p1.x() - p2.x();
   _C = -(_A*p1.x() + _B*p1.y());
   _angle = 0;
}

byte PSegment::crossP(PSegment seg, TP& crossp)
{
   // segments will coinside if    A1/A2 == B1/B2 == C1/C2
   // segments will be parallel if A1/A2 == B1/B2 != C1/C2
   if (0 == (_A*seg._B - seg._A*_B)) return 1;
   real X,Y;
   if ((0 != _A) && (0 != seg._B)) {
      X = - ((_C - (_B/seg._B) * seg._C) / (_A - (_B/seg._B) * seg._A));
      Y = - ((seg._C - (seg._A/_A) * _C) / (seg._B - (seg._A/_A) * _B));
   }
   else if ((0 != _B) && (0 != seg._A)) {
      X = - (seg._C - (seg._B/_B) * _C) / (seg._A - (seg._B/_B) * _A);
      Y = - (_C - (_A/seg._A) * seg._C) / (_B - (_A/seg._A) * seg._B);
   }
   else assert(0);
   crossp.setX((int4b)rint(X));
   crossp.setY((int4b)rint(Y));
   return 0;
}

PSegment* PSegment::ortho(TP p)
{
   PSegment* seg = DEBUG_NEW PSegment(-_B, _A, _B*p.x() - _A*p.y());
   return seg;
}

PSegment* PSegment::parallel(TP p)
{
   PSegment* seg = DEBUG_NEW PSegment( _A, _B, -_A*p.x() - _B*p.y());
   return seg;
}


//-----------------------------------------------------------------------------
// class SGBitSet
//-----------------------------------------------------------------------------
SGBitSet::SGBitSet(word  bit_length)
{
   _size = bit_length;
   if (0 == _size)
      _packet = NULL;
   else
   {
      word nb = _size / 8;
      _packet = DEBUG_NEW byte[nb+1];
      for (word i = 0; i <= nb; i++) _packet[i] = 0;
   }
}

SGBitSet::SGBitSet(const SGBitSet& bs)
{
   _size = bs.size();
   if (0 == _size)
      _packet = NULL;
   else
   {
      word nb = _size / 8;
      _packet = DEBUG_NEW byte[nb+1];
      for (word i = 0; i <= nb; i++) _packet[i] = bs._packet[i];
   }
}

void SGBitSet::set(word  bit)
{
   assert(bit <= _size);
   _packet[bit / 8] |= (0x01 << (bit % 8));
}

void SGBitSet::setall()
{
   assert(_size);
   for(word i = 0; i < _size / 8; i++)
      _packet[i] = 0xFF;
   _packet[(_size / 8)] = (0xFF >> (8 - (_size % 8)));
}

void SGBitSet::reset(word  bit)
{
   assert(bit <= _size);
   _packet[bit / 8] &= ~(0x01 << (bit % 8));
}

void SGBitSet::check_neighbours_set(bool wire)
{
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

bool SGBitSet::check(word  bit) const
{
   assert(bit <= _size);
   if (0 == _size) return false;
   return (0 != (_packet[bit / 8] & (0x01 << (bit % 8))));
}

bool SGBitSet::isallclear() const
{
   assert(_size);
   for(word i = 0; i <= _size / 8; i++)
      if (0x00 != _packet[i]) return false;
   return true;
}

bool SGBitSet::isallset() const
{
   assert(_size);
   for(word i = 0; i < _size / 8; i++)
      if (0xFF != _packet[i]) return false;
   if (_packet[(_size / 8)] != (byte)(0xFF >> (8 - (_size % 8)))) return false;
   return true;
}

void SGBitSet::swap(word bitA, word bitB)
{
   assert(bitA < _size);
   assert(bitB < _size);
   bool oldbA = check(bitA);
   bool oldbB = check(bitB);
   if (oldbA)   set(bitB);
   else       reset(bitB);
   if (oldbB)   set(bitA);
   else       reset(bitA);
}

void SGBitSet::clear()
{
   _size = 0;
   if (NULL != _packet)
   {
      delete [] _packet;
      _packet = NULL;
   }
}

bool SGBitSet::operator == (const SGBitSet& sop) const
{
   if (_size != sop.size()) return false;
   else
   {
      word nb = _size / 8;
      for (word i = 0; i <= nb; i++)
         if (_packet[i] != sop._packet[i]) return false;
   }
   return true;
}

SGBitSet SGBitSet::operator = (const SGBitSet& sop)
{
   if (NULL != _packet)
      delete [] _packet;
   _size = sop.size();
   if (0 == _size)
      _packet = NULL;
   else
   {
      word nb = _size / 8;
      _packet = DEBUG_NEW byte[nb+1];
      for (word i = 0; i <= nb; i++) _packet[i] = sop._packet[i];
   }
   return *this;
}

SGBitSet::~SGBitSet()
{
   if (NULL != _packet)
      delete [] _packet;
}


//-----------------------------------------------------------------------------
// class CTM
//-----------------------------------------------------------------------------
CTM::CTM(const TP& dp, real scale, real rotation, bool reflX)
{
   Initialize();
   if (reflX)          FlipX();
   if (0 != rotation)  Rotate(rotation);
   if (1 != scale   ) Scale(scale, scale);
   Translate((real) dp.x(),(real) dp.y());
}

CTM::CTM(const TP& bl, const TP& tr)
{
   if ((tr.x() == bl.x()) || (tr.y() == bl.y()))
   {
      Initialize();
   }
   else
   {
      _a = 2.0f / (real)(tr.x() - bl.x());
      _b = _c = 0.0f;
      _d = 2.0f / (real)(tr.y() - bl.y());
      _tx = -((real)(tr.x() + bl.x()) / (real)(tr.x() - bl.x()));
      _ty = -((real)(tr.y() + bl.y()) / (real)(tr.y() - bl.y()));
   }
}

CTM CTM::Translate( const TP& pnt )
{
   return (*this *= CTM(1,0,0,1,pnt.x(),pnt.y()));
}

CTM CTM::Rotate(const real alfa) {// alfa - in degrees
   double temp = (double) alfa;
   double alfaG = (temp * M_PI / 180.0);// translate in radians
   return (*this *= CTM(cos(alfaG),sin(alfaG),-sin(alfaG),cos(alfaG),0,0));
}

CTM CTM::Rotate(const real alfa, const TP& center) // alfa - in degrees
{
   Translate(-center.x(), -center.y());
   Rotate(alfa);
   Translate( center.x(),  center.y());
   return *this;
}

CTM CTM::Rotate(const TP& direction) // angle between X axis and the point (CIF)
{
   real alfa = round(atan2((real)direction.y() , (real)direction.x()) * 180.0 / M_PI);
   Rotate(alfa);
   return *this;
}

CTM CTM::Reversed() const
{
   real denom = (a() * d()) - (b() * c());
   return CTM(                 d()/denom,                   -b()/denom,// 0.0,
                               -c()/denom,                    a()/denom,// 0.0,
              (ty()*c() - tx()*d())/denom , (tx()*b() - ty()*a())/denom );// 1.0 );
}

CTM CTM::operator * (const CTM& op2) const
{
   CTM res;
   res._a  = a()  * op2.a() + b()  * op2.c();
   res._b  = a()  * op2.b() + b()  * op2.d();
   res._c  = c()  * op2.a() + d()  * op2.c();
   res._d  = c()  * op2.b() + d()  * op2.d();
   res._tx = tx() * op2.a() + ty() * op2.c() + op2.tx();
   res._ty = tx() * op2.b() + ty() * op2.d() + op2.ty();
   return res;
}

/*! Operator is used to multiply only the binding point (tx,ty) with a factor
    For example in the case of CIF import. Use with caution making sure that
    the original _tx, _ty were not influenced by the following rotate/scale/flip
    transformations
*/
CTM CTM::operator * (const real factor) const
{
   CTM res(*this);
   res._tx *= factor;
   res._ty *= factor;
   return res;
}

CTM CTM::operator = (const CTM& op2)
{
   _a = op2.a();_b = op2.b();_c = op2.c();_d = op2.d();
   _tx = op2.tx();_ty = op2.ty();
   return *this;
}

void CTM::Decompose(TP& trans, real& rot, real& scale, bool& flipX) const
{
   // Assuming that every CTM can be represented as a result
   // of 4 consecutive operations - flipX, rotate, scale and translate,
   // this function is extracting the scale, translation, rotation and
   // flip values of these operations.
   // Second presumption here is for the scale value returned. It is that
   // scX and scY are always the same
   real scX = sqrt(_a * _a + _c * _c);
//   real scY = sqrt(_b * _b + _d * _d);
   scale = scX;
   // rotation
//   real rot1= atan2(_b , _a);
   rot = round(atan2(_b , _a) * 180.0 / M_PI);
   // if (rot < 0) rot = 180 + abs(rot);
   // flip
   if (fabs(_a * _d) > fabs(_b * _c))
      flipX = ((_a * _d) > 0) ? false : true;
   else
      flipX = ((_b * _c) < 0) ? false : true;
   // translation
   trans.setX(static_cast<int4b>(_tx));
   trans.setY(static_cast<int4b>(_ty));
}

void CTM::oglForm(real* const oglm) const
{
   oglm[ 0] =   _a; oglm[ 1] =   _b; oglm[ 2] = 0.0f; oglm[ 3] = 0.0f;
   oglm[ 4] =   _c; oglm[ 5] =   _d; oglm[ 6] = 0.0f; oglm[ 7] = 0.0f;
   oglm[ 8] = 0.0f; oglm[ 9] = 0.0f; oglm[10] = 1.0f; oglm[11] = 0.0f;
   oglm[12] =  _tx; oglm[13] =  _ty; oglm[14] = 0.0f; oglm[15] = 1.0f;
}

void CTM::oglForm(float* const oglm) const
{
   oglm[ 0] =   _a; oglm[ 1] =   _b; oglm[ 2] = 0.0f; oglm[ 3] = 0.0f;
   oglm[ 4] =   _c; oglm[ 5] =   _d; oglm[ 6] = 0.0f; oglm[ 7] = 0.0f;
   oglm[ 8] = 0.0f; oglm[ 9] = 0.0f; oglm[10] = 1.0f; oglm[11] = 0.0f;
   oglm[12] =  _tx; oglm[13] =  _ty; oglm[14] = 0.0f; oglm[15] = 1.0f;
}

#if WIN32
/*
double round(double x)
{
   double ret;
   int y=int(x);

   if((x-double(y))>=0.5) ret = y+1; else ret = y;
   return ret;
}
*/
//WARNING! This replacement is far from precise. Needs improvement.
// There will be (most likely) differencies between Linux and
// Windows
/*
int8b lround(double x)
{
   int8b ret;
   int8b y=(int8b)x;

   if((x-double(y))>=0.5) ret = y+1; else ret = y;
   return ret;
}
*/
#endif

//Return true, if argument - space or equivalent;
//else return false
//space requires for split function
bool space(char c)
{
   return (0 != isspace(c));
}

//Return false, if argument - space or equivalent;
//else return true
//space requires for split function
bool not_space(char c)
{
   return (0 == isspace(c));
}

bool isDelimit(char c, char d)
{
   return (c==d)? true: false;
}


std::vector<std::string> split (const std::string& str, char delim)
{
   typedef std::string::const_iterator iter;
   std::vector<std::string> ret;

   iter i = str.begin();
   //Ignore leader spaces
   i = std::find_if(i, str.end(), not_space);
   while(i != str.end())
   {

      //Ignore delimiter
      i = std::find_if(i, str.end(), std::bind(std::not_equal_to<char>(), std::placeholders::_1, delim));
      //find out the end of next word

      iter j = std::find_if(i, str.end(), std::bind(std::equal_to<char>() , std::placeholders::_1, delim));

      //Copy character from range [i, j)
      if(i != str.end())
         ret.push_back(std::string(i, j));
   i=j;

   }

   return ret;
}

int8b polyarea(const PointVector& shape)
{
   int8b area = 0ll;
   word size = shape.size();
   word i,j;
   for (i = 0, j = 1; i < size; i++, j = (j+1) % size)
      area += ( (int8b)shape[i].x() * (int8b)shape[j].y() ) -
              ( (int8b)shape[j].x() * (int8b)shape[i].y() )   ;
   return area;
}

//! Greatest common denominator
unsigned GCD(unsigned arg1, unsigned arg2)
{
   if (arg1 < arg2)
   {
      unsigned swap = arg1;
      arg1 = arg2;
      arg2 = swap;
   }
   unsigned remainder = arg1 % arg2;
   if (0 == remainder)
      return arg2;
   else
      return GCD(arg2, remainder);
}

