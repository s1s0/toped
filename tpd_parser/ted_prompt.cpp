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
//        Created: Sun Mar 17 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Toped prompt
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <string>
#include <wx/string.h>
#include <wx/regex.h>
#include <wx/filename.h>
#include "tuidefs.h"
#include "ted_prompt.h"
#include "tell_yacc.h"

//-----------------------------------------------------------------------------
// Some external definitions
//-----------------------------------------------------------------------------
//??? I need help in this place
#ifdef WIN32
typedef unsigned int yy_size_t;
struct yy_buffer_state
{
   FILE*       yy_input_file;
   char*       yy_ch_buf;     // input buffer
   char*       yy_buf_pos;    // current position in input buffer
   yy_size_t   yy_buf_size;
   int         yy_n_chars;
   int         yy_is_our_buffer;
   int         yy_is_interactive;
   int         yy_at_bol;
   int         yy_fill_buffer;
   int         yy_buffer_status;
};

extern yy_buffer_state* tell_scan_string(const char *str);

#else
extern void* tell_scan_string(const char *str);
#endif

extern void delete_tell_lex_buffer( void* b );
extern int tellparse(); // Calls the bison generated parser
extern YYLTYPE telllloc; // parser current location - global variable, defined in bison

//extern const wxEventType    wxEVT_LOG_ERRMESSAGE;
console::ted_cmd*           Console = NULL;
extern const wxEventType    wxEVT_CONSOLE_PARSE;
extern const wxEventType    wxEVT_CANVAS_ZOOM;
extern const wxEventType    wxEVT_EXECEXTDONE;


//==============================================================================
bool console::patternFound(const wxString templ,  wxString str) {
   patternNormalize(str);
   wxRegEx src_tmpl(templ);
   VERIFY(src_tmpl.IsValid());
   return src_tmpl.Matches(str);
}

void console::patternNormalize(wxString& str) {
   wxRegEx regex;
   // replace tabs with spaces
   VERIFY(regex.Compile(wxT("\t")));
   regex.ReplaceAll(&str,wxT(" "));
   // remove continious spaces
   VERIFY(regex.Compile(wxT("[[:space:]]{2,}")));
   regex.ReplaceAll(&str,wxT(""));
   //remove leading spaces
   VERIFY(regex.Compile(wxT("^[[:space:]]")));
   regex.ReplaceAll(&str,wxT(""));
   // remove trailing spaces
   VERIFY(regex.Compile(wxT("[[:space:]]$")));
   regex.ReplaceAll(&str,wxT(""));
   //remove spaces before brackets and separators
   VERIFY(regex.Compile(wxT("([[:space:]])([\\{\\}\\,\\-\\+])")));
   regex.ReplaceAll(&str,wxT("\\2"));
   // remove spaces after brackets and separators
   VERIFY(regex.Compile(wxT("([\\{\\}\\,\\-\\+])([[:space:]])")));
   regex.ReplaceAll(&str,wxT("\\1"));

}

//==============================================================================
console::miniParser::miniParser(telldata::operandSTACK *cs,telldata::typeID et) {
   client_stack = cs;  _wait4type = et;
}

bool console::miniParser::getGUInput(wxString expression) {
   exp = expression;
   patternNormalize(exp);
   switch (_wait4type) {
      case         telldata::tn_pnt : return getPoint();
      case         telldata::tn_box : return getBox();
      case         telldata::tn_bnd : return getBind();
      case TLISTOF(telldata::tn_pnt): return getList();
               default: return false;// unexpected type
   }
}

bool console::miniParser::getPoint() {
   wxRegEx src_tmpl(point_tmpl);
   VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // get the coordinates
   VERIFY(src_tmpl.Compile(real_tmpl));
   src_tmpl.Matches(exp);
   // first one...
   wxString p1s = src_tmpl.GetMatch(exp);
   // The expression is greedy and if you try to get simply the second match,
   // then, you might get the fractional part of the first coord as second
   // coordinate, so remove and match again - not quie elegant, but works
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   src_tmpl.Matches(exp);
   wxString p2s = src_tmpl.GetMatch(exp);
   double p1,p2;
   p1s.ToDouble(&p1);p2s.ToDouble(&p2);
   // convert the coordinates to ttpoint ...
   telldata::ttpnt* pp = DEBUG_NEW telldata::ttpnt(p1,p2);
   // and push it into the operand stack
   client_stack->push(pp);
   return true;
}

