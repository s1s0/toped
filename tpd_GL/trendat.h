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
//        Created: Fri Dec 28 GMT 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Toped Rendering verteX data
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TRXDATA_H_
#define TRXDATA_H_

#include <GL/glew.h>
#include "drawprop.h"

#define TENDERER_USE_FLOATS
#ifdef TENDERER_USE_FLOATS
   #define TNDR_GLDATAT GLfloat
   #define TNDR_GLENUMT GL_FLOAT
#else
   #define TNDR_GLDATAT GLint
   #define TNDR_GLENUMT GL_INT
#endif


//=============================================================================
//
// Tesselation classes
//
//=============================================================================
typedef std::list<word> TeselVertices;

class TeselChunk {
   public:
                        TeselChunk(const TeselVertices&, GLenum, unsigned);
                        TeselChunk(const TeselChunk*, unsigned);
                        TeselChunk(const int*, unsigned, unsigned);
                        TeselChunk(const TeselChunk&); //copy constructor
                       ~TeselChunk();
      GLenum            type() const      {return _type;}
      word              size() const      {return _size;}
      const unsigned*   index_seq() const {return _index_seq;}
   private:
      unsigned*         _index_seq;  // index sequence
      word              _size;       // size of the index sequence
      GLenum            _type;
};

typedef std::list<TeselChunk> TeselChain;

class TeselTempData {
   public:
                        TeselTempData(unsigned);
                        TeselTempData(TeselChain* tc);
      void              setChainP(TeselChain* tc)  {_the_chain = tc;}
      void              newChunk(GLenum type)      {_ctype = type; _cindexes.clear();}
      void              newIndex(word vx)          {_cindexes.push_back(vx);}
      void              storeChunk();
      word              num_ftrs()                 { return _all_ftrs;}
      word              num_ftfs()                 { return _all_ftfs;}
      word              num_ftss()                 { return _all_ftss;}
   private:
      TeselChain*       _the_chain;
      GLenum            _ctype;
      TeselVertices     _cindexes;
      word              _all_ftrs;
      word              _all_ftfs;
      word              _all_ftss;
      unsigned          _offset;
};

class TessellPoly {
   public:
                        TessellPoly();
      void              tessellate(const int4b* pdata, unsigned psize);
      const TeselChain* tdata() const              { return &_tdata;  }
      word              num_ftrs() const           { return _all_ftrs;}
      word              num_ftfs() const           { return _all_ftfs;}
      word              num_ftss() const           { return _all_ftss;}
      bool              valid() const              { return (0 < (_all_ftrs + _all_ftfs + _all_ftss));}
      void              num_indexs(unsigned&, unsigned&, unsigned&) const;
      static GLUtriangulatorObj* tenderTesel; //! A pointer to the OpenGL object tesselator
#ifdef WIN32
      static GLvoid CALLBACK teselVertex(GLvoid *, GLvoid *);
      static GLvoid CALLBACK teselBegin(GLenum, GLvoid *);
      static GLvoid CALLBACK teselEnd(GLvoid *);
#else
      static GLvoid     teselVertex(GLvoid *, GLvoid *);
      static GLvoid     teselBegin(GLenum, GLvoid *);
      static GLvoid     teselEnd(GLvoid *);
#endif
   private:
      TeselChain        _tdata;
      word              _all_ftrs;
      word              _all_ftfs;
      word              _all_ftss;
};

namespace trend {

   typedef enum {lstr, llps, lnes} SlctTypes;


   //==========================================================================
   //
   // Regular objects from the DB (without texts)
   //
   //==========================================================================
   /**
      Represents convex polygons. Those are mainly the boxes from the data base
      (Note that at the time of writing all polygons in the DB are considered
      non-convex).
      Stores a REFERENCE to the vertex array (NOTE! Not the data itself!) of the
      original tdt object and its size.
      The only non-trivial method is cDataCopy which transfers the vertex data
      from the original object to a VBO mapped in the GPU buffers.
      This class is also a base for the Trx* class hierarchy.
   */
   class TrxCnvx {
      public:
                           TrxCnvx(const int4b* pdata, unsigned psize) :
                                    _cdata(pdata), _csize(psize){}
         virtual          ~TrxCnvx() {};
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual void      drctDrawContour();
         virtual void      drctDrawFill();
         unsigned          csize()     {return _csize;}
      protected:
         const int4b*      _cdata;  //! the vertexes of the object contour
         unsigned          _csize;  //! the number of vertexes in _cdata
   };

