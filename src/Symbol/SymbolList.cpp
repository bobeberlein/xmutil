#include "SymbolList.h"
#include "../XMUtil.h"
#include "Equation.h"
#include "Variable.h"

void SymbolList::SymbolListEntry::SetOwner(Variable* var)
{
	if (eType == EntryType_LIST) 
		this->u.pSymbolList->SetOwner(var); 
	else 
		this->u.pSymbol->SetOwner(var); 
}


SymbolList::SymbolList(SymbolNameSpace *sns,Symbol *first,bool bang) : SymbolTableBase(sns)
{
	vSymbols.push_back(SymbolListEntry(first, bang));
   pMapRange = NULL;
}

SymbolList::SymbolList(SymbolNameSpace *sns, SymbolList *first) : SymbolTableBase(sns)
{
	vSymbols.push_back(SymbolListEntry(first));
	pMapRange = NULL;
}



SymbolList::~SymbolList(void)
{
// do nothing symbols in one hash table or another
}

// set the owner of array - only if we are bigger than other owners
void SymbolList::SetOwner(Variable* var)
{
	if (vSymbols.empty())
		return;
	std::vector<Symbol*> expanded;
	for (size_t i = 0; i < vSymbols.size(); i++)
	{
		if (vSymbols[i].eType == EntryType_SYMBOL)
		{
			vSymbols[i].u.pSymbol->SetOwner(var);
			Equation::GetSubscriptElements(expanded, vSymbols[i].u.pSymbol);
		}
	}
	var->SetNelm(expanded.size());

	BOOST_FOREACH(Symbol* s, expanded)
	{
		s->SetOwner(var);
	}
}


void SymbolList::OutputComputable(ContextInfo *info)
{
	if (vSymbols.empty())
		return;
	*info << "[";
	for (size_t i = 0; i < vSymbols.size(); i++)
	{
		if (vSymbols[i].eType == EntryType_SYMBOL)
		{
			if (i)
				*info << ", ";
			// try to find the symbol in the lhs generic list - if there substitue specific otherwise
			// use the original symbol
			Symbol* s = info->GetLHSSpecific(vSymbols[i].u.pSymbol);
			*info << SpaceToUnderBar(s->GetName());
		}
	}
	*info << "]";
}
