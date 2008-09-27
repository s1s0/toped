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
#include <wx/wx.h>
#include <wx/regex.h>
#include "gds_io.h"
#include "../tpd_common/ttt.h"
#include "../tpd_common/outbox.h"

static GDSin::GdsFile*        InFile    = NULL;
//==============================================================================
GDSin::GdsRecord::GdsRecord(FILE* Gf, word rl, byte rt, byte dt)
{
   _recLen = rl; _recType = rt; _dataType = dt;
   if (rl)
   {
      _record = DEBUG_NEW byte[_recLen];
      _numread = fread(_record, 1, _recLen, Gf);
      _valid = (_numread == _recLen) ? true : false;
   }
   else
   {
      _record = NULL;
      _numread = 0;
      _valid = true;
   }
}

GDSin::GdsRecord::GdsRecord(byte rt, byte dt, word rl)
{
   _recType = rt; _dataType = dt;
   _recLen = rl+4; _index = 0;
   // compensation for odd length ASCII string
   if ((gdsDT_ASCII == _dataType) && (rl % 2)) _recLen++;
   _record = DEBUG_NEW byte[_recLen];
   add_int2b(_recLen);
   _record[_index++] = _recType;
   _record[_index++] = _dataType;
}

word GDSin::GdsRecord::flush(FILE* Gf)
{
   assert(_index == _recLen);
   word bytes_written = fwrite(_record, 1, _recLen, Gf);
   /*TODO !!! Error correction HERE instead of assertetion */
   assert(bytes_written == _recLen);
   return bytes_written;
}

bool GDSin::GdsRecord::retData(void* var, word curnum, byte len)
{
   byte      *rlb;
   char      *rlc;
   switch (_dataType)
   {
      case gdsDT_NODATA:// no data present
         var = NULL;return false;
      case gdsDT_BIT:// bit array
         rlb = (byte*)var; //assign pointer
         switch (len)
         {
            case 32:
               rlb[3] = _record[0];
               rlb[2] = _record[1];
               rlb[1] = _record[2];
               rlb[0] = _record[3];
               break;
            case 16:
               rlb[1] = _record[0];
               rlb[0] = _record[1];
               break;
            case 8:
               rlb[0] = _record[0];
               break;
            default:
               var = NULL;break;
         }
         break;
      case gdsDT_INT2B://2-byte signed integer
         rlb = (byte*)var;   //assign pointer
         rlb[0] = _record[curnum+1];
         rlb[1] = _record[curnum+0];
         break;
      case gdsDT_INT4B: // 4-byte signed integer
         rlb = (byte*)var;//assign pointer
         rlb[0] = _record[curnum+3];
         rlb[1] = _record[curnum+2];
         rlb[2] = _record[curnum+1];
         rlb[3] = _record[curnum+0];
         break;
      case gdsDT_REAL4B:// 4-bit real
      {
         // WARNING!!! not used and never checked !!!!
         _sg_int8 sign = (0x80 & _record[curnum])? -1:1; //sign
         byte exponent = 0x7f & _record[curnum]; // exponent
         _sg_int32 mantissa = 0; // mantissa
         byte* mant = (byte*)&mantissa;// take the memory possition
         mant[3] = 0x0;
         for (int i = 0; i < 3; i++)
            mant[i] = _record[curnum+3-i];
         double *rld = (double*)var; // assign pointer
         *rld = sign*(mantissa/pow(2.0,24)) * pow(16.0, exponent-64);
         break;
      }
      case gdsDT_REAL8B:// 8-byte real
         *(double*)var = gds2ieee(_record);
         break;
      case gdsDT_ASCII:// String
         rlc = (char*)var;
         if (len > 0)
         {
            for (word i = 0; i < len; rlc[i] = _record[curnum*len+i], i++);
            rlc[len] = 0x0;
         }
         else
         {
            for (word i = 0; i < _recLen; rlc[i] = _record[i], i++);
            rlc[_recLen] = 0x0;
         }
         break;
   }
   return true;
}

