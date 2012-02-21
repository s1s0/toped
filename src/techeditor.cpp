
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

wxListView* layerListPtr = NULL; //!Required by SortItem callback
///////////////////////////////////////////////////////////////////////////
//extern const wxEventType         wxEVT_CMD_BROWSER;

BEGIN_EVENT_TABLE(tui::TechEditorDialog, wxDialog)
   EVT_LIST_ITEM_FOCUSED( DTE_LAYERS_LIST  , tui::TechEditorDialog::onLayerSelected  )
   EVT_CHECKBOX         ( DTE_NEWLAYER     , tui::TechEditorDialog::OnNewLayer       )
   EVT_BUTTON           ( DTE_NEWCOLOR     , tui::TechEditorDialog::OnColorEditor    )
   EVT_BUTTON           ( DTE_NEWFILL      , tui::TechEditorDialog::OnFillEditor     )
   EVT_BUTTON           ( DTE_NEWSTYLE     , tui::TechEditorDialog::OnStyleEditor    )
   EVT_COMBOBOX         ( DTE_FILL_COMBO   , tui::TechEditorDialog::OnChangeProperty )
   EVT_COMBOBOX         ( DTE_LINE_COMBO   , tui::TechEditorDialog::OnChangeProperty )
   EVT_COMBOBOX         ( DTE_COLOR_COMBO  , tui::TechEditorDialog::OnChangeProperty )
   EVT_TEXT             ( DTE_LAYER_NUM    , tui::TechEditorDialog::OnChangeLayNum   )
   EVT_TEXT             ( DTE_LAYER_NAME   , tui::TechEditorDialog::OnChangeProperty )
   EVT_BUTTON           ( DTE_APPLY        , tui::TechEditorDialog::OnApply          )
   EVT_LIST_COL_CLICK   ( DTE_LAYERS_LIST  , tui::TechEditorDialog::OnLayListSort    )
END_EVENT_TABLE()

