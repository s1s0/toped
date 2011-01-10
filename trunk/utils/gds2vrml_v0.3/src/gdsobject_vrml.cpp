/* 
 * File: gdsobject_vrml.cpp
 * quick hack from gdsobject_pov.cpp to implement a gds2vrml functionality
 * Author: Armin Taschwer
 * Projekt: gds2vrml
 * Copyright (C) 2010 by Armin Taschwer
 */

// original header
/*
 * File: gdsobject_pov.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the POV-RAY output specific implementation of the GDSObject class.
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


#include <cstdio>
#include <cstring>
#include <cmath>

#include "gds_globals.h" //FIXME - this should be removed
#include "gdsobject_vrml.h"
#include "gds2vrml.h"

#define __M_PI 3.14159265358979323844

GDSObject_vrml::GDSObject_vrml(char *Name) : GDSObject(Name){
  blocked = true;  //if true object is block == output will be ignored
  defined = false; //if true object is already defined with DEF
  definedflip = false; //same as upper but for flipped references
  spaces = 0;
  transforming = true;
  flipped = false; //if true the reference (SREF, AREF) is flipped!
}

GDSObject_vrml::~GDSObject_vrml()
{
}

void GDSObject_vrml::OutputPathToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)
{
  if(!PathItems.empty()){
    float angleX, angleY;
    float h_mult;

    class GDSPath *path;

    for(unsigned long i=0; i<PathItems.size(); i++){
      path = PathItems[i];

	  
      if(path->GetWidth()){ // width must be > 0 !!
	//fprintf(fptr, "mesh2 { vertex_vectors { %d", 8*(path->GetPoints()-1));

	if (i==0 && PolygonItems.size() > 0) {
	  PrintSpaces(fptr); fprintf (fptr, "  ,\n");
	}
	if (i>0 && i<PathItems.size()) {
	  PrintSpaces(fptr); fprintf (fptr, "  ,\n");
	}
	if (IsRefFlipped()) {
	  h_mult = -1.0; 
	  PrintSpaces(fptr); fprintf (fptr, "  ## Flipped REF:path\n");
	} else {
	  h_mult = 1.0;
	}
	PrintSpaces(fptr); fprintf (fptr, "  Shape { ## Path\n"); // plane 1
	PrintSpaces(fptr); fprintf (fptr, "    appearance USE c%s\n",path->GetLayer()->Name); //uses "c<Layer>" for color definition
	PrintSpaces(fptr); fprintf (fptr, "    geometry IndexedFaceSet {\n"); //plane 2
	PrintSpaces(fptr); fprintf (fptr, "      coord Coordinate {\n"); //plane 3
	PrintSpaces(fptr); fprintf (fptr, "        point [\n");

	float BgnExtn; 
	float EndExtn;
	float extn_x, extn_y;
		
	switch(path->GetType()){
	case 1:
	case 2:
	  BgnExtn = path->GetWidth(); /* Width has already been scaled to half */
	  EndExtn = path->GetWidth();
	  break;
	case 4:
	  BgnExtn = path->GetBgnExtn();
	  EndExtn = path->GetEndExtn();
	  break;
	default:
	  BgnExtn = 0.0;
	  EndExtn = 0.0;
	  break;
	}
	for(unsigned int j=0; j<path->GetPoints()-1; j++){
	  float XCoords_j, XCoords_jpone;
	  float YCoords_j, YCoords_jpone;
	  float PathWidth = path->GetWidth();
		  
	  XCoords_j = path->GetXCoords(j);
	  XCoords_jpone = path->GetXCoords(j+1);
	  YCoords_j = path->GetYCoords(j);
	  YCoords_jpone = path->GetYCoords(j+1);
		  
	  angleX = cos(atan2(XCoords_j - XCoords_jpone, YCoords_jpone - YCoords_j));
	  angleY = sin(atan2(XCoords_j - XCoords_jpone, YCoords_jpone - YCoords_j));
		  
	  if(j==0 || j==path->GetPoints()-2){
	    extn_x = EndExtn * angleY;
	    extn_y = EndExtn * angleX;
	  }else{
	    extn_x = 0.0;
	    extn_y = 0.0;
	  }
		  
	  // 1
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_j + PathWidth * angleX + extn_x,
		  YCoords_j + PathWidth * angleY - extn_y,
		  -h_mult*(path->GetHeight() + path->GetThickness())
		  );
	  // 2
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_j - PathWidth * angleX + extn_x,
		  YCoords_j - PathWidth * angleY - extn_y,
		  -h_mult*(path->GetHeight() + path->GetThickness())
		  );
	  // 3
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_j - PathWidth * angleX + extn_x,
		  YCoords_j - PathWidth * angleY - extn_y,
		  -h_mult*path->GetHeight()
		  );
	  // 4
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_j + PathWidth * angleX + extn_x,
		  YCoords_j + PathWidth * angleY - extn_y,
		  -h_mult*path->GetHeight()
		  );
	  
	  // 5
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_jpone + PathWidth * angleX - extn_x,
		  YCoords_jpone + PathWidth * angleY - extn_y,
		  -h_mult*(path->GetHeight() + path->GetThickness())
		  );
		  		  
	  // 6
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_jpone - PathWidth * angleX - extn_x,
		  YCoords_jpone - PathWidth * angleY - extn_y,
		  -h_mult*(path->GetHeight() + path->GetThickness())
		  );
	  // 7
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f,\n", 
		  XCoords_jpone - PathWidth * angleX - extn_x,
		  YCoords_jpone - PathWidth * angleY - extn_y,
		  -h_mult*path->GetHeight()
		  );
	  // 8
	  PrintSpaces(fptr); 
	  fprintf(fptr, "          %.2f %.2f %.2f\n", 
		  XCoords_jpone + PathWidth * angleX - extn_x,
		  YCoords_jpone + PathWidth * angleY - extn_y,
		  -h_mult*path->GetHeight()
		  );
		  
	}
	unsigned int PathPoints = path->GetPoints();
	PrintSpaces(fptr); fprintf (fptr, "        ] ## end point\n");
	PrintSpaces(fptr); fprintf (fptr, "      } ## end coord\n"); // end plane 3
	PrintSpaces(fptr); fprintf (fptr, "      coordIndex [\n");
	for(unsigned int j=0; j<PathPoints-1; j++){
	  // print ,faces now
	  //int vertexindex[36] = {0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 6, 3, 6, 7, 1, 3, 7, 1, 5, 7, 0, 2, 4, 2, 4, 6};
	  /* "normal"
	    <------><------> 3, 2, 1, 0, 3, -1,
	    <------><------> 0, 1, 5, 4, 0, -1,
	    <------><------> 4, 5, 6, 7, 4, -1,
	    <------><------> 1, 2, 6, 5, 1, -1,
	    <------><------> 6, 2, 3, 7, 6, -1,
	    <------><------> 3, 0, 4, 7, 3, -1
	  */
	  /* "flipped"
	    <------><------> 3, 0, 1, 2, 3, -1,
	    <------><------> 0, 4, 5, 1, 0, -1,
	    <------><------> 4, 7, 6, 5, 4, -1,
	    <------><------> 1, 5, 6, 2, 1, -1,
	    <------><------> 6, 7, 3, 2, 6, -1,
	    <------><------> 3, 7, 4, 0, 3, -1
	  */
	  if (IsRefFlipped()) {
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 3+8*j, 0+8*j, 1+8*j, 2+8*j, 3+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 0+8*j, 4+8*j, 5+8*j, 1+8*j, 0+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 4+8*j, 7+8*j, 6+8*j, 5+8*j, 4+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 1+8*j, 5+8*j, 6+8*j, 2+8*j, 1+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 6+8*j, 7+8*j, 3+8*j, 2+8*j, 6+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d",       3+8*j, 7+8*j, 4+8*j, 0+8*j, 3+8*j);
	  } else {
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 3+8*j, 2+8*j, 1+8*j, 0+8*j, 3+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 0+8*j, 1+8*j, 5+8*j, 4+8*j, 0+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 4+8*j, 5+8*j, 6+8*j, 7+8*j, 4+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 1+8*j, 2+8*j, 6+8*j, 5+8*j, 1+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d,-1,\n", 6+8*j, 2+8*j, 3+8*j, 7+8*j, 6+8*j);
	    PrintSpaces(fptr); fprintf(fptr, "        %d,%d,%d,%d,%d",       3+8*j, 0+8*j, 4+8*j, 7+8*j, 3+8*j);
	  }
	  if (j < PathPoints-2) {
	    fprintf (fptr,",-1,\n");
	  } else {
	    fprintf (fptr,"\n");
	  }
	}
	PrintSpaces(fptr); fprintf (fptr, "      ] ## end coordIndex\n");
	//if(!path->Colour.Metal){
	//	fprintf(fptr, "pigment{rgbf <%.2f, %.2f, %.2f, %.2f>} ", path->Colour.R, path->Colour.G, path->Colour.B, path->Colour.F);
	//}else{
	//	fprintf(fptr, "pigment{rgbf <%.2f, %.2f, %.2f, %.2f>} finish{F_MetalA} ", path->Colour.R, path->Colour.G, path->Colour.B, path->Colour.F);
	//}
	/* [at]
	   fprintf(fptr, "texture{t%s}",path->GetLayer()->Name);
	   fprintf(fptr, "}\n");
	*/
	PrintSpaces(fptr); fprintf (fptr, "    } ## end geometry indexedfaceset\n"); //end plane 2
	PrintSpaces(fptr); fprintf (fptr, "  } ## end shape path\n"); //end plane 1
      }
    }
  }
}

