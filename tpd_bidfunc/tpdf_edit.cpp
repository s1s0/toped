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
//        Created: Wed Apr 25 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED edit functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include "tpdf_edit.h"

#include "../tpd_DB/datacenter.h"

extern DataCenter*               DATC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdCOPYSEL::stdCOPYSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdCOPYSEL::undo_cleanup()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdCOPYSEL::undo()
{
   TEUNDO_DEBUG("copy(point point) UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      //clean up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB()); 
      ATDB->select_fromList(get_ttlaylist(pl));
   DATC->unlockDB();   
   delete (pl);
   RefreshGL();
}

int tellstdfunc::stdCOPYSEL::execute()
{
   UNDOcmdQ.push_front(this);
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
      ATDB->copy_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale));
      OPstack.push(make_ttlaylist(ATDB->shapesel()));
   DATC->unlockDB();   
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   delete p1; delete p2;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCOPYSEL_D::stdCOPYSEL_D(telldata::typeID retype, bool eor) :
      stdCOPYSEL(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdCOPYSEL_D::execute()
{
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to copy");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_copy, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   OPstack.push(DEBUG_NEW telldata::ttpnt(w->p1().x(), w->p1().y()));
   OPstack.push(DEBUG_NEW telldata::ttpnt(w->p2().x(), w->p2().y()));
   delete w;
   return stdCOPYSEL::execute();
}

//=============================================================================
tellstdfunc::stdMOVESEL::stdMOVESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdMOVESEL::undo_cleanup()
{
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(deleted);
   delete added;
   delete deleted;
   delete failed;
   delete p1;
   delete p2;
}

void tellstdfunc::stdMOVESEL::undo()
{
   TEUNDO_DEBUG("move(point point) UNDO");
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();

   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(failed));
      ATDB->unselect_fromList(get_ttlaylist(added));
      laydata::selectList* fadead[3];
     byte i;
      for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::selectList());
      ATDB->move_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale),fadead);
      //@TODO Here - an internal check can be done - all 3 of the fadead lists
      // MUST be empty, otherwise - god knows what's wrong!
      for (i = 0; i < 3; delete fadead[i++]);
      ATDB->select_fromList(get_ttlaylist(failed));
      // put back the replaced (deleted) shapes
      ATDB->addlist(get_shlaylist(deleted));
      // and select them
      ATDB->select_fromList(get_ttlaylist(deleted));
      // delete the added shapes
      for (word j = 0 ; j < added->mlist().size(); j++) {
         ATDB->destroy_this(static_cast<telldata::ttlayout*>(added->mlist()[j])->data(),
                           static_cast<telldata::ttlayout*>(added->mlist()[j])->layer(), 
                           DATC->TEDLIB());
      }
   DATC->unlockDB();   
   delete failed;
   delete deleted;
   delete added;
   delete p1; delete p2;
   RefreshGL();
}

