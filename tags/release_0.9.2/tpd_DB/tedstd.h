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
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"

//==============================================================================
// Toped DaTa (TDT) file markers
//==============================================================================
#define TED_LEADSTRING     "TED"
#define tedf_REVISION      0x02
#define tedf_TIMECREATED   0x03
#define tedf_TIMEUPDATED   0x04
#define tedf_DESIGN        0x80
#define tedf_DESIGNEND     0x81
#define tedf_CELL          0x82
#define tedf_CELLEND       0x83
#define tedf_LAYER         0x84
#define tedf_CELLREF       0x85
#define tedf_CELLAREF      0x86
#define tedf_BOX           0x87
#define tedf_POLY          0x88
#define tedf_WIRE          0x89
#define tedf_TEXT          0x8A
#define tedf_LAYEREND      0x8B

//==============================================================================   
class PSegment {
public:
               PSegment() : _A(0), _B(0), _C(0) {};
               PSegment(real A, real B, real C) : _A(A), _B(B), _C(C) {};
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

namespace layprop {
   class DrawProperties;
}

namespace GDSin {
   class GdsFile;
}

namespace CIFin {
   class CifExportFile;
}

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

   typedef enum { sh_active, sh_deleted, sh_selected, sh_partsel, sh_merged } SH_STATUS;
   typedef enum {
      shp_OK         = 0x00,
      shp_ident      = 0x01, // identical or one line points removed
      shp_clock      = 0x02, // poits reordered to get antoclockwise order
      shp_box        = 0x04, // shape is a box
      shp_acute      = 0x08, // acute angle
      // critical
      shp_cross      = 0x40, // self crossing sequence
      shp_null       = 0x80, // 0 area - points are not forming a polygon
   } shape_status;
   
   class TEDfile;
   class tdtdata;
   class editobject;
   class tdtcell;
   class tdtdefaultcell;
   class tdtcellref;
   class tdtdesign;
   class tdtlibrary;
   class tdtlibdir;
   typedef  std::pair<tdtdata*, SGBitSet>           selectDataPair;
   typedef  std::list<selectDataPair>               dataList;
   typedef  std::map<word, dataList*>               selectList;
   typedef  std::list<tdtdata*>                     shapeList;
   typedef  std::map<word,shapeList*>               atticList;
   typedef  std::map<std::string, tdtdefaultcell*>  cellList;
   typedef  cellList::const_iterator                refnamepair;
   typedef  std::deque<const tdtcellref*>           cellrefstack;
   typedef  std::deque<editobject*>                 editcellstack;
   typedef  std::list<const cellList*>              LibCellLists;

   //==============================================================================
   class validator {
   public:
                           validator(const pointlist& plist) : _status(shp_OK), 
                                                            _plist(plist) {};
                           validator() : _status(shp_OK) {};
      bool                 valid()           {return _status < shp_cross;}
      bool                 recoverable()     {return _status < shp_null;}
      byte                 status()          {return _status;}
      bool                 box()             {return (0 != (_status & shp_box));}
      pointlist&           get_validated()   {return _plist;}
      word                 numpoints()       {return _plist.size();}
      virtual char*       failtype() = 0;
      virtual tdtdata*    replacement() = 0;
      virtual            ~validator() {};
   protected:
      byte                 _status;
      pointlist            _plist;
   };
   
//==============================================================================
   class   TEDfile {
   public:
                           TEDfile(const char*, laydata::tdtlibdir*); // for reading
                           TEDfile(std::string&, laydata::tdtlibdir*); // for writing
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
      void                 registercellwritten(std::string);
      bool                 checkcellwritten(std::string);
      refnamepair          linkcellref(std::string cellname);
      void                 get_cellchildnames(nameList*);
      bool                 status() const  {return _status;};
      word                 numread() const {return _numread;};
      tdtlibrary*          design() const  {return _design;};
      time_t               created() const {return _created;};
      time_t               lastUpdated() const {return _lastUpdated;};
      const laydata::tdtlibdir* TEDLIB() {return _TEDLIB;}
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
      tdtlibrary*          _design;
      nameList             _childnames;
      laydata::tdtlibdir*  _TEDLIB;       // catalog of available TDT libraries

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

   real polyarea(pointlist const shape);
}

#endif