void GDSObject_vrml::OutputPolygonToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)
{
  if(!PolygonItems.empty()){
    if(decompose){
      DecomposePOVPolygons(fptr);
    }else{
      class GDSPolygon *polygon;
      double h_mult;

      for(unsigned long i=0; i<PolygonItems.size(); i++){
	polygon = PolygonItems[i];


	if (i>0 && i<PolygonItems.size()) {
	  PrintSpaces(fptr); fprintf (fptr, "  ,\n");
	}
	if (IsRefFlipped()) {
	  h_mult = -1.0;
	  PrintSpaces(fptr); fprintf (fptr, "  ## Flipped REF:polygon\n");
	} else {
	  h_mult = 1.0;
	}
	PrintSpaces(fptr); fprintf (fptr, "  Transform { ## Polygon\n"); //plane 1
	PrintSpaces(fptr); fprintf (fptr, "  children\n");
	PrintSpaces(fptr); fprintf (fptr, "  Shape {\n"); //plane 2
	PrintSpaces(fptr); fprintf (fptr, "    appearance USE c%s\n",polygon->GetLayer()->Name); //uses "  c<Layer>" for color definition
	PrintSpaces(fptr); fprintf (fptr, "    geometry Extrusion {\n"); //plane 3
	PrintSpaces(fptr); fprintf (fptr, "      crossSection [\n");
	PrintSpaces(fptr); fprintf (fptr, "        ");
	
	for(unsigned int j=0; j<polygon->GetPoints(); j++){
	  //VRML97: points are oriented in the Y-Plane = 0
	  fprintf(fptr, "%.2f %.2f", polygon->GetXCoords(j), polygon->GetYCoords(j));
	  if (j+1<polygon->GetPoints()) {
	    fprintf (fptr, ", ");
	  }
	}
	fprintf (fptr, "\n");

	PrintSpaces(fptr); fprintf (fptr, "      ]\n"); //close crossSection
	double hs = h_mult* polygon->GetHeight();
	double ht = h_mult* (polygon->GetHeight()+polygon->GetThickness());

	double eps = 0.01;
	if (hs > -eps && hs < eps) hs = 0.0;
	if (ht > -eps && ht < eps) ht = 0.0;

	//BUG in view3dscene?! Bug in specification?! missinterpretation from specification?!
	// [ 0 -0.5 0, 0 -1 0 ] causes [not-wanted] flipping since the orientation of the spine 
	//   vector is wrong (in reality this is some dependent of the viewer implementation)
        // Workaround: [ 0 -1 0, 0 -0.5 0 ] // orientation of the vector has to be the same 
	//   as in the non-flipped view
	if (IsRefFlipped() ) {
	  double tp = hs;
	  hs = ht;
	  ht = tp;
	}
	//VRML97: points are in plane Y=0 --> thererfor height is defined by Y-Axis
	PrintSpaces(fptr); fprintf (fptr, "      spine [0 %.2f 0, 0 %.2f 0]\n", hs, ht);
	PrintSpaces(fptr); fprintf (fptr, "      solid TRUE\n");
	PrintSpaces(fptr); fprintf (fptr, "      ccw FALSE\n");

	PrintSpaces(fptr); fprintf(fptr, "    } ## end geometry extrusion\n"); //close plane 3
	PrintSpaces(fptr); fprintf(fptr, "  } ## end Shape polygon\n"); //close plane 2
	PrintSpaces(fptr); fprintf(fptr, "  rotation 1 0 0 -%.10f\n", __M_PI/2.0 ); //90° rotation
	PrintSpaces(fptr); fprintf(fptr, "  } ## end Transform polygon\n"); //close plane 1
      }
    }
  }
}

