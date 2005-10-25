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
//        Created: Fri Jan 24 2003
//         Author: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef  TELLIBIN_H
#define  TELLIBIN_H
#include "../tpd_parser/tellyzer.h"
#include "../tpd_DB/tedstd.h"

/*Two ways for arguments checking of the standard functions*/
// First one is to build the argumentmap structure in the
// constructor and not to define argsOK function. (see the commented examples
// for stdTELLSTATUS, stdGETCW, stdZOOMIN in the tellibin.cpp file). The argsOK
// function of the parent cmdSTDFUNC will check the arguments.
// Second one is not to build argumentmap map (i.e. to leave arguments=NULL)
// and to define argsOK. This is more flexible and looks like takes less space-
// at least a function parameters do not need to be allocated. They are
// anonymous anyway, so can not be used. Besides, internal function like echo
// for example, that should be flexible enough to take any type parameters can
// not be defined easily using argumrntmap structure.
// The contradiction here is the support methods like callingConv, that in the 
// second case have to be defined for each of the internal standard functions
// are boring to code because generally speaking they are not doing any real
// job, they need to be consistent with arsOK and this can be a source of
// stupid and silly mistakes.
// Once the decision is to do something wrong then let's do it right!
// To avoid boring class definition here we going to use the preprocessor
// with two types of definitions 
// TELL_STDCMD_CLASSA - inherit directly cmdSTDFUNC
// TELL_STDCMD_CLASSB - inherit some other class - usually form CLASSA

#ifndef TELL_STDCMD_CLASSA
#define TELL_STDCMD_CLASSA(name)                                  \
   class name : public cmdSTDFUNC {                               \
   public:                                                        \
      name(telldata::typeID retype):cmdSTDFUNC(NULL,retype) {};   \
      int         execute();                                      \
      int         argsOK(argumentQ* amap);                        \
      std::string callingConv();                                  \
   };
#endif

#ifndef TELL_STDCMD_CLASSA_UNDO
#define TELL_STDCMD_CLASSA_UNDO(name)                             \
   class name : public cmdSTDFUNC {                               \
   public:                                                        \
      name(telldata::typeID retype):cmdSTDFUNC(NULL,retype) {};   \
      int         execute();                                      \
      void        undo();                                         \
      void        undo_cleanup();                                 \
      int         argsOK(argumentQ* amap);                        \
      std::string callingConv();                                  \
   };
#endif

#ifndef TELL_STDCMD_CLASSB
#define TELL_STDCMD_CLASSB(name, father)                          \
   class name : public father {                                   \
   public:                                                        \
      name(telldata::typeID retype):father(retype) {};            \
      int         execute();                                      \
      int         argsOK(argumentQ* amap);                        \
      std::string callingConv();                                  \
   };
#endif

namespace tellstdfunc {
   using parsercmd::cmdSTDFUNC;
   using telldata::argumentQ;
   using parsercmd::argumentLIST;
   using parsercmd::argumentTYPE;

