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

CIFin::CifFile* CIFInFile = NULL;
//extern void*   cif_scan_string(const char *str);
//extern void    my_delete_yy_buffer( void* b );
extern int     cifparse(); // Calls the bison generated parser
extern FILE*   cifin;
extern int     cifdebug;


//=============================================================================
CIFin::CifBox::CifBox(CifData* last, _dbl_word length, _dbl_word width, TP* center, TP* direction) :
                           CifData(last), _length(length), _width(width), _center(center),
                                   _direction(direction) {};

//=============================================================================
CIFin::CifPoly::CifPoly(CifData* last, pointlist* poly) :
      CifData(last), _poly(poly) {};

//=============================================================================
CIFin::CifWire::CifWire(CifData* last, pointlist* poly, _dbl_word width) :
      CifData(last), _poly(poly), _width(width) {};

//=============================================================================
CIFin::CifRef::CifRef(CifData* last, _dbl_word cell, CTM* location) :
      CifData(last), _cell(cell), _location(location) {};

//=============================================================================
CIFin::CifLabelLoc::CifLabelLoc(CifData* last, std::string label, TP* location) :
      CifData(last), _label(label), _location(location) {};

//=============================================================================
CIFin::CifLabelSig::CifLabelSig(CifData* last, std::string label, TP* location) :
      CifLabelLoc(last, label, location) {};

//=============================================================================
CIFin::CifLayer::CifLayer(std::string name, CifLayer* last):
      _name(name), _last(last), _first(NULL) {}

void CIFin::CifLayer::addBox(_dbl_word length,_dbl_word width ,TP* center, TP* direction)
{
   _first = DEBUG_NEW CifBox(_first, length, width, center, direction);
}

void CIFin::CifLayer::addPoly(pointlist* poly)
{
   _first = DEBUG_NEW CifPoly(_first, poly);
}

void CIFin::CifLayer::addWire(pointlist* poly, _dbl_word width)
{
   _first = DEBUG_NEW CifWire(_first, poly, width);
}

void CIFin::CifLayer::addLabelLoc(std::string label, TP* loc)
{
   _first = DEBUG_NEW CifLabelLoc(_first, label, loc);
}

void CIFin::CifLayer::addLabelSig(std::string label, TP* loc)
{
   _first = DEBUG_NEW CifLabelSig(_first, label, loc);
}

//=============================================================================
CIFin::CifStructure::CifStructure(_dbl_word ID, CifStructure* last, _dbl_word a, _dbl_word b) :
      _ID(ID), _last(last), _a(a), _b(b), _cellName(""), _first(NULL),
          _refirst(NULL), _overlap(TP()), _orphan(true), _traversed(false) {}

CIFin::CifLayer* CIFin::CifStructure::secureLayer(std::string name)
{
   CifLayer* wlay = _first;
   while (NULL != wlay)
   {
      if (name == wlay->name()) return wlay;
      wlay = wlay->last();
   }
   _first = DEBUG_NEW CifLayer(name, _first);
   return _first;
}

void CIFin::CifStructure::collectLayers(CifLayerList& layList)
{
   CifLayer* wlay = _first;
   while (NULL != wlay)
   {
      layList.push_back(wlay);
      wlay = wlay->last();
   }
}

void CIFin::CifStructure::addRef(_dbl_word cell, CTM* location)
{
   _refirst = new CifRef(_refirst, cell, location);
}

void CIFin::CifStructure::hierPrep(CifFile& cfile)
{
   CifRef* _local = _refirst;
   while (NULL != _local)
   {
      CifStructure* celldef = cfile.getStructure(_local->cell());
      if (NULL != celldef)
      {
         celldef->parentFound();
         _children.push_back(celldef);
      }
      _local = _local->last();
   }
   _children.unique();
}

