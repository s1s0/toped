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
//        Created: Sun Apr 15 18:08:11 BST 2007
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Toped User Interface enum definitions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TUIDEFS_H
#define TUIDEFS_H
namespace tui
{
   typedef enum
   {
      ID_WIN_TOPED  = 100 ,
      ID_WIN_BROWSERS     ,
      ID_WIN_GLSTATUS     ,
      ID_WIN_COMMAND      ,
      ID_WIN_LOG_PANE     ,
      ID_WIN_TXT_LOG      ,
      ID_WIN_CANVAS       ,
      ID_TPD_CANVAS       ,
      ID_TPD_LAYERS       ,
      ID_TPD_CELLTREE     ,
      ID_GDS_CELLTREE     ,
      ID_CIF_CELLTREE     ,
      ID_OAS_CELLTREE     ,
      ID_DRC_CELLTREE     ,
      ID_TPD_STATUS       ,
      ID_TELL_FUNCS       ,
      ID_PNL_LAYERS       ,
      ID_PNL_CELLS        ,
      ID_PNL_DRC          ,
      ID_CMD_LINE         ,
      ID_TECH_EDITOR      ,
      ID_CELL_FILTER      ,
      ID_TECH_PANEL       ,
      //Warning!!! Do not use IDs between ID_DUMMY_WIN and ID_DUMMY_WIN_END
      ID_DUMMY_WIN = 500  ,
      ID_DUMMY_WIN_END = 600
   } WX_WINDOW_IDS_TYPE;

   typedef enum  {
      TMFILE_NEW = 100    ,
      TMFILE_OPEN         ,
      TMFILE_INCLUDE      ,
      TMLIB_LOAD          ,
      TMLIB_UNLOAD        ,
      // <- GDS stuff
      TMGDS_OPEN          ,
      TMGDS_IMPORT        ,
      TMGDS_EXPORTL       ,
      TMGDS_EXPORTC       ,
      TMGDS_TRANSLATE     ,
      TMGDS_CLOSE         ,
      // <- CIF stuff
      TMCIF_OPEN          ,
      TMCIF_EXPORTL       ,
      TMCIF_EXPORTC       ,
      TMCIF_TRANSLATE     ,
      TMCIF_CLOSE         ,
      // <- Oasis stuff
      TMOAS_OPEN          ,
      TMOAS_IMPORT        ,
      TMOAS_EXPORTL       ,
      TMOAS_EXPORTC       ,
      TMOAS_TRANSLATE     ,
      TMOAS_CLOSE         ,
      //
      TMFILE_SAVE         ,
      TMFILE_SAVEAS       ,
      TMPROP_SAVE         ,
      TMFILE_EXIT         ,

      TMEDIT_UNDO         ,
      TMEDIT_COPY         ,
      TMEDIT_MOVE         ,
      TMEDIT_DELETE       ,
      TMEDIT_ROTATE90     ,
      TMEDIT_FLIPX        ,
      TMEDIT_FLIPY        ,
      TMEDIT_POLYCUT      ,
      TMEDIT_MERGE        ,
      TMEDIT_RESIZE       ,

      TMVIEW_VIEWTOOLBAR  ,
      TMVIEW_VIEWSTATUSBAR,
      TMVIEW_ZOOMIN       ,
      TMVIEW_ZOOMOUT      ,
      TMVIEW_ZOOMALL      ,
      TMVIEW_ZOOMVISIBLE  ,
      TMVIEW_PANLEFT      ,
      TMVIEW_PANRIGHT     ,
      TMVIEW_PANUP        ,
      TMVIEW_PANDOWN      ,
      TMVIEW_PANCENTER    ,

      TMCELL_NEW          ,
      TMCELL_OPEN         ,
      TMCELL_REMOVE       ,
      TMCELL_PUSH         ,
      TMCELL_POP          ,
      TMCELL_TOP          ,
      TMCELL_PREV         ,
      TMCELL_REF_M        ,
      TMCELL_REF_B        ,
      TMCELL_AREF_M       ,
      TMCELL_AREF_B       ,
      TMCELL_REPORTLAY    ,
      TMCELL_GROUP        ,
      TMCELL_UNGROUP      ,

      TMDRAW_BOX          ,
      TMDRAW_POLY         ,
      TMDRAW_WIRE         ,
      TMDRAW_TEXT         ,

      TMSEL_SELECT_IN     ,
      TMSEL_PSELECT_IN    ,
      TMSEL_SELECT_ALL    ,
      TMSEL_UNSELECT_IN   ,
      TMSEL_PUNSELECT_IN  ,
      TMSEL_UNSELECT_ALL  ,
      TMSEL_REPORT_SLCTD  ,

      TMSET_ALLPROP       ,
//      TMSET_HTOOLSIZE     ,
//      TMSET_TOOLSIZE16    ,
//      TMSET_TOOLSIZE24    ,
//      TMSET_TOOLSIZE32    ,
//      TMSET_TOOLSIZE48    ,
//      TMSET_VTOOLSIZE     ,
//      TMSET_VTOOLSIZE16   ,
//      TMSET_VTOOLSIZE24   ,
//      TMSET_VTOOLSIZE32   ,
//      TMSET_VTOOLSIZE48   ,
      TMSET_UNDODEPTH     ,
      TMSET_DEFCOLOR      ,
      TMSET_DEFFILL       ,
      TMSET_DEFSTYLE      ,
      TMSET_TECHEDITOR    ,

      TMADD_RULER         ,
      TMCLEAR_RULERS      ,

      TMCADENCE_CONVERT   ,
      TMGET_SNAPSHOT      ,
      TMDRC_SHOW_ERR      ,
      TMDRC_SHOW_CLUSTER  ,
      TMDRC_OPEN_CELL     ,

      TMHELP_ABOUTAPP     ,
      //Warning!!! Do not use IDs between TMDUMMY and TMDUMMY_END
      TMDUMMY        = 500   ,
      TDUMMY_TOOL    = 2000  ,
      TMDUMMY_LAYER  = 10000 ,
      TMDUMMY_END    = 11000 ,
   } TOPED_MENUID;


