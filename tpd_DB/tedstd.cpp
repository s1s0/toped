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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun May 22 15:43:49 BST 2005
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Basic definitions and file handling of TDT data base
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <math.h>
#include <sstream>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>
#include "tedstd.h"
#include "ttt.h"
#include "outbox.h"
#include "tedesign.h"
#include "auxdat.h"

//=============================================================================
// class InputDBFile
//=============================================================================
/*!
 * The main purpose of the constructor is to create an input stream (_inStream)
 * from the fileName. It handles zip and gz files recognizing them by the
 * extension. The outcome of this operation is indicated in the _status field
 * of the class.
 * @param fileName - the fully qualified filename - OS dependent
 */
InputDBFile::InputDBFile( const wxString& fileName, bool forceSeek) :
      _inStream      (      NULL ),
      _gziped        (     false ),
      _ziped         (     false ),
      _forceSeek     ( forceSeek ),
      _fileLength    (         0 ),
      _filePos       (         0 ),
      _progresPos    (         0 ),
      _progresMark   (         0 ),
      _progresStep   (         0 ),
      _progresDivs   (       200 ),
      _status        (     false )
{
   std::ostringstream info;
   wxFileName wxImportFN(fileName);
   wxImportFN.Normalize();
   _fileName = wxImportFN.GetFullPath();
   if (wxImportFN.IsOk() && wxImportFN.FileExists())
   {
      wxString theExtention = wxImportFN.GetExt();
      _gziped = (wxT("gz") == wxImportFN.GetExt());
      _ziped  = (wxT("zip") == wxImportFN.GetExt());
      if (_ziped)
      {
         info << "Inflating the archive \"" << _fileName << "\" ...";
         tell_log(console::MT_INFO,info.str());
         if (unZip2Temp())
         {
            // zip files are inflated in a temporary location immediately
            info.str("");
            info << "Done";
            tell_log(console::MT_INFO,info.str());
            _inStream = DEBUG_NEW wxFFileInputStream(_tmpFileName,wxT("rb"));
            _status = true;
         }
         else
         {
            info.str("");
            info << "Failed!";
            tell_log(console::MT_ERROR,info.str());
         }
      }
      else if (_gziped)
      {
         // gz files are handled "as is" in the first import stage unless _forceSeek
         // is requested
         if (_forceSeek)
         {
            if (unZlib2Temp())
            {
               _inStream = DEBUG_NEW wxFFileInputStream(_tmpFileName,wxT("rb"));
               _status = true;
            }
         }
         else
         {
            wxInputStream* fstream = DEBUG_NEW wxFFileInputStream(_fileName,wxT("rb"));
            _inStream = DEBUG_NEW wxZlibInputStream(fstream);
            _status = true;
         }
      }
      else
      {
         // File is not compressed
         _inStream = DEBUG_NEW wxFFileInputStream(_fileName,wxT("rb"));
         _status = true;
      }
   }
   else
   {
      std::ostringstream info;
      info << "Invalid filename \"" << _fileName << "\"";
      tell_log(console::MT_ERROR,info.str());
   }
   if (!_status) return;
   assert(NULL != _inStream);
   //--------------------------------------------------------------------------
   // OK, we have an input stream now - check it is valid and accessible
   if (!(_inStream->IsOk()))
   {
      info << "File "<< _fileName <<" can NOT be opened";
      _status = false;
      delete _inStream;
      _inStream = NULL;
      return;
   }
   _fileLength = _inStream->GetLength();
   _progresStep = _fileLength / _progresDivs;
   if (_progresStep > 0)
      TpdPost::toped_status(console::TSTS_PRGRSBARON, _fileLength);
}

bool InputDBFile::readStream(void* buffer, size_t len, bool updateProgress)
{
   _inStream->Read(buffer,len);// read record header
   size_t numread = _inStream->LastRead();
   if (numread != len)
      return false;// error during read in
   // update file position
   _filePos += numread;
   // update progress indicator
   _progresPos += numread;
   if (    updateProgress
       && (_progresStep > 0)
       && (_progresStep < (_progresPos - _progresMark)))
   {
      _progresMark = _progresPos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresMark);
   }
   return true;
}