void GDSObject_vrml::OutputTextToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)
{
  if(!TextItems.empty()){
    class GDSText *text;
    //for (vector<class GDSText>::const_iterator text=TextItems.begin(); text!=TextItems.end(); ++text){
    for (unsigned int i=0; i<TextItems.size(); i++){
      text = TextItems[i];
      if(text->GetString()){
	if(Font){
	  fprintf(fptr, "text{ttf \"%s\" \"%s\" 0.2, 0 ", Font, text->GetString());
	}else{
	  fprintf(fptr, "text{ttf \"crystal.ttf\" \"%s\" 0.2, 0 ", text->GetString());
	}
	//fprintf(fptr, "texture{pigment{rgbf <%.2f,%.2f,%.2f,%.2f>}} ", text->Colour.R, text->Colour.G, text->Colour.B, text->Colour.F);
	fprintf(fptr, "texture{t%s}",text->GetLayer()->Name);
	if(text->GetMag()!=1.0){
	  fprintf(fptr, "scale <%.2f,%.2f,1> ", text->GetMag(), text->GetMag());
	}
	if(text->GetFlipped()){
	  fprintf(fptr, "scale <1,-1,1> ");
	}
	fprintf(fptr, "translate <%.2f,%.2f,%.2f> ", text->GetX(), text->GetY(), -text->GetZ());
	if(text->GetRY()){
	  fprintf(fptr, "Rotate_Around_Trans(<0,0,%.2f>,<%.2f,%.2f,%.2f>)", -text->GetRY(), text->GetX(), text->GetY(), -text->GetZ());
	}
	float htrans = 0.0, vtrans = 0.0;
	switch(text->GetHJust()){
	case 0:
	  htrans = -0.5*strlen(text->GetString());
	  break;
	case 1:
	  htrans = -0.25*strlen(text->GetString());
	  break;
	case 2:
	  htrans = 0;
	  break;
	}
	switch(text->GetVJust()){
	case 0:
	  vtrans = 0.0;
	  break;
	case 1:
	  vtrans = -0.5;
	  break;
	case 2:
	  vtrans = -1.0;
	  break;
	}
	if(htrans || vtrans){
	  if(text->GetRY()){
	    fprintf(fptr, "translate <%.2f,%.2f,0> ", vtrans, htrans);
	  }else{
	    fprintf(fptr, "translate <%.2f,%.2f,0> ", htrans, vtrans);
	  }
	}
	fprintf(fptr, "}\n");
      }
    }
  }
}

