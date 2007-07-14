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
//        Created: Sep 14 1999
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GDSII parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <cmath>
#include <sstream>
#include <string>
#include <time.h>
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

GDSin::GDSrecord::GDSrecord(byte rt, byte dt, word rl) {
   rectype = rt;datatype = dt;
   reclen = rl+4; index = 0;
   // compensation for odd length ASCII string
   if ((gdsDT_ASCII == datatype) && (rl % 2)) reclen++;
   record = new byte[reclen];
   add_int2b(reclen);
   record[index++] = rectype;
   record[index++] = datatype;
}

word GDSin::GDSrecord::flush(FILE* Gf) {
   assert(index == reclen);
   word bytes_written = fwrite(record,1,reclen,Gf);
   /*TODO !!! Error correction HERE instead of assertetion */
   assert(bytes_written == reclen);
   return bytes_written;
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
         // WARNING!!! not used and never checked !!!!
         _sg_int8 sign = (0x80 & record[curnum])? -1:1; //sign
         byte exponent = 0x7f & record[curnum]; // exponent
         _sg_int32 mantissa = 0; // mantissa
         byte* mant = (byte*)&mantissa;// take the memory possition
         mant[3] = 0x0;
         for (int i = 0; i < 3; i++)
            mant[i] = record[curnum+3-i];
         double *rld = (double*)var; // assign pointer
         *rld = sign*(mantissa/pow(2.0,24)) * pow(16.0, exponent-64);
         break;
      }
      case gdsDT_REAL8B:{// 8-byte real
         *(double*)var = gds2ieee(record);break;
      }
      case gdsDT_ASCII:// String
         rlc = (char*)var;
         if (len > 0){
            for (word i = 0; i < len; rlc[i] = record[curnum*len+i], i++);
            rlc[len] = 0x0;
         }
         else {
            for (word i = 0; i < reclen; rlc[i] = record[i], i++);
            rlc[reclen] = 0x0;
         }
         break;
   }
   return true;
}

double GDSin::GDSrecord::gds2ieee(byte* gds) {
   // zero is an exception (as always!) so check it first
   byte zerocheck;
   for (zerocheck = 0; zerocheck < 8; zerocheck++)
      if (0x00 != gds[zerocheck]) break;
   if (8 == zerocheck) return 0;
   // adjusting the exponent
   byte expcw [2] = {gds[1],gds[0]};
   word& expc = *((word*)&expcw);
   // IEEE has 2x exponent, while GDSII has 16x. To compensate
   // we need to multiply the exponent by 4, but, IEEE has 4 bits
   // wider exponent, so we need to shift GDS exp right by 4 positons.
   // in result, we are shifting right by two positions
   expc >>= 2;
   // fill-up the two leftmost positions of the exponent with
   // bits opposite to the excess bit
   // and also copy the excess bit
   if (!(0x40 & gds[0])){expc |= 0x3000; expc &= 0xBFFF;}
   else                 {expc &= 0xCFFF; expc |= 0x4000;}
   // clean-up
   expc &= 0x7FC0; // clean-up
   // compensate the difference in the excess notation
   expc -= 0x10;
   // Now normalize the mantissa - shift left until first 1 drops-out
   // The last byte will get some rubbish in it's LSBits, but it 
   // shouldn't matter because last four buts of the mantissa will be
   // chopped-out
   byte carry;
   do {
      carry = gds[1] & 0x80;
      for (byte i = 1; i < 7; i++) {
         gds[i] <<= 1;
         gds[i] |= (gds[i+1] >> 7); //carry
      }
      expc -= 0x10;
   } while (0 == carry);
   // copy the sign bit
   if    (0x80 & gds[0]) expc |= 0x8000;
   else                  expc &= 0x7FFF;
   // transfer the result into a new 8 byte string ...
   // ... copy the exponent first
   byte ieee[8];
   ieee[7] = expcw[1];ieee[6] = expcw[0];
   //... then copy the mantissa
   for  (byte i = 1; i <7; i++ ) {
      ieee[6-i] = (gds[i] << 4) | (gds[i+1] >> 4);
   }
   // last nibble of the mantissa
   ieee[6] |= gds[1] >> 4;
   // that should be it !
   return *((double*)&ieee);
}