size_t InputDBFile::readTextStream(char* buffer, size_t len )
{
//   size_t result = 0;
//   do
//   {
//      char cc = _inStream->GetC();
//      if (1 == _inStream->LastRead())
//      {
//         buffer[result++] = cc;
//      }
//      else
//         break;
//   } while (result < len);
//   return result;
   _inStream->Read(buffer,len);// read record header
   size_t numread = _inStream->LastRead();
   // update file position
   _filePos += numread;
   // update progress indicator
   _progresPos += numread;
   if(   (_progresStep > 0)
      && (_progresStep < (_progresPos - _progresMark)))
   {
      _progresMark = _progresPos;
      TpdPost::toped_status(console::TSTS_PROGRESS, _progresMark);
   }
   return numread;
}

void InputDBFile::closeStream()
{
   if ( NULL != _inStream )
   {
      delete _inStream;
      _inStream = NULL;
   }
   TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
//   _convLength = 0;
}

void InputDBFile::initFileMetrics(wxFileOffset size)
{
   _filePos     = 0;
   _progresPos  = 0;
   _progresMark = 0;
   _progresStep = size / _progresDivs;
   if (_progresStep > 0)
      TpdPost::toped_status(console::TSTS_PRGRSBARON, size);
}

bool InputDBFile::unZip2Temp()
{
   // Initialize an input stream - i.e. open the input file
   wxFFileInputStream inStream(_fileName);
   if (!inStream.Ok())
   {
      // input file does not exist
      return false;
   }
   // Create an input zip stream handling over the input file stream created above
   wxZipInputStream inZipStream(inStream);
   if (1 < inZipStream.GetTotalEntries()) return false;
   wxZipEntry* curZipEntry = inZipStream.GetNextEntry();
   if (NULL != curZipEntry)
   {
      wxFile* outFileHandler = NULL;
      _tmpFileName = wxFileName::CreateTempFileName(curZipEntry->GetName(), outFileHandler);
      wxFileOutputStream outStream(_tmpFileName);
      if (outStream.IsOk())
      {
         inZipStream.Read(outStream);
         return true;
      }
      else return false;
   }
   else
      return false;
}

bool InputDBFile::unZlib2Temp()
{
   std::ostringstream info;
   // Initialize an input stream - i.e. open the input file
   wxFFileInputStream inStream(_fileName);
   if (!inStream.Ok())
   {
      info << "Can't open the file " << _fileName;
      tell_log(console::MT_ERROR,info.str());
      return false;
   }
   // Create an input zlib stream handling over the input file stream created above
   wxZlibInputStream inZlibStream(inStream);
   wxFile* outFileHandler = NULL;
   _tmpFileName = wxFileName::CreateTempFileName(wxString(), outFileHandler);
   wxFileOutputStream outStream(_tmpFileName);
   if (outStream.IsOk())
   {
      info << " Inflating ... ";
      tell_log(console::MT_INFO,info.str());
      inZlibStream.Read(outStream);
      wxStreamError izlsStatus = inZlibStream.GetLastError();
      if (wxSTREAM_EOF == izlsStatus)
      {
         info.str("");
         info << " Done ";
         tell_log(console::MT_INFO,info.str());
         return true;
      }
      else
      {
         info << " Inflating finished with status " << izlsStatus << ". Can't continue";
         tell_log(console::MT_ERROR,info.str());
         return false;
      }
   }
   else
   {
      info << "Can't create a temporary file for deflating. Bailing out. ";
      tell_log(console::MT_ERROR,info.str());
      return false;
   }
}

InputDBFile::~InputDBFile()
{
   if (NULL != _inStream) delete _inStream;
}

//-----------------------------------------------------------------------------
// class InputTdtFile
//-----------------------------------------------------------------------------
InputTdtFile::InputTdtFile( wxString fileName, laydata::TdtLibDir* tedlib ) :
      // !Note forcing seekable (true) here is just to get the progress bar working
      // with compressed TDT files. The trouble is that we can't get the size of the gz
      // files without inflating them.
      InputDBFile(fileName, true),
      _TEDLIB     (tedlib)
{
   if (status())
   {
      try
      {
         getFHeader();
      }
      catch (EXPTNreadTDT&)
      {
         closeStream();
         setStatus(false);
         return;
      }
      bool versionOk = (0 ==_revision) &&
                       (9 < _subrevision) && (12 > _subrevision);
      if (!versionOk)
      {
         std::ostringstream ost;
         ost << "TDT format revision not supported: 0.10 or 0.11 expected";
         tell_log(console::MT_ERROR,ost.str());
         setStatus(versionOk);
      }
   }
}

