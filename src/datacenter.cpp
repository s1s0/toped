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
// ------------------------------------------------------------------------ =
//           $URL$
//  Creation date: Sat Jan 10 2004
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <sstream>
#include "datacenter.h"
#include "browsers.h"
#include "../tpd_DB/tedat.h"
#include "../tpd_common/outbox.h"
#include "../tpd_DB/viewprop.h"

extern   layprop::ViewProperties*   Properties;
extern   wxMutex                    DBLock;

//-----------------------------------------------------------------------------
// class gds2ted
//-----------------------------------------------------------------------------
GDSin::gds2ted::gds2ted(GDSin::GDSFile* src_lib, laydata::tdtdesign* dst_lib) {
   _src_lib = src_lib;
   _dst_lib = dst_lib;
   coeff = dst_lib->UU() / src_lib->Get_LibUnits();
}   

void GDSin::gds2ted::structure(const char* gname, bool recursive, bool overwrite) {
   // source structure
   GDSin::GDSstructure *src_structure = _src_lib->GetStructure(gname);
   if (!src_structure) {
      std::string news = "GDS structure named \"";
      news += gname; news += "\" does not exists";
      tell_log(console::MT_ERROR,news.c_str());
      return;
   }
   // proceed with children first
   if (recursive) {
      GDSin::ChildStructure nextofkin = src_structure->children;
      typedef GDSin::ChildStructure::iterator CI;
      for (CI ci = nextofkin.begin(); ci != nextofkin.end(); ci++)
         structure((*ci)->Get_StrName(), recursive, overwrite);
   }
   // check that destination structure with this name exists
   laydata::tdtcell* dst_structure = _dst_lib->checkcell(gname);
   std::ostringstream ost; ost << "GDS import: ";
   if (NULL != dst_structure) {
      if (overwrite) {
         /*SGREM! Erase the existing structure and convert*/
         ost << "Warning! Structure "<< gname << " should be overwritten, but cell erase is not implemened yet ...";
      }
   // Don't report this , except maybe in case of a verbose or similar option is introduced
   // On a large GDS file those messages are simply anoying
   //   else
   //      ost << "Structure "<< gname << " already exists. Omitted";
   }
   else {
      ost << "Importing structure " << gname << "...";
      tell_log(console::MT_INFO,ost.str().c_str());
      // first create a new cell
      dst_structure = _dst_lib->addcell(gname);
      // now call the cell converter
      convert(src_structure, dst_structure);
   }
   tell_log(console::MT_INFO,ost.str().c_str());
}

