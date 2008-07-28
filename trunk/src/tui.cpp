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
#include <wx/colordlg.h>
#include <wx/regex.h>
#include "tui.h"
#include "../tpd_DB/datacenter.h"

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
tui::getSize::getSize(wxFrame *parent, wxWindowID id, const wxString &title,
      wxPoint pos, real step, byte precision  ) : wxDialog(parent, id, title,
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
   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _wxText, step, 1, 10, 2, precision), 0, 0, 0);
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
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::tdtdesign* ATDB = DATC->lockDB(false);
      laydata::cellList const cll = ATDB->cells();
      laydata::cellList::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   DATC->unlockDB();   
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
   DATC->lockDB();
      laydata::LibCellLists *cll = DATC->getCells(ALL_LIB);
      for (laydata::LibCellLists::iterator curlib = cll->begin(); curlib != cll->end(); curlib++)
      {
         laydata::cellList::const_iterator CL;
         for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
         {
            _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
         }
      }
      delete cll;
   DATC->unlockDB();   
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
   DATC->lockDB();
      laydata::LibCellLists *cll = DATC->getCells(ALL_LIB);
      for (laydata::LibCellLists::iterator curlib = cll->begin(); curlib != cll->end(); curlib++)
      {
         laydata::cellList::const_iterator CL;
         for (CL = (*curlib)->begin(); CL != (*curlib)->end(); CL++)
         {
            _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
         }
      }
      delete cll;
   DATC->unlockDB();   
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
   step_sizer->Add(DEBUG_NEW sgSpinButton(this, _stepX, DATC->step(), 2, 200, 2, 3),
                                                  0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(0,0,1); // 
   step_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("step Y:"), 
                              wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT),
                                                0, wxALL | wxALIGN_CENTER, 10);
   step_sizer->Add(_stepY, 0, wxALL | wxALIGN_CENTER, 0);
   step_sizer->Add(DEBUG_NEW sgSpinButton(this, _stepY, DATC->step(), 2, 200, 2, 3),
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
   spin_sizer->Add(DEBUG_NEW sgSpinButton(this, _size, DATC->step(), 1, 20, 2, 3),
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
   laydata::tdtlibdir* LDR = DATC->TEDLIB();

   for (int curlib = 1; curlib < LDR->getLastLibRefNo(); curlib++)
   {
      _nameList->Append(wxString(LDR->getLibName(curlib).c_str(), wxConvUTF8));
   }

   DATC->unlockDB();
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
tui::getGDSimport::getGDSimport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  
{
   _overwrite = DEBUG_NEW wxCheckBox(this, -1, wxT("Overwrite existing cells"));
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Import recursively"));
   _recursive->SetValue(true);
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   GDSin::GDSFile* AGDSDB = DATC->lockGDS();
      GDSin::GDSstructure* gdss = AGDSDB->Get_structures();
      while (gdss) 
      {
         _nameList->Append(wxString(gdss->Get_StrName(), wxConvUTF8));
         gdss = gdss->GetLast();
      }
   DATC->unlockGDS();
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //   spin_sizer->Add(0,0,1); //
   spin_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   spin_sizer->Add(_overwrite, 0, wxALL | wxALIGN_LEFT, 5);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(_nameList, 1, wxEXPAND );
   topsizer->Add(spin_sizer, 0, wxEXPAND );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getCIFimport::getCIFimport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  
{
//   _overwrite = DEBUG_NEW wxCheckBox(this, -1, wxT("Overwrite existing cells"));
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Import recursively"));
   _recursive->SetValue(true);
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   CIFin::CifFile* ACIFDB = DATC->lockCIF();
      CIFin::CifStructure* cifs = ACIFDB->getFirstStructure();
      while (cifs) 
      {
         _nameList->Append(wxString(cifs->cellName().c_str(), wxConvUTF8));
         cifs = cifs->last();
      }
      _nameList->Append(wxString(ACIFDB->getTopStructure()->cellName().c_str(), wxConvUTF8));
      //-----------------------------------------------------------------------
      NMap inlays;
      nameList cifLayers;
      if (DATC->CIFgetLay(cifLayers))
      {
         word laynum = 1;
         for (nameList::iterator NLI = cifLayers.begin(); NLI != cifLayers.end(); NLI++)
         {
            inlays[*NLI] = laynum++;
         }
      }

   DATC->unlockCIF();
   if (init != wxT("")) _nameList->SetStringSelection(init,true);

//   tui::LayerRecord* _laylist = DEBUG_NEW tui::LayerRecord(this, std::string("boza"), 100);


   tui::LayerList* _laylist = DEBUG_NEW tui::LayerList(this, wxID_ANY, wxDefaultPosition, wxSize(100,100), inlays);
   _laylist->SetScrollbars( 10, 10, 50, 50 );

   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *lists_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   lists_sizer->Add(_nameList, 1, wxEXPAND );
   lists_sizer->Add(_laylist, 1, wxEXPAND);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
   // TOP sizer
   topsizer->Add(lists_sizer, 0, wxEXPAND | wxALIGN_CENTER );
   topsizer->Add(button_sizer, 0, wxEXPAND | wxALIGN_CENTER );

   SetSizer( topsizer );      // use the sizer for layout

   topsizer->SetSizeHints( this );   // set size hints to honour minimum size
}

//==============================================================================
tui::getGDSexport::getGDSexport(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,
      wxString init) : wxDialog(parent, id, title, pos, wxDefaultSize,
                                                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)  {
   _recursive = DEBUG_NEW wxCheckBox(this, -1, wxT("Export recursively"));
   _nameList = DEBUG_NEW wxListBox(this, -1, wxDefaultPosition, wxSize(-1,300));
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::cellList const cll = ATDB->cells();
      laydata::cellList::const_iterator CL;
      for (CL = cll.begin(); CL != cll.end(); CL++) {
         _nameList->Append(wxString(CL->first.c_str(), wxConvUTF8));
      }
   DATC->unlockDB();
   if (init != wxT("")) _nameList->SetStringSelection(init,true);
   // The window layout
   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   // First line up the important things
   wxBoxSizer *spin_sizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   //   spin_sizer->Add(0,0,1); //
   spin_sizer->Add(_recursive, 0, wxALL | wxALIGN_LEFT, 5);
   // Buttons
   wxBoxSizer *button_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 10 );
   button_sizer->Add(0,0,1); // 
   button_sizer->Add( DEBUG_NEW wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 10 );
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
   appears to have some strange "default" behaviour - the dash size is scaled
   with the line width. To avoid this one have to draw with line width 1 multiple
   times. The speed is not an issue here - so the only remaining thing is the
   strange looking code.
*/
tui::layset_sample::layset_sample(wxWindow *parent, wxWindowID id, wxPoint pos,
   wxSize size, word init) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   setColor(init);
   setFill(init);
   setLine(init);
   _selected = false;
}

void tui::layset_sample::setColor(const layprop::tellRGB& col)
{
   _color.Set(col.red(), col.green(), col.blue());
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

void tui::layset_sample::setFill(word layno)
{
   if (0 == layno)
      _brush = wxBrush();
   else
      setFill(DATC->getFill(layno));
}

void tui::layset_sample::setLine(word layno)
{
   if (0 == layno)
      _pen = wxPen();
   else
      setLine(DATC->getLine(layno));
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
   word init) : wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
   wxString init_color = wxT("");
   wxString init_fill = wxT("");
   wxString init_line = wxT("");
   if (init > 0)
   {
      _layno << init;
      _layname = wxString(DATC->getLayerName(init).c_str(), wxConvUTF8);
      init_color = wxString(DATC->getColorName(init).c_str(), wxConvUTF8);
      init_fill = wxString(DATC->getFillName(init).c_str(), wxConvUTF8);
      init_line = wxString(DATC->getLineName(init).c_str(), wxConvUTF8);
   }
   bool no_color = (wxT("") == init_color);
   bool no_fill = (wxT("") == init_fill);
   bool no_line = (wxT("") == init_line);
   wxTextCtrl* dwlayno    = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_layno));
   wxTextCtrl* dwlayname  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0,
                                          wxTextValidator(wxFILTER_ASCII, &_layname));

   /*if (init > 0)
   {
      dwlayno->SetEditable(false);
   }*/
   _sample   = DEBUG_NEW layset_sample( this, -1, wxDefaultPosition, wxDefaultSize, init);
   nameList all_names;
   wxArrayString all_strings;
   DATC->all_colors(all_names);
	if (!all_names.empty())
	{
		for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
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
	DATC->all_fills(all_names);
	if (!all_names.empty())
	{
		for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
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
	DATC->all_lines(all_names);
	if (!all_names.empty())
	{
		for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
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
   wxBoxSizer *line1_sizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
   line1_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Number:"), wxDefaultPosition, wxDefaultSize),
                                                0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
   line1_sizer->Add(dwlayno, 1, wxRIGHT | wxALIGN_CENTER, 5);
   line1_sizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Name:"),  wxDefaultPosition, wxDefaultSize),
                                                0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
   line1_sizer->Add(dwlayname, 2, wxRIGHT | wxALIGN_CENTER, 5);
   //
   wxBoxSizer *color_sizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Color"));
   color_sizer->Add(_colors, 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   color_sizer->Add(DEBUG_NEW wxCheckBox(this, ID_CBDEFCOLOR, wxT("default")), 0, wxALL | wxALIGN_RIGHT, 5);
   if (no_color)
   {
      static_cast<wxCheckBox*>(FindWindow(ID_CBDEFCOLOR))->SetValue(true);
      _colors->Enable(false);
   }
   wxBoxSizer *fill_sizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Pattern"));
   fill_sizer->Add(_fills , 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
   fill_sizer->Add(DEBUG_NEW wxCheckBox(this, ID_CBDEFPATTERN, wxT("default")), 0, wxALL | wxALIGN_RIGHT, 5);
   if (no_fill)
   {
      static_cast<wxCheckBox*>(FindWindow(ID_CBDEFPATTERN))->SetValue(true);
      _fills->Enable(false);
   }
   wxBoxSizer *line_sizer = DEBUG_NEW wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Selected Line"));
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
   wxBoxSizer *col3_sizer = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Sample") );
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
   const layprop::tellRGB color = DATC->getColor(std::string(color_name.mb_str()));
   _sample->setColor(color);

	//Next 2 strings need for Windows version
   const byte* fill = DATC->getFill(std::string(_fillname.mb_str()));
   _sample->setFill(fill);

	_sample->Refresh();
}

void tui::defineLayer::OnFillChanged(wxCommandEvent& cmdevent)
{
   _fillname = cmdevent.GetString();
   const byte* fill = DATC->getFill(std::string(_fillname.mb_str()));
   _sample->setFill(fill);
   _sample->Refresh();
}

void tui::defineLayer::OnLineChanged(wxCommandEvent& cmdevent)
{
   wxString line_name = cmdevent.GetString();
   const layprop::LineSettings* line = DATC->getLine(std::string(line_name.mb_str()));
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
      _sample->setColor(DATC->getColor(std::string("")));
   else
      _sample->setColor(DATC->getColor(std::string(_colors->GetStringSelection().mb_str())));
   _sample->Refresh();
}

void tui::defineLayer::OnDefaultPattern(wxCommandEvent& cmdevent)
{
   bool selected = (0 != cmdevent.GetInt());
   _fills->Enable(!selected);
   const byte* fill;
   if (selected)
      fill = DATC->getFill(std::string(""));
   else
      fill = DATC->getFill(std::string(_fills->GetStringSelection().mb_str()));
   _sample->setFill(fill);
   _sample->Refresh();
}

void tui::defineLayer::OnDefaultLine(wxCommandEvent& cmdevent)
{
   bool selected = (0 != cmdevent.GetInt());
   _lines->Enable(!selected);
   const layprop::LineSettings* line;
   if (selected)
      line = DATC->getLine(std::string(""));
   else
      line = DATC->getLine(std::string(_lines->GetStringSelection().mb_str()));
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
   wxSize size, std::string init) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   setColor(DATC->getColor(init));
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

tui::defineColor::defineColor(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos) :
      wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	std::string init_color;
   nameList all_names;
   DATC->all_colors(all_names);
   _colorList = DEBUG_NEW wxListBox(this, ID_ITEMLIST, wxDefaultPosition, wxSize(150,200), 0, NULL, wxLB_SORT);
	if (!all_names.empty())
	{
		init_color = *(all_names.begin());
	}
	
	for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      _colorList->Append(wxString(CI->c_str(), wxConvUTF8));
      _allColors[*CI] = DEBUG_NEW layprop::tellRGB(DATC->getColor(*CI));
   }
   _dwcolname  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxSize(150,-1), 0,
                                          wxTextValidator(wxFILTER_ASCII, &_colname));
   _colorsample = DEBUG_NEW color_sample( this, -1, wxDefaultPosition, wxSize(-1,50), init_color);
   
   _c_red    = DEBUG_NEW wxTextCtrl( this, ID_REDVAL , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_red));
   _c_green  = DEBUG_NEW wxTextCtrl( this, ID_GREENVAL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_green));
   _c_blue   = DEBUG_NEW wxTextCtrl( this, ID_BLUEVAL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_blue));
   _c_alpha  = DEBUG_NEW wxTextCtrl( this, ID_ALPHAVAL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                           wxTextValidator(wxFILTER_NUMERIC, &_alpha));
   
   wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("New Color") );
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

   wxBoxSizer *vsizer3 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Edit Color") );
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
   nameList all_names;
   wxColourData data;
   DATC->all_colors(all_names);
   word colnum = 0;
   const layprop::tellRGB* tell_color;
   for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
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
   const layprop::tellRGB* scol = getColor(std::string(color_name.mb_str()));
   
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

