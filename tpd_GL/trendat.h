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

// Include GLM
#include <glm/glm.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/gtc/matrix_transform.hpp>
//using namespace glm;
typedef glm::vec2               TPX  ; // single 2D verteX
typedef std::vector<TPX>        TPVX ; // array of 2D Vertexes

//=============================================================================
//
// Tesselation classes
//
//=============================================================================

class TessellChunk {
   public:
                        TessellChunk(const TessellChunk&);                        //copy constructor
                        TessellChunk(const TessellChunk& tcobj, unsigned offset); //copy the sequence and add offset
                        TessellChunk(unsigned offset);                          //generate a sequence (i / i+offset)
                        TessellChunk(const WordList&, GLenum, unsigned);
                        TessellChunk(const int*, unsigned, unsigned);
                       ~TessellChunk();
      GLenum            type() const      {return _type;}
      word              size() const      {return _size;}
      const unsigned*   index_seq() const {return _index_seq;}
   private:
      unsigned*         _index_seq;  // index sequence
      word              _size;       // size of the index sequence
      GLenum            _type;
};

typedef std::list<TessellChunk> TessellChain;

//=============================================================================
//
// Replacement of GLU tesselation(which is not maintained anymore)
//
//=============================================================================
class ECVertex {
  public:
     enum VTriStatus { // status of the triangle constituted by a vertex
                        vtsUnchecked  // not checked, because the angle is bigger than 180 degree. Vertex is not clipable
                       ,vtsGood       // checked, no vertexes lying inside the triabgle
                       ,vtsBad        // checked, but has a vertex inside the triangle
                       ,vtsVoid       // check is not possible, i.e. less than 3 points left in the sequence
                      };
                        ECVertex(const word idx) : _idx(idx), _angle(), _vrtxStatus(vtsUnchecked), _next(nullptr), _prev(nullptr) {};
      void              set_next(ECVertex* next) {_next = next;}
      void              set_prev(ECVertex* prev) {_prev = prev;}
      void              set_angle(double angle) {_angle = angle;}
      void              set_vrtxInside(VTriStatus vrtxInside) {_vrtxStatus = vrtxInside;}
      ECVertex*         next() const {return _next;}
      ECVertex*         prev() const {return _prev;}
      word              cidx() const {return _idx;}
      word              pidx() const {return _prev->cidx();}
      word              nidx() const {return _next->cidx();}
      double            angle() const {return _angle;}
      bool              clipable() const {return ((vtsGood ==_vrtxStatus) && (_angle > 0.0));}
   private:
      const word        _idx        ;
      double            _angle      ;
      VTriStatus        _vrtxStatus ;
      ECVertex*         _next       ;
      ECVertex*         _prev       ;
};

class EarClipping {
   public:
                        EarClipping(const int4b*, const word size);
                       ~EarClipping();
      bool              earClip(WordList& indexSeq);
   private:
      void              update(ECVertex*& item, bool direction);
      ECVertex*         clipVertex(ECVertex* item, bool direction);
      bool              checkStraightLine(ECVertex*&, bool direction);
      void              checkClipable(ECVertex* item);
      bool              checkInternal(ECVertex* item, word vIndex);
      bool              triangleArea(word idxA, word idxB, word idxP);
      bool              rewind();
      bool              trySeqUpdate(WordList& indexSeq);

      ECVertex*         _first;
      const int4b*      _data;
      word              _cursize;
      word              _initsize;
      WordSet           _clippedIndexes;
};

class TessellPoly {
   public:
                        TessellPoly();
                        TessellPoly(const TessellPoly*);//deep copy constructor
      void              tessellate(const int4b* pdata, unsigned psize);
      void              pushBackTChunk(TessellChunk chunk);
      void              tessellate3DBox();
      void              tessellate3DPoly(const unsigned idxShift);
      bool              valid() const    { return (0 < (_all_tria + _all_tstr));}
      word              num_tria() const { return _all_tria;}
      word              num_tstr() const { return _all_tstr;}
      const TessellChain* tdata() const    { return &_tdata;  }
      void              num_indexs(unsigned& iftrs, unsigned& iftss) const;
      void              clear();
   //====================================================================
   private:
      TessellChain      _tdata;
      word              _all_tria;// GL_TRIANGLES
      word              _all_tstr;// GL_TRIANGLE_STRIP

};


namespace trend {

