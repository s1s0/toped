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
//        Created: Wed Sep 12 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Base class for all openGL renderers
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
/** \file
   Toped is a graphical application and naturally the graphic rendering is a
   primary objective in terms of quality and speed. The openGL library is used
   for low level drawing and that brings some constraints which need to be
   addressed in the code. Those constraints are related to the various graphic
   platforms and development history of the library. Here is the summary:
    - The existing graphic platforms (hardware and drivers) which support openGL
      can be sorted in 3 groups (at the time of writing):
      -- supporting the latest openGL specification (3.30 and later) - those
         usually include recent graphic card with proprietary driver and are
         quite often associated with Windows desktop machines.
      -- limited openGL support - either the hardware or the drivers support only
         relatively old openGL revision (before 3.0). This group is normally
         associated with Linux desktop installations with open source drivers.
      -- basic openGL support - quite often only functionality from revision 1.1
         of the openGL specification. A major entry to this group is the
         virtualisation software and software for terminal emulation.
    - OpenGL specification rev.3.30 declared obsolete virtually all the functionality
      which does not relate to GLSL (shaders). In other words the library is
      completely different from the original spec. and the compatibility is now
      not a requirement. It is up to the vendor of the driver.

   To ensure that the tool performs reasonably well on all platforms listed above
   we need three different rederers. In the code they are called:
      - tolder - for platforms supporting openGL < 1.4. Using only functionality
        included in the revision 1.1.
      - tenderer - for platforms which support VBO, but have very limited or no
        support at all for GLSL (shaders). This implementation is using Virtual
        Buffer Objects.
      - toshader - for up to date platforms supporting openGL >= 3.30 This
        implementations is using shaders.

   It must be clear that the major part of the drawing acceleration is coming
   from the structure of the Toped database. The QuadTree dominates by far any
   rendering optimisation especially on big databases where the speed really
   matters, and DB traversing is the same for all rendering implementations. This
   is the main reason behind the fact that all renderers demonstrate comparable
   results.

   There is virtually no alternative to the DB traversing though. It  should be
   noted that each rendering view in this context is unique. Indeed, when the
   user changes the visual window, the data stream from the Toped DB traverser
   can't be predicted. Different objects will be streamed out depending on the
   location and size of the visual window, but also depending on the current
   visual properties - colours, lines, fills, layer status, cell depth, visual
   details etc. The assumption is (and I hope it's a fact) that the overall amount
   of the data is optimal with respect to the quality of the image and the speed
   of the drawing.
   The alternative to the above is to keep the entire DB in the graphic memory
   which is considered not feasible for a number of (hopefully obvious) reasons.

   All renderers are implemented as family of classes which inherit a pure virtual
   family declared here. The major differences between those class families are
   in the data collection and drawing (see the steps below). As already mentioned
   above, all renderers are sinking data from the Toped QuadTree DB and depend
   heavily on its ability to filter-out quickly and efficiently all objects
   irrelevant to the current view. The Toped DB is generally a hierarchical cell
   tree. Each cell contains a linear list of layers which in turn contain a
   QuadTree of the layout objects.

   The main task of the TrendBase is to convert
   the data stream from the DB traversing into a flat collection of object
   references (in tolder case) or in arrays of vertexes convenient for the
   graphical hardware. This is done in 3 steps:
      1. Traversing and sorting - data coming from the Toped DB traversing is
         sorted in the dedicated structures. Also some vital data statistics is
         done and some references gathered which will be used during the following
         steps
      2. VBO generation - the buffers are created and the sorted data is copied
         there. This step is virtually omitted when tolder is used.
      3. Drawing
   \verbatim
                     Layer 1                Layer 2                     Layer N
                   -------------         --------------              --------------
                  |   TrendLay   |      |  TrendLay    |            |  TrendLay    |
                  |              |      |              |            |              |
                  |  ----------  |      |  ----------  |            |  ----------  |
      cell_A      | | TrendTV  | |      | | TrendTV  | |     |      | | TrendTV  | |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
      cell_B      | | TrendTV  | |      | | TrendTV  | |     |      | | TrendTV  | |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
                  |    ______    |      |    ______    |     |      |    ______    |
                  |              |      |              |     |      |              |
                  |  ----------  |      |  ----------  |     |      |  ----------  |
      cell_Z      | | TrendTV  | |      | | TrendTV  | |     |      | | TrendTV  | |
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
   The first step is the most time consuming, but this is mainly the Toped DB
   traversing which is assumed optimal. The last one is the quickest one and the
   speed there depends on the current graphic platform. It is expected to improve
   from tolder < tenderer < toshader. The speed of the second step depends entirely
   on the implementation of the active renderer. It shall be noted that for the
   tolder second step is virtually missing, but the third step is expected to be
   much slower in comparison with the other two implementations.

   Memory usage:
   Only the first step consumes memory but it is minimised. There is no data
   copying instead data references are stored only. Technically the second step
   does consume memory of course, but this is the memory consumed by the graphic
   driver to map the VBO in the CPU memory. The TrendBase copies the data directly
   in the VBOs.
   Note, that the flow described above allows the screen to be redrawn very quickly
   using the third step only. This opens the door to all kinds of graphical effects.

   As shown on the graph above the TrendBase ends-up with a number of VBOs for
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
#ifndef BASETREND_H
#define BASETREND_H

#include <GL/glew.h>
#include "trendat.h"
#include <sstream>
#include <fstream>
// to cast properly the indices parameter in glDrawElements when
// drawing from VBO
//#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

// to avoid the compiler warnings the above define was replaced by this below
// (plus the line in the basetrend.cpp). This is taken from
// https://stackoverflow.com/questions/23177229/how-to-cast-int-to-const-glvoid
typedef void (*TFPTR_DrawElementsOffset)(GLenum,GLsizei,GLenum,uintptr_t);
extern TFPTR_DrawElementsOffset tpd_glDrawElements;

// Forward declarations
namespace layprop { class LayoutGrid;}
//=============================================================================
//
//
//
//=============================================================================
namespace trend {

   //! All uniform Variables in the shaders in the form glslu_<variable_name>
   enum glsl_Uniforms {  glslu_in_CTM
                       , glslu_in_Z
                       , glslu_in_Color
                       , glslu_in_Stipple
                       , glslu_in_LStipple
                       , glslu_in_LWidth
                       , glslu_in_StippleEn
                       , glslu_in_LStippleEn
                       , glslu_in_MStippleEn
                       , glslu_in_ScreenSize
                       , glslu_in_PatScale
                      };
   enum glsl_Programs { glslp_NULL
                       ,glslp_VF  //! Vertex and Fragment (default)
                       ,glslp_VG  //! Vertex Geometry and Fragment (line stipple)
                       ,glslp_PS  //! Vertex Geometry and Fragment (point sprites)
                       ,glslp_FB  //! Final rendering step when frame buffers are in use
                      };
   //! The actual location of all uniform variables in the shaders after glLinkProgram
   typedef std::map<glsl_Uniforms, GLint>           GlslUniVarLoc;
   typedef std::map<glsl_Programs, GlslUniVarLoc>   GlslUniVarAllLoc;
   typedef std::map<glsl_Uniforms, std::string>     GlslUniVarNames;
   typedef std::map<glsl_Programs, GlslUniVarNames> GlslUniVarAllNames;
   typedef std::map<glsl_Programs, GLint>           GlslProgramIDs;


   typedef std::list<TrxCnvx*>      SliceObjects;
   typedef std::list<TrxNcvx*>      SlicePolygons;
   typedef std::list<TrxWire*>      SliceWires;
   typedef std::list<TrxSelected*>  SliceSelected;
   typedef std::list<TrxCellRef*>   RefBoxList;

   /**
      Toped RENDerer Translation View - is the most fundamental class of the
      TrendBase. It sorts and stores a layer slice of the cell data. Most of
      the memory consumption during the first processing step (traversing and
      sorting) is concentrated in this class and naturally most of the TrendBase
      data processing happens in the methods of this class. The input data stream
      is sorted in 4 "bins" in a form of Tender object lists.
      - cont: holding contours of all abjects
      - line: holding central lines of wires only
      - cnvx: holding all convex polygons (box type only)
      - ncvx: non-convex polygons
      The corresponding
      objects are created by register* methods. Vertex or index data is not
      copied during this process, instead only data references are stored. The
      exception here are the wire objects which generate their contour data
      during the construction. Each object is sorted in exactly one bin depending
      whether it's going to be filled or not. The exception as always is the wire
      object which also goes to the line bin because of its central line.
    
      \verbatim
      -----------------------------------------------------------------
      | TrendBase |    not filled      |        filled      |          |
      |   data    |--------------------|--------------------|  enum    |
      | (vertexes)|  box | poly | wire |  box | poly | wire |          |
      |-----------|------|------|------|------|------|------|----------|
      | contour   |  x   |  x   |  x   |      |      |      |  OTcont  |
      | line      |      |      |  x   |      |      |  x   |  OTline  |
      | convex    |      |      |      |  x   |      |      |  OTcnvx  |
      | non-convex|      |      |      |      |  x   |  x   |  OTncvx  |
      ----------------------------------------------------------------
      \endverbatim

      In case of 3D rendering however the bins described above are not necessary.
      All objects will turn into 3D objects, all objects will be filled with
      a texture, there will not be any contours or lines drawn. All objects will
      be drawn in the same way - i.e. no matter convex or non-convex. All objects
      will need at least 12 triangles out of 8 vertices (in case for a box). All of
      them will use index buffer to save some bandwidth ti the graphic card. In
      a way, this makes the things much easier. In result - we'll have
      only one bin.

      \verbatim
      -------------------------------------------
      | TrendBase |        filled      |        |
      |   data    |--------------------|  enum  |
      | (vertexes)|  box | poly | wire |        |
      |-----------|------|------|------|--------|
      | non-convex|  x   |  x   |  x   |  ncvx  |
      -----------------------------------------
      \endverbatim

      The sorting step gathers also essential data statistics which will be used
      later to constitute the virtual buffers, to copy the data into the
      appropriate locations of the corresponding buffers and to draw the appropriate
      portions of the data using the appropriate openGL functions. This data is
      stored in the class fields in the form of four arrays with size of four each.
      One position per bin is allocated in each of those arrays (_vrtxnum[4],
      _alobjvx[4], _sizesvx[4], _firstvx[4]) indicating the overall number of
      vertexes, the total number of objects, the size of each object in terms of
      vertexes, and the offset of each first object vertex in the buffer

      If the non-convex data is to be filled, it is sorted further in another four
      index bins. The data statistics is also split further into four to accommodate
      that additional level of detail. This is necessary because the polygon
      tesselation generates 2 types of index data. In addition the wire tesselation
      is using yet another type of index data and all the above has to be sorted and
      stored. The array fields
    
    // vertex related data
    unsigned          _vrtxnum[4]; //! array with the total number of vertexes
    unsigned          _alobjvx[4]; //! array with the total number of objects that will be drawn with vertex related functions
    // index related data for non-convex polygons
    unsigned          _alindxs[4]; //! array with the total number of indexes
    unsigned          _alobjix[4]; //! array with the total number of objects that will be drawn with index related functions

      - _alindxs[4] - All indexes
      - _alobjix[4] - All index sequences
      - _sizesix[4] - array with number of indexes per object
      - _firstix[4] - array with first indexes of every object
      .
      hold a position for each of the four index bins. They represent the overall
      amount of indexes, the total number of indexed objects, the size of each
      object in terms of indexes and the offset of each first object index in the
      buffer.It has to be noted also that the openGL tesselation (used for
      non-convex polygons) produces a number of chunks of index data and each of
      them can be of any of the first 3 types. Wire tesselation produces a single
      chunk and it is always from one and the same type.  The table below summaries
      the indexing.

      \verbatim
      ------------------------------------------------------------------
      | Tesselation    | non convex object  |          |      openGL      |
      |         data   |--------------------|  enum    |     drawing      |
      |   (indexes)    |  polygon |  wire   |          |      method      |
      |----------------|----------|---------|----------|------------------|
      | triangles      |     x    |         |  ITtria  | GL_TRIANGLES     |
      | triangle fan   |     x    |         |  ITftfs  | GL_TRIANGLE_FAN  |
      | triangle strip |     x    |         |  ITtstr  | GL_TRIANGLE_STRIP|
      | quadratic strip|          |    x    |  ITfqss  | GL_QUAD_STRIP    |
      -------------------------------------------------------------------
      \endverbatim

      The collect() method implements the second step of the processing namely
      data collection (copy). It puts all vertex data in the linear vertex buffer
      structured in the following way

      \verbatim
      -------------------------------------------------------------------------
      ... || cont | line  | cnvx | ncvx || cont | line  | cnvx | ncvx || ...
      ... ||----------------------------||----------------------------|| ...
      ... || this TrendTV object data   ||  next TrendTV object data  || ...
      -------------------------------------------------------------------------
      \endverbatim

      and the corresponding index data in another index buffer with the following
      structure

      \verbatim
      -------------------------------------------------------------------------
      ... || ftrs | ftfs  | ftss | fqss || ftrs | ftfs  | ftss | fqss || ...
      ... ||----------------------------||----------------------------|| ...
      ... ||  this TrendTV index data   ||  next TrendTV index data   || ...
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
   class TrendTV {
      public:
         enum ObjectTypes {OTcntr,  // countour object
                           OTline,  // line object
                           OTcnvx,  // convex object
                           OTncvx,  // non-convex object, requires indexation
                           OBJ_TYPES};
         enum IndexTypes  {ITtria,  // triangles
                           ITtstr,  // triangle strips
                           IDX_TYPES};
                           TrendTV(TrxCellRef* const, bool, bool, unsigned, unsigned);
         virtual          ~TrendTV();
         void              registerBox   (TrxCnvx*);
         void              registerPoly  (TrxNcvx*, const TessellPoly*);
         void              registerWire  (TrxWire*);
         void              registerText  (TrxText*, TrxTextOvlBox*);

         void              register3DBox   (Trx3DBox*);
         void              register3DPoly  (Trx3DPoly*, const TessellPoly*);
         void              register3DWire  (Trx3DWire*);

         virtual void      collect(TPVX&, unsigned int*)  {assert(false);}
         virtual void      draw(layprop::DrawProperties*) = 0;
         virtual void      drawTexts(layprop::DrawProperties*) = 0;
         TrxCellRef*       swapRefCells(TrxCellRef*);

         unsigned          num_total_points();
         unsigned          num_total_indexs();
         unsigned          num_total_strings()  {return _num_total_strings;}
         bool              reusable() const     {return _reusable;}
         bool              filled() const       {return _filled;}
         std::string       cellName()           {return _refCell->name();}
      protected:
         virtual void      setAlpha(layprop::DrawProperties*);
         TrxCellRef*       _refCell;
         // collected data lists
         SliceObjects      _cont_data; //! Contour data
         SliceWires        _line_data; //! Line data
         SliceObjects      _cnvx_data; //! Convex polygon data (Only boxes are here at the moment. TODO - all convex polygons)
         SlicePolygons     _ncvx_data; //! Non convex data
         TrendStrings      _text_data; //! Text (strings)
         RefTxtList        _txto_data; //! Text overlapping boxes
         // vertex related data
         unsigned          _vrtxnum[OBJ_TYPES]; //! array with the total number of vertexes
         unsigned          _vobjnum[OBJ_TYPES]; //! array with the total number of objects that will be drawn with vertex related functions
         // index related data for non-convex polygons
         unsigned          _indxnum[IDX_TYPES]; //! array with the total number of indexes
         unsigned          _iobjnum[IDX_TYPES]; //! array with the total number of objects that will be drawn with index related functions
         //
         unsigned          _num_total_strings;
         bool              _filled;
         bool              _reusable;
         void              DEBUGprintOGLdata(const unsigned start, GLuint **_firstix, GLsizei **_sizesix, unsigned int *index_array, TPVX &point_array, unsigned int *size_index);


//         bool              _rend3D;
   };

   /**
      Very small class which holds the reusable TrendTV chunks. Here is what it is.
      The view usually contains relatively small number of cells which are referenced
      multiply times. They do contain the same data, but it should be visualised
      using different translation matrices. The simplest case is an array of cells
      object. It would be really a waste of CPU time to traverse those cells every
      time they are referenced. This also means that the amount of processing data
      will be determined by the number of references, not by the number of unique
      cells which in turn will slow-down the rendering and will consume much more
      memory and bandwidth to transfer it to the GPU.

      Here is the alternative approach. Each cell layer (chunk) is checked during
      the traversing for its location. There can be 3 cases:

         - the chunk is outside the view window - it is then discarded and is not
         reaching the TrendBase at all.
         - the chunk is partially inside the view window - it is traversed, but
         its data is considered unique and no further attempts to reuse it.
         - the chunk is entirely inside the view window

      The latter case is the interesting one. Before creating a new chunk of data
      (TrendTV object), TrendBase is checking whether it already exists
      (TrendLay::chunkExists). If it does - it is registered here, a new TrendReTV
      object is created and the traversing of the current layer is skipped. If the
      chunk doesn't exists, then a TrendTV object is created, but it is also
      registered as "reusable" and the traversing continues as normal.

      This class requires only a draw() method which takes care to replace temporary
      the translation matrix of the referenced TrendTV and then calls TrendTV::draw()
   */
   class TrendReTV {
      public:
                           TrendReTV(TrendTV* const chunk, TrxCellRef* const refCell):
                              _chunk(chunk), _refCell(refCell) {}
         virtual          ~TrendReTV() {}
         virtual void      draw(layprop::DrawProperties*) = 0;
         virtual void      drawTexts(layprop::DrawProperties*) = 0;
      protected:
         TrendTV*  const   _chunk;
         TrxCellRef* const _refCell;

   };

   /**
      Toped RENDering LAYer represents a DB layer in the rendering view. It
      generates and stores all the TrendTV objects (one per DB cell).

      A new TederTV object is created by the newSlice() method during the DB
      traversing. A reference to it is stored in the _layData list as well as
      in the _cslice field. All subsequent calls to box(), poly() and wire()
      methods create the corresponding object of class Trend* and store them
      calling the corresponding methods of _cslice object (of TrendTV class).
      If the data is selected then an object of the overloaded class is created
      and a reference to it is stored in _slct_data list as well.

      TrendLay keeps track of the total current amount of vertexes
      (_num_total_points) and indexes (_num_total_indexs) in order to initialise
      properly the offset fields of the generated TrendTV objects. When a new
      slice of class TrendTV is created, the current slice is post processed
      (method ppSlice()) in order to update the fields mentioned above. The
      final values stored in those fields are used by the child classes later
      to reserve eventually a proper amount of memory for the VBO's belonging
      to this layer.

      The rendering of the selected objects is done in this class. TrendTV does
      hold the object vertex data, but it is completely unaware of selection
      related properties. The primary reason for this is that data can be
      selected only in one of the generated TrendTV objects, because only one
      of them eventually belongs to the active cell. Having an additional VBO
      per TrendTV object for selected data would be a waste of course, because
      all but one of them will be empty anyway. Instead a single VBO is generated
      for the entire rendering view with the indexes of all selected vertexes in
      the active cell across all layers.

      Selected objects will be rendered twice. Once - together with all other
      objects in the TrendTV objects and again here, in this class in order to
      highlight them. An object can be fully or partially selected and depending
      on this it shall be rendered using the corresponding data structure:

      \verbatim
       --------------------------------------------------------------
      | TrendBase |  fully selected    | partially selected |        |
      |    data   |--------------------|--------------------|  enum  |
      | (indexes) |  box | poly | wire |  box | poly | wire |        |
      |-----------|------|------|------|------|------|------|--------|
      |line strips|      |      |      |   x  |   x  |   x  |  lstr  |
      | contours  |  x   |  x   |      |      |      |      |  llps  |
      |   lines   |      |      |  x   |      |      |      |  lnes  |
      --------------------------------------------------------------
      \endverbatim

      In the manner similar to TrendTV class, TrendLay sorts the selected objects
      in three "bins" and gathers some statistics about each of them. One position
      per bin is stored in each of the array fields (_asindxs[3], _asobjix[3],
      _sizslix[3], _fstslix[3]) representing the overall amount of indexes, the
      total number of indexed objects, the size of each object in terms of indexes
      and the offset of each first object index in the buffer. Additionally every
      selected object will need to store the offset of its vertex data within the
      slice representing the active cell in the vertex buffer. This is done by the
      selected objects themselves when vertex VBOs are collected via the
      re-implementation of cDataCopy virtual methods in the TenderS* classes.
      TrendLay doesn't take part in this process, TrendTV class is not aware of
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
      ... ||  this TrendLay index data ||  next TrendLay index data || ...
      -------------------------------------------------------------------------
      \endverbatim

      The drawing of the corresponding portion of the selected data buffer is done
      also in this class by the drawSelected() method. For this we need the vertex
      buffer and a proper offset in it which will point to the beginning of the data
      slice belonging to the active cell. This offset is captured during the data
      traversing in the field _stv_array_offset. Having done that, the drawing is
      now trivial

    \verbatim
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
    \endverbatim
   */
   class TrendLay {
      public:
         typedef std::list<TrendTV*>     TrendTVList;
         typedef std::list<TrendReTV*>   TrendReTVList;
         typedef std::map<std::string, TrendTV*> ReusableTTVMap;

                           TrendLay(bool);
         virtual          ~TrendLay();
         void              box  (const int4b*);
         void              box  (const int4b*,                               const SGBitSet*);
         void              box  (const int4b*,                               const SGBitSet*, const CTM&);
         void              box  (const TP&, const CTM&);
         void              poly (const int4b*, unsigned, const TessellPoly*);
         void              poly (const int4b*, unsigned, const TessellPoly*, const SGBitSet*);
         void              poly (const int4b*, unsigned, const TessellPoly*, const SGBitSet*, const CTM&);
         void              poly (const PointVector&, const CTM&);
         void              wire (int4b*, unsigned, WireWidth, bool);
         void              wire (int4b*, unsigned, WireWidth, bool, const SGBitSet*);
         void              wire (int4b*, unsigned, WireWidth, bool, const SGBitSet*, const CTM&);
         void              wire (const PointVector&, WireWidth, bool, const CTM&);
         void              text (const std::string*, const CTM&, const DBbox*, const TP&, bool);
         virtual void      newSlice(TrxCellRef* const, bool, bool /*, bool, unsigned*/) = 0;
         virtual void      newSlice(TrxCellRef* const, bool, bool, unsigned slctd_array_offset) = 0;
         virtual bool      chunkExists(TrxCellRef* const, bool) = 0;
         void              ppSlice();
         virtual void      draw(layprop::DrawProperties*) = 0;
         virtual void      drawSelected() = 0;
         virtual void      drawTexts(layprop::DrawProperties*) = 0;
         virtual void      collect(GLuint, GLuint) { assert(false); }
         virtual void      collectSelected(unsigned int*) { assert(false); }
         unsigned          total_points() {return _num_total_points;}
         unsigned          total_indexs() {return _num_total_indexs;}
         unsigned          total_slctdx();
         unsigned          total_strings(){return _num_total_strings;}

      protected:
         void              registerSBox  (TrxSBox*);
         void              registerSPoly (TrxSNcvx*);
         void              registerSWire (TrxSWire*);
         void              registerSOBox (TrxTextSOvlBox*);
         ReusableTTVMap    _reusableFData; // reusable filled chunks
         ReusableTTVMap    _reusableCData; // reusable contour chunks
         TrendTVList       _layData;
         TrendReTVList     _reLayData;
         TrendTV*          _cslice;    //!Working variable pointing to the current slice
         unsigned          _num_total_points;
         unsigned          _num_total_indexs;
         unsigned          _num_total_slctdx;
         unsigned          _num_total_strings;
         // Data related to selected objects
         SliceSelected     _slct_data;
         // index related data for selected objects
         unsigned          _asindxs[SLCT_TYPES]; //! array with the total number of indexes of selected objects
         unsigned          _asobjix[SLCT_TYPES]; //! array with the total number of selected objects
         bool              _rend3D;
   };

   /**
      Responsible for visualising the overlap boxes of the references. Pure virtual and
      relatively trivial class. One object of this class should be created only. All
      reference boxes are processed in a single VBO. This includes the selected ones.
   */
   class TrendRefLay {
      public:
                           TrendRefLay();
         virtual          ~TrendRefLay();
         void              addCellOBox(TrxCellRef*, word, bool);
         virtual void      collect(GLuint)  { assert(false); }
         virtual void      draw(layprop::DrawProperties*) = 0;
         unsigned          total_points();
         unsigned          total_indexes();
      protected:
         virtual void      setLine(layprop::DrawProperties*,bool) = 0;
         RefBoxList        _cellRefBoxes;
         RefBoxList        _cellSRefBoxes;
         // vertex related data
         unsigned          _alvrtxs; //! total number of vertexes
         unsigned          _alobjvx; //! total number of objects that will be drawn with vertex related functions
         // index related data for selected boxes
         unsigned          _asindxs; //! total number of selected vertexes
         unsigned          _asobjix; //! total number of objects that will be drawn with index related functions
   };

   class TrendMarks {
      public:
                           TrendMarks() {}
         virtual          ~TrendMarks() {}
         void              addRefMark (const TP& p, const CTM& ctm) {_refMarks.push_back(p*ctm);}
         void              addTextMark(const TP& p, const CTM& ctm) {_textMarks.push_back(p*ctm);}
         void              addARefMark(const TP& p, const CTM& ctm) {_arefMarks.push_back(p*ctm);}
         unsigned          total_points();
         virtual void      draw(layprop::DrawProperties*) = 0;
         virtual void      collect(GLuint)  { assert(false);}
      protected:
         typedef std::list<TP> PointList;
         PointList         _refMarks;
         PointList         _textMarks;
         PointList         _arefMarks;
   };

   class TrendGrid {
      public:
                  TrendGrid(unsigned size = 0, int* array = NULL, std::string color="") :
                     _size   ( size   ),
                     _array  ( array  ),
                     _color  ( color  ) {};
                 ~TrendGrid() {if (NULL != _array) delete [] _array;}
         unsigned          size () { return _size ;}
         const int*        array() { return _array;}
         std::string       color() { return _color;}
      private:
         unsigned          _size;
         int*              _array;
         std::string       _color;
   };

   class TrendGridC {
      public:
                           TrendGridC(TP, TP, int step, std::string color);
         unsigned          dump(TNDR_GLDATAT*, unsigned);
         unsigned          asize() {return _asize;}
         std::string       color() {return _color;}
      private:
         void              calculate();
         TP                _bl;
         TP                _tr;
         int               _step;
         std::string       _color;
         int               _X;
         int               _Y;
         unsigned          _asize;
   };
   //-----------------------------------------------------------------------------
   //
   typedef laydata::LayerContainer<TrendLay*> DataLay;
   typedef std::stack<TrxCellRef*> CellStack;
   typedef std::list<TrendGrid*> TrendGrids;
   typedef std::list<TrendGridC*> VGrids;
   /**
      Toped RENDerer BASE is the front-end class, the interface to the rest of
      the world. Pure virtual. All data collection and render views are initiated
      using an object of this class. There should be only one object of this
      class at a time. The object of this class must be destroyed before the next
      data collection or rendering view is invoked. The data gathered from the
      previous view must be considered invalid. The object structure created by
      this class is shown in the documentation of the module.
   */
   class TrendBase {
      public:
                           TrendBase( layprop::DrawProperties* drawprop, real UU );
         virtual          ~TrendBase();
         virtual void      setLayer(const LayerDef&, bool) = 0;
         virtual void      setHvrLayer(const LayerDef&) = 0;
         virtual void      setGrcLayer(bool, const LayerDef&) = 0;
         virtual bool      chunkExists(const LayerDef&, bool) = 0;
         void              pushCell(std::string, const CTM&, const DBbox&, bool, bool);
         void              setRmm(const CTM&);
         void              popCell()                              {_cellStack.pop();}
         const CTM&        topCTM() const                         {return  _cellStack.top()->ctm();}
         void              box  (const int4b* pdata)              {_clayer->box(pdata);}
         void              box  (const int4b* pdata, const SGBitSet* ss){_clayer->box(pdata, ss);}
         void              boxm (const int4b* pdata, const SGBitSet* ss){assert(_rmm);_clayer->box(pdata, ss, *_rmm);}
         void              boxt (const TP& p1)                    {_clayer->box(p1, *_rmm);}
         void              poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly)
                                                                  {_clayer->poly(pdata, psize, tpoly);}
         void              poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss)
                                                                  {_clayer->poly(pdata, psize, tpoly, ss);}
         void              polym(const int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss)
                                                                  {assert(_rmm);_clayer->poly(pdata, psize, tpoly, ss, *_rmm);}
         void              polyt(const PointVector& pdata)        {_clayer->poly(pdata, *_rmm);}
         void              grcpoly(int4b* pdata, unsigned psize);
         void              wire (int4b*, unsigned, WireWidth);
         void              wire (int4b*, unsigned, WireWidth, const SGBitSet*);
         void              wirem(int4b*, unsigned, WireWidth, const SGBitSet*);
         void              wiret(const PointVector&, WireWidth);
         void              grcwire (int4b*, unsigned, WireWidth);
         void              arefOBox(std::string, const CTM&, const DBbox&, bool);
         void              text (const std::string*, const CTM&, const DBbox&, const TP&, bool);
         void              textt(const std::string*, const CTM&, const TP&);

         virtual bool      collect() = 0;
         virtual bool      grcCollect() = 0;
         virtual bool      grdCollect(const layprop::LayoutGrid**) = 0;
         virtual bool      rlrCollect(const layprop::RulerList&, int4b, const DBlineList&) = 0;

         virtual void      draw() = 0;
         virtual void      grcDraw() = 0;
         virtual void      grdDraw() = 0;
         virtual void      rlrDraw()=0;
         void              set3Drendering()              { _rend3D = true;}
         bool              rend3D() const                { return _rend3D;}
      
         LayerDef          getTenderLay(const LayerDef& laydef)
                                                         {return _drawprop->getTenderLay(laydef)   ;}
         void              setState(layprop::PropertyState state)
                                                         {        _drawprop->setState(state)       ;}
         bool              layerHidden(const LayerDef& laydef) const
                                                         {return _drawprop->layerHidden(laydef)    ;}
         const CTM&        scrCTM() const                {return _drawprop->scrCtm()               ;}
         word              visualLimit() const           {return _drawprop->visualLimit()          ;}
         const DBbox&      clipRegion() const            {return _drawprop->clipRegion()           ;}
         void              postCheckCRS(const laydata::TdtCellRef* ref)
                                                         {        _drawprop->postCheckCRS(ref)     ;}
         bool              preCheckCRS(const laydata::TdtCellRef*, layprop::CellRefChainType&);
         void              initDrawRefStack(laydata::CellRefStack* crs)
                                                         {       _drawprop->initDrawRefStack(crs)  ;}
         void              clearDrawRefStack()           {       _drawprop->clearDrawRefStack()    ;}
         bool              adjustTextOrientation() const {return _drawprop->adjustTextOrientation();}
         layprop::DrawProperties*&   drawprop()          {return _drawprop                         ;}
         void              setDrawProp(layprop::DrawProperties* drawprop)
                                                         {       _drawprop = drawprop              ;}
         bool              grcDataEmpty()                {return _grcData.empty()                  ;}
      protected:
         virtual void      cleanUp();
         virtual void      grcCleanUp();
         virtual void      grdCleanUp();
         virtual void      rlrCleanUp();
         virtual void      setLayColor(const LayerDef& layer) = 0;
         virtual void      setStipple() = 0;
         virtual void      setLine(bool) = 0;
         void              genRulerMarks (const CTM&, DBline&, DBline&, DBline&, double&);
         layprop::DrawProperties*   _drawprop;
         real              _UU;
         DataLay           _data;            //!All editable data for drawing
         DataLay           _grcData;         //!All GRC      data for drawing
         TrendLay*         _clayer;          //!Working variable pointing to the current edit slice
         TrendLay*         _grcLayer;        //!Working variable pointing to the current GRC  slice
         TrendRefLay*      _refLayer;        //!All cell references with visible overlapping boxes
         CellStack         _cellStack;       //!Required during data traversing stage
         unsigned          _cslctd_array_offset; //! Current selected array offset
         //
         TrxCellRef*       _activeCS;
         byte              _dovCorrection;   //!Cell ref Depth of view correction (for Edit in Place purposes)
         RefBoxList        _hiddenRefBoxes;  //!Those cRefBox objects which didn't ended in the TrendRefLay structures
         TrendMarks*       _marks;           //!All kinds of object marks
         CTM*              _rmm;             //!Reverse motion matrix
         VGrids            _grid_props;      //! The properties of all visual grids
         unsigned          _num_grid_points; //! Number of all points in all grids
