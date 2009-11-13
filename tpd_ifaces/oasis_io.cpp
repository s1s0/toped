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
//        Created: October 10 2009
//      Copyright: (C) 2001-2009 Svilen Krustev - skr@toped.org.uk
//    Description: OASIS parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#include "tpdph.h"
#include "oasis_io.h"
#include "tedesign.h"
#include <sstream>


//===========================================================================
Oasis::Table::Table(OasisInFile& ofh)
{
   _strictMode   = ofh.getUnsignedInt(1);
   _offsetStart  = ofh.getUnsignedInt(8);
   _offsetEnd    = 0;
   _ieMode       = tblm_unknown;
   _nextIndex    = 0;
   _index        = 0;
}

void Oasis::Table::getCellNameTable(OasisInFile& ofh)
{
   if (0 != _offsetStart)
   {
      wxFileOffset savedPos = ofh.filePos();
      ofh.setPosition(_offsetStart);
      ofh.setPropContext(pc_cell);
      byte recType;
      do
      {
         recType = ofh.getUnsignedInt(1);
         switch(recType)
         {
            case oas_PROPERTY_1 : ofh.getProperty1(); break;
            case oas_PROPERTY_2 : ofh.getProperty2(); break;
            case oas_CELLNAME_1 : getTableRecord(ofh, tblm_implicit, true); break;
            case oas_CELLNAME_2 : getTableRecord(ofh, tblm_explicit, true); break;
            default : _offsetEnd = ofh.filePos() - 1; ofh.setPosition(savedPos); return;
         }
      } while (true);
   }
}

void Oasis::Table::getPropNameTable(OasisInFile& ofh)
{
   if (0 != _offsetStart)
   {
      wxFileOffset savedPos = ofh.filePos();
      ofh.setPosition(_offsetStart);
      byte recType;
      do
      {
         recType = ofh.getUnsignedInt(1);
         switch(recType)
         {
            case oas_PROPNAME_1 : getTableRecord(ofh, tblm_implicit, true); break;
            case oas_PROPNAME_2 : getTableRecord(ofh, tblm_explicit, true); break;
            default : _offsetEnd = ofh.filePos() - 1; ofh.setPosition(savedPos); return;
         }
      } while (true);
   }
}

void Oasis::Table::getPropStringTable(OasisInFile& ofh)
{
   if (0 != _offsetStart)
   {
      wxFileOffset savedPos = ofh.filePos();
      ofh.setPosition(_offsetStart);
      byte recType;
      do
      {
         recType = ofh.getUnsignedInt(1);
         switch(recType)
         {
            case oas_PROPSTRING_1 : getTableRecord(ofh, tblm_implicit, true); break;
            case oas_PROPSTRING_2 : getTableRecord(ofh, tblm_explicit, true); break;
            default : _offsetEnd = ofh.filePos() - 1; ofh.setPosition(savedPos); return;
         }
      } while (true);
   }
}

void Oasis::Table::getTextStringTable(OasisInFile& ofh)
{
   if (0 != _offsetStart)
   {
      wxFileOffset savedPos = ofh.filePos();
      ofh.setPosition(_offsetStart);
      byte recType;
      do
      {
         recType = ofh.getUnsignedInt(1);
         switch(recType)
         {
            case oas_TEXTSTRING_1 : getTableRecord(ofh, tblm_implicit, true); break;
            case oas_TEXTSTRING_2 : getTableRecord(ofh, tblm_explicit, true); break;
            default : _offsetEnd = ofh.filePos() - 1; ofh.setPosition(savedPos); return;
         }
      } while (true);
   }
}

/*! Reads a single NAME record and adds it to the corresponding name table*/
void  Oasis::Table::getTableRecord(OasisInFile& ofn, TableMode ieMode, bool tableRec)
{
   // check whether we've stepped into the Table space without really trying to read
   // the Table contents (it has already been parsed)
   if (!tableRec && (ofn.filePos() >= _offsetStart) && (ofn.filePos() <= _offsetEnd))
   {
      // move the current file pointer at the end of the table
      ofn.setPosition(_offsetEnd); return;
   }
   // now check for stray records in strict mode
   if (!tableRec && _strictMode)
      ofn.exception("A stray \"NAME\" record encountered in strict mode (13.10)");
   if (tblm_unknown == _ieMode)
      _ieMode = ieMode;
   else if (_ieMode != ieMode)
      ofn.exception("Uncompatible record types encountered in \"NAME\" records (15.5,16.4,17.4,18.4)");
   std::string value = ofn.getString();
   switch (_ieMode)
   {
      case tblm_implicit: _index = _nextIndex++; break;
      case tblm_explicit: _index = ofn.getUnsignedInt(4); break;
      default: assert(false);
   }
   if (_table.end() != _table.find(_index))
      ofn.exception("Name record with this index already exists (15.5,16.4,17.4,18.4)");
   else
      _table[_index] = value;
}

std::string Oasis::Table::getName(dword index) const
{
   NameTable::const_iterator record;
   if (_table.end() == (record = _table.find(index)))
      throw EXPTNreadOASIS("Name not found in the corresponding table (20.4,...)");
   else
      return record->second;
}

//===========================================================================
Oasis::OasisInFile::OasisInFile(std::string fn) : _cellNames(NULL), _textStrings(NULL), 
   _propNames(NULL), _propStrings(NULL), _layerNames(NULL), _xNames(NULL), _offsetFlag(false),
   _fileName(fn), _fileLength(0), _filePos(0), _progresPos(0)
{
   std::ostringstream info;
   info << "OASIS input file: \"" << _fileName << "\"";
   tell_log(console::MT_INFO, info.str());
   info.str("");
   wxString wxfname(_fileName.c_str(), wxConvUTF8 );
   _oasisFh.Open(wxfname,wxT("rb"));
   if (!(_oasisFh.IsOpened()))
   {// open the input file
      info << "File "<< _fileName <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      _status = false;
   }
   else
   {
      byte magicBytes[13];
      word numread = _oasisFh.Read(&magicBytes, 13);
      if (13 == numread)
      {
         _filePos = 13;
         _status = true;
         for(byte strindex = 0; strindex < 13; strindex++)
            if (magicBytes[strindex] != Oasis::oas_MagicBytes[strindex])
            { 
               _status = false; break;
            }
      }
      else
      {
         _status = false;
         info << "File "<< _fileName <<" doesn't appear to be a valid OASIS file";
         tell_log(console::MT_ERROR,info.str());
      }
   }
   if (_status)
   {
      // initialise the progress bar
      _fileLength = _oasisFh.Length();
      TpdPost::toped_status(console::TSTS_PRGRSBARON, _fileLength);
   }
}

bool Oasis::OasisInFile::reopenFile()
{
   _filePos = 0;
   _progresPos = 0;
   wxString wxfname(_fileName.c_str(), wxConvUTF8 );
   _oasisFh.Open(wxfname.c_str(),wxT("rb"));
   if (!(_oasisFh.IsOpened()))
   {// open the input file
      std::ostringstream info;
      info << "File "<< _fileName <<" can NOT be reopened";
      tell_log(console::MT_ERROR,info.str());
      return false;
   }
   return true;
}