double GDSin::GdsRecord::gds2ieee(byte* gds)
{
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

byte* GDSin::GdsRecord::ieee2gds(double inval)
{
   byte* ieee = ((byte*)&inval);
   byte* gds = DEBUG_NEW byte[8];
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

void GDSin::GdsRecord::add_int2b(const word data)
{
   byte* recpointer = (byte*)&data;
   _record[_index++] = recpointer[1];
   _record[_index++] = recpointer[0];
}

void GDSin::GdsRecord::add_int4b(const int4b data)
{
   byte* recpointer = (byte*)&data;
   _record[_index++] = recpointer[3];
   _record[_index++] = recpointer[2];
   _record[_index++] = recpointer[1];
   _record[_index++] = recpointer[0];
}

void GDSin::GdsRecord::add_ascii(const char* data)
{
   word slen = strlen(data);
   bool compensate = (0 != (slen % 2));
   word strindex = 0;
   while (strindex < slen)
      _record[_index++] = data[strindex++];
   if (compensate) _record[_index++] = 0x00;
   assert(compensate ? ((_recLen-4) == slen+1) : ((_recLen-4) == slen) );
}

void GDSin::GdsRecord::add_real8b(const real data)
{
   byte* gdsreal = ieee2gds(data);
   for (byte i = 0; i < 8; i++) _record[_index++] = gdsreal[i];
   delete [] gdsreal;
}

GDSin::GdsRecord::~GdsRecord()
{
   delete[] _record;
}

//==============================================================================
// class GdsFile
//==============================================================================
GDSin::GdsFile::GdsFile(std::string fn)
{
   InFile = this; _hierTree = NULL;_status = false;
   _laymap = NULL;
   _gdsiiWarnings = _gdsiiErrors = 0;
   _fileName = fn;
   _filePos = 0;
//   prgrs_pos = 0;
   _library = NULL;
//   prgrs = progrind;
   tell_log(console::MT_INFO, std::string("GDSII input file: \"") + fn + std::string("\""));
   std::string fname(convertString(_fileName));
   if (!(_gdsFh = fopen(fname.c_str(),"rb")))
   {// open the input file
      std::ostringstream info;
      info << "File "<< fn <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      return;
   }
//   file_length = _filelength(_gdsFh->_file);
   // The size of GDSII files is originaly multiple by 2048. This is
   // coming from the acient years when this format was supposed to be written 
   // on the magnetic tapes. In order to keep the tradition it's a good idea 
   // to check the file size and to issue a warning if it is not multiple on 2048.
//   div_t divi = div(file_length,2048);
//   if (divi.rem != 0) AddLog('W',"File size is not multiple of 2048");
//   prgrs->SetRange32(0,file_length);// initializes progress indicator control
//   prgrs->SetStep(1);
   GdsRecord* wr = NULL;

   do
   {// start reading
      wr = getNextRecord();
      if (wr)
      {
         switch (wr->recType())
         {
            case gds_HEADER:      wr->retData(&_streamVersion);
               delete wr;break;
            case gds_BGNLIB:      getTimes(wr);
               delete wr;break;
            case gds_LIBDIRSIZE:   wr->retData(&_libDirSize);
               delete wr;break;
            case gds_SRFNAME:      wr->retData(&_srfName);
               delete wr;break;
            case gds_LIBSECUR:// I don't need this info. Does anybody need it?
               delete wr;break;
            case gds_LIBNAME:   // down in the hierarchy. 
               //Start reading the library structure
               _library = DEBUG_NEW GdsLibrary(this, wr);
               //build the hierarchy tree
               _library->setHierarchy();
               closeFile();// close the input stream
//               prgrs_pos = file_length;
//               prgrs->SetPos(prgrs_pos); // fullfill progress indicator
               _status = true;
               tell_log(console::MT_INFO, "Done");
               delete wr;
               return; // go out
            default:   //parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS header - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete wr;
               return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

GDSin::GdsFile::GdsFile(std::string fn, const LayerMapGds* laymap, time_t acctime)
{
   InFile = this;_hierTree = NULL;
   _laymap = laymap;
   _gdsiiWarnings = _gdsiiErrors = 0;
   _fileName = fn;//initializing
   _filePos = 0;
   _streamVersion = 3;
   _library = NULL;
//   prgrs_pos = 0;
//   prgrs = progrind;
   std::string fname(convertString(_fileName));
   if (!(_gdsFh = fopen(fname.c_str(),"wb")))
   {// open the output file
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
   _tAccess.Year  = broken_time->tm_year;
   _tAccess.Month = broken_time->tm_mon+1;
   _tAccess.Day   = broken_time->tm_mday;
   _tAccess.Hour  = broken_time->tm_hour;
   _tAccess.Min   = broken_time->tm_min;
   _tAccess.Sec   = broken_time->tm_sec;
   time_t cur_time = time(NULL);
   broken_time = localtime(&cur_time);
   _tModif.Year  = broken_time->tm_year;
   _tModif.Month = broken_time->tm_mon+1;
   _tModif.Day   = broken_time->tm_mday;
   _tModif.Hour  = broken_time->tm_hour;
   _tModif.Min   = broken_time->tm_min;
   _tModif.Sec   = broken_time->tm_sec;
   // start writing   
   GdsRecord* wr = NULL;
   // ... GDS header
   wr = setNextRecord(gds_HEADER); wr->add_int2b(_streamVersion);
   flush(wr);
   //write BGNLIB record
   wr = setNextRecord(gds_BGNLIB); setTimes(wr);
   flush(wr);
}

void GDSin::GdsFile::getTimes(GdsRecord *wr)
{
   word cw;
   for (int i = 0; i<wr->recLen()/2; i++)
   {
      wr->retData(&cw,2*i);
      switch (i)
      {
         case 0 :_tModif.Year   = cw;break;
         case 1 :_tModif.Month  = cw;break;
         case 2 :_tModif.Day    = cw;break;
         case 3 :_tModif.Hour   = cw;break;
         case 4 :_tModif.Min    = cw;break;
         case 5 :_tModif.Sec    = cw;break;
         case 6 :_tAccess.Year  = cw;break;
         case 7 :_tAccess.Month = cw;break;
         case 8 :_tAccess.Day   = cw;break;
         case 9 :_tAccess.Hour  = cw;break;
         case 10:_tAccess.Min   = cw;break;
         case 11:_tAccess.Sec   = cw;break;
      }
   }
}

void GDSin::GdsFile::setTimes(GdsRecord* wr) {
   wr->add_int2b(_tModif.Year);
   wr->add_int2b(_tModif.Month);
   wr->add_int2b(_tModif.Day);
   wr->add_int2b(_tModif.Hour);
   wr->add_int2b(_tModif.Min);
   wr->add_int2b(_tModif.Sec);
   wr->add_int2b(_tAccess.Year);
   wr->add_int2b(_tAccess.Month);
   wr->add_int2b(_tAccess.Day);
   wr->add_int2b(_tAccess.Hour);
   wr->add_int2b(_tAccess.Min);
   wr->add_int2b(_tAccess.Sec);
}

GDSin::GdsRecord* GDSin::GdsFile::getNextRecord()
{
   char recheader[4]; // record header
   unsigned numread = fread(&recheader,1,4,_gdsFh);// read record header
   if (numread != 4)
      return NULL;// error during read in
   char rl[2];
   rl[0] = recheader[1];
   rl[1] = recheader[0];
   word reclen = *(word*)rl - 4; // record lenght
   GdsRecord* retrec = DEBUG_NEW GdsRecord(_gdsFh, reclen, recheader[2],recheader[3]);
   _filePos += reclen + 4;    // update file position
//   if (2048 < (file_pos - prgrs_pos))
//   {
//      prgrs_pos = file_pos;
//      prgrs->SetPos(prgrs_pos); // update progress indicator
//   }
   if (retrec->valid()) return retrec;
   else return NULL;// error during read in
}

GDSin::GdsRecord* GDSin::GdsFile::setNextRecord(byte rectype, word reclen)
{
   byte datatype;
   switch (rectype)
   {
      case gds_HEADER         :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 2         );
      case gds_BGNLIB         :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 24        );
      case gds_ENDLIB         :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_LIBNAME        :return DEBUG_NEW GdsRecord(rectype, gdsDT_ASCII   , reclen    );
      case gds_UNITS          :return DEBUG_NEW GdsRecord(rectype, gdsDT_REAL8B  , 16        );
      case gds_BGNSTR         :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 24        );
      case gds_STRNAME        :return DEBUG_NEW GdsRecord(rectype, gdsDT_ASCII   , reclen    );
      case gds_ENDSTR         :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_BOUNDARY       :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_PATH           :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_SREF           :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_AREF           :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_TEXT           :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_LAYER          :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 2         );
      case gds_DATATYPE       :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 2         );
      case gds_XY             :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT4B   , 8*reclen  );
      case gds_WIDTH          :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT4B   , 4         );
      case gds_ENDEL          :return DEBUG_NEW GdsRecord(rectype, gdsDT_NODATA  , 0         );
      case gds_SNAME          :return DEBUG_NEW GdsRecord(rectype, gdsDT_ASCII   , reclen    );
      case gds_COLROW         :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 4         );
      case gds_TEXTTYPE       :return DEBUG_NEW GdsRecord(rectype, gdsDT_INT2B   , 2         );
      case gds_STRING         :return DEBUG_NEW GdsRecord(rectype, gdsDT_ASCII   , reclen    );
      case gds_STRANS         :return DEBUG_NEW GdsRecord(rectype, gdsDT_BIT     , 2         );
      case gds_MAG            :return DEBUG_NEW GdsRecord(rectype, gdsDT_REAL8B  , 8         );
      case gds_ANGLE          :return DEBUG_NEW GdsRecord(rectype, gdsDT_REAL8B  , 8         );
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
   return DEBUG_NEW GdsRecord(rectype, datatype,0);
}

