/* 
 * File: gdsparse_vrml.h
 * quick hack from gdsparse_vrml.h to implement a gds2vrml functionality
 * Author: Armin Taschwer
 * Projekt: gds2vrml
 * Copyright (C) 2010 by Armin Taschwer
 */

// original header
/*
 * File: gdsparse_pov.h
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

#ifndef __GDSPARSE_VRML_H__
#define __GDSPARSE_VRML_H__

#include "config_cfg.h"
#include "process_cfg.h"
#include "gdsparse.h"

class GDSParse_vrml : public GDSParse
{
private:
  char *_camfile;

public:
  GDSParse_vrml (class GDSConfig *config, class GDSProcess *process, bool bounding_output, char *camfile, bool generate_process);
  ~GDSParse_vrml ();

  //class GDSObject_vrml *NewObject(char *Name);
  class GDSObject *NewObject(char *Name);
  void OutputHeader();
  void OutputFooter();
};

#endif // __GDSPARSE_VRML_H__

