/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tknERROR = 258,
     tknIF = 259,
     tknELSE = 260,
     tknWHILE = 261,
     tknREPEAT = 262,
     tknUNTIL = 263,
     tknSTRUCTdef = 264,
     tknVOIDdef = 265,
     tknREALdef = 266,
     tknBOOLdef = 267,
     tknINTdef = 268,
     tknPOINTdef = 269,
     tknBOXdef = 270,
     tknSTRINGdef = 271,
     tknLAYOUTdef = 272,
     tknLISTdef = 273,
     tknRETURN = 274,
     tknTRUE = 275,
     tknFALSE = 276,
     tknLEQ = 277,
     tknGEQ = 278,
     tknEQ = 279,
     tknNEQ = 280,
     tknAND = 281,
     tknOR = 282,
     tknIDENTIFIER = 283,
     tknFIELD = 284,
     tknSTRING = 285,
     tknREAL = 286,
     tknINT = 287
   };
#endif
#define tknERROR 258
#define tknIF 259
#define tknELSE 260
#define tknWHILE 261
#define tknREPEAT 262
#define tknUNTIL 263
#define tknSTRUCTdef 264
#define tknVOIDdef 265
#define tknREALdef 266
#define tknBOOLdef 267
#define tknINTdef 268
#define tknPOINTdef 269
#define tknBOXdef 270
#define tknSTRINGdef 271
#define tknLAYOUTdef 272
#define tknLISTdef 273
#define tknRETURN 274
#define tknTRUE 275
#define tknFALSE 276
#define tknLEQ 277
#define tknGEQ 278
#define tknEQ 279
#define tknNEQ 280
#define tknAND 281
#define tknOR 282
#define tknIDENTIFIER 283
#define tknFIELD 284
#define tknSTRING 285
#define tknREAL 286
#define tknINT 287




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 162 "tell_yacc.yy"
typedef union YYSTYPE {
   float                    real;
   bool                     ptypedef;
   int                      integer;
   char                    *parsestr;
   telldata::typeID         pttname;
   parsercmd::argumentLIST *pfarguments;
   parsercmd::argumentMAP  *parguments;
   parsercmd::cmdBLOCK     *pblock;
   parsercmd::cmdFUNC      *pfblock;
} YYSTYPE;
/* Line 1318 of yacc.c.  */
#line 113 "tell_yacc.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE telllval;

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE telllloc;


