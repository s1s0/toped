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

const dword Oasis::Iso3309Crc32::_crc32Poly     = 0x04c11db7u;  // The CRC polynomial
const dword Oasis::Iso3309Crc32::_crc32Constant = 0x38fb2284u;  // constant which matches polynomial above
const dword Oasis::Iso3309Crc32::_crc32LSBit    = 0x80000000u;
const dword Oasis::Iso3309Crc32::_crc32MSBit    = 0x00000001u;
const dword Oasis::Iso3309Crc32::_crc32AllBits  = 0xffffffffu;

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
            case oas_CBLOCK     : ofh.inflateCBlock(); break;
            default : _offsetEnd = ofh.setPosition(savedPos); return;
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
            case oas_CBLOCK     : ofh.inflateCBlock();break;
            default : _offsetEnd = ofh.setPosition(savedPos); return;
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
            case oas_CBLOCK     : ofh.inflateCBlock(); break;
            default : _offsetEnd = ofh.setPosition(savedPos); return;
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
            case oas_CBLOCK     : ofh.inflateCBlock(); break;
            default : _offsetEnd = ofh.setPosition(savedPos); return;
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
Oasis::CBlockInflate::CBlockInflate(wxFFile* ifn, wxFileOffset fofset, dword size_deflated, dword size_inflated)
{
   // initialize z_stream members
   zalloc       = 0;
   zfree        = 0;
   opaque       = 0;
   next_in      = DEBUG_NEW byte[size_deflated];
   next_out     = _output_buffer = DEBUG_NEW byte[size_inflated];
   avail_in     = ifn->Read(next_in, size_deflated);
   assert(avail_in = size_deflated);
   avail_out    = size_inflated;
   //
   _startPosInFile = fofset;
   if (Z_OK != (_state = inflateInit2(this, -15) ))
      throw EXPTNreadOASIS(msg);
   if (Z_STREAM_END != (_state = inflate(this,Z_NO_FLUSH)))
      throw EXPTNreadOASIS(msg);
   if (Z_OK != (_state = inflateEnd(this)))
      throw EXPTNreadOASIS(msg);
   _bufOffset = 0;
   _bufSize   = size_inflated;
//   delete [] next_in;
}

void Oasis::CBlockInflate::readUncompressedBuffer(void *pBuf, size_t nCount)
{
   if ((_bufOffset + nCount) > _bufSize)
      throw EXPTNreadOASIS("Read past the end of current CBLOCK (internal error)");
   else
   {
      for (size_t i = 0; i < nCount; i++, _bufOffset++)
         ((byte*)pBuf)[i] = _output_buffer[_bufOffset];
   }
}

Oasis::CBlockInflate::~CBlockInflate()
{
   delete [] _output_buffer;
}

//===========================================================================
Oasis::Iso3309Crc32::Iso3309Crc32()
{
   _pauseCalculation = false;
   // initialize auxiliary table
   tableLoad();
   // preload shift register, per CRC-32 spec
   _theCrc = ~_crc32AllBits;
}

void Oasis::Iso3309Crc32::tableLoad()
{
   /* initialize auxiliary table */
   dword   poly = reflect(_crc32Poly);
   for (dword i = 0; i < 256; i++)
   {
      dword c = i;
      for (dword j = 0; j < 8 /*Bits in 1 byte*/; j++)
         c = c & _crc32MSBit ? (c >> 1) ^ poly : (c >> 1);
      _crc32Table[i] = c;
   }
}

dword Oasis::Iso3309Crc32::reflect(dword in)
{
   dword   out = 0x0u;
   for (int i = 0; i < 32; i++)
   {
      if (in & 0x01)
         out |= (0x01 << (31 - i) );
      in >>= 1;
   }
   return(out);
}

void Oasis::Iso3309Crc32::add(const byte* buf, size_t len)
{
   if (_pauseCalculation) return;
   dword  val = _theCrc;
   val = ~val & _crc32AllBits;
   for (size_t i = 0; i < len; i++)
   {
      val = (val >> 8) ^ _crc32Table[ ( val ^ (dword) buf[i]) & 0xff ];
   }
   val = ~val & _crc32AllBits;
   _theCrc = val;
}

//===========================================================================
Oasis::OasisInFile::OasisInFile(std::string fn) : _cellNames(NULL), _textStrings(NULL),
   _propNames(NULL), _propStrings(NULL), _layerNames(NULL), _xNames(NULL), _offsetFlag(false),
   _fileName(fn), _fileLength(0), _filePos(0), _progresPos(0), _curCBlock(NULL),
   _validation(vs_unknown), _signature(0u)
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
      // Initialize the progress bar
      _fileLength = _oasisFh.Length();
      TpdPost::toped_status(console::TSTS_PRGRSBARON, _fileLength);
   }
}

bool Oasis::OasisInFile::reopenFile()
{
   _filePos = 0;
   _progresPos = 0;
   _curCBlock = NULL;
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

wxFileOffset Oasis::OasisInFile::setPosition(wxFileOffset filePos)
{
   wxFileOffset coffset;
   if (NULL != _curCBlock)
   {
      coffset = _curCBlock->startPosInFile() - 1;
      delete _curCBlock;
      _curCBlock = NULL;
   }
   else
      coffset = _filePos - 1;
   _oasisFh.Seek(filePos, wxFromStart);
   _filePos = filePos;
   return coffset;
}

void Oasis::OasisInFile::closeFile()
{
   if (_status)
      _oasisFh.Close();
}

void Oasis::OasisInFile::inflateCBlock()
{
   wxFileOffset cblockfPos = _filePos;
   byte compression_type = getUnsignedInt(2);
   if (0 != compression_type)
      exception("Unknown compression type in the CBLOCK (35.3)");
   dword size_uncompressed = getUnsignedInt(4);
   dword size_compressed   = getUnsignedInt(4);
   _curCBlock = DEBUG_NEW CBlockInflate(&_oasisFh , cblockfPos, size_compressed, size_uncompressed);
   _filePos += size_compressed;
   if (2048 < (_filePos - _progresPos))
   {
      _progresPos = _filePos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresPos);
   }
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
   _validation = (Oasis::ValidationScheme)valid_scheme;
   if (Oasis::vs_noValidation == _validation)
   {
      info << "OASIS file has no validation signature";
      tell_log(console::MT_INFO, info.str());
   }
   else
   {
      byte* sigbyte = (byte*) &_signature;
      sigbyte[0] = getByte();
      sigbyte[1] = getByte();
      sigbyte[2] = getByte();
      sigbyte[3] = getByte();
   }
}