void InputTdtFile::read(int libRef)
{
   if (tedf_DESIGN != getByte()) throw EXPTNreadTDT("Expecting DESIGN record");
   std::string name = getString();
   real         DBU = getReal();
   real          UU = getReal();
   tell_log(console::MT_DESIGNNAME, name);
   if (libRef > 0)
      _design = DEBUG_NEW laydata::TdtLibrary(name, DBU, UU, libRef, _created, _lastUpdated);
   else
      _design = DEBUG_NEW laydata::TdtDesign(name,_created, _lastUpdated, DBU,UU);
   _design->read(this);
   //Design end marker is read already in TdtDesign so don't search it here
   //byte design_end = getByte();
}

void InputTdtFile::getFHeader()
{
   // Get the leading string
   std::string _leadstr = getString();
   if (TED_LEADSTRING != _leadstr) throw EXPTNreadTDT("Bad leading record");
   getRevision();// Get format revision
   getTime();// Get file time stamps
//   checkIntegrity();
}

byte InputTdtFile::getByte()
{
   byte result;
   if (!readStream(&result,sizeof(byte), true))
      throw EXPTNreadTDT("Wrong number of bytes read");
   return result;
}

word InputTdtFile::getWord()
{
   word result;
   if (!readStream(&result,sizeof(word), true))
      throw EXPTNreadTDT("Wrong number of bytes read");
   return result;
}

LayerDef InputTdtFile::getLayer()
{
   word laynum = 0;
   word laytyp = 0;
   if (!readStream(&laynum,sizeof(word), true))
      throw EXPTNreadTDT("Wrong number of bytes read");
   if (!((_revision == 0x00) && (_subrevision < 0x0B)))
   {
      if (!readStream(&laytyp,sizeof(word), true))
         throw EXPTNreadTDT("Wrong number of bytes read");
   }
   return LayerDef(laynum,laytyp);
}

int4b InputTdtFile::get4b()
{
   int4b result;
   if (!readStream(&result,sizeof(int4b), true))
      throw EXPTNreadTDT("Wrong number of bytes read");
   return result;
}

WireWidth InputTdtFile::get4ub()
{
   WireWidth result;
   if (!readStream(&result,sizeof(WireWidth), true))
      throw EXPTNreadTDT("Wrong number of bytes read");
   return result;
}

real InputTdtFile::getReal()
{
   real result;
   if (!readStream(&result,sizeof(real), true))
      throw EXPTNreadTDT("Wrong number of bytes read");
   return result;
}

std::string InputTdtFile::getString()
{
   byte length = getByte();
   char* strc = DEBUG_NEW char[length+1];
   if (!readStream(strc,length, true))
   {
      delete[] strc;
      throw EXPTNreadTDT("Wrong number of bytes read");
   }
   strc[length] = 0x00;
   std::string str = strc;
   delete[] strc;
   return str;
}

TP InputTdtFile::getTP()
{
   int4b x = get4b();
   int4b y = get4b();
   return TP(x,y);
}

CTM InputTdtFile::getCTM()
{
   real _a  = getReal();
   real _b  = getReal();
   real _c  = getReal();
   real _d  = getReal();
   real _tx = getReal();
   real _ty = getReal();
   return CTM(_a, _b, _c, _d, _tx, _ty);
}

void InputTdtFile::getRevision()
{
   if (tedf_REVISION  != getByte()) throw EXPTNreadTDT("Expecting REVISION record");
   _revision = getWord();
   _subrevision = getWord();
   std::ostringstream ost;
   ost << "TDT format revision: " << _revision << "." << _subrevision;
   tell_log(console::MT_INFO,ost.str());
   if ((_revision != TED_CUR_REVISION) || (_subrevision > TED_CUR_SUBREVISION))
      throw EXPTNreadTDT("The TDT revision is not supported by this version of Toped");
}

void InputTdtFile::getTime()
{
   tm broken_time;
   if (tedf_TIMECREATED  != getByte()) throw EXPTNreadTDT("Expecting TIMECREATED record");
   broken_time.tm_mday = get4b();
   broken_time.tm_mon  = get4b();
   broken_time.tm_year = get4b();
   broken_time.tm_hour = get4b();
   broken_time.tm_min  = get4b();
   broken_time.tm_sec  = get4b();
   broken_time.tm_isdst = -1;
   _created = mktime(&broken_time);
   if (tedf_TIMEUPDATED  != getByte()) throw EXPTNreadTDT("Expecting TIMEUPDATED record");
   broken_time.tm_mday = get4b();
   broken_time.tm_mon  = get4b();
   broken_time.tm_year = get4b();
   broken_time.tm_hour = get4b();
   broken_time.tm_min  = get4b();
   broken_time.tm_sec  = get4b();
   broken_time.tm_isdst = -1;
   _lastUpdated = mktime(&broken_time);
}

