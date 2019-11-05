/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.1"

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

/* Copy the first part of user declarations.  */
#line 7 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:339  */

#include "../Symbol/Parse.h"
#include "VensimParseFunctions.h"
extern int vpyylex (void);
extern void vpyyerror (char const *);
#define YYSTYPE ParseUnion

#line 82 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
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
#ifndef YY_VPYY_D_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED
# define YY_VPYY_D_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED
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
    VPTT_tabbed_array = 279,
    VPTT_eqend = 280,
    VPTT_number = 281,
    VPTT_literal = 282,
    VPTT_symbol = 283,
    VPTT_units_symbol = 284,
    VPTT_function = 285
  };
#endif

/* Value type.  */


extern YYSTYPE vpyylval;

int vpyyparse (void);

#endif /* !YY_VPYY_D_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 159 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:358  */

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

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
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
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   306

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  94
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  224

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   285

#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,     2,     2,     2,    31,     2,     2,
      42,    43,    38,    34,    44,    33,     2,    39,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    45,     2,
      36,    35,    37,    49,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    46,     2,    47,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    32,     2,    41,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    82,    82,    83,    84,    85,    86,    87,    88,    89,
      93,    93,    97,   104,   105,   106,   107,   108,   109,   110,
     111,   116,   117,   118,   122,   123,   127,   131,   132,   133,
     134,   137,   138,   139,   140,   144,   145,   146,   147,   148,
     152,   153,   156,   157,   158,   162,   163,   164,   165,   170,
     171,   172,   173,   177,   178,   182,   183,   184,   185,   190,
     191,   196,   197,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   227,   228,   230,   235,
     236,   241,   242,   247,   248
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
  "VPTT_ge", "VPTT_le", "VPTT_tabbed_array", "VPTT_eqend", "VPTT_number",
  "VPTT_literal", "VPTT_symbol", "VPTT_units_symbol", "VPTT_function",
  "'%'", "'|'", "'-'", "'+'", "'='", "'<'", "'>'", "'*'", "'/'", "'^'",
  "'~'", "'('", "')'", "','", "':'", "'['", "']'", "'!'", "'?'", "$accept",
  "fulleq", "macrostart", "$@1", "macroend", "eqn", "lhs", "var",
  "sublist", "symlist", "subdef", "unitsrange", "urangenum", "number",
  "units", "interpmode", "exceptlist", "mapsymlist", "maplist", "exprlist",
  "exp", "tablevals", "xytablevals", "xytablevec", "tablepairs", YY_NULLPTR
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
     285,    37,   124,    45,    43,    61,    60,    62,    42,    47,
      94,   126,    40,    41,    44,    58,    91,    93,    33,    63
};
# endif