int tellstdfunc::stdMOVESEL::execute()
{
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   // move_selected returns 3 select lists : Failed/Deleted/Added
   // This is because of the modify operations
   laydata::selectList* fadead[3];
   byte i;
   for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::selectList());
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->move_selected(TP(p1->x(), p1->y(), DBscale), TP(p2->x(), p2->y(), DBscale), fadead);
      // save for undo operations ... 
      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
      UNDOPstack.push_front(make_ttlaylist(fadead[2])); // and added
      for (i = 0; i < 3; i++)
      {
         for (laydata::selectList::iterator CI = fadead[i]->begin(); CI != fadead[i]->end(); CI++)
         {
            laydata::dataList* sshape = CI->second;
            if (1 == i) // deleted list only
            {
               for (laydata::dataList::iterator CCI = sshape->begin(); CCI  != sshape->end(); CCI++)
               {
                  if (0 != CCI->second.size()) CCI->second.clear();
               }
            }
            delete sshape;
         }
         delete fadead[i];
      }
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << "," << *p2 << ");"; LogFile.flush();
   //delete p1; delete p2; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdMOVESEL_D::stdMOVESEL_D(telldata::typeID retype, bool eor) :
      stdMOVESEL(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdMOVESEL_D::execute()
{
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to move");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_move, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   OPstack.push(DEBUG_NEW telldata::ttpnt(w->p1().x(), w->p1().y()));
   OPstack.push(DEBUG_NEW telldata::ttpnt(w->p2().x(), w->p2().y()));
   delete w;
   return stdMOVESEL::execute();
}

//=============================================================================
tellstdfunc::stdROTATESEL::stdROTATESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdROTATESEL::undo_cleanup()
{
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   getOpValue(UNDOPstack, false);
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(deleted);
   delete added;
   delete deleted;
   delete failed;
   delete p1;
}

void tellstdfunc::stdROTATESEL::undo()
{
   TEUNDO_DEBUG("rotate(point real) UNDO");
   telldata::ttlist* added = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* deleted = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   telldata::ttlist* failed = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   real   angle  = 360 - getOpValue(UNDOPstack, true);
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->unselect_fromList(get_ttlaylist(failed));
      ATDB->unselect_fromList(get_ttlaylist(added));
      laydata::selectList* fadead[3];
      byte i;
      for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::selectList());
      ATDB->rotate_selected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      //@TODO Here - an internal check can be done - all 3 of the fadead lists
      // MUST be empty, otherwise - god knows what's wrong!
      for (i = 0; i < 3; delete fadead[i++]);
      ATDB->select_fromList(get_ttlaylist(failed));
      // put back the replaced (deleted) shapes
      ATDB->addlist(get_shlaylist(deleted));
      // and select them
      ATDB->select_fromList(get_ttlaylist(deleted));
      // delete the added shapes
      for (word j = 0 ; j < added->mlist().size(); j++)
      {
         ATDB->destroy_this(static_cast<telldata::ttlayout*>(added->mlist()[j])->data(),
                           static_cast<telldata::ttlayout*>(added->mlist()[j])->layer(),
                           DATC->TEDLIB());
      }
   DATC->unlockDB();
   delete failed;
   delete deleted;
   delete added;
   delete p1;
   RefreshGL();
}

int tellstdfunc::stdROTATESEL::execute()
{
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real   angle  = getOpValue();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttreal(angle));
   real DBscale = DATC->DBscale();
   // rotate_selected returns 3 select lists : Failed/Deleted/Added
   // This is because of the box rotation in which case box has to be converted to polygon
   // Failed shapes here should not exist but no explicit check for this
   laydata::selectList* fadead[3];
   byte i;
   for (i = 0; i < 3; fadead[i++] = DEBUG_NEW laydata::selectList());
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->rotate_selected(TP(p1->x(), p1->y(), DBscale), angle, fadead);
      telldata::ttlist* added = make_ttlaylist(fadead[2]);
      ATDB->select_fromList(get_ttlaylist(added));
      // save for undo operations ... 
      UNDOPstack.push_front(make_ttlaylist(fadead[0])); // first failed
      UNDOPstack.push_front(make_ttlaylist(fadead[1])); // then deleted
      UNDOPstack.push_front(added); // and added
      for (i = 0; i < 3; delete fadead[i++]);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< angle << "," << *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdROTATESEL_D::stdROTATESEL_D(telldata::typeID retype, bool eor) :
      stdROTATESEL(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

int tellstdfunc::stdROTATESEL_D::execute()
{
   real   angle  = getOpValue();
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to rotate");
      return EXEC_NEXT;
   }
   CTM rct;
   rct.Rotate(angle);
   OPstack.push(DEBUG_NEW telldata::ttreal(angle));
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_rotate, &OPstack, "", rct)) return EXEC_ABORT;
   return stdROTATESEL::execute();
}


//=============================================================================
tellstdfunc::stdFLIPXSEL::stdFLIPXSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdFLIPXSEL::undo_cleanup()
{
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete p1;
}

