#ifndef _XMUTIL_SYMLIST_H
#define _XMUTIL_SYMLIST_H

#include "SymbolTableBase.h"
#include "Symbol.h"
#include <vector>



class SymbolList 
   : public SymbolTableBase
{
public:
   SymbolList(SymbolNameSpace *sns,Symbol *first,int bang) ;
   ~SymbolList(void);
   SymbolList *Append(Symbol *last,int bang) { vSymbols.push_back(last) ; vMarked.push_back(bang); return this ;}
   int Length(void) { return vSymbols.size() ; }
   const Symbol* operator[]( int pos ) const {return vSymbols[pos] ; }
private :
   std::vector<Symbol*>vSymbols ;
   std::vector<char>vMarked ;
};


#endif