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
//  Creation date: Sep 14 1999
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

#include <cmath>
#include <sstream>
#include <string>
#include "gds_io.h"
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"

static GDSin::GDSFile*        InFile    = NULL;

//==============================================================================
GDSin::GDSrecord::GDSrecord(FILE* Gf, word rl, byte rt, byte dt) {
   reclen = rl;rectype = rt;datatype = dt;
   if (rl) {
      record = new byte[reclen];
      numread = fread(record,1,reclen,Gf);
      isvalid = (numread == reclen) ? true : false;
   }
   else {record = NULL;numread = 0;isvalid = true;}
}

bool GDSin::GDSrecord::Ret_Data(void* var, word curnum, byte len) {
   byte      *rlb;   
   char      *rlc;
   switch (datatype) {
      case gdsDT_NODATA:// no data present
         var = NULL;return false;
      case gdsDT_BIT:// bit array
         rlb = (byte*)var; //assign pointer
         switch (len) {
            case 32:
               rlb[3] = record[0];
               rlb[2] = record[1];
               rlb[1] = record[2];
               rlb[0] = record[3];
               break;
            case 16:
               rlb[1] = record[0];
               rlb[0] = record[1];
               break;
            case 8:
               rlb[0] = record[0];
               break;
            default:
               var = NULL;break;
         }
         break;
      case gdsDT_INT2B://2-byte signed integer
         rlb = (byte*)var;   //assign pointer
         rlb[0] = record[curnum+1];
         rlb[1] = record[curnum+0];
         break;
      case gdsDT_INT4B: // 4-byte signed integer
         rlb = (byte*)var;//assign pointer
         rlb[0] = record[curnum+3];
         rlb[1] = record[curnum+2];
         rlb[2] = record[curnum+1];
         rlb[3] = record[curnum+0];
         break;
      case gdsDT_REAL4B:{// 4-bit real 
         // SGREM!!! not used and never checked !!!!
         _sg_int8 sign = (0x80 & record[curnum])? -1:1; //sign
         byte exponent = 0x7f & record[curnum]; // exponent
         _sg_int32 mantissa = 0; // mantissa
         byte* mant = (byte*)&mantissa;// take the memory possition
         mant[3] = 0x0;
         for (int i = 0; i < 3; i++)
            mant[i] = record[curnum+3-i];
         double *rld = (double*)var; // assign pointer
         *rld = sign*(mantissa/pow(2,24)) * pow(16, exponent-64);
         break;
      }
      case gdsDT_REAL8B:{// 8-bit real
         byte i;
         _sg_int8 sign = (0x80 & record[curnum])? -1:1; //sign
         byte exponent = 0x7f & record[curnum]; // exponent
         _dbl_word mantissa0 = 0; // mantissa LSByte
         _sg_int32 mantissa1 = 0; // mantissa MSByte
         byte* mant = (byte*)&mantissa0;// take the memory possition
         for (i = 0; i < 4; i++)
            mant[i] = record[curnum+7-i];
         mant = (byte*)&mantissa1;
         for (i = 0; i < 3; i++)
            mant[i] = record[curnum+3-i];
         mant[3] = 0x0;
         double *rld = (double*)var; // assign pointer

         *rld = sign*((mantissa1*pow(2,32)+mantissa0)/pow(2,56)) *
            pow(16,exponent-64);
         break;
      }
      case gdsDT_ASCII:// String
         rlc = (char*)var;
         if (len > 0){
            for (word i = 0; i < len; rlc[i] = record[curnum*len+i++]);
            rlc[len] = 0x0;}
         else{
            for (word i = 0; i < reclen; rlc[i] = record[i++]);
            rlc[reclen] = 0x0;}
         break;
   }
   return true;
}

GDSin::GDSrecord::~GDSrecord() {
   delete[] record;
}