//         TrendGrids        _grids;           //!All grid points
         TrendStrings      _rulerTexts;      //!The labels on all rulers
         bool              _rend3D;

   };

   class ogl_logfile {
      public:
                           ogl_logfile() :_enabled(false) {};
         void              init(const std::string logFileName);
         void              close();
         ogl_logfile&      operator<< (const std::string&);
         ogl_logfile&      operator<< (const unsigned int _i);
         ogl_logfile&      flush();
      private:
         std::fstream     _file;
         bool             _enabled;
   };

   void checkOGLError(std::string);
   void reportOGLStatus(std::string loc);
//   void dumpOGLArrayFloat(const GLfloat* cindex_array, unsigned);
//   void dumpOGLArrayUint(const unsigned* cindex_array, unsigned);

   template <typename result_t, typename... gl_args_t, typename... args_t>
   result_t dbgl_call(char* fname, int lineNo, char* varName, result_t (*fun)(gl_args_t...), args_t... args) {
      std::ostringstream info;
      info << varName << " in "<< fname << " at line " << lineNo << " : ";
      if constexpr(!std::is_same<result_t, void>::value) {
         auto result = fun(std::forward<args_t>(args)...);
#ifndef NDEBUG
         trend::reportOGLStatus(info.str());
#endif
         return result;
       } else {
         fun(std::forward<args_t>(args)...);
#ifndef NDEBUG
         trend::reportOGLStatus(info.str());
#endif
       }
   }
}


#endif //BASETREND_H
