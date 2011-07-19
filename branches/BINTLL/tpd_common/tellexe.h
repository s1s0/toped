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
//        Created: Tue Jul 12 2011
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TELL binary
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TELLEXE_H
#define TELLEXE_H

const byte TLM_SWAP              = 0x01;
const byte TLM_SIGNX             = 0x80;
const byte TLM_SIGNY             = 0x40;

const byte TLB_NULL              = 0x00;
const byte TLB_OP_PLUS           = 0x20;
const byte TLB_OP_MINUS          = 0xA0;
const byte TLB_OP_CONCATENATE    = 0x21;
const byte TLB_OP_SHIFTPNT       = 0x22;
const byte TLB_OP_SHIFTPNT2      = 0x23;
const byte TLB_OP_SHIFTPNT3      = 0x24;
const byte TLB_OP_SHIFTPNT4      = 0x25;
const byte TLB_OP_SHIFTBOX       = 0x26;
const byte TLB_OP_SHIFTBOX3      = 0x27;
const byte TLB_OP_SHIFTBOX4      = 0x28;
const byte TLB_OP_BLOWBOX        = 0x29;
const byte TLB_OP_MULTIPLY       = 0x2A;
const byte TLB_OP_DIVISION       = 0x2B;
const byte TLB_OP_SCALEPNT       = 0x2C;
const byte TLB_OP_SCALEBOX       = 0x2D;

const byte TLB_OP_LT             = 0x30;
const byte TLB_OP_LET            = 0x31;
const byte TLB_OP_GT             = 0x32;
const byte TLB_OP_GET            = 0x33;
const byte TLB_OP_EQ             = 0x34;
const byte TLB_OP_NE             = 0x35;
const byte TLB_OP_NOT            = 0x36;
const byte TLB_OP_BWNOT          = 0x37;
const byte TLB_OP_AND            = 0x38;
const byte TLB_OP_BWAND          = 0x39;
const byte TLB_OP_OR             = 0x3A;
const byte TLB_OP_BWOR           = 0x3B;
//const byte TLB_POP_REAL          = 0x01;
//const byte TLB_POP_STRING        = 0x02;
//
//const byte TLB_PUSH_REAL         = 0x11;
//const byte TLB_PUSH_STRING       = 0x12;




   class TellBinFile {
         TellBinFile(std::string );
   };

#endif
