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
//        Created: Sat Jan 10 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TDT I/O and database access control
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "datacenter.h"
#include "../tpd_common/outbox.h"
#include "tedat.h"
#include "viewprop.h"
#include "ps_out.h"

// Global variables
DataCenter*               DATC = NULL;

//-----------------------------------------------------------------------------
// class gds2ted
//-----------------------------------------------------------------------------
GDSin::gds2ted::gds2ted(GDSin::GDSFile* src_lib, laydata::tdtdesign* dst_lib)
{
   _src_lib = src_lib;
   _dst_lib = dst_lib;
   coeff = dst_lib->UU() / src_lib->Get_LibUnits();
}

void GDSin::gds2ted::structure(const char* gname, bool recursive, bool overwrite)
{
   // source structure
   GDSin::GDSstructure *src_structure = _src_lib->GetStructure(gname);
   if (!src_structure)
   {
      std::string news = "GDS structure named \"";
      news += gname; news += "\" does not exists";
      tell_log(console::MT_ERROR,news);
      return;
   }
   // check that destination structure with this name exists
   laydata::tdtcell* dst_structure = _dst_lib->checkcell(gname);
   std::ostringstream ost; ost << "GDS import: ";
   if (NULL != dst_structure)
   {
      if (overwrite)
      {
         /*@TODO Erase the existing structure and convert*/
         ost << "Structure "<< gname << " should be overwritten, but cell erase is not implemened yet ...";
         tell_log(console::MT_WARNING,ost.str());
      }
   // Don't report this , except maybe in case of a verbose or similar option is introduced
   // On a large GDS file those messages are simply anoying
      else
      {
         ost << "Structure "<< gname << " already exists. Omitted";
         tell_log(console::MT_INFO,ost.str());
      }
   }
   else
   {
      // proceed with children first
      if (recursive)
      {
         GDSin::ChildStructure nextofkin = src_structure->children;
         typedef GDSin::ChildStructure::iterator CI;
         for (CI ci = nextofkin.begin(); ci != nextofkin.end(); ci++)
            structure((*ci)->Get_StrName(), recursive, overwrite);
      }
      ost << "Importing structure " << gname << "...";
      tell_log(console::MT_INFO,ost.str());
      // first create a new cell
      dst_structure = _dst_lib->addcell(gname);
      // now call the cell converter
      convert(src_structure, dst_structure);
   }
}

void GDSin::gds2ted::convert(GDSin::GDSstructure* src, laydata::tdtcell* dst)
{
   GDSin::GDSdata *wd = src->Get_Fdata();
   while( wd )
   {
      switch( wd->GetGDSDatatype() )
      {
//         case      gds_BOX: box(static_cast<GDSin::GDSbox*>(wd), dst);  break;
         case      gds_BOX:
         case gds_BOUNDARY: polygon(static_cast<GDSin::GDSpolygon*>(wd), dst);  break;
         case     gds_PATH: path(static_cast<GDSin::GDSpath*>(wd), dst);  break;
         case     gds_SREF: ref(static_cast<GDSin::GDSref*>(wd), dst);  break;
         case     gds_AREF: aref(static_cast<GDSin::GDSaref*>(wd), dst);  break;
         case     gds_TEXT: text(static_cast<GDSin::GDStext*>(wd), dst);  break;
                   default: {/*Error - unexpected type*/}
      }
      wd = wd->GetLast();
   }
   dst->resort();
//   dst->secure_layprop();
}

void GDSin::gds2ted::polygon(GDSin::GDSpolygon* wd, laydata::tdtcell* dst)
{
   laydata::tdtlayer* wl = static_cast<laydata::tdtlayer*>
                                          (dst->securelayer(wd->GetLayer()));
   pointlist &pl = wd->GetPlist();
   laydata::valid_poly check(pl);

   if (!check.valid())
   {
      std::ostringstream ost; ost << "Layer " << wd->GetLayer();
      ost << ": Polygon check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
   }
   else pl = check.get_validated() ;
   if (check.box())
   {
      wl->addbox(DEBUG_NEW TP(pl[0]), DEBUG_NEW TP(pl[2]),false);
   }
   else wl->addpoly(pl,false);
}

