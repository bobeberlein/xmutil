#ifndef _XMUTIL_SYMBOL_UNITEXPRESSION_H
#define _XMUTIL_SYMBOL_UNITEXPRESSION_H
#include "SymbolTableBase.h"
#include <vector>
class Units ; // forward declaration

class UnitExpression :
   public SymbolTableBase
{
public:
   UnitExpression(SymbolNameSpace *sns,Units *num);
   ~UnitExpression(void);
   UnitExpression *Multiply(UnitExpression *mult) ;
   UnitExpression *Divide(UnitExpression *denom) ;
   inline void SetRange(double minval,double maxval) { dMinVal = minval ; dMaxVal = maxval ;} 
private :
   std::vector<Units*>vNumerator ;
   std::vector<Units*>vDenominator ;
   double dMinVal ;
   double dMaxVal ;
   void Simplify(void) ;

};

#endif

