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

struct coord
{
   real x, y;
};

struct edge
{
   real x1, y1, x2, y2;
};

typedef std::vector <Calbr::coord> CoordsVector;


class drcRenderer
{
public:
   drcRenderer() {};
   ~drcRenderer() {};
   virtual void startWriting()=0;
	virtual void setError(unsigned int numError) {};
	virtual void showError(unsigned int numError) {};
   virtual void showAll() {};
	virtual void hideAll() {};
   virtual void addPoly(const CoordsVector   &coords)=0;
   virtual void addLine(const edge &edge)=0;
   virtual void endWriting()=0;
	virtual void zoom(const edge &edge)=0;
   virtual void explainError(real x, real y)=0;
};



class drcEdge
{
public:
   drcEdge(long ordinal, drcRenderer*   render) { _ordinal = ordinal; _render = render;};
   void            addCoord(long x1, long y1, long x2, long y2);
   edge*           coords() {return &_coords;};
	edge            getZoom() const;
   long            ordinal() const {return _ordinal;};
   void            addError(word la);
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
   void            addError(word la);
   static long     _precision;
private:
   CoordsVector    _coords;
   long            _ordinal;
   drcRenderer*    _render;

};

class drcRuleCheck
{
public:
   drcRuleCheck(unsigned int num, const std::string &name);

	unsigned int num(void) const {return _num;};
   std::string ruleCheckName()   const {return _ruleCheckName;}
   std::string timeStamp()       const {return _timeStamp;}
   long         curResCount()    const {return  _curResCount;}
   long         origResCount()   const {return _origResCount;}
   std::vector <Calbr::drcPolygon>* polygons() {return &_polygons;};
   std::vector <Calbr::drcEdge>* edges() {return &_edges;};
   void         setTimeStamp(const std::string &timeStamp);
   void         setCurResCount(int curResCount);
   void         setOrigResCount(int origResCount);
   void         addDescrString(const std::string & str);
   void         addPolygon(const Calbr::drcPolygon &poly);
   void         addEdge(const Calbr::drcEdge &theEdge);
	edge         getZoom(long ordinal);
private:
	unsigned int _num;
   long         _curResCount; //current result count
   long         _origResCount;//original result count
   std::string  _ruleCheckName;
   std::string  _timeStamp;
   std::string  _header;
   std::vector <std::string> _descrStrings;
   std::vector <Calbr::drcPolygon> _polygons;
   std::vector <Calbr::drcEdge> _edges;
};

typedef std::vector <Calbr::drcRuleCheck*> RuleChecksVector;

class CalbrFile
{
public:
   CalbrFile(const std::string &fileName, drcRenderer *render);
   ~CalbrFile();

   void              addResults();
   void              showError(const std::string & error, long  number);
	void              showAllErrors(void);
   void              hideAllErrors(void);
   void              explainError(real x, real y) {_render->explainError(x, y);};
   RuleChecksVector* results() {return &_RuleChecks;};
   bool              isOk(void)   {return _ok;}
	drcRenderer*		render() const {return _render;};
private:
   FILE*             _calbrFile;
   std::string       _fileName;
   std::string       _cellName;
   long              _precision;

   std::ifstream     _inFile;
   bool              parse(unsigned int num);
   RuleChecksVector  _RuleChecks;
   bool              _ok;
   drcRenderer*      _render;


};

wxString convert(int number, long precision);
}
#endif //CALBR_READER_H_INCLUDED