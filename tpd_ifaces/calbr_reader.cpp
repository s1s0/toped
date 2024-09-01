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
//          $URL$
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Reader of Mentor Graphics Calibre drc errors files
//===========================================================================
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//    Description: Reader of Mentor Graphics Calibre drc errors files
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <algorithm>
#include <wx/regex.h>
#include <wx/txtstrm.h>
#include "calbr_reader.h"

//long Calbr::drcPolygon::_precision = 0;
//long Calbr::drcEdge::_precision = 0;

#ifdef WIN32
   #define tpdSTRxxxCMP _stricmp
#else
   #define tpdSTRxxxCMP strcasecmp
#endif

//=============================================================================
auxdata::DrcPoly::DrcPoly(int4b* pdata, unsigned psize, unsigned ordinal) :
   AuxData     ( sh_drc    ),
   _pdata      ( pdata     ),
   _psize      ( psize     ),
   _ordinal    ( ordinal   )
{
   _teseldata.tessellate(_pdata, _psize);
}

auxdata::DrcPoly::~DrcPoly()
{
   delete [] _pdata;
}

DBbox auxdata::DrcPoly::overlap() const
{
   DBbox ovl(_pdata[0], _pdata[1]) ;
   for (word i = 1; i < _psize; i++)
      ovl.overlap(_pdata[2*i], _pdata[2*i+1]);
   return ovl;
}

void auxdata::DrcPoly::drawRequest(trend::TrendBase& rend) const
{
   rend.poly(_pdata, _psize, &_teseldata);
}

void auxdata::DrcPoly::drawSRequest(trend::TrendBase& rend, const SGBitSet*) const
{
   rend.poly(_pdata, _psize, NULL, NULL);
}

void auxdata::DrcPoly::info(std::ostringstream& ost, real DBU) const
{
   ost << "polygon - {";
   for (unsigned i = 0; i < _psize; i++)
   {
      TP cpnt(_pdata[2*i], _pdata[2*i+1]);
      cpnt.info(ost, DBU);
      if (i != _psize - 1) ost << " , ";
   }
   ost << "};";
}

void auxdata::DrcPoly::motionDraw(const layprop::DrawProperties&, CtmQueue& transtack,
                                 SGBitSet* /*plst*/) const
{
   CTM trans = transtack.front();
   PointVector* ptlist = DEBUG_NEW PointVector;
   ptlist->reserve(_psize);
   for (unsigned i = 0; i < _psize; i++)
   {
      ptlist->push_back( TP(_pdata[2*i], _pdata[2*i+1]) * trans);
   }
   DBGL_CALL(glBegin,GL_LINE_LOOP)
   for (unsigned i = 0; i < _psize; i++)
   {
      DBGL_CALL(glVertex2i,(*ptlist)[i].x(), (*ptlist)[i].y())
   }
   DBGL_CALL0(glEnd)
   ptlist->clear();
   delete ptlist;
}


bool auxdata::DrcPoly::pointInside(const TP pnt)const
{
   TP p0, p1;
   byte cc = 0;
   for (unsigned i = 0; i < _psize ; i++)
   {
      p0 = TP(_pdata[2 *   i             ], _pdata[2 *   i              + 1]);
      p1 = TP(_pdata[2 * ((i+1) % _psize)], _pdata[2 * ((i+1) % _psize) + 1]);
      if (((p0.y() <= pnt.y()) && (p1.y() >  pnt.y()))
        ||((p0.y() >  pnt.y()) && (p1.y() <= pnt.y())) ) {
         float tngns = (float) (pnt.y() - p0.y())/(p1.y() - p0.y());
         if (pnt.x() < p0.x() + tngns * (p1.x() - p0.x()))
            cc++;
      }
   }
   return (cc & 0x01) ? true : false;
}
//=============================================================================
auxdata::DrcSeg::DrcSeg(int4b* sdata, unsigned ssize, unsigned ordinal) :
   AuxData     ( sh_drc    ),
   _sdata      ( sdata     ),
   _ssize      ( ssize     ),
   _ordinal    ( ordinal   )
{}

auxdata::DrcSeg::~DrcSeg()
{
   delete [] _sdata;
}

