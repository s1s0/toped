/*
 * File: gds_globals.h
 * Author: Roger Light
 * Project gdsto3d
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

#ifndef _GDS_GLOBALS_H
#define _GDS_GLOBALS_H

extern int verbose_output;

void v_printf(const int level, const char *fmt, ...);

typedef enum{
	elBoundary,
	elBox,
	elPath,
	elSRef,
	elARef,
	elText
} gds_element_type;

/* Two consecutive zero bytes are a null word */

typedef unsigned char byte;

typedef struct{
	short RecordLength;
	byte RecordType;
	byte DataType;
	byte *Data;
} gds_header;

/* A word consists of 16 bits, numbered from 0 to 15, left to right. */

enum DataTypes{
	dtNoData,
	dtBitArray,
	/* A word containing bits or group of bits that represent data.
	** A bit array allows one wid to contain more than one piece of
	** information.
	*/
	dtTwoByteSignedInt,
	/* One word in 2s-complement representation. 
	** Range is -32,768 to 32,767
	** S - sign, M - magnitude
	** SMMMMMMM MMMMMMMM
	*/
	dtFourByteSignedInt,
	/* Two words in 2s-complement representation.
	** Range is -2,147,483,648 to 2,147,483,647
	** S - sign, M - magnitude
	** SMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM
	*/
	dtFourByteReal,		/* Not used */
	dtEightByteReal,
	/* 2 and 4 word floating point representation.
	** (Mantissa) * (16 raised to the true value of exponent field)
	**
	** Exponent is greater by 64 than the actual exponent.
	** So if E=65, then the exponent=1.
	**
	** Mantissa is always a positive fraction greater than or equal
	** to 1/16 and less than 1. 
	**
	** S - sign, E - exponent, M - mantissa
	** SEEEEEEE MMMMMMMM MMMMMMMM MMMMMMMM
	*/ 
	dtAsciiString
	/* A collection of bytes representing ascii characters. All odd
	** length strings are padded with a null character and the byte
	** count for the record containg this string includes the null
	** character.
	*/
};

enum RecordNumbers{
	rnHeader,		/* 0 */
	rnBgnLib,
	rnLibName,
	rnUnits,
	rnEndLib,
	rnBgnStr,
	rnStrName,
	rnEndStr,
	rnBoundary,
	rnPath,
	rnSRef,			/* 10 */
	rnARef,
	rnText,
	rnLayer,
	rnDataType,
	rnWidth,
	rnXY,
	rnEndEl,
	rnSName,
	rnColRow,
	rnTextNode,		/* 20 */
	rnNode,
	rnTextType,
	rnPresentation,
	rnSpacing,
	rnString,
	rnSTrans,
	rnMag,
	rnAngle,
	rnUInteger,
	rnUString,		/* 30 */
	rnRefLibs,
	rnFonts,
	rnPathType,
	rnGenerations,
	rnAttrTable,
	rnStypTable,
	rnStrType,
	rnElFlags,
	rnElKey,
	rnLinkType,		/* 40 */
	rnLinkKeys,
	rnNodeType,
	rnPropAttr,
	rnPropValue,
	rnBox,
	rnBoxType,
	rnPlex,
	rnBgnExtn,
	rnEndExtn,
	rnTapeNum,		/* 50 */
	rnTapeCode,
	rnStrClass,
	rnReserved,
	rnFormat,
	rnMask,
	rnEndMasks,
	rnLibDirSize,
	rnSrfName,
	rnLibSecur,
	rnBorder,		/* 60 */
	rnSoftFence,
	rnHardFence,
	rnSoftWire,
	rnHardWire,
	rnPathPort,
	rnNodePort,
	rnUserConstraint,
	rnSpacerError,
	rnContact		/* 69 */
};

#endif
