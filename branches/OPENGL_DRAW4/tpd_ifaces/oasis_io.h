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
#include <zlib.h>
#include "outbox.h"
#include "tedstd.h"

//! Contains all class & type definitions related to SEMI P39-0308 (OASIS)
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

   /*! The enum values correspond to the Point List Types as defined in the standard
       (7.7, Table 7). The last member 'dt_unknown' is added for maintaining proper
       state in the Oasis::PointList which in turn is used as a base class of
       Cell::_mod_pplist /polygon-point-list/ and Cell::_mod_wplist /path-point-list/
       modal variables
   */
   typedef enum { dt_manhattanH    = 0 ,
                  dt_manhattanV    = 1 ,
                  dt_mamhattanE    = 2 ,
                  dt_octangular    = 3 ,
                  dt_allangle      = 4 ,
                  dt_doubledelta   = 5 ,
                  dt_unknown       = 6  } PointListType;

   /*! The enum values correspond to the delta directions as defined in the standard
       for 1-delta (7.5.2), 2-delta(7.5.3) and 3-delta(7.5.4) primitives
   */
   typedef enum { dr_east          = 0 ,
                  dr_north         = 1 ,
                  dr_west          = 2 ,
                  dr_south         = 3 ,
                  dr_northeast     = 4 ,
                  dr_northwest     = 5 ,
                  dr_southwest     = 6 ,
                  dr_southeast     = 7  } DeltaDirections;

   /*! The enum values correspond to the Repetition Types as defined in the standard
       (7.6, Table 6). The last member 'rp_unknown' is added for maintaining proper
       state in the Oasis::Repetitions which in turn is used as a base class of
       Cell::_mod_repete /repetition modal variable/
   */
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

   /*! The enum values correspond to the Path Extenstion Shemes as defined in the
       standard for SS and EE bits (27.8, Table 15) The last member ex_unknown is
       added to maintain proper state in the Oasis::PathExtensions which in turn
       is used as a base class of Cell::_mod_exs /path-start-extention/ and
       Cell::_mod_exe /path-end-extention/ modal variables
   */
   typedef enum { ex_reuse         = 0 ,
                  ex_flush         = 1 ,
                  ex_hwidth        = 2 ,
                  ex_explicit      = 3 ,
                  ex_unknown       = 4  } ExtensionTypes;

   /*! Defines the mode of each of the 6 OASIS tables. According to the standard
       implicit (records oas_CELLNAME_1, oas_TEXTSTRING_1 etc.) and explicit
       (records oas_CELLNAME_2, oas_TEXTSTRING_2 etc.) records should not be mixed in.
       The first enum value 'tblm_unknown' is introduced for convinience to maintain
       properly the table mode.
   */
   typedef enum { tblm_unknown         ,
                  tblm_implicit        ,
                  tblm_explicit         } TableMode;

   /*! OASIS Standard Properties (Appendix 2) depend on the context they appear in.
   *   This enum values define all possible contexts for the property records.
   */
   typedef enum { pc_file              ,
                  pc_cell              ,
                  pc_element            } PropertyContext;


   typedef enum { md_absolute          ,
                  md_relative           } XYMode;

   /*! OASIS Standard END Record Validation Schemes (Table 12)
    *  Declares all possible validation schemes and also adding one for
    *  the cases when the scheme is not yet obtained or wrong
    */
   typedef enum { vs_noValidation = 0  ,
                  vs_crc32        = 1  ,
                  vs_checkSum32   = 2  ,
                  vs_unknown      = 3   } ValidationScheme;

   class OasisInFile;
   class Cell;

   typedef std::list<Cell*>        OasisCellList;

   /*! Declares the OASIS name tables.
   */
   class Table {
      public:
         typedef std::map<dword, std::string> NameTable;
                           Table(OasisInFile&);
         void              getCellNameTable(OasisInFile&);
         void              getPropNameTable(OasisInFile&);
         void              getPropStringTable(OasisInFile&);
         void              getTextStringTable(OasisInFile&);
         void              getTableRecord(OasisInFile&, TableMode, bool tableRec = false);
         std::string       getName(dword) const;
      private:
         wxFileOffset      _offsetStart;
         wxFileOffset      _offsetEnd;
         dword             _nextIndex;
         dword             _index;
         bool              _strictMode;
         TableMode         _ieMode; //implicit/explicit mode
         NameTable         _table;
   };

   /*! Represents all OASIS modal variables. Each class used as a TYPE argument of
       this template has to define a proper copy constructor and '=' operator. The
       template takes care about handling the state of the modal variables derived
       from it. ModalVar::_status field contains this state. Defines equal '='
       operator which should be used to assign (set) a value to the modal variable
       and a functor '()' which should be used to read the value of the modal variable.
       An exception will be thrown if the value of the modal variable is
       requested without being previously set.
   */
   template <class TYPE> class ModalVar {
      public:
                           ModalVar()                    {_status = false;}
         void              reset()                       {_status = false;}
         bool              status()                      {return _status;}
         TYPE&             operator = (const TYPE& value) {_value = value; _status = true; return _value;}
         TYPE&             operator() ()                 {if (!_status)
                                                             throw EXPTNreadOASIS("Uninitialized modal variable referenced (10.3)");
                                                          else
                                                             return _value;}
      private:
         bool              _status;
         TYPE              _value;
   };

   /*! The class represents OASIS Point Lists (7.7). Contains dedicated methods for
       parsing all Point List types and also for the corresponding coordinate calculation
       All parsed delta values are stored in the PointList::_delarr. The calculated
       coordinates are returned in the corresponding parameters of the calc* methods.
       Defines also a copy constructor and assign operator required by the
       Oasis::ModalVar template to define the corresponding modal variables
   */
   class PointList {
      public:
                           PointList() : _pltype(dt_unknown), _vcount(0), _delarr(NULL) {}
                           PointList(const PointList&);
                           PointList(OasisInFile&, PointListType);
                          ~PointList();
         void              calcPoints(PointVector&, int4b, int4b, bool poly=true);
         dword             vcount()          {return _vcount;}
         int4b*            delarr()          {return _delarr;}
         PointList&        operator = (const PointList&);
      private:
         void              readManhattanH(OasisInFile&);
         void              readManhattanV(OasisInFile&);
         void              readManhattanE(OasisInFile&);
         void              readOctangular(OasisInFile&);
         void              readAllAngle(OasisInFile&);
         void              readDoubleDelta(OasisInFile&);
         void              calcManhattanH(PointVector&, int4b, int4b, bool);
         void              calcManhattanV(PointVector&, int4b, int4b, bool);
         void              calcManhattanE(PointVector&, int4b, int4b);
         void              calcOctangular(PointVector&, int4b, int4b);
         void              calcAllAngle(PointVector&, int4b, int4b);
         void              calcDoubleDelta(PointVector&, int4b, int4b);
         PointListType     _pltype; //! Oasis point list type
         dword             _vcount; //! Number of vertexes in the list
         int4b*            _delarr; //! Delta sequence in XYXY... array
   };

   /*! The class represents OASIS Repetitions (7.6 and Table 6). Contains dedicated
       methods for parsing all Repetition types. The parsed repetition sequences
       are stored in the array Repetitions::_lcarray
       Defines also a copy constructor and assign operator required by the
       Oasis::ModalVar template to define the corresponding modal variables
   */
   class Repetitions {
      public:
                           Repetitions() : _rptype(rp_unknown), _bcount(0), _lcarray(NULL) {}
                           Repetitions(OasisInFile&, RepetitionTypes);
                          ~Repetitions();
         dword             bcount()                { return _bcount;    }
         int4b*            lcarray()               { return _lcarray;   }
         Repetitions&      operator = (const Repetitions&);
      private:
         void              readregXY(OasisInFile&);      //! Parces OASIS Repetition type 1
         void              readregX(OasisInFile&);       //! Parces OASIS Repetition type 2
         void              readregY(OasisInFile&);       //! Parces OASIS Repetition type 3
         void              readvarX(OasisInFile&);       //! Parces OASIS Repetition type 4
         void              readvarXxG(OasisInFile&);     //! Parces OASIS Repetition type 5
         void              readvarY(OasisInFile&);       //! Parces OASIS Repetition type 6
         void              readvarYxG(OasisInFile&);     //! Parces OASIS Repetition type 7
         void              readregDia2D(OasisInFile&);   //! Parces OASIS Repetition type 8
         void              readregDia1D(OasisInFile&);   //! Parces OASIS Repetition type 9
         void              readvarAny(OasisInFile&);     //! Parces OASIS Repetition type 10
         void              readvarAnyG(OasisInFile&);    //! Parces OASIS Repetition type 11
         RepetitionTypes   _rptype;  //! Oasis repetition type
         dword             _bcount;  //! Number of binding points in the sequence
         int4b*            _lcarray; //! Location sequence in XYXY ... array
   };

   class StdProperties {
      public:
                           StdProperties() : _context(pc_file) {}
         void              setContext(PropertyContext context) { _context = context;}
         void              getProperty1(OasisInFile&);
         void              getProperty2(OasisInFile&) {/*@TODO - getProperty2*/}
      private:
         PropertyContext   _context;
         ModalVar<std::string>  _propName;
//         ModalVar<>        _propValue;
   };
