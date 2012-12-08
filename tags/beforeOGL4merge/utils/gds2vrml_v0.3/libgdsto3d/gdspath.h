/*
 * File: gdspath.h
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

#ifndef __GDSPATH_H__
#define __GDSPATH_H__

#include "process_cfg.h"

class GDSPath
{
private:
	int 			_Type;
	float			_Height;
	float			_Thickness;
	unsigned int		_Points;
	float			_Width;
	float			_BgnExtn;
	float			_EndExtn;
	Point			*_Coords;
	Transform		_Rotate;
	struct ProcessLayer	*_Layer;

public:
	GDSPath(int PathType, float Height, float Thickness, unsigned int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *layer);
	~GDSPath();

	void AddPoint(unsigned int Index, float X, float Y);
	void SetRotation(float X, float Y, float Z);

	float GetXCoords(unsigned int Index);
	float GetYCoords(unsigned int Index);
	unsigned int GetPoints();

	float GetHeight();
	float GetThickness();
	float GetWidth();
	float GetBgnExtn();
	float GetEndExtn();

	int GetType();
	struct ProcessLayer *GetLayer();
};

#endif // __GDSPATH_H__