DBbox auxdata::DrcSeg::overlap() const
{
   DBbox ovl(_sdata[0], _sdata[1]) ;
   ovl.overlap(_sdata[2], _sdata[3]);
   for (word i = 1; i < _ssize; i++)
   {
      ovl.overlap(_sdata[4*i  ], _sdata[4*i+1]);
      ovl.overlap(_sdata[4*i+2], _sdata[4*i+3]);
   }
   return ovl;
}

void auxdata::DrcSeg::drawRequest(trend::TrendBase&) const
{
   //TODO
//   int boza = 0;
//   boza++;
}

void auxdata::DrcSeg::drawSRequest(trend::TrendBase&, const SGBitSet*) const
{
   //TODO
}

void auxdata::DrcSeg::motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const
{
   //TODO
}

void auxdata::DrcSeg::info(std::ostringstream&, real) const
{
   //TODO
}

bool auxdata::DrcSeg::pointInside(const TP)const
{
   return false;
   //TODO
}

//=============================================================================
clbr::DrcRule::DrcRule(/*unsigned num */) :
//   _num              ( num     ),
   _curResCount      ( 0l        ),
   _origResCount     ( 0l        ),
   _timeStamp        (           )
{
   _drcData = DEBUG_NEW auxdata::QuadTreeAux();
   _tmpData = DEBUG_NEW auxdata::QTreeTmpAux(_drcData);
}

clbr::DrcRule::DrcRule(const clbr::DrcRule& cpRule) :
   _curResCount      ( cpRule._curResCount      ),
   _origResCount     ( cpRule._origResCount     ),
   _timeStamp        ( cpRule._timeStamp        )
{
   _drcData = DEBUG_NEW auxdata::QuadTreeAux();
   _tmpData = DEBUG_NEW auxdata::QTreeTmpAux(_drcData);
}

void clbr::DrcRule::setTimeStamp(const std::string& timeStamp)
{
   _timeStamp = timeStamp;
}

void clbr::DrcRule::setCurResCount(int curResCount)
{
   _curResCount = curResCount;
}

void clbr::DrcRule::setOrigResCount(int origResCount)
{
   _origResCount = origResCount;
}

void clbr::DrcRule::addDescrString(const std::string& str)
{
   _descrStrings.push_back(str);
}

void clbr::DrcRule::addResult(auxdata::AuxData* data)
{
   _tmpData->put(data);
}

void clbr::DrcRule::addResults(const DrcRule& results)
{
   for(auxdata::QuadTreeAux::Iterator CD = results._drcData->begin(); CD != results._drcData->end(); ++CD)
   {
      _tmpData->put(*CD);
   }
}

void clbr::DrcRule::drawAll(trend::TrendBase& dRenderer)
{
   _drcData->openGlRender(dRenderer, NULL);
}

void clbr::DrcRule::parsed()
{
   _tmpData->commit();
}

clbr::DrcRule::~DrcRule()
{
   _drcData->freeMemory();
   delete _drcData;
   delete _tmpData;
}

//=============================================================================
clbr::DrcCell::DrcCell(CTM& ctm) :
   _ctm           ( ctm             ),
   _cellOverlap   ( DEFAULT_OVL_BOX )
{
}

void clbr::DrcCell::registerRuleRead(std::string rulename, DrcRule*& rule)
{
   RuleMap::iterator cRule = _rules.find(rulename);
   rule->parsed();
   if (_rules.end() == cRule)
   {
      _rules[rulename] = rule;
   }
   else
   {
      cRule->second->addResults(*rule);
      delete rule;
      rule = cRule->second;
   }
}

clbr::DrcRule* clbr::DrcCell::cloneRule(DrcRule* rule)
{
   assert(rule);
   clbr::DrcRule* nRule = DEBUG_NEW clbr::DrcRule(*rule);
//   rule->parsed();
   return nRule;
}

void clbr::DrcCell::drawAll(std::string cname, trend::TrendBase& dRenderer)
{
   dRenderer.pushCell(cname, _ctm, _cellOverlap, false, false);
   dRenderer.setLayer(DRC_LAY_DEF, false);
   for (RuleMap::const_iterator CR = _rules.begin(); CR != _rules.end(); CR++)
   {
      CR->second->drawAll(dRenderer);
   }
   dRenderer.popCell();
}

