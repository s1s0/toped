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
//        Created: Thu Jun 17 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Tell user interface classes
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include <math.h>
#include <sstream>
#include <wx/colordlg.h>
#include <wx/regex.h>
#include <wx/filename.h>
#include <wx/valgen.h>
#include "tui.h"
#include "tuidefs.h"
#include "datacenter.h"
#include "browsers.h"
#include "toped.h"
//#include "outbox.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern layprop::FontLibrary*     fontLib;
extern tui::TopedFrame*          Toped;

#if wxCHECK_VERSION(2, 8, 0)
#define tpdfOPEN wxFD_OPEN
#define tpdfSAVE wxFD_SAVE
#else
#define tpdfOPEN wxOPEN
#define tpdfSAVE wxSAVE
#endif

//==============================================================================
BEGIN_EVENT_TABLE(tui::sgSpinButton, wxSpinButton)
   EVT_SPIN(-1, tui::sgSpinButton::OnSpin)
END_EVENT_TABLE()

tui::sgSpinButton::sgSpinButton(wxWindow *parent, wxTextCtrl* textW, const float step,
   const float min, const float max, const float init, const int prec)
  : wxSpinButton(parent, -1, wxDefaultPosition, wxDefaultSize), _wxText(textW),
    _step(step), _prec(prec) {
   SetRange((int) rint(min / _step),(int) rint(max / _step));
   SetValue((int) rint(init / _step));
   wxString ws;
   ws.sprintf(wxT("%.*f"), _prec, init);
   _wxText->SetValue(ws);
}

void  tui::sgSpinButton::OnSpin(wxSpinEvent&) {
   wxString ws;
   ws.sprintf(wxT("%.*f"), _prec, GetValue() * _step);
   _wxText->SetValue(ws);
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::sgSliderControl, wxPanel)
   EVT_SCROLL(tui::sgSliderControl::OnScroll)
   EVT_TEXT_ENTER(wxID_ANY, sgSliderControl::OnTextEnter)
END_EVENT_TABLE()

tui::sgSliderControl::sgSliderControl(wxWindow *parent, int wId, int min, int max, int init)
  : wxPanel(parent, wId)
{
   wxBoxSizer *controlSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
   _slider = DEBUG_NEW wxSlider(this, wxID_ANY, init, min, max);
   wxString ws;
   ws.sprintf(wxT("%i"), init);
   _text = DEBUG_NEW wxTextCtrl(this, wxID_ANY, ws, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
   controlSizer->Add(_slider, 2, wxALL| wxALIGN_CENTER | wxEXPAND);
   controlSizer->Add(  _text, 1, wxALL| wxALIGN_CENTER | wxEXPAND);
   SetSizer(controlSizer);
   controlSizer->Fit(this);
//  _wxText->SetValue(ws);
}

void  tui::sgSliderControl::setValue(int value)
{
   _slider->SetValue(value);
   wxString ws;
   ws.sprintf(wxT("%i"), value);
   _text->SetValue(ws);
}

int  tui::sgSliderControl::getValue()
{
   return _slider->GetValue();
}

void  tui::sgSliderControl::OnScroll(wxScrollEvent& event)
{
   wxString ws;
   ws.sprintf(wxT("%i"), _slider->GetValue());
   _text->SetValue(ws);
   int eventID = event.GetEventType();
   if (wxEVT_SCROLL_CHANGED == eventID)
   {
      wxCommandEvent sliderEvent(wxEVT_COMMAND_ENTER, GetId());
      sliderEvent.SetInt(_slider->GetValue());
      wxPostEvent(GetParent(), sliderEvent);
   }
}

void tui::sgSliderControl::OnTextEnter(wxCommandEvent& WXUNUSED(event))
{
   wxString ws = _text->GetValue();
   long value;
   ws.ToLong(&value);
   _slider->SetValue(value);
   wxCommandEvent sliderEvent(wxEVT_COMMAND_ENTER, GetId());
   sliderEvent.SetInt(value);
   wxPostEvent(GetParent(), sliderEvent);
}

//==============================================================================
tui::getSize::getSize(wxFrame *parent, wxWindowID id, const wxString &title,
      wxPoint pos, real step, byte precision, const float init  ) : wxDialog(parent, id, title,
      pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
/*   int4b Istep = View.getstep();
   real DBscale = 1/DATC->UU();
   if (Istep > DBscale) precision = 0;
   else precision = fmod
   if (fmod(1/DATC->UU(), (float)View.getstep()) != 0)
      precision = */
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   _wxText = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   spin_sizer->Add(10,10,0);
   spin_sizer->Add(_wxText, 1, wxEXPAND, 0);
   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _wxText, step, 1, 10, init, precision), 0, 0, 0);
   spin_sizer->Add(10,10,0);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(10,10,0);
   topsizer->Add(spin_sizer, 0, wxEXPAND ); // no border and centre horizontally
   topsizer->Add(button_sizer, 0, wxEXPAND/*wxALIGN_CENTER*/ );
   SetSizer( topsizer );
   topsizer->SetSizeHints( this );
}

//==============================================================================
tui::getStep::getStep(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
   real init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                      wxDEFAULT_DIALOG_STYLE)  {
   wxString ws;
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   ws.sprintf(wxT("%.*f"), 3, init);
   _wxText = DEBUG_NEW wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_wxText, 0, wxEXPAND | wxALL ,10 );
   topsizer->Add(button_sizer, 0, wxEXPAND);
   SetSizer( topsizer );
   topsizer->SetSizeHints( this );
}

//==============================================================================
tui::getGrid::getGrid(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
   real ig0, real ig1, real ig2 ) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                      wxDEFAULT_DIALOG_STYLE)  {
   wxString ws;
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //grid 0
   wxBoxSizer *g0sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   ws.sprintf(wxT("%.*f"), 3, ig0);
   _wxGrid0 = DEBUG_NEW wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   g0sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Grid 0:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL, 5);
   g0sizer->Add(_wxGrid0, 0, wxEXPAND | wxALL ,5 );
   //grid 1
   wxBoxSizer *g1sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   ws.sprintf(wxT("%.*f"), 3, ig1);
   _wxGrid1 = DEBUG_NEW wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   g1sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Grid 1:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL, 5);
   g1sizer->Add(_wxGrid1, 0, wxEXPAND | wxALL ,5 );
   //grid 2
   wxBoxSizer *g2sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   ws.sprintf(wxT("%.*f"), 3, ig2);
   _wxGrid2 = DEBUG_NEW wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   g2sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Grid 2:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL, 5);
   g2sizer->Add(_wxGrid2, 0, wxEXPAND | wxALL ,5 );
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(g0sizer, 0, wxEXPAND);
   topsizer->Add(g1sizer, 0, wxEXPAND);
   topsizer->Add(g2sizer, 0, wxEXPAND);
   topsizer->Add(button_sizer, 0, wxEXPAND);
   SetSizer( topsizer );
   topsizer->SetSizeHints( this );
}

//==============================================================================
tui::getCellOpen::getCellOpen(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));

   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::CellMap const cll = tDesign->cells();
      laydata::CellMap::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   }
   DATC->unlockTDT(dbLibDir);
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getCellRef::getCellRef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
//   _rotation = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
//   _flip = DEBUG_NEW wxCheckBox(this, -1, wxT("Flip X"));
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_liblock))
   {
      laydata::LibCellLists *cll = dbLibDir->getCells(ALL_LIB);
      for (laydata::LibCellLists::iterator curlib = cll->begin(); curlib != cll->end(); curlib++)
      {
         laydata::CellMap::const_iterator CL;
         for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
         {
            _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
         }
      }
      delete cll;
   }
   DATC->unlockTDT(dbLibDir);
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
//   wxBoxSizer *spin_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
//   spin_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Rotation:"),
//                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
//                                                0, wxALL | wxALIGN_CENTER, 10);
//   spin_sizer->Add(_rotation, 0, wxALL | wxALIGN_CENTER, 0);
//   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _rotation, 90, 0, 360, 0, 0),
//                                                  0, wxALL | wxALIGN_CENTER, 0);
//   spin_sizer->Add(0,0,1); //
//   spin_sizer->Add(_flip, 0, wxALL | wxALIGN_CENTER, 10);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
      topsizer->Add(_nameList, 1, wxEXPAND );
//   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

tui::getCellARef::getCellARef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
//   _rotation = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
//   _flip = DEBUG_NEW wxCheckBox(this, -1, wxT("Flip X"));
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_liblock))
   {
      laydata::LibCellLists *cll = dbLibDir->getCells(ALL_LIB);
      for (laydata::LibCellLists::iterator curlib = cll->begin(); curlib != cll->end(); curlib++)
      {
         laydata::CellMap::const_iterator CL;
         for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
         {
            _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
         }
      }
      delete cll;
   }
   DATC->unlockTDT(dbLibDir);
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
//   wxBoxSizer *spin_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
//   spin_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Rotation:"),
//                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
//                                                0, wxALL | wxALIGN_CENTER, 10);
//   spin_sizer->Add(_rotation, 0, wxALL | wxALIGN_CENTER, 0);
//   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _rotation, 90, 0, 360, 0, 0),
//                                                  0, wxALL | wxALIGN_CENTER, 0);
//   spin_sizer->Add(0,0,1); //
//   spin_sizer->Add(_flip, 0, wxALL | wxALIGN_CENTER, 10);
   // Now Array specific controls
   _numX = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _numY = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   wxBoxSizer* num_sizer =  DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   num_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Rows:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   num_sizer->Add(_numX, 0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(DEBUG_NEW sgSpinButton(this, _numX, 1, 2, 200, 2, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(0,0,1); //
   num_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Columns:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   num_sizer->Add(_numY, 0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(DEBUG_NEW sgSpinButton(this, _numY, 1, 2, 200, 2, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(0,0,1); //
   //
   _stepX = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _stepY = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   wxBoxSizer* step_sizer =  DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   step_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("step X:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   step_sizer->Add(_stepX, 0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(DEBUG_NEW sgSpinButton(this, _stepX, PROPC->step(), 2, 200, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(0,0,1); //
   step_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("step Y:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   step_sizer->Add(_stepY, 0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(DEBUG_NEW sgSpinButton(this, _stepY, PROPC->step(), 2, 200, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);

   step_sizer->Add(0,0,1); //
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
      topsizer->Add(_nameList, 1, wxEXPAND );
//   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(num_sizer, 0, wxEXPAND );
   topsizer->Add(step_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

tui::getTextdlg::getTextdlg(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos)
           : wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)  {

   _size     = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
//   _rotation = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
//   _flip = DEBUG_NEW wxCheckBox(this, -1, wxT("Flip X"));
   _text = DEBUG_NEW wxTextCtrl(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );

   spin_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Size:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   spin_sizer->Add(_size, 0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _size, PROPC->step(), 1, 20, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(0,0,1); //

//   spin_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Rotation:"),
//                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
//                                                0, wxALL | wxALIGN_CENTER, 10);
//   spin_sizer->Add(_rotation, 0, wxALL | wxALIGN_CENTER, 0);
//   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _rotation, 90, 0, 360, 0, 0),
//                                                  0, wxALL | wxALIGN_CENTER, 0);
//   spin_sizer->Add(0,0,1); //
//   spin_sizer->Add(_flip, 0, wxALL | wxALIGN_CENTER, 10);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_text, 1, wxEXPAND | wxALIGN_CENTER, 10 );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
   _text->SetFocus();
}


//==============================================================================
tui::getLibList::getLibList(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
                              wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_liblock))
   {
      for (int curlib = 1; curlib < dbLibDir->getLastLibRefNo(); curlib++)
      {
         _nameList->Append(wxString(dbLibDir->getLibName(curlib).c_str(), wxConvUTF8));
      }
   }
   DATC->unlockTDT(dbLibDir);

   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getCIFimport::getCIFimport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init, const layprop::DrawProperties* drawProp) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   _overwrite = DEBUG_NEW wxCheckBox(this, -1, wxT("Overwrite existing cells"));
   _overwrite->SetValue(true);
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Import recursively"));
   _recursive->SetValue(true);
   _saveMap = DEBUG_NEW wxCheckBox(this, -1, wxT("Save Layer Map"));
   _techno = wxT("1.0");
   wxTextCtrl* dwtechno = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
         wxTextValidator(wxFILTER_NUMERIC, &_techno));

   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300), 0, NULL, wxLB_SORT);
   NameList cifLayers;
   ForeignDbFile* ACIFDB = NULL;
   if (DATC->lockCif(ACIFDB))
   {
      ACIFDB->getAllCells(*_nameList);
   }
   DATC->unlockCif(ACIFDB, true);
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   //-----------------------------------------------------------------------
   SIMap inlays;
   if (DATC->cifGetLayers(cifLayers))
   {
      word laynum = 1;
      for (NameList::iterator NLI = cifLayers.begin(); NLI != cifLayers.end(); NLI++)
      {
         inlays[*NLI] = laynum++;
      }
   }

   _layList = DEBUG_NEW tui::nameCboxList(this, wxID_ANY, wxDefaultPosition, wxSize(290,300), inlays, drawProp);

   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //
   wxBoxSizer *lsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );

   wxBoxSizer *lsizer2 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lsizer2->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Techno:"), wxDefaultPosition, wxDefaultSize),
                 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
   lsizer2->Add(dwtechno, 1, wxEXPAND);
   lsizer2->Add( _saveMap, 0, wxALL | wxALIGN_RIGHT, 5 );
   lsizer->Add(_layList , 1, wxEXPAND);
   lsizer->Add( lsizer2, 0, wxALL , 5 );
   //
   // First line up the important things
   wxBoxSizer *lists_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lists_sizer->Add(_nameList, 1, wxEXPAND );
   lists_sizer->Add(lsizer, 0, wxEXPAND);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(_overwrite, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(lists_sizer, 1, wxEXPAND | wxALIGN_CENTER );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getCIFexport::getCIFexport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init, const layprop::DrawProperties* drawProp) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Export recursively"));
   _recursive->SetValue(true);
   _saveMap = DEBUG_NEW wxCheckBox(this, -1, wxT("Save Layer Map"));
   _slang = DEBUG_NEW wxCheckBox(this, -1, wxT("Verbose CIF slang"));
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   WordList ull;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::CellMap const cll = tDesign->cells();
      laydata::CellMap::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
      //-----------------------------------------------------------------------
      dbLibDir->collectUsedLays(TARGETDB_LIB, ull);
   }
   DATC->unlockTDT(dbLibDir);
   if (init != wxT("")) _nameList->SetStringSelection(init,true);

   _layList = DEBUG_NEW tui::nameEboxList(this, wxID_ANY, wxDefaultPosition, wxSize(290,300), ull, drawProp);

   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //
   wxBoxSizer *lsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   lsizer->Add(_layList , 1, wxEXPAND);
   lsizer->Add( _saveMap, 0, wxALL | wxALIGN_RIGHT, 5 );
   //
   // First line up the important things
   wxBoxSizer *lists_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lists_sizer->Add(_nameList, 1, wxEXPAND );
   lists_sizer->Add(lsizer, 0, wxEXPAND);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(_slang    , 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(lists_sizer, 1, wxEXPAND | wxALIGN_CENTER );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getGDSimport::getGDSimport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init, const layprop::DrawProperties* drawProp) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   _overwrite = DEBUG_NEW wxCheckBox(this, -1, wxT("Overwrite existing cells"));
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Import recursively"));
   _saveMap = DEBUG_NEW wxCheckBox(this, -1, wxT("Save Layer Map"));
   _recursive->SetValue(true);
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300), 0, NULL, wxLB_SORT);
   ExtLayers gdsLayers;
   ForeignDbFile* AGDSDB = NULL;
   if (DATC->lockGds(AGDSDB))
   {
      AGDSDB->getAllCells(*_nameList);
   }
   DATC->unlockGds(AGDSDB, true);
   DATC->gdsGetLayers(gdsLayers);
   if (init != wxT(""))
   {
      _nameList->SetStringSelection(init,true);
      _nameList->SetFirstItem(init);
   }

   _layList = DEBUG_NEW tui::nameCbox3List(this, wxID_ANY, wxDefaultPosition, wxSize(300,300),
         gdsLayers, drawProp, wxT("GDS layer/type"));

   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //
   wxBoxSizer *lsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   lsizer->Add(_layList , 1, wxEXPAND);
   lsizer->Add( _saveMap, 0, wxALL | wxALIGN_RIGHT, 5 );
   //
   wxBoxSizer *lists_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lists_sizer->Add(_nameList, 1, wxEXPAND );
   lists_sizer->Add(lsizer, 0, wxEXPAND);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(_overwrite, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(lists_sizer, 1, wxEXPAND | wxALIGN_CENTER );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getOASimport::getOASimport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init, const layprop::DrawProperties* drawProp) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   _overwrite = DEBUG_NEW wxCheckBox(this, -1, wxT("Overwrite existing cells"));
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Import recursively"));
   _saveMap = DEBUG_NEW wxCheckBox(this, -1, wxT("Save Layer Map"));
   _recursive->SetValue(true);
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300), 0, NULL, wxLB_SORT);
   ExtLayers oasLayers;
   ForeignDbFile* AOASDB = NULL;
   if (DATC->lockOas(AOASDB))
   {
      AOASDB->getAllCells(*_nameList);
   }
   DATC->unlockOas(AOASDB, true);
   DATC->oasGetLayers(oasLayers);
   if (init != wxT(""))
   {
      _nameList->SetStringSelection(init,true);
      _nameList->SetFirstItem(init);
   }

   _layList = DEBUG_NEW tui::nameCbox3List(this, wxID_ANY, wxDefaultPosition, wxSize(300,300),
         oasLayers, drawProp, wxT("OAS layer/type"));

   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //
   wxBoxSizer *lsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   lsizer->Add(_layList , 1, wxEXPAND);
   lsizer->Add( _saveMap, 0, wxALL | wxALIGN_RIGHT, 5 );
   //
   wxBoxSizer *lists_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lists_sizer->Add(_nameList, 1, wxEXPAND );
   lists_sizer->Add(lsizer, 0, wxEXPAND);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(_overwrite, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(lists_sizer, 1, wxEXPAND | wxALIGN_CENTER );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getGDSexport::getGDSexport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init, const layprop::DrawProperties* drawProp) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Export recursively"));
   _recursive->SetValue(true);
   _saveMap = DEBUG_NEW wxCheckBox(this, -1, wxT("Save Layer Map"));
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   WordList ull;
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_dblock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      laydata::CellMap const cll = tDesign->cells();
      laydata::CellMap::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
      //-----------------------------------------------------------------------

      dbLibDir->collectUsedLays(TARGETDB_LIB, ull);
   }
   DATC->unlockTDT(dbLibDir);
   if (init != wxT("")) _nameList->SetStringSelection(init,true);

   _layList = DEBUG_NEW tui::nameEbox3List(this, wxID_ANY, wxDefaultPosition, wxSize(270,300), ull, drawProp);

   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //
   wxBoxSizer *lsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   lsizer->Add(_layList, 1, wxEXPAND);
   lsizer->Add( _saveMap, 0, wxALL | wxALIGN_RIGHT, 5 );
   //
   wxBoxSizer *lists_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lists_sizer->Add(_nameList, 1, wxEXPAND );
   lists_sizer->Add( lsizer  , 0, wxEXPAND );
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(lists_sizer, 1, wxEXPAND | wxALIGN_CENTER );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::layset_sample, wxWindow)
   EVT_PAINT(tui::layset_sample::OnPaint)
