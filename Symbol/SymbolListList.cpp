#include "SymbolListList.h"
#include "../XMUtil.h"


SymbolListList::SymbolListList(SymbolNameSpace *sns,SymbolList *first)
   : SymbolTableBase(sns)
{
   if(first)
      vSymbolLists.push_back(first) ;
}


SymbolListList::~SymbolListList(void)
{
   int n = vSymbolLists.size() ;
   for(int i = 0; i< n;i++)
      delete vSymbolLists[i] ;
}
