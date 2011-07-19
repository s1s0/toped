
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
#include "viewprop.h"
#include "techeditor.h"

extern layprop::PropertyCenter*  PROPC;

const wxString emptyFill = wxT("No Filling");

///////////////////////////////////////////////////////////////////////////
extern const wxEventType         wxEVT_CMD_BROWSER;

BEGIN_EVENT_TABLE(tui::TechEditorDialog, wxDialog)
   EVT_LISTBOX(ID_TE_LAYER, tui::TechEditorDialog::onLayerSelected) 
END_EVENT_TABLE()

tui::TechEditorDialog::TechEditorDialog( wxWindow* parent, wxWindowID id)//, const wxString& title, const wxPoint& pos, const wxSize& size, long style )  
:wxDialog( parent, id, wxT("Technology Editor"), wxDefaultPosition, wxSize(1200, 500))//, title, pos, size, style )
{
   wxSize size = GetSize();
	wxBoxSizer *sizer1= DEBUG_NEW wxBoxSizer(wxVERTICAL);
      wxBoxSizer *sizer2 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
         wxBoxSizer *hsizer0 = DEBUG_NEW wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Layers") );
         _layerList = DEBUG_NEW wxListBox(this, ID_TE_LAYER, wxDefaultPosition, wxSize(size.x/2, size.y-30));
            hsizer0->Add(_layerList, 1, wxEXPAND, 0);
         wxBoxSizer *vsizer0 = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Properties") );
            _layerColors = DEBUG_NEW ColorListComboBox();
            prepareColors();
            wxBoxSizer *vsizer1 = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Color") );
               _layerColors->Create(this,wxID_ANY,wxEmptyString,
                   wxDefaultPosition, wxSize(size.x/2, 30),
                  NULL,
                  wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
                  );
            vsizer1->Add(_layerColors);
            
            _layerFills = DEBUG_NEW FillListComboBox();
            prepareFills();
            wxBoxSizer *vsizer2 = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Fill") );
               _layerFills->Create(this,wxID_ANY,wxEmptyString,
                   wxDefaultPosition, wxSize(size.x/2, 30),
                  NULL,
                  wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
                  );
            vsizer2->Add(_layerFills);

            _layerLines = DEBUG_NEW  LineListComboBox();
            prepareLines();
            wxBoxSizer *vsizer3 = DEBUG_NEW wxStaticBoxSizer( wxVERTICAL, this, wxT("Line Style") );
               _layerLines->Create(this,wxID_ANY,wxEmptyString,
                   wxDefaultPosition, wxSize(size.x/2, 30),
                  NULL,
                  wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
                  );
            vsizer3->Add(_layerLines);

         vsizer0->Add(vsizer1);
         vsizer0->Add(vsizer2);
         vsizer0->Add(vsizer3);
      sizer2->Add(hsizer0, 1, wxEXPAND, 0);
      sizer2->Add(vsizer0, 1, wxEXPAND, 0);
      wxBoxSizer *sizer3 = DEBUG_NEW wxBoxSizer(wxHORIZONTAL);
      _applyButton  = DEBUG_NEW wxButton(this, tui::BT_TECH_APPLY, wxT("Apply"));
      _cancelButton = DEBUG_NEW wxButton(this, tui::BT_TECH_CANCEL, wxT("Cancel"));
      sizer3->Add(_applyButton, 1, wxEXPAND|wxBOTTOM, 3);
      sizer3->Add(_cancelButton, 1, wxEXPAND|wxBOTTOM, 3);
   sizer1->Add(sizer2);
   sizer1->Add(sizer3);
   prepareData();
   this->SetSizerAndFit(sizer1);
   sizer1->SetSizeHints( this );
}

tui::TechEditorDialog::~TechEditorDialog()
{
}

void  tui::TechEditorDialog::onLayerSelected(wxCommandEvent&)
{
   int selNumber = _layerList->GetSelection();
   wxString layerName = _layerList->GetString(selNumber);

   layprop::DrawProperties* drawProp;
   if (PROPC->lockDrawProp(drawProp))
   {
      word lay_no = drawProp->getLayerNo(std::string(layerName.mb_str(wxConvUTF8)));
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
      //Set active filling


   }
   PROPC->unlockDrawProp(drawProp);

   
}

void tui::TechEditorDialog::prepareData()
{
   _sizer = DEBUG_NEW wxBoxSizer(wxVERTICAL);
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
   layprop::tellRGB col(255, 255, 255, 255);
   wxColour color(col.red(), col.green(), col.blue(), col.alpha());
   const byte* ifill;

   wxRect r(rect);
   r.Deflate(3);
   r.height -= 2;
   wxBrush *brush;
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
   

   wxPen pen( color, 3, wxSOLID );

   brush->SetColour(color);
   dc.SetPen( pen );
   dc.SetBrush(*brush);
   dc.SetBackground(*wxBLACK);

   if ( !(flags & wxODCB_PAINTING_CONTROL) )
   {
      dc.DrawText(GetString( item ),
                  rect.x + 5,
                  (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
                  );
      dc.DrawRectangle(rect.x+rect.width/2 + 5, rect.y, 
                        rect.x+rect.width - 5,rect.y + rect.height);   }
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


void tui::LineListComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags ) const
{
   if ( item == wxNOT_FOUND )
            return;
   layprop::tellRGB col(255,0,0,0);
   wxColour color(col.red(), col.green(), col.blue(), col.alpha());
   const layprop::LineSettings *line;

   byte* ifill = DEBUG_NEW byte[128];
   std::fill(ifill, ifill + 127, 0);
   for(int i=0; i<=127; i++)
      ifill[i]=0;
   
   
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
      //last step
      //index++;
      dashes[index] = patscale*length;


      
      brush= tui::makeBrush(ifill, col);        
   }
     
   wxPen pen(wxT("black"), width, wxUSER_DASH);
   pen.SetDashes(index, dashes);

   PROPC->unlockDrawProp(drawProp);
 
   brush->SetColour(color);
   dc.SetPen( pen );
   dc.DrawText(GetString( item ),
               rect.x + 5,
               (rect.y - 1) + ( (rect.height/2) - dc.GetCharHeight()/2 )
               );

      dc.DrawLine(r.x+r.width/2 + 5, r.y+r.height/2, r.x+r.width - 5, r.y+r.height/2);

   delete brush;
}