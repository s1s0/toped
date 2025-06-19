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
//        Created: Thu Jun 19 19:46:34 EEST 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Toped User Interface wxWidgets customer events
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tuidefs.h"

wxDEFINE_EVENT(tui::wxEVT_CANVAS_STATUS     , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_RENDER_PARAMS     , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_CANVAS_PARAMS     , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_CMD_BROWSER       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_LOG_ERRMESSAGE    , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_MOUSE_ACCEL       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_MOUSE_INPUT       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_CANVAS_ZOOM       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_FUNC_BROWSER      , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_TPDSTATUS         , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_CANVAS_CURSOR     , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_CONSOLE_PARSE     , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_CURRENT_LAYER     , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_TOOLBARSIZE       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_TOOLBARADDITEM    , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_TOOLBARDELETEITEM , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_AUI_RESTORE       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_EDITLAYER         , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_EXITAPP           , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_EXECEXT           , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_EXECEXTPIPE       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_EXECEXTDONE       , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_RELOADTELLFUNCS   , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_TECHEDITUPDATE    , wxCommandEvent);
wxDEFINE_EVENT(tui::wxEVT_DRCDRAWPREP       , wxCommandEvent);


