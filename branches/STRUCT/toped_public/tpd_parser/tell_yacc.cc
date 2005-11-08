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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse tellparse
#define yylex   telllex
#define yyerror tellerror
#define yylval  telllval
#define yychar  tellchar
#define yydebug telldebug
#define yynerrs tellnerrs
#define yylloc telllloc

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




/* Copy the first part of user declarations.  */
#line 39 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"

#include <sstream>
#include "tellyzer.h"
#include "ted_prompt.h"
#include "../tpd_common/outbox.h"
/* Switch on verbose error reporting messages*/
#define YYERROR_VERBOSE 1
/*Current command block - defined in tellyzer.cpp*/
extern parsercmd::cmdBLOCK*       CMDBlock;
/*Global console object*/
extern console::ted_cmd*           Console;
/*Argument list structure used in function definitions*/
parsercmd::argumentLIST  *arglist = NULL;
/*Current tell variable name*/
telldata::tell_var *tellvar = NULL;
telldata::tell_var *tell_lvalue = NULL;
/*Current tell struct */
telldata::tell_type *tellstruct = NULL;
/* used for argument type checking during function call parse */
telldata::argumentQ   *argmap  = NULL;
/*taking care when a function is called from the argument list of another call*/
std::stack<telldata::argumentQ*>  argmapstack;
/* current function type (during function definition)*/
telldata::typeID funcretype;
/*number of return statements encountered*/
int returns = 0;
/*number of errors in a function defintion*/
int funcdeferrors = 0;
/*number of poits of the current polygon*/
unsigned listlength = 0;
void tellerror(std::string s, parsercmd::yyltype loc);
void tellerror (std::string s);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 162 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
typedef union YYSTYPE {
   float                    real;
   bool                     ptypedef;
   int                      integer;
   char                    *parsestr;
   telldata::typeID         pttname;
   parsercmd::argumentLIST *pfarguments;
    telldata::argumentQ    *plarguments;
    telldata::argumentID   *parguments;
   parsercmd::cmdBLOCK     *pblock;
   parsercmd::cmdFUNC      *pfblock;
} YYSTYPE;
/* Line 190 of yacc.c.  */
#line 196 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.cc"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

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