//==============================================================================
// class GDSFile
//==============================================================================
GDSin::GDSFile::GDSFile(const char* fn) {
   InFile = this;HierTree = NULL;
   GDSIIwarnings = GDSIIerrors = 0;
   filename = new char[strlen(fn)+1];//initializing
   strcpy(filename,fn);
   file_pos = 0;
//   prgrs_pos = 0;
   library = NULL;
//   prgrs = progrind;
   AddLog('B',fn);
   if (!(GDSfh = fopen(fn,"rb"))) {// open the input file
      char wc[255];
      sprintf(wc,"File %s can NOT be opened",fn);
      AddLog('E',wc);
      return; 
   }
//   file_length = _filelength(GDSfh->_file);
   // The size of GDSII files is originaly multiple of 2048. This is 
   // coming from the acient years when this format was supposed to be written 
   // on the magnetic tapes. In order to keep the tradition it's a good idea 
   // to check the file size and to issue a warning if it is not multiple on 2048.
//   div_t divi = div(file_length,2048);
//   if (divi.rem != 0) AddLog('W',"File size is not multiple of 2048");
//   prgrs->SetRange32(0,file_length);// initializes progress indicator control
//   prgrs->SetStep(1);
   GDSrecord* wr = NULL;
   AddLog('O',"Reading...");

   do {// start reading
      wr = GetNextRecord();
      if (wr)
         switch (wr->Get_rectype())   {
            case gds_HEADER:      wr->Ret_Data(&StreamVersion);
               delete wr;break;
            case gds_BGNLIB:      GetTimes(wr);
               delete wr;break;
            case gds_LIBDIRSIZE:   wr->Ret_Data(&libdirsize);
               delete wr;break;
            case gds_SRFNAME:      wr->Ret_Data(&srfname);   
               delete wr;break;
            case gds_LIBSECUR:// I don't need this info. Does anybody need it?
               delete wr;break;
            case gds_LIBNAME:   // down in the hierarchy. 
               //Start reading the library structure
               library = new GDSlibrary(this, wr);
               //build the hierarchy tree
               library->SetHierarchy();
               fclose(GDSfh);// close the input stream
//               prgrs_pos = file_length;
//               prgrs->SetPos(prgrs_pos); // fullfill progress indicator
               AddLog('O',"Done");
               delete wr; return; // go out
            default:   //parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete wr;return;
         }
      else   {AddLog('E',"Unexpected end of file");return;}
   }   
   while (true);
}

void GDSin::GDSFile::GetTimes(GDSrecord *wr) {
   word cw;
   for (int i = 0; i<wr->Get_reclen()/2; i++) {
      wr->Ret_Data(&cw,2*i);
      switch (i) {
         case 0 :t_modif.Year    = cw;break;
         case 1 :t_modif.Month = cw;break;
         case 2 :t_modif.Day    = cw;break;
         case 3 :t_modif.Hour    = cw;break;
         case 4 :t_modif.Min    = cw;break;
         case 5 :t_modif.Sec    = cw;break;
         case 6 :t_access.Year  = cw;break;
         case 7 :t_access.Month = cw;break;
         case 8 :t_access.Day     = cw;break;
         case 9 :t_access.Hour  = cw;break;
         case 10:t_access.Min     = cw;break;
         case 11:t_access.Sec     = cw;break;
      }
   }
}

GDSin::GDSrecord* GDSin::GDSFile::GetNextRecord() {
   char recheader[4]; // record header
   unsigned numread = fread(&recheader,1,4,GDSfh);// read record header
   if (numread != 4)   
      return NULL;// error during read in
   char rl[2];
   rl[0] = recheader[1];
   rl[1] = recheader[0];
   word reclen = *(word*)rl - 4; // record lenght
   GDSrecord* retrec = new GDSrecord(GDSfh, reclen, recheader[2],recheader[3]);
   file_pos += reclen+4;    // update file position
//   if (2048 < (file_pos - prgrs_pos))
//   {
//      prgrs_pos = file_pos;
//      prgrs->SetPos(prgrs_pos); // update progress indicator
//   }
   if (retrec->isvalid) return retrec;
   else return NULL;// error during read in
}

GDSin::GDSstructure* GDSin::GDSFile::GetStructure(const char* selection) {
   GDSstructure* Wstrct = library->Get_Fstruct();
   while (Wstrct)   {
      if (!strcmp(Wstrct->Get_StrName(),selection)) return Wstrct;
      Wstrct = Wstrct->GetLast();
   }
   return NULL;
}

double GDSin::GDSFile::Get_LibUnits() {
   return library->Get_DBU()/library->Get_UU();
}

double GDSin::GDSFile::Get_UserUnits() {
   return library->Get_UU();
}

void GDSin::GDSFile::GetHierTree() {
//   GDSstructure* boza = HierTree->GetNextRoot();
   GDSHierTree* root = HierTree->GetFirstRoot();
   std::string tab = "   ";
   while (root){
      PrintChildren(root, &tab);
      root = root->GetNextRoot();
   }   
}