bool console::miniParser::getBox()
{
   wxRegEx src_tmpl(box_tmpl);
   VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // remove the outside brackets
   VERIFY(src_tmpl.Compile(wxT("^\\{{2}")));
   src_tmpl.ReplaceAll(&exp,wxT("{"));
   VERIFY(src_tmpl.Compile(wxT("\\}{2}$")));
   src_tmpl.ReplaceAll(&exp,wxT("}"));
   // now we are going to extract the points
   VERIFY(src_tmpl.Compile(point_tmpl));
   telldata::ttpnt pp[2];
   for (int i = 0; i < 2; i++) {
      if (!src_tmpl.Matches(exp)) return false;
      wxString ps = src_tmpl.GetMatch(exp);
      src_tmpl.ReplaceFirst(&exp,wxT(""));

      wxRegEx crd_tmpl(real_tmpl);
      VERIFY(crd_tmpl.IsValid());
      crd_tmpl.Matches(ps);
      wxString p1s = crd_tmpl.GetMatch(ps);
      crd_tmpl.ReplaceFirst(&ps,wxT(""));
      crd_tmpl.Matches(ps);
      wxString p2s = crd_tmpl.GetMatch(ps);
      double p1,p2;
      p1s.ToDouble(&p1);p2s.ToDouble(&p2);
      // convert the coordinates to ttpoint ...
      pp[i] = telldata::ttpnt(p1,p2);
   }
   client_stack->push(DEBUG_NEW telldata::ttwnd(pp[0],pp[1]));
   return true;
}

bool console::miniParser::getBind()
{
   wxRegEx src_tmpl(bind_tmpl);
   VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // remove the outside brackets
   VERIFY(src_tmpl.Compile(wxT("^\\{{2}")));
   src_tmpl.ReplaceAll(&exp,wxT("{"));
   VERIFY(src_tmpl.Compile(wxT("\\}$")));
   src_tmpl.ReplaceAll(&exp,wxT(""));
   // let's extract the point first ...
   VERIFY(src_tmpl.Compile(point_tmpl));
   telldata::ttpnt pp;
   if (!src_tmpl.Matches(exp)) return false;
   wxString ps = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));

   wxRegEx crd_tmpl(real_tmpl);
   VERIFY(crd_tmpl.IsValid());
   crd_tmpl.Matches(ps);
   wxString p1s = crd_tmpl.GetMatch(ps);
   crd_tmpl.ReplaceFirst(&ps,wxT(""));
   crd_tmpl.Matches(ps);
   wxString p2s = crd_tmpl.GetMatch(ps);
   double p1,p2;
   p1s.ToDouble(&p1);p2s.ToDouble(&p2);
   // convert the coordinates to ttpoint ...
   pp = telldata::ttpnt(p1,p2);
   // ... now the rotation ...
   VERIFY(src_tmpl.Compile(real_tmpl));
   telldata::ttreal rot;
   if (!src_tmpl.Matches(exp)) return false;
   ps = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   ps.ToDouble(&p1);
   rot = telldata::ttreal(p1);

   // ... the flip ...
   VERIFY(src_tmpl.Compile(bool_tmpl));
   telldata::ttbool flip;
   if (!src_tmpl.Matches(exp)) return false;
   ps = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   if (wxT("true") == ps)
      flip = telldata::ttbool(true);
   else
      flip = telldata::ttbool(false);

   // ... and finally - the scale ...
   VERIFY(src_tmpl.Compile(real_tmpl));
   telldata::ttreal scl;
   if (!src_tmpl.Matches(exp)) return false;
   ps = src_tmpl.GetMatch(exp);
   src_tmpl.ReplaceFirst(&exp,wxT(""));
   ps.ToDouble(&p1);
   scl = telldata::ttreal(p1);

   client_stack->push(DEBUG_NEW telldata::ttbnd(pp,rot,flip,scl));
   return true;
}

