/*===========================================================================
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
//   This file is a part of Toped project (C) 2001-2006 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Fri Nov 08 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TELL parser
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
// type / operation |  +  |  -  |  *  |  /  |  u- |
//------------------+-----+-----+-----+-----+-----+
//   real           |  x  |  x  |  x  |  x  |  x  |
//   int            |  x  |  x  |  x  |  x  |  x  |
//   point          |  x  |  x  |     |     |     |
//   box            |  x  |  x  |     |     |     |
//   string         |     |     |     |     |     |
//   bool           |     |     |     |     |     |
//
//===========================================================================*/

/*Switch on code processing locations for more acurate error messages*/
%locations
%{
#include <sstream>
#include "tellyzer.h"
#include "ted_prompt.h"
#include "../tpd_common/outbox.h"
/* Switch on verbose error reporting messages*/
#define YYERROR_VERBOSE 1
/*Current command block - defined in tellyzer.cpp*/
extern parsercmd::cmdBLOCK*       CMDBlock;
/*Global console object*/
extern console::ted_cmd*           Console;
/*Current tell variable name*/
telldata::tell_var *tellvar = NULL;
telldata::tell_var *tell_lvalue = NULL;
/*Current tell struct */
telldata::tell_type *tellstruct = NULL;
/* used for argument type checking during function call parse */
telldata::argumentQ   *argmap  = NULL;
/*taking care when a function is called from the argument list of another call*/
std::stack<telldata::argumentQ*>  argmapstack;

/* Current function declaration structure */
parsercmd::FuncDeclaration *cfd;
/*number of errors in a function defintion*/
int funcdeferrors = 0;
/*number of poits of the current polygon*/
unsigned listlength = 0;
void tellerror(std::string s, parsercmd::yyltype loc);
void tellerror (std::string s);

%}
/*=============================================================================
Well some remarks may save some time in the future...
 The parser is supposed to work as a command line parser as well as a normal
 file parser. The language has a data types, local/global  variables, blocks,
 functions, cycles etc. stuff. So what's the main idea - nothing is executed
 during the parsing. The parser actually just creates a list of cmdVIRTUAL
 objects by calling their constructors. So if nothing's executed there is no
 way to calculate any result during parsing - so nothing to assign to
 $$ variable. In the same time it's a good idea to control data types during
 the parsing. Assignment statement, function parameters - these are all
 good exapmples. That's why $$ variable is used (where appropriate) to hold 
 the language types (enum telldata::type). Of course not all predicates have
 telldata::type. That might be a source of confusion - so some further comments
 concerning the %union statement below:
    real        - this holds the real numbers during parsing. tknREAL is the
                  only token of this type
    parsestr    - to hold variable/function names during parsing
    pttname     - that's the tell_type we spoke about above.
    pblock      - that is actually the list structure type where all commands
                  are stored for future execution. block is the only predicate
                  of this type
    pfblock     - same as previous, however to avoid generally some casting
                  the {} blocks and function blocks have been separated.
                  funcblock is obviously the predicate of this type
    pfarguments - Holds the function definition arguments. funcarguments is
                  the only predicate of this type
    pargumants  - Structure with function call arguments
Now something about the arrays. Actually there will be lists. Why? The main
 reason is that select functions may (and certainly will) return various
 objects - point, box, polygons, path, text etc together. Arrays by nature
 contain fixed number of homogenous objects and I can't think of a convinient
 language construct for selection that will return such a thing. That's why it
 seems better to introduce a list instead. The list members can be of any tell
 type and they don't need to be the same type.
 The question then becames - do we need a function that will return the type of
 a list member? Seems the answer is - NO!Of course I might be wrong - let's see
 Enough rubish! The constructs are:
 list <variable>         -> definition;
 variable[<index>]       -> indexing;
 word length(<variable>) -> standard function will return the number of members
 assignment will have the normal form and a normal type checks will apply.
 Besides polygons and paths will be represented as a list of points
Ooops! Second thought!
 The thing is that we have two different thigns here:
   One (array or list) is a langage construct
   Another (that thing that select will return) is a layout data base object(s).
 What does it mean...
 If the list are implemented as described above this effectively means that we'll
 loose a grip on the type checking or in words there is no way to check and
 control the types of the list components during the parsing. This is a major
 gap in the concept of a strict type checking in tell. From the other point of
 view the problem with the select still remains...
 Select however have to return a pointers to layout DB objects. POINTERS to the
 OBJECTS that ALREADY EXIST in the DB. We can NOT treat them as any other tell
 types and normal tell operations can not apply to them or at least they will
 need a special attention. Operation like delete will be unique for this
 type of objects. There is nothing like cell for example in the language but
 this is a major DB object...
 Well, it seems that IS the milestone where tell and toped split (or converge
 if you want. It's simple!
 box a = ((10,10,20,20)) -> is a tell construct defining and initializing the
                            tell variable a of type box
 addbox(a)               -> is creating a rectangular layout object in the
                             current cell.
 These two things have nothing in common from the moment when addbox(a) is
 executed. A lot of addbox fuctions can be executed with this variable.
 Select function will provide a way to get a reference to the existing layout
 objects. This references can be used aftewrards to move, copy, delete etc.
 these objects, but these operations will not affect by any means tell variable 'a'.
 Having this cleared-up the concept of lists is now crystal. All lists will be
 homogeneous and their definitions will look like.
  <tell_type> list <variable>
 thus the strict type-cheking is still in place. I still insist on the typename
 list (not array) because the size of these things will not be fixed. The side
 effect here is that polygons and wires now became a list of points. We will
 have list of words (for definition of the fills for example) - we can have
 list of any tell type and the type-check will still work perfectly!

 A new type obviously will be introduced to accomodate the pointers to the
 layout objects described above.This is still to be thought out but it seems it
 will look like
  layout <variable>
 There will not be a tell const available of this type. This will be the type
 returned from addbox, addpoly, addcell etc. functions.
 Select of course will return: 
  layout list select(box<variable>)
 Simple!

=============================================================================*/
%union {
   float                       real;
   bool                        ptypedef;
   int                         integer;
   char                       *parsestr;
    telldata::typeID           pttname;
   parsercmd::argumentLIST    *pfarguments;
    telldata::argumentQ       *plarguments;
    telldata::argumentID      *parguments;
   parsercmd::cmdBLOCK        *pblock;
   parsercmd::cmdFUNC         *pfblock;
   parsercmd::FuncDeclaration *pfdeclaration;
}
%start input
/*---------------------------------------------------------------------------*/
%token                 tknERROR
%token	               tknIF tknELSE tknWHILE tknREPEAT tknUNTIL tknSTRUCTdef
%token                 tknVOIDdef tknREALdef tknBOOLdef tknINTdef tknPOINTdef 
%token                 tknBOXdef tknSTRINGdef tknLAYOUTdef tknLISTdef tknRETURN
%token                 tknTRUE tknFALSE tknLEQ tknGEQ tknEQ tknNEQ tknAND tknOR
%token                 tknSW tknSE tknNE tknNW
%token <parsestr>      tknIDENTIFIER tknFIELD tknSTRING
%token <real>          tknREAL
%token <integer>       tknINT
/* parser types*/
%type <pttname>        primaryexpression unaryexpression
%type <pttname>        multiexpression addexpression expression
%type <pttname>        assignment fieldname funccall
%type <pttname>        lvalue telltype telltypeID variable anonymousvar
%type <pttname>        variabledeclaration andexpression eqexpression relexpression 
%type <pfarguments>    funcarguments
%type <parguments>     structure argument
%type <plarguments>    nearguments arguments
%type <pblock>         block
%type <pfblock>        funcblock
%type <ptypedef>       fielddeclaration typedefstruct
%type <pfdeclaration>  funcdeclaration
/*=============================================================================*/
%%
input:
     entrance                              {}
   | input  entrance                       {}