GDSin::GDSFile::~GDSFile() {
   delete[] filename;   delete library;
}

//==============================================================================
// class GDSlibrary
//==============================================================================
GDSin::GDSlibrary::GDSlibrary(GDSFile* cf, GDSrecord* cr) {
   int i;
   cr->Ret_Data(&libname);//Get library name
   // init section
   maxver = 3;   Fstruct = NULL;
   for (i=0; i<4;fonts[i++] = NULL);
   do   {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype())   {
            case gds_FORMAT://SGREM skipped record !!!
               AddLog('U',"FORMAT");
               delete cr;break;
            case gds_MASK://SGREM skipped record !!!
               AddLog('U',"MASK");
               delete cr;break;
            case gds_ENDMASKS://SGREM skipped record !!!
               AddLog('U',"ENDMASKS");
               delete cr;break;
            case gds_REFLIBS://SGREM skipped record !!!
               AddLog('U',"REFLIBS");
               delete cr;break;
            case gds_ATTRTABLE://SGREM skipped record !!!
               AddLog('U',"ATTRTABLE");
               delete cr;break;
            case gds_FONTS:// Read fonts
               for(i = 0; i < 4; i++)  {
                  fonts[i] = new char[45];
                  cr->Ret_Data(fonts[i],i,44);
               }
               delete cr;break;
            case gds_GENERATION:   cr->Ret_Data(&maxver);
               delete cr;break;
            case gds_UNITS:   
               cr->Ret_Data(&UU,0,8); // database units in one user unit
               cr->Ret_Data(&DBU,8,8); // database unit in meters
               delete cr;break;
            case gds_BGNSTR:   
               Fstruct = new GDSstructure(cf, Fstruct);
               AddLog('S',Fstruct->Get_StrName());
               delete cr;break;
            case gds_ENDLIB://end of library, exit form the procedure
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }   
   while (true);
}

void GDSin::GDSlibrary::SetHierarchy() {
   GDSstructure* ws = Fstruct;
   while (ws) {//for every structure
      GDSdata* wd = ws->Get_Fdata();
      while (wd) { //for every GDSdata of type SREF or AREF
      //put a pointer to GDSstructure
         word dt = wd->GetGDSDatatype();
         if ((gds_SREF == dt) || (gds_AREF == dt))
         {//means that GDSdata type is AREF or SREF 
            char* strname = ((GDSref*) wd)->GetStrname();
            GDSstructure* ws2 = Fstruct;
            while ((ws2) && (strcmp(strname,ws2->Get_StrName())))
               ws2 = ws2->GetLast();
            ((GDSref*) wd)->SetStructure(ws2);
            if (ws2)
            {
               ws->RegisterStructure(ws2);
               ws2->HaveParent = true;
            }
            else
            {//structure is referenced but not defined!
               char wstr[256];
               sprintf(wstr," Structure %s is referenced, but not defined!",
                       ((GDSref*)wd)->GetStrname());
               AddLog('W',wstr);
               //SGREM probably is a good idea to add default
               //GDSstructure here. Then this structure can be
               //visualized in the Hierarchy window as disabled
            }         
         }
         wd = wd->GetLast();
      }
      ws = ws->GetLast();
   }
}

GDSin::GDSHierTree* GDSin::GDSlibrary::HierOut() {
   GDSstructure* ws = Fstruct;
   GDSHierTree* Htree = NULL;
   while (ws){
      if (!ws->HaveParent)  Htree = ws->HierOut(Htree,NULL);
      ws = ws->GetLast();
   }
   return Htree;
}


GDSin::GDSlibrary::~GDSlibrary() {
   for(int i = 0; i < 4; i++)
      if (fonts[i]) delete fonts[i];
   GDSstructure* Wstruct;
   while (Fstruct){
      Wstruct = Fstruct->GetLast();
      delete Fstruct;
      Fstruct = Wstruct;
   }
}