void GDSin::gds2ted::path(GDSin::GDSpath* wd, laydata::tdtcell* dst)
{
   laydata::tdtlayer* wl = static_cast<laydata::tdtlayer*>
                                          (dst->securelayer(wd->GetLayer()));
   pointlist &pl = wd->GetPlist();
   laydata::valid_wire check(pl, wd->Get_width());

   if (!check.valid())
   {
      std::ostringstream ost; ost << "Layer " << wd->GetLayer();
      ost << ": Wire check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
      return;
   }
   else pl = check.get_validated() ;
   /* @TODO !!! GDS path type here!!!! */
   wl->addwire(pl, wd->Get_width(),false);
}

void GDSin::gds2ted::ref(GDSin::GDSref* wd, laydata::tdtcell* dst)
{
   if (NULL != _dst_lib->checkcell(wd->GetStrname()))
   {
      laydata::refnamepair striter = _dst_lib->getcellnamepair(wd->GetStrname());
      // Absolute magnification, absolute angle should be reflected somehow!!!
      dst->addcellref(_dst_lib,
                   striter, 
                   CTM(wd->GetMagn_point(), 
                   wd->GetMagnification(),
                   wd->GetAngle(),
                   (0 != wd->GetReflection())),
                   false
      );
   }
   else
   {
      std::string news = "Referenced structure \"";
      news += wd->GetStrname(); news += "\" not found. Reference ignored";
      tell_log(console::MT_ERROR,news);
   }
   // How about structures defined, but not parsed yet????
}

void GDSin::gds2ted::aref(GDSin::GDSaref* wd, laydata::tdtcell* dst)
{
   if (NULL != _dst_lib->checkcell(wd->GetStrname()))
   {
      laydata::refnamepair striter = _dst_lib->getcellnamepair(wd->GetStrname());
      // Absolute magnification, absolute angle should be reflected somehow!!!

      laydata::ArrayProperties arrprops(wd->Get_Xstep(),wd->Get_Ystep(),
                                 static_cast<word>(wd->Get_colnum()),
                                 static_cast<word>(wd->Get_rownum()));
      dst->addcellaref(_dst_lib,
         striter, 
         CTM( wd->GetMagn_point(), 
              wd->GetMagnification(), 
              wd->GetAngle(),
              (0 != wd->GetReflection()) ),
         arrprops,
         false
      );
   }
   else
   {
      std::string news = "Referenced structure \"";
      news += wd->GetStrname(); news += "\" not found. Reference ignored";
      tell_log(console::MT_ERROR,news);
   }
   // How about structures defined, but not parsed yet????
}

void GDSin::gds2ted::text(GDSin::GDStext* wd, laydata::tdtcell* dst)
{
   laydata::tdtlayer* wl = static_cast<laydata::tdtlayer*>
                                       (dst->securelayer(wd->GetLayer()));
   // @FIXME absolute magnification, absolute angle should be reflected somehow!!!
   wl->addtext(wd->GetText(), 
               CTM(wd->GetMagn_point(), 
                   wd->GetMagnification() / (_dst_lib->UU() *  OPENGL_FONT_UNIT),
                   wd->GetAngle(),
                   (0 != wd->GetReflection())));
}

//-----------------------------------------------------------------------------
// class CIF2TED
//-----------------------------------------------------------------------------
CIFin::CIF2TED::CIF2TED(CIFin::CifFile* src_lib, laydata::tdtdesign* dst_lib,
      NMap* cif_layers) : _src_lib (src_lib), _dst_lib(dst_lib),
                                    _cif_layers(cif_layers)
{
}


void CIFin::CIF2TED::top_structure(std::string top_str, bool overwrite)
{
   assert(_src_lib->hiertree());
   CIFin::CifStructure *src_structure = _src_lib->getStructure(top_str);
   if (NULL != src_structure)
   {
      CIFin::CIFHierTree* root = _src_lib->hiertree()->GetMember(src_structure);
      child_structure(root, overwrite);
      convert_prep(root, overwrite);
      root = root->GetNextRoot(TARGETDB_LIB);
   }
   else
   {
      std::ostringstream ost; ost << "CIF import: ";
      ost << "Structure \""<< top_str << "\" not found in the CIF DB in memory.";
      tell_log(console::MT_WARNING,ost.str());
   }
   // Convert the top structure
   //   hCellBrowser->AddRoot(wxString((_src_lib->Get_libname()).c_str(), wxConvUTF8));

}

