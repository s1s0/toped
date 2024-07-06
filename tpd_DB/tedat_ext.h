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
//    Description: Layout primitives with extensions
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
                          TdtBoxEXT(const TP& p1, const TP& p2):TdtBox(p1, p2), _extLong(0),  _clientData(NULL) {};
                          TdtBoxEXT(InputTdtFile* const tedfile):TdtBox(tedfile), _extLong(0), _clientData(NULL) {};
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
                          TdtPolyEXT(const PointVector& plist):TdtPoly(plist), /*_extInt(0),*/ _extLong(0),  _clientData(NULL) {};
                          TdtPolyEXT(int4b* plist, unsigned psize):TdtPoly(plist, psize), /*_extInt(0),*/ _extLong(0),  _clientData(NULL) {};
                          TdtPolyEXT(InputTdtFile* const tedfile):TdtPoly(tedfile), /*_extInt(0),*/ _extLong(0),  _clientData(NULL) {};
      virtual            ~TdtPolyEXT() {};
      virtual void        setLong(long extLong);
      virtual long        getLong(void);
      virtual void        setString(const std::string &extString);
      virtual std::string getString(void);
      virtual void        setClientData(void* clientData);
      virtual void*       getClientData(void);
   private:
//      int                 _extInt;
      long                _extLong;
      std::string         _extString;
      void*               _clientData;
   };

   class TdtWireEXT : public TdtWire
   {
    public:
                          TdtWireEXT(const PointVector& plist, word layno):TdtWire(plist, layno), /*_extInt(0),*/ _extLong(0),  _clientData(NULL) {};
                          TdtWireEXT(int4b* plist, unsigned psize, word layno):TdtWire(plist, psize, layno), /*_extInt(0),*/ _extLong(0),  _clientData(NULL) {};
                          TdtWireEXT(InputTdtFile* const tedfile): TdtWire(tedfile), /*_extInt(0),*/ _extLong(0),  _clientData(NULL) {};
      virtual            ~TdtWireEXT() {};
      virtual void        setLong(long extLong);
      virtual long        getLong(void);
      virtual void        setString(const std::string &extString);
      virtual std::string getString(void);
      virtual void        setClientData(void* clientData);
      virtual void*       getClientData(void);
   private:
//      int                 _extInt;
      long                _extLong;
      std::string         _extString;
      void*               _clientData;
   };

}
#endif //TEDAT_EXT_H
