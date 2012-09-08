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
//        Created: Fri Jan 24 2003
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Implementation of all TELL build-in functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <math.h>
#include <sstream>
#include "tellibin.h"
#include "ted_prompt.h"
#include "tedat.h"
#include "datacenter.h"
#include "viewprop.h"
#include "tuidefs.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;
extern console::TllCmdLine*      Console;

extern wxFrame*                  TopedMainW;
extern wxWindow*                 TopedCanvasW;


extern const wxEventType         wxEVT_CANVAS_PARAMS;
extern const wxEventType         wxEVT_CANVAS_ZOOM;
extern const wxEventType         wxEVT_MOUSE_INPUT;
extern const wxEventType         wxEVT_CANVAS_CURSOR;


//=============================================================================
int tellstdfunc::stdSPRINTF::argsOK(argumentQ* amap, bool& strict)
{
   strict = true;
   unsigned numArgs = amap->size();
   if (1 > numArgs) return -1; // i.e. error - at least one argument is expected
   telldata::ArgumentID carg((*(*amap)[0]));
   if (telldata::tn_string != carg()) return -1; //i.e. error - first argument must be a string
   for (unsigned i = 1; i < numArgs; i++)
   {
      carg = (*(*amap)[i]);
      if (!PRINTF_ABLE(carg()))
         return -1;
   }
   return 0; // OK
}

NameList* tellstdfunc::stdSPRINTF::callingConv(const telldata::typeMAP*)
{
   NameList* argtypes = DEBUG_NEW NameList();
   argtypes->push_back("void");
   argtypes->push_back("string");
   argtypes->push_back("<...>");
   return argtypes;
}

std::string tellstdfunc::stdSPRINTF::getHelp()
{
   std::string str("Write formatted data to the Tell log\n");
   str += "void printf( format [,param [,param [,…]]] )";
   return str;
}

int tellstdfunc::stdSPRINTF::execute()
{
   word numParams = getWordValue();
   assert(numParams > 0);
   std::stack<telldata::TellVar*> varstack;
   // get the function arguments from the argument stack using the info in
   // the arguments structure and push them in a local stack structure.
   while (numParams--)
   {
      telldata::TellVar *op = OPstack.top();OPstack.pop();
      varstack.push(op);
   }
   // OK the first one must be the format string - let's get it out
   //
   telldata::TtString* fttstr = static_cast<telldata::TtString*>(varstack.top());
   std::string workS(fttstr->value());
   delete fttstr;
   replaceESCs(workS);
   varstack.pop();
   int status = 1;
   while (!varstack.empty())
   { // the idea of this cycle is to loop until varstack is empty and in all
     // cases to clean-up the heap (varstack contents). Of course it is supposed
     // to replace the format strings from the original string, but this is just
     // in case if there is no error in the operation so far.
      telldata::TellVar* op = varstack.top();
      varstack.pop();
      if (1 == status)
         status = replaceNextFstr(workS, op);
      delete op;
   }
   // yet another check here - make sure that there is no more format sequences
   // remaining in the string.
   if (checkFstr(workS))
   {
      tell_log(console::MT_ERROR, "printf call contains less arguments than the number of %-tags in the format string");
      OPstack.push(DEBUG_NEW telldata::TtString());
      return EXEC_ABORT;
   }
   else if ( 1 == status)
   {
      OPstack.push(DEBUG_NEW telldata::TtString(workS));
      return EXEC_NEXT;
   }
   else if ( 0 == status)
   {  // more parameters than expected
      tell_log(console::MT_ERROR, "printf call contains more arguments than the number of %-tags in the format string");
      OPstack.push(DEBUG_NEW telldata::TtString());
      return EXEC_ABORT;
   }
   else if (-1 == status)
   {
      tell_log(console::MT_ERROR, "printf discrepancy between the format string and argument type");
      OPstack.push(DEBUG_NEW telldata::TtString());
      return EXEC_ABORT;
   }
   assert(false);
   return EXEC_ABORT;
}

//=============================================================================
int tellstdfunc::stdPRINTF::argsOK(argumentQ* amap, bool& strict)
{
   return stdSPRINTF::argsOK(amap, strict);
}

NameList* tellstdfunc::stdPRINTF::callingConv(const telldata::typeMAP*)
{
   NameList* argtypes = DEBUG_NEW NameList();
   argtypes->push_back("string");
   argtypes->push_back("string");
   argtypes->push_back("<...>");
   return argtypes;
}

