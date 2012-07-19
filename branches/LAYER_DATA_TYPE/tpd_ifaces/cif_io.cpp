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
#include "outbox.h"
#include "tedat.h"
#include "tedesign.h"


CIFin::CifFile* CIFInFile = NULL;
extern int     cifparse(); // Calls the bison generated parser
namespace CIFin {
   extern void    flushParserBuffer();
};

//=============================================================================
CIFin::CifBox::CifBox(CifData* last, dword length, dword width, TP* center, TP* direction) :
                           CifData(last), _length(length), _width(width), _center(center),
                                   _direction(direction) {}

void CIFin::CifBox::import ( ImportDB& iDB ) const
{
   PointVector plist;   plist.reserve(4);
   real cX, cY;

   cX = rint(((real)_center->x() - (real)_length/ 2.0f) * iDB.crossCoeff() );
   cY = rint(((real)_center->y() - (real)_width / 2.0f) * iDB.crossCoeff() );
   TP cpnt1( (int4b)cX, (int4b)cY );   plist.push_back(cpnt1);

   cX = rint(((real)_center->x() + (real)_length/ 2.0f) * iDB.crossCoeff() );
   cY = rint(((real)_center->y() - (real)_width / 2.0f) * iDB.crossCoeff() );
   TP cpnt2( (int4b)cX, (int4b)cY );   plist.push_back(cpnt2);

   cX = rint(((real)_center->x() + (real)_length/ 2.0f) * iDB.crossCoeff() );
   cY = rint(((real)_center->y() + (real)_width / 2.0f) * iDB.crossCoeff() );
   TP cpnt3( (int4b)cX, (int4b)cY );   plist.push_back(cpnt3);

   cX = rint(((real)_center->x() - (real)_length/ 2.0f) * iDB.crossCoeff() );
   cY = rint(((real)_center->y() + (real)_width / 2.0f) * iDB.crossCoeff() );
   TP cpnt4( (int4b)cX, (int4b)cY );   plist.push_back(cpnt4);
   if ( NULL != _direction )
   {
      CTM tmx;
      cX = (real)_center->x() * iDB.crossCoeff();
      cY = (real)_center->y() * iDB.crossCoeff();
      tmx.Translate(-cX,-cY);
      tmx.Rotate(*(_direction));
      tmx.Translate(cX,cY);
      plist[0] *=  tmx;
      plist[1] *=  tmx;
      plist[2] *=  tmx;
      plist[3] *=  tmx;
   }
   iDB.addPoly(plist);
}

CIFin::CifBox::~CifBox()
{
   delete _center;
   delete _direction;
}
//=============================================================================
CIFin::CifPoly::CifPoly(CifData* last, PointVector* poly) :
      CifData(last), _poly(poly) {}


void CIFin::CifPoly::import( ImportDB& iDB ) const
{
   PointVector plist;
   plist.reserve(_poly->size());
   for(PointVector::const_iterator CP = _poly->begin(); CP != _poly->end(); CP++)
   {
      TP pnt(*CP);
      pnt *= iDB.crossCoeff();
      plist.push_back(pnt);
   }
   iDB.addPoly(plist);
}

CIFin::CifPoly::~CifPoly()
{
   delete _poly;
}

//=============================================================================
CIFin::CifWire::CifWire(CifData* last, PointVector* poly, dword width) :
      CifData(last), _poly(poly), _width(width) {}

void CIFin::CifWire::import( ImportDB& iDB ) const
{
   PointVector plist;
   plist.reserve(_poly->size());
   for(PointVector::const_iterator CP = _poly->begin(); CP != _poly->end(); CP++)
   {
      TP pnt(*CP);
      pnt *= iDB.crossCoeff();
      plist.push_back(pnt);
   }
   dword width = (dword) rint(_width * iDB.crossCoeff());
   // actually all CIF wires correspond to GDS type 1 (rounded ends),
   // but they are silently converted here to type 2, because Toped
   // doesn't support wire type 1 at all
   iDB.addPath(plist, width, 2);
}

