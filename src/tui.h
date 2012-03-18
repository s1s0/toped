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
//          $URL$
//       Created: Thu Jun 17 2004
//    Originator: Svilen Krustev - skr@toped.org.uk
//   Description: Tell user interface classes
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#if !defined(TUI_H_INCLUDED)
#define TUI_H_INCLUDED

#include <wx/wx.h>
#include <wx/spinbutt.h>
#include <wx/propdlg.h>
#include "wx/odcombo.h"
#include "ttt.h"
#include "viewprop.h"

namespace tui {

   enum {
      COLOR_COMBO = 100 ,
      FILL_COMBO        ,
      LINE_COMBO        ,
      DRAW_SELECTED     ,
      ID_NEWITEM        ,
      ID_ITEMLIST       ,
      ID_REDVAL         ,
      ID_GREENVAL       ,
      ID_BLUEVAL        ,
      ID_ALPHAVAL       ,
      ID_BTNEDIT        ,
      ID_BTNAPPLY       ,
      ID_BTNCLEAR       ,
      ID_BTNFILL        ,
      ID_RADIOBSIZE     ,
      ID_CBDEFCOLOR     ,
      ID_CBDEFPATTERN   ,
      ID_CBDEFLINE      ,
      ID_SAVELAYMAP     ,
      ID_BTNDISPLAYADD  ,
      ID_BTNTECHADD     ,
      ID_BTNOUTFILE     ,
      ID_BTNCONVERT
   };

   typedef enum {
      IDLS_NEWSTYLE       ,
      IDLS_ITEMLIST       ,
      IDLS_NEWITEM        ,
      IDLS_BTNAPPLY       ,
      IDLS_PATVAL         ,
      IDLS_WIDTHVAL       ,
      IDLS_PATSCALEVAL
   } LineStyleIDs;

   typedef enum
   {
      ICON_SIZE_16x16 = 0, //must begin from 0
      ICON_SIZE_24x24,
      ICON_SIZE_32x32,
      ICON_SIZE_48x48,
      ICON_SIZE_END
   } IconSizes;

   const int IconSizesValues[ICON_SIZE_END] = {16, 24, 32, 48};
   //Order for this enum must corresponds IconSizes order
   enum
   {
      ICON_SIZE_16x16_V = 16,
      ICON_SIZE_24x24_V = 24,
      ICON_SIZE_32x32_V = 32,
      ICON_SIZE_48x48_V = 48
   };

   typedef enum {
      PDSET_CELLMARK      ,
      PDSET_CELLBOX       ,
      PDSET_TEXTMARK      ,
      PDSET_TEXTBOX       ,
      PDSET_TEXTORI       ,
      PDSET_TEXTFONTS     ,
      PDIMG_DETAIL        ,
      PDCELL_DOV          ,
      PDCELL_DAB          ,
      PDCELL_CHECKDOV     ,
      PDGRC_BLINKON       ,
      PDGRC_BLINKFREQ
   } RenderPropertyDialogID;

   typedef enum {
      CDMARKER_STEP       ,
      CDMARKER_STEP_SET   ,
      CDMARKER_MOTION     ,
      CDGRID_SET1         ,
      CDGRID_ENTER1       ,
      CDGRID_SET2         ,
      CDGRID_ENTER2       ,
      CDGRID_SET3         ,
      CDGRID_ENTER3       ,
      CDGRID_CBOX1        ,
      CDGRID_CBOX2        ,
      CDGRID_CBOX3        ,
      CDRECOVER_POLY      ,
      CDRECOVER_WIRE      ,
      CDMISC_LONGCURSOR   ,
      CDMISC_AUTOPAN      ,
      CDMISC_BOLDONHOOVER ,
      CDMISC_ZEROCROSS
   } CanvasPropertyDialogID;

   //--------------------------------------------------------------------------
   class getSize : public wxDialog {
   public:
      getSize(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, real step, byte precision, const float );
      wxString value() const {return _wxText->GetValue();};
   private:
      wxTextCtrl* _wxText;
   };

