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

#include "drawprop.h"
namespace layprop {

   //=============================================================================
   class SupplementaryData {
   public:
                        SupplementaryData() {_tmp_base = NULL;}
                        ~SupplementaryData() {delete _tmp_base;}
      void              addRuler(TP&, TP&, const real);
      void              clearRulers();
      void              mousePoint(const TP&);
      void              mouseStop();
      const RulerList&  getAllRulers() {return _rulers;}
   protected:
      RulerList         _rulers;
      TP*               _tmp_base;
   };

   //=============================================================================
   class LayoutGrid {
   public:
                        LayoutGrid(real st, std::string cl) :_step(st), _color(cl),
                                                                  _visual(false) {};
      void              Init(real st, std::string cl) {_step = st; _color = cl;}
      real              step() const           {return _step;}
      bool              visual() const         {return _visual;}
      std::string       color() const          {return _color;}
      void              turnover(bool state)   { _visual = state;};
   private:
      real              _step;
      std::string       _color;
      bool              _visual;
   };

   //==============================================================================
   /*! The most important facet of this class is that the only way to change \b any
   drawing properties in Toped is to call a method of PropertyCenter. It is important
   to note however, that this does not imply that all possible drawing properties in
   Toped have fields in this class. Those properties are split actually over two classes
   - PropertyCenter holds the fields related to the layoutcanvas - list of grids, marker
   rules and restrictions etc.
   - DrawProperties is the place holder for design related properties - colors, fill patterns,
   layout layers.

   The idea behind this split is that DrawProperties is used as a read-only property holder
   during drawing of the database, while PropertyCenter fields are used by the layoutcanvas
   only. \n
   In the same time, to keep the properties thread safe DrawProperties does not possess with
   methods to change its own fields. Indeed, Toped has only one object of this class and the
   only way to reach it is via PropertyCenter. The latter has methods that can change the
   fields of DrawProperties, but they are called in a thread safe manner.
   */
   class PropertyCenter {
   public:
      typedef  std::map<byte       , LayoutGrid*   >  gridlist;
                        PropertyCenter();
                       ~PropertyCenter();
      void              addUnpublishedLay(const LayerDef&);
      void              saveProperties(std::string, std::string);
      //
      const LayoutGrid* grid(byte) const;
      void              setGrid(byte, real, std::string);
      bool              viewGrid(byte, bool);
      void              setUU(real);
      void              setGdsLayMap(ExpLayMap* map);
      void              setCifLayMap(ExpLayMap* map);
      void              setOasLayMap(ExpLayMap* map);
      LayerDefSet       allUnselectable();
      DBlineList        getZCross();
      bool              lockDrawProp(DrawProperties*&, PropertyState state = DB);
      bool              tryLockDrawProp(DrawProperties*&, PropertyState state = DB);
      void              unlockDrawProp(DrawProperties*&, bool throwexception);

      void              setStep(real st)                 {_step = st;}
      void              setAutoPan(bool status)          {_autopan = status;}
      void              setZeroCross(bool status)        {_zeroCross = status;}
      void              setHighlightOnHover(bool hoh)    {_boldOnHover = hoh;}
      void              setMarkerAngle(byte angle)       {_markerAngle = angle;}
      void              setLaySelMask(word lsm)          {_layselmask = lsm;}
      void              addRuler(TP& p1, TP& p2)         {_supp_data.addRuler(p1,p2,_UU);}
      void              clearRulers()                    {_supp_data.clearRulers();}
      const RulerList&  getAllRulers()                   {return _supp_data.getAllRulers();}
      void              mousePoint(const TP& lp)         {_supp_data.mousePoint(lp);}
      void              mouseStop()                      {_supp_data.mouseStop();}
      const LayerDefList&  upLayers()                       {_uplaylist.sort(); _uplaylist.unique(); return _uplaylist;}
      void              clearUnpublishedLayers()         {_uplaylist.clear();}
      real              step() const                     {return _step;}
      int4b             stepDB() const                   {return (word)rint(_step*_DBscale);}
      real              UU() const                       {return _UU;}
      real              DBscale() const                  {return _DBscale;}
      bool              autopan() const                  {return _autopan;}
      bool              boldOnHover() const              {return _boldOnHover;}
      bool              zeroCross() const                {return _zeroCross;}
      byte              markerAngle() const              {return _markerAngle;}
      word              layselmask() const               {return _layselmask;}
      const ExpLayMap*  getGdsLayMap() const             {return _gdsLayMap;}
      const ExpLayMap*  getCifLayMap() const             {return _cifLayMap;}
      const ExpLayMap*  getOasLayMap() const             {return _oasLayMap;}
      bool              gridVisual(word no)              {return grid(no)->visual();}
   private:
      DrawProperties*      _drawprop;
      void                 saveScreenProps(FILE*) const;
      void                 saveLayerMaps(FILE*) const;
      real                 _DBscale;
      real                 _UU;           // The scale of the data base. It is mirrored here, on order
                                          // not to read it with every mouse move
      gridlist             _grid;         // the list of grids as defined by the tell command
      real                 _step;         // current marker step
      bool                 _autopan;      // view window moves automatically during shape drawing
      bool                 _zeroCross;    //
      bool                 _boldOnHover;  //
      byte                 _markerAngle;  // angle of restriction during shape drawing (0,45,90)
      SupplementaryData    _supp_data;    // supplementary data
      LayerDefList         _uplaylist;    // unpublished layer list
      word                 _layselmask;   // layout shape type selection mask
      ExpLayMap*           _gdsLayMap;    //
      ExpLayMap*           _cifLayMap;    //
      ExpLayMap*           _oasLayMap;    //
      wxMutex              _drawPLock;    // DrawPropwerties lock
   };

//   void USMap2String(USMap*, std::string&);
   void ExtLayerMap2String(ExpLayMap*, std::string&);

}
#endif