void clbr::DrcCell::getCellOverlap()
{
   if (_rules.empty())
      _cellOverlap = DEFAULT_OVL_BOX;
   else
   {
      RuleMap::iterator LCI = _rules.begin();
      _cellOverlap = LCI->second->overlap();
      while (++LCI != _rules.end())
         _cellOverlap.overlap(LCI->second->overlap());
   }
}

clbr::DrcCell::~DrcCell()
{
   for (RuleMap::const_iterator wc = _rules.begin(); wc != _rules.end(); wc++)
      delete wc->second;
   _rules.clear();
}

//=============================================================================
clbr::DrcLibrary::DrcLibrary(std::string name, real precision) :
   _name          ( name      ),
   _precision     ( precision )
{}

clbr::DrcCell* clbr::DrcLibrary::registerCellRead(std::string cellname, CTM& cCtm)
{
   DrcCell* cCell;
   if (_cells.end() != _cells.find(cellname))
   {
      cCell = _cells[cellname];
//      assert(cCtm == cCell->ctm());
   }
   else
   {
      cCell = DEBUG_NEW DrcCell(cCtm);
      _cells[cellname] = cCell;
   }
   return cCell;
}

WordList clbr::DrcLibrary::findSelected(const std::string &/*cell*/, TP* /*p1*/)
{//TODO
   //TdtDefaultCell* cell = checkCell("drc");
//   auxdata::DrcRule* theCell = checkCell(cell);
//   TP selp;
   WordList errorList;
//   laydata::AtticList* shapes = NULL;
//   laydata::ShapeList *shapeList;
//   TdtData *shape;
//   if (theCell)
//   {
//
//      layprop::DrawProperties* drawProp;
//      if (PROPC->lockDrawProp(drawProp, layprop::prsDRC))
//      {
//         selp = (*p1)*CTM().Reversed(); //Take identity matrix
//         //??? Add here Error List construction
//         shapes = theCell->findSelected(selp);
//         for(laydata::AtticList::Iterator it = shapes->begin(); it != shapes->end(); ++it)
//         {
//            word error;
//            shapeList = *it;
//            for (laydata::ShapeList::const_iterator it2 = shapeList->begin(); it2 != shapeList->end(); ++it2)
//            {
//               shape = dynamic_cast<TdtData*> (*it2);
//               error = shape->getLong();
//               errorList.push_back(error);
//            }
//         }
//      }
//      PROPC->unlockDrawProp(drawProp, true);
//      errorList.unique();
//      if(shapes != NULL)
//      {
//         for(laydata::AtticList::Iterator it = shapes->begin(); it != shapes->end(); ++it)
//         {
//            shapeList = *it;
//            delete shapeList;
//         }
//
//         delete shapes;
//      }
//      return errorList;
//   }
//   else
      return errorList;
}

clbr::DrcCell* clbr::DrcLibrary::checkCell(std::string name)
{
   if (_cells.end() == _cells.find(name))
      return NULL;
   else return _cells[name];
}

//void laydata::DrcLibrary::openGlDraw(layprop::DrawProperties& drawProp, std::string cell)
//{
//   drawProp.setState(layprop::DRC);
//   laydata::TdtDefaultCell* dst_structure = checkCell(cell);
//   if (dst_structure)
//   {
//      drawProp.initCtmStack();
////    drawProp->initDrawRefStack(NULL); // no references yet in the DRC DB
//      dst_structure->openGlDraw(drawProp);
////    drawProp->clearCtmStack();
//      drawProp.clearDrawRefStack();
//   }
//   drawProp.setState(layprop::DB);
//}

//void clbr::DrcLibrary::openGlRender(trend::TrendBase& renderer, std::string cell, CTM& cctm)
//{
//   renderer.setState(layprop::prsDRC);
//   DrcCell* dst_structure = checkCell(cell);
//   if (dst_structure)
//   {
////      dst_structure->openGlRender(renderer, cctm, false, false); TODO
//   }
//   renderer.setState(layprop::prsDB);
//
//}

void clbr::DrcLibrary::drawAll(trend::TrendBase& dRenderer)
{
   for (CellMap::const_iterator CC = _cells.begin();CC != _cells.end(); CC++)
   {
      CC->second->drawAll(CC->first, dRenderer);
   }
}

