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
//    Description: Canvas drawing properties
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef DRAWPROP_H
#define DRAWPROP_H

#include <GL/glew.h>
#include <string>
#include <math.h>
#include "tedstd.h"

namespace layprop {

   typedef enum {cell_mark, array_mark, text_mark} binding_marks;
   typedef enum {DB, DRC} PropertyState;
   typedef enum { crc_VIEW       = 0, // not in edit in place mode or not in the active cell chain
                  crc_PREACTIVE  = 1, // edit in place mode, and we're in the active cell chain before the active cell
                  crc_ACTIVE     = 2, // edit in place mode, the active cell
                  crc_POSTACTIVE = 3  // edit in place mode, and we're in the active cell chain after the active cell
                } CellRefChainType;

   //=============================================================================
   //
   //
   //
   class tellRGB {
      public:
                           tellRGB() : _red(0), _green(0), _blue(0), _alpha(0) {}
                           tellRGB(byte red, byte green, byte blue, byte alpha) :
                              _red(red), _green(green), _blue(blue), _alpha(alpha) {}
         byte              red()   const {return _red;     };
         byte              green() const {return _green;   };
         byte              blue()  const {return _blue;    };
         byte              alpha() const {return _alpha;   };
      private:
         byte              _red;
         byte              _green;
         byte              _blue;
         byte              _alpha;
   };

   //=============================================================================
   //
   //
   //
   class LineSettings
   {
      public:
         LineSettings(std::string color, word pattern, byte patscale, byte width) :
            _color(color), _pattern(pattern), _patscale(patscale), _width(width) {};
         std::string    color()     const {return _color;   }
         word           pattern()   const {return _pattern; }
         byte           patscale()  const {return _patscale;}
         byte           width()     const {return _width;   }
      private:
         std::string    _color;
         word           _pattern;
         byte           _patscale;
         byte           _width;
   };

   //=============================================================================
   //
   //
   //
   class LayerSettings  {
      public:
                           LayerSettings(std::string name, std::string color, std::string filltype, std::string sline):
                           _name(name), _color(color), _fill(filltype), _sline(sline),
                                 _hidden(false), _locked(false), _filled(filltype != "") {};
         std::string       color()    const {return _color;}
         std::string       fill()     const {return _fill;}
         bool              filled()   const {return _filled;}
         std::string       name()     const {return _name;}
         std::string       sline()    const {return _sline;}
         bool              hidden()   const {return _hidden;}
         bool              locked()   const {return _locked;}
         void              fillLayer(bool filled)  {_filled = filled;};
         friend class DrawProperties;
      private:
         std::string       _name;
         std::string       _color;
         std::string       _fill;
         std::string       _sline;
         bool              _hidden;
         bool              _locked;
         bool              _filled; //define filling visualisation
   };

   //=============================================================================
   //
   //
   //
   class LayerState {
   public:
                          LayerState(unsigned num, bool sh, bool sl, bool sf) : _number(num), _hidden(sh),
                             _locked(sl), _filled(sf) {};
                          LayerState(unsigned num, const LayerSettings& lset) : _number(num),
                             _hidden(lset.hidden()), _locked(lset.locked()), _filled(lset.filled()){}
      LayerNumber         number() const              {return _number;}
      bool                hidden() const              {return _hidden;}
      bool                locked() const              {return _locked;}
      bool                filled() const              {return _filled;}
   private:
      LayerNumber         _number;
      bool                _hidden;
      bool                _locked;
      bool                _filled;
   };

   //=============================================================================
   //
   //
   //
   class TGlfSymbol {
      public:
                        TGlfSymbol(FILE*);
                       ~TGlfSymbol();
         void           dataCopy(GLfloat*, GLuint*, word);
         byte           alvrtxs()   { return _alvrtxs;}
         byte           alchnks()   { return _alchnks;}
         friend class TGlfRSymbol;
      protected:
         byte           _alvrtxs;    //! Number of vertexs
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

   //=============================================================================
   //
   //
   //
   class TGlfRSymbol {
      public:
                        TGlfRSymbol(TGlfSymbol*, word, word);
                       ~TGlfRSymbol();
         void           draw(bool);
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

   //=============================================================================
   //
   //
   //
   class TGlfFont {
      public:
         TGlfFont(std::string, std::string&);
                       ~TGlfFont();
         void           getStringBounds(const std::string*, DBbox*);
         void           collect();
         bool           bindBuffers();
         void           drawString(const std::string*, bool);
         byte           status()        {return _status;}
      private:
         typedef std::map<byte, TGlfSymbol*> TFontMap;
         typedef std::map<byte, TGlfRSymbol*> FontMap;
         FontMap        _symbols;
         TFontMap       _tsymbols;
         word           _all_vertexes;
         word           _all_indexes;
         byte           _status;
         byte           _numSymbols;
         float          _pitch;
         float          _spaceWidth;
         GLuint         _pbuffer;
         GLuint         _ibuffer;
   };


