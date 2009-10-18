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
   // add a record to the hash table
}

//===========================================================================
Oasis::OasisInFile::OasisInFile(std::string fn) : _cellNames(NULL), _textStrings(NULL), 
   _propNames(NULL), _propStrings(NULL), _layerNames(NULL), _xNames(NULL),
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
   bool offsetFlag = getUnsignedInt(1);
   if (!offsetFlag)
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

void Oasis::OasisInFile::readLibrary()
{
   // get oas_START
   byte recType = getUnsignedInt(1);
   if (oas_START != recType) throw EXPTNreadOASIS("\"START\" record expected here (13.10)");
   readStartRecord();
   // Some sequences (CELL) does not have an explicit end record. So they end when a record
   // which can not be a member of the difinition appears. The trouble is that the last byte
   // read from (say) readCell should be reused in the loop here
   bool rlb = false;// reuse last byte
   Cell* curCell = NULL;
   do {
      if (!rlb) recType = getUnsignedInt(1);
      switch (recType)
      {  
         case oas_PAD         : rlb = false; break;
         case oas_PROPERTY_1  : /*@TODO*/ rlb = false; break;
         case oas_PROPERTY_2  : /*@TODO*/ rlb = false; break;
         case oas_CELL_1      : curCell = DEBUG_NEW Cell(); recType = curCell->readCell(*this, true) ; rlb = true; break;
         case oas_CELL_2      : curCell = DEBUG_NEW Cell(); recType = curCell->readCell(*this, false); rlb = true; break;
         case oas_CBLOCK      : /*@TODO*/ rlb = false; break;
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
         case oas_XNAME_1     : /*@TODO*/ rlb = false; break;
         case oas_XNAME_2     : /*@TODO*/ rlb = false; break;
         case oas_END         : /*@TODO readEndRecord(); */return;
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
   std::string cellName;
   if (refnum)
   {
      dword cellRefNum = ofn.getUnsignedInt(4);
      // find the cell name in the _cellName hash table by the ref number
   }
   else
   {
      cellName = ofn.getString();
   }
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
         case oas_PLACEMENT_1 : /*@TODO*/assert(false);break;
         case oas_PLACEMENT_2 : /*@TODO*/assert(false);break;
         case oas_TEXT        : /*@TODO*/assert(false);break;
         case oas_XELEMENT    : /*@TODO*/assert(false);break;
         // <geometry> records
         case oas_RECTANGLE   : skimRectangle(ofn); break;
         case oas_POLYGON     : skimPolygon(ofn);break;
         case oas_PATH        : /*@TODO*/assert(false);break;
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
   byte info = ofn.getByte();

   if ((info & Smask) && (info & Hmask))
      throw EXPTNreadOASIS("S&H masks are on simultaneously in rectangle info byte (25.7)");
   dword layno  = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype  = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   dword width  = (info & Wmask) ? (_mod_gwidth   = ofn.getUnsignedInt(4)) : _mod_gwidth();
   dword height = (info & Hmask) ? (_mod_gheight  = ofn.getUnsignedInt(4)) : 
                  (info & Smask) ? (_mod_gheight  = width                ) : _mod_gheight();
   int8b p1x    = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b p1y    = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask)
   {
      assert(false); //@TODO - parse repetition records!
   }
}

//------------------------------------------------------------------------------
void Oasis::Cell::skimPolygon(OasisInFile& ofn)
{
   byte info = ofn.getByte();

   dword layno  = (info & Lmask) ? (_mod_layer    = ofn.getUnsignedInt(4)) : _mod_layer();
   word  dtype  = (info & Dmask) ? (_mod_datatype = ofn.getUnsignedInt(2)) : _mod_datatype();
   //@ TODO ! Get the point list!
   int8b p1x    = (info & Xmask) ? (_mod_gx       = ofn.getInt(8)        ) : _mod_gx();
   int8b p1y    = (info & Ymask) ? (_mod_gy       = ofn.getInt(8)        ) : _mod_gy();
   if (info & Rmask)
   {
      assert(false); //@TODO - parse repetition records!
   }
}

//oasisread("/home/skr/toped/library/oasis/FOLL2.OAS");