//void clbr::DrcLibrary::hideAllErrors(void)
//{
//
//}

const clbr::RuleNameMap* clbr::DrcLibrary::rules()
{
   RuleNameMap* ruleMap = DEBUG_NEW clbr::RuleNameMap();
   for (CellMap::const_iterator wc = _cells.begin(); wc != _cells.end(); wc++)
   {
      std::string cellName = wc->first;
      const RuleMap* cRules = wc->second->rules();
      for (RuleMap::const_iterator wr = cRules->begin(); wr != cRules->end(); wr++)
      {
         std::string ruleName = wr->first;
         NameList* cellNames;
         if (ruleMap->end() == ruleMap->find(ruleName))
         {
            cellNames = DEBUG_NEW NameList();
            (*ruleMap)[ruleName] = cellNames;
         }
         else
            cellNames = (*ruleMap)[ruleName];
         cellNames->push_back(cellName);
      }
   }
   for (RuleNameMap::iterator wr = ruleMap->begin(); wr != ruleMap->end(); wr++)
   {
      wr->second->unique();
   }
   return ruleMap;
}

clbr::DrcLibrary::~DrcLibrary()
{
//   RuleMap::const_iterator wc;
   for (CellMap::const_iterator wc = _cells.begin(); wc != _cells.end(); wc++)
      delete wc->second;
   _cells.clear();
}

//=============================================================================
clbr::ClbrFile::ClbrFile(wxString wxfname, DrcLibrary*& drcDB) : InputDBFile (wxfname, false)
{
   std::ostringstream info;
   if (!status())
   {
      throw EXPTNdrc_reader("Failed to open the input file");
   }
   info << "Parsing \"" << fileName() << "\" using ASCII DRC grammar)";
   tell_log(console::MT_INFO,info.str());

   _textStream = DEBUG_NEW wxTextInputStream(*_inStream);

   real precision;
   std::string cellName = getTextWord();
   if (moreData()) precision = _textStream->ReadDouble();

   _drcDB = drcDB = DEBUG_NEW DrcLibrary(cellName, precision);

   CTM defCtm;
   DrcCell* topCell = _drcDB->registerCellRead(cellName, defCtm);

   // Now parse the rules
   do // i.e. at least one rule is expected
   {
      // Get the rule name and create a drcRuleCheck object with it
      // It is considered that at this point is legal to finish the file
      // That's why getTextWord() is not used, it might throw an exception
      if (moreData(false))
      {
         wxString wxRuleName = _textStream->ReadWord();
         _cRuleName =  std::string(wxRuleName.mb_str(wxConvUTF8));
         if (_cRuleName.empty())
            break;
         else
            _cRule = DEBUG_NEW DrcRule();
      }
      else break;
      // For every new rule reset the value of the current cell
      _cCell = topCell;
      ruleMetaData();
      ruleDrcResults();
      _cCell->registerRuleRead( _cRuleName, _cRule);
   } while (true);
}

bool clbr::ClbrFile::moreData(bool throwexception)
{
   if (_inStream->Eof())
   {
      if (throwexception)
         throw EXPTNdrc_reader("Unexpected End of File");
      else
         return false;
   }
   return true;
}

std::string clbr::ClbrFile::getTextWord()
{
   if (moreData())
   {
      wxString wxWord = _textStream->ReadWord();
      return std::string(wxWord.mb_str(wxConvUTF8));
   }
   return "";
}

std::string clbr::ClbrFile::getTextLine()
{
   if (moreData())
   {
      wxString wxWord = _textStream->ReadLine();
      return std::string(wxWord.mb_str(wxConvUTF8));
   }
   return "";
}

void clbr::ClbrFile::ruleMetaData()
{
   assert(_cRule);
   if (moreData()) _cRule->setCurResCount(_textStream->Read32());
   if (moreData()) _cRule->setOrigResCount(_textStream->Read32());
   unsigned int numInfoLines;
   if (moreData()) numInfoLines = _textStream->Read32();
   _cRule->setTimeStamp(getTextLine());

   for (unsigned int i = 0; i < numInfoLines; i++)
   {
      _cRule->addDescrString(getTextLine());
   }
}

