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
extern void*   new_cif_lex_buffer( FILE* cifin );
extern void    delete_cif_lex_buffer( void* b ) ;
extern int     cifparse(); // Calls the bison generated parser
extern FILE*   cifin;
extern int     cifdebug;
extern int     cifnerrs;


//=============================================================================
CIFin::CifBox::CifBox(CifData* last, _dbl_word length, _dbl_word width, TP* center, TP* direction) :
                           CifData(last), _length(length), _width(width), _center(center),
                                   _direction(direction) {}

CIFin::CifBox::~CifBox()
{
   delete _center;
   delete _direction;
}
//=============================================================================
CIFin::CifPoly::CifPoly(CifData* last, pointlist* poly) :
      CifData(last), _poly(poly) {}

CIFin::CifPoly::~CifPoly()
{
   delete _poly;
}
//=============================================================================
CIFin::CifWire::CifWire(CifData* last, pointlist* poly, _dbl_word width) :
      CifData(last), _poly(poly), _width(width) {}

CIFin::CifWire::~CifWire()
{
   delete _poly;
}
//=============================================================================
CIFin::CifRef::CifRef(CifData* last, _dbl_word cell, CTM* location) :
      CifData(last), _cell(cell), _location(location) {}

CIFin::CifRef::~CifRef()
{
   delete _location;
}
//=============================================================================
CIFin::CifLabelLoc::CifLabelLoc(CifData* last, std::string label, TP* location) :
      CifData(last), _label(label), _location(location) {}

CIFin::CifLabelLoc::~CifLabelLoc()
{
   delete _location;
}
//=============================================================================
CIFin::CifLabelSig::CifLabelSig(CifData* last, std::string label, TP* location) :
      CifLabelLoc(last, label, location) {}

//=============================================================================
CIFin::CifLayer::CifLayer(std::string name, CifLayer* last):
      _name(name), _last(last), _first(NULL) {}

CIFin::CifLayer::~CifLayer()
{
   CifData* wdata = _first;
   CifData* wdata4d;
   while (wdata)
   {
      wdata4d = wdata;
      wdata = wdata->last();
      delete wdata4d;
   }
}


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
      _ID(ID), _last(last), _a(a), _b(b), _name(""), _first(NULL),
          _refirst(NULL), _overlap(TP()), _orphan(true), _traversed(false) {}

CIFin::CifStructure::~CifStructure()
{
   // Remove all layers ...
   CifLayer* wlay = _first;
   CifLayer* wlay4d;
   while (NULL != wlay)
   {
      wlay4d = wlay;
      wlay = wlay->last();
      delete wlay4d;
   }
   // ... and all references
   CifRef* wref = _refirst;
   CifRef* wref4d;
   while (NULL != wref)
   {
      wref4d = wref;
      wref = wref->last();
      delete wref4d;
   }
}

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
   _refirst = DEBUG_NEW CifRef(_refirst, cell, location);
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

   if ("" == _name)
   {
      std::ostringstream tmp_name;
      tmp_name << "_cifCellNo_" << _ID;
      _name = tmp_name.str();
      std::ostringstream news;
      news << "Name \"" << _name << "\" assigned automatically to CIF cell "<< _ID ;
      tell_log(console::MT_INFO,news.str());
   }
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
   _curLay = NULL;
   _hierTree = NULL;
   _fileName = filename;
   std::ostringstream info;
   // Open the input file
	std::string fname(convertString(_fileName));
   if (!(_cifFh = fopen(fname.c_str(),"rt"))) // open the input file
   {
      _status = cfs_FNF; return;
   }
   // feed the flex with the buffer of the input file
   void* b = new_cif_lex_buffer( _cifFh );
   info << "Parsing \"" << _fileName << "\" using CIF grammar";
   tell_log(console::MT_INFO,info.str());
   CIFInFile = this;
   _default = DEBUG_NEW CifStructure(0,NULL);
   _default->cellNameIs(std::string(getFileNameOnly(filename) + "_cif"));

   // run the bison generated parser
   ciflloc.first_column = ciflloc.first_line = 1;
   ciflloc.last_column  = ciflloc.last_line  = 1;
/*   cifdebug = 1;*/
   cifparse();
   delete_cif_lex_buffer( b );
   if (cifnerrs > 0) _status = cfs_ERR;
   else              _status = cfs_POK;
   closeFile();
}

