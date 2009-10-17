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
#include "outbox.h"
#include <sstream>


//===========================================================================
Oasis::Table::Table(OasisInFile& ofh)
{
   _strictMode   = ofh.getUnsignedInt();
   _offset       = ofh.getUnsignedInt();
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
   _dbl_word index;
   switch (_ieMode)
   {
      case tblm_implicit: index = _nextIndex++; break;
      case tblm_explicit: index = ofn.getUnsignedInt(); break;
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

void Oasis::OasisInFile::read()
{
   // get oas_START
   _dbl_word recordType;

   recordType = getUnsignedInt();
   if (oas_START != recordType) throw EXPTNreadOASIS("\"START\" record expected here (13.10)");
   readStartRecord();
   do {
      switch (getUnsignedInt())
      {  
         case oas_PAD: break;
//         case oas_PROPERTY_1  :
//         case oas_PROPERTY_2  :
//         case oas_CELL_1      :
//         case oas_CELL_2      :
         case oas_CELLNAME_1  : _cellNames->getTableRecord(*this, tblm_implicit)   ; break;
         case oas_TEXTSTRING_1: _textStrings->getTableRecord(*this, tblm_implicit) ; break;
         case oas_PROPNAME_1  : _propNames->getTableRecord(*this, tblm_implicit)   ; break;
         case oas_LAYERNAME_1 : _layerNames->getTableRecord(*this, tblm_implicit)  ; break;
         case oas_PROPSTRING_1: _propStrings->getTableRecord(*this, tblm_implicit) ; break;
         case oas_CELLNAME_2  : _cellNames->getTableRecord(*this, tblm_explicit)   ; break;
         case oas_TEXTSTRING_2: _textStrings->getTableRecord(*this, tblm_explicit) ; break;
         case oas_PROPNAME_2  : _propNames->getTableRecord(*this, tblm_explicit)   ; break;
         case oas_LAYERNAME_2 : _layerNames->getTableRecord(*this, tblm_explicit)  ; break;
         case oas_PROPSTRING_2: _propStrings->getTableRecord(*this, tblm_explicit) ; break;
//         case oas_XNAME_1     :
//         case oas_XNAME_2     :
//         case oas_CBLOCK      :
         case oas_END : /*readEndRecord(); */return;
         default: throw EXPTNreadOASIS("Unexpected record in the current context");
      }
   } while (true);
}

void Oasis::OasisInFile::readStartRecord()
{
   std::ostringstream info;
   _version = getString();
   info << "OASIS version: \"" << _version << "\"";
   tell_log(console::MT_INFO, info.str());
   _unit = getReal();
   if (0 > _unit) throw EXPTNreadOASIS("Unacceptable \"unit\" value (13.10)");
   bool offsetFlag = getUnsignedInt();
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

_dbl_word Oasis::OasisInFile::getUnsignedInt()
{
   byte        cmask       = 0x7f; // masks the MSB of the byte
   byte        bytecounter = 0   ; // how many bytes were read
   byte        bytein            ; // last byte read from the file stream
   _dbl_word   result      = 0   ; // the result
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
         case 1: btres[0] |= bytein << 7;
                 btres[1]  = (bytein & cmask) >> 1;
                 break;
         case 2: btres[1] |= bytein << 6;
                 btres[2]  = (bytein & cmask) >> 2;
                 break;
         case 3: btres[2] |= bytein << 5;
                 btres[3]  = (bytein & cmask) >> 3;
                 break;
         default: throw EXPTNreadOASIS("Integer is too big (7.2.3)");
      }
      bytecounter++;
   } while (bytein & (~cmask));
   return result;
}

real Oasis::OasisInFile::getReal()
{
   _dbl_word      numerator   = 0;
   _dbl_word      denominator = 1;
   bool           sign        = false;
   float          fres;
   double         dres;
   switch (getUnsignedInt())
   {
      case 0: numerator   = getUnsignedInt(); break;
      case 1: numerator   = getUnsignedInt(); sign = true; break;
      case 2: denominator = getUnsignedInt(); break;
      case 3: denominator = getUnsignedInt(); sign = true; break;
      case 4: numerator   = getUnsignedInt(); denominator = getUnsignedInt(); break;
      case 5: numerator   = getUnsignedInt(); denominator = getUnsignedInt(); sign = true; break;
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

std::string Oasis::OasisInFile::getString()
{
   _dbl_word   length  = getUnsignedInt() ; //string length
   char*       theString = DEBUG_NEW char[length+1];

   if (length != _oasisFh.Read(theString,length))
      throw EXPTNreadOASIS("I/O error during read-in");
   theString[length] = 0x00;
   std::string result(theString);
   delete [] theString;
   return result;
}

Oasis::OasisInFile::~OasisInFile()
{
   if ( _cellNames  ) delete _cellNames;
   if ( _textStrings) delete _textStrings;
   if ( _propNames  ) delete _propNames;
   if ( _propStrings) delete _propStrings;
   if ( _layerNames ) delete _layerNames;
   if ( _xNames     ) delete _xNames;
}
//oasisread("/home/skr/toped/library/oasis/FOLL2.OAS");
