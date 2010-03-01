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
#include <wx/aui/aui.h>
#include <string>
#include "tedesign.h"
#include "calbr_reader.h"
#include "gds_io.h"
#include "cif_io.h"
#include "oasis_io.h"

// Forward declarations

namespace GDSin {
   class GdsStructure;
   typedef SGHierTree<GdsStructure>       GDSHierTree;
}
namespace CIFin {
   class CifStructure;
   typedef SGHierTree<CifStructure>       CIFHierTree;
}

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
      BICN_LIBCELL_HIER = 0,  // library cell with hierarchy  0
      BICN_DBCELL_HIER  = 1,  // DB      cell with hierarchy  1
      BICN_LIBCELL_FLAT = 2,  // library cell  w/o hierarchy  2
      BICN_DBCELL_FLAT  = 3,  // DB      cell  w/o hierarchy  3
      BICN_LIBRARYDB    = 4,  // library                      4
      BICN_TARGETDB     = 5,  // DB                           5
      BICN_UNDEFCELL    = 6
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
      protected:
         wxTreeItemId      _rbCellID;
         virtual void      onItemRightClick(wxTreeEvent&);
         virtual void      onBlankRMouseUp(wxMouseEvent&);
         virtual void      onReportlay(wxCommandEvent& WXUNUSED(event)) {assert(false);}
         bool              checkCorrupted(bool);
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
         bool              isDbOrLibItem(const wxTreeItemId);
         wxTreeItemId      _topStructure;
         wxTreeItemId      _activeStructure;
         wxTreeItemId      _dbroot; // The actual Root always invisible (because of the libraries)
         LibsRoot          _libsRoot;
         wxTreeItemId      _undefRoot;
         bool              _hierarchy_view;
         wxColor           _listColor;
         wxColor           _editColor;
         DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class GDSCellBrowser:public CellBrowser {
      public:
                           GDSCellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         void              showMenu(wxTreeItemId id, const wxPoint& pt);
         void              collectInfo(bool);
      private:
         void              collectChildren(const GDSin::GDSHierTree*, const wxTreeItemId&, bool);
         void              onItemRightClick(wxTreeEvent&);
         void              onBlankRMouseUp(wxMouseEvent&);
         void              onReportlay(wxCommandEvent& WXUNUSED(event));
         DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class CIFCellBrowser:public CellBrowser {
      public:
                           CIFCellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         void              showMenu(wxTreeItemId id, const wxPoint& pt);
         void              collectInfo(bool);
      private:
         void              collectChildren(const CIFin::CIFHierTree*, const wxTreeItemId&, bool);
         void              onItemRightClick(wxTreeEvent&);
         void              onBlankRMouseUp(wxMouseEvent&);
         void              onReportlay(wxCommandEvent& WXUNUSED(event));
         DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class OASCellBrowser:public CellBrowser {
      public:
                           OASCellBrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
         void              showMenu(wxTreeItemId id, const wxPoint& pt);
         void              collectInfo(bool);
      private:
         void              collectChildren(const Oasis::OASHierTree*, const wxTreeItemId&, bool);
         void              onItemRightClick(wxTreeEvent&);
         void              onBlankRMouseUp(wxMouseEvent&);
         void              onReportlay(wxCommandEvent& WXUNUSED(event));
         DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class TDTbrowser : public wxPanel {
      public:
                           TDTbrowser(wxWindow* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxTR_DEFAULT_STYLE);
                          ~TDTbrowser();
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
         CellBrowser*      _cellBrowser;
         bool              _hierarchy_view;
         DECLARE_EVENT_TABLE();
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
         DECLARE_EVENT_TABLE();
   };

   //===========================================================================
   class LayerInfo {
      public:
                              LayerInfo(const LayerInfo& lay);
                              LayerInfo(const std::string &name, const word layno);
         std::string          name()               { return _name;  }
         word                 layno()              { return _layno; }
      private:
         std::string          _name;
         word                 _layno;
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
                             ~LayerButton();
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
         word                 getLayNo()          {return _layer->layno();}

      private:
         void                 makeBrush(const layprop::DrawProperties*);
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

      DECLARE_EVENT_TABLE();
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
         typedef std::map <word, LayerButton*> LayerButtonMap;

         void                 onCommand(wxCommandEvent&);
         void                 addButton(LayerInfo& layer);
         LayerButton*         checkDefined(word);
         LayerButtonMap       _buttonMap;
         int                  _buttonCount;
         DECLARE_EVENT_TABLE();
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
         DECLARE_EVENT_TABLE();
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
      virtual void            onBlankRMouseUp(wxMouseEvent&);
      void                    onShowError(wxCommandEvent& WXUNUSED(event));
      void                    onShowCluster(wxCommandEvent& WXUNUSED(event));
      void                   showMenu(wxTreeItemId id, const wxPoint& pt);
   private:
      wxString               _error;
      wxString               _cluster;
      DECLARE_EVENT_TABLE();
   };

  //===========================================================================
   class DRCBrowser : public wxPanel {
   public:
                              DRCBrowser(wxWindow* parent, wxWindowID id);
      virtual                ~DRCBrowser();
      void                    deleteAllItems(void);
      void                    onShowAll(wxCommandEvent&);
      void                    onHideAll(wxCommandEvent&);
      void                    onExplainError(wxCommandEvent&);
   private:
      ErrorBrowser*           _errorBrowser;
      wxButton*               _showAllButton;
      wxButton*               _hideAllButton;
      wxButton*               _explainButton;
      DECLARE_EVENT_TABLE();
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
         int                 _gdsPageIndex;
         int                 _cifPageIndex;
         LayerBrowser*       _layers;
         DECLARE_EVENT_TABLE();
   };
 }

#endif //BROWSERS_H_INCLUDED
