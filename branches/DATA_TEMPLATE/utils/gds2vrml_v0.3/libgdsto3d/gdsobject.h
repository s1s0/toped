/*
 * File: gdsobject.h
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

#ifndef __GDSOBJECT_H__
#define __GDSOBJECT_H__

#include <cstdio>
#include <vector>
using namespace std;

#include "process_cfg.h"
#include "gdselements.h"
#include "gdspath.h"
#include "gdstext.h"
#include "gdspolygon.h"

class GDSObject
{
protected:
	vector<class GDSPath*> PathItems;
	vector<class GDSText*> TextItems;
	vector<class GDSPolygon*> PolygonItems;

	SRefElement *FirstSRef;
	SRefElement *LastSRef;
	ARefElement *FirstARef;
	ARefElement *LastARef;
	bool GotBoundary;
	bool IsOutput;
	int SRefCount, ARefCount;

	char *Name;

	struct _Boundary Boundary;
	float _width, _height;

	class GDSObject **SRefs;
	class GDSObject **ARefs;
public:
	GDSObject(char *Name);
	virtual ~GDSObject();

	void AddText(float newX, float newY, float newZ, bool newFlipped, float newMag, int newVJust, int newHJust, struct ProcessLayer *newlayer);
	class GDSText *GetCurrentText();

	void AddPolygon(float Height, float Thickness, int Points, struct ProcessLayer *layer);
	class GDSPolygon *GetCurrentPolygon();

	void AddSRef(char *Name, float X, float Y, int Flipped, float Mag);
	void SetSRefRotation(float X, float Y, float Z);

	void AddARef(char *Name, float X1, float Y1, float X2, float Y2, float X3, float Y3, int Columns, int Rows, int Flipped, float Mag);
	void SetARefRotation(float X, float Y, float Z);

	void AddPath(int PathType, float Height, float Thickness, int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *layer);
	class GDSPath *GetCurrentPath();

	char *GetName();

	virtual void OutputToFile(FILE *fptr, class GDSObjects *Objects, char *Font, float offx, float offy, long *objectid, struct ProcessLayer *firstlayer)=0;

	int HasASRef();
	class GDSObject *GetSRef(class GDSObjects *Objects, int Index);
	class GDSObject *GetARef(class GDSObjects *Objects, int Index);
	void IndexSRefs(class GDSObjects *Objects);
	void IndexARefs(class GDSObjects *Objects);

	struct _Boundary *GetBoundary(struct ObjectList *objectlist);
	float GetWidth();
	float GetHeight();

	bool GetIsOutput();
};

typedef class GDSObject * GDSObjectRef;

#endif // __GDSOBJECT_H__