bool GDSin::GdsFile::checkCellWritten(std::string cellname)
{
   for (nameList::const_iterator i = _childnames.begin();
                                 i != _childnames.end(); i++)
      if (cellname == *i) return true;
   return false;
//   return (_childnames.end() != _childnames.find(cellname));
}

void GDSin::GdsFile::registerCellWritten(std::string cellname)
{
   _childnames.push_back(cellname);
}

void GDSin::GdsFile::flush(GdsRecord* wr)
{
   _filePos += wr->flush(_gdsFh); delete wr;
}


GDSin::GdsStructure* GDSin::GdsFile::getStructure(const char* selection)
{
   GdsStructure* Wstrct = _library->fStruct();
   while (Wstrct)
   {
      if (!strcmp(Wstrct->name(),selection)) return Wstrct;
      Wstrct = Wstrct->last();
   }
   return NULL;
}

void GDSin::GdsFile::collectLayers(GdsLayers& layers)
{
   GdsStructure* wstrct = _library->fStruct();
   while (wstrct)
   {
      // There is no point to traverse the hierarchy here if we already have
      // all the cells listed
      wstrct->collectLayers(layers, false);
      wstrct = wstrct->last();
   }
}

void GDSin::GdsFile::getMappedLayType(word& gdslay, word& gdstype, word tdtlay)
{
   _laymap->getGdsLayType(gdslay, gdstype, tdtlay);
   //@TODO Error message if
}

double GDSin::GdsFile::libUnits()
{
   return _library->dbu()/_library->uu();
}

double GDSin::GdsFile::userUnits()
{
   return _library->uu();
}

void GDSin::GdsFile::updateLastRecord()
{
   word num_zeroes = 2048 - (_filePos % 2048);
   byte record = 0x00;
   word bytes_written = fwrite(&record,1, num_zeroes, _gdsFh);
   assert(bytes_written == num_zeroes);
   _filePos += bytes_written;
}

GDSin::GdsFile::~GdsFile() 
{
   delete _library;
   // get rid of the hierarchy tree
   const GDSHierTree* var1 = _hierTree;
   while (var1)
   {
      const GDSHierTree* var2 = var1->GetLast();
      delete var1; var1 = var2;
   }
}

