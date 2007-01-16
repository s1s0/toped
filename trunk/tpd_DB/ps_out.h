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
//        Created: Sun Jan 07 2007
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Post Script output
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef PS_OUT_H_DEFINED
#define PS_OUT_H_DEFINED

#include "../tpd_common/ttt.h"

      //namespace ...

class PSFile {
public:
                  PSFile(std::string);
   bool           checkCellWritten(std::string);
   void           registerCellWritten(std::string);
   void           formHeader(std::string, DBbox);
   void           formFooter();
   void           propSet(std::string, std::string);
   void           defineColor(std::string, byte, byte, byte);
   void           defineFill(std::string, const byte*);
   void           poly(const pointlist, const DBbox);
   void           wire(const pointlist, word, const DBbox);
   void           text(std::string, const CTM);
   void           cellref(std::string, const CTM);
   void           pspage(std::string, const DBbox);
                 ~PSFile();
protected:
   void           writeStdDefs();
   FILE*          _psfh;
   std::string    _fname;
   nameList       _childnames;
};

#endif // PS_OUT_H_DEFINED
