
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
//        Created: Wed Apr 11 2011
//     Originator:  Sergey Gaitukevich - gaitukevich.s@toped.org.uk
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

#include "tpdph.h"
#include "outbox.h"
#include "tuidefs.h"
#include "tui.h"
#include "viewprop.h"
#include "techeditor.h"
#include "toped.h"
#include "ted_prompt.h"

extern layprop::PropertyCenter*  PROPC;
extern tui::TopedFrame*          Toped;
extern console::TllCmdLine*      Console;

const wxString emptyFill = wxT("No Filling");

///////////////////////////////////////////////////////////////////////////
//extern const wxEventType         wxEVT_CMD_BROWSER;

BEGIN_EVENT_TABLE(tui::TechEditorDialog, wxDialog)
   EVT_LIST_ITEM_FOCUSED(ID_TE_LAYER       , tui::TechEditorDialog::onLayerSelected)
   EVT_BUTTON(BT_TECH_NEWLAYER   , tui::TechEditorDialog::OnLayerEditor )
   EVT_BUTTON(BT_TECH_NEWCOLOR   , tui::TechEditorDialog::OnColorEditor )
   EVT_BUTTON(BT_TECH_NEWFILL    , tui::TechEditorDialog::OnFillEditor )
   EVT_BUTTON(BT_TECH_NEWSTYLE   , tui::TechEditorDialog::OnStyleEditor )
   EVT_COMBOBOX(FILL_COMBO       , tui::TechEditorDialog::OnChangeProperty)
   EVT_COMBOBOX(LINE_COMBO       , tui::TechEditorDialog::OnChangeProperty)
   EVT_COMBOBOX(COLOR_COMBO      , tui::TechEditorDialog::OnChangeProperty)
   EVT_TEXT(ID_BTN_TE_NUM        , tui::TechEditorDialog::OnChangeProperty )
   EVT_TEXT(ID_BTN_TE_NAME       , tui::TechEditorDialog::OnChangeProperty )
   EVT_BUTTON(BT_TECH_APPLY      , tui::TechEditorDialog::OnApply )
END_EVENT_TABLE()

