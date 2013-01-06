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

#include <string>
#include <math.h>
#include "tedbac.h"

namespace laydata {
   const word _lmnone   = 0x0000;
   const word _lmbox    = 0x0001;
   const word _lmpoly   = 0x0002;
   const word _lmwire   = 0x0004;
   const word _lmtext   = 0x0008;
   const word _lmref    = 0x0010;
   const word _lmaref   = 0x0020;
   const word _lmpref   = 0x0040;
   const word _lmapref  = 0x0080;
   const word _lmgrcref = 0x0100;
   const word _lmall    = 0xffff;


   class TdtCellRef;
   typedef  std::deque<const TdtCellRef*>           CellRefStack;

   /**
    * The primary subject of this class is to generate the wire contour
    * from its central line. It is supposed to deal with all of the corner
    * cases and especially when the wire contains collinear segments. The
    * class requires the point data in integer array format. It is not
    * copying the original data - just stores a pointer to it. Thats' why
    * there is no destructor defined.
    */
   class WireContour {
      public:
                           WireContour(const int4b*, unsigned, const WireWidth);
         unsigned          csize()         {return _cdata.size(); } //! return the number of the contour points
         unsigned          lsize()         {return _lsize;        } //! return the number of the central line points
         void              getArrayData(int4b*);
         void              getVectorData(PointVector&);
         DBbox             getCOverlap();
      private:
         typedef std::list<TP> PointList;
         void              endPnts(word, word, bool);
         void              mdlPnts(word, word, word);
         void              mdlAcutePnts(word, word, word, int, int);
         byte              chkCollinear(word,word,word);
         void              colPnts(word,word,word);
         TP                mdlCPnt(word, word);
         int               orientation(word, word, word);
         double            getLambda(word i1, word i2, word ii);
         int               xangle(word i1, word i2);
         const int4b*      _ldata; //! The original wire central line. Do not delete it. Do not alter it!
         const unsigned    _lsize; //! The number of points in the wire central line
         const WireWidth   _width; //! The width of the wire
         PointList         _cdata; //! The generated contour line in a list of points form
   };

   /**
    * An auxiliary class to wrap around the WireContour class. It makes WireContour
    * usable with a point list input data or with data which needs coordinate
    * transformations. Also it defines a method to dump the wire contour in a PointVector
    * format which is usable by the basic renderer.
    */
   class WireContourAux {
      public:
                          WireContourAux(const int4b*, unsigned, const WireWidth, const CTM&);
                          WireContourAux(const PointVector&, const WireWidth);
                          WireContourAux(const PointVector&, const WireWidth, const TP);
                         ~WireContourAux();
         void             getRenderingData(PointVector&);
         void             getVectorLData(PointVector&);
         void             getVectorCData(PointVector&);
         void             getArrayLData(int4b*);
         void             getArrayCData(int4b*);
         unsigned         csize()         {return _wcObject->csize(); } //! return the number of the contour points
         unsigned         lsize()         {return _wcObject->lsize(); } //! return the number of the central line points
      private:
         WireContour*     _wcObject;
         int4b*           _ldata;
   };
}
namespace layprop {

