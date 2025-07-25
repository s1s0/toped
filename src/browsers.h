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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
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
#include <wx/treebase.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/aui/aui.h>
#include <string>
#include "tedesign.h"
#include "calbr_reader.h"

namespace browsers
{
   const int buttonHeight = 30;

   enum
   {
      CELLTREEOPENCELL  = 1000,
      GDSTREEREPORTLAY        ,
      CIFTREEREPORTLAY        ,
      OASTREEREPORTLAY        ,
      LAYERHIDESELECTED       ,
      LAYERSHOWSELECTED       ,
      LAYERLOCKSELECTED       ,
      LAYERUNLOCKSELECTED     ,
      LAYERCURRENTSELECTED    ,
      LAYERCURRENTEDIT
   };

   // all browser icons
   enum
   {
      BICN_LIBCELL_HIER   ,  // library cell with hierarchy
      BICN_LIBCELL_HIER_I ,  // library cell with hierarchy and invalid objects
      BICN_DBCELL_HIER    ,  // DB      cell with hierarchy
      BICN_DBCELL_HIER_I  ,  // DB      cell with hierarchy and invalid objects
      BICN_LIBCELL_FLAT   ,  // library cell  w/o hierarchy
      BICN_LIBCELL_FLAT_I ,  // library cell  w/o hierarchy and invalid objects
      BICN_DBCELL_FLAT    ,  // DB      cell  w/o hierarchy
      BICN_DBCELL_FLAT_I  ,  // DB      cell  w/o hierarchy and invalid objects
      BICN_LIBRARYDB      ,  // library
      BICN_TARGETDB       ,  // DB
      BICN_UNDEFCELL
   };


   //===========================================================================
   class CellBrowser: public wxTreeCtrl {
      public:
                           CellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         virtual          ~CellBrowser()  {};
         virtual  void     showMenu(wxTreeItemId id, const wxPoint& pt);
         bool              findItem(const wxString, wxTreeItemId&, const wxTreeItemId);
         bool              findChildItem(const wxString, wxTreeItemId&, const wxTreeItemId);
         void              copyItem(const wxTreeItemId, const wxTreeItemId, bool targetLib = true);
         void              highlightChildren(wxTreeItemId, wxColour);
         void              resetData(wxString);
         wxString          selectedCellName();
         wxString          topCellName();
         wxString          activeCellName();
         void              statusHighlight(wxString, wxString, wxString);
         wxString          rbCellName();
         virtual void      collectInfo(bool);
         void              setCellFilter(wxString & filter) {_cellFilter = filter;};
      protected:
         wxTreeItemId      _rbCellID;
         virtual void      onItemRightClick(wxTreeEvent&);
         //virtual void      onBlankRMouseUp(wxMouseEvent&);
         virtual void      onReportlay(wxCommandEvent& WXUNUSED(event)) {assert(false);}
         bool              checkCorrupted(bool);
         void              collectChildren(const ForeignCellTree*, const wxTreeItemId&, bool);
         bool              _corrupted;
      private:
         typedef std::list<wxTreeItemId> LibsRoot;
         void              initialize();
         void              collectChildren(const laydata::TDTHierTree*, int, const wxTreeItemId&);
         void              updateFlat();
         void              updateHier();
         void              onCommand(wxCommandEvent&);
         void              onLMouseDblClk(wxMouseEvent&);
         void              onWxOpenCell(wxCommandEvent&);
         void              tdtCellSpot(const wxTreeItemId&, const wxTreeItemId&);
         void              onTellAddCell(wxString, wxString, int);
         void              onTellRemoveCell(wxString, wxString, int);
         void              onTellRenameCell(wxString, wxString);
         void              onTellChangeGrc(wxString, bool);
         bool              isDbOrLibItem(const wxTreeItemId);
         wxTreeItemId      _topStructure;
         wxTreeItemId      _activeStructure;
         wxString          _topCellNameBackup;
         wxString          _activeCellNameBackup;
         wxTreeItemId      _dbroot; // The actual Root always invisible (because of the libraries)
         LibsRoot          _libsRoot;
         wxTreeItemId      _undefRoot;
         bool              _hierarchy_view;
         wxColor           _listColor;
         wxColor           _editColor;
         wxString          _cellFilter;
   };

   //===========================================================================
   class GDSCellBrowser:public CellBrowser {
      public:
                           GDSCellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         virtual void      showMenu(wxTreeItemId id, const wxPoint& pt);
         virtual void      collectInfo(bool);
      private:
         virtual void      onReportlay(wxCommandEvent& WXUNUSED(event));
   };

