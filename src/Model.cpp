#include "Model.h"
#include "Symbol/Symbol.h"
#include "Symbol/LeftHandSide.h"
#include "Symbol/Equation.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include "XMUtil.h"
#include "Xmile/XMILEGenerator.h"
#include <iostream>

Model::Model(void)
{
   dLevel = dRate = dAux = NULL ;
}


Model::~Model(void)
{
  // allocation is no longer clean ClearCompEquations() ;
}

Equation *Model::AddUnnamedVariable(ExpressionFunctionMemory *e) 
{
   ExpressionFunction *e2 ;
   std::string s ;
   assert(s.empty()) ;
   Variable *var = new Variable(&mSymbolNameSpace,s) ;
   ExpressionVariable *ev = new ExpressionVariable(&mSymbolNameSpace,var,NULL) ;
   LeftHandSide *lhs = new LeftHandSide(&mSymbolNameSpace,ev,NULL, NULL,0) ;
   e2 = new ExpressionFunction(&mSymbolNameSpace,e->GetFunction(),e->GetArgs()) ;
   Equation *eq = new Equation(&mSymbolNameSpace,lhs,e2,'=') ;
   //printf("Adding in a placeholder from function %s\n",e->GetFunction()->GetName().c_str()) ;
   var->AddEq(eq) ;
   vUnamedVars.push_back(var) ;
   return eq ;

}

void Model::ClearCompEquations(void)
{
   vActiveComps.clear() ;
   vInitialComps.clear() ;
   vUnchangingComps.clear() ;
   vInitialTimeComps.clear() ;

   SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable() ;
   BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
      SNSitToSymbol(it)->SetupState(NULL) ;
      SNSitToSymbol(it)->CheckPlaceholderVars(NULL) ;
   }
   BOOST_FOREACH(Variable *v,vUnamedVars) {
      v->GetEquation(0)->GetExpression()->RemoveFunctionArgs() ; // these are allocated in the real variable's equation
      v->SetupState(NULL) ;
      delete v ;
   }
   if(dLevel) {
      delete dLevel ;
      dLevel = NULL ;
   }
   if(dRate) {
      delete dRate ;
      dRate = NULL ;
   }
   if(dAux) {
      delete dAux ;
      dAux = NULL ;
   }

}
typedef struct {
   Variable *v ;
   int count ;
} SubInfoWCount ;
bool Model::OrganizeSubscripts(void)
{
   std::vector<SubInfoWCount>sublist ;
   std::vector<Variable *>subelm ;
   SubInfoWCount siwc ;
   try {
      SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable() ;
      BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
         siwc.v = static_cast<Variable *>SNSitToSymbol(it) ;
         if(siwc.count = siwc.v->SubscriptCount(subelm)) {
            sublist.push_back(siwc) ;
         }
      }
      mSymbolNameSpace.ConfirmAllAllocations() ;
   }
   catch(...) {
      mSymbolNameSpace.DeleteAllUnconfirmedAllocations() ;
      return false ;
   }
   return true ;
}

bool Model::ValidatePlaceholderVars(void)
{
   try {
      SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable() ;
      BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
         //printf("Checking placeholders out %s\n",SNSitToSymbol(it)->GetName().c_str()) ;
         SNSitToSymbol(it)->CheckPlaceholderVars(this) ;
      }
      mSymbolNameSpace.ConfirmAllAllocations() ;
   }
   catch(...) {
      mSymbolNameSpace.DeleteAllUnconfirmedAllocations() ;
      return false ;
   }
   return true ;
}