//==============================================================================
// class GDSstructure
//==============================================================================
GDSin::GDSstructure::GDSstructure(GDSFile *cf, GDSstructure* lst) {   
   int i;
   //initializing
   last = lst; Fdata = NULL;
   HaveParent = false;
   GDSrecord* cr = NULL;
   for (i = 0; i < GDS_MAX_LAYER; Compbylay[i++] = NULL);
   do   { //start reading 
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype()) {
            case gds_NODE://SGREM skipped record !!!
               AddLog('U',"NODE");
               delete cr;break;
            case gds_PROPATTR://SGREM skipped record !!!
               AddLog('U',"NODE");
               delete cr;break;
            case gds_STRCLASS://SGREM skipped record !!!
               AddLog('U',"STRCLASS");// CADANCE internal use only               
               delete cr;break;
            case gds_STRNAME:
               if (cr->Get_reclen() > 64)
                  strname[0] = 0x0;
               else cr->Ret_Data(&strname);
               delete cr;break;
            case gds_BOX: 
               Fdata = new GDSbox(cf, Fdata);
               Compbylay[Fdata->GetLayer()] = //put in layer sequence
                  Fdata->PutLaymark(Compbylay[Fdata->GetLayer()]);
               delete cr;break;
            case gds_BOUNDARY: 
               Fdata = new GDSpolygon(cf, Fdata);
               Compbylay[Fdata->GetLayer()] = //put in layer sequence
                  Fdata->PutLaymark(Compbylay[Fdata->GetLayer()]);
               delete cr;break;
            case gds_PATH: 
               Fdata = new GDSpath(cf, Fdata);
               Compbylay[Fdata->GetLayer()] = //put in layer sequence
                  Fdata->PutLaymark(Compbylay[Fdata->GetLayer()]);
               delete cr;break;
            case gds_TEXT:   
               Fdata = new GDStext(cf,Fdata);
               Compbylay[Fdata->GetLayer()] = //put in layer sequence
                  Fdata->PutLaymark(Compbylay[Fdata->GetLayer()]);
               delete cr;break;
            case gds_SREF:   
               Fdata = new GDSref(cf, Fdata);
               delete cr;break;
            case gds_AREF: 
               Fdata = new GDSaref(cf, Fdata);
               delete cr;break;
            case gds_ENDSTR:// end of structure, exit point
               for(i = 0;i < GDS_MAX_LAYER;i++)//collect all used layers 
                  NULL == Compbylay[i] ? Allay[i] = false:Allay[i] = true;
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else
      { AddLog('E',"Unexpected end of file");return;}
   }
   while (true);
}

bool GDSin::GDSstructure::RegisterStructure(GDSstructure* ws) {
   for (unsigned i=0; i < children.size(); i++) {
      if (NULL == children[i]) continue;
      else if (!strcmp(children[i]->Get_StrName(),ws->Get_StrName()))
         return false;
   }
   children.push_back(ws);
   return true;
}
   
GDSin::GDSHierTree* GDSin::GDSstructure::HierOut(GDSHierTree* Htree, GDSstructure* parent) {
   // collecting hierarchical information
   Htree = new GDSHierTree(this, parent, Htree);
   for (unsigned i = 0; i < children.size(); i++)
      if (NULL == children[i]) continue;
      else {
         Htree = children[i]->HierOut(Htree,this);
         // Collect all used layers here and down in hierarchy
         for(int j = 0 ; j < GDS_MAX_LAYER ; j++)
            Allay[j] |= children[i]->Get_Allay(j);
            //   if (children.GetAt(i)->Get_Allay(j)) Allay[j] = true;//same as above
      }
   return Htree;
}

GDSin::GDSstructure::~GDSstructure() {
   GDSdata* Wdata = Fdata;
   while (Fdata) {
      Wdata = Fdata->GetLast();
      delete Fdata;
      Fdata = Wdata;
   }
}

//==============================================================================
// class GDSdata
//==============================================================================
GDSin::GDSdata::GDSdata(GDSdata* lst) {
   elflags = 0; plex = 0;
   last = lst;layer = -1; singletype = -1;
   lastlay = NULL;
}

void GDSin::GDSdata::ReadPLEX(GDSrecord *cr) {
   cr->Ret_Data(&elflags,0,16);//get two bytes bit-array
}

void GDSin::GDSdata::ReadELFLAGS(GDSrecord *cr) {
   cr->Ret_Data(&plex);//get two bytes bit-array
}