END_EVENT_TABLE()

/*! This control has to draw a simple box using the selected layer properties -
  color, fill, line style using wx drawing engine. It appears not
  straight forward to mimic the openGL properties with wxPaintDC.
 - with color - the transparency (alpha channel) is missing and I'm not sure
   is it possible to do something about this.
 - fill - this seem to be relatively trivial using wxBitmap. That's of course
   because I had already similar code from Sergey.
 - line - here is the fun. It appears that stipple pen is not implemented
   (wxGTK 2.6.3), so bitmaps can't be used. So I had to implement manually
   the openGL line pattern using wxDash concept, that is not documented. It
   appears to have some strange "default" behavior - the dash size is scaled
   with the line width. To avoid this, one have to draw with line width 1 multiple
   times. The speed is not an issue here - so the only remaining thing is the
   strange looking code.
*/
tui::layset_sample::layset_sample(wxWindow *parent, wxWindowID id, wxPoint pos,
   wxSize size, word init, const layprop::DrawProperties* drawProp) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   if ((drawProp != NULL) && (LAST_EDITABLE_LAYNUM > init))
   {
      setColor(drawProp->getColor(init));
      setFill(drawProp->getFill(init));
      setLine(drawProp->getLine(init));
   }
   else
   {
      _color.Set(0,0,0);
      _brush = wxBrush();
      _pen = wxPen();
   }
   _selected = false;
}

void tui::layset_sample::setColor(const layprop::tellRGB& col)
{
   _color.Set(col.red(), col.green(), col.blue());
}

void tui::layset_sample::setFill(const byte* fill)
{
   if (NULL != fill)
   {
      wxBitmap stipplebrush((char  *)fill, 32, 32, 1);

#ifdef WIN32
      wxImage image(32, 32, (unsigned char  *)fill, true);
      image = stipplebrush.ConvertToImage();;
      stipplebrush = wxBitmap(image, 1);
      image = stipplebrush.ConvertToImage();
      int w = image.GetWidth();
      int h = image.GetHeight();

      for (int i=0; i<w; i++)
         for (int j=0; j<h; j++)
         {
            if((image.GetRed(i,j)==0)&& (image.GetGreen(i,j)==0) && (image.GetBlue(i,j)==0))
            {

               image.SetRGB(i, j, _color.Red(), _color.Green(), _color.Blue());
            }
            else
            {
               image.SetRGB(i, j, 0, 0, 0);
            }
         }

      //Recreate bitmap with new color
      stipplebrush = wxBitmap(image, 1);

#endif
      _brush = wxBrush(stipplebrush);
   }
   else
   {
      _brush = wxBrush();
   }
}

void tui::layset_sample::setLine(const layprop::LineSettings* line)
{
   _dashes.clear();
   if (NULL == line)
   {
      _pen = wxPen();
      _linew = 1;
   }
   else if (0xffff == line->pattern())
   {
      _pen = wxPen(_color);
      _linew = line->width();
   }
   else
   {
      //_pen.SetStipple(stipplepen); <- not implemented in wxGTK??
      _linew = line->width();
      _pen = wxPen(_color, 1, wxUSER_DASH);
      word pattern = line->pattern();
      bool current_pen = ((pattern & 0x0001) > 0);
      byte pixels = 0;
      for( byte i = 0; i < 16; i++)
      {
         word mask = 0x0001 << i;
         if (((pattern & mask) > 0) ^ current_pen)
         {
            _dashes.push_back(pixels * line->patscale());
            current_pen = (0 != (pattern & mask));
            pixels = 1;
         }
         else
            pixels++;
      }
      if (_dashes.size() % 2)
         _dashes.push_back(pixels * line->patscale());
      else
         _dashes[0] += pixels * line->patscale();
   }
}

void tui::layset_sample::drawOutline(wxPaintDC& dc, wxCoord w, wxCoord h)
{
   _pen.SetColour(_color);
   if (_dashes.size() > 0)
   {
      wxDash dash1[16];
      for( word i = 0; i < _dashes.size(); i++)
      {
         dash1[i] = _dashes[i];
      }
      _pen.SetDashes(_dashes.size(),dash1);
      _pen.SetCap(wxCAP_BUTT);
   }
   dc.SetPen(_pen);
   for (word i = 1; i <= _linew; i++)
   {
      dc.DrawLine(1  , i  , w  , i  );
      dc.DrawLine(w-i, 1  , w-i, h  );
      dc.DrawLine(w  , h-i, 1  , h-i);
      dc.DrawLine(i  , h  , i  , 1  );
   }
}

void tui::layset_sample::OnPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.SetBackground(*wxBLACK);
   _brush.SetColour(_color);

   dc.SetBrush(_brush);
   dc.Clear();
   wxCoord w, h;
   dc.GetSize(&w, &h);
   if (_selected)
   {
      dc.DrawRectangle(3, 3, w-6, h-6);
      drawOutline(dc, w, h);
   }
   else
   {
      wxPen lclPen(_color);
      dc.SetPen(lclPen);
      dc.DrawRectangle(2, 2, w-4, h-4);
   }
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::defineLayer, wxDialog)
   EVT_COMBOBOX(COLOR_COMBO      , tui::defineLayer::OnColorChanged   )
   EVT_COMBOBOX(FILL_COMBO       , tui::defineLayer::OnFillChanged    )
   EVT_COMBOBOX(LINE_COMBO       , tui::defineLayer::OnLineChanged    )
   EVT_CHECKBOX(DRAW_SELECTED    , tui::defineLayer::OnSelectedChanged)
   EVT_CHECKBOX(ID_CBDEFCOLOR    , tui::defineLayer::OnDefaultColor   )
   EVT_CHECKBOX(ID_CBDEFPATTERN  , tui::defineLayer::OnDefaultPattern )
   EVT_CHECKBOX(ID_CBDEFLINE     , tui::defineLayer::OnDefaultLine    )
END_EVENT_TABLE()

