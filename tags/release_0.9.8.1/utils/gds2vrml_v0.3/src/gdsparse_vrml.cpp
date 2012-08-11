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

#include <stdlib.h>

#include "config_cfg.h"
#include "process_cfg.h"
#include "gdsparse.h"
#include "gdsparse_vrml.h"
#include "gdsobject_vrml.h"

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
	fprintf (_optr, "      transparency %.2f\n", 1.0 - firstlayer->Filter);
		
	if(!firstlayer->Metal){
	  fprintf (_optr, "      shininess 0.2\n");
	}

	fprintf (_optr, "    }\n");
	fprintf (_optr, "  }\n");
	fprintf (_optr, "}\n");
      }
      firstlayer = firstlayer->Next;
    } while (firstlayer);


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

