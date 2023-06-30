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
#define yyparse         dpyyparse
#define yylex           dpyylex
#define yyerror         dpyyerror
#define yydebug         dpyydebug
#define yynerrs         dpyynerrs

#define yylval          dpyylval
#define yychar          dpyychar

/* First part of user prologue.  */
#line 10 "DYacc.y" /* yacc.c:337  */

#include "../Log.h"
#include "../Symbol/Parse.h"
#include "DynamoParseFunctions.h"
extern int dpyylex (void);
extern void dpyyerror (char const *);
#define YYSTYPE ParseUnion
#define YYFPRINTF fprintf

#line 88 "DYacc.tab.cpp" /* yacc.c:337  */
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
   by #include "DYacc.tab.hpp".  */
#ifndef YY_DPYY_DYACC_TAB_HPP_INCLUDED
# define YY_DPYY_DYACC_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int dpyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DPTT_aux = 258,
    DPTT_table = 259,
    DPTT_level = 260,
    DPTT_init = 261,
    DPTT_constant = 262,
    DPTT_eoq = 263,
    DPTT_groupstar = 264,
    DPTT_specs = 265,
    DPTT_save = 266,
    DPTT_and = 267,
    DPTT_macro = 268,
    DPTT_end_of_macro = 269,
    DPTT_or = 270,
    DPTT_not = 271,
    DPTT_ge = 272,
    DPTT_le = 273,
    DPTT_ne = 274,
    DPTT_number = 275,
    DPTT_symbol = 276,
    DPTT_units_symbol = 277,
    DPTT_function = 278,
    DPTT_dt_to_one = 279
  };
#endif

/* Value type.  */


extern YYSTYPE dpyylval;

int dpyyparse (void);

#endif /* !YY_DPYY_DYACC_TAB_HPP_INCLUDED  */



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
#define YYFINAL  22
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   221

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  165

#define YYUNDEFTOK  2
#define YYMAXUTOK   279

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
       2,     2,     2,    39,     2,     2,     2,     2,     2,     2,
      34,    35,    30,    26,    33,    25,     2,    31,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    36,    40,
      28,    27,    29,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    37,     2,    38,    32,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    86,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    98,   103,   104,   105,   112,   113,   114,   115,   116,
     120,   127,   131,   132,   136,   140,   141,   142,   143,   146,
     147,   148,   149,   153,   154,   155,   160,   165,   166,   167,
     168,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   198,   199,   201,   206,   207,   212,
     213,   218,   219
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DPTT_aux", "DPTT_table", "DPTT_level",
  "DPTT_init", "DPTT_constant", "DPTT_eoq", "DPTT_groupstar", "DPTT_specs",
  "DPTT_save", "DPTT_and", "DPTT_macro", "DPTT_end_of_macro", "DPTT_or",
  "DPTT_not", "DPTT_ge", "DPTT_le", "DPTT_ne", "DPTT_number",
  "DPTT_symbol", "DPTT_units_symbol", "DPTT_function", "DPTT_dt_to_one",
  "'-'", "'+'", "'='", "'<'", "'>'", "'*'", "'/'", "'^'", "','", "'('",
  "')'", "':'", "'['", "']'", "'!'", "';'", "$accept", "fulleq", "teqn",
  "tabledef", "eqn", "stock_eqn", "lhs", "var", "sublist", "symlist",
  "subdef", "number", "maplist", "exprlist", "exp", "tablevals",
  "xytablevals", "xytablevec", "tablepairs", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    45,    43,    61,    60,    62,
      42,    47,    94,    44,    40,    41,    58,    91,    93,    33,
      59
};
# endif

