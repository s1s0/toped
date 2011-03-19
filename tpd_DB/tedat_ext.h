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
//        Created: Sun Aug 9 2010
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Layout primitives with extentions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TEDAT_EXT_H
#define TEDAT_EXT_H
#include "tedat.h"

namespace laydata 
{
   class TdtBoxEXT : public TdtBox
   {
   public:
                          TdtBoxEXT(const TP& p1, const TP& p2):TdtBox(p1, p2) {};
                          TdtBoxEXT(InputTdtFile* const tedfile):TdtBox(tedfile) {};
      virtual            ~TdtBoxEXT() {};
      virtual void        setLong(long extLong);
      virtual long        getLong(void);
      virtual void        setString(const std::string &extString);
      virtual std::string getString(void);
      virtual void        setClientData(void* clientData);
      virtual void*       getClientData(void);
   private:
      long                _extLong;
      std::string         _extString;
      void*               _clientData;
   };

   class TdtPolyEXT : public TdtPoly   
   {
    public:
                          TdtPolyEXT(const PointVector& plist):TdtPoly(plist) {};
                          TdtPolyEXT(int4b* plist, unsigned psize):TdtPoly(plist, psize) {};
                          TdtPolyEXT(InputTdtFile* const tedfile):TdtPoly(tedfile) {};
      virtual            ~TdtPolyEXT() {};
      virtual void        setLong(long extLong);
      virtual long        getLong(void);
      virtual void        setString(const std::string &extString);
      virtual std::string getString(void);
      virtual void        setClientData(void* clientData);
      virtual void*       getClientData(void);
   private:
      int                 _extInt;
      long                _extLong;
      std::string         _extString;
      void*               _clientData;
   };

   class TdtWireEXT : public TdtWire
   {
    public:
                          TdtWireEXT(const PointVector& plist, word layno):TdtWire(plist, layno) {};
                          TdtWireEXT(int4b* plist, unsigned psize, word layno):TdtWire(plist, psize, layno) {};
                          TdtWireEXT(InputTdtFile* const tedfile): TdtWire(tedfile) {};
      virtual            ~TdtWireEXT() {};
      virtual void        setLong(long extLong);
      virtual long        getLong(void);
      virtual void        setString(const std::string &extString);
      virtual std::string getString(void);
      virtual void        setClientData(void* clientData);
      virtual void*       getClientData(void);
   private:
      int                 _extInt;
      long                _extLong;
      std::string         _extString;
      void*               _clientData;
   };


   //==============================================================================
   class TdtErrData  {
   public:
   //! The default constructor.
      TdtErrData(SH_STATUS sel = sh_active) : _status(sel)/*, _next(NULL) */{};
      //! Return the overlapping box of the object.
      virtual   DBbox      overlap()  const = 0;
      //! Return the overlapping box of the object.
//      virtual   void       vlOverlap(const layprop::DrawProperties&, DBbox&) const {assert(false);}
   //! Move the object relatively using the input CTM
//      virtual   Validator* move(const CTM&, SGBitSet& plst) = 0;
   //! Rotate or flip (transfer the object using input CTM
//      virtual   void       transfer(const CTM&) = 0;
   //! Copy the object and move it using the input CTM
//      virtual   TdtErrData*   copy(const CTM&) = 0;
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
//      virtual   void       motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const = 0;
   //! Print an object description on the toped console.
      virtual   void       info(std::ostringstream&, real) const = 0;
   //! Write the TdtErrData object in TDT file.
//      virtual   void       write(TEDfile* const tedfile) const = 0;
   //! Export the TdtErrData object in external format.
//      virtual   void       dbExport(DbExportFile&) const = 0;
   //! Write the TdtErrData object in PS file.
//      virtual   void       psWrite(PSFile&, const layprop::DrawProperties&) const = 0;
   //!
      virtual   bool       pointInside(const TP);
   //! shape cut with the input polygon
//      virtual   void       polyCut(PointVector&, ShapeList**) = 0;
   //! shrink/stretch
//      virtual   void       stretch(int bfactor, ShapeList**) = 0;
   //!
      virtual  PointVector shape2poly() const = 0;
      //!
      virtual  PointVector dumpPoints() const = 0;
   //! Set the _selected flag in case the object is entirely overlapped by select_in box
      void                 selectInBox(DBbox&, DataList*, bool);
      void                 selectThis(DataList*);
      bool                 unselect(DBbox&, SelectDataPair&, bool);
      void                 setStatus(SH_STATUS s) {_status = s;}
      SH_STATUS            status() const {return _status;}
      virtual word         numPoints() const = 0;
      virtual             ~TdtErrData(){};
//      virtual word         lType() const = 0;

   //********************
   protected:
//      virtual void         selectPoints(DBbox&, SGBitSet&) = 0;
//      virtual void         unselectPoints(DBbox&, SGBitSet&) = 0;
      SH_STATUS            _status;
   };

   //==============================================================================
   class TdtErrPoly : public TdtErrData   {
      public:
                           TdtErrPoly(const PointVector& plist);
                           TdtErrPoly(int4b* plist, unsigned psize);
         virtual          ~TdtErrPoly();
         virtual DBbox     overlap() const;
//         virtual Validator* move(const CTM&, SGBitSet& plst);
//         virtual void      transfer(const CTM&);
//         virtual TdtErrData*  copy(const CTM&);

         virtual void      openGlPrecalc(layprop::DrawProperties&, PointVector&) const;
         virtual void      openGlDrawLine(layprop::DrawProperties&, const PointVector&) const;
         virtual void      openGlDrawFill(layprop::DrawProperties&, const PointVector&) const;
         virtual void      openGlDrawSel(const PointVector&, const SGBitSet*) const;
         virtual void      drawRequest(tenderer::TopRend&) const;
         virtual void      drawSRequest(tenderer::TopRend&, const SGBitSet*) const;
//         virtual void      motionDraw(const layprop::DrawProperties&, CtmQueue&, SGBitSet*) const;

         virtual void      info(std::ostringstream&, real) const;
//         virtual void      write(TEDfile* const tedfile) const;
//         virtual void      dbExport(DbExportFile&) const;
//         virtual void      psWrite(PSFile&, const layprop::DrawProperties&) const;
         virtual word      numPoints() const {return _psize;}
         virtual bool      pointInside(const TP);
//         virtual void      polyCut(PointVector&, ShapeList**);
//         virtual void      stretch(int bfactor, ShapeList**);
         virtual PointVector shape2poly() const;
         virtual PointVector dumpPoints() const;
         virtual word      lType() const {return _lmpoly;}
      private:
//         void              selectPoints(DBbox&, SGBitSet&);
//         void              unselectPoints(DBbox&, SGBitSet&);
//         PointVector*      movePointsSelected(const SGBitSet&, const CTM&, const CTM& = CTM()) const;
         int4b*            _pdata;
         unsigned          _psize;
         TessellPoly       _teseldata;
   };

}
#endif //TEDAT_EXT_H