tui::TechEditorDialog::TechEditorDialog( wxWindow* parent, wxWindowID id) :
   wxDialog   ( parent, id, wxT("Technology Editor")),
   _curSelect (     0 )
{
//   wxSize size = GetSize();
   _layerList = DEBUG_NEW wxListView(this, ID_TE_LAYER, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES);
   _layerNumber = DEBUG_NEW wxTextCtrl( this, ID_BTN_TE_NUM , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                              wxTextValidator(wxFILTER_NUMERIC, &_layerNumberString));
   _layerName = DEBUG_NEW wxTextCtrl( this, ID_BTN_TE_NUM , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                    wxTextValidator(wxFILTER_ASCII , &_layerNameString));
   _layerColors = DEBUG_NEW ColorListComboBox();
   _layerColors->Create(this, COLOR_COMBO, wxEmptyString, wxDefaultPosition,wxDefaultSize /*wxSize(size.x/3, 30)*/, wxCB_READONLY );
   _layerFills = DEBUG_NEW FillListComboBox();
   _layerFills->Create(this, FILL_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize/*wxSize(size.x/3, 30)*/, wxCB_READONLY );
   _layerLines = DEBUG_NEW  LineListComboBox();
   _layerLines->Create(this, LINE_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize/*wxSize(size.x/3, 30)*/, wxCB_READONLY );
   _newColorButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWCOLOR, wxT("New"   ));
   _newFillButton  = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWFILL , wxT("New"   ));
   _newStyleButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWSTYLE, wxT("New"   ));
   _newLayerButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWLAYER, wxT("New"   ));
   _applyButton    = DEBUG_NEW wxButton(this, tui::BT_TECH_APPLY   , wxT("Apply" ));
   _cancelButton   = DEBUG_NEW wxButton(this, wxID_CANCEL          , wxT("Cancel"));

   wxBoxSizer *topSizer= DEBUG_NEW wxBoxSizer(wxVERTICAL);
      wxBoxSizer *mainSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
         wxBoxSizer *propSizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );

            wxBoxSizer *layDefSizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL);
               layDefSizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Number:"), wxDefaultPosition, wxDefaultSize),
                                                            0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
               layDefSizer->Add(_layerNumber, 1, wxRIGHT | wxALIGN_CENTER, 5);
               layDefSizer->Add( DEBUG_NEW wxStaticText(this, -1, wxT("Name:"), wxDefaultPosition, wxDefaultSize),
                                                            0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
               layDefSizer->Add(_layerName, 2, wxRIGHT | wxALIGN_CENTER, 5);
            wxBoxSizer *colorSizer = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Color") );
               colorSizer->Add(_layerColors, 3, wxALL | wxEXPAND, 5);
               colorSizer->Add(_newColorButton, 1, wxALL, 5);
            
            wxBoxSizer *fillSizer = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Fill") );
               fillSizer->Add(_layerFills, 3, wxALL | wxEXPAND, 5);
               fillSizer->Add(_newFillButton, 1, wxALL , 5);

            wxBoxSizer *styleSizer = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Line Style") );
               styleSizer->Add(_layerLines, 3, wxALL | wxEXPAND, 5);
               styleSizer->Add(_newStyleButton, 1, wxALL, 5);

         propSizer->Add(layDefSizer, 0, wxALL | wxEXPAND, 5);
         propSizer->Add(colorSizer, 0, wxALL | wxEXPAND, 5);
         propSizer->Add(fillSizer, 0, wxALL | wxEXPAND, 5);
         propSizer->Add(styleSizer, 0, wxALL | wxEXPAND, 5);
      mainSizer->Add(_layerList, 1, wxEXPAND, 0);
      mainSizer->Add(propSizer, 2, wxEXPAND, 0);

      wxBoxSizer *buttonSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
         buttonSizer->Add(0,0,1);
         buttonSizer->Add(_newLayerButton, 0, wxBOTTOM, 3);
         buttonSizer->Add(0,0,3);
         buttonSizer->Add(_applyButton   , 0, wxBOTTOM, 3);
         buttonSizer->Add(_cancelButton  , 0, wxBOTTOM, 3);
   topSizer->Add(mainSizer, 0, wxEXPAND);
   topSizer->Add(buttonSizer, 0, wxEXPAND);

   this->SetSizerAndFit(topSizer);
   topSizer->SetSizeHints( this );

   FindWindow(BT_TECH_APPLY)->Enable(false);
   updateData();
}

tui::TechEditorDialog::~TechEditorDialog()
{
}

void  tui::TechEditorDialog::OnLayerEditor(wxCommandEvent&)
{
   wxCommandEvent event;
   Toped->OnDefineLayer(event);
   updateData();
}

void  tui::TechEditorDialog::OnColorEditor(wxCommandEvent&)
{
   //TpdPost::postMenuEvent(tui::TMSET_DEFCOLOR);
   wxCommandEvent event;
   Toped->OnDefineColor(event);
   updateData();
}

void  tui::TechEditorDialog::OnFillEditor(wxCommandEvent&)
{
   wxCommandEvent event;
   Toped->OnDefineFill(event);
   updateData();
}

void  tui::TechEditorDialog::OnStyleEditor(wxCommandEvent&)
{
   wxCommandEvent event;
   Toped->OnDefineStyle(event);
   updateData();
}

void  tui::TechEditorDialog::OnChangeProperty(wxCommandEvent&)
{
   FindWindow(BT_TECH_APPLY)->Enable(true);
}


void  tui::TechEditorDialog::onLayerSelected(wxListEvent& levent)
{
   _curSelect = levent.GetIndex();
//   _curSelect = _layerList->GetFirstSelected();
   if (wxNOT_FOUND != _curSelect)
      updateDialog(_curSelect);
   FindWindow(BT_TECH_APPLY)->Enable(false);
}

