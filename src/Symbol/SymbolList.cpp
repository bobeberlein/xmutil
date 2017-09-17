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
			Equation::GetSubscriptElements(expanded, vSymbols[i].u.pSymbol);
		}
	}
	var->SetNelm(expanded.size());
	// the two that follow might end up doing the same thing depending on the content of the defining list
	for (size_t i = 0; i < vSymbols.size(); i++)
	{
		if (vSymbols[i].eType == EntryType_SYMBOL)
		{
			vSymbols[i].u.pSymbol->SetOwner(var);
		}
	}
	BOOST_FOREACH(Symbol* s, expanded)
	{
		s->SetOwner(var);
	}
	if (expanded[0]->Owner() != var)
		var->SetOwner(expanded[0]->Owner());
}


void SymbolList::OutputComputable(ContextInfo *info)
{
	if (vSymbols.empty())
		return;
	*info << "[";
	for (size_t i = 0; i < vSymbols.size(); i++)
	{
		if (i)
			*info << ", ";
		if (vSymbols[i].eType == EntryType_SYMBOL)
		{
			// try to find the symbol in the lhs generic list - if there substitue specific otherwise
			// use the original symbol
			Symbol* s = info->GetLHSSpecific(vSymbols[i].u.pSymbol);
			*info << SpaceToUnderBar(s->GetName());
		}
		else if (vSymbols[i].eType == EntryType_BANG_SYMBOL)
		{
			*info << "*"; // normally this is all 
			Symbol* s = vSymbols[i].u.pSymbol;
			if (s->Owner() != s)
				*info << SpaceToUnderBar(s->GetName());
		}
	}
	*info << "]";
}
