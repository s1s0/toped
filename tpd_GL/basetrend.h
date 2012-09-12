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
#ifndef BASETREND_H
#define BASETREND_H

#include <GL/glew.h>
#include "drawprop.h"

// to cast properly the indices parameter in glDrawElements when
// drawing from VBO
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))


//namespace trend {
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


   //-----------------------------------------------------------------------------
   //
   class BaseTrend {
      public:
                             BaseTrend( layprop::DrawProperties* drawprop, real UU ):
                                                             _drawprop(drawprop), _UU(UU) {};
         virtual            ~BaseTrend() {};
         virtual void        grid( const real, const std::string ) = 0;
         virtual void        setLayer(const LayerDef&, bool) = 0;
         virtual void        setGrcLayer(bool, const LayerDef&) = 0;
         virtual bool        chunkExists(const LayerDef&, bool) = 0;
         virtual void        pushCell(std::string, const CTM&, const DBbox&, bool, bool) = 0;
         virtual void        popCell() = 0;
         virtual const CTM&  topCTM() const = 0;
         virtual void        box  (const int4b* pdata) = 0;
         virtual void        box  (const int4b* pdata, const SGBitSet* ss) = 0;
         virtual void        poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly) = 0;
         virtual void        poly (const int4b* pdata, unsigned psize, const TessellPoly* tpoly, const SGBitSet* ss) = 0;
         virtual void        grcpoly(int4b* pdata, unsigned psize) = 0;
         virtual void        wire (int4b*, unsigned, WireWidth) = 0;
         virtual void        wire (int4b*, unsigned, WireWidth, const SGBitSet*) = 0;
         virtual void        grcwire (int4b*, unsigned, WireWidth) = 0;
         virtual void        arefOBox(std::string, const CTM&, const DBbox&, bool) = 0;
         virtual void        text (const std::string*, const CTM&, const DBbox&, const TP&, bool) = 0;
         virtual bool        collect() = 0;
         virtual bool        grcCollect() = 0;
         virtual void        draw() = 0;
         virtual void        grcDraw() = 0;
         virtual void        cleanUp() = 0;
         virtual void        grcCleanUp() = 0;
         //-----------------------------------------------------------------------------------------
         LayerDef            getTenderLay(const LayerDef& laydef)
                                                         {return _drawprop->getTenderLay(laydef)   ;}
         void                setState(layprop::PropertyState state)
                                                         {        _drawprop->setState(state)       ;}
         bool                layerHidden(const LayerDef& laydef) const
                                                         {return _drawprop->layerHidden(laydef)    ;}
         const CTM&          scrCTM() const              {return _drawprop->scrCtm()               ;}
         word                visualLimit() const         {return _drawprop->visualLimit()          ;}
         const DBbox&        clipRegion() const          {return _drawprop->clipRegion()           ;}
         void                postCheckCRS(const laydata::TdtCellRef* ref)
                                                         {        _drawprop->postCheckCRS(ref)     ;}
         virtual bool        preCheckCRS(const laydata::TdtCellRef*, layprop::CellRefChainType&) = 0;
         void                initDrawRefStack(laydata::CellRefStack* crs)
                                                         {       _drawprop->initDrawRefStack(crs)  ;}
         void                clearDrawRefStack()         {       _drawprop->clearDrawRefStack()    ;}
         bool                adjustTextOrientation() const
                                                         {return _drawprop->adjustTextOrientation();}
      protected:
         layprop::DrawProperties*   _drawprop;
         real                _UU;
   };

//}

#endif //BASETREND_H
