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
//        Created: Jun 14 1998
//      Copyright: (C) 2001-2006 Svilen Krustev - skr@toped.org.uk
//    Description: GDSII parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#if !defined(GDSIO_H_INCLUDED)
#define GDSIO_H_INCLUDED

#include <stdio.h>
#include <vector>
#include "../tpd_common/ttt.h"

//GDS data types
#define gdsDT_NODATA       0
#define gdsDT_BIT          1
#define gdsDT_INT2B        2
#define gdsDT_INT4B        3
#define gdsDT_REAL4B       4
#define gdsDT_REAL8B       5
#define gdsDT_ASCII        6
////////////////////////////////
#define GDS_MAX_LAYER            256
// GDS record types
// Described according to "Design Data Translators Reference Manual" - 
// CADance documentation, September 1994
#define gds_HEADER         0x00
#define gds_BGNLIB         0x01
#define gds_LIBNAME        0x02
#define gds_UNITS          0x03
#define gds_ENDLIB         0x04
#define gds_BGNSTR         0x05
#define gds_STRNAME        0x06
#define gds_ENDSTR         0x07
#define gds_BOUNDARY       0x08
#define gds_PATH           0x09
#define gds_SREF           0x0A
#define gds_AREF           0x0B
#define gds_TEXT           0x0C
#define gds_LAYER          0x0D
#define gds_DATATYPE       0x0E
#define gds_WIDTH          0x0F
#define gds_XY             0x10
#define gds_ENDEL          0x11
#define gds_SNAME          0x12
#define gds_COLROW         0x13
#define gds_TEXTNODE       0x14
#define gds_NODE           0x15
#define gds_TEXTTYPE       0x16
#define gds_PRESENTATION   0x17
#define gds_SPACING        0x18
#define gds_STRING         0x19
#define gds_STRANS         0x1A
#define gds_MAG            0x1B
#define gds_ANGLE          0x1C
#define gds_UINTEGER       0x1D
#define gds_USTRING        0x1E
#define gds_REFLIBS        0x1F
#define gds_FONTS          0x20
#define gds_PATHTYPE       0x21
#define gds_GENERATION     0x22
#define gds_ATTRTABLE      0x23
#define gds_STYPTABLE      0x24
#define gds_STRTYPE        0x25
#define gds_ELFLAGS        0x26
#define gds_ELKEY          0x27
#define gds_LINKTYPE       0x28
#define gds_LINKKEYS       0x29
#define gds_NODETYPE       0x2A
#define gds_PROPATTR       0x2B
#define gds_PROPVALUE      0x2C
#define gds_BOX            0x2D
#define gds_BOXTYPE        0x2E
#define gds_PLEX           0x2F
#define gds_BGNEXTN        0x30
#define gds_ENDEXTN        0x31
#define gds_TYPENUM        0x32
#define gds_TYPECODE       0x33
#define gds_STRCLASS       0x34
#define gds_RESERVED       0x35
#define gds_FORMAT         0x36
#define gds_MASK           0x37
#define gds_ENDMASKS       0x38
#define gds_LIBDIRSIZE     0x39
#define gds_SRFNAME        0x3A
#define gds_LIBSECUR       0x3B
#define gds_BORDER         0x3C
#define gds_SOFTFENCE      0x3D
#define gds_HARDFENCE      0x3E
#define gds_SOFTWIRE       0x3F
#define gds_HARDWIRE       0x40
#define gds_PATHPORT       0x41
#define gds_NODEPORT       0x42
#define gds_USERCONSTRAINT 0x43
#define gds_SPACER_ERROR   0x44
#define gds_CONTACT        0x45

namespace GDSin {
   class GdsFile;
   class GdsStructure;
   class GdsRecord;

   typedef std::vector<GdsStructure*>     ChildStructure;
   typedef SGHierTree<GdsStructure>       GDSHierTree;

   typedef struct {word Year,Month,Day,Hour,Min,Sec;} GDStime;

