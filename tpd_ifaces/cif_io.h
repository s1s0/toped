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
#if !defined(CIFIO_H_INCLUDED)
#define CIFIO_H_INCLUDED

#include <string>
#include <fstream>
#include "../tpd_common/ttt.h"

int ciflex(void);
int ciferror (char *s);

namespace CIFin {

/*
A formal CIF 2.0 definition in Wirth notation is given below. Taken from 
http://ai.eecs.umich.edu/people/conway/VLSI/ImplGuide/ImplGuide.pdf
"A guide to LSI Implementation", Second Edition, Robert W. Hon Carlo H. Sequin

cifFile              = {{blank} [command] semi} endCommand {blank}.

command              = primCommand | defDeleteCommand | 
                       defStartCommand semi {{blank}[primCommand]semi} defFinishCommand.

primCommand          = polygonCommand       | boxCommand    | roundFlashCommand |
                       wireCommand          | layerCommand  | callCommand       | 
                       userExtensionCommand | commentCommand

polygonCommand       = "P" <*{blank}*> path.
boxCommand           = "B" <*{blank}*> integer sep integer sep point [sep point]
roundFlashCommand    = "R" integer sep point.
wireCommand          = "W" <*{blank}*> integer sep path.
layerCommand         = "L" {blank} shortname.
defStartCommand      = "D" {blank} "S" integer [sep integer sep integer].
defFinishCommand     = "D" {blank} "F".
defDeleteCommand     = "D" {blank} "D" integer.
callCommand          = "C" <*{blank}*> integer transformation.
userExtensionCommand = digit userText
commentCommand       = "(" commentText ")".
endCommand           = "E".

transformation       = {{blank} ("T" <*{blank}*> point | "M" {blank} "X" | "M" {blank} "Y" | "R" <*{blank}*> point)}.
path                 = point {sep point}.
point                = sInteger sep sInteger.
sInteger             = {sep} ["-"]integerD.
integer              = {sep} integerD.
integerD             = digit{digit}.
shortname            = c[c][c][c]
c                    = digit | upperChar
userText             = {userChar}.
commentText          = {commentChar} | commentText "(" commentText ")" commentText.

semi                 = {blank};{blank}.
sep                  = upperChar | blank;
digit                = "0" | "1" | "2" | ... | "9"
upperChar            = "A" | "B" | "C" | ... | "Z"
blank                = any ASCII character except digit, upperChar, "-" , "(" , ")" , ";".
userChar             = any ASCII character except ";".
commentChar          = any ASCII character except "(" or ")".

<* ... *> -> included in the parser (or the scanner) because existing CIF files are using this syntax, but
             it breaks the formal syntax definition of CIF

===========================================================================================================
The user extensions below - as described in http://www.rulabinsky.com/cavd/text/chapb.html
(Steven M. Rubin Copyright © 1994)
===========================================================================================================

0 x y layer N name;           Set named node on specified layer and position
0V x1 y1 x2 y2 ... xn yn;     Draw vectors
2A "msg" T x y;               Place message above specified location
2B "msg" T x y;               Place message below specified location
2C "msg" T x y;               Place message centered at specified location
2L "msg" T x y;               Place message left of specified location
2R "msg" T x y;               Place message right of specified location
4A lowx lowy highx highy;     Declare cell boundary
4B instancename;              Attach instance name to cell
4N signalname x y;            Labels a signal at a location
9  cellname;                  Declare cell name
91 instancename;              Attach instance name to cell
94 label x y;                 Place label in specified location
95 label length width x y;    Place label in specified area

*/
   typedef enum {
      cif_BOX        ,
      cif_POLY       ,
      cif_WIRE       ,
      cif_REF        ,
      cif_LBL_LOC    ,
      cif_LBL_SIG
   } CifDataType;

   typedef enum {
      cfs_POK        , // parsed OK
      cfs_FNF        , // file not found
      cfs_ERR          // error during parsing
   } CifStatusType;

   class CifStructure;
   class CifFile;

   typedef SGHierTree<CifStructure>       CIFHierTree;
   typedef std::list<CifStructure*>       CIFSList;


   class CifData {
      public:
                             CifData(CifData* last) : _last(last) {};
         virtual            ~CifData(){};
         CifData*            last()         {return _last;}
         virtual CifDataType dataType() = 0;
      protected:
         CifData*    _last;
   };

   class CifBox : public CifData {
      public:
                     CifBox(CifData*, _dbl_word, _dbl_word, TP*, TP*);
                    ~CifBox();
         CifDataType dataType()     {return cif_BOX;}
         _dbl_word   length()       {return _length;}
         _dbl_word   width()        {return _width;}
         TP*         center()       {return _center;}
         TP*         direction()    {return _direction;}
      protected:
         _dbl_word   _length;
         _dbl_word   _width;
         TP*         _center;
         TP*         _direction;
   };

   class CifPoly : public CifData {
      public:
                     CifPoly(CifData* last, pointlist*);
                    ~CifPoly();
         CifDataType dataType()     {return cif_POLY;}
         pointlist*  poly()         {return _poly;}
      protected:
         pointlist*  _poly;
   };

   class CifWire : public CifData {
      public:
                     CifWire(CifData* last, pointlist*, _dbl_word);
                    ~CifWire();
         CifDataType dataType()     {return cif_WIRE;}
         pointlist*  poly()         {return _poly;}
         _dbl_word   width()        {return _width;}
      protected:
         pointlist*  _poly;
         _dbl_word   _width;
   };

   class CifRef : public CifData {
      public:
                     CifRef(CifData* last, _dbl_word, CTM*);
                    ~CifRef();
         CifRef*     last()                           {return static_cast<CifRef*>(CifData::last());}
         _dbl_word   cell()                           {return  _cell;}
         CTM*        location()                       {return  _location;}
         CifDataType dataType()                       {return  cif_REF;}
      protected:
         _dbl_word   _cell;
         CTM*        _location;
   };