tui::TechEditorDialog::TechEditorDialog( wxWindow* parent, wxWindowID id) :
   wxDialog   ( parent, id, wxT("Technology Editor")),
   _curSelect (     0 )
{
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      _layerList = DEBUG_NEW wxListView(this, DTE_LAYERS_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES);
      layerListPtr = _layerList;
      prepareLayers(drawProp);
      _layerNumber = DEBUG_NEW wxTextCtrl( this, DTE_LAYER_NUM , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT | wxTE_READONLY,
                              wxTextValidator(wxFILTER_NUMERIC, &_layerNumberString));
      _layerName = DEBUG_NEW wxTextCtrl( this, DTE_LAYER_NAME , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                    wxTextValidator(wxFILTER_ASCII , &_layerNameString));

      _layerColors = DEBUG_NEW ColorListComboBox(drawProp);
      _layerColors->Create(this, DTE_COLOR_COMBO, wxEmptyString, wxDefaultPosition,wxDefaultSize , wxCB_READONLY );
      _layerFills = DEBUG_NEW FillListComboBox(drawProp);
      _layerFills->Create(this, DTE_FILL_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCB_READONLY );
      _layerLines = DEBUG_NEW  LineListComboBox(drawProp);
      _layerLines->Create(this, DTE_LINE_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCB_READONLY );
   }
   PROPC->unlockDrawProp(drawProp, false);

   wxBoxSizer *mainSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
      wxBoxSizer *propSizer = DEBUG_NEW wxBoxSizer( wxVERTICAL );
         wxBoxSizer *layDefSizer = DEBUG_NEW wxBoxSizer( wxHORIZONTAL);
            layDefSizer->Add( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Number:"), wxDefaultPosition, wxDefaultSize),
                                                         0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
            layDefSizer->Add(_layerNumber, 1, wxRIGHT | wxALIGN_CENTER, 5);
            layDefSizer->Add( DEBUG_NEW wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize),
                                                         0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
            layDefSizer->Add(_layerName, 2, wxRIGHT | wxALIGN_CENTER, 5);
            layDefSizer->Add(DEBUG_NEW wxCheckBox(this, tui::DTE_NEWLAYER , wxT("new")) , 1, wxALL | wxALIGN_CENTER | wxEXPAND);

         wxBoxSizer *colorSizer = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Color") );
            colorSizer->Add(_layerColors, 3, wxALL | wxEXPAND, 5);
            colorSizer->Add(DEBUG_NEW wxButton(this, tui::DTE_NEWCOLOR, wxT("New"   )), 1, wxALL, 5);

         wxBoxSizer *fillSizer = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Fill") );
            fillSizer->Add(_layerFills, 3, wxALL | wxEXPAND, 5);
            fillSizer->Add(DEBUG_NEW wxButton(this, tui::DTE_NEWFILL , wxT("New"   )), 1, wxALL , 5);

         wxBoxSizer *styleSizer = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Line Style") );
            styleSizer->Add(_layerLines, 3, wxALL | wxEXPAND, 5);
            styleSizer->Add(DEBUG_NEW wxButton(this, tui::DTE_NEWSTYLE, wxT("New"   )), 1, wxALL, 5);

         wxBoxSizer *buttonSizer = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(0,0,1);
            buttonSizer->Add( DEBUG_NEW wxButton(this, tui::DTE_APPLY   , wxT("Apply" )), 0, wxBOTTOM, 3);
            buttonSizer->Add( DEBUG_NEW wxButton(this, wxID_CANCEL          , wxT("Cancel")), 0, wxBOTTOM, 3);

      propSizer->Add(layDefSizer, 0, wxALL | wxEXPAND, 5);
      propSizer->Add(colorSizer , 0, wxALL | wxEXPAND, 5);
      propSizer->Add(fillSizer  , 0, wxALL | wxEXPAND, 5);
      propSizer->Add(styleSizer , 0, wxALL | wxEXPAND, 5);
      propSizer->Add(buttonSizer, 0, wxALL | wxEXPAND);
   mainSizer->Add(_layerList, 1, wxEXPAND, 0);
   mainSizer->Add(propSizer, 2, wxEXPAND, 0);

   this->SetSizerAndFit(mainSizer);
   mainSizer->SetSizeHints( this );

   FindWindow(DTE_APPLY)->Enable(false);
   _layerList->Select(_curSelect, true);
#ifdef WIN32
   updateDialog();
#endif
}

tui::TechEditorDialog::~TechEditorDialog()
{
}

void  tui::TechEditorDialog::OnNewLayer(wxCommandEvent& cmdEvent)
{
   bool newChecked = (0 != cmdEvent.GetInt());
   _layerNumber->SetEditable(newChecked);
   if (newChecked)
   {
      _layerNumberString = "";
      _layerNumber->GetValidator()->TransferToWindow();
      _layerNameString = "";
      _layerName->GetValidator()->TransferToWindow();
      _layerColors->SetSelection(0);
      _layerFills->SetSelection(0);
      _layerLines->SetSelection(0);
      _layerNumber->SetFocus();
   }
   else
   {
      updateDialog();
      _layerName->SetFocus();
   }
   FindWindow(DTE_APPLY)->Enable(false);
}

void  tui::TechEditorDialog::OnColorEditor(wxCommandEvent&)
{
   wxCommandEvent event;
   Toped->OnDefineColor(event);
   _layerColors->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      _layerColors->populate(drawProp);
   }
   PROPC->unlockDrawProp(drawProp, false);
   updateDialog();
}

void  tui::TechEditorDialog::OnFillEditor(wxCommandEvent&)
{
   wxCommandEvent event;
   Toped->OnDefineFill(event);
   _layerFills->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      _layerFills->populate(drawProp);
   }
   PROPC->unlockDrawProp(drawProp, false);
   updateDialog();
}

void  tui::TechEditorDialog::OnStyleEditor(wxCommandEvent&)
{
   wxCommandEvent event;
   Toped->OnDefineStyle(event);
   _layerLines->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      _layerLines->populate(drawProp);
   }
   PROPC->unlockDrawProp(drawProp, false);
   updateDialog();
}

void  tui::TechEditorDialog::OnChangeProperty(wxCommandEvent&)
{
   if (!static_cast<wxCheckBox*>(FindWindow(tui::DTE_NEWLAYER))->IsChecked())
      // Make sure it's not a new layer
      FindWindow(DTE_APPLY)->Enable(true);
}

