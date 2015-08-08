/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         vpyyparse
#define yylex           vpyylex
#define yyerror         vpyyerror
#define yylval          vpyylval
#define yychar          vpyychar
#define yydebug         vpyydebug
#define yynerrs         vpyynerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 7 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"

#include "../Symbol/Parse.h"
#include "VensimParseFunctions.h"
extern int vpyylex (void);
extern void vpyyerror (char const *);
#define YYSTYPE ParseUnion

/* Line 371 of yacc.c  */
#line 83 "C:\\sdev\\xmutil\\Vensim\\VYacc.tab.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "VYacc.tab.h".  */
#ifndef YY_VPYY_C_SDEV_XMUTIL_VENSIM_VYACC_TAB_H_INCLUDED
# define YY_VPYY_C_SDEV_XMUTIL_VENSIM_VYACC_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int vpyydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     VPTT_dataequals = 258,
     VPTT_map = 259,
     VPTT_equiv = 260,
     VPTT_groupstar = 261,
     VPTT_and = 262,
     VPTT_macro = 263,
     VPTT_end_of_macro = 264,
     VPTT_or = 265,
     VPTT_not = 266,
     VPTT_hold_backward = 267,
     VPTT_look_forward = 268,
     VPTT_except = 269,
     VPTT_na = 270,
     VPTT_interpolate = 271,
     VPTT_raw = 272,
     VPTT_test_input = 273,
     VPTT_the_condition = 274,
     VPTT_implies = 275,
     VPTT_ge = 276,
     VPTT_le = 277,
     VPTT_tabbed_array = 278,
     VPTT_eqend = 279,
     VPTT_number = 280,
     VPTT_symbol = 281,
     VPTT_units_symbol = 282,
     VPTT_function = 283,
     VPTT_groupname = 284
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE vpyylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int vpyyparse (void *YYPARSE_PARAM);
#else
int vpyyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int vpyyparse (void);
#else
int vpyyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_VPYY_C_SDEV_XMUTIL_VENSIM_VYACC_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 177 "C:\\sdev\\xmutil\\Vensim\\VYacc.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   220

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  49
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  75
/* YYNRULES -- Number of states.  */
#define YYNSTATES  161

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   284

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    46,     2,     2,     2,    30,     2,     2,
      41,    42,    37,    34,    47,    33,     2,    38,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    43,     2,
      35,    32,    36,    48,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    44,     2,    45,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    31,     2,    40,     2,     2,     2,
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
      25,    26,    27,    28,    29
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    16,    21,    25,
      29,    30,    37,    39,    43,    48,    52,    57,    61,    63,
      66,    69,    71,    74,    78,    80,    83,    87,    92,    94,
     100,   104,   112,   114,   121,   123,   125,   127,   130,   133,
     135,   139,   143,   147,   149,   151,   153,   155,   158,   162,
     163,   166,   168,   172,   174,   176,   181,   185,   190,   194,
     198,   202,   206,   210,   214,   218,   222,   226,   230,   234,
     237,   240,   244,   246,   262,   268
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      50,     0,    -1,    24,    -1,     6,    -1,    51,    -1,    53,
      -1,    54,    40,    60,    40,    -1,    54,    40,    60,    31,
      -1,    54,    40,    40,    -1,    54,    40,    31,    -1,    -1,
       8,    52,    26,    41,    67,    42,    -1,     9,    -1,    55,
      32,    67,    -1,    55,    41,    69,    42,    -1,    55,     3,
      68,    -1,    26,    43,    59,    66,    -1,    55,    32,    23,
      -1,    56,    -1,    56,    65,    -1,    56,    64,    -1,    26,
      -1,    26,    57,    -1,    44,    58,    45,    -1,    26,    -1,
      26,    46,    -1,    58,    47,    26,    -1,    58,    47,    26,
      46,    -1,    26,    -1,    41,    26,    33,    26,    42,    -1,
      59,    47,    26,    -1,    59,    47,    41,    26,    33,    26,
      42,    -1,    63,    -1,    63,    44,    61,    47,    61,    45,
      -1,    62,    -1,    48,    -1,    25,    -1,    33,    25,    -1,
      34,    25,    -1,    27,    -1,    63,    38,    63,    -1,    63,
      37,    63,    -1,    41,    63,    42,    -1,    16,    -1,    17,
      -1,    12,    -1,    13,    -1,    14,    57,    -1,    65,    47,
      57,    -1,    -1,     4,    58,    -1,    68,    -1,    67,    47,
      68,    -1,    25,    -1,    56,    -1,    56,    41,    68,    42,
      -1,    41,    68,    42,    -1,    28,    41,    67,    42,    -1,
      68,    34,    68,    -1,    68,    33,    68,    -1,    68,    37,
      68,    -1,    68,    38,    68,    -1,    68,    35,    68,    -1,
      68,    22,    68,    -1,    68,    36,    68,    -1,    68,    21,
      68,    -1,    68,    10,    68,    -1,    68,     7,    68,    -1,
      68,    32,    68,    -1,    33,    68,    -1,    34,    68,    -1,
      68,    39,    68,    -1,    70,    -1,    44,    41,    62,    47,
      62,    42,    33,    41,    62,    47,    62,    42,    45,    47,
      70,    -1,    41,    62,    47,    62,    42,    -1,    70,    47,
      41,    62,    47,    62,    42,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    81,    81,    82,    83,    84,    85,    86,    87,    88,
      92,    92,    96,   103,   104,   105,   106,   107,   112,   113,
     114,   118,   119,   123,   127,   128,   129,   130,   133,   134,
     135,   136,   140,   141,   145,   146,   149,   150,   151,   155,
     156,   157,   158,   163,   164,   165,   166,   170,   171,   175,
     176,   180,   181,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   207,   208,   214,   215
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VPTT_dataequals", "VPTT_map",
  "VPTT_equiv", "VPTT_groupstar", "VPTT_and", "VPTT_macro",
  "VPTT_end_of_macro", "VPTT_or", "VPTT_not", "VPTT_hold_backward",
  "VPTT_look_forward", "VPTT_except", "VPTT_na", "VPTT_interpolate",
  "VPTT_raw", "VPTT_test_input", "VPTT_the_condition", "VPTT_implies",
  "VPTT_ge", "VPTT_le", "VPTT_tabbed_array", "VPTT_eqend", "VPTT_number",
  "VPTT_symbol", "VPTT_units_symbol", "VPTT_function", "VPTT_groupname",
  "'%'", "'|'", "'='", "'-'", "'+'", "'<'", "'>'", "'*'", "'/'", "'^'",
  "'~'", "'('", "')'", "':'", "'['", "']'", "'!'", "','", "'?'", "$accept",
  "fulleq", "macrostart", "$@1", "macroend", "eqn", "lhs", "var",
  "sublist", "symlist", "subdef", "unitsrange", "urangenum", "number",
  "units", "interpmode", "exceptlist", "maplist", "exprlist", "exp",
  "tablevals", "tablepairs", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
      37,   124,    61,    45,    43,    60,    62,    42,    47,    94,
     126,    40,    41,    58,    91,    93,    33,    44,    63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    49,    50,    50,    50,    50,    50,    50,    50,    50,
      52,    51,    53,    54,    54,    54,    54,    54,    55,    55,
      55,    56,    56,    57,    58,    58,    58,    58,    59,    59,
      59,    59,    60,    60,    61,    61,    62,    62,    62,    63,
      63,    63,    63,    64,    64,    64,    64,    65,    65,    66,
      66,    67,    67,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    69,    69,    70,    70
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     4,     4,     3,     3,
       0,     6,     1,     3,     4,     3,     4,     3,     1,     2,
       2,     1,     2,     3,     1,     2,     3,     4,     1,     5,
       3,     7,     1,     6,     1,     1,     1,     2,     2,     1,
       3,     3,     3,     1,     1,     1,     1,     2,     3,     0,
       2,     1,     3,     1,     1,     4,     3,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     3,     1,    15,     5,     7
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    10,    12,     2,    21,     0,     4,     5,     0,
       0,    18,     0,     0,     0,    22,     1,     0,     0,     0,
       0,    45,    46,     0,    43,    44,    20,    19,     0,    28,
       0,    49,    24,     0,    39,     9,     8,     0,     0,    32,
      53,    21,     0,     0,     0,     0,    54,    15,    17,    13,
      51,     0,     0,     0,    72,    47,     0,     0,     0,     0,
       0,    16,    25,    23,     0,     0,     7,     6,     0,     0,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,     0,     0,     0,    14,     0,    48,     0,     0,    50,
      30,     0,    26,    42,    41,    40,    35,     0,    34,     0,
      56,     0,    67,    66,    65,    63,    68,    59,    58,    62,
      64,    60,    61,    71,    52,    37,    38,     0,     0,     0,
      11,     0,     0,    27,     0,    57,    55,     0,     0,     0,
      29,     0,     0,    74,     0,     0,     0,    33,     0,     0,
      31,     0,    75,     0,     0,     0,     0,     0,     0,     0,
      73
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     6,     7,    12,     8,     9,    10,    46,    15,    33,
      31,    38,   107,   108,    39,    26,    27,    61,    49,    50,
      53,    54
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -52
static const yytype_int16 yypact[] =
{
      48,   -52,   -52,   -52,   -52,   -13,    16,   -52,   -52,    15,
       0,   123,    53,   -21,    62,   -52,   -52,   155,   151,    57,
     -32,   -52,   -52,   -20,   -52,   -52,   -52,    42,    56,   -52,
      75,    -2,    59,    28,   -52,   -52,   -52,   -23,    46,    55,
     -52,   -20,    79,   151,   151,   151,    80,    93,   -52,    70,
      93,    19,    83,   100,   113,   -52,   -20,   151,   132,    62,
     -16,   -52,   -52,   -52,   152,   117,   -52,   -52,   -23,   -23,
     -19,   151,   131,   131,     1,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   -52,
     156,   158,   133,    19,   -52,   146,   -52,    76,   162,   142,
     -52,   164,   145,   -52,   -52,   -52,   -52,   147,   -52,   114,
     -52,    74,   125,   136,    10,    10,   112,   131,   131,    10,
      10,   154,   154,   154,    93,   -52,   -52,    19,   150,    19,
     -52,   157,   165,   -52,   -19,   -52,   -52,   159,    19,   153,
     -52,   176,   160,   -52,   161,    19,   166,   -52,   171,   167,
     -52,   169,   -52,    19,   168,    19,   170,   172,   173,   175,
     113
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -52,   -52,   -52,   -52,   -52,   -52,   -52,   206,   -10,   148,
     -52,   -52,    77,   -51,   -18,   -52,   -52,   -52,   -50,   -17,
     -52,    54
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_int16 yytable[] =
{
      92,    47,    59,    18,    34,    29,    89,    97,    76,    51,
     100,    77,    52,    55,    90,    91,    16,    76,    37,    65,
      30,   109,    78,    79,    14,   101,    72,    73,    74,   106,
      13,    14,    19,    80,    81,    82,    83,    84,    85,    86,
      87,    20,   128,   110,    89,    60,    96,    85,    86,    87,
     104,   105,    90,    91,     1,    17,     2,     3,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,     4,    63,     5,    64,   137,    66,   139,    28,
      48,    76,    40,    41,    77,    42,    67,   144,    32,    56,
      43,    44,    68,    69,   149,    78,    79,    57,    45,    70,
      76,    58,   154,    77,   156,    62,    80,    81,    82,    83,
      84,    85,    86,    87,    78,    79,   136,    88,   130,    76,
      71,    75,    77,    88,    93,    80,    81,    82,    83,    84,
      85,    86,    87,    78,    79,    21,    22,    23,    76,    24,
      25,    77,    94,    76,    -1,    81,    82,    83,    84,    85,
      86,    87,    78,    79,    68,    69,   135,    78,    79,   103,
      95,    88,    85,    86,    87,    98,    83,    84,    85,    86,
      87,    83,    84,    85,    86,    87,    40,    41,   102,    42,
     127,   125,    34,   126,    43,    44,    35,   129,   131,    64,
     132,   133,    45,    87,   134,    36,    37,   138,   141,   140,
     145,   143,   146,   148,   151,   147,    11,    99,   150,   152,
     153,   142,   157,   160,     0,   155,    51,   158,     0,     0,
     159
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-52)))

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1)))

