
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
extern const wxEventType         wxEVT_CMD_BROWSER;

BEGIN_EVENT_TABLE(tui::TechEditorDialog, wxDialog)
   EVT_LISTBOX(ID_TE_LAYER       , tui::TechEditorDialog::onLayerSelected) 
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

tui::TechEditorDialog::TechEditorDialog( wxWindow* parent, wxWindowID id)//, const wxString& title, const wxPoint& pos, const wxSize& size, long style )  
:wxDialog( parent, id, wxT("Technology Editor"), wxDefaultPosition, wxSize(900, 300)) , _curSelect(0)
{
   wxSize size = GetSize();
	wxBoxSizer *sizer1= DEBUG_NEW wxBoxSizer(wxVERTICAL);
      wxBoxSizer *sizer2 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
         wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Layers") );
         _layerList = DEBUG_NEW wxListBox(this, ID_TE_LAYER, wxDefaultPosition, wxSize(size.x/2, size.y-30));
            hsizer0->Add(_layerList, 1, wxEXPAND, 0);
         wxBoxSizer *vsizer0 = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Properties") );
            wxBoxSizer *hsizer4 = DEBUG_NEW wxBoxSizer( wxHORIZONTAL);
               wxBoxSizer *hsizer5 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Layer Number") );
                  _layerNumber = DEBUG_NEW wxTextCtrl( this, ID_BTN_TE_NUM , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                             wxTextValidator(wxFILTER_NUMERIC, &_layerNumberString));
                  hsizer5->Add(_layerNumber, 0, wxALL | wxEXPAND, 5);
               wxBoxSizer *hsizer6 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Layer Name") );
                  _layerName = DEBUG_NEW wxTextCtrl( this, ID_BTN_TE_NUM , wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT,
                                             wxTextValidator(wxFILTER_ASCII , &_layerNameString));
                  _newLayerButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWLAYER, wxT("New"), wxDefaultPosition, wxDefaultSize);
                  hsizer6->Add(_layerName, 0, wxALL | wxEXPAND, 5);
                  hsizer6->Add(_newLayerButton, 0, wxALL | wxEXPAND, 5);
               hsizer4->Add(hsizer5);
               hsizer4->Add(hsizer6);
            _layerColors = DEBUG_NEW ColorListComboBox();
            wxBoxSizer *hsizer1 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Color") );
               _layerColors->Create(this,COLOR_COMBO ,wxEmptyString,
                   wxDefaultPosition, wxSize(size.x/3, 30),
                  NULL,
                  wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
                  );
               hsizer1->Add(_layerColors, 0, wxALL | wxEXPAND, 5);
               hsizer1->Add(0,0,1);
               _newColorButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWCOLOR, wxT("New"), wxDefaultPosition, wxDefaultSize);
               hsizer1->Add(_newColorButton, 0, wxALL | wxEXPAND, 5);
               hsizer1->SetSizeHints(this);
            
            _layerFills = DEBUG_NEW FillListComboBox();
            wxBoxSizer *hsizer2 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Fill") );
               _layerFills->Create(this,FILL_COMBO,wxEmptyString,
                   wxDefaultPosition, wxSize(size.x/3, 30),
                  NULL,
                  wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
                  );
               hsizer2->Add(_layerFills, 0, wxALL | wxEXPAND, 5);
               hsizer2->Add(0,0,1);
               _newFillButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWFILL, wxT("New"), wxDefaultPosition, wxDefaultSize);
               hsizer2->Add(_newFillButton, 0, wxALL | wxEXPAND, 5);
               hsizer2->SetSizeHints(this);

            _layerLines = DEBUG_NEW  LineListComboBox();

            wxBoxSizer *hsizer3 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Line Style") );
               _layerLines->Create(this, LINE_COMBO,wxEmptyString,
                   wxDefaultPosition, wxSize(size.x/3, 30),
                  NULL,
                  wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
                  );
               hsizer3->Add(_layerLines, 0, wxALL | wxEXPAND, 5);
               hsizer3->Add(0,0,1);
               _newStyleButton = DEBUG_NEW wxButton(this, tui::BT_TECH_NEWSTYLE, wxT("New"), wxDefaultPosition, wxDefaultSize);
               hsizer3->Add(_newStyleButton, 0, wxALL | wxEXPAND, 5);
               hsizer3->SetSizeHints(this);

         vsizer0->Add(hsizer4);
         vsizer0->Add(hsizer1);
         vsizer0->Add(hsizer2);
         vsizer0->Add(hsizer3);
      sizer2->Add(hsizer0, 1, wxEXPAND, 0);
      sizer2->Add(vsizer0, 1, wxEXPAND, 0);
      wxBoxSizer *sizer3 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
      _applyButton  = DEBUG_NEW wxButton(this, tui::BT_TECH_APPLY, wxT("Apply"));
      _cancelButton = DEBUG_NEW wxButton(this, wxID_CANCEL, wxT("Cancel"));
      sizer3->Add(_applyButton, 1, wxEXPAND|wxBOTTOM, 3);
      sizer3->Add(_cancelButton, 1, wxEXPAND|wxBOTTOM, 3);
   sizer1->Add(sizer2);
   sizer1->Add(sizer3);

   this->SetSizerAndFit(sizer1);
   sizer1->SetSizeHints( this );

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