void InputTdtFile::getCellChildNames(NameSet& cnames)
{
   // Be very very careful with the copy constructors and assignment of the
   // standard C++ lib containers. Here it seems OK.
   cnames = _childnames;
   //for (NameSet::const_iterator CN = _childnames.begin();
   //                              CN != _childnames.end() ; CN++)
   //   cnames->instert(*CN);
   _childnames.clear();
}

laydata::CellDefin InputTdtFile::linkCellRef(std::string cellname)
{
   // register the name of the referenced cell in the list of children
   _childnames.insert(cellname);
   laydata::CellMap::const_iterator striter = _design->_cells.find(cellname);
   laydata::CellDefin celldef = NULL;
   // link the cells instances with their definitions
   if (_design->_cells.end() == striter)
   {
   //   if (_design->checkCell(name))
   //   {
      // search the cell in the libraries because it's not in the DB
      if (!_TEDLIB->getLibCellRNP(cellname, celldef))
      {
         // Attention! In this case we've parsed a cell reference, before
         // the cell is defined. This might means:
         //   1. Cell is referenced, but not defined - i.e. library cell, but
         //      library is not loaded
         //   2. Circular reference ! Cell1 contains a reference of Cell2,
         //      that in turn contains a reference of Cell1. This is not allowed
         // We can not make a decision yet, because the entire file has not been
         // parsed yet. That is why we are assigning a default cell to the
         // referenced structure here in order to continue the parsing, and when
         // the entire file is parced the cell references without a proper pointer
         // to the structure need to be flaged as warning in case 1 and as error
         // in case 2.
         celldef = _TEDLIB->addDefaultCell(cellname, false);
      }
      else
         celldef->setOrphan(false);
   }
   else
   {
      celldef = striter->second;
      assert(NULL != celldef);
      celldef->setOrphan(false);
   }
   return celldef;
}

void InputTdtFile::cleanup()
{
   if (NULL != _design) delete _design;
}

//-----------------------------------------------------------------------------
// class TEDfile
//-----------------------------------------------------------------------------
OutputTdtFile::OutputTdtFile(std::string& filename, laydata::TdtLibDir* tedlib)
{ //writing
   _design = (*tedlib)();
   _revision=TED_CUR_REVISION;_subrevision=TED_CUR_SUBREVISION;
//   _TEDLIB = tedlib;
   std::string fname(convertString(filename));
   if (NULL == (_file = fopen(fname.c_str(), "wb"))) {
      std::string news = "File \"";
      news += filename.c_str(); news += "\" can not be created";
      tell_log(console::MT_ERROR,news);
      return;
   }
   putString(TED_LEADSTRING);
   putRevision();
   putTime();
   static_cast<laydata::TdtDesign*>(_design)->write(this);
   fclose(_file);
}

void OutputTdtFile::putWord(const word data) {
   fwrite(&data,2,1,_file);
}

void OutputTdtFile::putLayer(const LayerDef& laydef)
{
   word data = laydef.num();
   fwrite(&data,2,1,_file);
   data = laydef.typ();
   fwrite(&data,2,1,_file);
}

void OutputTdtFile::put4b(const int4b data) {
   fwrite(&data,4,1,_file);
}

void OutputTdtFile::put4ub(const WireWidth data) {
   fwrite(&data,4,1,_file);
}

void OutputTdtFile::putReal(const real data) {
   fwrite(&data, sizeof(real), 1, _file);
}

void OutputTdtFile::putTime()
{
   time_t ctime = _design->created();
   tm* broken_time = localtime(&ctime);
   putByte(tedf_TIMECREATED);
   put4b(broken_time->tm_mday);
   put4b(broken_time->tm_mon);
   put4b(broken_time->tm_year);
   put4b(broken_time->tm_hour);
   put4b(broken_time->tm_min);
   put4b(broken_time->tm_sec);
   //
   ctime = _design->lastUpdated();
   broken_time = localtime(&ctime);
   putByte(tedf_TIMEUPDATED);
   put4b(broken_time->tm_mday);
   put4b(broken_time->tm_mon);
   put4b(broken_time->tm_year);
   put4b(broken_time->tm_hour);
   put4b(broken_time->tm_min);
   put4b(broken_time->tm_sec);
}

