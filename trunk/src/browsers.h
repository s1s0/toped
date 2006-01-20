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
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Mon Aug 11 2003
//         Author: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2003 by Svilen Krustev
//    Description: Tell function definition browser, GDSII hierarchy browser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
//      Comments :
//===========================================================================

#if !defined(BROWSERS_H_INCLUDED)
#define BROWSERS_H_INCLUDED

#include <wx/notebook.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <string>
#include "../tpd_DB/tedesign.h"
#include "gds_io.h"

namespace browsers {
   typedef enum {
      BT_LAYER_DEFAULT,
      BT_LAYER_HIDE,
      BT_LAYER_LOCK,
      BT_LAYER_ADD,
      BT_LAYER_ACTION,
      BT_LAYER_DO,
      BT_LAYER_SELECTWILD,
      BT_LAYER_ACTIONWILD,
      BT_LAYER_NEW,
      BT_LAYER_EDIT,
      BT_CELL_OPEN,
      BT_CELL_HIGHLIGHT,
      BT_CELL_REF,
      BT_CELL_AREF,
      BT_CELL_ADD,
      BT_CELL_REMOVE,
      BT_ADDTDT_TAB,
      BT_ADDGDS_TAB,
      BT_CLEARGDS_TAB,
   } BROWSER_EVT_TYPE;
   
   enum {
      CellTree_OpenCell = 1000,
      GDSTree_ReportLay
   };
   //===========================================================================
   class topedlay_list : public wxListCtrl {
   public:
                     topedlay_list(wxWindow* parent, wxWindowID id = -1,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxLC_REPORT | wxLC_HRULES);
      virtual      ~topedlay_list();
      void                 addlayer(wxString, word);
      void                 defaultLayer(word, word);
      void                 hideLayer(word, bool);
      void                 lockLayer(word, bool);
   private:
      wxImageList*         _imageList;
      wxFont               _llfont_normal;
      wxFont               _llfont_bold;
   };
   
   //===========================================================================
   class layerbrowser : public wxPanel {
   public:
                           layerbrowser(wxWindow* parent, wxWindowID id);
      virtual             ~layerbrowser();
      topedlay_list*       layerlist() const {return _layerlist;};
   private:
      void                 OnNewLayer(wxCommandEvent&);
      void                 OnEditLayer(wxCommandEvent&);
      void                 OnXXXSelected(wxCommandEvent&);
      void                 OnCommand(wxCommandEvent&);
      void                 OnActiveLayer(wxListEvent&);
      void                 OnSelectWild(wxCommandEvent&);
      wxChoice*            action_select;
      wxChoice*            action_wild;
      topedlay_list*       _layerlist;
      DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class GDSbrowser : public wxTreeCtrl {
   public:
                        GDSbrowser(wxWindow *parent, wxWindowID id = -1, 
                        const wxPoint& pos = wxDefaultPosition, 
                        const wxSize& size = wxDefaultSize,
                        long style = wxTR_DEFAULT_STYLE):wxTreeCtrl(parent, id, pos, size, style) {};
      void              collectInfo();
      wxString          selectedCellname() const {if (RBcellID.IsOk())
                                       return GetItemText(RBcellID); else return "";}
   protected:
      void              collectChildren(GDSin::GDSHierTree *root, 
                                                   wxTreeItemId& lroot);
   private:
      wxTreeItemId      RBcellID;
      void              OnCommand(wxCommandEvent&);
      void              OnItemRightClick(wxTreeEvent&);
      void              OnBlankRMouseUp(wxMouseEvent&);
      void              OnWXImportCell(wxCommandEvent&);
      void              ShowMenu(wxTreeItemId id, const wxPoint& pt);
      void              OnGDSreportlay(wxCommandEvent& WXUNUSED(event));
      DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class TDTbrowser : public wxTreeCtrl {
   public:
                        TDTbrowser(wxWindow* parent, wxWindowID id = -1, 
                        const wxPoint& pos = wxDefaultPosition, 
                        const wxSize& size = wxDefaultSize,
                        long style = wxTR_DEFAULT_STYLE);
      virtual           ~TDTbrowser();
      void              collectInfo(const wxString, laydata::TDTHierTree*);
      void              initialize();
      wxString          selectedCellname() const {if (RBcellID.IsOk()) 
                                   return GetItemText(RBcellID); else return "";}
   protected:
      void              collectChildren(laydata::TDTHierTree *root, 
                                                 wxTreeItemId& lroot);
   private:
      wxTreeItemId      RBcellID;
      wxTreeItemId      top_structure;
      wxTreeItemId      active_structure;
      wxImageList*      _imageList;
      void              OnCommand(wxCommandEvent&);
      void              OnItemRightClick(wxTreeEvent&);
      void              OnBlankRMouseUp(wxMouseEvent&);
      void              OnWXOpenCell(wxCommandEvent&);
      void              OnWXCellARef(wxCommandEvent&);
      void              OnReportUsedLayers(wxCommandEvent&);
      void              OnTELLopencell(wxString);
      void              OnTELLhighlightcell(wxString);
      void              OnTELLaddcell(wxString, wxString, int);
      void              OnTELLremovecell(wxString, wxString, bool);
      void              ShowMenu(wxTreeItemId id, const wxPoint& pt);
      void              highlightChlidren(wxTreeItemId, wxColour);
      bool              findItem(const wxString, wxTreeItemId&, const wxTreeItemId);
      void              copyItem(const wxTreeItemId, const wxTreeItemId);
      DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class browserTAB : public wxNotebook {
   public:
                        browserTAB(wxWindow *parent, wxWindowID id = -1,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
                                                                  long style = 0);
      virtual          ~browserTAB();// {};
      topedlay_list*    TDTlayers() const    {return _TDTlayers->layerlist();};
      TDTbrowser*       TDTstruct() const    {return _TDTstruct;};
      wxString          TDTSelectedCellName() const {return _TDTstruct->selectedCellname();};
      wxString          TDTSelectedGDSName() const;// {return _GDSstruct->selectedCellname();};
   private:
      void              OnCommand(wxCommandEvent&);
      void              OnTELLaddTDTtab(const wxString, laydata::TDTHierTree*);
      void              OnTELLaddGDStab();
      void              OnTELLclearGDStab();
      GDSbrowser      *_GDSstruct;
      TDTbrowser      *_TDTstruct;
      layerbrowser    *_TDTlayers;
      //      CanvasPalette   *_TDTlayers;
      DECLARE_EVENT_TABLE();
   };
 
   void layer_status(BROWSER_EVT_TYPE, const word, const bool);
   void layer_add(const std::string, const word);
   void layer_default(const word, const word);
   void addTDTtab(std::string libname, laydata::TDTHierTree* tdtH);
   void addGDStab();
   void clearGDStab();
   void celltree_open(const std::string);
   void celltree_highlight(const std::string);
   void treeAddMember(const char*, const char*, int action = 0);
   void treeRemoveMember(const char*, const char*, bool orphan);
}

#endif //BROWSERS_H_INCLUDED
