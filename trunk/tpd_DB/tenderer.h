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

#ifndef TENDERER_H
#define TENDERER_H

#include <GL/glew.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "drawprop.h"

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
      word              num_ftrs()                 { return _num_ftrs;}
      word              num_ftfs()                 { return _num_ftfs;}
      word              num_ftss()                 { return _num_ftss;}

   private:
      TeselChain*       _the_chain;
      GLenum            _ctype;
      TeselVertices     _cindexes;
      word              _num_ftrs;
      word              _num_ftfs;
      word              _num_ftss;
      unsigned          _offset;
};

class TeselPoly {
   public:
                        TeselPoly(const int4b* pdata, unsigned psize);
                       ~TeselPoly();
      TeselChain*       tdata()                    { return &_tdata;  }
      word              num_ftrs()                 { return _num_ftrs;}
      word              num_ftfs()                 { return _num_ftfs;}
      word              num_ftss()                 { return _num_ftss;}
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
      word              _num_ftrs;
      word              _num_ftfs;
      word              _num_ftss;
};

/**
*   holds box representation - The same four points will be used for the
*   contour as well as for the fill
*/
class TenderObj {
   public:
                        TenderObj(int4b* pdata, unsigned psize) : _cdata(pdata), _csize(psize) {}
      virtual          ~TenderObj() {};
              int4b*    cdata()     {return _cdata;}  // contour data
              unsigned  csize()     {return _csize;}
      virtual int*      ldata()     {assert(0); return NULL;}
      virtual unsigned  lsize()     {assert(0); return 0   ;}
   protected:
      int4b*            _cdata;  // contour data
      unsigned          _csize;
};

class TenderRefBox : public TenderObj {
   public:
                        TenderRefBox(int4b* pdata) : TenderObj(pdata, 4) {}
      virtual          ~TenderRefBox() {delete [] _cdata;}
};
/**
*   holds polygon representations - the contour will be drawn using the
*   inherited _cdata holder. The chains of point indexes resulting from the
*   tesselation are stored in _tdata. They will be used together with the _cdata
*   during the polygon fill
*/
class TenderPoly : public TenderObj {
   public:
                        TenderPoly(int4b* pdata, unsigned psize):TenderObj(pdata, psize) {}
      void              TeselData(TeselChain*, unsigned);
      virtual          ~TenderPoly();
//      void              Tessel(TeselTempData*);
      TeselChain*       tdata()              {return &_tdata;}
/*      static GLUtriangulatorObj* tenderTesel; //! A pointer to the OpenGL object tesselator
#ifdef WIN32
      static GLvoid CALLBACK teselVertex(GLvoid *, GLvoid *);
      static GLvoid CALLBACK teselBegin(GLenum, GLvoid *);
      static GLvoid CALLBACK teselEnd(GLvoid *);
#else
      static GLvoid     teselVertex(GLvoid *, GLvoid *);
      static GLvoid     teselBegin(GLenum, GLvoid *);
      static GLvoid     teselEnd(GLvoid *);
#endif*/
   protected:
      TeselChain       _tdata;
};

/**
*   holds wire representation - the contour and the fill - exactly as in the
*   inherited class. The _ldata stores the central line which is effectively
*   the original point list from tdtwire
*/
class TenderWire : public TenderPoly {
   public:
                        TenderWire(int4b*, unsigned, const word, bool);
      virtual          ~TenderWire();
      void              Tessel(unsigned);
      virtual int*      ldata()                 {return _ldata;}
      virtual unsigned  lsize()                 {return _lsize;}
   protected:
      void              precalc(const word);
      DBbox*            endPnts(const word, word, word, bool);
      DBbox*            mdlPnts(const word, word, word, const word);
      int*              _ldata;  // central line data
      unsigned          _lsize;
};

/**
 *   holds the representation of selected objects and segments
 */
class TenderLine {
   public:
                        TenderLine(TenderObj*, const SGBitSet*);
                        TenderLine(TenderPoly*, const SGBitSet*);
                        TenderLine(TenderWire*, const SGBitSet*);
                       ~TenderLine();
      int*              ldata()                 {return _ldata;}
      unsigned          lsize()                 {return _lsize;}
   private:
      int4b*            _ldata;
      unsigned          _lsize;
      bool              _partial;
};