void OutputTdtFile::putRevision()
{
   putByte(tedf_REVISION);
   putWord(_revision);
   putWord(_subrevision);
}

void OutputTdtFile::putTP(const TP* p)
{
   put4b(p->x()); put4b(p->y());
}

void OutputTdtFile::putCTM(const CTM matrix)
{
   putReal(matrix.a());
   putReal(matrix.b());
   putReal(matrix.c());
   putReal(matrix.d());
   putReal(matrix.tx());
   putReal(matrix.ty());
}

void OutputTdtFile::putString(std::string str)
{
//   byte len = str.length();
//   fwrite(&len, 1,1, _file);
   putByte(str.length());
   fputs(str.c_str(), _file);
}

void OutputTdtFile::registerCellWritten(std::string cellname)
{
   _childnames.insert(cellname);
}

bool OutputTdtFile::checkCellWritten(std::string cellname)
{
   if (_childnames.end() == _childnames.find(cellname))
      return false;
   else
      return true;
}

bool laydata::pathConvert(PointVector& plist, int4b begext, int4b endext )
{
   word numpoints = plist.size();
   TP P1 = plist[0];
   // find the first neighbouring point which is not equivalent to P1
   int fnbr = 1;
   while ((fnbr < numpoints) && (P1 == plist[fnbr]))
      fnbr++;
   // get out with error, because the wire has effectively a single point and there is
   // no way on earth to find out in which direction it should be expanded
   if (fnbr == numpoints) return false;
   TP P2 = plist[fnbr];

   double sdX = P2.x() - P1.x();
   double sdY = P2.y() - P1.y();
   // The sign - a bit funny way - described in layout canvas
   int sign = ((sdX * sdY) >= 0) ? 1 : -1;
   double length = sqrt(sdY*sdY + sdX*sdX);
   assert(length);
   int4b y0 = (int4b) rint(P1.y() - sign*((begext*sdY)/length));
   int4b x0 = (int4b) rint(P1.x() - sign*((begext*sdX)/length));
//
   P2 = plist[numpoints-1];
   // find the first neighboring point which is not equivalent to P1
   fnbr = numpoints - 2;
   while ((P2 == plist[fnbr]) && (fnbr >= 0))
      fnbr--;
   // assert, because if it was found above, it should exists!
   assert(fnbr >= 0);
   P1 = plist[fnbr];

   sdX = P2.x() - P1.x();
   sdY = P2.y() - P1.y();
   sign = ((sdX * sdY) >= 0) ? 1 : -1;
   length = sqrt(sdY*sdY + sdX*sdX);
   assert(length);
   int4b yn = (int4b) rint(P2.y() + sign*((endext*sdY)/length));
   int4b xn = (int4b) rint(P2.x() + sign*((endext*sdX)/length));

   plist[0].setX(x0);
   plist[0].setY(y0);
   plist[numpoints-1].setX(xn);
   plist[numpoints-1].setY(yn);

   return true;
}


//=============================================================================
/*!
 * The main purpose of the constructor is to create an input stream (_inStream)
 * from the fileName. It handles zip and gz files recognizing them by the
 * extension. The outcome of this operation is indicated in the _status field
 * of the class.
 * @param fileName - the fully qualified filename - OS dependent
 */
ForeignDbFile::ForeignDbFile(const wxString& fileName, bool forceSeek) : InputDBFile(fileName, forceSeek),
      _hierTree      (      NULL ),
      _convLength    (         0 )
{
}

bool ForeignDbFile::reopenFile()
{
   if (_gziped)
   {
      if (_forceSeek)
         // It should've been already inflated by the constructor
         _inStream = DEBUG_NEW wxFFileInputStream(_tmpFileName,wxT("rb"));
      else if (unZlib2Temp())
         _inStream = DEBUG_NEW wxFFileInputStream(_tmpFileName,wxT("rb"));
      else return false;
   }
   else if (_ziped)
      _inStream = DEBUG_NEW wxFFileInputStream(_tmpFileName,wxT("rb"));
   else
      _inStream = DEBUG_NEW wxFFileInputStream(_fileName,wxT("rb"));

   if (!(_inStream->IsOk()))
   {// open the input file
      std::ostringstream info;
      info << "File "<< _fileName <<" can NOT be reopened";
      tell_log(console::MT_ERROR,info.str());
      return false;
   }
   if (!(_inStream->IsSeekable()))
   {
      std::ostringstream info;
      info << "The input stream in not seekable. Can't continue";
      tell_log(console::MT_ERROR,info.str());
      return false;
   }
   initFileMetrics(_convLength);
   return true;
}

