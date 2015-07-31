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
#line 7 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"

#include "../Symbol/Parse.h"
#include "VensimParseFunctions.h"
extern int vpyylex (void);
extern void vpyyerror (char const *);
#define YYSTYPE ParseUnion
#define YYDEBUG 1

/* Line 371 of yacc.c  */
#line 84 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.tab.c"

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
#ifndef YY_VPYY_C_USERS_BOB_DROPBOX_SDEV_XMUTIL_VENSIM_VYACC_TAB_H_INCLUDED
# define YY_VPYY_C_USERS_BOB_DROPBOX_SDEV_XMUTIL_VENSIM_VYACC_TAB_H_INCLUDED
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
     VPTT_or = 263,
     VPTT_not = 264,
     VPTT_hold_backward = 265,
     VPTT_look_forward = 266,
     VPTT_except = 267,
     VPTT_na = 268,
     VPTT_interpolate = 269,
     VPTT_raw = 270,
     VPTT_test_input = 271,
     VPTT_the_condition = 272,
     VPTT_implies = 273,
     VPTT_tabbed_array = 274,
     VPTT_eqend = 275,
     VPTT_number = 276,
     VPTT_symbol = 277,
     VPTT_units_symbol = 278,
     VPTT_function = 279,
     VPTT_groupname = 280
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