void CIFin::CIF2TED::child_structure(const CIFin::CIFHierTree* root, bool overwrite)
{
   const CIFin::CIFHierTree* Child= root->GetChild(TARGETDB_LIB);
   while (Child)
   {
      if ( !Child->GetItem()->traversed() )
      {
         // traverse children first
         child_structure(Child, overwrite);
         convert_prep(Child, overwrite);
      }
      Child = Child->GetBrother(TARGETDB_LIB);
   }
}

void CIFin::CIF2TED::convert_prep(const CIFin::CIFHierTree* item, bool overwrite)
{
   CIFin::CifStructure* src_structure = const_cast<CIFin::CifStructure*>(item->GetItem());
   std::string gname = src_structure->cellName();
         // check that destination structure with this name exists
   laydata::tdtcell* dst_structure = _dst_lib->checkcell(gname);
   std::ostringstream ost; ost << "CIF import: ";
   if (NULL != dst_structure)
   {
      if (overwrite)
      {
         /*@TODO Erase the existing structure and convert*/
         ost << "Structure "<< gname << " should be overwritten, but cell erase is not implemened yet ...";
         tell_log(console::MT_WARNING,ost.str());
      }
      else
      {
         ost << "Structure "<< gname << " already exists. Omitted";
         tell_log(console::MT_INFO,ost.str());
      }
   }
   else
   {
      ost << "Importing structure " << gname << "...";
      tell_log(console::MT_INFO,ost.str());
      // first create a new cell
      dst_structure = _dst_lib->addcell(gname);
      // finally call the cell converter
      convert(src_structure, dst_structure);
   }
   src_structure->set_traversed(true);
}


void CIFin::CIF2TED::convert(CIFin::CifStructure* src, laydata::tdtcell* dst)
{
   CIFin::CifLayer* swl = src->firstLayer();
   while( swl ) // loop trough the layers
   {
      if (_cif_layers->end() != _cif_layers->find(swl->name()))
      {
         laydata::tdtlayer* dwl =
               static_cast<laydata::tdtlayer*>(dst->securelayer((*_cif_layers)[swl->name()]));
         CIFin::CifData* wd = swl->firstData();
         while ( wd ) // loop trough data
         {
            switch (wd->dataType())
            {
               case cif_BOX     : box ( static_cast<CIFin::CifBox*     >(wd), dwl, swl->name() );break;
               case cif_POLY    : poly( static_cast<CIFin::CifPoly*    >(wd), dwl, swl->name() );break;
               case cif_WIRE    : wire( static_cast<CIFin::CifWire*    >(wd), dwl, swl->name() );break;
               case cif_LBL_LOC : lbll( static_cast<CIFin::CifLabelLoc*>(wd), dwl, swl->name() );break;
               case cif_LBL_SIG : lbls( static_cast<CIFin::CifLabelSig*>(wd), dwl, swl->name() );break;
               default    : assert(false);
            }
            wd = wd->last();
         }
      }
      else
      {
         std::ostringstream ost;
         ost << "CIF Layer name \"" << swl->name() << "\" is not defined in the function input parameter. Will be omitted";
         tell_log(console::MT_INFO,ost.str());
      }
      swl = swl->last();
   }
   CIFin::CifRef* swr = src->refirst();
   while ( swr )
   {
      ref(swr,dst);
      swr = swr->last();
   }
   dst->resort();
//   dst->secure_layprop();
}

void CIFin::CIF2TED::box ( CIFin::CifBox* wd, laydata::tdtlayer* wl, std::string layname)
{
   pointlist pl;
   pl.reserve(4);
   pl.push_back(TP( wd->center()->x() - wd->length() / 2, wd->center()->y() - wd->width() / 2 ));
   pl.push_back(TP( wd->center()->x() + wd->length() / 2, wd->center()->y() - wd->width() / 2 ));
   pl.push_back(TP( wd->center()->x() + wd->length() / 2, wd->center()->y() + wd->width() / 2 ));
   pl.push_back(TP( wd->center()->x() - wd->length() / 2, wd->center()->y() + wd->width() / 2 ));
   if (NULL != wd->direction())
   {
      CTM tmx;
      tmx.Translate(-wd->center()->x(),-wd->center()->x());
      tmx.Rotate(*(wd->direction()));
      tmx.Translate(-wd->center()->x(),-wd->center()->x());
      pl[0] *=  tmx;
      pl[1] *=  tmx;
      pl[2] *=  tmx;
      pl[3] *=  tmx;
   }

   laydata::valid_poly check(pl);

   assert(check.valid());
   pl = check.get_validated() ;
   if (check.box())
   {
      wl->addbox(DEBUG_NEW TP(pl[0]), DEBUG_NEW TP(pl[2]),false);
   }
   else wl->addpoly(pl,false);

}

