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

#include "tenderer.h"
namespace layprop {

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
                        ~SupplementaryData() {delete _tmp_base;}
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

   typedef  std::map<byte       , LayoutGrid*   >  gridlist;

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
      bool              addlayer(word);
      word              addlayer(std::string);
      void              addUnpublishedLay(word);
      const WordList&   upLayers() {_uplaylist.sort(); _uplaylist.unique(); return _uplaylist;}
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
      void              drawGrid(Tenderer&) const;
      void              drawZeroCross() const;
      void              setUU(real);
      void              setstep(real st)                 {_step = st;}
      void              setautopan(bool status)          {_autopan = status;}
      void              setZeroCross(bool status)        {_zeroCross = status;}
      void              setmarker_angle(byte angle)      {_marker_angle = angle;}
      real              step() const                     {return _step;}
      int4b             stepDB() const                   {return (word)rint(_step*_DBscale);}
      word              getLayerNo(std::string name) const {return _drawprop.getLayerNo(name);}
      std::string       getLayerName(word layno) const   {return _drawprop.getLayerName(layno);}
      real              UU() const                       {return _UU;}
      real              DBscale() const                  {return _DBscale;}
      bool              autopan() const                  {return _autopan;}
      bool              zeroCross() const                {return _zeroCross;}
      byte              marker_angle() const             {return _marker_angle;}
      DrawProperties&   drawprop()                       {return _drawprop;} // <-- That's rubbish!!! It most likely makes a copy! @FIXME
      DrawProperties*   drawprop_ptr()                   {return &_drawprop;}
      word              layselmask() const               {return _layselmask;}
      void              setlayselmask(word lsm)          {_layselmask = lsm;}
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
      void              all_layers(nameList& laylist) const {_drawprop.all_layers(laylist);}
      void              setGdsLayMap(USMap* map);
      void              setCifLayMap(USMap* map);
      const USMap*      getGdsLayMap() const             {return _gdsLayMap;}
      const USMap*      getCifLayMap() const             {return _cifLayMap;}
      void              all_colors(nameList&) const;
      void              all_fills(nameList&) const;
      void              all_lines(nameList&) const;

      //
   protected:
      DrawProperties       _drawprop;
   private:
      void                 saveScreenProps(FILE*) const;
      void                 saveLayerMaps(FILE*) const;
      real                 _DBscale;
      real                 _UU;           // The scale of the data base. It is mirrored here, on order
                                          // not to read it with every mouse move  
      gridlist             _grid;         // the list of grids as defined by the tell command
      real                 _step;         // current marker step
      bool                 _autopan;      // view window moves automatically during shape drawing
      bool                 _zeroCross;    //
      byte                 _marker_angle; // angle of restriction during shape drawing (0,45,90)
      SupplementaryData    _supp_data;    // supplementary data
      WordList             _uplaylist;    // unpublished layer list
      word                 _layselmask;   // layout shape type selection mask
      USMap*               _gdsLayMap;    //
      USMap*               _cifLayMap;    //
   };

   void USMap2String(USMap*, std::string&);

}
#endif
