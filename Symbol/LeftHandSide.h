#ifndef _XMUTIL_SYMBOL_LEFTHANDSIDE_H
#define _XMUTIL_SYMBOL_LEFTHANDSIDE_H


/* left hand side - mostly just ExpressionVariable but could include some extra
   stuff 
   */

#include "SymbolTableBase.h"
#include "Expression.h"
#include "SymbolListList.h"

class LeftHandSide :
   public SymbolTableBase
{
public:
   LeftHandSide(SymbolNameSpace *sns,ExpressionVariable *var,SymbolListList *exceptlist,int interpmode) ;
   ~LeftHandSide(void) ;
   Variable *GetVariable(void) {return pExpressionVariable->GetVariable() ; }
private :
   ExpressionVariable *pExpressionVariable ;
   SymbolListList *pExceptList ; 
   int iInterpMode ; // nonstandard - Vensim for data equations what else?

};


#endif