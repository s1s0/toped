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
CIFin::CIFWire::CIFWire(CIFData* last, pointlist* poly, word width) :
      CIFData(last), _poly(poly), _width(width) {};

//=============================================================================
CIFin::CIFRef::CIFRef(CIFData* last, word cell, CTM* location) :
      CIFData(last), _cell(cell), _location(location) {};

//=============================================================================
CIFin::CIFLabelLoc::CIFLabelLoc(CIFData* last, std::string label, TP* location) :
      CIFData(last), _label(label), _location(location) {};

//=============================================================================
CIFin::CIFLabelSig::CIFLabelSig(CIFData* last, std::string label, TP* location) :
      CIFLabelLoc(last, label, location) {};

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

void CIFin::CIFLayer::addWire(pointlist* poly, word width)
{
   _first = DEBUG_NEW CIFWire(_first, poly, width);
}

void CIFin::CIFLayer::addLabelLoc(std::string label, TP* loc)
{
   _first = DEBUG_NEW CIFLabelLoc(_first, label, loc);
}

void CIFin::CIFLayer::addLabelSig(std::string label, TP* loc)
{
   _first = DEBUG_NEW CIFLabelSig(_first, label, loc);
}

//=============================================================================
CIFin::CIFStructure::CIFStructure(word ID, CIFStructure* last, word a, word b) :
      _ID(ID), _last(last), _a(a), _b(b), _cellName(""), _first(NULL),
          _refirst(NULL), _overlap(TP()), _orphan(true), _traversed(false) {}

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

void CIFin::CIFStructure::collectLayers(CifLayerList& layList)
{
   CIFLayer* wlay = _first;
   while (NULL != wlay)
   {
      layList.push_back(wlay);
      wlay = wlay->last();
   }
}

void CIFin::CIFStructure::addRef(word cell, CTM* location)
{
   _refirst = new CIFRef(_refirst, cell, location);
}

void CIFin::CIFStructure::hierPrep(CIFFile& cfile)
{
   CIFRef* _local = _refirst;
   while (NULL != _local)
   {
      CIFStructure* celldef = cfile.getStructure(_local->cell());
      if (NULL != celldef)
      {
         celldef->parentFound();
         _children.push_back(celldef);
      }
      _local = _local->last();
   }
   _children.unique();
}

CIFin::CIFHierTree* CIFin::CIFStructure::hierOut(CIFHierTree* theTree, CIFStructure* parent)
{
   // collecting hierarchical information
   theTree = DEBUG_NEW CIFHierTree(this, parent, theTree);
   for (CIFSList::iterator CI = _children.begin(); CI != _children.end(); CI++)
   {
      theTree = (*CI)->hierOut(theTree, this);
   }
   return theTree;
}

//=============================================================================
CIFin::CIFFile::CIFFile(std::string filename)
{
   _first = _current = _default = NULL;
   _curlay = NULL;
   _hiertree = NULL;
   _filename = filename;
   std::ostringstream info;
   // feed the flex with the buffer of the input file
   //(cifin is a global variable defined in the flex generated scanner)
   if (!(cifin = fopen(filename.c_str(),"r")))
   {// open the input file
      info << "File "<< filename <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      _status = false;
      return;
   }
   info << "Parsing \"" << filename << "\" using CIF grammar";
   tell_log(console::MT_INFO,info.str());
   CIFInFile = this;
   _default = DEBUG_NEW CIFStructure(0,NULL);

   // run the bison generated parser
   ciflloc.first_column = ciflloc.first_line = 1;
   ciflloc.last_column  = ciflloc.last_line  = 1;
//   cifdebug = 1;
   cifparse();
//   my_delete_yy_buffer( buf );
   _status = true;
   fclose(cifin);
}

CIFin::CIFFile::~CIFFile()
{
   //@TODO
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

void CIFin::CIFFile::curCellName(char* cellname)
{
   if (NULL !=_current)
   {
      _current->cellNameIs(std::string(cellname));
   }
   else assert(false); // Implement a scratch cell - CIF definition allows data definition ourside the cell boundary
}

void CIFin::CIFFile::curCellOverlap(TP* bl, TP* tr)
{
   if (NULL !=_current)
   {
      _current->cellOverlapIs(bl,tr);
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

void CIFin::CIFFile::addWire(pointlist* poly, word width)
{
   _curlay->addWire(poly, width);
}

void CIFin::CIFFile::addRef(word cell, CTM* location)
{
   _current->addRef(cell, location);
}

void CIFin::CIFFile::addLabelLoc(char* label, TP* location, char* layname)
{
   CIFLayer* llay = _curlay;
   if (NULL != layname)
   {
      llay = _current->secureLayer(std::string(layname));
   }
   llay->addLabelLoc(std::string(label), location);
}

void CIFin::CIFFile::addLabelSig(char* label, TP* location)
{
   _curlay->addLabelSig(std::string(label), location);
}

void CIFin::CIFFile::collectLayers(nameList& cifLayers)
{
   CifLayerList allCiffLayers;
   CIFStructure* local = _first;
   std::ostringstream info;
   info << "\t <<List of layers>> \n";
   while (NULL != local)
   {
      local->collectLayers(allCiffLayers);
      local = local->last();
   }
   NMap laylist;
   // cifLayers.unique();
   // Unique doesn't seem to work properly after collecting all layers from 
   // all the cells. Not quite sure why.
   // Using the std::map instead
   for (CifLayerList::const_iterator LLI = allCiffLayers.begin(); LLI != allCiffLayers.end(); LLI++)
   {
      laylist[(*LLI)->name()] = 0; // map key is unique! 
   }
   // and after uniquifying - gather the unique layer names
   for (NMap::const_iterator LLI = laylist.begin(); LLI != laylist.end(); LLI++)
   {
      cifLayers.push_back(LLI->first);
   }
}

CIFin::CIFStructure* CIFin::CIFFile::getStructure(word cellno)
{
   CIFStructure* local = _first;
   while (NULL != local)
   {
      if (cellno == local->ID())
         return local;
      local = local->last();
   }
   assert(false); // Cell with this number not found ?!
}

void CIFin::CIFFile::hierPrep()
{
//   CifCellList allCiffCells;
   CIFStructure* local = _first;
/*   std::ostringstream info;
   info << "\t <<List of cells>> \n";*/
   while (NULL != local)
   {
//       allCiffCells.push_back(local);
//       info << local->ID() << "\t\t\""<< local->cellname() << "\"\n";
      local->hierPrep(*this);
      local = local->last();
   }
//   tell_log(console::MT_INFO,info.str());
}


void CIFin::CIFFile::hierOut()
{
   CIFStructure* local = _first;
   while (NULL != local)
   {
      if (local->orphan())
         _hiertree = local->hierOut(_hiertree,NULL);
      local = local->last();
   }
}