;   

entrance:
     statement ';'                         {
      if (!yynerrs)  CMDBlock->execute();
      else           CMDBlock = CMDBlock->cleaner();
   }
   | funcdefinition                        {}
   | funcdeclaration ';'                   {
//         CMDBlock->addUSERFUNCDECL(std::string($2),$1,arglist);
      delete($1); cfd = NULL;
   }
   | tknERROR                              {tellerror("Unexpected symbol", @1);}
   | error                                 {CMDBlock = CMDBlock->cleaner();/*yynerrs = 0;*/}
;

funcdeclaration:
     telltypeID tknIDENTIFIER '('          {
      cfd = new parsercmd::FuncDeclaration($2, $1) ;
   }
     funcarguments ')'                     {
      $$ = cfd;
      delete [] $2;
   }
;

funcdefinition:
     funcdeclaration funcblock             {
      if (!CMDBlock->addUSERFUNC($1, $2, @$))
         delete ($2);
      delete($1); cfd = NULL;
   }
;
 
block:
     '{'                                   {
         CMDBlock = new parsercmd::cmdBLOCK();
         CMDBlock->pushblk();
      }
     statements '}'                        {
         $$ = CMDBlock;
         CMDBlock = CMDBlock->popblk();
      }
;

funcblock :
     '{'                                   {
         CMDBlock = new parsercmd::cmdFUNC(cfd->argListCopy(),cfd->type());
         CMDBlock->pushblk();
      }
     statements '}'                        {
         $$ = static_cast<parsercmd::cmdFUNC*>(CMDBlock);
         CMDBlock = CMDBlock->popblk();
      }
