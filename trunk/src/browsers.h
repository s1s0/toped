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
//        Created: Mon Aug 11 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GDSII/TDT hierarchy browser, layer browser, TELL fuction
//                 definition browser
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
#include <wx/imaglist.h>
#include <string>
#include "../tpd_DB/tedesign.h"
#include "../tpd_DB/gds_io.h"

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
      BT_CELL_OPEN,
      BT_CELL_HIGHLIGHT,
      BT_CELL_REF,
      BT_CELL_AREF,
      BT_CELL_ADD,
      BT_CELL_REMOVE,
      BT_ADDTDT_TAB,
      BT_ADDGDS_TAB,
      BT_CLEARGDS_TAB,
      BT_CELLS_HIER,
      BT_CELLS_FLAT,
      BT_CELLS_HIER2,
      BT_CELLS_FLAT2

   } BROWSER_EVT_TYPE;
   
   enum {
      CELLTREEOPENCELL  = 1000,
      GDSTREEREPORTLAY        ,
      LAYERHIDESELECTED       ,
      LAYERSHOWSELECTED       ,
      LAYERLOCKSELECTED       ,
      LAYERUNLOCKSELECTED     ,
      LAYERCURRENTSELECTED
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
      void                 OnSort(wxListEvent&);
   private:
      wxImageList*         _imageList;
      wxFont               _llfont_normal;
      wxFont               _llfont_bold;
      DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class layerbrowser : public wxPanel {
   public:
                           layerbrowser(wxWindow* parent, wxWindowID id);
      virtual             ~layerbrowser();
      topedlay_list*       layerlist() const {return _layerlist;};
      word                 getFirstSelected();
   private:
      void                 OnNewLayer(wxCommandEvent&);
      void                 OnEditLayer(wxCommandEvent&);
      void                 OnXXXSelected(wxCommandEvent&);
      void                 OnCommand(wxCommandEvent&);
      void                 OnActiveLayerL(wxListEvent&);
      void                 OnActiveLayerM(wxCommandEvent&);
      void                 OnSelectWild(wxCommandEvent&);
      void                 OnItemRightClick(wxListEvent&);
      void                 OnHideSelected(wxCommandEvent&);
      void                 OnShowSelected(wxCommandEvent&);
      void                 OnLockSelected(wxCommandEvent&);
      void                 OnUnlockSelected(wxCommandEvent&);
      wxString             getAllSelected();
      
      wxChoice*            action_select;
      wxChoice*            action_wild;
      topedlay_list*       _layerlist;
      DECLARE_EVENT_TABLE();
   };

   class CellBrowser: public wxTreeCtrl
   {
   public:
      CellBrowser(wxWindow* parent, wxWindowID id = -1, 
                        const wxPoint& pos = wxDefaultPosition, 
                        const wxSize& size = wxDefaultSize,
                        long style = wxTR_DEFAULT_STYLE);
      virtual           ~CellBrowser();
      virtual  void     ShowMenu(wxTreeItemId id, const wxPoint& pt);
      bool              findItem(const wxString name, wxTreeItemId& item, const wxTreeItemId parent);
      void              copyItem(const wxTreeItemId, const wxTreeItemId);
      void              highlightChildren(wxTreeItemId, wxColour);
      wxTreeItemId      activeStructure(void) {return _activeStructure;};
   protected:
      wxTreeItemId      RBcellID;
   private:
      wxTreeItemId      top_structure;
      wxTreeItemId      _activeStructure;

      void              OnItemRightClick(wxTreeEvent&);
      void              OnLMouseDblClk(wxMouseEvent&);
      void              OnWXOpenCell(wxCommandEvent&);
      void              OnBlankRMouseUp(wxMouseEvent&);

      DECLARE_EVENT_TABLE();
   };

   class GDSCellBrowser:public CellBrowser
   {
   public:
      GDSCellBrowser(wxWindow* parent, wxWindowID id = -1, 
                        const wxPoint& pos = wxDefaultPosition, 
                        const wxSize& size = wxDefaultSize,
                        long style = wxTR_DEFAULT_STYLE);
      virtual  void     ShowMenu(wxTreeItemId id, const wxPoint& pt);
   private:
      void              OnItemRightClick(wxTreeEvent&);
      void              OnBlankRMouseUp(wxMouseEvent&);
      void              OnLMouseDblClk(wxMouseEvent&);
      void              OnGDSreportlay(wxCommandEvent& WXUNUSED(event));
      DECLARE_EVENT_TABLE();
   };


   //===========================================================================
   class GDSbrowser : public wxPanel {
   public:
                        GDSbrowser(wxWindow *parent, wxWindowID id = -1, 
                        const wxPoint& pos = wxDefaultPosition, 
                        const wxSize& size = wxDefaultSize,
                        long style = wxTR_DEFAULT_STYLE);
      void              collectInfo();
      wxString          selectedCellname() const {if (RBcellID.IsOk())
         return hCellBrowser->GetItemText(RBcellID); else return wxT("");}
      void DeleteAllItems(void);
   protected:
      void              collectChildren(GDSin::GDSHierTree *root, 
                                                   wxTreeItemId& lroot);
   private:
      wxButton*         _hierButton;
      wxButton*         _flatButton;
      wxTreeItemId      RBcellID;
      GDSCellBrowser*   hCellBrowser;//Hierarchy cell browser
      GDSCellBrowser*   fCellBrowser;//Flat cell browser
      void              OnCommand(wxCommandEvent&);
      void              OnWXImportCell(wxCommandEvent&);
      void              OnHierView(wxCommandEvent&);
      void              OnFlatView(wxCommandEvent&);
      DECLARE_EVENT_TABLE();
   };


   //===========================================================================
   class TDTbrowser : public wxPanel {
   public:
                        TDTbrowser(wxWindow* parent, wxWindowID id = -1, 
                        const wxPoint& pos = wxDefaultPosition, 
                        const wxSize& size = wxDefaultSize,
                        long style = wxTR_DEFAULT_STYLE);
      virtual           ~TDTbrowser();
      void              collectInfo(const wxString, laydata::TDTHierTree*);
      void              initialize();
      wxString          selectedCellname() const {if (RBcellID.IsOk()) 
         return hCellBrowser->GetItemText(RBcellID); else return wxT("");}
   protected:
      void              collectChildren(laydata::TDTHierTree *root, 
                                                 wxTreeItemId& lroot);
   private:
      wxTreeItemId      RBcellID;//+
      wxTreeItemId      top_structure;//+
      wxTreeItemId      active_structure;//+
      wxImageList*      _imageList;
      CellBrowser*      hCellBrowser;//Hierarchy cell browser
      CellBrowser*      fCellBrowser;//Flat cell browser
      //laydata::TDTHierTree*   tree;
      wxString          libName;
      wxButton*         _hierButton;
      wxButton*         _flatButton;
      void              OnCommand(wxCommandEvent&);
      void              OnWXCellARef(wxCommandEvent&);
      void              OnReportUsedLayers(wxCommandEvent&);
      void              OnTELLopencell(wxString);
      void              OnTELLhighlightcell(wxString);
      void              OnTELLaddcell(wxString, wxString, int);
      void              OnTELLremovecell(wxString, wxString, bool);
      void              OnHierView(wxCommandEvent&);
      void              OnFlatView(wxCommandEvent&);
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
      word              TDTSelectedLayNo()   {return _TDTlayers->getFirstSelected();}
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
