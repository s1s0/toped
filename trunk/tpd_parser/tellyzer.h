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
//        Created: Fri Nov 08 2002
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: TELL interpreter
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#if !defined(TELLYZER_H_INCLUDED)
#define TELLYZER_H_INCLUDED

#include <stdio.h>
#include <string>
#include <map>
#include <deque>
#include <stack>
#include <fstream>
#include "tldat.h"

#define  EXEC_NEXT      0
#define  EXEC_RETURN    1
#define  EXEC_ABORT     2

//-----------------------------------------------------------------------------
// Some forward declarations
//-----------------------------------------------------------------------------
int telllex(void);
int tellerror (std::string *s);

namespace  parsercmd {

   //! Used by lexer to include multiply files and for error tracing
   class lexer_files {
   public:
      lexer_files(void* fh, YYLTYPE* loc) :
                                 lexfilehandler(fh), location(loc) {};
      ~lexer_files()                               {delete location;};
      void*    lexfilehandler;
      YYLTYPE* location;
   };

   /*! Implements a simple preprocessor and more specifically the state of the
    * preprocessor. Hold all currently defined pre-processor variables as well
    * as the current pre-processor state with regards to #if(n)def/#else/#endif
    */
   class TellPreProc {
      public:
                           TellPreProc();
         void              define(std::string, std::string, const TpdYYLtype&);
         void              undefine(std::string, const TpdYYLtype&);
         void              define(std::string);
         void              cmdlDefine(std::string);
         void              preDefine(std::string, const TpdYYLtype&);
         bool              check(std::string, std::string&);
         bool              ppIfDef(std::string);
         bool              ppIfNDef(std::string);
         void              ppPush();
         bool              ppElse(const TpdYYLtype&);
         bool              ppEndIf(const TpdYYLtype&);
         void              checkEOF();
         void              markBOF();
         bool              lastError();
         void              pushTllFileName(std::string fn) {_tllFN.push(fn);}
         std::string       popTllFileName();
         bool              pragOnce();
         void              reset();
      private:
         void              ppError(std::string, const TpdYYLtype&);
         void              ppWarning(std::string, const TpdYYLtype&);
         typedef enum { ppINACTIVE, ppBYPASS, ppACTIVE, ppDEEPBYPASS } PpState;
         typedef std::map <std::string, std::string> VariableMap;
         typedef std::stack <PpState>     StateStack;
         typedef std::stack <unsigned>    DepthStack;
         typedef std::stack <std::string> FileNameStack;
         VariableMap       _variables;
         std::string       _preDef;
         bool              _lastError; //! Keeps the state of the last operation
         FileNameStack     _tllFN;   //! Currently open tll files
         StateStack        _ppState;
         DepthStack        _ifdefDepth;
         NameSet           _parsedFiles; //! parsed files which had #pragma once statement
   };

   class cmdVIRTUAL;
   class cmdSTDFUNC;
   class cmdFUNC;
   class cmdBLOCK;
   class FuncDeclaration;
   class cmdCALLBACK;
//-----------------------------------------------------------------------------
// Define the types of tell structures
//-----------------------------------------------------------------------------
// tell_tn     - list of tell types - used for argument type checking
// variableMAP - a map (name - variable) structure used to accommodate the
//               tell variables
// functionMAP - a map (name - function block) structure containing all defined
//               tell functions. There is only one defined variable of this
//               type funcMAP - static member of cmdBLOCK class.
// BlockSTACK  - a stack structure containing current blocks nesting. Only
//               one variable is defined of this class - blocks - static member
//               of cmdBLOCK.
// CmdQUEUE    - a queue structure containing the list of tell operators
//operandSTACK - a stack structure used for storing the operans and results.
//               A single static variable - OPstack - is defined in cmdVIRTUAL
//               class. Used during execution
// ArgumentTYPE- ???
// ArgumentLIST- ???
// argumentMAP - ???
//-----------------------------------------------------------------------------
   typedef  std::multimap<std::string, cmdSTDFUNC*>      FunctionMAP;
   typedef  std::map<std::string, cmdSTDFUNC*>           CBFuncMAP;
   typedef  std::deque<cmdBLOCK*>                        BlockSTACK;
   typedef  std::deque<cmdVIRTUAL*>                      CmdQUEUE;
   typedef  std::deque<cmdSTDFUNC*>                      UndoQUEUE;
   typedef  std::deque<void*>                            UndoUQUEUE;
   typedef  std::pair<std::string,telldata::TellVar*>    ArgumentTYPE;
   typedef  std::deque<ArgumentTYPE*>                    ArgumentLIST;
   typedef enum {sdbrSORTED, sdbrUNSORTED, sdbrDONTCARE} DbSortState;