CIFin::CifWire::~CifWire()
{
   delete _poly;
}

//=============================================================================
CIFin::CifRef::CifRef(CifData* last, dword cell, CTM* location) :
      CifData(last), _cell(cell), _location(location) {}

void CIFin::CifRef::import ( ImportDB& iDB ) const
{
   CifFile* cf = static_cast<CifFile*>(iDB.srcFile());
   ForeignCell* refd = cf->getStructure(_cell);
   std::string cell_name = refd->strctName();

   iDB.addRef(cell_name, (*_location) * iDB.crossCoeff());
}

CIFin::CifRef::~CifRef()
{
   delete _location;
}

//=============================================================================
CIFin::CifLabelLoc::CifLabelLoc(CifData* last, std::string label, TP* location) :
      CifData(last), _label(label), _location(location) {}

void CIFin::CifLabelLoc::import( ImportDB& iDB ) const
{
   // CIF doesn't have a concept of texts (as GDS)
   // text size and placement are just the default
   if (0.0 == iDB.technoSize()) return;
   TP pnt(*(_location));
   pnt *= iDB.crossCoeff();

   iDB.addText(_label, pnt, iDB.technoSize());
}

CIFin::CifLabelLoc::~CifLabelLoc()
{
   delete _location;
}

//=============================================================================
CIFin::CifLabelSig::CifLabelSig(CifData* last, std::string label, TP* location) :
      CifLabelLoc(last, label, location)
{}

void CIFin::CifLabelSig::import(  ImportDB& iDB ) const
{
   //TODO > what to do with those objects?
}

//=============================================================================
CIFin::CifLayer::CifLayer(std::string name, CifLayer* last):
      _name(name), _last(last), _first(NULL)
{}

CIFin::CifLayer::~CifLayer()
{
   const CifData* wdata = _first;
   const CifData* wdata4d;
   while (wdata)
   {
      wdata4d = wdata;
      wdata = wdata->last();
      delete wdata4d;
   }
}

void CIFin::CifLayer::addBox(dword length,dword width ,TP* center, TP* direction)
{
   _first = DEBUG_NEW CifBox(_first, length, width, center, direction);
}

void CIFin::CifLayer::addPoly(PointVector* poly)
{
   _first = DEBUG_NEW CifPoly(_first, poly);
}

void CIFin::CifLayer::addWire(PointVector* poly, dword width)
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
CIFin::CifStructure::CifStructure(dword ID, CifStructure* last, dword a, dword b) :
      ForeignCell(), _ID(ID), _last(last), _a(a), _b(b), _first(NULL),
      _refirst(NULL), _overlap(TP()) {}

CIFin::CifStructure::~CifStructure()
{
   // Remove all layers ...
   const CifLayer* wlay = _first;
   const CifLayer* wlay4d;
   while (NULL != wlay)
   {
      wlay4d = wlay;
      wlay = wlay->last();
      delete wlay4d;
   }
   // ... and all references
   const CifRef* wref = _refirst;
   const CifRef* wref4d;
   while (NULL != wref)
   {
      wref4d = wref;
      wref = wref->last();
      delete wref4d;
   }
}

CIFin::CifLayer* CIFin::CifStructure::secureLayer(std::string name)
{
   const CifLayer* wlay = _first;
   while (NULL != wlay)
   {
      if (name == wlay->name()) return const_cast<CIFin::CifLayer*>(wlay);
      wlay = wlay->last();
   }
   _first = DEBUG_NEW CifLayer(name, _first);
   return _first;
}

void CIFin::CifStructure::collectLayers(NameList& layList, bool hier) const
{
   const CifLayer* wlay = _first;
   while (NULL != wlay)
   {
      layList.push_back(wlay->name());
      wlay = wlay->last();
   }
   layList.sort();
   layList.unique();

   if (!hier) return;
   for (CIFSList::const_iterator CCS = _children.begin(); CCS != _children.end(); CCS++)
      (*CCS)->collectLayers(layList, hier);
}