// structure refernce
// one hierarchy level can contain several references
// --> they are represented by a linked list
// --> simple layout-vector: X->X->X->..->X
void GDSObject_vrml::OutputSRefToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)
{
  if(FirstSRef){

    SRefElement *sref = FirstSRef;

    if (!PathItems.empty() || !PolygonItems.empty()) {
      PrintSpaces(fptr); fprintf (fptr, "  ,\n");
    }

    do {
      GDSObject * obj = Objects->GetObjectRef(sref->Name);
      GDSObject_vrml * vrmlo = (GDSObject_vrml*)obj;

      vrmlo->UnsetBlocked();
      vrmlo->SetSpaces(spaces+2);

      vrmlo->SetRefFlipped(sref->Flipped);

      //flipping *before* rotation !! --> flipping is only allowed on X
      if (sref->Flipped) {
	SetSpaces(spaces+2);
	PrintSpaces(fptr); fprintf(fptr, "Transform {   ## SREF --> flipped=TRUE\n");
	PrintSpaces(fptr); fprintf(fptr, "  children [\n"); //plane 3
	vrmlo->SetSpaces(spaces+4);
      }
      //starts the recursive output (remember, the original (as defined in libgds) recursive
      //output is blocked by the variable "blocked")
      vrmlo->OutputToFile (fptr, Objects, Font, offx, offy, objectid, firstlayer);
      // Attention: not closed Transform { //plane 1

      if (sref->Flipped) {
	PrintSpaces(fptr); fprintf(fptr, "    rotation 1 0 0 %.10f\n", __M_PI);
	PrintSpaces(fptr); fprintf(fptr, "    } ## SREF --> flipped=TRUE\n"); //plane 1 of vrmlo->OutputToFile
	PrintSpaces(fptr); fprintf(fptr, "  ]\n"); //plane 3
	SetSpaces(spaces-2);
      }


      PrintSpaces(fptr); fprintf(fptr, "    translation %.2f %.2f 0\n",sref->X, sref->Y);

      if(sref->Rotate.Y){
	PrintSpaces(fptr); fprintf(fptr, "    rotation 0 0 1 %.10f\n", -sref->Rotate.Y*__M_PI/180.0);
      }

      //in VRML only positiv scale factors allowed
      if(sref->Mag!=1.0 && sref->Mag >= 0.0){
	PrintSpaces(fptr); fprintf(fptr, "    scale %.2f %.2f 1\n", sref->Mag, sref->Mag);
      }
      //PrintSpaces(fptr); fprintf(fptr, "    center %.2f %.2f 0 \n", -sref->X, -sref->Y);

      //usually this closes the Transform opened by vrmlo->OutputToFile
      PrintSpaces(fptr); fprintf(fptr, "  } ## SREF --> change sref pointer\n"); //plane 1 of vrmlo->OutputToFile 

      if (sref->Next) {
	PrintSpaces(fptr); fprintf (fptr, "  , ## it is going on\n");
      }

      sref = sref->Next;
    } while (sref);
  }
}

// array references:
// each hierarchy level can contain a punch of array references
// each AREF contains the parameter row and column
// AREFs are again represented as a linked list: X->X->X->...

// possible parameters: rotation, flipping (against X-axis), magnification
// the operations are always flipping before rotation

