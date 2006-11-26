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

#include <math.h>
#include <wx/colordlg.h>
#include "tui.h"
#include "datacenter.h"

extern DataCenter*                DATC;

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
tui::getSize::getSize(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos )
        : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                      wxDEFAULT_DIALOG_STYLE)  {
/*   int4b Istep = View.getstep();
   real DBscale = 1/DATC->UU();
   if (Istep > DBscale) precision = 0;
   else precision = fmod
   if (fmod(1/DATC->UU(), (float)View.getstep()) != 0)
      precision = */
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = new wxBoxSizer( wxHORIZONTAL );
   _wxText = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   spin_sizer->Add(10,10,0);
   spin_sizer->Add(_wxText, 1, wxEXPAND, 0);
   spin_sizer->Add(new sgSpinButton(this, _wxText, DATC->step(), 1, 10, 2, 3), 0, 0, 0);
   spin_sizer->Add(10,10,0);
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
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
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   ws.sprintf(wxT("%.*f"), 3, init);
   _wxText = new wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
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
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   //grid 0
   wxBoxSizer *g0sizer = new wxBoxSizer( wxHORIZONTAL );
   ws.sprintf(wxT("%.*f"), 3, ig0);
   _wxGrid0 = new wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   g0sizer->Add( new wxStaticText(this, -1, wxT("Grid 0:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL, 5);
   g0sizer->Add(_wxGrid0, 0, wxEXPAND | wxALL ,5 );
   //grid 1
   wxBoxSizer *g1sizer = new wxBoxSizer( wxHORIZONTAL );
   ws.sprintf(wxT("%.*f"), 3, ig1);
   _wxGrid1 = new wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   g1sizer->Add( new wxStaticText(this, -1, wxT("Grid 1:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL, 5);
   g1sizer->Add(_wxGrid1, 0, wxEXPAND | wxALL ,5 );
   //grid 2
   wxBoxSizer *g2sizer = new wxBoxSizer( wxHORIZONTAL );
   ws.sprintf(wxT("%.*f"), 3, ig2);
   _wxGrid2 = new wxTextCtrl( this, -1, ws, wxDefaultPosition, wxDefaultSize);
   g2sizer->Add( new wxStaticText(this, -1, wxT("Grid 2:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL, 5);
   g2sizer->Add(_wxGrid2, 0, wxEXPAND | wxALL ,5 );
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
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
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _nameList = new wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      laydata::cellList const cll = ATDB->cells();
      laydata::cellList::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   DATC->unlockDB();   
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}
 
tui::getCellRef::getCellRef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, 
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _rotation = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _flip = new wxCheckBox(this, -1, wxT("Flip X"));
   _nameList = new wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::cellList const cll = ATDB->cells();
      laydata::cellList::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   DATC->unlockDB();   
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = new wxBoxSizer( wxHORIZONTAL );
   spin_sizer->Add( new wxStaticText(this, -1, wxT("Rotation:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   spin_sizer->Add(_rotation, 0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(new sgSpinButton(this, _rotation, 90, 0, 360, 0, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(0,0,1); // 
   spin_sizer->Add(_flip, 0, wxALL | wxALIGN_CENTER, 10);
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
      topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}
 
tui::getCellARef::getCellARef(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, 
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _rotation = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _flip = new wxCheckBox(this, -1, wxT("Flip X"));
   _nameList = new wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::cellList const cll = ATDB->cells();
      laydata::cellList::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   DATC->unlockDB();   
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = new wxBoxSizer( wxHORIZONTAL );
   spin_sizer->Add( new wxStaticText(this, -1, wxT("Rotation:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   spin_sizer->Add(_rotation, 0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(new sgSpinButton(this, _rotation, 90, 0, 360, 0, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(0,0,1); // 
   spin_sizer->Add(_flip, 0, wxALL | wxALIGN_CENTER, 10);
   // Now Array specific controls
   _numX = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _numY = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   wxBoxSizer* num_sizer =  new wxBoxSizer( wxHORIZONTAL );
   num_sizer->Add( new wxStaticText(this, -1, wxT("Rows:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   num_sizer->Add(_numX, 0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(new sgSpinButton(this, _numX, 1, 2, 200, 2, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(0,0,1); // 
   num_sizer->Add( new wxStaticText(this, -1, wxT("Columns:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   num_sizer->Add(_numY, 0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(new sgSpinButton(this, _numY, 1, 2, 200, 2, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   num_sizer->Add(0,0,1); // 
   //
   _stepX = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _stepY = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   wxBoxSizer* step_sizer =  new wxBoxSizer( wxHORIZONTAL );
   step_sizer->Add( new wxStaticText(this, -1, wxT("step X:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   step_sizer->Add(_stepX, 0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(new sgSpinButton(this, _stepX, DATC->step(), 2, 200, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(0,0,1); // 
   step_sizer->Add( new wxStaticText(this, -1, wxT("step Y:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   step_sizer->Add(_stepY, 0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(new sgSpinButton(this, _stepY, DATC->step(), 2, 200, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   
   step_sizer->Add(0,0,1); // 
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
      topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(num_sizer, 0, wxEXPAND );
   topsizer->Add(step_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}
 
tui::getTextdlg::getTextdlg(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos) 
           : wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)  {
   
   _size     = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _rotation = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   _flip = new wxCheckBox(this, -1, wxT("Flip X"));
   _text = new wxTextCtrl(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize);
   // The window layout
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = new wxBoxSizer( wxHORIZONTAL );
   
   spin_sizer->Add( new wxStaticText(this, -1, wxT("Size:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   spin_sizer->Add(_size, 0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(new sgSpinButton(this, _size, DATC->step(), 1, 20, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(0,0,1); // 
   
   spin_sizer->Add( new wxStaticText(this, -1, wxT("Rotation:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   spin_sizer->Add(_rotation, 0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(new sgSpinButton(this, _rotation, 90, 0, 360, 0, 0),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   spin_sizer->Add(0,0,1); // 
   spin_sizer->Add(_flip, 0, wxALL | wxALIGN_CENTER, 10);
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_text, 1, wxEXPAND | wxALIGN_CENTER, 10 );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

tui::getGDSimport::getGDSimport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _overwrite = new wxCheckBox(this, -1, wxT("Overwrite existing cells"));
   _recursive = new wxCheckBox(this, -1, wxT("Import recursively"));
   _recursive->SetValue(true);
   _nameList = new wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   GDSin::GDSFile* AGDSDB = DATC->lockGDS();
      GDSin::GDSstructure* gdss = AGDSDB->Get_structures();
      while (gdss) {
         _nameList->Append(wxString(gdss->Get_StrName(), wxConvUTF8));
         gdss = gdss->GetLast();
      }
   DATC->unlockGDS();
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = new wxBoxSizer( wxVERTICAL );
   //   spin_sizer->Add(0,0,1); //
   spin_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   spin_sizer->Add(_overwrite, 0, wxALL | wxALIGN_LEFT, 5);
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

tui::getGDSexport::getGDSexport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _recursive = new wxCheckBox(this, -1, wxT("Export recursively"));
   _nameList = new wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::cellList const cll = ATDB->cells();
      laydata::cellList::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   DATC->unlockDB();
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = new wxBoxSizer( wxVERTICAL );
   //   spin_sizer->Add(0,0,1); //
   spin_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::layset_sample, wxWindow)
   EVT_PAINT(tui::layset_sample::OnPaint)
END_EVENT_TABLE()

tui::layset_sample::layset_sample(wxWindow *parent, wxWindowID id, wxPoint pos,
   wxSize size, word init) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   setColor(init);
   setFill(init);
}

void tui::layset_sample::setColor(const layprop::tellRGB *col)
{
   if (NULL != col)
      _color.Set(col->red(), col->green(), col->blue());
   else
      _color.Set(0,0,0);
}

void tui::layset_sample::setColor(word layno)
{
   if (0 == layno)
      _color.Set(0,0,0);
   else
      setColor(DATC->getColor(layno));
}

void tui::layset_sample::setFill(const byte* fill)
{
   if (NULL != fill)
   {
      wxBitmap stipplebrush((char  *)fill, 32, 32, 1);
      _brush = wxBrush(stipplebrush);
   }
   else
   {
      _brush = wxBrush();
   }
}

void tui::layset_sample::setFill(word layno)
{
   if (0 == layno)
      _brush = wxBrush();
   else
      setFill(DATC->getFill(layno));
}

void tui::layset_sample::OnPaint(wxPaintEvent&)
{
   wxPaintDC dc(this);
   dc.SetBackground(*wxBLACK);
   _brush.SetColour(_color);
   wxPen pen(_color);
   dc.SetPen(pen);
   dc.SetBrush(_brush);
   dc.Clear();
   wxCoord w, h;
   dc.GetSize(&w, &h);
   dc.DrawRectangle(3, 3, w-6, h-6);
}

//==============================================================================
BEGIN_EVENT_TABLE(tui::defineLayer, wxDialog)
   EVT_COMBOBOX(COLOR_COMBO, tui::defineLayer::OnColorChanged)
   EVT_COMBOBOX(FILL_COMBO, tui::defineLayer::OnFillChanged  )
   EVT_COMBOBOX(LINE_COMBO, tui::defineLayer::OnLineChanged  )
//   EVT_BUTTON(COLOR_DEFINE, tui::defineLayer::OnDefineColor  )
//   EVT_BUTTON(FILL_DEFINE , tui::defineLayer::OnDefineFill   )
//   EVT_BUTTON(LINE_DEFINE , tui::defineLayer::OnDefineLine   )
END_EVENT_TABLE()

tui::defineLayer::defineLayer(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
   word init) : wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
   wxString init_color = wxT("");
   wxString init_fill = wxT("");
   if (init > 0)
   {
      _layno << init;
      _layname = wxString(DATC->getLayerName(init).c_str(), wxConvUTF8);
      init_color = wxString(DATC->getColorName(init).c_str(), wxConvUTF8);
      init_fill = wxString(DATC->getFillName(init).c_str(), wxConvUTF8);
   }
   wxTextCtrl* dwlayno    = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_layno));
   wxTextCtrl* dwlayname  = new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0,
                                          wxTextValidator(wxFILTER_ASCII, &_layname));
   if (init > 0)
   {
      dwlayno->SetEditable(false);
   }
   _sample   = new layset_sample( this, -1, wxDefaultPosition, wxDefaultSize, init);
   nameList all_names;
   wxArrayString all_strings;
   DATC->all_colors(all_names);
   for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));
   }
   _colors   = new wxComboBox( this, COLOR_COMBO, init_color, wxDefaultPosition, wxDefaultSize,all_strings, wxCB_READONLY | wxCB_SORT);
   
   all_names.clear();
   all_strings.Clear();
   DATC->all_fills(all_names);
   for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));
   }
   _fills   = new wxComboBox( this, FILL_COMBO, init_fill, wxDefaultPosition, wxDefaultSize,all_strings,wxCB_READONLY | wxCB_SORT);
   
   all_names.clear();
   all_strings.Clear();
   DATC->all_lines(all_names);
   for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      all_strings.Add(wxString(CI->c_str(), wxConvUTF8));
   }
   _lines   = new wxComboBox( this, LINE_COMBO, wxT(""), wxDefaultPosition, wxDefaultSize,all_strings,wxCB_READONLY | wxCB_SORT);
   
   // The window layout
   wxBoxSizer *line1_sizer = new wxBoxSizer( wxHORIZONTAL );
   
   line1_sizer->Add( new wxStaticText(this, -1, wxT("Number:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                1, wxALL | wxALIGN_RIGHT, 5);
   line1_sizer->Add(dwlayno, 1, wxALL | wxALIGN_CENTER, 5);
   line1_sizer->Add( new wxStaticText(this, -1, wxT("Name:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                1, wxALL | wxALIGN_RIGHT, 5);
   line1_sizer->Add(dwlayname, 2, wxALL | wxALIGN_CENTER, 5);
   //
   wxBoxSizer *col1_sizer = new wxBoxSizer( wxVERTICAL );
   col1_sizer->Add( new wxStaticText(this, -1, wxT("Color:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_RIGHT, 10);
   col1_sizer->Add( new wxStaticText(this, -1, wxT("Fill:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_RIGHT, 10);
   col1_sizer->Add( new wxStaticText(this, -1, wxT("Line:"),
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_RIGHT, 10);

   wxBoxSizer *col2_sizer = new wxBoxSizer( wxVERTICAL );
   col2_sizer->Add(_colors, 0, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   col2_sizer->Add(_fills , 0, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   col2_sizer->Add(_lines , 0, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   
/*   wxBoxSizer *col3_sizer = new wxBoxSizer( wxVERTICAL );
   col3_sizer->Add(new wxButton(this, COLOR_DEFINE, wxT("New")) , 0, wxALL, 2);
   col3_sizer->Add(new wxButton(this, FILL_DEFINE , wxT("New")) , 0, wxALL, 2);
   col3_sizer->Add(new wxButton(this, LINE_DEFINE , wxT("New")) , 0, wxALL, 2);*/
   
   wxBoxSizer *line2_sizer = new wxBoxSizer( wxHORIZONTAL );
   line2_sizer->Add(col1_sizer, 0, wxEXPAND);
   line2_sizer->Add(col2_sizer, 2, wxEXPAND);
/*   line2_sizer->Add(col3_sizer, 1, wxEXPAND);*/
   line2_sizer->Add( _sample  , 1, wxEXPAND,10);
//   line3_sizer->Add(0,0,1); //
   // Buttons
   wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
   topsizer->Add(line1_sizer, 0, wxEXPAND );
   topsizer->Add(line2_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

void tui::defineLayer::OnColorChanged(wxCommandEvent& cmdevent)
{
    wxString color_name = cmdevent.GetString();
    const layprop::tellRGB* color = DATC->getColor(std::string(color_name.fn_str()));
    _sample->setColor(color);
    _sample->Refresh();
}

// void tui::defineLayer::OnDefineColor(wxCommandEvent& cmdevent)
// {
//    nameList all_names;
//    wxColourData data;
//    DATC->all_colors(all_names);
//    word colnum = 0;
//    for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
//    {
//       const layprop::tellRGB* tell_color= DATC->getColor(*CI);
//       wxColour colour(tell_color->red(), tell_color->green(), tell_color->blue());
//       if (16 == colnum)
//       {
//          data.SetColour(colour); break;
//       }
//       else
//          data.SetCustomColour(colnum++, colour);
//    }
//   wxColourDialog dialog(this, &data);
//   if (dialog.ShowModal() == wxID_OK)
//   {
//     wxColourData retData = dialog.GetColourData();
//     wxColour col = retData.GetColour();
// //    wxBrush brush(col, wxSOLID);
// //    myWindow->SetBackground(brush);
// //    myWindow->Clear();
// //    myWindow->Refresh();
//   }
// }

void tui::defineLayer::OnFillChanged(wxCommandEvent& cmdevent)
{
    wxString fill_name = cmdevent.GetString();
    const byte* fill = DATC->getFill(std::string(fill_name.fn_str()));
    _sample->setFill(fill);
    _sample->Refresh();
}

// void tui::defineLayer::OnDefineFill(wxCommandEvent& cmdevent)
// {
// }

void tui::defineLayer::OnLineChanged(wxCommandEvent& cmdevent)
{
    wxString line = _T("Line selected -> ");
    line += cmdevent.GetString();
    wxLogMessage(line);
}

// void tui::defineLayer::OnDefineLine(wxCommandEvent& cmdevent)
// {
// }

//==============================================================================
tui::defineColor::defineColor(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
   word init) : wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)  {

}