void CIFin::CifStructure::addRef(dword cell, CTM* location)
{
   _refirst = DEBUG_NEW CifRef(_refirst, cell, location);
}

void CIFin::CifStructure::linkReferences(CifFile& cfile)
{
   const CifRef* _local = _refirst;
   while (NULL != _local)
   {
      CifStructure* celldef = cfile.getStructure(_local->cell());
      if (NULL != celldef)
      {
         celldef->_haveParent = true;
         _children.push_back(celldef);
      }
      _local = _local->last();
   }
   _children.sort();
   _children.unique();

   if ("" == _strctName)
   {
      std::ostringstream tmp_name;
      tmp_name << "_cifCellNo_" << _ID;
      _strctName = tmp_name.str();
      std::ostringstream news;
      news << "Name \"" << strctName() << "\" assigned automatically to CIF cell "<< _ID ;
      tell_log(console::MT_INFO,news.str());
   }
}

ForeignCellTree* CIFin::CifStructure::hierOut(ForeignCellTree* theTree, CifStructure* parent)
{
   // collecting hierarchical information
   theTree = DEBUG_NEW ForeignCellTree(this, parent, theTree);
   for (CIFSList::iterator CI = _children.begin(); CI != _children.end(); CI++)
   {
      theTree = (*CI)->hierOut(theTree, this);
   }
   return theTree;
}

void CIFin::CifStructure::import(ImportDB& iDB)
{
   iDB.calcCrossCoeff(a() / b());
   const CIFin::CifLayer* swl = _first;
   while( swl ) // loop trough the layers
   {
      if (iDB.mapTdtLayer( swl->name() ))
      {
         const CIFin::CifData* wd = swl->firstData();
         while ( wd ) // loop trough data
         {
            wd->import(iDB);
            wd = wd->last();
         }
      }
      //else
      //{
      //   std::ostringstream ost;
      //   ost << "CIF Layer name \"" << swl->name() << "\" is not defined in the function input parameter. Will be omitted";
      //   tell_log(console::MT_INFO,ost.str());
      //}
      swl = swl->last();
   }
   // Now translate the references
   const CIFin::CifRef* swr = _refirst;
   while ( swr )
   {
      swr->import(iDB);
      swr = swr->last();
   }
}

//=============================================================================
CIFin::CifFile::CifFile(wxString wxfname) : ForeignDbFile(wxfname, false)
{
   _first = _current = _default = NULL;
   _curLay = NULL;
   std::ostringstream info;
   if (!status())
   {
      throw EXPTNcif_parser("Failed to open input file");
   }
   info << "Parsing \"" << fileName() << "\" using CIF grammar";
   tell_log(console::MT_INFO,info.str());
   CIFInFile = this;
   _default = DEBUG_NEW CifStructure(0,NULL);
   _default->setStrctName(std::string(getFileNameOnly() + "_cif"));

   // run the bison generated parser
   ciflloc.first_column = ciflloc.first_line = 1;
   ciflloc.last_column  = ciflloc.last_line  = 1;
   // This is to make sure that the parser will start from a clean buffer
   // It is preventing the case when after a syntax error in the CIF file,
   // another file can not be parsed, because the buffer is still full with
   // data
   flushParserBuffer();
   cifparse();
   linkReferences();
   // Close the input stream when done
   closeStream();
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
   delete _default;
}

void CIFin::CifFile::addStructure(dword ID, dword a, dword b)
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
      _current->setStrctName(std::string(cellname));
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

void CIFin::CifFile::addBox(dword length, dword width ,TP* center, TP* direction)
{
   _curLay->addBox(length, width, center, direction);
}

void CIFin::CifFile::addPoly(PointVector* poly)
{
   _curLay->addPoly(poly);
}

void CIFin::CifFile::addWire(PointVector* poly, dword width)
{
   _curLay->addWire(poly, width);
}