void CIFin::CIF2TED::poly( CIFin::CifPoly* wd, laydata::tdtlayer* wl, std::string layname)
{
   pointlist pl = *(wd->poly());
   laydata::valid_poly check(pl);

   if (!check.valid())
   {
      std::ostringstream ost; ost << "Layer " << layname;
      ost << ": Polygon check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
   }
   else pl = check.get_validated() ;
   if (check.box())
   {
      wl->addbox(DEBUG_NEW TP(pl[0]), DEBUG_NEW TP(pl[2]),false);
   }
   else wl->addpoly(pl,false);
}

void CIFin::CIF2TED::wire( CIFin::CifWire* wd, laydata::tdtlayer* wl, std::string layname)
{
   pointlist pl = *(wd->poly());
   laydata::valid_wire check(pl, wd->width());

   if (!check.valid())
   {
      std::ostringstream ost; ost << "Layer " << layname;
      ost << ": Wire check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str());
   }
   else pl = check.get_validated() ;
   wl->addwire(pl, wd->width(),false);
}

void CIFin::CIF2TED::ref ( CIFin::CifRef* wd, laydata::tdtcell* dst)
{
   CifStructure* refd = _src_lib->getStructure(wd->cell());
   std::string cell_name = refd->cellName();
   if (NULL != _dst_lib->checkcell(cell_name))
   {
      laydata::refnamepair striter = _dst_lib->getcellnamepair(cell_name);
      // Absolute magnification, absolute angle should be reflected somehow!!!
      dst->addcellref(_dst_lib, striter, *(wd->location()), false);
   }
   else
   {
      std::string news = "Referenced structure \"";
      news += cell_name; news += "\" not found. Reference ignored";
      tell_log(console::MT_ERROR,news);
   }
}

void CIFin::CIF2TED::lbll( CIFin::CifLabelLoc*,laydata::tdtlayer*, std::string )
{
}

void CIFin::CIF2TED::lbls( CIFin::CifLabelSig*,laydata::tdtlayer*, std::string )
{
}

//-----------------------------------------------------------------------------
// class DataCenter
//-----------------------------------------------------------------------------
DataCenter::DataCenter(std::string localDir) 
{
   _localDir = localDir;
   _GDSDB = NULL; _CIFDB = NULL;//_TEDDB = NULL;
   // initializing the static cell hierarchy tree
   laydata::tdtlibrary::initHierTreePtr();
   _tedfilename = "unnamed";
   _curlay = 1;
   _drawruler = false;
}
   
DataCenter::~DataCenter() {
   laydata::tdtlibrary::clearEntireHierTree();
   if (NULL != _GDSDB) delete _GDSDB;
   if (NULL != _CIFDB) delete _CIFDB;
   // _TEDLIB will be cleared automatically (not a pointer)
}
bool DataCenter::TDTcheckread(const std::string filename,
    const TpdTime& timeCreated, const TpdTime& timeSaved, bool& start_ignoring)
{
   bool retval = false;
   start_ignoring = false;
   laydata::TEDfile tempin(filename.c_str(), TEDLIB());
   if (!tempin.status()) return retval;

   std::string news = "Project created: ";
   TpdTime timec(tempin.created()); news += timec();
   tell_log(console::MT_INFO,news);
   news = "Last updated: ";
   TpdTime timeu(tempin.lastUpdated()); news += timeu();
   tell_log(console::MT_INFO,news);
   // File created time stamp must match exactly, otherwise it means
   // that we're reading not exactly the same file that is requested
   if (timeCreated != timec)
   {
      news = "time stamp \"Project created \" doesn't match";
      tell_log(console::MT_ERROR,news);
   }
   if (timeu.stdCTime() < timeSaved.stdCTime())
   {
      news = "time stamp \"Last updated \" is too old.";
      tell_log(console::MT_ERROR,news);
   }
   else if (timeu.stdCTime() > timeSaved.stdCTime())
   {
      news = "time stamp \"Last updated \" is is newer than requested.";
      news +="Some of the following commands will be ignored";
      tell_log(console::MT_WARNING,news);
      //Start ignoring
      start_ignoring = true;
      retval = true;
   }
   tempin.closeF();
   return retval;
} 

