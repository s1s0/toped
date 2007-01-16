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

#include <math.h>
#include <sstream>
#include "tedstd.h"
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"
#include "tedesign.h"

//-----------------------------------------------------------------------------
// class PSegment
//-----------------------------------------------------------------------------
PSegment::PSegment(TP p1,TP p2) {
   _A = p2.y() - p1.y();
   _B = p1.x() - p2.x();
   _C = -(_A*p1.x() + _B*p1.y());
}

byte PSegment::crossP(PSegment seg, TP& crossp) {
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
   
PSegment& PSegment::ortho(TP p) {
   PSegment* seg = new PSegment(-_B, _A, _B*p.x() - _A*p.y());
   return *seg;
}
//-----------------------------------------------------------------------------
// class TEDfile
//-----------------------------------------------------------------------------
laydata::TEDfile::TEDfile(const char* filename) { // reading
   _numread = 0;_position = 0;_design = NULL;
   if (NULL == (_file = fopen(filename, "rb"))) {
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

void laydata::TEDfile::read() {
   if (tedf_DESIGN != getByte()) throw EXPTNreadTDT("Expecting DESIGN record");
   std::string name = getString();
   real         DBU = getReal();
   real          UU = getReal();
   tell_log(console::MT_DESIGNNAME, name);
   _design = new tdtdesign(name,_created, _lastUpdated, DBU,UU);
   _design->read(this);
   //Design end marker is read already in tdtdesign so don't search it here
   //byte designend = getByte(); 
}

laydata::TEDfile::TEDfile(tdtdesign* design, std::string& filename) { //writing
   _design = design;_revision=0;_subrevision=6;
   if (NULL == (_file = fopen(filename.c_str(), "wb"))) {
      std::string news = "File \"";
      news += filename.c_str(); news += "\" can not be created";
      tell_log(console::MT_ERROR,news);
      return;
   }   
   putString(TED_LEADSTRING);
   putRevision();
   putTime();
   _design->write(this);
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
   char* strc = new char[length+1];
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
   time_t ctime = _design->created();
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
   _design->_lastUpdated = _lastUpdated;
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

void laydata::TEDfile::putTP(const TP* p) {
   put4b(p->x()); put4b(p->y());
}

void laydata::TEDfile::putCTM(const CTM matrix) {
   putReal(matrix.a());
   putReal(matrix.b());
   putReal(matrix.c());
   putReal(matrix.d());
   putReal(matrix.tx());
   putReal(matrix.ty());
}

void laydata::TEDfile::putString(std::string str) {
//   byte len = str.length();
//   fwrite(&len, 1,1, _file);
   putByte(str.length());
   fputs(str.c_str(), _file);
}

void laydata::TEDfile::registercellread(std::string cellname, tdtcell* cell) {
   if (_design->_cells.end() != _design->_cells.find(cellname)) 
   // There are several possiblirities here:
   // 1. Cell has been referenced before the definition takes place
   // 2. The same case 1, but the reason is circular reference. 
   // 3. Cell is defined more than once
   // Case 3 seems to be just theoretical and we should abort the reading 
   // and retun with error in this case.
   // Case 2 is really dangerous and once again theoretically we need to 
   // break the circularity. This might happen however once the whole file
   // is parced
   // Case 1 is quite OK, although, the write sequence should use the 
   // cell structure tree and start the writing from the leaf cells. At the 
   // moment writing is in kind of alphabetical order and case 1 is more
   // than possible. In the future it might me appropriate to issue a warning
   // for possible circular reference.
      if (NULL == _design->_cells[cellname]) {
         // case 1 or case 2 -> can't be distiguised in this moment
         //_design->_cells[cellname] = cell;
         // call has been referenced already, so it's not an orphan
         cell->parentfound();
      }
      else {
         // case 3 -> parsing should be stopped !
      }
   _design->_cells[cellname] = cell;
}

void laydata::TEDfile::registercellwritten(std::string cellname) {
   _childnames.push_back(cellname);
}   

bool laydata::TEDfile::checkcellwritten(std::string cellname) {
   for (nameList::const_iterator i = _childnames.begin();
                                 i != _childnames.end(); i++)
      if (cellname == *i) return true;
   return false;      
//   return (_childnames.end() != _childnames.find(cellname));
}   

laydata::refnamepair laydata::TEDfile::getcellinstance(std::string cellname) 
{
   // register the name of the referenced cell in the list of children
   _childnames.push_back(cellname);
   // link the cells instances with their definitions
   if (_design->_cells.end() == _design->_cells.find(cellname)) 
   {
   // Attention! In this case we've parsed a cell reference, before
   // the cell is defined. This might means:
   //   1. Cell is referenced, but simply not defined. 
   //   2. Circular reference ! Cell1 contains a reference of Cell2,
   //      that in turn contains a reference of Cell1. This is not allowed
   // We can not make a decision yet, because the entire file has not been
   // parsed yet. That is why we are assigning a NULL pointer to the 
   // referenced structure here in order to continue the parsing, and when 
   // the entire file is parced the cell references without a proper pointer
   // to the structure need to be flagged as warning in case 1 and as error 
   // in case 2.
   // Empty cell ctructure should be handled without a problem by the 
   // tdttcellref class and its ancestors
      _design->_cells[cellname] = NULL;
   }   
   else 
   {
      // Mark that the cell definition is referenced, i.e. it is not the top 
      // of the tree (orphan flag in the tdtcell), BUT just in case it is
      // not empty yet
      _design->_cells[cellname]->parentfound();
   }
   return _design->_cells.find(cellname);
}

void laydata::TEDfile::get_cellchildnames(nameList* cnames) {
   // leave only the unique names in the children's list
   _childnames.sort(); _childnames.unique();
   // Be very very careful with the copy constructors and assignment of the 
   // standard C++ lib containers. Here it seems OK.
   *cnames = _childnames;
   //for (nameList::const_iterator CN = _childnames.begin(); 
   //                              CN != _childnames.end() ; CN++)
   //   cnames->push_back(*CN);
   _childnames.clear();
}