void Oasis::OasisInFile::readLibrary()
{
   // get oas_START
   byte recType = getUnsignedInt(1);
   if (oas_START != recType) exception("\"START\" record expected here (13.10)");
   readStartRecord();
   // Some sequences (CELL) does not have an explicit end record. So they end when a record
   // which can not be a member of the definition appears. The trouble is that the last byte
   // read from the input stream should be reused in the loop here
   bool rlb = false;// reuse last byte
   Cell* curCell = NULL;
   do {
      if (!rlb) recType = getUnsignedInt(1);
      switch (recType)
      {
         case oas_PAD         : rlb = false; break;
         case oas_PROPERTY_1  :
            setPropContext(pc_file);
            getProperty1();
            rlb = false; break;
         case oas_PROPERTY_2  : /*There is nothing to do*/rlb = false; break;
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
         case oas_CBLOCK      : inflateCBlock(); rlb = false; break;
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
         case oas_XNAME_1     : assert(false);/*@TODO oas_XNAME_1*/ rlb = false; break;
         case oas_XNAME_2     : assert(false);/*@TODO oas_XNAME_1*/ rlb = false; break;
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
   rawRead(&bytein,1);
   return bytein;
}

float Oasis::OasisInFile::getFloat()
{
   float        floatin          ; // last 4 bytes read from the file stream
   rawRead(&floatin,4);
   return floatin;
}

double Oasis::OasisInFile::getDouble()
{
   double        doublein         ; // last 8 bytes read from the file stream
   rawRead(&doublein,8);
   return doublein;
}

//------------------------------------------------------------------------------
std::string Oasis::OasisInFile::getString(/*Oasis string type check*/)
{
   dword   length  = getUnsignedInt(2) ; //string length
   char* theString = DEBUG_NEW char[length+1];

   rawRead(theString,length);
   theString[length] = 0x00;
   std::string result(theString);
   delete [] theString;
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
      if (bytein & cmask)
      {
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
         if (bytecounter > length)
            exception("Unsigned integer with unexpected length(7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
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
      if ((0 == bytecounter) || (bytein & cmask))
      {
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
         if (bytecounter > length)
            exception("Unsigned integer with unexpected length(7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
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

void Oasis::OasisInFile::collectLayers(ExtLayers& layers)
{
   for (DefinitionMap::const_iterator CSTR = _definedCells.begin(); CSTR != _definedCells.end(); CSTR++)
      CSTR->second->collectLayers(layers, false);
}

void Oasis::OasisInFile::hierOut()
{
   _hierTree = NULL;
   for (DefinitionMap::const_iterator CSTR = _definedCells.begin(); CSTR != _definedCells.end(); CSTR++)
      if (!CSTR->second->haveParent())
         _hierTree = CSTR->second->hierOut(_hierTree, NULL);
}

size_t Oasis::OasisInFile::rawRead(void *pBuf, size_t nCount)
{
   if (NULL == _curCBlock)
   {
      if (nCount != _oasisFh.Read(pBuf,nCount))
         exception("I/O error during read-in");
      else _filePos += nCount;
      if (2048 < (_filePos - _progresPos))
      {
         _progresPos = _filePos;
         TpdPost::toped_status(console::TSTS_PROGRESS, _progresPos);
      }
      return nCount;
   }
   else
   {
      _curCBlock->readUncompressedBuffer(pBuf, nCount);
      if ( _curCBlock->endOfBuffer() )
      {
         delete _curCBlock;
         _curCBlock = NULL;
      }
      return nCount;
   }
}

bool Oasis::OasisInFile::calculateCRC(Oasis::Iso3309Crc32& crc32)
{
   if (reopenFile())
   {
      byte buf;
      while (_filePos < _fileLength - 4)
      {
         rawRead(&buf, 1);
         crc32.add(&buf, 1);
      }
      closeFile();
      return true;
   }
   return false;
}

bool Oasis::OasisInFile::calculateChecksum(dword& checksum)
{
   if (reopenFile())
   {
      qword wsum = 0ull; // get a 64 bit number to handle overflows
      byte buf;
      while (_filePos < _fileLength - 4)
      {
         rawRead(&buf, 1);
         wsum +=buf;
         wsum &= 0xffffffff;
      }
      closeFile();
      checksum = (dword) wsum;
      return true;
   }
   checksum = 0u;
   return false;
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
   _mod_gx     = 0;
   _mod_gy     = 0;
   _mod_px     = 0;
   _mod_py     = 0;
   _mod_tx     = 0;
   _mod_ty     = 0;
   _mod_xymode = md_absolute;
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
         case oas_PROPERTY_2  : /*nothing more to do*/;break;
         case oas_XYRELATIVE  : _mod_xymode = md_relative;break;
         case oas_XYABSOLUTE  : _mod_xymode = md_absolute;break;
         case oas_CBLOCK      : ofn.inflateCBlock(); break;
         // <element> records
         case oas_PLACEMENT_1 : skimReference(ofn, false);break;
         case oas_PLACEMENT_2 : skimReference(ofn, true );break;
         case oas_TEXT        : skimText(ofn);break;
         case oas_XELEMENT    : /*@TODO oas_XELEMENT*/assert(false);break;
         // <geometry> records
         case oas_RECTANGLE   : skimRectangle(ofn); break;
         case oas_POLYGON     : skimPolygon(ofn);break;
         case oas_PATH        : skimPath(ofn);break;
         case oas_TRAPEZOID_1 : skimTrapezoid(ofn, 1);break;
         case oas_TRAPEZOID_2 : skimTrapezoid(ofn, 2);break;
         case oas_TRAPEZOID_3 : skimTrapezoid(ofn, 3);break;
         case oas_CTRAPEZOID  : skimCTrapezoid(ofn);break;
         case oas_CIRCLE      : /*@TODO oas_CIRCLE*/assert(false);break;
         default:
            // last byte from the stream doesn't belong to this cell definition
            _cellSize = ofn.filePos() - _filePos - 1;
            return recType;
      }
   } while (true);
}

void Oasis::Cell::import(OasisInFile& ofn, laydata::TdtCell* dst_cell,
                           laydata::TdtLibDir* tdt_db, const LayerMapExt& theLayMap)
{
   ofn.setPosition(_filePos);
   initModals();
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
         case oas_XYRELATIVE  : _mod_xymode = md_relative;break;
         case oas_XYABSOLUTE  : _mod_xymode = md_absolute;break;
         case oas_CBLOCK      : ofn.inflateCBlock(); break;
         // <element> records
         case oas_PLACEMENT_1 : readReference(ofn, dst_cell, tdt_db, false);break;
         case oas_PLACEMENT_2 : readReference(ofn, dst_cell, tdt_db, true );break;
         case oas_TEXT        : readText(ofn, dst_cell, theLayMap);break;
         case oas_XELEMENT    : /*@TODO oas_XELEMENT*/assert(false);break;
         // <geometry> records
         case oas_RECTANGLE   : readRectangle(ofn, dst_cell, theLayMap); break;
         case oas_POLYGON     : readPolygon(ofn, dst_cell, theLayMap);break;
         case oas_PATH        : readPath(ofn, dst_cell, theLayMap);break;
         case oas_TRAPEZOID_1 : readTrapezoid(ofn, dst_cell, theLayMap, 1);break;
         case oas_TRAPEZOID_2 : readTrapezoid(ofn, dst_cell, theLayMap, 2);break;
         case oas_TRAPEZOID_3 : readTrapezoid(ofn, dst_cell, theLayMap, 3);break;
         case oas_CTRAPEZOID  : readCTrapezoid(ofn, dst_cell, theLayMap);break;
         case oas_CIRCLE      : /*@TODO oas_CIRCLE*/assert(false);break;
         default:
            // check that the cell size is the same as obtained by skim function
            assert(_cellSize == (ofn.filePos() - _filePos - 1));
            dst_cell->fixUnsorted();
            return;
      }
   } while (true);
}

void Oasis::Cell::collectLayers(ExtLayers& layers_map, bool hier)
{
   for (ExtLayers::const_iterator CL = _contSummary.begin(); CL != _contSummary.end(); CL++)
   {
      WordSet& data_types = layers_map[CL->first];
      data_types.insert(CL->second.begin(), CL->second.end());
   }
   if (!hier) return;
   for (OasisCellList::const_iterator CSTR = _children.begin(); CSTR != _children.end(); CSTR++)
      if (NULL == (*CSTR)) continue;
   else
      (*CSTR)->collectLayers(layers_map, hier);
}

//------------------------------------------------------------------------------
void Oasis::Cell::readRectangle(OasisInFile& ofn, laydata::TdtCell* dst_cell, const LayerMapExt& theLayMap)
{
   const byte Smask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;
   word       tdtlaynum;

   byte info = ofn.getByte();

   if ((info & Smask) && (info & Hmask))
      ofn.exception("S&H masks are ON simultaneously in rectangle info byte (25.7)");
   if (info & Lmask) _mod_layer    = ofn.getUnsignedInt(4);
   if (info & Dmask) _mod_datatype = ofn.getUnsignedInt(2);
   if (info & Wmask) _mod_gwidth   = ofn.getUnsignedInt(4);
   if (info & Hmask) _mod_gheight  = ofn.getUnsignedInt(4);
   else if (info & Smask) _mod_gheight  = _mod_gwidth();
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_gx = ofn.getInt(8);
      else /*md_relative*/              _mod_gx = ofn.getInt(8) + _mod_gx();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_gy = ofn.getInt(8);
      else /*md_relative*/              _mod_gy = ofn.getInt(8) + _mod_gy();
   }
   if (info & Rmask) readRepetitions(ofn);

   if ( theLayMap.getTdtLay(tdtlaynum, _mod_layer(), _mod_datatype() ) )
   {
      laydata::QTreeTmp* dwl = dst_cell->secureUnsortedLayer(tdtlaynum);
      if ((0 == _mod_gwidth()) || (0 == _mod_gheight()))
      {
         std::ostringstream winfo;
         winfo << "OASIS : Rectangle with zero area encountered. Skipped ...";
         tell_log(console::MT_WARNING, winfo.str());
      }
      else if (info & Rmask)
      {
         int4b* rptpnt = _mod_repete().lcarray();
         assert(rptpnt);
         for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
         {
            TP p1(_mod_gx()+rptpnt[2*rcnt]              ,_mod_gy()+rptpnt[2*rcnt+1]               );
            TP p2(_mod_gx()+rptpnt[2*rcnt]+_mod_gwidth(),_mod_gy()+rptpnt[2*rcnt+1]+_mod_gheight());
            dwl->putBox(p1, p2);
         }
      }
      else
      {
         TP p1(_mod_gx()              , _mod_gy()               );
         TP p2(_mod_gx()+_mod_gwidth(), _mod_gy()+_mod_gheight());
         dwl->putBox(p1, p2);
      }
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readPolygon(OasisInFile& ofn, laydata::TdtCell* dst_cell, const LayerMapExt& theLayMap)
{
   const byte Pmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;
   word       tdtlaynum;

   byte info = ofn.getByte();

   if (info & Lmask) _mod_layer    = ofn.getUnsignedInt(4);
   if (info & Dmask) _mod_datatype = ofn.getUnsignedInt(2);
   if (info & Pmask) _mod_pplist   = readPointList(ofn);
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_gx = ofn.getInt(8);
      else /*md_relative*/              _mod_gx = ofn.getInt(8) + _mod_gx();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_gy = ofn.getInt(8);
      else /*md_relative*/              _mod_gy = ofn.getInt(8) + _mod_gy();
   }
   if (info & Rmask)  readRepetitions(ofn);

   if ( theLayMap.getTdtLay(tdtlaynum, _mod_layer(), _mod_datatype() ) )
   {
      laydata::QTreeTmp* dwl = dst_cell->secureUnsortedLayer(tdtlaynum);
      if (info & Rmask)
      {
         int4b* rptpnt = _mod_repete().lcarray();
         assert(rptpnt);
         for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
         {
            pointlist laypl;
            _mod_pplist().calcPoints(laypl, _mod_gx()+rptpnt[2*rcnt],_mod_gy()+rptpnt[2*rcnt+1]);
            laydata::ValidPoly check(laypl);
            if (!check.valid())
            {
               std::ostringstream ost;
               ost << "Polygon check fails - {" << check.failType()
                   << " Layer: " << _mod_layer()
                   << " Data type: " << _mod_datatype()
                   << " }";
               tell_log(console::MT_ERROR, ost.str());
            }
            dwl->putPoly(laypl);
         }
      }
      else
      {
         pointlist laypl;
         _mod_pplist().calcPoints(laypl, _mod_gx(),_mod_gy());
         laydata::ValidPoly check(laypl);
         if (!check.valid())
         {
            std::ostringstream ost;
            ost << "Polygon check fails - {" << check.failType()
                << " Layer: " << _mod_layer()
                << " Data type: " << _mod_datatype()
                << " }";
            tell_log(console::MT_ERROR, ost.str());
         }
         dwl->putPoly(laypl);
      }
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::readPath(OasisInFile& ofn, laydata::TdtCell* dst_cell, const LayerMapExt& theLayMap)
{
   const byte Emask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Pmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;
   word       tdtlaynum;

   byte info = ofn.getByte();

   if (info & Lmask) _mod_layer    = ofn.getUnsignedInt(4);
   if (info & Dmask) _mod_datatype = ofn.getUnsignedInt(2);
   if (info & Wmask) _mod_pathhw   = ofn.getUnsignedInt(4);
   if (info & Emask) readExtensions(ofn);
   if (info & Pmask) _mod_wplist   = readPointList(ofn);
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_gx = ofn.getInt(8);
      else /*md_relative*/              _mod_gx = ofn.getInt(8) + _mod_gx();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_gy = ofn.getInt(8);
      else /*md_relative*/              _mod_gy = ofn.getInt(8) + _mod_gy();
   }
   if (info & Rmask) readRepetitions(ofn);

   if ( theLayMap.getTdtLay(tdtlaynum, _mod_layer(), _mod_datatype() ) )
   {
      laydata::QTreeTmp* dwl = dst_cell->secureUnsortedLayer(tdtlaynum);
      if (0 == _mod_pathhw())
      {
         std::ostringstream winfo;
         winfo << "OASIS : Path with zero width encountered. Skipped ...";
         tell_log(console::MT_WARNING, winfo.str());
      }
      else if (info & Rmask)
      {
         if (0 != _mod_pathhw())
         {
            int4b* rptpnt = _mod_repete().lcarray();
            assert(rptpnt);
            for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
            {
               pointlist laypl;
               _mod_wplist().calcPoints(laypl, _mod_gx()+rptpnt[2*rcnt], _mod_gy()+rptpnt[2*rcnt+1], false);
               bool pathConvertResult = true;
               if (info & Emask)
               {
                  int4b exts = _mod_exs().getExtension(_mod_pathhw());
                  int4b exte = _mod_exe().getExtension(_mod_pathhw());
                  if ( (0 != exts) || (0 != exte) )
                     pathConvertResult = laydata::pathConvert(laypl, laypl.size(), exts, exte);
               }
               if (pathConvertResult)
               {
                  laydata::ValidWire check(laypl, 2*_mod_pathhw());
                  if (!check.valid())
                  {
                     std::ostringstream ost;
                     ost << "Wire check fails - {" << check.failType()
                           << " Layer: " << _mod_layer()
                           << " Data type: " << _mod_datatype()
                           << " }";
                     tell_log(console::MT_ERROR, ost.str());
                  }
                  dwl->putWire(laypl, 2*_mod_pathhw());
               }
               else
               {
                  std::ostringstream ost;
                  ost << "Invalid single point path - { Layer: " << _mod_layer()
                        << " Data type: " << _mod_datatype()
                        << " }";
                  tell_log(console::MT_ERROR, ost.str());
               }
            }
         }
      }
      else
      {
         pointlist laypl;
         _mod_wplist().calcPoints(laypl, _mod_gx(), _mod_gy(), false);
         bool pathConvertResult = true;
         if (info & Emask)
         {
            int4b exts = _mod_exs().getExtension(_mod_pathhw());
            int4b exte = _mod_exe().getExtension(_mod_pathhw());
            if ( (0 != exts) || (0 != exte) )
               pathConvertResult = laydata::pathConvert(laypl, laypl.size(), exts, exte);
         }
         if (pathConvertResult)
         {
            laydata::ValidWire check(laypl, 2*_mod_pathhw());
            if (!check.valid())
            {
               std::ostringstream ost;
               ost << "Wire check fails - {" << check.failType()
                     << " Layer: " << _mod_layer()
                     << " Data type: " << _mod_datatype()
                     << " }";
               tell_log(console::MT_ERROR, ost.str());
            }
            dwl->putWire(laypl, 2*_mod_pathhw());
         }
         else
         {
            std::ostringstream ost;
            ost << "Invalid single point path - { Layer: " << _mod_layer()
                  << " Data type: " << _mod_datatype()
                  << " }";
            tell_log(console::MT_ERROR, ost.str());
         }
      }
   }
}

void Oasis::Cell::readTrapezoid(OasisInFile& ofn, laydata::TdtCell* dst_cell, const LayerMapExt& theLayMap, byte type)
{
   const byte Omask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;
   word       tdtlaynum;
   dword      deltaA  = 0;
   dword      deltaB  = 0;

   byte info = ofn.getByte();

   if (info & Lmask) _mod_layer    = ofn.getUnsignedInt(4);
   if (info & Dmask) _mod_datatype = ofn.getUnsignedInt(2);
   if (info & Wmask) _mod_gwidth   = ofn.getUnsignedInt(4);
   if (info & Hmask) _mod_gheight  = ofn.getUnsignedInt(4);
   switch (type)
   {
      case 1: deltaA = ofn.getUnsignedInt(4);
              deltaB = ofn.getUnsignedInt(4);break;
      case 2: deltaA = ofn.getUnsignedInt(4);break;
      case 3: deltaB = ofn.getUnsignedInt(4);break;
      default: assert(false);
   }
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_gx = ofn.getInt(8);
      else /*md_relative*/              _mod_gx = ofn.getInt(8) + _mod_gx();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_gy = ofn.getInt(8);
      else /*md_relative*/              _mod_gy = ofn.getInt(8) + _mod_gy();
   }
   if (info & Rmask) readRepetitions(ofn);

   if ( theLayMap.getTdtLay(tdtlaynum, _mod_layer(), _mod_datatype() ) )
   {
      laydata::QTreeTmp* dwl = dst_cell->secureUnsortedLayer(tdtlaynum);
      if (info & Rmask)
      {
         int4b* rptpnt = _mod_repete().lcarray();
         assert(rptpnt);
         for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
         {
            pointlist laypl;
            int8b p1xr = _mod_gx() + rptpnt[2*rcnt];
            int8b p1yr = _mod_gy() + rptpnt[2*rcnt+1];
            if (info & Omask)
            {// verticaly oriented
               laypl.push_back(TP(p1xr                 , p1yr                          )); // P
               laypl.push_back(TP(p1xr                 , p1yr + _mod_gheight()         )); // Q
               laypl.push_back(TP(p1xr + _mod_gwidth() , p1yr + _mod_gheight() - deltaB)); // S
               laypl.push_back(TP(p1xr + _mod_gwidth() , p1yr                  - deltaA)); // R
            }
            else
            { // horizontaly oriented
               laypl.push_back(TP(p1xr                         , p1yr + _mod_gheight())); // P
               laypl.push_back(TP(p1xr + _mod_gwidth()         , p1yr + _mod_gheight())); // Q
               laypl.push_back(TP(p1xr + _mod_gwidth() - deltaB, p1yr                 )); // S
               laypl.push_back(TP(p1xr                 - deltaA, p1yr                 )); // R
            }
            dwl->putPoly(laypl);
         }
      }
      else
      {
         pointlist laypl;
         if (info & Omask)
         {// verticaly oriented
            laypl.push_back(TP(_mod_gx()                , _mod_gy()                          )); // P
            laypl.push_back(TP(_mod_gx()                , _mod_gy() + _mod_gheight()         )); // Q
            laypl.push_back(TP(_mod_gx() + _mod_gwidth(), _mod_gy() + _mod_gheight() - deltaB)); // S
            laypl.push_back(TP(_mod_gx() + _mod_gwidth(), _mod_gy()                  - deltaA)); // R
         }
         else
         { // horizontaly oriented
            laypl.push_back(TP(_mod_gx()                         , _mod_gy() + _mod_gheight())); // P
            laypl.push_back(TP(_mod_gx() + _mod_gwidth()         , _mod_gy() + _mod_gheight())); // Q
            laypl.push_back(TP(_mod_gx() + _mod_gwidth() - deltaB, _mod_gy()                 )); // S
            laypl.push_back(TP(_mod_gx()                 - deltaA, _mod_gy()                 )); // R
         }
         dwl->putPoly(laypl);
      }
   }
}

void Oasis::Cell::readCTrapezoid(OasisInFile& ofn, laydata::TdtCell* dst_cell, const LayerMapExt& theLayMap)
{
   const byte Tmask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;
   word       tdtlaynum;
   std::ostringstream error;

   byte info = ofn.getByte();

   if (info & Lmask) _mod_layer    = ofn.getUnsignedInt(4);
   if (info & Dmask) _mod_datatype = ofn.getUnsignedInt(2);
   if (info & Tmask) _mod_trpztype = ofn.getUnsignedInt(4);
   if (info & Wmask)
   {
      _mod_gwidth   = ofn.getUnsignedInt(4);
      if ( (20 == _mod_trpztype())                             ||
           (21 == _mod_trpztype())                               )
      {
         error << "W flag is 1 for CTRAPEZOID of type"  << _mod_trpztype() << " (28.8)";
         ofn.exception(error.str());
      }
   }
   if (info & Hmask)
   {
      _mod_gheight  = ofn.getUnsignedInt(4);
      if ( ((16 <= _mod_trpztype()) && (_mod_trpztype() <= 19)) ||
            (22 == _mod_trpztype())                             ||
            (23 == _mod_trpztype())                             ||
            (25 == _mod_trpztype())                               )
      {
         error << "H flag is 1 for CTRAPEZOID of type"  << _mod_trpztype() << " (28.8)";
         ofn.exception(error.str());
      }
   }
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_gx = ofn.getInt(8);
      else /*md_relative*/              _mod_gx = ofn.getInt(8) + _mod_gx();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_gy = ofn.getInt(8);
      else /*md_relative*/              _mod_gy = ofn.getInt(8) + _mod_gy();
   }
   if (info & Rmask) readRepetitions(ofn);

   if ( theLayMap.getTdtLay(tdtlaynum, _mod_layer(), _mod_datatype() ) )
   {
      laydata::QTreeTmp* dwl = dst_cell->secureUnsortedLayer(tdtlaynum);
      if (info & Rmask)
      {
         //read the repetition record from the input stream
         int4b* rptpnt = _mod_repete().lcarray();
         assert(rptpnt);
         for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
         {
            pointlist laypl;
            genCTrapezoids(ofn, laypl,
                           _mod_gx()+ rptpnt[2*rcnt]  ,
                           _mod_gy()+ rptpnt[2*rcnt+1],
                           (info & Wmask) ? _mod_gwidth()  : 0,
                           (info & Hmask) ? _mod_gheight() : 0,
                           _mod_trpztype()             );
            dwl->putPoly(laypl);
         }

      }
      else
      {
         pointlist laypl;
         genCTrapezoids(ofn, laypl      ,
                        _mod_gx()       ,
                        _mod_gy()       ,
                        (info & Wmask) ? _mod_gwidth()  : 0,
                        (info & Hmask) ? _mod_gheight() : 0,
                        _mod_trpztype()  );
         dwl->putPoly(laypl);
      }
   }
}
//------------------------------------------------------------------------------
void Oasis::Cell::readText(OasisInFile& ofn, laydata::TdtCell* dst_cell, const LayerMapExt& theLayMap)
{
   const byte Cmask   = 0x40;
   const byte Nmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Tmask   = 0x02; // In the standard is T, but it looks like a typo
   const byte Lmask   = 0x01;
   word       tdtlaynum;

   byte info = ofn.getByte();
   if (info & Cmask) _mod_text      = ofn.getTextRefName(info & Nmask);
   if (info & Lmask) _mod_tlayer    = ofn.getUnsignedInt(4);
   if (info & Tmask) _mod_tdatatype = ofn.getUnsignedInt(2);
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_tx = ofn.getInt(8);
      else /*md_relative*/              _mod_tx = ofn.getInt(8) + _mod_tx();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_ty = ofn.getInt(8);
      else /*md_relative*/              _mod_ty = ofn.getInt(8) + _mod_ty();
   }
   if (info & Rmask) readRepetitions(ofn);
   //
   if ( theLayMap.getTdtLay(tdtlaynum, _mod_tlayer(), _mod_tdatatype() ) )
   {
      laydata::QTreeTmp* dwl = dst_cell->secureUnsortedLayer(tdtlaynum);
      if (info & Rmask)
      {
         int4b* rptpnt = _mod_repete().lcarray();
         assert(rptpnt);
         for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
         {
            TP p1(_mod_tx()+rptpnt[2*rcnt],_mod_ty()+rptpnt[2*rcnt+1]);
            dwl->putText( _mod_text(),CTM( p1  ,
                                           1.0 / (1e-3 *  OPENGL_FONT_UNIT) , // @FIXME! Font size!
                                           0.0 ,
                                           false
                                         )
                        );
         }
      }
      else
      {
         TP p1(_mod_tx(),_mod_ty());
         dwl->putText( _mod_text(),CTM( p1,
                                        1.0 / (1e-3 *  OPENGL_FONT_UNIT) , // @FIXME! Font size!
                                        0.0 ,
                                        false
                                      )
                     );
      }
   }
}

void Oasis::Cell::readReference(OasisInFile& ofn, laydata::TdtCell* dst_cell,
                                laydata::TdtLibDir* tdt_db, bool exma)
{
   const byte Cmask   = 0x80;
   const byte Nmask   = 0x40;
   const byte Xmask   = 0x20;
   const byte Ymask   = 0x10;
   const byte Rmask   = 0x08;
   const byte Mmask   = 0x04;
   const byte Amask   = 0x02;
   const byte Fmask   = 0x01;
   real       angle, magnification;

   byte info = ofn.getByte();

   if (info & Cmask) _mod_cellref  = ofn.getCellRefName(info & Nmask);
   if (exma)
   {
      angle         = (info & Amask) ? ofn.getReal() : 0.0;
      magnification = (info & Mmask) ? ofn.getReal() : 1.0;
   }
   else
   {
      angle         = 90.0 * (real)((info & (Mmask | Amask)) >> 1);
      magnification = 1.0;
   }
   if (magnification <= 0)
         ofn.exception("Bad magnification value (22.10)");
   if (info & Xmask)
   {
      if (md_absolute == _mod_xymode()) _mod_px = ofn.getInt(8);
      else /*md_relative*/              _mod_px = ofn.getInt(8) + _mod_px();
   }
   if (info & Ymask)
   {
      if (md_absolute == _mod_xymode()) _mod_py = ofn.getInt(8);
      else /*md_relative*/              _mod_py = ofn.getInt(8) + _mod_py();
   }

   if (info & Rmask) readRepetitions(ofn);
   //
   laydata::CellDefin strdefn = tdt_db->linkCellRef(_mod_cellref(), TARGETDB_LIB);
   if (info & Rmask)
   {
      int4b* rptpnt = _mod_repete().lcarray();
      assert(rptpnt);
      for (dword rcnt = 0; rcnt < _mod_repete().bcount(); rcnt++)
      {
         TP p1(_mod_px()+rptpnt[2*rcnt],_mod_py()+rptpnt[2*rcnt+1]);
         dst_cell->registerCellRef( strdefn,
                                    CTM(p1,
                                        magnification,
                                        angle,
                                        (info & Fmask)
                                       ),
                                       false
                                  );
      }
   }
   else
   {
      TP p1(_mod_px(),_mod_py());
      dst_cell->registerCellRef( strdefn,
                                 CTM(p1,
                                     magnification,
                                     angle,
                                     (info & Fmask)
                                    ),
                                    false
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
   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   updateContents(layno, dtype);
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

   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   updateContents(layno, dtype);
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

   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   updateContents(layno, dtype);
   if (info & Wmask) ofn.getUnsignedInt(4);
   if (info & Emask) readExtensions(ofn);
   if (info & Pmask) readPointList(ofn);
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimTrapezoid(OasisInFile& ofn, byte type)
{
// const byte Omask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   updateContents(layno, dtype);
   if (info & Wmask) ofn.getUnsignedInt(4);
   if (info & Hmask) ofn.getUnsignedInt(4);
   switch (type)
   {
      case 1: ofn.getUnsignedInt(4);ofn.getUnsignedInt(4);break;
      case 2: ofn.getUnsignedInt(4);break;
      case 3: ofn.getUnsignedInt(4);break;
      default: assert(false);
   }
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimCTrapezoid(OasisInFile& ofn)
{
   const byte Tmask   = 0x80;
   const byte Wmask   = 0x40;
   const byte Hmask   = 0x20;
   const byte Xmask   = 0x10;
   const byte Ymask   = 0x08;
   const byte Rmask   = 0x04;
   const byte Dmask   = 0x02;
   const byte Lmask   = 0x01;

   byte info = ofn.getByte();

   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   updateContents(layno, dtype);
   if (info & Tmask) ofn.getUnsignedInt(4);
   if (info & Wmask) ofn.getUnsignedInt(4);
   if (info & Hmask) ofn.getUnsignedInt(4);
   if (info & Xmask) ofn.getInt(8);
   if (info & Ymask) ofn.getInt(8);
   if (info & Rmask) readRepetitions(ofn);
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
   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   updateContents(layno, dtype);
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
// const byte Fmask   = 0x01;

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
void Oasis::Cell::genCTrapezoids(OasisInFile& ofn, pointlist& laypl,
      int4b gx, int4b gy, int4b width, int4b height, word ctype)
{
   dword      delta  = 0;
   std::ostringstream info;
   switch (ctype)
   {
      case  0:
      case  1:
      case  2:
      case  3:
      case  6:
      case  7: if (width < height)
               {
                  info << "(w < h) in CTRAPEZOID of type "  << ctype << " (28.8)";
                  ofn.exception(info.str());
               }
               else
                  delta = width - height;
               break;
      case  4:
      case  5: if (width < 2 * height)
               {
                  info << "(w < 2*h) in CTRAPEZOID of type "  << ctype << " (28.8)";
                  ofn.exception(info.str());
               }
               else
                  delta = width/2 - height;
               break;
      case  8:
      case  9:
      case 10:
      case 11:
      case 14:
      case 15: if (height < width)
              {
                 info << "(h < w) in CTRAPEZOID of type "  << ctype << " (28.8)";
                 ofn.exception(info.str());
              }
              else
                 delta = height - width;
              break;
      case 12:
      case 13: if (height < 2 * width)
               {
                  info << "(h < 2*w) in CTRAPEZOID of type "  << ctype << " (28.8)";
                  ofn.exception(info.str());
               }
               else
                  delta = height/2 - width;
   }

   switch (ctype)
   {
      case  0:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width -delta , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case  1:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width -delta , gy                 ));
         break;
      case  2:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx         +delta , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case  3:
         laypl.push_back(TP(gx         +delta , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case  4:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx         +delta , gy + height        ));
         laypl.push_back(TP(gx + width -delta , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case  5:
         laypl.push_back(TP(gx         +delta , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width -delta , gy                 ));
         break;
      case  6:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx         +delta , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width -delta , gy                 ));
         break;
      case  7:
         laypl.push_back(TP(gx         +delta , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width -delta , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case  8:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height -delta ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case  9:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height -delta ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 10:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy          +delta ));
         break;
      case 11:
         laypl.push_back(TP(gx                , gy          +delta ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 12:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height -delta ));
         laypl.push_back(TP(gx + width        , gy          +delta ));
         break;
      case 13:
         laypl.push_back(TP(gx                , gy          +delta ));
         laypl.push_back(TP(gx                , gy + height -delta ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 14:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height -delta ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy          +delta ));
         break;
      case 15:
         laypl.push_back(TP(gx                , gy          +delta ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height -delta ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      //------------------------------------------------ triangles -----
      case 16:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + width         ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 17:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + width         ));
         laypl.push_back(TP(gx + width        , gy + width         ));
         break;
      case 18:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx + width        , gy + width         ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 19:
         laypl.push_back(TP(gx                , gy + width         ));
         laypl.push_back(TP(gx + width        , gy + width         ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 20:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx + height       , gy + height        ));
         laypl.push_back(TP(gx + height * 2   , gy                 ));
         break;
      case 21:
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + height * 2   , gy + height        ));
         laypl.push_back(TP(gx + height       , gy                 ));
         break;
      case 22:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + width*2       ));
         laypl.push_back(TP(gx + width        , gy + width         ));
         break;
      case 23:
         laypl.push_back(TP(gx                , gy + width         ));
         laypl.push_back(TP(gx + width        , gy + width*2       ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      //------------------------------------------------- boxes -------
      case 24:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + height        ));
         laypl.push_back(TP(gx + width        , gy + height        ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      case 25:
         laypl.push_back(TP(gx                , gy                 ));
         laypl.push_back(TP(gx                , gy + width         ));
         laypl.push_back(TP(gx + width        , gy + width         ));
         laypl.push_back(TP(gx + width        , gy                 ));
         break;
      default:
         info << "Illegal CTRAPEZOID type "  << ctype << " (28.8)";
         ofn.exception(info.str());
   }
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
void Oasis::Cell::readExtensions(OasisInFile& ofn)
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
   // deal with the end extenstion
   extype = (scheme & 0x03);
   if (0 != extype)
   {
      _mod_exe = PathExtensions(ofn, (ExtensionTypes)extype);
   }
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

void Oasis::Cell::initModals()
{
   _mod_layer.reset();
   _mod_datatype.reset();
   _mod_gwidth.reset();
   _mod_gheight.reset();
   _mod_pathhw.reset();
   _mod_gx = 0;
   _mod_gy = 0;
   _mod_text.reset();
   _mod_tlayer.reset();
   _mod_tdatatype.reset();
   _mod_tx = 0;
   _mod_ty = 0;
   _mod_cellref.reset();
   _mod_px = 0;
   _mod_py = 0;
   _mod_pplist.reset();
   _mod_wplist.reset();
   _mod_repete.reset();
   _mod_exs.reset();
   _mod_exe.reset();
   _mod_xymode = md_absolute;
   _mod_trpztype.reset();
}

void Oasis::Cell::updateContents(int2b layer, int2b dtype)
{
   _contSummary[layer].insert(dtype);
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

Oasis::PointList::PointList(const PointList& plst)
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
   /*@TODO readDoubleDelta*/assert(false);
}

void Oasis::PointList::calcPoints(pointlist& plst, int4b p1x, int4b p1y, bool polyp)
{
   switch (_pltype)
   {
      case dt_manhattanH : calcManhattanH(plst, p1x, p1y, polyp) ; break;
      case dt_manhattanV : calcManhattanV(plst, p1x, p1y, polyp) ; break;
      case dt_mamhattanE : calcManhattanE(plst, p1x, p1y) ; break;
      case dt_octangular : calcOctangular(plst, p1x, p1y) ; break;
      case dt_allangle   : calcAllAngle(plst, p1x, p1y)   ; break;//+1
      case dt_doubledelta: calcDoubleDelta(plst, p1x, p1y); break;//+1
      default: assert(false);
   }
}

void Oasis::PointList::calcManhattanH(pointlist& plst, int4b p1x, int4b p1y, bool polyp)
{
   dword numpoints = (polyp ? _vcount + 2 : _vcount + 1);
   plst.reserve(numpoints);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      if (curp % 2) cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      else          cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      plst.push_back(cpnt);
   }
   if (polyp)
   {
      if (curp % 2) cpnt.setY(p1y);
      else          cpnt.setX(p1x);
      plst.push_back(cpnt);
   }
}

void Oasis::PointList::calcManhattanV(pointlist& plst, int4b p1x, int4b p1y, bool polyp)
{
   dword numpoints = (polyp ? _vcount + 2 : _vcount + 1);
   plst.reserve(numpoints);
   TP cpnt(p1x,p1y);
   plst.push_back(cpnt);
   dword curp;
   for (curp = 0; curp < _vcount; curp++)
   {
      if (curp % 2) cpnt.setX(cpnt.x() + _delarr[2*curp  ]);
      else          cpnt.setY(cpnt.y() + _delarr[2*curp+1]);
      plst.push_back(cpnt);
   }
   if (polyp)
   {
      if (curp % 2) cpnt.setX(p1x);
      else          cpnt.setY(p1y);
      plst.push_back(cpnt);
   }
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
   /*@TODO calcDoubleDelta*/assert(false);
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

void Oasis::Repetitions::readvarX(OasisInFile& ofn)
{//type 4
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   for (dword pj = 1; pj < _bcount; pj++)
   {
      p1x = ofn.getUnsignedInt(4);
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ] + p1x;
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1];
   }
}

void Oasis::Repetitions::readvarXxG(OasisInFile& ofn)
{//type 5
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   dword grid   = ofn.getUnsignedInt(4);
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   for (dword pj = 1; pj < _bcount; pj++)
   {
      p1x = ofn.getUnsignedInt(4);
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ] + p1x * grid;
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1];
   }
}