bool Model::SetupVariableStates(int pass/* 0 just assign, 1 determine sizes, 2 pass pointers for computation*/)
{
   ContextInfo info ;
   info.pSymbolNameSpace = &mSymbolNameSpace ;
   SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable() ;
   info.iComputeType = pass ; // flag to skip empty or count sizes
   try {
      if(pass == 2) {
         if(iNLevel) {
            dLevel = new double[iNLevel] ;
            dRate = new double[iNLevel] ;
         }
         else
            dLevel = dRate = NULL ;
         if(iNAux)
            dAux = new double[iNAux] ;
         else
            dAux = NULL ;
         info.pBaseAux = info.pCurAux = dAux ;
         info.pBaseRate = info.pCurRate = dRate ;
         info.pBaseLevel = info.pCurLevel = dLevel ;
      } else {
         info.pBaseAux = info.pCurAux = NULL ;
         info.pBaseRate = info.pCurRate = NULL ;
         info.pBaseLevel = info.pCurLevel = NULL ;
         info.iComputeType = 0 ;
      }
      BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
         SNSitToSymbol(it)->SetupState(&info) ;
      }
      // placeholder vars also need state set up 
      BOOST_FOREACH(Variable *v,vUnamedVars) {
         v->SetupState(&info) ;
      }
      mSymbolNameSpace.ConfirmAllAllocations() ;
      if(pass == 1) {
         iNLevel = (info.pCurLevel - info.pBaseLevel) ;
         iNAux = (info.pCurAux - info.pBaseAux) ;
      }
   }  catch(...) {
      // set all states to null - they will be deleted
      BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
         SNSitToSymbol(it)->SetupState(NULL) ; // clear if setup 
      }
      mSymbolNameSpace.DeleteAllUnconfirmedAllocations() ;
      if(dLevel) {
         delete dLevel ;
         dLevel = NULL ;
      }
      if(dRate) {
         delete dRate ;
         dRate = NULL ;
      }
      if(dAux) {
         delete dAux ;
         dAux = NULL ;
      }
      return false ;
   }
   return true ;
}

/* start anywhere - we just use the iterator order on the hash table - 
   and get every variable computed - this needs to be done for both
   active and initial value (potentially reinitial as well but that is
   left out for now).
   */
bool Model::OrderEquations(ContextInfo *info,bool tonly)
{
   bool haserr = false ;
   try {
      SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable() ;
      if(tonly) {
         Variable *v ;
         v = static_cast<Variable *>(mSymbolNameSpace.Find("INITIAL TIME"))  ;
         // note CheckComputed called with false for first otherwise these won't be
         // initialized
         if(!v || !v->CheckComputed(info,false))
            haserr = true ;
         v = static_cast<Variable *>(mSymbolNameSpace.Find("TIME STEP"))  ;
         if(!v || !v->CheckComputed(info,false))
            haserr = true ;
      } else {
         BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
            //printf("Looping to: %s\n",SNSitToSymbol(it)->GetName().c_str()) ;
            if(!SNSitToSymbol(it)->CheckComputed(info,true))
               haserr = true ; // continue looking for simultaneous even when false 
         }
         BOOST_FOREACH(Variable *v,vUnamedVars) {
            if(!v->CheckComputed(info,true))
               haserr = true ;
         }
      }

      mSymbolNameSpace.ConfirmAllAllocations() ;
   }
   catch(...) {
      mSymbolNameSpace.DeleteAllUnconfirmedAllocations() ;
      return false ;
   }
   return !haserr ;
}




bool Model::AnalyzeEquations(void)
{
   ContextInfo info ;

   ClearCompEquations() ; // will also reset comp flag and delete placeholder vars
   /* ValidatePlaceholderVars will create the variables required for functions that either
      have memory are are time dependent (and therefore not subject to change within
      a DT as used in all but Euler integration */
   if(!ValidatePlaceholderVars())
      return false ;
   /* SetupVariableStates will create states based on the variable equation types
      including subscript states needed to organize subscripts */
   if(!SetupVariableStates(0))
      return false ;
   if(!OrganizeSubscripts())
      return false ;
   if(!SetupVariableStates(1)) /* get size count */
      return false ;
   if(!SetupVariableStates(2)) /* allocate and assign computed value locations */
      return false ;
   // three passes on ordering equations first for initial
   // then for active - but putting out only those equations
   // that are not clearly unchanging - then again for
   // unchanging which are equations not put out in step
   // 2 then during execution 1 and 3 are computed once
   // and 2 is cycled through for integration
   // 

   // before the passes initialize time, then dt
   printf("\nInitial time \n") ;
   info.iComputeType = CF_initial ;
   info.pEquations = &vInitialTimeComps ;
   if(!OrderEquations(&info,true))
      return false ;


   printf("\nInitial equations \n") ;
   info.iComputeType = CF_initial ;
   info.pEquations = &vInitialComps ;
   if(!OrderEquations(&info,false))
      return false ;
   printf("\n\nActive equations \n") ;
   info.iComputeType = CF_active ;
   info.pEquations = &vActiveComps ;
   if(!OrderEquations(&info,false))
      return false ;
   printf("\n\nUnchanging equations \n") ;
   info.iComputeType = CF_unchanging ;
   info.pEquations = &vUnchangingComps ;
   if(!OrderEquations(&info,false))
      return false ;
   printf("\n\nRate equations \n") ;
   info.iComputeType = CF_rate ;
   info.pEquations = &vRateComps ;
   if(!OrderEquations(&info,false))
      return false ;
   return true ;
}

