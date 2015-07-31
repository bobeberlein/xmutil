#include "SymbolList.h"
#include "../XMUtil.h"


SymbolList::SymbolList(SymbolNameSpace *sns,Symbol *first,int bang) : SymbolTableBase(sns)
{
   vSymbols.push_back(first) ;
   vMarked.push_back(bang) ;
}


SymbolList::~SymbolList(void)
{
// do nothing symbols in one hash table or another
}
