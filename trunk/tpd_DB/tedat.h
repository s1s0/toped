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
//           $URL: http://perun/tpd_svn/trunk/toped_wx/tpd_DB/tedat.h $
//  Creation date: Tue Feb 25 2003
//         Author: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision: 213 $
//          $Date: 2005-09-26 21:45:08 +0100 (Mon, 26 Sep 2005) $
//        $Author: skr $
//===========================================================================
#ifndef TEDAT_H
#define TEDAT_H

#include <string>
#include <map>
#include <vector>
#include <GL/gl.h>
#include <GL/glu.h>
#include "tedstd.h"
#include "viewprop.h"

namespace laydata {

//==============================================================================
   /*! Abstract class - the base of all layout objects.\n To optimize the RAM 
       usage having in mind the huge potential number of objects, we must have 
       only the absolute minimum of data fields here or none at all. It was 
       decided not to use the standard C++ containers for the layout objects. 
       The main reason for this is the clipping algorithm and as a consequence 
       the quadTree class as a main data holder. In bref the tdtdata object 
       doesn't know neither the layer it belongs to nor the quadTree it is 
       sorted in. */
   class tdtdata  {
   public:
   //! The default constructor.
      tdtdata(SH_STATUS sel = sh_active) : _status(sel), _next(NULL) {};
      //! Return the overlapping box of the object.
      virtual   DBbox      overlap()  const = 0; // why not DBbox& ????
   //! Move the object relatively using the input CTM
      virtual   validator* move(const CTM&, const SGBitSet* plst = NULL) = 0;
   //! Rotate or flip (transfer the object using input CTM
      virtual   void       transfer(const CTM&) = 0;
   //! Copy the object and move it using the input CTM  
      virtual   tdtdata*   copy(const CTM&) = 0;
   //! Draw the object using OpenGL and the translation matrix stack.
      virtual   void       openGL_draw(ctmstack&, const layprop::DrawProperties&) const = 0;
   //! Draw the temporary objects during copy/move and similar operations
      virtual   void       tmp_draw(const layprop::DrawProperties&, ctmqueue&, SGBitSet* plst = NULL,
                                         bool under_construct=false) const = 0;
   //! Draw the merks on selected points of the object
      virtual   void       draw_select(CTM, const SGBitSet* = NULL) const = 0;
   //! Print an object description on the toped console.
      virtual   void       info(std::ostringstream&) const = 0;
   //! Write the tdtdata object in file.
      virtual   void       write(TEDrecord* const tedfile) const = 0;
   //!
      virtual   bool       point_inside(const TP);
   //! shape cut with the input polygon
      virtual   void       polycut(pointlist&, shapeList**) = 0;
   //! 
      virtual  const pointlist  shape2poly() = 0;
   //! Add a point to the tdtdata object. Used to handle the objects under construction on the screen.
      virtual   void       addpoint(TP ) {};
   //! Removes a point from the tdtdata object. Used to handle the objects under construction on the screen.
      virtual   void       rmpoint() {};
   //! A pointer to the OpenGL object tesselator
      static GLUtriangulatorObj *tessellObj; 
   //! The pointer to the user callback function for openGL polygon tessellation
      static GLvoid polyVertex(GLvoid *);
   //! Returns the next tdtdata object ot NULL if it doesn't exists
      tdtdata*             next() const         {return _next;};
   //! Changes the pointer to the next tdtddata object
      void                 nextis(tdtdata* nxt) {_next = nxt;};
   //! Set the _selected flag in case the object is entirely overlaped by select_in box
      void                 select_inBox(DBbox&, dataList*, bool);
      bool                 unselect(DBbox&, selectDataPair&, bool);
      void                 set_status(SH_STATUS s) {_status = s;};
      SH_STATUS            status() const {return _status;};
      virtual word         numpoints() const = 0;
      virtual             ~tdtdata(){}; 
   protected:
      virtual void        select_points(DBbox&, SGBitSet*) = 0;
      virtual void        unselect_points(DBbox&, SGBitSet*) = 0;
      SH_STATUS            _status;
   //! A pointer to the next tdtdata object
      tdtdata*             _next;
   };

//==============================================================================
   class tdtbox : public tdtdata   {
   public:
                           tdtbox() : tdtdata(), _p1(NULL), _p2(NULL) {};
                              tdtbox(TP* p1, TP* p2) : tdtdata(), _p1(p1), 
                                                         _p2(p2) {normalize();};
                           tdtbox(TEDrecord* const tedfile);
                          ~tdtbox();
      DBbox                overlap() const;
      validator*           move(const CTM&, const SGBitSet* plst = NULL);
      void                 transfer(const CTM&);
      tdtdata*             copy(const CTM&);
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&) const;
      void                 draw_select(CTM,const SGBitSet* = NULL) const;
      void                 tmp_draw(const layprop::DrawProperties&, ctmqueue&,
                             SGBitSet* plst = NULL, bool under_construct=false) const;
      void                 info(std::ostringstream&) const;
      void                 write(TEDrecord* const tedfile) const;
      void                 addpoint(TP p);
      void                 rmpoint();
      word                 numpoints() const {return 4;};
      void                 polycut(pointlist&, shapeList**);
//      tdtdata*             polymerge(tdtdata*);
      const pointlist      shape2poly();
      
