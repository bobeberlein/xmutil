#include "SymbolList.h"
#include "../XMUtil.h"


SymbolList::SymbolList(SymbolNameSpace *sns,Symbol *first,bool bang) : SymbolTableBase(sns)
{
	vSymbols.push_back(SymbolListEntry(first, bang));
   pMapRange = '\0';
}

SymbolList::SymbolList(SymbolNameSpace *sns, SymbolList *first) : SymbolTableBase(sns)
{
	vSymbols.push_back(SymbolListEntry(first));
	pMapRange = '\0';
}



SymbolList::~SymbolList(void)
{
// do nothing symbols in one hash table or another
}