void CIFin::CifFile::addRef(dword cell, CTM* location)
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

void CIFin::CifFile::collectLayers(NameList& cifLayers) const
{
   CifStructure* local = _first;
   while (NULL != local)
   {
      local->collectLayers(cifLayers, false);
      local = local->last();
   }
   cifLayers.sort();
   cifLayers.unique();
}

bool CIFin::CifFile::collectLayers(const std::string& name, NameList& cifLayers ) const
{
   const CIFin::CifStructure *src_structure = getStructure(name.c_str());
   if (NULL == src_structure) return false;
   src_structure->collectLayers(cifLayers, true);
   return true;
}

CIFin::CifStructure* CIFin::CifFile::getStructure(dword cellno)
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

const CIFin::CifStructure* CIFin::CifFile::getStructure(const std::string& cellname) const
{
   if (cellname == _default->strctName()) return _default;
   CifStructure* local = _first;
   while (NULL != local)
   {
      if (cellname == local->strctName())
         return local;
      local = local->last();
   }
   return NULL; // Cell with this name not found ?!
}

void CIFin::CifFile::linkReferences()
{
   _default->linkReferences(*this);
   CifStructure* local = _first;
   while (NULL != local)
   {
      local->linkReferences(*this);
      local = local->last();
   }
}

void CIFin::CifFile::hierOut()
{
   _hierTree = _default->hierOut(_hierTree,NULL);

   CifStructure* local = _first;
   while (NULL != local)
   {
      if (!local->haveParent())
         _hierTree = local->hierOut(_hierTree,NULL);
      local = local->last();
   }
}

void CIFin::CifFile::convertPrep(const NameList& topCells, bool recursive)
{
   assert(NULL != _hierTree);
   _convList.clear();
   for (NameList::const_iterator CN = topCells.begin(); CN != topCells.end(); CN++)
   {
      CIFin::CifStructure *src_structure = const_cast<CIFin::CifStructure*>(getStructure(*CN));
      if (NULL != src_structure)
      {
         ForeignCellTree* root = _hierTree->GetMember(src_structure);
         if (recursive) preTraverseChildren(root);
         if (!src_structure->traversed())
         {
            _convList.push_back(src_structure);
            src_structure->set_traversed(true);
            // TODO conversion length?
         }
      }
      else
      {
         std::ostringstream ost; ost << "CIF import: ";
         ost << "Structure \""<< *CN << "\" not found in the CIF DB in memory.";
         tell_log(console::MT_WARNING,ost.str());
      }
   }
}

void CIFin::CifFile::getTopCells(NameList& top_cell_list) const
{
   assert(NULL != _hierTree);
   ForeignCellTree* root = _hierTree->GetFirstRoot(TARGETDB_LIB);
   if (root)
   {
      do
         top_cell_list.push_back(std::string(root->GetItem()->strctName()));
      while (NULL != (root = root->GetNextRoot(TARGETDB_LIB)));
   }
   //else
   //{
   //   // Considering possible to have an empty CIF file
   //}
}

void CIFin::CifFile::getAllCells(wxListBox& cellsBox) const
{
   CIFin::CifStructure* cifs = _first;
   while (cifs)
   {
      cellsBox.Append(wxString(cifs->strctName().c_str(), wxConvUTF8));
      cifs = cifs->last();
   }
   cellsBox.Append(wxString(_default->strctName().c_str(), wxConvUTF8));

}


//=============================================================================
CIFin::CifExportFile::CifExportFile(std::string fn, laydata::TdtCell* topcell,
   ExpLayMap* laymap, bool recur, bool verbose) :  DbExportFile(fn, topcell, recur),
      _laymap(laymap), _verbose(verbose), _lastcellnum(0)
{
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
   unsigned dbuu = (unsigned) (1.0/_DBU);
   // clean the error from the conversion (round to step 10)
   dbuu = (int4b) (rint((dbuu + (5)) / 10) * 10);
   unsigned cifu = 100000000;
   unsigned gcd = GCD(dbuu,cifu); // get the greatest common denominator
   unsigned bfact = dbuu / gcd;
   unsigned afact = cifu / gcd;
   tell_log(console::MT_INFO, message);
   registerCellWritten(name);
   if (_verbose)
      _file << std::endl << "Definition Start #" << _lastcellnum << "with a = " << afact << " and b = " << bfact<< ";"<< std::endl;
   else
      _file << std::endl << "DS " << _lastcellnum << " " << afact << " " << bfact <<";" << std::endl;
   _file << "   9 "<< name << ";"<< std::endl;
}

