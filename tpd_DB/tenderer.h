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
/*!
The idea behind the Toped renderer:

*/
#ifndef TENDERER_H
#define TENDERER_H

#include <GL/glew.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

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
                       ~TeselChunk();
      GLenum            type() const      {return _type;}
      word              size() const      {return _size;}
      const unsigned*   index_seq() const {return _index_seq;}
   private:
      unsigned*         _index_seq;  // index sequence
      word              _size;       // size of the index sequence
      GLenum            _type;
};

typedef std::list<TeselChunk*> TeselChain;

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

class TeselPoly {
   public:
                        TeselPoly(const int4b* pdata, unsigned psize);
                       ~TeselPoly();
      TeselChain*       tdata()                    { return &_tdata;  }
      word              num_ftrs()                 { return _all_ftrs;}
      word              num_ftfs()                 { return _all_ftfs;}
      word              num_ftss()                 { return _all_ftss;}
      void              num_indexs(unsigned&, unsigned&, unsigned&);
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
/**
*   holds box representation - The same points will be used for the
*   contour as well as for the fill
*/
class TenderCnvx {
   public:
                        TenderCnvx(int4b* pdata, unsigned psize) :
                                 _cdata(pdata), _csize(psize){}
      virtual          ~TenderCnvx() {};
      virtual unsigned  cDataCopy(int*, unsigned&);
      unsigned          csize()     {return _csize;}
   protected:
      int4b*            _cdata;  // contour data
      unsigned          _csize;
};

/**
*   holds polygon representations - the contour will be drawn using the
*   inherited _cdata holder. The chains of point indexes resulting from the
*   tesselation are stored in _tdata. They will be used together with the _cdata
*   during the polygon fill
*/
class TenderNcvx : public TenderCnvx {
   public:
                        TenderNcvx(int4b* pdata, unsigned psize) :
                                   TenderCnvx(pdata, psize), _tdata(NULL) {}
      void              setTeselData(TeselPoly* tdata) {_tdata = tdata;}
      virtual          ~TenderNcvx(){};
      virtual TeselChain* tdata()              {return _tdata->tdata();}
   private:
      TeselPoly*        _tdata;
};

/**
*   holds wire representation - the contour and the fill - exactly as in the
*   inherited class. The _ldata stores the central line which is effectively
*   the original point list from tdtwire
*/
class TenderWire : public TenderNcvx {
   public:
                        TenderWire(int4b*, unsigned, const word, bool);
      virtual          ~TenderWire();
      void              Tesselate();
      virtual unsigned  lDataCopy(int*, unsigned&);
      unsigned          lsize()                 {return _lsize;}
      bool              center_line_only()      {return _center_line_only;}
      virtual TeselChain* tdata()               {return _tdata;}
   protected:
      void              precalc(const word);
      DBbox*            endPnts(const word, word, word, bool);
      DBbox*            mdlPnts(const word, word, word, const word);
      int*              _ldata;  // central line data
      unsigned          _lsize;
      bool              _center_line_only;
      TeselChain*       _tdata;
};

typedef enum {lins, llps, lstr} SlctTypes;

class TenderSelected {
   public:
                        TenderSelected(const SGBitSet* slist) :
                           _slist(slist), _offset(0) {}
      bool              partSelected() {return (NULL != _slist);}
      virtual SlctTypes type() = 0;
      virtual unsigned  ssize() = 0;
      virtual unsigned  sDataCopy(unsigned*, unsigned&) = 0;
   protected:
      const SGBitSet*   _slist;
      unsigned          _offset;
};

class TenderSCnvx : public TenderCnvx, public TenderSelected {
   public:
                        TenderSCnvx(int4b* pdata, unsigned psize, const SGBitSet* slist) :
                           TenderCnvx(pdata, psize), TenderSelected(slist) {}
      virtual unsigned  cDataCopy(int*, unsigned&);
      virtual SlctTypes type() { return ((NULL == _slist) ? llps : lstr);}
      virtual unsigned  ssize();
      virtual unsigned  sDataCopy(unsigned*, unsigned&);
};

class TenderSNcvx : public TenderNcvx, public TenderSelected  {
   public:
                        TenderSNcvx(int4b* pdata, unsigned psize, const SGBitSet* slist) :
                           TenderNcvx(pdata, psize), TenderSelected(slist) {}
      virtual unsigned  cDataCopy(int*, unsigned&);
      virtual SlctTypes type() { return ((NULL == _slist) ? llps : lstr);}
      virtual unsigned  ssize();
      virtual unsigned  sDataCopy(unsigned*, unsigned&);
};

class TenderSWire : public TenderWire, public TenderSelected {
   public:
                        TenderSWire(int4b* pdata, unsigned psize, const word width, bool clo, const SGBitSet* slist) :
                           TenderWire(pdata, psize, width, clo), TenderSelected(slist), _loffset(0u) {}
      virtual unsigned  cDataCopy(int*, unsigned&);
      virtual unsigned  lDataCopy(int*, unsigned&);
      virtual SlctTypes type() { return ((NULL == _slist) ? lins : lstr);}
      virtual unsigned  ssize();
      virtual unsigned  sDataCopy(unsigned*, unsigned&);
   private:
      unsigned          _loffset;
};

typedef std::list<TenderCnvx*>      SliceObjects;
typedef std::list<TenderNcvx*>      SlicePolygons;
typedef std::list<TenderWire*>      SliceWires;
typedef std::list<TenderSelected*>  SliceSelected;

/**
 *  Reference boxes
 */
class TenderRB {
   public:
                        TenderRB(const CTM&, const DBbox&);
      void              draw();
   private:
      CTM               _tmatrix;
      DBbox             _obox;
};

/**
*  translation view - effectively a layer slice of the visible cell data
*/
class TenderTV {
   public:
      enum {fqss, ftrs, ftfs, ftss} NcvxTypes;
      enum {cont, line, cnvx, ncvx} ObjtTypes;
                        TenderTV(CTM&, bool, unsigned, unsigned);
                       ~TenderTV();
      void              registerBox   (TenderCnvx*);
      void              registerPoly  (TenderNcvx*, TeselPoly*);
      void              registerWire  (TenderWire*);

