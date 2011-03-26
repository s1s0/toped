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
//        Created: Sun May 22 15:43:49 BST 2005
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Basic definitions and file handling of TDT data base
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TEDSTD_H_INCLUDED
#define TEDSTD_H_INCLUDED

#include <string>
#include "ttt.h"
#include "outbox.h"

//==============================================================================
// Toped DaTa (TDT) file markers
//==============================================================================
#define TED_LEADSTRING        "TED"
#define tedf_REVISION         0x02
#define tedf_TIMECREATED      0x03
#define tedf_TIMEUPDATED      0x04
#define tedf_DESIGN           0x80
#define tedf_DESIGNEND        0x81
#define tedf_CELL             0x82
#define tedf_CELLEND          0x83
#define tedf_LAYER            0x84
#define tedf_CELLREF          0x85
#define tedf_CELLAREF         0x86
#define tedf_BOX              0x87
#define tedf_POLY             0x88
#define tedf_WIRE             0x89
#define tedf_TEXT             0x8A
#define tedf_LAYEREND         0x8B
#define tedf_REFS             0x8C
#define tedf_REFSEND          0x8D
#define TED_CUR_REVISION      0
#define TED_CUR_SUBREVISION   9

//==============================================================================
class PSegment {
public:
               PSegment() : _A(0), _B(0), _C(0), _angle(0) {};
               PSegment(real A, real B, real C) : _A(A), _B(B), _C(C), _angle(0) {};
               PSegment(TP,TP);
   byte        crossP(PSegment, TP&);
   bool        empty() {return ((0 == _A) && (0 == _B));};
   PSegment*   ortho(TP);
   PSegment*   parallel(TP);
   PSegment    operator = (const PSegment s) {_A = s._A; _B = s._B; _C = s._C; return *this;};
private:
   real        _A, _B, _C;
   int         _angle;
};

namespace laydata {

   const word _lmnone   = 0x0000;
   const word _lmbox    = 0x0001;
   const word _lmpoly   = 0x0002;
   const word _lmwire   = 0x0004;
   const word _lmtext   = 0x0008;
   const word _lmref    = 0x0010;
   const word _lmaref   = 0x0020;
   const word _lmpref   = 0x0040;
   const word _lmapref  = 0x0080;
   const word _lmall    = 0xffff;

   // The definition below is a "strongly typed enum". Very tempting to use, but too new
   // and too risky for portability. gcc requires -std=c++0x option to stop the warnings
   // It's here just as a reminder for the future
   //   enum class SH_STATUS:byte { sh_active, sh_deleted, sh_selected, sh_partsel, sh_merged } ;
   typedef enum { sh_active, sh_deleted, sh_selected, sh_partsel, sh_merged } SH_STATUS;
   typedef enum {
      shp_OK         = 0x0000,
      shp_ident      = 0x0001, // identical or one line points removed
      shp_clock      = 0x0002, // points reordered to get anti clockwise order
      shp_box        = 0x0004, // shape is a box
      shp_acute      = 0x0008, // acute angle
      shp_collinear  = 0x0010, // collinear wire
      shp_shortends  = 0x0020, // wire with end segments shorter than the width
      // critical
      shp_cross      = 0x1000, // self crossing sequence
      shp_width      = 0x2000, // wire with width bigger than MAX_WIRE_WIDTH
      shp_null       = 0x8000, // 0 area - points are not forming a polygon
   } shape_status;

   const unsigned shp_valid      = shp_cross; //

   class TdtData;
   class EditObject;
   class TdtCell;
   class TdtDefaultCell;
   class TdtCellRef;
   class TdtDesign;
   class TdtLibrary;
   class TdtLibDir;
   class QTreeTmp;
   typedef  std::pair<TdtData*, SGBitSet>           SelectDataPair;
   typedef  std::list<SelectDataPair>               DataList;
   typedef  std::map<unsigned, DataList*>           SelectList;
   typedef  std::list<TdtData*>                     ShapeList;
   typedef  std::map<unsigned,ShapeList*>           AtticList;
   typedef  std::map<std::string, TdtDefaultCell*>  CellMap;
   typedef  TdtDefaultCell*                         CellDefin;
   typedef  std::deque<const TdtCellRef*>           CellRefStack;
   typedef  std::deque<EditObject*>                 EditCellStack;
   typedef  std::list<const CellMap*>               LibCellLists;
   typedef  std::list<TdtDefaultCell*>              CellDefList;
   typedef  dword                                   WireWidth;

   const WireWidth        MAX_WIRE_WIDTH          = 0x0FFFFFFF;