void clbr::ClbrFile::ruleDrcResults()
{
   assert(_cRule);
   do
   {
      int nextChar = _inStream->GetC();
      if (wxEOF == nextChar) return /*end of file*/;
      auxdata::AuxData* cError = NULL;
      switch ((char) nextChar)
      {
         case 'p' : cError = drcPoly(); break;
         case 'e' : cError = drcEdge(); break;
         case '\n':
         case '\r':
         case ' ' : continue; break;
           default: _inStream->Ungetch((char) nextChar);return /*new rule follows*/;
      }
      if (NULL!= cError) _cRule->addResult(cError);
   } while (true);
}

auxdata::DrcPoly* clbr::ClbrFile::drcPoly()
{
   unsigned int ordinal;
   unsigned int numVrtx;
   if (moreData()) ordinal = _textStream->Read32();
   if (moreData()) numVrtx = _textStream->Read32();
   int4b* pdata = DEBUG_NEW int4b[2*numVrtx];
   while (!checkCNnP()) {};
   for (unsigned int vrtx = 0; vrtx < 2*numVrtx; vrtx++)
   {
      if (moreData()) pdata[vrtx] = _textStream->Read32();
   }
   return DEBUG_NEW auxdata::DrcPoly(pdata, numVrtx, ordinal);
}

auxdata::DrcSeg* clbr::ClbrFile::drcEdge()
{
   unsigned int ordinal;
   unsigned int numEdgs;
   if (moreData()) ordinal = _textStream->Read32();
   if (moreData()) numEdgs = _textStream->Read32();

   while (!checkCNnP()) {};
   int4b* pdata = DEBUG_NEW int4b [numEdgs * 4];
   for (unsigned int vrtx = 0; vrtx < numEdgs * 4; vrtx++)
   {
      if (moreData()) pdata[vrtx] = _textStream->Read32();
   }
   return DEBUG_NEW auxdata::DrcSeg(pdata, numEdgs, ordinal);
}

bool clbr::ClbrFile::checkCNnP()
{
   int nextChar;
   std::string cnLine;
   if (moreData()) nextChar = _inStream->GetC();
   if (isdigit(nextChar) || ('-' == (char) nextChar))
   {
      _inStream->Ungetch((char) nextChar);
      return true;
   }
   else if ('C' != (char) nextChar)
   {
      _inStream->Ungetch((char) nextChar);// - should be a property!
   }
   else
   {
      if (moreData()) nextChar = _inStream->GetC();
      if ('N' != (char) nextChar)
      {
         _inStream->Ungetch((char) nextChar);// - should be a property!
      }
      else
      {
         readCN();
         return true;
      }
   }
   std::string property = getTextLine();// TODO parse a property
   return false;
}

void clbr::ClbrFile::readCN()
{
   wxString wxCellName;
   if (moreData()) wxCellName = _textStream->ReadWord();
   std::string cellName =  std::string(wxCellName.mb_str(wxConvUTF8));
   if (moreData(false))
   {
      int nextChar;
      do
      {
         nextChar = _inStream->GetC(); //_textStream->GetChar
      } while (isspace(nextChar));
      if ('c' == (char) nextChar)
      {// local CTM
         int ctmraw[6];
         for (unsigned int vrtx = 0; vrtx < 6; vrtx++)
         {
            if (moreData()) ctmraw[vrtx] = _textStream->Read32();
         }
         CTM cCtm(ctmraw[0], ctmraw[1], ctmraw[2], ctmraw[3], ctmraw[4], ctmraw[5]);
         _cCell->registerRuleRead( _cRuleName, _cRule);
         _cCell = _drcDB->registerCellRead(cellName, cCtm);
         _cRule = _cCell->cloneRule(_cRule);
      }
      else
         throw EXPTNdrc_reader("Expected valid CN (Cell Name) record");
   }
}

clbr::ClbrFile::~ClbrFile()
{
   delete _textStream;
   // delete all CIF structures
//   CifStructure* local = _first;
//   CifStructure* local4d;
//   while (NULL != local)
//   {
//      local4d = local;
//      local = local->last();
//      delete local4d;
//   }
//   delete _default;
}

