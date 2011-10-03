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
//        Created: Tue Apr 17 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED build-in functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef  TPDF_COMMON_H
#define  TPDF_COMMON_H

#include "tedstd.h"
#include "tellyzer.h"


//-----------------------------------------------------------------------------
//    === Two ways for _arguments checking of the standard functions ===
//
// First one is to build the argumentmap structure in the constructor and not
// define argsOK and callingConv methods. The argsOK method of the parent
// cmdSTDFUNC will check the _arguments.
//
// Second one is not to build argumentmap map (i.e. to leave _arguments=NULL)
// and to define argsOK. This one seems more flexible and looks like it takes
// less space - at least a function parameters do not need to be allocated. They
// are anonymous anyway, so their names can not be used.
// After implementation of the tell structures however this way looks like more
// hassle, because of the anonymous _arguments. We need to deal "per case" with
// the pain of determining the type of those _arguments, and this is not the error
// proof way. Besides in this case additional virtual method callingConv() has to
// be defined for every class.
//
// Bottom line, we are using the first way and the parent argsOK method for
// the majority of the functions. The exception here are functions like echo
// that should be flexible enough to get any type of parameters.
// It is defined using the second way.
//
// == Declaration of the standard functions using macro definitions ==
//
// To avoid boring and trivial class declaration here some preprocessor macros
// are used. Hope that in this case there is more convenience than harm using
// macroses with the following types of definitions


// TELL_STDCMD_CLASSA      - inherits directly cmdSTDFUNC - no UNDO
// The extra protected constructor is used by the inheritance class
// (if the class is inherited) because of the argument checks
#ifndef TELL_STDCMD_CLASSA
#define TELL_STDCMD_CLASSA(name)                                  \
   class name : public parsercmd::cmdSTDFUNC {                    \
   public:                                                        \
      name(telldata::typeID retype, bool eor);                    \
      int         execute();                                      \
      void        undo() {};                                      \
      void        undo_cleanup() {};                              \
   protected:                                                     \
      name(parsercmd::ArgumentLIST* al,telldata::typeID retype, bool eor) : \
                                    parsercmd::cmdSTDFUNC(al,retype,eor) {};\
   }
#endif

// TELL_STDCMD_CLASSA_UNDO - inherits directly cmdSTDFUNC - with UNDO
// The extra protected constructor is used by the inheritance class
// (if the class is inherited) because of the argument checks
#ifndef TELL_STDCMD_CLASSA_UNDO
#define TELL_STDCMD_CLASSA_UNDO(name)                             \
   class name : public parsercmd::cmdSTDFUNC {                    \
   public:                                                        \
      name(telldata::typeID retype, bool eor);                    \
      int         execute();                                      \
      void        undo();                                         \
      void        undo_cleanup();                                 \
   protected:                                                     \
      name(parsercmd::ArgumentLIST* al,telldata::typeID retype, bool eor, DbSortState rDBt= sdbrSORTED) : \
                   parsercmd::cmdSTDFUNC(al,retype, eor, rDBt) {};\
   }
#endif

// TELL_STDCMD_CLASSB - inherits one of the above class types,
// and is using the UNDO functionality of its ancestor.
// The classes of this type MUST use the protected constructor of their
// ancestor, otherwise the argument checks will be screwed-up.
// used for overloaded functions
#ifndef TELL_STDCMD_CLASSB
#define TELL_STDCMD_CLASSB(name, father)                          \
   class name : public father {                                   \
   public:                                                        \
      name(telldata::typeID retype, bool eor);                    \
      int         execute();                                      \
   }
#endif

// Using different type of argument checking - see the comments on top of the file
#ifndef TELL_STDCMD_CLASSC
#define TELL_STDCMD_CLASSC(name)                                  \
   class name : public parsercmd::cmdSTDFUNC {                    \
   public:                                                        \
      name(telldata::typeID retype, bool eor):parsercmd::cmdSTDFUNC(NULL,retype,eor) {};   \
      int         execute();                                      \
      void        undo() {};                                      \
      void        undo_cleanup() {};                              \
      int         argsOK(telldata::argumentQ* amap);              \
      NameList*   callingConv(const telldata::typeMAP*);          \
   }
#endif

// TELL_STDCMD_CLASSD - inherits TELL_STDCMD_CLASSC class type,
// the idea here is to cover the classes
#ifndef TELL_STDCMD_CLASSD
#define TELL_STDCMD_CLASSD(name, father)                          \
   class name : public father {                                   \
   public:                                                        \
      name(telldata::typeID retype, bool eor):father(retype,eor) {}; \
      int         execute();                                      \
      int         argsOK(telldata::argumentQ* amap);              \
      NameList*   callingConv(const telldata::typeMAP*);          \
   }
#endif

//#define TEUNDO_DEBUG_ON
#ifdef TEUNDO_DEBUG_ON
#define TEUNDO_DEBUG(a)  tell_log(console::MT_INFO,a);
#else
#define TEUNDO_DEBUG(a)
#endif

namespace tellstdfunc {

   telldata::TtInt*     getCurrentLayer();
   unsigned             secureLayer();
   void                 secureLayer(unsigned);
   bool                 waitGUInput(int, telldata::operandSTACK *,
                                    std::string name = "",
                                    const CTM trans = CTM(),
                                    int4b stepX = 0,
                                    int4b stepY = 0,
                                    word cols = 0,
                                    word rows = 0
                                   );
   PointVector*         t2tpoints(telldata::TtList *, real);
   telldata::TtList*    make_ttlaylist(laydata::SelectList*);
   telldata::TtList*    make_ttlaylist(laydata::AtticList*);
   laydata::SelectList* get_ttlaylist(telldata::TtList* llist);
   laydata::AtticList*  get_shlaylist(telldata::TtList* llist);
   laydata::DataList*   copyDataList(const laydata::DataList* dlist);
   laydata::SelectList* copySelectList(const laydata::SelectList* dlist);
   void                 cleanSelectList(laydata::SelectList* dlist);
   void                 cleanFadeadList(laydata::SelectList**);
   void                 clean_ttlaylist(telldata::TtList* llist);
   void                 clean_atticlist(laydata::AtticList*, bool destroy = false);
   void                 UpdateLV(unsigned int);
   void                 RefreshGL();
   void                 gridON(byte No, bool status);
   void                 updateLayerDefinitions(laydata::TdtLibDir*, NameList&, int);
   void                 initFuncLib(wxFrame*, wxWindow*);
   laydata::SelectList* filter_selist(const laydata::SelectList*, word mask);
   laydata::AtticList*  replace_str(laydata::AtticList*, std::string);
   bool                 secureLayDef(unsigned);
   void                 createDefaultTDT(std::string, laydata::TdtLibDir*, TpdTime&, bool, parsercmd::UndoQUEUE&, telldata::UNDOPerandQUEUE&);

}


#endif //TPDF_COMMON_H