//==============================================================================
// class GDSbox
//==============================================================================
GDSin::GDSbox::GDSbox(GDSFile* cf, GDSdata *lst):GDSdata(lst) {
   //SGREM this function is never checked !!!
   GDSrecord* cr = NULL;
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
      switch (cr->Get_rectype())
      {
         case gds_ELFLAGS:
            delete cr; break;
         case gds_PLEX:
            delete cr; break;
         case gds_LAYER:
            delete cr; break;

         case gds_BOXTYPE:
            delete cr; break;
         case gds_XY:
            delete cr; break;
         case gds_ENDEL:
            delete cr; break;
         default:{
            //parse error - not expected record type
            AddLog('E',"Wrong record type in the current context");
            delete cr;return;
         }
      }
   }
   while (cr->Get_rectype() != gds_ENDEL);
}

// laydata::tdtdata* GDSin::GDSbox::toTED() {
//    /*SGREM Shall we manifest something like gds type never checked ?! */
//    return NULL;
// }

//==============================================================================
// class GDSpolygon
//==============================================================================
GDSin::GDSpolygon::GDSpolygon(GDSFile* cf, GDSdata *lst):GDSdata(lst) {
   word i;
   GDSrecord* cr = NULL;
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype()) {
            case gds_ELFLAGS: ReadELFLAGS(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:   ReadPLEX(cr);// seems that it's not used
               delete cr;break;
            case gds_LAYER: cr->Ret_Data(&layer);
               delete cr;break;
            case gds_DATATYPE: cr->Ret_Data(&singletype);
               delete cr;break;
            case gds_XY: numpoints = (cr->Get_reclen())/8 - 1;
               // one point less because fist and last point coincide
               _plist.reserve(numpoints);
               for(i = 0; i < numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr; break;
            case gds_ENDEL://end of element, exit point
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }   
   while (true);
}

// laydata::tdtdata* GDSin::GDSpolygon::toTED() {
//    return NULL;
// }

//==============================================================================
// class GDSpath
//==============================================================================
GDSin::GDSpath::GDSpath(GDSFile* cf, GDSdata *lst):GDSdata(lst) {
   word i;
   pathtype = 0;bgnextn = 0; endextn = 0;width = 0;
   GDSrecord* cr = NULL;
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype())   {
            case gds_ELFLAGS: ReadELFLAGS(cr);// seems that's not used               
               delete cr;break;
            case gds_PLEX:   ReadPLEX(cr);// seems that's not used
               delete cr;break;
            case gds_LAYER: cr->Ret_Data(&layer);
               delete cr;break;
            case gds_DATATYPE: cr->Ret_Data(&singletype);
               delete cr;break;
            case gds_PATHTYPE: cr->Ret_Data(&pathtype);
               delete cr;break;
            case gds_WIDTH: cr->Ret_Data(&width);
               delete cr;break;
            case gds_BGNEXTN:   cr->Ret_Data(&bgnextn);
               delete cr;break;
            case gds_ENDEXTN:   cr->Ret_Data(&endextn);
               delete cr;break;
            case gds_XY:numpoints = (cr->Get_reclen())/8;
               _plist.reserve(numpoints);
               for(i = 0; i < numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               if (3 == pathtype) {
                  AddLog('W',"GDS Pathtype 3 digitized. Will be converted to Pathtype 2");
                  convert22(width/2, width/2);
               }   
               else if (4 == pathtype) {
                  AddLog('W',"GDS Pathtype 4 digitized. Will be converted to Pathtype 2");
                  convert22(bgnextn, endextn);
               }   
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }   
   while (cr->Get_rectype() != gds_ENDEL);
}

void GDSin::GDSpath::convert22(int4b begext, int4b endext) {
   TP P1 = _plist[0];
   TP P2 = _plist[1];
   double sdX = P2.x() - P1.x();
   double sdY = P2.y() - P1.y();
   // The sign - a bit funny way - described in layout canvas
   int sign = ((sdX * sdY) >= 0) ? 1 : -1;
   double length = sqrt(sdY*sdY + sdX*sdX);
   int4b y0 = (int4b) rint(P1.y() - sign*((begext*sdY)/length));
   int4b x0 = (int4b) rint(P1.x() - sign*((begext*sdX)/length));
//
   P1 = _plist[numpoints-2];
   P2 = _plist[numpoints-1];
   sdX = P2.x() - P1.x();
   sdY = P2.y() - P1.y();
   sign = ((sdX * sdY) >= 0) ? 1 : -1;
   length = sqrt(sdY*sdY + sdX*sdX);
   int4b yn = (int4b) rint(P2.y() + sign*((endext*sdY)/length));
   int4b xn = (int4b) rint(P2.x() + sign*((endext*sdX)/length));
   _plist[0].setX(x0);
   _plist[0].setY(y0);
   _plist[numpoints-1].setX(xn);
   _plist[numpoints-1].setY(yn);
}
//==============================================================================
// class GDStext
//==============================================================================
GDSin::GDStext::GDStext(GDSFile* cf, GDSdata *lst):GDSdata(lst) {
   word ba;
   // initializing
   font = 0; vertjust = 0;   horijust = 0;pathtype = 0;
   width = 0;abs_magn = 0;abs_angl = 0;reflection = 0;
   magnification = 1.0; angle = 0.0;
   text[0] = 0x0;
   GDSrecord* cr = NULL;
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype()) {
            case gds_ELFLAGS: ReadELFLAGS(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:   ReadPLEX(cr);// seems that it's not used               
               delete cr;break;
            case gds_LAYER: cr->Ret_Data(&layer);
               delete cr;break;
            case gds_TEXTTYPE: cr->Ret_Data(&singletype);
               delete cr;break;
            case gds_PATHTYPE: cr->Ret_Data(&pathtype);// ??? for test ???
               delete cr;break;
            case gds_WIDTH: cr->Ret_Data(&width);// seems not to be used
               delete cr;break;
            case gds_PRESENTATION:
               cr->Ret_Data(&ba,0,16);
               font = ba & 0x0030; font >>= 4;
               vertjust = ba & 0x000C;vertjust >>= 2;
               horijust = ba & 0x0003;
               delete cr;break;
            case gds_STRANS:
               cr->Ret_Data(&ba,0,16);
               reflection = ba & 0x8000; reflection >>= 15;//bit 0
               abs_magn = ba & 0x0004; abs_magn >>= 2; //bit 13
               abs_angl = ba & 0x0002;abs_angl >>= 1; //bit 14
               delete cr; break;
            case gds_MAG: cr->Ret_Data(&magnification);
               delete cr; break;
            case gds_ANGLE: cr->Ret_Data(&angle);
               delete cr; break;
            case gds_XY: magn_point = GDSin::get_TP(cr);
               delete cr;break;
            case gds_STRING: cr->Ret_Data(&text);
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }   
   while (true);
}
   
// laydata::tdtdata* GDSin::GDStext::toTED() {
//    return NULL;
// }

//==============================================================================
// class GDSref
//==============================================================================
GDSin::GDSref::GDSref(GDSdata *lst):GDSdata(lst) {
   abs_angl=abs_magn=reflection=false;
   refstr = NULL;
   magnification = 1.0; angle = 0.0;
}

GDSin::GDSref::GDSref(GDSFile* cf, GDSdata *lst):GDSdata(lst) {
   word ba;
   //initializing
   abs_angl=abs_magn=reflection=false;
   refstr = NULL;
   magnification = 1.0; angle = 0.0;
   GDSrecord* cr = NULL;
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype()) {
            case gds_ELFLAGS: ReadELFLAGS(cr);// seems that it's not used
               delete cr;break;

            case gds_PLEX:   ReadPLEX(cr); // seems that it's not used
               delete cr;break;
            case gds_SNAME:
               if (cr->Get_reclen() > 32)   strname[0] = 0x0;
               else cr->Ret_Data(&strname);
               delete cr;break;
            case gds_STRANS:
               cr->Ret_Data(&ba,0,16);
               reflection = ba & 0x8000;//mask all bits except 0
               abs_magn = ba & 0x0004;//mask all bits except 13
               abs_angl = ba & 0x0002;//mask all bits except 14
               delete cr; break;
            case gds_MAG: cr->Ret_Data(&magnification);
               delete cr; break;
            case gds_ANGLE: cr->Ret_Data(&angle);
               delete cr; break;
            case gds_XY: magn_point = GDSin::get_TP(cr);
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               // before exiting, init Current Translation Matrix
//               tmtrx = new PSCTM(magn_point,magnification,angle,reflection);
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else   {AddLog('E',"Unexpected end of file");return;}
   }   
   while (true);
}