   //==============================================================================
   class Validator {
   public:
                           Validator(const PointVector& plist) : _status(shp_OK),
                                                            _plist(plist) {};
      virtual             ~Validator() {};
      bool                 valid()           {return _status < shp_valid;}
      bool                 recoverable()     {return _status < shp_null;}
      byte                 status()          {return _status;}
      bool                 box()             {return (0 != (_status & shp_box));}
      PointVector          getValidated()    {return _plist;}
      word                 numpoints()       {return _plist.size();}
      virtual std::string  failType() = 0;
      virtual TdtData*     replacement() = 0;
   protected:
      unsigned             _status;
      PointVector          _plist;
   };

   //==============================================================================
   /*!
    * File compression explained: A plenty of compression algorithms out there -
    * Toped deals with two of them and the reason is that those are suitable for
    * layout file purposes and covered by wx library at the time of writing.
    *  - zip (http://en.wikipedia.org/wiki/ZIP_%28file_format%29) - compression
    *    algorithm and archiver
    *  - gzip stream compression algorithm using Lempel-Ziv coding (LZ77).
    *    (http://en.wikipedia.org/wiki/Gzip). Several implementations of this
    *    algo, but wx is using Zlib (http://www.zlib.net/)
    *
    *  From our prospective the big difference between the two is that the
    *  first one is also an archiver which means that a zip file can contain
    *  more than one file. gzip on the other hand is a stream compression which
    *  means that it contains a single file. In Linux traditionally tar is
    *  used as archiver and then the entire archive is compressed using gzip
    *
    *  Another quite important feature of both formats is that (as it appears
    *  at least in the wx implementation) both of them are not seekable. In
    *  other words they are not randomly accessible.
    *
    *  Having in mind all the above and the general pattern Toped is following
    *  for all imports (i.e. two stage conversion as described on the web site
    *  http://toped.org.uk/trm_ifaces.html) here is the general idea how the
    *  compressed input files are handled:
    *  - zip files - opened before the conversion. If they contain a single file
    *    it is inflated (decompressed) in a temporary location and then the new
    *    file is used in all conversion stages. If the original file contains more
    *    than one file - the processing is aborted and conversion is rejected.
    *  - gzip files - used "as is" in the first import stage where the access is
    *    sequential. Before the second stage, which requires random access the
    *    file is inflated in a temporary location and the product is used for the
    *    conversion.
    */
   class InputDBFile {
      public:
                              InputDBFile( wxString fileName, bool _forceSeek);
         virtual             ~InputDBFile();
         bool                 readStream(void*, size_t, bool updateProgress = false);
         size_t               readTextStream(char*, size_t);
         void                 closeStream();
         std::string          fileName()                       { return std::string(_fileName.mb_str(wxConvFile));}
         wxFileOffset         fileLength() const               { return _fileLength;   }
         wxFileOffset         filePos() const                  { return _filePos;      }
         bool                 status() const                   { return _status;       }
         void                 setStatus(bool stat)             {        _status = stat;}
      protected:
         void                 initFileMetrics(wxFileOffset);
         void                 setFilePos(wxFileOffset fp)      { _filePos = fp;     }
         bool                 unZlib2Temp();//! inflate the input zlib file in a temporary one
         bool                 unZip2Temp() ;//! inflate the input zip file in a temporary one
         wxInputStream*       _inStream    ;//! The input stream of the opened file
         bool                 _gziped      ;//! Indicates that the file is in compressed with gzip
         bool                 _ziped       ;//! Indicates that the file is in compressed with zip
         bool                 _forceSeek   ;//! Seekable stream requested
         wxString             _fileName    ;//! A fully validated name of the file. Path,extension, everything
         wxString             _tmpFileName ;//! The name of the eventually deflated file (if the input is compressed)
      private:
         wxFileOffset         _fileLength  ;//! The length of the file in bytes
         wxFileOffset         _filePos     ;//! Current position in the file
         wxFileOffset         _progresPos  ;//! Current position of the progress bar (Toped status line)
         wxFileOffset         _progresMark ;//! Marked  position of the progress bar (Toped status line)
         wxFileOffset         _progresStep ;//! Update step of the progress bar (Toped status line)
         unsigned const       _progresDivs ;//! Number of updates to the progress bar during the current operation
         bool                 _status      ;//! Used only in the constructor if the file can't be

   };

//==============================================================================
   class InputTdtFile : public InputDBFile {
      public:
                              InputTdtFile( wxString fileName, laydata::TdtLibDir* tedlib );
         virtual             ~InputTdtFile() {};
         void                 read(int libRef);
         void                 getCellChildNames(NameSet&);
         CellDefin            linkCellRef(std::string cellname);
         void                 cleanup();
         byte                 getByte();
         word                 getWord();
         int4b                get4b();
         WireWidth            get4ub();
         real                 getReal();
         std::string          getString();
         TP                   getTP();
         CTM                  getCTM();
         TdtLibrary*          design() const       {return _design;};
         const TdtLibDir*     TEDLIB()             {return _TEDLIB;}
         word                 revision() const     {return _revision;}
         word                 subRevision() const  {return _subrevision;}
         time_t               created() const      {return _created;};
         time_t               lastUpdated() const  {return _lastUpdated;};
      private:
         void                 getFHeader();
         void                 getRevision();
         void                 getTime();
         laydata::TdtLibDir*  _TEDLIB      ;//! Catalog of available TDT libraries (reference to DATC->_TEDLIB)
         TdtLibrary*          _design      ;//! A design created in memory from the contents of the input file
         word                 _revision    ;//! Revision (major) of the TDT format which this file carries
         word                 _subrevision ;//! Revision (minor) of the TDT format which this file carries
         time_t               _created     ;//! Time stamp indicating when the DB (not the file!) was created
         time_t               _lastUpdated ;//! Time stamp indicating when the DB (not the file!) was updated for the last time
         NameSet              _childnames  ;
   };