void Oasis::OasisInFile::setPosition(wxFileOffset filePos)
{
   _oasisFh.Seek(filePos, wxFromStart);
   _filePos = filePos;
}

void Oasis::OasisInFile::closeFile()
{
   if (_status)
      _oasisFh.Close();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Oasis::OasisInFile::readStartRecord()
{
   std::ostringstream info;
   _version = getString();
   info << "OASIS version: \"" << _version << "\"";
   tell_log(console::MT_INFO, info.str());
   _unit = getReal();
   if (0 > _unit) exception("Unacceptable \"unit\" value (13.10)");
   _offsetFlag = getUnsignedInt(1);
   if (!_offsetFlag)
   {
      // table offset structure is stored in the START record (here)
      _cellNames   = DEBUG_NEW Table(*this);
      _textStrings = DEBUG_NEW Table(*this);
      _propNames   = DEBUG_NEW Table(*this);
      _propStrings = DEBUG_NEW Table(*this);
      _layerNames  = DEBUG_NEW Table(*this);
      _xNames      = DEBUG_NEW Table(*this);
   }
   else
   {
      // table offset structure is stored in the END record
      wxFileOffset savedPos = _filePos;
      setPosition(_fileLength - 255);
      _cellNames   = DEBUG_NEW Table(*this);
      _textStrings = DEBUG_NEW Table(*this);
      _propNames   = DEBUG_NEW Table(*this);
      _propStrings = DEBUG_NEW Table(*this);
      _layerNames  = DEBUG_NEW Table(*this);
      _xNames      = DEBUG_NEW Table(*this);
      setPosition(savedPos);
   }
   _propNames->getPropNameTable(*this);
   _propStrings->getPropStringTable(*this);
   _cellNames->getCellNameTable(*this);
   _textStrings->getTextStringTable(*this);
}

void Oasis::OasisInFile::readEndRecord()
{
   if (_offsetFlag)
   {
      // table offset structure is stored in the END record (here)
      // skim them over. They should've been already read by
      // readStartRecord()
      for (byte curec = 0; curec < 6; curec++)
      {
         getUnsignedInt(1);
         getUnsignedInt(8);
      }
   }
   getString(); // <-- padding string
   std::ostringstream info;
   byte valid_scheme = getByte();
   if (2 < valid_scheme)
      exception("Unexpected validation scheme type ( not explicitly specified)");
   else if (0 == valid_scheme)
      info << "OASIS file has no validation signature";
   else
   {
      dword signature;
      byte * sigbyte = (byte*) &signature;
      sigbyte[3] = getByte();
      sigbyte[2] = getByte();
      sigbyte[1] = getByte();
      sigbyte[0] = getByte();
      if (1 == valid_scheme)
         info << "OASIS file: CRC32 validation signature is "<< signature;
      else
         info << "OASIS file: CHECKSUM32 validation signature" << signature;
   }
   tell_log(console::MT_INFO, info.str());
}

void Oasis::OasisInFile::readLibrary()
{
   // get oas_START
   byte recType = getUnsignedInt(1);
   if (oas_START != recType) exception("\"START\" record expected here (13.10)");
   readStartRecord();
   // Some sequences (CELL) does not have an explicit end record. So they end when a record
   // which can not be a member of the definition appears. The trouble is that the last byte
   // read from ine input stream should be reused in the loop here
   bool rlb = false;// reuse last byte
   Cell* curCell = NULL;
   do {
      if (!rlb) recType = getUnsignedInt(1);
      switch (recType)
      {  
         case oas_PAD         : rlb = false; break;
         case oas_PROPERTY_1  : assert(false);/*@TODO*/ rlb = false; break;
         case oas_PROPERTY_2  : assert(false);/*@TODO*/ rlb = false; break;
         case oas_CELL_1      : 
            curCell = DEBUG_NEW Cell(); 
            recType = curCell->skimCell(*this, true) ; 
            rlb = true;
            _definedCells[curCell->name()] = curCell;
            break;
         case oas_CELL_2      : 
            curCell = DEBUG_NEW Cell(); 
            recType = curCell->skimCell(*this, false); 
            rlb = true; 
            _definedCells[curCell->name()] = curCell;
            break;
         case oas_CBLOCK      : assert(false);/*@TODO*/rlb = false; break;
         // <name> records
         case oas_CELLNAME_1  : _cellNames->getTableRecord(*this, tblm_implicit)   ; rlb = false; break;
         case oas_TEXTSTRING_1: _textStrings->getTableRecord(*this, tblm_implicit) ; rlb = false; break;
         case oas_PROPNAME_1  : _propNames->getTableRecord(*this, tblm_implicit)   ; rlb = false; break;
         case oas_LAYERNAME_1 : _layerNames->getTableRecord(*this, tblm_implicit)  ; rlb = false; break;
         case oas_PROPSTRING_1: _propStrings->getTableRecord(*this, tblm_implicit) ; rlb = false; break;
         case oas_CELLNAME_2  : _cellNames->getTableRecord(*this, tblm_explicit)   ; rlb = false; break;
         case oas_TEXTSTRING_2: _textStrings->getTableRecord(*this, tblm_explicit) ; rlb = false; break;
         case oas_PROPNAME_2  : _propNames->getTableRecord(*this, tblm_explicit)   ; rlb = false; break;
         case oas_LAYERNAME_2 : _layerNames->getTableRecord(*this, tblm_explicit)  ; rlb = false; break;
         case oas_PROPSTRING_2: _propStrings->getTableRecord(*this, tblm_explicit) ; rlb = false; break;
         case oas_XNAME_1     : assert(false);/*@TODO*/ rlb = false; break;
         case oas_XNAME_2     : assert(false);/*@TODO*/ rlb = false; break;
         case oas_END         : 
            readEndRecord();
            closeFile();
            TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
            linkReferences();
            return;
         default: exception("Unexpected record in the current context");
      }
   } while (true);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
byte Oasis::OasisInFile::getByte()
{
   byte        bytein            ; // last byte read from the file stream
   if (1 != _oasisFh.Read(&bytein,1))
      exception("I/O error during read-in");
   else _filePos++;
   if (2048 < (_filePos - _progresPos))
   {
      _progresPos = _filePos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresPos);
   }
   return bytein;
}

float Oasis::OasisInFile::getFloat()
{
   float        floatin          ; // last 4 bytes read from the file stream
   if (4 != _oasisFh.Read(&floatin,4))
      exception("I/O error during read-in");
   else _filePos += 4;
   if (2048 < (_filePos - _progresPos))
   {
      _progresPos = _filePos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresPos);
   }
   return floatin;
}

