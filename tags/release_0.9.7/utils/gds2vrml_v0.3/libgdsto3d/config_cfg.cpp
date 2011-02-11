/*
 * File: config_cfg.cpp
 * Author: Roger Light
 * Project: gdsto3d
 * $Id: config_cfg.cpp 302 2008-02-26 10:47:01Z roger $
 * 
 * This parses the configuration file which contains camera and light
 * information.
 *
 * Copyright (C) 2004-2005 Roger Light.
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cstdio>
#include <cstring>

#include "config_cfg.h"

GDSConfig::GDSConfig()
{
	_ProcessFile = NULL;
	_Font = NULL;
	_Ambient = 1.2;
	_Scale = 1.0;

	_CameraPos.postype = ptCamera;
	_CameraPos.boundarypos = bpCentre;
	_CameraPos.XMod = 1.0;
	_CameraPos.YMod = 1.0;
	_CameraPos.ZMod = 1.0;
	_CameraPos.Next = NULL;

	_LookAtPos.postype = ptLookAt;
	_LookAtPos.boundarypos = bpCentre;
	_LookAtPos.XMod = 1.0;
	_LookAtPos.YMod = 1.0;
	_LookAtPos.ZMod = 0.0;
	_LookAtPos.Next = NULL;

	_FirstLight = NULL;
	_LastLight = NULL;
	_LightCount = 0;
	_Valid = true;

}

GDSConfig::GDSConfig(char *configfile)
{
	_ProcessFile = NULL;
	_Font = NULL;
	_Ambient = 1.2;
	_Scale = 1.0;
	_CameraPos.Next = NULL;
	_LookAtPos.Next = NULL;

	_FirstLight = NULL;
	_LastLight = NULL;
	_LightCount = 0;
	_Valid = true;


	int posstart_cnt = 0;
	int posend_cnt = 0;
	int globalstart_cnt = 0;
	int globalend_cnt = 0;
	char line[1024];
	int current_line = 0;

	PosType current_type = ptNone;

	FILE *cptr = NULL;

	bool in_global = false;
	bool got_ambient = false;
	bool got_processfile = false;
	bool got_font = false;
	bool got_scale = false;

	bool in_position = false;
	bool got_type = false;
	bool got_position = false;
	bool got_xmod = false;
	bool got_ymod = false;
	bool got_zmod = false;

	int i;

	cptr = fopen(configfile, "rt");
	
	if(!cptr){
		fprintf(stderr, "Error: Unable to open config file \"%s\"\n", configfile);
		_Valid = false;
		return;
	}

	while(!feof(cptr) && fgets(line, 1024, cptr)){
		if(line[0]!='#'){
			if(strstr(line, "PositionStart")){			
				posstart_cnt++;
			}else if(strstr(line, "PositionEnd")){
				posend_cnt++;
			}else if(strstr(line, "GlobalStart")){
				globalstart_cnt++;
			}else if(strstr(line, "GlobalEnd")){
				globalend_cnt++;
			}
		}
	}
	if(posstart_cnt!=posend_cnt){
		fprintf(stderr, "Invalid config file. ");
		fprintf(stderr, "There should be equal numbers of PositionStart and PositionEnd elements! ");
		fprintf(stderr, "(%d and %d found respectively)\n", posstart_cnt, posend_cnt);
		_Valid = false;
		return;
	}

	if(globalstart_cnt!=globalend_cnt || globalstart_cnt > 1 || globalend_cnt > 1){
		fprintf(stderr, "Invalid config file. ");
		fprintf(stderr, "There should be either 1 or 0 of both of GlobalStart and GlobalEnd elements! ");
		fprintf(stderr, "(%d and %d found respectively)\n", globalstart_cnt, globalend_cnt);
		_Valid = false;
		return;
	}

//	PosCount = posstart_cnt;

	fseek(cptr, 0, SEEK_SET);
	while(!feof(cptr) && fgets(line, 1024, cptr)){
		current_line++;
		if(line[0]!='#'){
			if(strstr(line, "GlobalStart")){
				if(in_position){
					fprintf(stderr, "Error: GlobalStart inside PositionStart on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				in_global = true;
				got_ambient = false;
				got_processfile = false;
			}else if(strstr(line, "Ambient:")){
				if(!in_global){
					fprintf(stderr, "Error: Ambient definition outside of GlobalStart and GlobalEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_ambient){
					fprintf(stderr, "Warning: Duplicate Ambient definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Ambient: %f", &_Ambient);
				}
				got_ambient = true;
			}else if(strstr(line, "Scale:")){
				if(!in_global){
					fprintf(stderr, "Error: Scale definition outside of GlobalStart and GlobalEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_scale){
					fprintf(stderr, "Warning: Duplicate Scale definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Scale: %f", &_Scale);
					if(_Scale<0.001){
						fprintf(stderr, "Warning: Scale is very small (<0.001)\n");
					}
				}
				got_scale = true;
			}else if(strstr(line, "ProcessFile:")){
				if(!in_global){
					fprintf(stderr, "Error: ProcessFile definition outside of GlobalStart and GlobalEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_processfile){
					fprintf(stderr, "Warning: Duplicate ProcessFile definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					if(_ProcessFile){
						delete [] _ProcessFile;
						_ProcessFile = NULL;
					}
					_ProcessFile = new char[256];
				
					strncpy(_ProcessFile, &line[13], 256);
					for(i=strlen(_ProcessFile)-1; i>=0; i--){
						if(_ProcessFile[i] == '\n'){
							_ProcessFile[i] = '\0';
							break;
						}
					}
				}
				got_processfile = true;
			}else if(strstr(line, "Font:")){
				if(!in_global){
					fprintf(stderr, "Error: Font definition outside of GlobalStart and GlobalEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_font){
					fprintf(stderr, "Warning: Duplicate Font definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					if(_Font){
						delete [] _Font;
						_Font = NULL;
					}
					_Font = new char[256];
				
					strncpy(_Font, &line[6], 256);
					for(i=strlen(_Font)-1; i>=0; i--){
						if(_Font[i] == '\n'){
							_Font[i] = '\0';
							break;
						}
					}
				}
				got_font = true;
			}else if(strstr(line, "GlobalEnd")){
				in_global = false;
			}else if(strstr(line, "PositionStart")){
				if(in_position){
					fprintf(stderr, "Error: PositionStart without PositionEnd not allowed. PositionEnd should appear before line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}else if(in_global){
					fprintf(stderr, "Error: PositionStart inside GlobalStart on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				in_position = true;
				got_type = false;
				got_position = false;
				got_xmod = false;
				got_ymod = false;
				got_zmod = false;
				current_type = ptNone;
			}else if(strstr(line, "Type:")){
				if(!in_position){
					fprintf(stderr, "Error: Type definition outside of PositionStart and PositionEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_type){
					fprintf(stderr, "Warning: Duplicate Type definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					if(strstr(line, "Type: Camera")){
						current_type = ptCamera;
					}else if(strstr(line, "Type: LookAt")){
						current_type = ptLookAt;
					}else if(strstr(line, "Type: Light")){
						current_type = ptLight;
						if(_LastLight){
							_LastLight->Next = new Position;
							_LastLight = _LastLight->Next;
							_LastLight->Next = NULL;
							_LastLight->boundarypos = bpCentre;
							_LastLight->XMod = 1.0;
							_LastLight->YMod = 1.0;
							_LastLight->ZMod = 1.0;
						}else{
							_FirstLight = new Position;
							_LastLight = _FirstLight;
							_LastLight->Next = NULL;
							_LastLight->boundarypos = bpCentre;
							_LastLight->XMod = 1.0;
							_LastLight->YMod = 1.0;
							_LastLight->ZMod = 1.0;
						}
					}else{
						fprintf(stderr, "Error: Unknown position type \"%s\" on line %d of config file.\n", line, current_line);
						_Valid = false;
						fclose(cptr);
						return;
					}
					got_type = true;
				}
			}else if(strstr(line, "Position:")){
				if(!in_position){
					fprintf(stderr, "Error: Position definition outside of PositionStart and PositionEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(!got_type){
					fprintf(stderr, "Error: Type must be defined before any other elements in a Position block.\n");
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_position){
					fprintf(stderr, "Warning: Duplicate Position definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					BoundaryPos thispos;
					if(strstr(line, "Position: Centre")){
						thispos = bpCentre;
					}else if(strstr(line, "Position: TopLeft")){
						thispos = bpTopLeft;
					}else if(strstr(line, "Position: TopRight")){
						thispos = bpTopRight;
					}else if(strstr(line, "Position: BottomLeft")){
						thispos = bpBottomLeft;
					}else if(strstr(line, "Position: BottomRight")){
						thispos = bpBottomRight;
					}else{
						fprintf(stderr, "Error: Unknown Position \"%s\" on line %d of config file.\n", line, current_line);
						_Valid = false;
						fclose(cptr);
						return;
					}
					switch(current_type){
						case ptCamera:
							_CameraPos.boundarypos = thispos;
							break;
						case ptLookAt:
							_LookAtPos.boundarypos = thispos;
							break;
						case ptLight:
							if(_LastLight){
								_LastLight->boundarypos = thispos;
							}else{
								fprintf(stderr, "Error: Position found but LastLight not initialised (this shouldn't happen, please contact the author)\n");
								_Valid = false;
								fclose(cptr);
								return;
							}
							break;
						case ptNone:
						default:
							fprintf(stderr, "Error: Unknown position type found (this shouldn't happen, please contact the author)\n");
							break;
					}
					got_position = true;
				}
				
			}else if(strstr(line, "XMod:")){
				if(!in_position){
					fprintf(stderr, "Error: XMod definition outside of PositionStart and PositionEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(!got_type){
					fprintf(stderr, "Error: Type must be defined before any other elements in a Position block.\n");
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_xmod){
					fprintf(stderr, "Error: Duplicate XMod definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					switch(current_type){
						case ptCamera:
							sscanf(line, "XMod: %f", &_CameraPos.XMod);
							break;
						case ptLookAt:
							sscanf(line, "XMod: %f", &_LookAtPos.XMod);
							break;
						case ptLight:
							if(_LastLight){
								sscanf(line, "XMod: %f", &_LastLight->XMod);
							}else{
								fprintf(stderr, "Error: XMod found but LastLight not initialised (this shouldn't happen, please contact the author)\n");
								_Valid = false;
								fclose(cptr);
								return;
							}
							break;
						case ptNone:
						default:
							fprintf(stderr, "Error: Unknown position type found (this shouldn't happen, please contact the author)\n");
							break;
					}
					got_xmod = true;
				}
			}else if(strstr(line, "YMod:")){
				if(!in_position){
					fprintf(stderr, "Error: YMod definition outside of PositionStart and PositionEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(!got_type){
					fprintf(stderr, "Error: Type must be defined before any other elements in a Position block.\n");
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_ymod){
					fprintf(stderr, "Error: Duplicate YMod definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					switch(current_type){
						case ptCamera:
							sscanf(line, "YMod: %f", &_CameraPos.YMod);
							break;
						case ptLookAt:
							sscanf(line, "YMod: %f", &_LookAtPos.YMod);
							break;
						case ptLight:
							if(_LastLight){
								sscanf(line, "YMod: %f", &_LastLight->YMod);
							}else{
								fprintf(stderr, "Error: YMod found but LastLight not initialised (this shouldn't happen, please contact the author)\n");
								_Valid = false;
								fclose(cptr);
								return;
							}
							break;
						case ptNone:
						default:
							fprintf(stderr, "Error: Unknown position type found (this shouldn't happen, please contact the author)\n");
							break;
					}
					got_ymod = true;
				}
			}else if(strstr(line, "ZMod:")){
				if(!in_position){
					fprintf(stderr, "Error: ZMod definition outside of PositionStart and PositionEnd on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(!got_type){
					fprintf(stderr, "Error: Type must be defined before any other elements in a Position block.\n");
					_Valid = false;
					fclose(cptr);
					return;
				}
				if(got_zmod){
					fprintf(stderr, "Error: Duplicate ZMod definition on line %d of config file. Ignoring new definition.\n", current_line);
				}else{
					switch(current_type){
						case ptCamera:
							sscanf(line, "ZMod: %f", &_CameraPos.ZMod);
							break;
						case ptLookAt:
							sscanf(line, "ZMod: %f", &_LookAtPos.ZMod);
							break;
						case ptLight:
							if(_LastLight){
								sscanf(line, "ZMod: %f", &_LastLight->ZMod);
							}else{
								fprintf(stderr, "Error: ZMod found but LastLight not initialised (this shouldn't happen, please contact the author)\n");
								_Valid = false;
								fclose(cptr);
								return;
							}
							break;
						case ptNone:
						default:
							fprintf(stderr, "Error: Unknown position type found (this shouldn't happen, please contact the author)\n");
							break;
					}
					got_zmod = true;
				}
			}else if(strstr(line, "PositionEnd")){
				if(!in_position){
					fprintf(stderr, "Error: PositionEnd without PositionStart on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}else if(!got_type){
					fprintf(stderr, "Error: PositionEnd without Type on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}else if(!got_position){
					fprintf(stderr, "Error: PositionEnd without Position on line %d of config file.\n", current_line);
					_Valid = false;
					fclose(cptr);
					return;
				}
				in_position = false;
			}
		}
	}
	fclose(cptr);
}

GDSConfig::~GDSConfig()
{
	if(_ProcessFile){
		delete [] _ProcessFile;
	}

	if(_FirstLight){
		Position *pos1;
		Position *pos2;

		pos1 = _FirstLight;

		while(pos1->Next){
			pos2 = pos1->Next;
			if(pos1){
				delete pos1;
			}
			pos1 = pos2;
		}
		if(pos1){
			delete pos1;
		}
	}
	if(_Font){
		delete [] _Font;
	}
}

bool GDSConfig::IsValid()
{
	return _Valid;
}

float GDSConfig::GetAmbient()
{
	return _Ambient;
}

float GDSConfig::GetScale()
{
	return _Scale;
}

char *GDSConfig::GetProcessFile()
{
	return _ProcessFile;
}

char *GDSConfig::GetFont()
{
	return _Font;
}

Position *GDSConfig::GetLookAtPos()
{
	return &_LookAtPos;
}

Position *GDSConfig::GetCameraPos()
{
	return &_CameraPos;
}

Position *GDSConfig::GetLightPos()
{
	return _FirstLight;
}

