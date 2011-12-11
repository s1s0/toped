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
#include "ttt.h"
#include "quadtree.h"
//#include "tedstd.h"

int ciflex(void);
int ciferror (const char *s);

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
      cfs_POK        , // parsed OK
      cfs_FNF        , // file not found
      cfs_ERR          // error during parsing
   } CifStatusType;

   class CifStructure;
   class CifFile;

   typedef std::list<CifStructure*>       CIFSList;


   class CifData {
      public:
                             CifData(CifData* last) : _last(last) {};
         virtual            ~CifData(){};
         const CifData*      last() const        {return _last;}
         virtual void        import ( ImportDB& iDB ) const = 0;
      protected:
         CifData*            _last;
   };

   class CifBox : public CifData {
      public:
                             CifBox(CifData*, dword, dword, TP*, TP*);
         virtual            ~CifBox();
         virtual void        import ( ImportDB& iDB ) const;
      protected:
         dword               _length;
         dword               _width;
         TP*                 _center;
         TP*                 _direction;
   };

   class CifPoly : public CifData {
      public:
                             CifPoly(CifData* last, PointVector*);
         virtual            ~CifPoly();
         virtual void        import( ImportDB& iDB ) const;
      protected:
         PointVector*        _poly;
   };

   class CifWire : public CifData {
      public:
                             CifWire(CifData* last, PointVector*, dword);
         virtual            ~CifWire();
         virtual void        import( ImportDB& iDB ) const;
      protected:
         PointVector*        _poly;
         dword               _width;
   };

   class CifRef : public CifData {
      public:
                             CifRef(CifData* last, dword, CTM*);
         virtual            ~CifRef();
         const CifRef*       last() const        {return static_cast<const CifRef*>(CifData::last());}
         dword               cell() const        {return  _cell;}
         const CTM*          location() const    {return  _location;}
         virtual void        import ( ImportDB& iDB ) const;
      protected:
         dword               _cell;
         CTM*                _location;
   };

   class CifLabelLoc : public CifData {
      public:
                             CifLabelLoc(CifData*, std::string, TP*);
         virtual            ~CifLabelLoc();
         virtual void        import( ImportDB& iDB ) const;
      protected:
         std::string         _label;
         TP*                 _location;
   };

   class CifLabelSig : public CifLabelLoc {
      public:
                             CifLabelSig(CifData*, std::string, TP*);
         virtual            ~CifLabelSig() {}
         virtual void        import( ImportDB& iDB ) const;
   };

   class CifLayer {
      public:
                        CifLayer(std::string name, CifLayer* last);
                       ~CifLayer();
         std::string    name() const                  {return _name;}
         const CifLayer* last() const                 {return _last;}
         const CifData* firstData() const             {return _first;}
         void           addBox(dword, dword, TP*, TP* direction = NULL);
         void           addPoly(PointVector* poly);
         void           addWire(PointVector* poly, dword width);
         void           addLabelLoc(std::string, TP*);
         void           addLabelSig(std::string, TP*);
      private:
         std::string    _name;
         CifLayer*      _last;
         CifData*       _first;
   };

   typedef std::list<CifLayer*>     CifLayerList;

   class CifStructure : public ForeignCell  {
      public:
                        CifStructure(dword, CifStructure*, dword=1, dword=1);
         virtual       ~CifStructure();
         void           cellOverlapIs(TP* bl, TP* tr) {_overlap = DBbox(*bl, *tr);}
         CifStructure*  last() const                  {return _last;}
         dword          ID() const                    {return _ID;}
         const CifLayer* firstLayer() const            {return _first;}
         const CifRef*  refirst() const               {return _refirst;}
         real           a() const                     {return (real)_a;}
         real           b() const                     {return (real)_b;}
         CifLayer*      secureLayer(std::string);
         void           addRef(dword cell, CTM* location);
         void           collectLayers(NameList&, bool) const;
         void           linkReferences(CifFile&);
         ForeignCellTree* hierOut(ForeignCellTree*, CifStructure*);
         virtual void   import(ImportDB&);
      private:
         dword          _ID;
         CifStructure*  _last;
         dword          _a;
         dword          _b;
         CifLayer*      _first;
         CifRef*        _refirst;
         DBbox          _overlap;
         CIFSList       _children;
   };

   class   CifFile : public ForeignDbFile {
      public:
                              CifFile(wxString);
         virtual             ~CifFile();
         void                 addStructure(dword, dword = 1, dword = 1);
         void                 doneStructure();
         void                 addBox(dword, dword, TP*, TP* direction = NULL);
         void                 addPoly(PointVector*);
         void                 addWire(PointVector*, dword);
         void                 addRef(dword, CTM*);
         void                 addLabelLoc(char*, TP*, char* layname = NULL);
         void                 addLabelSig(char*, TP*);
         void                 secureLayer(char*);
         void                 curCellName(char*);
         void                 curCellOverlap(TP*, TP*);
         CifStructure*        getStructure(dword);
         const CifStructure*  getStructure(const std::string&) const;

         virtual double       libUnits() const {return 1e-8;}
         virtual void         hierOut();
         virtual std::string  libname() const {return getFileNameOnly();}
         virtual void         getTopCells(NameList&) const;
         virtual void         getAllCells(wxListBox&) const;
         virtual void         convertPrep(const NameList&, bool);
         virtual void         collectLayers(NameList&) const;
         virtual bool         collectLayers(const std::string&, NameList& ) const;

      protected:
         void                 linkReferences();
         CifStructure*        _first;           //! pointer to the first defined cell
         CifStructure*        _current;         //! the working (current) cell
         CifStructure*        _default;         //! pointer to the default cell - i.e. the scratch pad
         CifLayer*            _curLay;          //!
   };

   class CifExportFile : public DbExportFile {
      public:
                        CifExportFile(std::string, laydata::TdtCell*, USMap*, bool, bool);
         virtual       ~CifExportFile();
         virtual void   definitionStart(std::string);
         virtual void   definitionFinish();
         virtual void   libraryStart(std::string, TpdTime&, real, real);
         virtual void   libraryFinish();
         virtual bool   layerSpecification(unsigned);
         virtual void   box(const int4b* const);
         virtual void   polygon(const int4b* const, unsigned);
         virtual void   wire(const int4b* const, unsigned, unsigned);
         virtual void   text(const std::string&, const CTM&);
         virtual void   ref(const std::string&, const CTM&);
         virtual void   aref(const std::string&, const CTM&, const laydata::ArrayProps&);
         virtual bool   checkCellWritten(std::string) const;
         virtual void   registerCellWritten(std::string);
      private:
         bool           pathConvert(PointVector&, unsigned, int4b );
         USMap*         _laymap;          //! Toped-CIF layer map
         SIMap          _cellmap;         //! tdt-cif map of all exported cells
         std::fstream   _file;            //! Output file handler
         bool           _verbose;         //! CIF output type
         unsigned       _lastcellnum;     //! The number of the last written cell
   };

}

#endif // !defined(CIFIO_H_INCLUDED)