void ForeignDbFile::setPosition(wxFileOffset filePos)
{
   wxFileOffset result = _inStream->SeekI(filePos, wxFromStart);
   assert(wxInvalidOffset != result);
   setFilePos(filePos);
}

/*! An auxiliary method to ForeignDbFile::convertPrep implementing
 * recursive traversing of the cell hierarchy tree.
 * @param root - The root of the hierarchy to be traversed
 */
void ForeignDbFile::preTraverseChildren(const ForeignCellTree* root)
{
   const ForeignCellTree* Child = root->GetChild(TARGETDB_LIB);
   while (NULL != Child)
   {
      if ( !Child->GetItem()->traversed() )
      {
         // traverse children first
         preTraverseChildren(Child);
         ForeignCell* sstr = const_cast<ForeignCell*>(Child->GetItem());
         if (!sstr->traversed())
         {
            _convList.push_back(sstr);
            sstr->set_traversed(true);
            _convLength += sstr->strSize();
         }
      }
      Child = Child->GetBrother(TARGETDB_LIB);
   }
}

std::string ForeignDbFile::getFileNameOnly() const
{
   wxFileName fName(_fileName);
   fName.Normalize();
   assert (fName.IsOk());
   wxString name = fName.GetName();
   return std::string(name.mb_str(wxConvFile ));
}

ForeignDbFile::~ForeignDbFile()
{
   // get rid of the hierarchy tree
   const ForeignCellTree* var1 = _hierTree;
   while (var1)
   {
      const ForeignCellTree* var2 = var1->GetLast();
      delete var1; var1 = var2;
   }
}

//=============================================================================
bool ENumberLayerCM::mapTdtLay(laydata::TdtCell* dstStruct, word extLayer, word extDataType)
{
   _extLayNumber = extLayer;
   _extDataType  = extDataType;
   LayerDef  newTdtLayDef(ERR_LAY_DEF);
   if (_layMap.getTdtLay(newTdtLayDef, _extLayNumber, _extDataType))
   {
      _tdtLayNumber = newTdtLayDef;
      _tmpLayer     = dstStruct->secureUnsortedLayer(_tdtLayNumber);
      return true;
   }
   return false;
}

std::string ENumberLayerCM::printSrcLayer() const
{
   std::ostringstream ostr;
   ostr << " Layer: "                << _extLayNumber
        << " Data type: "            << _extDataType;
   return ostr.str();
}

//=============================================================================
bool ENameLayerCM::mapTdtLay(laydata::TdtCell* dstStruct, const std::string& extName)
{
   _extLayName = extName;
   ImpLayMap::const_iterator layno;
   if ( _layMap.end() != (layno = _layMap.find(_extLayName)) )
   {
      _tdtLayNumber = layno->second;
      _tmpLayer     = dstStruct->secureUnsortedLayer(_tdtLayNumber);
      return true;
   }
   return false;
}

std::string ENameLayerCM::printSrcLayer() const
{
   std::ostringstream ostr;
   ostr << " Layer: \""              << _extLayName
        << "\"";
   return ostr.str();
}
//=============================================================================
ImportDB::ImportDB(ForeignDbFile* src_lib, laydata::TdtLibDir* tdt_db, const LayerMapExt& theLayMap) :
      _src_lib       ( src_lib                                    ),
      _tdt_db        ( tdt_db                                     ),
      _dst_structure ( NULL                                       ),
      _grc_structure ( NULL                                       ),
      _dbuCoeff      ( src_lib->libUnits() / (*_tdt_db)()->DBU()  ),
      _crossCoeff    ( _dbuCoeff                                  ),
      _technoSize    ( 0.0                                        )
{
   _layCrossMap = DEBUG_NEW ENumberLayerCM(theLayMap);
}

