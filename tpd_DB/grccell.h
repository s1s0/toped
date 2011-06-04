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
//        Created: Sat Mar 26 13:48:39 GMT 2011
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Handling of objects failing Graphical Rules Checks
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef GRCCELL_H_
#define GRCCELL_H_

#include "qtree_tmpl.h"

namespace auxdata {

   //==============================================================================
   class TdtAuxData  {
   public:
      //! The default constructor.
                           TdtAuxData(SH_STATUS sel = sh_active) : _status(sel){};
      //! Return the overlapping box of the object.
      virtual   DBbox      overlap()  const = 0;
      //! A preparation for drawing - calculating all drawing objects using translation matrix stack.
      virtual   void       openGlPrecalc(layprop::DrawProperties&, PointVector&) const = 0;
      //! Draw the outline of the objects
      virtual   void       openGlDrawLine(layprop::DrawProperties&, const PointVector&) const = 0;
      //! Draw the object texture
      virtual   void       openGlDrawFill(layprop::DrawProperties&, const PointVector&) const = 0;
      //! Draw the outlines of the selected objects
      virtual   void       openGlDrawSel(const PointVector&, const SGBitSet*) const = 0;
      //! Clean-up the calculated drawing objects
      virtual   void       openGlPostClean(layprop::DrawProperties&, PointVector& ptlist) const {ptlist.clear();}
      virtual   void       drawRequest(tenderer::TopRend&) const = 0;
      //! Draw the outlines of the selected objects
      virtual   void       drawSRequest(tenderer::TopRend&, const SGBitSet*) const = 0;
      //! Draw the objects in motion during copy/move and similar operations
      virtual   void       motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const = 0;
      //! Print an object description on the toped console.
      virtual   void       info(std::ostringstream&, real) const = 0;
      //! Write the TdtData object in TDT file.
      virtual   void       write(OutputTdtFile* const tedfile) const = 0;
      //! Export the TdtData object in external format.
      virtual   void       dbExport(DbExportFile&) const = 0;
      //!
      virtual   bool       pointInside(const TP) const = 0;
      //!
      virtual  PointVector dumpPoints() const = 0;
      //! Set the _selected flag in case the object is entirely overlapped by select_in box
//      void                 selectInBox(DBbox&, DataList*, bool);
//      void                 selectThis(DataList*);
//      bool                 unselect(DBbox&, SelectDataPair&, bool);
      void                 setStatus(SH_STATUS s) {_status = s;}
      SH_STATUS            status() const {return _status;}
//      virtual word         numPoints() const = 0;
      virtual             ~TdtAuxData(){};
   protected:
      SH_STATUS            _status;
   };

   //==============================================================================
   class TdtGrcPoly : public TdtAuxData   {
      public:
                           TdtGrcPoly(const PointVector&);
                           TdtGrcPoly(int4b*, unsigned);
                           TdtGrcPoly(InputTdtFile* const);
         virtual          ~TdtGrcPoly();
         virtual DBbox     overlap() const;

         virtual void      openGlPrecalc(layprop::DrawProperties&, PointVector&) const;
         virtual void      openGlDrawLine(layprop::DrawProperties&, const PointVector&) const;
         virtual void      openGlDrawFill(layprop::DrawProperties&, const PointVector&) const;
         virtual void      openGlDrawSel(const PointVector&, const SGBitSet*) const;
         virtual void      drawRequest(tenderer::TopRend&) const;
         virtual void      drawSRequest(tenderer::TopRend&, const SGBitSet*) const;
         virtual void      motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const;
         virtual void      info(std::ostringstream&, real) const;
         virtual void      write(OutputTdtFile* const tedfile) const;
         virtual void      dbExport(DbExportFile&) const;
//         virtual word      numPoints() const {return _psize;}
         virtual bool      pointInside(const TP)const;
         virtual PointVector dumpPoints() const;
      private:
         int4b*            _pdata;
         unsigned          _psize;
   };

   //==============================================================================
   class TdtGrcWire : public TdtAuxData   {
      public:
                           TdtGrcWire(const PointVector&, WireWidth);
                           TdtGrcWire(int4b*, unsigned, WireWidth);
                           TdtGrcWire(InputTdtFile* const);
         virtual          ~TdtGrcWire();
         virtual DBbox     overlap() const;

         virtual void      openGlPrecalc(layprop::DrawProperties&, PointVector&) const;
         virtual void      openGlDrawLine(layprop::DrawProperties&, const PointVector&) const;
         virtual void      openGlDrawFill(layprop::DrawProperties&, const PointVector&) const;
         virtual void      openGlDrawSel(const PointVector&, const SGBitSet*) const;
         virtual void      drawRequest(tenderer::TopRend&) const;
         virtual void      drawSRequest(tenderer::TopRend&, const SGBitSet*) const;
         virtual void      motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const;
         virtual void      info(std::ostringstream&, real) const;
         virtual void      write(OutputTdtFile* const tedfile) const;
         virtual void      dbExport(DbExportFile&) const;
//         virtual word      numPoints() const {return _psize;}
         virtual bool      pointInside(const TP)const;
         virtual PointVector dumpPoints() const;
      private:
         float             get_distance(const TP&, const TP&, const TP&)const;
         int4b*            _pdata;
         unsigned          _psize;
         WireWidth         _width;
   };

   typedef laydata::QTreeTmpl<TdtAuxData>    QuadTree;
   typedef laydata::QTStoreTmpl<TdtAuxData>  QTreeTmp;
   typedef std::map<unsigned, QuadTree*>     LayerList;
   typedef  std::pair<TdtAuxData*, SGBitSet> SelectDataPair;
   typedef  std::list<SelectDataPair>        DataList;
   typedef  std::map<unsigned, DataList*>    SelectList;
   typedef  std::map<unsigned, QTreeTmp*>    TmpLayerMap;


   class GrcCell {
      public:
                             GrcCell(std::string);
                             GrcCell(InputTdtFile* const, std::string);
         virtual            ~GrcCell();
         virtual void        write(OutputTdtFile* const) const;
         void                dbExport(DbExportFile&) const;
         virtual void        openGlDraw(layprop::DrawProperties&, bool active=false) const;
         virtual void        openGlRender(tenderer::TopRend&, const CTM&, bool, bool) const;
         virtual DBbox       getVisibleOverlap(const layprop::DrawProperties&);
         virtual void        collectUsedLays(WordList&) const;
         virtual void        motionDraw(const layprop::DrawProperties&, CtmQueue&, bool active=false) const;
         //
         QuadTree*           secureLayer(unsigned layno);
         QTreeTmp*           secureUnsortedLayer(unsigned layno);
         bool                fixUnsorted();
         //
         virtual DBbox       cellOverlap() const        {return _cellOverlap;}
         std::string         name() const               {return _name;}
      protected:
         void                readTdtLay(InputTdtFile* const);
         void                getCellOverlap();
         std::string         _name;         //! cell name
         LayerList           _layers;       //! all layers in the cell
         DBbox               _cellOverlap;  //! Overlap of the entire cell
         SelectList          _shapesel;     //! selected shapes
         TmpLayerMap         _tmpLayers;    //! All layers with unsorted data
   };

}
#endif /* GRCCELL_H_ */