   class CifLabelLoc : public CifData {
      public:
                     CifLabelLoc(CifData*, std::string, TP*);
         virtual   ~CifLabelLoc();
         CifDataType dataType()                       {return cif_LBL_LOC;}
         std::string text() const                     {return _label;}
         const TP*   location() const                 {return _location;}

      protected:
         std::string _label;
         TP*         _location;
   };

   class CifLabelSig : public CifLabelLoc {
      public:
                     CifLabelSig(CifData*, std::string, TP*);
                    ~CifLabelSig() {}
         CifDataType dataType()                       {return cif_LBL_SIG;}
   };

   class CifLayer {
      public:
                        CifLayer(std::string name, CifLayer* last);
                       ~CifLayer();
         std::string    name()                        {return _name;}
         CifLayer*      last()                        {return _last;}
         CifData*       firstData()                   {return _first;}
         void           addBox(_dbl_word, _dbl_word, TP*, TP* direction = NULL);
         void           addPoly(pointlist* poly);
         void           addWire(pointlist* poly, _dbl_word width);
         void           addLabelLoc(std::string, TP*);
         void           addLabelSig(std::string, TP*);
      private:
         std::string    _name;
         CifLayer*      _last;
         CifData*       _first;
   };

   typedef std::list<CifLayer*>     CifLayerList;

   class CifStructure  {
      public:
                        CifStructure(_dbl_word, CifStructure*, _dbl_word=1,_dbl_word=1);
                       ~CifStructure();
         void           cellNameIs(std::string name)  {_name = name;}
         void           cellOverlapIs(TP* bl, TP* tr) {_overlap = DBbox(*bl, *tr);}
         CifStructure*  last() const                  {return _last;}
         _dbl_word      ID() const                    {return _ID;}
         std::string    name() const                  {return _name;}
         void           parentFound()                 {_orphan = false;}
         bool           orphan()                      {return _orphan;}
         bool           traversed() const             {return _traversed;}
         void           set_traversed(bool trv)       { _traversed = trv;}
         CifLayer*      firstLayer()                  {return _first;}
         CifRef*        refirst()                     {return _refirst;}
         real           a()                           {return (real)_a;}
         real           b()                           {return (real)_b;}
         CifLayer*      secureLayer(std::string);
         void           addRef(_dbl_word cell, CTM* location);
         void           collectLayers(nameList&, bool);
         void           hierPrep(CifFile&);
         CIFHierTree*   hierOut(CIFHierTree*, CifStructure*);
      // to cover the requirements of the hierarchy template
         int            libID() const                 {return TARGETDB_LIB;}
      private:
         _dbl_word      _ID;
         CifStructure*  _last;
         _dbl_word      _a;
         _dbl_word      _b;
         std::string    _name;
         CifLayer*      _first;
         CifRef*        _refirst;
         DBbox          _overlap;
         bool           _orphan;
         bool           _traversed;       //! For hierarchy traversing purposes
         CIFSList       _children;
   };

   class   CifFile {
      public:
                        CifFile(std::string);
                        ~CifFile();
         CifStatusType  status() {return _status;}
         void           addStructure(_dbl_word, _dbl_word = 1, _dbl_word = 1);
         void           doneStructure();
         void           addBox(_dbl_word, _dbl_word, TP*, TP* direction = NULL);
         void           addPoly(pointlist*);
         void           addWire(pointlist*, _dbl_word);
         void           addRef(_dbl_word, CTM*);
         void           addLabelLoc(char*, TP*, char* layname = NULL);
         void           addLabelSig(char*, TP*);
         void           secureLayer(char*);
         void           curCellName(char*);
         void           curCellOverlap(TP*, TP*);
         void           collectLayers(nameList&);
         CifStructure*  getStructure(_dbl_word);
         CifStructure*  getStructure(std::string);
         void           hierPrep();
         void           hierOut();
         std::string    Get_libname() const  {return _fileName;}
         CIFHierTree*   hiertree()           {return _hierTree;}
         CifStructure*  getFirstStructure()  {return _first;}
         CifStructure*  getTopStructure()    {return _default;}
         void           closeFile();
      protected:
         FILE*          _cifFh;
         CifStatusType  _status;          //!
         CifStructure*  _first;           //! poiter to the first defined cell
         CifStructure*  _current;         //! the working (current) cell
         CifStructure*  _default;         //! pointer to the default cell - i.e. the scratch pad
         CifLayer*      _curLay;          //!
         CIFHierTree*   _hierTree;        //! Tree of instance hierarchy
         std::string    _fileName;        //! Input CIF file - including the path
   };

   class CifExportFile {
      public:
                        CifExportFile(std::string, USMap*, bool);
                       ~CifExportFile();
         void           definitionStart(std::string, real);
         void           definitionFinish();
         bool           layerSpecification(word);
         void           box(const unsigned, const unsigned, const TP&);
         void           polygon(const pointlist&);
         void           wire(const unsigned, const pointlist&);
         void           text(const std::string&, const TP&);
         void           call(const std::string& name, const CTM&);
         bool           checkCellWritten(std::string) const;
         void           registerCellWritten(std::string);
         std::fstream&  file()               {return _file;}
      private:
         USMap*         _laymap;          //! Toped-CIF layer map
         SIMap          _cellmap;         //! tdt-cif map of all exported cells
         std::string    _fileName;        //! Output CIF file name - including the path
         std::fstream   _file;            //! Output file handler
         bool           _verbose;         //! CIF output type
         unsigned       _lastcellnum;     //! The number of the last written cell
   };

}

#endif // !defined(CIFIO_H_INCLUDED)