ImportDB::ImportDB(ForeignDbFile* src_lib, laydata::TdtLibDir* tdt_db, const ImpLayMap& theLayMap, real techno) :
      _src_lib       ( src_lib                                    ),
      _tdt_db        ( tdt_db                                     ),
      _dst_structure ( NULL                                       ),
      _grc_structure ( NULL                                       ),
      _dbuCoeff      ( src_lib->libUnits() / (*_tdt_db)()->DBU()  ),
      _crossCoeff    ( _dbuCoeff                                  ),
      _technoSize    ( techno                                     )
{
   _layCrossMap = DEBUG_NEW ENameLayerCM(theLayMap);
}

void ImportDB::run(const NameList& top_str_names, bool overwrite, bool reopenFile)
{
   if (!reopenFile || (reopenFile && _src_lib->reopenFile()))
   {
      try
      {
         ForeignCellList wList = _src_lib->convList();
         for (ForeignCellList::iterator CS = wList.begin(); CS != wList.end(); CS++)
         {
            convert(*CS, overwrite);
            (*CS)->set_traversed(false); // restore the state for eventual second conversion
         }
         tell_log(console::MT_INFO, "Done");
      }
      catch (EXPTN&) {tell_log(console::MT_INFO, "Conversion aborted with errors");}
      TpdPost::toped_status(console::TSTS_PRGRSBAROFF);
      _src_lib->closeStream();
      (*_tdt_db)()->recreateHierarchy(_tdt_db);
   }
}

void ImportDB::convert(ForeignCell* src_structure, bool overwrite)
{
   std::string gname = src_structure->strctName();
   // check that destination structure with this name exists
   _dst_structure = static_cast<laydata::TdtCell*>((*_tdt_db)()->checkCell(gname));
   std::ostringstream ost;
   if (NULL != _dst_structure)
   {
      if (overwrite)
      {
         /*@TODO Erase the existing structure and convert*/
         ost << "Structure "<< gname << " should be overwritten, but cell erase is not implemented yet ...";
         tell_log(console::MT_WARNING,ost.str());
      }
      else
      {
         ost << "Structure "<< gname << " already exists. Skipped";
         tell_log(console::MT_INFO,ost.str());
      }
   }
   else
   {
      ost << "Importing " << gname << "...";
      tell_log(console::MT_INFO,ost.str());
      // first create a new cell
      _dst_structure = DEBUG_NEW laydata::TdtCell(gname);
      _grc_structure = DEBUG_NEW auxdata::GrcCell(gname);
      // call the cell converter
      src_structure->import(*this);
      // Sort the qtrees of the new cell
      bool emptyCell = _grc_structure->fixUnsorted();
      if (emptyCell)
         delete _grc_structure;
      else
         _dst_structure->addAuxRef(_grc_structure);
      _dst_structure->fixUnsorted();
      // and finally - register the cell
      (*_tdt_db)()->registerCellRead(gname, _dst_structure);
   }
}

bool ImportDB::mapTdtLayer(std::string layName)
{
   return _layCrossMap->mapTdtLay(_dst_structure, layName);
}

bool ImportDB::mapTdtLayer(word srcLayer, word srcDataType)
{
   return _layCrossMap->mapTdtLay(_dst_structure, srcLayer, srcDataType);
}

void ImportDB::addBox(const TP& p1, const TP& p2)
{
   laydata::QTreeTmp* tmpLayer = _layCrossMap->getTmpLayer();
   if ( NULL != tmpLayer )
   {
      tmpLayer->put(DEBUG_NEW laydata::TdtBox(p1,p2));
   }
}

void ImportDB::addPoly(PointVector& plist)
{
   laydata::QTreeTmp* tmpLayer = _layCrossMap->getTmpLayer();
   if ( NULL != tmpLayer )
   {
      bool boxObject;
      if (polyAcceptable(plist, boxObject /*srcLayer, srcDataType*/))
      {
         if (boxObject)  tmpLayer->put(DEBUG_NEW laydata::TdtBox(plist[0], plist[2]));
         else            tmpLayer->put(DEBUG_NEW laydata::TdtPoly(plist));
      }
      else
      {
         auxdata::QTreeTmpGrc* errlay = _grc_structure->secureUnsortedLayer(_layCrossMap->tdtLayNumber());
         errlay->put(DEBUG_NEW auxdata::TdtGrcPoly(plist));
      }
   }
}

