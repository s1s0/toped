/*
 * File: gdspolygon.h
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

#ifndef __GDSPOLYGON_H__
#define __GDSPOLYGON_H__

#include "process_cfg.h"

class GDSPolygon
{
private:
	float			_Height;
	float			_Thickness;
	unsigned int		_Points;
	Point			*_Coords;
	Transform		_Rotate;
	struct ProcessLayer	*_Layer;

public:
	GDSPolygon(float Height, float Thickness, unsigned int Points, struct ProcessLayer *Layer);
	~GDSPolygon();

	void AddPoint(unsigned int Index, float X, float Y);
	void SetRotation(float X, float Y, float Z);

	float GetHeight();
	float GetThickness();
	unsigned int GetPoints();
	float GetXCoords(unsigned int Index);
	float GetYCoords(unsigned int Index);
	float GetAngleCoords(unsigned int Index);
	void SetAngleCoords(unsigned int Index, float value);
	struct ProcessLayer *GetLayer();
};

#endif // __GDSPOLYGON_H__