   typedef enum  {
      ZOOM_WINDOW  = 0    ,
      ZOOM_WINDOWM        ,
      ZOOM_IN             ,
      ZOOM_OUT            ,
      ZOOM_LEFT           ,
      ZOOM_RIGHT          ,
      ZOOM_UP             ,
      ZOOM_DOWN           ,
      ZOOM_EMPTY          ,
      ZOOM_REFRESH
   } ZOOM_TYPE;

   typedef enum  {
      CM_CONTINUE  = 1    ,
      CM_ABORT            ,
      CM_RULER            ,
      CM_CHLAY            ,
      CM_CANCEL_LAST      ,
      CM_CLOSE            ,
      CM_AGAIN            ,
      CM_ROTATE           ,
      CM_FLIP
   } CONTEXT_MENU_TYPE;

   typedef enum {
      CNVS_POS_X          ,
      CNVS_POS_Y          ,
      CNVS_DEL_X          ,
      CNVS_DEL_Y          ,
      CNVS_SELECTED
   } CVSSTATUS_TYPE;

   typedef enum {
      RPS_CELL_MARK       ,
      RPS_CELL_BOX        ,
      RPS_TEXT_MARK       ,
      RPS_TEXT_BOX        ,
      RPS_TEXT_ORI        ,
      RPS_VISI_LIMIT      ,
      RPS_CELL_DAB        ,
      RPS_CELL_DOV        ,
      RPS_LD_FONT         ,
      RPS_SLCT_FONT       ,
      RPS_GRC_PERIOD
   } RENDER_PROPERTY_ENUMS;

   typedef enum {
      CPS_MARKER_MOTION   ,
      CPS_MARKER_STEP     ,
      CPS_GRID0_ON        ,
      CPS_GRID1_ON        ,
      CPS_GRID2_ON        ,
      CPS_GRID0_STEP      ,
      CPS_GRID1_STEP      ,
      CPS_GRID2_STEP      ,
      CPS_RECOVER_POLY    ,
      CPS_RECOVER_WIRE    ,
      CPS_AUTOPAN         ,
      CPS_ZERO_CROSS      ,
      CPS_LONG_CURSOR     ,
      CPS_BOLD_ON_HOVER
   } CANVAS_PROPERTY_ENUMS;

   typedef enum
   {
      BT_LAYER_DEFAULT,
      BT_LAYER_HIDE,
      BT_LAYER_LOCK,
      BT_LAYER_FILL,
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
      BT_CELL_RENAME,
      BT_CELL_MARK_GRC,
      BT_ADDTDT_LIB,
      BT_NEWTDT_DB,
      BT_ADDGDS_TAB,
      BT_CLEARGDS_TAB,
      BT_ADDCIF_TAB,
      BT_CLEARCIF_TAB,
      BT_ADDOAS_TAB,
      BT_CLEAROAS_TAB,
      BT_ADDDRC_TAB,
      BT_CLEARDRC_TAB,
      BT_CELLS_HIER,
      BT_CELLS_FLAT,
      BT_CELLS_HIER2,
      BT_CELLS_FLAT2,
      BT_DRC_SHOW_ALL,
      BT_DRC_HIDE_ALL,
      BT_DRC_RULES_HIER,
      BT_DRC_CELLS_HIER,
      BT_LAYER_SHOW_ALL,
      BT_LAYER_HIDE_ALL,
      BT_LAYER_LOCK_ALL,
      BT_LAYER_UNLOCK_ALL,
      BT_LAYER_SAVE_ST,
      BT_LAYER_LOAD_ST,
      BT_LAYER_STATES,
      BT_LAYSTATE_SAVE,
      BT_LAYSTATE_DELETE,
   } BROWSER_EVT_TYPE;

}

#endif //TUIDEFS_H