#define YYPACT_NINF -173

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-173)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      58,  -173,  -173,  -173,  -173,    16,    15,  -173,  -173,   -34,
       0,   142,     8,   -23,    41,  -173,  -173,   150,    -4,    48,
      88,  -173,  -173,    -3,  -173,  -173,  -173,    33,    42,  -173,
      64,     1,    67,    69,  -173,  -173,  -173,   -11,   -17,    12,
     -25,    -4,  -173,  -173,  -173,    -3,    82,    -4,    -4,    -4,
      85,   197,    87,  -173,    89,   197,  -173,   126,   132,    61,
     127,  -173,   152,   163,   141,   174,  -173,    -3,    -4,   190,
      -8,    38,  -173,  -173,   196,  -173,    80,  -173,   183,  -173,
    -173,  -173,   -11,   -11,   -17,   188,   235,   203,   203,   128,
      -4,    -4,    -4,    -4,    -4,    -4,    -4,    -4,    -4,    -4,
      -4,    -4,    -4,    -4,    -4,  -173,  -173,   185,    61,  -173,
    -173,    61,   204,  -173,   146,   217,  -173,   220,   205,  -173,
     222,   206,  -173,   -17,  -173,  -173,   208,  -173,   166,  -173,
     164,   177,   236,    19,    19,   203,   203,    19,    19,    19,
     188,   188,   188,   109,   197,    61,   209,  -173,    61,  -173,
     212,   211,    84,   224,  -173,    91,   -17,  -173,  -173,   218,
     221,    61,   223,  -173,    41,  -173,   238,   242,   -17,  -173,
      93,    47,  -173,   237,    61,   169,   234,   239,   240,   -17,
    -173,   241,   243,   248,   245,  -173,    41,  -173,  -173,   244,
      61,   246,   250,  -173,   178,  -173,   249,  -173,    61,  -173,
      61,   251,   247,    61,   252,   253,   255,   129,    61,   256,
     257,   258,   130,    37,    61,   259,   141,   174,   261,   256,
     137,   174,   262,   256
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    10,    12,     2,    24,     0,     4,     5,     0,
      18,    21,     0,     0,     0,    25,     1,     0,     0,     0,
       0,    51,    52,     0,    49,    50,    23,    22,     0,    31,
       0,    59,    27,     0,    45,     9,     8,     0,     0,     0,
      35,     0,    64,    63,    66,    24,     0,     0,     0,     0,
      65,    17,     0,    20,    13,    61,    42,     0,     0,     0,
       0,    91,     0,     0,    89,    86,    53,     0,     0,     0,
       0,     0,    19,    28,     0,    26,     0,    41,     0,    40,
       7,     6,     0,     0,     0,    81,     0,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    43,    44,     0,     0,    14,
      15,     0,     0,    54,     0,     0,    55,     0,    60,    33,
       0,    29,    48,     0,    47,    46,     0,    70,     0,    68,
       0,    80,    79,    78,    76,    72,    71,    82,    75,    77,
      73,    74,    85,     0,    62,     0,     0,    92,     0,    11,
       0,     0,     0,     0,    30,     0,     0,    69,    67,     0,
       0,     0,     0,    32,     0,    57,     0,     0,     0,    38,
       0,     0,    93,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,    56,     0,    34,    39,     0,
       0,     0,     0,    94,     0,    37,     0,    16,     0,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    90,    87,     0,     0,
       0,    88,     0,     0
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -173,  -173,  -173,  -173,  -173,  -173,  -173,   284,   -21,  -153,
    -173,  -173,   -83,   -20,   -33,  -173,  -173,  -173,  -173,   -58,
       7,   123,  -173,    86,  -172
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     6,     7,    12,     8,     9,    10,    50,    15,    33,
      31,    39,    78,    79,    40,    26,    27,   118,    72,    54,
      55,    62,    63,    64,    65
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      61,   126,    66,    18,    76,    29,    70,    17,    41,    56,
     114,   175,    42,    82,    83,    16,    57,    58,    34,    30,
     116,    84,    43,    44,    45,    51,    46,    91,   128,    47,
      48,    37,    77,   194,   117,    19,    28,   212,    49,   107,
     155,   217,    20,    14,    80,    71,   113,   221,    85,   124,
     125,   217,    52,    81,    87,    88,    89,   100,   101,   102,
      41,    13,    14,    56,    42,     1,   119,     2,     3,    32,
      57,    58,    53,   170,    43,    44,    45,    67,    46,    59,
     120,    47,    48,     4,    68,   178,     5,    56,   146,    59,
      49,   147,    69,   181,    57,    58,   189,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   165,    74,    56,    73,    75,    91,    82,    83,
      92,    57,    58,   122,    86,   160,   166,    90,   162,   103,
      59,    93,    94,   104,    60,   168,    91,   179,   169,    92,
     180,   173,    95,    96,    97,    98,    99,   100,   101,   102,
      93,    94,   105,   159,   184,    21,    22,    23,   106,    24,
      25,    95,    96,    97,    98,    99,   100,   101,   102,   108,
     196,   129,    91,   209,   112,    92,   210,   215,   201,    34,
     202,   209,    35,   205,   222,   111,    93,    94,   211,   149,
     104,    36,    37,    61,   218,   109,    38,    95,    96,    97,
      98,    99,   100,   101,   102,    91,   110,   158,    92,   157,
     104,    91,   185,    74,    92,   100,   101,   102,   112,    93,
      94,   199,    74,   115,   121,    93,    94,   123,   102,   145,
      95,    96,    97,    98,    99,   100,   101,   102,    97,    98,
      99,   100,   101,   102,    91,   150,   148,    41,   151,   152,
     153,    42,   156,   161,   154,   163,   164,   167,    93,    94,
     171,    43,    44,    45,   172,    46,   176,   174,    47,    48,
     177,    97,    98,    99,   100,   101,   102,    49,   127,   186,
     183,   192,   187,   190,    11,   206,   191,   188,   193,   197,
     204,   195,   198,   200,   182,   203,   207,   208,    59,   216,
       0,   213,   214,   219,   220,     0,   223
};