CIFin::CifFile::~CifFile()
{
   // delete all CIF structures
   CifStructure* local = _first;
   CifStructure* local4d;
   while (NULL != local)
   {
      local4d = local;
      local = local->last();
      delete local4d;
   }
   // get rid of the hierarchy tree
   const CIFHierTree* hlocal = _hierTree;
   const CIFHierTree* hlocal4d;
   while (hlocal)
   {
      hlocal4d = hlocal;
      hlocal = hlocal->GetLast();
      delete hlocal4d;
   }

   delete _default;
   closeFile();
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
      _curLay = _current->secureLayer(std::string(layname));
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
   _curLay->addBox(length, width, center, direction);
}

void CIFin::CifFile::addPoly(pointlist* poly)
{
   _curLay->addPoly(poly);
}

void CIFin::CifFile::addWire(pointlist* poly, _dbl_word width)
{
   _curLay->addWire(poly, width);
}

void CIFin::CifFile::addRef(_dbl_word cell, CTM* location)
{
   _current->addRef(cell, location);
}

void CIFin::CifFile::addLabelLoc(char* label, TP* location, char* layname)
{
   CifLayer* llay = _curLay;
   if (NULL != layname)
   {
      llay = _current->secureLayer(std::string(layname));
   }
   llay->addLabelLoc(std::string(label), location);
}

void CIFin::CifFile::addLabelSig(char* label, TP* location)
{
   _curLay->addLabelSig(std::string(label), location);
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
   SIMap laylist;
   // cifLayers.unique();
   // Unique doesn't seem to work properly after collecting all layers from 
   // all the cells. Not quite sure why.
   // Using the std::map instead
   for (CifLayerList::const_iterator LLI = allCiffLayers.begin(); LLI != allCiffLayers.end(); LLI++)
   {
      laylist[(*LLI)->name()] = 0; // map key is unique! 
   }
   // and after uniquifying - gather the unique layer names
   for (SIMap::const_iterator LLI = laylist.begin(); LLI != laylist.end(); LLI++)
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
   return NULL;
}

CIFin::CifStructure* CIFin::CifFile::getStructure(std::string cellname)
{
   if (cellname == _default->name()) return _default;
   CifStructure* local = _first;
   while (NULL != local)
   {
      if (cellname == local->name())
         return local;
      local = local->last();
   }
   return NULL; // Cell with this name not found ?!
}

void CIFin::CifFile::hierPrep()
{
   _default->hierPrep(*this);
   CifStructure* local = _first;
   while (NULL != local)
   {
      local->hierPrep(*this);
      local = local->last();
   }
}

void CIFin::CifFile::hierOut()
{
   _hierTree = _default->hierOut(_hierTree,NULL);

   CifStructure* local = _first;
   while (NULL != local)
   {
      if (local->orphan())
         _hierTree = local->hierOut(_hierTree,NULL);
      local = local->last();
   }
}

void CIFin::CifFile::closeFile()
{
   if (NULL != _cifFh)
   {
      fclose(_cifFh); _cifFh = NULL;
   }
   CIFInFile = NULL;
}

//=============================================================================
CIFin::CifExportFile::CifExportFile(std::string fn, USMap* laymap, bool verbose)
{
   _fileName = fn;
   _laymap = laymap;
   _verbose = verbose;
   _lastcellnum = 0;
   std::string fname(convertString(_fileName));
   _file.open(_fileName.c_str(), std::ios::out);
   //@TODO how to check for an error ?
   // start writing
   TpdTime timec(time(NULL));

   _file << "(              CIF   2.0       );"    << std::endl;
   _file << "(        generator : Toped 0.9.x );"  << std::endl;
   _file << "(             user : tbd );"          << std::endl;
   _file << "(          machine : tbd );"          << std::endl;
   _file << "(       time stamp : " << timec() << ");" << std::endl;

}

bool CIFin::CifExportFile::checkCellWritten(std::string cellname) const
{
   return (_cellmap.end() != _cellmap.find(cellname));
}

void CIFin::CifExportFile::registerCellWritten(std::string cellname)
{
   assert(_cellmap.end() == _cellmap.find(cellname));
   _cellmap[cellname] = ++_lastcellnum;
}