int tellstdfunc::stdPRINTF::execute()
{
   int result = stdSPRINTF::execute();
   if (EXEC_NEXT == result)
      tell_log(console::MT_GUIINPUT,getStringValue());
   return result;
}

//=============================================================================
int tellstdfunc::stdECHO::argsOK(argumentQ* amap, bool& strict)
{
   strict = true;
   return (!(amap->size() == 1));
}

NameList* tellstdfunc::stdECHO::callingConv(const telldata::typeMAP*)
{
   NameList* argtypes = DEBUG_NEW NameList();
   argtypes->push_back("void");
   argtypes->push_back("<...anything...>");
   return argtypes;
}

std::string tellstdfunc::stdECHO::getHelp()
{
   std::string str("Prints the value of a TELL variable\n");
   str += "void echo( variable )";
   return str;
}

int tellstdfunc::stdECHO::execute()
{
   telldata::TellVar *p = OPstack.top();OPstack.pop();
   std::ostringstream ost;
   echoWrapper(p,ost);
   tell_log(console::MT_INFO,ost.str());
   delete p;
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdTELLSTATUS::stdTELLSTATUS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdTELLSTATUS::execute()
{
#ifdef DB_MEMORY_TRACE
   MemTrack::TrackListMemoryUsage();
#else
   real DBscale = PROPC->DBscale();
   telldata::TellVar *y;
   std::string news;
   while (OPstack.size()) {
      y = OPstack.top(); OPstack.pop();
      y->echo(news, DBscale);
      tell_log(console::MT_ERROR,news);
   }
   news = "Bottom of the operand stack reached";
   tell_log(console::MT_ERROR,news);
#endif
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDISTANCE::stdDISTANCE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtList(telldata::tn_pnt)));
}

int tellstdfunc::stdDISTANCE::execute()
{
   // get the data from the stack
   telldata::TtList *pl = static_cast<telldata::TtList*>(OPstack.top());OPstack.pop();

   telldata::TtPnt* p1 = NULL;
   telldata::TtPnt* p2 = NULL;
   for (unsigned i = 0; i < pl->size(); i++) {
      p1 = p2;
      p2 = static_cast<telldata::TtPnt*>((pl->mlist())[i]);
      if (NULL != p1)
      {
         TP ap1 = TP(p1->x(),p1->y(), PROPC->DBscale());
         TP ap2 = TP(p2->x(),p2->y(), PROPC->DBscale());
         PROPC->addRuler(ap1,ap2);
/*         std::ostringstream info;
         info << "Distance {" << p1->x() << " , " << p1->y() <<"}  -  {"
                              << p2->x() << " , " << p2->y() <<"}  is ";
         tell_log(console::MT_WARNING,info.str());*/
      }
   }
   delete pl;
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdDISTANCE_D::stdDISTANCE_D(telldata::typeID retype, bool eor) :
      stdDISTANCE(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
}

int tellstdfunc::stdDISTANCE_D::execute()
{
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_line, &OPstack)) return EXEC_ABORT;
   // get the data from the stack
   telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   telldata::TtList* plst = DEBUG_NEW telldata::TtList(telldata::tn_pnt);
   plst->add(DEBUG_NEW telldata::TtPnt(w->p1().x(), w->p1().y()));
   plst->add(DEBUG_NEW telldata::TtPnt(w->p2().x(), w->p2().y()));
   OPstack.push(plst);
   delete w;
   return stdDISTANCE::execute();
}

//=============================================================================
tellstdfunc::stdCLEARRULERS::stdCLEARRULERS(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdCLEARRULERS::execute()
{
   PROPC->clearRulers();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdLONGCURSOR::stdLONGCURSOR(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtBool()));
}

