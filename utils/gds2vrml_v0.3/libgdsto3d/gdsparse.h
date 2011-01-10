/*
 * File: gdsparse.h
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

#ifndef __GDSPARSE_H__
#define __GDSPARSE_H__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_STDINT_H
#  include <stdint.h>
typedef uint16_t u_int16_t;
#endif

#include <sys/types.h>

#include "config_cfg.h"
#include "process_cfg.h"
#include "gds_globals.h"
#include "gdsobject.h"
#include "gdsobjects.h"
#include "gds_types.h"

typedef enum{
	cpCentre,
	cpBottomLeft,
	cpBottomRight,
	cpTopLeft,
	cpTopRight
} CameraPosition;

class GDSParse
{
protected:
	char			*_libname;
	char			*_topcellname;

	int16_t			_currentlayer;
	float			_currentwidth;
	int16_t			_currentpathtype;
	gds_element_type	_currentelement;
	int16_t			_currenttexttype;
	int16_t			_currentpresentation;
	char			*_textstring;
	int16_t			_currentstrans;
	float			_currentangle;
	int16_t			_currentdatatype;
	float			_currentmag;
	float			_currentbgnextn;
	float			_currentendextn;

	char			*_sname;
	int16_t			_arrayrows, _arraycols;
	float			_units;
	float			_angle;
	FILE			*_iptr;
	FILE			*_optr;
	class GDSProcess	*_process;
	class GDSConfig		*_config;
	
	int16_t			_recordlen;

	/* Output options */
	bool			_allow_multiple_output;
	bool			_output_children_first;
	bool			_bounding_output;
	bool			_use_outfile;
	bool			_generate_process;

	/*
	** Both of these variables have fixed bounds because
	** they are not dependant on the GDS2 spec, not on the
	** file we are parsing.
	** There will never be more than 70 records.
	** The maximum layer and datatype are both defined as
	** 255, but but could be as high as 32,767 because of
	** the way they are stored (2 byte int). It might be worth
	** checking if they are greater than 255
	*/
	bool			_unsupported[70];
	bool			_layer_warning[256][256];

	long			_PathElements;
	long			_BoundaryElements;
	long			_BoxElements;
	long			_TextElements;
	long			_SRefElements;
	long			_ARefElements;

	class GDSObjects	*_Objects;
	class GDSObject		*_CurrentObject;

	/* gds_parse.h functions */
	void ParseHeader();
	void ParseLibName();
	void ParseSName();
	void ParseUnits();
	void ParseStrName();
	void ParseXY();
	void ParseXYPath();
	void ParseXYBoundary();
	void ParseSTrans();

	void HandleSRef();
	void HandleARef();
	void HandleBoundary();
	void HandlePath();

	short GetBitArray();
	double GetEightByteReal();
	int32_t GetFourByteSignedInt();
	int16_t GetTwoByteSignedInt();
	char *GetAsciiString();

	void ReportUnsupported(const char *Name, enum RecordNumbers rn);
	
	bool ParseFile();

	/* Abstract functions to be implemented be inheriting class */
	virtual void OutputHeader() = 0;
	virtual void OutputFooter() = 0;
	/* End abstract functions */

	void RecursiveOutput(class GDSObject *Object, FILE *optr, float offx, float offy, long *objectid);
public:
	GDSParse (class GDSConfig *config, class GDSProcess *process, bool generate_process);
	virtual ~GDSParse ();

	void SetOutputOptions(bool bounding_output, bool use_outfile, bool allow_multiple_output, bool output_children_first);
	bool Parse(FILE *iptr);
	void Output(FILE *optr, char *topcell);
	virtual class GDSObject *NewObject(char *Name) = 0;
};

#endif // __GDSPARSE_H__