void CIFin::CifExportFile::definitionStart(std::string name)
{
   std::string message = "...converting " + name;
   tell_log(console::MT_INFO, message);
   registerCellWritten(name);
   if (_verbose)
      _file << std::endl << "Definition Start #" << _lastcellnum << ";"<< std::endl;
   else
      _file << std::endl << "DS " << _lastcellnum << ";" << std::endl;
   _file << "   9 "<< name << ";"<< std::endl;
}

void CIFin::CifExportFile::definitionFinish()
{
   if (_verbose)
      _file << "Definition Finish;" << std::endl;
   else
      _file << "DF;" << std::endl;
}

bool CIFin::CifExportFile::layerSpecification(word layno)
{
   if (0 == layno) return true;
   if (_laymap->end() == _laymap->find(layno))
   {
      std::stringstream message;
      message << "   Layer " << layno <<" not found in the layer map and will not be converted";
      tell_log(console::MT_INFO, message.str());
      return false;
   }
   if (_verbose)
      _file << "   Layer "<< (*_laymap)[layno] << " objects follow;" << std::endl;
   else
      _file << "L " << (*_laymap)[layno] << ";" << std::endl;
   return true;
}

void CIFin::CifExportFile::box(const unsigned length, const unsigned width, const TP& center)
{
   if (_verbose)
      _file << "      Box length = "<< length << " width = "<< width <<
            " and center = " << center.x() << "," << center.y() << ";" << std::endl;
   else
      _file << "      B"<< length << " " << width << " " << center.x() <<
            " " << center.y() << ";" << std::endl;
}

void CIFin::CifExportFile::polygon(const pointlist& plst)
{
   if (_verbose)
      _file <<"      Polygon with vertices";
   else
      _file <<"      P";
   for (pointlist::const_iterator CP = plst.begin(); CP != plst.end(); CP++)
      _file << " " << CP->x() << " " << CP->y();
   _file << ";"<< std::endl;
}

void CIFin::CifExportFile::wire(unsigned width, const pointlist& plst)
{
   if (_verbose)
      _file <<"      Wire width = " << width << "and points";
   else
      _file <<"      W" << width;
   for (pointlist::const_iterator CP = plst.begin(); CP != plst.end(); CP++)
      _file << " " << CP->x() << " " << CP->y();
   _file << ";"<< std::endl;
}

void CIFin::CifExportFile::text(const std::string& label, const TP& center)
{
   int loc;
   std::string labelr(label);
   while ((loc = labelr.find(' ')) >= 0 )
      labelr.replace(loc, 1, "_"); //@FIXME - this should be an option or ...?

   _file << "      94 "<< labelr << " "<< center.x() << " " << center.y() << ";" << std::endl;


}

void CIFin::CifExportFile::call(const std::string& cellname, const CTM& tmatrix)
{
   assert(_cellmap.end() != _cellmap.find(cellname));

   TP trans;
   real rot;
   real scale;
   bool flipX;

   tmatrix.Decompose(trans, rot, scale, flipX);
   if (1.0 != scale) assert(false); //@TODO CIF scaling ???
   int4b resultX = static_cast<int4b>(cos(rot*M_PI/180) * 1e6);
   int4b resultY = static_cast<int4b>(sin(rot*M_PI/180) * 1e6);
   if       (0 == resultX) resultY = abs(resultY) / resultY;
   else if (0 == resultY) resultX = abs(resultX) / resultX;
   else if (abs(resultX) == abs(resultY))
   {
      resultX = abs(resultX) / resultX;
      resultY = abs(resultY) / resultY;
   }
   else if (0 == (resultX % resultY))
      resultX /= resultY;
   else if (0 == (resultY % resultX))
      resultY /= resultX;

   if (_verbose)
   {
      _file <<"      Call symbol #" << _cellmap[cellname];
      if (       flipX) _file << " Mirrored in Y";
      if (0.0 != rot  ) _file << " Rotated to " << resultX << " " << resultY;
      _file << " Translated to " << trans.x() << " " << trans.y();
   }
   else
   {
      _file <<"      C" << _cellmap[cellname];
      if (       flipX) _file << " MY";
      if (0.0 != rot  ) _file << " R " << resultX << " " << resultY;
      _file << " T" << trans.x() << " " << trans.y();
   }
   _file << ";"<< std::endl;
}

CIFin::CifExportFile::~CifExportFile()
{
   _file << "End" << std::endl;
   _file.close();
   // don't delete _laymap - it's should be deleted where it had been created
}