;

returnstatement :
     tknRETURN                             {
      if (!cfd) tellerror("return statement outside function body", @1);
      else {
         parsercmd::cmdRETURN* rcmd = new parsercmd::cmdRETURN(cfd->type());
         if (rcmd->checkRetype(NULL)) CMDBlock->pushcmd(rcmd);
         else {
            tellerror("return value expected", @1);
            delete rcmd;
         }
      }
      cfd->incReturns();
   }
   | tknRETURN  argument                   {
      if (!cfd) tellerror("return statement outside function body", @1);
      else {
         parsercmd::cmdRETURN* rcmd = new parsercmd::cmdRETURN(cfd->type());
         if (rcmd->checkRetype($2)) CMDBlock->pushcmd(rcmd);
         else {
            tellerror("return type different from function type", @2);
            delete rcmd;
         }
         delete $2;
      }
      cfd->incReturns();
   }
;

ifstatement:
     tknIF '(' expression ')' block {
         if (telldata::tn_bool != $3) tellerror("bool type expected",@3);
         else CMDBlock->pushcmd(new parsercmd::cmdIFELSE($5, NULL));
      }

   | tknIF '(' expression ')' block tknELSE block  {
         if (telldata::tn_bool != $3) tellerror("bool type expected",@3);
         else CMDBlock->pushcmd(new parsercmd::cmdIFELSE($5,$7));
      }
;

whilestatement:
     tknWHILE '('                   {
         CMDBlock = new parsercmd::cmdBLOCK();
         CMDBlock->pushblk();
      }
     expression ')'                 {
         if (telldata::tn_bool != $4) tellerror("bool type expected", @4);
      }
     block {
         parsercmd::cmdBLOCK* condBlock = CMDBlock;
         CMDBlock = CMDBlock->popblk();
         CMDBlock->pushcmd(new parsercmd::cmdWHILE(condBlock,$7));
   }
;

repeatstatement:
     tknREPEAT block tknUNTIL '('   {
         CMDBlock = new parsercmd::cmdBLOCK();
         CMDBlock->pushblk();
      }
     expression ')'                 {
         parsercmd::cmdBLOCK* condBlock = CMDBlock;
         CMDBlock = CMDBlock->popblk();
         if (telldata::tn_bool != $6) {
            tellerror("bool type expected", @6);
            delete condBlock;
         }
         else CMDBlock->pushcmd(new parsercmd::cmdREPEAT(condBlock,$2));
   }
