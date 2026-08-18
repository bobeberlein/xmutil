/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_XPYY_XMILEEQYACC_TAB_HPP_INCLUDED
# define YY_XPYY_XMILEEQYACC_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int xpyydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    XPTT_number = 258,             /* XPTT_number  */
    XPTT_symbol = 259,             /* XPTT_symbol  */
    XPTT_if = 260,                 /* XPTT_if  */
    XPTT_then = 261,               /* XPTT_then  */
    XPTT_else = 262,               /* XPTT_else  */
    XPTT_and = 263,                /* XPTT_and  */
    XPTT_or = 264,                 /* XPTT_or  */
    XPTT_not = 265,                /* XPTT_not  */
    XPTT_mod = 266,                /* XPTT_mod  */
    XPTT_eq = 267,                 /* XPTT_eq  */
    XPTT_neq = 268,                /* XPTT_neq  */
    XPTT_lt = 269,                 /* XPTT_lt  */
    XPTT_lte = 270,                /* XPTT_lte  */
    XPTT_gt = 271,                 /* XPTT_gt  */
    XPTT_gte = 272,                /* XPTT_gte  */
    XPTT_safediv = 273,            /* XPTT_safediv  */
    XPTT_apostrophe = 274,         /* XPTT_apostrophe  */
    UMINUS = 275                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */


extern YYSTYPE xpyylval;


int xpyyparse (void);


#endif /* !YY_XPYY_XMILEEQYACC_TAB_HPP_INCLUDED  */