   //===========================================================================
   class CIFCellBrowser:public CellBrowser {
      public:
                           CIFCellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         virtual void      showMenu(wxTreeItemId id, const wxPoint& pt);
         virtual void      collectInfo(bool);
      private:
         virtual void      onReportlay(wxCommandEvent& WXUNUSED(event));
   };

   //===========================================================================
   class OASCellBrowser:public CellBrowser {
      public:
                           OASCellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         virtual void      showMenu(wxTreeItemId id, const wxPoint& pt);
         virtual void      collectInfo(bool);
      private:
         virtual void      onReportlay(wxCommandEvent& WXUNUSED(event));
   };

   //===========================================================================
   class TDTbrowser : public wxPanel {
      public:
                           TDTbrowser(wxWindow* parent, wxWindowID id = wxID_ANY,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         virtual          ~TDTbrowser();
         wxString          selectedCellName() const;
         void              refreshData(bool keepAct);
         CellBrowser*      cellBrowser() const        {return _cellBrowser;}
      private:
         void              onReportUsedLayers(wxCommandEvent& WXUNUSED(event));
         void              onHierView(wxCommandEvent&);
         void              onFlatView(wxCommandEvent&);
         wxImageList*      _imageList;
         wxButton*         _hierButton;
         wxButton*         _flatButton;
         wxTextCtrl*       _cellFilter;
         CellBrowser*      _cellBrowser;
         bool              _hierarchy_view;
   };

   //===========================================================================
   class XdbBrowser : public wxPanel {
      public:
                           XdbBrowser(wxWindow*, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         wxString          selectedCellName() const   {return _cellBrowser->selectedCellName();}
         void              deleteAllItems(void)       {_cellBrowser->DeleteAllItems();}
         void              collectInfo()              {_cellBrowser->collectInfo(_hierarchy_view);}
      private:
         void              onHierView(wxCommandEvent&);
         void              onFlatView(wxCommandEvent&);
         wxButton*         _hierButton;
         wxButton*         _flatButton;
         CellBrowser*      _cellBrowser;
         bool              _hierarchy_view;
   };

   //===========================================================================
   class LayerInfo {
      public:
                              LayerInfo(const LayerInfo& lay);
                              LayerInfo(const std::string&, const LayerDef&);
         std::string          name()               { return _name;  }
         LayerDef             laydef()             { return _laydef; }
      private:
         std::string          _name;
         LayerDef             _laydef;
   };

   //===========================================================================
   class LayerButton:public wxPanel {
      public:
                              LayerButton(wxWindow* parent, wxWindowID id,
                                          const wxPoint& pos = wxDefaultPosition,
                                          const wxSize& size = wxDefaultSize,
                                          long style = wxBU_AUTODRAW,
                                          const wxValidator& validator = wxDefaultValidator,
                                          const wxString& name = wxT("button"),
                                          LayerInfo *layer = NULL);
         virtual             ~LayerButton();
         void                 onLeftClick(wxMouseEvent&);
         void                 onMiddleClick(wxMouseEvent&);
         void                 onRightClick(wxMouseEvent&);
         void                 onPaint(wxPaintEvent&);
         void                 OnEditLayer(wxCommandEvent&);
         //Call when other button is selected
         void                 selectLayer(bool);
         void                 hideLayer(bool);
         void                 lockLayer(bool);
         void                 fillLayer(bool);
         void                 preparePicture();
         LayerDef             getLayDef()          {return _layer->laydef();}

      private:
         int                  _buttonWidth;
         int                  _buttonHeight;
         LayerInfo*           _layer;
         wxBitmap*            _picture;
         wxBrush*             _brush;
         wxPen*               _pen;
         bool                 _selected;
         bool                 _hidden;
         bool                 _locked;
         bool                 _filled;
   };

   //===========================================================================
   class LayerPanel:public wxScrolledWindow {
      public:
                              LayerPanel(wxWindow* parent, wxWindowID id = -1,
                                          const wxPoint& pos = wxDefaultPosition,
                                          const wxSize& size = wxDefaultSize,
                                          long style = wxHSCROLL |  wxVSCROLL,
                                          const wxString& name = wxT("LayerPanel"));
         virtual              ~LayerPanel();
         wxString             getAllSelected();
         void                 onPaint(wxPaintEvent&);
      private:
         typedef std::map <LayerDef, LayerButton*> LayerButtonMap;

         void                 onCommand(wxCommandEvent&);
         void                 addButton(LayerInfo& layer);
         LayerButton*         checkDefined(const LayerDef&);
         LayerButtonMap       _buttonMap;
         int                  _buttonCount;
   };