static const yytype_int16 yycheck[] =
{
      51,    18,     4,     3,    27,    26,    25,    57,     7,    41,
      26,    10,    44,    23,    33,    34,     0,     7,    41,    37,
      41,    71,    21,    22,    44,    41,    43,    44,    45,    48,
      43,    44,    32,    32,    33,    34,    35,    36,    37,    38,
      39,    41,    93,    42,    25,    47,    56,    37,    38,    39,
      68,    69,    33,    34,     6,    40,     8,     9,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    24,    45,    26,    47,   127,    31,   129,    26,
      23,     7,    25,    26,    10,    28,    40,   138,    26,    47,
      33,    34,    37,    38,   145,    21,    22,    41,    41,    44,
       7,    26,   153,    10,   155,    46,    32,    33,    34,    35,
      36,    37,    38,    39,    21,    22,    42,    47,    42,     7,
      41,    41,    10,    47,    41,    32,    33,    34,    35,    36,
      37,    38,    39,    21,    22,    12,    13,    14,     7,    16,
      17,    10,    42,     7,    32,    33,    34,    35,    36,    37,
      38,    39,    21,    22,    37,    38,    42,    21,    22,    42,
      47,    47,    37,    38,    39,    33,    35,    36,    37,    38,
      39,    35,    36,    37,    38,    39,    25,    26,    26,    28,
      47,    25,    27,    25,    33,    34,    31,    41,    26,    47,
      26,    46,    41,    39,    47,    40,    41,    47,    33,    42,
      47,    42,    26,    42,    33,    45,     0,    59,    42,    42,
      41,   134,    42,   159,    -1,    47,    41,    45,    -1,    -1,
      47
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     8,     9,    24,    26,    50,    51,    53,    54,
      55,    56,    52,    43,    44,    57,     0,    40,     3,    32,
      41,    12,    13,    14,    16,    17,    64,    65,    26,    26,
      41,    59,    26,    58,    27,    31,    40,    41,    60,    63,
      25,    26,    28,    33,    34,    41,    56,    68,    23,    67,
      68,    41,    44,    69,    70,    57,    47,    41,    26,     4,
      47,    66,    46,    45,    47,    63,    31,    40,    37,    38,
      44,    41,    68,    68,    68,    41,     7,    10,    21,    22,
      32,    33,    34,    35,    36,    37,    38,    39,    47,    25,
      33,    34,    62,    41,    42,    47,    57,    67,    33,    58,
      26,    41,    26,    42,    63,    63,    48,    61,    62,    67,
      42,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    25,    25,    47,    62,    41,
      42,    26,    26,    46,    47,    42,    42,    62,    47,    62,
      42,    33,    61,    42,    62,    47,    26,    45,    42,    62,
      42,    33,    42,    41,    62,    47,    62,    42,    45,    47,
      70
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
/* Line 1792 of yacc.c  */
#line 81 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { return VPTT_eqend ; }
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 82 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { return VPTT_groupstar ; }
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 83 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { return '|'; }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 84 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {return '|'; }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 85 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (4)].eqn),(yyvsp[(3) - (4)].uni)) ; return '~' ; }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 86 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (4)].eqn),(yyvsp[(3) - (4)].uni)) ; return '|' ; }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 87 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (3)].eqn),'\0') ; return '~' ;}
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 88 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (3)].eqn),'\0') ; return '|' ;}
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 92 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { vpyy_macro_start(); }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 92 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { vpyy_macro_expression((yyvsp[(3) - (6)].sym),(yyvsp[(5) - (6)].exl)) ;}
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 96 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok); vpyy_macro_end(); }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 103 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.eqn) = vpyy_addeq((yyvsp[(1) - (3)].lhs),'\0',(yyvsp[(3) - (3)].exl),'=') ; }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 104 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.eqn) = vpyy_addeq((yyvsp[(1) - (4)].lhs),(Expression *)(yyvsp[(3) - (4)].tbl),'\0','(') ; }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 105 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.eqn) = vpyy_addeq((yyvsp[(1) - (3)].lhs),(yyvsp[(3) - (3)].exn),'\0',VPTT_dataequals) ; }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 106 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[(1) - (4)].sym),'\0'),'\0','\0'),(Expression *)vpyy_symlist_expression((yyvsp[(3) - (4)].sml),(yyvsp[(4) - (4)].sml)),'\0',':') ; }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 107 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.eqn) = vpyy_addeq((yyvsp[(1) - (3)].lhs),(yyvsp[(3) - (3)].exn),'\0','=') ; }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 112 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.lhs) = vpyy_addexceptinterp((yyvsp[(1) - (1)].var),'\0','\0') ; }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 113 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[(1) - (2)].var),(yyvsp[(2) - (2)].sll),'\0') ;}
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 114 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[(1) - (2)].var),'\0',(yyvsp[(2) - (2)].tok)) ;}
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 118 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.var) = vpyy_var_expression((yyvsp[(1) - (1)].sym),'\0');}
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 119 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.var) = vpyy_var_expression((yyvsp[(1) - (2)].sym),(yyvsp[(2) - (2)].sml)) ;}
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 123 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.sml) = (yyvsp[(2) - (3)].sml) ;}
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 127 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist('\0',(yyvsp[(1) - (1)].sym),0,'\0') ; }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 128 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist('\0',(yyvsp[(1) - (2)].sym),1,'\0') ; }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 129 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist((yyvsp[(1) - (3)].sml),(yyvsp[(3) - (3)].sym),0,'\0') ;}
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 130 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist((yyvsp[(1) - (4)].sml),(yyvsp[(3) - (4)].sym),1,'\0') ;}
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 133 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist('\0',(yyvsp[(1) - (1)].sym),0,'\0') ; }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 134 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.sml) = vpyy_symlist('\0',(yyvsp[(2) - (5)].sym),0,(yyvsp[(4) - (5)].sym)) ;}
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 135 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist((yyvsp[(1) - (3)].sml),(yyvsp[(3) - (3)].sym),0,'\0') ; }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 136 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.sml) = vpyy_symlist((yyvsp[(1) - (7)].sml),(yyvsp[(4) - (7)].sym),0,(yyvsp[(6) - (7)].sym)) ; }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 140 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.uni) = (yyvsp[(1) - (1)].uni) ; }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 141 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.uni) = vpyy_unitsrange((yyvsp[(1) - (6)].uni),(yyvsp[(3) - (6)].num),(yyvsp[(5) - (6)].num)) ; }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 145 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.num) = (yyvsp[(1) - (1)].num) ; }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 146 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.num) = -1e30 ; }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 149 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.num) = (yyvsp[(1) - (1)].num) ; }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 150 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.num) = -(yyvsp[(2) - (2)].num) ;}
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 151 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.num) = (yyvsp[(2) - (2)].num) ;}
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 155 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.uni) = (yyvsp[(1) - (1)].uni) ; }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 156 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.uni) = vpyy_unitsdiv((yyvsp[(1) - (3)].uni),(yyvsp[(3) - (3)].uni));}
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 157 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.uni) = vpyy_unitsmult((yyvsp[(1) - (3)].uni),(yyvsp[(3) - (3)].uni));}
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 158 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.uni) = (yyvsp[(2) - (3)].uni) ; }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 163 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 164 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 165 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 166 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 170 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sll) = vpyy_chain_sublist('\0',(yyvsp[(2) - (2)].sml)) ; }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 171 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { vpyy_chain_sublist((yyvsp[(1) - (3)].sll),(yyvsp[(3) - (3)].sml)) ; (yyval.sll) = (yyvsp[(1) - (3)].sll) ; }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 175 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) = '\0' ; }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 176 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.sml) =  (yyvsp[(2) - (2)].sml) ; }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 180 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.exl) = vpyy_chain_exprlist('\0',(yyvsp[(1) - (1)].exn)) ;}
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 181 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[(1) - (3)].exl),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 185 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_num_expression((yyvsp[(1) - (1)].num)) ; }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 186 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = (Expression *)(yyvsp[(1) - (1)].var) ; }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 187 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_lookup_expression((yyvsp[(1) - (4)].var),(yyvsp[(3) - (4)].exn)) ; }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 188 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('(',(yyvsp[(2) - (3)].exn),'\0') ; }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 189 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_function_expression((yyvsp[(1) - (4)].fnc),(yyvsp[(3) - (4)].exl)) ;}
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 190 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('+',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 191 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 192 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('*',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 193 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('/',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 194 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('<',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 195 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression(VPTT_le,(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 196 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('>',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 197 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression(VPTT_ge,(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 198 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression(VPTT_or,(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 199 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression(VPTT_and,(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 200 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('=',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 201 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[(2) - (2)].exn),'\0') ; }
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 202 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('+',(yyvsp[(2) - (2)].exn),'\0') ; }
    break;

  case 71:
/* Line 1792 of yacc.c  */
#line 203 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('^',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 207 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tbl) = (yyvsp[(1) - (1)].tbl) ; }
    break;

  case 73:
/* Line 1792 of yacc.c  */
#line 209 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tbl) = vpyy_tablerange((yyvsp[(15) - (15)].tbl),(yyvsp[(3) - (15)].num),(yyvsp[(5) - (15)].num),(yyvsp[(9) - (15)].num),(yyvsp[(11) - (15)].num)) ; }
    break;

  case 74:
/* Line 1792 of yacc.c  */
#line 214 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    { (yyval.tbl) = vpyy_tablepair('\0',(yyvsp[(2) - (5)].num),(yyvsp[(4) - (5)].num)) ;}
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 215 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
    {(yyval.tbl) = vpyy_tablepair((yyvsp[(1) - (7)].tbl),(yyvsp[(4) - (7)].num),(yyvsp[(6) - (7)].num)) ;}
    break;


/* Line 1792 of yacc.c  */
#line 1965 "C:\\sdev\\xmutil\\Vensim\\VYacc.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
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
      if (!yypact_value_is_default (yyn))
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
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
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 221 "C:\\sdev\\xmutil\\Vensim\\VYacc.y"