void tui::defineColor::OnColorPropChanged(wxCommandEvent& WXUNUSED(cmdevent))
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

void tui::defineColor::OnColorNameAdded(wxCommandEvent& WXUNUSED(cmdevent))
{
   wxString color_name = _dwcolname->GetValue();
   nameNormalize(color_name);
   if ((wxT("") == color_name) || (wxT(" ") == color_name))
   {
      wxString msg;
      msg << wxT("Empty color name.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else if (_allColors.end() != _allColors.find(std::string(color_name.mb_str())))
   {
      wxString msg;
      msg << wxT("Color \"") << color_name << wxT("\" is already defined.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else
   {
      layprop::tellRGB* newcol = DEBUG_NEW layprop::tellRGB(0,0,0,178);
      std::string s_newcol = std::string(color_name.mb_str());
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
   std::string ss_name(s_name.mb_str());
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
   wxSize size, std::string init) : wxWindow(parent, id, pos, size, wxSUNKEN_BORDER)
{
   setFill(DATC->getFill(init));
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
   EVT_LISTBOX(ID_ITEMLIST     , tui::defineFill::OnFillSelected   )
    EVT_BUTTON(ID_BTNEDIT      , tui::defineFill::OnDefineFill     )
    EVT_BUTTON(ID_NEWITEM      , tui::defineFill::OnFillNameAdded  )
END_EVENT_TABLE()

tui::defineFill::defineFill(wxFrame *parent, wxWindowID id, const wxString &title, wxPoint pos) :
      wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
   nameList all_names;
   DATC->all_fills(all_names);
   _fillList = DEBUG_NEW wxListBox(this, ID_ITEMLIST, wxDefaultPosition, wxSize(150,200), 0, NULL, wxLB_SORT);
   std::string init_color; 
	if (!all_names.empty())
	{
		init_color = *(all_names.begin());
	}
	for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
   {
      _fillList->Append(wxString(CI->c_str(), wxConvUTF8));
      byte* pat = DEBUG_NEW byte[128];
      fillcopy(DATC->getFill(*CI), pat);
      _allFills[*CI] = pat;
   }
   _dwfilname  = DEBUG_NEW wxTextCtrl( this, -1, wxT(""), wxDefaultPosition, wxSize(150,-1), 0,
                                          wxTextValidator(wxFILTER_ASCII, &_filname));
   _fillsample = DEBUG_NEW fill_sample( this, -1, wxDefaultPosition, wxSize(-1,150), init_color);
   
   wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("New Fill") );
   hsizer0->Add( _dwfilname   , 0, wxALL | wxEXPAND, 5);
   hsizer0->Add(0,0,1); //
   hsizer0->Add( DEBUG_NEW wxButton( this, ID_NEWITEM  , wxT("Add")    ), 0, wxALL, 5 );
   
//   wxBoxSizer *hsizer5 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );
//   hsizer5->Add(DEBUG_NEW wxButton( this, ID_BTNEDIT  , wxT(" Define ") ), 0, wxALL | wxALIGN_RIGHT, 5);
   
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

   wxBoxSizer *vsizer3 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Edit Pattern") );
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
    fillcopy(getFill(std::string(fill_name.mb_str())),_current_pattern);
   _fillsample->setFill(_current_pattern);
   _fillsample->Refresh();
}

void tui::defineFill::OnFillNameAdded(wxCommandEvent& WXUNUSED(cmdevent))
{
   wxString fill_name = _dwfilname->GetValue();
   nameNormalize(fill_name);
   if ((wxT("") == fill_name) || (wxT(" ") == fill_name))
   {
      wxString msg;
      msg << wxT("Empty fill name.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else if (_allFills.end() != _allFills.find(std::string(fill_name.mb_str())))
   {
      wxString msg;
      msg << wxT("Pattern \"") << fill_name << wxT("\" is already defined.");
      wxMessageBox( msg, wxT( "Error" ), wxOK | wxICON_ERROR, this );
   }
   else
   {
      std::string s_newcol = std::string(fill_name.mb_str());
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
      std::string ss_name(s_name.mb_str());
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


//==========================================================================
tui::LayerRecord::LayerRecord( wxWindow *parent, std::string name, word layno, wxArrayString& all_strings) : wxPanel(parent)
{
   _layname = wxString(name.c_str(), wxConvUTF8);
   _layno.sprintf(wxT("%.3d"), layno);

   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL );

   wxTextCtrl* dwciflay  = DEBUG_NEW wxTextCtrl( this, -1, _layname, wxDefaultPosition, wxDefaultSize, 0,
         wxTextValidator(wxFILTER_ASCII, &_layname));

   wxComboBox* dwtpdlays = DEBUG_NEW wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, all_strings, wxCB_SORT);
   std::string ics = DATC->getLayerName(layno);
   dwtpdlays->SetStringSelection(wxString(ics.c_str(), wxConvUTF8));

//   wxTextCtrl* dwlayno    = DEBUG_NEW wxTextCtrl( this, -1, _layno, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
//         wxTextValidator(wxFILTER_NUMERIC, &_layno));

   topsizer->Add(dwciflay, 2, wxRIGHT | wxALIGN_CENTER, 5);
   topsizer->Add(dwtpdlays, 2, wxRIGHT | wxALIGN_CENTER, 5);

   SetSizer( topsizer );
   topsizer->SetSizeHints( this );
}

//--------------------------------------------------------------------------
BEGIN_EVENT_TABLE(tui::LayerList, wxScrolledWindow)
   EVT_SIZE( tui::LayerList::OnSize )
END_EVENT_TABLE()

tui::LayerList::LayerList(wxWindow* parent, wxWindowID id, wxPoint pnt, wxSize sz, const NMap& inlays) :
      wxScrolledWindow(parent, id, pnt, sz)
{
   // collect all defined layers
   nameList all_names;
   DATC->all_layers(all_names);
   wxArrayString all_strings;
	for( nameList::const_iterator CI = all_names.begin(); CI != all_names.end(); CI++)
		all_strings.Add(wxString(CI->c_str(), wxConvUTF8));

   wxBoxSizer *topsizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
   for (NMap::const_iterator CNM = inlays.begin(); CNM != inlays.end(); CNM++)
   {
      tui::LayerRecord* clr = DEBUG_NEW tui::LayerRecord(this, CNM->first, CNM->second, all_strings);
      _allLayers.push_back(clr);
      topsizer->Add(clr, 2, wxALIGN_CENTER, 5);
   }
   SetSizer( topsizer );
//   SetVirtualSizeHints(wxSize(100, 100), wxSize(200, 200));
//   topsizer->SetVirtualSizeHints(this);
//   topsizer->SetSizeHints( this );
}

void tui::LayerList::OnSize( wxSizeEvent &WXUNUSED(event) )
{
   /*Comment below as well as the whole method is taken from wx samples scrollsub.cpp*/
   // We need to override OnSize so that our scrolled
   // window a) does call Layout() to use sizers for
   // positioning the controls but b) does not query
   // the sizer for their size and use that for setting
   // the scrollable area as set that ourselves by
   // calling SetScrollbar() further down.

   Layout();

   AdjustScrollbars();
}