   /*** GdsRecord ***************************************************************
   >>> Constructor --------------------------------------------------------------
   > reads 'rl' bytes from the input file 'Gf'. Initializes all data fields
   > including 'isvalid'. This constructor is called ONLY from 'getNextRecord'
   > method of GdsFile class
   >> input parameters ->   Gf   - file handler
   >                        rl   - record length
   >                        rt   - record type
   >                        dt   - data type
   >>> Data fields --------------------------------------------------------------
   > reclen      - length of current GDS record
   > rectype   - type of current GdsRecord
   > datatype   - type of data that this record contain
   > record      - the information record
   > numread   - number of really read bytes in this record
   > isvalid   - true if numread == reclen, otherwise - false
   >>> Methods ------------------------------------------------------------------
   > Get_rectype()   - inline function - see definition
   > Get_record()      - inline function - see definition
   > Get_reclen()      - inline function - see definition
   > Ret_data(...)   - Return data function. This function is responsible for
   >                   proper GDS data treatement. It converts 'raw' GDS data
   >                   that 'record' field contains to C format. Called by all
   >                   GDS classes taking part in GDS reading.
   ******************************************************************************/
   class   GdsRecord {
      public:
                           GdsRecord(FILE* Gf, word rl, byte rt, byte dt);
                           GdsRecord(byte rt, byte dt, word rl);
         bool              retData(void* var, word curnum = 0, byte len = 0);
         word              flush(FILE* Gf);
         void              add_int2b(const word);
         void              add_int4b(const int4b);
         void              add_real8b(const real);
         void              add_ascii(const char*);
         byte              recType() const                     { return _recType;}
         byte*             record() const                      { return _record;}
         word              recLen() const                      { return _recLen;}
         bool              valid() const                       { return _valid;}
                          ~GdsRecord();
      private:
         byte*             ieee2gds(double);
         double            gds2ieee(byte*);
         bool              _valid;
         word              _recLen;
         byte              _recType;
         byte              _dataType;
         byte*             _record;
         word              _numread;
         word              _index;
   };

//#define gdsDT_NODATA       0
//#define gdsDT_BIT          1
//#define gdsDT_INT2B        2
//#define gdsDT_INT4B        3
//#define gdsDT_REAL4B       4
//#define gdsDT_REAL8B       5
//#define gdsDT_ASCII        6
   
   /*** GdsData *****************************************************************
   > This class is inherited by all GDSII data classes
   >>> Constructor --------------------------------------------------------------
   > Initialize data fields
   >> input parameters ->   lst - pointer to last GdsData
   >>> Data fields --------------------------------------------------------------
   > last               - last GdsData
   > lastlay            - last GdsData in layer sequence(GdsStructure.Compbylay[])
   > plex               - see gds_PLEX (not used anywhere but read from GDS input)
   > elflags            - see gds_ELFLAGS ( same as above )
   > singletype         - see gds_DATATYPE ( same as above )
   > layer               - GDS layer number this data belongs to
   >>> Methods ------------------------------------------------------------------
   > ReadPLEX()         - Takes the value of plex
   > ReadELFLAGS()      - Takes the value of elflags
   > PutLaymark()         - Updates lastlay pointer. Called by most of the
                          inherited constructors.
   > WritePS_v2()         - virtual method - Writes Post script output
   > Get_BoundBox()      - virtual method - Returns the box that overlaps the
                          figure
   > gdsDataType()   - virtual method - Returns gds_DATATYPE
   > GetLast()            - ...
   > GetLastInLay()      - ...
   > GetLayer()         - ...
   ******************************************************************************/
   class   GdsData {
      public:
                           GdsData();
         void              readPlex(GdsRecord*);
         void              readElflags(GdsRecord*);
         GdsData*          linkTo(GdsData* lst)                { _last = lst; return this; }
         GdsData*          last()                              { return _last;   }
         int2b             singleType()                        { return _singleType;}
         virtual         ~GdsData()                           {                  };
         virtual byte     gdsDataType() = 0;
      protected:
         GdsData*          _last;
         int4b             _plex;
         word              _elflags;
         int2b             _singleType;
   };

