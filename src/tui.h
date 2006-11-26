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
//   This file is a part of Toped project (C) 2001-2006 Toped developers    =
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
#include "../tpd_common/ttt.h"
#include "resourcecenter.h"
#include "../tpd_DB/viewprop.h"

namespace tui {
   
   enum {
      COLOR_COMBO = 100 ,
      FILL_COMBO        ,
      LINE_COMBO    //    ,
//      COLOR_DEFINE      ,
//      FILL_DEFINE       ,
//      LINE_DEFINE
   };
   
   class getSize : public wxDialog {
   public:
      getSize(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos );
      wxString value() const {return _wxText->GetValue();};
   private:   
      wxTextCtrl* _wxText;
   };

   class getStep : public wxDialog {
   public:
      getStep(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, 
                                                                     real init );
      wxString value() const {return _wxText->GetValue();};
   private:   
      wxTextCtrl* _wxText;
   };

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

   class getCellOpen : public wxDialog {
   public:
      getCellOpen(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
      wxString get_selectedcell() const {return _nameList->GetStringSelection();};
      wxListBox*  _nameList;
   };

   class getCellRef : public wxDialog {
   public:
      getCellRef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
      wxString get_selectedcell() const {return _nameList->GetStringSelection();};
      wxString get_angle() const {return _rotation->GetValue();};
      bool  get_flip() const {return _flip->GetValue();};
      wxTextCtrl* _rotation;
      wxCheckBox* _flip;
      wxListBox*  _nameList;
   };

   class getCellARef : public wxDialog {
   public:
      getCellARef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxString init);
      wxString get_selectedcell() const {return _nameList->GetStringSelection();};
      wxString get_angle() const {return _rotation->GetValue();};
      wxString get_col() const {return _numY->GetValue();};
      wxString get_row() const {return _numX->GetValue();};
      wxString get_stepX() const {return _stepX->GetValue();};
      wxString get_stepY() const {return _stepY->GetValue();};

      bool  get_flip() const {return _flip->GetValue();};
      wxTextCtrl* _rotation;
      wxTextCtrl* _numX;
      wxTextCtrl* _numY;
      wxTextCtrl* _stepX;
      wxTextCtrl* _stepY;
      wxCheckBox* _flip;
      wxListBox*  _nameList;
   };

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

   class getGDSimport : public wxDialog {
   public:
                     getGDSimport(wxFrame *parent, wxWindowID id, const wxString &title,
                                                                  wxPoint pos, wxString init);
      wxString       get_selectedcell() const {return _nameList->GetStringSelection();};
      bool           get_overwrite()    const {return _overwrite->GetValue();};
      bool           get_recursive()    const {return _recursive->GetValue();};
   private:
      wxCheckBox*    _overwrite;
      wxCheckBox*    _recursive;
      wxListBox*     _nameList;
   };

   class getGDSexport : public wxDialog {
   public:
                     getGDSexport(wxFrame *parent, wxWindowID id, const wxString &title,
                                                                  wxPoint pos, wxString init);
      wxString       get_selectedcell() const {return _nameList->GetStringSelection();};
      bool           get_recursive()    const {return _recursive->GetValue();};
   private:
      wxCheckBox*    _recursive;
      wxListBox*     _nameList;
   };

   class layset_sample : public wxWindow {
   public:
                     layset_sample(wxWindow*, wxWindowID, wxPoint, wxSize, word);
      void           setColor(word);
      void           setColor(const layprop::tellRGB*);
      void           setFill(word);
      void           setFill(const byte*);
      void           OnPaint(wxPaintEvent&);
   protected:
      wxColour       _color;
      wxBrush        _brush;
      DECLARE_EVENT_TABLE();
   };

   class defineLayer : public wxDialog {
   public:
                     defineLayer(wxFrame*, wxWindowID, const wxString&, wxPoint, word);
      void           OnColorChanged(wxCommandEvent&);
      void           OnFillChanged(wxCommandEvent&);
      void           OnLineChanged(wxCommandEvent&);
//      void           OnDefineColor(wxCommandEvent&);
//      void           OnDefineFill(wxCommandEvent&);
//      void           OnDefineLine(wxCommandEvent&);
   private:
      wxTextCtrl*    _layno;
      wxTextCtrl*    _layname;
      wxComboBox*    _colors;
      wxComboBox*    _fills;
      wxComboBox*    _lines;
      layset_sample* _sample;
      DECLARE_EVENT_TABLE();
   };
   
   class defineColor : public wxDialog {
   public:
                     defineColor(wxFrame *parent, wxWindowID id, const wxString &title,
                                                                  wxPoint pos, word init);
   private:
      layprop::tellRGB* _color;
   };
}
#endif

