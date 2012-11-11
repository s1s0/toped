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
//        Created: Sun Sep  9 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Render dispatcher
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TREND_H_
#define TREND_H_

#include "ttt.h"
#include "basetrend.h"

#define TSHDR_LOC_VERTEX 0 // TODO -> get this into something like glslUniVarLoc

namespace trend {

   typedef enum { tocom      // command line
                 ,tolder     // basic (i.e. openGL 1.1)
                 ,tenderer   // VBO
                 , toshader  // shaders
                } RenderType;

   /**
    * This class contains a raw symbol data from the GLF font files. It is used to
    * parse the symbol data from the file and if the current renderer is the tolder
    * it is used directly for symbol drawing.
    * If a VBO or shader rendering is used, then the objects of this class serve as
    * an intermediate data container for parsed symbol data before loading the font
    * VBOs.
    */
   class TGlfSymbol {
      public:
                        TGlfSymbol(FILE*);
                       ~TGlfSymbol();
         void           dataCopy(GLfloat*, GLuint*, word);
         void           draw(bool);
         byte           alvrtxs()   { return _alvrtxs;}
         byte           alchnks()   { return _alchnks;}
         float          minX()      { return _minX;   }
         float          maxX()      { return _maxX;   }
         float          minY()      { return _minY;   }
         float          maxY()      { return _maxY;   }
         friend class TGlfRSymbol;
      protected:
         byte           _alvrtxs;    //! Number of vertexes
         byte           _alcntrs;    //! Number of contours
         byte           _alchnks;    //! Number of index (tesselation) chunks

         float*         _vdata;      //! Vertex data
         byte*          _cdata;      //! Contour data
         byte*          _idata;      //! Index data

         float          _minX;
         float          _maxX;
         float          _minY;
         float          _maxY;
   };

   /**
    * Contains the data necessary to handle a GLF symbol when a VBO or shader
    * rendering is used. An object of this class is constructed using the
    * corresponding TGlfSymbol object and the most important method is draw
    * When a basic rendering is used - no objects of this class are created.
    */
   class TGlfRSymbol {
      public:
                        TGlfRSymbol(TGlfSymbol*, word, word);
                       ~TGlfRSymbol();
         void           drawSolid();
         void           drawWired();
         float          minX() { return _minX; }
         float          maxX() { return _maxX; }
         float          minY() { return _minY; }
         float          maxY() { return _maxY; }
      private:
         GLint*         _firstvx;    //! first vertex in the font matrix
         GLuint         _firstix;    //! first index in the font matrix
         byte           _alcntrs;    //! Number of contours
         byte           _alchnks;    //! Number of index (tesselation) chunks

         GLsizei*       _csize;      //! Sizes of the contours

         float          _minX;
         float          _maxX;
         float          _minY;
         float          _maxY;
   };

   /**
    * Base class handling a particular GLF font. Contains the parsing of the
    * GLF files and used for string drawing when the basic rendering is active.
    */
   class TolderGlfFont {
      public:
                        TolderGlfFont(std::string, std::string&);
         virtual       ~TolderGlfFont();
         virtual void   getStringBounds(const std::string&, DBbox*);
         virtual void   bindBuffers() {assert(false);}
         virtual void   drawString(const std::string&, bool, layprop::DrawProperties*);
         byte           status()        {return _status;}
      protected:
         typedef std::map<byte, TGlfSymbol*> TFontMap;
         byte           _status     ; //! of the initial font parsing in the constructor
         float          _pitch      ; //! Here it means the space between the symbols
         float          _spaceWidth ; //! The size of the space symbol
         TFontMap       _tsymbols   ;
   };

   /**
    * Objects of this class are used when a VBO or shader rendering is active.
    * The main difference with its parent class is the collect method which loads
    * the entire font into an openGL VBO during the construction. The virtual
    * methods are updated accordingly.
    * One point to note here is that despite the fact that it inherits _tsymbol
    * field from its parent - it is used only as an intermediate container during
    * the construction. The collect method called by the constructor kind of
    * upgrades all TGlfSymbol objects of the _tsymbol field to TGlfRSymbol objects
    * of the _symbol field and at the same time clears the former. So after the
    * construction the container _tsymbols is empty. The container _symbol is
    * used in all other methods.
    */
   class TenderGlfFont : public TolderGlfFont {
      public:
                        TenderGlfFont(std::string, std::string&);
         virtual       ~TenderGlfFont();
         virtual void   getStringBounds(const std::string&, DBbox*);
         virtual void   bindBuffers();
         virtual void   drawString(const std::string&, bool, layprop::DrawProperties*);
      protected:
         void           collect(const word, const word);
         typedef std::map<byte, TGlfRSymbol*> FontMap;
         FontMap        _symbols    ;
         GLuint         _pbuffer    ;
         GLuint         _ibuffer    ;
   };

   class ToshaderGlfFont : public TenderGlfFont {
      public:
                        ToshaderGlfFont(std::string, std::string&);
         virtual       ~ToshaderGlfFont() {}
         virtual void   drawString(const std::string&, bool, layprop::DrawProperties*);
   };
   //=============================================================================
   //
   // Class to take care about the shaders initialisation
   //
   class Shaders {
      public:
                                 Shaders();
         virtual                ~Shaders();
         void                    loadShadersCode(const std::string&);
         bool                    status() const { return _status;}
      private:
         bool                    compileShader(const std::string&, GLint&, GLint);
         char*                   loadFile(const std::string&, GLint&);
         void                    getShadersLog(GLint);
         void                    getProgramsLog(GLint);
         bool                    bindUniforms(GLuint);
         std::string             _fnShdrVertex;
         std::string             _fnShdrFragment;
         GLint                   _idShdrVertex;
         GLint                   _idShdrFragment;
         GlslUniVarNames         _glslUniVarNames;
         bool                    _status;
   };

   class TrendCenter {
      public:
                                TrendCenter(bool gui, bool forceBasic=true, bool sprtVbo=false, bool sprtShaders=false);
         virtual               ~TrendCenter();
//         RenderType             renderType() const {return _renderType;}
         void                   initShaders(const std::string&);
         trend::TrendBase*      getCRenderer();
         trend::TrendBase*      getHRenderer();
         void                   releaseCRenderer();
         void                   releaseHRenderer();
         void                   drawGrid();
         void                   drawZeroCross();
         void                   drawFOnly();
         //Font handling
         void                   loadLayoutFont(std::string);
         void                   getStringBounds(const std::string&, DBbox*);
         void                   drawString(const std::string& str, bool fill, layprop::DrawProperties*);
         void                   drawWiredString(const std::string& str);
         void                   drawSolidString(const std::string& str);
         bool                   selectFont(std::string str);
         word                   numFonts();
         void                   bindFont();
         void                   unbindFont();
      private:
         typedef std::map<std::string, TolderGlfFont*> OglFontCollectionMap;
         trend::TrendBase*      _cRenderer;    //! current renderer
         trend::TrendBase*      _hRenderer;    //! hoover renderer
         trend::Shaders*        _cShaders;     //! the shader init object (valid in toshader case only)
         OglFontCollectionMap   _oglFont;
         std::string            _activeFontName;
         RenderType             _renderType;
   };
}

#endif /* TREND_H_ */