byte* GDSin::GDSrecord::ieee2gds(double inval) {
   byte* ieee = ((byte*)&inval);
   byte* gds = new byte[8];
   // zero is an exception (as always!) so check it first
   if (0 == inval) {
      for (byte i = 0; i < 8; gds[i++] = 0x00);
      return gds;
   }
   //copy the mantissa
   for  (byte i = 1; i < 7; i++ ) {
      gds[i] = (ieee[7-i] << 4) | (ieee[6-i] >> 4);
   }
   gds[7] = ieee[0] << 4;
   // adjusting the exponent
   byte expcw [2] = {ieee[6],ieee[7]};
   word& expc = *((word*)&expcw);
   expc &= 0x7FF0; // clean-up
   //compensate the difference in excess notations
   expc += 0x10;
   // Now normalize the mantissa - shift right until the two LSBit of
   // the exponent are 00. First shift should introduce 1 on the leftmost
   // position of the manissa to take in mind the explicit 1 in the ieee
   // notation
   gds[0] = 0x01; 
   do {
      for (byte i = 7; i > 0; i--) {
         gds[i] >>= 1;
         gds[i] |= (gds[i-1] << 7); //carry
      }
      gds[0] = 0x00; 
      expc += 0x10;
   } while (0 != (expc & 0x0030));
   //make sure we are not trying to convert a number bigger than the one 
   //that GDS notation can cope with
   // copy the excess bit
   if (!(0x4000 & expc)) expc &= 0xEFFF;
   else                  expc |= 0x1000;
   // now multiply the exponent by 4 to convert in the 16x GDS exponent
   // here we are loosing silently the two most significant bits from the
   // ieee exponent. 
   expc <<= 2;
   // copy the sign bit
   if   (0x80 & ieee[7])  expc |= 0x8000;
   else                   expc &= 0x7FFF;
   gds[0] = expcw[1];
   return gds;
}

void GDSin::GDSrecord::add_int2b(const word data)
{
   byte* recpointer = (byte*)&data;
   record[index++] = recpointer[1];
   record[index++] = recpointer[0];
}

void GDSin::GDSrecord::add_int4b(const int4b data)
{
   byte* recpointer = (byte*)&data;
   record[index++] = recpointer[3];
   record[index++] = recpointer[2];
   record[index++] = recpointer[1];
   record[index++] = recpointer[0];
}

void GDSin::GDSrecord::add_ascii(const char* data)
{  
   word slen = strlen(data);
   bool compensate = (0 != (slen % 2));
   word strindex = 0;
   while (strindex < slen)
      record[index++] = data[strindex++];
   if (compensate) record[index++] = 0x00;
   assert(compensate ? ((reclen-4) == slen+1) : ((reclen-4) == slen) );
}

void GDSin::GDSrecord::add_real8b(const real data)
{
   byte* gdsreal = ieee2gds(data);
   for (byte i = 0; i < 8; i++) record[index++] = gdsreal[i];
   delete [] gdsreal;
}

GDSin::GDSrecord::~GDSrecord()
{
   delete[] record;
}

//==============================================================================
// class GDSFile
//==============================================================================
GDSin::GDSFile::GDSFile(const char* fn) {
   InFile = this; _hiertree = NULL;_status = false;
   GDSIIwarnings = GDSIIerrors = 0;
   filename = fn;
   file_pos = 0;
//   prgrs_pos = 0;
   library = NULL;
//   prgrs = progrind;
   AddLog('B',fn);
   if (!(GDSfh = fopen(filename.c_str(),"rb"))) {// open the input file
      std::ostringstream info;
      info << "File "<< fn <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      return;
   }
//   file_length = _filelength(GDSfh->_file);
   // The size of GDSII files is originaly multiple by 2048. This is
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
               closeFile();// close the input stream
//               prgrs_pos = file_length;
//               prgrs->SetPos(prgrs_pos); // fullfill progress indicator
               _status = true;
               AddLog('O',"Done");
               delete wr; 
               return; // go out
            default:   //parse error - not expected record type
               AddLog('E',"GDS header - wrong record type in the current context");
               delete wr;
               return;
         }
      else   {AddLog('E',"Unexpected end of file");return;}
   }   
   while (true);
}

