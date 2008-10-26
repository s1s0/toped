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
//        Created: Sat Jan 10 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TDT I/O and database access control
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef DATA_HANDLER_INCLUDED
#define DATA_HANDLER_INCLUDED
#include "tedesign.h"
#include "../tpd_ifaces/gds_io.h"
#include "../tpd_ifaces/cif_io.h"
#include "viewprop.h"


namespace GDSin {

   class Gds2Ted {
   public:
                           Gds2Ted(GDSin::GdsFile* src_lib, laydata::tdtdesign* dst_lib, const LayerMapGds&);
      void                 top_structure(std::string, bool, bool);
   protected:
      void                 child_structure(const GDSin::GDSHierTree*, bool);
      void                 convert_prep(const GDSin::GDSHierTree* item, bool);
      void                 convert(GDSin::GdsStructure*, laydata::tdtcell*);
      void                 poly(GDSin::GdsPolygon* , laydata::tdtlayer*, int2b);
      void                 wire(GDSin::GDSpath*    , laydata::tdtlayer*, int2b);
      void                 text(GDSin::GdsText*    , laydata::tdtlayer*);
      void                 ref (GDSin::GdsRef*     , laydata::tdtcell* );
      void                 aref(GDSin::GdsARef*    , laydata::tdtcell* );
      GDSin::GdsFile*      _src_lib;
      laydata::tdtdesign*  _dst_lib;
      const LayerMapGds&   _theLayMap;
      real                 _coeff; // DBU difference
   };
}

namespace CIFin {

   class Cif2Ted {
      public:
                              Cif2Ted(CIFin::CifFile*, laydata::tdtdesign*, SIMap*);
         void                 top_structure(std::string, bool, bool);
      protected:
         void                 child_structure(const CIFin::CIFHierTree*, bool);
         void                 convert_prep(const CIFin::CIFHierTree* item, bool);
         void                 convert(CIFin::CifStructure*, laydata::tdtcell*);
         void                 box ( CIFin::CifBox*     ,laydata::tdtlayer*, std::string );
         void                 poly( CIFin::CifPoly*    ,laydata::tdtlayer*, std::string );
         void                 wire( CIFin::CifWire*    ,laydata::tdtlayer*, std::string );
         void                 ref ( CIFin::CifRef*     ,laydata::tdtcell*);
         void                 lbll( CIFin::CifLabelLoc*,laydata::tdtlayer*, std::string );
         void                 lbls( CIFin::CifLabelSig*,laydata::tdtlayer*, std::string );
         CIFin::CifFile*      _src_lib;
         laydata::tdtdesign*  _dst_lib;
         SIMap*                _cif_layers;
   };

}

class DataCenter {
public:
                              DataCenter(std::string);
                             ~DataCenter(); 
   bool                       GDSparse(std::string filename);
   void                       GDSexport(const LayerMapGds&, std::string&, bool);
   void                       GDSexport(laydata::tdtcell*, const LayerMapGds&, bool, std::string&, bool);
   void                       importGDScell(const nameList&, const LayerMapGds&, bool recur, bool over);
   void                       GDSclose();
   void                       CIFclose();
   CIFin::CifStatusType       CIFparse(std::string filename);
   void                       CIFexport(USMap*, bool, std::string&);
   void                       CIFexport(laydata::tdtcell*, USMap*, bool, bool, std::string&);
   bool                       CIFgetLay(nameList&);
   bool                       gdsGetLayers(GdsLayers&);
   void                       CIFimport(const nameList&, SIMap*, bool, bool);
   void                       PSexport(laydata::tdtcell*, std::string&);
   bool                       TDTread(std::string);
   int                        TDTloadlib(std::string);
   bool                       TDTunloadlib(std::string);
   bool                       TDTwrite(const char* filename = NULL);
   bool                       TDTcheckwrite(const TpdTime&, const TpdTime&, bool&); 
   bool                       TDTcheckread(const std::string, const TpdTime&, const TpdTime&, bool&); 
   void                       newDesign(std::string, time_t);
   laydata::tdtdesign*        lockDB(bool checkACTcell = true);
   GDSin::GdsFile*            lockGDS(bool throwexception = true);
   CIFin::CifFile*            lockCIF(bool throwexception = true);
   laydata::tdtlibrary*       getLib(int libID) {return _TEDLIB.getLib(libID);}
   int                        getLastLibRefNo() {return _TEDLIB.getLastLibRefNo();}
   bool                       getCellNamePair(std::string name, laydata::refnamepair& striter);
   void                       unlockDB();
   void                       unlockGDS();
   void                       unlockCIF();
   void                       mouseStart(int input_type, std::string, const CTM, int4b, int4b, word, word);
   void                       mousePointCancel(TP&);
   void                       mousePoint(TP p);
   void                       mouseStop();
   void                       mouseFlip();
   void                       mouseRotate();
   void                       openGL_draw(const CTM&);
   void                       tmp_draw(const CTM&, TP, TP);
   const laydata::cellList&   cells();
   unsigned int               numselected()           {return (NULL != _TEDLIB()) ? _TEDLIB()->numselected() : 0 ;}
   void                       saveProperties(std::string fname)
                                                      {_properties.saveProperties(fname);}
   void                       defaultlayer(word layno){_curlay = layno;}
   void                       initcmdlayer()          {_curcmdlay = _curlay;}
   void                       setcmdlayer(word layno) {_curcmdlay = layno;}
   word                       curlay() const          {return _curlay;}
   word                       curcmdlay() const       {return _curcmdlay;}
   std::string                tedfilename() const     {return _tedfilename;};
   bool                       neversaved()  const     {return _neversaved;}; 
   bool                       modified() const        {return _TEDLIB.modified();};
   bool                       autopan() const         {return _properties.autopan();}
   const real                 step() const            {return _properties.step();}
   const layprop::LayoutGrid* grid(byte gn) const     {return _properties.grid(gn);}
   const int4b                stepDB() const          {return _properties.stepDB();}
   const real                 UU() const              {return _properties.UU();}
   const real                 DBscale() const         {return _properties.DBscale();}
   word                       getLayerNo(std::string name) const
                                                      {return _properties.getLayerNo(name);}
   std::string                getLayerName(word layno) const
                                                      {return _properties.getLayerName(layno);}
   byte                       marker_angle() const    {return _properties.marker_angle();}
   bool                       layerHidden(word layno) {return _properties.drawprop().layerHidden(layno);}
   bool                       layerLocked(word layno) {return _properties.drawprop().layerLocked(layno);}
   bool                       grid_visual(word no)    {return grid(no)->visual();}
   

