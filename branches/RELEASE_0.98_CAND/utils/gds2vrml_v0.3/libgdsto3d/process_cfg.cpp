/*
 * File: process_cfg.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is used to parse the process configuration file.
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


	/*
	** ProcessFile structure
	**
	** # at the start of a line is a comment
	**
	** StartLayer: Name
	** Layer: Number
	** Datatype: Number
	** Height: Number
	** Thickness: Number
	** Colour: <0, 0.5, 1>
	** Metal: 1/0
	** Transparent: 1/0
	** Show: 1/0
	** EndLayer
	*/

#include "process_cfg.h"
#include "gds_globals.h"

GDSProcess::GDSProcess()
{
	_Count = 0;
	_Valid = true;

	_FirstLayer = NULL;
}

GDSProcess::~GDSProcess ()
{
	if(_FirstLayer){
		struct ProcessLayer *layer1;
		struct ProcessLayer *layer2;

		layer1 = _FirstLayer;

		while(layer1->Next){
			layer2 = layer1->Next;
			if(layer1->Name){
				delete [] layer1->Name;
			}
			if(layer1){
				delete layer1;
			}
			layer1 = layer2;
		}
		if(layer1->Name){
			delete [] layer1->Name;
		}
		if(layer1){
			delete layer1;
		}
	}
}

