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
#include <sstream>


//===========================================================================
Oasis::Table::Table(OasisInFile& ofh)
{
   _strictMode   = ofh.getUnsignedInt(1);
   _offset       = ofh.getUnsignedInt(8);
   _ieMode       = tblm_unknown;
   _nextIndex    = 0;
}


/*! Reads a single NAME record and adds it to the corresponding name table*/
void  Oasis::Table::getTableRecord(OasisInFile& ofn, TableMode ieMode)
{
   if (_strictMode) throw EXPTNreadOASIS("A stray \"NAME\" record encountered in strict mode (13.10)");
   if (tblm_unknown == _ieMode)
      _ieMode = ieMode;
   else if (_ieMode != ieMode)
      throw EXPTNreadOASIS("Uncompatible record types encountered in \"NAME\" records (15.5,16.4,17.4,18.4)");
   std::string value = ofn.getString();
   dword index;
   switch (_ieMode)
   {
      case tblm_implicit: index = _nextIndex++; break;
      case tblm_explicit: index = ofn.getUnsignedInt(4); break;
      default: assert(false);
   }
   if (_table.end() != _table.find(index))
      throw EXPTNreadOASIS("Name record with this index already exists (15.5,16.4,17.4,18.4)");
   else
      _table[index] = value;
}

std::string Oasis::Table::getName(dword index)
{
   if (_table.end() == _table.find(index))
      throw EXPTNreadOASIS("Name not found in the corresponding table (20.4,...)");
   else
      return _table[index];
}

//===========================================================================
Oasis::OasisInFile::OasisInFile(std::string fn) : _cellNames(NULL), _textStrings(NULL), 
   _propNames(NULL), _propStrings(NULL), _layerNames(NULL), _xNames(NULL), _offsetFlag(false),
   _fileName(fn)
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
//   TpdPost::toped_status(console::TSTS_PRGRSBARON, _fileLength);
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
   if (0 > _unit) throw EXPTNreadOASIS("Unacceptable \"unit\" value (13.10)");
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
   }
}

void Oasis::OasisInFile::readEndRecord()
{
   if (_offsetFlag)
   {
      // table offset structure is stored in the END record (here)
      _cellNames   = DEBUG_NEW Table(*this);
      _textStrings = DEBUG_NEW Table(*this);
      _propNames   = DEBUG_NEW Table(*this);
      _propStrings = DEBUG_NEW Table(*this);
      _layerNames  = DEBUG_NEW Table(*this);
      _xNames      = DEBUG_NEW Table(*this);
   }
   getString(); // <-- padding string
   std::ostringstream info;
   byte valid_scheme = getByte();
   if (2 < valid_scheme)
      throw EXPTNreadOASIS("Unexpected validation scheme type ( not explicitly specified)");
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
   if (oas_START != recType) throw EXPTNreadOASIS("\"START\" record expected here (13.10)");
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
            recType = curCell->readCell(*this, true) ; 
            rlb = true; 
/*TMP*/     delete curCell;
            break;
         case oas_CELL_2      : 
            curCell = DEBUG_NEW Cell(); 
            recType = curCell->readCell(*this, false); 
            rlb = true; 
/*TMP*/     delete curCell;
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
         case oas_END         : readEndRecord(); return;
         default: throw EXPTNreadOASIS("Unexpected record in the current context");
      }
   } while (true);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
byte Oasis::OasisInFile::getByte()
{
   byte        bytein            ; // last byte read from the file stream
   if (1 != _oasisFh.Read(&bytein,1))
      throw EXPTNreadOASIS("I/O error during read-in");
   return bytein;
}
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
      if (1 != _oasisFh.Read(&bytein,1))
         throw EXPTNreadOASIS("I/O error during read-in");
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
         default: throw EXPTNreadOASIS("Integer is too big (7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
   if (bytecounter > length)
      throw EXPTNreadOASIS("Unsigned integer with unexpected length(7.2.3)");
   return result;
}

