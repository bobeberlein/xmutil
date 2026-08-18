/*
   XmileEqYacc.y - XMILE equation grammar.

   This file is the source of truth for the XPTT_* token enum used by the
   hand-written XmileEqLex and by the action-code bridge XmileParseFunctions.
   The .tab.{cpp,hpp} artifacts are checked in alongside this file, mirroring
   the convention used for src/Vensim/VYacc.* and src/Dynamo/DYacc.*.

   Regenerate the .tab.{cpp,hpp} from this file (from inside src/Xmile/) with:
     bison -o XmileEqYacc.tab.cpp -p xpyy -d XmileEqYacc.y
*/

%{
#include "../Log.h"
#include "../Symbol/Parse.h"
#include "XmileParseFunctions.h"
extern int xpyylex(void);
extern void xpyyerror(char const *);
#define YYSTYPE ParseUnion
#define YYFPRINTF XmutilLogf
%}

%token <num> XPTT_number
%token <lit> XPTT_symbol
%token <tok> XPTT_if XPTT_then XPTT_else
%token <tok> XPTT_and XPTT_or XPTT_not XPTT_mod
%token <tok> XPTT_eq XPTT_neq XPTT_lt XPTT_lte XPTT_gt XPTT_gte
%token <tok> XPTT_safediv XPTT_apostrophe

%type <exn> expr
%type <exl> arglist
%type <sml> subs sub_term

/* Precedence -- standard C-like (lowest to highest):
   logical-or < logical-and < equality < comparison < additive <
   multiplicative < unary < exponentiation. This deliberately differs from
   the Vensim grammar's lattice (which interleaves comparison between OR and
   AND); we match the simlin XMILE parser's precedence in
   third_party/simlin/.../parser/mod.rs so that an XMILE equation produces
   the same Expression tree shape an equivalent simlin parse would. The
   resulting node *types* (ExpressionAdd, ExpressionLogical, ...) are
   identical to the Vensim-produced ones; only the precedence of mixed
   expressions differs.

   The XPTT_then / XPTT_else tokens get the lowest precedence so the
   if/then/else production extends as far right as possible (matching the
   standard recursive-descent reading "if cond then A else B+C" = if-then-else
   over A and (B+C)). */
%right XPTT_then XPTT_else
%left XPTT_or
%left XPTT_and
%left XPTT_eq XPTT_neq
%left XPTT_lt XPTT_lte XPTT_gt XPTT_gte
%left '+' '-'
%left '*' '/' XPTT_safediv XPTT_mod
%right UMINUS XPTT_not
%right '^'
/* XPTT_apostrophe always produces a parse error in its action, but it still
   needs a precedence so bison doesn't see a shift/reduce conflict between the
   binary-op reductions and the prospect of shifting the apostrophe. Highest
   precedence here matches the "postfix unary" intuition. */
%left XPTT_apostrophe

%%

equation : expr                                  { xpyy_set_result($1); }
         ;

expr : XPTT_number                               { $$ = xpyy_num($1); }
     | XPTT_symbol                               { $$ = xpyy_resolve_symbol($1, NULL); }
     | XPTT_symbol '[' subs ']'                  { $$ = xpyy_resolve_symbol($1, $3); }
     | '(' expr ')'                              { $$ = xpyy_paren($2); }
     | '-' expr  %prec UMINUS                    { $$ = xpyy_unary('-', $2); }
     | '+' expr  %prec UMINUS                    { $$ = $2; }
     | XPTT_not expr                             { $$ = xpyy_logical_unary($2); }
     | expr '+' expr                             { $$ = xpyy_binop('+', $1, $3); }
     | expr '-' expr                             { $$ = xpyy_binop('-', $1, $3); }
     | expr '*' expr                             { $$ = xpyy_binop('*', $1, $3); }
     | expr '/' expr                             { $$ = xpyy_binop('/', $1, $3); }
     | expr XPTT_safediv expr                    { $$ = xpyy_safediv($1, $3); }
     | expr XPTT_mod expr                        { $$ = xpyy_function("MODULO", $1, $3); }
     | expr '^' expr                             { $$ = xpyy_binop('^', $1, $3); }
     | expr XPTT_eq expr                         { $$ = xpyy_logical('=', $1, $3); }
     | expr XPTT_neq expr                        { $$ = xpyy_logical_ne($1, $3); }
     | expr XPTT_lt expr                         { $$ = xpyy_logical('<', $1, $3); }
     | expr XPTT_lte expr                        { $$ = xpyy_logical_le($1, $3); }
     | expr XPTT_gt expr                         { $$ = xpyy_logical('>', $1, $3); }
     | expr XPTT_gte expr                        { $$ = xpyy_logical_ge($1, $3); }
     | expr XPTT_and expr                        { $$ = xpyy_logical_and($1, $3); }
     | expr XPTT_or expr                         { $$ = xpyy_logical_or($1, $3); }
     | XPTT_if expr XPTT_then expr XPTT_else expr
                                                 { $$ = xpyy_if($2, $4, $6); }
     | XPTT_symbol '(' ')'                       { $$ = xpyy_call($1, NULL); }
     | XPTT_symbol '(' arglist ')'               { $$ = xpyy_call($1, $3); }
     | expr XPTT_apostrophe                      { xpyyerror("postfix ' (transpose) is not supported"); YYABORT; }
     ;

arglist : expr                                   { $$ = xpyy_arglist(NULL, $1); }
        | arglist ',' expr                       { $$ = xpyy_arglist($1, $3); }
        ;

subs : sub_term                                  { $$ = xpyy_sub_init($1); }
     | subs ',' sub_term                         { $$ = xpyy_sub_append($1, $3); }
     ;

sub_term : XPTT_symbol                           { $$ = xpyy_sub_name($1); }
         | '*'                                   { $$ = xpyy_sub_star(NULL); }
         | '*' ':' XPTT_symbol                   { $$ = xpyy_sub_star($3); }
         | XPTT_symbol ':' XPTT_symbol           { $$ = xpyy_sub_range($1, $3); }
         | '@' XPTT_number                       { $$ = xpyy_sub_index((int)$2); }
         ;

%%
