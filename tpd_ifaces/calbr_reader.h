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
#include "auxdat.h"


namespace auxdata {
   //==============================================================================
   class DrcPoly : public AuxData   {
      public:
                           DrcPoly(int4b*, unsigned, unsigned);
         virtual          ~DrcPoly();
         virtual DBbox     overlap() const;

         virtual void      drawRequest(trend::TrendBase&) const;
         virtual void      drawSRequest(trend::TrendBase&, const SGBitSet*) const;
         virtual void      motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const;
         virtual void      info(std::ostringstream&, real) const;
         virtual bool      pointInside(const TP)const;
      private:
         int4b*            _pdata   ;
         unsigned          _psize   ;
         unsigned          _ordinal ;
   };

   //==============================================================================
   class DrcSeg : public AuxData   {
      public:
                           DrcSeg(int4b*, unsigned, unsigned);
         virtual          ~DrcSeg();
         virtual DBbox     overlap() const;

         virtual void      drawRequest(trend::TrendBase&) const;
         virtual void      drawSRequest(trend::TrendBase&, const SGBitSet*) const;
         virtual void      motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const;
         virtual void      info(std::ostringstream&, real) const;
         virtual bool      pointInside(const TP)const;
      private:
         int4b*            _sdata   ; //!Segment data
         unsigned          _ssize   ; //!Number of Segments
         unsigned          _ordinal ;
   };

}

namespace clbr
{
   /**
    * A couple of words about the data structure of the data base created out of
    * Calibre DRC result files.
    * After some trials and errors this looks the most natural structure, although
    * not the most trivial to implement because of the structure of the Calibre
    * file. The latter is certainly "Rule centric", but if we follow that structure
    * strictly, the eventual cell information is not quite easy to use.
    * So the conversion between the "rule centric" and "cell centric" DB is done
    * on the fly during file parsing. At the end of the day the parsing generates
    * the structure below:
    *    DrcLibrary
    *     |__DrcCell (1 or more)
    *         |__DrcRule (1 or more)
    *             |__DrcData (1 or more)
    * One and the same DrcRule can be in more than one DrcCell - of course
    * containing different errors.
    * The hierarchy above is virtually the same as the design hierarchy provided
    * that the DrcRule is replacing the tdtLayer. That should be convenient for
    * the DRC DB users.
    */
   //==============================================================================
   class DrcRule {
   public:
                            DrcRule();
                            DrcRule(const DrcRule&);
                           ~DrcRule();
      void                  setTimeStamp(const std::string& timeStamp);
      void                  setCurResCount(int curResCount);
      void                  setOrigResCount(int origResCount);
      void                  addDescrString(const std::string& str);
      void                  addResult(auxdata::AuxData*);
      void                  addResults(const DrcRule&);
      void                  parsed();
   private:
      bool                  fixUnsorted();
      unsigned              _curResCount      ;//current result count
      unsigned              _origResCount     ;//original result count
      std::string           _timeStamp        ;
      NameList              _descrStrings     ;
      auxdata::QuadTreeAux* _drcData          ;
      auxdata::QTreeTmpAux* _tmpData          ;
   };

   typedef  std::map<std::string, DrcRule*>  RuleMap;

   class DrcCell {
   public:
                            DrcCell(CTM&);
      virtual              ~DrcCell();
      void                  registerRuleRead(std::string, DrcRule*&);
      DrcRule*              cloneRule(DrcRule*);
      const RuleMap*        rules() {return &_rules;}
      const CTM&            ctm()   {return _ctm;}
   private:
      CTM                   _ctm;
      RuleMap               _rules;
   };

   typedef  std::map<std::string, DrcCell*>  CellMap;

   typedef std::map<std::string, NameList*> RuleNameMap;
   //==============================================================================
   class DrcLibrary {
   public:
                            DrcLibrary(std::string name, real precision);
      virtual              ~DrcLibrary();
      DrcCell*              registerCellRead(std::string, CTM&);
      WordList              findSelected(const std::string &cell, TP*); //use for DRCexplainerror
//      void                  openGlRender(trend::TrendBase&, std::string, CTM&);
      std::string           name()            const {return _name;}

      void                  showError(const std::string& cell, const std::string& error, long number)  {/*TODO*/}
      bool                  showCluster(const std::string& cell, const std::string& error)             {/*TODO*/ return true;}
      void                  showAllErrors(trend::TrendBase&);
//      void                  hideAllErrors(trend::TrendBase&);
      const CellMap*        cells() {return &_cells;}
      const RuleNameMap*    rules();
   protected:
      DrcCell*              checkCell(std::string name);
      std::string           _name;         //! design/library name
      CellMap               _cells;        //! list of all cells
      real                  _precision;    //
   };

   //==============================================================================
   class ClbrFile : public InputDBFile {
      public:
                            ClbrFile(wxString, DrcLibrary*&);
         virtual           ~ClbrFile();

      protected:
         bool               moreData(bool throwexception = true);
         std::string        getTextWord()    ;//
         std::string        getTextLine()    ;//
         void               ruleMetaData()   ;//! Read rule meta data
         void               ruleDrcResults() ;//! Read all DRC results in a rule
         auxdata::DrcPoly*  drcPoly()        ;//! Read DRC Poly data
         auxdata::DrcSeg*   drcEdge()        ;//! Read DRC Edge data
         bool               checkCNnP()      ;//! Check Cell Name and Parameters
         void               readCN()         ;//! Read Cell Name
         wxTextInputStream* _textStream      ;//!
         DrcCell*           _cCell           ;//! current Cell
         DrcRule*           _cRule           ;//! current Rule
         std::string        _cRuleName       ;//! current Rule Name
         DrcLibrary*        _drcDB           ;//! The DRC data base with parsed data
   };
}

#endif //CALBR_READER_H_INCLUDED