tui::defineLayer::defineLayer(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
   word init, const layprop::DrawProperties* drawProp) : wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
   _drawProp(drawProp)
{
   wxString init_color = wxT("");
   wxString init_fill = wxT("");
   wxString init_line = wxT("");
   if (init > 0)
   {
      _layno << init;
      _layname   = wxString(_drawProp->getLayerName(init).c_str(), wxConvUTF8);
      init_color = wxString(_drawProp->getColorName(init).c_str(), wxConvUTF8);
      init_fill  = wxString(_drawProp->getFillName(init).c_str() , wxConvUTF8);
      init_line  = wxString(_drawProp->getLineName(init).c_str() , wxConvUTF8);
   }
   bool no_color = (wxT("") == init_color);
   bool no_fill  = (wxT("") == init_fill);
   bool no_line  = (wxT("") == init_line);
   wxTextCtrl* dwlayno    = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_layno));
   wxTextCtrl* dwlayname  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0,
                                          wxTextValidator(wxFILTER_ASCII, &_layname));

   /*if (init > 0)
   {
      dwlayno->SetEditable(false);
   }*/
   _sample   = DEBUG_NEW layset_sample( this, -1, wxDefaultPosition, wxDefaultSize, init, _drawProp);
   NameList all_names;
   wxArrayString all_strings;
   _drawProp->allColors(all_names);
   if (!all_names.empty())
   {
      for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
         all_strings.Add(wxString(CI->c_str(), wxConvUTF8));
      if (no_color)
         init_color = wxString(all_names.begin()->c_str(), wxConvUTF8);
      _colors   = DEBUG_NEW wxComboBox( this, COLOR_COMBO, init_color, wxDefaultPosition, wxDefaultSize,all_strings, wxCB_READONLY | wxCB_SORT);
      _colors->SetStringSelection(init_color);
   }
   else
   {
      _colors   = DEBUG_NEW wxComboBox( this, COLOR_COMBO, wxT("") , wxDefaultPosition, wxDefaultSize);
   }

   all_names.clear();
   all_strings.Clear();
   _drawProp->allFills(all_names);
   if (!all_names.empty())
   {
      for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
         all_strings.Add(wxString(CI->c_str(), wxConvUTF8));
      if (no_fill)
         init_fill = wxString(all_names.begin()->c_str(), wxConvUTF8);
      _fills   = DEBUG_NEW wxComboBox( this, FILL_COMBO, init_fill, wxDefaultPosition, wxDefaultSize,all_strings,wxCB_READONLY | wxCB_SORT);
      _fills->SetStringSelection(init_fill);
   }
   else
   {
      _fills   = DEBUG_NEW wxComboBox( this, FILL_COMBO, wxT(""), wxDefaultPosition, wxDefaultSize);
   }

   all_names.clear();
   all_strings.Clear();
   _drawProp->allLines(all_names);
   if (!all_names.empty())
   {
      for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
      {
         all_strings.Add(wxString(CI->c_str(), wxConvUTF8));
      }
      if (no_line)
         init_line = wxString(all_names.begin()->c_str(), wxConvUTF8);
      _lines   = DEBUG_NEW wxComboBox( this, LINE_COMBO, init_line, wxDefaultPosition, wxDefaultSize, all_strings,wxCB_READONLY | wxCB_SORT);
      _lines->SetStringSelection(init_line);
   }
   else
   {
      _lines   = DEBUG_NEW wxComboBox( this, LINE_COMBO, wxT(""), wxDefaultPosition, wxDefaultSize);
   }

   // The window layout
   // NOTE! Static boxes MUST be created before all other controls which are about to
   // be encircled by them. Otherwise the dialog box might work somewhere (Windows & fc8)
   // but not everywhere! (fc9)
   wxBoxSizer *color_sizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Color"));
   wxBoxSizer *fill_sizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Pattern"));
   wxBoxSizer *line_sizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Selected Line"));
   wxBoxSizer *col3_sizer = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Sample") );


   wxBoxSizer *line1_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   line1_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Number:"), wxDefaultPosition, wxDefaultSize),
                                                0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
   line1_sizer->Add(dwlayno, 1, wxRIGHT | wxALIGN_CENTER, 5);
   line1_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Name:"),  wxDefaultPosition, wxDefaultSize),
                                                0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
   line1_sizer->Add(dwlayname, 2, wxRIGHT | wxALIGN_CENTER, 5);
   //
   color_sizer->Add(_colors, 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   color_sizer->Add(DEBUG_NEW wxCheckBox(this, ID_CBDEFCOLOR, wxT("default")), 0, wxALL | wxALIGN_RIGHT, 5);
   if (no_color)
   {
      static_cast<wxCheckBox*>(FindWindow(ID_CBDEFCOLOR))->SetValue(true);
      _colors->Enable(false);
   }
   fill_sizer->Add(_fills , 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   fill_sizer->Add(DEBUG_NEW wxCheckBox(this, ID_CBDEFPATTERN, wxT("default")), 0, wxALL | wxALIGN_RIGHT, 5);
   if (no_fill)
   {
      static_cast<wxCheckBox*>(FindWindow(ID_CBDEFPATTERN))->SetValue(true);
      _fills->Enable(false);
   }
   line_sizer->Add(_lines , 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   line_sizer->Add(DEBUG_NEW wxCheckBox(this, ID_CBDEFLINE, wxT("default")), 0, wxALL | wxALIGN_RIGHT, 5);
   if (no_line)
   {
      static_cast<wxCheckBox*>(FindWindow(ID_CBDEFLINE))->SetValue(true);
      _lines->Enable(false);
   }

   wxBoxSizer *col2_sizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   col2_sizer->Add(color_sizer, 0, wxEXPAND);
   col2_sizer->Add(fill_sizer , 0, wxEXPAND);
   col2_sizer->Add(line_sizer , 0, wxEXPAND);

   _selected = DEBUG_NEW wxCheckBox(this, DRAW_SELECTED, wxT("selected"));
   col3_sizer->Add( _sample  , 1, wxEXPAND);
   col3_sizer->Add(_selected , 0, wxALL | wxALIGN_LEFT | wxEXPAND, 5);

   wxBoxSizer *line2_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   line2_sizer->Add(col2_sizer, 3, wxEXPAND | wxALL, 5);
   line2_sizer->Add(col3_sizer, 1, wxEXPAND | wxALL, 5);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   topsizer->Add(line1_sizer , 0, wxEXPAND | wxTOP, 5 );
   topsizer->Add(line2_sizer , 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

void tui::defineLayer::OnColorChanged(wxCommandEvent& cmdevent)
{
   wxString color_name = cmdevent.GetString();
   const layprop::tellRGB color = _drawProp->getColor(std::string(color_name.mb_str(wxConvUTF8)));
   _sample->setColor(color);

   //Next 2 strings need for Windows version
   const byte* fill = _drawProp->getFill(std::string(_fillname.mb_str(wxConvUTF8)));
   _sample->setFill(fill);

   _sample->Refresh();
}

void tui::defineLayer::OnFillChanged(wxCommandEvent& cmdevent)
{
   _fillname = cmdevent.GetString();
   const byte* fill = _drawProp->getFill(std::string(_fillname.mb_str(wxConvUTF8)));
   _sample->setFill(fill);
   _sample->Refresh();
}

void tui::defineLayer::OnLineChanged(wxCommandEvent& cmdevent)
{
   wxString line_name = cmdevent.GetString();
   const layprop::LineSettings* line = _drawProp->getLine(std::string(line_name.mb_str(wxConvUTF8)));
   _sample->setLine(line);
   _sample->Refresh();
}

void tui::defineLayer::OnSelectedChanged(wxCommandEvent& cmdevent)
{
   bool selected = (0 != cmdevent.GetInt());
   _sample->setSelected(selected);
   _sample->Refresh();
}
void tui::defineLayer::OnDefaultColor(wxCommandEvent& cmdevent)
{
   bool selected = (0 != cmdevent.GetInt());
   _colors->Enable(!selected);
   if (selected)
      _sample->setColor(_drawProp->getColor(std::string("")));
   else
      _sample->setColor(_drawProp->getColor(std::string(_colors->GetStringSelection().mb_str(wxConvUTF8))));
   _sample->Refresh();
}

void tui::defineLayer::OnDefaultPattern(wxCommandEvent& cmdevent)
{
   bool selected = (0 != cmdevent.GetInt());
   _fills->Enable(!selected);
   const byte* fill;
   if (selected)
      fill = _drawProp->getFill(std::string(""));
   else
      fill = _drawProp->getFill(std::string(_fills->GetStringSelection().mb_str(wxConvUTF8)));
   _sample->setFill(fill);
   _sample->Refresh();
}

void tui::defineLayer::OnDefaultLine(wxCommandEvent& cmdevent)
{
   bool selected = (0 != cmdevent.GetInt());
   _lines->Enable(!selected);
   const layprop::LineSettings* line;
   if (selected)
      line = _drawProp->getLine(std::string(""));
   else
      line = _drawProp->getLine(std::string(_lines->GetStringSelection().mb_str(wxConvUTF8)));
   _sample->setLine(line);
   _sample->Refresh();
}

wxString tui::defineLayer::color()
{
   if (static_cast<wxCheckBox*>(FindWindow(ID_CBDEFCOLOR))->IsChecked())
      return wxT("");
   else
      return _colors->GetValue();
}

wxString tui::defineLayer::fill()
{
   if (static_cast<wxCheckBox*>(FindWindow(ID_CBDEFPATTERN))->IsChecked())
      return wxT("");
   else
      return _fills->GetValue();
}

wxString tui::defineLayer::line()
{
   if (static_cast<wxCheckBox*>(FindWindow(ID_CBDEFLINE))->IsChecked())
      return wxT("");
   else
      return _lines->GetValue();
}

tui::defineLayer::~defineLayer()
{
//   delete _colors;
//   delete _fills;
//   delete _lines;
//   delete _sample;
//   delete _selected;
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::color_sample, wxWindow)
   EVT_PAINT(tui::color_sample::OnPaint)
END_EVENT_TABLE()

tui::color_sample::color_sample(wxWindow *parent, wxWindowID id, wxPoint pos,
   wxSize size, std::string init, const layprop::DrawProperties* drawProp) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   setColor(drawProp->getColor(init));
}

void tui::color_sample::setColor(const layprop::tellRGB& col)
{
   _color.Set(col.red(), col.green(), col.blue());
}

void tui::color_sample::OnPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.SetBackground(*wxBLACK);
   wxBrush _brush(_color);

   dc.SetBrush(_brush);
   dc.Clear();
   wxCoord w, h;
   dc.GetSize(&w, &h);
   dc.DrawRectangle(0, 0, w, h);
}
//==============================================================================
BEGIN_EVENT_TABLE(tui::defineColor, wxDialog)
   EVT_LISTBOX(ID_ITEMLIST , tui::defineColor::OnColorSelected    )
   EVT_BUTTON(ID_BTNEDIT   , tui::defineColor::OnDefineColor      )
   EVT_BUTTON(ID_NEWITEM   , tui::defineColor::OnColorNameAdded   )
   EVT_BUTTON(ID_BTNAPPLY  , tui::defineColor::OnApply            )
   EVT_TEXT(ID_REDVAL      , tui::defineColor::OnColorPropChanged )
   EVT_TEXT(ID_GREENVAL    , tui::defineColor::OnColorPropChanged )
   EVT_TEXT(ID_BLUEVAL     , tui::defineColor::OnColorPropChanged )
   EVT_TEXT(ID_ALPHAVAL    , tui::defineColor::OnColorPropChanged )
END_EVENT_TABLE()

tui::defineColor::defineColor(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, const layprop::DrawProperties* drawProp) :
      wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), _drawProp(drawProp)
{
   std::string init_color;
   NameList all_names;
   _colorList = DEBUG_NEW wxListBox(this, ID_ITEMLIST, wxDefaultPosition, wxSize(150,200), 0, NULL, wxLB_SORT);
   _drawProp->allColors(all_names);
   if (!all_names.empty())
   {
      init_color = *(all_names.begin());
   }

   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      _colorList->Append(wxString(CI->c_str(), wxConvUTF8));
      _allColors[*CI] = DEBUG_NEW layprop::tellRGB(_drawProp->getColor(*CI));
   }
   // NOTE! Static boxes MUST be created before all other controls which are about to
   // be encircled by them. Otherwise the dialog box might work somewhere (Windows & fc8)
   // but not everywhere! (fc9)
   wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("New Color") );
   wxBoxSizer *vsizer3 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Edit Color") );

   _dwcolname  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxSize(150,-1), 0,
                                          wxTextValidator(wxFILTER_ASCII, &_colname));
   _colorsample = DEBUG_NEW color_sample( this, -1, wxDefaultPosition, wxSize(-1,50), init_color, _drawProp);

   _c_red    = DEBUG_NEW wxTextCtrl( this, ID_REDVAL , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_red));
   _c_green  = DEBUG_NEW wxTextCtrl( this, ID_GREENVAL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_green));
   _c_blue   = DEBUG_NEW wxTextCtrl( this, ID_BLUEVAL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_blue));
   _c_alpha  = DEBUG_NEW wxTextCtrl( this, ID_ALPHAVAL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_alpha));

   hsizer0->Add( _dwcolname   , 0, wxALL | wxEXPAND, 5);
   hsizer0->Add(0,0,1); //
   hsizer0->Add( DEBUG_NEW wxButton( this, ID_NEWITEM  , wxT("Add")    ), 0, wxALL, 5 );

   wxBoxSizer *hsizer1 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer1->Add( DEBUG_NEW wxStaticText(this, -1, wxT("R:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                1, wxALL | wxALIGN_RIGHT, 5);
   hsizer1->Add( _c_red   , 0, wxALL | wxEXPAND, 5);
   wxBoxSizer *hsizer2 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer2->Add( DEBUG_NEW wxStaticText(this, -1, wxT("G:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                1, wxALL | wxALIGN_RIGHT, 5);
   hsizer2->Add( _c_green   , 0, wxALL | wxEXPAND, 5);
   wxBoxSizer *hsizer3 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer3->Add( DEBUG_NEW wxStaticText(this, -1, wxT("B:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                1, wxALL | wxALIGN_RIGHT, 5);
   hsizer3->Add( _c_blue   , 0, wxALL | wxEXPAND, 5);
   wxBoxSizer *hsizer4 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer4->Add( DEBUG_NEW wxStaticText(this, -1, wxT("A:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                1, wxALL | wxALIGN_RIGHT, 5);
   hsizer4->Add( _c_alpha   , 0, wxALL | wxEXPAND, 5);

   wxBoxSizer *hsizer5 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer5->Add(DEBUG_NEW wxButton( this, ID_BTNAPPLY , wxT(" Apply ") , wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 0, wxALL | wxALIGN_RIGHT, 5);
   FindWindow(ID_BTNAPPLY)->Enable(false);

   hsizer5->Add(DEBUG_NEW wxButton( this, ID_BTNEDIT  , wxT(" Define "), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 0, wxALL | wxALIGN_RIGHT, 5);

   wxBoxSizer *vsizer2 = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   vsizer2->Add( _colorsample , 0, wxALL | wxEXPAND, 5);
   vsizer2->Add( hsizer1   , 0, wxEXPAND);
   vsizer2->Add( hsizer2   , 0, wxEXPAND);
   vsizer2->Add( hsizer3   , 0, wxEXPAND);
   vsizer2->Add( hsizer4   , 0, wxEXPAND);
   vsizer2->Add( hsizer5   , 0, wxEXPAND);

   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK    , wxT("OK") ), 0, wxALL, 5 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel")  ), 0, wxALL, 5 );

   vsizer3->Add( _colorList   , 0, wxALL | wxEXPAND, 5);
   vsizer3->Add(0,0,1); //
   vsizer3->Add( vsizer2      , 0, wxEXPAND );

   wxBoxSizer *top_sizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   top_sizer->Add( hsizer0      , 0, wxEXPAND );
   top_sizer->Add( vsizer3      , 0, wxEXPAND );
   top_sizer->Add( button_sizer , 0, wxEXPAND );

   SetSizer( top_sizer );      // use the sizer for layout

   top_sizer->SetSizeHints( this );   // set size hints to honour minimum size

}

void tui::defineColor::OnDefineColor(wxCommandEvent& cmdevent)
{
   NameList all_names;
   wxColourData data;
   _drawProp->allColors(all_names);
   word colnum = 0;
   const layprop::tellRGB* tell_color;
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      tell_color= getColor(*CI);
      wxColour colour(tell_color->red(), tell_color->green(), tell_color->blue());
      if (16 == colnum)
         break;
      else
         data.SetCustomColour(colnum++, colour);
   }
   wxString s_red   =   _c_red->GetValue();
   wxString s_green = _c_green->GetValue();
   wxString s_blue  =  _c_blue->GetValue();
   unsigned long d_red;    s_red.ToULong(&d_red);
   unsigned long d_green;s_green.ToULong(&d_green);
   unsigned long d_blue;  s_blue.ToULong(&d_blue);
   wxColour colour(d_red, d_green, d_blue);
   data.SetColour(colour);

   wxColourDialog dialog(this, &data);
   if (dialog.ShowModal() == wxID_OK)
   {
      wxColourData retData = dialog.GetColourData();
      wxColour col = retData.GetColour();

      wxString channel;
      channel << col.Red();
      _c_red->SetValue(channel);channel.Clear();
      channel << col.Green();
      _c_green->SetValue(channel);channel.Clear();
      channel << col.Blue();
      _c_blue->SetValue(channel);channel.Clear();
      _colorsample->setColor(layprop::tellRGB(col.Red(), col.Green(), col.Blue(),178));
      wxCommandEvent dummy;
      OnApply(dummy);
   }
}

void tui::defineColor::OnColorSelected(wxCommandEvent& cmdevent)
{
    wxString color_name = cmdevent.GetString();
   const layprop::tellRGB* scol = getColor(std::string(color_name.mb_str(wxConvUTF8)));

   wxString channel;
   channel << scol->red();
   _c_red->SetValue(channel);channel.Clear();
   channel << scol->green();
   _c_green->SetValue(channel);channel.Clear();
   channel << scol->blue();
   _c_blue->SetValue(channel);channel.Clear();
   channel << scol->alpha();
   _c_alpha->SetValue(channel);channel.Clear();

   FindWindow(ID_BTNAPPLY)->Enable(false);
}

void tui::defineColor::OnColorPropChanged(wxCommandEvent& WXUNUSED(event))
{
   wxString s_red   =   _c_red->GetValue();
   wxString s_green = _c_green->GetValue();
   wxString s_blue  =  _c_blue->GetValue();
   wxString s_alpha = _c_alpha->GetValue();
   unsigned long d_red;    s_red.ToULong(&d_red);
   unsigned long d_green;s_green.ToULong(&d_green);
   unsigned long d_blue;  s_blue.ToULong(&d_blue);
   unsigned long d_alpha;s_alpha.ToULong(&d_alpha);

   _colorsample->setColor(layprop::tellRGB(d_red, d_green, d_blue, d_alpha));
   _colorsample->Refresh();
   FindWindow(ID_BTNAPPLY)->Enable(true);
}

void tui::defineColor::OnColorNameAdded(wxCommandEvent& WXUNUSED(event))
{
   wxString color_name = _dwcolname->GetValue();
   nameNormalize(color_name);
   if ((wxT("") == color_name) || (wxT(" ") == color_name))
   {
      wxString msg;
      msg << wxT("Empty color name.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else if (_allColors.end() != _allColors.find(std::string(color_name.mb_str(wxConvUTF8))))
   {
      wxString msg;
      msg << wxT("Color \"") << color_name << wxT("\" is already defined.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else
   {
      layprop::tellRGB* newcol = DEBUG_NEW layprop::tellRGB(0,0,0,178);
      std::string s_newcol = std::string(color_name.mb_str(wxConvUTF8));
      _allColors[s_newcol] = newcol;
      int index = _colorList->Append(color_name);
      _colorList->Select(index);
      wxCommandEvent clrsel;
      clrsel.SetString(color_name);
      OnColorSelected(clrsel);
      _colorList->SetFirstItem(color_name);
   }
}

void tui::defineColor::nameNormalize(wxString& str)
{
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before brackets and separators
}

const layprop::tellRGB* tui::defineColor::getColor(std::string color_name) const
{
   colorMAP::const_iterator col_set = _allColors.find(color_name);
   if (_allColors.end() == col_set) return NULL;
   return col_set->second;
}

void tui::defineColor::OnApply(wxCommandEvent& WXUNUSED(event))
{
   wxString s_name  = _colorList->GetStringSelection();
   wxString s_red   =   _c_red->GetValue();
   wxString s_green = _c_green->GetValue();
   wxString s_blue  =  _c_blue->GetValue();
   wxString s_alpha = _c_alpha->GetValue();
   unsigned long d_red;    s_red.ToULong(&d_red);
   unsigned long d_green;s_green.ToULong(&d_green);
   unsigned long d_blue;  s_blue.ToULong(&d_blue);
   unsigned long d_alpha;s_alpha.ToULong(&d_alpha);

   layprop::tellRGB* scol = DEBUG_NEW layprop::tellRGB(d_red, d_green, d_blue, d_alpha);
   std::string ss_name(s_name.mb_str(wxConvUTF8));
   if (_allColors.end() != _allColors.find(ss_name))
   {
      delete _allColors[ss_name];
      _allColors[ss_name] = scol;
   }
   FindWindow(ID_BTNAPPLY)->Enable(false);
}

tui::defineColor::~defineColor()
{
//   delete _dwcolname;
//   delete _colorsample;
//   delete _c_red;
//   delete _c_green;
//   delete _c_blue;
//   delete _c_alpha;
//   delete _colorList;
   for(colorMAP::const_iterator CI = _allColors.begin(); CI != _allColors.end(); CI++)
      delete CI->second;
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::pattern_canvas, wxWindow)
   EVT_PAINT(tui::pattern_canvas::OnPaint)
   EVT_LEFT_UP (tui::pattern_canvas::OnMouseLeftUp)
   EVT_RIGHT_UP(tui::pattern_canvas::OnMouseRightUp)
END_EVENT_TABLE()

tui::pattern_canvas::pattern_canvas(wxWindow *parent, wxWindowID id, wxPoint pos,
      wxSize size, const byte* init) :  wxWindow(parent, id, pos, size, wxNO_BORDER)
{
   if (NULL != init)
      for(byte i = 0; i < 128; i++)
         _pattern[i] = init[i];
   else
      Clear();
   _brushsize = 1;
}

void tui::pattern_canvas::Clear()
{
   for(byte i = 0; i < 128; i++)
      _pattern[i] = 0x00;
}

void tui::pattern_canvas::Fill()
{
   for(byte i = 0; i < 128; i++)
      _pattern[i] = 0xff;
}

void tui::pattern_canvas::OnPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.SetBackground(*wxWHITE);
   wxPen pen(*wxLIGHT_GREY);
   dc.SetPen(pen);
   dc.Clear();
   wxCoord w, h;
   dc.GetSize(&w, &h);
   dc.DrawRectangle(0, 0, w, h);
   //draw the grid
   for (word i = 8; i < h; i+=8)
      dc.DrawLine(0  , i  , w  , i  );
   for (word j = 8; j < w; j+=8)
      dc.DrawLine(j  , 0  , j  , h  );
   pen.SetColour(*wxBLACK);
   dc.SetPen(pen);
   for (word i = 64; i < h; i+=64)
      dc.DrawLine(0  , i  , w  , i  );
   for (word j = 64; j < w; j+=64)
      dc.DrawLine(j  , 0  , j  , h  );
   // now draw the pattern
   wxBrush brush(*wxBLUE);
   dc.SetBrush(brush);
   for (byte i = 0; i < 128; i++)
      for (byte k = 0; k < 8; k++)
         if (_pattern[i] & (0x80 >> k))
         {
            dc.DrawRectangle(((i%4) * 8 + k) * 8, (i/4) * 8, 8, 8 );
         }
}

void tui::pattern_canvas::OnMouseLeftUp(wxMouseEvent& event)
{
   int cshift;
   switch (_brushsize)
   {
      case 3: cshift = -1;break;
      case 5: cshift = -2;break;
      default: cshift = 0;
   }
   wxPoint position = event.GetPosition();
   for (int bsizex = 0; bsizex < _brushsize; bsizex++)
      for (int bsizey = 0; bsizey < _brushsize; bsizey++)
      {
         int xpnt  = (int)rint(position.x / 8)  + bsizex + cshift;
         int yindx = (int)rint(position.y / 8)  + bsizey + cshift;
         int xindx = (int)rint(xpnt / 8);
         if ((xpnt > 31) || (yindx > 31) || (xpnt < 0) || (yindx < 0)) continue;
         _pattern[(yindx * 4 + xindx) % 128] |= (byte)(0x80 >> (xpnt % 8));
      }
   Refresh();
}

void tui::pattern_canvas::OnMouseRightUp(wxMouseEvent& event)
{
   int cshift;
   switch (_brushsize)
   {
      case 3: cshift = -1;break;
      case 5: cshift = -2;break;
      default: cshift = 0;
   }
   wxPoint position = event.GetPosition();
   for (int bsizex = 0; bsizex < _brushsize; bsizex++)
      for (int bsizey = 0; bsizey < _brushsize; bsizey++)
      {
         int xpnt  = (int)rint(position.x / 8)  + bsizex + cshift;
         int yindx = (int)rint(position.y / 8)  + bsizey + cshift;
         int xindx = (int)rint(xpnt / 8);
         if ((xpnt > 31) || (yindx > 31) || (xpnt < 0) || (yindx < 0)) continue;
         _pattern[(yindx * 4 + xindx) % 128] &= ~(byte)(0x80 >> (xpnt % 8));
      }
   Refresh();
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::drawFillDef, wxDialog)
   EVT_BUTTON( ID_BTNCLEAR    , tui::drawFillDef::OnClear)
   EVT_BUTTON( ID_BTNFILL     , tui::drawFillDef::OnFill )
   EVT_RADIOBOX(ID_RADIOBSIZE , tui::drawFillDef::OnBrushSize)
END_EVENT_TABLE()

tui::drawFillDef::drawFillDef(wxWindow *parent, wxWindowID id, const wxString &title,
   wxPoint pos, const byte* init): wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
    static const wxString brushsize[] =
    {
        wxT("1"),
        wxT("3x3"),
        wxT("5x5"),
    };

    _radioBrushSize = DEBUG_NEW wxRadioBox(this, ID_RADIOBSIZE, wxT("Brush size"),
                                   wxDefaultPosition, wxDefaultSize,
                                   WXSIZEOF(brushsize), brushsize);


   wxBoxSizer *vsizer1 = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   vsizer1->Add( _radioBrushSize , 0, wxALL, 5);
   vsizer1->Add(DEBUG_NEW wxButton( this, ID_BTNCLEAR , wxT(" Clear ") , wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
                0, wxALL | wxALIGN_RIGHT, 5);
   vsizer1->Add(DEBUG_NEW wxButton( this, ID_BTNFILL  , wxT(" Fill  "), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
                0, wxALL | wxALIGN_RIGHT, 5);

   _sampleDraw = DEBUG_NEW pattern_canvas(this, -1, wxDefaultPosition, wxSize(256,256), init);

   wxBoxSizer *hsizer1 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer1->Add( _sampleDraw , 0, wxALL, 5);
   hsizer1->Add( vsizer1   , 0, wxEXPAND);

   wxBoxSizer *hsizer2 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   hsizer2->Add(0,0,1);
   hsizer2->Add(DEBUG_NEW wxButton( this, wxID_OK    , wxT("OK") ), 0, wxALL, 5);
   hsizer2->Add(DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 5);

   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   topsizer->Add( hsizer1 , 0, wxEXPAND);
   topsizer->Add( hsizer2 , 0, wxEXPAND);

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size

}

void tui::drawFillDef::OnClear(wxCommandEvent& WXUNUSED(event))
{
   _sampleDraw->Clear();
   _sampleDraw->Refresh();
}

void tui::drawFillDef::OnFill(wxCommandEvent& WXUNUSED(event))
{
   _sampleDraw->Fill();
   _sampleDraw->Refresh();
}

void tui::drawFillDef::OnBrushSize(wxCommandEvent& event)
{
   byte bsize;
   switch (event.GetInt())
   {
      case 0: bsize = 1;break;
      case 1: bsize = 3;break;
      case 2: bsize = 5;break;
      default: bsize = 1;
   }
   _sampleDraw->setBrushSize(bsize);
}

tui::drawFillDef::~drawFillDef()
{
   delete _sampleDraw;
}


//==============================================================================
BEGIN_EVENT_TABLE(tui::fill_sample, wxWindow)
   EVT_PAINT(tui::fill_sample::OnPaint)
END_EVENT_TABLE()

tui::fill_sample::fill_sample(wxWindow *parent, wxWindowID id, wxPoint pos,
   wxSize size, std::string init, const layprop::DrawProperties* drawProp) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   setFill(drawProp->getFill(init));
}

void tui::fill_sample::setFill(const byte* fill)
{
   if (NULL != fill)
   {
      wxBitmap stipplebrush((char  *)fill, 32, 32, 1);
      wxImage image;
      image = stipplebrush.ConvertToImage();
      stipplebrush = wxBitmap(image, 1);
      _brush = wxBrush(stipplebrush);
   }
   else
   {
      _brush = wxBrush();
   }
   _brush.SetColour(*wxWHITE);
}

void tui::fill_sample::OnPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.SetBackground(*wxBLACK);

   dc.SetBrush(_brush);
   dc.Clear();
   wxCoord w, h;
   dc.GetSize(&w, &h);
   dc.DrawRectangle(0, 0, w, h);
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::defineFill, wxDialog)
   EVT_LISTBOX(ID_ITEMLIST    , tui::defineFill::OnFillSelected   )
   EVT_BUTTON(ID_BTNEDIT      , tui::defineFill::OnDefineFill     )
   EVT_BUTTON(ID_NEWITEM      , tui::defineFill::OnFillNameAdded  )
END_EVENT_TABLE()

tui::defineFill::defineFill(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, const layprop::DrawProperties* drawProp) :
      wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
   NameList all_names;
   drawProp->allFills(all_names);
   _fillList = DEBUG_NEW wxListBox(this, ID_ITEMLIST, wxDefaultPosition, wxSize(150,200), 0, NULL, wxLB_SORT);
   std::string init_color;
   if (!all_names.empty())
   {
      init_color = *(all_names.begin());
   }
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      _fillList->Append(wxString(CI->c_str(), wxConvUTF8));
      byte* pat = DEBUG_NEW byte[128];
      fillcopy(drawProp->getFill(*CI), pat);
      _allFills[*CI] = pat;
   }
   // NOTE! Static boxes MUST be created before all other controls which are about to
   // be encircled by them. Otherwise the dialog box might work somewhere (Windows & fc8)
   // but not everywhere! (fc9)
   wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("New Fill") );
   wxBoxSizer *vsizer3 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Edit Pattern") );

   _dwfilname  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxSize(150,-1), 0,
                                          wxTextValidator(wxFILTER_ASCII, &_filname));
   _fillsample = DEBUG_NEW fill_sample( this, -1, wxDefaultPosition, wxSize(-1,150), init_color, drawProp);

   hsizer0->Add( _dwfilname   , 0, wxALL | wxEXPAND, 5);
   hsizer0->Add(0,0,1); //
   hsizer0->Add( DEBUG_NEW wxButton( this, ID_NEWITEM  , wxT("Add")    ), 0, wxALL, 5 );

   wxBoxSizer *vsizer2 = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   vsizer2->Add( _fillsample , 0, wxALL | wxEXPAND, 5);
//   vsizer2->Add( hsizer5   , 0, wxEXPAND);
   vsizer2->Add(0,0,1); //
   vsizer2->Add(DEBUG_NEW wxButton( this, ID_BTNEDIT  , wxT(" Define ") ), 0, wxALL | wxALIGN_RIGHT, 5);

   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK    , wxT("OK") ), 0, wxALL, 5 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel")  ), 0, wxALL, 5 );

   vsizer3->Add( _fillList   , 0, wxALL | wxEXPAND, 5);
   vsizer3->Add(0,0,1); //
   vsizer3->Add( vsizer2      , 0, wxEXPAND );

   wxBoxSizer *top_sizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   top_sizer->Add( hsizer0      , 0, wxEXPAND );
   top_sizer->Add( vsizer3      , 0, wxEXPAND );
   top_sizer->Add( button_sizer , 0, wxEXPAND );

   SetSizer( top_sizer );      // use the sizer for layout

   top_sizer->SetSizeHints( this );   // set size hints to honour minimum size

}

void tui::defineFill::nameNormalize(wxString& str)
{
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before brackets and separators
}

void tui::defineFill::OnFillSelected(wxCommandEvent& cmdevent)
{
    wxString fill_name = cmdevent.GetString();
    fillcopy(getFill(std::string(fill_name.mb_str(wxConvUTF8))),_current_pattern);
   _fillsample->setFill(_current_pattern);
   _fillsample->Refresh();
}

void tui::defineFill::OnFillNameAdded(wxCommandEvent& WXUNUSED(event))
{
   wxString fill_name = _dwfilname->GetValue();
   nameNormalize(fill_name);
   if ((wxT("") == fill_name) || (wxT(" ") == fill_name))
   {
      wxString msg;
      msg << wxT("Empty fill name.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else if (_allFills.end() != _allFills.find(std::string(fill_name.mb_str(wxConvUTF8))))
   {
      wxString msg;
      msg << wxT("Pattern \"") << fill_name << wxT("\" is already defined.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else
   {
      std::string s_newcol = std::string(fill_name.mb_str(wxConvUTF8));
      byte* newpat = DEBUG_NEW byte[128];
      for(byte i = 0; i< 128; i++)
         newpat[i] = 0x55 << ((byte)(i/4)%2);
      _allFills[s_newcol] = newpat;
      int index = _fillList->Append(fill_name);
      _fillList->Select(index);
      wxCommandEvent clrsel;
      clrsel.SetString(fill_name);
      OnFillSelected(clrsel);
      _fillList->SetFirstItem(fill_name);
   }
}

void tui::defineFill::OnDefineFill(wxCommandEvent& cmdevent)
{
//   const byte* initpat = getFill(_current_pattern);
   drawFillDef dialog(this, -1, wxT("Define Pattern"), wxDefaultPosition, _current_pattern);
   if (dialog.ShowModal() == wxID_OK)
   {
      fillcopy(dialog.pattern(),_current_pattern);
      _fillsample->setFill(_current_pattern);
      _fillsample->Refresh();
      wxString s_name  = _fillList->GetStringSelection();
      std::string ss_name(s_name.mb_str(wxConvUTF8));
      if (_allFills.end() != _allFills.find(ss_name))
      {
         fillcopy(_current_pattern, _allFills[ss_name]);
      }

   }
}

const byte* tui::defineFill::getFill(const std::string fill_name) const
{
   fillMAP::const_iterator fill_set = _allFills.find(fill_name);
   if (_allFills.end() == fill_set) return NULL;
   return fill_set->second;
}

void tui::defineFill::fillcopy(const byte* pattern, byte* nfill)
{
   for(byte i = 0; i < 128; i++)
      nfill[i] = pattern[i];
}

tui::defineFill::~defineFill()
{
//   delete _dwfilname;
//   delete _fillsample;
//   delete _fillList;
   for(fillMAP::const_iterator CI = _allFills.begin(); CI != _allFills.end(); CI++)
      delete[] CI->second;
}


//==============================================================================
BEGIN_EVENT_TABLE(tui::style_sample, wxWindow)
   EVT_PAINT(tui::style_sample::OnPaint)
END_EVENT_TABLE()

tui::style_sample::style_sample(wxWindow *parent, wxWindowID id, wxPoint pos,
   wxSize size, std::string init, const layprop::DrawProperties* drawProp) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER),
   _pen(wxT("black"), 3, wxUSER_DASH)
{
   wxPen pen(wxT("black"), 3, wxUSER_DASH);
   style_def initStyle;
   initStyle.pattern = drawProp->getLine(init)->pattern();
   initStyle.pscale = drawProp->getLine(init)->patscale();
   initStyle.width = drawProp->getLine(init)->width();
   setStyle(initStyle);
}

void tui::style_sample::setStyle(const tui::style_def&  styledef)
{
   word pattern = styledef.pattern;
   byte width = styledef.width;
   byte patscale = styledef.pscale;

   if ((patscale == 0) || (width == 0) || pattern == 0) return;
   wxDash dashes[32];
   int index = 0;
         enum statetype {zero, one};
      int state;


//      int i = 0;

      int mask = 0x8000;
      int length = 0;
      if (pattern & mask) 
      {
         state = one; 
      }
      else 
      {
         state = zero;
      }
      for (int i= 0; i < 16; ++i)
      {
         if(pattern & mask)
         {
            if(state == one) 
            {
               length++;
            }
            else
            {
               dashes[index] = patscale*length;
               state = one;
               index++;
               length = 1;
            }
         }
         else
         {
            if(state == zero) 
            {
               length++;
            }
            else
            {
               dashes[index] = patscale*length;
               state = zero;
               index++;
               length = 1;
            }
         }
         mask = mask >> 1;
      }
      _pen.SetWidth(width);
      _pen.SetDashes(index, dashes);
}

void tui::style_sample::OnPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.SetBackground(*wxBLACK);
   dc.Clear();
   wxCoord w, h;
   dc.GetSize(&w, &h);
   dc.DrawRectangle(0, 0, w, h);
   dc.SetPen(_pen);
   dc.DrawLine(1,10,w-1,10);


}

//==============================================================================
BEGIN_EVENT_TABLE(tui::defineStyle, wxDialog)
      EVT_LISTBOX(ID_ITEMLIST   , tui::defineStyle::OnStyleSelected   )
      EVT_BUTTON(ID_NEWITEM     , tui::defineStyle::OnStyleNameAdded   )
      EVT_BUTTON(ID_BTNAPPLY    , tui::defineStyle::OnStyleApply     )
      EVT_TEXT(ID_WIDTHVAL      , tui::defineStyle::OnStylePropChanged )
      EVT_TEXT(ID_PATVAL        , tui::defineStyle::OnStylePropChanged )
      EVT_TEXT(ID_PATSCALEVAL   , tui::defineStyle::OnStylePropChanged )
END_EVENT_TABLE()

tui::defineStyle::defineStyle(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, const layprop::DrawProperties* drawProp) :
      wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
   NameList all_names;
   drawProp->allLines(all_names);
   _styleList = DEBUG_NEW wxListBox(this, ID_ITEMLIST, wxDefaultPosition, wxSize(150,100), 0, NULL, wxLB_SORT);
   std::string init_style;
   if (!all_names.empty())
   {
      init_style = *(all_names.begin());
   }
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      _styleList->Append(wxString(CI->c_str(), wxConvUTF8));
      const layprop::LineSettings *line;
      style_def curStyle;
      line = drawProp->getLine(*CI);
      curStyle.pattern = line->pattern();
      curStyle.pscale = line->patscale();
      curStyle.width = line->width();
      _allStyles[*CI] = curStyle;
   }
   // NOTE! Static boxes MUST be created before all other controls which are about to
   // be encircled by them. Otherwise the dialog box might work somewhere (Windows & fc8)
   // but not everywhere! (fc9)
   wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("New Style") );
   wxBoxSizer *vsizer3 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Edit Pattern") );

   _dwstylename  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxSize(150,-1), 0,
                                          wxTextValidator(wxFILTER_ASCII, &_stylename));
   _stylesample = DEBUG_NEW style_sample( this, -1, wxDefaultPosition, wxSize(-1,25), init_style, drawProp);

   hsizer0->Add( _dwstylename   , 0, wxALL | wxEXPAND, 5);
   hsizer0->Add(0,0,1); //
   hsizer0->Add( DEBUG_NEW wxButton( this, ID_NEWITEM  , wxT("Add")    ), 0, wxALL, 5 );

   wxBoxSizer *vsizer2 = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   vsizer2->Add( _stylesample , 0, wxALL | wxEXPAND, 5);
   vsizer2->Add(0,0,1); //
   wxBoxSizer *hsizer1 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL);
   wxBoxSizer *hsizer2 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL);
   wxBoxSizer *hsizer3 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL);
   hsizer1->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Pattern:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER , 10);
   _pattern = DEBUG_NEW wxTextCtrl( this, ID_PATVAL , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_patternString));
   hsizer2->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Width  :"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER , 10);
   _width    = DEBUG_NEW wxTextCtrl( this, ID_WIDTHVAL , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_widthString));
   hsizer3->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Scale   :"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER , 10);
   _patscale    = DEBUG_NEW wxTextCtrl( this, ID_PATSCALEVAL , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_patscaleString));
   hsizer1->Add(_pattern, 0, wxALL | wxEXPAND, 5);
   hsizer2->Add(_width, 0, wxALL | wxEXPAND, 5);
   hsizer3->Add(_patscale, 0, wxALL | wxEXPAND, 5);
   vsizer2->Add( hsizer1 , 0, wxALL | wxEXPAND, 5);
   vsizer2->Add( hsizer2 , 0, wxALL | wxEXPAND, 5);
   vsizer2->Add( hsizer3 , 0, wxALL | wxEXPAND, 5);
   vsizer2->Add(DEBUG_NEW wxButton( this, ID_BTNAPPLY  , wxT(" Apply ") ), 0, wxALL | wxALIGN_RIGHT, 5);
   FindWindow(ID_BTNAPPLY)->Enable(false);

   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(0,0,1); //
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK    , wxT("OK") ), 0, wxALL, 5 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel")  ), 0, wxALL, 5 );

   vsizer3->Add( _styleList   , 0, wxALL | wxEXPAND, 5);
   vsizer3->Add(0,0,1); //
   vsizer3->Add( vsizer2      , 0, wxEXPAND );

   wxBoxSizer *top_sizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   top_sizer->Add( hsizer0      , 0, wxEXPAND );
   top_sizer->Add( vsizer3      , 0, wxEXPAND );
   top_sizer->Add( button_sizer , 0, wxEXPAND );

   SetSizer( top_sizer );      // use the sizer for layout

   top_sizer->SetSizeHints( this );   // set size hints to honour minimum size
}