;

statements:
     statement                             {}
   | statements ';' statement              {}
;

statement:
                                           { }
   | variabledeclaration                   { }
   | assignment                            {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
   | ifstatement                           {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
   | whilestatement                        {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
   | repeatstatement                       {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
   | returnstatement                       {/*keep the return value in the stack*/}
   | funccall                              {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
   | recorddefinition                      { }
;

funccall:
     tknIDENTIFIER '('                     {
        argmap = new telldata::argumentQ;
        argmapstack.push(argmap);
   }
      arguments ')'                        {
      parsercmd::cmdSTDFUNC *fc = CMDBlock->getFuncBody($1,$4);
      if (fc) {
         CMDBlock->pushcmd(new parsercmd::cmdFUNCCALL(fc,$1));
         $$ = fc->gettype();
      }
      else tellerror("unknown function name or wrong parameter list",@1);
      telldata::argQClear(argmap);
      argmapstack.pop();
      delete argmap;
      if (argmapstack.size() > 0) argmap = argmapstack.top();
      else argmap = NULL;
      delete [] $1;
   }
;

assignment:
     lvalue '='                            {tell_lvalue = tellvar;}
   argument                                {
      /*because of the (possible) structure that has an unknown yet tn_usertypes type,
      here we are doing the type checking, using the type of the lvalue*/
      $$ = parsercmd::Assign(tell_lvalue, $4, @2);
      delete $4;
   }
;

arguments:
                                           {$$ = NULL;}
    | nearguments                          {$$ = $1;}
;

argument :
     funccall                              {$$ = new telldata::argumentID($1);}
   | expression                            {$$ = new telldata::argumentID($1);}
   | assignment                            {$$ = new telldata::argumentID($1);}
   | structure                             {$$ = $1;}
;

nearguments :
      argument                             {argmap->push_back($1); $$ = argmap;}
   | nearguments ',' argument              {argmap->push_back($3); $$ = argmap;}
;

funcarguments:
                                           {}
   | funcneargument                        {}
;

funcneargument:
     funcargument                          {}
   | funcneargument ',' funcargument       {}
;

funcargument:
     telltypeID tknIDENTIFIER              {
      tellvar = CMDBlock->newTellvar($1, @1);
      cfd->pushArg(new parsercmd::argumentTYPE($2,tellvar));
      delete [] $2;
   }
;

lvalue:
     variable                               {$$ = $1;}
   | variabledeclaration                    {$$ = $1;}
;

variable:
     tknIDENTIFIER                         {
      tellvar = CMDBlock->getID($1);
      if (tellvar) $$ = tellvar->get_type();
      else tellerror("variable not defined in this scope", @1);
      delete [] $1;
   }
   | fieldname                             {$$ = $1;}
;

variabledeclaration:
     telltypeID   tknIDENTIFIER            {$$ = $1;
      telldata::tell_var* v = CMDBlock->getID($2, true);
      if (!v) {/* if this variableID doesn't exist already in the local scope*/
         /* add it to the local variable map */
         tellvar = CMDBlock->newTellvar($1, @1);
         CMDBlock->addID($2,tellvar); 
      }
      else tellerror("variable already defined in this scope", @1);
      delete [] $2;
   }
;

fielddeclaration:
     telltypeID  tknIDENTIFIER              {
      const telldata::tell_type* ftype =
            CMDBlock->getTypeByID($1 & ~telldata::tn_listmask);
      if (!tellstruct->addfield($2, $1, ftype)) {
         tellerror("field with this name already defined in this strucutre", @2);
         $$ = false; // indicates that definition fails
      }
      else $$ = true;
      delete [] $2;
   }
;

telltypeID:
     telltype                              {$$ = $1;}
   | telltype tknLISTdef                   {$$ = $1 | telldata::tn_listmask;}
;

telltype:
     tknVOIDdef                            {$$ = telldata::tn_void;}
   | tknREALdef                            {$$ = telldata::tn_real;}
   | tknINTdef                             {$$ = telldata::tn_int;}
   | tknBOOLdef                            {$$ = telldata::tn_bool;}
   | tknPOINTdef                           {$$ = telldata::tn_pnt;}
   | tknBOXdef                             {$$ = telldata::tn_box;}
   | tknSTRINGdef                          {$$ = telldata::tn_string;}
   | tknLAYOUTdef                          {$$ = telldata::tn_layout;}
   | tknIDENTIFIER                         {
        const telldata::tell_type* ttype = CMDBlock->getTypeByName($1);
        if (NULL == ttype)  {
           tellerror("Bad type specifier", @1);YYABORT;
        }
        else $$ = ttype->ID();
        delete [] $1;
      }
;

recorddefinition:
     tknSTRUCTdef tknIDENTIFIER            {
        tellstruct = CMDBlock->requesttypeID($2);
        if (NULL == tellstruct) {
           tellerror("type with this name already defined", @1);
           delete [] $2;
           YYABORT;
        }
     }
     '{' typedefstruct '}'                {
        if ($5) CMDBlock->addlocaltype($2,tellstruct);
        else delete tellstruct;
        tellstruct = NULL;
        delete [] $2;
     }
;

typedefstruct:
     fielddeclaration                     { $$ = $1;      }
   | typedefstruct ';' fielddeclaration   { $$ = $1 && $3;}
;

fieldname:
     variable tknFIELD              {
      assert(NULL != tellvar);
      tellvar = tellvar->field_var($2);
      if (tellvar) $$ = tellvar->get_type();
      else tellerror("Bad field identifier", @2);
      delete [] $2;
    }
;

structure:
     '{'                                  {
        argmap = new telldata::argumentQ;
        argmapstack.push(argmap);
   }
      nearguments '}'                     {
        /*Important note!. Here we will get a list of components that could be
          a tell list or some kind of tell struct or even tell list of tell struct.
          There is no way at this moment to determine the type of the input structure
          for (seems) obvious reasons. So - the type check and the eventual pushcmd
          are postponed untill we get the recepient - i.e. the lvalue or the
          function call. $$ is assigned to argumentID, that caries the whole argument
          queue listed in structure*/
        parsercmd::cmdSTRUCT* struct_command = new parsercmd::cmdSTRUCT();
        CMDBlock->pushcmd(struct_command);
        $$ = new telldata::argumentID(argmap, struct_command);
        //argQClear(argmap);
        argmapstack.pop();
        delete argmap;
        if (argmapstack.size() > 0) argmap = argmapstack.top();
        else argmap = NULL;
   }
;

anonymousvar:
     telltypeID   structure               {
      // the structure is without a type at this moment, so here we do the type checking
      if (parsercmd::StructTypeCheck($1, $2, @2)) {
         tellvar = CMDBlock->newTellvar($1, @1);
         $$ = $1;
      }
      else tellerror("Type mismatch", @2);
   }
;

/*==EXPRESSION===============================================================*/
/*orexpression*/
expression : 
     andexpression                         {$$ = $1;}
   | expression tknOR andexpression        {$$ = parsercmd::BoolEx($1,$3,"||",@1,@2);}
;

andexpression :
     eqexpression                          {$$ = $1;}
   | andexpression tknAND eqexpression     {$$ = parsercmd::BoolEx($1,$3,"&&",@1,@2);}
;

eqexpression :
     relexpression                         {$$ = $1;}
   | eqexpression tknEQ relexpression      {$$ = parsercmd::BoolEx($1,$3,"==",@1,@2);}
   | eqexpression tknNEQ relexpression     {$$ = parsercmd::BoolEx($1,$3,"!=",@1,@2);}
;

relexpression :
     addexpression                         {$$ = $1;}
   | relexpression '<' addexpression       {$$ = parsercmd::BoolEx($1,$3,"<",@1,@2);}
   | relexpression '>' addexpression       {$$ = parsercmd::BoolEx($1,$3,">",@1,@2);}
   | relexpression tknLEQ addexpression    {$$ = parsercmd::BoolEx($1,$3,"<=",@1,@2);}
   | relexpression tknGEQ addexpression    {$$ = parsercmd::BoolEx($1,$3,">=",@1,@2);}
;

addexpression: 
     multiexpression                       {$$ = $1;}
   | addexpression '+' multiexpression     {$$ = parsercmd::Plus($1,$3,@1,@3);}
   | addexpression '-' multiexpression     {$$ = parsercmd::Minus($1,$3,@1,@3);}
   | addexpression tknNE multiexpression   {$$ = parsercmd::PointMv($1,$3,@1,@3,+1,+1);}
   | addexpression tknNW multiexpression   {$$ = parsercmd::PointMv($1,$3,@1,@3,-1,+1);}
   | addexpression tknSE multiexpression   {$$ = parsercmd::PointMv($1,$3,@1,@3,+1,-1);}
   | addexpression tknSW multiexpression   {$$ = parsercmd::PointMv($1,$3,@1,@3,-1,-1);}
;

multiexpression : 
     unaryexpression                       {$$ = $1;}
   | multiexpression '*' unaryexpression   {$$ = parsercmd::Multiply($1,$3,@1,@3);}
   | multiexpression '/' unaryexpression   {$$ = parsercmd::Divide($1,$3,@1,@3);}
;

unaryexpression : 
     primaryexpression	                   {$$ = $1;}
   | '-' primaryexpression                 {$$ = parsercmd::UMinus($2,@2);}
;

primaryexpression : 
     tknREAL                               {$$ = telldata::tn_real;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttreal($1),true));}
   | tknINT                                {$$ = telldata::tn_int;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttint($1),true));}
   | tknTRUE                               {$$ = telldata::tn_bool;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttbool(true),true));}
   | tknFALSE                              {$$ = telldata::tn_bool;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttbool(false),true));}
   | tknSTRING                             {$$ = telldata::tn_string;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttstring($1),true));
                                                                delete [] $1;}
   | variable                              {$$ = $1;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(tellvar));}
   | anonymousvar                          {$$ = $1;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(tellvar));}
   | '(' expression ')'                    {$$ = $2;}
   | tknERROR                              {tellerror("Unexpected symbol", @1);}
