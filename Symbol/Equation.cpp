#include "Equation.h"
#include "LeftHandSide.h"
#include "../Model.h"
#include "../XMUtil.h"

Equation::Equation(SymbolNameSpace *sns,LeftHandSide *lhs,Expression *ex,int tok)
   : SymbolTableBase(sns)
{
   pLeftHandSide = lhs ;
   pExpression = ex ;
   iEqType = tok ;
}


Equation::~Equation(void)
{
   if(HasGoodAlloc()) {
      delete pLeftHandSide ;
      delete pExpression ;
   }
}


Variable *Equation::GetVariable(void)
{
   return pLeftHandSide->GetVariable() ; 
}

void Equation::CheckPlaceholderVars(Model *m)
{
   pExpression->CheckPlaceholderVars(m,true) ;
}

std::string Equation::RHSFormattedXMILE()
{
	ContextInfo info;
	pExpression->OutputComputable(&info);
	return info.str();
}



void Equation::Execute(ContextInfo *info) 
{
   // info->LHS(pLeftHandSide) ;
   if(info->GetComputeType() == CF_initial)
      pLeftHandSide->GetVariable()->SetInitialValue(0,pExpression->Eval(info)) ;
   else
      pLeftHandSide->GetVariable()->SetActiveValue(0,pExpression->Eval(info)) ;
}
void Equation::OutputComputable(ContextInfo *info) 
{
   *info << pLeftHandSide->GetVariable()->GetAlternateName() ;
   // subscripts
   if (pLeftHandSide->GetSubs())
	   pLeftHandSide->GetSubs()->OutputComputable(info);
   *info << "=" ;
   if(info->GetComputeType() == CF_rate) 
      *info << pLeftHandSide->GetVariable()->GetAlternateName() <<  "+dt*("  ;
   pExpression->OutputComputable(info) ;
   if(info->GetComputeType() == CF_rate) 
      *info << ")"  ;
   *info << "\n" ;
}

#ifdef laksdjlkj
int Equation::SubscriptCount(void) 
{
   if(iEqType == ':') { /* a subscript equation */


      
   } /* equiv ?? */
   return 0 ;
}
#endif