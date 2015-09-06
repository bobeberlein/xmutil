#include "LeftHandSide.h"
#include "../XMUtil.h"


LeftHandSide::LeftHandSide(SymbolNameSpace *sns,ExpressionVariable *var,SymbolListList *exceptlist,int interpmode)
   : SymbolTableBase(sns) 
{
   pExpressionVariable = var ;
   pExceptList = exceptlist ;
   iInterpMode = interpmode ;
}

LeftHandSide::LeftHandSide(SymbolNameSpace* sns, const LeftHandSide& base, Variable *var)
	: SymbolTableBase(sns)
{
	pExpressionVariable = new ExpressionVariable(sns, var, base.pExpressionVariable->GetSubs());
	if (base.pExceptList)
		pExceptList = new SymbolListList(sns, base.pExceptList);
	else
		pExceptList = '\0';
	iInterpMode = base.iInterpMode;
}

LeftHandSide::~LeftHandSide(void)
{
   if(HasGoodAlloc()) {
      if(pExpressionVariable) 
         delete pExpressionVariable ;
      if(pExceptList)
         delete pExceptList ;
   }
}
