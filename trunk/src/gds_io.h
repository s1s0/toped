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
// ------------------------------------------------------------------------ =
//           $URL$
//  Creation date: Jun 14 1998
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
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
   class GDSFile;
   class GDSstructure;
   class GDSrecord;

   typedef std::vector<GDSstructure*>     ChildStructure;
   typedef SGHierTree<GDSstructure>       GDSHierTree;

   typedef struct {word Year,Month,Day,Hour,Min,Sec;} GDStime;

   /*** GDSrecord ***************************************************************
   >>> Constructor --------------------------------------------------------------
   > reads 'rl' bytes from the input file 'Gf'. Initializes all data fields
   > including 'isvalid'. This constructor is called ONLY from 'GetNextRecord'
   > method of GDSFile class
   >> input parameters ->   Gf   - file handler
   >                        rl   - record length
   >                        rt   - record type
   >                        dt   - data type
   >>> Data fields --------------------------------------------------------------
   > reclen      - length of current GDS record
   > rectype   - type of current GDSrecord
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
   class   GDSrecord
   {
   public:
                GDSrecord(FILE* Gf, word rl, byte rt, byte dt);
                GDSrecord(byte rt, byte dt, word rl);
      byte      Get_rectype() {return rectype;}
      byte*     Get_record()  {return record;}
      word      Get_reclen()  {return reclen;}
      bool      Ret_Data(void* var, word curnum = 0, byte len = 0);
      word      flush(FILE* Gf);
      bool      isvalid;
      void      add_int2b(const word);
      void      add_int4b(const int4b);
      void      add_real8b(const real);
      void      add_ascii(const char*);
      ~GDSrecord();
   protected:
      byte*     ieee2gds(double);
      double    gds2ieee(byte*);
      word      reclen;
      byte      rectype;
      byte      datatype;
      byte*     record;
   private:
      word      numread;
      word      index;
   };

//#define gdsDT_NODATA       0
//#define gdsDT_BIT          1
//#define gdsDT_INT2B        2
//#define gdsDT_INT4B        3
//#define gdsDT_REAL4B       4
//#define gdsDT_REAL8B       5
//#define gdsDT_ASCII        6
   
   /*** GDSdata *****************************************************************
   > This class is inherited by all GDSII data classes
   >>> Constructor --------------------------------------------------------------
   > Initialize data fields
   >> input parameters ->   lst - pointer to last GDSdata
   >>> Data fields --------------------------------------------------------------
   > last               - last GDSdata
   > lastlay            - last GDSdata in layer sequence(GDSstructure.Compbylay[])
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
   > GetGDSDatatype()   - virtual method - Returns gds_DATATYPE
   > GetLast()            - ...
   > GetLastInLay()      - ...
   > GetLayer()         - ...
   ******************************************************************************/
   class   GDSdata {
   public:
      GDSdata(GDSdata* lst);
      void            ReadPLEX(GDSrecord* cr);
      void            ReadELFLAGS(GDSrecord* cr);
      GDSdata*        GetLast(){return last;};
//      GDSdata*        GetLastInLay(){return lastlay;};
      int2b           GetLayer(){return layer;};
      GDSdata*        PutLaymark(GDSdata* lst){lastlay = lst;return this;};
      virtual byte    GetGDSDatatype() = 0;
      virtual ~GDSdata() {};
   protected:
      GDSdata*         last;
      GDSdata*         lastlay;
      int4b            plex;
      word             elflags;
      int2b            layer;
      int2b            singletype;
   };

   class   GDSbox:public GDSdata {
   public:
                     GDSbox(GDSFile *cf, GDSdata *lst);
      byte           GetGDSDatatype(){return gds_BOX;};
      pointlist&     GetPlist()      {return _plist;}
      virtual       ~GDSbox() {};
   protected:
      int2b          boxtype;
      pointlist      _plist;
   };

   /*** GDSpolygon **************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII polygon
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                      lst - pointer to last GDSstructure
   >>> Data fields --------------------------------------------------------------
   > numpoints            - number of polygon vertices
   > FPoint               - pointer to first polygon vertex
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - see GDSdata
   > Get_BoundBox()      - virtual method - see GDSdata
   > GetGDSDatatype()   - virtual method - see GDSdata
   ******************************************************************************/
   class   GDSpolygon:public GDSdata
   {
   public:
      GDSpolygon(GDSFile *cf, GDSdata *lst);
      byte           GetGDSDatatype(){return gds_BOUNDARY;};
      pointlist&     GetPlist()      {return _plist;}
      virtual       ~GDSpolygon() {};
   protected:
      word           numpoints;
      pointlist      _plist;
   };

   /*** GDSpath *****************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII path
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                      lst - pointer to last GDSstructure
   >>> Data fields --------------------------------------------------------------
   > numpoints            - number of path vertices
   > FPoint               - pointer to first path vertex
   > pathtype            - see gds_PATHTYPE
   > width               - see gds_WIDTH
   > bgnextn            - gds_BGNEXTN - not used for output
   > endextn            - gds_ENDEXTN - not used for output
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - see GDSdata
   > Get_BoundBox()      - virtual method - see GDSdata
   > GetGDSDatatype()   - virtual method - see GDSdata
   ******************************************************************************/
   class   GDSpath:public GDSdata
   {
   public:
      GDSpath(GDSFile *cf, GDSdata *lst);
      byte           GetGDSDatatype(){return gds_PATH;};
      pointlist&     GetPlist()      {return _plist;};
      int4b          Get_width()     {return width;};
      virtual       ~GDSpath() {};
   protected:
      void           convert22(int4b, int4b);
      int2b          pathtype;
      int4b          width;
      int4b          bgnextn;
      int4b          endextn;
      word           numpoints;
      pointlist      _plist;
   };

   /*** GDStext *****************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII text
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                      lst - pointer to last GDSstructure
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
   > WritePS_v2()         - virtual method - see GDSdata
   > Get_BoundBox()      - virtual method - see GDSdata
   > GetGDSDatatype()   - virtual method - see GDSdata
   ******************************************************************************/
   class   GDStext:public GDSdata
   {
   public:
      GDStext(GDSFile *cf, GDSdata *lst);
      byte            GetGDSDatatype(){return gds_TEXT;};
      char*           GetText() {return text;};
      word            GetReflection()     {return reflection;};
      TP              GetMagn_point()     {return magn_point;};
      double          GetMagnification()  {return magnification;};
      double          GetAngle()          {return angle;};
      virtual        ~GDStext() {};
   protected:
      word      font;
      word      vertjust;
      word      horijust;
      word      reflection;
      word      abs_magn;
      word      abs_angl;
      TP        magn_point;
      char      text[512];
      double    magnification;
      double    angle;
      int2b     pathtype;
      int4b     width;
   };

   /*** GDSref ******************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII reference
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                      lst - pointer to last GDSstructure
   >>> Data fields --------------------------------------------------------------
   > char               - name of referenced GDSstructure
   > refstr               - pointer to referenced GDSstructure
   > reflection         - mirror about X axis before angular rotation
   > magn_point         - displacement point;
   > magnification      - scaling
   > angle               - rotation angle
   > tmtrx               - translation matrix for this reference of GDSstructure
   >|abs_magn            - true absolute magnification, false relative one
   >|abs_angl            - true absolute angle, false relative angle
    |--------------------> All this params - not used for PS output
   >>> Methods ------------------------------------------------------------------
   > WritePS_v2()         - virtual method - not defined in this class!
   > Get_BoundBox()      - virtual method - not defined in this class!
   > GetGDSDatatype()   - virtual method - see GDSdata
   > SetStructure()      - Set refstr variable, called by GDSlibrary::SetHierarchy
   > GetStrname()         - ...
   > Get_PSCTM()         - ...
   > Get_refstr()         - ...
   ******************************************************************************/
   class   GDSref:public GDSdata
   {
   public:
      GDSref(GDSdata *lst);//default, called by GDSaref::GDSaref
      GDSref(GDSFile *cf, GDSdata *lst);
      void            SetStructure(GDSstructure* strct){refstr = strct;};
      byte            GetGDSDatatype(){return gds_SREF;};
      char*           GetStrname()        {return strname;};
      GDSstructure*   Get_refstr()        {return refstr;};
      word            GetReflection()     {return reflection;};
      TP              GetMagn_point()     {return magn_point;};
      double          GetMagnification()  {return magnification;};
      double          GetAngle()          {return angle;};
      virtual        ~GDSref() {};
   protected:
      char            strname[33];
      GDSstructure*   refstr;
      word            reflection;
      TP              magn_point;
      double          magnification;
      double          angle;
      word            abs_magn;
      word            abs_angl;
   };

   /*** GDSaref ******************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII array reference
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                      lst - pointer to last GDSstructure
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
   > GetGDSDatatype()   - virtual method - see GDSdata
   ******************************************************************************/
   class   GDSaref:public GDSref
   {
   public:
      GDSaref(GDSFile *cf, GDSdata *lst);
      int        Get_Xstep();
      int        Get_Ystep();
      int2b      Get_colnum(){return colnum;};
      int2b      Get_rownum(){return rownum;};
      byte       GetGDSDatatype(){return gds_AREF;};
      virtual   ~GDSaref() {};
   protected:
      TP         X_step;
      TP         Y_step;
      int2b      colnum;
      int2b      rownum;
   };

   /*** GDSstructure ************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII structure
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                      lst - pointer to last GDSstructure
   >>> Data fields --------------------------------------------------------------
   > HaveParent         - true if current structure has a parrent structure.
                          Used when hierarchy is created (GDSlibrary.HierOut)
   > children            - Array of pointers to all GDSstructures referenced
                          inside this structure. Used when hierarchy is created
   > tvstruct            - HTREEITEM structure used for hierarhy window;
   > Allay[...]         - An array with info whether a given layer is used in
                          GDS structure (and down in hierarhy) or not.
   > Compbylay[]        - An array of pointers with all GDSdata grouped by GDS
                          layer. This structure doesn't contain data of type GDSref
                          and GDSaref because they doesn't belong to any layer
   > UsedLayers         - list of pointers with all used GDS layers in this
                          structure and down in hierarchy
   > Fdata               - Pointer to first GDSdata of this structure
   > last               - Pointer to last (next) GDSstructure
   > strname[33]         - GDSII name of the structure
   >>> Methods ------------------------------------------------------------------
   > RegisterStructure()- Creates a tree of referenced GDSstructure. Changes
                          children valiable
   > HierOut()            - Updates HierList window. Here Allay[] and UsedLayers
                          variables are changed
   > PPreview()         - Calculates the box overlapping GDSstructure
   > PSOut_v2()         - Post Script output
   > GetLast()            - ...
   > Get_StrName()      - ...
   > Get_Fdata()         - ...
   > Get_GDSlay()         - ...
   > Get_Allay(byte i)   - ...
   ******************************************************************************/

   class   GDSstructure
   {
   public:
      GDSstructure(GDSFile* cf, GDSstructure* lst);
      bool              RegisterStructure(GDSstructure* ws);
      GDSHierTree*      HierOut(GDSHierTree* Htree, GDSstructure* parent);
      GDSstructure*     GetLast(){return last;}
      char*             Get_StrName(){return strname;}
      GDSdata*          Get_Fdata(){return Fdata;};
      bool              Get_Allay(byte i){return Allay[i];};
      ~GDSstructure();
      bool              HaveParent;
      ChildStructure    children;
   protected:
      bool              Allay[GDS_MAX_LAYER];
      GDSdata*          Compbylay[GDS_MAX_LAYER];
      GDSdata*          Fdata;
      char              strname[65];
      GDSstructure*     last;
   };

   /*** GDSLibrary **************************************************************
   >>> Constructor --------------------------------------------------------------
   > Reads a GDSII library
   >> input parameters ->   cf - pointer to the top GDSFile structure.
   >                       cr - pointer to last GDSrecord structure
   >>> Data fields --------------------------------------------------------------
   > libname            - library name (string)
   > fonts[4]            - GDSII font names used in the library;
   > DBU                  - Data Base Units in meters
   > UU                  - DBU's in one User Unit
   > maxver               - (generations) max versions
   > Fstruct            - pointer to first structure in the GDSII library
   >>> Methods ------------------------------------------------------------------
   > SetHierarchy()      - This method is called to organize hierarhy of the struc-
                          tures. Each GDSref or GDSaref receives a pointer to its
                          corresponding GDSstructure.
   > HierOut            - Called by corresponding method in GDSFile. Calls HierOut
                          method of GDSstructure. The goal of all that calls is to
                          update hierarchy window
   > Get_Fstruct()      - ...
   > Get_DBU()            - ...
   > Get_UU()            - ...
   ******************************************************************************/
   class   GDSlibrary
   {
   public:
      GDSlibrary(GDSFile* cf, GDSrecord* cr);
      void             SetHierarchy();
      GDSHierTree*     HierOut();
      GDSstructure*    Get_Fstruct(){return Fstruct;};
      double           Get_DBU(){return DBU;};
      double           Get_UU(){return UU;};
      std::string      Get_name() const {return std::string(libname);}
      ~GDSlibrary();
   protected:
      char             libname[256];
      char*            fonts[4];
      double           DBU;
      double           UU;
      int2b            maxver;
      GDSstructure*    Fstruct;
   };

   /*** GDSFile ***************************************************************
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
   > library      - pointer to GDSlibrary structure
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
   > GetNextRecord()      - Reads next record from the input stream. Retuns NULL if
   >                       error ocures during read in. This is the only function
   >                      used for reading of the input GDS file. Calls
   >                      'GDSrecord' constructor
   > Get_LibUnits()      - |Return library units
   > Get_UserUnits()    - |Return user units
                          |->Both methods call corresponding methods in GDSlibrary
   > GetStructure()      - Returns the pointer to GDSII structure with a given name
   > HierOut()            - Call corresponding method in GDSlibrary
   > GetReadErrors()      - Returns the number of errors during GDSII file reading
   > GetTimes()         - Reads values of t_access and t_modiff (see above)
   ******************************************************************************/
   class   GDSFile
   {
   public:
      GDSFile(const char*fn);
      GDSFile(std::string, TIME_TPD);
      GDSrecord*     GetNextRecord();
      GDSrecord*     GDSFile::SetNextRecord(byte rectype, word reclen = 0);
      double         Get_LibUnits();
      double         Get_UserUnits();
      void           SetTimes(GDSrecord* wr);
      bool           checkCellWritten(std::string);
      void           registerCellWritten(std::string);
      std::string    Get_libname() const {return library->Get_name();}
      GDSstructure*  GetStructure(const char* selection);
      GDSHierTree*   HierOut() {return (HierTree = library->HierOut());};
      int            GetReadErrors() {return GDSIIerrors;};
      int            GetGDSIIwarnings() {return GDSIIwarnings;};
      int            Inc_GDSIIerrors() {return ++GDSIIerrors;};
      int            Inc_GDSIIwarnings() {return ++GDSIIwarnings;};
      GDSstructure*  Get_structures() {return library->Get_Fstruct();};
      void           GetHierTree(); // temporary
      void           flush(GDSrecord*);
      void           closeFile() {if (NULL != GDSfh) {fclose(GDSfh); GDSfh = NULL;}}
      void           updateLastRecord();
      ~GDSFile();
   protected:
      void           GetTimes(GDSrecord* wr);
      FILE*          GDSfh;
      std::string    filename;
      int2b          StreamVersion;
      int2b          libdirsize;
      char           srfname[256];
      GDSlibrary*    library;
      long           file_length;
      long           file_pos;
      GDSHierTree*   HierTree; // Tree of instance hierarchy
      nameList       _childnames;
      int            GDSIIerrors;
      int            GDSIIwarnings;
      GDStime        t_modif;
      GDStime        t_access;
   };

   // Function definition
     TP   get_TP(GDSrecord* cr, word curnum = 0, byte len=4);
     void AddLog(char logtype, const char* message);
     void PrintChildren(GDSin::GDSHierTree*, std::string*);
}   
#endif // !defined(GDSIO_H_INCLUDED)