void Oasis::Repetitions::readvarY(OasisInFile& ofn)
{//type 6
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   for (dword pj = 1; pj < _bcount; pj++)
   {
      p1y = ofn.getUnsignedInt(4);
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ];
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1] + p1y;
   }
}

void Oasis::Repetitions::readvarYxG(OasisInFile& ofn)
{//type 7
   _bcount = ofn.getUnsignedInt(4) + 2;
   _lcarray = DEBUG_NEW int4b[2*_bcount];
   dword grid   = ofn.getUnsignedInt(4);
   int4b p1x = 0;
   int4b p1y = 0;
   _lcarray[0] = p1x;
   _lcarray[1] = p1y;
   for (dword pj = 1; pj < _bcount; pj++)
   {
      p1y = ofn.getUnsignedInt(4);
      _lcarray[2*pj  ] = _lcarray[2*(pj-1)  ];
      _lcarray[2*pj+1] = _lcarray[2*(pj-1)+1] + p1y * grid;
   }
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
Oasis::PathExtensions::PathExtensions(OasisInFile& ofn, ExtensionTypes exType) : _exType(exType)
{
   switch (_exType)
   {
      case ex_flush     :
      case ex_hwidth    : break;
      case ex_explicit  : _exEx = ofn.getInt(2); break;
      default: assert(false);
   }
}

int4b Oasis::PathExtensions::getExtension(int4b hwidth) const
{
   switch (_exType)
   {
      case ex_flush     : return 0;
      case ex_hwidth    : return hwidth;
      case ex_explicit  : return _exEx;
      default: assert(false);
   }
   return 0; // dummy statement
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
Oasis::Oas2Ted::Oas2Ted(OasisInFile* src_lib, laydata::TdtLibDir* tdt_db, const LayerMapExt& theLayMap) :
      _src_lib(src_lib), _tdt_db(tdt_db), _theLayMap(theLayMap),
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
      (*_tdt_db)()->recreateHierarchy(_tdt_db);
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
   laydata::TdtCell* dst_structure = static_cast<laydata::TdtCell*>((*_tdt_db)()->checkCell(gname));
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
      dst_structure = DEBUG_NEW laydata::TdtCell(gname);
      // call the cell converter
      src_structure->import(*_src_lib, dst_structure, _tdt_db, _theLayMap);
      // and finally - register the cell
      (*_tdt_db)()->registerCellRead(gname, dst_structure);
   }
}
// oasisimport("AMODUL", true, false);
