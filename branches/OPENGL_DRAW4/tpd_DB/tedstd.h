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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
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
#include "tedbac.h"

//==============================================================================
// Toped DaTa (TDT) file markers
//==============================================================================
const std::string TED_LEADSTRING("TED");
const byte tedf_REVISION        = 0x02;
const byte tedf_TIMECREATED     = 0x03;
const byte tedf_TIMEUPDATED     = 0x04;
const byte tedf_DESIGN          = 0x80;
const byte tedf_DESIGNEND       = 0x81;
const byte tedf_CELL            = 0x82;
const byte tedf_CELLEND         = 0x83;
const byte tedf_LAYER           = 0x84;
const byte tedf_CELLREF         = 0x85;
const byte tedf_CELLAREF        = 0x86;
const byte tedf_BOX             = 0x87;
const byte tedf_POLY            = 0x88;
const byte tedf_WIRE            = 0x89;
const byte tedf_TEXT            = 0x8A;
const byte tedf_LAYEREND        = 0x8B;
const byte tedf_REFS            = 0x8C;
const byte tedf_REFSEND         = 0x8D;
const byte tedf_GRC             = 0x8E;
const byte tedf_GRCEND          = 0x8F;
const byte tedf_DATATYPE        = 0x90;
//
const byte TED_CUR_REVISION     = 0x00;
const byte TED_CUR_SUBREVISION  = 0x0B;

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

namespace logicop {
   class  CrossFix;
}

namespace laydata {

   typedef enum {
      shp_OK         = 0x0000,
      shp_ident      = 0x0001, // identical or one line points removed
      shp_clock      = 0x0002, // points reordered to get anti clockwise order
      shp_box        = 0x0004, // shape is a box
      shp_acute      = 0x0008, // acute angle
      shp_collinear  = 0x0010, // collinear wire
      // ambiguous ( can be recovered eventually )
      shp_cross      = 0x0100, // self crossing sequence
      shp_shortends  = 0x0200, // wire with end segments shorter than the width
      // critical  ( unrecoverable, will be rejected )
      shp_null       = 0x0400, // 0 area - points are not forming a polygon
      shp_width      = 0x0800, // wire with width bigger than MAX_WIRE_WIDTH
      shp_exception  = 0x1000  // exception encountered during the shape checks
   } shape_status;

   class EditObject;
   class TdtCell;
   class TdtDefaultCell;
//   class TdtCellRef;
   class TdtDesign;
   class TdtLibrary;
   class TdtLibDir;
   typedef  LayerContainer<DataList*>               SelectList;
   typedef  LayerContainer<ShapeList*>              AtticList;
   typedef  std::map<std::string, TdtDefaultCell*>  CellMap;
   typedef  TdtDefaultCell*                         CellDefin;
//   typedef  std::deque<const TdtCellRef*>           CellRefStack;
   typedef  std::deque<EditObject*>                 EditCellStack;
   typedef  std::list<const CellMap*>               LibCellLists;
   typedef  std::list<TdtDefaultCell*>              CellDefList;

