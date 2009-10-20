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
//           $URL$
//        Created: Sun May 22 15:43:49 BST 2005
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Basic definitions and file handling of TDT data base
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
#include "tedstd.h"
#include "ttt.h"
#include "outbox.h"
#include "tedesign.h"

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
// class TEDfile
//-----------------------------------------------------------------------------
laydata::TEDfile::TEDfile(const char* filename, laydata::tdtlibdir* tedlib)  // reading
{
   _numread = 0;_position = 0;_design = NULL;
   _TEDLIB = tedlib;
	std::string fname(convertString(filename));
	if (NULL == (_file = fopen(fname.c_str(), "rb"))) {
      std::string news = "File \"";
      news += filename; news += "\" not found or unaccessable";
      tell_log(console::MT_ERROR,news);
      _status = false; return;
   }
   try
   {
      getFHeader();
   }
   catch (EXPTNreadTDT)
   {
      fclose(_file);
      _status = false;
      return;
   }
   _status = true;
}

void laydata::TEDfile::getFHeader()
{
   // Get the leading string 
   std::string _leadstr = getString();
   if (TED_LEADSTRING != _leadstr) throw EXPTNreadTDT("Bad leading record");
   // Get format revision
   getRevision();
   // Get file time stamps
   getTime(/*timeCreated, timeSaved*/);
//   checkIntegrity();
}

void laydata::TEDfile::read(int libRef) 
{
   if (tedf_DESIGN != getByte()) throw EXPTNreadTDT("Expecting DESIGN record");
   std::string name = getString();
   real         DBU = getReal();
   real          UU = getReal();
   tell_log(console::MT_DESIGNNAME, name);
   if (libRef > 0)
      _design = DEBUG_NEW tdtlibrary(name, DBU, UU, libRef);
   else
      _design = DEBUG_NEW tdtdesign(name,_created, _lastUpdated, DBU,UU);
   _design->read(this);
   //Design end marker is read already in tdtdesign so don't search it here
   //byte designend = getByte(); 
}

laydata::TEDfile::TEDfile(std::string& filename, laydata::tdtlibdir* tedlib) 
{ //writing
   _design = (*tedlib)();
   _revision=TED_CUR_REVISION;_subrevision=TED_CUR_SUBREVISION;
   _TEDLIB = tedlib;
	std::string fname(convertString(filename));
   if (NULL == (_file = fopen(fname.c_str(), "wb"))) {
      std::string news = "File \""; 
      news += filename.c_str(); news += "\" can not be created";
      tell_log(console::MT_ERROR,news);
      return;
   }
   putString(TED_LEADSTRING);
   putRevision();
   putTime();
   static_cast<laydata::tdtdesign*>(_design)->write(this);
   fclose(_file);
}

void laydata::TEDfile::cleanup() 
{
   if (NULL != _design) delete _design;
}

byte laydata::TEDfile::getByte() 
{
   byte result;
   byte length = sizeof(byte);
   if (1 != (_numread = fread(&result, length, 1, _file)))
      throw EXPTNreadTDT("Wrong number of bytes read");
   _position += length;
   return result;
}

word laydata::TEDfile::getWord() 
{
   word result;
   byte length = sizeof(word);
   if (1 != (_numread = fread(&result, length, 1, _file)))
      throw EXPTNreadTDT("Wrong number of bytes read");
   _position += length;
   return result;
}

int4b laydata::TEDfile::get4b() 
{
   int4b result;
   byte length = sizeof(int4b);
   if (1 != (_numread = fread(&result, length, 1, _file)))
      throw EXPTNreadTDT("Wrong number of bytes read");
   _position += length;
   return result;
}

real laydata::TEDfile::getReal() {
   real result;
   byte length = sizeof(real);
   if (1 != (_numread = fread(&result, length, 1, _file)))
      throw EXPTNreadTDT("Wrong number of bytes read");
   _position += length;
   return result;
}

std::string laydata::TEDfile::getString() 
{
   std::string str;
   byte length = getByte();
   char* strc = DEBUG_NEW char[length+1];
   _numread = fread(strc, length, 1, _file);
   strc[length] = 0x00;
   if (_numread != 1) 
   {
      delete[] strc;
      throw EXPTNreadTDT("Wrong number of bytes read");
   }
   _position += length; str = strc;
   delete[] strc;
   return str;
}

TP laydata::TEDfile::getTP() 
{
   int4b x = get4b();
   int4b y = get4b();
   return TP(x,y);
}

CTM laydata::TEDfile::getCTM() 
{
   real _a  = getReal();
   real _b  = getReal();
   real _c  = getReal();
   real _d  = getReal();
   real _tx = getReal();
   real _ty = getReal();
   return CTM(_a, _b, _c, _d, _tx, _ty);
}

