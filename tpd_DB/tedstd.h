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
#define TED_CUR_SUBREVISION   7

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
      shp_OK         = 0x00,
      shp_ident      = 0x01, // identical or one line points removed
      shp_clock      = 0x02, // points reordered to get anti clockwise order
      shp_box        = 0x04, // shape is a box
      shp_acute      = 0x08, // acute angle
      shp_collinear  = 0x10, // collinear wire
      // critical
      shp_cross      = 0x40, // self crossing sequence
      shp_null       = 0x80, // 0 area - points are not forming a polygon
   } shape_status;

   class TdtData;
   class EditObject;
   class TdtCell;
   class TdtDefaultCell;
   class TdtCellRef;
   class TdtDesign;
   class TdtLibrary;
   class TdtLibDir;
   typedef  std::pair<TdtData*, SGBitSet>           SelectDataPair;
   typedef  std::list<SelectDataPair>               DataList;
   typedef  std::map<unsigned, DataList*>           SelectList;
   typedef  std::list<TdtData*>                     ShapeList;
   typedef  std::map<unsigned,ShapeList*>           AtticList;
   typedef  std::map<std::string, TdtDefaultCell*>  CellList;
   typedef  TdtDefaultCell*                         CellDefin;
   typedef  std::deque<const TdtCellRef*>           CellRefStack;
   typedef  std::deque<EditObject*>                 EditCellStack;
   typedef  std::list<const CellList*>              LibCellLists;
   typedef  std::list<TdtDefaultCell*>              CellDefList;

   //==============================================================================
   class Validator {
   public:
                           Validator(const pointlist& plist) : _status(shp_OK),
                                                            _plist(plist) {};
      virtual             ~Validator() {};
      bool                 valid()           {return _status < shp_cross;}
      bool                 recoverable()     {return _status < shp_null;}
      byte                 status()          {return _status;}
      bool                 box()             {return (0 != (_status & shp_box));}
      pointlist            getValidated()    {return _plist;}
      word                 numpoints()       {return _plist.size();}
      virtual std::string  failType() = 0;
      virtual TdtData*     replacement() = 0;
   protected:
      byte                 _status;
      pointlist            _plist;
   };

//==============================================================================
   class   TEDfile {
   public:
                           TEDfile(const char*, laydata::TdtLibDir*); // for reading
                           TEDfile(std::string&, laydata::TdtLibDir*); // for writing
      void                 closeF() {fclose(_file);};
      void                 read(int libRef);
      void                 cleanup();
      std::string          getString();
      void                 putString(std::string str);
      real                 getReal();
      void                 putReal(const real);
      byte                 getByte();
      void                 putByte(const byte ch) {fputc(ch, _file);};
      word                 getWord();
      void                 putWord(const word);
      int4b                get4b();
      void                 put4b(const int4b);
      TP                   getTP();
      void                 putTP(const TP*);
      CTM                  getCTM();
      void                 putCTM(const CTM);
      void                 registerCellWritten(std::string);
      bool                 checkCellWritten(std::string);
      CellDefin            linkCellRef(std::string cellname);
      void                 getCellChildNames(NameSet&);
      bool                 status() const  {return _status;};
      word                 numRead() const {return _numread;};
      TdtLibrary*          design() const  {return _design;};
      time_t               created() const {return _created;};
      time_t               lastUpdated() const {return _lastUpdated;};
      const laydata::TdtLibDir* TEDLIB()   {return _TEDLIB;}
      word                 revision()      {return _revision;}
      word                 subRevision()   {return _subrevision;}
   protected:
      bool                 _status;
      word                 _numread;
   private:
      void                 getFHeader();
      void                 getTime();
      void                 putTime();
      void                 getRevision();
      void                 putRevision();
      long int             _position;
      FILE*                _file;
      word                 _revision;
      word                 _subrevision;
      time_t               _created;
      time_t               _lastUpdated;
      TdtLibrary*          _design;
      NameSet              _childnames;
      laydata::TdtLibDir*  _TEDLIB;       // catalog of available TDT libraries

   };

   class ArrayProperties
   {
      public:
         ArrayProperties() : _stepX(0), _stepY(0), _cols(0), _rows(0) {}
         ArrayProperties(int4b stepX, int4b stepY, word cols, word rows) :
            _stepX(stepX), _stepY(stepY), _cols(cols), _rows(rows) {}
         bool                valid() {return ((_cols != 0) && (_rows != 0));}
         int4b               stepX() const {return _stepX;}
         int4b               stepY() const {return _stepY;}
         word                cols()  const {return _cols;}
         word                rows()  const {return _rows;}
      private:
         int4b               _stepX;
         int4b               _stepY;
         word                _cols;
         word                _rows;
   };

   bool pathConvert(pointlist&, word, int4b, int4b );


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
                           WireContour(const int4b*, unsigned, const word);
         unsigned          csize()         {return _cdata.size(); } //! return the number of the contour points
         unsigned          lsize()         {return _lsize;        } //! return the number of the central line points
         void              getArrayData(int4b*);
         void              getVectorData(pointlist&);
         DBbox             getCOverlap();
      private:
         typedef std::list<TP> PointList;
         void              endPnts(word, word, bool);
         void              mdlPnts(word, word, word);
         byte              chkCollinear(word,word,word);
         void              colPnts(word,word,word);
         TP                mdlCPnt(word, word);
         int               orientation(word, word, word);
         float             getLambda(word i1, word i2, word ii);
         const int4b*      _ldata; //! The original wire central line. Do not delete it. Do not alter it!
         const unsigned    _lsize; //! The number of points in the wire central line
         const word        _width; //! The width of the wire
         PointList         _cdata; //! The generated contour line in a list of points form
   };

   /**
    * An auxiliary class to wrap around the WireContour class. It makes WireContour
    * usable with a point list input data or with data which needs coordinate
    * transformations. Also it defines a method to dump the wire contour in a pointlist
    * format which is usable by the basic renderer.
    */
   class WireContourAux {
      public:
                          WireContourAux(const int4b*, unsigned, const word, const CTM&);
                          WireContourAux(const pointlist&, const word);
                         ~WireContourAux();
         void             getRenderingData(pointlist&);
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
      virtual void            aref(const std::string&, const CTM&, const laydata::ArrayProperties&) = 0;
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

class PSFile;
#endif