   enum SlctTypes{  STlstr // line strip
                   ,STllps // line loops
                   ,STlnes // lines (individual)
                   ,SLCT_TYPES
   };


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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
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
         virtual void      setTeselData(const TessellPoly* tdata) {_tdata = tdata;}
         virtual void      drctDrawFill();
         const TessellChain* tdata()              {return _tdata->tdata();}
      protected:
         const TessellPoly*    _tdata; //! polygon tesselation data
   };

   /**
      Holds the wires from the data base. This class is not quite trivial and it
      causes all kinds of troubles in the overall class hierarchy. It inherits
      TrxNcvx (is it a good idea?). Theoretically in the general case this is a
      non-convex polygon. The wire as a DB object is very specific though. Only the
      central line is stored. The contour is calculated if required on the fly from
      the build-in methods called from the constructor. Instead of using general
      purpose tesselation (expensive), the tesselation data is extracted directly from
      the contour data virtually without calculations. This is one of the few classes
      in the hierarchy which generates vertex and index data. In other words - _cdata
      and _tdata fields belong to this object, they're not references. This means also
      that they shall be cleaned by the destructor.
   */
   class TrxWire : public TrxNcvx {
      public:
                           TrxWire(const int4b*, unsigned, WireWidth, bool);
         virtual          ~TrxWire();
         virtual void      Tesselate();
         virtual void      setTeselData(const TessellPoly*) {assert(false); /*if you hit this assert, use Tesselate method!*/}
         virtual unsigned  lDataCopy(TPVX&, unsigned&, const unsigned);
         virtual void      drctDrawFill();
         virtual void      drctDrawCLine();
         unsigned          lsize()                 {return _lsize;}
         bool              center_line_only()      {return _celno;}
         const TessellPoly* tpdata()               {return _tdata;}

      protected:
                           TrxWire(unsigned, const WireWidth, bool);
         const int4b*      _ldata; //! the vertexes of the wires central line. A link to TDT wire object data
         unsigned          _lsize; //! the number of vertexes in the central line
         bool              _celno; //! indicates whether the center line only shall be drawn
   };
   
   /**
    Object of this class are used to render boxes when rend3D mode is active. The difference
    with TrxBox is that the field _tdata contains data which belongs to this object
    NOTE! All 3D objects inherit TrxNcvx - i.e. they do have a tesselation data
    */
   class Trx3DBox : public TrxNcvx { // the difference with TrxNcvx is the destructor!
      public:
                           Trx3DBox(const int4b* pdata) : TrxNcvx(pdata, 4) {};
         virtual          ~Trx3DBox() {delete _tdata;}
         virtual void      drctDrawFill() {assert(false);}
   };

   /**
    Used to render polygons when rend3D mode is active.
    Unlike TrxNcvx, the tesselation data _tdata is not a reference to the
    tessellation data in the TDT. It copies the original, and then expands it as needed for 3D rendering
    */
   class Trx3DPoly : public TrxNcvx {
      public:
                           Trx3DPoly(const int4b* pdata, unsigned psize) : TrxNcvx(pdata, psize) {};
         virtual          ~Trx3DPoly() {delete _tdata;}
         virtual void      drctDrawFill() {assert(false);}
   };

   /**
    Used to render wires when rend3D mode is active. Expands the TrxWire object Tesselate
    method to accomodate the 3D requirements.
    */
   class Trx3DWire : public TrxWire {
      public:
                           Trx3DWire(const int4b* pdata, unsigned psize, WireWidth width):
                              TrxWire  (pdata, psize, width, false) {}
         virtual void      Tesselate();
         virtual void      drctDrawFill()  {assert(false);}
         virtual void      drctDrawCLine() {assert(false);}
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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
         virtual void      drctDrawContour();
      protected:
         DBbox             _obox;
//         int4b             _obox[8];
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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
         virtual SlctTypes type() { return ((NULL == _slist) ? STllps : STlnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
         virtual void      drctDrawSlctd();
   };

   class TrxSBox : public TrxBox, public TrxSelected {
      public:
                           TrxSBox(const int4b* pdata, const SGBitSet* slist) :
                              TrxBox(pdata), TrxSelected(slist) {}
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
         virtual SlctTypes type() { return ((NULL == _slist) ? STllps : STlnes);}
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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
         virtual SlctTypes type() { return ((NULL == _slist) ? STllps : STlnes);}
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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
         virtual unsigned  lDataCopy(TPVX&, unsigned&, const unsigned);
         virtual SlctTypes type() { return ((NULL == _slist) ? STlstr : STlnes);}
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
         virtual unsigned  cDataCopy(TPVX&, unsigned&, const unsigned);
         virtual SlctTypes type() { return STllps;}
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
         unsigned          cDataCopy(TPVX&, unsigned&);
         void              drctDrawContour();
      private:
         std::string       _name;
         real              _translation[16];
         CTM               _ctm;
         DBbox             _obox;
         word              _alphaDepth;
   };

}


#endif /* TRXDATA_H_ */