void CIFin::CifExportFile::definitionFinish()
{
   if (_verbose)
      _file << "Definition Finish;" << std::endl;
   else
      _file << "DF;" << std::endl;
}

void CIFin::CifExportFile::libraryStart(std::string libname, TpdTime& libtime, real DBU, real UU)
{
   _file << "(       TDT source : " << libname << ");" << std::endl;
   _file << "(    Last Modified : " << libtime() << ");" << std::endl;

   if (NULL == _topcell)
   {
      _file << "(         Top Cell :  - );" << std::endl;
   }
   else
   {
      _file << "(         Top Cell : " << _topcell->name() << ");" << std::endl;
   }
   _DBU = DBU;
   _UU = UU;
}

void CIFin::CifExportFile::libraryFinish()
{
   // nothing to do for CIF export
}

bool CIFin::CifExportFile::layerSpecification(LayerNumber layno)
{
   if (REF_LAY == layno) return true;
   if (_laymap->end() == _laymap->find(layno))
   {
      //std::stringstream message;
      //message << "   Layer " << layno <<" not found in the layer map and will not be converted";
      //tell_log(console::MT_INFO, message.str());
      return false;
   }
   if (_verbose)
      _file << "   Layer "<< (*_laymap)[layno] << " objects follow;" << std::endl;
   else
      _file << "L " << (*_laymap)[layno] << ";" << std::endl;
   return true;
}

void CIFin::CifExportFile::box(const int4b* const pdata)
{
   unsigned int length = abs(pdata[2] - pdata[0]);
   unsigned int width  = abs(pdata[3] - pdata[1]);
   TP center((pdata[0] + pdata[2]) / 2, (pdata[1] + pdata[3]) / 2);

   if (_verbose)
      _file << "      Box length = "<< length << " width = "<< width <<
            " and center = " << center.x() << "," << center.y() << ";" << std::endl;
   else
      _file << "      B"<< length << " " << width << " " << center.x() <<
            " " << center.y() << ";" << std::endl;
}

void CIFin::CifExportFile::polygon(const int4b* const pdata, unsigned psize)
{
   if (_verbose)
      _file <<"      Polygon with vertices";
   else
      _file <<"      P";
   for (unsigned i = 0; i < psize; i++)
      _file << " " << pdata[2*i] << " " << pdata[2*i+1];
   _file << ";"<< std::endl;
}

void CIFin::CifExportFile::wire(const int4b* const pdata, unsigned psize, unsigned width)
{
   // Convert data to point list
   PointVector plist;
   plist.reserve(psize);
   for (unsigned i = 0; i < psize; i++)
      plist.push_back(TP(pdata[2*i], pdata[2*i+1]));

   // Convert from wire type 0 (Toped natural) to type 2 (the closest to CIF natural type 1)
   if (pathConvert(plist, psize, width/2))
   {
      // Convert data back to array
      int4b* cPdata = DEBUG_NEW int4b[psize*2];
      unsigned index = 0;
      for (unsigned i = 0; i < psize; i++)
      {
         cPdata[index++] = plist[i].x();
         cPdata[index++] = plist[i].y();
      }

      // write ...
      if (_verbose)
         _file <<"      Wire width = " << width << "and points";
      else
         _file <<"      W" << width;
      for (unsigned i = 0; i < psize; i++)
         _file << " " << cPdata[2*i] << " " << cPdata[2*i+1];
      _file << ";"<< std::endl;

      delete [] cPdata;
   }
   else
   {
      // generate a polygon, the piece of wire we have in the DB can't be described as
      // a CIF wire (2 points with distance <= 2 * w)
      // Not much point generating a box here - the complications come if the wire
      // is not parallel to one of the axises.
      laydata::WireContour cwParr(pdata, psize, width);
      int4b* cPdata = DEBUG_NEW int4b[2 * cwParr.csize()];
      cwParr.getArrayData(cPdata);

      if (_verbose)
         _file <<"      Polygon with vertices";
      else
         _file <<"      P";
      for (unsigned i = 0; i < cwParr.csize(); i++)
         _file << " " << cPdata[2*i] << " " << cPdata[2*i+1];
      _file << ";"<< std::endl;

      delete [] cPdata;
   }
}

