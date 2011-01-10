/* 
 * File: gdsparse_vrml.cpp
 * quick hack from gdsparse_pov.cpp to implement a gds2vrml functionality
 * Author: Armin Taschwer
 * Projekt: gds2vrml
 * Copyright (C) 2010 by Armin Taschwer
 */

// original header
/*
 * File: gdsparse_pov.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the POV-RAY output specific implementation of the GDSParse class.
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
//#include <cstdlib>
//#include <cmath>

#include <stdlib.h>

#include "config_cfg.h"
#include "process_cfg.h"
#include "gdsparse.h"
#include "gdsparse_vrml.h"
#include "gdsobject_vrml.h"
//#include "gds_globals.h"
//#include "gds2pov.h"
//#include "gdstext.h"
//#include "gdspolygon.h"


extern int verbose_output;

GDSParse_vrml::GDSParse_vrml (class GDSConfig *config, class GDSProcess *process, bool bounding_output, char* camfile, bool generate_process) : GDSParse(config, process, generate_process)
{
  _config = config;
  _camfile = camfile;
  SetOutputOptions(bounding_output, true, false, true);
}

GDSParse_vrml::~GDSParse_vrml ()
{
}

//class GDSObject_vrml *GDSParse_vrml::NewObject(char *Name)
class GDSObject *GDSParse_vrml::NewObject(char *Name)
{
  return new class GDSObject_vrml(Name);
}

void GDSParse_vrml::OutputFooter()
{

  /* [at]: nothing to do
     if(!_bounding_output){
     if(_topcellname){
     //fprintf(_optr, "object { str_%s }\n", _topcellname);
     }else{
     if(_Objects->GetObjectRef(0)){
     //fprintf(_optr, "object { str_%s }\n",
     //_Objects->GetObjectRef(0)->GetName());
     }
     }
     }
  */
}

