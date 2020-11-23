/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.3.2"

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
#define yydebug         vpyydebug
#define yynerrs         vpyynerrs

#define yylval          vpyylval
#define yychar          vpyychar

/* First part of user prologue.  */
#line 7 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:337  */

#include "../Symbol/Parse.h"
#include "VensimParseFunctions.h"
extern int vpyylex (void);
extern void vpyyerror (char const *);
#define YYSTYPE ParseUnion

#line 86 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:337  */
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
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
   by #include "VYacc.tab.hpp".  */
#ifndef YY_VPYY_C_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED
# define YY_VPYY_C_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int vpyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    VPTT_dataequals = 258,
    VPTT_with_lookup = 259,
    VPTT_map = 260,
    VPTT_equiv = 261,
    VPTT_groupstar = 262,
    VPTT_and = 263,
    VPTT_macro = 264,
    VPTT_end_of_macro = 265,
    VPTT_or = 266,
    VPTT_not = 267,
    VPTT_hold_backward = 268,
    VPTT_look_forward = 269,
    VPTT_except = 270,
    VPTT_na = 271,
    VPTT_interpolate = 272,
    VPTT_raw = 273,
    VPTT_test_input = 274,
    VPTT_the_condition = 275,
    VPTT_implies = 276,
    VPTT_ge = 277,
    VPTT_le = 278,
    VPTT_ne = 279,
    VPTT_tabbed_array = 280,
    VPTT_eqend = 281,
    VPTT_number = 282,
    VPTT_literal = 283,
    VPTT_symbol = 284,
    VPTT_units_symbol = 285,
    VPTT_function = 286
  };
#endif

/* Value type.  */


extern YYSTYPE vpyylval;

int vpyyparse (void);

#endif /* !YY_VPYY_C_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED  */



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
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

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
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  17
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   318

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  52
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  98
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  230

#define YYUNDEFTOK  2
#define YYMAXUTOK   286

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,     2,     2,     2,    32,     2,     2,
      43,    44,    39,    35,    45,    34,     2,    40,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    46,    51,
      37,    36,    38,    50,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    47,     2,    48,    41,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    33,     2,    42,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    83,    83,    84,    85,    86,    87,    88,    89,    90,
      94,    94,    98,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   118,   119,   120,   124,   125,   129,   133,   134,
     135,   136,   139,   140,   141,   142,   146,   147,   148,   149,
     150,   154,   155,   158,   159,   160,   164,   165,   166,   167,
     172,   173,   174,   175,   179,   180,   184,   185,   186,   187,
     192,   193,   198,   199,   200,   201,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     232,   233,   235,   240,   241,   246,   247,   252,   253
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VPTT_dataequals", "VPTT_with_lookup",
  "VPTT_map", "VPTT_equiv", "VPTT_groupstar", "VPTT_and", "VPTT_macro",
  "VPTT_end_of_macro", "VPTT_or", "VPTT_not", "VPTT_hold_backward",
  "VPTT_look_forward", "VPTT_except", "VPTT_na", "VPTT_interpolate",
  "VPTT_raw", "VPTT_test_input", "VPTT_the_condition", "VPTT_implies",
  "VPTT_ge", "VPTT_le", "VPTT_ne", "VPTT_tabbed_array", "VPTT_eqend",
  "VPTT_number", "VPTT_literal", "VPTT_symbol", "VPTT_units_symbol",
  "VPTT_function", "'%'", "'|'", "'-'", "'+'", "'='", "'<'", "'>'", "'*'",
  "'/'", "'^'", "'~'", "'('", "')'", "','", "':'", "'['", "']'", "'!'",
  "'?'", "';'", "$accept", "fulleq", "macrostart", "$@1", "macroend",
  "eqn", "lhs", "var", "sublist", "symlist", "subdef", "unitsrange",
  "urangenum", "number", "units", "interpmode", "exceptlist", "mapsymlist",
  "maplist", "exprlist", "exp", "tablevals", "xytablevals", "xytablevec",
  "tablepairs", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,    37,   124,    45,    43,    61,    60,    62,    42,
      47,    94,   126,    40,    41,    44,    58,    91,    93,    33,
      63,    59
};
# endif

