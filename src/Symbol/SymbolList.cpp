#include "SymbolList.h"
#include "../XMUtil.h"

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

void SymbolList::SetOwner(Variable* var)
{
	if (vSymbols.empty())
		return;
	for (size_t i = 0; i < vSymbols.size(); i++)
	{
		vSymbols[i].SetOwner(var);
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
			*info << SpaceToUnderBar(vSymbols[i].u.pSymbol->GetName());
		}
	}
	*info << "]";
}