   //===========================================================================
   class LayerBrowser : public wxPanel {
      public:
                              LayerBrowser(wxWindow* parent, wxWindowID id);
         virtual             ~LayerBrowser();
         LayerPanel*          getLayerPanel() {return _layerPanel;};
      private:
         void                 onShowAll(wxCommandEvent& WXUNUSED(event));
         void                 onHideAll(wxCommandEvent& WXUNUSED(event));
         void                 onLockAll(wxCommandEvent& WXUNUSED(event));
         void                 onUnlockAll(wxCommandEvent& WXUNUSED(event));
         void                 onSaveState(wxCommandEvent& WXUNUSED(event));
         void                 onLoadState(wxCommandEvent& WXUNUSED(event));
         void                 onCommand(wxCommandEvent&);
         wxString             getAllSelected();
         LayerPanel*          _layerPanel;
         wxBoxSizer*          _thesizer;
   };
 //===========================================================================
   class ErrorBrowser: public wxTreeCtrl
   {
   public:
                              ErrorBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
      void                    onLMouseDblClk(wxMouseEvent&);
      virtual void            onItemRightClick(wxTreeEvent&);
      //virtual void            onBlankRMouseUp(wxMouseEvent&);
      void                    onOpenCell(wxCommandEvent&);
      void                    onShowError(wxCommandEvent& WXUNUSED(event));
      void                    onShowCluster(wxCommandEvent& WXUNUSED(event));
      void                    showMenu(wxTreeItemId id, const wxPoint& pt);
   private:
      bool                    checkCellName(const std::string &str);
      //wxString               _error;
      wxString                _cluster;
      //wxString               _cell;
      wxTreeItemId            _rbCellID;
   };

  //===========================================================================
   class DRCBrowser : public wxPanel {
   public:
                              DRCBrowser(wxWindow* parent, wxWindowID id);
      virtual                ~DRCBrowser();
      void                    deleteAllItems(void);
      void                    onShowAll(wxCommandEvent&);
      void                    onHideAll(wxCommandEvent&);
//      void                    onExplainError(wxCommandEvent&);
      void                    onRulesHierarchy(wxCommandEvent&);
      void                    onCellsHierarchy(wxCommandEvent&);
   private:
      void                    addRuleCheck( const wxTreeItemId &rootId, std::string, clbr::DrcRule* check);
      wxTreeItemId            addCellCheck( const wxTreeItemId &rootId, std::string/*, Calbr::DrcRule* check*/);
      void                    showRuleHierarchy();
      void                    showCellHierarchy();
      ErrorBrowser*           _errorBrowser;
      wxButton*               _showAllButton;
      wxButton*               _hideAllButton;
//      wxButton*               _explainButton;
      wxButton*               _rulesButton;
      wxButton*               _cellsButton;
   };

   //===========================================================================
   class browserTAB : public wxAuiNotebook {
      public:
                              browserTAB(wxWindow *parent, wxWindowID id = -1,
                                          const wxPoint& pos = wxDefaultPosition,
                                          const wxSize& size = wxDefaultSize,
                                          long style = 0);
         virtual             ~browserTAB();
         wxString             tdtSelectedGdsName() const;
         wxString             tdtSelectedCifName() const;
         wxString             tdtSelectedOasName() const;
         wxString             tdtSelectedCellName() const   { return _tdtStruct->selectedCellName();}
      private:
         void                 onCommand(wxCommandEvent&);
         void                 onTellRefreshTdtLib(bool);
         void                 onTellAddGdsTab();
         void                 onTellClearGdsTab();
         void                 onTellAddCifTab();
         void                 onTellClearCifTab();
         void                 onTellAddOasTab();
         void                 onTellClearOasTab();
         void                 onTellAddDRCTab();
         void                 onTellClearDRCTab();
         XdbBrowser*         _gdsStruct;
         XdbBrowser*         _cifStruct;
         XdbBrowser*         _oasStruct;
         TDTbrowser*         _tdtStruct;
         DRCBrowser*         _drcStruct;
//         int                 _gdsPageIndex;
//         int                 _cifPageIndex;
         LayerBrowser*       _layers;
   };

   //type of wxTreeItem for DRC browser
   enum 
   {
      ITEM_ROOT   = 0,
      ITEM_CELL   = 1,
      ITEM_ERR    = 2,
      ITEM_ERR_NUM= 3
   };

   //Aux class designed for store information about type of data in wxTreeItem for DRC browser 
   class DRCItemData:public wxTreeItemData
   {
   public:
      DRCItemData(int typ) { type = typ;};
      int getType() {return type;};
   private:
      int type;
   };
 }

   
#endif //BROWSERS_H_INCLUDED
