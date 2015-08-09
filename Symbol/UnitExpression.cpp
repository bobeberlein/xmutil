#include "UnitExpression.h"
#include "Units.h"
#include "../XMUtil.h"


UnitExpression::UnitExpression(SymbolNameSpace *sns,Units *num)
  : SymbolTableBase(sns) 
{
   vNumerator.push_back(num) ;
   dMinVal = dMaxVal = dIncrement = -1e30 ; // later
}


UnitExpression::~UnitExpression(void)
{

}

UnitExpression *UnitExpression::Multiply(UnitExpression *mult) 
{
   int i,n ;
   n=mult->vNumerator.size() ;
   for(i=0;i<n;i++)
      vNumerator.push_back(mult->vNumerator[i]) ;
   n=mult->vDenominator.size() ;
   for(i=0;i<n;i++)
      vDenominator.push_back(mult->vDenominator[i]) ;
   delete mult ;
   Simplify() ;
   return this ;
}
UnitExpression *UnitExpression::Divide(UnitExpression *denom) 
{
   int i,n ;
   n=denom->vDenominator.size() ;
   for(i=0;i<n;i++)
      vNumerator.push_back(denom->vDenominator[i]) ;
   n=denom->vNumerator.size() ;
   for(i=0;i<n;i++)
      vDenominator.push_back(denom->vNumerator[i]) ;
   delete denom ;
   Simplify() ;
   return this ;
}

void UnitExpression::Simplify()
{
   // brute force
   int i,j,ntop,nbottom ;
   ntop = vNumerator.size() ;
   nbottom = vDenominator.size() ;
   for(i=0;i<ntop;i++) {
      for(j=0;j<nbottom;j++) {
         if(vNumerator[i] == vDenominator[j]) {
            // remove top and bottom
            vNumerator.erase(vNumerator.begin() + i) ;
            vDenominator.erase(vDenominator.begin() + j) ;
            i--,j--,ntop--,nbottom-- ;
            break ; //inner loop start again next numerator element 
         }
      }
   }
}