double Oasis::OasisInFile::getDouble()
{
   double        doublein         ; // last 8 bytes read from the file stream
   if (8 != _oasisFh.Read(&doublein,8))
      exception("I/O error during read-in");
   else _filePos += 8;
   if (2048 < (_filePos - _progresPos))
   {
      _progresPos = _filePos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresPos);
   }
   return doublein;
}

//------------------------------------------------------------------------------
std::string Oasis::OasisInFile::getString(/*Oasis string type check*/)
{
   dword   length  = getUnsignedInt(2) ; //string length
   char* theString = DEBUG_NEW char[length+1];

   if (length != _oasisFh.Read(theString,length))
      exception("I/O error during read-in");
   else _filePos += length;
   theString[length] = 0x00;
   std::string result(theString);
   delete [] theString;
   if (2048 < (_filePos - _progresPos))
   {
      _progresPos = _filePos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresPos);
   }
   return result;
}

//------------------------------------------------------------------------------
qword Oasis::OasisInFile::getUnsignedInt(byte length)
{
   assert((length > 0) && (length < 9));
   byte        cmask       = 0x7f; // masks the MSB of the byte
   byte        bytecounter = 0   ; // how many bytes were read
   byte        bytein            ; // last byte read from the file stream
   qword       result      = 0   ; // the result
   // the result in array of bytes representation
   byte       *btres       = (byte*)&result; 
   do
   {
      bytein = getByte();
      switch (bytecounter)
      {
         case 0: btres[0]  = bytein & cmask; 
                 break;
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7: btres[bytecounter-1] |= bytein << (8-bytecounter);
                 btres[bytecounter  ]  = (bytein & cmask) >> bytecounter;
                 break;
         default: exception("Integer is too big (7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
   if (bytecounter > length)
      exception("Unsigned integer with unexpected length(7.2.3)");
   return result;
}

int8b Oasis::OasisInFile::getInt(byte length)
{
   assert((length > 0) && (length < 9));
   const byte  cmask       = 0x7f; // masks the MSB of the byte
   const byte  smask       = 0x01; // highlights the sign
   byte        bytecounter = 0   ; // how many bytes were read
   byte        bytein            ; // last byte read from the file stream
   byte        sign              ;
   int8b       result      = 0   ; // the result
   // the result in array of bytes representation
   byte       *btres       = (byte*)&result; 
   do
   {
      bytein = getByte();
      switch (bytecounter)
      {
         case 0: btres[0]  = (bytein & cmask) >> 1;
                 sign      =  bytein & smask;
                 break;
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6: btres[bytecounter-1] |= bytein << (7-bytecounter);
                 btres[bytecounter  ]  = (bytein & cmask) >> (bytecounter + 1);
                 break;
         case 7: btres[bytecounter-1] |= bytein;
         default: exception("Integer is too big (7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
   if (bytecounter > length)
      exception("Unsigned integer with unexpected length(7.2.3)");
   if (sign) return -result;
   else      return  result;
}
//------------------------------------------------------------------------------
real Oasis::OasisInFile::getReal(char type)
{
   dword      numerator   = 0;
   dword      denominator = 1;
   bool       sign        = false;
   byte       realType    = 8;
   if ( 0 > type)
      realType = getUnsignedInt(1);
   else
      realType = type;
   switch (realType)
   {
      case 0: numerator   = getUnsignedInt(4); break;
      case 1: numerator   = getUnsignedInt(4); sign = true; break;
      case 2: denominator = getUnsignedInt(4); break;
      case 3: denominator = getUnsignedInt(4); sign = true; break;
      case 4: numerator   = getUnsignedInt(4); denominator = getUnsignedInt(4); break;
      case 5: numerator   = getUnsignedInt(4); denominator = getUnsignedInt(4); sign = true; break;
      case 6: return getFloat();
      case 7: return getDouble();
      default: exception("Unexpected \"real\" type.(7.3.3)");
   }
   if (0 == denominator) exception("Denominator is 0 in \"real\" representation (7.3.3)");
   real result = (sign) ? - ((real) numerator / (real) denominator) :
                            ((real) numerator / (real) denominator)  ;
   return result;
}

//------------------------------------------------------------------------------
std::string Oasis::OasisInFile::getTextRefName(bool ref)
{
   if (ref)
   {
      dword refnum = getUnsignedInt(4);
      return _textStrings->getName(refnum);
   }
   else return getString();
}

std::string Oasis::OasisInFile::getCellRefName(bool ref)
{
   if (ref)
   {
      dword refnum = getUnsignedInt(4);
      return _cellNames->getName(refnum);
   }
   else return getString();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Oasis::OasisInFile::exception(std::string message)
{
   std::ostringstream info;
   info << message << " @ position " << _filePos;
   throw EXPTNreadOASIS(info.str());
}

void Oasis::OasisInFile::linkReferences()
{
   for (DefinitionMap::const_iterator CSTR = _definedCells.begin(); CSTR != _definedCells.end(); CSTR++)
   {//for every structure
      CSTR->second->linkReferences(*this);
   }
}

Oasis::Cell* Oasis::OasisInFile::getCell(const std::string selection)
{
   DefinitionMap::iterator striter;
   if (_definedCells.end() != (striter = _definedCells.find(selection)))
      return striter->second;
   else
      return NULL;
}

void Oasis::OasisInFile::hierOut()
{
   _hierTree = NULL;
   for (DefinitionMap::const_iterator CSTR = _definedCells.begin(); CSTR != _definedCells.end(); CSTR++)
      if (!CSTR->second->haveParent())
         _hierTree = CSTR->second->hierOut(_hierTree, NULL);
}

Oasis::OasisInFile::~OasisInFile()
{
   if ( _cellNames  ) delete _cellNames;
   if ( _textStrings) delete _textStrings;
   if ( _propNames  ) delete _propNames;
   if ( _propStrings) delete _propStrings;
   if ( _layerNames ) delete _layerNames;
   if ( _xNames     ) delete _xNames;
   closeFile();
}


//==============================================================================
Oasis::Cell::Cell()
{
   // See spec chap.10 (page 11). The modal variables below have to be 
   // initialised to 0
   _mod_gx = 0;
   _mod_gy = 0;
   _haveParent = false;
   _traversed  = false;
}

byte Oasis::Cell::skimCell(OasisInFile& ofn, bool refnum)
{
   _name = ofn.getCellRefName(refnum);
   _filePos = ofn.filePos();
   std::ostringstream info;
   info << "OASIS : Reading cell \"" << _name << "\"";
   tell_log(console::MT_INFO, info.str());
   ofn.setPropContext(pc_cell);
   do
   {
      byte recType = ofn.getUnsignedInt(1);
      switch (recType)
      {
         case oas_PAD         : break;
         case oas_PROPERTY_1  : ofn.getProperty1();break;
         case oas_PROPERTY_2  : ofn.getProperty2();break;
         case oas_XYRELATIVE  : /*@TODO*/assert(false);break;
         case oas_XYABSOLUTE  : /*@TODO*/assert(false);break;
         case oas_CBLOCK      : /*@TODO*/assert(false);break;
         // <element> records
         case oas_PLACEMENT_1 : skimReference(ofn, false);break;
         case oas_PLACEMENT_2 : skimReference(ofn, true );break;
         case oas_TEXT        : skimText(ofn);break;
         case oas_XELEMENT    : /*@TODO*/assert(false);break;
         // <geometry> records
         case oas_RECTANGLE   : skimRectangle(ofn); break;
         case oas_POLYGON     : skimPolygon(ofn);break;
         case oas_PATH        : skimPath(ofn);break;
         case oas_TRAPEZOID_1 : /*@TODO*/assert(false);break;
         case oas_TRAPEZOID_2 : /*@TODO*/assert(false);break;
         case oas_TRAPEZOID_3 : /*@TODO*/assert(false);break;
         case oas_CTRAPEZOID  : /*@TODO*/assert(false);break;
         case oas_CIRCLE      : /*@TODO*/assert(false);break;
         default:
            // last byte from the stream doesn't belong to this cell definition
            _cellSize = ofn.filePos() - _filePos - 1;
            return recType;
      }
   } while (true);
}

void Oasis::Cell::import(OasisInFile& ofn, laydata::tdtcell* dst_cell,
                           laydata::tdtlibdir* tdt_db/*, const LayerMapGds&*/)
{
   ofn.setPosition(_filePos);
   std::ostringstream info;
   info << "OASIS : Importing cell \"" << _name << "\"";
   tell_log(console::MT_INFO, info.str());
   ofn.setPropContext(pc_cell);
   do
   {
      byte recType = ofn.getUnsignedInt(1);
      switch (recType)
      {
         case oas_PAD         : break;
         case oas_PROPERTY_1  : ofn.getProperty1();break;
         case oas_PROPERTY_2  : ofn.getProperty2();break;
         case oas_XYRELATIVE  : /*@TODO*/assert(false);break;
         case oas_XYABSOLUTE  : /*@TODO*/assert(false);break;
         case oas_CBLOCK      : /*@TODO*/assert(false);break;
         // <element> records
         case oas_PLACEMENT_1 : readReference(ofn, dst_cell, tdt_db, false);break;
         case oas_PLACEMENT_2 : readReference(ofn, dst_cell, tdt_db, true );break;
         case oas_TEXT        : readText(ofn);break;
         case oas_XELEMENT    : /*@TODO*/assert(false);break;
         // <geometry> records
         case oas_RECTANGLE   : readRectangle(ofn, dst_cell); break;
         case oas_POLYGON     : readPolygon(ofn, dst_cell);break;
         case oas_PATH        : readPath(ofn, dst_cell);break;
         case oas_TRAPEZOID_1 : /*@TODO*/assert(false);break;
         case oas_TRAPEZOID_2 : /*@TODO*/assert(false);break;
         case oas_TRAPEZOID_3 : /*@TODO*/assert(false);break;
         case oas_CTRAPEZOID  : /*@TODO*/assert(false);break;
         case oas_CIRCLE      : /*@TODO*/assert(false);break;
         default:
            // check that the cell size is the same as obtained by skim function
            assert(_cellSize == (ofn.filePos() - _filePos - 1));
            dst_cell->resort();
            return;
      }
   } while (true);
}
//------------------------------------------------------------------------------
void Oasis::Cell::readRectangle(OasisInFile& ofn, laydata::tdtcell* dst_cell)
{
   const byte Smask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   if ((info & Smask) && (info & Hmask))
      ofn.exception("S&H masks are ON simultaneously in rectangle info byte (25.7)");
   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   dword width       = (info & Wmask) ? (_mod_gwidth   = ofn.getUnsignedInt(4)) : _mod_gwidth();
   dword height      = (info & Hmask) ? (_mod_gheight  = ofn.getUnsignedInt(4)) : 
                       (info & Smask) ? (_mod_gheight  = width                ) : _mod_gheight();
   int8b p1x         = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b p1y         = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();

   laydata::tdtlayer* dwl = static_cast<laydata::tdtlayer*>(dst_cell->securelayer(layno));
   if (info & Rmask) 
   {
      //read the repetition record from the input stream
      readRepetitions(ofn);
      int4b* rptpnt = _mod_repete().lcarray();
      assert(rptpnt);
      for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
      {
         TP p1(p1x+rptpnt[2*rcnt],p1y+rptpnt[2*rcnt+1]);
         TP p2(p1x+rptpnt[2*rcnt]+width,p1y+rptpnt[2*rcnt+1]+height);
         dwl->addbox(p1, p2, false);
      }
   }
   else
   {
      TP p1(p1x,p1y);
      TP p2(p1x+width, p1y+height);
      dwl->addbox(p1, p2, false);
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readPolygon(OasisInFile& ofn, laydata::tdtcell* dst_cell)
{
   const byte Pmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   dword     layno   = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word      dtype   = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   PointList plist   = (info & Pmask) ? (_mod_pplist   = readPointList(ofn)   ) : _mod_pplist();
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();

   laydata::tdtlayer* dwl = static_cast<laydata::tdtlayer*>(dst_cell->securelayer(layno));
   if (info & Rmask) 
   {
      //read the repetition record from the input stream
      readRepetitions(ofn);
      int4b* rptpnt = _mod_repete().lcarray();
      assert(rptpnt);
      for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
      {
         pointlist laypl;
         plist.calcPoints(laypl, p1x+rptpnt[2*rcnt],p1y+rptpnt[2*rcnt+1]);
         dwl->addpoly(laypl, false);
      }
   }
   else
   {
      pointlist laypl;
      plist.calcPoints(laypl, p1x,p1y);
      dwl->addpoly(laypl, false);
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readPath(OasisInFile& ofn, laydata::tdtcell* dst_cell)
{
   const byte Emask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Pmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   dword     layno   = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word      dtype   = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   word      hwidth  = (info & Wmask) ? (_mod_pathhw   = ofn.getUnsignedInt(4)) : _mod_pathhw();
   PathExtensions exs,exe;
   if (info & Emask)
   {
      readExtensions(ofn, exs, exe);
   }
   else
   {
      exs = _mod_exs();
      exe = _mod_exe();
   }
   PointList plist   = (info & Pmask) ? (_mod_wplist   = readPointList(ofn)   ) : _mod_wplist();
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();

   laydata::tdtlayer* dwl = static_cast<laydata::tdtlayer*>(dst_cell->securelayer(layno));
   if (info & Rmask) 
   {
      //read the repetition record from the input stream
      readRepetitions(ofn);
      int4b* rptpnt = _mod_repete().lcarray();
      assert(rptpnt);
      for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
      {
         pointlist laypl;
         plist.calcPoints(laypl, p1x+rptpnt[2*rcnt],p1y+rptpnt[2*rcnt+1]);
         dwl->addwire(laypl, hwidth, false);
      }
   }
   else
   {
      pointlist laypl;
      plist.calcPoints(laypl, p1x,p1y);
      dwl->addwire(laypl, hwidth, false);
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readText(OasisInFile& ofn)
{
   const byte Cmask   = 0x40;
   const byte Nmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02; // In the standard is T, but it looks like a typo
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();
   std::string text  = (info & Cmask) ? (_mod_text     = ofn.getTextRefName(info & Nmask)) :
                                                                                  _mod_text();
   dword     layno   = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word      dtype   = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask) readRepetitions(ofn);
//   Repetitions rpt = _mod_repete();
}

void Oasis::Cell::readReference(OasisInFile& ofn, laydata::tdtcell* dst_cell, 
                                laydata::tdtlibdir* tdt_db, bool exma)
{
   const byte Cmask   = 0x80;
   const byte Nmask   = 0x40;
   const byte Xmask   = 0x20;
   const byte Ymask   = 0x10;
   const byte Rmask   = 0x08;
   const byte Mmask   = 0x04;
   const byte Amask   = 0x02;
   const byte Fmask   = 0x01;

   byte info = ofn.getByte();
   std::string name  = (info & Cmask) ? (_mod_cellref  = ofn.getCellRefName(info & Nmask)) :
                                                                                  _mod_cellref();
   bool flip = (info & Fmask);
   real angle, magnification;
   if (exma)
   { 
      angle = (info & Amask) ? ofn.getReal() : 0.0;
      magnification = (info & Mmask) ? ofn.getReal() : 1.0;
   }
   else
   {
      angle = 90.0 * (real)((info & (Mmask | Amask)) >> 1);
      magnification = 1.0;
   }
   if (magnification <= 0)
         ofn.exception("Bad magnification value (22.10)");
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   //
   laydata::CellDefin strdefn = tdt_db->linkcellref(name, TARGETDB_LIB);
   if (info & Rmask) 
   {
      //read the repetition record from the input stream
      readRepetitions(ofn);
      int4b* rptpnt = _mod_repete().lcarray();
      assert(rptpnt);
      for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
      {
         TP p1(p1x+rptpnt[2*rcnt],p1y+rptpnt[2*rcnt+1]);
         dst_cell->registerCellRef( strdefn,
                                    CTM(p1,
                                        magnification,
                                        angle,
                                        flip
                                       )
                                  );
      }
   }
   else
   {
      TP p1(p1x,p1y);
      dst_cell->registerCellRef( strdefn,
                                 CTM(p1,
                                     magnification,
                                     angle,
                                     flip
                                    )
                               );
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimRectangle(OasisInFile& ofn)
{
   const byte Smask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   if ((info & Smask) && (info & Hmask))
      ofn.exception("S&H masks are ON simultaneously in rectangle info byte (25.7)");
   if (info & Lmask) ofn.getUnsignedInt(4);
   if (info & Dmask) ofn.getUnsignedInt(2);
   if (info & Wmask) ofn.getUnsignedInt(4);
   if (info & Hmask) ofn.getUnsignedInt(4);
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimPolygon(OasisInFile& ofn)
{
   const byte Pmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   if (info & Lmask) ofn.getUnsignedInt(4);
   if (info & Dmask) ofn.getUnsignedInt(2);
   if (info & Pmask) readPointList(ofn);
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimPath(OasisInFile& ofn)
{
   const byte Emask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Pmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   if (info & Lmask) ofn.getUnsignedInt(4);
   if (info & Dmask) ofn.getUnsignedInt(2);
   if (info & Wmask) ofn.getUnsignedInt(4);
   if (info & Emask)
   {
      PathExtensions exs,exe;
      readExtensions(ofn, exs, exe);
   }
   if (info & Pmask) readPointList(ofn);
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
//   Repetitions rpt = _mod_repete();
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimText(OasisInFile& ofn)
{
   const byte Cmask   = 0x40;
   const byte Nmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02; // In the standard is T, but it looks like a typo
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();
   if (info & Cmask) ofn.getTextRefName(info & Nmask);
   if (info & Lmask) ofn.getUnsignedInt(4);
   if (info & Dmask) ofn.getUnsignedInt(2);
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
}

void Oasis::Cell::skimReference(OasisInFile& ofn, bool exma)
{
   const byte Cmask   = 0x80;
   const byte Nmask   = 0x40;
   const byte Xmask   = 0x20;
   const byte Ymask   = 0x10;
   const byte Rmask   = 0x08;
   const byte Mmask   = 0x04;
   const byte Amask   = 0x02;
   const byte Fmask   = 0x01;

   byte info = ofn.getByte();

   std::string name  = (info & Cmask) ? (_mod_cellref  = ofn.getCellRefName(info & Nmask)) :
                                                                                  _mod_cellref();
   if (exma)
   { 
      if (info & Amask) ofn.getReal();
      if (info & Mmask) ofn.getReal();
   }
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);

   if (info & Rmask) readRepetitions(ofn);
   _referenceNames.insert(name);
}

//------------------------------------------------------------------------------
Oasis::PointList Oasis::Cell::readPointList(OasisInFile& ofn)
{
   byte plty = ofn.getByte();
   if (plty >= dt_unknown)
      ofn.exception("Bad point list type (7.7.8)");
   else
   {
      PointList result(ofn, (PointListType)plty);
      return result;
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readRepetitions(OasisInFile& ofn)
{
   byte rpty = ofn.getByte();
   if (rpty >= rp_unknown)
      ofn.exception("Bad repetition type (7.6.14)");
   else if (0 != rpty)
   {
      _mod_repete = Repetitions(ofn, (RepetitionTypes)rpty);
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readExtensions(OasisInFile& ofn, PathExtensions& exs, PathExtensions& exe)
{
   byte scheme = ofn.getByte();
   if (scheme & 0xF0)
      ofn.exception("Bad extention type (27.? - not explicitly ruled-out)");
   // deal with the start extenstion
   byte extype = (scheme & 0x0c) >> 2;
   if (0 != extype)
   {
      _mod_exs = PathExtensions(ofn, (ExtensionTypes)extype);
   }
   exs = _mod_exs();
   // deal with the end extenstion
   extype = (scheme & 0x03);
   if (0 != extype)
   {
      _mod_exe = PathExtensions(ofn, (ExtensionTypes)extype);
   }
   exe = _mod_exe();
}

void Oasis::Cell::linkReferences(OasisInFile& ofn)
{
   for (NameSet::const_iterator CRN = _referenceNames.begin(); CRN != _referenceNames.end(); CRN++)
   {
      Cell* ws2 = ofn.getCell(*CRN);
      if (ws2)
      {
         _children.push_back(ws2);
         ws2->_haveParent = true;
      }
      else
      {
         char wstr[256];
         sprintf(wstr," Structure %s is referenced, but not defined!",CRN->c_str() );
         tell_log(console::MT_WARNING,wstr);
      }
   }
}

Oasis::OASHierTree* Oasis::Cell::hierOut(OASHierTree* Htree, Cell* parent)
{
   // collecting hierarchical information
   Htree = DEBUG_NEW OASHierTree(this, parent, Htree);
   for (OasisCellList::const_iterator CSTR = _children.begin(); CSTR != _children.end(); CSTR++)
   {
      if (NULL == (*CSTR)) continue;
      else
      {
         Htree = (*CSTR)->hierOut(Htree,this);
      }
   }
   return Htree;
}

//==============================================================================
Oasis::PointList::PointList(OasisInFile& ofn, PointListType pltype) : _pltype(pltype)
{
   _vcount = ofn.getUnsignedInt(4);
   _delarr = DEBUG_NEW int4b[2*_vcount];
   switch (_pltype)
   {
      case dt_manhattanH : readManhattanH(ofn) ; break;//+2
      case dt_manhattanV : readManhattanV(ofn) ; break;//+2
      case dt_mamhattanE : readManhattanE(ofn) ; break;//+1
      case dt_octangular : readOctangular(ofn) ; break;//+1
      case dt_allangle   : readAllAngle(ofn)   ; break;//+1
      case dt_doubledelta: readDoubleDelta(ofn); break;//+1
      default: assert(false);
   }
}

Oasis::PointList::PointList(PointList& plst)
{
   _pltype = plst._pltype;
   _vcount = plst._vcount;
   _delarr = DEBUG_NEW int4b[2*_vcount];
   for (dword cpnt = 0; cpnt < 2*_vcount; cpnt++)
      _delarr[cpnt] = plst._delarr[cpnt];
}

Oasis::PointList::~PointList()
{
   if (NULL != _delarr) delete [] _delarr;
}

Oasis::PointList& Oasis::PointList::operator = (const PointList& plst)
{
   if (NULL != _delarr) delete [] _delarr;
   _pltype = plst._pltype;
   _vcount = plst._vcount;
   _delarr = DEBUG_NEW int4b[2*_vcount];
   for (dword cpnt = 0; cpnt < 2*_vcount; cpnt++)
      _delarr[cpnt] = plst._delarr[cpnt];
   return *this;
}

void Oasis::PointList::readManhattanH(OasisInFile& ofb)
{
   for (dword ccrd = 0; ccrd < _vcount; ccrd++)
   {
      if (ccrd % 2) {_delarr[2*ccrd] = 0            ; _delarr[2*ccrd+1] = ofb.getInt(8);}
      else          {_delarr[2*ccrd] = ofb.getInt(8); _delarr[2*ccrd+1] = 0;            }
   }
}

void Oasis::PointList::readManhattanV(OasisInFile& ofb)
{
   for (dword ccrd = 0; ccrd < _vcount; ccrd++)
   {
      if (ccrd % 2) {_delarr[2*ccrd] = ofb.getInt(8); _delarr[2*ccrd+1] = 0;            }
      else          {_delarr[2*ccrd] = 0            ; _delarr[2*ccrd+1] = ofb.getInt(8);}
   }
}

void Oasis::PointList::readManhattanE(OasisInFile& ofb)
{
   qword             data;
   DeltaDirections   direction;
   byte*             bdata = (byte*)&data;
   for (dword ccrd = 0; ccrd < _vcount; ccrd++)
   {
      data        = ofb.getUnsignedInt(8);
      int4b idata = (data >> 2);
      direction   = (DeltaDirections)(bdata[0] & 0x03);
      switch (direction)
      {
         case dr_east : _delarr[2*ccrd] = idata; _delarr[2*ccrd+1] = 0    ; break;
         case dr_north: _delarr[2*ccrd] = 0    ; _delarr[2*ccrd+1] = idata; break;
         case dr_west : _delarr[2*ccrd] =-idata; _delarr[2*ccrd+1] = 0    ; break;
         case dr_south: _delarr[2*ccrd] = 0    ; _delarr[2*ccrd+1] =-idata; break;
         default: assert(false);
      }
   }
}

void Oasis::PointList::readOctangular(OasisInFile& ofb)
{
   qword             data;
   DeltaDirections   direction;
   byte*             bdata = (byte*)&data;
   for (dword ccrd = 0; ccrd < _vcount; ccrd++)
   {
      data        = ofb.getUnsignedInt(8);
      int4b idata = (data >> 3);
      direction   = (DeltaDirections)(bdata[0] & 0x07);
      switch (direction)
      {
         case dr_east     : _delarr[2*ccrd] = idata; _delarr[2*ccrd+1] = 0    ; break;
         case dr_north    : _delarr[2*ccrd] = 0    ; _delarr[2*ccrd+1] = idata; break;
         case dr_west     : _delarr[2*ccrd] =-idata; _delarr[2*ccrd+1] = 0    ; break;
         case dr_south    : _delarr[2*ccrd] = 0    ; _delarr[2*ccrd+1] =-idata; break;
         case dr_northeast: _delarr[2*ccrd] = idata; _delarr[2*ccrd+1] = idata; break;
         case dr_northwest: _delarr[2*ccrd] =-idata; _delarr[2*ccrd+1] = idata; break;
         case dr_southeast: _delarr[2*ccrd] = idata; _delarr[2*ccrd+1] =-idata; break;
         case dr_southwest: _delarr[2*ccrd] =-idata; _delarr[2*ccrd+1] =-idata; break;
         default: assert(false);
      }
   }
}

void Oasis::PointList::readAllAngle(OasisInFile& ofb)
{
   for (dword ccrd = 0; ccrd < _vcount; ccrd++)
   {
      readDelta(ofb, _delarr[2*ccrd  ], _delarr[2*ccrd+1]);
   }
}

void Oasis::PointList::readDoubleDelta(OasisInFile& ofb)
{
   /*@TODO*/assert(false);
}

void Oasis::PointList::calcPoints(pointlist& plst, int4b p1x, int4b p1y)
{
   switch (_pltype)
   {
      case dt_manhattanH : calcManhattanH(plst, p1x, p1y) ; break;
      case dt_manhattanV : calcManhattanV(plst, p1x, p1y) ; break;
      case dt_mamhattanE : calcManhattanE(plst, p1x, p1y) ; break;
      case dt_octangular : calcOctangular(plst, p1x, p1y) ; break;
      case dt_allangle   : calcAllAngle(plst, p1x, p1y)   ; break;//+1
      case dt_doubledelta: calcDoubleDelta(plst, p1x, p1y); break;//+1
      default: assert(false);
   }

}

void Oasis::PointList::calcManhattanH(pointlist& plst, int4b p1x, int4b p1y)
{
   plst.reserve(_vcount + 2);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      if (curp % 2) cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      else          cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      plst.push_back(cpnt);
   }
   if (curp % 2) cpnt.setY(p1y);
   else          cpnt.setX(p1x);
   plst.push_back(cpnt);
}

void Oasis::PointList::calcManhattanV(pointlist& plst, int4b p1x, int4b p1y)
{
   plst.reserve(_vcount + 2);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      if (curp % 2) cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      else          cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      plst.push_back(cpnt);
   }
   if (curp % 2) cpnt.setX(p1x);
   else          cpnt.setY(p1y);
   plst.push_back(cpnt);
}

void Oasis::PointList::calcManhattanE(pointlist& plst, int4b p1x, int4b p1y)
{
   plst.reserve(_vcount + 1);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      plst.push_back(cpnt);
   }
}

void Oasis::PointList::calcOctangular(pointlist& plst, int4b p1x, int4b p1y)
{
   plst.reserve(_vcount + 1);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      plst.push_back(cpnt);
   }
}

void Oasis::PointList::calcAllAngle(pointlist& plst, int4b p1x, int4b p1y)
{
   plst.reserve(_vcount + 1);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      plst.push_back(cpnt);
   }
}

void Oasis::PointList::calcDoubleDelta(pointlist& plst, int4b p1x, int4b p1y)
{
   /*@TODO*/assert(false);
}

//==============================================================================
Oasis::Repetitions::Repetitions(OasisInFile& ofn, RepetitionTypes rptype) : 
                                    _rptype(rptype), _bcount(0), _lcarray(NULL)
{
   switch (_rptype)
   {
      case rp_regXY   : readregXY(ofn)    ; break;
      case rp_regX    : readregX(ofn)     ; break;
      case rp_regY    : readregY(ofn)     ; break;
      case rp_varX    : readvarX(ofn)     ; break;
      case rp_varXxG  : readvarXxG(ofn)   ; break;
      case rp_varY    : readvarY(ofn)     ; break;
      case rp_varYxG  : readvarYxG(ofn)   ; break;
      case rp_regDia2D: readregDia2D(ofn) ; break;
      case rp_regDia1D: readregDia1D(ofn) ; break;
      case rp_varAny  : readvarAny(ofn)   ; break;
      case rp_varAnyG : readvarAnyG(ofn)  ; break;
      default: assert(false);
   }
}

Oasis::Repetitions::~Repetitions()
{
   if (NULL !=_lcarray)
      delete [] _lcarray;
}

Oasis::Repetitions& Oasis::Repetitions::operator = (const Repetitions& rpts)
{
   if (NULL != _lcarray) delete [] _lcarray;
   _rptype  = rpts._rptype;
   _bcount  = rpts._bcount;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   for (dword crpt = 0; crpt < 2*_bcount; crpt++)
      _lcarray[crpt] = rpts._lcarray[crpt];
   return *this;
}

void Oasis::Repetitions::readregXY(OasisInFile& ofn)
{//type 1
   dword countx = ofn.getUnsignedInt(4) + 2;
   dword county = ofn.getUnsignedInt(4) + 2;
   dword stepx  = ofn.getUnsignedInt(4);
   dword stepy  = ofn.getUnsignedInt(4);
   _bcount  = countx*county;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   dword p1y = 0;
   for (dword yi = 0; yi < county; yi++)
   {
      dword p1x = 0;
      for (dword xi = 0; xi < countx; xi++)
      {
         _lcarray[2*(yi*countx+xi)  ] = p1x;
         _lcarray[2*(yi*countx+xi)+1] = p1y;
         p1x += stepx;
      }
      p1y += stepy;
   }
}

void Oasis::Repetitions::readregX(OasisInFile& ofn)
{//type 2
   dword countx = ofn.getUnsignedInt(4) + 2;
   dword stepx  = ofn.getUnsignedInt(4);
   _bcount  = countx;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   dword p1y = 0;
   dword p1x = 0;
   for (dword xi = 0; xi < countx; xi++)
   {
      _lcarray[2*xi  ] = p1x;
      _lcarray[2*xi+1] = p1y;
      p1x += stepx;
   }
}

void Oasis::Repetitions::readregY(OasisInFile& ofn)
{//type 3
   dword county = ofn.getUnsignedInt(4) + 2;
   dword stepy  = ofn.getUnsignedInt(4);
   _bcount  = county;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   dword p1y = 0;
   dword p1x = 0;
   for (dword yi = 0; yi < county; yi++)
   {
      _lcarray[2*yi  ] = p1x;
      _lcarray[2*yi+1] = p1y;
      p1y += stepy;
   }
}

void Oasis::Repetitions::readvarX(OasisInFile&)
{//type 4
   /*@TODO*/assert(false);
}

void Oasis::Repetitions::readvarXxG(OasisInFile&)
{//type 5
   /*@TODO*/assert(false);
}

void Oasis::Repetitions::readvarY(OasisInFile&)
{//type 6
   /*@TODO*/assert(false);
}

void Oasis::Repetitions::readvarYxG(OasisInFile&)
{//type 7
   /*@TODO*/assert(false);
}

void Oasis::Repetitions::readregDia2D(OasisInFile& ofn)
{//type 8
   dword countn = ofn.getUnsignedInt(4) + 2;
   dword countm = ofn.getUnsignedInt(4) + 2;
   int4b nx, ny, mx, my;
   readDelta (ofn, nx, ny);
   readDelta (ofn, mx, my);
   _bcount  = countn * countm;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   int4b s1x = 0;
   int4b s1y = 0;
   for (dword mi = 0; mi < countm; mi++)
   {
      int4b p1x = s1x;
      int4b p1y = s1y;
      for (dword nj = 0; nj < countn; nj++)
      {
         _lcarray[2*(mi*countn + nj)    ] = p1x;
         _lcarray[2*(mi*countn + nj) + 1] = p1y;
         p1x += nx; p1y += ny;
      }
      s1x += mx; s1y += my;
   }
   
}

void Oasis::Repetitions::readregDia1D(OasisInFile& ofn)
{//type 9
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   readDelta(ofn, p1x, p1y);
   for (dword pj = 1; pj < _bcount; pj++)
   {
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ] + p1x;
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1] + p1y;
   }
}

void Oasis::Repetitions::readvarAny(OasisInFile& ofn)
{//type 10
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   for (dword pj = 1; pj < _bcount; pj++)
   {
      readDelta(ofn, p1x, p1y);
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ] + p1x;
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1] + p1y;
   }
}

void Oasis::Repetitions::readvarAnyG(OasisInFile& ofn)
{//type 11
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   dword grid   = ofn.getUnsignedInt(4);
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   for (dword pj = 1; pj < _bcount; pj++)
   {
      readDelta(ofn, p1x, p1y);
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ] + p1x * grid;
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1] + p1y * grid;
   }
}

//==============================================================================
void Oasis::StdProperties::getProperty1(OasisInFile& ofn)
{
   const byte Umask   = 0xf0;
   const byte Vmask   = 0x08;
   const byte Cmask   = 0x04;
   const byte Nmask   = 0x02;
   const byte Smask   = 0x01;

   byte info = ofn.getByte();

   std::string pname = (info & Cmask) ?
                       (info & Nmask) ? (_propName     = ofn.propNames()->getName(ofn.getUnsignedInt(4))) :
                                        (_propName     = ofn.getString()                                ) :
                                         _propName();
   dword numValues   = 0;
   if (info & Vmask)
   {
      if (info & Umask)
         ofn.exception("Bad property value-count (31.5 - UUUU must be 0)");
   }
   else
   {
      if (15 == (numValues = ((info & Umask) >> 4)))
         numValues = ofn.getUnsignedInt(2);
   }
   for (word i = 0; i < numValues; i++)
   {
      byte propValueType = ofn.getUnsignedInt(1);
      if (15 < propValueType)
         ofn.exception("Bad property value type (7.8.2)");
      switch (propValueType)
      {
         case  8: ofn.getUnsignedInt(8); break;
         case  9: ofn.getInt(8); break;
         case 10: ofn.getString(); break;// a-string
         case 11: ofn.getString(); break;// b-string
         case 12: ofn.getString(); break;// n-string
         case 13: ofn.propStrings()->getName(ofn.getUnsignedInt(4)); break;// a-string
         case 14: ofn.propStrings()->getName(ofn.getUnsignedInt(4)); break;// b-string
         case 15: ofn.propStrings()->getName(ofn.getUnsignedInt(4)); break;// n-string
         default: ofn.getReal(propValueType);
      }
   }
   if (info & Smask)
   {
   }
   else
   {
   }
}

//==============================================================================
Oasis::PathExtensions::PathExtensions(OasisInFile& ofn, ExtensionTypes extype) : _extype(extype)
{
   switch (_extype)
   {
      case ex_flush     : 
      case ex_hwidth    : break;
      case ex_explicit  : _exex = ofn.getInt(2);
      default: assert(false);
   }
}

//==============================================================================
void Oasis::readDelta(OasisInFile& ofb, int4b& deltaX, int4b& deltaY)
{
   DeltaDirections   direction;
   qword             data  = ofb.getUnsignedInt(8);
   byte*             bdata = (byte*)&data;
   if (bdata[0] & 0x01)
   { // g delta 2
      if (bdata[0] & 0x02) deltaX =-(int4b)(data >> 2);
      else                 deltaX = (data >> 2);
      deltaY = ofb.getInt(8);
   }
   else
   { //g delta 1
      direction = (DeltaDirections)((bdata[0] & 0x0e) >> 1);
      switch (direction)
      {
         case dr_east     : deltaX = (data >> 4); deltaY = 0          ; break;
         case dr_north    : deltaX = 0          ; deltaY = (data >> 4); break;
         case dr_west     : deltaX =-(int4b)(data >> 4); deltaY = 0          ; break;
         case dr_south    : deltaX = 0          ; deltaY =-(int4b)(data >> 4); break;
         case dr_northeast: deltaX = (data >> 4); deltaY = (data >> 4); break;
         case dr_northwest: deltaX =-(int4b)(data >> 4); deltaY = (data >> 4); break;
         case dr_southeast: deltaX = (data >> 4); deltaY =-(int4b)(data >> 4); break;
         case dr_southwest: deltaX =-(int4b)(data >> 4); deltaY =-(int4b)(data >> 4); break;
         default: assert(false);
      }
   }
}


//-----------------------------------------------------------------------------
// class Oas2Ted
//-----------------------------------------------------------------------------
Oasis::Oas2Ted::Oas2Ted(OasisInFile* src_lib, laydata::tdtlibdir* tdt_db/*, const LayerMapGds& theLayMap*/) :
      _src_lib(src_lib), _tdt_db(tdt_db),/* _theLayMap(theLayMap),*/
               _coeff((*_tdt_db)()->UU() / src_lib->libUnits()), _conversionLength(0)
{}

void Oasis::Oas2Ted::run(const nameList& top_str_names, bool recursive, bool overwrite)
{
   assert(_src_lib->hierTree());

   for (nameList::const_iterator CN = top_str_names.begin(); CN != top_str_names.end(); CN++)
   {
      Cell* src_structure = _src_lib->getCell(*CN);
      if (NULL != src_structure)
      {
         Oasis::OASHierTree* root = _src_lib->hierTree()->GetMember(src_structure);
         if (recursive) preTraverseChildren(root);
         if (!src_structure->traversed())
         {
            _convertList.push_back(src_structure);
            src_structure->set_traversed(true);
            _conversionLength += src_structure->cellSize();
         }
      }
      else
      {
         std::ostringstream ost; ost << "OASIS import: ";
         ost << "Structure \""<< *CN << "\" not found in the OASIS DB.";
         tell_log(console::MT_WARNING,ost.str());
      }
   }
   if (_src_lib->reopenFile())
   {
      TpdPost::toped_status(console::TSTS_PRGRSBARON, _conversionLength);
      try
      {
         for (OasisCellList::iterator CS = _convertList.begin(); CS != _convertList.end(); CS++)
         {
            convert(*CS, overwrite);
            (*CS)->set_traversed(false); // restore the state for eventual second conversion
         }
         tell_log(console::MT_INFO, "Done");
      }
      catch (EXPTNreadOASIS) {tell_log(console::MT_INFO, "Conversion aborted with errors");}
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      _src_lib->closeFile();
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      (*_tdt_db)()->recreate_hierarchy(_tdt_db);
   }
}

void Oasis::Oas2Ted::preTraverseChildren(const Oasis::OASHierTree* root)
{
   const Oasis::OASHierTree* Child = root->GetChild(TARGETDB_LIB);
   while (Child)
   {
      if ( !Child->GetItem()->traversed() )
      {
         // traverse children first
         preTraverseChildren(Child);
         Oasis::Cell* sstr = const_cast<Oasis::Cell*>(Child->GetItem());
         if (!sstr->traversed())
         {
            _convertList.push_back(sstr);
            sstr->set_traversed(true);
            _conversionLength += sstr->cellSize();
         }
      }
      Child = Child->GetBrother(TARGETDB_LIB);
   }
}

void Oasis::Oas2Ted::convert(Oasis::Cell* src_structure, bool overwrite)
{
   std::string gname = src_structure->name();
   // check that destination structure with this name exists
   laydata::tdtcell* dst_structure = static_cast<laydata::tdtcell*>((*_tdt_db)()->checkcell(gname));
   std::ostringstream ost; ost << "OASIS import: ";
   if (NULL != dst_structure)
   {
      if (overwrite)
      {
         /*@TODO Erase the existing structure and convert*/
         ost << "Structure "<< gname << " should be overwritten, but cell erase is not implemened yet ...";
         tell_log(console::MT_WARNING,ost.str());
      }
      else
      {
         ost << "Structure "<< gname << " already exists. Skipped";
         tell_log(console::MT_INFO,ost.str());
      }
   }
   else
   {
      ost << "Structure " << gname << "...";
      tell_log(console::MT_INFO,ost.str());
      // first create a new cell
      dst_structure = DEBUG_NEW laydata::tdtcell(gname);
      // call the cell converter
      src_structure->import(*_src_lib, dst_structure, _tdt_db/*, _theLayMap*/);
      // and finally - register the cell
      (*_tdt_db)()->registercellread(gname, dst_structure);
   }
}
// oasisimport("AMODUL", true, false);