#define YYPACT_NINF -67

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-67)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     208,    10,    32,    32,    10,    10,   -67,   -67,   -67,   -67,
      63,    41,    66,   -14,   -67,    53,    83,    74,   102,    93,
     114,   116,   -67,   -19,   122,   -67,   -67,    79,    72,   -67,
      50,   -67,    32,   -67,   -67,   -67,   123,   109,    96,    19,
      79,   -67,   111,    79,    79,    79,   112,    18,   100,   -67,
     127,   135,    50,   124,   -67,   128,   129,   132,   133,     1,
     -67,   130,   137,   -13,   -67,   -67,   136,   -67,   144,   159,
     121,   121,    54,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,   -67,
     -67,   134,    50,   -67,   -67,    50,   143,    50,    50,    79,
     147,   -67,   157,   148,   -67,   -24,   -67,   -21,   100,   100,
      29,   142,    -2,    -2,    -2,   121,   121,    -2,    -2,    -2,
     144,   144,   144,    50,   150,   -67,    50,   -67,   -67,    18,
     146,   161,   -67,   175,   -67,   -67,   153,    50,   156,   -67,
     169,   -67,   -67,   162,    50,   164,   167,   168,   -67,   170,
     -67,    50,   172,    50,   171,    70,   173,   187,    78,    30,
     188,   132,   133,   173,   133
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     2,     3,     4,     5,
       0,    22,     0,    18,    21,    22,     0,     0,     0,     0,
       0,     0,     1,     0,     0,    23,    10,     0,     0,     6,
       0,     9,     0,     8,     7,    29,     0,    36,    25,     0,
       0,    41,     0,     0,     0,     0,    42,    15,    37,    33,
       0,     0,     0,     0,    69,     0,     0,    67,    64,    11,
      12,     0,     0,     0,    19,    26,     0,    24,    59,     0,
      61,    62,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
      35,     0,     0,    16,    17,     0,     0,     0,     0,     0,
       0,    31,     0,    27,    47,     0,    44,     0,    38,    39,
      58,    57,    55,    53,    56,    49,    48,    60,    52,    54,
      50,    51,    63,     0,     0,    70,     0,    14,    13,    20,
       0,     0,    28,     0,    45,    43,     0,     0,     0,    30,
       0,    46,    71,     0,     0,     0,     0,     0,    32,     0,
      72,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    68,    65,     0,    66
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -67,   -67,   -67,   -67,    13,   -67,    85,    22,   -67,   -67,
     -67,   -30,   -67,   -66,   -39,   -67,   -67,    43,   -22
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    10,    16,    59,    12,    18,    13,    46,    25,    39,
      37,    54,    64,    47,    48,    55,    56,    57,    58
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      60,    68,    35,   105,    70,    71,    72,   107,   101,   133,
      76,   134,    74,    27,   135,    36,    75,    20,    21,    75,
      28,   102,    91,    14,    14,    14,    14,    14,    86,    87,
      88,    11,    97,   129,    98,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      49,    74,    66,    15,    61,    50,    51,    67,    75,    86,
      87,    88,   124,    22,    52,   125,    76,   127,   128,    77,
      49,    78,    79,    80,    26,    50,    51,    23,    24,    81,
      82,    83,    84,    85,    86,    87,    88,    17,    19,   106,
      24,    29,    49,   136,   108,    40,   138,    50,    51,    41,
      15,    30,    42,   156,    43,    44,    52,   143,   157,    53,
      31,    96,    76,    45,   147,    77,   160,    78,    79,    80,
      32,   152,    33,   154,    34,    81,    82,    83,    84,    85,
      86,    87,    88,    76,   158,    65,    77,   162,    78,    79,
      80,   164,    63,    38,    62,    69,    73,    89,    83,    84,
      85,    86,    87,    88,    76,    90,    99,   103,    92,    78,
      79,    80,   100,    93,    94,    95,    96,   123,   130,    83,
      84,    85,    86,    87,    88,    40,    88,   126,   131,    41,
      15,   139,    42,   137,    43,    44,   140,   132,   142,   144,
     145,    40,   149,    45,   104,    41,    15,   146,    42,   148,
      43,    44,   161,   150,   151,   153,   155,    52,     0,    45,
     141,     1,     2,     3,     4,     5,     6,     7,     8,     9,
     159,   163
};

