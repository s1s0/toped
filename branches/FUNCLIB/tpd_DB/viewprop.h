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
//   This file is a part of Toped project (C) 2001-2007 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Sep 29 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Canvas visual properties
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef LAYERPROP_H
#define LAYERPROP_H

#include <string>
#include <math.h>
#include "tedstd.h"
#include "ps_out.h"
namespace layprop {

   typedef enum {cell_mark, array_mark, text_mark} binding_marks;
   
   class ViewProperties;
   class DrawProperties;

   //=============================================================================
   class SDLine {
   public:
                        SDLine(const TP& p1,const TP& p2, const real);
      void              draw(const DBline&, const DBline&, const DBline&, const double, const real) const;
   private:
      typedef std::list<DBline> LineList;
      unsigned          nonius(const DBline&, const DBline&, real, LineList& llst) const;
      DBline            _ln;
      std::string       _value;
      TP                _center;
      double            _length;
      real              _sinus;
      real              _cosinus;
      real              _angle;
   };

   //=============================================================================
   class SupplementaryData {
   public:
                        SupplementaryData() {_tmp_base = NULL;}
      void              addRuler(TP&, TP&, const real);
      void              clearRulers();
      void              drawRulers(const CTM&, real);
      void              tmp_draw(const TP&, const TP&, real, const CTM&, const real);
      void              mousePoint(const TP&);
      void              mouseStop();
      typedef std::list<SDLine> ruler_collection;
   protected:
      void              getConsts(const CTM&, DBline&, DBline&, DBline&, double&);
      ruler_collection  _rulers;
      TP*               _tmp_base;
   };

   //=============================================================================
   class LayoutGrid {
   public:
                        LayoutGrid(real st, std::string cl) :_step(st), _color(cl), 
                                                                  _visual(false) {};
      void              Init(real st, std::string cl) {_step = st; _color = cl;}
      void              Draw(const DrawProperties&, const real);
      real              step() const           {return _step;}
      bool              visual() const         {return _visual;}
      std::string       color() const          {return _color;}
      void              turnover(bool state)   { _visual = state;};
   private:
      real              _step;
      std::string       _color;
      bool              _visual;
   };


   //=============================================================================
   class tellRGB {
   public:
                        tellRGB(byte red, byte green, byte blue, byte alpha) :
                           _red(red), _green(green), _blue(blue), _alpha(alpha) {};
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

   typedef  std::map<byte       , LayoutGrid*   >  gridlist;
   typedef  std::map<std::string, tellRGB*      >  colorMAP;
   typedef  std::map<std::string, byte*         >  fillMAP;
   typedef  std::map<std::string, LineSettings* >  lineMAP;

   //=============================================================================
   class LayerSettings  {
   public:
      LayerSettings(std::string name, std::string color, std::string filltype, std::string sline): 
                                 _name(name), _color(color), _fill(filltype), _sline(sline),
                                                _hidden(false), _locked(false) {};
      std::string       color()    const {return _color;}
      std::string       fill()     const {return _fill;}
      std::string       name()     const {return _name;}
      std::string       sline()    const {return _sline;}
      bool              hidden()   const {return _hidden;}
      bool              locked()   const {return _locked;}
      friend class ViewProperties;
   private:
      std::string       _name;
      std::string       _color;
      std::string       _fill;
      std::string       _sline;
      bool              _hidden;
      bool              _locked;
   };

   typedef  std::map<word, LayerSettings*>      laySetList;