void GDSObject_vrml::OutputARefToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)
{
  if(FirstARef){
    ARefElement *aref = FirstARef;

    /*
    ARefElement dummyaref;
    dummyaref.Next = FirstARef;
    ARefElement *aref = &dummyaref;
    */
    float dx, dy;
    float ax,ay;

    do {
      if (aref->Columns && aref->Rows) {

	ax = (float)(aref->X2 - aref->X1);
	ay = (float)(aref->Y2 - aref->Y1);
	//printf ("\nax = %f, ay = %f\n\n",ax,ay);
	dx = sqrtf(ax*ax + ay*ay) / (float) aref->Columns;
	ax = (float)(aref->X3 - aref->X1);
	ay = (float)(aref->Y3 - aref->Y1);
	//printf ("\nax = %f, ay = %f\n\n",ax,ay);
	dy = sqrtf(ax*ax + ay*ay) / (float) aref->Rows;

	
	GDSObject * obj = Objects->GetObjectRef(aref->Name);
	GDSObject_vrml * vrmlo = (GDSObject_vrml*)obj;

	vrmlo->UnsetBlocked();
	vrmlo->SetSpaces(spaces+12);
	
	vrmlo->SetRefFlipped(aref->Flipped); //important for subcells

	SetSpaces(spaces+4);
	PrintSpaces(fptr); fprintf(fptr, "Transform {   ## AREF-Top-Object!!!\n");
	PrintSpaces(fptr); fprintf(fptr, "  children [\n");
	

	//flipping *before* rotation !!
	if (aref->Flipped) {
	  SetSpaces(spaces+2);
	  PrintSpaces(fptr); fprintf(fptr, "Transform {   ## Objects are flipped!!!\n");
	  PrintSpaces(fptr); fprintf(fptr, "  children [\n");
	  vrmlo->SetSpaces(spaces+4);
	}
	SetSpaces (spaces+4); 

	for (int x = 0; x < aref->Columns; x++) {
	  for (int y = 0; y < aref->Rows; y++) {

	    PrintSpaces(fptr); fprintf(fptr, "Transform {   ## AREF Object [ %d, %d]\n",x,y); //plane 0
	    PrintSpaces(fptr); fprintf(fptr, "  children [\n");
	    //vrmlo->SetSpaces (spaces + 4);
	    
	    //starts the recursive output (remember, the original (as defined in libgds) recursive
	    //output is blocked by the variable "blocked")
	    vrmlo->OutputToFile (fptr, Objects, Font, offx, offy, objectid, firstlayer);
	    // Attention: not closed Transform { //plane 1

	    //vrmlo->SetSpaces (spaces - 4);
	    PrintSpaces(fptr); fprintf(fptr, "    } ## AREF vrmlo->Object closed \n"); //closes Transformation opened by upper vrmlo->OutputToFile
	    PrintSpaces(fptr); fprintf(fptr, "  ]\n"); //closes children in front of vrmlo->OutputToFile
    
	    PrintSpaces(fptr); fprintf(fptr, "  translation %.2f %.2f 0\n", dx*x, dy*y);
	    PrintSpaces(fptr); fprintf(fptr, "}   ## AREF Object closed [ %d, %d]\n", x,y); // plane 0

	    if (x+1 < aref->Columns || y+1 < aref->Rows) {
	      PrintSpaces(fptr); fprintf(fptr, ",\n");
	    }
	  }
	}
	SetSpaces (spaces-4); 

	if (aref->Flipped) {
	  PrintSpaces(fptr); fprintf(fptr, "  ]\n");
	  PrintSpaces(fptr); fprintf(fptr, "  rotation 1 0 0 %.10f\n", __M_PI);
	  PrintSpaces(fptr); fprintf(fptr, "}\n");
	  SetSpaces(spaces-4);
	}

	PrintSpaces(fptr); fprintf(fptr, "  ] ## AREF-top end children\n");
	PrintSpaces(fptr); fprintf(fptr, "  translation %.2f %.2f 0\n",aref->X1, aref->Y1);
	
	if(aref->Rotate.Y){
	  PrintSpaces(fptr); fprintf(fptr, "  rotation 0 0 1 %.10f\n", -aref->Rotate.Y*__M_PI/180.0);
	}
	
	//in VRML only positiv scale factors allowed
	if(aref->Mag!=1.0 && aref->Mag >= 0.0){
	  PrintSpaces(fptr); fprintf(fptr, "  scale %.2f %.2f 1\n", aref->Mag, aref->Mag);
	}
	//PrintSpaces(fptr); fprintf(fptr, "    center %.2f %.2f 0 \n", -aref->X, -aref->Y);
	
	
	PrintSpaces(fptr); fprintf(fptr, "} ## AREF --> before aref->Next\n"); //closes Transformation opened by OutputToFile()
	
	if (aref->Next) {
	  PrintSpaces(fptr); fprintf (fptr, "  ,\n");
	}
	SetSpaces (spaces-4);

      } else {
	PrintSpaces(fptr); fprintf(fptr, "## WARNING: improper AREF defined: col: %d, row: %d", 
				   aref->Columns, aref->Rows);
      }
      aref = aref->Next;
    } while (aref);


    /* */
    /*
    while(aref->Next){
      aref = aref->Next;
      if(aref->Rotate.Y == 90.0 || aref->Rotate.Y == -90.0){
	if(aref->Columns && aref->Rows && (aref->X3 - aref->X1) && (aref->Y2 - aref->Y1)){
	  dx = (float)(aref->X3 - aref->X1) / (float)aref->Columns;
	  dy = (float)(aref->Y2 - aref->Y1) / (float)aref->Rows;

	  fprintf(fptr, "#declare dx = %.2f;\n", dx);
	  fprintf(fptr, "#declare dy = %.2f;\n", dy);

	  fprintf(fptr, "#declare colcount = 0;\n");
	  fprintf(fptr, "#declare cols = %d;\n", aref->Columns);
	  fprintf(fptr, "#declare rows = %d;\n", aref->Rows);
	  fprintf(fptr, "#while (colcount < cols)\n");
	  fprintf(fptr, "\t#declare rowcount = 0;");
	  fprintf(fptr, "\t#while (rowcount < rows)\n");
	  fprintf(fptr, "\t\tobject{str_%s ", aref->Name);
	  if(aref->Mag!=1.0){
	    fprintf(fptr, "scale <%.2f,%.2f,1> ", aref->Mag, aref->Mag);
	  }
	  if(aref->Flipped){
	    fprintf(fptr, "scale <1,-1,1> ");
	  }
	  fprintf(fptr, "translate <%.2f+dx*colcount,%.2f+dy*rowcount,0>", aref->X1, aref->Y1);
	  if(aref->Rotate.Y){
	    fprintf(fptr, " Rotate_Around_Trans(<0,0,%.2f>,<%.2f+dx*colcount,%.2f+dy*rowcount,0>)", -aref->Rotate.Y, aref->X1, aref->Y1);
	  }
	  fprintf(fptr, "}\n");

	  fprintf(fptr, "\t\t#declare rowcount = rowcount + 1;\n");
	  fprintf(fptr, "\t#end\n");
	  fprintf(fptr, "\t#declare colcount = colcount + 1;\n");
	  fprintf(fptr, "#end\n");
	}
      }else{
	if(aref->Columns && aref->Rows && (aref->X2 - aref->X1) && (aref->Y3 - aref->Y1)){
	  dx = (float)(aref->X2 - aref->X1) / (float)aref->Columns;
	  dy = (float)(aref->Y3 - aref->Y1) / (float)aref->Rows;

	  fprintf(fptr, "#declare dx = %.2f;\n", dx);
	  fprintf(fptr, "#declare dy = %.2f;\n", dy);

	  fprintf(fptr, "#declare colcount = 0;\n");
	  fprintf(fptr, "#declare cols = %d;\n", aref->Columns);
	  fprintf(fptr, "#declare rows = %d;\n", aref->Rows);
	  fprintf(fptr, "#while (colcount < cols)\n");
	  fprintf(fptr, "\t#declare rowcount = 0;");
	  fprintf(fptr, "\t#while (rowcount < rows)\n");
	  fprintf(fptr, "\t\tobject{str_%s ", aref->Name);
	  if(aref->Flipped){
	    fprintf(fptr, "scale <1,-1,1> ");
	  }
	  fprintf(fptr, "translate <%.2f+dx*colcount,%.2f+dy*rowcount,0>", aref->X1, aref->Y1);
	  if(aref->Rotate.Y){
	    fprintf(fptr, " Rotate_Around_Trans(<0,0,%.2f>,<%.2f+dx*colcount,%.2f+dy*rowcount,0>)", -aref->Rotate.Y, aref->X1, aref->Y1);
	  }
	  fprintf(fptr, "}\n");

	  fprintf(fptr, "\t\t#declare rowcount = rowcount + 1;\n");
	  fprintf(fptr, "\t#end\n");
	  fprintf(fptr, "\t#declare colcount = colcount + 1;\n");
	  fprintf(fptr, "#end\n");
	}
      }
      } */


  }    
}


