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
//        Created: Sun Jan 11 2008
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

#include <GL/glu.h>
#include "drawprop.h"

typedef std::list<word> TeselVertices;

class TeselChunk {
   public:
                        TeselChunk(const TeselVertices&, GLenum, unsigned);
                        TeselChunk(const int*, unsigned, unsigned);
                       ~TeselChunk();
      GLenum            type()     {return _type;}
      word              size()     {return _size;}
      unsigned*         index_seq(){return _index_seq;}
   private:
      unsigned*         _index_seq; // index sequence
      word              _size;      // size of the index sequence
      GLenum            _type;
};

typedef std::list<TeselChunk*> TeselChain;

class TeselTempData {
   public:
                        TeselTempData(unsigned);
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


//-----------------------------------------------------------------------------
// holds box representation - The same four points will be used for the
// contour as well as for the fill
//
class TenderObj {
   public:
                        TenderObj():_cdata(NULL), _csize(0) {}
                        TenderObj(const TP*, const TP*);
      virtual          ~TenderObj();
      virtual int*      cdata()     {return _cdata;}  // contour data
      virtual unsigned  csize()     {return _csize;}
      virtual int*      ldata()     {assert(0); return NULL;}
      virtual unsigned  lsize()     {assert(0); return 0   ;}
   protected:
                        TenderObj(const pointlist&);
      int*              _cdata;  // contour data
      unsigned          _csize;
};

//-----------------------------------------------------------------------------
// holds polygon representations - the contour will be drawn using the
// inherited _cdata holder. The _fdata stores the tesselated triangles
// which will be used for the fill
class TenderPoly : public TenderObj {
   public:
                        TenderPoly() : TenderObj() {}
                        TenderPoly(const pointlist&);
      virtual          ~TenderPoly();
      void              Tessel(TeselTempData*);
      TeselChain*       tdata()              {return &_tdata;}
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
   protected:
      TeselChain        _tdata;
};

//-----------------------------------------------------------------------------
// holds wire representation - the contour and the fill - exactly as in the 
// inherited class. The _ldata stores the central line which is effectively 
// the original points from tdtwire
class TenderWire : public TenderPoly {
   public:
                        TenderWire(const pointlist&, const word, bool);
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

typedef std::list<TenderObj*>  SliceObjects;
typedef std::list<TenderPoly*> SlicePolygons;

//-----------------------------------------------------------------------------
// translation view - effectively a layer slice of the visible cell data
class TenderTV {
   public:
                        TenderTV(CTM& translation);
      void              box  (const TP*, const TP*);
      void              poly (const pointlist&);
      void              wire (const pointlist&, word, bool);
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
      unsigned          _num_fqus; // fill quad
      unsigned          _num_fqss; // fill quad strip
      unsigned          _num_ftrs; // fill triangle
      unsigned          _num_ftfs; // fill triangle fan
      unsigned          _num_ftss; // fill triangle strip
};

//-----------------------------------------------------------------------------
//
typedef std::list<TenderTV*> TenderLay;
typedef std::map<word, TenderLay*> DataLay;

class Tenderer {
   public:
                        Tenderer( layprop::DrawProperties* drawprop, real UU );
//                     ~Tenderer();
      void              Grid( const real, const std::string );
      void              setLayer(word);
      void              pushCTM(CTM& trans)                    {_ctrans = trans;_drawprop->pushCTM(trans);}
      void              popCTM()                               {_drawprop->popCTM(); _ctrans = _drawprop->topCTM();}
      void              box (const TP* p1, const TP* p2)       {_cslice->box(p1,p2);}
      void              poly (const pointlist& plst)           {_cslice->poly(plst);}
      void              wire (const pointlist& plst, word w);
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
      DataLay           _data;
      TenderTV*         _cslice;    //!Working variable pointing to the current slice
      CTM               _ctrans;    //!Working variable storing the current translation
};


#endif //TENDERER_H