void tui::TechEditorDialog::OnChangeLayNum(wxCommandEvent&)
{
   if (static_cast<wxCheckBox*>(FindWindow(tui::DTE_NEWLAYER))->IsChecked())
   {
      _layerNumber->GetValidator()->TransferFromWindow();
      unsigned long layNo;
      _layerNumberString.ToULong(&layNo);
      bool applyEnable = true;
      if (LAST_EDITABLE_LAYNUM < layNo)
         applyEnable = false;
      else
      {
         for(WordList::const_iterator it = _allLayNums.begin(); it != _allLayNums.end(); ++it)
            if (layNo == *it)
            {
               applyEnable = false;
               break;
            }
      }
      FindWindow(DTE_APPLY)->Enable(applyEnable);
   }
}


void  tui::TechEditorDialog::onLayerSelected(wxListEvent& levent)
{
   _curSelect = levent.GetIndex();
   _layerNumber->SetEditable(false);
   static_cast<wxCheckBox*>(FindWindow(tui::DTE_NEWLAYER))->SetValue(false);
   if (wxNOT_FOUND != _curSelect)
      updateDialog();
   FindWindow(DTE_APPLY)->Enable(false);
}

void  tui::TechEditorDialog::updateDialog()
{
   wxListItem row;
   row.SetId(_curSelect);
   row.SetMask(wxLIST_MASK_TEXT);
   row.SetColumn(0);
   if (!_layerList->GetItem(row)) return;
   _layerNumberString = row.GetText();
   _layerNumber->GetValidator()->TransferToWindow();
   row.SetColumn(1);
   if (!_layerList->GetItem(row)) return;
   _layerNameString = row.GetText();
   _layerName->GetValidator()->TransferToWindow();

   unsigned long layNo;
   _layerNumberString.ToULong(&layNo);

   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      //Set active color
      std::string colorName = drawProp->getColorName(layNo);
      int colorNumber = _layerColors->FindString(wxString(colorName.c_str(), wxConvUTF8));
      if (colorNumber != wxNOT_FOUND)
         _layerColors->SetSelection(colorNumber);
      else
         _layerColors->SetSelection(0);
      //Set active filling
      std::string fillName = drawProp->getFillName(layNo);
      int fillNumber = _layerFills->FindString(wxString(fillName.c_str(), wxConvUTF8));
      if (fillNumber != wxNOT_FOUND)
         _layerFills->SetSelection(fillNumber);
      else
         _layerFills->SetSelection(0);
      //Set active style
      std::string lineName = drawProp->getLineName(layNo);
      int lineNumber = _layerLines->FindString(wxString(lineName.c_str(), wxConvUTF8));
      if (lineNumber != wxNOT_FOUND)
         _layerLines->SetSelection(lineNumber);
      else
         _layerLines->SetSelection(0);
   }
   PROPC->unlockDrawProp(drawProp, false);
}