bool console::miniParser::getList() {
   wxRegEx src_tmpl(pointlist_tmpl);
   VERIFY(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // remove the outside brackets
   VERIFY(src_tmpl.Compile(wxT("^\\{")));
   src_tmpl.ReplaceAll(&exp,wxT(""));
   VERIFY(src_tmpl.Compile(wxT("\\}$")));
   src_tmpl.ReplaceAll(&exp,wxT(""));
   // now we are going to extract the points
   VERIFY(src_tmpl.Compile(point_tmpl));
   telldata::ttlist *pl = DEBUG_NEW telldata::ttlist(telldata::tn_pnt);
   telldata::ttpnt* pp = NULL;
   while (src_tmpl.Matches(exp)) {
      wxString ps = src_tmpl.GetMatch(exp);
      src_tmpl.ReplaceFirst(&exp,wxT(""));
      wxRegEx crd_tmpl(real_tmpl);
      VERIFY(crd_tmpl.IsValid());
      crd_tmpl.Matches(ps);
      wxString p1s = crd_tmpl.GetMatch(ps);
      crd_tmpl.ReplaceFirst(&ps,wxT(""));
      crd_tmpl.Matches(ps);
      wxString p2s = crd_tmpl.GetMatch(ps);
      double p1,p2;
      p1s.ToDouble(&p1);p2s.ToDouble(&p2);
      pp = DEBUG_NEW telldata::ttpnt(p1,p2);
      // add it to the point list
      pl->add(pp);
   }
   // and push it into the operand stack
   client_stack->push(pl);
   return true;
}

//==============================================================================
console::parse_thread::parse_thread(wxWindow* canvas_wnd, wxThreadKind kind):
      wxThread       ( kind                         ),
      _canvas_wnd    ( canvas_wnd                   ),
      _mutexCondition( DEBUG_NEW wxCondition(_mutex))
{}

void* console::parse_thread::Entry()
{
   if (wxMUTEX_DEAD_LOCK == _mutex.Lock())
   {
      tell_log(console::MT_ERROR, "TELL Mutex found deadlocked on Entry!");
      return NULL;
   }
   do {
      _mutexCondition->Wait();
      if (TestDestroy())
         break; // Thread Exit point

      telllloc.first_column = telllloc.first_line = 1;
      telllloc.last_column  = telllloc.last_line  = 1;
      telllloc.filename = NULL;
      parsercmd::cmdSTDFUNC::setThreadExecution(true);
      TpdPost::toped_status(TSTS_THREADON, _command);

   #ifdef PARSER_PROFILING
         HiResTimer profTimer;
   #endif
      try {
         void* b = tell_scan_string(_command.mb_str(wxConvUTF8) );
         tellparse();
         delete_tell_lex_buffer( b );
      }
      catch (EXPTNtell_parser&)
      {
         // Not sure we can make something here.flex has thrown an exception
         // but it could be the file system or dynamic memory
         //@TODO check for available dynamic memory
      }
   #ifdef PARSER_PROFILING
         profTimer.report("Time elapsed by the last parser run: ");
   #endif
      if (Console->exitRequested())
      {
         Console->setExitRequest(false);
         TpdPost::quitApp(true);
      }
      else if (Console->canvas_invalid())
      {
         wxCommandEvent eventZOOM(wxEVT_CANVAS_ZOOM);
         eventZOOM.SetInt(tui::ZOOM_REFRESH);
         wxPostEvent(_canvas_wnd, eventZOOM);
         Console->set_canvas_invalid(false);
      }
      TpdPost::toped_status(TSTS_THREADOFF);
      parsercmd::cmdSTDFUNC::setThreadExecution(false);
   } while(true);
   _mutex.Unlock();
   return NULL;
};

void console::parse_thread::OnExit()
{
   delete _mutexCondition;
}

//==============================================================================
// The ted_cmd event table
BEGIN_EVENT_TABLE( console::ted_cmd, wxTextCtrl )
   EVT_TECUSTOM_COMMAND(wxEVT_CONSOLE_PARSE, wxID_ANY, ted_cmd::onParseCommand)
   EVT_TEXT_ENTER(wxID_ANY, ted_cmd::onGetCommand)
   EVT_KEY_UP(ted_cmd::onKeyUP)
END_EVENT_TABLE()

//==============================================================================
console::ted_cmd::ted_cmd(wxWindow *parent, wxWindow *canvas) :
      wxTextCtrl( parent, tui::ID_CMD_LINE, wxT(""), wxDefaultPosition, wxDefaultSize,
                  wxTE_PROCESS_ENTER | wxNO_BORDER), _puc(NULL), _numpoints(0)
{
   _canvas = canvas;
   _exitRequested = false;
   _execExternal  = false;
   _mouseIN_OK = true;
   Console = this;
   _history_position = _cmd_history.begin();
   _canvas_invalid = false;
   spawnTellThread();
};

// Note! getCommand and onGetCommand should be overloaded methods, however
// wxEvent macroses (or maybe the whole system) are not happy if there is
// an overloaded function of the one listed in the EVT_* macros. This is the
// only reason for this functions to have different names. They do (almost)
// the same thing

/*! Called when the command line is entered. Updates the command history,
   log window and spawns the tell parser in a new thread                   */
void console::ted_cmd::onGetCommand(wxCommandEvent& WXUNUSED(event))
{
   if (_puc)  getGUInput(); // run the local GUInput parser
   else if (_execExternal)
   {
      TpdPost::execPipe(GetValue());
      Clear();
   }
   else {
      wxString command = GetValue();
      tell_log(MT_COMMAND, command);
      _cmd_history.push_back(std::string(command.mb_str(wxConvUTF8)));
      _history_position = _cmd_history.end();
      Clear();

      runTellCommand(command);
   }
}

void console::ted_cmd::getCommand(bool thread)
{
   if (_puc)  getGUInput(); // run the local GUInput parser
   else
   {
      wxString command = GetValue();
      tell_log(MT_COMMAND, command);
      _cmd_history.push_back(std::string(command.mb_str(wxConvUTF8)));
      _history_position = _cmd_history.end();
      Clear();
      if (!thread)
      { // executing the parser without thread
         // essentially the same code as in parse_thread::Entry, but
         // without the mutexes
         telllloc.first_column = telllloc.first_line = 1;
         telllloc.last_column  = telllloc.last_line  = 1;
         telllloc.filename = NULL;
         try {
            void* b = tell_scan_string( command.mb_str(wxConvUTF8) );
            tellparse();
            delete_tell_lex_buffer( b );
         }
         catch (EXPTNtell_parser&)
         {
            // Not sure we can make something here.flex has thrown an exception
            // but it could be the file system or dynamic memory
            //@TODO check for available dynamic memory
            // see the same comment @line 307
         }
         // Make sure that exit command didn't get trough
         assert(!exitRequested());
      }
      else
         runTellCommand(command);
   }
}


void console::ted_cmd::spawnTellThread(/*wxString command*/)
{
   // executing the parser in a separate thread
   //wxTHREAD_JOINABLE, wxTHREAD_DETACHED
   // A Memo on wxMutexes
   // A mutex can be owned by one thread at a time. That's the whole point
   // of them. The thread that owns the mutex is the one which called
   // wxMutex.Lock(). When a mutex is unlocked, it is orphan, i.e. it is
   // not owned by anybody. Now the exciting part.
   // - What if wxMutex.Lock() is called twice by the same thread?
   //   This seems to be called "recursive mutexes" and the bottom line
   //   there is - don't try it. (not portable, who knows how it works)
   //   In other words this means that if you lock the mutex in thread X
   //   you should better unlock it in the same thread, otherwise the
   //   hell will break loose.
   // - About thread synchronization. wxCondition always have a mutex
   //   associated with it. A call to wxCondition::Signal() method will
   //   wake-up the thread which currently owns that mutex. Once again -
   //   if you are intending to stop the thread X and wait for an event
   //   from thread Y then you must lock the mutex from thread X
   //   beforehand. Otherwise the thread X is as good as dead.
   //
   // And the most exciting part - wxCondition::Broadcast()
   // It says that it will wake-up all the threads waiting for this
   // condition. How on earth you can associate more than one thread to
   // a single mutex? This part I don't understand.
   //

   _tellThread = DEBUG_NEW parse_thread(/*command, */_canvas);
   _threadWaits4 = _tellThread->mutexCondition();
   VERIFY(_threadWaits4->IsOk());

   wxThreadError result = _tellThread->Create();
   if (wxTHREAD_NO_ERROR == result)
      _tellThread->Run();
   else
   {
      tell_log( MT_ERROR, "Can't execute the command in a separate thread");
      //delete(pthrd);
   }
}

void console::ted_cmd::runTellCommand(const wxString& cmd)
{
   wxMutexError result = _tellThread->_mutex.TryLock();
   if (wxMUTEX_BUSY == result)
   {
      // Don't pile-up commands(threads). If previous command
      // didn't finish leave the thread immediately.
      // This check can't be done in the main thread. See the comment
      // in ted_cmd::spawnParseThread below
      tell_log( MT_WARNING, "Busy. Command above skipped");
   }
   else
   {
      _tellThread->setCommand(cmd);
      _tellThread->_mutex.Unlock();
      _threadWaits4->Signal();
   }
}

void console::ted_cmd::stopParserThread()
{
   wxMutexError result;
   do
   {
      result = _tellThread->_mutex.TryLock();
   } while (wxMUTEX_BUSY == result);
   _tellThread->setCommand(wxT(""));
   _tellThread->_mutex.Unlock();
	_threadWaits4->Signal();
   _tellThread->Delete();

}

// Note! parseCommand and onParseCommand should be overloaded methods, however
// wxEvent macroses (or maybe the whole system) are not happy if there is
// an overloaded function of the one listed in the EVT_* macros. This is the
// only reason for this functions to have different names. They do the same
// thing
// onParseCommand is called from wxEVT_CONSOLE_PARSE event while getCommand()
// is called directly
void console::ted_cmd::onParseCommand(wxCommandEvent& event)
{
   if (NULL != _puc) return; // don't accept commands during shape input sessions
   SetValue(event.GetString());
   getCommand(true);
}

void console::ted_cmd::parseCommand(wxString cmd, bool thread)
{
   if (NULL != _puc) return; // don't accept commands during shape input sessions
   SetValue(cmd);
   getCommand(thread);
}

void console::ted_cmd::onKeyUP(wxKeyEvent& event) {

   if ((WXK_UP != event.GetKeyCode()) &&  (WXK_DOWN != event.GetKeyCode())) {
      event.Skip();return;
   }
   if (WXK_DOWN != event.GetKeyCode())
      if (_cmd_history.begin() == _history_position)
         _history_position = _cmd_history.end();
      else _history_position--;
   else
      if (_cmd_history.end() == _history_position)
         _history_position = _cmd_history.begin();
      else _history_position++;
      if (_cmd_history.end() == _history_position) SetValue(wxT(""));
   else
   {
      SetValue(wxString(_history_position->c_str(), wxConvUTF8));
   }
}

void console::ted_cmd::waitGUInput(telldata::operandSTACK *clst, console::ACTIVE_OP input_type, const CTM& trans)
{
   telldata::typeID ttype;
   switch (input_type)
   {
      case console::op_line   :
      case console::op_dbox   :
      case console::op_copy   :
      case console::op_move   : ttype = telldata::tn_box; break;
      case console::op_rotate :
      case console::op_flipY  :
      case console::op_flipX  :
      case console::op_point  : ttype = telldata::tn_pnt; break;
      case console::op_cbind  :
      case console::op_abind  :
      case console::op_tbind  : ttype = telldata::tn_bnd; break;
      default:ttype = TLISTOF(telldata::tn_pnt); break;
   }
   _puc = DEBUG_NEW miniParser(clst, ttype);
   _numpoints = 0;
   _initrans = _translation = trans;
   _mouseIN_OK = true;
   _guinput.Clear();
   tell_log(MT_GUIPROMPT);
   Connect(-1, wxEVT_COMMAND_ENTER,
           (wxObjectEventFunction) (wxEventFunction)
           (wxCommandEventFunction)&ted_cmd::onGUInput);

   TpdPost::toped_status(TSTS_THREADWAIT);
}

void console::ted_cmd::waitExternal(wxString cmdExt)
{
   Connect(-1, wxEVT_EXECEXTDONE,
           (wxObjectEventFunction) (wxEventFunction)
           (wxCommandEventFunction)&ted_cmd::onExternalDone);
   _execExternal = true;
   TpdPost::toped_status(TSTS_THREADWAIT);
   TpdPost::execExt(cmdExt);
}

void console::ted_cmd::getGUInput(bool from_keyboard) {
   wxString command;
   if (from_keyboard) { // input is from keyboard
      command = GetValue();
      tell_log(MT_GUIINPUT, command);
      tell_log(MT_EOL);
      Clear();
   }
   else   command = _guinput;
   //parse the data from the prompt
   if (_puc->getGUInput(command)) {
      //if the proper pattern was found
      Disconnect(-1, wxEVT_COMMAND_ENTER);
      delete _puc; _puc = NULL;
      _mouseIN_OK = true;
      // wake-up the thread expecting this data
      _threadWaits4->Signal();
   }
   else {
      tell_log(MT_ERROR, "Bad input data, Try again...");
      tell_log(MT_GUIPROMPT);
   }
   _guinput.Clear();
   _numpoints = 0;
   _translation = _initrans;
}

void console::ted_cmd::onGUInput(wxCommandEvent& evt)
{
   switch (evt.GetInt()) {
      case -4: _translation.FlipY();break;
      case -3: _translation.Rotate(90.0);break;
      case -2: cancelLastPoint();break;
      case -1:   // abort current  mouse input
         Disconnect(-1, wxEVT_COMMAND_ENTER);
         delete _puc; _puc = NULL;
         _mouseIN_OK = false;
         tell_log(MT_WARNING, "input aborted");
         tell_log(MT_EOL);
         // wake-up the thread expecting this data
         _threadWaits4->Signal();
         break;
      case  0:  {// left mouse button
         telldata::ttpnt* p = static_cast<telldata::ttpnt*>(evt.GetClientData());
         mouseLB(*p);
         delete p;
         break;
         }
      case  2: {
         telldata::ttpnt* p = static_cast<telldata::ttpnt*>(evt.GetClientData());
         mouseRB();
         delete p;
         break;
         }
      default: assert(false);
   }
}

void console::ted_cmd::onExternalDone(wxCommandEvent& evt)
{
   Disconnect(-1, wxEVT_EXECEXTDONE);
   _execExternal = false;
   _threadWaits4->Signal();
}

void console::ted_cmd::mouseLB(const telldata::ttpnt& p) {
   wxString ost1, ost2;
   // prepare the point string for the input log window
   ost1 << wxT("{ ")<< p.x() << wxT(" , ") << p.y() << wxT(" }");
   // take care about the entry brackets ...
   if (_numpoints == 0)
      switch (_puc->wait4type()) {
         case TLISTOF(telldata::tn_pnt):
         case         telldata::tn_box : ost2 << wxT("{ ") << ost1; break;
         case         telldata::tn_bnd :
         {
            TP bp;
            real ang;
            real sc;
            bool flipX;
            _translation.Decompose(bp, ang, sc, flipX);
            ost2 << wxT("{ ") << ost1 << wxT(", ")
                              << ang  << wxT(", ")
                              << (flipX ? wxT("true") : wxT("false")) << wxT(", ")
                              << sc   << wxT("}");
                      break;
         }
         default                       : ost2 << ost1;
      }
   // ... and separators between the points
   else ost2 << wxT(" , ") << ost1;
   // print the current point in the log window
   tell_log(MT_GUIINPUT, ost2);
   // and update the current input string
   _guinput << ost2;
   // actualize the number of points entered
   _numpoints++;
   // If there is nothing else to wait ... call end of mouse input
   if (((_numpoints == 1) && ((telldata::tn_pnt == _puc->wait4type()) ||
                              (telldata::tn_bnd == _puc->wait4type())   ))
   ||  ((_numpoints == 2) &&  (telldata::tn_box == _puc->wait4type())   ))
      mouseRB();
}

void console::ted_cmd::mouseRB() {
   // End of input is not accepted if ...
   if ( (_numpoints == 0) || ((_numpoints == 1)                      &&
                              (telldata::tn_pnt != _puc->wait4type()) &&
                              (telldata::tn_bnd != _puc->wait4type())    )
      ) return;
   // put the proper closing bracket
   wxString close;
   switch (_puc->wait4type()) {
      case TLISTOF(telldata::tn_pnt):
      case         telldata::tn_box : close = wxT(" }"); break;
      default         : close = wxT("")  ;
   }
   // print it
   tell_log(MT_GUIINPUT, close);
   tell_log(MT_EOL);
   // and update the current input string
   _guinput << close;
   // and go parse it
   getGUInput(false); // from GUI
   _guinput.Clear();
}

void console::ted_cmd::cancelLastPoint() {
   tell_log(MT_WARNING, "last point canceled");
   int pos = _guinput.Find('{',true);
   _guinput = _guinput.Left(pos-2);
   if (_numpoints > 0) _numpoints--;
   tell_log(MT_GUIPROMPT);
   tell_log(MT_GUIINPUT, _guinput);
}

bool console::ted_cmd::findTellFile(const char* fname, std::string& validName)
{
   // Check the original string first
   wxFileName inclFN(wxString(fname,wxConvUTF8));
   inclFN.Normalize();
   if (inclFN.IsOk() && inclFN.FileExists())
   {
      validName = std::string(inclFN.GetFullPath().mb_str(wxConvFile ));
      return true;
   }
   // See whether we can find the file name among the search paths
   wxString absFileName = _tllIncludePath.FindAbsoluteValidPath((wxString(fname,wxConvUTF8)));
   if (!absFileName.IsEmpty())
   {
      validName = std::string(absFileName.mb_str(wxConvFile ));
      return true;
   }
   validName = fname;
   return false;
}

console::ted_cmd::~ted_cmd()
{
   _cmd_history.clear();
   // Because we're using detachable threads - the thread is supposed to delet itself
   // delete _tellThread
}