//   -- File level standard properties
//   SMAX_SIGNED_INTEGER_WIDTH           - getUnsignedInt
//   SMAX_UNSIGNED_INTEGER_WIDTH         - getUnsignedInt
//   SMAX_STRING_LENGTH                  - getUnsignedInt
//   S_POLYGON_MAX_VERTICES              - getUnsignedInt
//   S_PATH_MAX_VERTICES                 - getUnsignedInt
//   S_TOP_CELL                          - getUnsignedInt
//   S_BOUNDING_BOX_AVAILABLE            - getUnsignedInt
//
//   -- Cell level standard properties
//   S_BOUNDING_BOX                      - getUnsignedInt; getInt x 2; getUnsignedInt x 2
//   S_CELL_OFFSET                       - getUnsignedInt;
//
//   -- Element level Properties
//   S_GDS_PROPERTY                      - getUnsignedInt; getString

   class PathExtensions {
      public:
                           PathExtensions() : _exType(ex_unknown), _exEx(0) {}
                           PathExtensions(OasisInFile&, ExtensionTypes);
         int4b             getExtension(int4b) const;
      private:
         ExtensionTypes    _exType; //! Oasis path extension type
         int4b             _exEx;   //! Explicit extension
   };

   class Cell : public ForeignCell {
      public:
                           Cell();
         virtual          ~Cell() {};
         byte              skimCell(OasisInFile&, bool);
         virtual void      import(ImportDB&);
         void              linkReferences(OasisInFile&);
         ForeignCellTree*  hierOut(ForeignCellTree*, Cell*);
         void              collectLayers(ExtLayers&, bool);
      private:
         void              skimRectangle(OasisInFile&);
         void              skimPolygon(OasisInFile&);
         void              skimPath(OasisInFile&);
         void              skimTrapezoid(OasisInFile&, byte);
         void              skimCTrapezoid(OasisInFile&);
         void              skimText(OasisInFile&);
         void              skimReference(OasisInFile&, bool);
         void              readRectangle(OasisInFile&, ImportDB&);
         void              readPolygon(OasisInFile&, ImportDB&);
         void              readPath(OasisInFile&, ImportDB&);
         void              readTrapezoid(OasisInFile&, ImportDB&, byte);
         void              readCTrapezoid(OasisInFile&, ImportDB&);
         void              readText(OasisInFile&, ImportDB&);
         void              readReference(OasisInFile&, ImportDB&, bool);
         PointList         readPointList(OasisInFile&);
         void              readRepetitions(OasisInFile&);
         void              readExtensions(OasisInFile&);
         void              updateContents(int2b, int2b);
         void              genCTrapezoids(OasisInFile&, PointVector&, int4b, int4b, int4b, int4b, word);
         void              initModals();
         //
         ModalVar<dword>   _mod_layer       ; //! OASIS modal variable layer
         ModalVar< word>   _mod_datatype    ; //! OASIS modal variable datatype
         ModalVar<dword>   _mod_gwidth      ; //! OASIS modal variable geometry-w
         ModalVar<dword>   _mod_gheight     ; //! OASIS modal variable geometry-h
         ModalVar< word>   _mod_pathhw      ; //! OASIS modal variable path-halfwidth
         ModalVar<int4b>   _mod_gx          ; //! OASIS modal variable geometry-x
         ModalVar<int4b>   _mod_gy          ; //! OASIS modal variable geometry-y
         ModalVar<std::string> _mod_cellref ; //! OASIS modal variable placement-cell
         ModalVar<int4b>   _mod_px          ; //! OASIS modal variable placement-x
         ModalVar<int4b>   _mod_py          ; //! OASIS modal variable placement-y
         ModalVar<std::string>   _mod_text  ; //! OASIS modal variable text-string
         ModalVar<dword>   _mod_tlayer      ; //! OASIS modal variable textlayer
         ModalVar< word>   _mod_tdatatype   ; //! OASIS modal variable texttype
         ModalVar<int4b>   _mod_tx          ; //! OASIS modal variable text-x
         ModalVar<int4b>   _mod_ty          ; //! OASIS modal variable text-y
         ModalVar<XYMode>  _mod_xymode      ; //! OASIS modal variable xy-mode
         ModalVar<word>    _mod_trpztype    ; //! OASIS modal variable ctrapezoid-type

         ModalVar<PointList>     _mod_pplist; //! OASIS modal variable polygon point list
         ModalVar<PointList>     _mod_wplist; //! OASIS modal variable path point list
         ModalVar<Repetitions>   _mod_repete; //! OASIS modal variable repetition
         ModalVar<PathExtensions> _mod_exs  ; //! OASIS modal variable path-start-extension
         ModalVar<PathExtensions> _mod_exe  ; //! OASIS modal variable path-end-extension
         //
         //TODO - OASIS modal variable circle-radius
         //TODO - OASIS modal variable last-property-name
         //TODO - OASIS modal variable last-value-list
         NameSet           _referenceNames  ; //! All names of the structures referenced in this cell
         OasisCellList     _children        ; //! Pointers to all Cell structures referenced in this cell
         ExtLayers         _contSummary     ; //! Layer contents summary
   };

   class CBlockInflate : public z_stream {
      public:
                           CBlockInflate(ForeignDbFile&, wxFileOffset, dword, dword);
         void              readUncompressedBuffer(void *pBuf, size_t nCount);
         bool              endOfBuffer() const  {return _bufOffset == _bufSize;}
         wxFileOffset      startPosInFile() const {return _startPosInFile;}
         virtual          ~CBlockInflate();
      private:
         byte*             _output_buffer;
         byte*             _input_buffer;
         int               _state;
         wxFileOffset      _bufOffset;
         wxFileOffset      _bufSize;
         wxFileOffset      _startPosInFile;
   };

   /*!
    * Implementation of the OASIS CRC. The polynomial is as as specified in
    * ISO 3309 and ITU-T V.42 used in Ethernet, FDDI, cksum, etc.
    * polynomial is x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10
    *                    + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + x^0
    * The implementation is taken from the SEMI standard with some little trims
    * to fit into the Toped coding style.
    */
   class Iso3309Crc32 {
      public:
                              Iso3309Crc32();
         void                 add(const byte*, size_t);
         dword                theCrc() const              {return _theCrc;}
      private:
         void                 tableLoad();
         dword                reflect(dword);
         dword                _crc32Table[256];
         dword                _theCrc;
         bool                 _pauseCalculation;
         static const dword   _crc32Poly;
         static const dword   _crc32Constant;
         static const dword   _crc32LSBit;
         static const dword   _crc32MSBit;
         static const dword   _crc32AllBits;
   };

   class OasisInFile : public ForeignDbFile {
      public:
         typedef std::map<std::string, Cell*> DefinitionMap;
                              OasisInFile(wxString);
         virtual             ~OasisInFile();
         virtual void         hierOut();
         wxFileOffset         oasSetPosition(wxFileOffset);
         void                 inflateCBlock();
         bool                 calculateCRC(Iso3309Crc32&);
         bool                 calculateChecksum(dword& checksum);
         virtual double       libUnits() const  {return _unit;    }
         virtual std::string  libname() const{return getFileNameOnly();}
         void                 setPropContext(PropertyContext context)
                                             {_properties.setContext(context);}
         const Table*         cellNames() const  { return _cellNames;}
         const Table*         textStrings() const{ return _textStrings;}
         const Table*         propNames() const  { return _propNames;}
         const Table*         propStrings() const{ return _propStrings;}
         const Table*         layerNames() const { return _layerNames;}
         const Table*         xNames() const     { return _xNames;}
         ValidationScheme     validation() const {return _validation;}
         dword                signature() const  {return _signature;}

         virtual void         getTopCells(NameList&) const;
         virtual void         getAllCells(wxListBox&) const;
         virtual void         convertPrep(const NameList&, bool);
         //----------------------------------------------------------------------
         byte                 getByte();
         qword                getUnsignedInt(byte);
         int8b                getInt(byte);
         real                 getReal(char type = -1);
         std::string          getString();
         std::string          getTextRefName(bool);
         std::string          getCellRefName(bool);
         void                 exception(std::string);
         Cell*                getCell(const std::string) const;
         virtual void         collectLayers(ExtLayers&) const;
         virtual bool         collectLayers(const std::string&, ExtLayers&) const;
         void                 getProperty1()     { _properties.getProperty1(*this);}
         void                 getProperty2()     { _properties.getProperty2(*this);}
      private:
         void                 readLibrary();
         float                getFloat();
         double               getDouble();
         void                 readStartRecord();
         void                 readEndRecord();
         size_t               rawRead(void *pBuf, size_t nCount);
         //
         void                 linkReferences();
         // Oasis tables
         Table*               _cellNames;
         Table*               _textStrings;
         Table*               _propNames;
         Table*               _propStrings;
         Table*               _layerNames;
         Table*               _xNames;
         //
         StdProperties        _properties;
         //! All Cells defined in the file with a pointer to the respective Cell object
         DefinitionMap        _definedCells;
         //
         //! Position of the table offset fields. (false) - in the START record; (true) in the END record
         bool                 _offsetFlag;
         std::string          _version;   //! OASIS version record retrieved from the file
         real                 _unit;      //! OASIS unit (DBU) retrieved from the file
         CBlockInflate*       _curCBlock; //! Current uncompressed CBLOCK
         ValidationScheme     _validation;//! Validation Scheme of this OASIS file
         dword                _signature; //! The signature of the OASIS file (depends on the validation scheme)
   };

   void readDelta(OasisInFile&, int4b&, int4b&);
}

#endif //OASIS_H_INCLUDED