int8b Oasis::OasisInFile::getInt(byte length)
{
   assert((length > 0) && (length < 9));
   const byte  cmask       = 0x7f; // masks the MSB of the byte
   byte        bytecounter = 0   ; // how many bytes were read
   byte        bytein            ; // last byte read from the file stream
   byte        sign              ;
   int8b       result      = 0   ; // the result
   // the result in array of bytes representation
   byte       *btres       = (byte*)&result; 
   do
   {
      if (1 != _oasisFh.Read(&bytein,1))
         throw EXPTNreadOASIS("I/O error during read-in");
      switch (bytecounter)
      {
         case 0: btres[0]  = (bytein & cmask) >> 1;
                 sign      = bytein << 7;
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
         default: throw EXPTNreadOASIS("Integer is too big (7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
   if (bytecounter > length)
      throw EXPTNreadOASIS("Unsigned integer with unexpected length(7.2.3)");
   btres[7] = sign;
   return result;
}
//------------------------------------------------------------------------------
real Oasis::OasisInFile::getReal()
{
   dword      numerator   = 0;
   dword      denominator = 1;
   bool           sign        = false;
   float          fres;
   double         dres;
   switch (getUnsignedInt(1))
   {
      case 0: numerator   = getUnsignedInt(4); break;
      case 1: numerator   = getUnsignedInt(4); sign = true; break;
      case 2: denominator = getUnsignedInt(4); break;
      case 3: denominator = getUnsignedInt(4); sign = true; break;
      case 4: numerator   = getUnsignedInt(4); denominator = getUnsignedInt(4); break;
      case 5: numerator   = getUnsignedInt(4); denominator = getUnsignedInt(4); sign = true; break;
      case 6: if (4 != _oasisFh.Read(&fres,4)) throw EXPTNreadOASIS("I/O error during read-in");
              return fres;
      case 7: if (8 != _oasisFh.Read(&dres,8)) throw EXPTNreadOASIS("I/O error during read-in");
              return dres;
      default: throw EXPTNreadOASIS("Unexpected \"real\" type.(7.3.3)");
   }
   if (0 == denominator) throw EXPTNreadOASIS("Denominator is 0 in \"real\" representation (7.3.3)");
   real result = (sign) ? - ((real) numerator / (real) denominator) :
                            ((real) numerator / (real) denominator)   ;
   return result;
}

//------------------------------------------------------------------------------
std::string Oasis::OasisInFile::getString()
{
   dword   length  = getUnsignedInt(2) ; //string length
   char*       theString = DEBUG_NEW char[length+1];

   if (length != _oasisFh.Read(theString,length))
      throw EXPTNreadOASIS("I/O error during read-in");
   theString[length] = 0x00;
   std::string result(theString);
   delete [] theString;
   return result;
}

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
Oasis::OasisInFile::~OasisInFile()
{
   if ( _cellNames  ) delete _cellNames;
   if ( _textStrings) delete _textStrings;
   if ( _propNames  ) delete _propNames;
   if ( _propStrings) delete _propStrings;
   if ( _layerNames ) delete _layerNames;
   if ( _xNames     ) delete _xNames;
}


//==============================================================================
Oasis::Cell::Cell()
{
   // See spec chap.10 (page 11). The modal variables below have to be 
   // initialised to 0
   _mod_gx = 0;
   _mod_gy = 0;
   
}

byte Oasis::Cell::readCell(OasisInFile& ofn, bool refnum)
{
   std::string cellName = ofn.getCellRefName(refnum);
   std::ostringstream info;
   info << "OASIS : Reading cell \"" << cellName << "\"";
   tell_log(console::MT_INFO, info.str());
   do
   {
      byte recType = ofn.getUnsignedInt(1);
      switch (recType)
      {
         case oas_PAD         : break;
         case oas_PROPERTY_1  : /*@TODO*/assert(false);break;
         case oas_PROPERTY_2  : /*@TODO*/assert(false);break;
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
         default: return recType;
      }
   } while (true);
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
      throw EXPTNreadOASIS("S&H masks are ON simultaneously in rectangle info byte (25.7)");
   dword layno       = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype       = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   dword width       = (info & Wmask) ? (_mod_gwidth   = ofn.getUnsignedInt(4)) : _mod_gwidth();
   dword height      = (info & Hmask) ? (_mod_gheight  = ofn.getUnsignedInt(4)) : 
                       (info & Smask) ? (_mod_gheight  = width                ) : _mod_gheight();
   int8b p1x         = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b p1y         = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask) skimRepetitions(ofn);
//   Repetitions rpt = _mod_repete();
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

   dword     layno   = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word      dtype   = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   PointList plist   = (info & Pmask) ? (_mod_pplist   = skimPointList(ofn)   ) : _mod_pplist();
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask) skimRepetitions(ofn);
//   Repetitions rpt = _mod_repete();
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

   dword     layno   = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word      dtype   = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   word      hwidth  = (info & Wmask) ? (_mod_pathhw   = ofn.getUnsignedInt(2)) : _mod_pathhw();
   PathExtensions exs,exe;
   if (info & Emask)
   {
      skimExtensions(ofn, exs, exe);
   }
   else
   {
      exs = _mod_exs();
      exe = _mod_exe();
   }
   PointList plist   = (info & Pmask) ? (_mod_wplist   = skimPointList(ofn)   ) : _mod_wplist();
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask) skimRepetitions(ofn);
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
   std::string text  = (info & Cmask) ? (_mod_text     = ofn.getTextRefName(info & Nmask)) :
                                                                                  _mod_text();
   dword     layno   = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word      dtype   = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask) skimRepetitions(ofn);