void tui::TechEditorDialog::prepareLayers(layprop::DrawProperties* drawProp)
{
   _layerList->ClearAll();
   _layerList->InsertColumn(0, wxT("No"));
   _layerList->InsertColumn(1, wxT("Name"));

   _allLayNums = drawProp->getAllLayers();
   for(WordList::const_iterator it = _allLayNums.begin(); it != _allLayNums.end(); ++it)
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

void tui::TechEditorDialog::updateLayerList()
{
   if (static_cast<wxCheckBox*>(FindWindow(tui::DTE_NEWLAYER))->IsChecked())
   { // new layer added
      unsigned long layNo;
      _layerNumberString.ToULong(&layNo);
      _allLayNums.push_back(layNo);

      wxListItem row;
      unsigned long newItem = _layerList->GetItemCount();
      row.SetMask(wxLIST_MASK_DATA | wxLIST_MASK_TEXT);
      row.SetId(newItem);
      row.SetData(newItem);

      row.SetText( _layerNumberString);
      _layerList->InsertItem(row);
      _layerList->SetColumnWidth(0, wxLIST_AUTOSIZE);
      //
      row.SetColumn(1);
      row.SetMask(wxLIST_MASK_TEXT);
      row.SetText(_layerNameString);
      _layerList->SetItem(row);
      _layerList->SetColumnWidth(1, wxLIST_AUTOSIZE);

      _layerList->SortItems(tui::wxListCtrlItemCompare, 0l);
      _curSelect = _layerList->FindItem(-1, newItem);
      _layerList->Select(_curSelect, true);
      _layerList->EnsureVisible(_curSelect);
   }
   else
   {  //existing layer changed
      wxListItem row;
      row.SetId(_curSelect);
      row.SetMask(wxLIST_MASK_TEXT);
      row.SetColumn(1);
      row.SetText(_layerNameString);
      _layerList->SetItem(row);
   }
}

void tui::TechEditorDialog::OnLayListSort(wxListEvent& cmdEvent)
{
   int col = cmdEvent.GetColumn();
   wxListItem row;
   row.SetId(_curSelect);
   row.SetMask(wxLIST_MASK_DATA);
   row.SetColumn(0);
   if (!_layerList->GetItem(row)) return;
   long itemSel = row.GetData();
   switch (col)
   {
      case 0: _layerList->SortItems(tui::wxListCtrlItemCompare, 0l);break;
      case 1: _layerList->SortItems(tui::wxListCtrlItemCompare, 1l);break;
   }
   _curSelect = _layerList->FindItem(-1, itemSel);
   _layerList->Select(_curSelect, true);
   _layerList->EnsureVisible(_curSelect);
}

void tui::TechEditorDialog::OnApply(wxCommandEvent&)
{   
   wxString ost;
  
   _layerNumber->GetValidator()->TransferFromWindow();
   _layerName->GetValidator()->TransferFromWindow();
#if wxCHECK_VERSION(2,9,0)
   wxString s_fill         = _layerFills->GetValue();
   wxString s_style        = _layerLines->GetValue();
   wxString s_color        = _layerColors->GetValue();
#else
   wxString s_fill         = _layerFills->GetStringSelection();
   wxString s_style        = _layerLines->GetStringSelection();
   wxString s_color        = _layerColors->GetStringSelection();
#endif

   unsigned long d_number;  _layerNumberString.ToULong(&d_number);

   ost   << wxT("layprop(\"")          << _layerNameString
                  << wxT("\" , ")      << d_number //wxT("\" , \"\" , ")
                  << wxT(" , \"")      << s_color
                  << wxT("\" , \"")    << s_fill
                  << wxT("\" , \"")    << s_style
                  << wxT("\");");

   Console->parseCommand(ost);

   updateLayerList();
   static_cast<wxCheckBox*>(FindWindow(tui::DTE_NEWLAYER))->SetValue(false);
   FindWindow(DTE_APPLY)->Enable(false);
}

//=============================================================================
tui::ColorListComboBox::ColorListComboBox(layprop::DrawProperties* drawProp) : wxOwnerDrawnComboBox()
{
   populate(drawProp);
}

void tui::ColorListComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND ) return;

   layprop::tellRGB col(127,127,127,127);

   std::string colorName(GetString( item ).mb_str(wxConvUTF8));
   layprop::ColorMap::const_iterator colIter = _colors.find(colorName);
   if (_colors.end() != colIter)
      col =  *(colIter->second);

   wxRect r(rect);
   r.Deflate(3);
   r.height -= 2;

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


void tui::ColorListComboBox::populate(layprop::DrawProperties* drawProp)
{
   NameList allColors;
   drawProp->allColors(allColors);
   Append(wxT(""));
   for (NameList::const_iterator CC = allColors.begin(); CC != allColors.end(); CC++)
   {
      layprop::tellRGB* coldef = DEBUG_NEW layprop::tellRGB(drawProp->getColor(*CC));
      _colors[*CC] = coldef;
      Append(wxString((*CC).c_str(), wxConvUTF8));
   }

}

void tui::ColorListComboBox::Clear()
{
   wxOwnerDrawnComboBox::Clear();
   for (layprop::ColorMap::iterator CMI = _colors.begin(); CMI != _colors.end(); CMI++)
      delete CMI->second;
}