   void                       setautopan(bool status) {_properties.setautopan(status);}
   void                       setmarker_angle(byte angle)
                                                      {_properties.setmarker_angle(angle);}
   void                       setstep(real st)        {_properties.setstep(st);}
   void                       setClipRegion(DBbox clipR)
                                                      {_properties.setClipRegion(clipR);}
   void                       setScrCTM(CTM ScrCTM)   {_properties.setScrCTM(ScrCTM);}
   void                       setCurrentOp(console::ACTIVE_OP op)
                                                      {_properties.setCurrentOp(op);}
   const console::ACTIVE_OP   currentop() const       {return _properties.currentop();}

   bool                       addlayer(std::string, word, std::string, std::string, std::string);
   bool                       addlayer(std::string, word);
   bool                       addlayer(word layno);
   word                       addlayer(std::string);
   void                       addline(std::string, std::string, word, byte, byte);
   void                       addcolor(std::string, byte, byte, byte, byte);
   void                       addfill(std::string, byte*);
   void                       hideLayer(word, bool);
   void                       lockLayer(word, bool);
   void                       setcellmarks_hidden(bool);
   void                       settextmarks_hidden(bool);
   void                       setcellbox_hidden(bool);
   void                       settextbox_hidden(bool);
   void                       setGrid(byte, real, std::string);
   bool                       viewGrid(byte, bool);
   void                       addRuler(TP&, TP&);
   void                       clearRulers();
   void                       switch_drawruler(bool st) {_drawruler = st;}
   bool                       drawruler() {return _drawruler;}
   void                       all_layers(nameList& laylist) const {_properties.all_layers(laylist);}
   void                       all_colors(nameList& colist)  const {_properties.all_colors(colist); }
   void                       all_fills(nameList& filist)   const {_properties.all_fills(filist);  }
   void                       all_lines(nameList& linelist) const {_properties.all_lines(linelist);}
   const byte*                getFill(word layno) {return _properties.drawprop().getFill(layno);}
   const byte*                getFill(std::string fill_name) {return _properties.drawprop().getFill(fill_name);}
   const layprop::tellRGB&    getColor(word layno) {return _properties.drawprop().getColor(layno);}
   const layprop::tellRGB&    getColor(std::string color_name) {return _properties.drawprop().getColor(color_name);}
   const layprop::LineSettings* getLine(word layno) {return _properties.drawprop().getLine(layno);}
   const layprop::LineSettings* getLine(std::string line_name) {return _properties.drawprop().getLine(line_name);}
   const std::string          getColorName(word layno) {return _properties.drawprop().getColorName(layno);}
   const std::string          getFillName(word layno) {return _properties.drawprop().getFillName(layno);}
   const std::string          getLineName(word layno) {return _properties.drawprop().getLineName(layno);}
   const WordList             upLayers() {return _properties.upLayers();}
   void                       clearUnpublishedLayers() {_properties.clearUnpublishedLayers();}
   const word                 layselmask() {return _properties.layselmask();}
   void                       setlayselmask(word lsm) {_properties.setlayselmask(lsm);}
   laydata::tdtlibdir*        TEDLIB() {return &_TEDLIB;}
   laydata::LibCellLists*     getCells(int libID);
   void                       setGdsLayMap(USMap* map)   {_properties.setGdsLayMap(map);}
   void                       setCifLayMap(USMap* map)   {_properties.setCifLayMap(map);}
   const USMap*               getGdsLayMap() const       {return _properties.getGdsLayMap();}
   const USMap*               getCifLayMap() const       {return _properties.getCifLayMap();}
   LayerMapGds*               secureGdsLayMap(bool);
   LayerMapCif*               secureCifLayMap(bool);

   protected:
   laydata::tdtlibdir         _TEDLIB;       // catalog of available TDT libraries
   GDSin::GdsFile*            _GDSDB;        // GDS parsed data
   CIFin::CifFile*            _CIFDB;        // CIF parsed data
   layprop::ViewProperties    _properties;   // properties data base
   std::string                _tedfilename;
   bool                       _neversaved;
private:
   word                       _curlay;       // current drawing layer
   word                       _curcmdlay;    // layer used during current drawing operation
   bool                       _drawruler;    // draw a ruler while coposing a shape interactively
   std::string                _localDir;
   wxMutex                    DBLock;
   wxMutex                    GDSLock;
   wxMutex                    CIFLock;
   wxMutex                    PROPLock;

};

void initDBLib(std::string);

#endif
