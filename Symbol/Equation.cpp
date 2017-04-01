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

void Equation::GetVarsUsed(std::vector<Variable*>& vars) 
{ 
	pExpression->GetVarsUsed(vars); 
}

int Equation::SubscriptCount(std::vector<Symbol *> &elmlist, bool want_parent)
{
	if (iEqType == ':') /* a subscript equation */
		return 0;
	// we just need to look at the LHS variable and count the number of subscripts on it - all usage should be the same
	LeftHandSide* lhs = this->GetLeft();
	if (!lhs)
		return 0;
	SymbolList* subs = lhs->GetSubs();
	if (!subs)
		return 0;
	int n =  subs->Length();
	for (int i = 0; i < n; i++)
	{
		const SymbolList::SymbolListEntry& sub = (*subs)[i];
		if (sub.eType == SymbolList::EntryType_SYMBOL) // only valid type
			elmlist.push_back(sub.u.pSymbol);
	}
	return n;
}
