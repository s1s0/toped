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
//        Created: Wed Apr 11 2011
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Technology Editor 
//                 
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#ifndef __TECHEDITOR__
#define __TECHEDITOR__

#include <wx/panel.h>
#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include "tui.h"

namespace tui
{
   typedef enum {
      DTE_LAYERS_LIST    ,
      DTE_NEWLAYER       ,
      DTE_NEWCOLOR       ,
      DTE_NEWFILL        ,
      DTE_NEWSTYLE       ,
      DTE_FILL_COMBO     ,
      DTE_LINE_COMBO     ,
      DTE_COLOR_COMBO    ,
      DTE_LAYER_NUM      ,
      DTE_LAYER_TYPE     ,
      DTE_LAYER_NAME     ,
      DTE_APPLY
   } DialogTechEditor;

   ///////////////////////////////////////////////////////////////////////////////
   /// Class ColorListComboBox
   ///////////////////////////////////////////////////////////////////////////////
   class ColorListComboBox : public wxOwnerDrawnComboBox
   {
      public:
                      ColorListComboBox( layprop::DrawProperties* );
         virtual     ~ColorListComboBox();
         virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
         virtual void Clear();
         void         populate( layprop::DrawProperties* );
         //virtual void OnDrawBackground( wxDC& dc, const wxRect& rect, int item, int flags ) const;
      private:
         layprop::ColorMap _colors;
   };


   ///////////////////////////////////////////////////////////////////////////////
   /// Class FillingListComboBox
   ///////////////////////////////////////////////////////////////////////////////
   class FillListComboBox : public wxOwnerDrawnComboBox
   {
      public:
                      FillListComboBox( layprop::DrawProperties* );
         virtual     ~FillListComboBox();
         virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
         virtual void Clear();
         void         populate( layprop::DrawProperties* );
   //      virtual void OnDrawBackground( wxDC& dc, const wxRect& rect, int item, int flags ) const;
      private:
         layprop::FillMap _fills;
   };

   ///////////////////////////////////////////////////////////////////////////////
   /// Class LineListComboBox
   ///////////////////////////////////////////////////////////////////////////////
   class LineListComboBox : public wxOwnerDrawnComboBox
   {
      public:
                      LineListComboBox( layprop::DrawProperties* );
         virtual     ~LineListComboBox();
         virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
         virtual wxCoord OnMeasureItem(size_t size) const {return 24;};
         virtual void Clear();
         void         populate( layprop::DrawProperties* );
         //virtual void OnDrawBackground( wxDC& dc, const wxRect& rect, int item, int flags ) const;
      private:
         layprop::LineMap _lines;
   };

   ///////////////////////////////////////////////////////////////////////////////
   /// Class TechEditorDialog
   ///////////////////////////////////////////////////////////////////////////////
   class TechEditorDialog : public wxDialog
   {
      public:
                                 TechEditorDialog( wxWindow* parent,wxWindowID id = wxID_ANY);
         virtual                ~TechEditorDialog();
         void                    onLayerSelected(wxListEvent&);
         void                    OnNewLayer(wxCommandEvent&);
         void                    OnColorEditor(wxCommandEvent&);
         void                    OnFillEditor(wxCommandEvent&);
         void                    OnStyleEditor(wxCommandEvent&);
         void                    OnChangeProperty(wxCommandEvent&);
         void                    OnChangeLayNum(wxCommandEvent&);
         void                    OnChangeLayType(wxCommandEvent&);
         void                    OnApply(wxCommandEvent&);
         void                    OnLayListSort(wxListEvent&);
         void                    OnRemotePropUpdate(wxCommandEvent&);
         class LayerListPanel : public wxListView
         {
            public:
               struct LayerLine {
                                 LayerLine() : _laydef(DEFAULT_LAY_NUMBER, DEFAULT_LAY_DATATYPE), _name("") {}
                                 LayerLine(LayerDef laydef, std::string name) :
                                    _laydef(laydef), _name(name) {}
                  LayerDef       _laydef;
                  std::string    _name;
               };
                                 LayerListPanel( wxWindow *parent, wxWindowID winid, long style) :
                                    wxListView(parent, winid, wxDefaultPosition, wxDefaultSize, style) {}
               unsigned long     getItemLayerNum(TmpWxIntPtr item1);
               unsigned long     getItemLayerTyp(TmpWxIntPtr item1);
               std::string       getItemLayerName(TmpWxIntPtr item1);
               void              addItemLayer(unsigned long, const LayerDef& laydef, std::string);
               void              clearAll();
            private:
               typedef std::map<TmpWxIntPtr,LayerLine> LayerItems;
               LayerItems        _layerItems;
         };
      private:
         void                    prepareLayers(layprop::DrawProperties*);
         void                    updateDialog();
         void                    updateLayerList();
         void                    checkNewLayer();
         LayerListPanel*         _layerList;
         ColorListComboBox*      _layerColors;
         FillListComboBox*       _layerFills;
         LineListComboBox*       _layerLines;

         wxTextCtrl*             _layerNumber;
         wxTextCtrl*             _layerDtype;
         wxTextCtrl*             _layerName;
         wxString                _layerNumberString;
         wxString                _layerDtypeString;
         wxString                _layerNameString;
         int                     _curSelect;//!Data related to current selection
         LayerDefList            _allLayNums;

         DECLARE_EVENT_TABLE()
   };

   int wxCALLBACK wxListCtrlItemCompare(TmpWxIntPtr, TmpWxIntPtr, TmpWxIntPtr);

}

#endif //__TECHEDITOR__