   /*** cmdVIRTUAL **************************************************************
   > virtual class inherited by all tell classes
   >>> Constructor --------------------------------------------------------------
   > -
   >>> Data fields --------------------------------------------------------------
   > OPstack            - Operand stack used to store operans and the results of
   >                      every executed operation
   >>> Methods ------------------------------------------------------------------
   > execute()          - Execute the operator (or command)
   ******************************************************************************/
   class cmdVIRTUAL {
   public:
                   cmdVIRTUAL(): _opstackerr(false) {};
      virtual int  execute() = 0;
              real getOpValue(telldata::operandSTACK& OPs = OPstack);
              word getWordValue(telldata::operandSTACK& OPs = OPstack);
              byte getByteValue(telldata::operandSTACK& OPs = OPstack);
       std::string getStringValue(telldata::operandSTACK& OPs = OPstack);
              bool getBoolValue(telldata::operandSTACK& OPs = OPstack);
             dword getIndexValue(telldata::operandSTACK& OPs = OPstack);
              real getOpValue(telldata::UNDOPerandQUEUE&, bool);
              word getWordValue(telldata::UNDOPerandQUEUE&, bool);
              byte getByteValue(telldata::UNDOPerandQUEUE&, bool);
       std::string getStringValue(telldata::UNDOPerandQUEUE&, bool);
              bool getBoolValue(telldata::UNDOPerandQUEUE&, bool);
      virtual ~cmdVIRTUAL() {};
   protected:
      static telldata::operandSTACK       OPstack;      // Operand stack
      static telldata::UNDOPerandQUEUE    UNDOPstack;   // undo operand stack
      static UndoQUEUE                    UNDOcmdQ;     // undo command stack
      static UndoUQUEUE                   UNDOUstack;   // undo utility stack
      bool                                _opstackerr;  // error while extracting operands
   };

   /*****************************************************************************
    The definition of the following classes is trivial
   ******************************************************************************/
   class cmdPLUS:public cmdVIRTUAL {
   public:
                  cmdPLUS(telldata::typeID retype): _retype(retype) {};
      virtual    ~cmdPLUS() {};
      virtual int execute();
   private:
      telldata::typeID _retype;
   };

   class cmdCONCATENATE:public cmdVIRTUAL {
   public:
                  cmdCONCATENATE(){}
      virtual    ~cmdCONCATENATE(){}
      virtual int execute();
   };

   class cmdMINUS:public cmdVIRTUAL {
   public:
                  cmdMINUS(telldata::typeID retype): _retype(retype) {};
      virtual    ~cmdMINUS(){}
      virtual int execute();
   private:
      telldata::typeID _retype;
   };

   class cmdSHIFTPNT:public cmdVIRTUAL {
   public:
                  cmdSHIFTPNT(int sign, bool swap):_sign(sign), _swapOperands(swap) {};
      virtual    ~cmdSHIFTPNT(){}
      virtual int execute();
   private:
      int _sign;
      bool _swapOperands;
   };

   class cmdSHIFTPNT2:public cmdVIRTUAL {
   public:
                  cmdSHIFTPNT2(int sign = 1):_sign(sign) {};
      virtual    ~cmdSHIFTPNT2() {}
      virtual int execute();
   private:
      int _sign;
   };

   class cmdSHIFTPNT3:public cmdVIRTUAL {
   public:
                  cmdSHIFTPNT3(int signX, int signY): _signX(signX), _signY(signY) {};
      virtual    ~cmdSHIFTPNT3() {}
      virtual int execute();
   private:
      int _signX;
      int _signY;
   };

   class cmdSHIFTPNT4:public cmdVIRTUAL {
   public:
                  cmdSHIFTPNT4(int signX, int signY): _signX(signX), _signY(signY) {};
      virtual    ~cmdSHIFTPNT4() {}
      virtual int execute();
   private:
      int _signX;
      int _signY;
   };

   class cmdSHIFTBOX:public cmdVIRTUAL {
   public:
                  cmdSHIFTBOX(int sign, bool swap):_sign(sign), _swapOperands(swap) {};
      virtual    ~cmdSHIFTBOX() {}
      virtual int execute();
   private:
      int _sign;
      bool _swapOperands;
   };

   class cmdSHIFTBOX3:public cmdVIRTUAL {
   public:
                  cmdSHIFTBOX3(int signX, int signY): _signX(signX), _signY(signY) {};
      virtual    ~cmdSHIFTBOX3() {}
      virtual int execute();
   private:
      int _signX;
      int _signY;
   };

   class cmdSHIFTBOX4:public cmdVIRTUAL {
   public:
                  cmdSHIFTBOX4(int signX, int signY): _signX(signX), _signY(signY) {};
      virtual    ~cmdSHIFTBOX4() {}
      virtual int execute();
   private:
      int _signX;
      int _signY;
   };

   class cmdBLOWBOX:public cmdVIRTUAL {
   public:
                  cmdBLOWBOX(int sign, bool swap): _sign(sign), _swapOperands(swap) {};
      virtual    ~cmdBLOWBOX() {}
      virtual int execute();
   private:
      int _sign;
      bool _swapOperands;
   };

