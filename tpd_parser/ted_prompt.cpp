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
//  Creation date: Sun Mar 17 2002
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <wx/regex.h>
#include "ted_prompt.h"
#include "tell_yacc.h"

//-----------------------------------------------------------------------------
// Some external definitions
//-----------------------------------------------------------------------------
extern void* tell_scan_string(const char *str);
extern void my_delete_yy_buffer( void* b );
extern int tellparse(); // Calls the bison generated parser
extern YYLTYPE telllloc; // parser current location - global variable, defined in bison
wxMutex              Mutex;


extern const wxEventType    wxEVT_LOG_ERRMESSAGE;
console::ted_cmd*           Console = NULL;

//==============================================================================
bool console::patternFound(const wxString templ,  wxString str) {
   patternNormalize(str);
   wxRegEx src_tmpl(templ);
   assert(src_tmpl.IsValid());
   return src_tmpl.Matches(str);
}

void console::patternNormalize(wxString& str) {
   wxRegEx regex;
   // replace tabs with spaces
   assert(regex.Compile("\\t"));
   regex.ReplaceAll(&str," ");
   // remove continious spaces
   assert(regex.Compile("[[:space:]]{2,}"));
   regex.ReplaceAll(&str,"");
   //remove leading spaces
   assert(regex.Compile("^[[:space:]]"));
   regex.ReplaceAll(&str,"");
   // remove trailing spaces
   assert(regex.Compile("[[:space:]]$"));
   regex.ReplaceAll(&str,"");
   //remove spaces before brackets and separators
   assert(regex.Compile("([[:space:]])([\\(\\)\\,\\-\\+])"));
   regex.ReplaceAll(&str,"\\2");
   // remove spaces after brackets and separators
   assert(regex.Compile("([\\(\\)\\,\\-\\+])([[:space:]])"));
   regex.ReplaceAll(&str,"\\1");

}

//==============================================================================
console::miniParser::miniParser(parsercmd::operandSTACK *cs,telldata::typeID et) {
   client_stack = cs;  _wait4type = et;
}

bool console::miniParser::getGUInput(wxString expression) {
   exp = expression;
   patternNormalize(exp);
   switch (_wait4type) {
      case         telldata::tn_pnt : return getPoint();
      case         telldata::tn_box : return getBox();
      case TLISTOF(telldata::tn_pnt): return getList();
               default: return false;// unexpected type
   }
}

