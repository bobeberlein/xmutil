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
#line 10 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:337  */

#include "../Symbol/Parse.h"
#include "VensimParseFunctions.h"
extern int vpyylex (void);
extern void vpyyerror (char const *);
#define YYSTYPE ParseUnion

#line 86 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:337  */
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
#define YYNRULES  99
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  232

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
static const yytype_uint16 yyrline[] =
{
       0,    86,    86,    87,    88,    89,    90,    91,    92,    93,
      97,    97,   101,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   121,   122,   123,   127,   128,   132,   136,   137,
     138,   139,   142,   143,   144,   145,   149,   150,   151,   152,
     153,   157,   158,   161,   162,   163,   167,   168,   169,   170,
     175,   176,   177,   178,   182,   183,   187,   188,   189,   190,
     195,   196,   201,   202,   203,   204,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   236,   237,   239,   244,   245,   250,   251,   256,   257
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

#define YYPACT_NINF -160

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-160)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      13,  -160,  -160,  -160,  -160,    -3,    10,  -160,  -160,    -6,
       2,   203,    -5,    -2,   -22,    26,  -160,  -160,   101,   250,
      25,    69,  -160,  -160,    20,  -160,  -160,  -160,    52,    27,
    -160,  -160,    89,     1,    71,    90,  -160,  -160,  -160,   -17,
     -16,   -25,    61,   250,  -160,  -160,  -160,    20,    86,   250,
     250,   250,   136,   170,   141,  -160,   -36,   170,  -160,   162,
     169,    37,   154,  -160,   159,   168,   153,   178,  -160,    20,
     250,   190,   -15,    64,  -160,  -160,   196,  -160,    75,  -160,
     222,  -160,  -160,  -160,   -17,   -17,   -16,   214,   210,   191,
     191,   128,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,  -160,
    -160,   223,    37,  -160,  -160,    37,   193,  -160,   109,   227,
    -160,   240,   225,  -160,   242,   226,  -160,   -16,  -160,  -160,
     231,  -160,   129,  -160,   131,   117,   211,    22,    22,    22,
     191,   191,    22,    22,    22,   214,   214,   214,    87,   170,
     170,    37,   235,  -160,    37,  -160,   228,   236,    70,   249,
    -160,    92,   -16,  -160,   230,  -160,   243,   244,    37,   245,
    -160,    26,  -160,   258,   260,   -16,  -160,   138,  -160,    59,
    -160,   247,    37,    97,   246,   251,   248,   -16,  -160,   254,
     255,   264,   256,  -160,    26,  -160,  -160,   253,    37,   259,
     261,  -160,   126,  -160,   257,  -160,    37,  -160,    37,   262,
     265,    37,   271,   266,   263,   143,    37,   268,   267,   269,
     195,    31,    37,   270,   153,   178,   272,   268,   215,   178,
     273,   268
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
       0,     0,     0,    96,     0,     0,    94,    91,    54,     0,
       0,     0,     0,     0,    19,    29,     0,    27,     0,    42,
       0,    41,     7,     6,     0,     0,     0,    86,     0,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    44,
      45,     0,     0,    14,    15,     0,     0,    55,     0,     0,
      56,     0,    61,    34,     0,    30,    49,     0,    48,    47,
       0,    74,     0,    71,     0,    85,    84,    82,    80,    83,
      76,    75,    87,    79,    81,    77,    78,    90,     0,    63,
      64,     0,     0,    97,     0,    11,     0,     0,     0,     0,
      31,     0,     0,    72,     0,    70,     0,     0,     0,     0,
      33,     0,    58,     0,     0,     0,    39,     0,    73,     0,
      98,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,    57,     0,    35,    40,     0,     0,     0,
       0,    99,     0,    38,     0,    16,     0,    59,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    95,    92,     0,     0,     0,    93,
       0,     0
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -160,  -160,  -160,  -160,  -160,  -160,  -160,   294,   -20,  -159,
    -160,  -160,   -70,   -21,   -37,  -160,  -160,  -160,  -160,   -19,
     -18,   134,  -160,    96,   -72
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
      63,    53,    78,    13,    68,    19,    72,    31,    82,   107,
      17,    58,   183,    36,   120,   108,   130,    83,    59,    60,
       1,    32,     2,     3,    29,    87,    39,    30,   121,    54,
      93,    89,    90,    91,    79,   202,    18,    43,    20,     4,
     111,    44,     5,    14,    15,    21,    73,   128,   129,   117,
      55,   118,    45,    46,    47,    34,    48,   161,    58,    49,
      50,   103,   104,   105,    58,    59,    60,    15,    51,   132,
      70,    59,    60,   134,    61,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   152,   177,   123,   153,    93,    58,    69,    94,   172,
      84,    85,    61,    59,    60,   186,   189,   124,    86,    95,
      96,    97,    61,   173,    84,    85,    62,   197,    71,   126,
      75,    98,    99,   100,   101,   102,   103,   104,   105,    88,
     167,    36,   166,   169,    37,    76,    93,   175,    77,    94,
     176,   193,    76,    38,    39,   220,   149,   181,    40,   225,
      95,    96,    97,   155,   107,   229,   103,   104,   105,   225,
     108,   192,    98,    99,   100,   101,   102,   103,   104,   105,
     207,    76,   133,   163,   164,   165,   107,   204,    93,    92,
     108,    94,   108,   187,   106,   209,   188,   210,   217,   109,
     213,   218,    95,    96,    97,   219,   110,   112,   115,    93,
      63,   226,    94,   113,    98,    99,   100,   101,   102,   103,
     104,   105,   114,    95,    96,    97,    22,    23,    24,    93,
      25,    26,    43,   116,   119,   125,    44,   100,   101,   102,
     103,   104,   105,    95,    96,    97,   154,    45,    46,    47,
     116,    48,    43,   223,    49,    50,    44,   100,   101,   102,
     103,   104,   105,    51,   131,   105,   156,    45,    46,    47,
     217,    48,    43,   230,    49,    50,    44,   127,   151,   157,
     158,   159,   170,    51,   178,   160,   162,    45,    46,    47,
     168,    48,   171,   174,    49,    50,   179,   184,   180,   185,
     182,   191,   194,    51,    11,   195,   196,   198,   200,   199,
     201,   203,   208,   205,   206,   214,   216,   211,     0,   212,
     215,    61,   221,   190,   222,   227,   228,   224,   231
};