   class cmdMULTIPLY:public cmdVIRTUAL {
   public:
                  cmdMULTIPLY(telldata::typeID type):_retype(type)  {}
      virtual    ~cmdMULTIPLY() {}
      virtual int execute();
   private:
      telldata::typeID  _retype;
   };

   class cmdDIVISION:public cmdVIRTUAL {
   public:
                  cmdDIVISION(telldata::typeID type):_retype(type)  {}
      virtual    ~cmdDIVISION() {}
      virtual int execute();
   private:
      telldata::typeID  _retype;
   };

   class cmdSCALEPNT:public cmdVIRTUAL {
   public:
                  cmdSCALEPNT(bool up, bool swap) : _up(up), _swapOperands(swap) {};
      virtual    ~cmdSCALEPNT() {}
      virtual int execute();
   private:
      bool _up;
      bool _swapOperands;
   };

   class cmdSCALEBOX:public cmdVIRTUAL {
   public:
                  cmdSCALEBOX(bool up, bool swap) : _up(up), _swapOperands(swap) {};
      virtual    ~cmdSCALEBOX() {}
      virtual int execute();
   private:
      bool _up;
      bool _swapOperands;
   };

   class cmdUMINUS:public cmdVIRTUAL {
   public:
                  cmdUMINUS(telldata::typeID type):_type(type) {};
      virtual    ~cmdUMINUS() {}
      virtual int execute();
   private:
      telldata::typeID  _type;
   };

   class cmdLT:public cmdVIRTUAL {
   public:
                  cmdLT() {}
      virtual    ~cmdLT() {}
      virtual int execute();
   };

   class cmdLET:public cmdVIRTUAL {
   public:
                  cmdLET() {}
      virtual    ~cmdLET() {}
      virtual int execute();
   };

   class cmdGT:public cmdVIRTUAL {
   public:
                  cmdGT() {}
      virtual    ~cmdGT() {}
      virtual int execute();
   };

   class cmdGET:public cmdVIRTUAL {
   public:
                  cmdGET() {}
      virtual    ~cmdGET() {}
      virtual int execute();
   };

   class cmdEQ:public cmdVIRTUAL {
   public:
                  cmdEQ() {}
      virtual    ~cmdEQ() {}
      virtual int execute();
   };

   class cmdNE:public cmdVIRTUAL {
   public:
                  cmdNE() {}
      virtual    ~cmdNE() {}
      virtual int execute();
   };

   class cmdNOT:public cmdVIRTUAL {
   public:
                  cmdNOT() {}
      virtual    ~cmdNOT() {}
      virtual int execute();
   };

   class cmdBWNOT:public cmdVIRTUAL {
   public:
                  cmdBWNOT() {}
      virtual    ~cmdBWNOT() {}
      virtual int execute();
   };

   class cmdAND:public cmdVIRTUAL {
   public:
                  cmdAND() {}
      virtual    ~cmdAND() {}
      virtual int execute();
   };

   class cmdBWAND:public cmdVIRTUAL {
   public:
                  cmdBWAND() {}
      virtual    ~cmdBWAND() {}
      virtual int execute();
   };

   class cmdOR:public cmdVIRTUAL {
   public:
                  cmdOR() {}
      virtual    ~cmdOR() {}
      virtual int execute();
   };

   class cmdBWOR:public cmdVIRTUAL {
   public:
                  cmdBWOR() {}
      virtual    ~cmdBWOR() {}
      virtual int execute();
   };

   class cmdSTACKRST:public cmdVIRTUAL {
   public:
                  cmdSTACKRST() {}
      virtual    ~cmdSTACKRST() {}
      virtual int execute();
   };

   class cmdLISTSIZE:public cmdVIRTUAL {
   public:
                  cmdLISTSIZE(telldata::TellVar* var);
      virtual    ~cmdLISTSIZE() {}
      virtual int execute();
   protected:
      telldata::TellVar*  _var;
      telldata::TellVar*  _initVar;
   };

   class cmdASSIGN:public cmdVIRTUAL {
   public:
                  cmdASSIGN(telldata::TellVar* var, byte indexed): _var(var), _indexed(indexed) {};
      virtual    ~cmdASSIGN() {}
      virtual int execute();
   protected:
      telldata::TellVar*   _var;
      byte                 _indexed;
   };

   class cmdPUSH:public cmdVIRTUAL {
   public:
                  cmdPUSH(telldata::TellVar *v, byte indexed, bool constant=false):
                        _var(v),  _indexed(indexed), _constant(constant) {};
      virtual    ~cmdPUSH() {if (_constant) delete _var;};
      virtual int execute();
   private:
      telldata::TellVar*   _var;
      byte                 _indexed;
      bool                 _constant;
   };

