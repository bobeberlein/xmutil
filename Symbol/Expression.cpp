#include "Expression.h"
#include "ExpressionList.h"
#include "../Model.h"
#include "../XMUtil.h"


Expression::Expression(SymbolNameSpace *sns)
   : SymbolTableBase(sns) 
{
}


Expression::~Expression(void)
{
}

double Expression::Eval(ContextInfo *info)
{
 return FLT_MAX  ;
}

void Expression::OutputComputable(ContextInfo *info)
{
}

ExpressionFunction::~ExpressionFunction()
{
    if(HasGoodAlloc()) 
       delete pArgs; 
}

void ExpressionFunction::CheckPlaceholderVars(Model *m,bool isfirst)
 {
    pArgs->CheckPlaceholderVars(m) ; 
}


void ExpressionFunctionMemory::CheckPlaceholderVars(Model *m,bool isfirst) 
{
   if(isfirst || !m) {
      pPlacholderEquation = '\0' ; // deletion is handled by Model
   }
   else {
      pPlacholderEquation = m->AddUnnamedVariable(this) ;
   }
}

