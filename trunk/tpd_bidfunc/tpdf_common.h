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
//    === Two ways for arguments checking of the standard functions ===
//
// First one is to build the argumentmap structure in the constructor and not
// define argsOK and callingConv methods. The argsOK method of the parent
// cmdSTDFUNC will check the arguments.
//
// Second one is not to build argumentmap map (i.e. to leave arguments=NULL)
// and to define argsOK. This one seems more flexible and looks like it takes
// less space - at least a function parameters do not need to be allocated. They
// are anonymous anyway, so their names can not be used.
// After implementation of the tell structures however this way looks like more
// hassle, because of the anonymous arguments. We need to deal "per case" with
// the pain of determining the type of those arguments, and this is not the error
// proof way. Besides in this case additional virtual method callingConv() has to
// be defined for every class.
//
// Bottom line, we are using the first way and the parent argsOK method for
// the majority of the functions. The exception here are functions like echo (the
// only exception ?) that should be flexible enough to get any type of parameters.
// It is defined using the second way.
//
// == Declaration of the standard functions using macro definitions ==
//
// To avoid boring and trivial class declaration here some preprocessor macros
// are used. Hope that in this case there is more convinience than harm using
// macroses
// with the following types of definitions


// TELL_STDCMD_CLASSA      - inherits directly cmdSTDFUNC - no UNDO
// The extra protected constructor is used by the inheritance class
// (if the class is inherited) because of the argument checks
#ifndef TELL_STDCMD_CLASSA
#define TELL_STDCMD_CLASSA(name)                                  \
   class name : public cmdSTDFUNC {                               \
   public:                                                        \
      name(telldata::typeID retype, bool eor);                    \
      int         execute();                                      \
      void        undo() {};                                      \
      void        undo_cleanup() {};                              \
   protected:                                                     \
      name(parsercmd::argumentLIST* al,telldata::typeID retype, bool eor) : \
                                         cmdSTDFUNC(al,retype,eor) {};\
   }
#endif

// TELL_STDCMD_CLASSA_UNDO - inherits directly cmdSTDFUNC - with UNDO
// The extra protected constructor is used by the inheritance class
// (if the class is inherited) because of the argument checks
#ifndef TELL_STDCMD_CLASSA_UNDO
#define TELL_STDCMD_CLASSA_UNDO(name)                             \
   class name : public cmdSTDFUNC {                               \
   public:                                                        \
      name(telldata::typeID retype, bool eor);                    \
      int         execute();                                      \
      void        undo();                                         \
      void        undo_cleanup();                                 \
   protected:                                                     \
      name(parsercmd::argumentLIST* al,telldata::typeID retype, bool eor) : \
                                         cmdSTDFUNC(al,retype, eor) {};\
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

// Using different type of argument checking. Only echo is defined
// of this class - see the comments on top of the file
#ifndef TELL_STDCMD_CLASSC
#define TELL_STDCMD_CLASSC(name)                                  \
   class name : public cmdSTDFUNC {                               \
   public:                                                        \
      name(telldata::typeID retype, bool eor):cmdSTDFUNC(NULL,retype,eor) {};   \
      int         execute();                                      \
      void        undo() {};                                      \
      void        undo_cleanup() {};                              \
      int         argsOK(argumentQ* amap);                        \
      nameList*   callingConv(const telldata::typeMAP*);          \
   }
#endif

//#define TEUNDO_DEBUG_ON
#ifdef TEUNDO_DEBUG_ON
#define TEUNDO_DEBUG(a)  tell_log(console::MT_INFO,a);
#else
#define TEUNDO_DEBUG(a)
#endif

namespace tellstdfunc {

   telldata::ttint*     getCurrentLayer();
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
   pointlist*           t2tpoints(telldata::ttlist *, real);
   telldata::ttlist*    make_ttlaylist(laydata::SelectList*);
   telldata::ttlist*    make_ttlaylist(laydata::AtticList*);
   laydata::SelectList* get_ttlaylist(telldata::ttlist* llist);
   laydata::AtticList*  get_shlaylist(telldata::ttlist* llist);
   void                 clean_ttlaylist(telldata::ttlist* llist);
   void                 clean_atticlist(laydata::AtticList*, bool destroy = false);
   void                 UpdateLV();
   void                 RefreshGL();
   void                 gridON(byte No, bool status);
   void                 updateLayerDefinitions(laydata::TdtLibDir*, nameList&, int);
   void                 initFuncLib(wxFrame*, wxWindow*);
   laydata::SelectList* filter_selist(const laydata::SelectList*, word mask);
   laydata::AtticList*  replace_str(laydata::AtticList*, std::string);
   bool                 secureLayDef(unsigned);
//   void                 makeGdsLays(ExtLayers&);
   void                 createDefaultTDT(std::string, TpdTime&, parsercmd::undoQUEUE&, telldata::UNDOPerandQUEUE&);

}


#endif //TPDF_COMMON_H