   //=============================================================================
   //
   // Wrapper to abstract-out the Glf implementation. Should be temporary until
   // new implementation is up&running
   //
   class FontLibrary {
      public:
                                FontLibrary(bool);
                               ~FontLibrary();
         bool                   LoadLayoutFont(std::string);
         bool                   selectFont(std::string);
         void                   getStringBounds(const std::string*, DBbox*);
         void                   drawString(const std::string*, bool);
         void                   drawWiredString(std::string);
         void                   drawSolidString(std::string);
         bool                   bindFont();
         void                   unbindFont();
         void                   allFontNames(NameList&);
         word                   numFonts();
         std::string            getActiveFontName() const {return _activeFontName;}
      private:
         typedef std::map<std::string, TGlfFont*> OglFontCollectionMap;
         typedef std::map<std::string, int>       RamFontCollectionMap;
         OglFontCollectionMap   _oglFont;
         RamFontCollectionMap   _ramFont;
         bool                   _fti; // font type implementation ()
         std::string            _activeFontName;
   };

   //=============================================================================
   typedef  std::map<std::string, tellRGB*      >        ColorMap;
   typedef  std::map<std::string, byte*         >        FillMap;
   typedef  std::map<std::string, LineSettings* >        LineMap;
   typedef  std::map<LayerNumber, LayerSettings*>        LaySetList;
   typedef  std::pair <LayerNumber, std::list<LayerState> > LayStateList;

   //==============================================================================
   /*! This class serves as a carrying case for all drawing properties during the
   drawing of the database. The fields of DrawProperties can be split logically
   on two parts:
   - locked properties - these are the properties that stay constant during the
   drawing process. It is important to note that the class can not change
   those properties, despite the fact that these are fields of its own. Instead
   a friend class PropertyCenter is doing this. The reason for this is primarily
   thread safety.
   - Changeable properties - these are changed during the drawing process - for
   example current drawing layer, colors etc.
    */
   class DrawProperties {
      public:
                                    DrawProperties();
                                   ~DrawProperties();
         // Called during the rendering - protected in the render initialization
         void                       setCurrentColor(LayerNumber layno);
         bool                       setCurrentFill(bool) const;
         void                       setLineProps(bool selected = false) const;
         void                       initDrawRefStack(laydata::CellRefStack*);
         void                       clearDrawRefStack();
         void                       postCheckCRS(const laydata::TdtCellRef*);
         CellRefChainType           preCheckCRS(const laydata::TdtCellRef*);
         void                       drawReferenceMarks(const TP&, const binding_marks) const;
         void                       drawTextBoundary(const PointVector& ptlist) const;
         void                       drawCellBoundary(const PointVector& ptlist) const;
         void                       setGridColor(std::string colname) const;
         LayerNumber                getTenderLay(LayerNumber layno) const;//!return layno if _propertyState == DB or predefined layer otherwise
         void                       psWrite(PSFile&) const;
         void                       adjustAlpha(word factor);
         const CTM&                 scrCtm() const       {return  _scrCtm;}
         word                       visualLimit() const  {return _visualLimit;}
         const DBbox&               clipRegion() const   {return _clipRegion;}
         void                       initCtmStack()       {_tranStack.push(CTM());}
         void                       clearCtmStack()      {while (!_tranStack.empty()) _tranStack.pop();}
         void                       pushCtm(CTM& last)   {_tranStack.push(last);}
         void                       popCtm()             {_tranStack.pop();}
         const CTM&                 topCtm() const       {assert(_tranStack.size());return _tranStack.top();}
         void                       setState (PropertyState state)
                                                         {_propertyState = state;};
//         LayerNumber                drawingLayer() const {return _drawingLayer;}
         bool                       isTextBoxHidden() const
                                                         {return _textBoxHidden;}
         bool                       isCellBoxHidden() const
                                                         {return _cellBoxHidden;}
         bool                       adjustTextOrientation() const
                                                         {return _adjustTextOrientation;}
         byte                       cellDepthView()      {return _cellDepthView;}
         // Protected elsewhere
         console::ACTIVE_OP         currentOp() const    {return _currentOp;}
         void                       setCurrentOp(console::ACTIVE_OP actop)
                                                         {_currentOp = actop;}
         void                       setClipRegion(DBbox clipR)
                                                         {_clipRegion = clipR;}
         void                       setScrCTM(CTM ScrCTM){_scrCtm = ScrCTM;}
         void                       setVisualLimit(word mva)
                                                         {_visualLimit = mva;}
         void                       setCellDepthAlphaEbb(byte ebb)
                                                         {_cellDepthAlphaEbb = ebb;}
         void                       setCellDepthView(byte dov)
                                                         {_cellDepthView = dov;}
         void                       allUnselectable(DWordSet&);
         void                       allInvisible(DWordSet&);
         // Properly protected in tpd_bidfunc or the functions called from there
         bool                       addLayer(std::string, LayerNumber, std::string, std::string, std::string);
         bool                       addLayer(std::string, LayerNumber);
         bool                       addLayer(LayerNumber);
         LayerNumber                addLayer(std::string);
         void                       addColor(std::string name, byte R, byte G, byte B, byte A);
         void                       addFill(std::string name, byte *ptrn);
         void                       addLine(std::string, std::string, word, byte, byte);
         void                       hideLayer(LayerNumber layno, bool hide);
         void                       lockLayer(LayerNumber layno, bool lock);
         void                       fillLayer(LayerNumber layno, bool fill);
         void                       pushLayerStatus();
         void                       popLayerStatus();
         void                       popBackLayerStatus();
         bool                       saveLaysetStatus(const std::string&);
         bool                       saveLaysetStatus(const std::string&, const WordSet&, const WordSet&, const WordSet&, unsigned);
         bool                       loadLaysetStatus(const std::string&);
         bool                       deleteLaysetStatus(const std::string&);
         bool                       getLaysetStatus(const std::string&, WordSet&, WordSet&, WordSet&, unsigned);
         void                       savePatterns(FILE*) const;
         void                       saveColors(FILE*) const;
         void                       saveLayers(FILE*) const;
         void                       saveLines(FILE*) const;
         void                       saveLayState(FILE*) const;
         void                       setCellMarksHidden(bool hide)    {_cellMarksHidden = hide;}
         void                       setTextMarksHidden(bool hide)    {_textMarksHidden = hide;}
         void                       setCellboxHidden(bool hide)      {_cellBoxHidden = hide;}
         void                       setTextboxHidden(bool hide)      {_textBoxHidden = hide;}
         void                       setAdjustTextOrientation(bool ori)
                                                                     {_adjustTextOrientation = ori;}
         void                       defaultLayer(LayerNumber layno)  {_curlay = layno;}
         LayerNumber                curLay() const                   {return _curlay;}