tui::defineStyle::~defineStyle()
{
}

void tui::defineStyle::OnStyleSelected(wxCommandEvent& cmdevent)
{
   wxString style_name = cmdevent.GetString();
   try 
   {
      _current_style = getStyle(std::string(style_name.mb_str(wxConvUTF8)));
   }
   catch(EXPTNgui_problem) { return;}
   _stylesample->setStyle(_current_style);
   _stylesample->Refresh();
   wxString tempStr;
   tempStr << _current_style.pattern;
   _pattern->SetValue(tempStr);
   tempStr.Clear();
   tempStr << _current_style.width;
   _width->SetValue(tempStr);
   tempStr.Clear();
   tempStr << _current_style.pscale;
   _patscale->SetValue(tempStr);
   FindWindow(ID_BTNAPPLY)->Enable(false);
}

void tui::defineStyle::OnStyleNameAdded(wxCommandEvent& WXUNUSED(event))
{
   wxString style_name = _dwstylename->GetValue();
   nameNormalize(style_name);
   if ((wxT("") == style_name) || (wxT(" ") == style_name))
   {
      wxString msg;
      msg << wxT("Empty style name.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else if (_allStyles.end() != _allStyles.find(std::string(style_name.mb_str(wxConvUTF8))))
   {
      wxString msg;
      msg << wxT("Style \"") << style_name << wxT("\" is already defined.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else
   {
      std::string s_newcol = std::string(style_name.mb_str(wxConvUTF8));
      style_def newStyle;
      newStyle.width = 5; newStyle.pscale = 5; newStyle.pattern = 0x5555;
      _allStyles[s_newcol] = newStyle;
      int index = _styleList->Append(style_name);
      _styleList->Select(index);
      wxCommandEvent clrsel;
      clrsel.SetString(style_name);
      OnStyleSelected(clrsel);
      _styleList->SetFirstItem(style_name);
   }
}

void tui::defineStyle::nameNormalize(wxString& str)
{
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before brackets and separators
}

tui::style_def tui::defineStyle::getStyle(const std::string& style_name)
{
  styleMAP::const_iterator style_set = _allStyles.find(style_name);
  if (_allStyles.end() == style_set) 
  {
     std::string err;
     err += "Style " +  style_name + " doesn't exist";
     throw(EXPTNgui_problem(err));
  }
  return style_set->second;
}

void tui::defineStyle::OnStylePropChanged(wxCommandEvent& event)
{
   wxString s_pattern  = _pattern->GetValue();
   wxString s_width    = _width->GetValue();
   wxString s_patscale = _patscale->GetValue();

   unsigned long d_pattern;  s_pattern.ToULong(&d_pattern);
   unsigned long d_width;    s_width.ToULong(&d_width);
   unsigned long d_patscale; s_patscale.ToULong(&d_patscale);
   style_def tempStyle;
   tempStyle.pattern  = d_pattern;
   tempStyle.width    = d_width;
   tempStyle.pscale   = d_patscale;
   _stylesample->setStyle(tempStyle);
   _stylesample->Refresh();
   FindWindow(ID_BTNAPPLY)->Enable(true);
}

void tui::defineStyle::OnStyleApply(wxCommandEvent& event)
{

   wxString s_name      = _styleList->GetStringSelection();
   wxString s_pattern   = _pattern->GetValue();
   wxString s_width     = _width->GetValue();
   wxString s_patscale  = _patscale->GetValue();

   unsigned long d_pattern;  s_pattern.ToULong(&d_pattern);
   unsigned long d_width;    s_width.ToULong(&d_width);
   unsigned long d_patscale; s_patscale.ToULong(&d_patscale);

   style_def defStyle;
   defStyle.pattern  = d_pattern;
   defStyle.width    = d_width;
   defStyle.pscale   = d_patscale;

   std::string ss_name(s_name.mb_str(wxConvUTF8));
   if (_allStyles.end() != _allStyles.find(ss_name))
   {
      _allStyles[ss_name] = defStyle;
   }
   FindWindow(ID_BTNAPPLY)->Enable(false);
}
//==========================================================================
tui::nameCboxRecords::nameCboxRecords( wxWindow *parent, wxPoint pnt, wxSize sz,
            const SIMap& inlays, wxArrayString& all_strings, int row_height, const layprop::DrawProperties* drawProp)
            : wxPanel(parent, wxID_ANY, pnt, sz), _drawProp(drawProp)
{
   _cifMap = DATC->secureCifLayMap(_drawProp, true);
   word rowno = 0;
   for (SIMap::const_iterator CNM = inlays.begin(); CNM != inlays.end(); CNM++)
   {
      wxString cifln  = wxString(CNM->first.c_str(), wxConvUTF8);
      word tdtLay;
      if (!_cifMap->getTdtLay(tdtLay, CNM->first)) tdtLay = CNM->second;
      wxString wxics  = wxString(_drawProp->getLayerName(tdtLay).c_str(), wxConvUTF8);

      wxCheckBox* dwciflay  = DEBUG_NEW wxCheckBox( this, wxID_ANY, cifln,
         wxPoint(  5,(row_height+5)*rowno + 5), wxSize(100,row_height) );
      wxComboBox*   dwtpdlays = DEBUG_NEW wxComboBox  ( this, wxID_ANY, wxics,
         wxPoint(110,(row_height+5)*rowno + 5), wxSize(150,row_height), all_strings, wxCB_SORT);
      dwciflay->SetValue(true);
      _allRecords.push_back(LayerRecord(dwciflay, dwtpdlays));
      rowno++;
   }
}

SIMap* tui::nameCboxRecords::getTheMap(layprop::DrawProperties* drawProp)
{
   SIMap* cif_lay_map = DEBUG_NEW SIMap();
   for (AllRecords::const_iterator CNM = _allRecords.begin(); CNM != _allRecords.end(); CNM++ )
   {
      if (!CNM->_ciflay->GetValue()) continue;
      std::string layname = std::string(CNM->_tdtlay->GetValue().mb_str(wxConvUTF8));
      // the user didn't put a tdt correspondence for this CIF layer - so we'll try to use the CIF name
      if ("" == layname)
         layname = std::string(CNM->_ciflay->GetLabel().mb_str(wxConvUTF8));
      unsigned layno = _drawProp->getLayerNo(layname);
      if (ERR_LAY == layno)
      {
//         layno = drawProp->addLayer(layname);
         layno = drawProp->addLayer(layname);
         TpdPost::layer_add(layname, layno);
      }
      (*cif_lay_map)[std::string(CNM->_ciflay->GetLabel().mb_str(wxConvUTF8))] = layno;
   }
   return cif_lay_map;
}

USMap* tui::nameCboxRecords::getTheFullMap(layprop::DrawProperties* drawProp)
{
   SIMap* umap = getTheMap(drawProp);
   USMap* nmap = _cifMap->updateMap(umap);
   delete umap;
   return nmap;
}

//==========================================================================
tui::nameCbox3Records::nameCbox3Records( wxWindow *parent, wxPoint pnt, wxSize sz,
            const ExtLayers& inlays, wxArrayString& all_strings, int row_height, const layprop::DrawProperties* drawProp)
            : wxPanel(parent, wxID_ANY, pnt, sz), _drawProp(drawProp)
{
   _gdsLayMap = DATC->secureGdsLayMap(_drawProp, true);
   word rowno = 0;
   for (ExtLayers::const_iterator CNM = inlays.begin(); CNM != inlays.end(); CNM++)
   {
      wxString sGdsLay;
      sGdsLay << CNM->first;
      for (WordSet::const_iterator CTP = CNM->second.begin(); CTP != CNM->second.end(); CTP++)
      {
         wxString sGdsDtype;
         sGdsDtype << *CTP;
         word wTdtLay;
         if (!_gdsLayMap->getTdtLay( wTdtLay, CNM->first, *CTP)) wTdtLay = CNM->first;
         wxString sTdtLay(_drawProp->getLayerName(wTdtLay).c_str(), wxConvUTF8);

         wxCheckBox* dwgdslay  = DEBUG_NEW wxCheckBox( this, wxID_ANY, sGdsLay,
            wxPoint(  5,(row_height+5)*rowno + 5), wxSize(55,row_height), wxALIGN_LEFT );
         wxStaticText* dwgdstype = DEBUG_NEW wxStaticText( this, wxID_ANY, sGdsDtype,
            wxPoint( 65,(row_height+5)*rowno + 5), wxSize(55,row_height), wxALIGN_LEFT );
         wxComboBox*   dwtpdlays = DEBUG_NEW wxComboBox  ( this, wxID_ANY, sTdtLay,
            wxPoint( 125,(row_height+5)*rowno + 5), wxSize(150,row_height), all_strings, wxCB_SORT);
         dwgdslay->SetValue(true);
         _allRecords.push_back(LayerRecord(dwgdslay, dwgdstype, dwtpdlays));
         rowno++;
      }
   }
}

USMap* tui::nameCbox3Records::getTheMap()
{
   USMap* gds_lay_map = DEBUG_NEW USMap();
   for (AllRecords::const_iterator CNM = _allRecords.begin(); CNM != _allRecords.end(); CNM++ )
   {
      if (!CNM->_gdslay->GetValue()) continue;
      unsigned layno;
      std::string layname = std::string(CNM->_tdtlay->GetValue().mb_str(wxConvUTF8));
      if ("" == layname)
      {
         long lint;
         CNM->_gdslay->GetLabel().ToLong(&lint);
         layno = lint;
      }
      else layno = _drawProp->getLayerNo(layname);
      std::ostringstream gdslaytype;
      if (gds_lay_map->end() != gds_lay_map->find(layno))
         gdslaytype << (*gds_lay_map)[layno] << ","
                    <<  std::string(CNM->_gdstype->GetLabel().mb_str(wxConvUTF8));
      else
         gdslaytype <<  std::string(CNM->_gdslay->GetLabel().mb_str(wxConvUTF8)) << ";"
                    <<  std::string(CNM->_gdstype->GetLabel().mb_str(wxConvUTF8))   ;
      (*gds_lay_map)[layno] = gdslaytype.str();
   }
   return gds_lay_map;
}

USMap* tui::nameCbox3Records::getTheFullMap()
{
   USMap* umap = getTheMap();
   USMap* nmap = _gdsLayMap->updateMap(umap, true);
   delete umap;
   return nmap;
}

//==========================================================================
BEGIN_EVENT_TABLE(tui::nameCboxList, wxScrolledWindow)
      EVT_SIZE( tui::nameCboxList::OnSize )
END_EVENT_TABLE()

tui::nameCboxList::nameCboxList(wxWindow* parent, wxWindowID id, wxPoint pnt, wxSize sz, const SIMap& inlays, const layprop::DrawProperties* drawProp) :
      wxScrolledWindow(parent, id, pnt, sz, wxBORDER_RAISED)
{
   // collect all defined layers
   NameList all_names;
   drawProp->allLayers(all_names);
   wxArrayString all_strings;
   int line_height = (int)(GetFont().GetPointSize() * 2.5);
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));

   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("CIF layer"),
      wxPoint(  5, 5), wxSize(100,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);
   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("TDT layer"),
      wxPoint(110, 5), wxSize(150,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);

   wxSize panelsz = GetClientSize();
   panelsz.SetHeight(panelsz.GetHeight() - line_height);
   _laypanel = DEBUG_NEW tui::nameCboxRecords(this, wxPoint(0,line_height), panelsz, inlays, all_strings, line_height, drawProp);
   SetTargetWindow(_laypanel); // trget scrollbar window
   if (inlays.size() > (unsigned) sz.GetHeight() / (line_height + 5))
      SetScrollbars(  0, (line_height+5),  0, inlays.size() );
}

void tui::nameCboxList::OnSize( wxSizeEvent &WXUNUSED(event) )
{
   //Comment below as well as the part of the method is taken from wx samples scrollsub.cpp
   // We need to override OnSize so that our scrolled window
   // a) does call Layout() to use sizers for positioning the controls but
   // b) does not query the sizer for their size and use that for setting the scrollable
   // area as set that ourselves by calling SetScrollbar() in the constructor.

   Layout();
   wxSize client_size = GetClientSize();
   _laypanel->SetClientSize(client_size);
   AdjustScrollbars();
}

//==========================================================================
BEGIN_EVENT_TABLE(tui::nameCbox3List, wxScrolledWindow)
      EVT_SIZE( tui::nameCbox3List::OnSize )
END_EVENT_TABLE()

tui::nameCbox3List::nameCbox3List(wxWindow* parent, wxWindowID id, wxPoint pnt, wxSize sz,
      const ExtLayers& inlays, const layprop::DrawProperties* drawProp, wxString inName) :
      wxScrolledWindow(parent, id, pnt, sz, wxBORDER_RAISED)
{
   // collect all defined layers
   NameList all_names;
   drawProp->allLayers(all_names);
   wxArrayString all_strings;
   int line_height = (int)(GetFont().GetPointSize() * 2.5);
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));

   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, inName,
      wxPoint(  5, 5), wxSize(120,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);
   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("TDT layer"),
      wxPoint(125, 5), wxSize(150,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);

   wxSize panelsz = GetClientSize();
   panelsz.SetHeight(panelsz.GetHeight() - line_height);
   _laypanel = DEBUG_NEW tui::nameCbox3Records(this, wxPoint(0,line_height), panelsz, inlays, all_strings, line_height, drawProp);
   SetTargetWindow(_laypanel); // target scrollbar window
   if (inlays.size() > (unsigned) sz.GetHeight() / (line_height + 5))
      SetScrollbars(  0, (line_height+5),  0, _laypanel->getNumRows() + 1 );
}