   /*! Anonymous variable. This class exists purely to clean-up the anonymous
    * variable created during the parsing. All the functionality is done in
    * the assign object which is supposed to get into the stack before this
    * object.
    */
   class cmdANOVAR:public cmdVIRTUAL {
   public:
                  cmdANOVAR(telldata::TellVar *v) : _var(v) {};
      virtual    ~cmdANOVAR() { delete _var;}
      virtual int execute() {return EXEC_NEXT;}
   private:
      telldata::TellVar*   _var;
   };

   class cmdSTRUCT: public cmdVIRTUAL {
   public:
                  cmdSTRUCT() : _arg(NULL) {}
      virtual    ~cmdSTRUCT()                         {if (NULL != _arg) delete _arg;}
      void        setargID(telldata::ArgumentID* arg) {_arg = DEBUG_NEW telldata::ArgumentID(*arg);}
      virtual int execute();
   private:
      telldata::TellVar*      getList();
      telldata::ArgumentID*  _arg;
   };

   /*!
    * Function reference. This class holds a function reference or in C terms a
    * pointer to an existing TELL function. It is part of the call back mechanism
    * implemented in TELL. In parse time an object of this class is pushed in the
    * command queue and initialized with the name of the referenced function and
    * with the default typeID for a function reference. A pointer to this object
    * is also stored in the corresponding ArgumentID object which will do the
    * type checks later and eventually update the _funcBody and _ID fields.\n
    * In run time (execute()) those parameters will be transfered to the
    * corresponding telldata::TtCallBack variable through the operand stack thus
    * completing the substitution of the original empty function.
    * There are two possible argument checks that an object of this type might
    * get involved in. The first one is a simple assignment. The second one is
    * a function call. The latter implements more complex parameter evaluation
    * which in turn requires a "late update" of this object. This mechanism is
    * implemented via _preCheckfBody field and the methods that use it.
    *
    */
   class cmdFUNCREF: public cmdVIRTUAL {
   public:
                  cmdFUNCREF(std::string fn) : _funcBody(NULL), _preCheckfBody(NULL), _funcName(fn), _ID(telldata::tn_anyfref) {}
      virtual    ~cmdFUNCREF()                     {/*nothing to clean-up here*/}
      void        setFuncBody(cmdSTDFUNC* funcBody, telldata::typeID ID) {_funcBody = funcBody; _ID = ID;}
      void        setFuncBody(telldata::typeID ID) {assert(_preCheckfBody); _funcBody = _preCheckfBody; _ID = ID;}
      void        setPreCheckfBody(cmdSTDFUNC* funcBody) {_preCheckfBody = funcBody;}
      std::string funcName() const              {return _funcName;}
      virtual int execute();
   private:
      cmdSTDFUNC*       _funcBody;
      cmdSTDFUNC*       _preCheckfBody;
      std::string       _funcName;
      telldata::typeID  _ID;
   };

   /*!
    * To handle tell functions with variable number of arguments (like printf)
    * we need to know in run time how many parameters were pushed in the OPstack
    * This class is doing exactly this. It stores the _numParams field extracted
    * in parse time and pushes it in the OPstack just before the call of a function
    * with a variable number of arguments.
    */
   class cmdNUMFPARAMS: public cmdVIRTUAL {
   public:
                  cmdNUMFPARAMS(const telldata::argumentQ* aq) : _numParams(aq->size()) {}
      virtual    ~cmdNUMFPARAMS()                     {/*nothing to clean-up here*/}
      virtual int execute();
   private:
      unsigned          _numParams;
   };

   class cmdLISTADD : public cmdVIRTUAL {
   public:
                  cmdLISTADD(telldata::TellVar* listarg, bool prefix, bool index) :
                     _listarg(static_cast<telldata::TtList*>(listarg)), _prefix(prefix), _index(index) {};
      virtual    ~cmdLISTADD(){}
      virtual int execute();
   protected:
                  cmdLISTADD(cmdLISTADD* indxcmd) :
                     _listarg(indxcmd->_listarg),_prefix(indxcmd->_prefix), _index(indxcmd->_index) {};
      dword       getIndex();
      // don't delete this and don't get confused. It's only a pointer to a variable,
      // that normally should be in the operand stack. List operations are an exception -
      // see the comments in the parser (tell_yacc.yy)
      telldata::TtList*     _listarg;
      bool                  _prefix;
      bool                  _index;
      bool                  _empty_list;
   };

   class cmdLISTUNION : public cmdLISTADD {
   public:
                  cmdLISTUNION(cmdLISTADD* indxcmd) : cmdLISTADD(indxcmd) {}
      virtual    ~cmdLISTUNION() {}
      virtual int execute();
   };

   class cmdLISTSUB : public cmdVIRTUAL {
   public:
                  cmdLISTSUB(telldata::TellVar* listarg, bool prefix, bool index) :
                     _listarg(static_cast<telldata::TtList*>(listarg)), _prefix(prefix), _index(index) {};
      virtual    ~cmdLISTSUB() {}
      virtual int execute();
   private:
      telldata::TtList*     _listarg;
      bool                  _prefix;
      bool                  _index;
   };