bool console::miniParser::getPoint() {
   wxRegEx src_tmpl(point_tmpl);
   assert(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // get the coordinates
   assert(src_tmpl.Compile(real_tmpl));
   src_tmpl.Matches(exp);
   // first one...
   wxString p1s = src_tmpl.GetMatch(exp);
   // The expression is greedy and if you try to get simply the second match,
   // then, you might get the fractional part of the first coord as second 
   // coordinate, so remove and match again - not quie elegant, but works
   src_tmpl.ReplaceFirst(&exp,"");
   src_tmpl.Matches(exp);
   wxString p2s = src_tmpl.GetMatch(exp);
   double p1,p2;
   p1s.ToDouble(&p1);p2s.ToDouble(&p2);
   // convert the coordinates to ttpoint ...
   telldata::ttpnt* pp = new telldata::ttpnt(p1,p2);
   // and push it into the operand stack
   client_stack->push(pp);
   return true;
}

bool console::miniParser::getBox() {
   wxRegEx src_tmpl(box_tmpl);
   assert(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // remove the outside brackets
   assert(src_tmpl.Compile("^\\({2}"));
   src_tmpl.ReplaceAll(&exp,"(");    
   assert(src_tmpl.Compile("\\){2}$"));
   src_tmpl.ReplaceAll(&exp,")");    
   // now we are going to extract the points
   assert(src_tmpl.Compile(point_tmpl));
   telldata::ttpnt pp[2];
   for (int i = 0; i < 2; i++) {
      if (!src_tmpl.Matches(exp)) return false;
      wxString ps = src_tmpl.GetMatch(exp);
      src_tmpl.ReplaceFirst(&exp,"");
      
      wxRegEx crd_tmpl(real_tmpl);
      assert(crd_tmpl.IsValid());
      crd_tmpl.Matches(ps);
      wxString p1s = crd_tmpl.GetMatch(ps);
      crd_tmpl.ReplaceFirst(&ps,"");
      crd_tmpl.Matches(ps);
      wxString p2s = crd_tmpl.GetMatch(ps);
      double p1,p2;
      p1s.ToDouble(&p1);p2s.ToDouble(&p2);
      // convert the coordinates to ttpoint ...
      pp[i] = telldata::ttpnt(p1,p2);
   }
   client_stack->push(new telldata::ttwnd(pp[0],pp[1]));
   return true;
}

bool console::miniParser::getList() {
   wxRegEx src_tmpl(pointlist_tmpl);
   assert(src_tmpl.IsValid());
   // search the entire pattern
   if (!src_tmpl.Matches(exp)) return false;
   // remove the outside brackets
   assert(src_tmpl.Compile("^\\{"));
   src_tmpl.ReplaceAll(&exp,"");    
   assert(src_tmpl.Compile("\\}$"));
   src_tmpl.ReplaceAll(&exp,"");    
   // now we are going to extract the points
   assert(src_tmpl.Compile(point_tmpl));
   telldata::ttlist *pl = new telldata::ttlist(telldata::tn_pnt);
   telldata::ttpnt* pp = NULL;
   while (src_tmpl.Matches(exp)) {
      wxString ps = src_tmpl.GetMatch(exp);
      src_tmpl.ReplaceFirst(&exp,"");
      wxRegEx crd_tmpl(real_tmpl);
      assert(crd_tmpl.IsValid());
      crd_tmpl.Matches(ps);
      wxString p1s = crd_tmpl.GetMatch(ps);
      crd_tmpl.ReplaceFirst(&ps,"");
      crd_tmpl.Matches(ps);
      wxString p2s = crd_tmpl.GetMatch(ps);
      double p1,p2;
      p1s.ToDouble(&p1);p2s.ToDouble(&p2);
      pp = new telldata::ttpnt(p1,p2);
      // add it to the point list
      pl->add(pp);
   }
   // and push it into the operand stack
   client_stack->push(pl);
   return true;
}

//==============================================================================
void* console::parse_thread::Entry() {
//   wxLogMessage(_T("Mouse is %s (%ld, %ld)"), where.c_str(), x, y);
//   wxLogMessage(_T("Mutex try to lock..."));
   while (wxMUTEX_NO_ERROR != Mutex.TryLock());
//   wxLogMessage(_T("Mutex locked!"));
   telllloc.first_column = telllloc.first_line = 1;
   telllloc.last_column  = telllloc.last_line  = 1;
   telllloc.filename = NULL;
   void* b = tell_scan_string( command.c_str() );
   tellparse();
   my_delete_yy_buffer( b );
   Mutex.Unlock();
//   wxLogMessage(_T("Mutex unlocked"));
};

//==============================================================================
// The ted_cmd event table
BEGIN_EVENT_TABLE( console::ted_cmd, wxTextCtrl )
   EVT_TEXT_ENTER(wxID_ANY, ted_cmd::getCommand)
   EVT_KEY_UP(ted_cmd::OnKeyUP)
END_EVENT_TABLE()

//==============================================================================
console::ted_cmd::ted_cmd(wxWindow *parent) :
   wxTextCtrl( parent, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxNO_BORDER),
   puc(NULL), _numpoints(0) {
   
   threadWaits4 = new wxCondition(Mutex);
   assert(threadWaits4->IsOk());
   _mouseIN_OK = true;
   Console = this;
   _history_position = _cmd_history.begin();
};

void console::ted_cmd::getCommand(wxCommandEvent& WXUNUSED(event)) {
   if (puc)  getGUInput(); // run the local GUInput parser
   else {
      wxString command = GetValue();
      tell_log(MT_COMMAND, command.c_str());
      _cmd_history.push_back(command.c_str());
      _history_position = _cmd_history.end();
      Clear();
      parse_thread *pthrd = new parse_thread(command);
      pthrd->Create();
      pthrd->Run();
   }   
}

void console::ted_cmd::getCommandA() {
   if (puc)  getGUInput(); // run the local GUInput parser
   else {
      wxString command = GetValue();
      tell_log(MT_COMMAND, command.c_str());
      _cmd_history.push_back(command.c_str());
      _history_position = _cmd_history.end();
      Clear();
      parse_thread *pthrd = new parse_thread(command);
      pthrd->Create();
      pthrd->Run();
   }   
}

void console::ted_cmd::OnKeyUP(wxKeyEvent& event) {

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
   if (_cmd_history.end() == _history_position) SetValue("");
   else SetValue(*_history_position);
}

void console::ted_cmd::parseCommand(wxString cmd) {
   SetValue(cmd);
   getCommandA();
}
   

void console::ted_cmd::waitGUInput(parsercmd::operandSTACK *clst,telldata::typeID ttype) {
   puc = new miniParser(clst, ttype);_numpoints = 0;
   _mouseIN_OK = true;
   _guinput.Clear();
   tell_log(MT_GUIPROMPT);
   Connect(-1, wxEVT_COMMAND_ENTER,
           (wxObjectEventFunction) (wxEventFunction)
           (wxCommandEventFunction)&ted_cmd::OnGUInput);
}

void console::ted_cmd::getGUInput(bool from_keyboard) {
   wxString command;
   if (from_keyboard) { // input is from keyboard
      command = GetValue();
      tell_log(MT_GUIINPUT, command.c_str());
      tell_log(MT_EOL);
      Clear();
   }   
   else   command = _guinput;
   if (puc->getGUInput(command)) { //parse the data from the prompt
      //if the proper pattern was found
//      setColor(darkGreen);
      Disconnect(-1, wxEVT_COMMAND_ENTER);
      delete puc; puc = NULL;
      _mouseIN_OK = true;
      // wake-up the thread expecting this data
      threadWaits4->Signal();
   }
   else {
      tell_log(MT_GUIPROMPT);
      // SGREM !! Error message here!!
//      setColor(red);
//      append(gui_prompt);
//      if (telldata::tn_pnt_list == puc->wait4type()) {
//         int para, index;
//         getCursorPosition(&para,&index);
//         insertAt("{ ",paragraphs()-1,3);
//      }
   }
   _guinput.Clear();
}

void console::ted_cmd::OnGUInput(wxCommandEvent& evt) {
   if (-1 == evt.GetInt()) {
      Disconnect(-1, wxEVT_COMMAND_ENTER);
      delete puc; puc = NULL;
      _mouseIN_OK = false;
      // wake-up the thread expecting this data
      threadWaits4->Signal();
   }   
   else if (0 == evt.GetInt()) {
      telldata::ttpnt* p = static_cast<telldata::ttpnt*>(evt.GetClientData());
      mouseLB(*p);
   }
   else if (2 == evt.GetInt())
      mouseRB();
}

void console::ted_cmd::mouseLB(const telldata::ttpnt& p) {
   wxString ost1, ost2;
   // prepare the point string for the input log window
   ost1 << "( "<< p.x() << " , " << p.y() << " )";
   // take care about the entry brackets ...
   if (_numpoints == 0) 
      switch (puc->wait4type()) {
         case TLISTOF(telldata::tn_pnt): ost2 << "{ " << ost1; break;
         case         telldata::tn_box : ost2 << "( " << ost1; break;
         default                       : ost2 << ost1;
      }
   // ... and separators between the points
   else ost2 << " , " << ost1;
   // print the current point in the log window
   tell_log(MT_GUIINPUT, ost2.c_str());
   // and update the current input string
   _guinput << ost2;
   // actualize the number of points entered
   _numpoints++;
   // If there is nothing else to wait ... call end of mouse input
   if ( (_numpoints == 1) && (telldata::tn_pnt == puc->wait4type())
     || (_numpoints == 2) && (telldata::tn_box == puc->wait4type())) mouseRB();
}
      
void console::ted_cmd::mouseRB() {
   // End of input is not accepted if ... 
   if ( (_numpoints == 0) 
      ||((_numpoints == 1) && (telldata::tn_pnt != puc->wait4type()))) return;
   // put the proper closing bracket
   wxString close;
   switch (puc->wait4type()) {
      case TLISTOF(telldata::tn_pnt): close = " }"; break;
      case         telldata::tn_box : close = " )"; break;
      default         : close = ""  ;
   }
   // print it
   tell_log(MT_GUIINPUT, close.c_str());
   tell_log(MT_EOL);
   // and update the current input string
   _guinput << close;
   // and go parse it
   getGUInput(false); // from GUI
   _guinput.Clear();
}

console::ted_cmd::~ted_cmd() {
   delete threadWaits4; _cmd_history.clear();
}
