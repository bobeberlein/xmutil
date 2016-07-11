#include "Variable.h"
#include <boost/foreach.hpp>
#include <assert.h>
#include "../Symbol/Expression.h"
#include "../Symbol/LeftHandSide.h"
#include "../XMUtil.h"
#include "boost/lexical_cast.hpp"
#include <iostream>

// model Variable - this has subscript (families) units
// and the comment attached to it - inside of expressions
// we use an ExpressionVariable which has a pointer back
// to this

Variable::Variable(SymbolNameSpace *sns,const std::string &name) 
	: Symbol(sns, name)
{
   pVariableContent = '\0' ;
   mVariableType = XMILE_Type_UNKNOWN; // till typed

}


Variable::~Variable(void)
{
   if(pVariableContent) {
      if(HasGoodAlloc())
         pVariableContent->Clear() ;
      delete pVariableContent ;
      pVariableContent = '\0' ;
   }
}

XMILE_Type Variable::MarkFlows(SymbolNameSpace* sns)
{
	if (!pVariableContent)
		return mVariableType;

	std::vector<Equation*>equations = pVariableContent->GetAllEquations();

	if (equations.empty())
	{
		// todo data variables have empty equations  - could fill something in here???
		return mVariableType;
	}

	/* if the equations are INTEG this is a stock and we need to validate flows - if we need to make
	   up flows it has to be done here so that all the equations get the same net flow name
       we make up flows if the active part of INTEG uses something other then +/- of flows or 
	   if there are multiple equations that don't match (even if they all use +/- of flows)

	   */
	//first pass - just figure out if there is anything to do - 
	bool gotone = false;
	BOOST_FOREACH(Equation* eq, equations)
	{
		Expression* exp = eq->GetExpression();
		if (exp->TestMarkFlows(sns, '\0', '\0'))
		{
			gotone = true;
			break; // one is all that is needed
		}
	}
	if (!gotone)
	{
		if (mVariableType == XMILE_Type_UNKNOWN)
			mVariableType = XMILE_Type_AUX;
		return mVariableType;
	}
	mVariableType = XMILE_Type_STOCK;

	// second pass, get the flow lists for everyone -- NOTE there is a bug in this code
	// because we don't check subscripts on the flows list so they may match even though
	// they shouldn't eg STOCK[A]=INTEG(FLOW[B],0) STOCK[B]=INTEG(FLOW[A],0) 
	std::vector<FlowList> flow_lists;
	flow_lists.resize(equations.size());
	size_t i = 0;
	bool match = true;
	BOOST_FOREACH(Equation* eq, equations)
	{
		Expression* exp = eq->GetExpression();
		if (!exp->TestMarkFlows(sns, &flow_lists[i], '\0') || !flow_lists[i].Valid())
			match = false;
		else if (i > 0 && !(flow_lists[i] == flow_lists[i - 1])) 
			match = false; // all must be the same
		i++;
	}
	if (match)
	{
		BOOST_FOREACH(Variable* v, flow_lists[0].Inflows())
		{
			v->SetVariableType(XMILE_Type_FLOW);
			mInflows.push_back(v);
		}
		BOOST_FOREACH(Variable* v, flow_lists[0].Outflows())
		{
			v->SetVariableType(XMILE_Type_FLOW);
			mOutflows.push_back(v);
		}
		return mVariableType; // done
	}

	// mismatched for invalid flow equations - create a flow variable and add it to the model
	std::string basename = this->GetName() + " net flow";
	std::string name = basename;
	i = 0;
	while (sns->Find(name))
	{
		++i;
		name = basename + "_" + boost::lexical_cast<std::string>(i);
	}
	Variable* v = new Variable(sns, name);
	v->SetVariableType(XMILE_Type_FLOW);
	mInflows.push_back(v);

	// now we swap the active part of the INTEG equation for v and set v's equation to
	// the active part - this is equation by equation 
	i = 0;
	BOOST_FOREACH(Equation* eq, equations)
	{
		// left hand side for this variable
		LeftHandSide *lhs = new LeftHandSide(sns, *eq->GetLeft(),v); // replace var in lhs equation
		Equation *neweq = new Equation(sns, lhs, flow_lists[i].ActiveExpression(), '=');
		v->AddEq(neweq);
		flow_lists[i].SetNewVariable(v);
		eq->GetExpression()->TestMarkFlows(sns, &flow_lists[i], eq);
		i++;
	}
	// don't do this - we get some memory leakage but risk a crash otherwise v->MarkGoodAlloc();

	return mVariableType;
}

void VariableContent::Clear(void)
{
   if(pState) {
      delete pState ;
      pState = 0 ;
   }
}

void VariableContentVar::Clear(void)
{
   // clear memory
   VariableContent::Clear() ;
   if(pUnits)
      delete pUnits ;
   int i ;
   int n = vSubscripts.size() ;
   for(i=0;i<n;i++)
      delete vSubscripts[i] ;
   n = vEquations.size() ;
   for(i=0;i<n;i++)
      delete vEquations[i] ;
   // comment takes care of itself 
}

void Variable::AddEq(Equation *eq) 
{
   if(!pVariableContent) {
      try {
         pVariableContent = new VariableContentVar ;
		 pVariableContent->SetAlternateName(this->GetName()); // until overidden
      }
      catch(...) {
         throw "Memory failure adding equations" ;
      }
   }
  pVariableContent->AddEq(eq) ; 
}

VariableContent::VariableContent(void)
{
   pState = '\0' ;
}

VariableContent::~VariableContent(void) 
{
}