typedef std::list<TenderObj*>  SliceObjects;
typedef std::list<TenderPoly*> SlicePolygons;
typedef std::list<TenderLine*> SliceLines;

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
 *  Translation View of the selected shapes
 */
class TenderTVB {
   public:
                        TenderTVB();
      void              add(TenderObj*, const SGBitSet*);
      void              add(TenderPoly*, const SGBitSet*);
      void              add(TenderWire*, const SGBitSet*);
      void              draw_lloops();
      void              draw_lines();
      void              draw_lsegments();
   private:
      SliceLines        _ln_data;
      SliceLines        _ll_data;
      SliceLines        _ls_data;
      unsigned          _num_ln_points;
      unsigned          _num_ll_points;
      unsigned          _num_ls_points;
      unsigned          _num_ln; // lines
      unsigned          _num_ll; // line loop
      unsigned          _num_ls; // line segment
};

/**
*  translation view - effectively a layer slice of the visible cell data
*/
class TenderTV {
   public:
                        TenderTV(CTM&, bool);
      TenderObj*        box  (int4b*);
      TenderPoly*       poly (int4b*, unsigned, TeselPoly*);
      TenderWire*       wire (int4b*, unsigned, word, bool);
      const CTM*        tmatrix()            {return &_tmatrix;}
      void              draw_contours();
      void              draw_lines();
      void              draw_fqus();
      void              draw_fpolygons();
   private:
      CTM               _tmatrix;
      SliceObjects      _contour_data;
      SliceObjects      _line_data;
      SliceObjects      _fqu_data;
      SlicePolygons     _fpolygon_data;
      unsigned          _num_contour_points; //
      unsigned          _num_line_points;    // central line of wires
      unsigned          _num_polygon_points; // non-convex (polygons & wires)
      unsigned          _num_contours;
      unsigned          _num_lines;
      unsigned          _num_fqus; //! fill quad
      unsigned          _num_fqss; //! fill quad strip
      unsigned          _num_ftrs; //! fill triangle
      unsigned          _num_ftfs; //! fill triangle fan
      unsigned          _num_ftss; //! fill triangle strip
      bool              _filled;
};

//-----------------------------------------------------------------------------
//
//! contains all the data across cells on a given layer
typedef std::list<TenderTV*> TenderLay;
typedef std::map<word, TenderLay*> DataLay;
typedef std::map<word, TenderTVB*> DataSel;
typedef std::list<TenderRB*> TenderRBL;

class Tenderer {
   public:
                        Tenderer( layprop::DrawProperties* drawprop, real UU );
//                     ~Tenderer();
      void              Grid( const real, const std::string );
      void              setLayer(word);
      void              setSdataContainer(word);
      void              pushCell(const CTM&, const DBbox&, bool, bool);
      void              popCTM()                               {_drawprop->popCTM(); _ctrans = _drawprop->topCTM();}
      void              box  (int4b* pdata)                    {_cslice->box(pdata);}
      void              box  (int4b*, const SGBitSet*);
      void              poly (int4b* pdata, unsigned psize, TeselPoly* tpoly)    {_cslice->poly(pdata, psize, tpoly);}
      void              poly (int4b*, unsigned, TeselPoly* , const SGBitSet*);
      void              wire (int4b*, unsigned, word);
      void              wire (int4b*, unsigned, word, const SGBitSet*);
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
      DataSel           _sdata;     //!Selected data - to be redrawn on top of the _data
      TenderTV*         _cslice;    //!Working variable pointing to the current slice
      TenderTVB*        _sslice;    //!Selected shapes in the active cell
      TenderRBL         _oboxes;    //!All reference overlapping boxes
      TenderRBL         _osboxes;   //!All selected reference overlapping boxes
      CTM               _atrans;    //!The translation of the active cell
      CTM               _ctrans;    //!Working variable storing the current translation
};


class HiResTimer {
   public:
      HiResTimer();
      void           report(char*);
   private:
      timeval        _start_time;
      timeval        _end_time;
#ifdef WIN32
		// System frequency of timer for Windows.
		LARGE_INTEGER	_freq;
		LARGE_INTEGER	_inittime;
#endif

};

#endif //TENDERER_H