void tui::nameCbox3List::OnSize( wxSizeEvent &WXUNUSED(event) )
{
   //Comment below as well as the part of the method is taken from wx samples scrollsub.cpp
   // We need to override OnSize so that our scrolled window
   // a) does call Layout() to use sizers for positioning the controls but
   // b) does not query the sizer for their size and use that for setting the scrollable
   // area as set that ourselves by calling SetScrollbar() in the constructor.

   Layout();
   wxSize client_size = GetClientSize();
   _laypanel->SetClientSize(client_size);
   AdjustScrollbars();
}

//==========================================================================
tui::nameEboxRecords::nameEboxRecords( wxWindow *parent, wxPoint pnt, wxSize sz,
            const WordList& inlays, wxArrayString& all_strings, int row_height, const layprop::DrawProperties* drawProp)
            : wxPanel(parent, wxID_ANY, pnt, sz), _drawProp(drawProp)
{
   word rowno = 0;
   _cifMap = DATC->secureCifLayMap(_drawProp, false);
   for (WordList::const_iterator CNM = inlays.begin(); CNM != inlays.end(); CNM++)
   {
      word layno = *CNM;
      wxString tpdlay  = wxString(_drawProp->getLayerName(*CNM).c_str(), wxConvUTF8);
      wxString ciflay;
      std::string cifName;
      if ( _cifMap->getCifLay(cifName, layno) )
         ciflay = wxString(cifName.c_str(), wxConvUTF8);
      else
         ciflay << wxT("L") << layno;
      if (4 < ciflay.Length()) ciflay.Clear(); // Should not be longer than 4
      wxCheckBox* dwtpdlays  = DEBUG_NEW wxCheckBox( this, wxID_ANY, tpdlay,
            wxPoint(  5,(row_height+5)*rowno + 5), wxSize(100,row_height) );
      wxTextCtrl*   dwciflay = DEBUG_NEW wxTextCtrl ( this, wxID_ANY, ciflay,
            wxPoint(110,(row_height+5)*rowno + 5), wxSize(145,row_height));
      dwtpdlays->SetValue(true);
      _allRecords.push_back(LayerRecord(dwtpdlays, dwciflay));
      rowno++;
   }
}