#define YYPACT_NINF -164

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-164)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      57,  -164,  -164,  -164,  -164,    -3,    61,  -164,  -164,   -31,
       2,   147,    12,    56,    -9,    68,  -164,  -164,   154,    -4,
      53,    91,  -164,  -164,    85,  -164,  -164,  -164,   100,   135,
    -164,  -164,   151,     1,   140,    45,  -164,  -164,  -164,   -17,
     -18,   -23,    80,    -4,  -164,  -164,  -164,    85,   157,    -4,
      -4,    -4,   174,   202,   175,  -164,    11,   202,  -164,   184,
     193,    41,   179,  -164,   183,   185,   178,   188,  -164,    85,
      -4,   194,    -8,     7,  -164,  -164,   205,  -164,   142,  -164,
     190,  -164,  -164,  -164,   -17,   -17,   -18,   210,   241,   208,
     208,   133,    -4,    -4,    -4,    -4,    -4,    -4,    -4,    -4,
      -4,    -4,    -4,    -4,    -4,    -4,    -4,    -4,    -4,  -164,
    -164,   207,    41,  -164,  -164,    41,   211,  -164,    78,   226,
    -164,   227,   213,  -164,   230,   212,  -164,   -18,  -164,  -164,
     215,  -164,   170,  -164,   168,    33,   242,    14,    14,    14,
     208,   208,    14,    14,    14,   210,   210,   210,   113,   202,
     202,    41,   217,  -164,    41,  -164,   243,   225,     8,   240,
    -164,    47,   -18,  -164,  -164,   245,   246,    41,   244,  -164,
      68,  -164,   257,   262,   -18,  -164,    83,   -29,  -164,   248,
      41,    54,   247,   250,   249,   -18,  -164,   252,   254,   265,
     256,  -164,    68,  -164,  -164,   253,    41,   258,   260,  -164,
     150,  -164,   251,  -164,    41,  -164,    41,   259,   261,    41,
     272,   263,   266,    94,    41,   267,   268,   269,    95,    36,
      41,   270,   178,   188,   264,   267,   118,   188,   271,   267
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    10,    12,     2,    25,     0,     4,     5,     0,
      18,    22,     0,     0,     0,     0,    26,     1,     0,     0,
       0,     0,    52,    53,     0,    50,    51,    24,    23,     0,
      20,    32,     0,    60,    28,     0,    46,     9,     8,     0,
       0,     0,    36,     0,    67,    66,    69,    25,     0,     0,
       0,     0,    68,    17,     0,    21,    13,    62,    43,     0,
       0,     0,     0,    95,     0,     0,    93,    90,    54,     0,
       0,     0,     0,     0,    19,    29,     0,    27,     0,    42,
       0,    41,     7,     6,     0,     0,     0,    85,     0,    87,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    44,
      45,     0,     0,    14,    15,     0,     0,    55,     0,     0,
      56,     0,    61,    34,     0,    30,    49,     0,    48,    47,
       0,    73,     0,    71,     0,    84,    83,    81,    79,    82,
      75,    74,    86,    78,    80,    76,    77,    89,     0,    63,
      64,     0,     0,    96,     0,    11,     0,     0,     0,     0,
      31,     0,     0,    72,    70,     0,     0,     0,     0,    33,
       0,    58,     0,     0,     0,    39,     0,     0,    97,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,    57,     0,    35,    40,     0,     0,     0,     0,    98,
       0,    38,     0,    16,     0,    59,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    94,    91,     0,     0,     0,    92,     0,     0
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -164,  -164,  -164,  -164,  -164,  -164,  -164,   311,   -20,  -163,
    -164,  -164,   -85,   -21,   -37,  -164,  -164,  -164,  -164,   -55,
       9,   141,  -164,    93,    48
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     6,     7,    12,     8,     9,    10,    52,    16,    35,
      33,    41,    80,    81,    42,    27,    28,   122,    74,    56,
      57,    64,    65,    66,    67
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      63,   130,    78,    13,    68,    19,    72,   181,    43,    58,
      82,    18,    44,    36,    61,   118,    59,    60,   187,    83,
      31,   120,    93,    45,    46,    47,    39,    48,    53,   200,
      49,    50,    79,   132,    32,   121,   123,   171,    20,    51,
     111,    29,   161,    14,    15,    21,    73,   128,   129,   117,
     124,   172,    87,   103,   104,   105,   107,    54,    89,    90,
      91,    17,   108,    58,     1,    43,     2,     3,    58,    44,
      59,    60,   103,   104,   105,    59,    60,   176,    55,    61,
      45,    46,    47,     4,    48,    30,     5,    49,    50,   184,
      76,   152,   174,    77,   153,   175,    51,    34,   191,    76,
     195,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,    58,    84,
      85,    93,   155,   107,    94,    59,    60,    86,   185,   108,
     166,   186,    15,   168,    61,    95,    96,    97,    62,   215,
     116,    93,   216,   221,    94,    69,   179,    98,    99,   100,
     101,   102,   103,   104,   105,    95,    96,    97,   165,   190,
      22,    23,    24,   215,    25,    26,   228,    98,    99,   100,
     101,   102,   103,   104,   105,   202,    93,   133,    70,    94,
      71,    84,    85,   207,    36,   208,   126,    37,   211,    75,
      95,    96,    97,   217,   205,    76,    38,    39,    63,   224,
      88,    40,    98,    99,   100,   101,   102,   103,   104,   105,
      93,   109,   164,    94,   163,   107,    93,    92,   106,    94,
     110,   108,   112,   115,    95,    96,    97,   113,   119,   114,
      95,    96,    97,   116,   125,   127,    98,    99,   100,   101,
     102,   103,   104,   105,   100,   101,   102,   103,   104,   105,
      93,   105,   151,    43,   154,   156,   157,    44,   158,   159,
     162,   160,   167,   218,    95,    96,    97,   223,    45,    46,
      47,   170,    48,   227,   173,    49,    50,   223,   100,   101,
     102,   103,   104,   105,    51,   131,   182,   169,   177,   180,
     178,   183,   189,   192,   193,   196,   206,   194,   197,   198,
     199,   201,   203,   204,   209,   210,   212,   213,   226,   214,
      61,    11,   222,   219,   220,   225,   229,     0,   188
};