   class   TEDfile {
   public:
                           TEDfile(std::string&, laydata::TdtLibDir*); // for writing
      void                 closeF() {fclose(_file);};
      void                 putString(std::string str);
      void                 putReal(const real);
      void                 putByte(const byte ch) {fputc(ch, _file);};
      void                 putWord(const word);
      void                 put4b(const int4b);
      void                 put4ub(const WireWidth);
      void                 putTP(const TP*);
      void                 putCTM(const CTM);
      void                 registerCellWritten(std::string);
      bool                 checkCellWritten(std::string);
   protected:
   private:
      void                 putTime();
      void                 putRevision();
      FILE*                _file;
      word                 _revision;
      word                 _subrevision;
      time_t               _lastUpdated;
      TdtLibrary*          _design;
      NameSet              _childnames;
   };

   class ArrayProps
   {
      public:
         ArrayProps() : _colStep(0,0), _rowStep(0,0), _cols(0), _rows(0) {}
//         ArrayProperties(int4b stepX, int4b stepY, word cols, word rows) :
//            _colStep(stepX,0), _rowStep(0,stepY), _cols(cols), _rows(rows) {}
         ArrayProps(const TP& colStep, const TP& rowStep, word cols, word rows) :
            _colStep(colStep), _rowStep(rowStep), _cols(cols), _rows(rows) {}
         bool                valid() {return ((_cols != 0) && (_rows != 0));}
         const TP&           colStep() const {return _colStep;}
         const TP&           rowStep() const {return _rowStep;}
         TP                  displ(int col, int row) const {return TP((_colStep.x() * col) + (_rowStep.x() * row),
                                                                      (_colStep.y() * col) + (_rowStep.y() * row) );}
         word                cols()    const {return _cols;}
         word                rows()    const {return _rows;}
      private:
         TP                  _colStep;
         TP                  _rowStep;
         word                _cols;
         word                _rows;
   };

   bool pathConvert(PointVector&, int4b, int4b );


   /**
    * The primary subject of this class is to generate the wire contour
    * from its central line. It is supposed to deal with all of the corner
    * cases and especially when the wire contains collinear segments. The
    * class requires the point data in integer array format. It is not
    * copying the original data - just stores a pointer to it. Thats' why
    * there is no destructor defined.
    */
   class WireContour {
      public:
                           WireContour(const int4b*, unsigned, const WireWidth);
         unsigned          csize()         {return _cdata.size(); } //! return the number of the contour points
         unsigned          lsize()         {return _lsize;        } //! return the number of the central line points
         void              getArrayData(int4b*);
         void              getVectorData(PointVector&);
         DBbox             getCOverlap();
      private:
         typedef std::list<TP> PointList;
         void              endPnts(word, word, bool);
         void              mdlPnts(word, word, word);
         void              mdlAcutePnts(word, word, word, int, int);
         byte              chkCollinear(word,word,word);
         void              colPnts(word,word,word);
         TP                mdlCPnt(word, word);
         int               orientation(word, word, word);
         double            getLambda(word i1, word i2, word ii);
         int               xangle(word i1, word i2);
         const int4b*      _ldata; //! The original wire central line. Do not delete it. Do not alter it!
         const unsigned    _lsize; //! The number of points in the wire central line
         const WireWidth   _width; //! The width of the wire
         PointList         _cdata; //! The generated contour line in a list of points form
   };