bool DataCenter::TDTread(std::string filename)
{
   laydata::TEDfile tempin(filename.c_str(), &_TEDLIB);
   if (!tempin.status()) return false;

   try
   {
      tempin.read(TARGETDB_LIB);
   }
   catch (EXPTNreadTDT)
   {
      tempin.closeF();
      tempin.cleanup();
      return false;
   }
   tempin.closeF();
   _TEDLIB.deleteDB();//Erase existing data
   _tedfilename = filename;
   _neversaved = false;
   _TEDLIB.setDB(static_cast<laydata::tdtdesign*>(tempin.design()));
   _TEDLIB()->assign_properties(_properties);
   // Update Canvas scale
   _properties.setUU(_TEDLIB()->UU());
   return true;
}

int DataCenter::TDTloadlib(std::string filename)
{
   laydata::TEDfile tempin(filename.c_str(), &_TEDLIB);
   if (!tempin.status()) return -1;
   int libRef = _TEDLIB.getLastLibRefNo();
   try
   {
      tempin.read(libRef);
   }
   catch (EXPTNreadTDT)
   {
      tempin.closeF();
      tempin.cleanup();
      return -1;
   }
   tempin.closeF();
   _TEDLIB.addlibrary(tempin.design(), libRef);
   // Relink everything
   _TEDLIB.relink();
   return libRef;
}

bool DataCenter::TDTunloadlib(std::string libname)
{
   // Unhook the library from the list of known DB's, get a pointer to it
   laydata::tdtlibrary* tberased = _TEDLIB.removelibrary(libname);
   if ( NULL != tberased )
   {
      // Relink everything
      _TEDLIB.relink();
      // remove tberased cells from hierarchy tree 
      tberased->clearHierTree();
      // get the new hierarchy
      _TEDLIB.reextract_hierarchy();
      // after all above - remove the library (TODO! undo)
      delete tberased;
      return true;
   }
   else return false;
}


bool DataCenter::TDTcheckwrite(const TpdTime& timeCreated, const TpdTime& timeSaved, bool& stop_ignoring)
{
   std::string news;
   stop_ignoring = false;
   // File created time stamp must match exactly, otherwise it means
   // that we're saving not exactly the same file that is requested
   if (timeCreated.stdCTime() != _TEDLIB()->created())
   {
      news = "time stamp \"Project created \" doesn't match. File save aborted";
      tell_log(console::MT_ERROR,news);
      return false;
   }
   if (_TEDLIB()->lastUpdated() < timeSaved.stdCTime())
   {
      news = "Database in memory is older than the file. File save operation ignored.";
      tell_log(console::MT_WARNING,news);
      _neversaved = false;
      return false;
   }
   else if (_TEDLIB()->lastUpdated() > timeSaved.stdCTime())
      // database in memory is newer than the file - save has to go ahead
      // ignore on recovery has to stop
      stop_ignoring = true;
   else
   {
      // database in memory is exactly the same as the file. The save 
      // is going to be spared, ignore on recovery though has to stop
      stop_ignoring = true;
      return false;
   }
   return true;
}

bool DataCenter::TDTwrite(const char* filename)
{
   if (filename)  _tedfilename = filename;
   laydata::TEDfile tempin(_tedfilename, &_TEDLIB);
   _neversaved = false;
   return true;
}

void DataCenter::GDSexport(std::string& filename, bool x2048)
{
   std::string nfn;
   //Get actual time
   GDSin::GDSFile gdsex(filename, time(NULL));
   _TEDLIB()->GDSwrite(gdsex, NULL, true);
   if (x2048) gdsex.updateLastRecord();
   gdsex.closeFile();
}

void DataCenter::GDSexport(laydata::tdtcell* cell, bool recur, std::string& filename, bool x2048)
{
   std::string nfn;
   //Get actual time
   GDSin::GDSFile gdsex(filename, time(NULL));
   _TEDLIB()->GDSwrite(gdsex, cell, recur);
   if (x2048) gdsex.updateLastRecord();
   gdsex.closeFile();
}

bool DataCenter::GDSparse(std::string filename)
{
   bool status;
   if (lockGDS(false))
   {
      std::string news = "Removing existing GDS data from memory...";
      tell_log(console::MT_WARNING,news);
      GDSclose();
      unlockGDS();
   }
   // parse the GDS file - don't forget to lock the GDS mutex here!
   while (wxMUTEX_NO_ERROR != GDSLock.TryLock());
   _GDSDB = DEBUG_NEW GDSin::GDSFile(filename.c_str());
   status = _GDSDB->status();
   if (status)
   {
      // generate the hierarchy tree of cells
      _GDSDB->HierOut();
   }
   else
   {
      if (NULL != _GDSDB) 
      {
         delete _GDSDB;
         _GDSDB = NULL;
      }
   }
   unlockGDS();
   return status;
}