   class cmdLISTSLICE : public cmdVIRTUAL {
   public:
                  cmdLISTSLICE(telldata::TellVar* listarg, bool prefix, bool index) :
                    _listarg(static_cast<telldata::TtList*>(listarg)), _prefix(prefix), _index(index) {};
      virtual    ~cmdLISTSLICE() {}
      virtual int execute();
   private:
      telldata::TtList*     _listarg;
      bool                  _prefix;
      bool                  _index;
   };

   class cmdRETURN:public cmdVIRTUAL {
   public:
                  cmdRETURN(telldata::typeID tID) : _retype(tID) {};
      virtual    ~cmdRETURN() {}
      bool        checkRetype(telldata::ArgumentID* arg);
      virtual int execute()  {return EXEC_RETURN;};
   private:
      telldata::typeID  _retype;
   };

   class cmdFUNCCALL: public cmdVIRTUAL {
   public:
                  cmdFUNCCALL(cmdSTDFUNC* bd, std::string fn):_funcbody(bd), _funcname(fn) {};
      virtual    ~cmdFUNCCALL() {}
      virtual int execute();
   protected:
      cmdSTDFUNC*       _funcbody;
      std::string       _funcname;
   };

   /*** cmdBLOCK ****************************************************************
   > The class defines the tell block of operators structure. It is the base
   > class for
   >>> Constructor --------------------------------------------------------------
   > -
   >>> Data fields --------------------------------------------------------------
   > _cmdQ      - Contains the list of tell commands
   > _varLocal  - Contains the map of the local variables
   > blocks    - static - structure of the nested blocks. Used during the parsing
   > funcMAP   - static - map of defined tell functions. No nested functions
                 allowed, so all of them are defined in the main block
   >>> Methods ------------------------------------------------------------------
   > addFUNC   -
   > addID     - add a new variable to the variableMAP structure. The new
                 variable is initialized. Called when an identifier is matched.
                 If _varLocal exists, the variable is added there, otherwise
                 VARglobal map is used
   > getID     - extracts TellVar* from variableMAP by name. Local variableMAP
                 has a priority(if exists). Returns NULL if such a name is not
                 found in both (_varLocal, VARglobal) maps.
   > getfuncID - extracts the cmdBLOCK* from the funcMAP by name. Returns NULL
                 if function name doesn't exist
   > pushcmd   - add the parsed command to the _cmdQ queue
   > pushblock - push current block in the blocks stack structure
   > popblock  - returns (and removes) the top of the blocks structure
   > run       - override of the QThread::run(). The execution of the block
                 called from the parser needs to start in a separate thread
                 because of the interactive functions.
   > cleaner   - clean the instruction queue down to the previous ';'
   ******************************************************************************/
   class cmdBLOCK:public virtual cmdVIRTUAL {
   public:
                                 cmdBLOCK();
                                 cmdBLOCK(telldata::typeID lltID) :
                                                            _nextLclTypeID(lltID){};
      virtual                   ~cmdBLOCK();
      virtual int                execute();
      void                       cleaner(bool fullreset = false);
      virtual void               addFUNC(std::string, cmdSTDFUNC*);
      virtual void               addUSERFUNC(FuncDeclaration*, cmdFUNC*, TpdYYLtype);
      virtual cmdFUNC*           addUSERFUNCDECL(FuncDeclaration*, TpdYYLtype);
      void                       addID(const char*, telldata::TellVar*);
      void                       addconstID(const char*, telldata::TellVar*, bool initialized);
      void                       addlocaltype(const char*, telldata::TType*);
      void                       addAnoLoType(telldata::TType*);
      telldata::TCompType*       secureCompType(char*&);
      telldata::TCallBackType*   secureCallBackType(const char*);
      const telldata::TType*     getTypeByName(char*&) const;
      const telldata::TType*     getTypeByID(const telldata::typeID ID) const;
      telldata::TellVar*         getID(const char*, bool local=false) const;
      telldata::TellVar*         newTellvar(telldata::typeID, const char*, TpdYYLtype);
      telldata::TellVar*         newFuncArg(telldata::typeID, TpdYYLtype);
      bool                       defValidate(const FuncDeclaration* decl, cmdFUNC*&);
      bool                       declValidate(const FuncDeclaration*, TpdYYLtype);
      cmdSTDFUNC*  const         getFuncBody(const char*, telldata::argumentQ*, std::string&) const;
      cmdSTDFUNC*  const         getIntFuncBody(std::string) const;
      void                       pushcmd(cmdVIRTUAL* cmd) {_cmdQ.push_back(cmd);};
      void                       pushblk()                {_blocks.push_front(this);};
      cmdBLOCK*                  popblk();
      cmdBLOCK*                  rootBlock()              {return _blocks.back();}
      void                       copyContents(cmdFUNC*);
      telldata::variableMAP*     copyVarLocal();
      void                       restoreVarLocal(telldata::variableMAP&);
      void                       initializeVarLocal();
      bool                       checkDbSortState(DbSortState);
      word                       undoDepth() {return _undoDepth;}
      void                       setUndoDepth(word ud) {_undoDepth = ud;}
   protected:
      bool                       addCALLBACKDECL(std::string, cmdCALLBACK*, TpdYYLtype);
      cmdSTDFUNC* const          getLocalFuncBody(const char*, telldata::argumentQ*) const;
      telldata::variableMAP     _varLocal;  //! list of local variables
      telldata::typeMAP         _typeLocal; //! list of local types
      telldata::TypeList        _typeAnoLo; //! list of anonymous local types (callbacks only so far)
      CBFuncMAP                 _lclFuncMAP;//! local function map (callback arguments only)
      CmdQUEUE                  _cmdQ;      //! list of commands
      static BlockSTACK         _blocks;
      static FunctionMAP        _funcMAP;
      static FunctionMAP        _internalFuncMap;
      static word               _undoDepth;
      static bool               _dbUnsorted;
      telldata::typeID          _nextLclTypeID;
   };

