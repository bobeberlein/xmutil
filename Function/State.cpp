#include "State.h"
#include "../Symbol/Variable.h"
#include "../XMUtil.h"


State::State(SymbolNameSpace *sns) : SymbolTableBase(sns) 
{
   pVals = '\0' ;
   iNVals = 0 ;
   cComputeFlag = 0 ;
   cDynamicDependencyFlag = 0 ;
   cSparse = 0 ;
}


State::~State(void)
{
}