//   Repetitions rpt = _mod_repete();
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
   bool flip = (info & Fmask);
   real angle, magnification;
   if (exma)
   { 
      angle = (info & Amask) ? ofn.getReal() : 0.0;
      magnification = (info & Mmask) ? ofn.getReal() : 1.0;
   }
   else
   {
      byte angle = 90.0 * (real)((info & (Mmask | Amask)) >> 1);
      magnification = 1.0;
   }
   if (magnification <= 0)
         throw EXPTNreadOASIS("Bad magnification value (22.10)");
   int8b     p1x     = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b     p1y     = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();

   if (info & Rmask) skimRepetitions(ofn);
//   Repetitions rpt = _mod_repete();
}

//------------------------------------------------------------------------------
Oasis::PointList Oasis::Cell::skimPointList(OasisInFile& ofn)
{
   byte plty = ofn.getByte();
   if (plty >= dt_unknown)
      throw EXPTNreadOASIS("Bad point list type (7.7.8)");
   else
      return PointList(ofn, (PointListType)plty);
//   {
//      PointList* result = DEBUG_NEW PointList(ofn, (PointListType)plty);
//      return *result;
//   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimRepetitions(OasisInFile& ofn)
{
   byte rpty = ofn.getByte();
   if (rpty >= rp_unknown)
      throw EXPTNreadOASIS("Bad repetition type (7.6.14)");
   else if (0 != rpty)
   {
      _mod_repete = Repetitions(ofn, (RepetitionTypes)rpty);
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimExtensions(OasisInFile& ofn, PathExtensions& exs, PathExtensions& exe)
{
   byte scheme = ofn.getByte();
   if (scheme & 0xF0)
      throw EXPTNreadOASIS("Bad extention type (27.? - not explicitly ruled-out)");
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

//==============================================================================
Oasis::PointList::PointList(OasisInFile& ofn, PointListType pltype) : _pltype(pltype)
{
   _vcount = ofn.getUnsignedInt(4);
   _delarr = DEBUG_NEW int4b[2*_vcount];
   switch (_pltype)
   {
      case dt_manhattanH : readManhattanH(ofn) ; break;
      case dt_manhattanV : readManhattanV(ofn) ; break;
      case dt_mamhattanE : readManhattanE(ofn) ; break;
      case dt_octangular : readOctangular(ofn) ; break;
      case dt_allangle   : readAllAngle(ofn)   ; break;
      case dt_doubledelta: readDoubleDelta(ofn); break;
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
      data      = ofb.getUnsignedInt(8);
      direction = (DeltaDirections)(bdata[0] & 0x03);
      switch (direction)
      {
         case dr_east : _delarr[2*ccrd] = (data >> 2); _delarr[2*ccrd+1] = 0          ; break;
         case dr_north: _delarr[2*ccrd] = 0          ; _delarr[2*ccrd+1] = (data >> 2); break;
         case dr_west : _delarr[2*ccrd] =-(data >> 2); _delarr[2*ccrd+1] = 0          ; break;
         case dr_south: _delarr[2*ccrd] = 0          ; _delarr[2*ccrd+1] =-(data >> 2); break;
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
      data      = ofb.getUnsignedInt(8);
      direction = (DeltaDirections)(bdata[0] & 0x07);
      switch (direction)
      {
         case dr_east     : _delarr[2*ccrd] = (data >> 3); _delarr[2*ccrd+1] = 0          ; break;
         case dr_north    : _delarr[2*ccrd] = 0          ; _delarr[2*ccrd+1] = (data >> 3); break;
         case dr_west     : _delarr[2*ccrd] =-(data >> 3); _delarr[2*ccrd+1] = 0          ; break;
         case dr_south    : _delarr[2*ccrd] = 0          ; _delarr[2*ccrd+1] =-(data >> 3); break;
         case dr_northeast: _delarr[2*ccrd] = (data >> 3); _delarr[2*ccrd+1] = (data >> 3); break;
         case dr_northwest: _delarr[2*ccrd] =-(data >> 3); _delarr[2*ccrd+1] = (data >> 3); break;
         case dr_southeast: _delarr[2*ccrd] = (data >> 3); _delarr[2*ccrd+1] =-(data >> 3); break;
         case dr_southwest: _delarr[2*ccrd] =-(data >> 3); _delarr[2*ccrd+1] =-(data >> 3); break;
         default: assert(false);
      }
   }
}

void Oasis::PointList::readAllAngle(OasisInFile& ofb)
{
   for (dword ccrd = 0; ccrd < _vcount; ccrd++)
   {
      readDelta(ofb, _delarr[2*ccrd], _delarr[2*ccrd+1]);
   }
}

void Oasis::PointList::readDoubleDelta(OasisInFile& ofb)
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
         _lcarray[yi*countx+xi  ] = p1x;
         _lcarray[yi*countx+xi+1] = p1y;
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
      _lcarray[xi  ] = p1x;
      _lcarray[xi+1] = p1y;
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
      _lcarray[yi  ] = p1x;
      _lcarray[yi+1] = p1y;
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
         _lcarray[mi*countn + nj    ] = p1x;
         _lcarray[mi*countn + nj + 1] = p1y;
         p1x += nx; p1y += ny;
      }
      s1x += mx; s1y += my;
   }
   
}

void Oasis::Repetitions::readregDia1D(OasisInFile&)
{//type 9
   /*@TODO*/assert(false);
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
      if (bdata[0] & 0x02) deltaX =-(data >> 2);
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
         case dr_west     : deltaX =-(data >> 4); deltaY = 0          ; break;
         case dr_south    : deltaX = 0          ; deltaY =-(data >> 4); break;
         case dr_northeast: deltaX = (data >> 4); deltaY = (data >> 4); break;
         case dr_northwest: deltaX =-(data >> 4); deltaY = (data >> 4); break;
         case dr_southeast: deltaX = (data >> 4); deltaY =-(data >> 4); break;
         case dr_southwest: deltaX =-(data >> 4); deltaY =-(data >> 4); break;
         default: assert(false);
      }
   }
}
//oasisread("/home/skr/toped/library/oasis/FOLL2.OAS");
//oasisread("/home/skr/toped/library/oasis/FOLL.oas");
//oasisread("C:\Users\skr\Development\toped\FOLL2.OAS");
//oasisread("C:\Users\skr\Development\toped\FOLL.oas");
