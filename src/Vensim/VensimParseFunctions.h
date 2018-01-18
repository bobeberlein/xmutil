#ifndef _XMUTIL_VENSIM_VENSIMPARSEFUNCTIONS_H
#define _XMUTIL_VENSIM_VENSIMPARSEFUNCTIONS_H
#include "../Symbol/Parse.h"

extern void vpyy_addfulleq(Equation *eq,UnitExpression *un) ;
extern Equation *vpyy_addeq(LeftHandSide *lhs,Expression *ex,ExpressionList *exl,int token) ;
extern Equation *vpyy_add_lookup(LeftHandSide *lhs, Expression *ex, ExpressionTable *tvl, int legacy);
extern LeftHandSide *vpyy_addexceptinterp(ExpressionVariable *var, SymbolListList *except, int interpmode);
extern SymbolList *vpyy_symlist(SymbolList *in,Variable *add,int bang,Variable *end) ;
extern SymbolList *vpyy_mapsymlist(SymbolList* in,Variable *maprange, SymbolList *list);
extern UnitExpression *vpyy_unitsdiv(UnitExpression *num, UnitExpression *denom);
extern UnitExpression *vpyy_unitsmult(UnitExpression *f,UnitExpression *s) ;
extern UnitExpression *vpyy_unitsrange(UnitExpression *f,double minval,double maxval,double increment) ;
extern SymbolListList *vpyy_chain_sublist(SymbolListList *sll,SymbolList *nsl) ;
extern ExpressionList *vpyy_chain_exprlist(ExpressionList *el,Expression *e) ;
extern Expression *vpyy_num_expression(double num) ;
extern Expression *vpyy_literal_expression(const char *tok);
extern ExpressionVariable *vpyy_var_expression(Variable *var,SymbolList *subs) ;
extern ExpressionSymbolList *vpyy_symlist_expression(SymbolList *subs,SymbolList *map) ;
extern Expression *vpyy_operator_expression(int oper,Expression *exp1,Expression *exp2) ;
extern Expression *vpyy_function_expression(Function *func,ExpressionList *args) ;
extern Expression *vpyy_lookup_expression(ExpressionVariable *var,Expression *exp) ;
extern ExpressionTable *vpyy_tablepair(ExpressionTable *table,double x,double y) ;
extern ExpressionTable *vpyy_tablevec(ExpressionTable *table, double val);
extern ExpressionTable *vpyy_tablerange(ExpressionTable *table, double x1, double y1, double x2, double y2);
extern void vpyy_macro_start();
extern void vpyy_macro_expression(Variable *name, ExpressionList *margs);
extern void vpyy_macro_end();

#endif