bool Model::Simulate(void)
{
   ContextInfo info ;
   try {
      double t,s,e,dt ;
      int i,n ;
      Variable *time = static_cast<Variable *>(mSymbolNameSpace.Find("Time")) ;
      Variable *start = static_cast<Variable *>(mSymbolNameSpace.Find("INITIAL TIME")) ;
      Variable *end = static_cast<Variable *>(mSymbolNameSpace.Find("FINAL TIME")) ;
      Variable *step = static_cast<Variable *>(mSymbolNameSpace.Find("TIME STEP")) ;
      n = iNLevel ;

      info.iComputeType = CF_initial ;
      BOOST_FOREACH(Equation *e,vInitialTimeComps) {
         e->Execute(&info) ;
         //printf("%s = %g\n",e->GetVariable()->GetName().c_str(),e->GetVariable()->Eval(&info)) ;
      }
      if(start)
         s = start->Eval(&info) ;
      else
         s = 0 ;
      if(step)
         dt = step->Eval(&info) ;
      else
         dt = 1 ;
      info.dTime = s ;
      info.dDT = dt ;
      BOOST_FOREACH(Equation *e,vInitialComps) {
         e->Execute(&info) ;
         //printf("%s = %g\n",e->GetVariable()->GetName().c_str(),e->GetVariable()->Eval(&info)) ;
      }
      // now the active equations
      info.iComputeType = CF_active ;
      // first the unchanging variables
      //printf("\n Unchanging\n") ;
      BOOST_FOREACH(Equation *e,vUnchangingComps) {
         e->Execute(&info) ;
         //printf("%s = %g\n",e->GetVariable()->GetName().c_str(),e->GetVariable()->Eval(&info)) ;
      }

      // now over time
      if(end)
         e = end->Eval(&info) ;
      else
         e = 100 ;
      printf("Time") ;
      BOOST_FOREACH(Equation *e,vActiveComps) {
         printf("\t%s",e->GetVariable()->GetName().c_str()) ;
      } ;
      BOOST_FOREACH(Equation *e,vRateComps) {
         printf("\t%s",e->GetVariable()->GetName().c_str()) ;
      } printf("\n") ;

      for(t=s;t<=e;t += dt) {
         info.dTime = t ;
         //printf("\n\nAt time %g\n",t) ;
         if(time)
            time->SetActiveValue(0,t) ;
         printf("%g",t) ;
         BOOST_FOREACH(Equation *e,vActiveComps) {
            e->Execute(&info) ;
            printf("\t%g",e->GetVariable()->Eval(&info)) ;
         } 
         BOOST_FOREACH(Equation *e,vRateComps) {
            e->Execute(&info) ;
            printf("\t%g",e->GetVariable()->Eval(&info)) ;
         } printf("\n") ;
         if(step)
            info.dDT = dt = step->Eval(&info) ;
         // update states
         for(i=0;i<n;i++) {
            dLevel[i] += dt*this->dRate[i] ;
         }
         if(end)
            e = end->Eval(&info) ;
      }
   } catch(...) {
      std::cout << "Error of some sort" << std::endl ;
      return false ;
   }
   return true ;
}