      void              collect(int*, unsigned int*, unsigned int*);
      void              draw();

      unsigned          num_total_points();
      unsigned          num_total_indexs();
      const CTM*        tmatrix()            {return &_tmatrix;}
   protected:
      void              collectIndexs(unsigned int*, TeselChain*, unsigned*, unsigned*, unsigned);

      CTM               _tmatrix;
      // collected data lists
      SliceObjects      _cont_data; //! Countour data
      SliceWires        _line_data; //! Line data
      SliceObjects      _cnvx_data; //! Convex polygon data (Only boxes are here at the moment. TODO - all convex polygons)
      SlicePolygons     _ncvx_data; //! Non convex data
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
      unsigned          _point_array_offset;
      unsigned          _index_array_offset;
      //
      bool              _filled;
};

class TenderLay {
   public:
      typedef std::list<TenderTV*> TenderTVList;

                        TenderLay();
                       ~TenderLay();
      void              box  (int4b*,                       bool, const SGBitSet*);
      void              poly (int4b*, unsigned, TeselPoly*, bool, const SGBitSet*);
      void              wire (int4b*, unsigned, word, bool, bool, const SGBitSet*);

      void              newSlice(CTM&, bool, bool, unsigned);
      void              ppSlice(); //! Post process the slice
      void              draw(bool);
      void              drawSelected();
      void              collect(bool, GLuint, GLuint);
      void              collectSelected(unsigned int* /*, TeselChain*, unsigned*, unsigned*, unsigned*/);
      unsigned          total_points() {return _num_total_points;}
      unsigned          total_indexs() {return _num_total_indexs;}
      unsigned          total_slctdx();