void GDSObject_vrml::OutputToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)
{
  if (IsBlocked()) {
    return;
  }

  //if(fptr && !IsOutput){
  if(fptr){
    //fprintf(fptr, "#declare str_%s = union {\n", Name);
	
    if (( !IsRefFlipped() && IsDefined()) || 
	(  IsRefFlipped() && IsFlipDefined()) ) {

      // following hierarchy level is closed by the OutputXRef methods!
      PrintSpaces(fptr); fprintf (fptr, "Transform {  ## OutputToFile --> defined = TRUE\n");
      PrintSpaces(fptr); fprintf (fptr, "  children [\n"); // plane 1
      if (IsRefFlipped () ) {
	PrintSpaces(fptr); fprintf (fptr, "    USE sf_%s\n", Name);
      } else {
	PrintSpaces(fptr); fprintf (fptr, "    USE s_%s\n", Name);
      }
    } else {

      // following hierarchy level is closed by the OutputXRef methods!
      PrintSpaces(fptr); fprintf (fptr, "Transform {  ## OutputToFile --> defined = FALSE\n");
      PrintSpaces(fptr); fprintf (fptr, "  children [\n"); //plane 1
      if (IsRefFlipped()) {
	SetFlipDefined(true);
	PrintSpaces(fptr); fprintf (fptr, "  DEF sf_%s Transform { ## OutputToFile --> flipped = TRUE\n", Name); 
      } else {
	SetDefined(true);
	PrintSpaces(fptr); fprintf (fptr, "  DEF s_%s Transform { ## OutputToFile --> flipped=FALSE\n", Name);
      }
      PrintSpaces(fptr); fprintf (fptr, "    children [\n"); //plane 2
  
      SetSpaces(spaces+2);
      OutputPolygonToFile(fptr, Objects, Font, offx, offy, objectid, firstlayer);
      OutputPathToFile(fptr, Objects, Font, offx, offy, objectid, firstlayer);
      OutputSRefToFile(fptr, Objects, Font, offx, offy, objectid, firstlayer);
      OutputTextToFile(fptr, Objects, Font, offx, offy, objectid, firstlayer);
      OutputARefToFile(fptr, Objects, Font, offx, offy, objectid, firstlayer);
      SetSpaces(spaces-2);
      PrintSpaces(fptr); fprintf(fptr, "    ]\n"); //plane 2
      PrintSpaces(fptr); fprintf(fptr, "  } ## OutputToFile\n");
    }
    PrintSpaces(fptr); fprintf(fptr, "  ] ## end top children OutputToFile\n"); //plane1
    if (!transforming) { // performs only on topmost object and is closing plane 1
      PrintSpaces(fptr); fprintf(fptr, "} ## OutputToFile --> transforming = FALSE\n");
    }
    //else: closing of Transform will always be performed in parent reference object
  }
  //IsOutput = true;
}