void laydata::TEDfile::getTime()
{
   tm broken_time;
   if (tedf_TIMECREATED  != getByte()) throw EXPTNreadTDT("Expecting TIMECREATED record");
   broken_time.tm_mday = get4b();
   broken_time.tm_mon  = get4b();
   broken_time.tm_year = get4b();
   broken_time.tm_hour = get4b();
   broken_time.tm_min  = get4b();
   broken_time.tm_sec  = get4b();
   broken_time.tm_isdst = -1;
   _created = mktime(&broken_time);
   if (tedf_TIMEUPDATED  != getByte()) throw EXPTNreadTDT("Expecting TIMEUPDATED record");
   broken_time.tm_mday = get4b();
   broken_time.tm_mon  = get4b();
   broken_time.tm_year = get4b();
   broken_time.tm_hour = get4b();
   broken_time.tm_min  = get4b();
   broken_time.tm_sec  = get4b();
   broken_time.tm_isdst = -1;
   _lastUpdated = mktime(&broken_time);
}

void laydata::TEDfile::getRevision()
{
   if (tedf_REVISION  != getByte()) throw EXPTNreadTDT("Expecting REVISION record");
   _revision = getWord();
   _subrevision = getWord();
   std::ostringstream ost; 
   ost << "TDT format revision: " << _revision << "." << _subrevision;
   tell_log(console::MT_INFO,ost.str());
   if ((_revision != TED_CUR_REVISION) || (_subrevision > TED_CUR_SUBREVISION))
      throw EXPTNreadTDT("The TDT revision is not maintained by this version of Toped");
}

void laydata::TEDfile::putWord(const word data) {
   fwrite(&data,2,1,_file);
}

void laydata::TEDfile::put4b(const int4b data) {
   fwrite(&data,4,1,_file);
}

void laydata::TEDfile::putReal(const real data) {
   fwrite(&data, sizeof(real), 1, _file);
}


void laydata::TEDfile::putTime() 
{
   time_t ctime = static_cast<laydata::tdtdesign*>(_design)->created();
   tm* broken_time = localtime(&ctime);
   putByte(tedf_TIMECREATED);
   put4b(broken_time->tm_mday);
   put4b(broken_time->tm_mon);
   put4b(broken_time->tm_year);
   put4b(broken_time->tm_hour);
   put4b(broken_time->tm_min);
   put4b(broken_time->tm_sec);
   //
   _lastUpdated = time(NULL);
   static_cast<laydata::tdtdesign*>(_design)->_lastUpdated = _lastUpdated;
   broken_time = localtime(&_lastUpdated);
   putByte(tedf_TIMEUPDATED);
   put4b(broken_time->tm_mday);
   put4b(broken_time->tm_mon);
   put4b(broken_time->tm_year);
   put4b(broken_time->tm_hour);
   put4b(broken_time->tm_min);
   put4b(broken_time->tm_sec);
}

void laydata::TEDfile::putRevision() 
{
   putByte(tedf_REVISION);
   putWord(_revision);
   putWord(_subrevision);
}

void laydata::TEDfile::putTP(const TP* p)
{
   put4b(p->x()); put4b(p->y());
}

void laydata::TEDfile::putCTM(const CTM matrix)
{
   putReal(matrix.a());
   putReal(matrix.b());
   putReal(matrix.c());
   putReal(matrix.d());
   putReal(matrix.tx());
   putReal(matrix.ty());
}

void laydata::TEDfile::putString(std::string str)
{
//   byte len = str.length();
//   fwrite(&len, 1,1, _file);
   putByte(str.length());
   fputs(str.c_str(), _file);
}

void laydata::TEDfile::registercellwritten(std::string cellname)
{
   _childnames.insert(cellname);
}

bool laydata::TEDfile::checkcellwritten(std::string cellname)
{
   if (_childnames.end() == _childnames.find(cellname))
      return false;
   else
      return true;
}

laydata::CellDefin laydata::TEDfile::linkcellref(std::string cellname)
{
   // register the name of the referenced cell in the list of children
   _childnames.insert(cellname);
   cellList::const_iterator striter = _design->_cells.find(cellname);
   laydata::CellDefin celldef = NULL;
   // link the cells instances with their definitions
   if (_design->_cells.end() == striter) 
   {
   //   if (_design->checkcell(name))
   //   {
      // search the cell in the libraries because it's not in the DB
      if (!_TEDLIB->getLibCellRNP(cellname, celldef))
      {
         // Attention! In this case we've parsed a cell reference, before
         // the cell is defined. This might means:
         //   1. Cell is referenced, but not defined - i.e. library cell, but 
         //      library is not loaded
         //   2. Circular reference ! Cell1 contains a reference of Cell2,
         //      that in turn contains a reference of Cell1. This is not allowed
         // We can not make a decision yet, because the entire file has not been
         // parsed yet. That is why we are assigning a default cell to the 
         // referenced structure here in order to continue the parsing, and when 
         // the entire file is parced the cell references without a proper pointer
         // to the structure need to be flaged as warning in case 1 and as error
         // in case 2.
         celldef = _TEDLIB->adddefaultcell(cellname, false);
      }
      else
         celldef->parentfound();
   }
   else 
   {
      celldef = striter->second;
      assert(NULL != celldef);
      celldef->parentfound();
   }
   return celldef;
}

void laydata::TEDfile::get_cellchildnames(NameSet& cnames) {
   // Be very very careful with the copy constructors and assignment of the
   // standard C++ lib containers. Here it seems OK.
   cnames = _childnames;
   //for (NameSet::const_iterator CN = _childnames.begin();
   //                              CN != _childnames.end() ; CN++)
   //   cnames->instert(*CN);
   _childnames.clear();
}