static const yytype_int16 yycheck[] =
{
      20,    84,    23,     3,    37,    28,     5,    41,    12,    26,
      68,   164,    16,    38,    39,     0,    33,    34,    29,    42,
      28,    46,    26,    27,    28,    18,    30,     8,    86,    33,
      34,    42,    49,   186,    42,    35,    28,   209,    42,    59,
     123,   213,    42,    46,    32,    44,    67,   219,    41,    82,
      83,   223,     4,    41,    47,    48,    49,    38,    39,    40,
      12,    45,    46,    26,    16,     7,    28,     9,    10,    28,
      33,    34,    24,   156,    26,    27,    28,    44,    30,    42,
      42,    33,    34,    25,    42,   168,    28,    26,   108,    42,
      42,   111,    28,    46,    33,    34,   179,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,    28,    44,    26,    48,    47,     8,    38,    39,
      11,    33,    34,    43,    42,   145,    42,    42,   148,    42,
      42,    22,    23,    44,    46,    44,     8,    44,    47,    11,
      47,   161,    33,    34,    35,    36,    37,    38,    39,    40,
      22,    23,    26,    44,   174,    13,    14,    15,    26,    17,
      18,    33,    34,    35,    36,    37,    38,    39,    40,    42,
     190,    43,     8,    44,    44,    11,    47,    47,   198,    29,
     200,    44,    32,   203,    47,    44,    22,    23,   208,    43,
      44,    41,    42,   213,   214,    43,    46,    33,    34,    35,
      36,    37,    38,    39,    40,     8,    43,    43,    11,    43,
      44,     8,    43,    44,    11,    38,    39,    40,    44,    22,
      23,    43,    44,    33,    28,    22,    23,    44,    40,    44,
      33,    34,    35,    36,    37,    38,    39,    40,    35,    36,
      37,    38,    39,    40,     8,    28,    42,    12,    28,    44,
      28,    16,    44,    44,    48,    43,    45,    33,    22,    23,
      42,    26,    27,    28,    43,    30,    28,    44,    33,    34,
      28,    35,    36,    37,    38,    39,    40,    42,    43,    45,
      43,    33,    43,    42,     0,    33,    43,    47,    43,    43,
      43,    47,    42,    44,   171,    44,    43,    42,    42,   213,
      -1,    44,    44,    44,    43,    -1,    44
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,     9,    10,    25,    28,    51,    52,    54,    55,
      56,    57,    53,    45,    46,    58,     0,    41,     3,    35,
      42,    13,    14,    15,    17,    18,    65,    66,    28,    28,
      42,    60,    28,    59,    29,    32,    41,    42,    46,    61,
      64,    12,    16,    26,    27,    28,    30,    33,    34,    42,
      57,    70,     4,    24,    69,    70,    26,    33,    34,    42,
      46,    63,    71,    72,    73,    74,    58,    44,    42,    28,
       5,    44,    68,    48,    44,    47,    64,    49,    62,    63,
      32,    41,    38,    39,    46,    70,    42,    70,    70,    70,
      42,     8,    11,    22,    23,    33,    34,    35,    36,    37,
      38,    39,    40,    42,    44,    26,    26,    63,    42,    43,
      43,    44,    44,    58,    69,    33,    28,    42,    67,    28,
      42,    28,    43,    44,    64,    64,    62,    43,    69,    43,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    44,    63,    63,    42,    43,
      28,    28,    44,    28,    48,    62,    44,    43,    43,    44,
      63,    44,    63,    43,    45,    28,    42,    33,    44,    47,
      62,    42,    43,    63,    44,    59,    28,    28,    62,    44,
      47,    46,    71,    43,    63,    43,    45,    43,    47,    62,
      42,    43,    33,    43,    59,    47,    63,    43,    42,    43,
      44,    63,    63,    44,    43,    63,    33,    43,    42,    44,
      47,    63,    74,    44,    44,    47,    73,    74,    63,    44,
      43,    74,    47,    44
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    51,    51,    51,    51,    51,    51,    51,
      53,    52,    54,    55,    55,    55,    55,    55,    55,    55,
      55,    56,    56,    56,    57,    57,    58,    59,    59,    59,
      59,    60,    60,    60,    60,    61,    61,    61,    61,    61,
      62,    62,    63,    63,    63,    64,    64,    64,    64,    65,
      65,    65,    65,    66,    66,    67,    67,    67,    67,    68,
      68,    69,    69,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    71,    71,    71,    72,
      72,    73,    73,    74,    74
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     4,     4,     3,     3,
       0,     6,     1,     3,     4,     4,    10,     3,     1,     4,
       3,     1,     2,     2,     1,     2,     3,     1,     2,     3,
       4,     1,     5,     3,     7,     1,     6,     8,     5,     7,
       1,     1,     1,     2,     2,     1,     3,     3,     3,     1,
       1,     1,     1,     2,     3,     1,     5,     3,     7,     0,
       2,     1,     3,     1,     1,     1,     1,     4,     3,     4,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     2,     2,     3,     1,    15,    17,     1,
      15,     1,     3,     5,     7
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
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
                       &(yyvsp[(yyi + 1) - (yynrhs)])
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
                  (unsigned long) yystacksize));

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
| yyreduce -- Do a reduction.  |
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
#line 82 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { return VPTT_eqend ; }
#line 1400 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 3:
#line 83 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { return VPTT_groupstar ; }
#line 1406 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 4:
#line 84 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { return '|'; }
#line 1412 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 5:
#line 85 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {return '|'; }
#line 1418 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 6:
#line 86 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {vpyy_addfulleq((yyvsp[-3].eqn),(yyvsp[-1].uni)) ; return '~' ; }
#line 1424 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 7:
#line 87 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {vpyy_addfulleq((yyvsp[-3].eqn),(yyvsp[-1].uni)) ; return '|' ; }
#line 1430 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 8:
#line 88 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {vpyy_addfulleq((yyvsp[-2].eqn),NULL) ; return '~' ;}
#line 1436 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 9:
#line 89 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {vpyy_addfulleq((yyvsp[-2].eqn),NULL) ; return '|' ;}
#line 1442 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 10:
#line 93 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { vpyy_macro_start(); }
#line 1448 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 11:
#line 93 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { vpyy_macro_expression((yyvsp[-3].sym),(yyvsp[-1].exl)) ;}
#line 1454 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 12:
#line 97 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tok) = (yyvsp[0].tok); vpyy_macro_end(); }
#line 1460 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 13:
#line 104 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),NULL,(yyvsp[0].exl),'=') ; }
#line 1466 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 14:
#line 105 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 0) ; }
#line 1472 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 15:
#line 106 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 1) ; }
#line 1478 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 16:
#line 107 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[-9].lhs),(yyvsp[-5].exn),(yyvsp[-2].tbl), 0) ; }
#line 1484 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 17:
#line 108 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),(yyvsp[0].exn),NULL,VPTT_dataequals) ; }
#line 1490 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 18:
#line 109 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.eqn) = vpyy_add_lookup((yyvsp[0].lhs),NULL,NULL, 0) ; }
#line 1496 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 19:
#line 110 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.eqn) = vpyy_addeq(vpyy_addexceptinterp(vpyy_var_expression((yyvsp[-3].sym),NULL),NULL,NULL),(Expression *)vpyy_symlist_expression((yyvsp[-1].sml),(yyvsp[0].sml)),NULL,':') ; }
#line 1502 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 20:
#line 111 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.eqn) = vpyy_addeq((yyvsp[-2].lhs),(yyvsp[0].exn),NULL,'=') ; }
#line 1508 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 21:
#line 116 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.lhs) = vpyy_addexceptinterp((yyvsp[0].var),NULL,NULL) ; }
#line 1514 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 22:
#line 117 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[-1].var),(yyvsp[0].sll),NULL) ;}
#line 1520 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 23:
#line 118 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.lhs) = vpyy_addexceptinterp((yyvsp[-1].var),NULL,(yyvsp[0].tok)) ;}
#line 1526 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 24:
#line 122 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.var) = vpyy_var_expression((yyvsp[0].sym),NULL);}
#line 1532 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 25:
#line 123 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.var) = vpyy_var_expression((yyvsp[-1].sym),(yyvsp[0].sml)) ;}
#line 1538 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 26:
#line 127 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.sml) = (yyvsp[-1].sml) ;}
#line 1544 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 27:
#line 131 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1550 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 28:
#line 132 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[-1].sym),1,NULL) ; }
#line 1556 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 29:
#line 133 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1562 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 30:
#line 134 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-3].sml),(yyvsp[-1].sym),1,NULL) ;}
#line 1568 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 31:
#line 137 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1574 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 32:
#line 138 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.sml) = vpyy_symlist(NULL,(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ;}
#line 1580 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 33:
#line 139 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ; }
#line 1586 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 34:
#line 140 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.sml) = vpyy_symlist((yyvsp[-6].sml),(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ; }
#line 1592 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 35:
#line 144 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = (yyvsp[0].uni) ; }
#line 1598 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 36:
#line 145 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = vpyy_unitsrange((yyvsp[-5].uni),(yyvsp[-3].num),(yyvsp[-1].num),-1) ; }
#line 1604 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 37:
#line 146 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = vpyy_unitsrange((yyvsp[-7].uni),(yyvsp[-5].num),(yyvsp[-3].num),(yyvsp[-1].num)) ; }
#line 1610 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 38:
#line 147 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = vpyy_unitsrange(NULL,(yyvsp[-3].num),(yyvsp[-1].num),-1) ; }
#line 1616 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 39:
#line 148 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = vpyy_unitsrange(NULL,(yyvsp[-5].num),(yyvsp[-3].num),(yyvsp[-1].num)) ; }
#line 1622 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 40:
#line 152 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1628 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 41:
#line 153 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.num) = -1e30 ; }
#line 1634 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 42:
#line 156 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1640 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 43:
#line 157 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.num) = -(yyvsp[0].num) ;}
#line 1646 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 44:
#line 158 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.num) = (yyvsp[0].num) ;}
#line 1652 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 45:
#line 162 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = (yyvsp[0].uni) ; }
#line 1658 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 46:
#line 163 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.uni) = vpyy_unitsdiv((yyvsp[-2].uni),(yyvsp[0].uni));}
#line 1664 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 47:
#line 164 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.uni) = vpyy_unitsmult((yyvsp[-2].uni),(yyvsp[0].uni));}
#line 1670 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 48:
#line 165 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.uni) = (yyvsp[-1].uni) ; }
#line 1676 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 49:
#line 170 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1682 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 50:
#line 171 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1688 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 51:
#line 172 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1694 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 52:
#line 173 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tok) = (yyvsp[0].tok) ; }
#line 1700 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 53:
#line 177 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sll) = vpyy_chain_sublist(NULL,(yyvsp[0].sml)) ; }
#line 1706 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 54:
#line 178 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { vpyy_chain_sublist((yyvsp[-2].sll),(yyvsp[0].sml)) ; (yyval.sll) = (yyvsp[-2].sll) ; }
#line 1712 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 55:
#line 182 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1718 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 56:
#line 183 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_mapsymlist(NULL, (yyvsp[-3].sym), (yyvsp[-1].sml)); }
#line 1724 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 57:
#line 184 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1730 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 58:
#line 185 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = vpyy_mapsymlist((yyvsp[-6].sml), (yyvsp[-3].sym), (yyvsp[-1].sml));}
#line 1736 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 59:
#line 190 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) = NULL ; }
#line 1742 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 60:
#line 191 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.sml) =  (yyvsp[0].sml) ; }
#line 1748 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 61:
#line 196 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.exl) = vpyy_chain_exprlist(NULL,(yyvsp[0].exn)) ;}
#line 1754 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 62:
#line 197 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.exl) = vpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1760 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 63:
#line 201 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_num_expression((yyvsp[0].num)) ; }
#line 1766 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 64:
#line 202 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_num_expression(-1E38);}
#line 1772 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 65:
#line 203 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = (Expression *)(yyvsp[0].var) ; }
#line 1778 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 66:
#line 204 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_literal_expression((yyvsp[0].lit)) ; }
#line 1784 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 67:
#line 205 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_lookup_expression((yyvsp[-3].var),(yyvsp[-1].exn)) ; }
#line 1790 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 68:
#line 206 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('(',(yyvsp[-1].exn),NULL) ; }
#line 1796 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 69:
#line 207 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-3].fnc),(yyvsp[-1].exl)) ;}
#line 1802 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 70:
#line 208 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_function_expression((yyvsp[-2].fnc),NULL) ;}
#line 1808 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 71:
#line 209 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('+',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1814 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 72:
#line 210 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('-',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1820 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 73:
#line 211 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('*',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1826 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 74:
#line 212 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('/',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1832 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 75:
#line 213 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('<',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1838 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 76:
#line 214 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_le,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1844 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 77:
#line 215 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('>',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1850 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 78:
#line 216 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_ge,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1856 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 79:
#line 217 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_or,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1862 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 80:
#line 218 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_and,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1868 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 81:
#line 219 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression(VPTT_not,(yyvsp[0].exn),NULL) ; }
#line 1874 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 82:
#line 220 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('=',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1880 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 83:
#line 221 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('-',NULL, (yyvsp[0].exn)) ; }
#line 1886 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 84:
#line 222 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('+',NULL, (yyvsp[0].exn)) ; }
#line 1892 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 85:
#line 223 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.exn) = vpyy_operator_expression('^',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1898 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 86:
#line 227 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1904 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 87:
#line 229 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1910 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 88:
#line 231 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-14].num),(yyvsp[-12].num),(yyvsp[-8].num),(yyvsp[-6].num)) ; }
#line 1916 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 89:
#line 235 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1922 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 90:
#line 237 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = vpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1928 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 91:
#line 241 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = vpyy_tablevec(NULL,(yyvsp[0].num)) ;}
#line 1934 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 92:
#line 242 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.tbl) = vpyy_tablevec((yyvsp[-2].tbl),(yyvsp[0].num)) ;}
#line 1940 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 93:
#line 247 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    { (yyval.tbl) = vpyy_tablepair(NULL,(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1946 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;

  case 94:
#line 248 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1651  */
    {(yyval.tbl) = vpyy_tablepair((yyvsp[-6].tbl),(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1952 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
    break;


#line 1956 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.tab.cpp" /* yacc.c:1651  */
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

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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
#line 254 "D:\\sdev\\xmutil\\src\\Vensim\\VYacc.y" /* yacc.c:1910  */
