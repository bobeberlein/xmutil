/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

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