   protected:   
      void                 select_points(DBbox&, SGBitSet*);
      void                 unselect_points(DBbox&, SGBitSet*);
   private:
      void                 normalize();
      pointlist&           movePointsSelected(const SGBitSet*, const CTM&, const CTM& = CTM()) const;
      TP*                 _p1;
      TP*                 _p2;
   };

//==============================================================================
   class tdtpoly : public tdtdata   {
   public:
                           tdtpoly():tdtdata() {};
                           tdtpoly(pointlist& plist) : tdtdata(), _plist(plist) {};
                           tdtpoly(TEDrecord* const tedfile);
//                          ~tdtpoly() {};
      DBbox                overlap() const;
      validator*           move(const CTM&, const SGBitSet* plst = NULL);
      void                 transfer(const CTM&);
      tdtdata*             copy(const CTM&);
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&) const;
      void                 tmp_draw(const layprop::DrawProperties&, ctmqueue&,
                              SGBitSet* plst = NULL, bool under_construct=false) const;
      void                 draw_select(CTM,const SGBitSet* = NULL) const;
      void                 info(std::ostringstream&) const;
      void                 write(TEDrecord* const tedfile) const;
      void                 addpoint(TP p) {_plist.push_back(p);};
      void                 rmpoint() {_plist.pop_back();};
      word                 numpoints() const {return _plist.size();};
      bool                 point_inside(const TP);
      void                 polycut(pointlist&, shapeList**);
//      tdtdata*             polymerge(tdtdata*);
      const pointlist      shape2poly() {return _plist;};
//   protected:   
   private:
      void                 select_points(DBbox&, SGBitSet*);
      void                 unselect_points(DBbox&, SGBitSet*);
      pointlist&           movePointsSelected(const SGBitSet*, const CTM&, const CTM& = CTM()) const;
      pointlist            _plist;
   };

//==============================================================================
   class tdtwire : public tdtdata   {
   public:
                           tdtwire(word width) : tdtdata(), _width(width) {};
                           tdtwire(pointlist& plist, word width) : tdtdata(), 
                                                _plist(plist) , _width(width) {};
                           tdtwire(TEDrecord* const tedfile);
//                          ~tdtwire() {};
      DBbox                overlap() const;
      validator*           move(const CTM&, const SGBitSet* plst = NULL);
      void                 transfer(const CTM&);
      tdtdata*             copy(const CTM&);
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&) const;
      void                 tmp_draw(const layprop::DrawProperties&, ctmqueue&,
                              SGBitSet* plst = NULL, bool under_construct=false) const;
      void                 draw_select(CTM,const SGBitSet* = NULL) const;
      void                 info(std::ostringstream&) const;
      void                 write(TEDrecord* const tedfile) const;
      void                 addpoint(TP p) {_plist.push_back(p);};
      void                 rmpoint() {_plist.pop_back();};
      word                 numpoints() const {return _plist.size();};
      bool                 point_inside(const TP);
      void                 polycut(pointlist&, shapeList**){};
//      tdtdata*             polymerge(tdtdata*){return NULL;};
      const pointlist      shape2poly(){return pointlist();};
//   protected:   
   private:
      void                 select_points(DBbox&, SGBitSet*);
      void                 unselect_points(DBbox&, SGBitSet*);
      pointlist&           movePointsSelected(const SGBitSet*, const CTM&, const CTM& = CTM()) const;
      void                 drawSegment(const layprop::DrawProperties&, const TP&,
                              const TP&, const TP&, const TP&, bool, bool) const;
      DBbox*               endPnts(const TP&, const TP&, bool first) const;
      DBbox*               mdlPnts(const TP&, const TP&, const TP&) const;
      float                get_distance(TP p1, TP p2, TP p0);
      pointlist            _plist;
      word                 _width;
   };

