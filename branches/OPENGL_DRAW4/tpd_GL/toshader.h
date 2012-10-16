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
//        Created: Sun Oct 14 BST 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: OpenGL shader renderer
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TOSHADER_H
#define TOSHADER_H

#include <GL/glew.h>
#include "tenderer.h"

namespace trend {
   class ToshaderTV : public TenderTV {
      public:
                           ToshaderTV(TrendRef* const, bool, bool, unsigned, unsigned);
         virtual          ~ToshaderTV() {};

         //virtual void      collect(TNDR_GLDATAT*, unsigned int*);
         virtual void      draw(layprop::DrawProperties*);
         //virtual void      drawTexts(layprop::DrawProperties*);
      //private:
      //   void              collectIndexs(unsigned int*, const TeselChain*, unsigned*, unsigned*, unsigned);
      //   GLsizei*          _sizesvx[4]; //! arrays of sizes for vertex sets
      //   GLsizei*          _firstvx[4]; //! arrays of first vertexes
      //   GLsizei*          _sizesix[4]; //! arrays of sizes for indexes sets
      //   GLuint*           _firstix[4]; //! arrays of first indexes
      //   // offsets in the VBO
      //   unsigned          _point_array_offset; //! The offset of this chunk of vertex data in the vertex VBO
      //   unsigned          _index_array_offset; //! The offset of this chunk of index  data in the index  VBO
   };

   class ToshaderReTV : public TenderReTV {
      public:
                           ToshaderReTV(TrendTV* const chunk, TrendRef* const refCell):
                              TenderReTV(chunk, refCell) {}
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawTexts(layprop::DrawProperties*);
   };

   class ToshaderLay : public TenderLay {
      public:
                           ToshaderLay();
         virtual          ~ToshaderLay();
         virtual void      newSlice(TrendRef* const, bool, bool /*, bool, unsigned*/);
         virtual void      newSlice(TrendRef* const, bool, bool, unsigned slctd_array_offset);
         virtual bool      chunkExists(TrendRef* const, bool);
         virtual void      draw(layprop::DrawProperties*);
         virtual void      drawSelected();
         virtual void      drawTexts(layprop::DrawProperties*);
         //virtual void      collect(bool, GLuint, GLuint);
         //virtual void      collectSelected(unsigned int*);

      //private:
      //   GLuint            _pbuffer;
      //   GLuint            _ibuffer;
      //   // index related data for selected objects
      //   GLsizei*          _sizslix[3]; //! arrays of sizes for indexes sets of selected objects
      //   GLuint*           _fstslix[3]; //! arrays of first indexes for selected objects
      //   // offsets in the VBO
      //   unsigned          _stv_array_offset; //! first point in the TenderTV with selected objects in this layer
      //   unsigned          _slctd_array_offset; //! first point in the VBO with selected indexes
   };

   class ToshaderRefLay : public TenderRefLay {
      public:
                           ToshaderRefLay();
         virtual          ~ToshaderRefLay();
         //virtual void      collect(GLuint);
         virtual void      draw(layprop::DrawProperties*);
   //   private:
   //      GLuint            _pbuffer;
   //      // vertex related data
   //      GLsizei*          _sizesvx; //! array of sizes for vertex sets
   //      GLsizei*          _firstvx; //! array of first vertexes
   //      // index related data for selected boxes
   //      GLsizei*          _sizslix; //! array of sizes for indexes sets
   //      GLsizei*          _fstslix; //! array of first indexes
   };

   class Toshader : public Tenderer {
      public:
                           Toshader( layprop::DrawProperties* drawprop, real UU );
         virtual          ~Toshader();
         virtual void      grid( const real, const std::string );
         virtual void      zeroCross();
         virtual void      setLayer(const LayerDef&, bool);
         virtual void      setHvrLayer(const LayerDef&);
         virtual void      setGrcLayer(bool, const LayerDef&);
         virtual bool      chunkExists(const LayerDef&, bool);
         //virtual bool      collect();
         //virtual bool      grcCollect();
         virtual void      draw();
         virtual void      grcDraw();
         virtual void      cleanUp();
         virtual void      grcCleanUp();
      //private:
      //   unsigned          _num_ogl_buffers; //! Number of generated openGL VBOs
      //   unsigned          _num_ogl_grc_buffers; //!
      //   GLuint*           _ogl_buffers;     //! Array with the "names" of all openGL buffers
      //   GLuint*           _ogl_grc_buffers; //! Array with the "names" of the GRC related openGL buffers
      //   GLuint            _sbuffer;         //! The "name" of the selected index buffer
   };

}

#endif // TOSHADER_H