   class   GdsBox:public GdsData {
      public:
                              GdsBox(GdsFile*, int2b&);
         byte                 gdsDataType()                     { return gds_BOX;}
         pointlist&           plist()                           { return _plist; }
         virtual            ~GdsBox()                          {                 }
      protected:
         pointlist            _plist;
   };

   /*** GdsPolygon **************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII polygon
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                      lst - pointer to last GdsStructure
   >>> Data fields --------------------------------------------------------------
   > numpoints            - number of polygon vertices
   > FPoint               - pointer to first polygon vertex
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - see GdsData
   > Get_BoundBox()      - virtual method - see GdsData
   > gdsDataType()   - virtual method - see GdsData
   ******************************************************************************/
   class   GdsPolygon:public GdsData {
      public:
                              GdsPolygon(GdsFile*, int2b&);
         byte                 gdsDataType()                    { return gds_BOUNDARY;  }
         pointlist&           plist()                          { return _plist;        }
         virtual            ~GdsPolygon()                     {                        }
      protected:
         word                 _numpoints;
         pointlist            _plist;
   };

   /*** GDSpath *****************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII path
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                      lst - pointer to last GdsStructure
   >>> Data fields --------------------------------------------------------------
   > numpoints            - number of path vertices
   > FPoint               - pointer to first path vertex
   > pathtype            - see gds_PATHTYPE
   > width               - see gds_WIDTH
   > bgnextn            - gds_BGNEXTN - not used for output
   > endextn            - gds_ENDEXTN - not used for output
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - see GdsData
   > Get_BoundBox()      - virtual method - see GdsData
   > gdsDataType()   - virtual method - see GdsData
   ******************************************************************************/
   class   GDSpath:public GdsData {
      public:
                              GDSpath(GdsFile*, int2b&);
         byte                 gdsDataType()                    { return gds_PATH;}
         pointlist&           plist()                          { return _plist;  }
         int4b                width()                          { return _width;  }
         virtual            ~GDSpath()                        {                  }
      protected:
         void                 convert22(int4b, int4b);
         int2b                _pathtype;
         int4b                _width;
         int4b                _bgnextn;
         int4b                _endextn;
         word                 _numpoints;
         pointlist            _plist;
   };

   /*** GdsText *****************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII text
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                      lst - pointer to last GdsStructure
   >>> Data fields --------------------------------------------------------------
   > reflection         - mirror about X axis before angular rotation
   > magn_point         - displacement point;
   > text[512]            - text string
   > magnification      - text (font) magnification (height)
   > angle               - text rotation angle
   >|font               - 00, 01, 10, 11 -> font 0,1,2,3
   >|vertjust            - 00 - top , 01 - middle, 10 - bottom
   >|horijust            - 00 - left, 01 - center, 10 - right
   >|abs_magn            - true absolute magnification, false relative one
   >|abs_angl            - true absolute angle, false relative angle
   >|pathtype            - this param is not clear from GDSII description
   >|width               - this param is not clear from GDSII description
    |--------------------> All this params - not used for PS output
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - see GdsData
   > Get_BoundBox()      - virtual method - see GdsData
   > gdsDataType()   - virtual method - see GdsData
   ******************************************************************************/
   class   GdsText:public GdsData {
      public:
                              GdsText(GdsFile *cf, int2b&);
         byte                 gdsDataType(){return gds_TEXT;};
         char*                text()                           { return _text;          }
         word                 reflection()                     { return _reflection;    }
         TP                   magnPoint()                      { return _magnPoint;     }
         double               magnification()                  { return _magnification; }
         double               angle()                          { return _angle;         }
         virtual            ~GdsText()                        {                         }
      protected:
         word                 _font;
         word                 _vertJust;
         word                 _horiJust;
         word                 _reflection;
         word                 _absMagn;
         word                 _absAngl;
         TP                   _magnPoint;
         char                 _text[512];
         double               _magnification;
         double               _angle;
         int2b                _pathType;
         int4b                _width;
   };

