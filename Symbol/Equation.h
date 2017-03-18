#ifndef _XMUTIL_SYMBOL_EQUATION_H
#define _XMUTIL_SYMBOL_EQUATION_H
#include "../ContextInfo.h"
#include "SymbolTableBase.h"

class Model ;

class Equation :
   public SymbolTableBase
{
public:
   Equation(SymbolNameSpace *sns,LeftHandSide *lhs,Expression *ex,int token/* = ( dataequals not standard */) ;
   ~Equation(void);
   LeftHandSide *GetLeft(void) { return pLeftHandSide ; }
   Variable *GetVariable(void) ; // LeftHandSide forward declared can't put function here
   void GetVarsUsed(std::vector<Variable*>& vars);
   inline Expression *GetExpression(void)  { return pExpression ; }
   inline bool IsTable(void) { return iEqType == '(' ; }
   int SubscriptCount(std::vector<Symbol *> &elmlist);
   void Execute(ContextInfo *info) ;
   void OutputComputable(ContextInfo *info) ;
   void CheckPlaceholderVars(Model *m) ;
   std::string RHSFormattedXMILE(); // need a_b*c
private :
   LeftHandSide *pLeftHandSide ;
   Expression *pExpression ;
   int iEqType ;
};

#endif