//==============================================================================
// class GdsLibrary
//==============================================================================
GDSin::GdsLibrary::GdsLibrary(GdsFile* cf, GdsRecord* cr)
{
   int i;
   cr->retData(&_name);//Get library name
   // init section
   _maxver = 3;   _fStruct = NULL;
   for (i = 0; i < 4; _fonts[i++] = NULL);
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_FORMAT:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'FORMAT' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_MASK:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'MASK' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_ENDMASKS:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'ENDMASKS' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_REFLIBS:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'REFLIBS' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_ATTRTABLE:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'ATTRTABLE' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_FONTS:// Read fonts
               for(i = 0; i < 4; i++)  {
                  _fonts[i] = DEBUG_NEW char[45];
                  cr->retData(_fonts[i],i,44);
               }
               delete cr;break;
            case gds_GENERATION:   cr->retData(&_maxver);
               delete cr;break;
            case gds_UNITS:   
               cr->retData(&_uu,0,8); // database units in one user unit
               cr->retData(&_dbu,8,8); // database unit in meters
               delete cr;break;
            case gds_BGNSTR:
               _fStruct = DEBUG_NEW GdsStructure(cf, _fStruct);
               tell_log(console::MT_INFO,std::string("...") + _fStruct->name());
               delete cr;break;
            case gds_ENDLIB://end of library, exit form the procedure
               delete cr;return;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS Library - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }   
   while (true);
}

void GDSin::GdsLibrary::setHierarchy()
{
   GdsStructure* ws = _fStruct;
   while (ws)
   {//for every structure
      GdsData* wd = ws->fDataAt(0);
      while (wd)
      { //for every GdsData of type SREF or AREF
      //put a pointer to GdsStructure
         word dt = wd->gdsDataType();
         if ((gds_SREF == dt) || (gds_AREF == dt))
         {//means that GdsData type is AREF or SREF 
            char* strname = ((GdsRef*) wd)->strName();
            GdsStructure* ws2 = _fStruct;
            while ((ws2) && (strcmp(strname,ws2->name())))
               ws2 = ws2->last();
            ((GdsRef*) wd)->SetStructure(ws2);
            if (ws2)
            {
               ws->registerStructure(ws2);
               ws2->_haveParent = true;
            }
            else
            {//structure is referenced but not defined!
               char wstr[256];
               sprintf(wstr," Structure %s is referenced, but not defined!",
                       ((GdsRef*)wd)->strName());
               tell_log(console::MT_WARNING,wstr);
               InFile->incGdsiiWarnings();
               //SGREM probably is a good idea to add default
               //GdsStructure here. Then this structure can be
               //visualized in the Hierarchy window as disabled
            }
         }
         wd = wd->last();
      }
      ws = ws->last();
   }
}

GDSin::GDSHierTree* GDSin::GdsLibrary::hierOut()
{
   GdsStructure* ws = _fStruct;
   GDSHierTree* Htree = NULL;
   while (ws)
   {
      if (!ws->_haveParent)  Htree = ws->hierOut(Htree,NULL);
      ws = ws->last();
   }
   return Htree;
}


GDSin::GdsLibrary::~GdsLibrary()
{
   for(int i = 0; i < 4; i++)
      if (_fonts[i]) delete _fonts[i];
   GdsStructure* Wstruct;
   while (_fStruct)
   {
      Wstruct = _fStruct->last();
      delete _fStruct;
      _fStruct = Wstruct;
   }
}