void CIFin::CifExportFile::text(const std::string& label, const CTM& trans)
{
   int loc;
   std::string labelr(label);
   while ((loc = labelr.find(' ')) >= 0 )
      labelr.replace(loc, 1, "_"); //@FIXME - this should be an option or ...?

   _file << "      94 "<< labelr << " "<< (int)trans.tx() << " " << (int)trans.ty() << ";" << std::endl;
}

void CIFin::CifExportFile::ref(const std::string& cellname, const CTM& tmatrix)
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

void CIFin::CifExportFile::aref(const std::string& name,
                                const CTM& translation, const laydata::ArrayProps& arrprops)
{
   for (int i = 0; i < arrprops.cols(); i++)
   {// start/stop rows
      for(int j = 0; j < arrprops.rows(); j++)
      { // start/stop columns
         // ... get the translation matrix ...
         CTM refCTM(arrprops.displ(i,j), 1, 0, false);
         refCTM *= translation;
         ref(name, refCTM);
      }
   }
}

bool CIFin::CifExportFile::pathConvert(PointVector& plist, unsigned numpoints, int4b adj )
{
   TP P1 = plist[0];
   // find the first neighboring point which is not equivalent to P1
   unsigned fnbr = 1;
   while ((fnbr < numpoints) && (P1 == plist[fnbr]))
      fnbr++;
   // The wire has effectively a single point and this is not a valid wire. This
   // condition should've been caught by the object validation checks.
   assert(fnbr != numpoints);
   TP P2 = plist[fnbr];

   double sdX = P2.x() - P1.x();
   double sdY = P2.y() - P1.y();
   // The sign - a bit funny way - described in layout canvas
   int sign = ((sdX * sdY) >= 0) ? -1 : 1;
   double length = sqrt(sdY*sdY + sdX*sdX);
   if ( (2 == numpoints) && ((int4b) rint(length) <= 2*adj) ) return false;
   assert(length);
   int4b y0 = (int4b) rint(P1.y() - sign*((adj*sdY)/length));
   int4b x0 = (int4b) rint(P1.x() - sign*((adj*sdX)/length));
//
   P2 = plist[numpoints-1];
   // find the first neighboring point which is not equivalent to P1
   fnbr = numpoints - 2;
   while ((P2 == plist[fnbr]) && (fnbr >= 0))
      fnbr--;
   P1 = plist[fnbr];

   sdX = P2.x() - P1.x();
   sdY = P2.y() - P1.y();
   sign = ((sdX * sdY) >= 0) ? -1 : 1;
   length = sqrt(sdY*sdY + sdX*sdX);
   int4b yn = (int4b) rint(P2.y() + sign*((adj*sdY)/length));
   int4b xn = (int4b) rint(P2.x() + sign*((adj*sdX)/length));

   plist[0].setX(x0);
   plist[0].setY(y0);
   plist[numpoints-1].setX(xn);
   plist[numpoints-1].setY(yn);

   return true;
}

CIFin::CifExportFile::~CifExportFile()
{
   _file << "End" << std::endl;
   _file.close();
   // don't delete _laymap - it's should be deleted where it had been created
}