   /**
    * A specialisation of the parent class to cover the specifics of the box
    * rendering
    */
   class TrxBox : public TrxCnvx {
      public:
                           TrxBox(const int4b* pdata) : TrxCnvx(pdata, 4) {}
         virtual          ~TrxBox() {};
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual void      drctDrawContour();
         virtual void      drctDrawFill();
   };

   /**
      Represents non-convex polygons - most of the polygon objects in the DB.
      Inherits TrxCnvx. The only addition is the tesselation data (_tdata)
      which is utilised if the object is to be filled.
   */
   class TrxNcvx : public TrxCnvx {
      public:
                           TrxNcvx(const int4b* pdata, unsigned psize) :
                                    TrxCnvx(pdata, psize), _tdata(NULL) {}
         virtual          ~TrxNcvx(){};
         void              setTeselData(const TessellPoly* tdata) {_tdata = tdata;}
         virtual void      drctDrawFill();
         virtual const TeselChain* tdata()              {return _tdata->tdata();}
      private:
         const TessellPoly*    _tdata; //! polygon tesselation data
   };

   /**
      Holds the wires from the data base. This class is not quite trivial and it
      causes all kinds of troubles in the overall class hierarchy. It inherits
      TrxNcvx (is it a good idea?). Theoretically in the general case this is a
      non-convex polygon. The wire as a DB object is very specific though. Only the
      central line is stored. The contour is calculated if required on the fly from
      the build-in methods called from the constructor. Instead of using general
      purpose tesselation (slow!), the tesselation data is extracted directly from
      the contour data virtually without calculations. This is one of the few classes
      in the hierarchy which generates vertex and index data which means that it has
      to be properly cleaned-up.
   */
   class TrxWire : public TrxNcvx {
      public:
                           TrxWire(int4b*, unsigned, WireWidth, bool);
         virtual          ~TrxWire();
         void              Tesselate();
         virtual unsigned  lDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual void      drctDrawFill();
         virtual void      drctDrawCLine();
         unsigned          lsize()                 {return _lsize;}
         bool              center_line_only()      {return _celno;}
         virtual const TeselChain* tdata()               {return _tdata;}
      protected:
                           TrxWire(unsigned, const WireWidth, bool);
         int4b*            _ldata; //! the vertexes of the wires central line
         unsigned          _lsize; //! the number of vertexes in the central line
         bool              _celno; //! indicates whether the center line only shall be drawn
         TeselChain*       _tdata; //! wire tesselation data
   };

   //==========================================================================
   //
   // Text objects from the DB
   //
   //==========================================================================
   /**
   * Text objects don't share any properties (fields) with the rest of the class
   * hierarchy objects, and also they are rendered separately. That's why they
   * are defined outside that hierarchy.
   */
   class TrxText {
      public:
                           TrxText(const std::string*, const CTM&);
         void              draw(bool, layprop::DrawProperties*) const;
         const CTM&        ctm() const {return _ctm;}
      private:
         const std::string* _text;
         CTM                _ctm; //! Font Translation matrix
   };
   typedef std::list<TrxText*> TrendStrings;

   /**
   *  Text reference boxes
   */
   class TrxTextOvlBox {
      public:
                           TrxTextOvlBox(const DBbox&, const CTM&);
         virtual          ~TrxTextOvlBox() {}
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual void      drctDrawContour();
      protected:
         int4b             _obox[8];
   };
   typedef std::list<TrxTextOvlBox*> RefTxtList;