void GDSObject_vrml::DecomposePOVPolygons(FILE *fptr)
{
  unsigned long faceindex;

  if(!PolygonItems.empty() && !IsBlocked()){
    class GDSPolygon *polygon;

    faceindex = 0;

    for(unsigned long i=0; i<PolygonItems.size(); i++){
      polygon = PolygonItems[i];

      /* Output vertices */
      fprintf(fptr, "mesh2 { vertex_vectors { %d", 2*(polygon->GetPoints()-1));
      for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	fprintf(fptr, ",<%.2f,%.2f,%.2f>", 
		polygon->GetXCoords(j),
		polygon->GetYCoords(j),
		polygon->GetHeight() + polygon->GetThickness()
		);
      }
      for(unsigned int j=0; j<polygon->GetPoints()-1; j++){

	fprintf(fptr, ",<%.2f,%.2f,%.2f>", 
		polygon->GetXCoords(j),
		polygon->GetYCoords(j),
		polygon->GetHeight()
		);
      }

      /*
       * Calculate angles between adjacent vertices.
       * We do this to tell what "type" of polygon we are dealing with.
       * Specifically, where any change of convex/concave takes place.
       * Because the first and last points are identical we do not need
       * to worry about the final vertex angle (it is already calculated 
       * in the 0th vertex).
       */

      Point pA, pB;

      pA.X = polygon->GetXCoords(0)-polygon->GetXCoords(polygon->GetPoints()-2);
      pA.Y = polygon->GetYCoords(0)-polygon->GetYCoords(polygon->GetPoints()-2);
      pB.X = polygon->GetXCoords(1)-polygon->GetXCoords(0);
      pB.Y = polygon->GetYCoords(1)-polygon->GetYCoords(0);

      float theta1;
      float theta2;

      theta1 = atan2(pA.X, pA.Y);
      theta2 = atan2(pB.X, pB.Y);
      polygon->SetAngleCoords(0, theta1 - theta2);

      for(unsigned int j=1; j<polygon->GetPoints()-1; j++){
	pA.X = polygon->GetXCoords(j)-polygon->GetXCoords(j-1);
	pA.Y = polygon->GetYCoords(j)-polygon->GetYCoords(j-1);

	pB.X = polygon->GetXCoords(j+1)-polygon->GetXCoords(j);
	pB.Y = polygon->GetYCoords(j+1)-polygon->GetYCoords(j);

	theta1 = atan2(pA.X, pA.Y);
	theta2 = atan2(pB.X, pB.Y);

	polygon->SetAngleCoords(j, theta1 - theta2);
      }
      int positives = 0;
      int negatives = 0;
      for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	polygon->SetAngleCoords(j, (float)asin(sin((double)polygon->GetAngleCoords(j))));
	if(polygon->GetAngleCoords(j)>=0){
	  positives++;
	}else{
	  negatives++;
	}
      }

      int bendindex1;

      if(!positives || !negatives){
	fprintf(fptr, "} face_indices { %d", 2*(polygon->GetPoints()-3) + 2*(polygon->GetPoints()-1));
	for(unsigned int j=1; j<polygon->GetPoints()-2; j++){
	  fprintf(fptr, ",<%d,%d,%d>",0,j,j+1);
	  fprintf(fptr, ",<%d,%d,%d>",polygon->GetPoints()-1,j+polygon->GetPoints()-1,j+polygon->GetPoints()-1+1);
	}
      }else if(positives==1 && negatives>1){
	bendindex1 = -1;
	fprintf(fptr, "} face_indices { %d", 2*(polygon->GetPoints()-2) + 2*(polygon->GetPoints()-1));
	for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	  if(polygon->GetAngleCoords(j)>=0){
	    bendindex1 = (int)j;
	    break;
	  }
	}
	for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	  if((int)j!=bendindex1){
	    fprintf(fptr, ",<%d,%d,%d>", bendindex1, j, j+1);
	    fprintf(fptr, ",<%d,%d,%d>", bendindex1+polygon->GetPoints()-1, j+polygon->GetPoints()-1, (j+polygon->GetPoints()>=2*(polygon->GetPoints()-1))?j+1:j+polygon->GetPoints());
	  }
	}
      }else if(negatives==1 && positives>1){
	bendindex1 = -1;
	fprintf(fptr, "} face_indices { %d", 2*(polygon->GetPoints()-2) + 2*(polygon->GetPoints()-1));
	for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	  if(polygon->GetAngleCoords(j)<0){
	    bendindex1 = j;
	    break;
	  }
	}
	for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	  if((int)j!=bendindex1){
	    fprintf(fptr, ",<%d,%d,%d>", bendindex1, j, j+1);
	    fprintf(fptr, ",<%d,%d,%d>", bendindex1+polygon->GetPoints()-1, j+polygon->GetPoints()-1, (j+polygon->GetPoints()>=2*(polygon->GetPoints()-1))?j+1:j+polygon->GetPoints());
	  }
	}
	/*}else if(negatives==2 && positives>2){
	  bendindex1 = -1;
	  bendindex2 = -1;

	  fprintf(fptr, "} face_indices{%d", 2*(polygon->GetPoints()-2) + 2*(polygon->GetPoints()-1));
	  for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	  if(polygon->GetAngleCoords(j)<0 && bendindex1 == -1){
	  bendindex1 = j;
	  }else if(polygon->GetAngleCoords(j)<0){
	  bendindex2 = j;
	  break;
	  }
	  }
	  for(unsigned int j=bendindex1; j<=bendindex2; j++){
	  fprintf(fptr, "<%d,%d,%d>",);
	  }
	  for(unsigned int j=0; j<bendindex1; j++){
	  fprintf(fptr, ",<%d,%d,%d>",);
	  }
	*/
      }else{
	fprintf(fptr, "} face_indices { %d", 2*(polygon->GetPoints()-1));
      }

      /* Always output the vertical faces regardless of whether we fill in the horizontal faces or not */
      for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	fprintf(fptr, ",<%d,%d,%d>",j,j+polygon->GetPoints()-1,(j+polygon->GetPoints()>=2*(polygon->GetPoints()-1))?j:j+polygon->GetPoints());
	fprintf(fptr, ",<%d,%d,%d>",j,j+1,(j+polygon->GetPoints()>=2*(polygon->GetPoints()-1))?j:j+polygon->GetPoints());
      }
      fprintf(fptr,"}");
      fprintf(fptr, "texture{t%s}", polygon->GetLayer()->Name);
      fprintf(fptr, "}\n");

      for(unsigned int j=0; j<polygon->GetPoints()-1; j++){
	if(polygon->GetAngleCoords(j)>=0){
	  fprintf(fptr,"text{ttf \"crystal.ttf\" \"%d+\" 0.2, 0 ", j);
	}else{
	  fprintf(fptr,"text{ttf \"crystal.ttf\" \"%d-\" 0.2, 0 ", j);
	}
	fprintf(fptr, " scale <1.5,1.5,1.5>");
	fprintf(fptr, " translate <%.2f,%.2f,%.2f> texture{pigment{rgb <1,1,1>}}}\n", \
		polygon->GetXCoords(j), \
		polygon->GetYCoords(j), polygon->GetHeight() - 1);
      }

      printf("+ve %d, -ve %d\n\n", positives, negatives);
    }

  }
}

