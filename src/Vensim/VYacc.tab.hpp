/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
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

#ifndef YY_VPYY_C_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED
# define YY_VPYY_C_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED
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
     VPTT_function = 285,
     VPTT_groupname = 286
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

#endif /* !YY_VPYY_C_SDEV_XMUTIL_SRC_VENSIM_VYACC_TAB_HPP_INCLUDED  */