int tellstdfunc::stdLONGCURSOR::execute()
{
   bool        longcur  = getBoolValue();

   wxCommandEvent eventGRIDUPD(wxEVT_CANVAS_PARAMS);
   eventGRIDUPD.SetId(tui::CPS_LONG_CURSOR);
   eventGRIDUPD.SetInt((longcur ? 1 : 0));
   wxPostEvent(TopedMainW, eventGRIDUPD);

   wxCommandEvent eventCNVS(wxEVT_CANVAS_CURSOR);
   eventCNVS.SetInt((longcur ? 1 : 0));
   wxPostEvent(TopedCanvasW, eventCNVS);

   LogFile << LogFile.getFN() << "(" << LogFile._2bool(longcur) << ");"; LogFile.flush();
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNDO::stdUNDO(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdUNDO::execute() {
   if (UNDOcmdQ.size() > 0) {
      UNDOcmdQ.front()->undo(); UNDOcmdQ.pop_front();
      LogFile << LogFile.getFN() << "();"; LogFile.flush();
   }
   else {
      std::string news = "UNDO buffer is empty";
      tell_log(console::MT_ERROR,news);
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREDRAW::stdREDRAW(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdREDRAW::execute()
{
   RefreshGL();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMWIN::stdZOOMWIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtPnt()));
}

int tellstdfunc::stdZOOMWIN::execute() {
   telldata::TtPnt *p1 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   telldata::TtPnt *p2 = static_cast<telldata::TtPnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   DBbox* box = DEBUG_NEW DBbox(TP(p1->x(), p1->y(), DBscale),
                          TP(p2->x(), p2->y(), DBscale));
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(TopedCanvasW, eventZOOM);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMWINb::stdZOOMWINb(telldata::typeID retype, bool eor) :
      stdZOOMWIN(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtWnd()));
}

int tellstdfunc::stdZOOMWINb::execute() {
   telldata::TtWnd *w = static_cast<telldata::TtWnd*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   DBbox* box = DEBUG_NEW DBbox(TP(w->p1().x(), w->p1().y(), DBscale),
                          TP(w->p2().x(), w->p2().y(), DBscale));
   wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
   eventZOOM.SetInt(tui::ZOOM_WINDOW);
   eventZOOM.SetClientData(static_cast<void*>(box));
   wxPostEvent(TopedCanvasW, eventZOOM);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMALL::stdZOOMALL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{}

int tellstdfunc::stdZOOMALL::execute() {
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      DBbox* ovl  = DEBUG_NEW DBbox(tDesign->activeOverlap());
      wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
      eventZOOM.SetInt(tui::ZOOM_WINDOW);
      eventZOOM.SetClientData(static_cast<void*>(ovl));
      wxPostEvent(TopedCanvasW, eventZOOM);
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdZOOMVISIBLE::stdZOOMVISIBLE(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype, eor)
{}

int tellstdfunc::stdZOOMVISIBLE::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      layprop::DrawProperties* drawProp;
      if (PROPC->lockDrawProp(drawProp))
      {
         DBbox* ovl  = DEBUG_NEW DBbox(tDesign->getVisibleOverlap(*drawProp));
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(tui::ZOOM_WINDOW);
         eventZOOM.SetClientData(static_cast<void*>(ovl));
         wxPostEvent(TopedCanvasW, eventZOOM);
      }
      // WARNING! Don't throw an exception here. Otherwise the TDT mutex will remain locked!
      PROPC->unlockDrawProp(drawProp, false);
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::getPOINT::getPOINT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::getPOINT::execute() {
   // flag the prompt that we expect a single point & handle a pointer to
   // the operand stack
   Console->waitGUInput(&OPstack, console::op_point, CTM());
   //
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(-1);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Console->_threadWaits4->Wait();
   // ... and continue when the thread is woken up
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   if (Console->mouseIN_OK())  return EXEC_NEXT;
   else return EXEC_RETURN;
}

//=============================================================================
tellstdfunc::getPOINTLIST::getPOINTLIST(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::getPOINTLIST::execute() {
   // flag the prompt that we expect a list of points & handle a pointer to
   // the operand stack
   Console->waitGUInput(&OPstack, console::op_dpoly, CTM());
   //
   wxCommandEvent eventMOUSEIN(wxEVT_MOUSE_INPUT);
   eventMOUSEIN.SetInt(-1);
   eventMOUSEIN.SetExtraLong(1);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   // force the thread in wait condition until the ted_prompt has our data
   Console->_threadWaits4->Wait();
   // ... and continue when the thread is woken up
   eventMOUSEIN.SetExtraLong(0);
   wxPostEvent(TopedCanvasW, eventMOUSEIN);
   if (Console->mouseIN_OK())  return EXEC_NEXT;
   else return EXEC_RETURN;
}

//=============================================================================
tellstdfunc::stdEXEC::stdEXEC(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{
   _arguments->push_back(DEBUG_NEW ArgumentTYPE("", DEBUG_NEW telldata::TtString()));
}

int tellstdfunc::stdEXEC::execute()
{
   std::string extCmd = getStringValue();
   if (_threadExecution)
   {
      Console->waitExternal(wxString(extCmd.c_str(), wxConvUTF8));
      Console->_threadWaits4->Wait();
   }
   else
   {
//      tell_log(console::MT_WARNING,"exit command in recovery mode ignored");
   }
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdEXIT::stdEXIT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::stdEXIT::execute()
{
   if (_threadExecution)
   {
      Console->setExitRequest(true);
      // Don't log the command. It's getting very messy if it gets
      // in recovery mode!
      //LogFile << LogFile.getFN() << "();"; LogFile.flush();
      return EXEC_ABORT;
   }
   else
   {
      tell_log(console::MT_WARNING,"exit command in recovery mode ignored");
      return EXEC_NEXT;
   }
}

//============================================================================
tellstdfunc::intrnlSORT_DB::intrnlSORT_DB(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::ArgumentLIST,retype,eor)
{}

int tellstdfunc::intrnlSORT_DB::execute()
{
   laydata::TdtLibDir* dbLibDir = NULL;
   if (DATC->lockTDT(dbLibDir, dbmxs_celllock))
   {
      laydata::TdtDesign* tDesign = (*dbLibDir)();
      tDesign->fixUnsorted();
      LogFile << "// $sort_db( );"; LogFile.flush();
   }
   DATC->unlockTDT(dbLibDir, true);
   return EXEC_NEXT;
}


int tellstdfunc::replaceNextFstr(std::string& str, telldata::TellVar* val)
{
   wxString wxStr(str.c_str(), wxConvUTF8);
   wxRegEx src_tmpl(tellstdfunc::fspec_tmpl);
   VERIFY(src_tmpl.IsValid());
   if (src_tmpl.Matches(wxStr))
   {
      size_t start, length;
      bool status = src_tmpl.GetMatch(&start, &length);
      assert(status); // i.e. if Matches() returned true, the same shall be returned by GetMatch()
      char* formatChars = new char[length+1];
      char replacement[1024]; // TODO - any chance for a decent length here?
      str.copy(formatChars, length, start);
      formatChars[length] = 0x00;
      int numChars = -1;
      switch (val->get_type())
      {
         case telldata::tn_int    :
            numChars = sprintf(replacement, formatChars, static_cast<telldata::TtInt*>(val)->value());
            break;
         case telldata::tn_real   :
            numChars = sprintf(replacement, formatChars, static_cast<telldata::TtReal*>(val)->value());
            break;
         case telldata::tn_bool   :
            numChars = sprintf(replacement, formatChars, static_cast<telldata::TtBool*>(val)->value());
            break;
         case telldata::tn_string : {
            std::string interStr = static_cast<telldata::TtString*>(val)->value();
            numChars = sprintf(replacement, formatChars, interStr.c_str());
            break;
         }
         default: assert(false); break;
      }
      if (0 > numChars)
         return -1; //format string found, but there is a discrepancy with the corresponding variable
      else
         str.replace(start, length, replacement);
      return 1; // OK
   }
   return 0; // format string not found
}

void tellstdfunc::replaceESCs(std::string& str)
{
//   dblesc_tmpl
   wxString wxStr(str.c_str(), wxConvUTF8);
   wxRegEx src_tmpl(tellstdfunc::esc_tmpl);
   VERIFY(src_tmpl.IsValid());
   if (src_tmpl.Matches(wxStr))
   {
      src_tmpl.ReplaceAll(&wxStr, wxT("\n"));
      str = std::string(wxStr.mb_str(wxConvUTF8));
   }
}

bool tellstdfunc::checkFstr(const std::string& str)
{
   wxString wxStr(str.c_str(), wxConvUTF8);
   wxRegEx src_tmpl(tellstdfunc::fspec_tmpl);
   VERIFY(src_tmpl.IsValid());
   return src_tmpl.Matches(wxStr);
}

/*
UNDO/REDO operation - some preliminary thoughts
   Where to fit it. Two major possibilities here:
      - generating string commands and using the parser
      - Maintaining UNDO command stack and UNDO operand stack
   1. At this moment seems that the first way has two major drawback - it will
   mess around with the threads. Undo will be a command that will have to
   reinvoke the parser to parse another command. The other thing that seems
   to be potentially hazardous is the maintenance of the operand stack - it
   seems impossible  to execute properly undo/redo if it is included somewhere
   in a function, script, file, preproc.definition or god knows where else in
   the language.
   2. Separate UNDO command/operand stack. This solution seems much more robust
   (and of course more complicated to implement). At least it is free of the
   troubles described above (though have others maybe...)
   The table below is trying to clarify operations over normal and UNDO operand
   stacks and when they are executed.
   ============================================================================
                    |    filled during        |         emptied
   ============================================================================
    CMD_STACK       |     parsing             |        execution
   ----------------------------------------------------------------------------
    UNDO CMD_STACK  | execution of UNDOable   | undo execution  OR reaching
                    |      command            |  the limit of UNDO CMD_STACK
   ----------------------------------------------------------------------------
    OP_STACK        |     execution           |        execution
   ----------------------------------------------------------------------------
    UNDO OP_STACK   | execution of UNDOable   |  undo execution  OR reaching
                    |      command            |  the limit of UNDO CMD_STACK
   ----------------------------------------------------------------------------
   What is needed for implementation...
    Additional method (UNDO) in the cmdSTDFUNC. Thus undo will be executed
    in a similar way as normal commands, but one command at a time - when
    undo() command is invoked - and calling undo() method instead of execute()
    This has a nasty drawback - all internal commands has to be implemented
    separately and initialy this will swallow a lot of time. It seems to be
    flexible though and will require minimum amount of additional toped functions.


=========================================================================================
|   TELL command   |   tellibin class  |  Docu  |   LOG  |  UNDO  |        Note         |
=========================================================================================
|  "echo"          |  stdECHO          |        |    -   |   -    |                     |
|  "status"        |  stdTELLSTATUS    |        |    -   |   -    |                     |
|  "newdesign"     |  stdNEWDESIGN     |   OK   |    x   |   x*   | if not initial      |
|  "newcell"       |  stdNEWCELL       |   OK   |    x   |   x*   | if not initial      |
|  "gdsread"       |  GDSread          |   OK   |    x   |   -    |                     |
|  "importcell"    |  GDSimport       |   OK   |    x   |   -*   | issue a warning     |
|  "tdtread"       |  TDTread          |   OK   |    x   |   -*   | issue a warning     |
|  "tdtsave"       |  TDTsave          |   OK   |    x   |   -    |                     |
|  "tdtsaveas"     |  TDTsaveas        |   OK   |    x   |   -    |                     |
|  "openCell"      |  stdOPENCELL      |   OK   |    x   |   x*   | if not initial      |
|  "usinglayer"    |  stdUSINGLAYER    |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdADDBOX        |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdADDBOX_D      |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdADDBOXr       |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdADDBOXr_D     |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdADDBOXp       |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdADDBOXp_D     |   OK   |    x   |   x    |                     |
|  "addBox"        |  stdDRAWBOX       |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addBox"        |  stdDRAWBOX_D     |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addPoly"       |  stdADDPOLY       |   OK   |    x   |   x    |                     |
|  "addPoly"       |  stdADDPOLY_D     |   OK   |    x   |   x    |                     |
|  "addPoly"       |  stdDRAWPOLY      |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addPoly"       |  stdDRAWPOLY_D    |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addWire"       |  stdADDWIRE       |   OK   |    x   |   x    |                     |
|  "addWire"       |  stdADDWIRE_D     |   OK   |    x   |   x    |                     |
|  "addWire"       |  stdDRAWWIRE      |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addWire"       |  stdDRAWWIRE_D    |   OK   |    x*  |   x    |   as add<blah-blah> |
|  "addText"       |  stdADDTEXT       |   OK   |    x   |   x    |                     |
|  "cellref"       |  stdCELLREF       |   OK   |    x   |   x    |                     |
|  "cellaref"      |  stdCELLAREF      |   OK   |    x   |   x    |                     |
|  "select"        |  stdSELECT        |   OK   |    x   |   x    |                     |
|  "select"        |  stdSELECT_I      |   OK   |    x   |   x    |                     |
|  "select"        |  stdSELECT_TL     |   OK   |    -*  |   x    |  see note 3         |
|  "pselect"       |  stdPNTSELECT     |   OK   |    x   |   x    |                     |
|  "pselect"       |  stdPNTSELECT_I   |   OK   |    x   |   x    |                     |
|  "unselect"      |  stdUNSELECT      |   OK   |    x   |   x    |                     |
|  "unselect"      |  stdUNSELECT_I    |   OK   |    x   |   x    |                     |
|  "unselect"      |  stdUNSELECT_TL   |   OK   |    -*  |   x    |  see note 3         |
|  "punselect"     |  stdPNTUNSELECT   |   OK   |    x   |   x    |                     |
|  "punselect"     |  stdPNTUNSELECT_I |   OK   |    x   |   x    |                     |
|  "selectAll"    |  stdSELECTALL     |   OK   |    x   |   x    |                     |
|  "unselectAll"  |  stdUNSELECTALL   |   OK   |    x   |   x    |                     |
-----------------------------------------------------------------------------------------
|      operation on the toped data     |        |        |        |                     |
-----------------------------------------------------------------------------------------
|  "move"          |  stdMOVESEL       |   OK   |    x   |   x    |                     |
|  "move"          |  stdMOVESEL_D     |   OK   |    x   |   x    |                     |
|  "copy"          |  stdCOPYSEL       |   OK   |    x   |   x    |                     |
|  "copy"          |  stdCOPYSEL_D     |   OK   |    x   |   x    |                     |
|  "delete"        |  stdDELETESEL     |   OK   |    x   |   x    |                     |
|  "group"         |  stdGROUP         |   OK   |    x   |   x    |                     |
|  "ungroup"       |  stdUNGROUP       |   OK   |    x   |   x    |                     |
-----------------------------------------------------------------------------------------
|        toped specific functons       |        |        |        |                     |
-----------------------------------------------------------------------------------------
|  "getCW"         |  stdGETCW         |   OK   |    -   |   -    |                     |
|  "zoom"          |  stdZOOMWIN       |   OK   |    -   |   -    |  see note 1         |
|  "zoom"          |  stdZOOMWINb      |   OK   |    -   |   -    |  see note 1         |
|  "zoomall"       |  stdZOOMALL       |   OK   |    -   |   -    |  see note 1         |
|  "layprop"       |  stdLAYPROP       |   OK   |    x   |   x*   |  if redefined only  |
|  "hidelayer"     |  stdHIDELAYER     |   OK   |    x   |   x    |                     |
|  "locklayer"     |  stdLOCKLAYER     |   OK   |    x   |   x    |                     |
|  "filllayer"     |  stdFILLLAYER     |   OK   |    x   |   x    |                     |
|  "definecolor"   |  stdCOLORDEF      |   OK   |    x   |   x*   |  if redefined only  |
|  "definefill"    |  stdFILLDEF       |   OK   |    x   |   x*   |  if redefined only  |
|  "definegrid"    |  stdGRIDDEF       |   OK   |    x   |   x*   |  if redefined only  |
|  "step"          |  stdSTEP          |   OK   |    x   |   x*   |  if redefined only  |
|  "grid"          |  stdGRID          |   OK   |    x   |   x    |                     |
|  "getpoint"      |  getPOINT         |   OK   |    -   |   -    |  see note 2         |
|  "getpointlist"  |  getPOINTLIST     |   OK   |    -   |   -    |                     |
=========================================================================================
   Note 1: For zooming a separate stack of available views can be maintained.
   Note 2: For getpoint operation, a local undo should be maintained. Not here,
           but in the tedprompt maybe.
   Note 3: It is a real trouble how to log select/unselect from list! Have no
           idea really!!! Something really to think hard...
*/

void tellstdfunc::echoWrapper(telldata::TellVar* p, std::ostringstream& ost)
{
   if ( TLISALIST( p->get_type() ) )
   {
      telldata::TtList* wvar = static_cast<telldata::TtList*>(p);
      if (wvar->mlist().empty())
      {
         ost << "empty list";
      }
      else
      {
         ost << " list members: { ";
         for (unsigned i = 0; i < wvar->mlist().size(); i++)
         {
            if (i > 0)  ost << " , ";
            echoWrapper(wvar->mlist()[i], ost);
         }
         ost << " } ";
      }

   }
   else switch (p->get_type())
   {
      case telldata::tn_NULL      :assert(false);/*TODO*/break;
      case telldata::tn_void      :assert(false);/*TODO*/break;
      case telldata::tn_int:
      {
         ost << static_cast<telldata::TtInt*>(p)->value();
         break;
      }
      case telldata::tn_real:
      {
         std::scientific(ost);
         ost << static_cast<telldata::TtReal*>(p)->value();
         break;
      }
      case telldata::tn_bool:
      {
         if (static_cast<telldata::TtBool*>(p)->value())
            ost << "true";
         else
            ost << "false";
         break;
      }
      case telldata::tn_string:
      {
         ost << "\"" << static_cast<telldata::TtString*>(p)->value() << "\"";
         break;
      }
      case telldata::tn_layout:
      {
         telldata::TtLayout* wvar = static_cast<telldata::TtLayout*>(p);
         if (NULL == wvar->data())
            ost << "< !EMPTY! >";
         else
         {
            if ( wvar->layer().editable() )
               ost << "layer " << wvar->layer() << " :";
            wvar->data()->info(ost, PROPC->DBscale());
         }
         if (wvar->selp() && (wvar->selp()->size() > 0)) ost << " - partially selected";
         break;
      }
      case telldata::tn_auxilary:
      {
         telldata::TtAuxdata* wvar = static_cast<telldata::TtAuxdata*>(p);
         if (NULL == wvar->data())
            ost << "< !EMPTY! >";
         else
         {
            if ( wvar->layer().editable() )
               ost << "layer " << wvar->layer() << " :";
            wvar->data()->info(ost, PROPC->DBscale());
         }
         break;
      }
      case telldata::tn_anyfref:
      {
         telldata::TtCallBack* wvar = static_cast<telldata::TtCallBack*>(p);
         if ( (NULL == wvar->fcbBody()) || (wvar->fcbBody()->declaration()) )
            ost << "NULL";
         else
            ost << "pointing to <TODO> function";
         break;
      }
      case telldata::tn_composite:
      {
         telldata::TtUserStruct* wvar = static_cast<telldata::TtUserStruct*>(p);
         ost << "struct members:\n";
         for (telldata::recfieldsNAME::const_iterator CI = wvar->fieldList().begin(); CI != wvar->fieldList().end(); CI++)
         {
            ost << CI->first << ": ";
            echoWrapper(CI->second, ost);
            ost << "\n";
         }
         break;
      }
      case telldata::tn_pnt:
      {
         telldata::TtPnt* wvar = static_cast<telldata::TtPnt*>(p);
         ost << "{X = "  << wvar->x()
             << ", Y = " << wvar->y()
             << "}";
         break;
      }
      case telldata::tn_box:
      {
         telldata::TtWnd* wvar = static_cast<telldata::TtWnd*>(p);
         ost << "P1: X = " << wvar->p1().x() << ": Y = " << wvar->p1().y() << " ; " <<
                "P2: X = " << wvar->p2().x() << ": Y = " << wvar->p2().y() ;
         break;
      }
      case telldata::tn_bnd:
      {
         telldata::TtBnd* wvar = static_cast<telldata::TtBnd*>(p);
         ost << "P: X = " << wvar->p().x() << ": Y = " << wvar->p().y() << " ; " <<
                "rot = "  << wvar->rot().value() << ": flipX " << (wvar->flx().value() ? "true" : "false") << " ; "  <<
                "scale = " << wvar->sc().value();
         break;
      }
      case telldata::tn_laymap:
      {
         telldata::TtLMap* wvar = static_cast<telldata::TtLMap*>(p);
         ost << "layer = "
             << wvar->layer().value()
             << " : value = \""
             << wvar->value().value() << "\"";
         break;
      }
      case telldata::tn_hshstr:
      {
         telldata::TtHshStr* wvar = static_cast<telldata::TtHshStr*>(p);
         ost << "key = "  << wvar->key().value() << " : value = \"" << wvar->value().value() << "\"";
         break;
      }
      case telldata::tn_layer:
      {
         telldata::TtLayer* wvar = static_cast<telldata::TtLayer*>(p);
         ost << "{num = "  << wvar->num()
             << ", typ = " << wvar->typ()
             << "}";
         ost << ost.str();
         break;
      }
      case telldata::tn_usertypes :assert(false);/*TODO*/break;
      default                     :assert(false);/*TODO*/break;
   }
}
