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
//   This file is a part of Toped project (C) 2001-2008 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun May 04 2008
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: CIF parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "cif_io.h"
#include "cif_yacc.h"
#include "../tpd_common/outbox.h"

CIFin::CIFFile* CIFInFile = NULL;
//extern void*   cif_scan_string(const char *str);
//extern void    my_delete_yy_buffer( void* b );
extern int     cifparse(); // Calls the bison generated parser
extern FILE*   cifin;
extern int     cifdebug;


//=============================================================================
CIFin::CIFBox::CIFBox(CIFData* last, word length, word width, TP* center, TP* direction) :
                           CIFData(last), _length(length), _width(width), _center(center),
                                   _direction(direction) {};

//=============================================================================
CIFin::CIFPoly::CIFPoly(CIFData* last, pointlist* poly) :
      CIFData(last), _poly(poly) {};

//=============================================================================
CIFin::CIFLayer::CIFLayer(std::string name, CIFLayer* last):
      _name(name), _last(last), _first(NULL) {}

void CIFin::CIFLayer::addBox(word length,word width ,TP* center, TP* direction)
{
   _first = DEBUG_NEW CIFBox(_first, length, width, center, direction);
}

void CIFin::CIFLayer::addPoly(pointlist* poly)
{
   _first = DEBUG_NEW CIFPoly(_first, poly);
}
//=============================================================================
CIFin::CIFStructure::CIFStructure(word ID, CIFStructure* last, word a, word b) :
      _ID(ID), _last(last), _a(a), _b(b), _cellname(""), _first(NULL) { }

CIFin::CIFLayer* CIFin::CIFStructure::secureLayer(std::string name)
{
   CIFLayer* wlay = _first;
   while (NULL != wlay)
   {
      if (name == wlay->name()) return wlay;
      wlay = wlay->last();
   }
   _first = DEBUG_NEW CIFLayer(name, _first);
   return _first;
}

//=============================================================================
CIFin::CIFFile::CIFFile(std::string filename)
{
   _first = _current = _default = NULL;
   _curlay = NULL;
   // feed the flex with the buffer of the input file
   //(cifin is a global variable defined in the flex generated scanner)
   if (!(cifin = fopen(filename.c_str(),"r")))
   {// open the input file
      std::ostringstream info;
      info << "File "<< filename <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      _status = 0;
      return;
   }
   CIFInFile = this;
   _default = DEBUG_NEW CIFStructure(0,NULL);

   // run the bison generated parser
   ciflloc.first_column = ciflloc.first_line = 1;
   ciflloc.last_column  = ciflloc.last_line  = 1;
//   cifdebug = 1;
   cifparse();
//   my_delete_yy_buffer( buf );
   fclose(cifin);
}

void CIFin::CIFFile::addStructure(word ID, word a, word b)
{
   _first = DEBUG_NEW CIFStructure(ID,_first, a,b);
   _current = _first;
}

void CIFin::CIFFile::doneStructure()
{
   _current = _default;
}

void CIFin::CIFFile::secureLayer(char* layname)
{
   if (NULL !=_current)
   {
      _curlay = _current->secureLayer(std::string(layname));
   }
   else assert(false); // Implement a scratch cell - CIF definition allows data definition ourside the cell boundary
}

void CIFin::CIFFile::addBox(word length,word width ,TP* center, TP* direction)
{
   _curlay->addBox(length, width, center, direction);
}

void CIFin::CIFFile::addPoly(pointlist* poly)
{
   _curlay->addPoly(poly);
}