void tellstdfunc::stdFLIPXSEL::undo()
{
   TEUNDO_DEBUG("flipX(point) UNDO");
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), true);
   DATC->unlockDB();
   delete p1; 
   RefreshGL();
}

int tellstdfunc::stdFLIPXSEL::execute()
{
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), true);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPXSEL_D::stdFLIPXSEL_D(telldata::typeID retype, bool eor) :
      stdFLIPXSEL(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdFLIPXSEL_D::execute()
{
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to flip");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_flipX, &OPstack)) return EXEC_ABORT;
   return stdFLIPXSEL::execute();
}

//=============================================================================
tellstdfunc::stdFLIPYSEL::stdFLIPYSEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdFLIPYSEL::undo_cleanup()
{
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete p1;
}

void tellstdfunc::stdFLIPYSEL::undo()
{
   TEUNDO_DEBUG("flipY(point) UNDO");
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), false);
   DATC->unlockDB();
   delete p1; 
   RefreshGL();
}

int tellstdfunc::stdFLIPYSEL::execute()
{
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   telldata::ttpnt    *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = DATC->DBscale();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->flip_selected(TP(p1->x(), p1->y(), DBscale), false);
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
   //delete p1; undo will delete them
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdFLIPYSEL_D::stdFLIPYSEL_D(telldata::typeID retype, bool eor) :
      stdFLIPYSEL(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdFLIPYSEL_D::execute()
{
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to flip");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_flipY, &OPstack)) return EXEC_ABORT;
   return stdFLIPYSEL::execute();
}

//=============================================================================
tellstdfunc::stdDELETESEL::stdDELETESEL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdDELETESEL::undo_cleanup()
{
   telldata::ttlist* und = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(und);
   delete und;
}

void tellstdfunc::stdDELETESEL::undo()
{
   TEUNDO_DEBUG("delete() UNDO");
   telldata::ttlist* und = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->addlist(get_shlaylist(und));
      ATDB->select_fromList(get_ttlaylist(und));
   DATC->unlockDB();   
   delete (und);
   UpdateLV();
}

int tellstdfunc::stdDELETESEL::execute()
{
   UNDOcmdQ.push_front(this);
   laydata::atticList* sh_delist = DEBUG_NEW laydata::atticList();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->delete_selected(sh_delist, DATC->TEDLIB());
   DATC->unlockDB();   
   UNDOPstack.push_front(make_ttlaylist(sh_delist));
   clean_atticlist(sh_delist); delete sh_delist;
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   UpdateLV();   
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY::lgcCUTPOLY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_pnt)));
}

void tellstdfunc::lgcCUTPOLY::undo_cleanup()
{
   telldata::ttlist* pl4 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl3 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl2 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(pl3); // deleted shapes
   delete pl1; delete pl2;
   delete pl3; delete pl4;
}