         // Used in dialogue boxes and drawing - partially protected for now
         bool                       layerHidden(LayerNumber layno) const;
         bool                       layerLocked(LayerNumber layno) const;
         bool                       layerFilled(LayerNumber layno) const;
         bool                       selectable(LayerNumber layno) const;

         // Used in dialogue boxes an protected during construction of the dialogue boxes
         LayerTMPList               getAllLayers() const;
         std::string                getLayerName(LayerNumber layno) const;
         std::string                getColorName(LayerNumber layno) const;
         std::string                getFillName(LayerNumber layno) const;
         std::string                getLineName(LayerNumber layno) const;
         const byte*                getFill(LayerNumber layno) const;
         const byte*                getFill(std::string) const;
         const tellRGB&             getColor(LayerNumber layno) const;
         const tellRGB&             getColor(std::string) const;
         const LineSettings*        getLine(LayerNumber layno) const;
         const LineSettings*        getLine(std::string) const;
         void                       allLayers(NameList&) const;
         void                       allColors(NameList&) const;
         void                       allFills(NameList&) const;
         void                       allLines(NameList&) const;
         LayerNumber                getLayerNo(std::string name) const;
      private:
         typedef std::deque<LayStateList>            LayStateHistory;
         typedef std::map<std::string, LayStateList> LayStateMap;
         const LaySetList&          getCurSetList() const;
         const LayerSettings*       findLayerSettings(LayerNumber) const;
         LaySetList                 _laySetDb;
         LaySetList                 _laySetDrc;
         ColorMap                   _layColors;
         FillMap                    _layFill;
         LineMap                    _lineSet;
         LayerNumber                _curlay;       // current drawing layer
         DBbox                      _clipRegion;
         CTM                        _scrCtm;
         word                       _visualLimit;   // that would be 40 pixels
         byte                       _cellDepthAlphaEbb;
         byte                       _cellDepthView; //
         bool                       _cellMarksHidden;
         bool                       _cellBoxHidden;
         bool                       _textMarksHidden;
         bool                       _textBoxHidden;
         bool                       _adjustTextOrientation;
         console::ACTIVE_OP         _currentOp;    //
         bool                       _blockFill;
         laydata::CellRefStack*     _refStack;
         CtmStack                   _tranStack;
         LayerNumber                _drawingLayer;
         LayStateMap                _layStateMap;  //
         LayStateHistory            _layStateHistory; //! for undo purposes of layer status related TELL function
         static const tellRGB       _defaultColor;
         static const byte          _defaultFill[128];
         static const LineSettings  _defaultSeline;
         PropertyState              _propertyState; //type of drawing
   };

}
#endif //DRAWPROP_H
