#ifndef _XMUTIL_XMILE_XMILEPARSEFUNCTIONS_H
#define _XMUTIL_XMILE_XMILEPARSEFUNCTIONS_H

// Action-code bridge between the bison-generated XmileEqYacc parser and the
// xmutil engine. Each xpyy_* helper is called from a reduction in the .y file;
// the body lives in XmileParseFunctions.cpp and reaches the active model state
// through the process-global XmileReader pointer (XPObject). The shim mirrors
// VensimParseFunctions.{h,cpp}: the grammar stays declarative, all
// engine-aware logic is here.

class Expression;
class ExpressionList;
class SymbolList;

// Capture the top-level reduction result so XmileReader::ParseEquation can
// retrieve it after xpyyparse() returns. The grammar's single `equation : expr`
// rule calls this with the parsed Expression*; the reader owns the pointer
// from that point on.
void xpyy_set_result(Expression *e);

// Standard bison error sink. Implementation appends a positioned message to
// the reader's active errs vector.
void xpyyerror(char const *s);

Expression *xpyy_num(double n);

// Resolve a bare identifier (XPTT_symbol) to an ExpressionVariable. The XMILE
// bare-keyword table is consulted first (time, dt, initial_time, ...); on
// miss the name is looked up (and created if absent) as an ordinary Variable
// in the model namespace. subs is non-null only for the `name [ subs ]`
// production.
Expression *xpyy_resolve_symbol(const char *name, SymbolList *subs);

// Binary arithmetic dispatch ('+', '-', '*', '/', '^').
Expression *xpyy_binop(int op, Expression *l, Expression *r);

// Unary arithmetic dispatch -- currently only '-'. Folds the sign into an
// adjacent ExpressionNumber when possible, matching VensimParse's behaviour
// so the resulting tree is indistinguishable from a Vensim-parsed one.
Expression *xpyy_unary(int op, Expression *e);

// Wrap a parenthesized sub-expression in an explicit ExpressionParen node.
// Without this, an MDL writer would have no signal to re-emit the parens, so
// a tree like Mul(a, Sub(b, c)) parsed from "a * (b - c)" would round-trip
// as "a * b - c". Vensim's grammar does the same (VensimParse.cpp), so the
// two front-ends produce structurally identical trees for parenthesized
// expressions.
Expression *xpyy_paren(Expression *inner);

// Logical operators. The single-char form covers '<' '>' '='; the named
// variants (le/ge/ne/and/or) emit ExpressionLogical with the corresponding
// VPTT_* code imported from Vensim's grammar, again so downstream walkers
// can't tell the difference between an XMILE-parsed and an MDL-parsed tree.
Expression *xpyy_logical(int op, Expression *l, Expression *r);
Expression *xpyy_logical_le(Expression *l, Expression *r);
Expression *xpyy_logical_ge(Expression *l, Expression *r);
Expression *xpyy_logical_ne(Expression *l, Expression *r);
Expression *xpyy_logical_and(Expression *l, Expression *r);
Expression *xpyy_logical_or(Expression *l, Expression *r);
Expression *xpyy_logical_unary(Expression *operand);  // VPTT_not (operand only)

// `//` operator -> ZIDZ(l, r).
Expression *xpyy_safediv(Expression *l, Expression *r);

// Direct construction of a 2-arg call to a Vensim-canonical function name --
// used for the `%` (MODULO) operator. vensimName is looked up unmodified in
// the model namespace.
Expression *xpyy_function(const char *vensimName, Expression *a, Expression *b);

// `if c then t else e` -> IF THEN ELSE(c, t, e).
Expression *xpyy_if(Expression *c, Expression *t, Expression *e);

// Generic function call. xmileName is run through the un-rename table
// (XmileFunctions::LookupFunction) and the resulting Function*'s argument
// order is normalized (XmileFunctions::ReorderArgs).
Expression *xpyy_call(const char *xmileName, ExpressionList *args);

// Standard list-chain helper. NULL prev -> new singleton list.
ExpressionList *xpyy_arglist(ExpressionList *prev, Expression *e);

// Subscript-list construction helpers. The grammar builds a flat SymbolList
// per `[ ... ]` bracket; each sub_term reduction yields a singleton list and
// the chaining rule appends them. `:`-ranges and `@N` indexers are encoded as
// SymbolList entries and carried through unexpanded -- no pass expands a
// range into its element list or resolves an index. Bare `*` wildcards are
// bound post-parse by Model::ResolveWildcardSubscripts.
SymbolList *xpyy_sub_init(SymbolList *first);
SymbolList *xpyy_sub_append(SymbolList *list, SymbolList *next);
SymbolList *xpyy_sub_name(const char *name);
SymbolList *xpyy_sub_star(const char *bound);
SymbolList *xpyy_sub_range(const char *lo, const char *hi);
SymbolList *xpyy_sub_index(int idx);

// Token source called by the bison-generated parser. Trivial shim onto the
// active XmileEqLex on the reader.
int xpyylex(void);

#endif
