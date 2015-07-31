#include "Variable.h"
#include <boost/foreach.hpp>
#include <assert.h>
#include "../Symbol/Expression.h"
#include "../XMUtil.h"

// model Variable - this has subscript (families) units
// and the comment attached to it - inside of expressions
// we use an ExpressionVariable which has a pointer back
// to this

Variable::Variable(SymbolNameSpace *sns,const std::string &name) 
   : Symbol(sns,name)
{
   pVariableContent = '\0' ;

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