   /*** GdsRef ******************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII reference
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                      lst - pointer to last GdsStructure
   >>> Data fields --------------------------------------------------------------
   > char               - name of referenced GdsStructure
   > refstr               - pointer to referenced GdsStructure
   > reflection         - mirror about X axis before angular rotation
   > magn_point         - displacement point;
   > magnification      - scaling
   > angle               - rotation angle
   > tmtrx               - translation matrix for this reference of GdsStructure
   >|abs_magn            - true absolute magnification, false relative one
   >|abs_angl            - true absolute angle, false relative angle
    |--------------------> All this params - not used for PS output
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - not defined in this class!
   > Get_BoundBox()      - virtual method - not defined in this class!
   > gdsDataType()   - virtual method - see GdsData
   > SetStructure()      - Set refstr variable, called by GdsLibrary::SetHierarchy
   > GetStrname()         - ...
   > Get_PSCTM()         - ...
   > Get_refstr()         - ...
   ******************************************************************************/
   class   GdsRef:public GdsData
   {
   public:
                              GdsRef();//default, called by GdsARef::GdsARef
                              GdsRef(GdsFile *cf);
      void                    SetStructure(GdsStructure* strct){ _refStr = strct;       }
      byte                    gdsDataType()                    { return gds_SREF;      }
      char*                   strName()                        { return _strName;      }
      GdsStructure*           refStr()                         { return _refStr;       }
      word                    reflection()                     { return _reflection;   }
      TP                      magnPoint()                      { return _magnPoint;    }
      double                  magnification()                  { return _magnification;}
      double                  angle()                          { return _angle;        }
      virtual               ~GdsRef()                         {                        }
   protected:
      GdsStructure*           _refStr;
      word                    _reflection;
      TP                      _magnPoint;
      double                  _magnification;
      double                  _angle;
      word                    _absMagn;
      word                    _absAngl;
      char                    _strName[33];
   };

   /*** GdsARef ******************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII array reference
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                      lst - pointer to last GdsStructure
   >>> Data fields --------------------------------------------------------------
   > X_step               - position that is displaced from the reference point
                          by the inter-column spacing times the number
                          of columns.
   > Y_step               - position that is displaced from the reference point
                          by the inter-row spacing times the number of rows.
   > colnum               - number of columns in the array
   > rownum               - number of rows in the array
   >>> Methods ------------------------------------------------------------------
   > Get_Xstep()         - returns inter-column spacing
   > Get_Ystep()         - returns inter-row spacing
   > WritePS_v2()         - virtual method - not defined in this class!
   > Get_BoundBox()      - virtual method - not defined in this class!
   > gdsDataType()   - virtual method - see GdsData
   ******************************************************************************/
   class   GdsARef:public GdsRef {
      public:
                              GdsARef(GdsFile*);
         int                  getXStep();
         int                  getYStep();
         byte                 gdsDataType()                    { return gds_AREF;   }
         int2b                columns()                        { return _columns;   }
         int2b                rows()                           { return _rows;      }
         virtual            ~GdsARef()                        {                     }
      protected:
         TP                   _xStep;
         TP                   _yStep;
         int2b                _columns;
         int2b                _rows;
   };

   /*** GdsStructure ************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII structure
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                      lst - pointer to last GdsStructure
   >>> Data fields --------------------------------------------------------------
   > HaveParent         - true if current structure has a parrent structure.
                          Used when hierarchy is created (GdsLibrary.HierOut)
   > children            - Array of pointers to all GdsStructures referenced
                          inside this structure. Used when hierarchy is created
   > tvstruct            - HTREEITEM structure used for hierarhy window;
   > Allay[...]         - An array with info whether a given layer is used in
                          GDS structure (and down in hierarhy) or not.
   > Compbylay[]        - An array of pointers with all GdsData grouped by GDS
                          layer. This structure doesn't contain data of type GdsRef
                          and GdsARef because they doesn't belong to any layer
   > UsedLayers         - list of pointers with all used GDS layers in this
                          structure and down in hierarchy
   > Fdata               - Pointer to first GdsData of this structure
   > last               - Pointer to last (next) GdsStructure
   > strname[33]         - GDSII name of the structure
   >>> Methods ------------------------------------------------------------------
   > RegisterStructure()- Creates a tree of referenced GdsStructure. Changes
                          children valiable
   > HierOut()            - Updates HierList window. Here Allay[] and UsedLayers
                          variables are changed
   > PPreview()         - Calculates the box overlapping GdsStructure
   > PSOut_v2()         - Post Script output
   > GetLast()            - ...
   > Get_StrName()      - ...
   > Get_Fdata()         - ...
   > Get_GDSlay()         - ...
   > Get_Allay(byte i)   - ...
   ******************************************************************************/