static const yytype_int16 yycheck[] =
{
      21,    19,    39,     6,    24,     3,     5,    29,    33,    45,
       0,    27,   171,    30,    29,    51,    86,    42,    34,    35,
       7,    43,     9,    10,    29,    43,    43,    29,    43,     4,
       8,    49,    50,    51,    50,   194,    42,    12,    36,    26,
      61,    16,    29,    46,    47,    43,    45,    84,    85,    69,
      25,    70,    27,    28,    29,    29,    31,   127,    27,    34,
      35,    39,    40,    41,    27,    34,    35,    47,    43,    88,
      43,    34,    35,    92,    43,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   112,   162,    29,   115,     8,    27,    45,    11,    29,
      39,    40,    43,    34,    35,   175,    47,    43,    47,    22,
      23,    24,    43,    43,    39,    40,    47,   187,    29,    44,
      49,    34,    35,    36,    37,    38,    39,    40,    41,    43,
     151,    30,    45,   154,    33,    45,     8,    45,    48,    11,
      48,    44,    45,    42,    43,   217,   164,   168,    47,   221,
      22,    23,    24,    44,    45,   227,    39,    40,    41,   231,
      51,   182,    34,    35,    36,    37,    38,    39,    40,    41,
      44,    45,    44,    44,    45,    44,    45,   198,     8,    43,
      51,    11,    51,    45,    43,   206,    48,   208,    45,    27,
     211,    48,    22,    23,    24,   216,    27,    43,    45,     8,
     221,   222,    11,    44,    34,    35,    36,    37,    38,    39,
      40,    41,    44,    22,    23,    24,    13,    14,    15,     8,
      17,    18,    12,    45,    34,    29,    16,    36,    37,    38,
      39,    40,    41,    22,    23,    24,    43,    27,    28,    29,
      45,    31,    12,    48,    34,    35,    16,    36,    37,    38,
      39,    40,    41,    43,    44,    41,    29,    27,    28,    29,
      45,    31,    12,    48,    34,    35,    16,    45,    45,    29,
      45,    29,    44,    43,    44,    49,    45,    27,    28,    29,
      45,    31,    46,    34,    34,    35,    43,    29,    44,    29,
      45,    44,    46,    43,     0,    44,    48,    43,    34,    44,
      44,    48,    45,    44,    43,    34,    43,    45,    -1,    44,
      44,    43,    45,   179,    45,    45,    44,   221,    45
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
      64,    44,    71,    44,    71,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    45,    65,    65,    43,    44,    29,    29,    45,    29,
      49,    64,    45,    44,    45,    44,    45,    65,    45,    65,
      44,    46,    29,    43,    34,    45,    48,    64,    44,    43,
      44,    65,    45,    61,    29,    29,    64,    45,    48,    47,
      73,    44,    65,    44,    46,    44,    48,    64,    43,    44,
      34,    44,    61,    48,    65,    44,    43,    44,    45,    65,
      65,    45,    44,    65,    34,    44,    43,    45,    48,    65,
      76,    45,    45,    48,    75,    76,    65,    45,    44,    76,
      48,    45
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
      72,    73,    73,    73,    74,    74,    75,    75,    76,    76
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
       4,     3,     4,     5,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     2,     2,
       3,     1,    15,    17,     1,    15,     1,     3,     5,     7
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
#line 86 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { return VPTT_eqend ; }
#line 1412 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 3:
#line 87 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { return VPTT_groupstar ; }
#line 1418 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 4:
#line 88 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { return '|'; }
#line 1424 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 5:
#line 89 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {return '|'; }
#line 1430 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 6:
#line 90 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-3].eqn),(yyvsp[-1].uni)) ; return '~' ; }
#line 1436 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 7:
#line 91 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-3].eqn),(yyvsp[-1].uni)) ; return '|' ; }
#line 1442 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 8:
#line 92 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-2].eqn),NULL) ; return '~' ;}
#line 1448 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 9:
#line 93 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {vpyy_addfulleq((yyvsp[-2].eqn),NULL) ; return '|' ;}
#line 1454 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 10:
#line 97 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { vpyy_macro_start(); }
#line 1460 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 11:
#line 97 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { vpyy_macro_expression((yyvsp[-3].sym),(yyvsp[-1].exl)) ;}
#line 1466 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 12:
#line 101 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok); vpyy_macro_end(); }
#line 1472 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 13:
#line 108 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),NULL,(yyvsp[0].exl),'=') ; }
#line 1478 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 14:
#line 109 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 0) ; }
#line 1484 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 15:
#line 110 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 1) ; }
#line 1490 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 16:
#line 111 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-9].lhs),(yyvsp[-5].exn),(yyvsp[-2].tbl), 0) ; }
#line 1496 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 17:
#line 112 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),(yyvsp[0].exn),NULL,VPTT_dataequals) ; }
#line 1502 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 18:
#line 113 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[0].lhs),NULL,NULL, 0) ; }
#line 1508 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 19:
#line 114 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[-3].sym),NULL),NULL,NULL),(Expression *)vpyy_symlist_expression((yyvsp[-1].sml),(yyvsp[0].sml)),NULL,':') ; }
#line 1514 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 20:
#line 115 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[-2].sym),NULL),NULL,NULL),(Expression *)vpyy_symlist_expression(vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL),NULL),NULL,VPTT_equiv) ; }
#line 1520 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 21:
#line 116 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),(yyvsp[0].exn),NULL,'=') ; }
#line 1526 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 22:
#line 121 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.lhs) = vpyy_addexceptinterp((yyvsp[0].var),NULL,NULL) ; }
#line 1532 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 23:
#line 122 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[-1].var),(yyvsp[0].sll),NULL) ;}
#line 1538 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 24:
#line 123 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[-1].var),NULL,(yyvsp[0].tok)) ;}
#line 1544 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 25:
#line 127 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.var) = vpyy_var_expression((yyvsp[0].sym),NULL);}
#line 1550 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 26:
#line 128 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.var) = vpyy_var_expression((yyvsp[-1].sym),(yyvsp[0].sml)) ;}
#line 1556 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 27:
#line 132 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = (yyvsp[-1].sml) ;}
#line 1562 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 28:
#line 136 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1568 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 29:
#line 137 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[-1].sym),1,NULL) ; }
#line 1574 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 30:
#line 138 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1580 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 31:
#line 139 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-3].sml),(yyvsp[-1].sym),1,NULL) ;}
#line 1586 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 32:
#line 142 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1592 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 33:
#line 143 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = vpyy_symlist(NULL,(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ;}
#line 1598 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 34:
#line 144 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ; }
#line 1604 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 35:
#line 145 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = vpyy_symlist((yyvsp[-6].sml),(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ; }
#line 1610 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 36:
#line 149 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = (yyvsp[0].uni) ; }
#line 1616 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 37:
#line 150 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange((yyvsp[-5].uni),(yyvsp[-3].num),(yyvsp[-1].num),-1) ; }
#line 1622 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 38:
#line 151 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange((yyvsp[-7].uni),(yyvsp[-5].num),(yyvsp[-3].num),(yyvsp[-1].num)) ; }
#line 1628 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 39:
#line 152 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange(NULL,(yyvsp[-3].num),(yyvsp[-1].num),-1) ; }
#line 1634 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 40:
#line 153 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = vpyy_unitsrange(NULL,(yyvsp[-5].num),(yyvsp[-3].num),(yyvsp[-1].num)) ; }
#line 1640 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 41:
#line 157 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1646 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 42:
#line 158 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = -1e30 ; }
#line 1652 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 43:
#line 161 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1658 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 44:
#line 162 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = -(yyvsp[0].num) ;}
#line 1664 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 45:
#line 163 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ;}
#line 1670 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 46:
#line 167 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = (yyvsp[0].uni) ; }
#line 1676 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 47:
#line 168 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.uni) = vpyy_unitsdiv((yyvsp[-2].uni),(yyvsp[0].uni));}
#line 1682 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 48:
#line 169 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.uni) = vpyy_unitsmult((yyvsp[-2].uni),(yyvsp[0].uni));}
#line 1688 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 49:
#line 170 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.uni) = (yyvsp[-1].uni) ; }
#line 1694 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 50:
#line 175 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1700 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 51:
#line 176 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1706 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 52:
#line 177 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1712 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 53:
#line 178 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1718 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 54:
#line 182 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sll) = vpyy_chain_sublist(NULL,(yyvsp[0].sml)) ; }
#line 1724 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 55:
#line 183 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { vpyy_chain_sublist((yyvsp[-2].sll),(yyvsp[0].sml)) ; (yyval.sll) = (yyvsp[-2].sll) ; }
#line 1730 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 56:
#line 187 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1736 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 57:
#line 188 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_mapsymlist(NULL, (yyvsp[-3].sym), (yyvsp[-1].sml)); }
#line 1742 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 58:
#line 189 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1748 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 59:
#line 190 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = vpyy_mapsymlist((yyvsp[-6].sml), (yyvsp[-3].sym), (yyvsp[-1].sml));}
#line 1754 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 60:
#line 195 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = NULL ; }
#line 1760 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 61:
#line 196 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.sml) =  (yyvsp[0].sml) ; }
#line 1766 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 62:
#line 201 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = vpyy_chain_exprlist(NULL,(yyvsp[0].exn)) ;}
#line 1772 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 63:
#line 202 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1778 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 64:
#line 203 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1784 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 65:
#line 204 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = (yyvsp[-1].exl) ; }
#line 1790 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 66:
#line 208 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_num_expression((yyvsp[0].num)) ; }
#line 1796 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 67:
#line 209 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_num_expression(-1E38);}
#line 1802 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 68:
#line 210 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = (Expression *)(yyvsp[0].var) ; }
#line 1808 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 69:
#line 211 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_literal_expression((yyvsp[0].lit)) ; }
#line 1814 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 70:
#line 212 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_lookup_expression((yyvsp[-3].var),(yyvsp[-1].exl)) ; }
#line 1820 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 71:
#line 213 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('(',(yyvsp[-1].exn),NULL) ; }
#line 1826 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 72:
#line 214 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-3].fnc),(yyvsp[-1].exl)) ;}
#line 1832 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 73:
#line 215 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-4].fnc),vpyy_chain_exprlist((yyvsp[-2].exl),vpyy_literal_expression("?"))) ;}
#line 1838 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 74:
#line 216 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-2].fnc),NULL) ;}
#line 1844 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 75:
#line 217 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('+',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1850 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 76:
#line 218 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1856 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 77:
#line 219 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('*',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1862 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 78:
#line 220 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('/',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1868 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 79:
#line 221 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('<',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1874 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 80:
#line 222 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_le,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1880 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 81:
#line 223 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('>',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1886 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 82:
#line 224 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_ge,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1892 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 83:
#line 225 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_ne,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1898 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 84:
#line 226 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_or,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1904 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 85:
#line 227 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_and,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1910 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 86:
#line 228 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_not,(yyvsp[0].exn),NULL) ; }
#line 1916 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 87:
#line 229 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('=',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1922 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 88:
#line 230 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('-',NULL, (yyvsp[0].exn)) ; }
#line 1928 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 89:
#line 231 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('+',NULL, (yyvsp[0].exn)) ; }
#line 1934 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 90:
#line 232 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = vpyy_operator_expression('^',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1940 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 91:
#line 236 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1946 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 92:
#line 238 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1952 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 93:
#line 240 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-14].num),(yyvsp[-12].num),(yyvsp[-8].num),(yyvsp[-6].num)) ; }
#line 1958 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 94:
#line 244 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1964 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 95:
#line 246 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1970 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 96:
#line 250 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablevec(NULL,(yyvsp[0].num)) ;}
#line 1976 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 97:
#line 251 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.tbl) = vpyy_tablevec((yyvsp[-2].tbl),(yyvsp[0].num)) ;}
#line 1982 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 98:
#line 256 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = vpyy_tablepair(NULL,(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1988 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 99:
#line 257 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1652  */
    {(yyval.tbl) = vpyy_tablepair((yyvsp[-6].tbl),(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1994 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
    break;


#line 1998 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1652  */
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
#line 263 "C:\\sdev\\xmutil\\\\src\\Vensim\\VYacc.y" /* yacc.c:1918  */