//bool GDSProcess::Parse(char *processfile)
void GDSProcess::Parse(char *processfile)
{
	int layerstart_cnt = 0;
	int layerend_cnt = 0;
	char line[1024];

	int current_line = 0;
	int current_element = -1;

	/* State variables */
	bool in_layer = false;
	bool got_layer = false;
	bool got_datatype = false;
	bool got_height = false;
	bool got_thickness = false;
	bool got_red = false;
	bool got_green = false;
	bool got_blue = false;
	bool got_filter = false;
	bool got_metal = false;
	bool got_show = false; 
	/* End State variables */
	bool showing;

	struct ProcessLayer NewLayer;
	NewLayer.Name = NULL;

	FILE *pptr = NULL;

	pptr = fopen(processfile, "rt");
	
	if(!pptr){
		fprintf(stderr, "Unable to open process file \"%s\".\n", processfile);
		_Valid = false;
		return;
	}

	while(!feof(pptr) && fgets(line, 1024, pptr)){
		if(line[0]!='#'){
			if(strstr(line, "LayerStart")){			
				layerstart_cnt++;
			}else if(strstr(line, "LayerEnd")){
				layerend_cnt++;
			}
		}
	}
	if(layerstart_cnt!=layerend_cnt){
		fprintf(stderr, "Invalid process file. ");
		fprintf(stderr, "There should be equal numbers of LayerStart and LayerEnd elements! ");
		fprintf(stderr, "(%d and %d found respectively)\n", layerstart_cnt, layerend_cnt);
		_Valid = false;
		fclose(pptr);
		return;
	}

	_Count = layerstart_cnt;

	fseek(pptr, 0, SEEK_SET);
	while(!feof(pptr) && fgets(line, 1024, pptr)){
		current_line++;
		if(line[0]!='#'){
			if(strstr(line, "LayerStart:")){
				if(in_layer){
					fprintf(stderr, "Error: LayerStart without LayerEnd not allowed. LayerEnd should appear before line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				in_layer = true;
				got_layer = false;
				got_datatype = false;
				got_height = false;
				got_thickness = false;
				got_red = false;
				got_green = false;
				got_blue = false;
				got_filter = false;
				got_metal = false;
				got_show = false;
				current_element++;

				if(NewLayer.Name){
					delete [] NewLayer.Name;
					NewLayer.Name = NULL;
				}
				NewLayer.Name = new char[strlen(line)-strlen("LayerStart: ")+1];
//				strncpy(NewLayer.Name, line+strlen("LayerStart: "), strlen(line)-strlen("LayerStart: "));
				strcpy(NewLayer.Name, line+strlen("LayerStart: "));
				NewLayer.Name[strlen(NewLayer.Name)-1] = '\0';
				NewLayer.Layer = 0;
				NewLayer.Datatype = -1;
				NewLayer.Height = 0.0;
				NewLayer.Thickness = 0.0;
				NewLayer.Red = 0.0;
				NewLayer.Green = 0.0;
				NewLayer.Blue = 0.0;
				NewLayer.Filter = 0.0;
				NewLayer.Metal = 0;
				NewLayer.Show = false;
				NewLayer.Next = NULL;
			}else if(strstr(line, "Layer:")){
				if(!in_layer){
					fprintf(stderr, "Error: Layer definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_layer){
					fprintf(stderr, "Warning: Duplicate Layer definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Layer: %d", &NewLayer.Layer);
					got_layer = true;
				}
			}else if(strstr(line, "Datatype:")){
				if(!in_layer){
					fprintf(stderr, "Error: Datatype definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_datatype){
					fprintf(stderr, "Warning: Duplicate Datatype definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Datatype: %d", &NewLayer.Datatype);
					got_datatype = true;
				}
			}else if(strstr(line, "Height:")){
				if(!in_layer){
					fprintf(stderr, "Error: Height definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_height){
					fprintf(stderr, "Warning: Duplicate Height definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Height: %f", &NewLayer.Height);
					got_height = true;
				}
			}else if(strstr(line, "Thickness:")){
				if(!in_layer){
					fprintf(stderr, "Error: Thickness definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_thickness){
					fprintf(stderr, "Warning: Duplicate Thickness definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Thickness: %f", &NewLayer.Thickness);
					got_thickness = true;
				}
			}else if(strstr(line, "Red:")){
				if(!in_layer){
					fprintf(stderr, "Error: Red definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_red){
					fprintf(stderr, "Warning: Duplicate Red definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Red: %f", &NewLayer.Red);
					got_red = true;
				}
			}else if(strstr(line, "Green:")){
				if(!in_layer){
					fprintf(stderr, "Error: Green definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_green){
					fprintf(stderr, "Warning: Duplicate Green definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Green: %f", &NewLayer.Green);
					got_green = true;
				}
			}else if(strstr(line, "Blue:")){
				if(!in_layer){
					fprintf(stderr, "Error: Blue definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_blue){
					fprintf(stderr, "Warning: Duplicate Blue definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Blue: %f", &NewLayer.Blue);
					got_blue = true;
				}
			}else if(strstr(line, "Filter:")){
				if(!in_layer){
					fprintf(stderr, "Error: Filter definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_filter){
					fprintf(stderr, "Warning: Duplicate Filter definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Filter: %f", &NewLayer.Filter);
					got_filter = true;
				}
			}else if(strstr(line, "Metal:")){
				if(!in_layer){
					fprintf(stderr, "Error: Metal definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_metal){
					fprintf(stderr, "Warning: Duplicate Metal definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Metal: %d", &NewLayer.Metal);
					got_metal = true;
				}
			}else if(strstr(line, "Show:")){
				if(!in_layer){
					fprintf(stderr, "Error: Show definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_show){
					fprintf(stderr, "Warning: Duplicate Show definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Show: %d", &NewLayer.Show);
					got_show = true;
				}
			}else if(strstr(line, "LayerEnd")){
				showing = NewLayer.Show;
				if(!in_layer){
					fprintf(stderr, "Error: LayerEnd without LayerStart on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}else if(!got_layer){
					fprintf(stderr, "Error: LayerEnd without Layer on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}else if(!got_height){
					//v_printf(1, "Error: LayerEnd without Height on line %d of process file.\n", current_line);
					showing = false;
				}else if(!got_thickness){
//					v_printf(1, "Error: LayerEnd without Thickness on line %d of process file.\n", current_line);
					showing = false;
				}
				AddLayer(&NewLayer);
					if(!showing){
						if(NewLayer.Datatype == -1){
							v_printf(1, "Notice: Not showing layer %d (all datatypes)\n", NewLayer.Layer);
						}else{
							v_printf(1, "Notice: Not showing layer %d datatype %d\n", NewLayer.Layer, NewLayer.Datatype);
						}
					}

				if(NewLayer.Name){
					delete [] NewLayer.Name;
					NewLayer.Name = NULL;
				}
				in_layer = false;
			}
		}
	}
	if(NewLayer.Name){
		delete [] NewLayer.Name;
		NewLayer.Name = NULL;
	}
	fclose(pptr);
	v_printf(1, "\n");
}

struct ProcessLayer *GDSProcess::GetLayer(int Number, int Datatype)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	if(Number == -1) return NULL;

	while(layer){
		if(layer->Layer == Number && layer->Datatype == -1){
			return layer;
		}else if(layer->Layer == Number && layer->Datatype == Datatype){
			return layer;
		}
		layer = layer->Next;
	}
	return NULL;
}

struct ProcessLayer *GDSProcess::GetLayer(const char *Name)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	while(layer){
		if(strcmp(Name, layer->Name) == 0){
			return layer;
		}
		layer = layer->Next;
	}
	return NULL;
}

struct ProcessLayer *GDSProcess::GetLayer()
{
	return _FirstLayer;
}

int GDSProcess::LayerCount()
{
	return _Count;
}


void GDSProcess::AddLayer(int Layer, int Datatype)
{
	struct ProcessLayer NewLayer;

	NewLayer.Name = NULL;
	NewLayer.Layer = Layer;
	NewLayer.Datatype = Datatype;
	NewLayer.Height = 0.0;
	NewLayer.Thickness = 0.0;
	NewLayer.Red = 0.0;
	NewLayer.Green = 0.0;
	NewLayer.Blue = 0.0;
	NewLayer.Filter = 0.0;
	NewLayer.Metal = 0;
	NewLayer.Show = false;
	NewLayer.Next = NULL;

	AddLayer(&NewLayer);
}


void GDSProcess::AddLayer(struct ProcessLayer *NewLayer)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	if(_FirstLayer){
		while(layer->Next){
			layer = layer->Next;
		}
		layer->Next = new struct ProcessLayer;
		layer = layer->Next;
		layer->Next = NULL;
	}else{
		_FirstLayer = new struct ProcessLayer;
		layer = _FirstLayer;
		layer->Next = NULL;
	}

	layer->Name = NULL;
	if(NewLayer->Name){
		layer->Name = new char[strlen(NewLayer->Name)+1];
		strcpy(layer->Name, NewLayer->Name);
	}
	layer->Layer = NewLayer->Layer;
	layer->Datatype = NewLayer->Datatype;
	layer->Height = NewLayer->Height;
	layer->Thickness = NewLayer->Thickness;
	layer->Show = NewLayer->Show;
	layer->Red = NewLayer->Red;
	layer->Green = NewLayer->Green;
	layer->Blue = NewLayer->Blue;
	layer->Filter = NewLayer->Filter;
	layer->Metal = NewLayer->Metal;
}

bool GDSProcess::IsValid()
{
	return _Valid;
}

float GDSProcess::GetHighest()
{
	float Highest = -10000.0;
	struct ProcessLayer *layer;

	layer = _FirstLayer;
	while(layer){
		if(layer->Height + layer->Thickness > Highest && layer->Show){
			Highest = layer->Height + layer->Thickness;
		}
		layer = layer->Next;
	}
	return Highest;
}

float GDSProcess::GetLowest()
{
	float Lowest = 10000.0;
	struct ProcessLayer *layer;

	layer = _FirstLayer;
	while(layer){
		if(layer->Height < Lowest && layer->Show){
			Lowest = layer->Height;
		}
		layer = layer->Next;
	}
	return Lowest;
}


bool GDSProcess::Save(const char *filename)
{
	struct ProcessLayer *layer;
	FILE *fptr = NULL;

	if(!filename) return false;

	fptr = fopen(filename, "wt");
	if(!fptr) return false;

	layer = _FirstLayer;
	while(layer){
		fprintf(fptr, "LayerStart: LAYER-%d-%d\n", layer->Layer, layer->Datatype);
		fprintf(fptr, "Layer: %d\n", layer->Layer);
		fprintf(fptr, "Height: 0\n");
		fprintf(fptr, "Thickness: 0\n");
		fprintf(fptr, "Red: 0.0\n");
		fprintf(fptr, "Green: 0.0\n");
		fprintf(fptr, "Blue: 0.0\n");
		fprintf(fptr, "Filter: 0.0\n");
		fprintf(fptr, "Metal: 0\n");
		fprintf(fptr, "Show: 1\n");
		fprintf(fptr, "LayerEnd\n\n");

		layer = layer->Next;
	}
	fclose(fptr);

	return true;
}

