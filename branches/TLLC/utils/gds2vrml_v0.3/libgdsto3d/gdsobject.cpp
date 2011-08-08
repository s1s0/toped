/*
 * File: gdsobject.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the GDSObject class which corresponds to a GDS SRef.
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

#include "gdsobject.h"
#include "gdsobjects.h"
#include "gds_globals.h"

GDSObject::GDSObject(char *NewName)
{
	FirstSRef = NULL;
	LastSRef = NULL;
	FirstARef = NULL;
	LastARef = NULL;

	SRefCount = 0;
	ARefCount = 0;
	SRefs = NULL;
	ARefs = NULL;

	Name = new char[strlen(NewName)+1];
	strcpy(Name, NewName);

	GotBoundary = false;
	Boundary.XMax = Boundary.YMax = -1000000.0;
	Boundary.XMin = Boundary.YMin =  1000000.0;
	_width = 0.0;
	_height = 0.0;

	IsOutput = false;
}

GDSObject::~GDSObject()
{
	while(!PolygonItems.empty()){
		delete PolygonItems[PolygonItems.size()-1];
		PolygonItems.pop_back();
	}

	while(!PathItems.empty()){
		delete PathItems[PathItems.size()-1];
		PathItems.pop_back();
	}

	while(!TextItems.empty()){
		delete TextItems[TextItems.size()-1];
		TextItems.pop_back();
	}

	if(FirstSRef){
		SRefElement *sref1;
		SRefElement *sref2;

		sref1 = FirstSRef;
		while(sref1->Next){
			sref2 = sref1->Next;
			if(sref1->Name) delete [] sref1->Name;
			delete sref1;
			sref1 = sref2;
		}
		if(sref1){
			if(sref1->Name) delete [] sref1->Name;
			delete sref1;
		}
	}

	if(FirstARef){
		ARefElement *aref1;
		ARefElement *aref2;

		aref1 = FirstARef;
		while(aref1->Next){
			aref2 = aref1->Next;
			if(aref1->Name) delete [] aref1->Name;
			delete aref1;
			aref1 = aref2;
		}
		if(aref1){
			if(aref1->Name) delete [] aref1->Name;
			delete aref1;
		}
	}

	if(SRefs){
		delete [] SRefs;
	}

	if(ARefs){
		delete [] ARefs;
	}

	delete [] Name;
}

void GDSObject::AddText(float newX, float newY, float newZ, bool newFlipped, float newMag, int newVJust, int newHJust, struct ProcessLayer *newlayer)
{
	TextItems.push_back(new class GDSText(newX, newY, newZ, newFlipped, newMag, newVJust, newHJust, newlayer));
}

class GDSText *GDSObject::GetCurrentText()
{
	if(TextItems.size()){
		return TextItems[TextItems.size()-1];
	}else{
		return NULL;
	}
}

char *GDSObject::GetName()
{
	return Name;
}

void GDSObject::AddPolygon(float Height, float Thickness, int Points, struct ProcessLayer *layer)
{
	PolygonItems.push_back(new class GDSPolygon(Height, Thickness, Points, layer));
}

class GDSPolygon *GDSObject::GetCurrentPolygon()
{
	return PolygonItems[PolygonItems.size()-1];
}

void GDSObject::AddSRef(char *Name, float X, float Y, int Flipped, float Mag)
{
	SRefElement *NewSRef = new SRefElement;

	NewSRef->Next = NULL;
	NewSRef->Name = NULL;

	if(LastSRef){
		LastSRef->Next = NewSRef;
		LastSRef = NewSRef;
	}else{
		FirstSRef = NewSRef;
		LastSRef = NewSRef;
	}

	NewSRef->Name = new char[strlen(Name)+1];
	strcpy(NewSRef->Name, Name);
	NewSRef->X = X;
	NewSRef->Y = Y;
	NewSRef->Rotate.X = 0.0;
	NewSRef->Rotate.Y = 0.0;
	NewSRef->Rotate.Z = 0.0;
	NewSRef->Flipped = Flipped;
	NewSRef->Mag = Mag;
	NewSRef->object = NULL;

	SRefCount++;
}

void GDSObject::SetSRefRotation(float X, float Y, float Z)
{
	if(LastSRef){
		LastSRef->Rotate.X = X;
		LastSRef->Rotate.Y = Y;
		LastSRef->Rotate.Z = Z;
	}
}

void GDSObject::AddARef(char *Name, float X1, float Y1, float X2, float Y2, float X3, float Y3, int Columns, int Rows, int Flipped, float Mag)
{
	ARefElement *NewARef = new ARefElement;

	NewARef->Next = NULL;
	NewARef->Name = NULL;

	if(LastARef){
		LastARef->Next = NewARef;
		LastARef = NewARef;
	}else{
		FirstARef = NewARef;
		LastARef = NewARef;
	}

	NewARef->Name = new char[strlen(Name)+1];
	strcpy(NewARef->Name, Name);
	NewARef->X1 = X1;
	NewARef->Y1 = Y1;
	NewARef->X2 = X2;
	NewARef->Y2 = Y2;
	NewARef->X3 = X3;
	NewARef->Y3 = Y3;
	NewARef->Columns = Columns;
	NewARef->Rows = Rows;
	NewARef->Rotate.X = 0.0;
	NewARef->Rotate.Y = 0.0;
	NewARef->Rotate.Z = 0.0;
	NewARef->Flipped = Flipped;
	NewARef->Mag = Mag;
	NewARef->object = NULL;

	ARefCount++;
}

void GDSObject::SetARefRotation(float X, float Y, float Z)
{
	if(LastARef){
		LastARef->Rotate.X = X;
		LastARef->Rotate.Y = Y;
		LastARef->Rotate.Z = Z;
	}
}

struct _Boundary *GDSObject::GetBoundary(struct ObjectList *objectlist)
{
	if(GotBoundary){
		return &Boundary;
	}

	struct ObjectList dummyobject;

	if(!PolygonItems.empty()){
		class GDSPolygon *polygon;
		for(unsigned long i=0; i<PolygonItems.size(); i++){
			polygon = PolygonItems[i];
			for(unsigned int j=0; j<polygon->GetPoints(); j++){
				if(polygon->GetXCoords(j) > Boundary.XMax){
					Boundary.XMax = polygon->GetXCoords(j);
				}
				if(polygon->GetXCoords(j) < Boundary.XMin){
					Boundary.XMin = polygon->GetXCoords(j);
				}
				if(polygon->GetYCoords(j) > Boundary.YMax){
					Boundary.YMax = polygon->GetYCoords(j);
				}
				if(polygon->GetYCoords(j) < Boundary.YMin){
					Boundary.YMin = polygon->GetYCoords(j);
				}
			}
		}
	}

	/* FIXME - need to take width into account? */
	if(!PathItems.empty()){
		class GDSPath *path;
		for(unsigned long i=0; i<PathItems.size(); i++){
			path = PathItems[i];
			for(unsigned int j=0; j<path->GetPoints(); j++){
				if(path->GetXCoords(j) > Boundary.XMax){
					Boundary.XMax = path->GetXCoords(j);
				}
				if(path->GetXCoords(j) < Boundary.XMin){
					Boundary.XMin = path->GetXCoords(j);
				}
				if(path->GetYCoords(j) > Boundary.YMax){
					Boundary.YMax = path->GetYCoords(j);
				}
				if(path->GetYCoords(j) < Boundary.YMin){
					Boundary.YMin = path->GetYCoords(j);
				}
			}
		}
	}

	if(FirstSRef){
		SRefElement dummysref;
		dummysref.Next=FirstSRef;
		SRefElement *sref = &dummysref;

		struct ObjectList *object;
		dummyobject.Next = objectlist;
		struct _Boundary *NewBound;

		while(sref->Next){
			sref = sref->Next;
			if(strcmp(sref->Name, this->Name)!=0){
				object = &dummyobject;

				while(object->Next){
					object = object->Next;
					if(strcmp(object->Object->GetName(), sref->Name)==0){
						NewBound = object->Object->GetBoundary(objectlist);
						if(sref->X + NewBound->XMax > Boundary.XMax){
							Boundary.XMax = sref->X + NewBound->XMax;
						}
						if(sref->X - NewBound->XMin < Boundary.XMin){
							Boundary.XMin = sref->X - NewBound->XMin;
						}
						if(sref->Y + NewBound->YMax > Boundary.YMax){
							Boundary.YMax = sref->Y + NewBound->YMax;
						}
						if(sref->Y - NewBound->YMin < Boundary.YMin){
							Boundary.YMin = sref->Y - NewBound->YMin;
						}
						break;
					}
				}
			}
		}
	}

	if(FirstARef){
		ARefElement dummyaref;
		dummyaref.Next = FirstARef;

		ARefElement *aref = &dummyaref;

		struct ObjectList *object;
		dummyobject.Next = objectlist;
		
		struct _Boundary *NewBound;
		while(aref->Next){
			aref = aref->Next;
			if(strcmp(aref->Name, this->Name)!=0){
				object = &dummyobject;
				object = &dummyobject;
				while(object->Next){
					object = object->Next;
					if(strcmp(object->Object->GetName(), aref->Name)==0){
						NewBound = object->Object->GetBoundary(objectlist);
						if(aref->X2 + NewBound->XMax > Boundary.XMax){
							Boundary.XMax = aref->X2 + NewBound->XMax;
						}
						if(aref->X1 - NewBound->XMin < Boundary.XMin){
							Boundary.XMin = aref->X1 - NewBound->XMin;
						}
						if(aref->Y3 + NewBound->YMax > Boundary.YMax){
							Boundary.YMax = aref->Y3 + NewBound->YMax;
						}
						if(aref->Y1 - NewBound->YMin < Boundary.YMin){
							Boundary.YMin = aref->Y1 - NewBound->YMin;
						}
						break;
					}
				}
			}
		}
	}

	if(PathItems.empty() && PolygonItems.empty() && !FirstSRef && !FirstARef){
		Boundary.XMax = Boundary.XMin = Boundary.YMax = Boundary.YMin = 0;
	}

	v_printf(2, "%s\tXMax=%.2f\tXMin=%.2f\tYMax: %.2f\tYMin: %.2f\n", Name, Boundary.XMax, Boundary.XMin, Boundary.YMax, Boundary.YMin);
	GotBoundary = true;

	_width = Boundary.XMax - Boundary.XMin;
	_height = Boundary.YMax - Boundary.YMin;

	return &Boundary;
}