// laydata::tdtdata* GDSin::GDSref::toTED() {
//    return NULL;
// }

//==============================================================================
// class GDSaref
//==============================================================================
GDSin::GDSaref::GDSaref(GDSFile* cf, GDSdata *lst):GDSref(lst) {
   word ba;
   //initializing
   GDSrecord* cr = NULL;   
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype()) {
            case gds_ELFLAGS:ReadELFLAGS(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:ReadPLEX(cr);// seems that it's not used
               delete cr;break;
            case gds_SNAME:
               if (cr->Get_reclen() > 32)   strname[0] = 0x0;
               else cr->Ret_Data(&strname);
               delete cr;break;
            case gds_STRANS:
               cr->Ret_Data(&ba,0,16);
               reflection = ba & 0x8000;//mask all bits except 0
               abs_magn = ba & 0x0004;//mask all bita except 13
               abs_angl = ba & 0x0002;//mask all bita except 14
               delete cr; break;
            case gds_MAG: cr->Ret_Data(&magnification);
               delete cr; break;
            case gds_ANGLE: cr->Ret_Data(&angle);
               delete cr; break;
            case gds_XY:
               magn_point = GDSin::get_TP(cr,0);
               X_step = GDSin::get_TP(cr,1);
               Y_step = GDSin::get_TP(cr,2);
               delete cr;break;
            case gds_COLROW://return number of columns & rows in the array 
               cr->Ret_Data(&colnum);
               cr->Ret_Data(&rownum,2);
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               // before exiting, init Current Translation Matrix
//               tmtrx = new PSCTM(magn_point,magnification,angle,reflection);
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"Wrong record type in the current context");
               delete cr;return;
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }
   while (true);
}

