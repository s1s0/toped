/*
 * File: gdsobjects.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the GDSObjects class which is a container class for the GDSObject
 * class.
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

#include <cstring>

#include "gdsobjects.h"

GDSObjects::GDSObjects()
{
	FirstObject = NULL;
	LastObject = NULL;
	Count = 0;
	Boundary = NULL;
}

GDSObjects::~GDSObjects()
{
	struct ObjectList *object;

	object = FirstObject;

	if(object){
		while(object->Next){
			object = object->Next;
			delete object->Prev->Object;
			delete object->Prev;
		}
		delete object->Object;
		delete object;
	}
	if(Boundary){
		delete Boundary;
	}
}

class GDSObject *GDSObjects::AddObject(char *Name, class GDSObject *newobject)
{
	struct ObjectList *object = new struct ObjectList;
	//object->Object = new class GDSObject(Name);
	object->Object = newobject;
	if(FirstObject){
		LastObject->Next = object;
		object->Prev = LastObject;
		LastObject = object;
		LastObject->Next = NULL;
	}else{
		FirstObject = object;
		LastObject = object;
		object->Next = NULL;
		object->Prev = NULL;
	}
	Count++;
	return object->Object;
}

class GDSObject *GDSObjects::GetObjectRef(int Index)
{
	if(FirstObject && Index<Count){
		struct ObjectList *object = FirstObject;
		for(int i=0; i<Index && object; i++){
			object = object->Next;
		}
		return object->Object;
	}else{
		return NULL;
	}
}

class GDSObject *GDSObjects::GetObjectRef(char *Name)
{
	if(FirstObject && Name){	
		struct ObjectList *object = FirstObject;

		while(object->Next){
			if(strcmp(Name, object->Object->GetName())==0){
				return object->Object;
			}
			//printf ("****** CELL: %s\n", object->Object->GetName());
			object = object->Next;
		}
		if(strcmp(Name, object->Object->GetName())==0){
			return object->Object;
		}
	}
	//printf ("****** nothing found\n");
	return NULL;
}


int GDSObjects::GetCount()
{
	return Count;
}

struct _Boundary *GDSObjects::GetBoundary()
{
	if(!Boundary){
		Boundary = new struct _Boundary;
	}

	Boundary->XMax = Boundary->YMax = -10000000.0;
	Boundary->XMin = Boundary->YMin =  10000000.0;

	if(FirstObject){
		struct ObjectList *objectlist = LastObject;
		struct _Boundary *object_bound;

		while(objectlist->Prev){
			object_bound = objectlist->Object->GetBoundary(FirstObject);

			if(object_bound->XMax > Boundary->XMax){
				Boundary->XMax = object_bound->XMax;
			}
			if(object_bound->XMin < Boundary->XMin){
				Boundary->XMin = object_bound->XMin;
			}
			if(object_bound->YMax > Boundary->YMax){
				Boundary->YMax = object_bound->YMax;
			}
			if(object_bound->YMin < Boundary->YMin){
				Boundary->YMin = object_bound->YMin;
			}

			objectlist = objectlist->Prev;
		}
		object_bound = objectlist->Object->GetBoundary(FirstObject);

		if(object_bound->XMax > Boundary->XMax){
			Boundary->XMax = object_bound->XMax;
		}
		if(object_bound->XMin < Boundary->XMin){
			Boundary->XMin = object_bound->XMin;
		}
		if(object_bound->YMax > Boundary->YMax){
			Boundary->YMax = object_bound->YMax;
		}
		if(object_bound->YMin < Boundary->YMin){
			Boundary->YMin = object_bound->YMin;
		}
	}
	return Boundary;
}

struct ObjectList *GDSObjects::GetObjectList()
{
	return FirstObject;
}

