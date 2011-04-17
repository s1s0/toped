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
//   This file is a part of Toped project (C) 2001-2009 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Jan 11 2009
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
/** \file
   Toped is a graphical application and naturally the graphic rendering is a primary
   objective in terms of quality and speed. The Toped rENDERER (TENDERER) is the first
   serious attempt in this project to utilize the power of todays graphic hardware via
   openGL library. The original rendering (which is preserved) is implementing the
   simplest possible rendering approach and is using the most basic openGL functions to
   achieve maximum compatibility. The goal of the TopRend is to be a base for future
   updates in terms of graphic effects, 3D rendering, shaders etc.

   It must be clear that the main rendering acceleration is coming from the structure
   of the Toped database. The QuadTree dominates by far any other rendering optimization
   especially on big databases where the speed really matters. This is the main reason
   behind the fact that the original renderer demonstrates comparable results with
   TopRend. There are virtually no enhancements possible there though. It should be
   noted also that each rendering view in this context is unique. Indeed, when the user
   changes the visual window, the data stream from the Toped DB traverser can't be
   predicted. Different objects will be streamed out depending on the location and
   size of the visual window, but also depending on the current visual properties -
   colors, lines, fills, layer status, cell depth, visual details etc. The assumption
   though (and I hope it's a fact) is that the overall amount of the data is optimal
   with respect to the quality of the image.

   Both renderers are sinking data from the Toped QuadTree data base and depend heavily
   on its property to filter-out quickly and effectively all invisible objects. The
   Toped DB is generally a hierarchical cell tree. Each cell contains a linear list
   of layers which in turn contains a QuadTree of the layout objects. The main task
   of the TopRend is to convert the data stream from the DB traversing into arrays
   of vertexes (VBOs) convenient for the graphical hardware.

   This is done in 3 steps:
      1. Traversing and sorting - data coming from the Toped DB traversing is sorted
         in the dedicated structures. Also some vital data statistics is done and some
         references gathered which will be used during the following steps
      2. VBO generation - the buffers are created and the sorted data is copied there.
      3. Drawing
   \verbatim
                     Layer 1                Layer 2                     Layer N
                   -------------         --------------              --------------
                  |  TenderLay   |      |  TenderLay   |            |  TenderLay   |
                  |              |      |              |            |              |
                  |  ----------  |      |  ----------  |            |  ----------  |
      cell_A      | | TenderTV | |      | | TenderTV | |     |      | | TenderTV | |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
      cell_B      | | TenderTV | |      | | TenderTV | |     |      | | TenderTV | |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
                  |    ______    |      |    ______    |     |      |    ______    |
                  |              |      |              |     |      |              |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
      cell_Z      | | TenderTV | |      | | TenderTV | |     |      | | TenderTV | |
                  |  ----------  |      |  ----------  |            |  ----------  |
                   --------------        --------------              --------------
                          v                     v                           v
                          v                     v                           v
                   -------------         --------------              --------------
                  |  ---------  |       |  ----------  |     |      |  ----------  |
                  | |point VBO| |       | | point VBO| |     |      | | point VBO| |
                  |  ---------  |       |  ----------  |            |  ----------  |
                  |  ---------  |       |  ----------  |     |      |  ----------  |
                  | |index VBO| |       | | index VBO| |     |      | | index VBO| |
                  |  ---------  |       |  ----------  |            |  ----------  |
                   -------------         --------------              --------------
                   ----------------------------------------------------------------
                  |                 index of selected objects VBO                  |
                   ----------------------------------------------------------------
   \endverbatim
   Speed:
   The first step is the most time consuming from all three, but this is mainly the
   Toped DB traversing which is assumed optimal. The last one is the quickest one
   and the speed there can be improved even more if shaders are used. The speed of
   the second step depends entirely on the implementation of the TopRend.

   Memory usage:
   Only the first step consumes memory but it is minimized. There is no data copying
   instead data references are stored only. Technically the second step does consume
   memory of course, but this is the memory consumed by the graphic driver to map the
   VBO in the CPU memory. The TopRend copies the data directly in the VBOs

   Note, that the flow described above allows the screen to be redrawn very quickly
   using the third step only. This opens the door to all kinds of graphical effects.

   As shown on the graph above the TopRend ends-up with a number of VBOs for
   drawing. They are generated "per layer" basis. For each used layer there might
   be maximum two buffers:
   - point VBO - containing the vertexes of all objects on this layer across all
      cells
   - index VBO - containing the tesselation indexes of all non-convex polygons
      on this layer across all cells


   As a whole all vertex data is copied only once. Index data (polygon tesselation)
   is copied only once as well. Wire tesselation is done on the fly and data is
   stored directly in the index VBOs. Selected objects are indexed on the fly and
   the results are stored directly in the dedicated buffer. The index VBO
   containing the selected objects is common for all layers (as shown above)

*/
#ifndef TENDERER_H
#define TENDERER_H