int GDSin::GDSaref::Get_Xstep() {
   int ret = (int) sqrt(pow((X_step.x() - magn_point.x()),2) +
      pow((X_step.y() - magn_point.y()),2)) / colnum;
   return ret;
}

int GDSin::GDSaref::Get_Ystep() {
   int ret = (int) sqrt(pow((Y_step.x() - magn_point.x()),2) +
      pow((Y_step.y() - magn_point.y()),2)) / rownum;
   return ret;
}

// laydata::tdtdata* GDSin::GDSaref::toTED() {
//    return NULL;
// }

//-----------------------------------------------------------------------------
void GDSin::PrintChildren(GDSin::GDSHierTree* parent, std::string* tabnum){
   GDSstructure* cs = parent->GetItem();
   std::string mytab(*tabnum);
   std::string outname(mytab);
   outname += cs->Get_StrName();
   AddLog('A', outname.c_str());
   // Go down and print children first
   GDSHierTree* Child= parent->GetChild();
   if (Child) {
//      mytab.ReplaceChar('_',' ');
      mytab += "|__";
   }
   while (Child) {
      PrintChildren(Child, &mytab);
      Child = Child->GetBrother();
   }
}

//-----------------------------------------------------------------------------
TP GDSin::get_TP(GDSin::GDSrecord *cr, word curnum, byte len) {
   int4b GDS_X, GDS_Y;
   cr->Ret_Data(&GDS_X, curnum*len*2, len);
   cr->Ret_Data(&GDS_Y, curnum*len*2+len, len);
   return TP(GDS_X,GDS_Y);
}

//-----------------------------------------------------------------------------
void GDSin::AddLog(char logtype, const char* message){
   std::ostringstream ost; 
   switch (logtype){
      case 'B':{
         ost << "Input file: " << message; break;
      }
      case 'P':{
         ost << "Output file: " << message; break;
      }
      case 'S':{
         ost << "......" << message; break;
      }
      case 'W':{
         ost << "WARNING:" << message;
         if (InFile) InFile->Inc_GDSIIwarnings(); break;
      }
      case 'E':{
         ost << "ERROR:" << message;
         if (InFile) InFile->Inc_GDSIIerrors(); break;
      }
      case 'U':{
         ost << "WARNING:Record type '" << message;
         ost << "' found in GDSII and skipped";
         if (InFile) InFile->Inc_GDSIIwarnings();  break;
      }
      case 'I':{
         ost << "Reading PS settings file '" << message << "'..."; break;
      }
      case 'N':{
         ost << "Invalid number of parameters - line " << message;
         ost << " ... skipped";  break;
      }
      case 'M':{
         ost << "Invalid line  " << message << " ... skipped"; break;
      }
      default: ost << message;
   }
   tell_log(console::MT_INFO,ost.str().c_str());
} 