void GDSParse_vrml::OutputHeader()
{
  if(_optr && _Objects){
    fprintf(_optr, "#VRML V2.0 utf8\n\n");
    fprintf(_optr, "## automatically generated from gds2vrml\n");
    fprintf(_optr, "## Copyright (C) 2009 Roger Light, 2010 Armin Taschwer\n");
    fprintf(_optr, "## http://atchoo.org/gds2pov/\n\n");

    /* [-at-]
       fprintf(_optr, "#include \"colors.inc\"\n");
       fprintf(_optr, "#include \"metals.inc\"\n");
       fprintf(_optr, "#include \"transforms.inc\"\n");

       struct _Boundary *Boundary = _Objects->GetBoundary();
       float half_widthX = (Boundary->XMax - Boundary->XMin)/2;
       float half_widthY = (Boundary->YMax - Boundary->YMin)/2;
       float centreX = half_widthX + Boundary->XMin;
       float centreY = half_widthY + Boundary->YMin;

       float distance;
       if(half_widthX > half_widthY){
       distance = half_widthX * 1.8;
       }else{
       distance = half_widthY * 1.8;
       }

       fprintf(_optr, "#declare sizeX = %.2f;\n", Boundary->XMax - Boundary->XMin);
       fprintf(_optr, "#declare sizeY = %.2f;\n", Boundary->YMax - Boundary->YMin);
       //		fprintf(_optr, "#declare sizeZ = %.2f\n", Boundary->ZMax - Boundary->ZMin);

       fprintf(_optr, "// TopLeft: %.2f, %.2f\n", Boundary->XMin, Boundary->YMax);
       fprintf(_optr, "// TopRight: %.2f, %.2f\n", Boundary->XMax, Boundary->YMax);
       fprintf(_optr, "// BottomLeft: %.2f, %.2f\n", Boundary->XMin, Boundary->YMin);
       fprintf(_optr, "// BottomRight: %.2f, %.2f\n", Boundary->XMax, Boundary->YMin);
       fprintf(_optr, "// Centre: %.2f, %.2f\n", centreX, centreY);

       float XMod = _config->GetCameraPos()->XMod;
       float YMod = _config->GetCameraPos()->YMod;
       float ZMod = _config->GetCameraPos()->ZMod;
    */

    /* _camfile is a possible camera include file. Depends on the -e option
     * If it is null, use the normal camera else use the include */
    /*[at]
      if(!_camfile){
      switch(_config->GetCameraPos()->boundarypos){
      case bpCentre:
      // Default camera angle = 67.38
      // Half of this is 33.69
      // tan(33.69) = 0.66666 = 1/1.5
      // Make it slightly larger so that we have a little bit of a border: 1.5+20% = 1.8
	
      fprintf(_optr, "camera {\n\tlocation <%.2f,%.2f,%.2f>\n", centreX*XMod, centreY*YMod, -distance*ZMod);
      break;
      case bpTopLeft:
      fprintf(_optr, "camera {\n\tlocation <%.2f, %.2f, %.2f>\n", Boundary->XMin*XMod, Boundary->YMax*YMod, -distance*ZMod);
      break;
      case bpTopRight:
      fprintf(_optr, "camera {\n\tlocation <%.2f, %.2f, %.2f>\n", Boundary->XMax*XMod, Boundary->YMax*YMod, -distance*ZMod);
      break;
      case bpBottomLeft:
      fprintf(_optr, "camera {\n\tlocation <%.2f, %.2f, %.2f>\n", Boundary->XMin*XMod, Boundary->YMin*YMod, -distance*ZMod);
      break;
      case bpBottomRight:
      fprintf(_optr, "camera {\n\tlocation <%.2f, %.2f, %.2f>\n", Boundary->XMax*XMod, Boundary->YMin*YMod, -distance*ZMod);
      break;
      }

      fprintf(_optr, "\tsky <0,0,-1>\n"); //This fixes the look at rotation (hopefully)

      XMod = _config->GetLookAtPos()->XMod;
      YMod = _config->GetLookAtPos()->YMod;
      ZMod = _config->GetLookAtPos()->ZMod;

      switch(_config->GetLookAtPos()->boundarypos){
      case bpCentre:
      fprintf(_optr, "\tlook_at <%.2f,%.2f,%.2f>\n}\n", centreX*XMod, centreY*YMod, -distance*ZMod);
      break;
      case bpTopLeft:
      fprintf(_optr, "\tlook_at <%.2f,%.2f,%.2f>\n}\n", Boundary->XMin*XMod, Boundary->YMax*YMod, -distance*ZMod);
      break;
      case bpTopRight:
      fprintf(_optr, "\tlook_at <%.2f,%.2f,%.2f>\n}\n", Boundary->XMax*XMod, Boundary->YMax*YMod, -distance*ZMod);
      break;
      case bpBottomLeft:
      fprintf(_optr, "\tlook_at <%.2f,%.2f,%.2f>\n}\n", Boundary->XMin*XMod, Boundary->YMin*YMod, -distance*ZMod);
      break;
      case bpBottomRight:
      fprintf(_optr, "\tlook_at <%.2f,%.2f,%.2f>\n}\n", Boundary->XMax*XMod, Boundary->YMin*YMod, -distance*ZMod);
      break;
      }
      }else{
      fprintf(_optr, "#include %s\n", _camfile);
      }

      if(_config->GetLightPos()!=NULL){
      Position dummypos;
      dummypos.Next = _config->GetLightPos();

      Position *LightPos = &dummypos;

      while(LightPos->Next){
      LightPos = LightPos->Next;
      XMod = LightPos->XMod;
      YMod = LightPos->YMod;
      ZMod = LightPos->ZMod;

      switch(LightPos->boundarypos){
      case bpCentre:
      fprintf(_optr, "light_source {<%.2f,%.2f,%.2f> White }\n", centreX*XMod, centreY*YMod, -distance*ZMod);
      break;
      case bpTopLeft:
      fprintf(_optr, "light_source {<%.2f,%.2f,%.2f> White }\n", Boundary->XMin*XMod, Boundary->YMax*YMod, -distance*ZMod);
      break;
      case bpTopRight:
      fprintf(_optr, "light_source {<%.2f,%.2f,%.2f> White }\n", Boundary->XMax*XMod, Boundary->YMax*YMod, -distance*ZMod);
      break;
      case bpBottomLeft:
      fprintf(_optr, "light_source {<%.2f,%.2f,%.2f> White }\n", Boundary->XMin*XMod, Boundary->YMin*YMod, -distance*ZMod);
      break;
      case bpBottomRight:
      fprintf(_optr, "light_source {<%.2f,%.2f,%.2f> White }\n", Boundary->XMax*XMod, Boundary->YMin*YMod, -distance*ZMod);
      break;
      }
      }
      }else{
      fprintf(_optr, "light_source {<%.2f,%.2f,%.2f> White }\n", centreX, centreY, -distance);
      }

      fprintf(_optr, "background { color Black }\n");
      fprintf(_optr, "global_settings { ambient_light rgb <%.2f,%.2f,%.2f> }\n", _config->GetAmbient(), _config->GetAmbient(), _config->GetAmbient());
    */

    /* Output layer texture information */
    
    struct ProcessLayer *firstlayer;
    firstlayer = _process->GetLayer();

    fprintf (_optr, "## some material definitions\n");

    do {

      if(firstlayer->Show){

	fprintf (_optr, "Shape {\n");
	fprintf (_optr, "  geometry Box { size 0 0 0 }\n");
	fprintf (_optr, "  appearance DEF c%s Appearance {\n", firstlayer->Name);
	fprintf (_optr, "    material Material {\n");
	fprintf (_optr, "      diffuseColor %.2f %.2f %.2f\n",
		 firstlayer->Red, firstlayer->Green, firstlayer->Blue);
	fprintf (_optr, "      transparency %.2f\n", firstlayer->Filter);
		
	if(!firstlayer->Metal){
	  //fprintf(_optr, "#declare t%s = pigment{rgbf <%.2f, %.2f, %.2f, %.2f>}\n", 
	  //		  firstlayer->Name, firstlayer->Red,firstlayer->Green, firstlayer->Blue, firstlayer->Filter);
	  fprintf (_optr, "      shininess 0.2\n");
	}
	//else{
	//fprintf(_optr, "#declare t%s = texture{pigment{rgbf <%.2f, %.2f, %.2f, %.2f>} finish{F_MetalA}}\n", firstlayer->Name, firstlayer->Red, firstlayer->Green, firstlayer->Blue, firstlayer->Filter);
	//}

	fprintf (_optr, "    }\n");
	fprintf (_optr, "  }\n");
	fprintf (_optr, "}\n");
      }
      firstlayer = firstlayer->Next;
    } while (firstlayer);

    /* [at]
       if(firstlayer->Show){
       if(!firstlayer->Metal){
       fprintf(_optr, "#declare t%s = pigment{rgbf <%.2f, %.2f, %.2f, %.2f>}\n", 
       firstlayer->Name, firstlayer->Red, firstlayer->Green, firstlayer->Blue, firstlayer->Filter);
       }else{
       fprintf(_optr, "#declare t%s = texture{pigment{rgbf <%.2f, %.2f, %.2f, %.2f>} finish{F_MetalA}}\n", firstlayer->Name, firstlayer->Red, firstlayer->Green, firstlayer->Blue, firstlayer->Filter);
       }
       }
	
       if(_bounding_output){
       fprintf(_optr, "box {<%.2f,%.2f,%.2f> <%.2f,%.2f,%.2f> texture { pigment { rgb <0.75, 0.75, 0.75> } } }", Boundary->XMin, Boundary->YMin,_units*_process->GetLowest(),Boundary->XMax, Boundary->YMax,_units*_process->GetHighest());
       }
    */

    GDSObject * obj;

    if(_topcellname){
      obj = _Objects->GetObjectRef(_topcellname);
      if (obj == NULL) {
	printf ("ERROR: \"%s\" couldn't be found\n",_topcellname);
	printf ("HINT: Cadence usually convert dots to underscores, etc.\n");
	exit(1);
      }
    }else{
      if(_Objects->GetObjectRef(0)){
	obj = _Objects->GetObjectRef(0);
      } else {
	printf ("ERROR: didn't find any object\n");
	exit(1);
      }
    }

    if (obj) {
      GDSObject_vrml *vrmlo = (GDSObject_vrml*)obj;
      vrmlo->UnsetBlocked();
      vrmlo->UnsetTransforming();
    } else {
      printf ("Sorry, no objects found ..\n");
      exit(1);
    }

    //the recursive OutputToFile should be called internally
    
  }
    
}