bool Model::OutputComputable(bool wantshort)
{
   ContextInfo info ;
   try {
      if(wantshort)
         GenerateShortNames() ;
      else
         GenerateCanonicalNames() ;
      info.iComputeType = CF_initial ;
      printf("------------- initial time -----------------\n") ;
      BOOST_FOREACH(Equation *e,vInitialTimeComps) {
         e->OutputComputable(&info) ;
      }
      printf("------------- initialization -----------------\n") ;
      BOOST_FOREACH(Equation *e,vInitialComps) {
         e->OutputComputable(&info) ;
      }
      info.iComputeType = CF_active ;
      printf("------------- Unchanging -----------------\n") ;
      info.iComputeType = CF_active ;
      BOOST_FOREACH(Equation *e,vUnchangingComps) {
         e->OutputComputable(&info) ;
      }
      printf("------------- active -----------------\n") ;
      BOOST_FOREACH(Equation *e,vActiveComps) {
         e->OutputComputable(&info) ;
      } 
      info.iComputeType = CF_rate ;
      printf("------------- rates -----------------\n") ;
      BOOST_FOREACH(Equation *e,vRateComps) {
         e->OutputComputable(&info) ;
      } 
   } catch(...) {
      std::cout << "Error of some sort" << std::endl ;
      return false ;
   }
   return true ;
}

bool Model::MarkVariableTypes(SymbolNameSpace* ns)
{
	try {
		SymbolNameSpace::HashTable *ht;
		if (ns)
			ht = ns->GetHashTable();
		else
		{
			ht = mSymbolNameSpace.GetHashTable();
			ns = &mSymbolNameSpace;
		}
		std::vector<Variable*> vars;
		BOOST_FOREACH(const SymbolNameSpace::iterator it, *ht) {
			Symbol* sym = SNSitToSymbol(it);

			if (sym->isType() == Symtype_Variable)
				vars.push_back(static_cast<Variable*>(sym));
		}
		// 
		BOOST_FOREACH(Variable* var, vars)
		{
			var->MarkFlows(ns); // may change number of entries so can't be in above loop
		}
		// don't do this - we have broken the allocation setup mSymbolNameSpace.ConfirmAllAllocations();
	}
	catch (...) {
		ns->DeleteAllUnconfirmedAllocations();
		return false;
	}

#ifdef dodump
	// - dump eveything - mostly just to see how the translation is going
	ContextInfo info;

	SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
	BOOST_FOREACH(const SymbolNameSpace::iterator it, *ht) {
		Symbol* sym = SNSitToSymbol(it);

		if (sym->isType() == Symtype_Variable)
		{
			Variable*var = static_cast<Variable*>(sym);
			VariableContent* content = var->Content();
			if (content) { // array elements don't have
				BOOST_FOREACH(Equation* eq, content->GetAllEquations())
				{
					eq->OutputComputable(&info);
				}
			}
		}
	}
#endif

	return true;
}

void Model::AttachStragglers()
{
	SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
	std::vector<Variable*> vars;
	BOOST_FOREACH(const SymbolNameSpace::iterator it, *ht) {
		Symbol* sym = SNSitToSymbol(it);

		if (sym->isType() == Symtype_Variable)
			vars.push_back(static_cast<Variable*>(sym));
	}
	// first try - anything that is not defined somewhere see if a ghost appears somewhere
	// and change that to the definition
	BOOST_FOREACH(Variable* var, vars)
	{
		if (!var->GetView())
		{
			BOOST_FOREACH(View* view, vViews)
			{
				if (view->UpgradeGhost(var))
					break;
			}
		}
	}
	// now try undefined flows - attach to associated stocks
	BOOST_FOREACH(Variable* var, vars)
	{
		if (!var->GetView() && var->VariableType() == XMILE_Type_FLOW)
		{
			// we don't know flows uses so just look at all stocks
			Variable* upstream = NULL;
			Variable* downstream = NULL;
			BOOST_FOREACH(Variable* stock, vars)
			{
				if (stock->VariableType() == XMILE_Type_STOCK)
				{
					BOOST_FOREACH(Variable* in, stock->Inflows())
					{
						if (in == var)
						{
							downstream = stock;
							break;
						}
					}
					BOOST_FOREACH(Variable* out, stock->Outflows())
					{
						if (out == var)
						{
							upstream = stock;
							break;
						}
					}
					if (upstream && downstream)
						break;
				}
			}
			if (upstream && upstream->GetView())
			{
				upstream->GetView()->AddFlowDefinition(var, upstream, downstream);
			}
			else if (downstream && downstream->GetView())
			{
				downstream->GetView()->AddFlowDefinition(var, upstream, downstream);
			}
		}
	}
	// next pass look for inputs - if there are any put the var next to them
	// next pass look for otuputs - if there are any put the var next to them
	// finally dump everything remaining at 40,40 on the first view
	if (!vViews.empty())
	{
		View* dump_view = vViews[0];
		BOOST_FOREACH(Variable* var, vars)
		{
			if (!var->GetView() && var->VariableType() != XMILE_Type_ARRAY && var->VariableType() != XMILE_Type_ARRAY_ELM && var->VariableType() != XMILE_Type_UNKNOWN)
				dump_view->AddVarDefinition(var, 200, 200);
		}
	}

	// now everything is defined (and only defined once) - we need to make sure there are no missing connectors
	BOOST_FOREACH(View* view, vViews)
	{
		view->CheckLinksIn();
	}

}


