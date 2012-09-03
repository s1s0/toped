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

#include "drawprop.h"

      //namespace ...

class PSFile {
public:
                  PSFile(std::string, const layprop::DrawProperties&);
   bool           checkCellWritten(std::string);
   void           registerCellWritten(std::string);
   void           cellHeader(std::string, DBbox);
   void           cellFooter();
   void           propSet(std::string, std::string);
   void           poly(const int4b* const, unsigned, const DBbox);
   void           wire(const int4b* const, unsigned, WireWidth, const DBbox);
   void           text(std::string, const CTM);
   void           cellref(std::string, const CTM);
   void           pspage_header(const DBbox);
   void           pspage_footer(std::string);
   bool           hier()   {return _hierarchical;}
                 ~PSFile();
protected:
   void           writeStdDefs();
   void           writeProperties(const layprop::DrawProperties&);
   void           defineColor(std::string, byte, byte, byte);
   void           defineFill(std::string, const byte*);
   FILE*          _psfh;
   std::string    _fname;
   NameList       _childnames;
   bool           _hierarchical;
};

#endif // PS_OUT_H_DEFINED