static const yytype_int16 yycheck[] =
{
      21,    86,    39,     6,    24,     3,     5,   170,    12,    27,
      33,    42,    16,    30,    43,    70,    34,    35,    47,    42,
      29,    29,     8,    27,    28,    29,    43,    31,    19,   192,
      34,    35,    50,    88,    43,    43,    29,    29,    36,    43,
      61,    29,   127,    46,    47,    43,    45,    84,    85,    69,
      43,    43,    43,    39,    40,    41,    45,     4,    49,    50,
      51,     0,    51,    27,     7,    12,     9,    10,    27,    16,
      34,    35,    39,    40,    41,    34,    35,   162,    25,    43,
      27,    28,    29,    26,    31,    29,    29,    34,    35,   174,
      45,   112,    45,    48,   115,    48,    43,    29,    44,    45,
     185,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    27,    39,
      40,     8,    44,    45,    11,    34,    35,    47,    45,    51,
     151,    48,    47,   154,    43,    22,    23,    24,    47,    45,
      45,     8,    48,    48,    11,    45,   167,    34,    35,    36,
      37,    38,    39,    40,    41,    22,    23,    24,    45,   180,
      13,    14,    15,    45,    17,    18,    48,    34,    35,    36,
      37,    38,    39,    40,    41,   196,     8,    44,    43,    11,
      29,    39,    40,   204,    30,   206,    44,    33,   209,    49,
      22,    23,    24,   214,    44,    45,    42,    43,   219,   220,
      43,    47,    34,    35,    36,    37,    38,    39,    40,    41,
       8,    27,    44,    11,    44,    45,     8,    43,    43,    11,
      27,    51,    43,    45,    22,    23,    24,    44,    34,    44,
      22,    23,    24,    45,    29,    45,    34,    35,    36,    37,
      38,    39,    40,    41,    36,    37,    38,    39,    40,    41,
       8,    41,    45,    12,    43,    29,    29,    16,    45,    29,
      45,    49,    45,   215,    22,    23,    24,   219,    27,    28,
      29,    46,    31,   225,    34,    34,    35,   229,    36,    37,
      38,    39,    40,    41,    43,    44,    29,    44,    43,    45,
      44,    29,    44,    46,    44,    43,    45,    48,    44,    34,
      44,    48,    44,    43,    45,    44,    34,    44,    44,    43,
      43,     0,   219,    45,    45,    45,    45,    -1,   177
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,     9,    10,    26,    29,    53,    54,    56,    57,
      58,    59,    55,     6,    46,    47,    60,     0,    42,     3,
      36,    43,    13,    14,    15,    17,    18,    67,    68,    29,
      29,    29,    43,    62,    29,    61,    30,    33,    42,    43,
      47,    63,    66,    12,    16,    27,    28,    29,    31,    34,
      35,    43,    59,    72,     4,    25,    71,    72,    27,    34,
      35,    43,    47,    65,    73,    74,    75,    76,    60,    45,
      43,    29,     5,    45,    70,    49,    45,    48,    66,    50,
      64,    65,    33,    42,    39,    40,    47,    72,    43,    72,
      72,    72,    43,     8,    11,    22,    23,    24,    34,    35,
      36,    37,    38,    39,    40,    41,    43,    45,    51,    27,
      27,    65,    43,    44,    44,    45,    45,    60,    71,    34,
      29,    43,    69,    29,    43,    29,    44,    45,    66,    66,
      64,    44,    71,    44,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    45,    65,    65,    43,    44,    29,    29,    45,    29,
      49,    64,    45,    44,    44,    45,    65,    45,    65,    44,
      46,    29,    43,    34,    45,    48,    64,    43,    44,    65,
      45,    61,    29,    29,    64,    45,    48,    47,    73,    44,
      65,    44,    46,    44,    48,    64,    43,    44,    34,    44,
      61,    48,    65,    44,    43,    44,    45,    65,    65,    45,
      44,    65,    34,    44,    43,    45,    48,    65,    76,    45,
      45,    48,    75,    76,    65,    45,    44,    76,    48,    45
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    52,    53,    53,    53,    53,    53,    53,    53,    53,
      55,    54,    56,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    58,    58,    58,    59,    59,    60,    61,    61,
      61,    61,    62,    62,    62,    62,    63,    63,    63,    63,
      63,    64,    64,    65,    65,    65,    66,    66,    66,    66,
      67,    67,    67,    67,    68,    68,    69,    69,    69,    69,
      70,    70,    71,    71,    71,    71,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      73,    73,    73,    74,    74,    75,    75,    76,    76
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     4,     4,     3,     3,
       0,     6,     1,     3,     4,     4,    10,     3,     1,     4,
       3,     3,     1,     2,     2,     1,     2,     3,     1,     2,
       3,     4,     1,     5,     3,     7,     1,     6,     8,     5,
       7,     1,     1,     1,     2,     2,     1,     3,     3,     3,
       1,     1,     1,     1,     2,     3,     1,     5,     3,     7,
       0,     2,     1,     3,     3,     2,     1,     1,     1,     1,
       4,     3,     4,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     2,     2,     3,
       1,    15,    17,     1,    15,     1,     3,     5,     7
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
            else
              goto append;

          append:
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

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
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
    default: /* Avoid compiler warnings. */
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
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
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
# else /* defined YYSTACK_RELOCATE */
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
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
      yychar = yylex ();
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

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
#line 83 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { return VPTT_eqend ; }
#line 1409 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 3:
#line 84 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { return VPTT_groupstar ; }
#line 1415 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 4:
#line 85 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { return '|'; }
#line 1421 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 5:
#line 86 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {return '|'; }
#line 1427 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 6:
#line 87 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-3].eqn),(yyvsp[-1].uni)) ; return '~' ; }
#line 1433 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 7:
#line 88 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-3].eqn),(yyvsp[-1].uni)) ; return '|' ; }
#line 1439 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 8:
#line 89 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-2].eqn),NULL) ; return '~' ;}
#line 1445 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 9:
#line 90 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-2].eqn),NULL) ; return '|' ;}
#line 1451 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 10:
#line 94 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { vpyy_macro_start(); }
#line 1457 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 11:
#line 94 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { vpyy_macro_expression((yyvsp[-3].sym),(yyvsp[-1].exl)) ;}
#line 1463 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 12:
#line 98 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok); vpyy_macro_end(); }
#line 1469 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 13:
#line 105 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),NULL,(yyvsp[0].exl),'=') ; }
#line 1475 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 14:
#line 106 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 0) ; }
#line 1481 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 15:
#line 107 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 1) ; }
#line 1487 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 16:
#line 108 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-9].lhs),(yyvsp[-5].exn),(yyvsp[-2].tbl), 0) ; }
#line 1493 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 17:
#line 109 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),(yyvsp[0].exn),NULL,VPTT_dataequals) ; }
#line 1499 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 18:
#line 110 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[0].lhs),NULL,NULL, 0) ; }
#line 1505 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 19:
#line 111 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[-3].sym),NULL),NULL,NULL),(Expression *)vpyy_symlist_expression((yyvsp[-1].sml),(yyvsp[0].sml)),NULL,':') ; }
#line 1511 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 20:
#line 112 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[-2].sym),NULL),NULL,NULL),(Expression *)vpyy_symlist_expression(vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL),NULL),NULL,VPTT_equiv) ; }
#line 1517 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 21:
#line 113 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),(yyvsp[0].exn),NULL,'=') ; }
#line 1523 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 22:
#line 118 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.lhs) = vpyy_addexceptinterp((yyvsp[0].var),NULL,NULL) ; }
#line 1529 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 23:
#line 119 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[-1].var),(yyvsp[0].sll),NULL) ;}
#line 1535 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 24:
#line 120 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[-1].var),NULL,(yyvsp[0].tok)) ;}
#line 1541 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 25:
#line 124 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.var) = vpyy_var_expression((yyvsp[0].sym),NULL);}
#line 1547 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 26:
#line 125 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.var) = vpyy_var_expression((yyvsp[-1].sym),(yyvsp[0].sml)) ;}
#line 1553 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 27:
#line 129 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = (yyvsp[-1].sml) ;}
#line 1559 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 28:
#line 133 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1565 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 29:
#line 134 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[-1].sym),1,NULL) ; }
#line 1571 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 30:
#line 135 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1577 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 31:
#line 136 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-3].sml),(yyvsp[-1].sym),1,NULL) ;}
#line 1583 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 32:
#line 139 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1589 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 33:
#line 140 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = vpyy_symlist(NULL,(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ;}
#line 1595 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 34:
#line 141 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ; }
#line 1601 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 35:
#line 142 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = vpyy_symlist((yyvsp[-6].sml),(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ; }
#line 1607 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 36:
#line 146 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = (yyvsp[0].uni) ; }
#line 1613 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 37:
#line 147 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange((yyvsp[-5].uni),(yyvsp[-3].num),(yyvsp[-1].num),-1) ; }
#line 1619 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 38:
#line 148 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange((yyvsp[-7].uni),(yyvsp[-5].num),(yyvsp[-3].num),(yyvsp[-1].num)) ; }
#line 1625 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 39:
#line 149 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange(NULL,(yyvsp[-3].num),(yyvsp[-1].num),-1) ; }
#line 1631 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 40:
#line 150 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange(NULL,(yyvsp[-5].num),(yyvsp[-3].num),(yyvsp[-1].num)) ; }
#line 1637 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 41:
#line 154 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1643 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 42:
#line 155 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = -1e30 ; }
#line 1649 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 43:
#line 158 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1655 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 44:
#line 159 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = -(yyvsp[0].num) ;}
#line 1661 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 45:
#line 160 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ;}
#line 1667 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 46:
#line 164 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = (yyvsp[0].uni) ; }
#line 1673 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 47:
#line 165 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.uni) = vpyy_unitsdiv((yyvsp[-2].uni),(yyvsp[0].uni));}
#line 1679 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 48:
#line 166 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.uni) = vpyy_unitsmult((yyvsp[-2].uni),(yyvsp[0].uni));}
#line 1685 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 49:
#line 167 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = (yyvsp[-1].uni) ; }
#line 1691 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 50:
#line 172 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1697 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 51:
#line 173 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1703 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 52:
#line 174 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1709 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 53:
#line 175 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1715 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 54:
#line 179 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sll) = vpyy_chain_sublist(NULL,(yyvsp[0].sml)) ; }
#line 1721 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 55:
#line 180 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { vpyy_chain_sublist((yyvsp[-2].sll),(yyvsp[0].sml)) ; (yyval.sll) = (yyvsp[-2].sll) ; }
#line 1727 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 56:
#line 184 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1733 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 57:
#line 185 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_mapsymlist(NULL, (yyvsp[-3].sym), (yyvsp[-1].sml)); }
#line 1739 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 58:
#line 186 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1745 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 59:
#line 187 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_mapsymlist((yyvsp[-6].sml), (yyvsp[-3].sym), (yyvsp[-1].sml));}
#line 1751 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 60:
#line 192 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = NULL ; }
#line 1757 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 61:
#line 193 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) =  (yyvsp[0].sml) ; }
#line 1763 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 62:
#line 198 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = vpyy_chain_exprlist(NULL,(yyvsp[0].exn)) ;}
#line 1769 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 63:
#line 199 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1775 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 64:
#line 200 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1781 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 65:
#line 201 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = (yyvsp[-1].exl) ; }
#line 1787 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 66:
#line 205 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_num_expression((yyvsp[0].num)) ; }
#line 1793 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 67:
#line 206 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_num_expression(-1E38);}
#line 1799 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 68:
#line 207 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = (Expression *)(yyvsp[0].var) ; }
#line 1805 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 69:
#line 208 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_literal_expression((yyvsp[0].lit)) ; }
#line 1811 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 70:
#line 209 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_lookup_expression((yyvsp[-3].var),(yyvsp[-1].exn)) ; }
#line 1817 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 71:
#line 210 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('(',(yyvsp[-1].exn),NULL) ; }
#line 1823 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 72:
#line 211 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-3].fnc),(yyvsp[-1].exl)) ;}
#line 1829 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 73:
#line 212 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-2].fnc),NULL) ;}
#line 1835 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 74:
#line 213 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('+',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1841 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 75:
#line 214 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1847 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 76:
#line 215 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('*',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1853 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 77:
#line 216 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('/',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1859 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 78:
#line 217 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('<',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1865 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 79:
#line 218 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_le,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1871 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 80:
#line 219 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('>',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1877 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 81:
#line 220 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_ge,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1883 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 82:
#line 221 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_ne,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1889 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 83:
#line 222 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_or,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1895 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 84:
#line 223 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_and,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1901 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 85:
#line 224 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_not,(yyvsp[0].exn),NULL) ; }
#line 1907 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 86:
#line 225 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('=',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1913 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 87:
#line 226 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('-',NULL, (yyvsp[0].exn)) ; }
#line 1919 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 88:
#line 227 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('+',NULL, (yyvsp[0].exn)) ; }
#line 1925 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 89:
#line 228 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('^',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1931 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 90:
#line 232 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1937 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 91:
#line 234 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1943 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 92:
#line 236 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-14].num),(yyvsp[-12].num),(yyvsp[-8].num),(yyvsp[-6].num)) ; }
#line 1949 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 93:
#line 240 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1955 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 94:
#line 242 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1961 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 95:
#line 246 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablevec(NULL,(yyvsp[0].num)) ;}
#line 1967 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 96:
#line 247 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.tbl) = vpyy_tablevec((yyvsp[-2].tbl),(yyvsp[0].num)) ;}
#line 1973 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 97:
#line 252 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablepair(NULL,(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1979 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 98:
#line 253 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.tbl) = vpyy_tablepair((yyvsp[-6].tbl),(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1985 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;


#line 1989 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}
#line 259 "C:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1918  */