   //==============================================================================
   class Validator {
   public:
                           Validator(const PointVector& plist) : _status(shp_OK),
                                                            _plist(plist) {};
      virtual             ~Validator() {};
      bool                 valid()           {return _status < ambiguous();}
      bool                 acceptable()      {return _status < critical();}
      int                  status()          {return _status;}
      bool                 box()             {return (0 != (_status & shp_box));}
      bool                 crossing()        {return (0 != (_status & shp_cross));}
      bool                 shortSegments()   {return (0 != (_status & shp_shortends));}
      PointVector          getValidated()    {return _plist;}
      word                 numpoints()       {return _plist.size();}
      virtual std::string  failType() = 0;
      virtual ShapeList*   replacements() = 0;
   protected:
      virtual shape_status critical()        {return ambiguous();}
      shape_status         ambiguous()       {return shp_cross;}
      int                  _status;
      PointVector          _plist;
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


}

namespace auxdata {
   class GrcCell;
   typedef std::map<std::string, GrcCell*>  GrcCellMap;
}

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
                           InputDBFile( const wxString& fileName, bool _forceSeek);
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
      laydata::CellDefin   linkCellRef(std::string cellname);
      void                 cleanup();
      byte                 getByte();
      word                 getWord();
      LayerDef             getLayer();
      int4b                get4b();
      WireWidth            get4ub();
      real                 getReal();
      std::string          getString();
      TP                   getTP();
      CTM                  getCTM();
      laydata::TdtLibrary* design() const       {return _design;};
      const laydata::TdtLibDir*
                           TEDLIB()             {return _TEDLIB;}
      word                 revision() const     {return _revision;}
      word                 subRevision() const  {return _subrevision;}
      time_t               created() const      {return _created;};
      time_t               lastUpdated() const  {return _lastUpdated;};
   private:
      void                 getFHeader();
      void                 getRevision();
      void                 getTime();
      laydata::TdtLibDir*  _TEDLIB      ;//! Catalog of available TDT libraries (reference to DATC->_TEDLIB)
      laydata::TdtLibrary* _design      ;//! A design created in memory from the contents of the input file
      word                 _revision    ;//! Revision (major) of the TDT format which this file carries
      word                 _subrevision ;//! Revision (minor) of the TDT format which this file carries
      time_t               _created     ;//! Time stamp indicating when the DB (not the file!) was created
      time_t               _lastUpdated ;//! Time stamp indicating when the DB (not the file!) was updated for the last time
      NameSet              _childnames  ;
};

class   OutputTdtFile {
public:
                        OutputTdtFile(std::string&, laydata::TdtLibDir*);
   void                 closeF() {fclose(_file);};
   void                 putString(std::string str);
   void                 putReal(const real);
   void                 putByte(const byte ch) {fputc(ch, _file);};
   void                 putWord(const word);
   void                 putLayer(const LayerDef&);
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
   laydata::TdtLibrary* _design;
   NameSet              _childnames;
};

class DbExportFile {
   public:
                              DbExportFile(std::string fn, laydata::TdtCell* topcell, bool recur) :
                                 _fileName(fn), _topcell(topcell), _recur(recur), _DBU(1e-9), _UU(1e-3) {};
      virtual                ~DbExportFile() {};
      virtual void            definitionStart(std::string) = 0;
      virtual void            definitionFinish() = 0;
      virtual void            libraryStart(std::string, TpdTime&, real, real) = 0;
      virtual void            libraryFinish() = 0;
      virtual bool            layerSpecification(const LayerDef&) = 0;
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
class ForeignDbFile : public InputDBFile {
   public:
                           ForeignDbFile(const wxString&, bool);
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
      virtual             ~ForeignCell() {}
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
                              LayerCrossMap() : _tdtLayNumber(TLL_LAY_DEF), _tmpLayer(NULL) {}
      virtual                ~LayerCrossMap()  {}
      laydata::QTreeTmp*      getTmpLayer()     {return _tmpLayer;}
      LayerDef                tdtLayNumber()    {return _tdtLayNumber;}
      virtual bool            mapTdtLay(laydata::TdtCell*, word, word)
                                                         {assert(false); return false;}
      virtual bool            mapTdtLay(laydata::TdtCell*,const std::string&)
                                                         {assert(false); return false;}
      virtual std::string     printSrcLayer() const      {assert(false); return std::string("");}
   protected:
      LayerDef                _tdtLayNumber  ; //! Current layer number
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
                              ENameLayerCM(const ImpLayMap& lmap) :
                                 _layMap(lmap), _extLayName("") {}
      virtual bool            mapTdtLay(laydata::TdtCell*, const std::string&);
      virtual std::string     printSrcLayer() const;
   private:
      const ImpLayMap&         _layMap;
      std::string             _extLayName;
};

//==========================================================================
class ImportDB {
   public:
                              ImportDB(ForeignDbFile*, laydata::TdtLibDir*, const LayerMapExt&);
                              ImportDB(ForeignDbFile*, laydata::TdtLibDir*, const ImpLayMap&, real);
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
      auxdata::GrcCell*       _grc_structure ; //! Current error structure
      real                    _dbuCoeff      ; //! The DBU ratio between the foreign and local DB
      real                    _crossCoeff    ; //! Current cross coefficient
      real                    _technoSize    ; //! technology size (used for conversion of some texts)
};


class PSFile;
#endif
