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
#include "tedstd.h"
#include "ps_out.h"

namespace layprop {

   typedef enum {cell_mark, array_mark, text_mark} binding_marks;

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

   //=============================================================================
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

   //=============================================================================
   typedef  std::map<std::string, tellRGB*      >  colorMAP;
   typedef  std::map<std::string, byte*         >  fillMAP;
   typedef  std::map<std::string, LineSettings* >  lineMAP;
   typedef  std::map<word       , LayerSettings*>  laySetList;



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
         void                       setLineProps(bool selected = false) const;
         bool                       layerHidden(word layno) const;
         bool                       layerLocked(word layno) const;
         const CTM&                 ScrCTM() const       {return  _ScrCTM;}
         const DBbox&               clipRegion() const   {return _clipRegion;}
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
         void                       draw_text_boundary(const pointlist& ptlist);
         void                       draw_cell_boundary(const pointlist& ptlist);
         word                       getLayerNo(std::string name) const;
         std::string                getLayerName(word layno) const;
         std::string                getColorName(word layno) const;
         std::string                getFillName(word layno) const;
         std::string                getLineName(word layno) const;
         void                       all_layers(nameList&) const;
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

}
#endif //DRAWPROP_H