   //==============================================================================
   /*! This class serves as a carying case for all drawing properties during the
   drawing of the database. The fields of DrawProperties can be split logically 
   on two parts:
   - locked properties - these are the properties that stay constant during the
   drawing process. It is important to note that the class can not change 
   those properties, despite the fact that these are fields of its own. Instead 
   a friend class ViewProperties is doing this. The reason for this is primarily
   thread safety.
   - changable properties - these are changed during the drawing process - for 
   example current drawing layer, colours etc.
   */
   class DrawProperties {
   public:
                                 DrawProperties();
                                ~DrawProperties();
      void                       setCurrentColor(word layno);
      void                       setGridColor(std::string colname) const;
      bool                       getCurrentFill() const;
      bool                       getCurrentBoundary() const;
      void                       setLineProps(bool selected = false) const;
      bool                       layerHidden(word layno) const;
      bool                       layerLocked(word layno) const;
      const CTM&                 ScrCTM() const       {return  _ScrCTM;};
      const DBbox&               clipRegion() const   {return _clipRegion;};
      console::ACTIVE_OP         currentop() const    {return _currentop;}
      void                       blockfill(laydata::cellrefstack*);
      void                       unblockfill();
      void                       pushref(const laydata::tdtcellref*);
      byte                       popref(const laydata::tdtcellref*);
      void                       initCTMstack()       {_transtack.push(CTM());}
      void                       clearCTMstack()      {while (!_transtack.empty()) _transtack.pop();}
      void                       pushCTM(CTM& last)   {_transtack.push(last);}
      void                       popCTM()             {_transtack.pop();}
      const CTM&                 topCTM() const       {assert(_transtack.size());return _transtack.top();}
      void                       draw_reference_marks(const TP&, const binding_marks) const;
      word                       getLayerNo(std::string name) const;
      std::string                getLayerName(word layno) const;
      std::string                getColorName(word layno) const;
      std::string                getFillName(word layno) const;
      std::string                getLineName(word layno) const;
      word                       drawinglayer() const {return _drawinglayer;}
      const byte*                getFill(word layno) const;
      const byte*                getFill(std::string) const;
      const tellRGB&             getColor(word layno) const;
      const tellRGB&             getColor(std::string) const;
      const LineSettings*        getLine(word layno) const;
      const LineSettings*        getLine(std::string) const;
      void                       PSwrite(PSFile&) const;
      friend class ViewProperties;
   protected:
      laySetList                 _layset;
      colorMAP                   _laycolors;
      fillMAP                    _layfill;
      lineMAP                    _lineset;
      DBbox                      _clipRegion;
      CTM                        _ScrCTM;
      bool                       _cellmarks_hidden;
      bool                       _cellbox_hidden;
      bool                       _textmarks_hidden;
      bool                       _textbox_hidden;
      void                       savePatterns(FILE*) const;
      void                       saveColors(FILE*) const;
      void                       saveLayers(FILE*) const;
      void                       saveLines(FILE*) const;

   private:
      bool                       _blockfill;
      laydata::cellrefstack*     _refstack;
      ctmstack                   _transtack;
      word                       _drawinglayer;
      console::ACTIVE_OP         _currentop;
      static const tellRGB       _defaultColor;
      static const byte          _defaultFill[128];
      static const LineSettings  _defaultSeline;
   };