void  tui::TechEditorDialog::onLayerSelected(wxCommandEvent&)
{
   _curSelect = _layerList->GetSelection();

   updateDialog(_curSelect);
   FindWindow(BT_TECH_APPLY)->Enable(false);
}

void  tui::TechEditorDialog::updateDialog(int selectNum)
{   
   wxString layerName = _layerList->GetString(selectNum);
   std::string layName = std::string(layerName.mb_str(wxConvUTF8));
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      word lay_no = drawProp->getLayerNo(layName);
      //Set Layer Number
       wxString tempStr;
       tempStr<<lay_no;
      _layerNumber->SetValue(tempStr);

      //Set active layer name
      _layerName->SetValue(layerName);
      //Set active color
      std::string colorName = drawProp->getColorName(lay_no);
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
      std::string fillName = drawProp->getFillName(lay_no);
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
      std::string lineName = drawProp->getLineName(lay_no);
      int lineNumber = _layerLines->FindString(wxString(lineName.c_str(), wxConvUTF8));
      if (lineNumber != wxNOT_FOUND)
      {
         _layerLines->SetSelection(lineNumber);
      }
      else
      {
         tell_log(console::MT_WARNING, "There is no appropriate line style");
      }

      _curLayNo      = lay_no;
      _curLayerName  = layName;
      _curColorName  = colorName;
      _curFillName   = fillName;
      _curLineName   = lineName;
   }
   PROPC->unlockDrawProp(drawProp);

}

void tui::TechEditorDialog::updateData()
{
   prepareLayers();
   prepareColors();
   prepareFills();
   prepareLines();
}

void tui::TechEditorDialog::prepareLayers()
{
   _layerList->Clear();
   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      WordList lays = drawProp->getAllLayers();

      for(WordList::const_iterator it = lays.begin(); it != lays.end(); ++it)
      {
         std::string layerName = drawProp->getLayerName((*it));
         _layerList->Append(wxString(layerName.c_str(), wxConvUTF8));
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
 //  updateDialog(_curSelect);
   wxString ost;
  
   wxString s_name      = wxString(_curLayerName.c_str(), wxConvUTF8);
   wxString s_number;   s_number << _curLayNo;
   wxString s_fill      = wxString(_curFillName.c_str(), wxConvUTF8);
   wxString s_style     = wxString(_curLineName.c_str(), wxConvUTF8);
   wxString s_color     = wxString(_curColorName.c_str(), wxConvUTF8);

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
   updateDialog(_curSelect);
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
   wxBrush *brush;
   layprop::DrawProperties* drawProp;
   byte width;
   byte patscale;
   wxDash dashes[32];
   int index = 0;

   if (PROPC->lockDrawProp(drawProp))
   {
      wxString str = GetString( item );
      std::string lineName(str.mb_str(wxConvUTF8));
      line = drawProp->getLine(lineName);
      word pattern = line->pattern(); 
      width = line->width();
      patscale = line->patscale();
      enum statetype {zero, one};
      int state;


      int i = 0;

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
       dashes[index] = patscale*length;
   }
     
   wxPen pen(wxT("black"), width, wxUSER_DASH);
   pen.SetDashes(index, dashes);

   PROPC->unlockDrawProp(drawProp);
 
   dc.SetPen( pen );
   dc.DrawText(GetString( item ),
               rect.x + 5,
               (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
               );

      dc.DrawLine(r.x+r.width/2 + 5, r.y+r.height/2, r.x+r.width - 5, r.y+r.height/2);
}
