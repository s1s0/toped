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
//        Created: Thu Jul 24 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL 3D renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef T3DER
#define T3DER

#include "toshader.h"
namespace trend {

   void setShaderMVP(layprop::DrawProperties* drawprop, const TrxCellRef* refCell);

   class Trx3D {
      public:
                              Trx3D(const int4b* pdata, const unsigned psize) :  _cdata(pdata), _csize(psize), _tdata(NULL) {}
                             ~Trx3D() {delete _tdata;}
         void                 setTeselData(TessellPoly* tdata) {_tdata = tdata;}
         unsigned             csize() const {return _csize;}
         const TessellChain*  tdata() const {return _tdata->tdata();}
         virtual unsigned     cDataCopy(TPVX3&, unsigned&, const unsigned);//       {assert(false);}
      protected:
         const int4b*          _cdata;  //! the vertexes of the object contour
         unsigned              _csize;  //! the number of vertexes in _cdata
         TessellPoly*          _tdata;  //! polygon tesselation data
         int4b                 _z[2];    
   };

   /**
    Object of this class are used to render boxes when rend3D mode is active. The difference
    with TrxBox is that the field _tdata contains data which belongs to this object
    NOTE! All 3D objects inherit TrxNcvx - i.e. they do have a tesselation data
    */
   class Trx3DBox : public Trx3D { // the difference with TrxNcvx is the destructor!
      public:
                              Trx3DBox(const int4b* pdata) : Trx3D(pdata, 4) {};
         virtual unsigned     cDataCopy(TPVX3&, unsigned&, const unsigned);
   };

   /**
    Used to render polygons when rend3D mode is active.
    Unlike TrxNcvx, the tesselation data _tdata is not a reference to the
    tessellation data in the TDT. It copies the original, and then expands it as needed for 3D rendering
    */
   class Trx3DPoly : public Trx3D {
      public:
                              Trx3DPoly(const int4b* pdata, unsigned psize) : Trx3D(pdata, psize) {};
//         virtual unsigned     cDataCopy(TPVX3&, unsigned&, const unsigned);
   };

   /**
    Used to render wires when rend3D mode is active. Expands the TrxWire object Tesselate
    method to accomodate the 3D requirements.
    */
   class Trx3DWire : public Trx3D {
      public:
                              Trx3DWire(const int4b* pdata, unsigned psize, WireWidth width);
//                              TrxWire  (pdata, psize, width, false) {}
//         virtual unsigned     cDataCopy(TPVX3&, unsigned&, const unsigned);
         void                 Tesselate();
         const TessellPoly*   tpdata()               {return _tdata;}
   };
   
   //===========================================================================
   class T3DTV : public TrendTV{
      public:
         typedef std::list<Trx3D*>      Slice3DPolygons;

                           T3DTV(TrxCellRef* const, bool, bool, unsigned, unsigned);
         virtual          ~T3DTV();

         void              register3DBox   (Trx3DBox*);
         void              register3DPoly  (Trx3DPoly*, const TessellPoly*);
         void              register3DWire  (Trx3DWire*);

         virtual void      registerBox   (TrxCnvx*)                              {assert(false);}
         virtual void      registerPoly  (TrxNcvx*, const TessellPoly*)          {assert(false);}
         virtual void      registerWire  (TrxWire*)                              {assert(false);}
         virtual void      registerText  (TrxText*, TrxTextOvlBox*)              {assert(false);}

         virtual void      collect(TPVX&, unsigned int*)                        {assert(false);}
         void              collect(TPVX3&, unsigned int*);
         virtual void      draw(layprop::DrawProperties*);//                        {assert(false);}
         virtual void      drawTexts(layprop::DrawProperties*)                   {assert(false);} // TODO move the method away from TrendTV
//         TrxCellRef*       swapRefCells(TrxCellRef*);

         unsigned          num_total_points();
         unsigned          num_total_indexs();
///         unsigned          num_total_strings()  {return _num_total_strings;}
      protected:
         virtual void      setAlpha(layprop::DrawProperties*)                    {assert(false);}
         void              drawTriQuads();
//         TrxCellRef*       _refCell;
         // collected data lists
//         SliceObjects      _cnvx_data; //! Convex polygon data (Only boxes are here at the moment. TODO - all convex polygons)
         Slice3DPolygons   _ncvx_data; //! Non convex data