   class   GdsStructure {
      public:
         typedef std::map<int2b, GdsData*> LayMap;
                              GdsStructure(GdsFile*, GdsStructure*);
         bool                 registerStructure(GdsStructure* ws);
         GDSHierTree*         hierOut(GDSHierTree* Htree, GdsStructure* parent);
         GdsData*             fDataAt(int2b);
         void                 collectLayers(GdsLayers&, bool hier);
         GdsStructure*        last()                           { return _last;         }
         const char*          name() const                     { return _name;         }
         bool                 allLay(byte i)                   { return _allLay[i];    }
         bool                 traversed() const                { return _traversed;    }
         void                 set_traversed(bool trv)          { _traversed = trv;      }
         int                  libID() const                    { return TARGETDB_LIB;  } // to cover the requirements of the hierarchy template
                             ~GdsStructure();
         bool                 _haveParent;
         ChildStructure       _children;
      protected:
         void                 linkDataIn(GdsData*, int2b);
         bool                 _allLay[GDS_MAX_LAYER];
         LayMap               _layers;
         char                 _name[65];
         GdsStructure*        _last;
         bool                 _traversed;       //! For hierarchy traversing purposes
   };

   /*** GDSLibrary **************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII library
   >> input parameters ->   cf - pointer to the top GdsFile structure.
   >                       cr - pointer to last GdsRecord structure
   >>> Data fields --------------------------------------------------------------
   > libname            - library name (string)
   > fonts[4]            - GDSII font names used in the library;
   > DBU                  - Data Base Units in meters
   > UU                  - DBU's in one User Unit
   > maxver               - (generations) max versions
   > Fstruct            - pointer to first structure in the GDSII library
   >>> Methods ------------------------------------------------------------------
   > SetHierarchy()      - This method is called to organize hierarhy of the struc-
                          tures. Each GdsRef or GdsARef receives a pointer to its
                          corresponding GdsStructure.
   > HierOut            - Called by corresponding method in GdsFile. Calls HierOut
                          method of GdsStructure. The goal of all that calls is to
                          update hierarchy window
   > Get_Fstruct()      - ...
   > Get_DBU()            - ...
   > Get_UU()            - ...
   ******************************************************************************/
   class   GdsLibrary
   {
   public:
                              GdsLibrary(GdsFile* , GdsRecord* );
      void                    setHierarchy();
      GDSHierTree*            hierOut();
      GdsStructure*           fStruct()                        { return _fStruct;            }
      double                  dbu()                            { return _dbu;                }
      double                  uu()                             { return _uu;                 }
      std::string             name() const                     { return std::string(_name);  }
                             ~GdsLibrary();
   protected:
      char                    _name[256];
      char*                   _fonts[4];
      double                  _dbu;
      double                  _uu;
      int2b                   _maxver;
      GdsStructure*           _fStruct;
   };

   class LayerMapGds {
      public:
                              LayerMapGds(GdsLayers*);
                              LayerMapGds(const USMap&, GdsLayers*);
         bool                 getTdtLay(word&, word, word) const;
         bool                 getGdsLayType(word&, word&, word) const;
         bool                 status() {return _status;}
      private:
         typedef std::map< word, word  >     GdtTdtMap;
         typedef std::map< word, GdtTdtMap>  GlMap;
         bool                 parseLayTypeString(wxString&, word);
         void                 patternNormalize(wxString&);
         void                 getList(wxString&, WordList&);
         GlMap                _theMap;
         bool                 _status;
         GdsLayers*           _alist; // all available GDS layers with their data types
   };

