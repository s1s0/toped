/*
 * File: gdsparse.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the GDSParse class which is used to parse a GDS file and create a
 * GDSObjects object containing the contents of the file according to the 
 * process configuration.
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

#include <cmath>
#include <cstring>

#include "gdsparse.h"

extern int verbose_output;

GDSParse::GDSParse (class GDSConfig *config, class GDSProcess *process, bool generate_process)
{
	_iptr = NULL;
	_optr = NULL;
	_libname = NULL;
	_sname = NULL;
	_textstring = NULL;
	_Objects = NULL;

	_PathElements = 0;
	_BoundaryElements = 0;
	_BoxElements = 0;
	_TextElements = 0;
	_SRefElements = 0;
	_ARefElements = 0;

	_currentangle = 0.0;
	_currentwidth = 0.0;
	_currentstrans = 0;
	_currentpathtype = 0;
	_currentlayer = -1;
	_currentdatatype = -1;
	_currentmag = 1.0;
	_currentbgnextn = 0.0;
	_currentendextn = 0.0;
	_currenttexttype = 0;
	_currentpresentation = 0;
//	_currentelement = 0;
	_arrayrows = 0;
	_arraycols = 0;
	_angle = 0.0;
	_units = 0.0;
	_recordlen = 0;
	_CurrentObject = NULL;

	_bounding_output = false;
	_use_outfile = false;
	_allow_multiple_output = false;
	_output_children_first = false;
	_generate_process = generate_process;

	_process = process;
	_config = config;

	for(int i=0; i<70; i++){
		_unsupported[i] = false;
	}
	for(int i=0; i<256; i++){
		for(int j=0; j<256; j++){
			_layer_warning[i][j] = false;
		}
	}
}

GDSParse::~GDSParse ()
{
	if(_libname){
		delete [] _libname;
	}
	if(_sname){
		delete [] _sname;
	}
	if(_textstring){
		delete [] _textstring;
	}
	if(_Objects){
		delete _Objects;
	}
}

bool GDSParse::Parse(FILE *iptr)
{
	_iptr = iptr;
	if(_iptr){
		_Objects = new GDSObjects;
		
		//DEBUG
		//printf("GDSParse::Parse(%p)\n",iptr);

		bool result = ParseFile();

		v_printf(1, "\nSummary:\n\tPaths:\t\t%ld\n\tBoundaries:\t%ld\n\tBoxes:\t\t%ld\n\tStrings:\t%ld\n\tStuctures:\t%ld\n\tArrays:\t\t%ld\n",
			_PathElements, _BoundaryElements, _BoxElements, _TextElements, _SRefElements, _ARefElements);

		return result;
	}else{
		return -1;
	}
}

void GDSParse::Output(FILE *optr, char *topcell)
{
	_topcellname = topcell;

	if(_use_outfile){
		_optr = optr;
	}
	if(_optr || !_use_outfile){
		OutputHeader();

		if(!_bounding_output){
			long objectid = 0;
			if(topcell){
				RecursiveOutput(_Objects->GetObjectRef(topcell), _optr, 0.0, 0.0, &objectid);
			}else{
				RecursiveOutput(_Objects->GetObjectRef(0), _optr, 0.0, 0.0, &objectid);
			}
		}

		OutputFooter();
	}
}

void GDSParse::SetOutputOptions(bool bounding_output, bool use_outfile, bool allow_multiple_output, bool output_children_first)
{
	_bounding_output = bounding_output;
	_use_outfile = use_outfile;
	_allow_multiple_output = allow_multiple_output;
	_output_children_first = output_children_first;
}

void GDSParse::RecursiveOutput(class GDSObject *Object, FILE *_optr, float offx, float offy, long *objectid)
{
	if(!Object){
		return;
	}
	
	if(Object->GetIsOutput() && _allow_multiple_output == false){
		return;
	}

	if(_output_children_first && Object->HasASRef()){
		GDSObject *child;

		int i=0;
		do{
			child = Object->GetSRef(_Objects, i);
			if(child && (child != Object)){
				RecursiveOutput(child, _optr, offx, offy, objectid);
			}

			i++;
		}while(child);

		i = 0;
		do{
			child = Object->GetARef(_Objects, i);
			if(child && (child != Object)){
				RecursiveOutput(child, _optr, offx, offy, objectid);
			}
			i++;
		}while(child);

	}

	struct ProcessLayer *layer = NULL;
	if(_process != NULL){
		layer = _process->GetLayer();
	}

	Object->OutputToFile(_optr, _Objects, _config->GetFont(), offx, offy, objectid, layer);
}

bool GDSParse::ParseFile()
{
	byte recordtype, datatype;
	char *tempstr;
	struct ProcessLayer *layer = NULL;

	if(!_iptr){
		return -1;
	}

	fseek(_iptr, 0, SEEK_SET);
	while(!feof(_iptr)){
		_recordlen = GetTwoByteSignedInt();
		fread(&recordtype, 1, 1, _iptr);
		fread(&datatype, 1, 1, _iptr);
		_recordlen -= 4;
		switch(recordtype){
			case rnHeader:
				v_printf(2, "HEADER\n");
				ParseHeader();
				break;
			case rnBgnLib:
				v_printf(2, "BGNLIB\n");
				while(_recordlen){
					GetTwoByteSignedInt();
				}
				break;
			case rnLibName:
				v_printf(2, "LIBNAME ");
				ParseLibName();
				break;
			case rnUnits:
				v_printf(2, "UNITS\n");
				ParseUnits();
				break;
			case rnEndLib:
				v_printf(2, "ENDLIB\n");
				fseek(_iptr, 0, SEEK_END);
				return 0;
				break;
			case rnEndStr:
				v_printf(2, "ENDSTR\n");
				break;
			case rnEndEl:
				v_printf(2, "ENDEL\n\n");
				/* Empty, no need to parse */
				break;
			case rnBgnStr:
				v_printf(2, "BGNSTR\n");
				while(_recordlen){
					GetTwoByteSignedInt();
				}
				break;
			case rnStrName:
				v_printf(2, "STRNAME ");
				ParseStrName();
				break;
			case rnBoundary:
				v_printf(2, "BOUNDARY ");
				_currentelement = elBoundary;
				break;
			case rnPath:
				v_printf(2, "PATH ");
				_currentelement = elPath;
				break;
			case rnSRef:
				v_printf(2, "SREF ");
				_currentelement = elSRef;
				break;
			case rnARef:
				v_printf(2, "AREF ");
				_currentelement = elARef;
				break;
			case rnText:
				v_printf(2, "TEXT ");
				_currentelement = elText;
				break;
			case rnLayer:
				_currentlayer = GetTwoByteSignedInt();
				v_printf(2, "LAYER (%d)\n", _currentlayer);
				break;
			case rnDataType:
				_currentdatatype = GetTwoByteSignedInt();
				v_printf(2, "DATATYPE (%d)\n", _currentdatatype);
				break;
			case rnWidth:
				_currentwidth = (float)(GetFourByteSignedInt()/2);
				if(_currentwidth > 0){
					_currentwidth *= _units;
				}
				v_printf(2, "WIDTH (%.3f)\n", _currentwidth*2);
				// Scale to a half to make width correct when adding and
				// subtracting
				break;
			case rnXY:
				v_printf(2, "XY ");
				switch(_currentelement){
					case elBoundary:
						_BoundaryElements++;
						ParseXYBoundary();
						break;
					case elBox:
						_BoxElements++;
						ParseXYBoundary();
						break;
					case elPath:
						_PathElements++;
						ParseXYPath();
						break;
					default:
						ParseXY();
						break;
				}
				break;
			case rnColRow:
				_arraycols = GetTwoByteSignedInt();
				_arrayrows = GetTwoByteSignedInt();
				v_printf(2, "COLROW (Columns = %d Rows = %d)\n", _arraycols, _arrayrows);
				break;
			case rnSName:
				ParseSName();
				break;
			case rnPathType:
				if(!_unsupported[rnPathType]){
					v_printf(1, "Incomplete support for GDS2 record type: PATHTYPE\n");
					_unsupported[rnPathType] = true;
				}
				//FIXME
				_currentpathtype = GetTwoByteSignedInt();
				v_printf(2, "PATHTYPE (%d)\n", _currentpathtype);
				break;
			case rnTextType:
				ReportUnsupported("TEXTTYPE", rnTextType);
				_currenttexttype = GetTwoByteSignedInt();
				v_printf(2, "TEXTTYPE (%d)\n", _currenttexttype);
				break;
			case rnPresentation:
				_currentpresentation = GetTwoByteSignedInt();
				v_printf(2, "PRESENTATION (%d)\n", _currentpresentation);
				break;
			case rnString:
				v_printf(2, "STRING ");
				if(_textstring){
					delete [] _textstring;
					_textstring = NULL;
				}
				_textstring = GetAsciiString();
				/* Only set string if the current object is valid, the text string is valid 
				 * and we are using a layer that is defined and being shown.
				 */
				if(_CurrentObject && _CurrentObject->GetCurrentText() && _textstring){
					if(_process != NULL){
						layer = _process->GetLayer(_currentlayer, _currentdatatype);
						if(layer && layer->Show){
							_CurrentObject->GetCurrentText()->SetString(_textstring);
						}
					}else{
						_CurrentObject->GetCurrentText()->SetString(_textstring);
					}
					v_printf(2, "(\"%s\")", _textstring);
					delete [] _textstring;
					_textstring = NULL;
				}else if(!_textstring){
					return -1;
				}
				v_printf(2, "\n");
				break;
			case rnSTrans:
				if(!_unsupported[rnSTrans]){
					v_printf(1, "Incomplete support for GDS2 record type: STRANS\n");
					_unsupported[rnSTrans] = true;
				}
				//FIXME
				_currentstrans = GetTwoByteSignedInt();
				v_printf(2, "STRANS (%d)\n", _currentstrans);
				break;
			case rnMag:
				_currentmag = GetEightByteReal();
				v_printf(2, "MAG (%f)\n", _currentmag);
				break;
			case rnAngle:
				_currentangle = (float)GetEightByteReal();
				v_printf(2, "ANGLE (%f)\n", _currentangle);
				break;