   //==============================================================================
   /*! The most important facet of this class is that the only way to change \b any
   drawing properties in Toped is to call a method of ViewProperties. It is important
   to note however, that this does not imply that all possible drawing properties in
   Toped have fields in this class. Those properties are split actually over two classes
   - ViewProperties holds the fields related to the layoutcanvas - list of grids, marker
   rules and restrictions etc.
   - DrawProperties is the placeholder for design related properties - colors, fill patterns,
   layout layers.
   
   The idea behind this split is that DrawProperties is used as a read-only property holder
   during drawing of the database, while ViewProperties fields are used by the layoutcanvas
   only. \n
   In the same time, to keep the properties thread safe DrawProperties does not posses with
   methods to change its own fields. Indeed, Toped has only one object of this class and the 
   only way to reach it is via ViewProperties. The latter has methods that can change the 
   fields of DrawProperties, but they are called in a thread safe manner.
   */
   class ViewProperties {
   public:
                        ViewProperties();
                       ~ViewProperties(); 
      bool              addlayer(std::string, word, std::string, std::string, std::string);
      bool              addlayer(std::string, word);
      void              addUnpublishedLay(word);
      const laydata::ListOfWords& upLayers() {_uplaylist.unique(); return _uplaylist;}
      void              clearUnpublishedLayers() {_uplaylist.clear();}
      void              addcolor(std::string name, byte R, byte G, byte B, byte A);
      void              addfill(std::string name, byte *ptrn);
      void              addline(std::string, std::string, word, byte, byte);
      void              hideLayer(word layno, bool hide);
      void              lockLayer(word layno, bool lock);
      bool              selectable(word layno) const;
      void              saveProperties(std::string) const;
      //      
      const LayoutGrid* grid(byte) const;
      void              setGrid(byte, real, std::string);
      bool              viewGrid(byte, bool);
      void              drawGrid() const;
      void              setUU(real);
      void              setstep(real st)                 {_step = st;}
      void              setautopan(bool status)          {_autopan = status;}
      void              setmarker_angle(byte angle)      {_marker_angle = angle;}
      real              step() const                     {return _step;}
      int4b             stepDB() const                   {return (word)rint(_step*_DBscale);}
      word              getLayerNo(std::string name) const {return _drawprop.getLayerNo(name);}
      std::string       getLayerName(word layno) const   {return _drawprop.getLayerName(layno);}
      real              UU() const                       {return _UU;}
      real              DBscale() const                  {return _DBscale;}
      bool              autopan() const                  {return _autopan;}
      byte              marker_angle() const             {return _marker_angle;}
      DrawProperties&   drawprop()                       {return _drawprop;}
      void              setcellmarks_hidden(bool hide)   {_drawprop._cellmarks_hidden = hide;}
      void              settextmarks_hidden(bool hide)   {_drawprop._textmarks_hidden = hide;}
      void              setcellbox_hidden(bool hide)     {_drawprop._cellbox_hidden = hide;}
      void              settextbox_hidden(bool hide)     {_drawprop._textbox_hidden = hide;}
      void              setScrCTM(CTM ScrCTM)            {_drawprop._ScrCTM = ScrCTM;}
      void              setClipRegion(DBbox clipR)       {_drawprop._clipRegion = clipR;}
      void              setCurrentOp(console::ACTIVE_OP actop)
                                                         {_drawprop._currentop = actop;}

      void              addRuler(TP& p1, TP& p2)         {_supp_data.addRuler(p1,p2,_UU);}
      void              clearRulers()                    {_supp_data.clearRulers();}
      void              drawRulers(const CTM& layCTM)    {_supp_data.drawRulers(layCTM, stepDB());}
      void              tmp_draw(const CTM& layCTM, const TP& base, const TP& newp)
                                                         {_supp_data.tmp_draw( base, newp, _UU, layCTM, stepDB());}
      void              mousePoint(const TP& lp)         {_supp_data.mousePoint(lp);}
      void              mouseStop()                      {_supp_data.mouseStop();}
      console::ACTIVE_OP currentop() const               {return _drawprop.currentop();}
      void              all_colors(nameList&) const;
      void              all_fills(nameList&) const;
      void              all_lines(nameList&) const;

      //
   protected:
      DrawProperties    _drawprop;
   private:
      void               saveScreenProps(FILE*) const;
      real              _DBscale; 
      real              _UU;           // The scale of the data base. It is doubled here, on order 
                                       // not to read it with every mouse move  
      gridlist          _grid;         // the list of grids as defined by the tell command
      real              _step;         // current marker step
      bool              _autopan;      // view window moves automatically during shape drawing
      byte              _marker_angle; // angle of restriction during shape drawing (0,45,90)
      SupplementaryData _supp_data;    // supplementary data
      laydata::ListOfWords _uplaylist;    // unpublished layer list
   };
}
#endif