//==============================================================================
// class GdsStructure
//==============================================================================
GDSin::GdsStructure::GdsStructure(GdsFile *cf, GdsStructure* lst)
{
   _traversed = false;
   int i;
   int2b layer;
   //initializing
   _last = lst;
   GdsData* cData = NULL;
   _haveParent = false;
   GdsRecord* cr = NULL;
   for (i = 0; i < GDS_MAX_LAYER; _allLay[i++] = false);
   do
   { //start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_NODE:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'NODE' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_PROPATTR:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'PROPATTR' skipped");
               InFile->incGdsiiWarnings();
               delete cr;break;
            case gds_STRCLASS:// skipped record !!!
               tell_log(console::MT_WARNING, " GDSII record type 'STRCLASS' skipped");
               InFile->incGdsiiWarnings();// CADANCE internal use only
               delete cr;break;
            case gds_STRNAME:
               if (cr->recLen() > 64)
                  _name[0] = 0x0;
               else cr->retData(&_name);
               delete cr;break;
            case gds_BOX: 
               cData = DEBUG_NEW GdsBox(cf, layer);
               linkDataIn(cData, layer);
               delete cr;break;
            case gds_BOUNDARY: 
               cData = DEBUG_NEW GdsPolygon(cf, layer);
               linkDataIn(cData, layer);
               delete cr;break;
            case gds_PATH: 
               cData = DEBUG_NEW GDSpath(cf, layer);
               linkDataIn(cData, layer);
               delete cr;break;
            case gds_TEXT:
               cData = DEBUG_NEW GdsText(cf, layer);
               linkDataIn(cData, layer);
               delete cr;break;
            case gds_SREF:
               cData = DEBUG_NEW GdsRef(cf);
               linkDataIn(cData, 0);
               delete cr;break;
            case gds_AREF: 
               cData = DEBUG_NEW GdsARef(cf);
               linkDataIn(cData, 0);
               delete cr;break;
            case gds_ENDSTR:// end of structure, exit point
               delete cr;return;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS structure - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

void GDSin::GdsStructure::linkDataIn(GdsData* data, int2b layer)
{
   if (_layers.end() == _layers.find(layer))
      _layers[layer] = data->linkTo(NULL);
   else
      _layers[layer] = data->linkTo(_layers[layer]);
   _allLay[layer] = true;
}

GDSin::GdsData* GDSin::GdsStructure::fDataAt(int2b layer)
{
   if (_layers.end() == _layers.find(layer))
      return NULL;
   else
      return _layers[layer];
}

void GDSin::GdsStructure::collectLayers(GdsLayers& layers_map, bool hier)
{
   for (LayMap::const_iterator CL = _layers.begin(); CL != _layers.end(); CL++)
   {
      if (0 == CL->first) continue;
      WordList data_types;
      if (layers_map.end() != layers_map.find(CL->first))
         data_types = layers_map[CL->first];
      GdsData* wdata = CL->second;
      while (NULL != wdata)
      {
         data_types.push_back(wdata->singleType());
         wdata = wdata->last();
      }
      data_types.sort();
      data_types.unique();
      layers_map[CL->first] = data_types;
   }

   for (unsigned i = 0; i < _children.size(); i++)
      if (NULL == _children[i]) continue;
   else
      _children[i]->collectLayers(layers_map, hier);
}

bool GDSin::GdsStructure::registerStructure(GdsStructure* ws)
{
   for (unsigned i=0; i < _children.size(); i++)
   {
      if (NULL == _children[i]) continue;
      else if (!strcmp(_children[i]->name(), ws->name()))
         return false;
   }
   _children.push_back(ws);
   return true;
}

GDSin::GDSHierTree* GDSin::GdsStructure::hierOut(GDSHierTree* Htree, GdsStructure* parent)
{
   // collecting hierarchical information
   Htree = DEBUG_NEW GDSHierTree(this, parent, Htree);
   for (unsigned i = 0; i < _children.size(); i++)
      if (NULL == _children[i]) continue;
      else
      {
         Htree = _children[i]->hierOut(Htree,this);
         // Collect all used layers here and down in hierarchy
         for(int j = 1 ; j < GDS_MAX_LAYER ; j++)
            _allLay[j] |= _children[i]->allLay(j);
      }
   return Htree;
}

GDSin::GdsStructure::~GdsStructure()
{
   for (LayMap::iterator CL = _layers.begin(); CL != _layers.end(); CL++)
   {
      GdsData* wData  = CL->second;
      GdsData* wData2;
      while (wData)
      {
         wData2 = wData->last();
         delete wData;
         wData = wData2;
      }
   }
}

//==============================================================================
// class GdsData
//==============================================================================
GDSin::GdsData::GdsData() :
   _last(NULL), _plex(0), _elflags(0), _singleType(-1)
{}

void GDSin::GdsData::readPlex(GdsRecord *cr)
{
   cr->retData(&_elflags,0,16);//get two bytes bit-array
}

void GDSin::GdsData::readElflags(GdsRecord *cr)
{
   cr->retData(&_plex);//get two bytes bit-array
}

//==============================================================================
// class GdsBox
//==============================================================================
GDSin::GdsBox::GdsBox(GdsFile* cf, int2b& layer) : GdsData()
{
   GdsRecord* cr = NULL;
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_ELFLAGS:readElflags(cr);// seems that it's not used
               delete cr; break;
            case gds_PLEX:   readPlex(cr);// seems that it's not used
               delete cr; break;
            case gds_LAYER: cr->retData(&layer);
               delete cr; break;
            case gds_BOXTYPE:cr->retData(&_singleType);
               delete cr; break;
            case gds_PROPATTR:
               InFile->incGdsiiWarnings(); delete cr; break;
            case gds_PROPVALUE: tell_log(console::MT_WARNING, "GDS box - PROPVALUE record ignored");
               InFile->incGdsiiWarnings(); delete cr; break;
            case gds_XY: {
               word numpoints = (cr->recLen())/8 - 1;
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
               tell_log(console::MT_ERROR, "GDS box - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
            }
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

//==============================================================================
// class GdsPolygon
//==============================================================================
GDSin::GdsPolygon::GdsPolygon(GdsFile* cf, int2b& layer) : GdsData()
{
   word i;
   GdsRecord* cr = NULL;
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_ELFLAGS: readElflags(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:   readPlex(cr);// seems that it's not used
               delete cr;break;
            case gds_LAYER: cr->retData(&layer);
               delete cr;break;
            case gds_DATATYPE: cr->retData(&_singleType);
               delete cr;break;
            case gds_PROPATTR: tell_log(console::MT_WARNING,"GDS boundary - PROPATTR record ignored");
               InFile->incGdsiiWarnings(); delete cr;break;
            case gds_PROPVALUE: tell_log(console::MT_WARNING,"GDS boundary - PROPVALUE record ignored");
               InFile->incGdsiiWarnings(); delete cr;break;
            case gds_XY: _numpoints = (cr->recLen())/8 - 1;
               // one point less because fist and last point coincide
               _plist.reserve(_numpoints);
               for(i = 0; i < _numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr; break;
            case gds_ENDEL://end of element, exit point
               delete cr;return;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS boundary - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

// laydata::tdtdata* GDSin::GdsPolygon::toTED() {
//    return NULL;
// }

//==============================================================================
// class GDSpath
//==============================================================================
GDSin::GDSpath::GDSpath(GdsFile* cf, int2b& layer):GdsData()
{
   word i;
   _pathtype = 0; _bgnextn = 0; _endextn = 0; _width = 0;
   GdsRecord* cr = NULL;
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_ELFLAGS: readElflags(cr);// seems that's not used
               delete cr;break;
            case gds_PLEX:   readPlex(cr);// seems that's not used
               delete cr;break;
            case gds_LAYER: cr->retData(&layer);
               delete cr;break;
            case gds_DATATYPE: cr->retData(&_singleType);
               delete cr;break;
            case gds_PATHTYPE: cr->retData(&_pathtype);
               delete cr;break;
            case gds_WIDTH: cr->retData(&_width);
               delete cr;break;
            case gds_BGNEXTN:   cr->retData(&_bgnextn);
               delete cr;break;
            case gds_ENDEXTN:   cr->retData(&_endextn);
               delete cr;break;
            case gds_PROPATTR: tell_log(console::MT_WARNING,"GDS path - PROPATTR record ignored");
               InFile->incGdsiiWarnings(); delete cr;break;
            case gds_PROPVALUE: tell_log(console::MT_WARNING,"GDS path - PROPVALUE record ignored");
               InFile->incGdsiiWarnings(); delete cr;break;
            case gds_XY:_numpoints = (cr->recLen())/8;
               _plist.reserve(_numpoints);
               for(i = 0; i < _numpoints; i++)  _plist.push_back(GDSin::get_TP(cr, i));
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               if (2 == _pathtype)
               {
                  tell_log(console::MT_INFO,"GDS Pathtype 2 digitized. Will be converted to Pathtype 0");
                  convert22(_width/2, _width/2);
               }
               else if (4 == _pathtype)
               {
                  tell_log(console::MT_INFO,"GDS Pathtype 4 digitized. Will be converted to Pathtype 0");
                  convert22(_bgnextn, _endextn);
               }
               delete cr;return;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS path - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (cr->recType() != gds_ENDEL);
}

void GDSin::GDSpath::convert22(int4b begext, int4b endext)
{
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
   P1 = _plist[_numpoints-2];
   P2 = _plist[_numpoints-1];
   sdX = P2.x() - P1.x();
   sdY = P2.y() - P1.y();
   sign = ((sdX * sdY) >= 0) ? 1 : -1;
   length = sqrt(sdY*sdY + sdX*sdX);
   int4b yn = (int4b) rint(P2.y() + sign*((endext*sdY)/length));
   int4b xn = (int4b) rint(P2.x() + sign*((endext*sdX)/length));
   _plist[0].setX(x0);
   _plist[0].setY(y0);
   _plist[_numpoints-1].setX(xn);
   _plist[_numpoints-1].setY(yn);
}

//==============================================================================
// class GdsText
//==============================================================================
GDSin::GdsText::GdsText(GdsFile* cf, int2b& layer):GdsData()
{
   word ba;
   // initializing
   _font = 0; _vertJust = 0; _horiJust = 0; _pathType = 0;
   _width = 0; _absMagn = 0; _absAngl = 0; _reflection = 0;
   _magnification = 1.0; _angle = 0.0;
   _text[0] = 0x0;
   GdsRecord* cr = NULL;
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_ELFLAGS: readElflags(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:   readPlex(cr);// seems that it's not used
               delete cr;break;
            case gds_LAYER: cr->retData(&layer);
               delete cr;break;
            case gds_TEXTTYPE: cr->retData(&_singleType);
               delete cr;break;
            case gds_PATHTYPE: cr->retData(&_pathType);// ??? for test ???
               delete cr;break;
            case gds_WIDTH: cr->retData(&_width);// seems not to be used
               delete cr;break;
            case gds_PROPATTR: tell_log(console::MT_WARNING,"GDS text - PROPATTR record ignored");
               InFile->incGdsiiWarnings(); delete cr;break;
            case gds_PROPVALUE: tell_log(console::MT_WARNING,"GDS text - PROPVALUE record ignored");
               InFile->incGdsiiWarnings(); delete cr;break;
            case gds_PRESENTATION:
               cr->retData(&ba,0,16);
               _font = ba & 0x0030; _font >>= 4;
               _vertJust = ba & 0x000C;_vertJust >>= 2;
               _horiJust = ba & 0x0003;
               delete cr;break;
            case gds_STRANS:
               cr->retData(&ba,0,16);
               _reflection = ba & 0x8000; _reflection >>= 15;//bit 0
               _absMagn = ba & 0x0004; _absMagn >>= 2; //bit 13
               _absAngl = ba & 0x0002; _absAngl >>= 1; //bit 14
               delete cr; break;
            case gds_MAG: cr->retData(&_magnification);
               delete cr; break;
            case gds_ANGLE: cr->retData(&_angle);
               delete cr; break;
            case gds_XY: _magnPoint = GDSin::get_TP(cr);
               delete cr;break;
            case gds_STRING: cr->retData(&_text);
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               delete cr;return;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS text - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

//==============================================================================
// class GdsRef
//==============================================================================
GDSin::GdsRef::GdsRef() : GdsData(), _refStr(NULL),
   _reflection(false), _magnPoint(TP()), _magnification(1.0), _angle(0.0),
   _absMagn(false), _absAngl(false)
{
   _strName[0] = 0x0;
}

GDSin::GdsRef::GdsRef(GdsFile* cf) : GdsData()
{
   word ba;
   //initializing
   _absAngl = _absMagn = _reflection = false;
   _refStr = NULL;
   _magnification = 1.0; _angle = 0.0;
   int tmp; //Dummy variable. Use for gds_PROPATTR
   char tmp2[128]; //Dummy variable. Use for gds_PROPVALUE
   std::ostringstream ost;
   GdsRecord* cr = NULL;
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_ELFLAGS: readElflags(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:   readPlex(cr); // seems that it's not used
               delete cr;break;
            case gds_SNAME:
               if (cr->recLen() > 32)   _strName[0] = 0x0;
               else cr->retData(&_strName);
               delete cr;break;
            case gds_STRANS:
               cr->retData(&ba,0,16);
               _reflection = ba & 0x8000;//mask all bits except 0
               _absMagn = ba & 0x0004;//mask all bits except 13
               _absAngl = ba & 0x0002;//mask all bits except 14
               delete cr; break;
            case gds_MAG: cr->retData(&_magnification);
               delete cr; break;
            case gds_ANGLE: cr->retData(&_angle);
               delete cr; break;
            case gds_XY: _magnPoint = GDSin::get_TP(cr);
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               // before exiting, init Current Translation Matrix
//               tmtrx = DEBUG_NEW PSCTM(magn_point,magnification,angle,reflection);
               delete cr;return;
            case gds_PROPATTR://@TODO Not implemented yet+++++
               cr->retData(&tmp);
               delete cr; break;
            case gds_PROPVALUE: //@TODO Not implemented yet+++++
               cr->retData(&tmp2);
               ost << "Property attribute  " << tmp << " with value \"" << tmp2 << "\" ignored" ; break;
               tell_log(console::MT_WARNING, ost.str());
               InFile->incGdsiiWarnings();
               delete cr; break;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS sref - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

// laydata::tdtdata* GDSin::GdsRef::toTED() {
//    return NULL;
// }

//==============================================================================
// class GdsARef
//==============================================================================
GDSin::GdsARef::GdsARef(GdsFile* cf):GdsRef()
{
   word ba;
   int tmp; //Dummy variable. Use for gds_PROPATTR
   char tmp2[128]; //Dummy variable. Use for gds_PROPVALUE
   std::ostringstream ost;
   //initializing
   GdsRecord* cr = NULL;
   do
   {//start reading
      cr = cf->getNextRecord();
      if (cr)
      {
         switch (cr->recType())
         {
            case gds_ELFLAGS:readElflags(cr);// seems that it's not used
               delete cr;break;
            case gds_PLEX:readPlex(cr);// seems that it's not used
               delete cr;break;
            case gds_SNAME:
               if (cr->recLen() > 32)   _strName[0] = 0x0;
               else cr->retData(&_strName);
               delete cr;break;
            case gds_STRANS:
               cr->retData(&ba,0,16);
               _reflection = ba & 0x8000;//mask all bits except 0
               _absMagn = ba & 0x0004;//mask all bita except 13
               _absAngl = ba & 0x0002;//mask all bita except 14
               delete cr; break;
            case gds_MAG: cr->retData(&_magnification);
               delete cr; break;
            case gds_ANGLE: cr->retData(&_angle);
               delete cr; break;
            case gds_XY:
               _magnPoint = GDSin::get_TP(cr,0);
               _xStep = GDSin::get_TP(cr,1);
               _yStep = GDSin::get_TP(cr,2);
               delete cr;break;
            case gds_COLROW://return number of columns & rows in the array 
               cr->retData(&_columns);
               cr->retData(&_rows,2);
               delete cr;break;
            case gds_ENDEL://end of element, exit point
               // before exiting, init Current Translation Matrix
//               tmtrx = DEBUG_NEW PSCTM(magn_point,magnification,angle,reflection);
               delete cr;return;
            case gds_PROPATTR://TODO not implemented yet+++++
               cr->retData(&tmp);
               delete cr; break;
            case gds_PROPVALUE://TODO not implemented yet+++++
               cr->retData(&tmp2);
               ost << "Property attribute  " << tmp << " with value \"" << tmp2 << "\" ignored" ; break;
               tell_log(console::MT_WARNING, ost.str());
               InFile->incGdsiiWarnings();
               delete cr; break;
            default://parse error - not expected record type
               tell_log(console::MT_ERROR, "GDS aref - wrong record type in the current context");
               InFile->incGdsiiErrors();
               delete cr;return;
         }
      }
      else
      {
         tell_log(console::MT_ERROR, "Unexpected end of file");
         InFile->incGdsiiErrors();
         return;
      }
   }
   while (true);
}

int GDSin::GdsARef::getXStep()
{
   int ret = (int) sqrt(pow(float((_xStep.x() - _magnPoint.x())),2) +
                        pow(float((_xStep.y() - _magnPoint.y())),2)   ) / _columns;
   return ret;
}

int GDSin::GdsARef::getYStep()
{
   int ret = (int) sqrt(pow(float((_yStep.x() - _magnPoint.x())),2) +
                        pow(float((_yStep.y() - _magnPoint.y())),2)   ) / _rows;
   return ret;
}

//-----------------------------------------------------------------------------
/*void GDSin::PrintChildren(GDSin::GDSHierTree* parent, std::string* tabnum){
   GdsStructure* cs = parent->GetItem();
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
TP GDSin::get_TP(GDSin::GdsRecord *cr, word curnum, byte len)
{
   int4b GDS_X, GDS_Y;
   cr->retData(&GDS_X, curnum*len*2, len);
   cr->retData(&GDS_Y, curnum*len*2+len, len);
   return TP(GDS_X,GDS_Y);
}

//=============================================================================
GDSin::LayerMapGds::LayerMapGds(GdsLayers* alist) : _theMap(), _status(true), _alist(alist)
{
   assert(NULL != alist);
   for (GdsLayers::const_iterator NLI = alist->begin(); NLI != alist->end(); NLI++)
   {
      for (WordList::const_iterator NTI = NLI->second.begin(); NTI != NLI->second.end(); NTI++)
         _theMap[NLI->first].insert(std::make_pair(*NTI, NLI->first));
   }
}

GDSin::LayerMapGds::LayerMapGds(const USMap& inlist, GdsLayers* alist)
   : _theMap(), _status(true), _alist(alist)
{
   for (USMap::const_iterator CE = inlist.begin(); CE != inlist.end(); CE++)
   {
      wxString exp(CE->second.c_str(), wxConvUTF8);
      patternNormalize(exp);
      _status &= parseLayTypeString(exp, CE->first);
   }
}

bool GDSin::LayerMapGds::parseLayTypeString(wxString& exp, word tdtLay)
{
   const wxString tmplLayNumbers    = wxT("[[:digit:]\\,\\-]*");
   const wxString tmplTypeNumbers   = wxT("[[:digit:]\\,\\-]*|\\*");


   wxRegEx src_tmpl(tmplLayNumbers+wxT("\\;")+tmplTypeNumbers); VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp))
   {
      wxString wxmsg;
      wxmsg << wxT("Can't make sence from the string \"") << exp << wxT("\"");
      std::string msg(wxmsg.mb_str(wxConvUTF8));  
      tell_log(console::MT_ERROR,msg);
      return false;
   }
   //separate the layer expression from data type expression
   src_tmpl.Compile(tmplLayNumbers+wxT("\\;")); VERIFY(src_tmpl.IsValid());
   src_tmpl.Matches(exp);
   wxString lay_exp = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   wxString type_exp = exp;
   // we need to remove the ';' separator that left in the lay_exp
   src_tmpl.Compile(wxT("\\;")); VERIFY(src_tmpl.IsValid());
   src_tmpl.Matches(exp);
   src_tmpl.ReplaceFirst(&lay_exp,wxT(""));

   WordList llst;
   // get the listed layers
   getList(  lay_exp , llst);
   // now for every listed layer
   for ( WordList::const_iterator CL = llst.begin(); CL != llst.end(); CL++ )
   {
      if (NULL != _alist)
      {// GDS to TDT conversion
         // if the layer is listed among the GDS used layers
         if ( _alist->end() != _alist->find(*CL) )
         {
            if (wxT('*') == type_exp)
            { // for all data types
               // get all GDS data types for the current layer and copy them
               // to the map
               for ( WordList::const_iterator CT = (*_alist)[*CL].begin(); CT != (*_alist)[*CL].end(); CT++)
                  _theMap[*CL].insert(std::make_pair(*CT, tdtLay));
            }
            else
            {// for particular (listed) data type
               WordList dtlst;
               getList( type_exp , dtlst);
               for ( WordList::const_iterator CT = dtlst.begin(); CT != dtlst.end(); CT++)
                  _theMap[*CL].insert(std::make_pair(*CT, tdtLay));
            }
         }
         // else ignore the mapped GDS layer
      }
      else
      {// TDT tp GDS conversion
         if (wxT('*') == type_exp)
         { // for all data types
            // get all GDS data types for the current layer and copy them
            // to the map
            _theMap[*CL].insert(std::make_pair(0, tdtLay));
         }
         else
         {// for particular (listed) data type
            WordList dtlst;
            getList( type_exp , dtlst);
//            for ( WordList::const_iterator CT = dtlst.begin(); CT != dtlst.end(); CT++)
            //@TODO Wqarning message
            _theMap[tdtLay].insert(std::make_pair(dtlst.front(), *CL));
         }
      }
   }
   return true;
}

void GDSin::LayerMapGds::patternNormalize(wxString& str)
{
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before separators
   VERIFY(regex.Compile(wxT("([[:space:]])([\\-\\;\\,])")));
   regex.ReplaceAll(&str,wxT("\\2"));
   // remove spaces after separators
   VERIFY(regex.Compile(wxT("([\\-\\;\\,])([[:space:]])")));
   regex.ReplaceAll(&str,wxT("\\1"));

}

void GDSin::LayerMapGds::getList(wxString& exp, WordList& data)
{
   wxRegEx number_tmpl(wxT("[[:digit:]]*"));
   wxRegEx separ_tmpl(wxT("[\\,\\-]{1,1}"));
   unsigned long conversion;
   bool last_was_separator = true;
   char separator = ',';
   VERIFY(number_tmpl.IsValid());
   VERIFY(separ_tmpl.IsValid());

   do
   {
      if (last_was_separator)
      {
         number_tmpl.Matches(exp);
         number_tmpl.GetMatch(exp).ToULong(&conversion);
         number_tmpl.ReplaceFirst(&exp,wxT(""));
         if (',' == separator)
            data.push_back((word)conversion);
         else
         {
            for (word numi = data.back() + 1; numi <= conversion; numi++)
               data.push_back(numi);
         }
      }
      else
      {
         separ_tmpl.Matches(exp);
         if      (wxT("-") == separ_tmpl.GetMatch(exp))
            separator = '-';
         else if (wxT(",") == separ_tmpl.GetMatch(exp))
            separator = ',';
         else assert(false);
         separ_tmpl.ReplaceFirst(&exp,wxT(""));
      }
      last_was_separator = !last_was_separator;
   } while (!exp.IsEmpty());

}

bool GDSin::LayerMapGds::getTdtLay(word& tdtlay, word gdslay, word gdstype) const
{
   // All that this function is doing is:
   // tdtlay = _theMap[gdslay][gdstype]
   // A number of protections are in place though as well as const_cast
   tdtlay = gdslay; // the default value
   if (_theMap.end()       == _theMap.find(gdslay)       ) return false;
   GlMap::const_iterator glmap = _theMap.find(gdslay);
   if (glmap->second.end() == glmap->second.find(gdstype)) return false;
   GdtTdtMap::const_iterator tltype = glmap->second.find(gdstype);
   tdtlay = tltype->second;
   return true;
}

bool GDSin::LayerMapGds::getGdsLayType(word& gdslay, word& gdstype, word tdtlay) const
{
   // All that this function is doing is:
   // tdtlay = _theMap[gdslay][gdstype]
   // A number of protections are in place though as well as const_cast
   gdslay  = tdtlay; // the default value
   gdstype = 0;
   if (_theMap.end()       == _theMap.find(tdtlay)       ) return false;
   GlMap::const_iterator glmap = _theMap.find(tdtlay);
   if (1 != glmap->second.size())   return false;
   gdstype =  glmap->second.begin()->first;
   gdslay =   glmap->second.begin()->second;
   return true;
}