void  tui::TechEditorDialog::updateDialog(int selectNum)
{
   wxListItem row;
   row.SetId(selectNum);
   //   row.SetColumn(0);
   if (!_layerList->GetItem(row)) return;
   wxString layNum = row.GetText();
   row.SetColumn(1);
   if (!_layerList->GetItem(row)) return;
   wxString layerName = row.GetText();
   _layerNumber->SetValue(layNum);
   _layerName->SetValue(layerName);
   unsigned long layNo;
   layNum.ToULong(&layNo);

   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      //Set active color
      std::string colorName = drawProp->getColorName(layNo);
      int colorNumber = _layerColors->FindString(wxString(colorName.c_str(), wxConvUTF8));
      if (colorNumber != wxNOT_FOUND)
      {
         _layerColors->SetSelection(colorNumber);
      }
      else
      {
         tell_log(console::MT_WARNING, "There is no appropriate color");
      }
      //Set active filling
      std::string fillName = drawProp->getFillName(layNo);
      int fillNumber = _layerFills->FindString(wxString(fillName.c_str(), wxConvUTF8));
      if (fillNumber != wxNOT_FOUND)
      {
         _layerFills->SetSelection(fillNumber);
      }
      else
      {
         fillNumber = _layerFills->FindString(emptyFill);
         _layerFills->SetSelection(fillNumber);
      }
      //Set active style
      std::string lineName = drawProp->getLineName(layNo);
      int lineNumber = _layerLines->FindString(wxString(lineName.c_str(), wxConvUTF8));
      if (lineNumber != wxNOT_FOUND)
      {
         _layerLines->SetSelection(lineNumber);
      }
      else
      {
         tell_log(console::MT_WARNING, "There is no appropriate line style");
      }
   }
   PROPC->unlockDrawProp(drawProp);

}

void tui::TechEditorDialog::updateData()
{
   int currentSelection = _curSelect;
   prepareLayers();
   _layerList->Select(currentSelection, true);
   _curSelect = currentSelection;
   prepareColors();
   prepareFills();
   prepareLines();
}

void tui::TechEditorDialog::prepareLayers()
{
   _layerList->ClearAll();
   _layerList->InsertColumn(0, wxT("No"));
   _layerList->InsertColumn(1, wxT("Name"));

   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      WordList lays = drawProp->getAllLayers();
      for(WordList::const_iterator it = lays.begin(); it != lays.end(); ++it)
      {
         wxListItem row;
         row.SetMask(wxLIST_MASK_DATA | wxLIST_MASK_TEXT);
         row.SetId(_layerList->GetItemCount());
         row.SetData(_layerList->GetItemCount());

         wxString dummy;
         dummy << *it;
         row.SetText( dummy);
         _layerList->InsertItem(row);
         _layerList->SetColumnWidth(0, wxLIST_AUTOSIZE);
         //
         row.SetColumn(1);
         row.SetMask(wxLIST_MASK_TEXT);
         row.SetText(wxString(drawProp->getLayerName(*it).c_str(), wxConvUTF8));
         _layerList->SetItem(row);
         _layerList->SetColumnWidth(1, wxLIST_AUTOSIZE);
         //
      }
   }
   PROPC->unlockDrawProp(drawProp);
}

void tui::TechEditorDialog::prepareColors()
{
   _layerColors->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      NameList all_names;
      drawProp->allColors(all_names);
      for(NameList::const_iterator it=all_names.begin(); it!=all_names.end(); ++it)
      {
         _layerColors->Append(wxString((*it).c_str(), wxConvUTF8));
      }
   }
   PROPC->unlockDrawProp(drawProp);
}

void tui::TechEditorDialog::prepareFills()
{
   _layerFills->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      NameList all_names;
      drawProp->allFills(all_names);
      for(NameList::const_iterator it=all_names.begin(); it!=all_names.end(); ++it)
      {
         _layerFills->Append(wxString((*it).c_str(), wxConvUTF8));
      }
      _layerFills->Append(emptyFill);
   }
   PROPC->unlockDrawProp(drawProp);
}