   TELL_STDCMD_CLASSA(stdECHO          )
   TELL_STDCMD_CLASSA(stdTELLSTATUS    )
   TELL_STDCMD_CLASSA(stdUNDO          )
   TELL_STDCMD_CLASSA(stdREDRAW        )
   TELL_STDCMD_CLASSA(stdZOOMWIN       )
   TELL_STDCMD_CLASSB(stdZOOMWINb     , stdZOOMWIN   )
   TELL_STDCMD_CLASSA(stdZOOMALL       )
   TELL_STDCMD_CLASSA(TDTread          )       // reset undo buffers
   TELL_STDCMD_CLASSA(TDTsave          )
   TELL_STDCMD_CLASSA(TDTsaveas        )
   TELL_STDCMD_CLASSA(GDSread          )
   TELL_STDCMD_CLASSA(GDSconvert       )
   TELL_STDCMD_CLASSA(GDSreportlay     )
   TELL_STDCMD_CLASSA(GDSclose         )
   TELL_STDCMD_CLASSA(getPOINT         )
   TELL_STDCMD_CLASSA(getPOINTLIST     )
   TELL_STDCMD_CLASSA(stdNEWDESIGN     )       // reset undo buffers
   TELL_STDCMD_CLASSA(stdREPORTSLCTD   )
   TELL_STDCMD_CLASSA(stdREPORTLAY     )
   TELL_STDCMD_CLASSB(stdREPORTLAYc   , stdREPORTLAY  )
   //
   TELL_STDCMD_CLASSA_UNDO(stdLAYPROP     )  //
   TELL_STDCMD_CLASSA_UNDO(stdCOLORDEF    )  //
   TELL_STDCMD_CLASSA_UNDO(stdFILLDEF     )  //
   TELL_STDCMD_CLASSA_UNDO(stdHIDELAYER   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDELAYERS  )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDECELLMARK)  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdHIDETEXTMARK)  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdLOCKLAYER   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdLOCKLAYERS  )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdNEWCELL     )  // * WAITS for ATDB->remove_cell() !!!
   TELL_STDCMD_CLASSA_UNDO(stdOPENCELL    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITPUSH    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITPOP     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITPREV    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdEDITTOP     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdCELLREF     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdCELLAREF    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUSINGLAYER  )  // undo - implemented**
   TELL_STDCMD_CLASSA_UNDO(stdADDBOX      )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdDRAWBOX     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdADDBOXr     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdADDBOXp     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdADDPOLY     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdDRAWPOLY    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdADDWIRE     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdDRAWWIRE    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdADDTEXT     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdGRIDDEF     )  //
   TELL_STDCMD_CLASSA_UNDO(stdGRID        )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSTEP        )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdAUTOPAN     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSHAPEANGLE  )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSELECT      )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSELECTIN    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSELECT_TL   )  //
   TELL_STDCMD_CLASSA_UNDO(stdPNTSELECT   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdSELECTALL   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECT    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECTIN  )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECT_TL )  //
   TELL_STDCMD_CLASSA_UNDO(stdPNTUNSELECT )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNSELECTALL )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdCOPYSEL     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdMOVESEL     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdROTATESEL   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdFLIPXSEL    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdFLIPYSEL    )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdDELETESEL   )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdGROUP       )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(stdUNGROUP     )  // undo - implemented* WAITS for ATDB->remove_cell() !!!
   TELL_STDCMD_CLASSA_UNDO(lgcCUTPOLY     )  // undo - implemented
   TELL_STDCMD_CLASSA_UNDO(lgcMERGE       )  // undo - implemented
   //
   TELL_STDCMD_CLASSB(stdADDBOX_D     , stdADDBOX     )
   TELL_STDCMD_CLASSB(stdDRAWBOX_D    , stdDRAWBOX    )
   TELL_STDCMD_CLASSB(stdADDBOXr_D    , stdADDBOXr    )
   TELL_STDCMD_CLASSB(stdADDBOXp_D    , stdADDBOXp    )
   TELL_STDCMD_CLASSB(stdADDPOLY_D    , stdADDPOLY    )
   TELL_STDCMD_CLASSB(stdDRAWPOLY_D   , stdDRAWPOLY   )
   TELL_STDCMD_CLASSB(stdADDWIRE_D    , stdADDWIRE    )
   TELL_STDCMD_CLASSB(stdDRAWWIRE_D   , stdDRAWWIRE   )
   TELL_STDCMD_CLASSB(stdCOPYSEL_D    , stdCOPYSEL    )
   TELL_STDCMD_CLASSB(stdMOVESEL_D    , stdMOVESEL    )
   TELL_STDCMD_CLASSB(stdSELECT_I     , stdSELECT     )
   TELL_STDCMD_CLASSB(stdPNTSELECT_I  , stdPNTSELECT  )
   TELL_STDCMD_CLASSB(stdUNSELECT_I   , stdUNSELECT   )
   TELL_STDCMD_CLASSB(stdPNTUNSELECT_I, stdPNTUNSELECT)
   TELL_STDCMD_CLASSB(lgcCUTPOLY_I    , lgcCUTPOLY    )
   TELL_STDCMD_CLASSB(stdUSINGLAYER_S , stdUSINGLAYER )  //
       

//   laydata::tdtdesign*  currentDesign();
   telldata::ttint*     CurrentLayer();
   bool                 waitGUInput(int, parsercmd::operandSTACK *);
   pointlist&           t2tpoints(telldata::ttlist *, real);
   telldata::ttlist*    make_ttlaylist(laydata::selectList*);
   telldata::ttlist*    make_ttlaylist(laydata::atticList*);
   laydata::selectList* get_ttlaylist(telldata::ttlist* llist);
   laydata::atticList*  get_shlaylist(telldata::ttlist* llist);
   void                 clean_ttlaylist(telldata::ttlist* llist);
   void                 UpdateLV();
   void                 RefreshGL();
   void                 gridON(byte No, bool status);
}

#endif
/*
---------------------------------------------------------------------------|
---------------------------------------------------------------------------|
          |                   Data Base (layout) sub-type           ||     |
          |------+------+------+------+------+------||--------------|| NB ||
 Operation|  box | poly | wire |  ref | aref | text || point | cell ||    ||
----------+------+------+------+------+------|------||-------+------||----||
----------+------+------+------+------+------|------||-------+------||----||
 select   |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||  OK   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 unselect |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||  OK   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 copy     |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||   -   |  x   || 2  ||
----------+------+------+------+------+------|------||-------+------||----||
 move     |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||  OK   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 delete   |  OK  |  OK  |  OK  |  OK  |  OK  |  OK  ||   -   |  x   || 2  ||
----------+------+------+------+------+------|------||-------+------||----||
 merge    |  v   |   v  |   x  |   -  |   -  |  -   ||   -   |  -   || 1  ||
----------+------+------+------+------+------|------||-------+------||----||
 cut      |  v   |   v  |   x  |   -  |   x  |  -   ||   -   |  -   || 1  ||
----------+------+------+------+------+------|------||-------+------||----||
 group    |  OK  |   OK |   OK |   OK |   OK |  OK  ||   -   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 ungroup  |  -   |   -  |   -  |   OK |   OK |  -   ||   -   |  -   ||    ||
----------+------+------+------+------+------|------||-------+------||----||
 info/edit|  x   |   x  |   x  |   x  |   x  |  x   ||   x   |  -   || 3  ||
----------+------+------+------+------+------|------||-------+------||----||
---------------------------------------------------------------------------|
?mtrx? - a matrix of any object (not cell only as aref in GDSII). Defined
         by: object/stepX/stepY/i/j
1. There will be an auto type conversion here. Poly is the universal,
   but less effective type. Box and wire will be used where is
   possible
2. It is not an operation on the layout type. It should be something
   similar to copy one cell under a new name or to delete a cell. Obviously
   theese operations (functions) should be called differently.
3. The idea of "edit" operation is to have a manual oportunity to change the
   layout objects. It will be extremely usefull for text/ref/aref objects in
   order to change the text/cell. Changing the layer will be possible 
   via this operation.
   
*/
