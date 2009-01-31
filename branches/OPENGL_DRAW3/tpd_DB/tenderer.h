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

//-----------------------------------------------------------------------------
// class used during the tessalation
class TeselTempData {
   public:
                        TeselTempData(word** pfdata, unsigned int* fsize) :
                           _fsize(fsize), _pfdata(pfdata), _teseldata(NULL) {}
   unsigned int*        _fsize;
   word**               _pfdata;
   TeselVertices*       _teseldata;
};


//-----------------------------------------------------------------------------
// holds box representation - The same four points will be used for the
// contour as well as for the fill
//
class TenderObj {
   public:
                        TenderObj():_cdata(NULL), _csize(0) {}
                        TenderObj(const TP*, const TP*);
      virtual void      Tessel() {};
      int*              cdata()  {return _cdata;};  // contour data
      unsigned int      csize()  {return _csize;}
   protected:
                        TenderObj(const pointlist&);
      int*              _cdata;  // contour data
      unsigned int      _csize;
};

//-----------------------------------------------------------------------------
// holds polygon representations - the contour will be drawn using the
// inherited _cdata holder. The _fdata stores the tesselated triangles
// which will be used for the fill
class TenderPoly : public TenderObj {
   public:
                        TenderPoly() : TenderObj(), _fdata(NULL), _fsize(0) {}
                        TenderPoly(const pointlist&);

      virtual void      Tessel();
      static GLUtriangulatorObj* tenderTesel; //! A pointer to the OpenGL object tesselator
#ifdef WIN32
      static GLvoid CALLBACK teselVertex(GLvoid *, GLvoid *);
      static GLvoid CALLBACK teselBegin(GLenum*, GLvoid *);
      static GLvoid CALLBACK teselEnd(GLvoid *);
#else
      static GLvoid     teselVertex(GLvoid *, GLvoid *);
      static GLvoid     teselBegin(GLenum, GLvoid *);
      static GLvoid     teselEnd(GLvoid *);
#endif
   protected:
      //@FIXME -> we need another structure here for the tesselation
      // results. Tesselation returns a number of point sequences
      // which can be in one of the 3 formats:
      // GLU_TRIANGLE_FAN; GLU_TRIANGLE STRIP; GLU_TRIANGLE
      word*             _fdata;  // fill data
      unsigned int      _fsize;
};

//-----------------------------------------------------------------------------
// holds wire representation - the contour and the fill - exactly as in the 
// inherited class. The _ldata stores the central line which is effectively 
// the original points from tdtwire
class TenderWire : public TenderPoly {
   public:
                        TenderWire(const pointlist&, const word, bool);
      virtual void      Tessel() {};
   protected:
      void              precalc(const word);
      DBbox*            endPnts(const word, word, word, bool);
      DBbox*            mdlPnts(const word, word, word, const word);
      int*              _ldata;  // central line data
      unsigned int      _lsize;
};

typedef std::list<TenderObj*> SliceObjects;
//-----------------------------------------------------------------------------
// translation view - effectively a layer slice of the visible cell data
class TenderTV {
   public:
                        TenderTV(CTM& translation) : _tmatrix(translation),
                                 _num_contour_points(0l), _num_objects(0) {}
      void              box  (const TP*, const TP*);
      void              poly (const pointlist&);
      void              wire (const pointlist&, word, bool);
      const CTM*        tmatrix() {return &_tmatrix;}
      unsigned long     num_contour_points() {return _num_contour_points;  }
      unsigned          num_objects()        {return _num_objects;         }
      SliceObjects*     data()               {return &_data;               }
   private:
      CTM               _tmatrix;
      SliceObjects      _data;
      unsigned long     _num_contour_points;
      unsigned          _num_objects;
};

//-----------------------------------------------------------------------------
//
class Tenderer {
   public:
                        Tenderer( layprop::DrawProperties* drawprop, real UU );
//                     ~Tenderer();
      void              Grid( const real, const std::string );
//       void              add_data(const laydata::atticList*, const SLMap*);
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
      typedef std::list<TenderTV*> TenderLay;
      typedef std::map<word, TenderLay*> DataLay;
      layprop::DrawProperties*   _drawprop;
      real              _UU;
      DataLay           _data;
      TenderTV*         _cslice;    //!Working variable pointing to the current slice
      CTM               _ctrans;    //!Working variable storing the current translation
};


#endif //TENDERER_H