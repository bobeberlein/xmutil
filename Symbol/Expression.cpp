#include "Expression.h"
#include "ExpressionList.h"
#include "../Model.h"
#include "../XMUtil.h"
#include "Equation.h"
#include "LeftHandSide.h"


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

static void is_all_plus_minus(Expression *e, FlowList* fl,bool neg)
{
	if (!e)
		fl->SetValid(false);
	else if (e->GetType() == EXPTYPE_Variable) {
		ExpressionVariable *ev = static_cast<ExpressionVariable*>(e);
		if (neg)
			fl->AddOutflow(ev->GetVariable());
		else
			fl->AddInflow(ev->GetVariable());
	}
	else if (e->GetType() == EXPTYPE_Operator)
	{
		const char *op = e->GetOperator();
		if (op && (*op == '-' || *op == '+') && op[1] == '\0')
		{
			is_all_plus_minus(e->GetArg(0), fl, neg);
			if (*op == '-')
				neg = !neg;
			is_all_plus_minus(e->GetArg(1), fl, neg);
		}
		else
			fl->SetValid(false);
	}
}

bool ExpressionFunctionMemory::TestMarkFlows(SymbolNameSpace *sns, FlowList *fl, Equation *eq)
{
	if (this->GetFunction()->GetName() != "INTEG" && this->GetFunction()->GetName() != "SINTEG")
		return false;
	// only care about active part here - if it all a+b+c-d-e or similar then we are good to go otherwise
	// we need to make up a new variable and then use that as the equation in place of what was here
	Expression* e = this->GetArgs()->GetExp(0);
	if (eq) // make a change
	{
		assert(fl);
		// we set the first argument for the variable in the flow list 
		ExpressionVariable* ev = new ExpressionVariable(sns, fl->NewVariable(), eq->GetLeft()->GetSubs());
		this->GetArgs()->SetExp(0, ev);

	}
	else if (fl) // populat flow list
	{
		is_all_plus_minus(e, fl, false);
		fl->SetActiveExpression(e);
	}
	return true;
}

void FlowList::AddInflow(Variable *v)
{
	if (std::find(vInflows.begin(), vInflows.end(), v) != vInflows.end())
		bValid = false;
	else if (std::find(vOutflows.begin(), vOutflows.end(), v) != vOutflows.end())
		bValid = false;
	else
		vInflows.push_back(v);
}

void FlowList::AddOutflow(Variable *v)
{
	if (std::find(vInflows.begin(), vInflows.end(), v) != vInflows.end())
		bValid = false;
	else if (std::find(vOutflows.begin(), vOutflows.end(), v) != vOutflows.end())
		bValid = false;
	else
		vOutflows.push_back(v);
}

bool FlowList::operator == (const FlowList& rhs)
{
	if (!bValid || !rhs.bValid || vInflows.size() != rhs.vInflows.size() ||
		vOutflows.size() != rhs.vOutflows.size())
		return false;
	BOOST_FOREACH(Variable* v, rhs.vInflows) {
		if (std::find(vInflows.begin(), vInflows.end(), v) == vInflows.end())
			return false;
	}
	BOOST_FOREACH(Variable* v, rhs.vOutflows) {
		if (std::find(vOutflows.begin(), vOutflows.end(), v) == vOutflows.end())
			return false;
	}
	return true;
}