   private:
      void              registerSBox  (TenderSCnvx*);
      void              registerSPoly (TenderSNcvx*);
      void              registerSWire (TenderSWire*);
      TenderTVList      _layData;
      TenderTV*         _cslice;    //!Working variable pointing to the current slice
      unsigned          _num_total_points;
      unsigned          _num_total_indexs;
      unsigned          _num_total_slctdx;
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
      // offsets in the VBO
      unsigned          _slctd_array_offset;
};

//-----------------------------------------------------------------------------
//
//! contains all the data across cells on a given layer
typedef std::map<word, TenderLay*> DataLay;
typedef std::list<TenderRB*> TenderRBL;

class Tenderer {
   public:
                        Tenderer( layprop::DrawProperties* drawprop, real UU );
                       ~Tenderer();
      void              Grid( const real, const std::string );
      void              setLayer(word, bool);
      void              pushCell(const CTM&, const DBbox&, bool, bool);
      void              popCTM()                               {_drawprop->popCTM(); _ctrans = _drawprop->topCTM();}
      void              box  (int4b* pdata)                    {_clayer->box(pdata, false, NULL);}
      void              box  (int4b* pdata, const SGBitSet* ss){_clayer->box(pdata, true, ss);}
      void              poly (int4b* pdata, unsigned psize, TeselPoly* tpoly)
                                                               {_clayer->poly(pdata, psize, tpoly, false, NULL);}
      void              poly (int4b* pdata, unsigned psize, TeselPoly* tpoly, const SGBitSet* ss)
                                                               {_clayer->poly(pdata, psize, tpoly, true, ss);}
      void              wire (int4b*, unsigned, word);
      void              wire (int4b*, unsigned, word, const SGBitSet*);
      void              collect();
      void              draw();

      // temporary!
      void              initCTMstack()                {        _drawprop->initCTMstack()        ;}
      void              clearCTMstack()               {        _drawprop->clearCTMstack()       ;}
      void              setCurrentColor(word layno)   {        _drawprop->setCurrentColor(layno);}
      bool              layerHidden(word layno) const {return  _drawprop->layerHidden(layno)    ;}
      const CTM&        ScrCTM() const                {return  _drawprop->ScrCTM()              ;}
      const CTM&        topCTM() const                {return  _drawprop->topCTM()              ;}
      const DBbox&      clipRegion() const            {return  _drawprop->clipRegion()          ;}
      void              pushref(const laydata::tdtcellref* ref)
                                                      {        _drawprop->pushref(ref)          ;}
      byte              popref(const laydata::tdtcellref* ref)
                                                      {return  _drawprop->popref(ref)           ;}
   private:
      layprop::DrawProperties*   _drawprop;
      real              _UU;
      DataLay           _data;      //!All data for drawing
      TenderLay*        _clayer;    //!Working variable pointing to the current slice
      unsigned          _cslctd_array_offset; //! Current selected array offset
      TenderRBL         _oboxes;    //!All reference overlapping boxes
      TenderRBL         _osboxes;   //!All selected reference overlapping boxes
      CTM               _atrans;    //!The translation of the active cell
      CTM               _ctrans;    //!Working variable storing the current translation
      unsigned          _num_ogl_buffers;
      //
      GLuint*           _ogl_buffers; //! Array with the "names" of all openGL buffers
      GLuint            _sbuffer; //! The "name" of the selected index buffer
};

void checkOGLError(std::string);

class HiResTimer {
   public:
      HiResTimer();
      void           report(std::string);
   private:
      timeval        _start_time;
      timeval        _end_time;
#ifdef WIN32
      // System frequency of timer for Windows.
      LARGE_INTEGER  _freq;
      LARGE_INTEGER  _inittime;
#endif

};

#endif //TENDERER_H

//class TenderRefBox : public TenderCnvx {
//   public:
//                        TenderRefBox(int4b* pdata) : TenderCnvx(pdata, 4) {}
//      virtual          ~TenderRefBox() {delete [] _cdata;}
//};
// class TenderLine {
//    public:
//                         TenderLine(TenderCnvx*, const SGBitSet*);
//                         TenderLine(TenderNcvx*, const SGBitSet*);
//                         TenderLine(TenderWire*, const SGBitSet*);
//                        ~TenderLine();
//       int*              ldata()                 {return _ldata;}
//       unsigned          lsize()                 {return _lsize;}
//    private:
//       int4b*            _ldata;
//       unsigned          _lsize;
//       bool              _partial;
// };