#include <GL/glew.h>
#include "drawprop.h"

//=============================================================================
//
//
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

//=============================================================================
//
//
//
//=============================================================================
namespace tenderer {
   /**
   *  Text reference boxes
   */
   class TextOvlBox {
      public:
                           TextOvlBox(const DBbox&, const CTM&);
         virtual          ~TextOvlBox() {}
         virtual unsigned  cDataCopy(int*, unsigned&);
      private:
         int4b             _obox[8];
   };

   /**
   * Text objects
   */
   class TenderText {
      public:
                           TenderText(const std::string*, const CTM&);
         void              draw(bool);
      private:
         const std::string* _text;
         real              _ftm[16]; //! Font translation matrix
   };

   /**
      Represents convex polygons. Those are mainly the boxes from the data base.
      Stores a reference to the vertex array of the original object and its size.
      The only non-trivial method is cDataCopy which transfers the vertex data
      from the original object to a VBO mapped in the CPU memory
      This class is also a base for the Tender* class hierarchy.
   */
   class TenderCnvx {
      public:
                           TenderCnvx(int4b* pdata, unsigned psize) :
                                    _cdata(pdata), _csize(psize){}
         virtual          ~TenderCnvx() {};
         virtual unsigned  cDataCopy(int*, unsigned&);
         unsigned          csize()     {return _csize;}
      protected:
         int4b*            _cdata;  //! the vertexes of the object contour
         unsigned          _csize;  //! the number of vertexes in _cdata
   };

   class TenderBox : public TenderCnvx {
      public:
                           TenderBox(int4b* pdata) : TenderCnvx(pdata, 4) {}
         virtual unsigned  cDataCopy(int*, unsigned&);
   };

   /**
      Represents non-convex polygons - most of the poly objects in the DB. Inherits
      TenderCnvx. The only addition is the tesselation data (_tdata) which is
      utilized if the object is to be filled.
   */
   class TenderNcvx : public TenderCnvx {
      public:
                           TenderNcvx(int4b* pdata, unsigned psize) :
                                    TenderCnvx(pdata, psize), _tdata(NULL) {}
         void              setTeselData(const TessellPoly* tdata) {_tdata = tdata;}
         virtual          ~TenderNcvx(){};
         virtual const TeselChain* tdata()              {return _tdata->tdata();}
      private:
         const TessellPoly*    _tdata; //! polygon tesselation data
   };

   /**
      Holds the wires from the data base. This class is not quite trivial and it
      causes all kinds of troubles in the overall class hierarchy. It inherits
      TenderNcvx (is it a good idea?). Theoretically in the general case this is a
      non-convex polygon. The wire as a DB object is very specific though. Only the
      central line is stored. The contour is calculated if required on the fly from
      the build-in methods called from the constructor. Instead of using general
      purpose tesselation (slow!), the tesselation data is extracted directly from
      the contour data virtually without calculations. This is the only class in the
      hierarchy which generates vertex and index data which means that it has to be
      properly cleaned-up.
   */
   class TenderWire : public TenderNcvx {
      public:
                           TenderWire(int4b*, unsigned, const laydata::WireWidth, bool);
         virtual          ~TenderWire();
         void              Tesselate();
         virtual unsigned  lDataCopy(int*, unsigned&);
         unsigned          lsize()                 {return _lsize;}
         bool              center_line_only()      {return _celno;}
         virtual const TeselChain* tdata()               {return _tdata;}
      protected:
         int*              _ldata; //! the vertexes of the wires central line
         unsigned          _lsize; //! the number of vertexes in the central line
         bool              _celno; //! indicates whether the center line only shall be drawn
         TeselChain*       _tdata; //! wire tesselation data
   };

   typedef enum {lstr, llps, lnes} SlctTypes;

   /**
      Very small pure virtual class which primary purpose is to minimize the
      memory usage of the TopRend by reusing the vertex data. It also optimises
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
         This will increase the overall memory usage of the TopRend, because
         every graphical object will have more fields which will be unused
         actually most of the time. Besides it will (theoretically) slow down
         the processing, because will introduce conditional statements in all
         stages of the object processing.
         - The introduction of this class is an attempt to utilize the best of
         the ideas above, but without their drawbacks. A selected object with
         the corresponding selection related fields will be generated only on
         demand. The classes which deal with selected objects will inherit
         their respective parents and this class which will bring the
         additional selection related fields and methods. Selected objects
         will be rendered twice. Once - together with all other objects and
         then once again - to highlight them. Note though, that it will happen
         virtually without additional memory usage, because the second pass
         will reuse the data in their parent objects and will just index it.
         The amount of additional data to the GPU is also minimized,
         because the only additional data transfered is the index array of
         the selected vertexes. Vertex data is already there and will be reused.
   */
   class TenderSelected {
      public:
                           TenderSelected(const SGBitSet* slist) :
                              _slist(slist), _offset(0) {}
         virtual          ~TenderSelected() {}
         bool              partSelected() {return (NULL != _slist);}
         virtual SlctTypes type() = 0;
         virtual unsigned  ssize() = 0;
         virtual unsigned  sDataCopy(unsigned*, unsigned&) = 0;
      protected:
         const SGBitSet*   _slist;  //! A bit set array with selected vertexes
         unsigned          _offset; //! The offset of the first vertex in the point VBO
   };