void ImportDB::addPath(PointVector& plist, int4b width, short pathType, int4b bgnExtn, int4b endExtn)
{
   laydata::QTreeTmp* tmpLayer = _layCrossMap->getTmpLayer();
   if ( NULL != tmpLayer )
   {
      bool pathConvertResult = true;
      if      (2 == pathType)
         pathConvertResult = laydata::pathConvert(plist, width/2, width/2);
      else if (4 == pathType)
         pathConvertResult = laydata::pathConvert(plist, bgnExtn, endExtn);

      if (pathConvertResult)
      {
         if (pathAcceptable(plist, width))
            tmpLayer->put(DEBUG_NEW laydata::TdtWire(plist, width));
         else
         {
            auxdata::QTreeTmpGrc* errlay = _grc_structure->secureUnsortedLayer(_layCrossMap->tdtLayNumber());
            errlay->put(DEBUG_NEW auxdata::TdtGrcWire(plist, width));
         }
      }
      else
      {
         std::ostringstream ost;
         ost << "Invalid single point path - { "
             <<  _layCrossMap->printSrcLayer()
             << " }";
         tell_log(console::MT_ERROR, ost.str());
      }
   }
}

void ImportDB::addText(std::string tString, TP bPoint, double magnification, double angle, bool reflection)
{
   laydata::QTreeTmp* tmpLayer = _layCrossMap->getTmpLayer();
   if ( NULL != tmpLayer )
   {
      // @FIXME absolute magnification, absolute angle should be reflected somehow!!!
      tmpLayer->put(DEBUG_NEW laydata::TdtText(tString,
                                               CTM( bPoint,
                                                    magnification / ((*_tdt_db)()->UU() *  OPENGL_FONT_UNIT),
                                                    angle,
                                                    reflection
                                                  )
                                              )
                    );
   }
}

void ImportDB::addRef(std::string strctName, TP bPoint, double magnification,
                      double angle, bool reflection)
{
   // @FIXME absolute magnification, absolute angle should be reflected somehow!!!
   laydata::CellDefin strdefn = _tdt_db->linkCellRef(strctName, TARGETDB_LIB);
   _dst_structure->registerCellRef( strdefn,
                                    CTM(bPoint,
                                        magnification,
                                        angle,
                                        reflection
                                       )
                                  );
}

void ImportDB::addRef(std::string strctName, CTM location)
{
   // @FIXME absolute magnification, absolute angle should be reflected somehow!!!
   laydata::CellDefin strdefn = _tdt_db->linkCellRef(strctName, TARGETDB_LIB);
   _dst_structure->registerCellRef( strdefn, location);
}

void ImportDB::addARef(std::string strctName, TP bPoint, double magnification,
                      double angle, bool reflection, laydata::ArrayProps& aprop)
{
   // @FIXME absolute magnification, absolute angle should be reflected somehow!!!
   laydata::CellDefin strdefn = _tdt_db->linkCellRef(strctName, TARGETDB_LIB);
   _dst_structure->registerCellARef( strdefn,
                                     CTM(bPoint,
                                         magnification,
                                         angle,
                                         reflection
                                        ),
                                     aprop
                                  );
}

bool ImportDB::polyAcceptable(PointVector& plist, bool& box)
{
   laydata::ValidPoly check(plist);
   if (!check.valid())
   {
      std::ostringstream ost;
      ost << "Polygon check fails - {" << check.failType()
          << _layCrossMap->printSrcLayer()
          << " }";
      tell_log(console::MT_ERROR, ost.str());
   }
   if (check.valid())
   {
      plist = check.getValidated();
      box = check.box();
      return true;
   }
   else return false;
}

bool ImportDB::pathAcceptable(PointVector& plist, int4b width )
{
   laydata::ValidWire check(plist, width);

   if (!check.valid())
   {
      std::ostringstream ost;
      ost << "Wire check fails - {" << check.failType()
          << _layCrossMap->printSrcLayer()
          << " }";
      tell_log(console::MT_ERROR, ost.str());
      return false;
   }
   else if (laydata::shp_shortends == check.status())
   {
      //TODO - automatic conversion option
      std::ostringstream ost;
      ost << "Wire check fails - { Short end segments "
          << _layCrossMap->printSrcLayer()
          << " }";
      tell_log(console::MT_ERROR, ost.str());
      return false;
   }
   else
   {
      plist = check.getValidated();
      return true;
   }
}

ImportDB::~ImportDB()
{
   delete _layCrossMap;
}