USMap* tui::nameEboxRecords::getTheMap()
{
   USMap* cif_lay_map = DEBUG_NEW USMap();
   for (AllRecords::const_iterator CNM = _allRecords.begin(); CNM != _allRecords.end(); CNM++ )
   {
      if (!CNM->_tdtlay->GetValue()) continue;
      std::string layname = std::string(CNM->_tdtlay->GetLabel().mb_str(wxConvUTF8));
      assert("" != layname);
      unsigned layno = _drawProp->getLayerNo(layname);
      assert(layno);
      (*cif_lay_map)[layno] = std::string(CNM->_ciflay->GetValue().mb_str(wxConvUTF8));
   }
   return cif_lay_map;
}

USMap* tui::nameEboxRecords::getTheFullMap()
{
   USMap* umap = getTheMap();
   USMap* nmap = _cifMap->updateMap(umap);
   delete umap;
   return nmap;
}

//--------------------------------------------------------------------------
BEGIN_EVENT_TABLE(tui::nameEboxList, wxScrolledWindow)
      EVT_SIZE( tui::nameEboxList::OnSize )
END_EVENT_TABLE()

tui::nameEboxList::nameEboxList(wxWindow* parent, wxWindowID id, wxPoint pnt, wxSize sz, const WordList& inlays, const layprop::DrawProperties* drawProp) :
      wxScrolledWindow(parent, id, pnt, sz, wxBORDER_RAISED)
{
   // collect all defined layers
   NameList all_names;
   drawProp->allLayers(all_names);
   wxArrayString all_strings;
   int line_height = (int)(GetFont().GetPointSize() * 2.5);
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));

   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("TDT layer"),
      wxPoint(  5, 5), wxSize(100,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);
   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("CIF layer"),
      wxPoint(110, 5), wxSize(150,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);

   wxSize panelsz = GetClientSize();
   panelsz.SetHeight(panelsz.GetHeight() - line_height);
   _laypanel = DEBUG_NEW tui::nameEboxRecords(this, wxPoint(0,line_height), panelsz, inlays, all_strings, line_height, drawProp);
   SetTargetWindow(_laypanel); // trget scrollbar window
   if (inlays.size() > (unsigned) sz.GetHeight() / (line_height + 5))
      SetScrollbars(  0, (line_height+5),  0, inlays.size() );
}

void tui::nameEboxList::OnSize( wxSizeEvent &WXUNUSED(event) )
{
   //Comment below as well as the part of the method is taken from wx samples scrollsub.cpp
   // We need to override OnSize so that our scrolled window
   // a) does call Layout() to use sizers for positioning the controls but
   // b) does not query the sizer for their size and use that for setting the scrollable
   // area as set that ourselves by calling SetScrollbar() in the constructor.

   Layout();
   wxSize client_size = GetClientSize();
   _laypanel->SetClientSize(client_size);
   AdjustScrollbars();
}

//==========================================================================
tui::nameEbox3Records::nameEbox3Records( wxWindow *parent, wxPoint pnt, wxSize sz,
            const WordList& inlays, wxArrayString& all_strings, int row_height, const layprop::DrawProperties* drawProp)
            : wxPanel(parent, wxID_ANY, pnt, sz), _drawProp(drawProp)
{
   _gdsLayMap= DATC->secureGdsLayMap(_drawProp, false);
   word rowno = 0;
   for (WordList::const_iterator CNM = inlays.begin(); CNM != inlays.end(); CNM++)
   {
      word wGdsLay, wGdsType;
      if (!_gdsLayMap->getExtLayType(wGdsLay, wGdsType, *CNM))
      {
         wGdsLay  = *CNM;
         wGdsType = 0;
      }
      wxString tpdlay  = wxString(_drawProp->getLayerName(*CNM).c_str(), wxConvUTF8);
      wxString gdslay, gdstype;
      gdslay << wGdsLay;
      gdstype <<wGdsType;

      wxCheckBox* dwtpdlays  = DEBUG_NEW wxCheckBox( this, wxID_ANY, tpdlay,
         wxPoint(  5,(row_height+5)*rowno + 5), wxSize(100,row_height) );
      wxTextCtrl*   dwgdslay = DEBUG_NEW wxTextCtrl ( this, wxID_ANY, gdslay,
         wxPoint(110,(row_height+5)*rowno + 5), wxSize(70,row_height));
      wxTextCtrl*   dwgdstyp = DEBUG_NEW wxTextCtrl ( this, wxID_ANY, gdstype,
         wxPoint(180,(row_height+5)*rowno + 5), wxSize(70,row_height));
      dwtpdlays->SetValue(true);
      _allRecords.push_back(LayerRecord(dwtpdlays, dwgdslay, dwgdstyp));
      rowno++;
   }
}

USMap* tui::nameEbox3Records::getTheMap()
{
   USMap* gds_lay_map = DEBUG_NEW USMap();
   for (AllRecords::const_iterator CNM = _allRecords.begin(); CNM != _allRecords.end(); CNM++ )
   {
      if (!CNM->_tdtlay->GetValue()) continue;
      std::string layname = std::string(CNM->_tdtlay->GetLabel().mb_str(wxConvUTF8));
      assert("" != layname);
      unsigned layno = _drawProp->getLayerNo(layname);
      assert(layno);
      std::ostringstream gdslaytype;
      gdslaytype <<  std::string(CNM->_gdslay->GetValue().mb_str(wxConvUTF8)) << ";"
                 <<  std::string(CNM->_gdstype->GetValue().mb_str(wxConvUTF8))   ;
      (*gds_lay_map)[layno] = gdslaytype.str();
   }
   return gds_lay_map;
}

USMap* tui::nameEbox3Records::getTheFullMap()
{
   USMap* umap = getTheMap();
   USMap* nmap = _gdsLayMap->updateMap(umap, false);
   delete umap;
   return nmap;
}

//--------------------------------------------------------------------------
BEGIN_EVENT_TABLE(tui::nameEbox3List, wxScrolledWindow)
      EVT_SIZE( tui::nameEbox3List::OnSize )
END_EVENT_TABLE()

tui::nameEbox3List::nameEbox3List(wxWindow* parent, wxWindowID id, wxPoint pnt, wxSize sz, const WordList& inlays, const layprop::DrawProperties* drawProp) :
      wxScrolledWindow(parent, id, pnt, sz, wxBORDER_RAISED)
{
   // collect all defined layers
   NameList all_names;
   drawProp->allLayers(all_names);
   wxArrayString all_strings;
   int line_height = (int)(GetFont().GetPointSize() * 2.5);
   for( NameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));

   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("TDT layer"),
      wxPoint(  5, 5), wxSize(100,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);
   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("GDS layer"),
      wxPoint(110, 5), wxSize(70,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);
   /*(void)*/ DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("GDS type"),
      wxPoint(180, 5), wxSize(70,line_height), wxALIGN_CENTER | wxBORDER_SUNKEN);

   wxSize panelsz = GetClientSize();
   panelsz.SetHeight(panelsz.GetHeight() - line_height);
   _laypanel = DEBUG_NEW tui::nameEbox3Records(this, wxPoint(0,line_height), panelsz, inlays, all_strings, line_height, drawProp);
   SetTargetWindow(_laypanel); // target scrollbar window
   if (inlays.size() > (unsigned) sz.GetHeight() / (line_height + 5))
      SetScrollbars(  0, (line_height+5),  0, inlays.size() );
}

void tui::nameEbox3List::OnSize( wxSizeEvent &WXUNUSED(event) )
{
   //Comment below as well as the part of the method is taken from wx samples scrollsub.cpp
   // We need to override OnSize so that our scrolled window
   // a) does call Layout() to use sizers for positioning the controls but
   // b) does not query the sizer for their size and use that for setting the scrollable
   // area as set that ourselves by calling SetScrollbar() in the constructor.

   Layout();
   wxSize client_size = GetClientSize();
   _laypanel->SetClientSize(client_size);
   AdjustScrollbars();
}

BEGIN_EVENT_TABLE(tui::cadenceConvert, wxDialog)
   EVT_BUTTON(ID_BTNDISPLAYADD, tui::cadenceConvert::onDisplayAdd)
   EVT_BUTTON(ID_BTNTECHADD, tui::cadenceConvert::onTechAdd)
   EVT_BUTTON(ID_BTNOUTFILE, tui::cadenceConvert::onOutputFile)
   EVT_BUTTON(ID_BTNCONVERT, tui::cadenceConvert::onConvert)
END_EVENT_TABLE()

tui::cadenceConvert::cadenceConvert(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos):
   wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
   _displayList = DEBUG_NEW wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxSize(200, -1));
   _techList = DEBUG_NEW wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxSize(200, -1));
   _outputFile = DEBUG_NEW wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxSize(200, -1));

   wxBoxSizer *topSizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
      wxBoxSizer *vertSizer = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("") );
         wxBoxSizer *displaySizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            displaySizer->Add(10,10,0);
            displaySizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, _T("Display file: ")), 0, 0, 0);
            displaySizer->Add(_displayList, 1, wxEXPAND, 10);
            displaySizer->Add(DEBUG_NEW wxButton(this, ID_BTNDISPLAYADD, _T("Add ...")), 0, 0, 0);
            displaySizer->Add(10,10,0);
         wxBoxSizer *techSizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            techSizer->Add(10,10,0);
            techSizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, _T("Tech file:    ")), 0, 0, 0);
            techSizer->Add(_techList, 1, wxEXPAND, 10 );
            techSizer->Add(DEBUG_NEW wxButton( this, ID_BTNTECHADD, wxT("Add ...") ), 0, 0, 0 );
            techSizer->Add(10,10,0);
         wxBoxSizer *outputSizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            outputSizer->Add(10,10,0);
            outputSizer->Add(DEBUG_NEW wxStaticText(this, wxID_ANY, _T("Output file:   ")), 0, 0, 0);
            outputSizer->Add(_outputFile, 1, wxEXPAND, 10 );
            outputSizer->Add(DEBUG_NEW wxButton( this, ID_BTNOUTFILE, wxT("Add ...") ), 0, 0, 0 );
            outputSizer->Add(10,10,0);
      vertSizer->Add(displaySizer, 0, wxEXPAND ); // no border and centre horizontally
      vertSizer->Add(techSizer, 0, wxEXPAND/*wxALIGN_CENTER*/ );
      vertSizer->Add(outputSizer, 0, wxEXPAND/*wxALIGN_CENTER*/ );

   topSizer->Add(vertSizer, 0, wxEXPAND);
   topSizer->Add(10,10,0);
   topSizer->Add(DEBUG_NEW wxButton( this, ID_BTNCONVERT, wxT("Convert") ), 0, wxALIGN_CENTER , 0 );
   topSizer->Add(10,10,0);
   SetSizer( topSizer );
   topSizer->SetSizeHints( this );
}

void  tui::cadenceConvert::onDisplayAdd(wxCommandEvent& evt)
{
   wxFileDialog dlg(this, wxT("Select a display file"), wxT(""), wxT(""),
      wxT("Display files (*.drf)|*.drf|All files(*.*)|*.*"),
      tpdfOPEN);
   if (wxID_OK == dlg.ShowModal())
   {
      wxString filename = dlg.GetPath();
      wxString ost;
      _displayList->SetValue(filename);
   }
}

void  tui::cadenceConvert::onTechAdd(wxCommandEvent& evt)
{
   wxFileDialog dlg(this, wxT("Select a tech file"), wxT(""), wxT(""),
      wxT("All files(*.*)|*.*"),
      tpdfOPEN);
   if (wxID_OK == dlg.ShowModal())
   {
      wxString filename = dlg.GetPath();
      wxString ost;
      _techList->SetValue(filename);
   }
}

void  tui::cadenceConvert::onOutputFile(wxCommandEvent& evt)
{
   wxFileDialog dlg(this, wxT("Select output tell file"), wxT(""), wxT(""),
      wxT("tell files (*.tll)|*.tll|All files(*.*)|*.*"),
      tpdfSAVE|wxFD_OVERWRITE_PROMPT);
   if (wxID_OK == dlg.ShowModal())
   {
      wxString filename = dlg.GetPath();
      wxString ost;
      _outputFile->SetValue(filename);
   }
}

