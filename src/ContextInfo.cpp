#include "ContextInfo.h"
#include "Symbol/Symbol.h"
#include "Symbol/Variable.h"

Symbol* ContextInfo::GetLHSSpecific(Symbol* dim)
{
	if (!pLHSElmsGeneric || dim->isType()  != Symtype_Variable)
		return dim;
	if (dim->Owner() && dim->Owner() != dim)
	{
		// either an element - just return dim - or a subrange - 
		Variable* v = static_cast<Variable*>(dim->Owner());
		std::vector<Equation*> eqns = v->GetAllEquations();
		if (eqns.empty())
			return dim;
		// this is a subrange - 
		Symbol* subelm = GetLHSSpecific(v);
		dim = dim;
	}
	int n = pLHSElmsGeneric->size();
	int i;
	for (i = 0; i < n; i++)
	{
		if ((*pLHSElmsGeneric)[i] == dim)
			return (*pLHSElmsSpecific)[i];
	}
	return dim;
}