void GDSObject::AddPath(int PathType, float Height, float Thickness, int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *layer)
{
	PathItems.push_back(new class GDSPath(PathType, Height, Thickness, Points, Width, BgnExtn, EndExtn, layer));
}

class GDSPath *GDSObject::GetCurrentPath()
{
	return PathItems[PathItems.size()-1];
}

int GDSObject::HasASRef()
{
	return (LastARef || LastSRef);
}

void GDSObject::IndexSRefs(class GDSObjects *Objects)
{
	if(!FirstSRef) return;

	SRefElement *sref;
	if(SRefs){
		delete [] SRefs;
		SRefs = NULL;
	}

	SRefs = new GDSObjectRef[SRefCount];

	sref = FirstSRef;
	int i=0;
	while(sref->Next){
		SRefs[i] = Objects->GetObjectRef(sref->Name);
		i++;
		sref = sref->Next;
	}
	SRefs[i] = Objects->GetObjectRef(sref->Name);
}

void GDSObject::IndexARefs(class GDSObjects *Objects)
{
	if(!FirstARef) return;

	ARefElement *aref;
	if(ARefs){
		delete [] ARefs;
		ARefs = NULL;
	}

	ARefs = new GDSObjectRef[ARefCount];

	aref = FirstARef;
	int i=0;
	while(aref->Next){
		ARefs[i] = Objects->GetObjectRef(aref->Name);
		i++;
		aref = aref->Next;
	}
	ARefs[i] = Objects->GetObjectRef(aref->Name);
}

class GDSObject *GDSObject::GetSRef(class GDSObjects *Objects, int Index)
{
	if(!SRefs && FirstSRef){
		IndexSRefs(Objects);
	}
	if(FirstSRef && Index<SRefCount){
		return SRefs[Index];
	}
	return NULL;
}

class GDSObject *GDSObject::GetARef(class GDSObjects *Objects, int Index)
{
	if(!ARefs && FirstARef){
		IndexARefs(Objects);
	}
	if(FirstARef && Index<ARefCount){
		return ARefs[Index];
	}
	return NULL;
}

bool GDSObject::GetIsOutput()
{
	return IsOutput;
}

float GDSObject::GetWidth()
{
	return _width;
}

float GDSObject::GetHeight()
{
	return _height;
}