   /**
    * An auxiliary class to wrap around the WireContour class. It makes WireContour
    * usable with a point list input data or with data which needs coordinate
    * transformations. Also it defines a method to dump the wire contour in a PointVector
    * format which is usable by the basic renderer.
    */
   class WireContourAux {
      public:
                          WireContourAux(const int4b*, unsigned, const WireWidth, const CTM&);
                          WireContourAux(const PointVector&, const WireWidth);
                          WireContourAux(const PointVector&, const WireWidth, const TP);
                         ~WireContourAux();
         void             getRenderingData(PointVector&);
         void             getLData(PointVector&);
         void             getCData(PointVector&);
      private:
         WireContour*     _wcObject;
         int4b*           _ldata;
   };

}

class DbExportFile {
   public:
                              DbExportFile(std::string fn, laydata::TdtCell* topcell, bool recur) :
                                 _fileName(fn), _topcell(topcell), _recur(recur), _DBU(1e-9), _UU(1e-3) {};
      virtual                ~DbExportFile() {};
      virtual void            definitionStart(std::string) = 0;
      virtual void            definitionFinish() = 0;
      virtual void            libraryStart(std::string, TpdTime&, real, real) = 0;
      virtual void            libraryFinish() = 0;
      virtual bool            layerSpecification(unsigned) = 0;
      virtual void            box(const int4b* const) = 0;
      virtual void            polygon(const int4b* const, unsigned) = 0;
      virtual void            wire(const int4b* const, unsigned, unsigned) = 0;
      virtual void            text(const std::string&, const CTM&) = 0;
      virtual void            ref(const std::string&, const CTM&) = 0;
      virtual void            aref(const std::string&, const CTM&, const laydata::ArrayProps&) = 0;
      virtual bool            checkCellWritten(std::string) const = 0;
      virtual void            registerCellWritten(std::string) = 0;
      const laydata::TdtCell* topcell() const   {return _topcell; }
      bool                    recur() const     {return _recur;   }
      real                    DBU() const       {return _DBU;     }
      real                    UU() const        {return _UU;      }
   protected:
      std::string             _fileName;  //! Output file name - including the path
      laydata::TdtCell*       _topcell;
      bool                    _recur;
      real                    _DBU;
      real                    _UU;
};

class ForeignCell;
class ImportDB;
typedef SGHierTree<ForeignCell> ForeignCellTree;
typedef std::list<ForeignCell*> ForeignCellList;
class ForeignDbFile : public laydata::InputDBFile {
   public:
                           ForeignDbFile(wxString, bool);
      virtual             ~ForeignDbFile();
      bool                 reopenFile();
      void                 setPosition(wxFileOffset);
      std::string          getFileNameOnly() const;
      virtual double       libUnits() const = 0;
      virtual void         hierOut() = 0 ;
      virtual std::string  libname() const = 0;
      virtual void         getTopCells(NameList&) const = 0;
      virtual void         getAllCells(wxListBox&) const = 0;
      virtual void         convertPrep(const NameList&, bool) = 0;
      // If you hit any of the asserts below - it most likely means that you're using wrong
      // combination of ForeignDbFile extend class type and parameters for this function call
      // ExtLayers is used for GDS/OASIS, NameList is used for CIF
      virtual void         collectLayers(ExtLayers&) const {assert(false);}
      virtual void         collectLayers(NameList& ) const {assert(false);}
      virtual bool         collectLayers(const std::string&, ExtLayers&) const {assert(false); return false;}
      virtual bool         collectLayers(const std::string&, NameList& ) const {assert(false); return false;}
      ForeignCellList&     convList()                       { return _convList;  }
      ForeignCellTree*     hierTree()                       { return _hierTree;}
   protected:
      void                 preTraverseChildren(const ForeignCellTree*);
      ForeignCellList      _convList   ;//! The list of cells for conversion in bottom-up order
      ForeignCellTree*     _hierTree   ;//! Tree of instance hierarchy
      wxFileOffset         _convLength ;//! The amount of data (in bytes) subjected to conversion
};

