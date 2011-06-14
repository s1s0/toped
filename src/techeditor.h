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
//           $URL: https://toped.googlecode.com/svn/trunk/src/techeditor.h $
//        Created: Wed Apr 11 2011
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Technology Editor 
//                 
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 1714 $
//          $Date: 2011-01-09 01:21:00 +0800 (��, 09 ��� 2011) $
//        $Author: gaitukevich.s $
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
///////////////////////////////////////////////////////////////////////////////
/// Class ColorListComboBox
///////////////////////////////////////////////////////////////////////////////
class ColorListComboBox : public wxOwnerDrawnComboBox
{
   public:
      virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
      //virtual void OnDrawBackground( wxDC& dc, const wxRect& rect, int item, int flags ) const;
};


///////////////////////////////////////////////////////////////////////////////
/// Class FillingListComboBox
///////////////////////////////////////////////////////////////////////////////
class FillListComboBox : public wxOwnerDrawnComboBox
{
   public:
      virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
      //virtual void OnDrawBackground( wxDC& dc, const wxRect& rect, int item, int flags ) const;
};


///////////////////////////////////////////////////////////////////////////////
/// Class TechEditorDialog
///////////////////////////////////////////////////////////////////////////////
class TechEditorDialog : public wxDialog 
{
	protected:
      //wxScrolledWindow*     _layerPanel;
      wxListBox*              _layerList;
      wxOwnerDrawnComboBox*   _layerColors;
      wxOwnerDrawnComboBox*   _layerFills;
      wxButton*	            _applyButton;
      wxButton*	            _cancelButton;
      wxBoxSizer*	            _sizer;
      wxArrayString           _colorItems; 

      void        prepareColors();
      void        prepareFills();
      void        prepareData();
   public:
                  TechEditorDialog( wxWindow* parent,wxWindowID id = wxID_ANY);
                  ~TechEditorDialog();
            void  onLayerSelected(wxCommandEvent&);
      DECLARE_EVENT_TABLE()

};
}

#endif //__TECHEDITOR__