   //--------------------------------------------------------------------------
   class getStep : public wxDialog {
   public:
      getStep(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
                                                                     real init );
      wxString value() const {return _wxText->GetValue();};
   private:
      wxTextCtrl* _wxText;
   };

   //--------------------------------------------------------------------------
   class getGrid : public wxDialog {
   public:
      getGrid(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
                                                   real ig0, real ig1, real ig2);
      wxString grid0() const {return _wxGrid0->GetValue();};
      wxString grid1() const {return _wxGrid1->GetValue();};
      wxString grid2() const {return _wxGrid2->GetValue();};
   private:
      wxTextCtrl* _wxGrid0;
      wxTextCtrl* _wxGrid1;
      wxTextCtrl* _wxGrid2;
   };

   //--------------------------------------------------------------------------
   class getCellOpen : public wxDialog {
   public:
      getCellOpen(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
      wxString get_selectedcell() const {return _nameList->GetStringSelection();};
      wxListBox*  _nameList;
   };

   //--------------------------------------------------------------------------
   class getCellRef : public wxDialog {
   public:
      getCellRef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
      wxString get_selectedcell() const {return _nameList->GetStringSelection();};
      wxListBox*  _nameList;
   };

   //--------------------------------------------------------------------------
   class getCellARef : public wxDialog {
   public:
      getCellARef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
      wxString get_selectedcell() const {return _nameList->GetStringSelection();};
//      wxString get_angle() const {return _rotation->GetValue();};
      wxString get_col() const {return _numY->GetValue();};
      wxString get_row() const {return _numX->GetValue();};
      wxString get_stepX() const {return _stepX->GetValue();};
      wxString get_stepY() const {return _stepY->GetValue();};

//      bool  get_flip() const {return _flip->GetValue();};
//      wxTextCtrl* _rotation;
      wxTextCtrl* _numX;
      wxTextCtrl* _numY;
      wxTextCtrl* _stepX;
      wxTextCtrl* _stepY;
//      wxCheckBox* _flip;
      wxListBox*  _nameList;
   };

   //--------------------------------------------------------------------------
   class getTextdlg : public wxDialog {
   public:
      getTextdlg(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos);
      wxString    get_text()  const {return _text->GetValue();};
      wxString    get_size()  const {return _size->GetValue();};
      wxString    get_angle() const {return _rotation->GetValue();};
      bool        get_flip() const  {return _flip->GetValue();};
   protected:
      wxTextCtrl* _text;
      wxTextCtrl* _size;
      wxTextCtrl* _rotation;
      wxCheckBox* _flip;
   };