;

%%
/*-------------------------------------------------------------------------*/
int yyerror (char *s) {  /* Called by yyparse on error */
   std::ostringstream ost;
   ost << "line " << telllloc.first_line << ": col " << telllloc.first_column
       << ": " << s;
   tell_log(console::MT_ERROR,ost.str());
   return 0;
}

void tellerror (std::string s, YYLTYPE loc) {
   if (cfd) cfd->incErrors();
   else     yynerrs++;
   std::ostringstream ost;
   ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
   if (loc.filename) {
      std::string fn = loc.filename;
      ost << "in file \"" << fn << "\" : ";
   }
   ost << s;
   tell_log(console::MT_ERROR,ost.str());
}

void tellerror (std::string s) {
   if (cfd) cfd->incErrors();
   else     yynerrs++;
   std::ostringstream ost;
   ost << "line " << telllloc.first_line << ": col " << telllloc.first_column << ": " << s;
   tell_log(console::MT_ERROR,ost.str());
}
// 
// {
//          /*Check whether such a function is already defined */
//          if (NULL != CMDBlock->funcDefined($2,arglist)) {
//             tellerror("function already defined",@$);
//             delete [] $2;
//             parsercmd::ClearArgumentList(arglist); delete(arglist); arglist = NULL;
//             YYABORT;
// }
// }
