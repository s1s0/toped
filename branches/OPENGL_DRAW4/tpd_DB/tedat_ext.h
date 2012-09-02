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

}
#endif //TEDAT_EXT_H