void tui::TechEditorDialog::prepareLines()
{
   _layerLines->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      NameList all_lines;
      drawProp->allLines(all_lines);

      for(NameList::const_iterator it=all_lines.begin(); it!=all_lines.end(); ++it)
      {
         _layerLines->Append(wxString((*it).c_str(), wxConvUTF8));
      }
   }
   PROPC->unlockDrawProp(drawProp);
}

void tui::TechEditorDialog::OnApply(wxCommandEvent&)
{   
   wxString ost;
  
   wxString s_name         = _layerName->GetValue();
   wxString s_number       = _layerNumber->GetValue();

#if wxCHECK_VERSION(2,9,0)
   wxString s_fill         = static_cast<wxTextEntry*>(_layerFills)->GetStringSelection();
   wxString s_style        = static_cast<wxTextEntry*>(_layerLines)->GetStringSelection();
   wxString s_color        = static_cast<wxTextEntry*>(_layerColors)->GetStringSelection();
#else
   wxString s_fill         = _layerFills->GetStringSelection();
   wxString s_style        = _layerLines->GetStringSelection();
   wxString s_color        = _layerColors->GetStringSelection();
#endif

   unsigned long d_number;  s_number.ToULong(&d_number);

   //If there is no filling clear s_fill
   if (!s_fill.Cmp(emptyFill))
   {
      s_fill = wxT("");
   }
   ost   << wxT("layprop(\"") << s_name
                  << wxT("\" , ")      << d_number //wxT("\" , \"\" , ")
                  << wxT(" , \"")     << s_color
                  << wxT("\" , \"")        << s_fill
                  << wxT("\" , \"")        << s_style
                  << wxT("\");");

   Console->parseCommand(ost);
   updateData();
   FindWindow(BT_TECH_APPLY)->Enable(false);
   updateDialog(_curSelect); // TODO remove this from here! We need an event from the second thread when the properties had been changed
}

void tui::ColorListComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND )
            return;
   layprop::tellRGB col(0,0,0,0);

   wxRect r(rect);
   r.Deflate(3);
   r.height -= 2;

   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      std::string colorName(GetString( item ).mb_str(wxConvUTF8));
      col   = drawProp->getColor(colorName);
   }
   PROPC->unlockDrawProp(drawProp);
   
   wxColour color(col.red(), col.green(), col.blue(), col.alpha());
   wxPen pen( color, 3, wxSOLID );

   wxBrush brush(color, wxSOLID);
   dc.SetPen( pen );
   dc.SetBrush(brush);

   if ( !(flags & wxODCB_PAINTING_CONTROL) )
   {
      dc.DrawText(GetString( item ),
                  rect.x + 5,
                  (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
                  );
      dc.DrawRectangle(r.x+r.width/2 + 5, r.y, 
                        r.x+r.width - 5, r.y+r.height);
   }
   else
   {
      dc.DrawText(GetString( item ),
                  rect.x + 5,
                  (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
                  );
      dc.DrawRectangle(r.x+r.width/2 + 5, r.y, 
                        r.x+r.width - 5, r.y+r.height);
   }
}

/*void tui::ColorListComboBox::OnDrawBackground(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND )
            return;
   wxRect r(rect);
   r.Deflate(3);
   r.height -= 2;

   int penStyle = wxSOLID;
   wxPen pen( dc.GetTextForeground(), 3, penStyle );
   // Get text colour as pen colour
   dc.SetPen( pen );

   if ( !(flags & wxODCB_PAINTING_CONTROL) )
   {
      dc.DrawText(GetString( item ),
                  r.x + 3,
                  (r.y + 0) + ( (r.height/2) - dc.GetCharHeight() )/2
                  );

      dc.DrawLine( r.x+r.width/2 + 5, r.y+((r.height/4)*3), r.x+r.width - 5, r.y+((r.height/4)*3) );
   }
   else
   {
      dc.DrawLine( r.x+5, r.y+r.height/2, r.x+r.width - 5, r.y+r.height/2 );
   }
}*/


void tui::FillListComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND )
            return;