//==============================================================================
   class tdtcellref : public tdtdata  {
   public:
                           tdtcellref(refnamepair str, CTM trans) : tdtdata(), 
                                          _structure(str), _translation(trans) {};
                           tdtcellref(TEDrecord* const tedfile);
//                          ~tdtcellref() {};
      DBbox                overlap() const;
      validator*           move(const CTM& trans, const SGBitSet*) {
                                            _translation *= trans; return NULL;};
      void                 transfer(const CTM& trans) {_translation *= trans;};
      tdtdata*             copy(const CTM& trans) {return new tdtcellref(
                                               _structure,_translation*trans);};
//       tdtcellref*          getshapeover(TP);
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&) const;
      void                 tmp_draw(const layprop::DrawProperties&, ctmqueue&,
                              SGBitSet* plst = NULL, bool under_construct=false) const;
      void                 draw_select(CTM,const SGBitSet* = NULL) const;
      void                 info(std::ostringstream&) const;
      void                 write(TEDrecord* const tedfile) const;
      virtual void         ungroup(tdtdesign*, tdtcell*, atticList*);
      std::string          cellname() const {return _structure->first;};
      tdtcell*             structure() const{return _structure->second;};
      word                 numpoints() const {return 1;};
      CTM                  translation() const {return _translation;};
      void                 polycut(pointlist&, shapeList**) {};
//      tdtdata*             polymerge(tdtdata*){return NULL;};
      const pointlist      shape2poly() {return pointlist();};
   protected:
      void                 select_points(DBbox&, SGBitSet*) {};
      void                 unselect_points(DBbox&, SGBitSet*) {return;};
      refnamepair          _structure; // pair (name - cell) pointer
      CTM                  _translation;
   private:
      bool                 ref_visible(ctmstack&, const layprop::DrawProperties&) const;
   };

//==============================================================================
   class tdtcellaref : public tdtcellref  {
   public:
                           tdtcellaref(refnamepair str, CTM trans, int4b stepX, 
                              int4b stepY, word cols, word rows) : 
                              tdtcellref(str, trans), _stepX(stepX), _stepY(stepY), 
                                                       _cols(cols), _rows(rows) {};
                           tdtcellaref(TEDrecord* const tedfile);
//                          ~tdtcellaref() {};
      DBbox                overlap() const;
      DBbox                clear_overlap() const;
      tdtdata*             copy(const CTM& trans) {return new tdtcellaref(
                              _structure,_translation * trans,_stepX, _stepY, 
                                                                _cols, _rows);};
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&) const;
      void                 info(std::ostringstream&) const;
      void                 write(TEDrecord* const tedfile) const;
      void                 ungroup(tdtdesign*, tdtcell*, atticList*);
   private:   
      bool                 aref_visible(ctmstack&, const layprop::DrawProperties&, int*) const;
      int4b               _stepX;
      int4b               _stepY;
      word                _cols;
      word                _rows;
   };
   
//==============================================================================
   class tdttext : public tdtdata  {
   public:
                           tdttext(std::string text, CTM trans);
                           tdttext(TEDrecord* const tedfile);
//                          ~tdttext() {};
      DBbox                overlap() const;
      validator*           move(const CTM& trans, const SGBitSet*) {
                                            _translation *= trans; return NULL;};
      void                 transfer(const CTM& trans)  {_translation *= trans;};
      tdtdata*             copy(const CTM& trans) {return new tdttext(
                                                  _text,_translation * trans);};
      void                 openGL_draw(ctmstack&, const layprop::DrawProperties&) const;
      void                 tmp_draw(const layprop::DrawProperties&, ctmqueue&,
                              SGBitSet* plst = NULL, bool under_construct=false) const;
      void                 draw_select(CTM,const SGBitSet* = NULL) const;
      void                 info(std::ostringstream&) const;
      void                 write(TEDrecord* const tedfile) const;
      word                 numpoints() const {return 1;};
      void                 polycut(pointlist&, shapeList**){};
//      tdtdata*             polymerge(tdtdata*){return NULL;};
      const pointlist      shape2poly() {return pointlist();};
   protected:  
      void                 select_points(DBbox&, SGBitSet*) {return;};
      void                 unselect_points(DBbox&, SGBitSet*) {return;};
   private:   
      std::string         _text;
      CTM                 _translation;
      word                _width; 
   };

//==============================================================================

   class valid_poly : public validator {
   public:
                        valid_poly(const pointlist&);
      laydata::tdtdata* replacement();
      char*             failtype();
   private:
      void              angles();
      void              identical();
      void              normalize();
      void              selfcrossing();
   };
   
   //===========================================================================
   class valid_wire : public validator {
   public:
                        valid_wire(const pointlist&, word);
      laydata::tdtdata* replacement();
      char*             failtype();
   private:
      void              angles();
      void              identical();
      void              normalize();
      void              selfcrossing();
      word              _width;
   };
   //===========================================================================
   int            xangle(TP&, TP&);
   
   void draw_select_marks(DBbox box);
   void draw_select_mark(const TP&);
   void draw_overlapping_box(const DBbox&, const GLushort);
   tdtdata* polymerge(const pointlist&, const pointlist&);
   tdtdata* createValidShape(const pointlist&);
}
   
#endif
