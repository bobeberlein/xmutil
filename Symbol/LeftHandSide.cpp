#include "LeftHandSide.h"
#include "../XMUtil.h"


LeftHandSide::LeftHandSide(SymbolNameSpace *sns,ExpressionVariable *var,SymbolListList *exceptlist,int interpmode)
   : SymbolTableBase(sns) 
{
   pExpressionVariable = var ;
   pExceptList = exceptlist ;
   iInterpMode = interpmode ;
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