   //==========================================================================
   //
   // Selected objects from the DB
   //
   //==========================================================================
   /**
      Very small pure virtual class which primary purpose is to minimise the
      memory usage of the TrendBase by reusing the vertex data. It also optimises
      the amount of data transfered to the graphic card as a whole.
      Here is the idea.

      In most of the rendering cases there will be no data selected in the
      active cell so this class will not be used at all. When a data is
      selected though, the graphical objects have to be rendered twice - once
      as normal objects and the second time - to highlight them in some way.
      There are several ways to achieve this:
         - to store the selected objects in a separate tree during traversing
         and sorting and then to render them separately. This however implies
         that the memory usage and data transfer to the GPU will be doubled
         for the selected objects.
         - to introduce additional selection related fields in all Tender classes
         and to alter the processing depending on the values of those fields.
         This will increase the overall memory usage of the TrendBase, because
         every graphical object will have more fields which will be unused
         actually most of the time. Besides it will (theoretically) slow down
         the processing, because will introduce conditional statements in all
         stages of the object processing.

         The introduction of this class is an attempt to utilise the best of
         the ideas above, but without their drawbacks. A selected object with
         the corresponding selection related fields will be generated only on
         demand. The classes which deal with selected objects will inherit
         their respective parents and this class which will bring the
         additional selection related fields and methods. Selected objects
         will be rendered twice. Once - together with all other objects and
         then once again - to highlight them. Note though, that it will happen
         virtually without additional memory usage, because the second pass
         will reuse the data in their parent objects and will just index it.
         The amount of additional data to the GPU is also minimised,
         because the only additional data transfered is the index array of
         the selected vertexes. Vertex data is already there and will be reused.
   */
   class TrxSelected {
      public:
                           TrxSelected(const SGBitSet* slist) :
                              _slist(slist), _offset(0) {}
         virtual          ~TrxSelected() {}
         bool              partSelected() {return (NULL != _slist);}
         virtual SlctTypes type() = 0;
         virtual unsigned  ssize() = 0;
         virtual unsigned  sDataCopy(unsigned*, unsigned&) = 0;
         virtual void      drctDrawSlctd() = 0;
      protected:
         const SGBitSet*   _slist;  //! A bit set array with selected vertexes
         unsigned          _offset; //! The offset of the first vertex in the point VBO
   };

   /**
      Holds a selected or partially selected convex polygon. Its primary purpose is
      to generate the indexes for the contour (_cdata) data of its primary parent.
      Implements the virtual methods of its parents. Doesn't have any own fields or
      methods.
      The re-implementation of the cDataCopy() method inherited from the primary parent
      worths to be mentioned. It updates the inherited _offset field which is vital
      for the consequent indexing of the selected vertexes.
   */
   class TrxSCnvx : public TrxCnvx, public TrxSelected {
      public:
                           TrxSCnvx(int4b* pdata, unsigned psize, const SGBitSet* slist) :
                              TrxCnvx(pdata, psize), TrxSelected(slist) {}
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? llps : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
         virtual void      drctDrawSlctd();
   };

   class TrxSBox : public TrxBox, public TrxSelected {
      public:
                           TrxSBox(const int4b* pdata, const SGBitSet* slist) :
                              TrxBox(pdata), TrxSelected(slist) {}
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? llps : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
         virtual void      drctDrawSlctd();
   };

   /**
      Holds a selected or partially selected non-convex polygon. Its primary purpose
      is to generate the indexes for the contour (_cdata) data of its primary parent.
      Implements the virtual methods of its parents. Doesn't have any own fields or
      methods.
      The re-implementation of the cDataCopy() has the same function as described in
      TrxSCnvx class
   */
   class TrxSNcvx : public TrxNcvx, public TrxSelected  {
      public:
                           TrxSNcvx(const int4b* pdata, unsigned psize, const SGBitSet* slist) :
                              TrxNcvx(pdata, psize), TrxSelected(slist) {}
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? llps : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
         virtual void      drctDrawSlctd();
   };

   /**
      Holds a selected or partially selected wires. Like its primary parent this is
      not quite trivial class. Because of the specifics of the wire storage it has
      to store an additional VBO offset parameter for the central line. Note that
      unlike other selected objects, this one is indexing primary the central line
      data (_ldata) of its parent and in addition the contour data (_cdata) in
      some of the selection cases. The class implements all virtual methods of its
      parents.
      Here the re-implementation of two methods has to be highlighted. cDataCopy()
      updates inherited _offset field. lDataCopy in turn updates _loffset field.
      Both of them vital for proper indexing of the selected vertexes.
   */
   class TrxSWire : public TrxWire, public TrxSelected {
      public:
                           TrxSWire(int4b* pdata, unsigned psize, const WireWidth width, bool clo, const SGBitSet* slist) :
                              TrxWire(pdata, psize, width, clo), TrxSelected(slist), _loffset(0u) {}
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual unsigned  lDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? lstr : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
         virtual void      drctDrawSlctd();
      protected:
                           TrxSWire(unsigned psize, const WireWidth width, bool clo, const SGBitSet* slist) :
                              TrxWire(psize, width, clo), TrxSelected(slist), _loffset(0u) {}
      private:
         unsigned          _loffset;
   };