         GLsizei*          _sizesvx[OBJ_TYPES]; //! arrays of sizes for vertex sets
         GLsizei*          _firstvx[OBJ_TYPES]; //! arrays of first vertexes
         GLsizei*          _sizesix[IDX_TYPES]; //! arrays of sizes for indexes sets
         GLuint*           _firstix[IDX_TYPES]; //! arrays of first indexes
         // offsets in the VBO
         unsigned          _point_array_offset; //! The offset of this chunk of vertex data in the vertex VBO
         unsigned          _index_array_offset; //! The offset of this chunk of index  data in the index  VBO
         // vertex related data
//         unsigned          _vrtxnum[OBJ_TYPES]; //! array with the total number of vertexes
//         unsigned          _vobjnum[OBJ_TYPES]; //! array with the total number of objects that will be drawn with vertex related functions
         // index related data for non-convex polygons
//         unsigned          _indxnum[IDX_TYPES]; //! array with the total number of indexes
//         unsigned          _iobjnum[IDX_TYPES]; //! array with the total number of objects that will be drawn with index related functions
         //
///         unsigned          _num_total_strings;
//         bool              _filled;
//         bool              _reusable;
         void              collectIndexs(unsigned int*, const TessellChain*, unsigned*, unsigned*, const unsigned);

//         void              DEBUGprintOGLdata(const unsigned start, GLuint **_firstix, GLsizei **_sizesix, unsigned int *index_array, TPVX &point_array, unsigned int *size_index);


//         bool              _rend3D;
   };

   
   //===========================================================================
   class T3DLay : public TrendLay{
      public:
                           T3DLay();
         virtual          ~T3DLay();
         virtual void      box  (const int4b*);
         virtual void      poly (const int4b*, unsigned, const TessellPoly*);
         virtual void      wire (int4b*, unsigned, WireWidth, bool);
         virtual void      text (const std::string*, const CTM&, const DBbox*, const TP&, bool)          { assert(false);}

         virtual void      newSlice(TrxCellRef* const, bool, bool);
         virtual void      newSlice(TrxCellRef* const, bool, bool, unsigned /*slctd_array_offset*/)      { assert(false); }
         virtual bool      chunkExists(TrxCellRef* const, bool)                                          { assert(false); }
//         void              ppSlice();
         virtual void      draw(layprop::DrawProperties*);//                                                { assert(false); }
         virtual void      drawSelected()                                                                { assert(false); }
         virtual void      drawTexts(layprop::DrawProperties*)                                           { assert(false); }
         virtual void      collect(GLuint, GLuint);
//         virtual void      collectSelected(unsigned int*) { assert(false); }
//         unsigned          total_points() {return _num_total_points;}
//         unsigned          total_indexs() {return _num_total_indexs;}
///         unsigned          total_slctdx();
///         unsigned          total_strings(){return _num_total_strings;}

      protected:
         GLuint            _pbuffer;
         GLuint            _ibuffer;

//         void              registerSBox  (TrxSBox*);
//         void              registerSPoly (TrxSNcvx*);
//         void              registerSWire (TrxSWire*);
//         void              registerSOBox (TrxTextSOvlBox*);

//         ReusableTTVMap    _reusableFData; // reusable filled chunks
//         ReusableTTVMap    _reusableCData; // reusable contour chunks
//         TrendTVList       _layData;
//         TrendReTVList     _reLayData;
//         TrendTV*          _cslice;    //!Working variable pointing to the current slice
//         unsigned          _num_total_points;
//         unsigned          _num_total_indexs;
///         unsigned          _num_total_slctdx;
///         unsigned          _num_total_strings;
         // Data related to selected objects
///         SliceSelected     _slct_data;
///         // index related data for selected objects
///         unsigned          _asindxs[SLCT_TYPES]; //! array with the total number of indexes of selected objects
///         unsigned          _asobjix[SLCT_TYPES]; //! array with the total number of selected objects
   };
   
   //===========================================================================
   class T3Der : public TrendBase {
   public:
                        T3Der( layprop::DrawProperties* drawprop, real UU);
      virtual          ~T3Der();
      virtual void      pushCell(std::string, const CTM&, const DBbox&, bool, bool);
      virtual void      setLayer(const LayerDef&, bool);
      virtual void      setHvrLayer(const LayerDef&)                                         {assert(false);}
      virtual void      setGrcLayer(bool, const LayerDef&)                                   {assert(false);}
      virtual bool      chunkExists(const LayerDef&, bool);

      virtual bool      collect();
      virtual bool      grcCollect()                                                         {assert(false);}
      virtual bool      grdCollect(const layprop::LayoutGrid**)                              {assert(false);}
      virtual bool      rlrCollect(const layprop::RulerList&, int4b, const DBlineList&)      {assert(false);}

      virtual void      draw();//                                                               {assert(false);}
      virtual void      grcDraw()                                                            {assert(false);}
      virtual void      rlrDraw()                                                            {assert(false);}
      virtual void      grdDraw()                                                            {assert(false);}
      virtual void      arefOBox(std::string, const CTM&, const DBbox&, bool)                {assert(false);}
      virtual void      text (const std::string*, const CTM&, const DBbox&, const TP&, bool);

   protected:
//      unsigned          _cslctd_array_offset; //! Current selected array offset

      virtual void      setLayColor(const LayerDef& layer);//                                   {assert(false);}
      virtual void      setStipple()                                                         {assert(false);}
      virtual void      setLine(bool)                                                        {assert(false);}
      virtual void      cleanUp()                                                            {assert(false);}
      virtual void      grdCleanUp()                                                         {assert(false);}
      virtual void      rlrCleanUp()                                                         {assert(false);}

//      TrendRefLay*      _refLayer;        //!All cell references with visible overlapping boxes
      unsigned          _num_ogl_buffers; //! Number of generated openGL VBOs
      GLuint*           _ogl_buffers;     //! Array with the "names" of all openGL buffers

   private:
      void              windowVAO()                                                          {assert(false);}
   };
}

#endif  //T3DER