void GDSin::gds2ted::convert(GDSin::GDSstructure* src, laydata::tdtcell* dst) {
   GDSin::GDSdata *wd = src->Get_Fdata();
   while( wd ){
      switch( wd->GetGDSDatatype() ){
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

void GDSin::gds2ted::polygon(GDSin::GDSpolygon* wd, laydata::tdtcell* dst) {
   laydata::tdtlayer* wl = static_cast<laydata::tdtlayer*>
                                          (dst->securelayer(wd->GetLayer()));
   pointlist &pl = wd->GetPlist();
   laydata::valid_poly check(pl);

   if (!check.valid()) {
      std::ostringstream ost; ost << "Layer " << wd->GetLayer();
      ost << ": Polygon check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str().c_str());
   }   
   else pl = check.get_validated() ;
   if (check.box()) {
      wl->addbox(new TP(pl[0]), new TP(pl[2]),false);
   }
   else wl->addpoly(pl,false);
}

void GDSin::gds2ted::path(GDSin::GDSpath* wd, laydata::tdtcell* dst) {
   laydata::tdtlayer* wl = static_cast<laydata::tdtlayer*>
                                          (dst->securelayer(wd->GetLayer()));
   pointlist &pl = wd->GetPlist();
   laydata::valid_wire check(pl, wd->Get_width());

   if (!check.valid()) {
      std::ostringstream ost; ost << "Layer " << wd->GetLayer();
      ost << ": Wire check fails - " << check.failtype();
      tell_log(console::MT_ERROR, ost.str().c_str());
   }   
   else pl = check.get_validated() ;
/* SGREM !!! GDS path type here!!!! */
   wl->addwire(pl, wd->Get_width(),false);
}

void GDSin::gds2ted::ref(GDSin::GDSref* wd, laydata::tdtcell* dst) {
   if (NULL != _dst_lib->checkcell(wd->GetStrname())) {
      laydata::refnamepair striter = _dst_lib->getcellnamepair(wd->GetStrname());
      // Absolute magnification, absolute angle should be reflected somehow!!!
      dst->addcellref(_dst_lib,
                   striter, 
                   CTM(wd->GetMagn_point(), 
                   wd->GetMagnification(),
                   wd->GetAngle(),
                   wd->GetReflection()),
                   false
      );
   }
   else {
      std::string news = "Referenced structure \"";
      news += wd->GetStrname(); news += "\" not found. Reference ignored";
      tell_log(console::MT_ERROR,news.c_str());
   }
   // How about structures defined, but not parsed yet????
}

void GDSin::gds2ted::aref(GDSin::GDSaref* wd, laydata::tdtcell* dst) {
   if (NULL != _dst_lib->checkcell(wd->GetStrname())) {
      laydata::refnamepair striter = _dst_lib->getcellnamepair(wd->GetStrname());
      // Absolute magnification, absolute angle should be reflected somehow!!!
      dst->addcellaref(_dst_lib,
         striter, 
         CTM(wd->GetMagn_point(), wd->GetMagnification(), 
                                          wd->GetAngle(),wd->GetReflection()),
         wd->Get_Xstep(), wd->Get_Ystep(),
         static_cast<word>(wd->Get_colnum()), 
         static_cast<word>(wd->Get_rownum()),
         false
      );
   }
   else {
      std::string news = "Referenced structure \"";
      news += wd->GetStrname(); news += "\" not found. Reference ignored";
      tell_log(console::MT_ERROR,news.c_str());
   }
   // How about structures defined, but not parsed yet????
}

void GDSin::gds2ted::text(GDSin::GDStext* wd, laydata::tdtcell* dst) {
   laydata::tdtlayer* wl = static_cast<laydata::tdtlayer*>
                                       (dst->securelayer(wd->GetLayer()));
   // SGREM absolute magnification, absolute angle should be reflected somehow!!!
   wl->addtext(wd->GetText(), 
               CTM(wd->GetMagn_point(), 
                   wd->GetMagnification() / (_dst_lib->UU() *  OPENGL_FONT_UNIT),
                   wd->GetAngle(),wd->GetReflection()));
}

//-----------------------------------------------------------------------------
// class DataCenter
//-----------------------------------------------------------------------------
DataCenter::DataCenter() {
   _GDSDB = NULL; _TEDDB = NULL;
   _tedfilename = "unnamed";
   _tedtimestamp = time(NULL);
}
   
DataCenter::~DataCenter() {
   if (NULL != _GDSDB) delete _GDSDB;
   if (NULL != _TEDDB) delete _TEDDB;
}

bool DataCenter::TDTread(std::string filename) {
   laydata::TEDfile tempin(filename.c_str());
   if (tempin.status()) {
      delete _TEDDB;//Erase existing data
      _tedfilename = filename;
      _neversaved = false;
      _tedtimestamp = tempin.timestamp();
      _TEDDB = tempin.design();
      _TEDDB->btreeAddMember    = &browsers::treeAddMember;
      _TEDDB->btreeRemoveMember = &browsers::treeRemoveMember;
      // get the hierarchy
      browsers::addTDTtab(_TEDDB->name(), _TEDDB->hiertree());
      // Update Canvas scale
      Properties->setUU(_TEDDB->UU());
      return true;
   }
   else {
      // don't clear the tempin.design. It is already done in tempin read constructor
      return false;
   }   
}

void DataCenter::TDTwrite(const char* filename) {
   std::string nfn;
   if (filename)  _tedfilename = filename;
   laydata::TEDfile tempin(_TEDDB, _tedfilename);
   _tedtimestamp = tempin.timestamp();
   _neversaved = false;
}

void DataCenter::GDSexport(std::string& filename)
{
   std::string nfn;
   //Get actual time
   _tedtimestamp = time(NULL);
   GDSin::GDSFile gdsex(filename, _tedtimestamp);
   _TEDDB->GDSwrite(gdsex, NULL, true);
   gdsex.updateLastRecord();gdsex.closeFile();
}

void DataCenter::GDSexport(laydata::tdtcell* cell, bool recur, std::string& filename)
{
   std::string nfn;
   //Get actual time
   _tedtimestamp = time(NULL);
   GDSin::GDSFile gdsex(filename, _tedtimestamp);
   _TEDDB->GDSwrite(gdsex, cell, recur);
   gdsex.updateLastRecord();gdsex.closeFile();
}

void DataCenter::GDSparse(std::string filename, std::list<std::string>& topcells) {
    // parse the GDS file
    _GDSDB = new GDSin::GDSFile(filename.c_str());
    // Build the hierarchy tree of the GDS DB and add a browser TAB
    browsers::addGDStab(_GDSDB->Get_libname(), _GDSDB->HierOut());

    GDSin::GDSHierTree* root = _GDSDB->HierOut()->GetFirstRoot();
    do {
       topcells.push_back(std::string(root->GetItem()->Get_StrName()));
    } while (NULL != (root = root->GetNextRoot()));
}

void DataCenter::importGDScell(const char* name, bool recur, bool over) {
   if (_GDSDB) {
      GDSin::gds2ted converter(_GDSDB, _TEDDB);
      converter.structure(name, recur, over);
      _TEDDB->modified = true;
      browsers::addTDTtab(_TEDDB->name(), _TEDDB->hiertree());
   }
   else throw EXPTNactive_GDS();
}

void DataCenter::reportGDSlay(const char* name) {
   if (_GDSDB) {
      GDSin::GDSstructure *src_structure = _GDSDB->GetStructure(name);
      std::ostringstream ost; 
      if (!src_structure) {
         ost << "GDS structure named \"" << name << "\" does not exists";
         tell_log(console::MT_ERROR,ost.str().c_str());
         return;
      }
      ost << "GDS layers found in \"" << name <<"\": ";
      for(int i = 0 ; i < GDS_MAX_LAYER ; i++)
         if (src_structure->Get_Allay(i)) ost << i << " ";
      tell_log(console::MT_INFO,ost.str().c_str());
   }
   else throw EXPTNactive_GDS();
}

void DataCenter::GDSclose() {
   browsers::clearGDStab();
   if (NULL != _GDSDB) delete _GDSDB;
   _GDSDB = NULL;
}

void DataCenter::newDesign(std::string name) {
   if (_TEDDB) {
      // Checks before closing(save?) available only when the command is launched 
      // via GUI(void TopedFrame::OnNewDesign(). If the command is typed directly 
      // on the command line, or parsed from file - no checks are executed.
      // In other words if we are already here we will destroy the current design
      // without much talking.
      // UNDO buffers will be reset as well in tellstdfunc::stdNEWDESIGN::execute()
      // but there is still a chance to restore everything - using the log file.
      delete _TEDDB;
   }   
   _TEDDB = new laydata::tdtdesign(name);
   _TEDDB->btreeAddMember    = &browsers::treeAddMember;
   _TEDDB->btreeRemoveMember = &browsers::treeRemoveMember;
    
   browsers::addTDTtab(name, _TEDDB->hiertree());
   _tedfilename = name + ".tdt";
   _neversaved = true;
   Properties->setUU(_TEDDB->UU());
   
}

laydata::tdtdesign*  DataCenter::lockDB(bool checkACTcell) {
   if (_TEDDB) {
      if (checkACTcell) _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      return _TEDDB;
   }
   else throw EXPTNactive_DB();      
}

void DataCenter::unlockDB() {
   DBLock.Unlock();
}

unsigned int DataCenter::numselected() const {
   if (_TEDDB) return _TEDDB->numselected();
   else return 0;
}

void DataCenter::mouseStart(int input_type) {
   if (_TEDDB) {
      _TEDDB->check_active();
      _TEDDB->mouseStart(input_type);
   }
   else throw EXPTNactive_DB();
}

void DataCenter::mousePoint(TP p) {
   if (_TEDDB) _TEDDB->mousePoint(p);
}

void DataCenter::mousePointCancel(TP& lp) {
   if (_TEDDB)  _TEDDB->mousePointCancel(lp);
}

void DataCenter::mouseStop() {
   if (_TEDDB) _TEDDB->mouseStop();
   else throw EXPTNactive_DB();
}

void DataCenter::openGL_draw(const layprop::DrawProperties& drawprop) {
// Maybe we need another try/catch in the layoutcanvas ?   
   if (_TEDDB) {
//      _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      _TEDDB->openGL_draw(drawprop);
      DBLock.Unlock();
   }
// 
//   else throw EXPTNactive_DB();      
}

void DataCenter::tmp_draw(const layprop::DrawProperties& drawprop,
                              TP base, TP newp) {
   if (_TEDDB) {
//      _TEDDB->check_active();
      while (wxMUTEX_NO_ERROR != DBLock.TryLock());
      _TEDDB->tmp_draw(drawprop, base, newp);
      DBLock.Unlock();
   }
// 
//   else throw EXPTNactive_DB();      
}

const laydata::cellList& DataCenter::cells() {
   if (_TEDDB) return _TEDDB->cells();
   else throw EXPTNactive_DB();
};


GDSin::GDSstructure* DataCenter::GDSstructures() {
   if (_GDSDB) return _GDSDB->Get_structures();
   else throw EXPTNactive_GDS();
}