#endif /* !YY_VPYY_C_USERS_BOB_DROPBOX_SDEV_XMUTIL_VENSIM_VYACC_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 174 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.tab.c"

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
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   176

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  63
/* YYNRULES -- Number of states.  */
#define YYNSTATES  138

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   280

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    40,     2,     2,     2,    26,     2,     2,
      35,    36,    31,    30,    41,    29,     2,    32,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    37,     2,
       2,    28,     2,    42,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    38,     2,    39,    33,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    27,     2,    34,     2,     2,     2,
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
      25
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,    12,    17,    21,    25,    29,
      34,    38,    43,    47,    49,    52,    55,    57,    60,    64,
      66,    69,    73,    78,    80,    86,    90,    98,   100,   107,
     109,   111,   113,   116,   119,   121,   125,   129,   133,   135,
     137,   139,   141,   144,   148,   149,   152,   154,   158,   160,
     162,   167,   171,   176,   180,   184,   188,   192,   195,   198,
     202,   204,   220,   226
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      44,     0,    -1,    20,    -1,     6,    -1,    45,    34,    51,
      34,    -1,    45,    34,    51,    27,    -1,    45,    34,    34,
      -1,    45,    34,    27,    -1,    46,    28,    58,    -1,    46,
      35,    60,    36,    -1,    46,     3,    59,    -1,    22,    37,
      50,    57,    -1,    46,    28,    19,    -1,    47,    -1,    47,
      56,    -1,    47,    55,    -1,    22,    -1,    22,    48,    -1,
      38,    49,    39,    -1,    22,    -1,    22,    40,    -1,    49,
      41,    22,    -1,    49,    41,    22,    40,    -1,    22,    -1,
      35,    22,    29,    22,    36,    -1,    50,    41,    22,    -1,
      50,    41,    35,    22,    29,    22,    36,    -1,    54,    -1,
      54,    38,    52,    41,    52,    39,    -1,    53,    -1,    42,
      -1,    21,    -1,    29,    21,    -1,    30,    21,    -1,    23,
      -1,    54,    32,    54,    -1,    54,    31,    54,    -1,    35,
      54,    36,    -1,    14,    -1,    15,    -1,    10,    -1,    11,
      -1,    12,    48,    -1,    56,    41,    48,    -1,    -1,     4,
      49,    -1,    59,    -1,    58,    41,    59,    -1,    21,    -1,
      47,    -1,    47,    35,    59,    36,    -1,    35,    59,    36,
      -1,    24,    35,    58,    36,    -1,    59,    30,    59,    -1,
      59,    29,    59,    -1,    59,    31,    59,    -1,    59,    32,
      59,    -1,    29,    59,    -1,    30,    59,    -1,    59,    33,
      59,    -1,    61,    -1,    38,    35,    53,    41,    53,    36,
      29,    35,    53,    41,    53,    36,    39,    41,    61,    -1,
      35,    53,    41,    53,    36,    -1,    61,    41,    35,    53,
      41,    53,    36,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    73,    73,    74,    75,    76,    77,    78,    82,    83,
      84,    85,    86,    91,    92,    93,    97,    98,   102,   106,
     107,   108,   109,   112,   113,   114,   115,   119,   120,   124,
     125,   128,   129,   130,   134,   135,   136,   137,   142,   143,
     144,   145,   149,   150,   154,   155,   159,   160,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     179,   180,   186,   187
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VPTT_dataequals", "VPTT_map",
  "VPTT_equiv", "VPTT_groupstar", "VPTT_and", "VPTT_or", "VPTT_not",
  "VPTT_hold_backward", "VPTT_look_forward", "VPTT_except", "VPTT_na",
  "VPTT_interpolate", "VPTT_raw", "VPTT_test_input", "VPTT_the_condition",
  "VPTT_implies", "VPTT_tabbed_array", "VPTT_eqend", "VPTT_number",
  "VPTT_symbol", "VPTT_units_symbol", "VPTT_function", "VPTT_groupname",
  "'%'", "'|'", "'='", "'-'", "'+'", "'*'", "'/'", "'^'", "'~'", "'('",
  "')'", "':'", "'['", "']'", "'!'", "','", "'?'", "$accept", "fulleq",
  "eqn", "lhs", "var", "sublist", "symlist", "subdef", "unitsrange",
  "urangenum", "number", "units", "interpmode", "exceptlist", "maplist",
  "exprlist", "exp", "tablevals", "tablepairs", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,    37,   124,    61,    45,
      43,    42,    47,    94,   126,    40,    41,    58,    91,    93,
      33,    44,    63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    43,    44,    44,    44,    44,    44,    44,    45,    45,
      45,    45,    45,    46,    46,    46,    47,    47,    48,    49,
      49,    49,    49,    50,    50,    50,    50,    51,    51,    52,
      52,    53,    53,    53,    54,    54,    54,    54,    55,    55,
      55,    55,    56,    56,    57,    57,    58,    58,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      60,    60,    61,    61
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     4,     4,     3,     3,     3,     4,
       3,     4,     3,     1,     2,     2,     1,     2,     3,     1,
       2,     3,     4,     1,     5,     3,     7,     1,     6,     1,
       1,     1,     2,     2,     1,     3,     3,     3,     1,     1,
       1,     1,     2,     3,     0,     2,     1,     3,     1,     1,
       4,     3,     4,     3,     3,     3,     3,     2,     2,     3,
       1,    15,     5,     7
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,     2,    16,     0,     0,     0,    13,     0,     0,
      17,     1,     0,     0,     0,     0,    40,    41,     0,    38,
      39,    15,    14,    23,     0,    44,    19,     0,    34,     7,
       6,     0,     0,    27,    48,    16,     0,     0,     0,     0,
      49,    10,    12,     8,    46,     0,     0,     0,    60,    42,
       0,     0,     0,     0,    11,    20,    18,     0,     0,     5,
       4,     0,     0,     0,     0,    57,    58,     0,     0,     0,
       0,     0,     0,     0,     0,    31,     0,     0,     0,     0,
       9,     0,    43,     0,    45,    25,     0,    21,    37,    36,
      35,    30,     0,    29,     0,    51,     0,    54,    53,    55,
      56,    59,    47,    32,    33,     0,     0,     0,     0,     0,
      22,     0,    52,    50,     0,     0,     0,    24,     0,     0,
      62,     0,     0,     0,    28,     0,     0,    26,     0,    63,
       0,     0,     0,     0,     0,     0,     0,    61
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,     6,    40,    10,    27,    25,    32,    92,
      93,    33,    21,    22,    54,    43,    44,    47,    48
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -46
static const yytype_int8 yypact[] =
{
       9,   -46,   -46,    38,    27,   -24,     0,    94,   -15,    -1,
     -46,   -46,    30,    21,   -13,    36,   -46,   -46,     3,   -46,
     -46,   -46,    -4,   -46,    37,    -3,    12,    79,   -46,   -46,
     -46,    -5,    34,    35,   -46,     3,    23,    21,    21,    21,
      28,    81,   -46,    31,    81,    25,    51,    33,    52,   -46,
       3,    65,    -1,   -10,   -46,   -46,   -46,    80,     8,   -46,
     -46,    -5,    -5,   -16,    21,    84,    84,    59,    21,    21,
      21,    21,    21,    21,    21,   -46,    86,    98,    60,    25,
     -46,    87,   -46,    99,    82,   -46,   102,    85,   -46,   -46,
     -46,   -46,    88,   -46,   -17,   -46,    67,    84,    84,    93,
      93,    93,    81,   -46,   -46,    25,    89,    25,    91,   103,
     -46,   -16,   -46,   -46,    92,    25,    90,   -46,   111,    95,
     -46,   100,    25,   101,   -46,   106,   104,   -46,   107,   -46,
      25,    97,    25,   105,   108,   109,   110,    52
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -46,   -46,   -46,   -46,   139,   -14,    96,   -46,   -46,    32,
     -45,   -29,   -46,   -46,   -46,   112,    10,   -46,    13
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      78,    52,    58,    13,    49,    75,    42,    23,    34,    35,
      12,    36,    85,    76,    77,     1,    37,    38,    28,   112,
      24,    26,    39,    41,    74,    86,    91,    11,    14,     2,
      31,     3,    89,    90,   106,    15,    82,    50,    53,    61,
      62,     9,    34,    35,    88,    36,    75,    65,    66,    67,
      37,    38,    55,    28,    76,    77,    39,    29,    64,    51,
     114,    59,   116,    68,    30,    31,    61,    62,    60,    80,
     121,    45,    74,    63,    46,     8,     9,   126,    96,    97,
      98,    99,   100,   101,   102,   131,    79,   133,    69,    70,
      71,    72,    73,    81,    83,    95,    69,    70,    71,    72,
      73,   105,    87,   113,    16,    17,    18,   103,    19,    20,
      69,    70,    71,    72,    73,    71,    72,    73,    56,   104,
      57,   108,   107,    57,   109,   110,    73,   117,   120,   111,
     115,   122,   118,   123,   124,   128,   125,   127,   132,     7,
     129,   134,   130,   119,     0,    45,     0,   135,    84,   137,
     136,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    94
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-46)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      45,     4,    31,     3,    18,    21,    19,    22,    21,    22,
      34,    24,    22,    29,    30,     6,    29,    30,    23,    36,
      35,    22,    35,    13,    41,    35,    42,     0,    28,    20,
      35,    22,    61,    62,    79,    35,    50,    41,    41,    31,
      32,    38,    21,    22,    36,    24,    21,    37,    38,    39,
      29,    30,    40,    23,    29,    30,    35,    27,    35,    22,
     105,    27,   107,    35,    34,    35,    31,    32,    34,    36,
     115,    35,    41,    38,    38,    37,    38,   122,    68,    69,
      70,    71,    72,    73,    74,   130,    35,   132,    29,    30,
      31,    32,    33,    41,    29,    36,    29,    30,    31,    32,
      33,    41,    22,    36,    10,    11,    12,    21,    14,    15,
      29,    30,    31,    32,    33,    31,    32,    33,    39,    21,
      41,    22,    35,    41,    22,    40,    33,    36,    36,    41,
      41,    41,    29,    22,    39,    29,    36,    36,    41,     0,
      36,    36,    35,   111,    -1,    35,    -1,    39,    52,   136,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    64
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,    20,    22,    44,    45,    46,    47,    37,    38,
      48,     0,    34,     3,    28,    35,    10,    11,    12,    14,
      15,    55,    56,    22,    35,    50,    22,    49,    23,    27,
      34,    35,    51,    54,    21,    22,    24,    29,    30,    35,
      47,    59,    19,    58,    59,    35,    38,    60,    61,    48,
      41,    22,     4,    41,    57,    40,    39,    41,    54,    27,
      34,    31,    32,    38,    35,    59,    59,    59,    35,    29,
      30,    31,    32,    33,    41,    21,    29,    30,    53,    35,
      36,    41,    48,    29,    49,    22,    35,    22,    36,    54,
      54,    42,    52,    53,    58,    36,    59,    59,    59,    59,
      59,    59,    59,    21,    21,    41,    53,    35,    22,    22,
      40,    41,    36,    36,    53,    41,    53,    36,    29,    52,
      36,    53,    41,    22,    39,    36,    53,    36,    29,    36,
      35,    53,    41,    53,    36,    39,    41,    61
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
#line 73 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { return VPTT_eqend ; }
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 74 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { return VPTT_groupstar ; }
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 75 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (4)].eqn),(yyvsp[(3) - (4)].uni)) ; return '~' ; }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 76 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (4)].eqn),(yyvsp[(3) - (4)].uni)) ; return '|' ; }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 77 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (3)].eqn),'\0') ; return '~' ;}
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 78 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {vpyy_addfulleq((yyvsp[(1) - (3)].eqn),'\0') ; return '|' ;}
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 82 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.eqn) = vpyy_addeq((yyvsp[(1) - (3)].lhs),'\0',(yyvsp[(3) - (3)].exl),'=') ; }
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 83 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.eqn) = vpyy_addeq((yyvsp[(1) - (4)].lhs),(Expression *)(yyvsp[(3) - (4)].tbl),'\0','(') ; }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 84 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.eqn) = vpyy_addeq((yyvsp[(1) - (3)].lhs),(yyvsp[(3) - (3)].exn),'\0',VPTT_dataequals) ; }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 85 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[(1) - (4)].sym),'\0'),'\0','\0'),(Expression *)vpyy_symlist_expression((yyvsp[(3) - (4)].sml),(yyvsp[(4) - (4)].sml)),'\0',':') ; }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 86 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.eqn) = vpyy_addeq((yyvsp[(1) - (3)].lhs),(yyvsp[(3) - (3)].exn),'\0','=') ; }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 91 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.lhs) = vpyy_addexceptinterp((yyvsp[(1) - (1)].var),'\0','\0') ; }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 92 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[(1) - (2)].var),(yyvsp[(2) - (2)].sll),'\0') ;}
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 93 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[(1) - (2)].var),'\0',(yyvsp[(2) - (2)].tok)) ;}
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 97 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.var) = vpyy_var_expression((yyvsp[(1) - (1)].sym),'\0');}
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 98 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.var) = vpyy_var_expression((yyvsp[(1) - (2)].sym),(yyvsp[(2) - (2)].sml)) ;}
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 102 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.sml) = (yyvsp[(2) - (3)].sml) ;}
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 106 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist('\0',(yyvsp[(1) - (1)].sym),0,'\0') ; }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 107 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist('\0',(yyvsp[(1) - (2)].sym),1,'\0') ; }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 108 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist((yyvsp[(1) - (3)].sml),(yyvsp[(3) - (3)].sym),0,'\0') ;}
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 109 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist((yyvsp[(1) - (4)].sml),(yyvsp[(3) - (4)].sym),1,'\0') ;}
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 112 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist('\0',(yyvsp[(1) - (1)].sym),0,'\0') ; }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 113 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.sml) = vpyy_symlist('\0',(yyvsp[(2) - (5)].sym),0,(yyvsp[(4) - (5)].sym)) ;}
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 114 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = vpyy_symlist((yyvsp[(1) - (3)].sml),(yyvsp[(3) - (3)].sym),0,'\0') ; }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 115 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.sml) = vpyy_symlist((yyvsp[(1) - (7)].sml),(yyvsp[(4) - (7)].sym),0,(yyvsp[(6) - (7)].sym)) ; }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 119 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.uni) = (yyvsp[(1) - (1)].uni) ; }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 120 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.uni) = vpyy_unitsrange((yyvsp[(1) - (6)].uni),(yyvsp[(3) - (6)].num),(yyvsp[(5) - (6)].num)) ; }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 124 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.num) = (yyvsp[(1) - (1)].num) ; }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 125 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.num) = -1e30 ; }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 128 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.num) = (yyvsp[(1) - (1)].num) ; }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 129 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.num) = -(yyvsp[(2) - (2)].num) ;}
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 130 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.num) = (yyvsp[(2) - (2)].num) ;}
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 134 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.uni) = (yyvsp[(1) - (1)].uni) ; }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 135 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.uni) = vpyy_unitsdiv((yyvsp[(1) - (3)].uni),(yyvsp[(3) - (3)].uni));}
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 136 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.uni) = vpyy_unitsmult((yyvsp[(1) - (3)].uni),(yyvsp[(3) - (3)].uni));}
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 137 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.uni) = (yyvsp[(2) - (3)].uni) ; }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 142 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 143 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 144 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 145 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tok) = (yyvsp[(1) - (1)].tok) ; }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 149 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sll) = vpyy_chain_sublist('\0',(yyvsp[(2) - (2)].sml)) ; }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 150 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { vpyy_chain_sublist((yyvsp[(1) - (3)].sll),(yyvsp[(3) - (3)].sml)) ; (yyval.sll) = (yyvsp[(1) - (3)].sll) ; }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 154 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) = '\0' ; }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 155 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.sml) =  (yyvsp[(2) - (2)].sml) ; }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 159 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.exl) = vpyy_chain_exprlist('\0',(yyvsp[(1) - (1)].exn)) ;}
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 160 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[(1) - (3)].exl),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 164 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_num_expression((yyvsp[(1) - (1)].num)) ; }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 165 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = (Expression *)(yyvsp[(1) - (1)].var) ; }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 166 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_lookup_expression((yyvsp[(1) - (4)].var),(yyvsp[(3) - (4)].exn)) ; }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 167 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('(',(yyvsp[(2) - (3)].exn),'\0') ; }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 168 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_function_expression((yyvsp[(1) - (4)].fnc),(yyvsp[(3) - (4)].exl)) ;}
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 169 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('+',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 170 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 171 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('*',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 172 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('/',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 173 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[(2) - (2)].exn),'\0') ; }
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 174 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[(2) - (2)].exn),'\0') ; }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 175 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.exn) = vpyy_operator_expression('^',(yyvsp[(1) - (3)].exn),(yyvsp[(3) - (3)].exn)) ; }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 179 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tbl) = (yyvsp[(1) - (1)].tbl) ; }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 181 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tbl) = vpyy_tablerange((yyvsp[(15) - (15)].tbl),(yyvsp[(3) - (15)].num),(yyvsp[(5) - (15)].num),(yyvsp[(9) - (15)].num),(yyvsp[(11) - (15)].num)) ; }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 186 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    { (yyval.tbl) = vpyy_tablepair('\0',(yyvsp[(2) - (5)].num),(yyvsp[(4) - (5)].num)) ;}
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 187 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
    {(yyval.tbl) = vpyy_tablepair((yyvsp[(1) - (7)].tbl),(yyvsp[(4) - (7)].num),(yyvsp[(6) - (7)].num)) ;}
    break;


/* Line 1792 of yacc.c  */
#line 1859 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.tab.c"
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
#line 193 "C:\\Users\\bob\\Dropbox\\sdev\\XMUtil\\Vensim\\VYacc.y"