   class TextSOvlBox : public TextOvlBox, public TenderSelected {
      public:
                           TextSOvlBox(const DBbox& box, const CTM& mtrx) :
                              TextOvlBox(box, mtrx), TenderSelected(NULL) {}
         virtual          ~TextSOvlBox() {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return llps;}
         virtual unsigned  ssize(){ return 4;}
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
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
   class TenderSCnvx : public TenderCnvx, public TenderSelected {
      public:
                           TenderSCnvx(int4b* pdata, unsigned psize, const SGBitSet* slist) :
                              TenderCnvx(pdata, psize), TenderSelected(slist) {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? llps : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
   };

   class TenderSBox : public TenderBox, public TenderSelected {
      public:
                           TenderSBox(int4b* pdata, const SGBitSet* slist) :
                              TenderBox(pdata), TenderSelected(slist) {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? llps : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
   };

   /**
      Holds a selected or partially selected non-convex polygon. Its primary purpose
      is to generate the indexes for the contour (_cdata) data of its primary parent.
      Implements the virtual methods of its parents. Doesn't have any own fields or
      methods.
      The re-implementation of the cDataCopy() has the same function as described in
      TenderSCnvx class
   */
   class TenderSNcvx : public TenderNcvx, public TenderSelected  {
      public:
                           TenderSNcvx(int4b* pdata, unsigned psize, const SGBitSet* slist) :
                              TenderNcvx(pdata, psize), TenderSelected(slist) {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? llps : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
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
   class TenderSWire : public TenderWire, public TenderSelected {
      public:
                           TenderSWire(int4b* pdata, unsigned psize, const laydata::WireWidth width, bool clo, const SGBitSet* slist) :
                              TenderWire(pdata, psize, width, clo), TenderSelected(slist), _loffset(0u) {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual unsigned  lDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return ((NULL == _slist) ? lstr : lnes);}
         virtual unsigned  ssize();
         virtual unsigned  sDataCopy(unsigned*, unsigned&);
      private:
         unsigned          _loffset;
   };

   /**
   *
   */
   class TenderFlashing {
      public:
                           TenderFlashing() : _offset(0) {}
         virtual          ~TenderFlashing() {}
         virtual SlctTypes type() = 0;
         virtual unsigned  fDataCopy(unsigned*, unsigned&) = 0;
      protected:
         unsigned          _offset; //! The offset of the first vertex in the point VBO
   };

   /**
      Holds a non-convex polygon which are supposed to flash. Its primary purpose
      is to generate the indexes for the contour (_cdata) data of its primary parent.
      Implements the virtual methods of its parents. Doesn't have any own fields or
      methods.
      The re-implementation of the cDataCopy() has the same function as described in
      TenderSCnvx class
   */
   class TenderFNcvx : public TenderNcvx, public TenderFlashing  {
      public:
                           TenderFNcvx(int4b* pdata, unsigned psize) :
                              TenderNcvx(pdata, psize), TenderFlashing() {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return llps;}
         virtual unsigned  fDataCopy(unsigned*, unsigned&);
   };

   /**
      Holds a wires which are supposed to flash. Like its primary parent this is
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
   class TenderFWire : public TenderWire, public TenderFlashing {
      public:
                           TenderFWire(int4b* pdata, unsigned psize, const laydata::WireWidth width, bool clo) :
                              TenderWire(pdata, psize, width, clo), TenderFlashing(), _loffset(0u) {}
         virtual unsigned  cDataCopy(int*, unsigned&);
         virtual unsigned  lDataCopy(int*, unsigned&);
         virtual SlctTypes type() { return lstr;}
         virtual unsigned  fDataCopy(unsigned*, unsigned&);
      private:
         unsigned          _loffset;
   };

   /**
   *  Cell reference boxes & reference related data
   */
   class TenderRef {
      public:
                           TenderRef(std::string, const CTM&, const DBbox&, word);
                           TenderRef();
         std::string       name()         {return _name;}
         real* const       translation()  {return _translation;}
         CTM&              ctm()          {return _ctm;}
         word              alphaDepth()   {return _alphaDepth;}
         unsigned          cDataCopy(int*, unsigned&);
      private:
         std::string       _name;
         real              _translation[16];
         CTM               _ctm;
         int4b             _obox[8];
         word              _alphaDepth;
   };

   typedef std::list<TenderCnvx*>      SliceObjects;
   typedef std::list<TenderNcvx*>      SlicePolygons;
   typedef std::list<TenderWire*>      SliceWires;
   typedef std::list<TenderSelected*>  SliceSelected;
   typedef std::list<TenderFlashing*>  SliceFlashing;
   typedef std::list<TenderRef*>       RefBoxList;

   /**
      TENDERer Translation View - is the most fundamental class of the TopRend.
      It sorts and stores a layer slice of the cell data. Most of the memory
      consumption during the first processing step (traversing and sorting) is
      concentrated in this class and naturally most of the TopRend data processing
      happens in the methods of this class. The input data stream is sorted in 4
      "bins" in a form of Tender object lists. The corresponding objects are
      created by register* methods. Vertex or index data is not copied during this
      process, instead only data references are stored. The exception here are the
      wire objects which generate their contour data during the construction. Each
      object is sorted in exactly one bin depending whether it's going to be filled
      or not. The exception as always is the wire object which also goes to the
      line bin because of its central line.
      \verbatim
      ---------------------------------------------------------------
      | TopRend   |    not filled      |        filled      |        |
      |   data    |--------------------|--------------------|  enum  |
      | (vertexes)|  box | poly | wire |  box | poly | wire |        |
      |-----------|------|------|------|------|------|------|--------|
      | contour   |  x   |  x   |  x   |      |      |      |  cont  |
      | line      |      |      |  x   |      |      |  x   |  line  |
      | convex    |      |      |      |  x   |      |      |  cnvx  |
      | non-convex|      |      |      |      |  x   |  x   |  ncvx  |
      --------------------------------------------------------------
      \endverbatim

      The sorting step gathers also essential data statistics which will be used
      later to constitute the virtual buffers, to copy the data into the
      appropriate locations of the corresponding buffers and to draw the appropriate
      portions of the data using the appropriate openGL functions. This data is
      stored in the class fields in the form of four arrays with size of four each.
      One position per bin is allocated in each of those arrays (_alvrtxs[4],
      _alobjvx[4], _sizesvx[4], _firstvx[4]) indicating the overall number of
      vertexes, the total number of objects, the size of each object in terms of
      vertexes, and the offset of each first object vertex in the buffer

      If the non-convex data is to be filled, it is sorted further in another four
      index bins. The data statistics is also split further into four to accommodate
      that additional level of detail. This is necessary because the polygon
      tesselation generates 3 types of index data. In addition the wire tesselation
      is using yet another type of index data and all the above has to be sorted and
      stored. The array fields _alindxs[4], _alobjix[4], _sizesix[4], _firstix[4]
      hold a position for each of the four index bins. They represent the overall
      amount of indexes, the total number of indexed objects, the size of each
      object in terms of indexes and the offset of each first object index in the
      buffer.It has to be noted also that the openGL tesselation (used for
      non-convex polygons) produces a number of chunks of index data and each of
      them can be of any of the first 3 types. Wire tesselation produces a single
      chunk and it is always from one and the same type.  The table below summaries
      the indexing.

      \verbatim
      -----------------------------------------------------------------
      | Tesselation    | non convex object  |        |      openGL      |
      |         data   |--------------------|  enum  |     drawing      |
      |   (indexes)    |  polygon |  wire   |        |      method      |
      |----------------|----------|---------|--------|------------------|
      | triangles      |     x    |         |  ftrs  | GL_TRIANGLES     |
      | triangle fan   |     x    |         |  ftfs  | GL_TRIANGLE_FAN  |
      | triangle strip |     x    |         |  ftss  | GL_TRIANGLE_STRIP|
      | quadratic strip|          |    x    |  fqss  | GL_QUAD_STRIP    |
      -----------------------------------------------------------------
      \endverbatim

      The collect() method implements the second step of the processing namely
      data collection (copy). It puts all vertex data in the linear vertex buffer
      structured in the following way

      \verbatim
      -------------------------------------------------------------------------
      ... || cont | line  | cnvx | ncvx || cont | line  | cnvx | ncvx || ...
      ... ||----------------------------||----------------------------|| ...
      ... || this TenderTV object data  ||  next TenderTV object data || ...
      -------------------------------------------------------------------------
      \endverbatim

      and the corresponding index data in another index buffer with the following
      structure

      \verbatim
      -------------------------------------------------------------------------
      ... || ftrs | ftfs  | ftss | fqss || ftrs | ftfs  | ftss | fqss || ...
      ... ||----------------------------||----------------------------|| ...
      ... ||  this TenderTV index data  ||  next TenderTV index data  || ...
      -------------------------------------------------------------------------
      \endverbatim

      The index data relates to the ncvx bin only and if the latter is not existing
      the corresponding portion of the index buffer will be with the length 0

      The actual drawing process refers to the ready VBOs which by this moment
      should be stored in the GPU memory. It then implements relatively simple
      algorithm calling the corresponding openGL functions for each portion of
      the VBOs using also as additional parameters the same additional data
      gathered during the traversing and sorting step.

      \verbatim
       -------------------------------------------------------------------
      |      |                                           |       VBO      |
      | data |            openGL function                |----------------|
      | type |                                           | vertex | index |
      |------|-------------------------------------------|--------|-------|
      | cont | glMultiDrawArrays(GL_LINE_LOOP  ...)      |    x   |       |
      |------|-------------------------------------------|--------|-------|
      | line | glMultiDrawArrays(GL_LINE_STRIP ...)      |    x   |       |
      |------|-------------------------------------------|--------|-------|
      | cnvx | glMultiDrawArrays(GL_LINE_LOOP  ...)      |    x   |       |
      |      | glMultiDrawArrays(GL_QUADS      ...)      |    x   |       |
      |------|-------------------------------------------|--------|-------|
      | ncvx | glMultiDrawArrays(GL_LINE_LOOP  ...)      |    x   |       |
      |      | glMultiDrawElements(GL_TRIANGLES     ...) |    x   |   x   |
      |      | glMultiDrawElements(GL_TRIANGLE_FAN  ...) |    x   |   x   |
      |      | glMultiDrawElements(GL_TRIANGLE_STRIP...) |    x   |   x   |
      |      | glMultiDrawElements(GL_QUAD_STRIP    ...) |    x   |   x   |
      -------------------------------------------------------------------
      \endverbatim
   */
   class TenderTV {
      public:
         enum {fqss, ftrs, ftfs, ftss} NcvxTypes;
         enum {cont, line, cnvx, ncvx} ObjtTypes;
         typedef std::list<TenderText*> TenderStrings;
         typedef std::list<TextOvlBox*> RefTxtList;
                           TenderTV(TenderRef* const, bool, bool, unsigned, unsigned);
                          ~TenderTV();
         void              registerBox   (TenderCnvx*);
         void              registerPoly  (TenderNcvx*, const TessellPoly*);
         void              registerWire  (TenderWire*);
         void              registerText  (TenderText*, TextOvlBox*);

         void              collect(int*, unsigned int*, unsigned int*);
         void              draw(layprop::DrawProperties*);
         void              drawTexts(layprop::DrawProperties*);
         TenderRef*        swapRefCells(TenderRef*);

         unsigned          num_total_points();
         unsigned          num_total_indexs();
         unsigned          num_total_strings()  {return _num_total_strings;}
         bool              reusable() const     {return _reusable;}
         bool              filled() const       {return _filled;}
         std::string       cellName()           {return _refCell->name();}
      protected:
         void              collectIndexs(unsigned int*, const TeselChain*, unsigned*, unsigned*, unsigned);

         TenderRef*        _refCell;
         // collected data lists
         SliceObjects      _cont_data; //! Contour data
         SliceWires        _line_data; //! Line data
         SliceObjects      _cnvx_data; //! Convex polygon data (Only boxes are here at the moment. TODO - all convex polygons)
         SlicePolygons     _ncvx_data; //! Non convex data
         TenderStrings     _text_data; //! Text (strings)
         RefTxtList        _txto_data; //! Text overlapping boxes
         // vertex related data
         unsigned          _alvrtxs[4]; //! array with the total number of vertexes
         unsigned          _alobjvx[4]; //! array with the total number of objects that will be drawn with vertex related functions
         GLsizei*          _sizesvx[4]; //! arrays of sizes for vertex sets
         GLsizei*          _firstvx[4]; //! arrays of first vertexes
         // index related data for non-convex polygons
         unsigned          _alindxs[4]; //! array with the total number of indexes
         unsigned          _alobjix[4]; //! array with the total number of objects that will be drawn with index related functions
         GLsizei*          _sizesix[4]; //! arrays of sizes for indexes sets
         GLuint*           _firstix[4]; //! arrays of first indexes
         // offsets in the VBO
         unsigned          _point_array_offset; //! The offset of this chunk of vertex data in the vertex VBO
         unsigned          _index_array_offset; //! The offset of this chunk of index  data in the index  VBO
         //
         unsigned          _num_total_strings;
         bool              _filled;
         bool              _reusable;
   };

   /**
      Very small class which holds the reusable TenderTV chunks. Here is what it is.
      The view usually contains relatively small number of cells which are referenced
      multiply times. They do contain the same data, but it should be visualized
      using different translation matrices. The simplest case is an array of cells
      object. It would be really a waste of CPU time to traverse those cells every
      time they are referenced. This also means that the amount of processing data
      will be determined by the number of references, not by the number of unique
      cells which in turn will slow-down the rendering and will consume much more
      memory and bandwidth to transfer it to the GPU.

      Here is the alternative approach. Each cell layer (chunk) is checked during
      the traversing for its location. There can be 3 cases:

         - the chunk is outside the view window - it is then discarded and is not
         reaching the TopRend at all.
         - the chunk is partially inside the view window - it is traversed, but
         its data is considered unique and no further attempts to reuse it.
         - the chunk is entirely inside the view window

      The latter case is the interesting one. Before creating a new chunk of data
      (TenderTV object), TopRend is checking whether it already exists
      (TenderLay::chunkExists). If it does - it is registered here, a new TenderReTV
      object is created and the traversing of the current layer is skipped. If the
      chunk doesn't exists, then a TenderTV object is created, but it is also
      registered as "reusable" and the traversing continues as normal.

      This class requires only a draw() method which takes care to replace temporary
      the translation matrix of the referenced TenderTV and then calls TenderTV::draw()
   */
   class TenderReTV {
      public:
                           TenderReTV(TenderTV* const chunk, TenderRef* const refCell):
                              _chunk(chunk), _refCell(refCell) {}
         void              draw(layprop::DrawProperties*);
         void              drawTexts(layprop::DrawProperties*);
      private:
         TenderTV* const   _chunk;
         TenderRef* const  _refCell;

   };

   /**
      TENDER LAYer represents a DB layer in the rendering view. It generates,
      stores and sorts all the TenderTV objects (one per DB cell) and also
      Initializes the corresponding virtual buffers (the vertex and eventually
      the index) for this layer. This class is responsible also for gathering of
      the selected data indexes.

      A new TederTV object is created by the newSlice() method during the DB
      traversing. A reference to it is stored in the _layData list as well as
      in the _cslice field. All subsequent calls to box(), poly() and wire()
      methods create the corresponding object of class Tender* and store them
      calling the corresponding methods of _cslice object (of TenderTV class).
      If the data is selected then an object of the overloaded class is created
      and a reference to it is stored in _slct_data list as well.

      TenderLay keeps track of the total current amount of vertexes
      (_num_total_points) and indexes (_num_total_indexs) in order to initialize
      properly the offset fields of the generated TenderTV objects. When a new
      slice of class TenderTV is created, the current slice is post processed
      (method ppSlice()) in order to update the fields mentioned above. The
      final values stored in those fields are used later to reserve a proper
      amount of memory for the VBO's belonging to this layer.

      The rendering of the selected objects is done in this class. TenderTV does
      hold the object vertex data, but it is completely unaware of selection
      related properties. The primary reason for this is that data can be
      selected only in one of the generated TenderTV objects, because only one
      of them eventually belongs to the active cell. Having an additional VBO
      per TenderTV object for selected data would be a waste of course, because
      all but one of them will be empty anyway. Instead a single VBO is generated
      for the entire rendering view with the indexes of all selected vertexes in
      the active cell across all layers.

      Selected objects will be rendered twice. Once - together with all other
      objects in the TenderTV objects and again here, in this class in order to
      highlight them. An object can be fully or partially selected and depending
      on this it shall be rendered using the corresponding data structure:

      \verbatim
       --------------------------------------------------------------
      |  TopRend  |  fully selected    | partially selected |        |
      |    data   |--------------------|--------------------|  enum  |
      | (indexes) |  box | poly | wire |  box | poly | wire |        |
      |-----------|------|------|------|------|------|------|--------|
      |line strips|      |      |      |   x  |   x  |   x  |  lstr  |
      | contours  |  x   |  x   |      |      |      |      |  llps  |
      |   lines   |      |      |  x   |      |      |      |  lnes  |
      --------------------------------------------------------------
      \endverbatim

      In the manner similar to TenderTV class, TenderLay sorts the selected objects
      in three "bins" and gathers some statistics about each of them. One position
      per bin is stored in each of the array fields (_asindxs[3], _asobjix[3],
      _sizslix[3], _fstslix[3]) representing the overall amount of indexes, the
      total number of indexed objects, the size of each object in terms of indexes
      and the offset of each first object index in the buffer. Additionally every
      selected object will need to store the offset of its vertex data within the
      slice representing the active cell in the vertex buffer. This is done by the
      selected objects themselves when vertex VBOs are collected via the
      re-implementation of cDataCopy virtual methods in the TenderS* classes.
      TenderLay doesn't take part in this process, TenderTV class is not aware of
      it either - it happens "naturally" thanks to the class hierarchy.

      The collectSelected() method implements the second step of the processing of
      selected data. It must be called after the collect() method, because the
      latter is the one that indirectly gathers the object vertex offsets described
      previously. collectSelected() puts all index data in the linear vertex buffer
      structured in the following way:

      \verbatim
      -------------------------------------------------------------------------
      ... ||   lstr  |   llps  |  lnes  ||   lstr  |   llps  |  lnes  || ...
      ... ||----------------------------||----------------------------|| ...
      ... ||  this TenderLay index data ||  next TenderLay index data || ...
      -------------------------------------------------------------------------
      \endverbatim

      The drawing of the corresponding portion of the selected data buffer is done
      also in this class by the drawSelected() method. For this we need the vertex
      buffer and a proper offset in it which will point to the beginning of the data
      slice belonging to the active cell. This offset is captured during the data
      traversing in the field _stv_array_offset. Having done that, the drawing is
      now trivial

      ----------------------------------------------------------------
      |      |                                        |       VBO      |
      | data |            openGL function             |----------------|
      | type |                                        | vertex | index |
      |------|----------------------------------------|--------|-------|
      | lstr | glMultiDrawElements(GL_LINE_STRIP ...) |    x   |   x   |
      |------|----------------------------------------|--------|-------|
      | llps | glMultiDrawElements(GL_LINE_LOOP  ...) |    x   |   x   |
      |------|----------------------------------------|--------|-------|
      | lnes | glMultiDrawElements(GL_LINES      ...) |    x   |   x   |
      ----------------------------------------------------------------
   */
   class TenderLay {
      public:
         typedef std::list<TenderTV*>     TenderTVList;
         typedef std::list<TenderReTV*>   TenderReTVList;
         typedef std::map<std::string, TenderTV*> ReusableTTVMap;

                           TenderLay();
                          ~TenderLay();
         void              box  (int4b*,                       bool, const SGBitSet*);
         void              poly (int4b*, unsigned, const TessellPoly*, bool, const SGBitSet*);
         void              wire (int4b*, unsigned, laydata::WireWidth, bool, bool, const SGBitSet*);
         void              text (const std::string*, const CTM&, const DBbox*, const TP&, bool);

         void              fpoly(int4b*, unsigned, const TessellPoly*);
         void              fwire(int4b*, unsigned, laydata::WireWidth, bool);
         void              newSlice(TenderRef* const, bool, bool, bool, unsigned, unsigned);
         bool              chunkExists(TenderRef* const, bool);
         void              ppSlice();
         void              draw(layprop::DrawProperties*);
         void              drawSelected();
         void              drawFlashing();
         void              drawTexts(layprop::DrawProperties*);
         void              collect(bool, GLuint, GLuint);
         void              collectSelected(unsigned int*);
         void              collectFlashing(unsigned int*);
         unsigned          total_points() {return _num_total_points;}
         unsigned          total_indexs() {return _num_total_indexs;}
         unsigned          total_slctdx();
         unsigned          total_flshdx();
         unsigned          total_strings(){return _num_total_strings;}

      private:
         void              registerSBox  (TenderSBox*);
         void              registerSPoly (TenderSNcvx*);
         void              registerSWire (TenderSWire*);
         void              registerSOBox (TextSOvlBox*);
         void              registerFPoly (TenderFNcvx*);
         void              registerFWire (TenderFWire*);
         ReusableTTVMap    _reusableFData; // reusable filled chunks
         ReusableTTVMap    _reusableCData; // reusable contour chunks
         TenderTVList      _layData;
         TenderReTVList    _reLayData;
         TenderTV*         _cslice;    //!Working variable pointing to the current slice
         unsigned          _num_total_points;
         unsigned          _num_total_indexs;
         unsigned          _num_total_slctdx;
         unsigned          _num_total_strings;
         GLuint            _pbuffer;
         GLuint            _ibuffer;
         bool              _has_selected;
         // Data related to selected objects
         SliceSelected     _slct_data;
         // index related data for selected objects
         unsigned          _asindxs[3]; //! array with the total number of indexes of selected objects
         unsigned          _asobjix[3]; //! array with the total number of selected objects
         GLsizei*          _sizslix[3]; //! arrays of sizes for indexes sets of selected objects
         GLuint*           _fstslix[3]; //! arrays of first indexes for selected objects
         // Data related to flashing objects
         SliceFlashing     _flsh_data;
         // index related data for selected objects
         unsigned          _afindxs[2]; //! array with the total number of indexes of flashing objects
         unsigned          _afobjix[2]; //! array with the total number of flashing objects
         GLsizei*          _sizflix[2]; //! arrays of sizes for indexes sets of flashing objects
         GLuint*           _fstflix[2]; //! arrays of first indexes for flashing objects
         // offsets in the VBO
         unsigned          _stv_array_offset; //! first point in the TenderTV with selected objects in this layer
         unsigned          _ftv_array_offset; //! first point in the TenderTV with flashing objects in this layer
         unsigned          _slctd_array_offset; //! first point in the VBO with selected indexes
         unsigned          _flshd_array_offset; //! first point in the VBO with flashing indexes
   };

   /**
      Responsible for visualizing the overlap boxes of the references. Relatively
      trivial class. One object of this class should be created only. All reference
      boxes are processed in a single VBO. This includes the selected ones.
   */
   class TenderRefLay {
      public:
                           TenderRefLay();
                          ~TenderRefLay();
         void              addCellOBox(TenderRef*, word, bool);
         void              collect(GLuint);
         void              draw(layprop::DrawProperties*);
         unsigned          total_points();
         unsigned          total_indexes();
      private:
         RefBoxList        _cellRefBoxes;
         RefBoxList        _cellSRefBoxes;
         GLuint            _pbuffer;
         // vertex related data
         unsigned          _alvrtxs; //! total number of vertexes
         unsigned          _alobjvx; //! total number of objects that will be drawn with vertex related functions
         GLsizei*          _sizesvx; //! array of sizes for vertex sets
         GLsizei*          _firstvx; //! array of first vertexes
         // index related data for selected boxes
         unsigned          _asindxs; //! total number of selected vertexes
         unsigned          _asobjix; //! total number of objects that will be drawn with index related functions
         GLsizei*          _sizslix; //! array of sizes for indexes sets
         GLsizei*          _fstslix; //! array of first indexes
   };

   //-----------------------------------------------------------------------------
   //
   typedef std::map<unsigned, TenderLay*> DataLay;
   typedef std::stack<TenderRef*> CellStack;

   /**
      Toped RENDERER is the front-end class, the interface to the rest of the world.
      All render views are initiated using an object of this class. There should be
      only one object of this class at a time. The object of this class must be
      destroyed before the next rendering view is invoked. The data gathered for
      the previous view must be considered invalid. The object structure created
      by this class is shown in the documentation of the module.
   */
   class TopRend {
      public:
                           TopRend( layprop::DrawProperties* drawprop, real UU );
                          ~TopRend();
         void              Grid( const real, const std::string );
         void              setLayer(unsigned, bool);
         bool              chunkExists(unsigned, bool);
         void              pushCell(std::string, const CTM&, const DBbox&, bool, bool);
         void              popCell()                              {_cellStack.pop();}
         const CTM&        topCTM() const                         {return  _cellStack.top()->ctm();}
         void              box  (int4b* pdata)                    {_clayer->box(pdata, false, NULL);}
         void              box  (int4b* pdata, const SGBitSet* ss){_clayer->box(pdata, true, ss);}
         void              poly (int4b* pdata, unsigned psize, const TessellPoly* tpoly)
                                                                  {_clayer->poly(pdata, psize, tpoly, false, NULL);}
         void              poly (int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss)
                                                                  {_clayer->poly(pdata, psize, tpoly, true, ss);}
         void              fpoly(int4b* pdata, unsigned psize, const TessellPoly* tpoly = NULL)
                                                                  {_clayer->fpoly(pdata, psize, tpoly);}
         void              wire (int4b*, unsigned, laydata::WireWidth);
         void              wire (int4b*, unsigned, laydata::WireWidth, const SGBitSet*);
         void              fwire(int4b*, unsigned, laydata::WireWidth);
         void              arefOBox(std::string, const CTM&, const DBbox&, bool);
         void              text (const std::string*, const CTM&, const DBbox&, const TP&, bool);
         bool              collect();
         void              draw();
         void              cleanUp();

         //return layno if _propertyState == DB or predefined layer in other case
         unsigned          getTenderLay(unsigned layno);
         //set state of DrawProperties
         void              setState(layprop::PropertyState state) {_drawprop->setState(state);};
         // temporary!
         bool              layerHidden(unsigned layno) const
                                                         {return  _drawprop->layerHidden(layno)    ;}
         const CTM&        ScrCTM() const                {return  _drawprop->scrCtm()              ;}
         word              visualLimit() const           {return  _drawprop->visualLimit()         ;}
         const DBbox&      clipRegion() const            {return  _drawprop->clipRegion()          ;}
         void              postCheckCRS(const laydata::TdtCellRef* ref)
                                                         {        _drawprop->postCheckCRS(ref)     ;}
         bool              preCheckCRS(const laydata::TdtCellRef*, layprop::CellRefChainType&);
         void              initDrawRefStack(laydata::CellRefStack* crs)
                                                         {        _drawprop->initDrawRefStack(crs) ;}
         void              clearDrawRefStack()           {        _drawprop->clearDrawRefStack()   ;}
         bool              adjustTextOrientation() const {return  _drawprop->adjustTextOrientation();}
      private:
         layprop::DrawProperties*   _drawprop;
         real              _UU;
         DataLay           _data;            //!All data for drawing
         TenderLay*        _clayer;          //!Working variable pointing to the current slice
         TenderRefLay      _refLayer;
         CellStack         _cellStack;       //!Required during data traversing stage
         unsigned          _cslctd_array_offset; //! Current selected array offset
         unsigned          _cflshd_array_offset; //! Current flashing array offset
         //
         unsigned          _num_ogl_buffers; //! Number of generated openGL VBOs
         GLuint*           _ogl_buffers;     //! Array with the "names" of all openGL buffers
         GLuint            _sbuffer;         //! The "name" of the selected index buffer
         GLuint            _fbuffer;         //! The "name" of the flashing index buffer
         TenderRef*        _activeCS;
         byte              _dovCorrection;   //!Cell ref Depth of view correction (for Edit in Place purposes)
         RefBoxList        _hiddenRefBoxes;  //! Those cRefBox objects which didn't ended in the TenderRefLay structures
   };

   void checkOGLError(std::string);
}

#endif //TENDERER_H
