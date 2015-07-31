#ifndef _XMUTIL_SYMBOLLISTLIST_H
#define _XMUTIL_SYMBOLLISTLIST_H

#include "SymbolTableBase.h"
#include "SymbolList.h"
#include <vector>

class SymbolListList 
   : public SymbolTableBase
{
public:
   SymbolListList(SymbolNameSpace *sns,SymbolList *first);
   SymbolListList *Append(SymbolList *last) { vSymbolLists.push_back(last) ; return this ;}
   int Length(void) { return vSymbolLists.size() ; }
   const SymbolList* operator[]( int pos ) const {return vSymbolLists[pos] ; }
   ~SymbolListList(void);
private :
   std::vector<SymbolList*>vSymbolLists ;
};


#endif