   class cmdMAIN:public cmdBLOCK {
   public:
                                cmdMAIN();
      virtual                  ~cmdMAIN();
      virtual int               execute();
      virtual void              addFUNC(std::string, cmdSTDFUNC*);
      void                      addIntFUNC(std::string, cmdSTDFUNC*);
      virtual void              addUSERFUNC(FuncDeclaration*, cmdFUNC*, TpdYYLtype);
      virtual cmdFUNC*          addUSERFUNCDECL(FuncDeclaration*, TpdYYLtype);
      void                      addGlobalType(std::string, telldata::TType*);
      void                      recoveryDone();
   };

   class cmdSTDFUNC:public virtual cmdVIRTUAL {
   public:
                                 cmdSTDFUNC(ArgumentLIST* vm, telldata::typeID tt, bool eor, DbSortState rDBt = sdbrSORTED):
                                    _arguments(vm), _returntype(tt), _execOnRecovery(eor), _dbSortStatus(rDBt) {};
      virtual                   ~cmdSTDFUNC();
      virtual int                execute() = 0;
      virtual void               undo() = 0;
      virtual void               undo_cleanup() = 0;
      void                       reduce_undo_stack();
      virtual NameList*          callingConv(const telldata::typeMAP*);
      virtual int                argsOK(telldata::argumentQ* amap, bool&);
      telldata::typeID           gettype() const {return _returntype;};
      virtual bool               internal() {return true;}
      virtual bool               declaration() {return false;}
      bool                       execOnRecovery() {return _execOnRecovery;}
      bool                       ignoreOnRecovery() { return _ignoreOnRecovery;}
      void                       set_ignoreOnRecovery(bool ior) {_ignoreOnRecovery = ior;}
      static void                setThreadExecution(bool te) {_threadExecution = te;}
      DbSortState                dbSortStatus() {return _dbSortStatus;}
      const ArgumentLIST*        getArguments() {return _arguments;}
      friend void cmdMAIN::recoveryDone();
   protected:
      ArgumentLIST*              _arguments;
      telldata::typeID           _returntype;
      bool                       _execOnRecovery;
      static bool                _ignoreOnRecovery;
      static bool                _threadExecution;
      const DbSortState          _dbSortStatus;
   };

   class cmdFUNC:public cmdSTDFUNC, public cmdBLOCK {
   public:
                              cmdFUNC(ArgumentLIST*, telldata::typeID, bool, TpdYYLtype);
      virtual                ~cmdFUNC() {}
      virtual int             execute();
      virtual bool            internal() {return false;}
      virtual bool            declaration() {return _declaration;}
      virtual void            undo() {};
      virtual void            undo_cleanup() {};
      void                    setDefined(bool defined = true) {_declaration = !defined;}
   protected:
      typedef std::stack<telldata::variableMAP*> LocalVarStack;
      typedef std::list<telldata::TellVar*> BackupList;
//      bool                    addCALLBACKPARAM(std::string, cmdCALLBACK*, TpdYYLtype);
      bool                    _declaration;
      BackupList*             backupOperandStack();
      void                    restoreOperandStack(BackupList*);
      word                    _recursyLevel;
      LocalVarStack           _VARLocalStack;
   };


