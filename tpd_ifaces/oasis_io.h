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

   // Types of point lists
   typedef enum { dt_manhattanH    = 0 ,
                  dt_manhattanV    = 1 ,
                  dt_mamhattanE    = 2 ,
                  dt_octangular    = 3 ,
                  dt_allangle      = 4 ,
                  dt_doubledelta   = 5 ,
                  dt_unknown       = 6  } PointListType;
   // Types of coordinate deltas
   typedef enum { dr_east          = 0 ,
                  dr_north         = 1 ,
                  dr_west          = 2 ,
                  dr_south         = 3 ,
                  dr_northeast     = 4 ,
                  dr_northwest     = 5 ,
                  dr_southeast     = 6 ,
                  dr_southwest     = 7  } DeltaDirections;
   // Types of geometry repetitions
   typedef enum { rp_reuse         = 0 ,
                  rp_regXY         = 1 ,
                  rp_regX          = 2 ,
                  rp_regY          = 3 ,
                  rp_varX          = 4 ,
                  rp_varXxG        = 5 ,
                  rp_varY          = 6 ,
                  rp_varYxG        = 7 ,
                  rp_regDia2D      = 8 ,
                  rp_regDia1D      = 9 ,
                  rp_varAny        =10 ,
                  rp_varAnyG       =11 ,
                  rp_unknown       =12  } RepetitionTypes;
   // Types of path extensions
   typedef enum { ex_reuse         = 0 ,
                  ex_flush         = 1 ,
                  ex_hwidth        = 2 ,
                  ex_explicit      = 3 ,
                  ex_unknown       = 4  } ExtensionTypes;
   
   typedef enum { tblm_unknown         , 
                  tblm_implicit        , 
                  tblm_explicit         } TableMode;

   class OasisInFile;
   
   
   class Table {
      public:
         typedef std::map<dword, std::string> NameTable;
                           Table(OasisInFile&);
         void              getTableRecord(OasisInFile&, TableMode);
         std::string       getName(dword);
      private:
         qword             _offset;
         dword             _nextIndex;
         bool              _strictMode;
         TableMode         _ieMode; //implicit/explicit mode
         NameTable         _table;
         
   };

   template <class TYPE> class ModalVar {
      public:
                           ModalVar()                    {_status = false;}
         void              reset()                       {_status = false;}
         bool              status()                      {return _status;}
         TYPE&             operator = (const TYPE& value) {_value = value; _status = true; return _value;}
         TYPE&             operator() ()                 {if (!_status) 
                                                             throw EXPTNreadOASIS("Uninitialised modal variable referenced (10.3)");
                                                          else 
                                                             return _value;}
      private:
         bool              _status;
         TYPE              _value;
   };

   class PointList {
      public:
                           PointList() : _pltype(dt_unknown), _vcount(0), _delarr(NULL) {}
                           PointList(PointList&);
                           PointList(OasisInFile&, PointListType);
                          ~PointList();
         PointList&        operator = (const PointList&);
      private:
         void              readManhattanH(OasisInFile&);
         void              readManhattanV(OasisInFile&);
         void              readManhattanE(OasisInFile&);
         void              readOctangular(OasisInFile&);
         void              readAllAngle(OasisInFile&);
         void              readDoubleDelta(OasisInFile&);
         PointListType     _pltype; //! Oasis point list type
         dword             _vcount; //! Number of vertexes in the list
         int4b*            _delarr; //! Delta sequence in XYXY... array
   };

   class Repetitions {
      public:
                           Repetitions() : _rptype(rp_unknown), _bcount(0), _lcarray(NULL) {}
                           Repetitions(OasisInFile&, RepetitionTypes);
                          ~Repetitions();
         Repetitions&      operator = (const Repetitions&);
      private:
         void              readregXY(OasisInFile&);
         void              readregX(OasisInFile&);
         void              readregY(OasisInFile&);
         void              readvarX(OasisInFile&);
         void              readvarXxG(OasisInFile&);
         void              readvarY(OasisInFile&);
         void              readvarYxG(OasisInFile&);
         void              readregDia2D(OasisInFile&);
         void              readregDia1D(OasisInFile&);
         void              readvarAny(OasisInFile&);
         void              readvarAnyG(OasisInFile&);
         RepetitionTypes   _rptype;  //! Oasis repetition type
         dword             _bcount;  //! Number of binding points in the sequence
         int4b*            _lcarray; //! Location sequence in XYXY ... array
   };

   class PathExtensions {
      public:
                           PathExtensions() : _extype(ex_unknown), _exex(0) {}
                           PathExtensions(OasisInFile&, ExtensionTypes);
      private:
         ExtensionTypes    _extype; //! Oasis path extension type
         int4b             _exex;   //! Explicit extension
   };
   
   class Cell {
      public:
                           Cell();
         byte              skimCell(OasisInFile&, bool);
      private:
         void              skimRectangle(OasisInFile&);
         void              skimPolygon(OasisInFile&);
         void              skimPath(OasisInFile&);
         void              skimText(OasisInFile&);
         void              skimReference(OasisInFile&, bool);
         void              readRectangle(OasisInFile&);
         void              readPolygon(OasisInFile&);
         void              readPath(OasisInFile&);
         void              readText(OasisInFile&);
         void              readReference(OasisInFile&, bool);
         PointList         readPointList(OasisInFile&);
         void              readRepetitions(OasisInFile&);
         void              readExtensions(OasisInFile&, PathExtensions&, PathExtensions&);
         ModalVar<dword>   _mod_layer       ; //! OASIS modal variable layer
         ModalVar< word>   _mod_datatype    ; //! OASIS modal variable datatype
         ModalVar<dword>   _mod_gwidth      ; //! OASIS modal variable geometry-w
         ModalVar<dword>   _mod_gheight     ; //! OASIS modal variable geometry-h
         ModalVar< word>   _mod_pathhw      ; //! OASIS modal variable path-halfwidth
         ModalVar<int4b>   _mod_gx          ; //! OASIS modal variable geometry-x
         ModalVar<int4b>   _mod_gy          ; //! OASIS modal variable geometry-y
         ModalVar<std::string>   _mod_text  ; //! OASIS modal variable text-string
         ModalVar<std::string>   _mod_cellref;//! OASIS modal variable placement-cell
         ModalVar<PointList>     _mod_pplist; //! OASIS modal variable polygon point list
         ModalVar<PointList>     _mod_wplist; //! OASIS modal variable path point list
         ModalVar<Repetitions>   _mod_repete; //! OASIS modal variable repetition
         ModalVar<PathExtensions> _mod_exs  ; //! OASIS modal variable path-start-extention
         ModalVar<PathExtensions> _mod_exe  ; //! OASIS modal variable path-end-extention
         NameSet           _referenceNames  ; //! All structures referenced here
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
         std::string       getTextRefName(bool);
         std::string       getCellRefName(bool);
         void              exception(std::string);
      private:
         float             getFloat();
         double            getDouble();
         void              readStartRecord();
         void              readEndRecord();
         void              closeFile();
         // Oasis tables
         Table*            _cellNames;
         Table*            _textStrings;
         Table*            _propNames;
         Table*            _propStrings;
         Table*            _layerNames;
         Table*            _xNames;
         //
         bool              _offsetFlag;
         std::string       _fileName;
         wxFileOffset      _fileLength;
         wxFileOffset      _filePos;
         wxFileOffset      _progresPos;
         wxFFile           _oasisFh;
         std::string       _version;
         real              _unit;
         bool              _status;
   };

   void readDelta(OasisInFile&, int4b&, int4b&);
}

#endif //OASIS_H_INCLUDED