/*			case rnUInteger:
				break;
Not used in GDS2 spec	case rnUString:
				break;
*/
			case rnRefLibs:
				ReportUnsupported("REFLIBS", rnRefLibs);
				tempstr = GetAsciiString();
				v_printf(2, "REFLIBS (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnFonts:
				ReportUnsupported("FONTS", rnFonts);
				tempstr = GetAsciiString();
				v_printf(2, "FONTS (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnGenerations:
				ReportUnsupported("GENERATIONS", rnGenerations);
				v_printf(2, "GENERATIONS\n");
				v_printf(2, "\t");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, "\n");
				break;
			case rnAttrTable:
				ReportUnsupported("ATTRTABLE", rnAttrTable);
				tempstr = GetAsciiString();
				v_printf(2, "ATTRTABLE (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnStypTable:
				ReportUnsupported("STYPTABLE", rnStypTable);
				v_printf(2, "STYPTABLE (\"%d\")\n", GetTwoByteSignedInt());
				break;
			case rnStrType:
				ReportUnsupported("STRTYPE", rnStrType);
				tempstr = GetAsciiString();
				v_printf(2, "STRTYPE (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnElFlags:
				ReportUnsupported("ELFLAGS", rnElFlags);
				v_printf(2, "ELFLAGS (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnElKey:
				ReportUnsupported("ELKEY", rnElKey);
				v_printf(2, "ELKEY (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnLinkType:
				ReportUnsupported("LINKTYPE", rnLinkType);
				v_printf(2, "LINKTYPE (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnLinkKeys:
				ReportUnsupported("LINKKEYS", rnLinkKeys);
				v_printf(2, "LINKKEYS (");
				while(_recordlen){
					v_printf(2, "%ld ", GetFourByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnNodeType:
				ReportUnsupported("NODETYPE", rnNodeType);
				v_printf(2, "NODETYPE (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnPropAttr:
				ReportUnsupported("PROPATTR", rnPropAttr);
				v_printf(2, "PROPATTR (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnPropValue:
				ReportUnsupported("PROPVALUE", rnPropValue);
				tempstr = GetAsciiString();
				v_printf(2, "PROPVALUE (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnBox:
				ReportUnsupported("BOX", rnBox);
				v_printf(2, "BOX\n");
				/* Empty */
				_currentelement = elBox;
				break;
			case rnBoxType:
				ReportUnsupported("BOXTYPE", rnBoxType);
				v_printf(2, "BOXTYPE (%d)\n", GetTwoByteSignedInt());
				break;
			case rnPlex:
				ReportUnsupported("PLEX", rnPlex);
				v_printf(2, "PLEX (");
				while(_recordlen){
					v_printf(2, "%ld ", GetFourByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnBgnExtn:
				ReportUnsupported("BGNEXTN", rnBgnExtn);
				_currentbgnextn = _units * (float)GetFourByteSignedInt();
				v_printf(2, "BGNEXTN (%f)\n", _currentbgnextn);
				break;
			case rnEndExtn:
				ReportUnsupported("ENDEXTN", rnEndExtn);
				_currentendextn = _units * (float)GetFourByteSignedInt();
				v_printf(2, "ENDEXTN (%ld)\n", _currentendextn);
				break;
			case rnTapeNum:
				ReportUnsupported("TAPENUM", rnTapeNum);
				v_printf(2, "TAPENUM\n");
				v_printf(2, "\t");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, "\n");
				break;
			case rnTapeCode:
				ReportUnsupported("TAPECODE", rnTapeCode);
				v_printf(2, "TAPECODE\n");
				v_printf(2, "\t");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, "\n");
				break;
			case rnStrClass:
				ReportUnsupported("STRCLASS", rnStrClass);
				v_printf(2, "STRCLASS (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnReserved:
				ReportUnsupported("RESERVED", rnReserved);
				v_printf(2, "RESERVED\n");
				/* Empty */
				break;
			case rnFormat:
				ReportUnsupported("FORMAT", rnFormat);
				v_printf(2, "FORMAT (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnMask:
				ReportUnsupported("MASK", rnMask);
				tempstr = GetAsciiString();
				v_printf(2, "MASK (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnEndMasks:
				ReportUnsupported("ENDMASKS", rnEndMasks);
				v_printf(2, "ENDMASKS\n");
				/* Empty */
				break;
			case rnLibDirSize:
				ReportUnsupported("LIBDIRSIZE", rnLibDirSize);
				v_printf(2, "LIBDIRSIZE (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnSrfName:
				ReportUnsupported("SRFNAME", rnSrfName);
				tempstr = GetAsciiString();
				v_printf(2, "SRFNAME (\"%s\")\n", tempstr);
				delete [] tempstr;
				break;
			case rnLibSecur:
				ReportUnsupported("LIBSECUR", rnLibSecur);
				v_printf(2, "LIBSECUR (");
				while(_recordlen){
					v_printf(2, "%d ", GetTwoByteSignedInt());
				}
				v_printf(2, ")\n");
				break;
			case rnBorder:
				ReportUnsupported("BORDER", rnBorder);
				v_printf(2, "BORDER\n");
				/* Empty */
				break;
			case rnSoftFence:
				ReportUnsupported("SOFTFENCE", rnSoftFence);
				v_printf(2, "SOFTFENCE\n");
				/* Empty */
				break;
			case rnHardFence:
				ReportUnsupported("HARDFENCE", rnHardFence);
				v_printf(2, "HARDFENCE\n");
				/* Empty */
				break;
			case rnSoftWire:
				ReportUnsupported("SOFTWIRE", rnSoftWire);
				v_printf(2, "SOFTWIRE\n");
				/* Empty */
				break;
			case rnHardWire:
				ReportUnsupported("HARDWIRE", rnHardWire);
				v_printf(2, "HARDWIRE\n");
				/* Empty */
				break;
			case rnPathPort:
				ReportUnsupported("PATHPORT", rnPathPort);
				v_printf(2, "PATHPORT\n");
				/* Empty */
				break;
			case rnNodePort:
				ReportUnsupported("NODEPORT", rnNodePort);
				v_printf(2, "NODEPORT\n");
				/* Empty */
				break;
			case rnUserConstraint:
				ReportUnsupported("USERCONSTRAINT", rnUserConstraint);
				v_printf(2, "USERCONSTRAINT\n");
				/* Empty */
				break;
			case rnSpacerError:
				ReportUnsupported("SPACERERROR", rnSpacerError);
				v_printf(2, "SPACERERROR\n");
				/* Empty */
				break;
			case rnContact:
				ReportUnsupported("CONTACT", rnContact);
				v_printf(2, "CONTACT\n");
				/* Empty */
				break;
			default:
				v_printf(1, "Unknown record type (%d) at position %ld.", recordtype, ftell(_iptr));

				return -1;
				break;
	
		}
	}	
	return 0;
}

void GDSParse::ParseHeader()
{
	short version;
	version = GetTwoByteSignedInt();
	v_printf(2, "\tVersion = %d\n", version);
}

void GDSParse::ParseLibName()
{
	char *str;
	str = GetAsciiString();
	if(_libname){
		delete [] _libname;
		_libname = NULL;
	}
	_libname = new char[strlen(str)+1];
	if(_libname){
		strcpy(_libname, str);
		v_printf(2, " (\"%s\")\n", _libname);
	}else{
		fprintf(stderr, "\nUnable to allocate memory for string (%d)\n", strlen(str)+1);
	}
	delete [] str;
}

void GDSParse::ParseSName()
{
	v_printf(2, "SNAME ");

	char *str;
	str = GetAsciiString();
	if(_sname){
		delete [] _sname;
		_sname = NULL;
	}
	_sname = new char[strlen(str)+1];
	if(_sname){
		strcpy(_sname, str);
		for(unsigned int i=0; i<strlen(_sname); i++){
			if(_sname[i] && (_sname[i] < 48 || _sname[i] > 57) && (_sname[i] < 65 || _sname[i] > 90) && (_sname[i] < 97 || _sname[i] > 122)){
				_sname[i] = '_';
			}
		}
		v_printf(2, "(\"%s\")\n", _sname);
	}else{
		fprintf(stderr, "Unable to allocate memory for string (%d)\n", strlen(str)+1);
	}
	delete [] str;
}

void GDSParse::ParseUnits()
{
	double tmp;
	_units = (float)GetEightByteReal() * _config->GetScale(); 
	tmp = GetEightByteReal();
	v_printf(1, "DB units/user units = %g\nSize of DB units in metres = %g\nSize of user units in m = %g\n\n", 1/_units, tmp, tmp/_units);
}

void GDSParse::ParseStrName()
{
	char *str=NULL;
	str = GetAsciiString();

	if(str){
		// Disallow invalid characters in POV-Ray _names.
		for(unsigned int i=0; i<strlen(str); i++){
			if(str[i] && (str[i] < 48 || str[i] > 57) && (str[i] < 65 || str[i] > 90) && (str[i] < 97 || str[i] > 122)){
				str[i] = '_';
			}
		}
		v_printf(2, "(\"%s\")", str);

		// This calls our own NewObject function which is pure virtual so the end 
		// user must define it. This means we can always add a unknown object as
		// long as it inherits from GDSObject.
		_CurrentObject = _Objects->AddObject(str, NewObject(str));
		delete [] str;
	}
	v_printf(2, "\n");
}

void GDSParse::ParseXYPath()
{
	float X, Y;
	int points = _recordlen/8;
	int i;
	struct ProcessLayer *thislayer = NULL;

	if(_process != NULL){
		thislayer = _process->GetLayer(_currentlayer, _currentdatatype);

		if(thislayer==NULL){
			// _layer_warning only has fixed bounds at the moment.
			// Not sure how to best make it dynamic.

			if(!_generate_process){
				if(_currentlayer == -1 || _currentdatatype == -1 || !_layer_warning[_currentlayer][_currentdatatype]){
					v_printf(1, "Notice: Layer found in gds2 file that is not defined in the process configuration. Layer is %d, datatype %d.\n", _currentlayer, _currentdatatype);
					v_printf(1, "\tIgnoring this layer.\n");
					_layer_warning[_currentlayer][_currentdatatype] = true;
				}
			}else{
				if(!_layer_warning[_currentlayer][_currentdatatype]){
					_process->AddLayer(_currentlayer, _currentdatatype);
					_layer_warning[_currentlayer][_currentdatatype] = true;
				}
			}
			while(_recordlen){
				GetFourByteSignedInt();
			}
			_currentwidth = 0.0; // Always reset to default for paths in case width not specified
			_currentpathtype = 0;
			_currentangle = 0.0;
			_currentdatatype = -1;
			_currentmag = 1.0;
			return;
		}
	}

	if(_currentwidth){
		/* FIXME - need to check for -ve value and then not scale */
		if(thislayer && thislayer->Thickness && thislayer->Show && _CurrentObject){
			_CurrentObject->AddPath(_currentpathtype, _units*thislayer->Height, _units*thislayer->Thickness, points, _currentwidth, _currentbgnextn, _currentendextn, thislayer);
		}
		for(i=0; i<points; i++){
			X = _units * (float)GetFourByteSignedInt();
			Y = _units * (float)GetFourByteSignedInt();
			v_printf(2, "(%.3f,%.3f) ", X, Y);
			if(thislayer && thislayer->Thickness && thislayer->Show && _CurrentObject){
				_CurrentObject->GetCurrentPath()->AddPoint(i, X, Y);
			}
		}
	}else{
		for(i=0; i<points; i++){
			GetFourByteSignedInt();
			GetFourByteSignedInt();
		}
	}
	v_printf(2, "\n");
	_currentwidth = 0.0; // Always reset to default for paths in case width not specified
	_currentpathtype = 0;
	_currentangle = 0.0;
	_currentdatatype = -1;
	_currentmag = 1.0;
	_currentbgnextn = 0.0;
	_currentendextn = 0.0;
}


void GDSParse::ParseXYBoundary()
{
	float X, Y;
	float firstX=0.0, firstY=0.0;
	int points = _recordlen/8;
	int i;
	struct ProcessLayer *thislayer = NULL;

	if(_process != NULL){
		thislayer = _process->GetLayer(_currentlayer, _currentdatatype);

		if(thislayer==NULL){
			if(!_generate_process){
				if(_currentlayer == -1 || _currentdatatype == -1 || !_layer_warning[_currentlayer][_currentdatatype]){
					v_printf(1, "Notice: Layer found in gds2 file that is not defined in the process configuration. Layer is %d, datatype %d.\n", _currentlayer, _currentdatatype);
					v_printf(1, "\tIgnoring this layer.\n");
					_layer_warning[_currentlayer][_currentdatatype] = true;
				}
			}else{
				if(!_layer_warning[_currentlayer][_currentdatatype]){
					_process->AddLayer(_currentlayer, _currentdatatype);
					_layer_warning[_currentlayer][_currentdatatype] = true;
				}
			}
			while(_recordlen){
				GetFourByteSignedInt();
			}
			_currentwidth = 0.0; // Always reset to default for paths in case width not specified
			_currentpathtype = 0;
			_currentangle = 0.0;
			_currentdatatype = -1;
			_currentmag = 1.0;
			return;
		}
	}

	if(thislayer && thislayer->Thickness && thislayer->Show && _CurrentObject){
		//FIXME - why was this points+1 ? _CurrentObject->AddPolygon(_units*thislayer->Height, _units*thislayer->Thickness, points+1, thislayer->Name);
		_CurrentObject->AddPolygon(_units*thislayer->Height, _units*thislayer->Thickness, points, thislayer);
	}

	for(i=0; i<points; i++){
		X = _units * (float)GetFourByteSignedInt();
		Y = _units * (float)GetFourByteSignedInt();
		v_printf(2, "(%.3f,%.3f) ", X, Y);
		if(i==0){
			firstX = X;
			firstY = Y;
		}
		if(thislayer && thislayer->Thickness && thislayer->Show && _CurrentObject){
			_CurrentObject->GetCurrentPolygon()->AddPoint(i, X, Y);
		}
	}
	v_printf(2, "\n");
	if(thislayer && thislayer->Thickness && thislayer->Show && _CurrentObject){
		_CurrentObject->GetCurrentPolygon()->AddPoint(i, firstX, firstY);
		//_CurrentObject->GetCurrentPolygon()->SetColour(thislayer->Red, thislayer->Green, thislayer->Blue, thislayer->Filter, thislayer->Metal);
	}
	_currentwidth = 0.0; // Always reset to default for paths in case width not specified
	_currentpathtype = 0;
	_currentangle = 0.0;
	_currentdatatype = -1;
	_currentmag = 1.0;
	_currentbgnextn = 0.0;
	_currentendextn = 0.0;
}

void GDSParse::ParseXY()
{
	float X, Y;
	float firstX=0.0, firstY=0.0, secondX=0.0, secondY=0.0;
	struct ProcessLayer *thislayer = NULL;
	int Flipped;

	if(_process != NULL){
		thislayer = _process->GetLayer(_currentlayer, _currentdatatype);
	}
	Flipped = ((u_int16_t)(_currentstrans & 0x8000) == (u_int16_t)0x8000) ? 1 : 0;

	switch(_currentelement){
		case elSRef:
			_SRefElements++;
			X = _units * (float)GetFourByteSignedInt();
			Y = _units * (float)GetFourByteSignedInt();
			v_printf(2, "(%.3f,%.3f)\n", X, Y);

			if(_CurrentObject){
				_CurrentObject->AddSRef(_sname, X, Y, Flipped, _currentmag);
				if(_currentangle){
					_CurrentObject->SetSRefRotation(0, -_currentangle, 0);
				}
			}
			break;

		case elARef:
			_ARefElements++;
			firstX = _units * (float)GetFourByteSignedInt();
			firstY = _units * (float)GetFourByteSignedInt();
			secondX = _units * (float)GetFourByteSignedInt();
			secondY = _units * (float)GetFourByteSignedInt();
			X = _units * (float)GetFourByteSignedInt();
			Y = _units * (float)GetFourByteSignedInt();
			v_printf(2, "(%.3f,%.3f) ", firstX, firstY);
			v_printf(2, "(%.3f,%.3f) ", secondX, secondY);
			v_printf(2, "(%.3f,%.3f)\n", X, Y);

			if(_CurrentObject){
				_CurrentObject->AddARef(_sname, firstX, firstY, secondX, secondY, X, Y, _arraycols, _arrayrows, Flipped, _currentmag);
				if(_currentangle){
					_CurrentObject->SetARefRotation(0, -_currentangle, 0);
				}
			}
			break;

		case elText:
			_TextElements++;

			if(thislayer==NULL){
				if(!_generate_process){
					v_printf(2, "Notice: Layer found in gds2 file that is not defined in the process configuration. Layer is %d, datatype %d.\n", _currentlayer, _currentdatatype);
					v_printf(2, "\tIgnoring this string.\n");
				}else{
					if(!_layer_warning[_currentlayer][_currentdatatype]){
						_process->AddLayer(_currentlayer, _currentdatatype);
						_layer_warning[_currentlayer][_currentdatatype] = true;
					}
				}
				while(_recordlen){
					GetFourByteSignedInt();
				}
				_currentwidth = 0.0; // Always reset to default for paths in case width not specified
				_currentpathtype = 0;
				_currentangle = 0.0;
				_currentdatatype = 0;
				_currentmag = 1.0;
				return;
			}

			X = _units * (float)GetFourByteSignedInt();
			Y = _units * (float)GetFourByteSignedInt();
			v_printf(2, "(%.3f,%.3f)\n", X, Y);

			if(_CurrentObject && _CurrentObject->GetCurrentText()){
				int vert_just, horiz_just;

				vert_just = (((((unsigned long)_currentpresentation & 0x8 ) == (unsigned long)0x8 ) ? 2 : 0) + (((((unsigned long)_currentpresentation & 0x4 ) == (unsigned long)0x4 ) ? 1 : 0)));
				horiz_just = (((((unsigned long)_currentpresentation & 0x2 ) == (unsigned long)0x2 ) ? 2 : 0) + (((((unsigned long)_currentpresentation & 0x1 ) == (unsigned long)0x1 ) ? 1 : 0)));

				_CurrentObject->AddText(X, Y, _units*thislayer->Height, Flipped, _currentmag, vert_just, horiz_just, thislayer);
				if(_currentangle){
					_CurrentObject->GetCurrentText()->SetRotation(0.0, -_currentangle, 0.0);
				}
			}
			break;
		default:
			while(_recordlen){
				GetFourByteSignedInt();
			}
			break;
	}
	_currentwidth = 0.0; // Always reset to default for paths in case width not specified
	_currentpathtype = 0;
	_currentangle = 0.0;
	_currentdatatype = -1;
	_currentmag = 1.0;
	_currentpresentation = 0;
}

short GDSParse::GetBitArray()
{
	byte byte1;

	fread(&byte1, 1, 1, _iptr);
	fread(&byte1, 1, 1, _iptr);

	_recordlen-=2;
	return 0;
}

double GDSParse::GetEightByteReal()
{
	byte value;
	byte b8, b2, b3, b4, b5, b6, b7;
	double sign=1.0;
	double exponent;
	double mant;

	fread(&value, 1, 1, _iptr);
	if(value & 128){
		value -= 128;
		sign = -1.0;
	}
	exponent = (double )value;
	exponent -= 64.0;
	mant=0.0;

	fread(&b2, 1, 1, _iptr);
	fread(&b3, 1, 1, _iptr);
	fread(&b4, 1, 1, _iptr);
	fread(&b5, 1, 1, _iptr);
	fread(&b6, 1, 1, _iptr);
	fread(&b7, 1, 1, _iptr);
	fread(&b8, 1, 1, _iptr);

	mant += b8;
	mant /= 256.0;
	mant += b7;
	mant /= 256.0;
	mant += b6;
	mant /= 256.0;
	mant += b5;
	mant /= 256.0;
	mant += b4;
	mant /= 256.0;
	mant += b3;
	mant /= 256.0;
	mant += b2;
	mant /= 256.0;

	_recordlen-=8;

	return sign*(mant*pow(16.0,exponent));
}

int32_t GDSParse::GetFourByteSignedInt()
{
	int32_t value;
	fread(&value, 4, 1, _iptr);
	
	_recordlen-=4;

#if __BYTE_ORDER == __LITTLE_ENDIAN
	return endian_swap_long(value);
#else
	return value;
#endif
}

int16_t GDSParse::GetTwoByteSignedInt()
{
	int16_t value;

	fread(&value, 2, 1, _iptr);

	_recordlen-=2;

#if __BYTE_ORDER == __LITTLE_ENDIAN
	return endian_swap_short(value);
#else
	return value;
#endif
}

char *GDSParse::GetAsciiString()
{
	char *str=NULL;
	
	if(_recordlen>0){
		_recordlen += _recordlen%2; /* Make sure length is even */
		str = new char[_recordlen+1];
		if(!str){
			fprintf(stderr, "Unable to allocate memory for ascii string (%d)\n", _recordlen);
			return NULL;
		}
		fread(str, 1, _recordlen, _iptr);
		str[_recordlen] = 0;
		_recordlen = 0;
	}
	return str;
}

void GDSParse::ReportUnsupported(const char *Name, enum RecordNumbers rn)
{
	if(!_unsupported[rn]){
		v_printf(1, "Unsupported GDS2 record type: %s\n", Name);
		_unsupported[rn] = true;
	}

}

