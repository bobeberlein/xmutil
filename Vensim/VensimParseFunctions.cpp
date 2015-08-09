#include "VensimParse.h"
#include "VensimParseFunctions.h"
#include "../Symbol/Parse.h"
#include "../XMUtil.h"


extern "C" void vpyy_addfulleq(Equation *eq,UnitExpression *un) 
{ return  VPObject->AddFullEq(eq,un) ; }
extern "C" Equation *vpyy_addeq(LeftHandSide *lhs,Expression *ex,ExpressionList *exl,int token) 
{ return VPObject->AddEq(lhs,ex,exl,token) ; }
extern "C" Equation *vpyy_add_lookup(LeftHandSide *lhs,Expression *ex,ExpressionTable *tvl) 
{ return VPObject->AddTable(lhs,ex,tvl) ; }
extern "C" LeftHandSide *vpyy_addexceptinterp(ExpressionVariable *var,SymbolListList *except,int interpmode) 
{ return VPObject->AddExceptInterp(var,except,interpmode) ;  }
extern "C" SymbolList *vpyy_symlist(SymbolList *in,Variable *add,int bang,Variable *end) 
{ return VPObject->SymList(in,add,bang,end) ; }
extern "C" UnitExpression *vpyy_unitsdiv(UnitExpression *num,UnitExpression *denom) 
{ return VPObject->UnitsDiv(num,denom) ; }
extern "C" UnitExpression *vpyy_unitsmult(UnitExpression *f,UnitExpression *s) 
{ return VPObject->UnitsMult(f,s) ; }
extern "C" UnitExpression *vpyy_unitsrange(UnitExpression *f,double minval,double maxval) 
{ return VPObject->UnitsRange(f,minval,maxval) ; }
extern "C" SymbolListList *vpyy_chain_sublist(SymbolListList *sll,SymbolList *nsl) 
{ return VPObject->ChainSublist(sll,nsl) ; }
extern "C" ExpressionList *vpyy_chain_exprlist(ExpressionList *el,Expression *e) 
{ return VPObject->ChainExpressionList(el,e) ; }
extern "C" Expression *vpyy_num_expression(double num) 
{ return VPObject->NumExpression(num) ; }
extern "C" ExpressionVariable *vpyy_var_expression(Variable *var,SymbolList *subs) 
{ return VPObject->VarExpression(var,subs) ; }
extern "C" ExpressionSymbolList *vpyy_symlist_expression(SymbolList *sym,SymbolList *map) 
{ return VPObject->SymlistExpression(sym,map) ; }
extern "C" Expression *vpyy_operator_expression(int oper,Expression *exp1,Expression *exp2) 
{ return VPObject->OperatorExpression(oper,exp1,exp2) ; }
extern "C" Expression *vpyy_function_expression(Function *func,ExpressionList *eargs) 
{ return VPObject->FunctionExpression(func,eargs) ; }
extern "C" Expression *vpyy_lookup_expression(ExpressionVariable *var,Expression *exp)
{ return VPObject->LookupExpression(var,exp) ; }
extern "C" ExpressionTable *vpyy_tablepair(ExpressionTable *table,double x,double y) 
{ return VPObject->TablePairs(table,x,y) ; }
extern "C" ExpressionTable *vpyy_tablerange(ExpressionTable *table,double x1,double y1,double x2,double y2)
{ return VPObject->TableRange(table,x1,y1,x2,y2) ; }
extern "C" void vpyy_macro_start() 
{ VPObject->MacroStart(); }
extern "C" void vpyy_macro_expression(Variable *name, ExpressionList *margs)
{ VPObject->MacroExpression(name, margs);}
extern "C" void vpyy_macro_end() 
{ VPObject->MacroEnd(); }

/* the default functions called by parser */
extern "C" int vpyylex(void) 
{ return VPObject->yylex() ; }
extern "C" int vpyyerror(const char *str) 
{ return VPObject->yyerror(str) ; }


