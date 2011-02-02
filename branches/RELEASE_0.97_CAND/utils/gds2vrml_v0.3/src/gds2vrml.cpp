/* 
 * File: gds2vrml.cpp
 * quick hack from gds2vrml.cpp to implement a gds2vrml functionality
 * Author: Armin Taschwer
 * Projekt: gds2vrml
 * Copyright (C) 2010 by Armin Taschwer
 */

// original header
/*
 * File: gds2pov
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the main body of the gds2pov program.
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

#include "config_cfg.h"
#include "process_cfg.h"
#include "gds_globals.h"
#include "gds2vrml.h"
#include "gdsparse_vrml.h"

extern int verbose_output;
bool decompose;

void printusage()
{
  printf("gds2vrml  version %s\n", VERSION);
  printf("Copyright (C) 2004-2008 Roger Light, 2009-2010 Armin Taschwer\n");
  printf("for the original gds2pov refer to http://atchoo.org/gds2pov/\n\n");
  printf("gds2vrml comes with ABSOLUTELY NO WARRANTY.  You may distribute gds2pov freely\nas described in the readme.txt distributed with this file.\n\n");
  printf("gds2vrml is a program for converting a GDS2 file to a VRML scene file.\n\n");
  printf("Usage: gds2vrml [-b] [-c config.txt] [-d] [-e camera.pov] [-h] [-i input.gds] [-o output.pov] [-p process.txt] [-q] [-t topcell] [-v]\n\n");
  printf("Options\n");
  printf(" -b\t\tOutput bounding box instead of layout to allow easier and\n\t\tquicker placing of the camera\n");
  printf(" -c\t\tSpecify config file\n");
  printf(" -d\t\tDecompose polygons into triangles (use mesh2 object instead of prism)\n");
  printf(" -e\t\tUse external camera include file instead of specifying camera internally\n");
  printf(" -g\t\tGenerate a process file based on the input gds2 file (suppresses POV-Ray file generation).\n");
  printf(" -h\t\tDisplay this help\n");
  printf(" -i\t\tInput GDS2 file (stdin if not specified)\n");
  printf(" -o\t\tOutput VRML file (stdout if not specified)\n");
  printf(" -p\t\tSpecify process file\n");
  printf(" -q\t\tQuiet output\n");
  printf(" -t\t\tSpecify top cell name\n");
  printf(" -v\t\tVerbose output\n\n");
}

int main(int argc, char *argv[])
{
  bool bounding_output = false;
  bool generate_process = false;

  char *gdsfile=NULL;
  char *vrmlfile=NULL;
  char *camfile=NULL;

  char *configfile=NULL;
  char *processfile=NULL;
  char *topcell=NULL;

  verbose_output = 1;
  decompose = false;

  if(argc>21){
    fprintf(stderr, "Error: Invalid number of arguments.\n\n");
    printusage();
    return 1;
  }

  for(int i=1; i<argc; i++){
    if(argv[i][0] == '-'){
      if(strncmp(argv[i], "-b", strlen("-b"))==0){
	bounding_output = true;
      }else if(strncmp(argv[i], "-c", strlen("-c"))==0){

	if(i==argc-1){
	  printf("Error: -c switch given but no config file specified.\n\n");
	  printusage();
	  return 1;
	}else{
	  configfile = argv[i+1];
	}
      }else if(strncmp(argv[i], "-d", strlen("-d"))==0){
	decompose = true;
      }else if(strncmp(argv[i], "-e", strlen("-e"))==0){
	if(i==argc-1){
	  fprintf(stderr, "Error: -e switch given but no camera file specified.\n\n");
	  printusage();
	  return 1;
	}else{
	  camfile = argv[i+1];
	}
      }else if(strncmp(argv[i], "-g", strlen("-g"))==0){
	generate_process = true;
      }else if(strncmp(argv[i], "-h", strlen("-h"))==0){
	printusage();
	return 0;
      }else if(strncmp(argv[i], "-i", strlen("-i"))==0){
	if(i==argc-1){
	  fprintf(stderr, "Error: -i switch given but no input file specified.\n\n");
	  printusage();
	  return 1;
	}else{
	  gdsfile = argv[i+1];
	}
      }else if(strncmp(argv[i], "-o", strlen("-o"))==0){
	if(i==argc-1){
	  fprintf(stderr, "Error: -o switch given but no output file specified.\n\n");
	  printusage();
	  return 1;
	}else{
	  vrmlfile = argv[i+1];
	}
      }else if(strncmp(argv[i], "-p", strlen("-p"))==0){
	if(i==argc-1){
	  printf("Error: -p switch given but no process file specified.\n\n");
	  printusage();
	  return 1;
	}else{
	  processfile = argv[i+1];
	}
      }else if(strncmp(argv[i], "-q", strlen("-q"))==0){
	verbose_output--;
      }else if(strncmp(argv[i], "-t", strlen("-t"))==0){
	if(i==argc-1){
	  printf("Error: -t switch given but no top cell specified.\n\n");
	  printusage();
	  return 1;
	}else{
	  topcell = argv[i+1];
	}
      }else if(strncmp(argv[i], "-v", strlen("-v"))==0){
	verbose_output++;
      }else{
	printusage();
	return 1;
      }
      //}else{
      // Assume it is a process/config file specified on a previous arg
    }
  }


  /************ Load config ****************/

  class GDSConfig *config=NULL;

  if(configfile){
    config = new GDSConfig(configfile);
  }else{
    config = new GDSConfig(); // Start with default positions
  }
  if(!config){
    fprintf(stderr, "Error: Out of memory.\n");
  }else if(!config->IsValid()){
    fprintf(stderr, "Error: %s is not a valid config file.\n", configfile);
    delete config;
    return -1;
  }

  /************ Load process ****************/

  class GDSProcess *process=NULL;
  /* 
  ** Order of precedence for process.txt:
  ** -p switch (given as an argument to this function)
  ** Specified in config file
  ** Use process.txt if none others specified.
  */
  if(processfile == NULL){
    if(config->GetProcessFile()!=NULL){
      processfile = config->GetProcessFile();
    }else{
      processfile = new char[13];
      strncpy(processfile, "process.txt", strlen("process.txt")+1);
    }
  }
  process = new GDSProcess();
  if(!process){
    fprintf(stderr, "Error: Out of memory.\n");
    delete config;
    return -1;
  }
  if(!generate_process){
    process->Parse(processfile);
    if(!process->IsValid()){
      fprintf(stderr, "Error: %s is not a valid process file\n", processfile);
      delete config;
      delete process;
      return -1;
    }else if(process->LayerCount()==0){
      fprintf(stderr, "Error: No layers found in \"%s\".\n", processfile);
      delete config;
      delete process;
      return -1;
    }
  }

  /************ Open GDS2 file and parse ****************/

  FILE *iptr;
  if(gdsfile){
    iptr = fopen(gdsfile, "rb");
  }else{
    iptr = stdin;
  }
  if(iptr){
    class GDSParse_vrml *Parser = new class GDSParse_vrml(config, process, bounding_output, camfile, generate_process);
    if(!Parser->Parse(iptr)){
      if(!generate_process){
	FILE *optr;
	if(vrmlfile){
	  optr = fopen(vrmlfile, "wt");
	}else{
	  optr = stdout;
	}

	if(optr){
	  Parser->Output(optr, topcell);
	  if(optr != stdout){
	    fclose(optr);
	  }
	}else{
	  fprintf(stderr, "Error: Unable to open %s.\n", vrmlfile);
	}
      }else{
	process->Save(processfile);
      }
    }

    if(iptr != stdin){
      fclose(iptr);
    }


    delete Parser;
    delete config;
    delete process;
  }else{
    fprintf(stderr, "Error: Unable to open %s.\n", gdsfile);
    delete config;
    delete process;
    return -1;
  }
  return 0;
}