   /*!
    * Every TELL variable of a callback type will invoke an object of this class.
    * In a sense it creates a function declaration in the parser which will allow
    * the incoming calls of that call back function to be parsed exactly as a
    * normal function call. During run time though the function body (_fbody)
    * shall be replaced (see cmdFUNCREF) before the execution comes to this object.
    * Then it's easy - we just execute the function body of the referenced function.\n
    * An important note (couple of nights already lost on this).
    * During the construction we're creating an argument list which is effectively
    * a copy of the typeID list in the corresponding telldata::TCallBackType i.e.
    * the type definition of this callback. That argument list contains anonymous
    * _arguments i.e. their names are empty strings, but this is OK, because they
    * are to be used in the argument checks and adjustments. Now this is the first
    * difference with the "regular" function declarations in TELL. The second and
    * more important one is that when the regular function which had been declared
    * gets defined, then its function body, local variables, local types etc are
    * pasted into the original object created by the declaration.\n
    * With callbacks this mechanism is not quite feasible unless we restrict
    * function references to user defined functions only. Much simpler mechanism
    * is implemented here which is - calling the referenced function directly.
    * The important part here is - don't try to extract the parameter values from
    * the operand stack in run time, _arguments of this object are to be ignored
    * in run time, they are here solely for argument checking purposes.
    */
   class cmdCALLBACK : public cmdFUNC {
   public:
                              cmdCALLBACK(const telldata::TypeIdList&, telldata::typeID, TpdYYLtype loc);
      virtual                ~cmdCALLBACK() {}
      void                    setFBody(parsercmd::cmdSTDFUNC* fbody) {_fbody = fbody; setDefined();}
      void                    unSetFBody() {_fbody = NULL; setDefined(false);}
      virtual int             execute();
   private:
      parsercmd::cmdSTDFUNC*  _fbody;
   };

   class cmdIFELSE: public cmdVIRTUAL {
   public:
                              cmdIFELSE(cmdBLOCK* tb, cmdBLOCK* fb):_trueblock(tb),_falseblock(fb) {};
      virtual                ~cmdIFELSE() {delete _trueblock; delete _falseblock;}
      virtual int             execute();
   private:
      cmdBLOCK*               _trueblock;
      cmdBLOCK*               _falseblock;
   };

   class cmdWHILE: public cmdVIRTUAL {
   public:
                               cmdWHILE(cmdBLOCK* cnd, cmdBLOCK* bd):_condblock(cnd),_body(bd) {};
      virtual                 ~cmdWHILE() {delete _condblock; delete _body;}
      virtual int              execute();
   private:
      cmdBLOCK*                _condblock;
      cmdBLOCK*                _body;
   };

   class cmdREPEAT: public cmdVIRTUAL {
   public:
                               cmdREPEAT(cmdBLOCK* cnd, cmdBLOCK* bd): _condblock(cnd), _body(bd) {};
      virtual                 ~cmdREPEAT() {delete _condblock; delete _body;}
      virtual int              execute();
   private:
      cmdBLOCK*                _condblock;
      cmdBLOCK*                _body;
   };

   class cmdFOREACH: public cmdVIRTUAL {
   public:
                           cmdFOREACH(telldata::TellVar* var, telldata::TellVar* listvar) :
                                       _var(var), _listvar(listvar), _header(NULL), _body(NULL) {};
      virtual             ~cmdFOREACH();
      void                 addBlocks(cmdBLOCK* hd, cmdBLOCK* bd)
                                                    {_header = hd; _body = bd;}
      virtual int          execute();
   private:
      telldata::TellVar*   _var;
      telldata::TellVar*   _listvar;
      cmdBLOCK*            _header;
      cmdBLOCK*            _body;
   };

   telldata::typeID UMinus(telldata::typeID, TpdYYLtype);
   telldata::typeID   Plus(telldata::typeID, telldata::typeID, TpdYYLtype, TpdYYLtype);
   telldata::typeID  Minus(telldata::typeID, telldata::typeID, TpdYYLtype, TpdYYLtype);
   telldata::typeID  PointMv(telldata::typeID, telldata::typeID, TpdYYLtype, TpdYYLtype, int, int);
   telldata::typeID  Multiply(telldata::typeID, telldata::typeID, TpdYYLtype, TpdYYLtype);
   telldata::typeID  Divide(telldata::typeID, telldata::typeID, TpdYYLtype, TpdYYLtype);
   telldata::typeID  Assign(telldata::TellVar*, byte, telldata::ArgumentID*, TpdYYLtype);
   telldata::typeID  Uninsert(telldata::TellVar*, telldata::ArgumentID*, parsercmd::cmdLISTADD*, TpdYYLtype);
   telldata::typeID  BoolEx(telldata::typeID, telldata::typeID, std::string, TpdYYLtype, TpdYYLtype);
   telldata::typeID  BoolEx(telldata::typeID, std::string, TpdYYLtype);

   telldata::TellVar* newCallBackArgument(telldata::typeID, TpdYYLtype);

   bool              StructTypeCheck(telldata::typeID, telldata::ArgumentID*, TpdYYLtype);
   bool              ListIndexCheck(telldata::typeID, TpdYYLtype, telldata::typeID, TpdYYLtype);
   bool              ListSliceCheck(telldata::typeID, TpdYYLtype, telldata::typeID, TpdYYLtype, telldata::typeID, TpdYYLtype);
   bool              ListSliceCheck(telldata::typeID, TpdYYLtype, telldata::typeID, TpdYYLtype);
   void              ClearArgumentList(ArgumentLIST*);
   bool              vplFunc(std::string);