static const yytype_int16 yycheck[] =
{
      30,    40,    21,    69,    43,    44,    45,    73,    21,    33,
      12,    35,    33,    27,    35,    34,    40,     4,     5,    40,
      34,    34,    52,     1,     2,     3,     4,     5,    30,    31,
      32,    21,    31,    99,    33,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      20,    33,    33,    21,    32,    25,    26,    38,    40,    30,
      31,    32,    92,     0,    34,    95,    12,    97,    98,    15,
      20,    17,    18,    19,     8,    25,    26,    36,    37,    25,
      26,    27,    28,    29,    30,    31,    32,     2,     3,    35,
      37,     8,    20,   123,   133,    16,   126,    25,    26,    20,
      21,    27,    23,    33,    25,    26,    34,   137,    38,    37,
       8,    33,    12,    34,   144,    15,    38,    17,    18,    19,
      27,   151,     8,   153,     8,    25,    26,    27,    28,    29,
      30,    31,    32,    12,   156,    39,    15,   159,    17,    18,
      19,   163,    33,    21,    21,    34,    34,    20,    27,    28,
      29,    30,    31,    32,    12,    20,    26,    21,    34,    17,
      18,    19,    25,    35,    35,    33,    33,    33,    21,    27,
      28,    29,    30,    31,    32,    16,    32,    34,    21,    20,
      21,    35,    23,    33,    25,    26,    25,    39,    35,    33,
      21,    16,    25,    34,    35,    20,    21,    35,    23,    35,
      25,    26,   159,    35,    34,    33,    35,    34,    -1,    34,
      35,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      33,    33
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      42,    21,    45,    47,    48,    21,    43,    47,    46,    47,
      45,    45,     0,    36,    37,    49,     8,    27,    34,     8,
      27,     8,    27,     8,     8,    21,    34,    51,    21,    50,
      16,    20,    23,    25,    26,    34,    48,    54,    55,    20,
      25,    26,    34,    37,    52,    56,    57,    58,    59,    44,
      52,    48,    21,    33,    53,    39,    33,    38,    55,    34,
      55,    55,    55,    34,    33,    40,    12,    15,    17,    18,
      19,    25,    26,    27,    28,    29,    30,    31,    32,    20,
      20,    52,    34,    35,    35,    33,    33,    31,    33,    26,
      25,    21,    34,    21,    35,    54,    35,    54,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    33,    52,    52,    34,    52,    52,    54,
      21,    21,    39,    33,    35,    35,    52,    33,    52,    35,
      25,    35,    35,    52,    33,    21,    35,    52,    35,    25,
      35,    34,    52,    33,    52,    35,    33,    38,    59,    33,
      38,    58,    59,    33,    59
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    41,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    43,    44,    44,    44,    45,    45,    45,    45,    45,
      46,    47,    48,    48,    49,    50,    50,    50,    50,    51,
      51,    51,    51,    52,    52,    52,    53,    54,    54,    54,
      54,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    56,    56,    56,    57,    57,    58,
      58,    59,    59
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     4,     4,     1,     4,
       5,     1,     1,     2,     3,     1,     2,     3,     4,     1,
       5,     3,     7,     1,     2,     2,     0,     1,     3,     3,
       2,     1,     1,     4,     3,     4,     5,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     2,     2,     3,     1,    15,    17,     1,    15,     1,
       3,     5,     7
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
#line 86 "DYacc.y" /* yacc.c:1652  */
    { return DPTT_eoq;}
#line 1354 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 3:
#line 87 "DYacc.y" /* yacc.c:1652  */
    { return DPTT_groupstar; }
#line 1360 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 4:
#line 88 "DYacc.y" /* yacc.c:1652  */
    { return DPTT_specs; }
#line 1366 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 5:
#line 89 "DYacc.y" /* yacc.c:1652  */
    { return DPTT_save; }
#line 1372 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 6:
#line 90 "DYacc.y" /* yacc.c:1652  */
    { dpyy_addfulleq((yyvsp[-1].eqn),DPTT_table) ; return DPTT_eoq;}
#line 1378 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 7:
#line 91 "DYacc.y" /* yacc.c:1652  */
    { dpyy_addfulleq((yyvsp[-1].eqn),DPTT_constant) ; return DPTT_eoq; }
#line 1384 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 8:
#line 92 "DYacc.y" /* yacc.c:1652  */
    { dpyy_addfulleq((yyvsp[-1].eqn),DPTT_init) ; return DPTT_eoq;}
#line 1390 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 9:
#line 93 "DYacc.y" /* yacc.c:1652  */
    { dpyy_addfulleq((yyvsp[-1].eqn),DPTT_level) ; return DPTT_eoq;}
#line 1396 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 10:
#line 94 "DYacc.y" /* yacc.c:1652  */
    { dpyy_addfulleq((yyvsp[-1].eqn),DPTT_aux) ; return DPTT_eoq;}
#line 1402 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 11:
#line 98 "DYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = dpyy_add_lookup((yyvsp[-2].lhs),NULL,(yyvsp[0].tbl),0) ; }
#line 1408 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 12:
#line 103 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablevec(NULL,(yyvsp[0].num)) ;}
#line 1414 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 13:
#line 104 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablevec((yyvsp[-2].tbl),(yyvsp[0].num)) ;}
#line 1420 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 14:
#line 105 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablevec((yyvsp[-2].tbl),(yyvsp[0].num)) ;}
#line 1426 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 15:
#line 112 "DYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = dpyy_addeq((yyvsp[-2].lhs),NULL,(yyvsp[0].exl),'=') ; }
#line 1432 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 16:
#line 113 "DYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = dpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 0) ; }
#line 1438 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 17:
#line 114 "DYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = dpyy_add_lookup((yyvsp[-3].lhs),NULL,(yyvsp[-1].tbl), 1) ; }
#line 1444 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 18:
#line 115 "DYacc.y" /* yacc.c:1652  */
    { (yyval.eqn) = dpyy_add_lookup((yyvsp[0].lhs),NULL,NULL, 0) ; }
#line 1450 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 19:
#line 116 "DYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = dpyy_addeq(dpyy_addexceptinterp(dpyy_var_expression((yyvsp[-3].sym),NULL),NULL,0),(Expression *)dpyy_symlist_expression((yyvsp[-1].sml),(yyvsp[0].sml)),NULL,':') ; }
#line 1456 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 20:
#line 120 "DYacc.y" /* yacc.c:1652  */
    {(yyval.eqn) = dpyy_addstockeq((yyvsp[-4].lhs),(yyvsp[-2].var),(yyvsp[0].exl),'=') ; }
#line 1462 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 21:
#line 127 "DYacc.y" /* yacc.c:1652  */
    { (yyval.lhs) = dpyy_addexceptinterp((yyvsp[0].var),NULL,0) ; }
#line 1468 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 22:
#line 131 "DYacc.y" /* yacc.c:1652  */
    { (yyval.var) = dpyy_var_expression((yyvsp[0].sym),NULL);}
#line 1474 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 23:
#line 132 "DYacc.y" /* yacc.c:1652  */
    { (yyval.var) = dpyy_var_expression((yyvsp[-1].sym),(yyvsp[0].sml)) ;}
#line 1480 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 24:
#line 136 "DYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = (yyvsp[-1].sml) ;}
#line 1486 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 25:
#line 140 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = dpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1492 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 26:
#line 141 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = dpyy_symlist(NULL,(yyvsp[-1].sym),1,NULL) ; }
#line 1498 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 27:
#line 142 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = dpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ;}
#line 1504 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 28:
#line 143 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = dpyy_symlist((yyvsp[-3].sml),(yyvsp[-1].sym),1,NULL) ;}
#line 1510 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 29:
#line 146 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = dpyy_symlist(NULL,(yyvsp[0].sym),0,NULL) ; }
#line 1516 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 30:
#line 147 "DYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = dpyy_symlist(NULL,(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ;}
#line 1522 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 31:
#line 148 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = dpyy_symlist((yyvsp[-2].sml),(yyvsp[0].sym),0,NULL) ; }
#line 1528 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 32:
#line 149 "DYacc.y" /* yacc.c:1652  */
    {(yyval.sml) = dpyy_symlist((yyvsp[-6].sml),(yyvsp[-3].sym),0,(yyvsp[-1].sym)) ; }
#line 1534 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 33:
#line 153 "DYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ; }
#line 1540 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 34:
#line 154 "DYacc.y" /* yacc.c:1652  */
    {(yyval.num) = -(yyvsp[0].num) ;}
#line 1546 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 35:
#line 155 "DYacc.y" /* yacc.c:1652  */
    {(yyval.num) = (yyvsp[0].num) ;}
#line 1552 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 36:
#line 160 "DYacc.y" /* yacc.c:1652  */
    { (yyval.sml) = NULL ; }
#line 1558 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 37:
#line 165 "DYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = dpyy_chain_exprlist(NULL,(yyvsp[0].exn)) ;}
#line 1564 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 38:
#line 166 "DYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = dpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1570 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 39:
#line 167 "DYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = dpyy_chain_exprlist((yyvsp[-2].exl),(yyvsp[0].exn)) ; }
#line 1576 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 40:
#line 168 "DYacc.y" /* yacc.c:1652  */
    {(yyval.exl) = (yyvsp[-1].exl) ; }
#line 1582 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 41:
#line 172 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_num_expression((yyvsp[0].num)) ; }
#line 1588 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 42:
#line 173 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = (Expression *)(yyvsp[0].var) ; }
#line 1594 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 43:
#line 174 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_lookup_expression((yyvsp[-3].var),(yyvsp[-1].exl)) ; }
#line 1600 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 44:
#line 175 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('(',(yyvsp[-1].exn),NULL) ; }
#line 1606 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 45:
#line 176 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_function_expression((yyvsp[-3].fnc),(yyvsp[-1].exl)) ;}
#line 1612 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 46:
#line 177 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_function_expression((yyvsp[-4].fnc),dpyy_chain_exprlist((yyvsp[-2].exl),dpyy_literal_expression("?"))) ;}
#line 1618 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 47:
#line 178 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_function_expression((yyvsp[-2].fnc),NULL) ;}
#line 1624 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 48:
#line 179 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('+',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1630 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 49:
#line 180 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('-',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1636 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 50:
#line 181 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('*',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1642 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 51:
#line 182 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('/',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1648 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 52:
#line 183 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('<',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1654 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 53:
#line 184 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression(DPTT_le,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1660 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 54:
#line 185 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('>',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1666 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 55:
#line 186 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression(DPTT_ge,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1672 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 56:
#line 187 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression(DPTT_ne,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1678 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 57:
#line 188 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression(DPTT_or,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1684 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 58:
#line 189 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression(DPTT_and,(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1690 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 59:
#line 190 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression(DPTT_not,(yyvsp[0].exn),NULL) ; }
#line 1696 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 60:
#line 191 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('=',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1702 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 61:
#line 192 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('-',NULL, (yyvsp[0].exn)) ; }
#line 1708 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 62:
#line 193 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('+',NULL, (yyvsp[0].exn)) ; }
#line 1714 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 63:
#line 194 "DYacc.y" /* yacc.c:1652  */
    { (yyval.exn) = dpyy_operator_expression('^',(yyvsp[-2].exn),(yyvsp[0].exn)) ; }
#line 1720 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 64:
#line 198 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1726 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 65:
#line 200 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1732 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 66:
#line 202 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablerange((yyvsp[0].tbl),(yyvsp[-14].num),(yyvsp[-12].num),(yyvsp[-8].num),(yyvsp[-6].num)) ; }
#line 1738 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 67:
#line 206 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = (yyvsp[0].tbl) ; }
#line 1744 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 68:
#line 208 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablerange((yyvsp[0].tbl),(yyvsp[-12].num),(yyvsp[-10].num),(yyvsp[-6].num),(yyvsp[-4].num)) ; }
#line 1750 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 69:
#line 212 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablevec(NULL,(yyvsp[0].num)) ;}
#line 1756 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 70:
#line 213 "DYacc.y" /* yacc.c:1652  */
    {(yyval.tbl) = dpyy_tablevec((yyvsp[-2].tbl),(yyvsp[0].num)) ;}
#line 1762 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 71:
#line 218 "DYacc.y" /* yacc.c:1652  */
    { (yyval.tbl) = dpyy_tablepair(NULL,(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1768 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;

  case 72:
#line 219 "DYacc.y" /* yacc.c:1652  */
    {(yyval.tbl) = dpyy_tablepair((yyvsp[-6].tbl),(yyvsp[-3].num),(yyvsp[-1].num)) ;}
#line 1774 "DYacc.tab.cpp" /* yacc.c:1652  */
    break;


#line 1778 "DYacc.tab.cpp" /* yacc.c:1652  */
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
#line 225 "DYacc.y" /* yacc.c:1918  */

