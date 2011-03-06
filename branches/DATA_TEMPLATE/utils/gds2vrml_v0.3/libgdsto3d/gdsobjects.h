/*
 * File: gdsobjects.h
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

#ifndef __GDSOBJECTS_H__
#define __GDSOBJECTS_H__

#include "gdsobject.h"

struct ObjectList{
	struct ObjectList *Next;
	struct ObjectList *Prev;
	class GDSObject *Object;
};

class GDSObjects
{
private:
	struct ObjectList *FirstObject;
	struct ObjectList *LastObject;
	struct _Boundary *Boundary;
	int Count;

public:
	GDSObjects();
	~GDSObjects();

	class GDSObject *AddObject(char *Name, class GDSObject *newobject);
	class GDSObject *GetObjectRef(int Index);
	class GDSObject *GetObjectRef(char *Name);
	struct ObjectList *GetObjectList();
	struct _Boundary *GetBoundary();
	int GetCount();
};

#endif // __GDSOBJECTS_H__