void tellstdfunc::lgcCUTPOLY::undo()
{
   TEUNDO_DEBUG("cutpoly() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // now unselect all
      ATDB->unselect_all();
      // get the list of cut-offs
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      delete pl;
      // now get the list of cuts ...
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      delete pl;
      // now get the list of deleted shapes
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // put them back
      ATDB->addlist(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // ... and restore the selection
      ATDB->select_fromList(get_ttlaylist(pl)); 
   DATC->unlockDB();
   delete pl;
   UpdateLV();   
}

int tellstdfunc::lgcCUTPOLY::execute()
{
   if (DATC->numselected() == 0) 
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
   else {
      // get the data from the stack
      telldata::ttlist *pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
      real DBscale = DATC->DBscale();
      pointlist *plist = t2tpoints(pl,DBscale);
      laydata::valid_poly check(*plist);
      delete plist;
      if (!check.valid()) {
         tell_log(console::MT_ERROR, "Invalid cutting polygon encountered");
      }
      else {
         //cutpoly returns 3 Attic lists -> Delete/AddSelect/AddOnly,  
         // create and initialize them here
         laydata::atticList* dasao[3];
         for (byte i = 0; i < 3; dasao[i++] = DEBUG_NEW laydata::atticList());
         laydata::tdtdesign* ATDB = DATC->lockDB();
            if (ATDB->cutpoly(check.get_validated() ,dasao)) {
               // push the command for undo
               UNDOcmdQ.push_front(this);
               UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
               // unselect everything
               ATDB->unselect_all();

               telldata::ttlist* shdeleted = make_ttlaylist(dasao[0]);
               // select the shapes to delete & delete them ...
               ATDB->select_fromList(get_ttlaylist(shdeleted));
               laydata::atticList* sh_delist = DEBUG_NEW laydata::atticList();
               ATDB->delete_selected(sh_delist, DATC->TEDLIB());
               // ... not forgetting to save them in the undo data stack for undo
               UNDOPstack.push_front(make_ttlaylist(sh_delist));
               // clean-up the delete attic list
               clean_atticlist(sh_delist); delete sh_delist;
               delete shdeleted;

               // add the result of the cut...
               telldata::ttlist* shaddselect = make_ttlaylist(dasao[1]);
               telldata::ttlist* shaddonly = make_ttlaylist(dasao[2]);
               ATDB->addlist(dasao[1]);
               UNDOPstack.push_front(shaddselect);
               // ... the cut-offs ....
               ATDB->addlist(dasao[2]);
               UNDOPstack.push_front(shaddonly);
               // and finally select the_cut
               ATDB->select_fromList(get_ttlaylist(shaddselect));
               LogFile << "polycut("<< *pl << ");"; LogFile.flush();
               clean_atticlist(dasao[0]); delete (dasao[0]);
               // delete dasao[1]; delete dasao[2]; - deleted by ATDB->addlist
            }
         DATC->unlockDB();
      }
      delete pl;
      UpdateLV();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::lgcCUTPOLY_I::lgcCUTPOLY_I(telldata::typeID retype, bool eor) :
      lgcCUTPOLY(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::lgcCUTPOLY_I::execute()
{
   if (DATC->numselected() == 0) {
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dpoly, &OPstack)) return EXEC_ABORT;
   return lgcCUTPOLY::execute();
}

//=============================================================================
tellstdfunc::lgcCUTBOX_I::lgcCUTBOX_I(telldata::typeID retype, bool eor) :
      lgcCUTPOLY(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::lgcCUTBOX_I::execute()
{
   if (DATC->numselected() == 0) {
      tell_log(console::MT_ERROR,"No selected shapes. Nothing to cut");
      return EXEC_NEXT;
   }
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   telldata::ttwnd *bx = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();

   telldata::ttlist *pl = DEBUG_NEW telldata::ttlist(telldata::tn_pnt);
   pl->add(DEBUG_NEW telldata::ttpnt(bx->p1().x(), bx->p1().y()));
   pl->add(DEBUG_NEW telldata::ttpnt(bx->p1().x(), bx->p2().y()));
   pl->add(DEBUG_NEW telldata::ttpnt(bx->p2().x(), bx->p2().y()));
   pl->add(DEBUG_NEW telldata::ttpnt(bx->p2().x(), bx->p1().y()));
   OPstack.push(pl);
   delete bx;
   return lgcCUTPOLY::execute();
}

//=============================================================================
tellstdfunc::lgcMERGE::lgcMERGE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::lgcMERGE::undo_cleanup()
{
   telldata::ttlist* pl3 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl2 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(pl2);
   delete pl1; delete pl2;
   delete pl3;
}

void tellstdfunc::lgcMERGE::undo()
{
   TEUNDO_DEBUG("merge() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // now unselect all
      ATDB->unselect_all();
      // get the shapes resulted from the merge operation
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      delete pl;
      // now get the list of deleted shapes
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // put them back
      ATDB->addlist(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // ... and restore the selection
      ATDB->select_fromList(get_ttlaylist(pl)); 
   DATC->unlockDB();   
   delete pl;
   UpdateLV();   
}

int tellstdfunc::lgcMERGE::execute()
{
   if (DATC->numselected() == 0) {
      tell_log(console::MT_ERROR,"No objects selected. Nothing to cut");
   }
   else {
      //merge returns 2 Attic lists -> Delete/AddMerged
      // create and initialize them here
      laydata::atticList* dasao[2];
     byte i;
      for (i = 0; i < 2; dasao[i++] = DEBUG_NEW laydata::atticList());
      // create a list of currently selected shapes
      laydata::tdtdesign* ATDB = DATC->lockDB();
         telldata::ttlist* listselected = make_ttlaylist(ATDB->shapesel());
         if (ATDB->merge(dasao)) {
/*-!-*/     DATC->unlockDB();
            // push the command for undo
            UNDOcmdQ.push_front(this);
            // save the list of originally selected shapes
            UNDOPstack.push_front(listselected);
            // save the list of deleted shapes
            UNDOPstack.push_front(make_ttlaylist(dasao[0]));
            // add the result of the merge...
            UNDOPstack.push_front(make_ttlaylist(dasao[1]));
            LogFile << "merge( );"; LogFile.flush();
         }
         else {
/*-!-*/     DATC->unlockDB();
            delete listselected;
         }   
      // clean-up the lists
      for (i = 0; i < 2; i++)
      {
         clean_atticlist(dasao[i]);
         delete(dasao[i]);
      }
   }
   UpdateLV();
   return EXEC_NEXT;
}



//=============================================================================
tellstdfunc::lgcSTRETCH::lgcSTRETCH(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttreal()));
}

void tellstdfunc::lgcSTRETCH::undo_cleanup()
{
   telldata::ttlist* pl3 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl2 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(pl3); // deleted shapes
   delete pl1; delete pl2;
   delete pl3;
}

void tellstdfunc::lgcSTRETCH::undo()
{
   TEUNDO_DEBUG("bloat() UNDO");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // now unselect all
      ATDB->unselect_all();
      // now get the list of cuts ...
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      delete pl;
      // now get the list of deleted shapes
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // put them back
      ATDB->addlist(get_shlaylist(pl));
      delete pl;
      // and finally, get the list of shapes being selected before the cut
      pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // ... and restore the selection
      ATDB->select_fromList(get_ttlaylist(pl)); 
   DATC->unlockDB();
   delete pl;
   UpdateLV();
}

int tellstdfunc::lgcSTRETCH::execute()
{
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No object selected. Nothing to modify");
   }
   else
   {
      real bfactor = getOpValue();
      if (0.0 == bfactor)
      {
         tell_log(console::MT_WARNING,"Resize argument is 0. Nothing was changed");
      }
      else
      {
         //expand/shrink returns 2 Attic lists -> Delete/AddSelect,
         // create and initialize them here
         laydata::atticList* dasao[2];
         for (byte i = 0; i < 2; dasao[i++] = DEBUG_NEW laydata::atticList());
         laydata::tdtdesign* ATDB = DATC->lockDB();
            real DBscale = DATC->DBscale();
            if (ATDB->stretch((int) rint(bfactor * DBscale), dasao))
            {
               // push the command for undo
               UNDOcmdQ.push_front(this);
               // put the list of selected shapes in undo stack
               UNDOPstack.push_front(make_ttlaylist(ATDB->shapesel()));
               // unselect everything
               ATDB->unselect_all();

               telldata::ttlist* shdeleted = make_ttlaylist(dasao[0]);
               // select the shapes to delete & delete them ...
               ATDB->select_fromList(get_ttlaylist(shdeleted));
               laydata::atticList* sh_delist = DEBUG_NEW laydata::atticList();
               ATDB->delete_selected(sh_delist, DATC->TEDLIB());
               // ... not forgetting to save them in the undo data stack for undo
               UNDOPstack.push_front(make_ttlaylist(sh_delist));
               // clean-up the delete attic list
               clean_atticlist(sh_delist); delete sh_delist;
               delete shdeleted;

               // add the result of the expand/shrink...
               telldata::ttlist* shaddselect = make_ttlaylist(dasao[1]);
               ATDB->addlist(dasao[1]);
               UNDOPstack.push_front(shaddselect);
               // and finally select the_cut
               ATDB->select_fromList(get_ttlaylist(shaddselect));
               LogFile << "resize("<< bfactor << ");"; LogFile.flush();
               clean_atticlist(dasao[0]); delete (dasao[0]);
               // delete dasao[1]; delete dasao[2]; - deleted by ATDB->addlist
            }
            else
            {
               for (byte i = 0; i < 2; delete dasao[i++]);
            }
         DATC->unlockDB();
         UpdateLV();
      }
   }
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdCHANGELAY::stdCHANGELAY(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdCHANGELAY::undo_cleanup()
{
   getWordValue(UNDOPstack, false);
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdCHANGELAY::undo()
{
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   word src = getWordValue(UNDOPstack, true);
   laydata::tdtdesign* ATDB = DATC->lockDB();
      ATDB->transferLayer(get_ttlaylist(pl), src);
   DATC->unlockDB();
   delete pl;
   RefreshGL();
}

int tellstdfunc::stdCHANGELAY::execute()
{
   laydata::tdtdesign* ATDB = DATC->lockDB();
      laydata::selectList *listselected = ATDB->shapesel();
   DATC->unlockDB();
   if (listselected->empty())
   {
      std::ostringstream ost;
      ost << "No objects selected";
      tell_log(console::MT_ERROR,ost.str());
   }
   else
   {
      // prepare undo stacks
      UNDOcmdQ.push_front(this);
      word target = getWordValue();
      UNDOPstack.push_front(DEBUG_NEW telldata::ttint(target));
      UNDOPstack.push_front(make_ttlaylist(listselected));
      ATDB = DATC->lockDB();
         ATDB->transferLayer(target);
      DATC->unlockDB();
      LogFile << "changelayer("<< target << ");";LogFile.flush();
      RefreshGL();
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdCHANGEREF::stdCHANGEREF(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdCHANGEREF::undo_cleanup()
{
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
   delete pl1;
}

void tellstdfunc::stdCHANGEREF::undo()
{
   TEUNDO_DEBUG("ungroup() CHANGEREF");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // first save the list of all currently selected components
      laydata::selectList *savelist = ATDB->copy_selist();
      // now unselect all
      ATDB->unselect_all();
      // get the list of DEBUG_NEW references from the UNDO stack
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      // now get the list of the old cell ref's from the UNDO stack
      telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // and add them to the target cell
      ATDB->addlist(get_shlaylist(pl1)); 
      // select the restored cell refs
      ATDB->select_fromList(get_ttlaylist(pl1)); 
      // now restore selection
      ATDB->select_fromList(savelist);
   DATC->unlockDB();
   // finally - clean-up behind
   delete pl;
   delete pl1;
   UpdateLV();
}

int tellstdfunc::stdCHANGEREF::execute()
{
   std::string newref = getStringValue();
   laydata::shapeList* cells4u = NULL;
   laydata::tdtdesign* ATDB = DATC->lockDB();
      bool refok = ATDB->checkValidRef(newref);
      if (refok)
         cells4u = ATDB->ungroup_prep(DATC->TEDLIB());
   DATC->unlockDB();
   if (refok)
   {
      if (cells4u->empty())
      {
         tell_log(console::MT_ERROR,"No cell references selected");
         delete cells4u;
      }
      else
      {
         ATDB = DATC->lockDB();
            laydata::atticList* undol2 = ATDB->changeref(cells4u, newref);
         DATC->unlockDB();
         assert(NULL != undol2);
         UNDOcmdQ.push_front(this);
         // Push the list of the cells to be ungroupped first
         laydata::atticList undol;
         undol[0] = cells4u;
         UNDOPstack.push_front(make_ttlaylist(&undol));
         UNDOPstack.push_front(make_ttlaylist(undol2));
         delete cells4u;
         delete undol2;
         LogFile << LogFile.getFN() << "(\"" << newref << "\");"; LogFile.flush();
         RefreshGL();
      }
   }
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdCHANGESTRING::stdCHANGESTRING(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttstring()));
}

void tellstdfunc::stdCHANGESTRING::undo_cleanup()
{
   telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   clean_ttlaylist(pl1);delete pl;
   delete pl1;
}

void tellstdfunc::stdCHANGESTRING::undo()
{
   TEUNDO_DEBUG("ungroup() CHANGESTR");
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // first save the list of all currently selected components
      laydata::selectList *savelist = ATDB->copy_selist();
      // now unselect all
      ATDB->unselect_all();
      // get the list of new texts from the UNDO stack
      telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // select them ...
      ATDB->select_fromList(get_ttlaylist(pl));
      //... and delete them cleaning up the memory (don't store in the Attic)
      ATDB->delete_selected(NULL, DATC->TEDLIB());
      // now get the list of the old text objects from the UNDO stack
      telldata::ttlist* pl1 = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
      // and add them to the target cell
      ATDB->addlist(get_shlaylist(pl1)); 
      // select the restored cell refs
      ATDB->select_fromList(get_ttlaylist(pl1)); 
      // now restore selection
      ATDB->select_fromList(savelist);
   DATC->unlockDB();
   // finally - clean-up behind
   delete pl;
   delete pl1;
   RefreshGL();
}

int tellstdfunc::stdCHANGESTRING::execute()
{
   std::string newstring = getStringValue();
   laydata::tdtdesign* ATDB = DATC->lockDB();
      // first save the list of all currently selected components ...
      laydata::selectList* savelist = ATDB->copy_selist();
      // get a list of selected texts only
      laydata::selectList* texts4u = filter_selist(savelist, laydata::_lmtext);
      if (texts4u->empty())
      {
         for (laydata::selectList::const_iterator CL = savelist->begin(); CL != savelist->end(); CL++)
            delete CL->second;
         delete savelist;
         for (laydata::selectList::const_iterator CL = texts4u->begin(); CL != texts4u->end(); CL++)
            delete CL->second;
         delete texts4u;
         tell_log(console::MT_ERROR,"No text objects selected");
      }
      else
      {// just if we have selected texts
         UNDOcmdQ.push_front(this);
         // now unselect all ...
         ATDB->unselect_all();
         // ... and select back only text shapes
         ATDB->select_fromList(texts4u);
         // delete them from the DB - get back the list of deleted shapes.
         laydata::atticList* fha = DEBUG_NEW laydata::atticList();
         ATDB->delete_selected(fha, DATC->TEDLIB());
         // save the deleted shapes in the UNDO data stack
         UNDOPstack.push_front(make_ttlaylist(fha));
         // replace the strings
         laydata::atticList* fhba = replace_str(fha, newstring);
         telldata::ttlist* fhb = make_ttlaylist(fhba);
         // save the new texts in the UNDO data stack
         UNDOPstack.push_front(fhb);
         // add the new objects back to the DB
         ATDB->addlist(get_shlaylist(fhb));
         // now restore selection
         ATDB->select_fromList(savelist);
         ATDB->select_fromList(get_ttlaylist(fhb));
         // that's it!
         clean_atticlist(fha); delete fha;
         clean_atticlist(fhba);delete fhba;
         LogFile << LogFile.getFN() << "(\"" << newstring << "\");"; LogFile.flush();
         RefreshGL();
      }
   DATC->unlockDB();
   return EXEC_NEXT;
}
/*   laydata::selectList* texts4u = filter_selected(laydata::_lmtext);
   if (texts4u->empty())
   {
      tell_log(console::MT_ERROR,"No text objetcs selected");
   }
   else
   {
      laydata::tdtdesign* ATDB = DATC->lockDB();
         laydata::atticList* undol2 = ATDB->changestring(texts4u, newstring);
      DATC->unlockDB();
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
      RefreshGL();
   }
  delete texts4u;*/
