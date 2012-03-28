/*
 * File: gdspolygon.h
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the GDSPolyon class which is used to represent the GDS boundary
 * object.
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

#include "gdsobject.h"
#include "gdspolygon.h"

GDSPolygon::GDSPolygon(float Height, float Thickness, unsigned int Points, struct ProcessLayer *Layer)
{
	_Coords = NULL;
	_Coords = new Point[Points+1]; //FIXME - debug +1
	_Height = Height;
	_Thickness = Thickness;
	_Points = Points;
	_Layer = Layer;
}

GDSPolygon::~GDSPolygon()
{
	if(_Coords) delete [] _Coords;
}

void GDSPolygon::AddPoint(unsigned int Index, float X, float Y)
{
	if(_Points >= Index){
		_Coords[Index].X = X;
		_Coords[Index].Y = Y;
	}
}


void GDSPolygon::SetRotation(float X, float Y, float Z)
{
	_Rotate.X = X;
	_Rotate.Y = Y;
	_Rotate.Z = Z;
}

float GDSPolygon::GetXCoords(unsigned int Index)
{
	return _Coords[Index].X;
}

float GDSPolygon::GetYCoords(unsigned int Index)
{
	return _Coords[Index].Y;
}

float GDSPolygon::GetAngleCoords(unsigned int Index)
{
	return _Coords[Index].Angle;
}

void GDSPolygon::SetAngleCoords(unsigned int Index, float Value)
{
	_Coords[Index].Angle = Value;
}

unsigned int GDSPolygon::GetPoints()
{
	return _Points;
}

float GDSPolygon::GetHeight()
{
	return _Height;
}

float GDSPolygon::GetThickness()
{
	return _Thickness;
}

struct ProcessLayer *GDSPolygon::GetLayer()
{
	return _Layer;
}