CIFin::CIFHierTree* CIFin::CifStructure::hierOut(CIFHierTree* theTree, CifStructure* parent)
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
CIFin::CifFile::CifFile(std::string filename)
{
   _first = _current = _default = NULL;
   _curlay = NULL;
   _hiertree = NULL;
   _filename = filename;
   std::ostringstream info;
   // feed the flex with the buffer of the input file
   //(cifin is a global variable defined in the flex generated scanner)
   if (!(cifin = fopen(_filename.c_str(),"r")))
   {// open the input file
      info << "File "<< _filename <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      _status = false;
      return;
   }
   info << "Parsing \"" << _filename << "\" using CIF grammar";
   tell_log(console::MT_INFO,info.str());
   CIFInFile = this;
   _default = DEBUG_NEW CifStructure(0,NULL);
   _default->cellNameIs(getFileNameOnly(filename));

   // run the bison generated parser
   ciflloc.first_column = ciflloc.first_line = 1;
   ciflloc.last_column  = ciflloc.last_line  = 1;
/*   cifdebug = 1;*/
   cifparse();
//   my_delete_yy_buffer( buf );
   _status = true;
   fclose(cifin);
}

CIFin::CifFile::~CifFile()
{
   //@TODO
}

void CIFin::CifFile::addStructure(_dbl_word ID, _dbl_word a, _dbl_word b)
{
   _first = DEBUG_NEW CifStructure(ID,_first, a,b);
   _current = _first;
}

void CIFin::CifFile::doneStructure()
{
   _current = _default;
}

void CIFin::CifFile::secureLayer(char* layname)
{
   if (NULL !=_current)
   {
      _curlay = _current->secureLayer(std::string(layname));
   }
   else assert(false); // Implement a scratch cell - CIF definition allows data definition ourside the cell boundary
}

void CIFin::CifFile::curCellName(char* cellname)
{
   if (NULL !=_current)
   {
      _current->cellNameIs(std::string(cellname));
   }
   else assert(false); // Implement a scratch cell - CIF definition allows data definition ourside the cell boundary
}

void CIFin::CifFile::curCellOverlap(TP* bl, TP* tr)
{
   if (NULL !=_current)
   {
      _current->cellOverlapIs(bl,tr);
   }
   else assert(false); // Implement a scratch cell - CIF definition allows data definition ourside the cell boundary
}

void CIFin::CifFile::addBox(_dbl_word length, _dbl_word width ,TP* center, TP* direction)
{
   _curlay->addBox(length, width, center, direction);
}

void CIFin::CifFile::addPoly(pointlist* poly)
{
   _curlay->addPoly(poly);
}

void CIFin::CifFile::addWire(pointlist* poly, _dbl_word width)
{
   _curlay->addWire(poly, width);
}

void CIFin::CifFile::addRef(_dbl_word cell, CTM* location)
{
   _current->addRef(cell, location);
}

void CIFin::CifFile::addLabelLoc(char* label, TP* location, char* layname)
{
   CifLayer* llay = _curlay;
   if (NULL != layname)
   {
      llay = _current->secureLayer(std::string(layname));
   }
   llay->addLabelLoc(std::string(label), location);
}

void CIFin::CifFile::addLabelSig(char* label, TP* location)
{
   _curlay->addLabelSig(std::string(label), location);
}

void CIFin::CifFile::collectLayers(nameList& cifLayers)
{
   CifLayerList allCiffLayers;
   CifStructure* local = _first;
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

CIFin::CifStructure* CIFin::CifFile::getStructure(_dbl_word cellno)
{
   CifStructure* local = _first;
   while (NULL != local)
   {
      if (cellno == local->ID())
         return local;
      local = local->last();
   }
   assert(false); // Cell with this number not found ?!
}

CIFin::CifStructure* CIFin::CifFile::getStructure(std::string cellname)
{
   if (cellname == _default->cellName()) return _default;
   CifStructure* local = _first;
   while (NULL != local)
   {
      if (cellname == local->cellName())
         return local;
      local = local->last();
   }
   return NULL; // Cell with this name not found ?!
}

void CIFin::CifFile::hierPrep()
{
//   CifCellList allCiffCells;
   _default->hierPrep(*this);
   CifStructure* local = _first;
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


void CIFin::CifFile::hierOut()
{
   _hiertree = _default->hierOut(_hiertree,NULL);

   CifStructure* local = _first;
   while (NULL != local)
   {
      if (local->orphan())
         _hiertree = local->hierOut(_hiertree,NULL);
      local = local->last();
   }
}