//==========================================================================
class ForeignCell {
   public:
                           ForeignCell() : _strctName(""), _traversed(false),
                                    _haveParent(false), _filePos(0), _cellSize(0) {};
      virtual void         import(ImportDB&) = 0;
      bool                 traversed() const                { return _traversed;    }
      void                 set_traversed(bool tf)           { _traversed = tf;      }
      std::string          strctName() const                { return _strctName;    }
      void                 setStrctName(std::string  nm)    { _strctName = nm;      }
      int                  libID() const                    { return TARGETDB_LIB;  } // to cover the requirements of the hierarchy template
      bool                 haveParent() const               { return _haveParent;   }
      wxFileOffset         strSize() const                  { return _cellSize;     }
   protected:
      std::string          _strctName  ;//! Name of the Cell
      bool                 _traversed  ;//! Indicates the cell was already traversed during the conversion
      bool                 _haveParent ;//! Indicates that the cell is referenced
      wxFileOffset         _filePos    ;//! The starting position of the cell description in the input stream
      wxFileOffset         _cellSize   ;//! The size (in byte) of the cell description in the input file
};

//==========================================================================
// If you hit any of the asserts below - it most likely means that you're using wrong
// combination of ForeignDbFile extend class type and parameters for this function call
// ExtLayers is used for GDS/OASIS, NameList is used for CIF
class LayerCrossMap {
   public:
                              LayerCrossMap() : _tdtLayNumber(0), _tmpLayer(NULL) {}
      laydata::QTreeTmp*      getTmpLayer()     {return _tmpLayer;}
      virtual bool            mapTdtLay(laydata::TdtCell*, word, word)
                                                         {assert(false); return false;}
      virtual bool            mapTdtLay(laydata::TdtCell*,const std::string&)
                                                         {assert(false); return false;}
      virtual std::string     printSrcLayer() const      {assert(false); return std::string("");}
   protected:
      word                    _tdtLayNumber  ; //! Current layer number
      laydata::QTreeTmp*      _tmpLayer      ; //! Current target layer
};

class ENumberLayerCM : public LayerCrossMap {
   public:
                              ENumberLayerCM(const LayerMapExt& lmap) :
                                 _layMap(lmap), _extLayNumber(0),
                                 _extDataType(0) {}
      virtual bool            mapTdtLay(laydata::TdtCell*,word, word);
      virtual std::string     printSrcLayer() const;
   private:
      const LayerMapExt&      _layMap;
      word                    _extLayNumber;
      word                    _extDataType;
};

class ENameLayerCM : public LayerCrossMap {
   public:
                              ENameLayerCM(const SIMap& lmap) :
                                 _layMap(lmap), _extLayName("") {}
      virtual bool            mapTdtLay(laydata::TdtCell*, const std::string&);
      virtual std::string     printSrcLayer() const;
   private:
      const SIMap&            _layMap;
      std::string             _extLayName;
};

//==========================================================================
class ImportDB {
   public:
                              ImportDB(ForeignDbFile*, laydata::TdtLibDir*, const LayerMapExt&);
                              ImportDB(ForeignDbFile*, laydata::TdtLibDir*, const SIMap&, real);
                             ~ImportDB();
      void                    run(const NameList&, bool, bool reopenFile = true);
      bool                    mapTdtLayer(std::string);
      bool                    mapTdtLayer(word, word);
      void                    addBox(const TP&, const TP&);
      void                    addPoly(PointVector&);
      void                    addPath(PointVector&, int4b, short pathType = 0, int4b bgnExtn = 0, int4b endExtn = 0);
      void                    addText(std::string, TP, double magnification, double angle = 0, bool reflection = false);
      void                    addRef(std::string, TP, double, double, bool);
      void                    addRef(std::string, CTM);
      void                    addARef(std::string, TP, double, double, bool, laydata::ArrayProps&);
      void                    calcCrossCoeff(real cc) { _crossCoeff = _dbuCoeff * cc;}
      ForeignDbFile*          srcFile()               { return _src_lib;   }
      real                    technoSize()            { return _technoSize;}
      real                    crossCoeff()            { return _crossCoeff;}
   protected:
      void                    convert(ForeignCell*, bool);
      bool                    polyAcceptable(PointVector&, bool&);
      bool                    pathAcceptable(PointVector&, int4b);
      LayerCrossMap*          _layCrossMap   ;
      ForeignDbFile*          _src_lib       ;
      laydata::TdtLibDir*     _tdt_db        ;
      laydata::TdtCell*       _dst_structure ; //! Current target structure
      real                    _dbuCoeff      ; //! The DBU ratio between the foreign and local DB
      real                    _crossCoeff    ; //! Current cross coefficient
      real                    _technoSize    ; //! technology size (used for conversion of some texts)
};


class PSFile;
#endif
