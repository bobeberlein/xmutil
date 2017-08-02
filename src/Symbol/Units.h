#ifndef _XMUTIL_SYMBOL_UNITS_H
#define _XMUTIL_SYMBOL_UNITS_H
#include "Symbol.h"
#include "UnitExpression.h"
#include <vector>

class Units :
   public Symbol
{
public:
   Units(SymbolNameSpace *sns,const std::string &name)  ;
   ~Units(void);
   inline SYMTYPE isType(void) {return Symtype_Units ; }
private :
   UnitExpression *pUnitExpression ; // in case a derived unit of measure 
   // other stuff? 

};

#endif