tui::ColorListComboBox::~ColorListComboBox()
{
   Clear();
}

//=============================================================================
tui::FillListComboBox::FillListComboBox(layprop::DrawProperties* drawProp) : wxOwnerDrawnComboBox()
{
   populate( drawProp );
}

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

   wxRect r(rect);
   r.Deflate(3);
   r.height -= 2;
   wxBrush *brush = NULL;
   std::string fillname(GetString( item ).mb_str(wxConvUTF8));
   layprop::FillMap::const_iterator fillSet = _fills.find(fillname);
   //Note!- empty fill shall come naturally here, if not found among the _fills
   if (_fills.end() != fillSet)
      brush = tui::makeBrush(fillSet->second, col);
   else
      brush = DEBUG_NEW wxBrush(color, wxTRANSPARENT);
   
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

void tui::FillListComboBox::populate(layprop::DrawProperties* drawProp)
{
   NameList allColors;
   drawProp->allFills(allColors);
   Append(wxT(""));
   for (NameList::const_iterator CC = allColors.begin(); CC != allColors.end(); CC++)
   {
      byte* ptrn = DEBUG_NEW byte[128];
      memcpy(ptrn,drawProp->getFill(*CC),128);
      _fills[*CC] = ptrn;
      Append(wxString((*CC).c_str(), wxConvUTF8));
   }
}

void tui::FillListComboBox::Clear()
{
   wxOwnerDrawnComboBox::Clear();
   for (layprop::FillMap::iterator FMI = _fills.begin(); FMI != _fills.end(); FMI++)
      delete [] FMI->second;
}

tui::FillListComboBox::~FillListComboBox()
{
   Clear();
}

//=============================================================================
tui::LineListComboBox::LineListComboBox(layprop::DrawProperties* drawProp) : wxOwnerDrawnComboBox()
{
   populate( drawProp );
}

void tui::LineListComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND )
            return;
   layprop::tellRGB col(255,0,0,0);
   wxColour color(col.red(), col.green(), col.blue(), col.alpha());

   wxRect r(rect);

   std::string lineName(GetString( item ).mb_str(wxConvUTF8));

   layprop::LineMap::const_iterator lineSet = _lines.find(lineName);
   if (_lines.end() != lineSet)
   {
      const layprop::LineSettings *line = lineSet->second;
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
}

void tui::LineListComboBox::populate(layprop::DrawProperties* drawProp)
{
   NameList allColors;
   drawProp->allLines(allColors);
   Append(wxT(""));
   for (NameList::const_iterator CC = allColors.begin(); CC != allColors.end(); CC++)
   {
      _lines[*CC] = DEBUG_NEW layprop::LineSettings(*drawProp->getLine(*CC));
      Append(wxString((*CC).c_str(), wxConvUTF8));
   }
}

void tui::LineListComboBox::Clear()
{
   wxOwnerDrawnComboBox::Clear();
   for (layprop::LineMap::iterator LMI = _lines.begin(); LMI != _lines.end(); LMI++)
      delete LMI->second;
}

tui::LineListComboBox::~LineListComboBox()
{
   Clear();
}

int wxCALLBACK tui::wxListCtrlItemCompare(long item1, long item2, long column)
{
   wxListItem li1, li2;
   li1.SetMask(wxLIST_MASK_TEXT);
   li1.SetColumn(column);
   li1.SetId(layerListPtr->FindItem(-1, item1));
   layerListPtr->GetItem(li1);
   li2.SetMask(wxLIST_MASK_TEXT);
   li2.SetColumn(column);
   li2.SetId(layerListPtr->FindItem(-1, item2));
   layerListPtr->GetItem(li2);
   wxString s1 = li1.GetText();
   wxString s2 = li2.GetText();
   if (0 == column)
   {
      int result;
      unsigned long ln1, ln2;
      s1.ToULong(&ln1); s2.ToULong(&ln2);
      if      (ln1 < ln2) result = -1;
      else if (ln1 > ln2) result =  1;
      else result = 0;
      return result;
   }
   else return s1.CompareTo(s2.c_str());
}