GDSin::GDSFile::GDSFile(std::string fn, time_t acctime) {
   InFile = this;_hiertree = NULL;
   GDSIIwarnings = GDSIIerrors = 0;
   filename = fn;//initializing
   file_pos = 0;
   StreamVersion = 3;
   library = NULL;
//   prgrs_pos = 0;
//   prgrs = progrind;
   AddLog('P',fn.c_str());
   if (!(GDSfh = fopen(filename.c_str(),"wb"))) {// open the output file
      std::ostringstream info;
      info << "File "<< fn <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      //@TODO: Exception Here
      return;
   }//
   time_t acctim_N = acctime;
   tm* broken_time = localtime(&acctim_N);
   if (broken_time == NULL)
   {
      std::ostringstream info;
      info << "Error during defining time";
      tell_log(console::MT_ERROR,info.str());
   }
   t_access.Year  = broken_time->tm_year;
   t_access.Month = broken_time->tm_mon;
   t_access.Day   = broken_time->tm_mday;
   t_access.Hour  = broken_time->tm_hour;
   t_access.Min   = broken_time->tm_min;
   t_access.Sec   = broken_time->tm_sec;
   time_t cur_time = time(NULL);
   broken_time = localtime(&cur_time);
   t_modif.Year  = broken_time->tm_year;
   t_modif.Month = broken_time->tm_mon;
   t_modif.Day   = broken_time->tm_mday;
   t_modif.Hour  = broken_time->tm_hour;
   t_modif.Min   = broken_time->tm_min;
   t_modif.Sec   = broken_time->tm_sec;
   // start writing   
   GDSrecord* wr = NULL;
   // ... GDS header
   wr = SetNextRecord(gds_HEADER); wr->add_int2b(StreamVersion);
   flush(wr);
   //write BGNLIB record
   wr = SetNextRecord(gds_BGNLIB); SetTimes(wr);
   flush(wr);
}

void GDSin::GDSFile::GetTimes(GDSrecord *wr) {
   word cw;
   for (int i = 0; i<wr->Get_reclen()/2; i++) {
      wr->Ret_Data(&cw,2*i);
      switch (i) {
         case 0 :t_modif.Year   = cw;break;
         case 1 :t_modif.Month  = cw;break;
         case 2 :t_modif.Day    = cw;break;
         case 3 :t_modif.Hour   = cw;break;
         case 4 :t_modif.Min    = cw;break;
         case 5 :t_modif.Sec    = cw;break;
         case 6 :t_access.Year  = cw;break;
         case 7 :t_access.Month = cw;break;
         case 8 :t_access.Day   = cw;break;
         case 9 :t_access.Hour  = cw;break;
         case 10:t_access.Min   = cw;break;
         case 11:t_access.Sec   = cw;break;
      }
   }
}