   typedef enum {cell_mark, array_mark, text_mark} binding_marks;
   typedef enum {DB, DRC} PropertyState;
   typedef enum { crc_VIEW       = 0, // not in edit in place mode or not in the active cell chain
                  crc_PREACTIVE  = 1, // edit in place mode, and we're in the active cell chain before the active cell
                  crc_ACTIVE     = 2, // edit in place mode, the active cell
                  crc_POSTACTIVE = 3  // edit in place mode, and we're in the active cell chain after the active cell
                } CellRefChainType;
   typedef byte LayMark[30];
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
         void              setAlpha(byte alpha) {_alpha = alpha;}
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
                        LineSettings():_color(""), _pattern(0xffff), _patscale(1), _width(1){};
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
                          LayerState(const LayerDef& laydef, bool sh, bool sl, bool sf) : _laydef(laydef), _hidden(sh),
                             _locked(sl), _filled(sf) {};
                          LayerState(const LayerDef& laydef, const LayerSettings& lset) : _laydef(laydef),
                             _hidden(lset.hidden()), _locked(lset.locked()), _filled(lset.filled()){}
      LayerDef            layDef() const              {return _laydef;}
      bool                hidden() const              {return _hidden;}
      bool                locked() const              {return _locked;}
      bool                filled() const              {return _filled;}
   private:
      LayerDef            _laydef;
      bool                _hidden;
      bool                _locked;
      bool                _filled;
   };

   //=============================================================================
   typedef  std::map<std::string, tellRGB*      >        ColorMap;
   typedef  std::map<std::string, byte*         >        FillMap;
   typedef  std::map<std::string, LineSettings* >        LineMap;
   typedef  laydata::LayerContainer<LayerSettings*>      LaySetList;
   typedef  std::pair <LayerDef, std::list<LayerState> > LayStateList;

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
   example current drawing layer, colours etc.
    */
   class DrawProperties {
      public:
                                    DrawProperties();
                                   ~DrawProperties();
         // Called during the rendering - protected in the render initialisation
         bool                       setCurrentColor(const LayerDef&, layprop::tellRGB&);
         const byte*                getCurrentFill() const;
         void                       getCurrentLine(LineSettings&, bool) const;
         bool                       getAlpha(word factor, layprop::tellRGB& theColor);
         void                       initDrawRefStack(laydata::CellRefStack*);
         void                       clearDrawRefStack();
         void                       postCheckCRS(const laydata::TdtCellRef*);
         CellRefChainType           preCheckCRS(const laydata::TdtCellRef*);
//         void                       drawReferenceMarks(const TP&, const binding_marks) const;
         LayerDef                   getTenderLay(const LayerDef&) const;//!return the same if _propertyState == DB or predefined layer otherwise
         const CTM&                 scrCtm() const       {return  _scrCtm;}
         word                       visualLimit() const  {return _visualLimit;}
         const DBbox&               clipRegion() const   {return _clipRegion;}
         void                       initCtmStack()       {_tranStack.push(CTM(_clipRegion.p1(), _clipRegion.p2()));}
         void                       clearCtmStack()      {while (!_tranStack.empty()) _tranStack.pop();}
         void                       pushCtm(const CTM& last)   {_tranStack.push(last);}
         void                       popCtm()             {_tranStack.pop();}
         const CTM&                 topCtm() const       {assert(_tranStack.size());return _tranStack.top();}
         void                       setState (PropertyState state)
                                                         {_propertyState = state;};
//         LayerNumber                drawingLayer() const {return _drawingLayer;}
         bool                       textBoxHidden() const
                                                         {return _textBoxHidden;}
         bool                       textMarksHidden() const
                                                         {return _textMarksHidden;}
         bool                       cellBoxHidden() const
                                                         {return _cellBoxHidden;}
         bool                       cellMarksHidden() const
                                                         {return _cellMarksHidden;}
         bool                       adjustTextOrientation() const
                                                         {return _adjustTextOrientation;}
         byte                       cellDepthView()      {return _cellDepthView;}

         const byte*                ref_mark_bmp()       {return _ref_mark_bmp ;}
         const byte*                text_mark_bmp()      {return _text_mark_bmp;}
         const byte*                aref_mark_bmp()      {return _aref_mark_bmp;}

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
         void                       allUnselectable(LayerDefSet&);
         void                       allInvisible(LayerDefSet&);
         // Properly protected in tpd_bidfunc or the functions called from there
         bool                       addLayer(std::string, const LayerDef&, std::string, std::string, std::string);
         bool                       addLayer(std::string, const LayerDef&);
         bool                       addLayer(const LayerDef&);
         LayerDef                   addLayer(std::string);
         void                       addColor(std::string name, byte R, byte G, byte B, byte A);
         void                       addFill(std::string name, byte *ptrn);
         void                       addLine(std::string, std::string, word, byte, byte);
         void                       hideLayer(const LayerDef&, bool);
         void                       lockLayer(const LayerDef&, bool lock);
         void                       fillLayer(const LayerDef&, bool fill);
         void                       pushLayerStatus();
         void                       popLayerStatus();
         void                       popBackLayerStatus();
         bool                       saveLaysetStatus(const std::string&);
         bool                       saveLaysetStatus(const std::string&, const LayerDefSet&, const LayerDefSet&, const LayerDefSet&, const LayerDef&);
         bool                       loadLaysetStatus(const std::string&);
         bool                       deleteLaysetStatus(const std::string&);
         bool                       getLaysetStatus(const std::string&, LayerDefSet&, LayerDefSet&, LayerDefSet&, LayerDef&);
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
         void                       defaultLayer(LayerDef layno)     {_curlay = layno;}
         LayerDef                   curLay() const                   {return _curlay;}

         // Used in dialogue boxes and drawing - partially protected for now
         bool                       layerHidden(const LayerDef&) const;
         bool                       layerLocked(const LayerDef&) const;
         bool                       layerFilled(const LayerDef&) const;
         bool                       selectable(const LayerDef&) const;

         // Used in dialogue boxes an protected during construction of the dialogue boxes
         LayerDefList               getAllLayers() const;
         std::string                getLayerName(const LayerDef&) const;
         std::string                getColorName(const LayerDef&) const;
         std::string                getFillName(const LayerDef&) const;
         std::string                getLineName(const LayerDef&) const;
         const byte*                getFill(const LayerDef&) const;
         const byte*                getFill(std::string) const;
         const tellRGB&             getColor(const LayerDef&) const;
         const tellRGB&             getColor(std::string) const;
         const LineSettings*        getLine(const LayerDef&) const;
         const LineSettings*        getLine(std::string) const;
         void                       allLayers(NameList&) const;
         void                       allColors(NameList&) const;
         void                       allFills(NameList&) const;
         void                       allLines(NameList&) const;
         LayerDef                   getLayerNo(std::string name) const;
      private:
         typedef std::deque<LayStateList>            LayStateHistory;
         typedef std::map<std::string, LayStateList> LayStateMap;
         const LaySetList&          getCurSetList() const;
         const LayerSettings*       findLayerSettings(const LayerDef&) const;
         LaySetList                 _laySetDb;
         LaySetList                 _laySetDrc;
         ColorMap                   _layColors;
         FillMap                    _layFill;
         LineMap                    _lineSet;
         LayerDef                   _curlay;       // current drawing layer
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
         LayerDef                   _drawingLayer;
         LayStateMap                _layStateMap;  //
         LayStateHistory            _layStateHistory; //! for undo purposes of layer status related TELL function
         static const tellRGB       _dfltColor;
         static const byte          _dfltFill[128];
         static const LineSettings  _dfltLine     ; //! Default Line
         static const LineSettings  _dfltSLine    ; //! Default Selected Line
         static const LineSettings  _dfltCellBnd  ; //! Default Cell Boundary
         static const LineSettings  _dfltCellSBnd ; //! Default Selected Cell Boundary
         static const LineSettings  _dfltTextBnd  ; //! Default Text Boundary
         static const LineSettings  _dfltTextSBnd ; //! Default Selected Text Boundary
         static const LayMark       _ref_mark_bmp ; //!
         static const LayMark       _text_mark_bmp; //!
         static const LayMark       _aref_mark_bmp; //!
         PropertyState              _propertyState; //type of drawing
   };
}
#endif //DRAWPROP_H