void DataCenter::importGDScell(const nameList& top_names, bool recur, bool over)
{
   if (NULL == lockGDS())
   {
      std::string news = "No GDS data in memory. Parse GDS file first";
      tell_log(console::MT_ERROR,news);
   }
   else
   {
      // Lock the DB here manually. Otherwise the cell browser is going mad
      GDSin::gds2ted converter(_GDSDB, _TEDLIB());
      for (nameList::const_iterator CN = top_names.begin(); CN != top_names.end(); CN++)
         converter.structure(CN->c_str(), recur, over);
      _TEDLIB()->modified = true;
      unlockGDS();
   }
}

void DataCenter::GDSclose()
{
   lockGDS();
      delete _GDSDB;
      _GDSDB = NULL;
   unlockGDS();
}

void DataCenter::CIFclose()
{
   lockCIF();
   delete _CIFDB;
   _CIFDB = NULL;
   unlockCIF();
}

CIFin::CifStatusType DataCenter::CIFparse(std::string filename)
{
   if (NULL != _CIFDB)
   {
      std::string news = "Removing existing CIF data from memory...";
      tell_log(console::MT_WARNING,news);
      CIFclose();
   }
   // parse the CIF file - don't forget to lock the CIF mutex here!
   while (wxMUTEX_NO_ERROR != CIFLock.TryLock());
   _CIFDB = DEBUG_NEW CIFin::CifFile(filename);

   CIFin::CifStatusType status = _CIFDB->status();
   if (CIFin::cfs_POK == status)
   {
      // generate the hierarchy tree of cells
      _CIFDB->hierPrep();
      _CIFDB->hierOut();
   }
   else
   {
      if (NULL != _CIFDB)
      {
         delete _CIFDB;
         _CIFDB = NULL;
      }
   }
   unlockCIF();
   return status;
}

bool DataCenter::CIFgetLay(nameList& cifLayers)
{
   if (NULL == _CIFDB) return false;
   else 
   {
      _CIFDB->collectLayers(cifLayers);
      return true;
   }
}

void DataCenter::CIFimport( const nameList& top_names, NMap* cifLayers, bool overwrite )
{
   // DB shold have been locked at this point (from the tell functions)
   if (NULL == lockCIF())
   {
      std::string news = "No CIF data in memory. Parse CIF file first";
      tell_log(console::MT_ERROR,news);
   }
   else
   {
      CIFin::CIF2TED converter(_CIFDB, _TEDLIB(), cifLayers);
      for (nameList::const_iterator CN = top_names.begin(); CN != top_names.end(); CN++)
         converter.top_structure(*CN, overwrite);
      _TEDLIB()->modified = true;
      unlockCIF();
   }
}

void DataCenter::PSexport(laydata::tdtcell* cell, std::string& filename)
{
   //Get actual time
   PSFile psex(filename);
   _properties.drawprop().PSwrite(psex);
   _TEDLIB()->PSwrite(psex, cell, _properties.drawprop());
//   gdsex.closeFile();
}

void DataCenter::newDesign(std::string name, time_t created) 
{
   if (_TEDLIB()) 
   {
      // Checks before closing(save?) available only when the command is launched 
      // via GUI(void TopedFrame::OnNewDesign(). If the command is typed directly 
      // on the command line, or parsed from file - no checks are executed.
      // In other words if we are already here we will destroy the current design
      // without much talking.
      // UNDO buffers will be reset as well in tellstdfunc::stdNEWDESIGN::execute()
      // but there is still a chance to restore everything - using the log file.
      _TEDLIB()->clearHierTree();
      _TEDLIB.deleteDB();
   }
   _TEDLIB.setDB(DEBUG_NEW laydata::tdtdesign(name, created, 0));
   _TEDLIB()->assign_properties(_properties);
   _tedfilename = _localDir + name + ".tdt";
   _neversaved = true;
   _properties.setUU(_TEDLIB()->UU());
}

laydata::tdtdesign*  DataCenter::lockDB(bool checkACTcell) 
{
   if (_TEDLIB()) 
   {
      if (checkACTcell) _TEDLIB()->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      return _TEDLIB();
   }
   else throw EXPTNactive_DB();
}