bool Model::RenameVariable(Variable *v,const std::string &newname)
{
   assert(!newname.empty()) ;
   if(mSymbolNameSpace.Find(newname)) {
      if(!newname.compare(v->GetName()))
         return true ; // nothing to do
      return false ;
   }
   if(!mSymbolNameSpace.Remove(v))
      return false ;
   v->SetName(newname) ;
   mSymbolNameSpace.Insert(v) ;
   return true ;
}

void Model::GenerateCanonicalNames(void)
{
   assert(0) ;
}

void Model::GenerateShortNames(void)
{
   Variable *v ;
   int i = 0 ;
   std::string s ;
   SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable() ;
   BOOST_FOREACH(const SymbolNameSpace::iterator it,*ht) {
      v = static_cast<Variable *>SNSitToSymbol(it) ;
      if(v->isType() == Symtype_Variable) {
         s = "v" + boost::lexical_cast<std::string>(i); ;
         i++;
         v->SetAlternateName(s) ;
         //v->SetAlternateName(v->GetName()) ;
      }
   }
   BOOST_FOREACH(v,vUnamedVars) {
      s = "v" + boost::lexical_cast<std::string>(i); ;
      i++;
      v->SetAlternateName(s) ;
   }
}

double Model::GetConstanValue(const char *str, double val)
{
	Symbol* s = mSymbolNameSpace.Find(str);
	if (s && s->isType() == Symtype_Variable)
	{
		Variable* v = static_cast<Variable*>(s);
		Equation* eq = v->GetEquation(0);
		if (eq)
		{
			Expression* exp = eq->GetExpression();
			if (exp && exp->GetType() == EXPTYPE_Number)
				val = exp->Eval(NULL);
		}
	}
	return val;
}

void Model::SetUnwanted(const char *str, const char *defname)
{
	Symbol* s = mSymbolNameSpace.Find(str);
	if (s && s->isType() == Symtype_Variable)
	{
		Variable* v = static_cast<Variable*>(s);
		v->SetUnwanted(true);
		v->SetAlternateName(defname);
	}
}


std::vector<Variable*> Model::GetVariables(SymbolNameSpace *ns)
{
	std::vector<Variable*> vars;
	SymbolNameSpace::HashTable* ht;
	if (ns)
		ht = ns->GetHashTable();
	else
		ht = mSymbolNameSpace.GetHashTable();
	for (auto it = ht->begin(); it != ht->end(); it++)
	{
		Symbol* s = it->second;
		if (s->isType() == Symtype_Variable)
			vars.push_back(static_cast<Variable*>(s));
	}
	return vars;
}

bool Model::WriteToXMILE(const std::string& path, std::vector<std::string>& errs)
{
	bool success = true;

	// sim specs are different 

	XMILEGenerator generator(this);
	success = generator.Generate(path, errs);

	return success;
}