   //--------------------------------------------------------------------------
   class sgSpinButton : public wxSpinButton {
   public:
      sgSpinButton(wxWindow *parent, wxTextCtrl* textW, const float step,
      const float min, const float max, const float value, const int prec) ;
   private:
      wxTextCtrl* _wxText;
      float _step;
      int   _prec;
      void  OnSpin(wxSpinEvent&);
      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class sgSliderControl : public wxPanel {
   public:
                        sgSliderControl(wxWindow*, int, int, int, int);
      void              setValue(int);
      int               getValue();
   private:
      wxTextCtrl*       _text;
      wxSlider*         _slider;
      void              OnScroll(wxScrollEvent&);
      void              OnTextEnter(wxCommandEvent& WXUNUSED(event));
      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class getLibList : public wxDialog {
      public:
         getLibList(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
         wxString get_selected() const {return _nameList->GetStringSelection();};
         wxListBox*  _nameList;
   };

   //==========================================================================
   class NameCboxRecords : public wxPanel {
      public:
                               NameCboxRecords(wxWindow*, wxPoint, wxSize, const SIMap&, wxArrayString&, int, const layprop::DrawProperties*);
         virtual              ~NameCboxRecords() {delete _cifMap;}
         SIMap*                getTheMap(layprop::DrawProperties*);
         USMap*                getTheFullMap(layprop::DrawProperties*);
      private:
         class LayerRecord {
            public:
                                 LayerRecord(wxCheckBox* ciflay, wxComboBox* tdtlay) : _ciflay(ciflay), _tdtlay(tdtlay) {};
               wxCheckBox*       _ciflay;
               wxComboBox*       _tdtlay;
         };
         typedef std::list<LayerRecord> AllRecords;
         const layprop::DrawProperties* _drawProp;
         AllRecords              _allRecords;
         LayerMapCif*           _cifMap;
   };

   //==========================================================================
   class NameCbox3Records : public wxPanel {
      public:
                              NameCbox3Records(wxWindow*, wxPoint, wxSize, const ExtLayers&, wxArrayString&, int, const layprop::DrawProperties*);
         virtual             ~NameCbox3Records() {delete _gdsLayMap;}
         USMap*               getTheMap();
         USMap*               getTheFullMap();
         word                 getNumRows() const {return _allRecords.size();}
      private:
         class LayerRecord {
            public:
                              LayerRecord(wxCheckBox* gdslay, wxStaticText* gdstype, wxComboBox* tdtlay) :
                                 _gdslay(gdslay), _gdstype(gdstype), _tdtlay(tdtlay) {};
               wxCheckBox*       _gdslay;
               wxStaticText*     _gdstype;
               wxComboBox*       _tdtlay;
         };
         typedef std::list<LayerRecord> AllRecords;
         const layprop::DrawProperties* _drawProp;
         AllRecords              _allRecords;
         LayerMapExt*            _gdsLayMap;
   };

   //==========================================================================
   class NameEboxRecords : public wxPanel {
      public:
                              NameEboxRecords(wxWindow*, wxPoint, wxSize, const WordList&, wxArrayString&, int, const layprop::DrawProperties*);
         virtual             ~NameEboxRecords() {delete _cifMap;}
         USMap*               getTheMap();
         USMap*               getTheFullMap();
      private:
         class LayerRecord {
            public:
                              LayerRecord(wxCheckBox* tdtlay, wxTextCtrl* ciflay) :
                                 _tdtlay(tdtlay), _ciflay(ciflay) {};
               wxCheckBox*    _tdtlay;
               wxTextCtrl*    _ciflay;
         };
         typedef std::list<LayerRecord> AllRecords;
         const layprop::DrawProperties* _drawProp;
         AllRecords           _allRecords;
         LayerMapCif*         _cifMap;
   };

   //==========================================================================
   class NameEbox3Records : public wxPanel {
      public:
                              NameEbox3Records(wxWindow*, wxPoint, wxSize, const WordList&, wxArrayString&, int, const layprop::DrawProperties*);
         virtual             ~NameEbox3Records() {delete _gdsLayMap;}
         USMap*               getTheMap();
         USMap*               getTheFullMap();
      private:
         class LayerRecord {
            public:
                              LayerRecord(wxCheckBox* tdtlay, wxTextCtrl* gdslay, wxTextCtrl* gdstype) :
                                 _tdtlay(tdtlay), _gdslay(gdslay), _gdstype(gdstype) {};
               wxCheckBox*    _tdtlay;
               wxTextCtrl*    _gdslay;
               wxTextCtrl*    _gdstype;
         };
         typedef std::list<LayerRecord> AllRecords;
         const layprop::DrawProperties* _drawProp;
         AllRecords           _allRecords;
         LayerMapExt*         _gdsLayMap;
   };

   //--------------------------------------------------------------------------
   class NameCboxList : public wxScrolledWindow {
      public:
                              NameCboxList(wxWindow*, wxWindowID, wxPoint, wxSize, const SIMap&, const layprop::DrawProperties*);
         SIMap*               getTheMap(layprop::DrawProperties* drawProp)
                                              {return _laypanel->getTheMap(drawProp);}
         USMap*               getTheFullMap(layprop::DrawProperties* drawProp)
                                              {return _laypanel->getTheFullMap(drawProp);}
         void                 OnSize( wxSizeEvent& WXUNUSED(event));
      private:
         tui::NameCboxRecords*   _laypanel;
         DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class NameCbox3List : public wxScrolledWindow {
      public:
                              NameCbox3List(wxWindow*, wxWindowID, wxPoint, wxSize, const ExtLayers&, const layprop::DrawProperties*, wxString);
         USMap*               getTheMap()     {return _laypanel->getTheMap();}
         USMap*               getTheFullMap() {return _laypanel->getTheFullMap();}
         void                 OnSize( wxSizeEvent& WXUNUSED(event));
      private:
         tui::NameCbox3Records*   _laypanel;
         DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class NameEboxList : public wxScrolledWindow {
      public:
                              NameEboxList(wxWindow*, wxWindowID, wxPoint, wxSize, const WordList&, const layprop::DrawProperties*);
         USMap*               getTheMap()     {return _laypanel->getTheMap();}
         USMap*               getTheFullMap() {return _laypanel->getTheFullMap();}
         void                 OnSize( wxSizeEvent& WXUNUSED(event));
      private:
         tui::NameEboxRecords* _laypanel;
         DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class nameEbox3List : public wxScrolledWindow {
      public:
                              nameEbox3List(wxWindow*, wxWindowID, wxPoint, wxSize, const WordList&, const layprop::DrawProperties*);
         USMap*               getTheMap()     {return _laypanel->getTheMap();}
         USMap*               getTheFullMap() {return _laypanel->getTheFullMap();}
         void                 OnSize( wxSizeEvent& WXUNUSED(event));
      private:
         tui::NameEbox3Records* _laypanel;
         DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class GetCIFimport : public wxDialog {
   public:
                        GetCIFimport(wxFrame *parent, wxWindowID id, const wxString &title,
                                 wxPoint pos, wxString init, const layprop::DrawProperties*);
      wxString          getSelectedCell() const {return _nameList->GetStringSelection();}
      wxString          getTechno()       const {return _techno;}
      bool              getOverwrite()    const {return _overwrite->GetValue();}
      bool              getRecursive()    const {return _recursive->GetValue();}
      bool              getSaveMap()      const {return _saveMap->GetValue();}
      SIMap*            getCifLayerMap(layprop::DrawProperties* drawProp)
                                                {return _layList->getTheMap(drawProp);}
      USMap*            getFullCifLayerMap(layprop::DrawProperties* drawProp)
                                                {return _layList->getTheFullMap(drawProp);}
   private:
      wxCheckBox*       _overwrite;
      wxCheckBox*       _recursive;
      wxCheckBox*       _saveMap;
      wxListBox*        _nameList;
      NameCboxList*     _layList;
      wxString          _techno;
   };

   //--------------------------------------------------------------------------
   class GetCIFexport : public wxDialog {
   public:
                        GetCIFexport(wxFrame *parent, wxWindowID id, const wxString &title,
                                   wxPoint pos, wxString init, const layprop::DrawProperties*);
      wxString          get_selectedcell() const {return _nameList->GetStringSelection();}
      bool              get_recursive()    const {return _recursive->GetValue();}
      bool              get_slang()        const {return _slang->GetValue();}
      bool              getSaveMap()      const {return _saveMap->GetValue();}
      USMap*            getCifLayerMap()         {return _layList->getTheMap();}
      USMap*            getFullCifLayerMap()     {return _layList->getTheFullMap();}
   private:
      wxCheckBox*       _recursive;
      wxCheckBox*       _saveMap;
      wxCheckBox*       _slang;
      wxListBox*        _nameList;
      NameEboxList*     _layList;
   };

   //--------------------------------------------------------------------------
   class GetGDSimport : public wxDialog {
   public:
                        GetGDSimport(wxFrame *parent, wxWindowID id, const wxString &title,
                                    wxPoint pos, wxString init, const layprop::DrawProperties*);
      wxString          get_selectedcell() const {return _nameList->GetStringSelection();};
      bool              get_overwrite()    const {return _overwrite->GetValue();};
      bool              get_recursive()    const {return _recursive->GetValue();};
      bool              getSaveMap()       const {return _saveMap->GetValue();}
      USMap*            getGdsLayerMap()         {return _layList->getTheMap();}
      USMap*            getFullGdsLayerMap()     {return _layList->getTheFullMap();}
   private:
      wxCheckBox*       _overwrite;
      wxCheckBox*       _recursive;
      wxCheckBox*       _saveMap;
      wxListBox*        _nameList;
      NameCbox3List*    _layList;
   };

   //--------------------------------------------------------------------------
   class GetOASimport : public wxDialog {
   public:
                        GetOASimport(wxFrame *parent, wxWindowID id, const wxString &title,
                                  wxPoint pos, wxString init, const layprop::DrawProperties*);
      wxString          get_selectedcell() const {return _nameList->GetStringSelection();};
      bool              get_overwrite()    const {return _overwrite->GetValue();};
      bool              get_recursive()    const {return _recursive->GetValue();};
      bool              getSaveMap()       const {return _saveMap->GetValue();}
      USMap*            getOasLayerMap()         {return _layList->getTheMap();}
      USMap*            getFullOasLayerMap()     {return _layList->getTheFullMap();}
   private:
      wxCheckBox*       _overwrite;
      wxCheckBox*       _recursive;
      wxCheckBox*       _saveMap;
      wxListBox*        _nameList;
      NameCbox3List*    _layList;
   };

   //--------------------------------------------------------------------------
   class GetGDSexport : public wxDialog {
   public:
                        GetGDSexport(wxFrame *parent, wxWindowID id, const wxString &title,
                                  wxPoint pos, wxString init, const layprop::DrawProperties*);
      wxString          get_selectedcell() const {return _nameList->GetStringSelection();};
      bool              get_recursive()    const {return _recursive->GetValue();};
      bool              getSaveMap()       const {return _saveMap->GetValue();}
      USMap*            getGdsLayerMap()         {return _layList->getTheMap();}
      USMap*            getFullGdsLayerMap()     {return _layList->getTheFullMap();}
      private:
      wxCheckBox*       _recursive;
      wxCheckBox*       _saveMap;
      wxListBox*        _nameList;
      nameEbox3List*    _layList;
   };

   //--------------------------------------------------------------------------
   class ColorSample : public wxWindow {
   public:
                     ColorSample(wxWindow*, wxWindowID, wxPoint, wxSize);
      void           setColor(const layprop::tellRGB&);
      void           OnPaint(wxPaintEvent&);
   protected:
      wxColour             _color;
      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class DefineColor : public wxDialog
   {
   public:
      typedef  std::pair<bool, layprop::tellRGB>         RcsColor;
      typedef  std::map<std::string, RcsColor>           ColorLMap;
                        DefineColor(wxWindow *parent, wxWindowID id, const wxString &title,
                                             wxPoint pos, const layprop::DrawProperties*);
      virtual          ~DefineColor() {}
      void              OnDefineColor(wxCommandEvent&);
      void              OnColorSelected(wxCommandEvent&);
      void              OnApply(wxCommandEvent& WXUNUSED(event));
      void              OnColorPropChanged(wxCommandEvent& WXUNUSED(event));
      void              OnColorNameAdded(wxCommandEvent& WXUNUSED(event));
      const ColorLMap&  allColors() const {return _allColors;}
   private:
      void                    nameNormalize(wxString&);
      layprop::tellRGB        getColor(std::string color_name) const;
      ColorLMap               _allColors;
      wxListBox*              _colorList;
      wxTextCtrl*             _dwcolname;
      ColorSample*            _colorsample;
      wxString                _colname;
      wxString                _red;
      wxString                _green;
      wxString                _blue;
      wxString                _alpha;
      const layprop::DrawProperties*  _drawProp;

      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class PatternCanvas : public wxWindow {
   public:
                     PatternCanvas(wxWindow*, wxWindowID, wxPoint, wxSize, const byte*);
      void           OnPaint(wxPaintEvent&);
      void           Clear();
      void           Fill();
      void           setBrushSize(byte bsize) {_brushsize = bsize;}
      byte*          pattern() {return _pattern;}
   protected:
      void           OnMouseLeftUp(wxMouseEvent& event);
      void           OnMouseRightUp(wxMouseEvent& event);
   private:
      byte           _pattern[128];
      byte           _brushsize;
      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class DrawFillDef : public wxDialog {
   public:
                     DrawFillDef(wxWindow *parent, wxWindowID id, const wxString &title,
                                                                  wxPoint pos, const byte*);
      byte*          pattern() {return _sampleDraw->pattern();}
      virtual       ~DrawFillDef();
   protected:
      void              OnClear(wxCommandEvent& WXUNUSED(event));
      void              OnFill(wxCommandEvent& WXUNUSED(event));
      void              OnBrushSize(wxCommandEvent&);
      PatternCanvas*   _sampleDraw;
      wxRadioBox*       _radioBrushSize;
      DECLARE_EVENT_TABLE();
   };

   class FillSample : public wxWindow {
   public:
                     FillSample(wxWindow*, wxWindowID, wxPoint, wxSize);
      void           setFill(const byte*);
      void           OnPaint(wxPaintEvent&);
   protected:
      wxBrush        _brush;
      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class DefineFill : public wxDialog
   {
   public:
      typedef  std::map<std::string, byte*         >  fillMAP;
                     DefineFill(wxFrame *parent, wxWindowID id, const wxString &title,
                                             wxPoint pos, const layprop::DrawProperties*);
      virtual       ~DefineFill();
      void           OnDefineFill(wxCommandEvent&);
      void           OnFillSelected(wxCommandEvent&);
      void           OnFillNameAdded(wxCommandEvent& WXUNUSED(event));
      fillMAP&       allPatterns() {return _allFills;}
   private:
      void           nameNormalize(wxString&);
      void           fillcopy(const byte*, byte*);
      const byte*    getFill(const std::string) const;
      fillMAP        _allFills;
      wxListBox*     _fillList;
      wxTextCtrl*    _dwfilname;
      FillSample*    _fillsample;
      wxString       _filname;
      byte           _current_pattern[128];

      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   struct style_def
   {
      word     pattern;
      byte     pscale;
      byte     width;
   };

   //--------------------------------------------------------------------------
   class StyleBinaryView : public wxTextCtrl {
   public:
                     StyleBinaryView(wxWindow*, wxWindowID);
      virtual void   ChangeValue(const word&);
      word           GetValue();
      void           OnKey(wxKeyEvent&);
   private:
      wxString       _patternString;
      static const wxTextPos   _pattern_size = 16;
      DECLARE_EVENT_TABLE();

   };
   //--------------------------------------------------------------------------
   class LineStyleSample : public wxControl {
   public:
                     LineStyleSample(wxWindow*, wxWindowID, std::string, const layprop::DrawProperties*);
      virtual       ~LineStyleSample();
      void           setStyle(const tui::style_def& styledef);
      void           OnPaint(wxPaintEvent&);
   protected:
      wxBrush        _brush;
      wxPen          _pen;
      DECLARE_EVENT_TABLE();
   };

   typedef  std::map<std::string, style_def       >  styleMAP;

   class DefineLineStyle : public wxDialog {
   public:
                     DefineLineStyle(wxFrame *parent, wxWindowID id, const wxString &title,
                                             wxPoint pos, const layprop::DrawProperties*);
      virtual       ~DefineLineStyle();
      void           OnStyleSelected(wxCommandEvent&);
      void           OnStyleNameAdded(wxCommandEvent&);
      void           OnStyleNameChanged(wxCommandEvent&);
      void           OnStylePropChanged(wxCommandEvent&);
      void           OnStyleApply(wxCommandEvent&);
      styleMAP&      allStyles() {return _allStyles;}
   private:
      style_def      getStyle(const std::string&);
      void           nameNormalize(wxString& str);
      void           updateDialog();
      styleMAP       _allStyles;
      wxListBox*     _styleList;
      wxTextCtrl*    _dwstylename;
      LineStyleSample*  _stylesample;
      wxString       _stylename;
      style_def      _current_style;
      StyleBinaryView* _pattern;
      wxTextCtrl*    _width;
      wxString       _widthString;
      wxTextCtrl*    _patscale;
      wxString       _patscaleString;
      DECLARE_EVENT_TABLE();
   };

   //--------------------------------------------------------------------------
   class cadenceConvert : public wxDialog
   {
   public:
      cadenceConvert(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos);
   protected:
      wxTextCtrl* _displayList;
      wxTextCtrl* _techList;
      wxTextCtrl* _outputFile;
   private:
      void  onDisplayAdd(wxCommandEvent&);
      void  onTechAdd(wxCommandEvent&);
      void  onOutputFile(wxCommandEvent&);
      void  onConvert(wxCommandEvent&);
      DECLARE_EVENT_TABLE();
   };

   class TopedPropertySheets : public wxPropertySheetDialog {
   public:
                                TopedPropertySheets(wxWindow*);
      void                      updateRenderSheet(wxCommandEvent& evt) {_renderingSheet->update(evt);}
      void                      updateCanvasSheet(wxCommandEvent& evt) {   _canvasSheet->update(evt);}
   private:
      class RenderingPSheet : public wxPanel {
      public:
                                RenderingPSheet(wxWindow*);
         void                   update(wxCommandEvent&);
      private:
         void                   OnCellCheckDov(wxCommandEvent&);
         void                   OnCellBox (wxCommandEvent&);
         void                   OnCellMark(wxCommandEvent&);
         void                   OnTextBox (wxCommandEvent&);
         void                   OnTextMark(wxCommandEvent&);
         void                   OnTextOri (wxCommandEvent&);
         void                   OnTextFont(wxCommandEvent&);
         void                   OnCellDov (wxCommandEvent&);
         void                   OnCellDab (wxCommandEvent&);
         void                   OnImageDetail (wxCommandEvent&);
         void                   OnGrcBlinkOn(wxCommandEvent&);
         void                   OnGrcBlinkFreq(wxCommandEvent&);
         wxCheckBox*            _cbDepthOfViewLimit;
         wxCheckBox*            _cbGrcBlinkOn;
         sgSliderControl*       _cellDepthOfView;
         sgSliderControl*       _cellDepthEbb;
         sgSliderControl*       _imageDetail;
         sgSliderControl*       _cbGrcBlinkFreq;
         DECLARE_EVENT_TABLE();
      };
      class CanvasPSheet : public wxPanel {
      public:
                                CanvasPSheet(wxWindow*);
         void                   update(wxCommandEvent&);
      private:
         void                   OnMarkerStep  (wxCommandEvent& WXUNUSED(event));
         void                   OnMarkerMotion(wxCommandEvent&);
         void                   OnGridOn1     (wxCommandEvent&);
         void                   OnGridOn2     (wxCommandEvent&);
         void                   OnGridOn3     (wxCommandEvent&);
         void                   OnGridSet1    (wxCommandEvent& WXUNUSED(event));
         void                   OnGridSet2    (wxCommandEvent& WXUNUSED(event));
         void                   OnGridSet3    (wxCommandEvent& WXUNUSED(event));
         void                   OnRecoverPoly (wxCommandEvent&);
         void                   OnRecoverWire (wxCommandEvent&);
         void                   OnLongCorsor  (wxCommandEvent&);
         void                   OnAutoPan     (wxCommandEvent&);
         void                   OnBoldOnHoover(wxCommandEvent&);
         void                   OnZeroCross   (wxCommandEvent&);
         DECLARE_EVENT_TABLE();
      };
      RenderingPSheet*          _renderingSheet;
      CanvasPSheet*             _canvasSheet;
   };

   //Return brush for corresponding color and filling
   //This function is used for visualisation in GUI (layer buttons, technology editor etc)
   wxBrush* makeBrush(const byte* ifill, const layprop::tellRGB col);
   //! Makes a structure ready to be used in wxPen for dashed lines
   unsigned makePenDash(word, byte, wxDash*&);
}

//Print output of external functions
//to Toped console
void ShowOutput(const wxString& cmd,
                         const wxArrayString& output,
                         const wxString& title);
#endif