void ShowOutput(const wxString& cmd          ,
                const wxArrayString& output  ,
                const wxString& title         )
{
    size_t count = output.GetCount();
    if ( !count )
        return;
    for ( size_t n = 0; n < count; n++ )
    {
       tell_log(console::MT_INFO, output[n]);
    }
}
void tui::cadenceConvert::onConvert(wxCommandEvent& evt)
{
   if (_displayList->IsEmpty() || _techList->IsEmpty())
   {
      wxMessageDialog dlg(this, wxT("Please add display and tech files"), wxT("Warning!"));
      dlg.ShowModal();
   }
   else
   {
      wxString str;
      //Looking for $TPD_GLOBAL/virtuoso2tll
      str.Append(wxString(DATC->globalDir().c_str(),wxConvFile));
#ifdef WIN32
      //For windows full path shall be replaced with short one(Program Files->Progra~1)
      wxFileName filename=wxFileName(str);
      str=filename.GetShortPath();
      str.Append(wxT("virtuoso2tll.exe "));
#else
      str.Append(wxT("utils/cadence/"));
      str.Append(wxT("virtuoso2tll.ss "));
#endif

      //prepare command line _arguments
      wxString strtemp = _outputFile->GetValue();
      str.Append(strtemp);
      str.Append(wxT(" "));
      str.Append(_displayList->GetValue());
      str.Append(wxT(" "));
      str.Append(_techList->GetValue());
      //Replace all slashes to double slashes
      str.Replace(wxT("\\") , wxT("\\\\"), true);
      tell_log(console::MT_INFO, str);

      wxArrayString output, errors;
      int code = wxExecute(str, output, errors);
      if ( code != -1 )
      {
         ShowOutput(str, output, _T("Output"));
         ShowOutput(str, errors, _T("Errors"));
      }
   }
}
//=============================================================================
tui::TopedPropertySheets::TopedPropertySheets(wxWindow* parent)
{
   Create(parent, wxID_ANY, wxT("Properties"), wxDefaultPosition, wxDefaultSize,
       wxDEFAULT_DIALOG_STYLE| (int)wxPlatform::IfNot(wxOS_WINDOWS_CE, wxRESIZE_BORDER));
   GetSizer()->Add(DEBUG_NEW wxButton(this, wxID_OK, wxT("Hide")), 0, wxALL | wxALIGN_RIGHT, 5);
   wxBookCtrlBase* notebook = GetBookCtrl();
   _canvasSheet    = DEBUG_NEW CanvasPSheet(notebook);
   _renderingSheet = DEBUG_NEW RenderingPSheet(notebook);
   notebook->AddPage(_canvasSheet   , wxT("Canvas")   , true, -1);
   notebook->AddPage(_renderingSheet, wxT("Rendering"), true, -1);
   LayoutDialog();
}

//=============================================================================
BEGIN_EVENT_TABLE(tui::TopedPropertySheets::RenderingPSheet, wxPanel)
    EVT_CHECKBOX(PDCELL_CHECKDOV      , tui::TopedPropertySheets::RenderingPSheet::OnCellCheckDov  )
    EVT_CHECKBOX(PDSET_CELLBOX        , tui::TopedPropertySheets::RenderingPSheet::OnCellBox       )
    EVT_CHECKBOX(PDSET_CELLMARK       , tui::TopedPropertySheets::RenderingPSheet::OnCellMark      )
    EVT_CHECKBOX(PDSET_TEXTBOX        , tui::TopedPropertySheets::RenderingPSheet::OnTextBox       )
    EVT_CHECKBOX(PDSET_TEXTMARK       , tui::TopedPropertySheets::RenderingPSheet::OnTextMark      )
    EVT_CHECKBOX(PDSET_TEXTORI        , tui::TopedPropertySheets::RenderingPSheet::OnTextOri       )
    EVT_CHECKBOX(PDGRC_BLINKON        , tui::TopedPropertySheets::RenderingPSheet::OnGrcBlinkOn    )
    EVT_COMBOBOX(PDSET_TEXTFONTS      , tui::TopedPropertySheets::RenderingPSheet::OnTextFont      )
    EVT_COMMAND_ENTER(PDIMG_DETAIL    , tui::TopedPropertySheets::RenderingPSheet::OnImageDetail   )
    EVT_COMMAND_ENTER(PDCELL_DOV      , tui::TopedPropertySheets::RenderingPSheet::OnCellDov       )
    EVT_COMMAND_ENTER(PDCELL_DAB      , tui::TopedPropertySheets::RenderingPSheet::OnCellDab       )
    EVT_COMMAND_ENTER(PDGRC_BLINKFREQ , tui::TopedPropertySheets::RenderingPSheet::OnGrcBlinkFreq  )
END_EVENT_TABLE()

