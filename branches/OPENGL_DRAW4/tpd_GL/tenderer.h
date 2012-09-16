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
   achieve maximum compatibility. The goal of the Tenderer is to be a base for future
   updates in terms of graphic effects, 3D rendering, shaders etc.

   It must be clear that the main rendering acceleration is coming from the structure
   of the Toped database. The QuadTree dominates by far any other rendering optimisation
   especially on big databases where the speed really matters. This is the main reason
   behind the fact that the original renderer demonstrates comparable results with
   Tenderer. There are virtually no enhancements possible there though. It should be
   noted also that each rendering view in this context is unique. Indeed, when the user
   changes the visual window, the data stream from the Toped DB traverser can't be
   predicted. Different objects will be streamed out depending on the location and
   size of the visual window, but also depending on the current visual properties -
   colours, lines, fills, layer status, cell depth, visual details etc. The assumption
   though (and I hope it's a fact) is that the overall amount of the data is optimal
   with respect to the quality of the image.

   Both renderers are sinking data from the Toped QuadTree data base and depend heavily
   on its property to filter-out quickly and effectively all invisible objects. The
   Toped DB is generally a hierarchical cell tree. Each cell contains a linear list
   of layers which in turn contains a QuadTree of the layout objects. The main task
   of the Tenderer is to convert the data stream from the DB traversing into arrays
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
   the second step depends entirely on the implementation of the Tenderer.

   Memory usage:
   Only the first step consumes memory but it is minimised. There is no data copying
   instead data references are stored only. Technically the second step does consume
   memory of course, but this is the memory consumed by the graphic driver to map the
   VBO in the CPU memory. The Tenderer copies the data directly in the VBOs

   Note, that the flow described above allows the screen to be redrawn very quickly
   using the third step only. This opens the door to all kinds of graphical effects.

   As shown on the graph above the Tenderer ends-up with a number of VBOs for
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
#include "basetrend.h"

namespace trend {
   /**
      TENDERer Translation View - is the most fundamental class of the Tenderer.
      It sorts and stores a layer slice of the cell data. Most of the memory
      consumption during the first processing step (traversing and sorting) is
      concentrated in this class and naturally most of the Tenderer data processing
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
      | Tenderer   |    not filled      |        filled      |        |
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
   class TenderTV : public TrendTV {
      public:
                           TenderTV(TenderRef* const, bool, bool, unsigned, unsigned);
         virtual          ~TenderTV();

         virtual void      collect(TNDR_GLDATAT*, unsigned int*, unsigned int*);
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
      private:
         void              collectIndexs(unsigned int*, const TeselChain*, unsigned*, unsigned*, unsigned);
         GLsizei*          _sizesvx[4]; //! arrays of sizes for vertex sets
         GLsizei*          _firstvx[4]; //! arrays of first vertexes
         GLsizei*          _sizesix[4]; //! arrays of sizes for indexes sets
         GLuint*           _firstix[4]; //! arrays of first indexes
         // offsets in the VBO
         unsigned          _point_array_offset; //! The offset of this chunk of vertex data in the vertex VBO
         unsigned          _index_array_offset; //! The offset of this chunk of index  data in the index  VBO
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
         reaching the Tenderer at all.
         - the chunk is partially inside the view window - it is traversed, but
         its data is considered unique and no further attempts to reuse it.
         - the chunk is entirely inside the view window

      The latter case is the interesting one. Before creating a new chunk of data
      (TenderTV object), Tenderer is checking whether it already exists
      (TenderLay::chunkExists). If it does - it is registered here, a new TenderReTV
      object is created and the traversing of the current layer is skipped. If the
      chunk doesn't exists, then a TenderTV object is created, but it is also
      registered as "reusable" and the traversing continues as normal.

      This class requires only a draw() method which takes care to replace temporary
      the translation matrix of the referenced TenderTV and then calls TenderTV::draw()
   */
   class TenderReTV : public TrendReTV {
      public:
                           TenderReTV(TrendTV* const chunk, TenderRef* const refCell):
                              TrendReTV(chunk, refCell) {}
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
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
      |  Tenderer  |  fully selected    | partially selected |        |
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
   class TenderLay : public TrendLay {
      public:
                           TenderLay();
         virtual          ~TenderLay();
         virtual void      newSlice(TenderRef* const, bool, bool /*, bool, unsigned*/);
         virtual void      newSlice(TenderRef* const, bool, bool, unsigned slctd_array_offset);
         virtual bool      chunkExists(TenderRef* const, bool);
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawSelected();
         virtual void      drawTexts(layprop::DrawProperties*);
         virtual void      collect(bool, GLuint, GLuint);
         virtual void      collectSelected(unsigned int*);

      private:
         GLuint            _pbuffer;
         GLuint            _ibuffer;
         // index related data for selected objects
         GLsizei*          _sizslix[3]; //! arrays of sizes for indexes sets of selected objects
         GLuint*           _fstslix[3]; //! arrays of first indexes for selected objects
         // offsets in the VBO
         unsigned          _stv_array_offset; //! first point in the TenderTV with selected objects in this layer
         unsigned          _slctd_array_offset; //! first point in the VBO with selected indexes
   };

   /**
      Responsible for visualising the overlap boxes of the references. Implements
      pure virtual classes of its parent for the tenderer. All reference boxes
      are processed in a single VBO. This includes the selected ones.
   */
   class TenderRefLay : public TrendRefLay {
      public:
                           TenderRefLay();
         virtual          ~TenderRefLay();
         virtual void      collect(GLuint);
         virtual void      draw(layprop::DrawProperties*);
      private:
         GLuint            _pbuffer;
         // vertex related data
         GLsizei*          _sizesvx; //! array of sizes for vertex sets
         GLsizei*          _firstvx; //! array of first vertexes
         // index related data for selected boxes
         GLsizei*          _sizslix; //! array of sizes for indexes sets
         GLsizei*          _fstslix; //! array of first indexes
   };

   /**
      Toped rENDERER is the front-end class for the VBO renderer. All data parsing
      functionality is implemented in the parent class. VBO collection and drawing
      is implemented here.
   */
   class Tenderer : public TrendBase {
      public:
                           Tenderer( layprop::DrawProperties* drawprop, real UU );
         virtual          ~Tenderer();
         virtual void      grid( const real, const std::string );
         virtual void      setLayer(const LayerDef&, bool);
         virtual void      setGrcLayer(bool, const LayerDef&);
         virtual bool      chunkExists(const LayerDef&, bool);
         virtual bool      collect();
         virtual bool      grcCollect();
         virtual void      draw();
         virtual void      grcDraw();
         virtual void      cleanUp();
         virtual void      grcCleanUp();
      private:
         unsigned          _num_ogl_buffers; //! Number of generated openGL VBOs
         unsigned          _num_ogl_grc_buffers; //!
         GLuint*           _ogl_buffers;     //! Array with the "names" of all openGL buffers
         GLuint*           _ogl_grc_buffers; //! Array with the "names" of the GRC related openGL buffers
         GLuint            _sbuffer;         //! The "name" of the selected index buffer
   };

}

#endif //TENDERER_H