void GDSin::GDSFile::SetTimes(GDSrecord* wr) {
   wr->add_int2b(t_modif.Year);
   wr->add_int2b(t_modif.Month);
   wr->add_int2b(t_modif.Day);
   wr->add_int2b(t_modif.Hour);
   wr->add_int2b(t_modif.Min);
   wr->add_int2b(t_modif.Sec);
   wr->add_int2b(t_access.Year);
   wr->add_int2b(t_access.Month);
   wr->add_int2b(t_access.Day);
   wr->add_int2b(t_access.Hour);
   wr->add_int2b(t_access.Min);
   wr->add_int2b(t_access.Sec);
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

GDSin::GDSrecord* GDSin::GDSFile::SetNextRecord(byte rectype, word reclen) {
   byte datatype;
   switch (rectype) {
      case gds_HEADER         :return new GDSrecord(rectype, gdsDT_INT2B , 2);
      case gds_BGNLIB         :return new GDSrecord(rectype, gdsDT_INT2B , 24);
      case gds_ENDLIB         :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_LIBNAME        :return new GDSrecord(rectype, gdsDT_ASCII , reclen);
      case gds_UNITS          :return new GDSrecord(rectype, gdsDT_REAL8B, 16);
      case gds_BGNSTR         :return new GDSrecord(rectype, gdsDT_INT2B , 24);
      case gds_STRNAME        :return new GDSrecord(rectype, gdsDT_ASCII , reclen);
      case gds_ENDSTR         :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_BOUNDARY       :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_PATH           :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_SREF           :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_AREF           :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_TEXT           :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_LAYER          :return new GDSrecord(rectype, gdsDT_INT2B, 2);
      case gds_DATATYPE       :return new GDSrecord(rectype, gdsDT_INT2B, 2);
      case gds_XY             :return new GDSrecord(rectype, gdsDT_INT4B, 8*reclen);
      case gds_WIDTH          :return new GDSrecord(rectype, gdsDT_INT4B, 4);
      case gds_ENDEL          :return new GDSrecord(rectype, gdsDT_NODATA, 0);
      case gds_SNAME          :return new GDSrecord(rectype, gdsDT_ASCII, reclen);
      case gds_COLROW         :return new GDSrecord(rectype, gdsDT_INT2B , 4);
      case gds_TEXTTYPE       :return new GDSrecord(rectype, gdsDT_INT2B, 2);
      case gds_STRING         :return new GDSrecord(rectype, gdsDT_ASCII, reclen);
      case gds_STRANS         :return new GDSrecord(rectype, gdsDT_BIT, 2);
      case gds_MAG            :return new GDSrecord(rectype, gdsDT_REAL8B, 8);
      case gds_ANGLE          :return new GDSrecord(rectype, gdsDT_REAL8B, 8);
      case gds_PROPATTR       :datatype = gdsDT_INT2B;break;
      case gds_PROPVALUE      :datatype = gdsDT_ASCII;break;
                       default: assert(false); //the rest should not be used
//----------------------------------------------------------------------------------
// The record types below are not used currently in GDS export
//       case gds_NODE           :datatype = gdsDT_NODATA;break;
//       case gds_PRESENTATION   :datatype = gdsDT_BIT;break;
//       case gds_REFLIBS        :datatype = gdsDT_ASCII;break;
//       case gds_FONTS          :datatype = gdsDT_ASCII;break;
//       case gds_PATHTYPE       :datatype = gdsDT_INT2B;break;
//       case gds_GENERATION     :datatype = gdsDT_INT2B;break;
//       case gds_ATTRTABLE      :datatype = gdsDT_ASCII;break;
//       case gds_ELFLAGS        :datatype = gdsDT_BIT;break;
//       case gds_NODETYPE       :datatype = gdsDT_INT2B;break;
//       case gds_PROPATTR       :datatype = gdsDT_INT2B;break;//
//       case gds_PROPVALUE      :datatype = gdsDT_ASCII;break;//
//       case gds_FORMAT         :datatype = gdsDT_INT2B;break;
//       case gds_BORDER         :datatype = gdsDT_NODATA;break;
//       case gds_SOFTFENCE      :datatype = gdsDT_NODATA;break;
//       case gds_HARDFENCE      :datatype = gdsDT_NODATA;break;
//       case gds_SOFTWIRE       :datatype = gdsDT_NODATA;break;
//       case gds_HARDWIRE       :datatype = gdsDT_NODATA;break;
//       case gds_PATHPORT       :datatype = gdsDT_NODATA;break;
//       case gds_NODEPORT       :datatype = gdsDT_NODATA;break;
//       case gds_USERCONSTRAINT :datatype = gdsDT_NODATA;break;
//       case gds_SPACER_ERROR   :datatype = gdsDT_NODATA;break;
//       case gds_CONTACT        :datatype = gdsDT_NODATA;break;

/*
      deprecated, should not be used for export
      case gds_BOX            :datatype = gdsDT_NODATA;break;
      case gds_BOXTYPE        :datatype = gdsDT_INT2B;break;
      case gds_PLEX           :datatype = gdsDT_INT4B;break;
      case gds_BGNEXTN        :datatype = gdsDT_INT4B;break;
      case gds_ENDEXTN        :datatype = gdsDT_INT4B;break;
      case gds_MASK           :datatype = gdsDT_ASCII;break;
      case gds_ENDMASKS       :datatype = gdsDT_NODATA;break;
      case gds_LIBSECUR       :datatype = gdsDT_INT2B;break;
      case gds_SRFNAME        :datatype = gdsDT_ASCII;break;
      case gds_LIBDIRSIZE     :datatype = gdsDT_INT2B;break;
      case gds_TEXTNODE       :datatype = gdsDT_NODATA;break;
      case gds_STYPTABLE      :datatype = gdsDT_ASCII;break;
      case gds_STRTYPE        :datatype = gdsDT_INT2B;break;
      case gds_ELKEY          :datatype = gdsDT_INT4B;break;
      case gds_STRCLASS       :datatype = gdsDT_BIT;break;
      case gds_RESERVED       :datatype = gdsDT_INT4B;break;
      case gds_TAPENUM        :datatype = gdsDT_INT2B;break;
      case gds_TAPECODE       :datatype = gdsDT_INT2B;break;
      case gds_SPACING:
      case gds_UINTEGER:
      case gds_USTRING:
      case gds_LINKTYPE:
      case gds_LINKKEYS:
      */
   }
   return new GDSrecord(rectype, datatype,0);
}

bool GDSin::GDSFile::checkCellWritten(std::string cellname)
{
   for (nameList::const_iterator i = _childnames.begin();
                                 i != _childnames.end(); i++)
      if (cellname == *i) return true;
   return false;      
//   return (_childnames.end() != _childnames.find(cellname));
}

void GDSin::GDSFile::registerCellWritten(std::string cellname)
{
   _childnames.push_back(cellname);
}

void GDSin::GDSFile::flush(GDSrecord* wr)
{
   file_pos += wr->flush(GDSfh);delete wr;
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

void GDSin::GDSFile::updateLastRecord()
{
   word num_zeroes = 2048 - (file_pos % 2048);
   byte record = 0x00;
   word bytes_written = fwrite(&record,1, num_zeroes, GDSfh);
   assert(bytes_written == num_zeroes);
   file_pos += bytes_written;
}

GDSin::GDSFile::~GDSFile() 
{
   delete library;
   // get rid of the hierarchy tree
   GDSHierTree* droot;
   while (_hiertree)
   {
      droot = _hiertree; _hiertree = droot->GetLast();
      delete droot;
   }
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
            case gds_FORMAT:// skipped record !!!
               AddLog('U',"FORMAT");
               delete cr;break;
            case gds_MASK:// skipped record !!!
               AddLog('U',"MASK");
               delete cr;break;
            case gds_ENDMASKS:// skipped record !!!
               AddLog('U',"ENDMASKS");
               delete cr;break;
            case gds_REFLIBS:// skipped record !!!
               AddLog('U',"REFLIBS");
               delete cr;break;
            case gds_ATTRTABLE:// skipped record !!!
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
               AddLog('E',"GDS Library - wrong record type in the current context");
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
            case gds_NODE:// skipped record !!!
               AddLog('U',"NODE");
               delete cr;break;
            case gds_PROPATTR:// skipped record !!!
               AddLog('U',"NODE");
               delete cr;break;
            case gds_STRCLASS:// skipped record !!!
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
               AddLog('E',"GDS structure - wrong record type in the current context");
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
   GDSrecord* cr = NULL;
   do {//start reading
      cr = cf->GetNextRecord();
      if (cr)
         switch (cr->Get_rectype()) {
            case gds_ELFLAGS:ReadELFLAGS(cr);// seems that it's not used
               delete cr; break;
            case gds_PLEX:   ReadPLEX(cr);// seems that it's not used
               delete cr; break;
            case gds_LAYER: cr->Ret_Data(&layer);
               delete cr; break;
            case gds_BOXTYPE:cr->Ret_Data(&boxtype);// Don't know what is this !!!
               delete cr; break;
            case gds_PROPATTR: AddLog('W',"GDS box - PROPATTR record ignored");
               delete cr;break;
            case gds_PROPVALUE: AddLog('W',"GDS box - PROPVALUE record ignored");
               delete cr;break;
            case gds_XY: {
               word numpoints = (cr->Get_reclen())/8 - 1;
               // one point less because fist and last point coincide
               assert(numpoints == 4);
               _plist.reserve(numpoints);
               for(word i = 0; i < numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr; break;
               }
            case gds_ENDEL://end of element, exit point
               delete cr;return;
            default:{
               //parse error - not expected record type
               AddLog('E',"GDS box - wrong record type in the current context");
               delete cr;return;
            }
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }
   while (true);
}

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
            case gds_PROPATTR: AddLog('W',"GDS boundary - PROPATTR record ignored");
               delete cr;break;
            case gds_PROPVALUE: AddLog('W',"GDS boundary - PROPVALUE record ignored");
               delete cr;break;
            case gds_XY: numpoints = (cr->Get_reclen())/8 - 1;
               // one point less because fist and last point coincide
               _plist.reserve(numpoints);
               for(i = 0; i < numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr; break;
            case gds_ENDEL://end of element, exit point
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"GDS boundary - wrong record type in the current context");
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
            case gds_PROPATTR: AddLog('W',"GDS path - PROPATTR record ignored");
               delete cr;break;
            case gds_PROPVALUE: AddLog('W',"GDS path - PROPVALUE record ignored");
               delete cr;break;
            case gds_XY:numpoints = (cr->Get_reclen())/8;
               _plist.reserve(numpoints);
               for(i = 0; i < numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               if (2 == pathtype) {
                  AddLog('W',"GDS Pathtype 2 digitized. Will be converted to Pathtype 0");
                  convert22(width/2, width/2);
               }   
               else if (4 == pathtype) {
                  AddLog('W',"GDS Pathtype 4 digitized. Will be converted to Pathtype 0");
                  convert22(bgnextn, endextn);
               }
               delete cr;return;
            default://parse error - not expected record type
               AddLog('E',"GDS path - wrong record type in the current context");
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
            case gds_PROPATTR: AddLog('W',"GDS text - PROPATTR record ignored");
               delete cr;break;
            case gds_PROPVALUE: AddLog('W',"GDS text - PROPVALUE record ignored");
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
               AddLog('E',"GDS text - wrong record type in the current context");
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
   int tmp; //Dummy variable. Use for gds_PROPATTR
   char tmp2[128]; //Dummy variable. Use for gds_PROPVALUE
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
            //TODO Not implemented yet+++++
            case gds_PROPATTR:
               cr->Ret_Data(&tmp);
               delete cr; break;
            //TODO Not implemented yet+++++
            case gds_PROPVALUE:
               cr->Ret_Data(&tmp2);
               AddLog('A',tmp2, tmp);
               delete cr; break;
            default://parse error - not expected record type
               AddLog('E',"GDS sref - wrong record type in the current context");
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
   int tmp; //Dummy variable. Use for gds_PROPATTR
   char tmp2[128]; //Dummy variable. Use for gds_PROPVALUE
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
            //TODO not implemented yet+++++
            case gds_PROPATTR:
               cr->Ret_Data(&tmp);
               delete cr; break;
            //TODO not implemented yet+++++
            case gds_PROPVALUE:
               cr->Ret_Data(&tmp2);
               AddLog('A',tmp2, tmp);
               delete cr; break;
            default://parse error - not expected record type
               AddLog('E',"GDS aref - wrong record type in the current context");
               delete cr;return;
         }
      else {AddLog('E',"Unexpected end of file");return;}
   }
   while (true);
}

int GDSin::GDSaref::Get_Xstep() {
   int ret = (int) sqrt(pow(float((X_step.x() - magn_point.x())),2) +
      pow(float((X_step.y() - magn_point.y())),2)) / colnum;
   return ret;
}

int GDSin::GDSaref::Get_Ystep() {
   int ret = (int) sqrt(pow(float((Y_step.x() - magn_point.x())),2) +
      pow(float((Y_step.y() - magn_point.y())),2)) / rownum;
   return ret;
}

//-----------------------------------------------------------------------------
/*void GDSin::PrintChildren(GDSin::GDSHierTree* parent, std::string* tabnum){
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
*/
//-----------------------------------------------------------------------------
TP GDSin::get_TP(GDSin::GDSrecord *cr, word curnum, byte len) {
   int4b GDS_X, GDS_Y;
   cr->Ret_Data(&GDS_X, curnum*len*2, len);
   cr->Ret_Data(&GDS_Y, curnum*len*2+len, len);
   return TP(GDS_X,GDS_Y);
}

//-----------------------------------------------------------------------------
void GDSin::AddLog(char logtype, const char* message, const int number){
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
      case 'A':{
         ost << "Property attribute  " << number << " with value " << message << " ignored" ; break;
      }
      default: ost << message;
   }
   tell_log(console::MT_INFO,ost.str());
} 