   /*** GdsFile ***************************************************************
   >>> Constructor --------------------------------------------------------------
   > Opens the input GDS file and start reading it. Initializes all data fields
   >> input parameters ->   fn - GDSII file name for reading
   >                  progind - pointer to progress indicator control
   >                       lw - pointer to Log window control
   >>> Data fields --------------------------------------------------------------
   > GDSfh         - GDS input file handler
   > filename      - GDS input file name
   > file_length  - length of the input GDS file
   > file_pos      - current position of the input GDS file during read in
   > library      - pointer to GdsLibrary structure
   > prgrs         - pointer to the progress indicator control
   > prgrs_pos      - current position of the progress indicator control
   > logwin         - pointer to the error log edit control
   > StreamVerion   - GDS specific data - see gds_HEADER
   > libdirsize   - GDS specific data - see gds_LIBDIRSIZE
   > srfname      - GDS specific data - see gds_SRFNAME
   > t_access      - Date&time of last access to the GDSII file
   > t_modif      - Date&time of last modification of the GDSII file
                    Last two parameters not used anywhere
   >>> Methods ------------------------------------------------------------------
   > getNextRecord()      - Reads next record from the input stream. Retuns NULL if
   >                       error ocures during read in. This is the only function
   >                      used for reading of the input GDS file. Calls
   >                      'GdsRecord' constructor
   > libUnits()      - |Return library units
   > userUnits()    - |Return user units
                          |->Both methods call corresponding methods in GdsLibrary
   > GetStructure()      - Returns the pointer to GDSII structure with a given name
   > HierOut()            - Call corresponding method in GdsLibrary
   > GetReadErrors()      - Returns the number of errors during GDSII file reading
   > GetTimes()         - Reads values of t_access and t_modiff (see above)
   ******************************************************************************/
   class   GdsFile   {
      public:
                              GdsFile(std::string);
                              GdsFile(std::string, const LayerMapGds*, time_t);
         GdsRecord*           getNextRecord();
         GdsRecord*           setNextRecord(byte, word reclen = 0);
         double               libUnits();
         double               userUnits();
         void                 setTimes(GdsRecord*);
         bool                 checkCellWritten(std::string);
         void                 registerCellWritten(std::string);
         void                 flush(GdsRecord*);
         void                 updateLastRecord();
         GdsStructure*        getStructure(const char*);
         void                 collectLayers(GdsLayers&);
         void                 getMappedLayType(word& gdslay, word& gdstype, word tdtlay);
         std::string          libname() const                  { return _library->name();       }
         void                 hierOut()                        { _hierTree = _library->hierOut();}
         GDSHierTree*         hierTree()                       { return _hierTree;              }
         int                  gdsiiErrors()                    { return _gdsiiErrors;           }
         int                  gdsiiWarnings()                  { return _gdsiiWarnings;         }
         int                  incGdsiiErrors()                 { return ++_gdsiiErrors;         }
         int                  incGdsiiWarnings()               { return ++_gdsiiWarnings;       }
         GdsStructure*        getStructures()                  { return _library->fStruct();    }
         void                 closeFile()                      { if (NULL != _gdsFh) {fclose(_gdsFh); _gdsFh = NULL;}}
         bool                 status()                         { return _status;                }
                              ~GdsFile();
      protected:
         void                 getTimes(GdsRecord* wr);
         FILE*                _gdsFh;
         std::string          _fileName;
         int2b                _streamVersion;
         int2b                _libDirSize;
         char                 _srfName[256];
         GdsLibrary*          _library;
         long                 _fileLength;
         long                 _filePos;
         GDSHierTree*         _hierTree; // Tree of instance hierarchy
         nameList             _childnames;
         int                  _gdsiiErrors;
         int                  _gdsiiWarnings;
         GDStime              _tModif;
         GDStime              _tAccess;
         bool                 _status;
         const LayerMapGds*   _laymap;
   };

   // Function definition
     TP   get_TP(GdsRecord* cr, word curnum = 0, byte len=4);
//     void PrintChildren(GDSin::GDSHierTree*, std::string*);
}   
#endif // !defined(GDSIO_H_INCLUDED)