tui::TopedPropertySheets::RenderingPSheet::RenderingPSheet(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
   wxBoxSizer* topSizer = DEBUG_NEW wxBoxSizer(wxVERTICAL);
      // Image details (Quality)
      wxBoxSizer *imgSizer  = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Image detail (square pixels)"));
         _imageDetail = DEBUG_NEW sgSliderControl(this, PDIMG_DETAIL, 1, 100, 40);
      imgSizer->Add(_imageDetail, 0, wxALL | wxALIGN_CENTER | wxEXPAND);

      // Cell related rendering properties
      wxBoxSizer *topCellSizer = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Cells"));
         // Cell Bounding box and reference marks
         wxBoxSizer *cellSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
            wxCheckBox* cellOvlBox = DEBUG_NEW wxCheckBox(this, PDSET_CELLBOX , wxT("Overlapping box"));
            wxCheckBox* cellMarks  = DEBUG_NEW wxCheckBox(this, PDSET_CELLMARK, wxT("Reference marks"));
         cellSizer->Add(cellOvlBox, 1, wxALL | wxALIGN_CENTER | wxEXPAND);
         cellSizer->Add(cellMarks , 1, wxALL | wxALIGN_CENTER | wxEXPAND);
         // Cell Depth of view
         wxBoxSizer *cdovSizer = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Visible hierarchy depth"));
         _cbDepthOfViewLimit = DEBUG_NEW wxCheckBox(this, PDCELL_CHECKDOV , wxT("unlimited"));
         _cbDepthOfViewLimit->SetValue(true);
         _cellDepthOfView = DEBUG_NEW sgSliderControl(this, PDCELL_DOV, 1, 8, 8);
         _cellDepthOfView->Enable(false);
         cdovSizer->Add(_cbDepthOfViewLimit, 0, wxALL| wxALIGN_CENTER | wxEXPAND);
         cdovSizer->Add(   _cellDepthOfView, 0, wxALL| wxALIGN_CENTER | wxEXPAND);
         // Cell Depth brightness ebb (shadow)
         wxBoxSizer *ebbSizer  = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Hierarchy depth opacity"));
            _cellDepthEbb = DEBUG_NEW sgSliderControl(this, PDCELL_DAB, 0, 80, 0);
         ebbSizer->Add(_cellDepthEbb, 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      // Pack all cell related properties
      topCellSizer->Add(cellSizer , 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      topCellSizer->Add(10,10,0);
      topCellSizer->Add(cdovSizer , 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      topCellSizer->Add(10,10,0);
      topCellSizer->Add(ebbSizer  , 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      // Text related rendering properties
      wxBoxSizer *topTextSizer = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Texts"));
         wxBoxSizer *textSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
            wxCheckBox* textOvlBox = DEBUG_NEW wxCheckBox(this, PDSET_TEXTBOX , wxT("Overlapping box"));
            wxCheckBox* textMarks  = DEBUG_NEW wxCheckBox(this, PDSET_TEXTMARK, wxT("Reference marks"));
         textSizer->Add(textOvlBox, 1, wxALL | wxALIGN_CENTER | wxEXPAND);
         textSizer->Add(textMarks , 1, wxALL | wxALIGN_CENTER | wxEXPAND);
         //
         wxCheckBox* textOrien  = DEBUG_NEW wxCheckBox(this, PDSET_TEXTORI, wxT("Adjust orientation"));
         wxArrayString allFontNames_wx;
         NameList allFontNames_std;
         wxComboBox* allFonts = DEBUG_NEW wxComboBox(this, PDSET_TEXTFONTS,
               wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY | wxCB_SORT);
      topTextSizer->Add(textSizer, 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      topTextSizer->Add(textOrien, 0, wxALL | wxALIGN_LEFT             );
      topTextSizer->Add(allFonts , 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      //GRC related rendering properties
      wxBoxSizer *grcSizer = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Invalid objects"));
      _cbGrcBlinkOn = DEBUG_NEW wxCheckBox(this, PDGRC_BLINKON , wxT("blinking"));
      _cbGrcBlinkOn->SetValue(true);
      _cbGrcBlinkFreq = DEBUG_NEW sgSliderControl(this, PDGRC_BLINKFREQ, 1, 10, 5);
      _cbGrcBlinkFreq->Enable(true);
      grcSizer->Add(_cbGrcBlinkOn  , 0, wxALL| wxALIGN_CENTER | wxEXPAND);
      grcSizer->Add(_cbGrcBlinkFreq, 0, wxALL| wxALIGN_CENTER | wxEXPAND);

   // Pack everything
   topSizer->Add(   imgSizer , 0, wxEXPAND | wxALL, 5);//   topSizer->Add(10,10,0);
   topSizer->Add(topCellSizer, 0, wxEXPAND | wxALL, 5);//   topSizer->Add(10,10,0);
   topSizer->Add(topTextSizer, 0, wxEXPAND | wxALL, 5);
   topSizer->Add(grcSizer    , 0, wxEXPAND | wxALL, 5);

   SetSizer(topSizer);
   topSizer->Fit(this);
}

void tui::TopedPropertySheets::RenderingPSheet::OnCellBox (wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("hidecellbox(") << (cmdEvent.GetInt() ? wxT("false") : wxT("true")) << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnCellMark(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("hidecellmarks(") << (cmdEvent.GetInt() ? wxT("false") : wxT("true")) << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnTextBox (wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("hidetextbox(") << (cmdEvent.GetInt() ? wxT("false") : wxT("true")) << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnTextMark(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("hidetextmarks(") << (cmdEvent.GetInt() ? wxT("false") : wxT("true")) << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnTextOri(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"ADJUST_TEXT_ORIENTATION\", \"")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT("\"});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnTextFont(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"SELECT_TEXT_FONT\", \"")
       << cmdEvent.GetString()
       << wxT("\"});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnCellCheckDov(wxCommandEvent& cmdevent)
{
   wxString ost;
   bool enable = (0 == cmdevent.GetInt());
   _cellDepthOfView->Enable(enable);
   _cellDepthOfView->Refresh();
   if (enable)
   {
      ost << wxT("setparams({\"CELL_VIEW_DEPTH\", \"")
          << _cellDepthOfView->getValue()
          << wxT("\"});");
   }
   else
   {
      ost << wxT("setparams({\"CELL_VIEW_DEPTH\", \"ALL\"});");
   }
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnCellDov (wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"CELL_VIEW_DEPTH\", \"")
       << cmdEvent.GetInt()
       << wxT("\"});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnCellDab (wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"CELL_DEPTH_ALPHA_EBB\", \"")
       << cmdEvent.GetInt()
       << wxT("\"});");
   TpdPost::parseCommand(ost);

}

void tui::TopedPropertySheets::RenderingPSheet::OnImageDetail (wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"MIN_VISUAL_AREA\", \"")
       << cmdEvent.GetInt()
       << wxT("\"});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnGrcBlinkOn(wxCommandEvent& cmdevent)
{
   wxString ost;
   bool enable = (1 == cmdevent.GetInt());
   _cbGrcBlinkFreq->Enable(enable);
   _cbGrcBlinkFreq->Refresh();
   if (enable)
   {
      ost << wxT("setparams({\"GRC_BLINK_PERIOD\", \"")
          << _cbGrcBlinkFreq->getValue()
          << wxT("\"});");
   }
   else
   {
      ost << wxT("setparams({\"GRC_BLINK_PERIOD\", \"0\"});");
   }
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::OnGrcBlinkFreq (wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"GRC_BLINK_PERIOD\", \"")
       << cmdEvent.GetInt()
       << wxT("\"});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::RenderingPSheet::update(wxCommandEvent& evt)
{
   wxWindow* targetControl;
   switch (evt.GetId())
   {
      case RPS_CELL_MARK    :
         targetControl = FindWindow(PDSET_CELLMARK);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case RPS_CELL_BOX     :
         targetControl = FindWindow(PDSET_CELLBOX );assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case RPS_TEXT_MARK   :
         targetControl = FindWindow(PDSET_TEXTMARK);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt());
         break;
      case RPS_TEXT_BOX     :
         targetControl = FindWindow(PDSET_TEXTBOX );assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt());
         break;
      case RPS_TEXT_ORI     :
         targetControl = FindWindow(PDSET_TEXTORI );assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt());
         break;
      case RPS_VISI_LIMIT   :
         _imageDetail->setValue(evt.GetInt());
         break;
      case RPS_CELL_DAB     :
         _cellDepthEbb->setValue(evt.GetInt());
         break;
      case RPS_CELL_DOV     :
         if (0 != evt.GetInt())
            _cellDepthOfView->setValue(evt.GetInt());
         break;
      case RPS_LD_FONT      :
         targetControl = FindWindow(PDSET_TEXTFONTS);assert(targetControl);
         static_cast<wxComboBox*>(targetControl)->Append(evt.GetString() );
         break;
      case RPS_SLCT_FONT    :
         targetControl = FindWindow(PDSET_TEXTFONTS);assert(targetControl);
         static_cast<wxComboBox*>(targetControl)->SetStringSelection(evt.GetString() );
         break;
      case RPS_GRC_PERIOD   :
         if (0 == evt.GetInt())
         {
            _cbGrcBlinkFreq->Enable(false);
            _cbGrcBlinkOn->SetValue(false);
            Toped->view()->setBlinkInterval(0);
         }
         else
         {
            _cbGrcBlinkFreq->setValue(evt.GetInt());
            _cbGrcBlinkOn->SetValue(true);
            Toped->view()->setBlinkInterval(100*evt.GetInt());
         }
         break;
      default: assert(false);
   }
}

//=============================================================================
BEGIN_EVENT_TABLE(tui::TopedPropertySheets::CanvasPSheet, wxPanel)
    EVT_TEXT_ENTER(CDMARKER_STEP    ,tui::TopedPropertySheets::CanvasPSheet::OnMarkerStep    )
    EVT_BUTTON    (CDMARKER_STEP_SET,tui::TopedPropertySheets::CanvasPSheet::OnMarkerStep    )
    EVT_RADIOBOX(CDMARKER_MOTION    ,tui::TopedPropertySheets::CanvasPSheet::OnMarkerMotion  )
    EVT_RADIOBOX(CDRECOVER_POLY     ,tui::TopedPropertySheets::CanvasPSheet::OnRecoverPoly   )
    EVT_RADIOBOX(CDRECOVER_WIRE     ,tui::TopedPropertySheets::CanvasPSheet::OnRecoverWire   )
    EVT_CHECKBOX(CDGRID_CBOX1       ,tui::TopedPropertySheets::CanvasPSheet::OnGridOn1       )
    EVT_CHECKBOX(CDGRID_CBOX2       ,tui::TopedPropertySheets::CanvasPSheet::OnGridOn2       )
    EVT_CHECKBOX(CDGRID_CBOX3       ,tui::TopedPropertySheets::CanvasPSheet::OnGridOn3       )
    EVT_CHECKBOX(CDMISC_LONGCURSOR  ,tui::TopedPropertySheets::CanvasPSheet::OnLongCorsor    )
    EVT_CHECKBOX(CDMISC_AUTOPAN     ,tui::TopedPropertySheets::CanvasPSheet::OnAutoPan       )
    EVT_CHECKBOX(CDMISC_BOLDONHOOVER,tui::TopedPropertySheets::CanvasPSheet::OnBoldOnHoover  )
    EVT_CHECKBOX(CDMISC_ZEROCROSS   ,tui::TopedPropertySheets::CanvasPSheet::OnZeroCross     )
    EVT_TEXT_ENTER(CDGRID_SET1      ,tui::TopedPropertySheets::CanvasPSheet::OnGridSet1      )
    EVT_TEXT_ENTER(CDGRID_SET2      ,tui::TopedPropertySheets::CanvasPSheet::OnGridSet2      )
    EVT_TEXT_ENTER(CDGRID_SET3      ,tui::TopedPropertySheets::CanvasPSheet::OnGridSet3      )
    EVT_BUTTON    (CDGRID_ENTER1    ,tui::TopedPropertySheets::CanvasPSheet::OnGridSet1      )
    EVT_BUTTON    (CDGRID_ENTER2    ,tui::TopedPropertySheets::CanvasPSheet::OnGridSet2      )
    EVT_BUTTON    (CDGRID_ENTER3    ,tui::TopedPropertySheets::CanvasPSheet::OnGridSet3      )
END_EVENT_TABLE()

tui::TopedPropertySheets::CanvasPSheet::CanvasPSheet(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
   wxBoxSizer* topSizer = DEBUG_NEW wxBoxSizer(wxVERTICAL);
      // Image details (Quality)
      wxBoxSizer *markerSizer  = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Marker"));

         wxBoxSizer *stepSizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            wxStaticText* stepStatic = DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Step:"));
            wxTextCtrl*   stepValue  = DEBUG_NEW wxTextCtrl  (this, CDMARKER_STEP, wxT(""),
                                                              wxDefaultPosition,
                                                              wxDefaultSize,
                                                              wxTE_RIGHT | wxTE_PROCESS_ENTER,
                                                              wxTextValidator(wxFILTER_NUMERIC));
            wxButton*     stepEnter = DEBUG_NEW wxButton(this, CDMARKER_STEP_SET, wxT("Set"));
         stepSizer->Add(stepStatic, 0, wxTOP | wxBOTTOM | wxLEFT  | wxALIGN_CENTER | wxEXPAND, 5);
         stepSizer->Add(stepValue , 1, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);
         stepSizer->Add(stepEnter , 0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);

         static const wxString motionOptions[] = { wxT("Free"), wxT("45 deg"), wxT("90 deg") };
         wxRadioBox* motion = DEBUG_NEW wxRadioBox(this, CDMARKER_MOTION, wxT("Motion"),
                                                   wxDefaultPosition,
                                                   wxDefaultSize,
                                                   WXSIZEOF(motionOptions),
                                                   motionOptions ,
                                                   1,
                                                   wxRA_VERTICAL
                                                   );
      markerSizer->Add(stepSizer, 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      markerSizer->Add(motion   , 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      //
      wxBoxSizer *gridSizer = DEBUG_NEW wxStaticBoxSizer(wxVERTICAL, this, wxT("Grid Step"));
         wxBoxSizer *gr1Sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            wxCheckBox* gr1CBox = DEBUG_NEW wxCheckBox(this, CDGRID_CBOX1, wxT("Set 1:"));
            wxTextCtrl* gr1TBox = DEBUG_NEW wxTextCtrl(this, CDGRID_SET1, wxT(""),
                                                       wxDefaultPosition,
                                                       wxDefaultSize,
                                                       wxTE_RIGHT | wxTE_PROCESS_ENTER,
                                                       wxTextValidator(wxFILTER_NUMERIC));
            wxButton*  gr1Enter = DEBUG_NEW wxButton(this, CDGRID_ENTER1, wxT("Set"));

            gr1Sizer->Add(gr1CBox, 0, wxTOP | wxBOTTOM | wxLEFT  | wxALIGN_CENTER | wxEXPAND, 5);
            gr1Sizer->Add(gr1TBox ,1, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);
            gr1Sizer->Add(gr1Enter,0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);

         wxBoxSizer *gr2Sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            wxCheckBox* gr2CBox = DEBUG_NEW wxCheckBox(this, CDGRID_CBOX2, wxT("Set 2:"));
            wxTextCtrl* gr2TBox = DEBUG_NEW wxTextCtrl(this, CDGRID_SET2, wxT(""),
                                                       wxDefaultPosition,
                                                       wxDefaultSize,
                                                       wxTE_RIGHT | wxTE_PROCESS_ENTER,
                                                       wxTextValidator(wxFILTER_NUMERIC));
            wxButton*  gr2Enter = DEBUG_NEW wxButton(this, CDGRID_ENTER2, wxT("Set"));

            gr2Sizer->Add(gr2CBox, 0, wxTOP | wxBOTTOM | wxLEFT  | wxALIGN_CENTER | wxEXPAND, 5);
            gr2Sizer->Add(gr2TBox ,1, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);
            gr2Sizer->Add(gr2Enter,0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);

         wxBoxSizer *gr3Sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
            wxCheckBox* gr3CBox = DEBUG_NEW wxCheckBox(this, CDGRID_CBOX3, wxT("Set 3:"));
            wxTextCtrl* gr3TBox = DEBUG_NEW wxTextCtrl(this, CDGRID_SET3, wxT(""),
                                                       wxDefaultPosition,
                                                       wxDefaultSize,
                                                       wxTE_RIGHT | wxTE_PROCESS_ENTER,
                                                       wxTextValidator(wxFILTER_NUMERIC));
            wxButton*  gr3Enter = DEBUG_NEW wxButton(this, CDGRID_ENTER3, wxT("Set"));

            gr3Sizer->Add(gr3CBox, 0, wxTOP | wxBOTTOM | wxLEFT  | wxALIGN_CENTER | wxEXPAND, 5);
            gr3Sizer->Add(gr3TBox ,1, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);
            gr3Sizer->Add(gr3Enter,0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER | wxEXPAND, 5);

      gridSizer->Add(gr1Sizer, 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      gridSizer->Add(gr2Sizer, 0, wxALL | wxALIGN_CENTER | wxEXPAND);
      gridSizer->Add(gr3Sizer, 0, wxALL | wxALIGN_CENTER | wxEXPAND);

      wxBoxSizer *recoverySizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Invalid Input Objects"));
         static const wxString recoOptions[] = { wxT("Reject"), wxT("Recover")};
         wxRadioBox* polyReco = DEBUG_NEW wxRadioBox(this, CDRECOVER_POLY, wxT("Polygons"),
                                                     wxDefaultPosition,
                                                     wxDefaultSize,
                                                     WXSIZEOF(recoOptions),
                                                     recoOptions ,
                                                     1,
                                                     wxRA_HORIZONTAL
                                                    );
         wxRadioBox* wireReco = DEBUG_NEW wxRadioBox(this, CDRECOVER_WIRE, wxT("Wires"),
                                                     wxDefaultPosition,
                                                     wxDefaultSize,
                                                     WXSIZEOF(recoOptions),
                                                     recoOptions ,
                                                     1,
                                                     wxRA_HORIZONTAL
                                                    );
         recoverySizer->Add(polyReco   , 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
         recoverySizer->AddStretchSpacer(0);
         recoverySizer->Add(wireReco   , 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);


      wxCheckBox* longCursor   = DEBUG_NEW wxCheckBox(this, CDMISC_LONGCURSOR  , wxT("Long Cursor"));
      wxCheckBox* autoPan      = DEBUG_NEW wxCheckBox(this, CDMISC_AUTOPAN     , wxT("Auto Pan"));
      wxCheckBox* boldOnHover  = DEBUG_NEW wxCheckBox(this, CDMISC_BOLDONHOOVER, wxT("Highlight on hover"));
      wxCheckBox* zeroCross    = DEBUG_NEW wxCheckBox(this, CDMISC_ZEROCROSS   , wxT("Mark the (0,0) position"));

   // Pack everything
   topSizer->Add( markerSizer   , 0, wxEXPAND | wxALL, 2);
   topSizer->Add( gridSizer     , 0, wxEXPAND | wxALL, 2);
   topSizer->Add( recoverySizer , 0, wxEXPAND | wxALL, 2);
   topSizer->Add( longCursor    , 0, wxEXPAND | wxALL, 2);
   topSizer->Add( autoPan       , 0, wxEXPAND | wxALL, 2);
   topSizer->Add( boldOnHover   , 0, wxEXPAND | wxALL, 2);
   topSizer->Add( zeroCross     , 0, wxEXPAND | wxALL, 2);
//
   SetSizer(topSizer);
   topSizer->Fit(this);
}

void tui::TopedPropertySheets::CanvasPSheet::OnMarkerStep(wxCommandEvent& WXUNUSED(event))
{
   wxWindow* targetControl;
   targetControl = FindWindow(CDMARKER_STEP);assert(targetControl);
   wxString contents = static_cast<wxTextCtrl*>(targetControl)->GetValue( );

   wxString ost;
   ost << wxT("step(")
       << contents
       << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnMarkerMotion(wxCommandEvent& cmdEvent)
{
   byte intVal;
   switch (cmdEvent.GetInt())
   {
      case 0: intVal =  0; break;
      case 1: intVal = 45; break;
      case 2: intVal = 90; break;
      default: assert(false);
   }
   wxString ost;
   ost << wxT("shapeangle(")
       << intVal
       << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnGridOn1(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("grid(0, ")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnGridOn2(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("grid(1, ")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnGridOn3(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("grid(2, ")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT(");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnGridSet1(wxCommandEvent& WXUNUSED(event))
{
   wxWindow* targetControl;
   targetControl = FindWindow(CDGRID_SET1);assert(targetControl);
   wxString contents = static_cast<wxTextCtrl*>(targetControl)->GetValue( );

   wxString ost;
   ost << wxT("definegrid(0,")
       << contents
       << wxT(",\"white\");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnGridSet2(wxCommandEvent& WXUNUSED(event))
{
   wxWindow* targetControl;
   targetControl = FindWindow(CDGRID_SET2);assert(targetControl);
   wxString contents = static_cast<wxTextCtrl*>(targetControl)->GetValue( );

   wxString ost;
   ost << wxT("definegrid(1,")
       << contents
       << wxT(",\"white\");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnGridSet3(wxCommandEvent& WXUNUSED(event))
{
   wxWindow* targetControl;
   targetControl = FindWindow(CDGRID_SET3);assert(targetControl);
   wxString contents = static_cast<wxTextCtrl*>(targetControl)->GetValue( );

   wxString ost;
   ost << wxT("definegrid(2,")
       << contents
       << wxT(",\"white\");");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnRecoverPoly (wxCommandEvent& cmdEvent)
{
   bool boolVal;
   switch (cmdEvent.GetInt())
   {
      case 0: boolVal = false ; break;
      case 1: boolVal = true  ; break;
      default: assert(false);
   }
   wxString ost;
   ost << wxT("setparams({\"RECOVER_POLY\",")
       << (boolVal ? wxT("\"true\"") : wxT("\"false\""))
       << wxT("});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnRecoverWire (wxCommandEvent& cmdEvent)
{
   bool boolVal;
   switch (cmdEvent.GetInt())
   {
      case 0: boolVal = false ; break;
      case 1: boolVal = true  ; break;
      default: assert(false);
   }
   wxString ost;
   ost << wxT("setparams({\"RECOVER_WIRE\",")
       << (boolVal ? wxT("\"true\"") : wxT("\"false\""))
       << wxT("});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnLongCorsor(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("longcursor( ")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT(" );");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnAutoPan(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("autopan( ")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT(" );");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnBoldOnHoover(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("setparams({\"HIGHLIGHT_ON_HOVER\", \"")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT("\"});");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::OnZeroCross(wxCommandEvent& cmdEvent)
{
   wxString ost;
   ost << wxT("zerocross( ")
       << (cmdEvent.GetInt() ? wxT("true") : wxT("false"))
       << wxT(" );");
   TpdPost::parseCommand(ost);
}

void tui::TopedPropertySheets::CanvasPSheet::update(wxCommandEvent& evt)
{
   wxWindow* targetControl;
   switch (evt.GetId())
   {
      case CPS_MARKER_STEP:
         targetControl = FindWindow(CDMARKER_STEP);assert(targetControl);
         static_cast<wxTextCtrl*>(targetControl)->SetValue(evt.GetString());
         break;
      case CPS_MARKER_MOTION:
         targetControl = FindWindow(CDMARKER_MOTION);assert(targetControl);
         switch (evt.GetInt())
         {
            case  0: static_cast<wxRadioBox*>(targetControl)->SetSelection(0); break;
            case 45: static_cast<wxRadioBox*>(targetControl)->SetSelection(1); break;
            case 90: static_cast<wxRadioBox*>(targetControl)->SetSelection(2); break;
            default: assert(false);
         }
         break;
      case CPS_GRID0_ON     :
         targetControl = FindWindow(CDGRID_CBOX1);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case CPS_GRID1_ON     :
         targetControl = FindWindow(CDGRID_CBOX2);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case CPS_GRID2_ON     :
         targetControl = FindWindow(CDGRID_CBOX3);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case CPS_GRID0_STEP :
         targetControl = FindWindow(CDGRID_SET1);assert(targetControl);
         static_cast<wxTextCtrl*>(targetControl)->SetValue(evt.GetString() );
         break;
      case CPS_GRID1_STEP :
         targetControl = FindWindow(CDGRID_SET2);assert(targetControl);
         static_cast<wxTextCtrl*>(targetControl)->SetValue(evt.GetString() );
         break;
      case CPS_GRID2_STEP :
         targetControl = FindWindow(CDGRID_SET3);assert(targetControl);
         static_cast<wxTextCtrl*>(targetControl)->SetValue(evt.GetString() );
         break;
      case CPS_RECOVER_POLY :
         targetControl = FindWindow(CDRECOVER_POLY);assert(targetControl);
         static_cast<wxRadioBox*>(targetControl)->SetSelection(evt.GetInt());
         break;
      case CPS_RECOVER_WIRE :
         targetControl = FindWindow(CDRECOVER_WIRE);assert(targetControl);
         static_cast<wxRadioBox*>(targetControl)->SetSelection(evt.GetInt());
         break;
      case CPS_LONG_CURSOR    :
         targetControl = FindWindow(CDMISC_LONGCURSOR);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case CPS_AUTOPAN   :
         targetControl = FindWindow(CDMISC_AUTOPAN);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      case CPS_BOLD_ON_HOVER :
         targetControl = FindWindow(CDMISC_BOLDONHOOVER); assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt());
         break;
      case CPS_ZERO_CROSS :
         targetControl = FindWindow(CDMISC_ZEROCROSS);assert(targetControl);
         static_cast<wxCheckBox*>(targetControl)->SetValue(evt.GetInt() );
         break;
      default: assert(false);
   }
}

//Return brush for corresponding color and filling 
wxBrush* tui::makeBrush(const byte* ifill, const layprop::tellRGB col)
{
   wxBrush *brush;
   wxBitmap *stipplebrush = DEBUG_NEW wxBitmap((char  *)ifill, 32, 32, 1);
   wxImage image;
   image = stipplebrush->ConvertToImage();
#ifdef WIN32
   //Change white color for current one


   int w = image.GetWidth();
   int h = image.GetHeight();
   for (int i=0; i<w; i++)
      for (int j=0; j<h; j++)
      {
         if((image.GetRed(i,j)==0) && (image.GetGreen(i,j)==0) && (image.GetBlue(i,j)==0))
         {
            image.SetRGB(i, j, col.red(), col.green(), col.blue());
         }
         else
         {
            image.SetRGB(i, j, 0, 0, 0);
         }
      }
   delete stipplebrush;
   //Recreate bitmap with new color
   stipplebrush = DEBUG_NEW wxBitmap(image, 1);
#endif
   brush = DEBUG_NEW wxBrush(   *stipplebrush);
   delete stipplebrush;
   return brush;
}