void DataCenter::unlockDB() 
{
   DBLock.Unlock();
}

GDSin::GDSFile* DataCenter::lockGDS(bool throwexception) 
{
   // Carefull HERE! When GDS is locked form the main thread
   // (GDS browser), then there is no catch pending -i.e.
   // throwing an exception will make the things worse
   // When it is locked from the parser command - then exception
   // is fine 
   if (_GDSDB) 
   {
      while (wxMUTEX_NO_ERROR != GDSLock.TryLock());
      return _GDSDB;
   }
   else {
      if (throwexception) throw EXPTNactive_GDS();
      else return NULL;
   }
}

void DataCenter::unlockGDS()
{
   GDSLock.Unlock();
}

CIFin::CifFile* DataCenter::lockCIF(bool throwexception)
{
   // Carefull HERE! When CIF is locked form the main thread
   // (CIF browser), then there is no catch pending -i.e.
   // throwing an exception will make the things worse
   // When it is locked from the parser command - then exception
   // is fine 
   if (_CIFDB)
   {
      while (wxMUTEX_NO_ERROR != CIFLock.TryLock());
      return _CIFDB;
   }
   else {
      if (throwexception) throw EXPTNactive_CIF();
      else return NULL;
   }
}

void DataCenter::unlockCIF()
{
   CIFLock.Unlock();
}

void DataCenter::mouseStart(int input_type, std::string name, const CTM trans,
                            int4b stepX, int4b stepY, word cols, word rows)
{
   if (console::op_line == input_type) return;
   if (_TEDLIB())
   {
      _TEDLIB()->check_active();
      switch (input_type)
      {
         case console::op_dbox:   _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdtbox()  ); break;
         case console::op_dpoly:  _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdtpoly()) ; break;
         case console::op_cbind:
         {
            assert ("" != name);
            laydata::refnamepair striter;
            CTM eqm;
            VERIFY(DATC->getCellNamePair(name, striter));
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdtcellref(striter, eqm) );
            break;
         }
         case console::op_abind:
         {
            assert ("" != name);
            assert(0 != cols);assert(0 != rows);assert(0 != stepX);assert(0 != stepY);
            laydata::refnamepair striter;
            CTM eqm;
            VERIFY(DATC->getCellNamePair(name, striter));
            laydata::ArrayProperties arrprops(stepX, stepY, cols, rows);
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdtcellaref(striter, eqm, arrprops) );
            break;
         }
         case console::op_tbind:
         {
            assert ("" != name);
            CTM eqm(trans);
            eqm.Scale(1/(UU()*OPENGL_FONT_UNIT), 1/(UU()*OPENGL_FONT_UNIT));
            _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdttext(name, eqm) );
            break;
         }
         case console::op_rotate: _TEDLIB()->set_tmpctm( trans );
         default:
         {
            if (0  < input_type)
               _TEDLIB()->set_tmpdata( DEBUG_NEW laydata::tdtwire(input_type) );
         }
      }
      initcmdlayer();
   }
   else throw EXPTNactive_DB();
}

void DataCenter::mousePoint(TP p)
{
   if ((console::op_line == currentop()) || _drawruler)
      _properties.mousePoint(p);
   if ((NULL != _TEDLIB()) && (console::op_cbind != currentop())
                           && (console::op_abind != currentop())
                           && (console::op_tbind != currentop()) 
                           && (console::op_line  != currentop()) )
      _TEDLIB()->mousePoint(p);
}

void DataCenter::mousePointCancel(TP& lp)
{
   if (console::op_line == currentop()) return;
   if (_TEDLIB())
      _TEDLIB()->mousePointCancel(lp);
}

void DataCenter::mouseStop()
{
   if (console::op_line == currentop())
      _properties.mouseStop();
   else if (_TEDLIB()) _TEDLIB()->mouseStop();
   else throw EXPTNactive_DB();
}

void DataCenter::mouseFlip()
{
   if (_TEDLIB()) _TEDLIB()->mouseFlip();
}

void DataCenter::mouseRotate()
{
   if (_TEDLIB()) _TEDLIB()->mouseRotate();
}

void DataCenter::openGL_draw(const CTM& layCTM) {
// Maybe we need another try/catch in the layoutcanvas ?   
   if (_TEDLIB()) {
//      _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
      _properties.drawGrid();
      _TEDLIB()->openGL_draw(_properties.drawprop());
      _properties.drawRulers(layCTM);
      DBLock.Unlock();
      PROPLock.Unlock();
   }
// 
//   else throw EXPTNactive_DB();      
}