   /*!
    * structure used during the parsing
   */
   class FuncDeclaration {
   public:
                                       FuncDeclaration(std::string name, telldata::typeID type):
                                          _name(name), _type(type), _numReturns(0), _numErrors(0)
                                          {_argList = DEBUG_NEW parsercmd::ArgumentLIST;}
                                      ~FuncDeclaration();
      const std::string                name() const     {return _name;}
      const telldata::typeID           type() const     {return _type;}
      const parsercmd::ArgumentLIST*   argList() const  {return _argList;}
      void                             pushArg(parsercmd::ArgumentTYPE* arg) {_argList->push_back(arg);}
      void                             incReturns() {_numReturns++;}
      void                             incErrors() {_numErrors++;}
      word                             numReturns() const {return _numReturns;}
      word                             numErrors() const {return _numErrors;}
      parsercmd::ArgumentLIST*         argListCopy() const;
   private:
      std::string                      _name;
      telldata::typeID                 _type;
      parsercmd::ArgumentLIST*         _argList;
      word                             _numReturns;
      word                             _numErrors;
   };

}

namespace telldata {
   //==============================================================================
   /*!
    * Callbacks in TELL are partially implemented using objects of this type.
    * The complexity here is coming from the fact that a TtCallBack object can
    * exists in several states:
    * - declared - _fcbBody exists, but is empty - i.e. nothing has been assigned
    *              to this object. That's usually the state after the variable
    *              declaration.
    * - defined  - the object simply carries a pointer to the definition of a
    *              function (_fBody). Such object is generated by the @operator
    * - linked   - a declared object received a pointer to a function and is ready
    *              to be executed. This happens when a callback variable has been
    *              initialized or assigned
    * - empty    - in the special cases when callback type is used in structures
    *              or lists a _fcbBody is not created because a callback can't
    *              be executed as a direct call to a field of a structure or a
    *              member of a record. So variables of this type are created
    *              empty. They can only get defined, but never linked or declared
    * The callback will be properly executed in the TELL code only if the object
    * is in linked state.\n
    * The other important note is that creating an object in declared state
    * implies that a function with the same name as the callback variable is
    * created (_fcbBody), and a pointer to it is stored in the local function
    * map of the current block. The pointer to that function is stored here, but
    * that object shall be neither deleted nor copied (I mean deep copy). This
    * is very important for the basic callback functionality - i.e. calling a
    * function with callback parameters. A callback parameter of a function will
    * generate an object in declared state. That object will be turned into
    * linked state only in run time, but by then the function body had been
    * already parsed and references to _fcbBody already linked in the function
    * block.
    */
   class TtCallBack : public TellVar {
   public:
                           TtCallBack(const typeID ID, parsercmd::cmdCALLBACK*);
                           TtCallBack(const typeID ID, parsercmd::cmdSTDFUNC*);
                           TtCallBack(const TtCallBack&);
                           TtCallBack(const typeID ID);
      virtual             ~TtCallBack() {}
      virtual void         initialize();
      virtual TellVar*     selfcopy() const;
//      virtual void         echo(std::string&, real);
      virtual void         assign(TellVar*);
      parsercmd::cmdCALLBACK* fcbBody()        { return _fcbBody;}
      parsercmd::cmdSTDFUNC*  fBody()          { return _fBody;  }
   protected:
      parsercmd::cmdCALLBACK* _fcbBody;
      parsercmd::cmdSTDFUNC*  _fBody;
      bool                 _definition;
   };
}

namespace console{
   class toped_logfile {
      public:
                           toped_logfile() :_enabled(true) {};
         void              close();
         void              init(const std::string logFileName, bool append = false);
         void              setFN(std::string fn) {_funcname = fn;};
         std::string       getFN() const {return _funcname;};
         void              setEnabled(bool enabled) {_enabled = enabled;}
         std::string       _2bool(bool b) {return b ? "true" : "false";};
         toped_logfile&    operator<< (const byte);
         toped_logfile&    operator<< (const word);
         toped_logfile&    operator<< (const int4b);
         toped_logfile&    operator<< (const real);
         toped_logfile&    operator<< (const std::string&);
         toped_logfile&    operator<< (const telldata::TtPnt&);
         toped_logfile&    operator<< (const telldata::TtWnd&);
         toped_logfile&    operator<< (const telldata::TtBnd&);
         toped_logfile&    operator<< (const telldata::TtLMap&);
         toped_logfile&    operator<< (const telldata::TtList&);
         toped_logfile&    operator<< (const telldata::TtLayer&);
         toped_logfile&    flush();
      private:
         std::fstream     _file;
         std::string      _funcname;
         bool             _enabled;
   };
}
#endif
