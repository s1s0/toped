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
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Reader of Mentor Graphics Calibre drc errors files
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#if !defined(CALBR_READER_H_INCLUDED)
#define CALBR_READER_H_INCLUDED

#include <fstream>
#include <string>
#include <vector>
#include <wx/wx.h>
#include "ttt.h"
#include "outbox.h"
#include "tedesign.h"

namespace Calbr
{

 /*  struct coord
   {
      real x, y;
   };*/

   struct edge
   {
      long x1, y1, x2, y2;
   };

   typedef std::vector <TP> CoordsVector;


   class drcRenderer
   {
      public:
                              drcRenderer() {};
         virtual              ~drcRenderer() {};
         virtual void         startWriting(const std::string &cell)=0;
         virtual void         setError(unsigned int numError) {};
         virtual bool         showError(unsigned int numError) {return false;}
         virtual void         showAll() {};
         virtual void         hideAll() {};
         void                 setTranformation(CTM &ctm) {_ctm = ctm;};
         virtual void         addPoly(const CoordsVector   &coords)=0;
         virtual void         addLine(const edge &edge)=0;
         virtual void         endWriting()=0;
         virtual void         zoom(const edge &edge)=0;
         void                 setCellName(const std::string & cellName) {_cellName = cellName;};
      protected:
         std::string          _cellName;
         CTM                  _ctm;
   };



   class drcEdge
   {
      public:
                         drcEdge(long ordinal, drcRenderer*   render) { _ordinal = ordinal; _render = render;};
         void            addCoord(long x1, long y1, long x2, long y2);
         edge*           coords() {return &_coords;};
         edge            getZoom() const;
         long            ordinal() const {return _ordinal;};
         void            addError();
         static long     _precision;
      private:
         edge            _coords;
         long            _ordinal;
         drcRenderer*    _render;
   };

   class drcPolygon
   {
      public:
                         drcPolygon(long ordinal, drcRenderer*   render) { _ordinal = ordinal; _render = render;};
         void            addCoord(long x, long y);
         CoordsVector*   coords() {return &_coords;};
         edge            getZoom()  const;
         long            ordinal() const {return _ordinal;};
         void            addError();
         static long     _precision;
      private:
         CoordsVector    _coords;
         long            _ordinal;
         drcRenderer*    _render;
   };

   class drcRuleCheck; //forward declaration
   typedef std::vector <Calbr::drcRuleCheck*> RuleChecksVector;


   struct cellNameStruct
   {
      bool              spaceCoords;
      CTM               transfMatrix;
      RuleChecksVector  _RuleChecks;
   };

   class drcRuleCheck
   {
      public:
                              drcRuleCheck(unsigned int num, const std::string &name);
                              drcRuleCheck(const drcRuleCheck& ruleCheck);
                              ~drcRuleCheck();

         unsigned int         num(void) const {return _num;};
         std::string          ruleCheckName()   const {return _ruleCheckName;}
         std::string          timeStamp()       const {return _timeStamp;}
         long                 curResCount()     const {return  _curResCount;}
         long                 origResCount()    const {return _origResCount;}
         std::vector <Calbr::drcPolygon>* polygons() {return &_polygons;};
         std::vector <Calbr::drcEdge>* edges() {return &_edges;};
         void                 setTimeStamp(const std::string &timeStamp);
         void                 setCurResCount(int curResCount);
         void                 setOrigResCount(int origResCount);
         void                 addDescrString(const std::string & str);
         void                 addPolygon(const Calbr::drcPolygon &poly);
         void                 addEdge(const Calbr::drcEdge &theEdge);
         edge                 getZoom(long ordinal);
         edge                 getZoom(void);
      private:
         unsigned int         _num;//TODO Replace with LayerDef
         long                 _curResCount; //current result count
         long                 _origResCount;//original result count
         std::string          _ruleCheckName;
         std::string          _timeStamp;
         std::string          _header;
         edge                 _border;
         bool                 _borderInit;
         std::vector <std::string> _descrStrings;
         std::vector <Calbr::drcPolygon> _polygons;
         std::vector <Calbr::drcEdge> _edges;
   };




   typedef std::map <std::string, cellNameStruct*> CellDRCMap;

   class CalbrFile
   {
      public:
                           CalbrFile(const std::string &fileName, drcRenderer *render);
                          ~CalbrFile();
         void              readFile();
         void              addResults();
         void              addRuleCheck(drcRuleCheck* check);
         void              showError(const std::string & cell, const std::string & error, long  number);
         void              showCluster(const std::string & cell, const std::string & error);
         void              showAllErrors(void);
         void              hideAllErrors(void);
         std::string       explainError(word lay);
         bool              isCellNameMode(void);
         RuleChecksVector* resultsFlat(void) {return &_RuleChecks;};
         CellDRCMap*       cellDRCMap(void) {return &_cellDRCMap;};
         CTM               getCTM(const std::string & cell);
         bool              isOk(void)   {return _ok;}
         drcRenderer*      render() const {return _render;};
         std::string       cellName() {return _curCellName;};
         std::string       topCellName() {return _topCellName;};
      private:
         FILE*             _calbrFile;
         std::string       _fileName;
         std::string       _topCellName;
         long              _precision;

         std::ifstream     _inFile;
         bool              parse(unsigned int num);
         bool              parsePoly(char* ruleCheckName, drcPolygon & poly, int numberOfElem);
         bool              parseEdge(char* ruleCheckName, drcEdge & edge, int numberOfElem);
         bool              parseCellNameMode(const std::string &parseString);
         void              appendRuleCheckToCellName(void);
         drcRuleCheck*     _curRuleCheck;
         std::string       _curRuleCheckName;
         RuleChecksVector  _RuleChecks;
         CellDRCMap        _cellDRCMap;
         bool              _ok;
         drcRenderer*      _render;
         bool              _isCellNameMode;
         std::string       _curCellName; //use for CellNameMode;
         edge              _border;
   };

   wxString convert(int number, long precision);
}
#endif //CALBR_READER_H_INCLUDED
