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
// ------------------------------------------------------------------------ =
//           $URL: http://perun/tpd_svn/trunk/toped_wx/tpd_DB/viewprop.h $
//  Creation date: Sun Sep 29 2002
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision: 218 $
//          $Date: 2005-09-30 23:04:05 +0100 (Fri, 30 Sep 2005) $
//        $Author: skr $
//===========================================================================

#ifndef LAYERPROP_H
#define LAYERPROP_H

#include <string>
#include <math.h>
#include "tedstd.h"
namespace layprop {

   typedef enum {cell_mark, array_mark, text_mark} binding_marks;
   typedef enum {op_none, op_move, op_copy} ACTIVE_OP;

   class ViewProperties;
   class DrawProperties;
   //=============================================================================
   class LayoutGrid {
   public:
                        LayoutGrid(real st, std::string cl) :_step(st), _color(cl), 
                                                                  _visual(true) {};
      void              Init(real st, std::string cl) {_step = st; _color = cl; 
                                                                  _visual = true;};
      void              Draw(const DrawProperties&, const real);
      real              step() const           {return _step;};
      bool              visual() const         {return _visual;};
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
      byte              red()   {return _red;     };
      byte              green() {return _green;   };
      byte              blue()  {return _blue;    };
      byte              alpha() {return _alpha;   };
   protected:
      byte              _red;
      byte              _green;
      byte              _blue;
      byte              _alpha;
   };

   typedef  std::map<byte       , LayoutGrid*>  gridlist;
   typedef  std::map<std::string, tellRGB*   >  colorMAP;
   typedef  std::map<std::string, byte*      >  fillMAP;

   //=============================================================================
   class LayerSettings  {
   public:
      LayerSettings(std::string name, std::string color, std::string filltype): 
                                 _name(name), _color(color), _filltype(filltype), 
                                                _hidden(false), _locked(false) {};
      std::string       getcolor() const {return _color;};
      std::string       getfill()  const {return _filltype;};
      std::string       name()     const {return _name;};
      bool              hidden()   const {return _hidden;}; 
      bool              locked()   const {return _locked;}; 
      friend class ViewProperties;
   private:
      std::string       _name;
      std::string       _color;
      std::string       _filltype;
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
      void              setCurrentColor(word layno);
      void              setGridColor(std::string colname) const;
      bool              getCurrentFill() const;
      bool              layerHidden(word layno) const;
      bool              layerLocked(word layno) const;
      const CTM&        ScrCTM() const                   {return  _ScrCTM;};
      const DBbox&      clipRegion() const               {return _clipRegion;};
      ACTIVE_OP         currentop() const                {return _currentop;}
      void              blockfill(laydata::cellrefstack*);
      void              unblockfill();
      void              pushref(const laydata::tdtcellref*);
      byte              popref(const laydata::tdtcellref*);
      void              draw_reference_marks(const TP&, const binding_marks) const;
      word              getlayerNo(std::string name) const;
      friend class ViewProperties;
   protected:
      laySetList        _layset;
      colorMAP          _laycolors;
      fillMAP           _layfill;
      DBbox             _clipRegion;
      CTM               _ScrCTM;
      bool              _cellmarks_hidden;
      bool              _textmarks_hidden;
   private:
      bool              _blockfill;
      laydata::cellrefstack*  _refstack;
      word              _drawinglayer; 
      ACTIVE_OP         _currentop; 
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
      void              addlayer(std::string name, word layno, std::string col,
                                                               std::string fill);
      void              addcolor(std::string name, byte R, byte G, byte B, byte A);
      void              addfill(std::string name, byte *ptrn);
      void              hideLayer(word layno, bool hide);
      void              lockLayer(word layno, bool lock);
      bool              selectable(word layno) const;
      //      
      const LayoutGrid* grid(byte) const;
      void              setGrid(byte, real, std::string);
      bool              viewGrid(byte, bool);
      void              drawGrid() const;
      void              setcellmarks_hidden(bool hide);
      void              settextmarks_hidden(bool hide);
      void              setstep(real st)                 {_step = st;};
      real              step() const                     {return _step;};
      int4b             stepDB() const                   {return (word)rint(_step*_DBscale);};
      void              defaultlayer(word layno)         {_curlay = layno;};
      word              curlay() const                   {return _curlay;};
      word              getlayerNo(std::string name) const {return _drawprop.getlayerNo(name);};
      void              setUU(real);//                  {_DBscale = DB;};
      real              UU() const                       {return _UU;};
      real              DBscale() const                  {return _DBscale;};
      void              setautopan(bool status)          {_autopan = status;};
      bool              autopan() const                  {return _autopan;};
      void              setmarker_angle(byte angle)      {_marker_angle = angle;};
      byte              marker_angle() const             {return _marker_angle;};
      void              setScrCTM(CTM ScrCTM)            {_drawprop._ScrCTM = ScrCTM;};
      void              setClipRegion(DBbox clipR)       {_drawprop._clipRegion = clipR;};
      DrawProperties&   drawprop()                       {return _drawprop;};
      void              setCurrentOp(int actop);
      //
   protected:
      DrawProperties    _drawprop;
   private:
      real              _DBscale; 
      real              _UU;           // The scale of the data base. It is doubled here, on order 
                                       // not to read it with every mouse move  
      gridlist          _grid;         // the list of grids as defined by the tell command
      real              _step;         // current marker step
      word              _curlay;       // current drawing layer
      bool              _autopan;      // view window moves automatically during shape drawing
      byte              _marker_angle; // angle of restriction during shape drawing (0,45,90) 
   };
}
#endif
