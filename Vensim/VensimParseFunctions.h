#ifndef _XMUTIL_VENSIM_VENSIMPARSEFUNCTIONS_H
#define _XMUTIL_VENSIM_VENSIMPARSEFUNCTIONS_H
#include "../Symbol/Parse.h"
#ifdef __cplusplus
#define CNF "C"
#else
#define CNF 
#endif

extern CNF void vpyy_addfulleq(Equation *eq,UnitExpression *un) ;
extern CNF Equation *vpyy_addeq(LeftHandSide *lhs,Expression *ex,ExpressionList *exl,int token) ;
extern CNF Equation *vpyy_add_lookup(LeftHandSide *lhs, Expression *ex, ExpressionTable *tvl);
extern CNF LeftHandSide *vpyy_addexceptinterp(ExpressionVariable *var, SymbolListList *except, int interpmode);
extern CNF SymbolList *vpyy_symlist(SymbolList *in,Variable *add,int bang,Variable *end) ;
extern CNF UnitExpression *vpyy_unitsdiv(UnitExpression *num,UnitExpression *denom) ;
extern CNF UnitExpression *vpyy_unitsmult(UnitExpression *f,UnitExpression *s) ;
extern CNF UnitExpression *vpyy_unitsrange(UnitExpression *f,double minval,double maxval) ;
extern CNF SymbolListList *vpyy_chain_sublist(SymbolListList *sll,SymbolList *nsl) ;
extern CNF ExpressionList *vpyy_chain_exprlist(ExpressionList *el,Expression *e) ;
extern CNF Expression *vpyy_num_expression(double num) ;
extern CNF ExpressionVariable *vpyy_var_expression(Variable *var,SymbolList *subs) ;
extern CNF ExpressionSymbolList *vpyy_symlist_expression(SymbolList *subs,SymbolList *map) ;
extern CNF Expression *vpyy_operator_expression(int oper,Expression *exp1,Expression *exp2) ;
extern CNF Expression *vpyy_function_expression(Function *func,ExpressionList *args) ;
extern CNF Expression *vpyy_lookup_expression(ExpressionVariable *var,Expression *exp) ;
extern CNF ExpressionTable *vpyy_tablepair(ExpressionTable *table,double x,double y) ;
extern CNF ExpressionTable *vpyy_tablerange(ExpressionTable *table,double x1,double y1,double x2,double y2) ;
extern CNF void vpyy_macro_start();
extern CNF void vpyy_macro_expression(Variable *name, ExpressionList *margs);
extern CNF void vpyy_macro_end();

#endif