   class TrxTextSOvlBox : public TrxTextOvlBox, public TrxSelected {
      public:
                           TrxTextSOvlBox(const DBbox& box, const CTM& mtrx) :
                              TrxTextOvlBox(box, mtrx), TrxSelected(NULL) {}
         virtual          ~TrxTextSOvlBox() {}
         virtual unsigned  cDataCopy(TNDR_GLDATAT*, unsigned&);
         virtual SlctTypes type() { return llps;}
         virtual unsigned  ssize(){ return 4;}
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
         virtual void      drctDrawSlctd();
   };

   //==========================================================================
   //
   // Moving objects from the DB (move/copy/rotate/flip visualising)
   //
   //==========================================================================
   class TrxSMBox : public TrxSBox {
      public:
                           TrxSMBox(const int4b* pdata, const SGBitSet* slist, const CTM& rmm);
         virtual          ~TrxSMBox();
      private:
         PointVector*      movePointsSelected(const SGBitSet&, const CTM&, const CTM& = CTM()) const;
         enum {
               p1x  = 0,
               p1y  = 1,
               p2x  = 2,
               p2y  = 3
         };
   };

   class TrxSMNcvx : public TrxSNcvx{
      public:
                           TrxSMNcvx(const int4b* pdata, unsigned psize, const SGBitSet* slist, const CTM& rmm);
                          ~TrxSMNcvx();
      private:
         PointVector*      movePointsSelected(const SGBitSet&, const CTM&, const CTM& = CTM()) const;
   };

   class TrxSMWire : public TrxSWire {
      public:
                           TrxSMWire(int4b* pdata, unsigned psize, const WireWidth width, bool clo, const SGBitSet* slist, const CTM& rmm);
         virtual          ~TrxSMWire();
      private:
         PointVector*      movePointsSelected(const SGBitSet&, const CTM&, const CTM& = CTM()) const;
   };


   //==========================================================================
   //
   // Objects under construction
   //
   //==========================================================================
   /**
    * Responsible for rendering of boxes under construction. Unlike its parent this
    * class actually contains the data which is about to be rendered. The latter is
    * created in the constructor. It takes the first point selected by the user and
    * creates a box generating the second point in the current marker location.
    */
   class TrxTBox : public TrxBox {
      public:
                           TrxTBox(const TP&, const CTM&);
         virtual          ~TrxTBox();
   };

   /**
    * Responsible for rendering of polygons under construction. Unlike its parent this
    * class actually generates the data which is about to be rendered. The latter is
    * created in the constructor. It takes vertexes which are already marked (entered)
    * by the user (mouse clicks), but also adds one more vertex in the current position
    * of the marker.
    */
   class TrxTNcvx : public TrxNcvx {
      public:
                           TrxTNcvx(const PointVector&, const CTM&);
         virtual          ~TrxTNcvx();
   };


   /**
    * Responsible for rendering of wires under construction. This class generates all
    * the data which is about to be rendered. The data is generated in the constructor.
    * It takes vertexes which are already marked (entered) by the user (mouse clicks),
    * but also adds one more vertex in the current position of the marker.
    */
   class TrxTWire : public TrxWire {
      public:
                           TrxTWire(const PointVector& pdata, WireWidth width, bool center_only, const CTM& rmm);
         virtual          ~TrxTWire();
   };


   //==========================================================================
   //
   // Cell references
   //
   //==========================================================================
   /**
   *  Cell reference boxes & reference related data
   */
   class TrxCellRef {
      public:
                           TrxCellRef(std::string, const CTM&, const DBbox&, word);
                           TrxCellRef();
         std::string       name()         {return _name;}
         real*             translation()  {return _translation;}
         const CTM&        ctm() const    {return _ctm;}
         word              alphaDepth()   {return _alphaDepth;}
         unsigned          cDataCopy(TNDR_GLDATAT*, unsigned&);
         void              drctDrawContour();
      private:
         std::string       _name;
         real              _translation[16];
         CTM               _ctm;
         TNDR_GLDATAT      _obox[8];
         word              _alphaDepth;
   };

}


#endif /* TRXDATA_H_ */