/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 220 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.cc"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYLTYPE_IS_TRIVIAL) && YYLTYPE_IS_TRIVIAL \
             && defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  64
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   215

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  49
/* YYNRULES -- Number of rules. */
#define YYNRULES  103
/* YYNRULES -- Number of states. */
#define YYNSTATES  160

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   287

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      34,    35,    44,    42,    39,    43,     2,    45,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    33,
      40,    38,    41,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    36,     2,    37,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    13,    15,    17,    18,
      19,    28,    29,    34,    35,    40,    42,    45,    51,    59,
      60,    61,    69,    70,    78,    80,    84,    85,    87,    89,
      91,    93,    95,    97,    99,   101,   102,   108,   109,   114,
     115,   117,   119,   121,   123,   125,   127,   131,   132,   134,
     136,   140,   143,   145,   147,   149,   151,   154,   157,   159,
     162,   164,   166,   168,   170,   172,   174,   176,   178,   180,
     181,   188,   190,   194,   197,   198,   203,   205,   209,   211,
     215,   217,   221,   225,   227,   231,   235,   239,   243,   245,
     249,   253,   255,   259,   263,   265,   268,   270,   272,   274,
     276,   278,   280,   284
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      47,     0,    -1,    48,    -1,    47,    48,    -1,    64,    33,
      -1,    49,    -1,     3,    -1,     1,    -1,    -1,    -1,    79,
      28,    34,    50,    72,    35,    51,    54,    -1,    -1,    36,
      53,    63,    37,    -1,    -1,    36,    55,    63,    37,    -1,
      19,    -1,    19,    70,    -1,     4,    34,    87,    35,    52,
      -1,     4,    34,    87,    35,    52,     5,    52,    -1,    -1,
      -1,     6,    34,    59,    87,    35,    60,    52,    -1,    -1,
       7,    52,     8,    34,    62,    87,    35,    -1,    64,    -1,
      63,    33,    64,    -1,    -1,    77,    -1,    67,    -1,    57,
      -1,    58,    -1,    61,    -1,    56,    -1,    65,    -1,    81,
      -1,    -1,    28,    34,    66,    69,    35,    -1,    -1,    75,
      38,    68,    70,    -1,    -1,    71,    -1,    65,    -1,    87,
      -1,    67,    -1,    85,    -1,    70,    -1,    71,    39,    70,
      -1,    -1,    73,    -1,    74,    -1,    73,    39,    74,    -1,
      79,    28,    -1,    76,    -1,    77,    -1,    28,    -1,    84,
      -1,    79,    28,    -1,    80,    28,    -1,    80,    -1,    80,
      18,    -1,    10,    -1,    11,    -1,    13,    -1,    12,    -1,
      14,    -1,    15,    -1,    16,    -1,    17,    -1,    28,    -1,
      -1,     9,    28,    82,    36,    83,    37,    -1,    78,    -1,
      83,    33,    78,    -1,    76,    29,    -1,    -1,    36,    86,
      71,    37,    -1,    88,    -1,    87,    27,    88,    -1,    89,
      -1,    88,    26,    89,    -1,    90,    -1,    89,    24,    90,
      -1,    89,    25,    90,    -1,    91,    -1,    90,    40,    91,
      -1,    90,    41,    91,    -1,    90,    22,    91,    -1,    90,
      23,    91,    -1,    92,    -1,    91,    42,    92,    -1,    91,
      43,    92,    -1,    93,    -1,    92,    44,    93,    -1,    92,
      45,    93,    -1,    94,    -1,    43,    94,    -1,    31,    -1,
      32,    -1,    20,    -1,    21,    -1,    30,    -1,    76,    -1,
      34,    87,    35,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   199,   199,   200,   204,   208,   209,   210,   214,   219,
     214,   245,   245,   256,   256,   267,   279,   294,   299,   306,
     310,   306,   321,   321,   337,   338,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   354,   354,   374,   374,   383,
     384,   388,   389,   390,   391,   396,   397,   401,   402,   406,
     407,   411,   419,   420,   424,   430,   434,   447,   457,   458,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   480,
     480,   495,   496,   500,   510,   510,   533,   534,   538,   539,
     543,   544,   545,   549,   550,   551,   552,   553,   557,   558,
     559,   563,   564,   565,   569,   570,   574,   576,   578,   580,
     582,   585,   588,   589
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tknERROR", "tknIF", "tknELSE",
  "tknWHILE", "tknREPEAT", "tknUNTIL", "tknSTRUCTdef", "tknVOIDdef",
  "tknREALdef", "tknBOOLdef", "tknINTdef", "tknPOINTdef", "tknBOXdef",
  "tknSTRINGdef", "tknLAYOUTdef", "tknLISTdef", "tknRETURN", "tknTRUE",
  "tknFALSE", "tknLEQ", "tknGEQ", "tknEQ", "tknNEQ", "tknAND", "tknOR",
  "tknIDENTIFIER", "tknFIELD", "tknSTRING", "tknREAL", "tknINT", "';'",
  "'('", "')'", "'{'", "'}'", "'='", "','", "'<'", "'>'", "'+'", "'-'",
  "'*'", "'/'", "$accept", "input", "entrance", "funcdefinition", "@1",
  "@2", "block", "@3", "funcblock", "@4", "returnstatement", "ifstatement",
  "whilestatement", "@5", "@6", "repeatstatement", "@7", "statements",
  "statement", "funccall", "@8", "assignment", "@9", "arguments",
  "argument", "nearguments", "funcarguments", "funcneargument",
  "funcargument", "lvalue", "variable", "variabledeclaration",
  "fielddeclaration", "telltypeID", "telltype", "recorddefinition", "@10",
  "typedefstruct", "fieldname", "structure", "@11", "expression",
  "andexpression", "eqexpression", "relexpression", "addexpression",
  "multiexpression", "unaryexpression", "primaryexpression", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,    59,    40,    41,   123,   125,    61,    44,
      60,    62,    43,    45,    42,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    46,    47,    47,    48,    48,    48,    48,    50,    51,
      49,    53,    52,    55,    54,    56,    56,    57,    57,    59,
      60,    58,    62,    61,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    66,    65,    68,    67,    69,
      69,    70,    70,    70,    70,    71,    71,    72,    72,    73,
      73,    74,    75,    75,    76,    76,    77,    78,    79,    79,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    82,
      81,    83,    83,    84,    86,    85,    87,    87,    88,    88,
      89,    89,    89,    90,    90,    90,    90,    90,    91,    91,
      91,    92,    92,    92,    93,    93,    94,    94,    94,    94,
      94,    94,    94,    94
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     2,     1,     1,     1,     0,     0,
       8,     0,     4,     0,     4,     1,     2,     5,     7,     0,
       0,     7,     0,     7,     1,     3,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     5,     0,     4,     0,
       1,     1,     1,     1,     1,     1,     3,     0,     1,     1,
       3,     2,     1,     1,     1,     1,     2,     2,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       6,     1,     3,     2,     0,     4,     1,     3,     1,     3,
       1,     3,     3,     1,     3,     3,     3,     3,     1,     3,
       3,     1,     3,     3,     1,     2,     1,     1,     1,     1,
       1,     1,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     7,     6,     0,     0,     0,     0,    60,    61,    63,
      62,    64,    65,    66,    67,    15,    54,     0,     2,     5,
      32,    29,    30,    31,     0,    33,    28,     0,    52,    27,
       0,    58,    34,    55,     0,    19,    11,     0,    69,   103,
      98,    99,   100,    96,    97,     0,    74,     0,    41,    43,
      16,   101,    53,     0,    44,    42,    76,    78,    80,    83,
      88,    91,    94,    35,     1,     3,     4,    37,    73,    56,
      59,    54,   101,     0,     0,    26,     0,     0,     0,     0,
      95,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    39,     0,     8,     0,     0,     0,
      24,    22,     0,   102,    45,     0,    77,    79,    81,    82,
      86,    87,    84,    85,    89,    90,    92,    93,     0,    40,
      38,    47,    17,    20,    26,    12,     0,    68,    71,     0,
       0,    75,     0,    36,     0,    48,    49,     0,     0,     0,
      25,     0,    57,     0,    70,    46,     9,     0,    51,    18,
      21,    23,    72,     0,    50,    13,    10,    26,     0,    14
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    17,    18,    19,   121,   153,    37,    75,   156,   157,
      20,    21,    22,    74,   139,    23,   126,    99,    24,    25,
      94,    26,    95,   118,   104,   105,   134,   135,   136,    27,
      72,    29,   128,    53,    31,    32,    77,   130,    33,    54,
      79,    55,    56,    57,    58,    59,    60,    61,    62
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -100
static const short int yypact[] =
{
     162,  -100,  -100,   -20,    -6,    24,    18,  -100,  -100,  -100,
    -100,  -100,  -100,  -100,  -100,   124,    14,    98,  -100,  -100,
    -100,  -100,  -100,  -100,    29,  -100,  -100,    31,    42,    35,
      62,    75,  -100,  -100,     9,  -100,  -100,    88,  -100,  -100,
    -100,  -100,  -100,  -100,  -100,     9,  -100,    33,  -100,  -100,
    -100,   -19,  -100,    78,  -100,    76,    90,   104,    36,   100,
     102,  -100,  -100,  -100,  -100,  -100,  -100,  -100,  -100,    84,
    -100,  -100,    42,    20,     9,   187,    86,    94,    22,   124,
    -100,  -100,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,   124,   124,  -100,    24,    45,    -2,
    -100,  -100,    10,  -100,  -100,   -26,    90,   104,    36,    36,
     100,   100,   100,   100,   102,   102,  -100,  -100,   116,   114,
    -100,    10,   128,  -100,   187,  -100,     9,  -100,  -100,   131,
      41,  -100,   124,  -100,   129,   141,  -100,   142,    24,    24,
    -100,    65,  -100,    10,  -100,  -100,  -100,    10,  -100,  -100,
    -100,  -100,  -100,   150,  -100,  -100,  -100,   187,    54,  -100
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -100,  -100,   170,  -100,  -100,  -100,   -88,  -100,  -100,  -100,
    -100,  -100,  -100,  -100,  -100,  -100,  -100,    32,   -68,   -13,
    -100,   -11,  -100,  -100,    -7,   111,  -100,  -100,    60,  -100,
       0,    -9,    49,     1,   -99,  -100,  -100,  -100,  -100,  -100,
    -100,   -29,   106,   125,   -51,    96,    59,    69,   163
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -69
static const short int yytable[] =
{
      28,    30,    48,   129,    49,    73,    52,   100,    50,   122,
      68,   131,    39,   132,    34,    51,    78,    28,    30,   -52,
       7,     8,     9,    10,    11,    12,    13,    14,    35,    40,
      41,   124,   -68,   108,   109,   125,    39,    71,   127,    42,
      43,    44,   -68,    45,   129,    98,    38,    82,    63,    82,
     149,   150,    47,    40,    41,    97,   140,   103,    86,    87,
      36,    71,    66,    42,    43,    44,    48,    45,    49,    67,
      52,    68,    82,   -53,   143,    28,    88,    89,   144,    51,
     123,    48,    48,    49,    49,    52,    52,   124,   120,   100,
      69,   159,    82,    70,    51,    51,    76,   141,    64,     1,
     151,     2,     3,    82,     4,     5,    81,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    83,    15,    96,    48,
     101,    49,   137,    52,    28,   145,    16,    39,    84,    85,
     102,   -26,    51,   138,     7,     8,     9,    10,    11,    12,
      13,    14,    90,    91,    40,    41,    92,    93,   137,   114,
     115,   133,    16,   132,    42,    43,    44,    28,    45,   142,
      46,   116,   117,     1,   146,     2,     3,    47,     4,     5,
     148,     6,     7,     8,     9,    10,    11,    12,    13,    14,
     147,    15,   110,   111,   112,   113,   155,    65,   106,   158,
      16,     3,   152,     4,     5,   -26,     6,     7,     8,     9,
      10,    11,    12,    13,    14,   119,    15,   154,   107,     0,
      80,     0,     0,     0,     0,    16
};

static const short int yycheck[] =
{
       0,     0,    15,   102,    15,    34,    15,    75,    15,    97,
      29,    37,     3,    39,    34,    15,    45,    17,    17,    38,
      10,    11,    12,    13,    14,    15,    16,    17,    34,    20,
      21,    33,    18,    84,    85,    37,     3,    28,    28,    30,
      31,    32,    28,    34,   143,    74,    28,    27,    34,    27,
     138,   139,    43,    20,    21,    35,   124,    35,    22,    23,
      36,    28,    33,    30,    31,    32,    79,    34,    79,    38,
      79,    29,    27,    38,    33,    75,    40,    41,    37,    79,
      35,    94,    95,    94,    95,    94,    95,    33,    95,   157,
      28,    37,    27,    18,    94,    95,     8,   126,     0,     1,
      35,     3,     4,    27,     6,     7,    28,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    26,    19,    34,   132,
      34,   132,   121,   132,   124,   132,    28,     3,    24,    25,
      36,    33,   132,     5,    10,    11,    12,    13,    14,    15,
      16,    17,    42,    43,    20,    21,    44,    45,   147,    90,
      91,    35,    28,    39,    30,    31,    32,   157,    34,    28,
      36,    92,    93,     1,    35,     3,     4,    43,     6,     7,
      28,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      39,    19,    86,    87,    88,    89,    36,    17,    82,   157,
      28,     4,   143,     6,     7,    33,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    94,    19,   147,    83,    -1,
      47,    -1,    -1,    -1,    -1,    28
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,     3,     4,     6,     7,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    19,    28,    47,    48,    49,
      56,    57,    58,    61,    64,    65,    67,    75,    76,    77,
      79,    80,    81,    84,    34,    34,    36,    52,    28,     3,
      20,    21,    30,    31,    32,    34,    36,    43,    65,    67,
      70,    76,    77,    79,    85,    87,    88,    89,    90,    91,
      92,    93,    94,    34,     0,    48,    33,    38,    29,    28,
      18,    28,    76,    87,    59,    53,     8,    82,    87,    86,
      94,    28,    27,    26,    24,    25,    22,    23,    40,    41,
      42,    43,    44,    45,    66,    68,    34,    35,    87,    63,
      64,    34,    36,    35,    70,    71,    88,    89,    90,    90,
      91,    91,    91,    91,    92,    92,    93,    93,    69,    71,
      70,    50,    52,    35,    33,    37,    62,    28,    78,    80,
      83,    37,    39,    35,    72,    73,    74,    79,     5,    60,
      64,    87,    28,    33,    37,    70,    35,    39,    28,    52,
      52,    35,    78,    51,    74,    36,    54,    55,    63,    37
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value, Location);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  fprintf (yyoutput, ": ");

# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended. */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif


  yyvsp[0] = yylval;
    yylsp[0] = yylloc;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, yylsp - yylen, yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 199 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 3:
#line 200 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 4:
#line 204 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
    if (!yynerrs)  CMDBlock->execute();
    else           CMDBlock->cleaner();
   }
    break;

  case 5:
#line 208 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 6:
#line 209 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {tellerror("Unexpected symbol", (yylsp[0]));}
    break;

  case 7:
#line 210 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {CMDBlock->cleaner();}
    break;

  case 8:
#line 214 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         /*Create a new variableMAP structure containing the arguments*/
         arglist = new parsercmd::argumentLIST;
         funcretype = (yyvsp[-2].pttname);returns = 0;funcdeferrors = 0;
      }
    break;

  case 9:
#line 219 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         /*Check whether such a function is already defined */
         if (NULL != CMDBlock->funcDefined((yyvsp[-4].parsestr),arglist)) {
            tellerror("function already defined",(yyloc)); 
            delete [] (yyvsp[-4].parsestr);
            parsercmd::ClearArgumentList(arglist); delete(arglist); arglist = NULL;
            YYABORT;
         }
      }
    break;

  case 10:
#line 228 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         if ((telldata::tn_void != (yyvsp[-7].pttname)) && (0 == returns)) {
            tellerror("function must return a value", (yyloc));
            delete((yyvsp[0].pfblock));// arglist is cleard by the cmdSTDFUNC destructor
         }
         else  if (funcdeferrors > 0) {
            tellerror("function definition is ignored because of the errors above", (yyloc));
            delete((yyvsp[0].pfblock));// arglist is cleard by the cmdSTDFUNC destructor
         }
         else
            CMDBlock->addFUNC(std::string((yyvsp[-6].parsestr)),(yyvsp[0].pfblock));
         arglist = NULL;
         delete [] (yyvsp[-6].parsestr);
   }
    break;

  case 11:
#line 245 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         CMDBlock = new parsercmd::cmdBLOCK();
         CMDBlock->pushblk();
      }
    break;

  case 12:
#line 249 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         (yyval.pblock) = CMDBlock;
         CMDBlock = CMDBlock->popblk();
      }
    break;

  case 13:
#line 256 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         CMDBlock = new parsercmd::cmdFUNC(arglist,funcretype);
         CMDBlock->pushblk();
      }
    break;

  case 14:
#line 260 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         (yyval.pfblock) = static_cast<parsercmd::cmdFUNC*>(CMDBlock);
         CMDBlock = CMDBlock->popblk();
      }
    break;

  case 15:
#line 267 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      if      (!arglist) tellerror("return statement outside function body", (yylsp[0]));
      else {
         parsercmd::cmdRETURN* rcmd = new parsercmd::cmdRETURN(funcretype);
         if (rcmd->checkRetype(NULL)) CMDBlock->pushcmd(rcmd);
         else {
            tellerror("return value expected", (yylsp[0]));
            delete rcmd;
         }
      }
      returns++;
   }
    break;

  case 16:
#line 279 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      if (!arglist) tellerror("return statement outside function body", (yylsp[-1]));
      else {
         parsercmd::cmdRETURN* rcmd = new parsercmd::cmdRETURN(funcretype);
         if (rcmd->checkRetype((yyvsp[0].parguments))) CMDBlock->pushcmd(rcmd);
         else {
            tellerror("return type different from function type", (yylsp[0]));
            delete rcmd;
         }
      }
      returns++;
   }
    break;

  case 17:
#line 294 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         if (telldata::tn_bool != (yyvsp[-2].pttname)) tellerror("bool type expected",(yylsp[-2]));
         else CMDBlock->pushcmd(new parsercmd::cmdIFELSE((yyvsp[0].pblock), NULL));
      }
    break;

  case 18:
#line 299 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         if (telldata::tn_bool != (yyvsp[-4].pttname)) tellerror("bool type expected",(yylsp[-4]));
         else CMDBlock->pushcmd(new parsercmd::cmdIFELSE((yyvsp[-2].pblock),(yyvsp[0].pblock)));
      }
    break;

  case 19:
#line 306 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         CMDBlock = new parsercmd::cmdBLOCK();
         CMDBlock->pushblk();
      }
    break;

  case 20:
#line 310 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         if (telldata::tn_bool != (yyvsp[-1].pttname)) tellerror("bool type expected", (yylsp[-1]));
      }
    break;

  case 21:
#line 313 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         parsercmd::cmdBLOCK* condBlock = CMDBlock;
         CMDBlock = CMDBlock->popblk();
         CMDBlock->pushcmd(new parsercmd::cmdWHILE(condBlock,(yyvsp[0].pblock)));
   }
    break;

  case 22:
#line 321 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         CMDBlock = new parsercmd::cmdBLOCK();
         CMDBlock->pushblk();
      }
    break;

  case 23:
#line 325 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
         parsercmd::cmdBLOCK* condBlock = CMDBlock;
         CMDBlock = CMDBlock->popblk();
         if (telldata::tn_bool != (yyvsp[-1].pttname)) {
            tellerror("bool type expected", (yylsp[-1]));
            delete condBlock;
         }
         else CMDBlock->pushcmd(new parsercmd::cmdREPEAT(condBlock,(yyvsp[-5].pblock)));
   }
    break;

  case 24:
#line 337 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 25:
#line 338 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 26:
#line 342 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    { }
    break;

  case 27:
#line 343 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    { }
    break;

  case 28:
#line 344 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
    break;

  case 29:
#line 345 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
    break;

  case 30:
#line 346 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
    break;

  case 31:
#line 347 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
    break;

  case 32:
#line 348 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {/*keep the return value in the stack*/}
    break;

  case 33:
#line 349 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {CMDBlock->pushcmd(new parsercmd::cmdSTACKRST());}
    break;

  case 34:
#line 350 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    { }
    break;

  case 35:
#line 354 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
        argmap = new telldata::argumentQ;
        argmapstack.push(argmap);
   }
    break;

  case 36:
#line 358 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      parsercmd::cmdSTDFUNC *fc = CMDBlock->getFuncBody((yyvsp[-4].parsestr),(yyvsp[-1].parguments)->child());
      if (fc) {
         CMDBlock->pushcmd(new parsercmd::cmdFUNCCALL(fc,(yyvsp[-4].parsestr)));
         (yyval.pttname) = fc->gettype();
      }
      else tellerror("unknown function name or wrong parameter list",(yylsp[-4]));
      argmapstack.pop();
      argmap->clear(); delete argmap;
      if (argmapstack.size()) argmap = argmapstack.top();
      else argmap = NULL;
      delete [] (yyvsp[-4].parsestr);
   }
    break;

  case 37:
#line 374 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {tell_lvalue = tellvar;}
    break;

  case 38:
#line 375 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      /*because of the (possible) structure that has an unknown yet tn_usertypes type,
      here we are doing the type checking, using the type of the lvalue*/
      (yyval.pttname) = parsercmd::Assign(tell_lvalue, (yyvsp[0].parguments), (yylsp[-2]));
   }
    break;

  case 39:
#line 383 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.parguments) = new telldata::argumentID();}
    break;

  case 40:
#line 384 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.parguments) = new telldata::argumentID((yyvsp[0].plarguments));}
    break;

  case 41:
#line 388 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.parguments) = new telldata::argumentID((yyvsp[0].pttname));}
    break;

  case 42:
#line 389 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.parguments) = new telldata::argumentID((yyvsp[0].pttname));}
    break;

  case 43:
#line 390 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.parguments) = new telldata::argumentID((yyvsp[0].pttname));}
    break;

  case 44:
#line 391 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.parguments) = (yyvsp[0].parguments);}
    break;

  case 45:
#line 396 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {argmap->push_back((yyvsp[0].parguments)); (yyval.plarguments) = argmap;}
    break;

  case 46:
#line 397 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {argmap->push_back((yyvsp[0].parguments)); (yyval.plarguments) = argmap;}
    break;

  case 47:
#line 401 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 48:
#line 402 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 49:
#line 406 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 50:
#line 407 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {}
    break;

  case 51:
#line 411 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      tellvar = CMDBlock->newTellvar((yyvsp[-1].pttname), (yylsp[-1]));
      arglist->push_back(new parsercmd::argumentTYPE((yyvsp[0].parsestr),tellvar));
      delete [] (yyvsp[0].parsestr);
   }
    break;

  case 52:
#line 419 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 53:
#line 420 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 54:
#line 424 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      tellvar = CMDBlock->getID((yyvsp[0].parsestr));
      if (tellvar) (yyval.pttname) = tellvar->get_type();
      else tellerror("variable not defined in this scope", (yylsp[0]));
      delete [] (yyvsp[0].parsestr);
   }
    break;

  case 55:
#line 430 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 56:
#line 434 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[-1].pttname);
      telldata::tell_var* v = CMDBlock->getID((yyvsp[0].parsestr), true);
      if (!v) {/* if this variableID doesn't exist already in the local scope*/
         /* add it to the local variable map */
         tellvar = CMDBlock->newTellvar((yyvsp[-1].pttname), (yylsp[-1]));
         CMDBlock->addID((yyvsp[0].parsestr),tellvar); 
      }
      else tellerror("variable already defined in this scope", (yylsp[-1]));
      delete [] (yyvsp[0].parsestr);
   }
    break;

  case 57:
#line 447 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      if (!tellstruct->addfield((yyvsp[0].parsestr), (yyvsp[-1].pttname), CMDBlock->getTypeByID((yyvsp[-1].pttname)))) {
         tellerror("field with this name already defined in this strucutre", (yylsp[0]));
         (yyval.ptypedef) = false; // indicates that definition fails
      }
      else (yyval.ptypedef) = true;
   }
    break;

  case 58:
#line 457 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 59:
#line 458 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[-1].pttname) | telldata::tn_listmask;}
    break;

  case 60:
#line 462 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_void;}
    break;

  case 61:
#line 463 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_real;}
    break;

  case 62:
#line 464 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_int;}
    break;

  case 63:
#line 465 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_bool;}
    break;

  case 64:
#line 466 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_pnt;}
    break;

  case 65:
#line 467 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_box;}
    break;

  case 66:
#line 468 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_string;}
    break;

  case 67:
#line 469 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_layout;}
    break;

  case 68:
#line 470 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
        const telldata::tell_type* ttype = CMDBlock->getTypeByName((yyvsp[0].parsestr));
        if (NULL == ttype)  {
           tellerror("Bad type specifier", (yylsp[0]));YYABORT;
        }
        else (yyval.pttname) = ttype->ID();
      }
    break;

  case 69:
#line 480 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
        tellstruct = CMDBlock->requesttypeID((yyvsp[0].parsestr));
        if (NULL == tellstruct) {
           tellerror("type with this name already defined", (yylsp[-1]));
           YYABORT;
        }
     }
    break;

  case 70:
#line 487 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
        if ((yyvsp[-1].ptypedef)) CMDBlock->addlocaltype((yyvsp[-4].parsestr),tellstruct);
        else delete tellstruct;
        tellstruct = NULL;
     }
    break;

  case 71:
#line 495 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    { (yyval.ptypedef) = (yyvsp[0].ptypedef);      }
    break;

  case 72:
#line 496 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    { (yyval.ptypedef) = (yyvsp[-2].ptypedef) && (yyvsp[0].ptypedef);}
    break;

  case 73:
#line 500 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
      assert(NULL != tellvar);
      tellvar = tellvar->field_var((yyvsp[0].parsestr));
      if (tellvar) (yyval.pttname) = tellvar->get_type();
      else tellerror("Bad field identifier", (yylsp[0]));
      delete [] (yyvsp[0].parsestr);
    }
    break;

  case 74:
#line 510 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
        argmap = new telldata::argumentQ;
        argmapstack.push(argmap);
   }
    break;

  case 75:
#line 514 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {
        /*Important note!. Here we will get a list of components that could be
          a tell list or some kind of tell struct or even tell list of tell struct.
          There is no way at this moment to determine the type of the input structure
          for (seems) obvious reasons. So - the type check and the eventual pushcmd
          are postponed untill we get the recepient - i.e. the lvalue or the
          function call. $$ is assigned to argumentID, that caries the whole argument
          queue listed in structure*/
        (yyval.parguments) = new telldata::argumentID(argmap);
        CMDBlock->pushcmd(new parsercmd::cmdSTRUCT((yyval.parguments)));
        argmapstack.pop();
        if (argmapstack.size()) argmap = argmapstack.top();
        else argmap = NULL;
   }
    break;

  case 76:
#line 533 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 77:
#line 534 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),"||",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 78:
#line 538 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 79:
#line 539 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),"&&",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 80:
#line 543 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 81:
#line 544 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),"==",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 82:
#line 545 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),"!=",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 83:
#line 549 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 84:
#line 550 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),"<",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 85:
#line 551 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),">",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 86:
#line 552 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),"<=",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 87:
#line 553 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::BoolEx((yyvsp[-2].pttname),(yyvsp[0].pttname),">=",(yylsp[-2]),(yylsp[-1]));}
    break;

  case 88:
#line 557 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 89:
#line 558 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::Plus((yyvsp[-2].pttname),(yyvsp[0].pttname),(yylsp[-2]),(yylsp[0]));}
    break;

  case 90:
#line 559 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::Minus((yyvsp[-2].pttname),(yyvsp[0].pttname),(yylsp[-2]),(yylsp[0]));}
    break;

  case 91:
#line 563 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 92:
#line 564 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::Multiply((yyvsp[-2].pttname),(yyvsp[0].pttname),(yylsp[-2]),(yylsp[0]));}
    break;

  case 93:
#line 565 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::Divide((yyvsp[-2].pttname),(yyvsp[0].pttname),(yylsp[-2]),(yylsp[0]));}
    break;

  case 94:
#line 569 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);}
    break;

  case 95:
#line 570 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = parsercmd::UMinus((yyvsp[0].pttname),(yylsp[0]));}
    break;

  case 96:
#line 574 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_real;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttreal((yyvsp[0].real)),true));}
    break;

  case 97:
#line 576 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_int;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttint((yyvsp[0].integer)),true));}
    break;

  case 98:
#line 578 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_bool;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttbool(true),true));}
    break;

  case 99:
#line 580 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_bool;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttbool(false),true));}
    break;

  case 100:
#line 582 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = telldata::tn_string;
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(new telldata::ttstring((yyvsp[0].parsestr)),true));
                                                                delete [] (yyvsp[0].parsestr);}
    break;

  case 101:
#line 585 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[0].pttname);
      CMDBlock->pushcmd(new parsercmd::cmdPUSH(tellvar));}
    break;

  case 102:
#line 588 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {(yyval.pttname) = (yyvsp[-1].pttname);}
    break;

  case 103:
#line 589 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"
    {tellerror("Unexpected symbol", (yylsp[0]));}
    break;


    }

/* Line 1037 of yacc.c.  */
#line 2008 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.cc"

  yyvsp -= yylen;
  yyssp -= yylen;
  yylsp -= yylen;

  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
                 yyerror_range[0] = *yylsp;
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp, yylsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyerror_range[0] = yylsp[1-yylen];
  yylsp -= yylen;
  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping", yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though. */
  YYLLOC_DEFAULT (yyloc, yyerror_range - 1, 2);
  *++yylsp = yyloc;

  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval, &yylloc);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 592 "/troy_home/skr_local/toped_public/tpd_parser/tell_yacc.yy"

/*-------------------------------------------------------------------------*/
int yyerror (char *s) {  /* Called by yyparse on error */
   std::ostringstream ost;
   ost << "line " << telllloc.first_line << ": col " << telllloc.first_column
       << ": " << s;
   tell_log(console::MT_ERROR,ost.str().c_str());
   return 0;
}

void tellerror (std::string s, YYLTYPE loc) {
   if (NULL != arglist) funcdeferrors++;
   else                 yynerrs++;
   std::ostringstream ost;
   ost << "line " << loc.first_line << ": col " << loc.first_column << ": ";
   if (loc.filename) {
      std::string fn = loc.filename;
      ost << "in file \"" << fn << "\" : ";
   }
   ost << s;
   tell_log(console::MT_ERROR,ost.str().c_str());
}

void tellerror (std::string s) {
   if (NULL != arglist) funcdeferrors++;
   else                 yynerrs++;
   std::ostringstream ost;
   ost << "line " << telllloc.first_line << ": col " << telllloc.first_column << ": " << s;
   tell_log(console::MT_ERROR,ost.str().c_str());
}


