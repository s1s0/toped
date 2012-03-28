/* 
 * File: gdsobject_vrml.h
 * quick hack from gdsobject_pov.h to implement a gds2vrml functionality
 * Author: Armin Taschwer
 * Projekt: gds2vrml
 * Copyright (C) 2010 by Armin Taschwer
 */

// original header
/*
 * File: gdsobject_pov.h
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#ifndef __GDSOBJECT_POV_H__
#define __GDSOBJECT_POV_H__

#include "gdsobjects.h"

#include "gdsobject.h"


class GDSObject_vrml : public GDSObject
{
private:
  bool defined; //is the object (sref,aref,text) already written as VRML-definition?
  bool definedflip; //also a flipped object(sref,aref,text) is already defined?
  bool blocked; //is the object blocked for output?
  /* to get a hierarchical output the definitions of the block have to
	 be done in the top-most cell only!
	 Therefor, before the top-most cell will be processed all outputs are
	 blocked
  */

  int spaces; //number of indentation spaces
  bool transforming; //true if actual object has to be transformed
  //(default = true for all blocks,except the top-cell)

  bool flipped; //if the references (SREF, AREF) are flipped the heigths have to be negated

  void * object_list; //pointer to GDSObjects structure;
public:
  GDSObject_vrml(char *Name);
  ~GDSObject_vrml();

  void DecomposePOVPolygons(FILE *fptr);
  virtual void OutputToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer);
  void OutputPathToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer);
  void OutputPolygonToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer);
  void OutputTextToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer);
  void OutputSRefToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer);
  void OutputARefToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer);

  void SetRefFlipped (bool b) { flipped = b; }
  bool IsRefFlipped () { return flipped; }
  void SetDefined (bool b) { defined = b; }
  void SetFlipDefined (bool b) { definedflip = b; }
  bool IsDefined () { return defined; }
  bool IsFlipDefined () { return definedflip; }
  void UnsetBlocked () { blocked = false; }
  bool IsBlocked () { return blocked; }
  void SetSpaces (int i) { spaces = i; } 
  void PrintSpaces (FILE *fp) { for (int i=0;i<spaces;i++) fprintf(fp," "); }
  void UnsetTransforming() { transforming = false; }
  void SetObjects (void *a) {object_list = a;}

};

#endif // __GDSOBJECT_POV_H__

