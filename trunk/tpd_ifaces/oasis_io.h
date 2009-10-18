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
#ifndef OASIS_H_INCLUDED
#define OASIS_H_INCLUDED

#include <wx/ffile.h>
#include "outbox.h"

namespace Oasis {
   const byte oas_PAD              =  0;
   const byte oas_START            =  1;
   const byte oas_END              =  2;
   const byte oas_CELLNAME_1       =  3;
   const byte oas_CELLNAME_2       =  4;
   const byte oas_TEXTSTRING_1     =  5;
   const byte oas_TEXTSTRING_2     =  6;
   const byte oas_PROPNAME_1       =  7;
   const byte oas_PROPNAME_2       =  8;
   const byte oas_PROPSTRING_1     =  9;
   const byte oas_PROPSTRING_2     = 10;
   const byte oas_LAYERNAME_1      = 11;
   const byte oas_LAYERNAME_2      = 12;
   const byte oas_CELL_1           = 13;
   const byte oas_CELL_2           = 14;
   const byte oas_XYABSOLUTE       = 15;
   const byte oas_XYRELATIVE       = 16;
   const byte oas_PLACEMENT_1      = 17;
   const byte oas_PLACEMENT_2      = 18;
   const byte oas_TEXT             = 19;
   const byte oas_RECTANGLE        = 20;
   const byte oas_POLYGON          = 21;
   const byte oas_PATH             = 22;
   const byte oas_TRAPEZOID_1      = 23;
   const byte oas_TRAPEZOID_2      = 24;
   const byte oas_TRAPEZOID_3      = 25;
   const byte oas_CTRAPEZOID       = 26;
   const byte oas_CIRCLE           = 27;
   const byte oas_PROPERTY_1       = 28;
   const byte oas_PROPERTY_2       = 29;
   const byte oas_XNAME_1          = 30;
   const byte oas_XNAME_2          = 31;
   const byte oas_XELEMENT         = 32;
   const byte oas_XGEOMETRY        = 33;
   const byte oas_CBLOCK           = 34;
   const byte oas_MagicBytes[]     = {0x25, 0x53, 0x45, 0x4d, 0x49, 0x2D, 0x4F, 0x41, 0x53, 0x49, 0x53, 0x0D, 0x0A};
   // info-byte masks
   const byte Smask                 = 0b10000000;
   const byte Wmask                 = 0b01000000;
   const byte Hmask                 = 0b00100000;
   const byte Xmask                 = 0b00010000;
   const byte Ymask                 = 0b00001000;
   const byte Rmask                 = 0b00000100;
   const byte Dmask                 = 0b00000010;
   const byte Lmask                 = 0b00000001;
   const byte Emask                 = Smask;
   const byte Pmask                 = Hmask;
   
   class OasisInFile;
   
//   class Record {
//      public:
//         Record();
//         
//   };
   typedef enum {tblm_unknown, tblm_implicit, tblm_explicit} TableMode;
   
   class Table {
      public:
                           Table(OasisInFile&);
         void              getTableRecord(OasisInFile&, TableMode);
      private:
         qword             _offset;
         dword             _nextIndex;
         bool              _strictMode;
         TableMode         _ieMode; //implicit/explicit mode
      
   };

   template <class TYPE> class ModalVar {
      public:
                           ModalVar()                    {_status = false;}
         void              reset()                       {_status = false;}
         bool              status()                      {return _status;}
         TYPE              operator = (const TYPE value) {_value = value; _status = true; return _value;}
         TYPE              operator() ()                 {if (!_status) 
                                                             throw EXPTNreadOASIS("Uninitialised modal variable referenced (10.3)");
                                                          else 
                                                             return _value;}
      private:
         bool              _status;
         TYPE              _value;
   };

   class Cell {
      public:
                           Cell();
         byte              readCell(OasisInFile&, bool);
      private:
         void              skimRectangle(OasisInFile&);
         void              skimPolygon(OasisInFile&);
         ModalVar<dword>   _mod_layer     ; //! OASIS modal variable layer
         ModalVar< word>   _mod_datatype  ; //! OASIS modal variable datatype
         ModalVar<dword>   _mod_gwidth    ; //! OASIS modal variable geometry-w
         ModalVar<dword>   _mod_gheight   ; //! OASIS modal variable geometry-h
         ModalVar<int4b>   _mod_gx        ; //! OASIS modal variable geometry-x
         ModalVar<int4b>   _mod_gy        ; //! OASIS modal variable geometry-y
   };
   
   class OasisInFile {
      public:
                           OasisInFile(std::string);
                          ~OasisInFile();
         void              readLibrary();
         bool              status ()         {return _status;}
         void              hierOut ()        {/*@TODO!*/}
         byte              getByte();
         qword             getUnsignedInt(byte);
         int8b             getInt(byte);
         real              getReal();
         std::string       getString();
      private:
         
         void              readStartRecord();
         // Oasis tables
         Table*            _cellNames;
         Table*            _textStrings;
         Table*            _propNames;
         Table*            _propStrings;
         Table*            _layerNames;
         Table*            _xNames;
         //
         std::string       _fileName;
         wxFileOffset      _fileLength;
         wxFFile           _oasisFh;
         std::string       _version;
         real              _unit;
         bool              _status;
   };
}

#endif //OASIS_H_INCLUDED