void VariableContentVar::CheckPlaceholderVars(Model *m)
{
   BOOST_FOREACH(Equation *eq,vEquations) {
      eq->CheckPlaceholderVars(m) ;
   }
}


bool VariableContentVar::CheckComputed(Symbol *parent,ContextInfo *info,bool first)
{
   //printf("Checking out %s\n",GetName().c_str()) ;
   if(!pState) {
      //printf("   - not computable ignoring\n") ;
      return true ;
   }
   if(pState->cComputeFlag & info->GetComputeType()) {
      if(info->GetComputeType() == CF_active) {
         if(pState->HasMemory())
            info->AddDDF(DDF_level) ; // not recorded in cDynamicDependencyFlag
         else
            info->AddDDF(pState->cDynamicDependencyFlag) ;
      }
      return true ; // done
   }
   int intype = info->GetComputeType() << 1 ;
   if(pState->cComputeFlag & intype) {
      if(info->GetComputeType() == CF_initial)
         std::cout << "Simultaneous initial equations found " << std::endl ;
      else {
         if(pState->HasMemory()) { // first call was for rates - now for level
            assert(!first) ;
            info->AddDDF(DDF_level) ;
            return true ;
         }
         std::cout << "Simultaneous active equations found " << std::endl ;
      }
      std::cout << "     " << parent->GetName() << std::endl ;
      pState->cComputeFlag &= ~intype ;
      return false ;
   } else if(!first && (info->GetComputeType() != CF_initial) && pState->HasMemory()) {
      //printf("Not tracing further for level  %s\n",GetName().c_str()) ;
      info->AddDDF(DDF_level) ; // this is the level - even if the rate is a constant (only a 0 rate would really be unchanging)
      return true ;
   } else if(first && info->GetComputeType() == CF_initial && !pState->HasMemory()) {
         return true ; /* don't need to do anything, if var is needed will be called not first */
   } else { // really need to check
      unsigned char ddf = info->GetDDF() ;
      if(info->GetComputeType() == CF_active) {
         info->ClearDDF() ;
         if(!pState->UpdateOnPartialStep())
            info->AddDDF(DDF_time_varying) ;
      }
      pState->cComputeFlag |= intype ;
      BOOST_FOREACH(Equation*e,vEquations) {
         if(!e->GetExpression()->CheckComputed(info)) {
            std::cout << "     " << parent->GetName() << std::endl ;
            pState->cComputeFlag &= ~intype ;
            pState->cComputeFlag |= info->GetComputeType() ; // don't reenter
            return false ;
         }
      }
      if(info->GetComputeType() == CF_active) {
         pState->cDynamicDependencyFlag = info->GetDDF() ;
         info->AddDDF(ddf) ;
      }
   }

   pState->cComputeFlag = (pState->cComputeFlag|info->GetComputeType())&(~intype) ;

   /* add the equations - some can be ignored */
   if(info->GetComputeType() == CF_active) {
      if(pState->HasMemory() ||
         !(pState->cDynamicDependencyFlag & (DDF_level|DDF_data|DDF_time_varying)))
         return true ;
   } else if(info->GetComputeType() == CF_unchanging) {
      if(pState->HasMemory() ||
         pState->cDynamicDependencyFlag & (DDF_level|DDF_data|DDF_time_varying))
         return true ;
   } else if(info->GetComputeType() == CF_rate) {
      if(!pState->HasMemory())
         return true ;
   }
   printf("Outputting equations for  %s\n",parent->GetName().c_str()) ;
   BOOST_FOREACH(Equation*e,vEquations) {
      info->PushEquation(e) ;
   }
   return true ;
}

void  VariableContentVar::SetupState(ContextInfo *info)
{
   bool hasmemory = false ;
   bool timedependent = false ;
   if(!info) {
      if(pState)
         delete pState ;
      pState = '\0' ;
      return ;
   }
   if(!pState) { /* create it - actually this is called twice with value assignment on second pass */
      if(info->GetComputeType())
         return ; // do nothing no state allocated on the previous try
      // find out what defines this to determine type
      bool haseq = false ;
      BOOST_FOREACH(Equation*e,vEquations) {
         haseq = true ;
         if(e->GetExpression()->GetType() == EXPTYPE_Table) {
            return ; // for now no state assigned 
         }
         Function *f = e->GetExpression()->GetFunction() ;
         if(f) {
            if(!f->IsMemoryless())
               hasmemory = true ;
            if(f->IsTimeDependent())
               timedependent = true ;
         }
      }
      // empty equation causes what??? 
      if(!haseq)
         timedependent = true ; // consistent with exog variables
      if(hasmemory) 
         pState = new StateLevel(info->GetSymbolNameSpace()) ;
      else if(timedependent) 
         pState = new StateTime(info->GetSymbolNameSpace()) ;
      else
         pState = new State(info->GetSymbolNameSpace()) ;
      pState->iNVals = 1 ; // todo arrays
   }
   // the following addresses will be nonsense on the first pass we just
   // use them to set the requires size for the integration state vector
   if(pState->HasMemory()) {
      pState->pVals = info->GetLevelP(pState->iNVals) ;
      pState->SetRateP(info->GetRateP(pState->iNVals)) ;
   }
   else
      pState->pVals = info->GetAuxP(pState->iNVals) ;

}

int VariableContentVar::SubscriptCount(std::vector<Symbol *> &elmlist)
{
   int count ;
   if(vEquations.empty())
      return 0 ;
   if(count = vEquations[0]->SubscriptCount()) {
      if(vEquations.size() != 1)
         throw "Bad subscript equations" ;

      return count ;
   }
   return 0 ;
}