void DataCenter::tmp_draw(const CTM& layCTM, TP base, TP newp) 
{
   if ((console::op_line == currentop()) || _drawruler)
   {
      // ruller
      while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
      _properties.tmp_draw(layCTM, base, newp);
      PROPLock.Unlock();
   }
   if ((console::op_line != currentop())  && (NULL !=_TEDLIB()))
   {
//      _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      _TEDLIB()->tmp_draw(_properties.drawprop(), base, newp);
      DBLock.Unlock();
   }
// 
//   else throw EXPTNactive_DB();      
}

const laydata::cellList& DataCenter::cells() {
   if (_TEDLIB()) return _TEDLIB()->cells();
   else throw EXPTNactive_DB();
};


bool DataCenter::addlayer(std::string name, word layno, std::string col,
                                       std::string fill, std::string sline)

{
   bool status;
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.addlayer(name, layno, col, fill, sline);
   PROPLock.Unlock();
   return status;
}

bool DataCenter::addlayer(std::string name, word layno)
{
   bool status;
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.addlayer(name, layno);
   PROPLock.Unlock();
   return status;
}

void DataCenter::addline(std::string name, std::string col, word pattern,
                                      byte patscale, byte width)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addline(name, col, pattern, patscale, width);
   PROPLock.Unlock();
}

void DataCenter::addcolor(std::string name, byte R, byte G, byte B, byte A)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addcolor(name, R, G, B, A);
   PROPLock.Unlock();
}

void DataCenter::addfill(std::string name, byte *ptrn)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addfill(name, ptrn);
   PROPLock.Unlock();
}

void DataCenter::hideLayer(word layno, bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.hideLayer(layno, hide);
   PROPLock.Unlock();
}

void DataCenter::lockLayer(word layno, bool lock)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.lockLayer(layno, lock);
   PROPLock.Unlock();
}

void DataCenter::setcellmarks_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.setcellmarks_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::settextmarks_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.settextmarks_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::setcellbox_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.setcellbox_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::settextbox_hidden(bool hide)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.settextbox_hidden(hide);
   PROPLock.Unlock();
}

void DataCenter::setGrid(byte No, real step, std::string colname)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.setGrid(No, step, colname);
   PROPLock.Unlock();
}

bool DataCenter::viewGrid(byte No, bool status)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   status = _properties.viewGrid(No, status);
   PROPLock.Unlock();
   return status;
}

void DataCenter::addRuler(TP& p1, TP& p2)
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.addRuler(p1, p2);
   PROPLock.Unlock();
}

void DataCenter::clearRulers()
{
   while (wxMUTEX_NO_ERROR != PROPLock.TryLock());
   _properties.clearRulers();
   PROPLock.Unlock();
}

bool DataCenter::getCellNamePair(std::string name, laydata::refnamepair& striter) 
{
   laydata::tdtdesign* ATDB = lockDB();
   if (ATDB->checkcell(name))
   {
      striter = ATDB->getcellnamepair(name);
      unlockDB();
      return true;
   }
   unlockDB();
   // search the cell in the libraries because it's not in the DB
   return _TEDLIB.getLibCellRNP(name, striter);
}

laydata::LibCellLists* DataCenter::getCells(int libID)
{
   laydata::LibCellLists* all_cells = DEBUG_NEW laydata::LibCellLists();
   if (libID == ALL_LIB)
   {
      if (NULL != _TEDLIB())
         all_cells->push_back(&(_TEDLIB()->cells()));
      for (int i = 1; i < _TEDLIB.getLastLibRefNo(); i++)
         all_cells->push_back(&(_TEDLIB.getLib(i)->cells()));
   }
   else if ( (libID == TARGETDB_LIB) && (NULL != _TEDLIB()) )
      all_cells->push_back(&(_TEDLIB()->cells()));
   else if (libID == UNDEFCELL_LIB)
      all_cells->push_back(&(_TEDLIB.getUndefinedCells()));
   else if (libID < _TEDLIB.getLastLibRefNo()) 
      all_cells->push_back(&(_TEDLIB.getLib(libID)->cells()));
   return all_cells;
}

void initDBLib(std::string localDir)
{
   DATC = DEBUG_NEW DataCenter(localDir);
}