#ifdef WIN32
   layprop::tellRGB col(255, 255, 255, 255);
#else
   layprop::tellRGB col(0, 0, 0, 255);// Black
#endif
   wxColour color(col.red(), col.green(), col.blue(), col.alpha());
   const byte* ifill;

   wxRect r(rect);
   r.Deflate(3);
   r.height -= 2;
   wxBrush *brush = NULL;
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      wxString str = GetString( item );
      std::string fillname(str.mb_str(wxConvUTF8));
      if(str.Cmp(emptyFill)) 
      {
         ifill   = drawProp->getFill(fillname);
         brush= tui::makeBrush(ifill, col);
      }
      else
      {
         brush = DEBUG_NEW wxBrush(color, wxTRANSPARENT);
      }
   }
   PROPC->unlockDrawProp(drawProp);
   
   if (NULL != brush)
   {
      wxPen pen( color, 2, wxSOLID );

      brush->SetColour(color);
      dc.SetPen( pen );
      dc.SetBrush(*brush);
      //dc.SetBackground(*wxBLACK);
      //dc.Clear();

      if ( !(flags & wxODCB_PAINTING_CONTROL) )
      {
         dc.DrawText(GetString( item ),
                     rect.x + 5,
                     (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
                     );
         dc.DrawRectangle(rect.x+rect.width/2 + 5, rect.y,
                           rect.x+rect.width - 5,rect.y + rect.height);
      }
      else
      {
         dc.DrawText(GetString( item ),
                     rect.x + 5,
                     (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
                     );
         dc.DrawRectangle(rect.x+rect.width/2 + 5, rect.y,
                           rect.x+rect.width - 5, rect.y+rect.height);
      }
      delete brush;
   }
}

//void tui::FillListComboBox::OnDrawBackground(wxDC& dc, const wxRect& rect, int /*item*/, int /*flags */) const
//{
//   layprop::tellRGB col(0, 0, 0, 255);// Black
//   wxColour color(col.red(), col.green(), col.blue(), col.alpha());
//
//   wxBrush brush(color    , wxSOLID);
//   wxPen   pen  ( color, 2, wxSOLID );
//   dc.SetPen  ( pen );
//   dc.SetBrush(brush);
//
//   dc.DrawRectangle( rect.x + rect.width/2 + 5, rect.y             ,
//                     rect.x + rect.width   - 5, rect.y + rect.height );
//}

void tui::LineListComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND )
            return;
   layprop::tellRGB col(255,0,0,0);
   wxColour color(col.red(), col.green(), col.blue(), col.alpha());
   const layprop::LineSettings *line;
   wxRect r(rect);
   layprop::DrawProperties* drawProp;

   if (PROPC->lockDrawProp(drawProp))
   {
      wxString str = GetString( item );
      std::string lineName(str.mb_str(wxConvUTF8));
      line = drawProp->getLine(lineName);
      word pattern = line->pattern(); 
      byte width = line->width();
      byte pathscale = line->patscale();

      wxDash* dashes = NULL;
      unsigned numElements= makePenDash(pattern, pathscale, dashes);

      wxPen pen(wxT("black"), width, wxUSER_DASH);
//      pen.SetWidth(width);
      pen.SetDashes(numElements, dashes);

      dc.SetPen( pen );
      dc.DrawText(GetString( item ),
                  rect.x + 5,
                  (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
                  );

      dc.DrawLine(r.x+r.width/2 + 5, r.y+r.height/2, r.x+r.width - 5, r.y+r.height/2);

      pen.SetDashes(0, NULL);
      delete [] dashes;

   }
   PROPC->unlockDrawProp(drawProp);
}
