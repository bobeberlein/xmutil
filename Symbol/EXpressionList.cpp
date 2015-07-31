#include "ExpressionList.h"
#include <boost/foreach.hpp>
#include "../XMUtil.h"


ExpressionList::ExpressionList(SymbolNameSpace *sns)
   : SymbolTableBase(sns) 
{

}


ExpressionList::~ExpressionList(void)
{
   if(this->HasGoodAlloc()) {
      BOOST_FOREACH(Expression *e,vExpressions) {
         delete e ;
      }
   }
}

void ExpressionList::CheckPlaceholderVars(Model *m) 
{
   BOOST_FOREACH(Expression *e,vExpressions) {
      e->CheckPlaceholderVars(m,false) ;
   }
}

bool ExpressionList::CheckComputed(ContextInfo *info,unsigned wantargs) 
{
   int i = 1 ;
   BOOST_FOREACH(Expression *e,vExpressions) {
      if(i&wantargs) {
         if(!e->CheckComputed(info))
            return false ;
      }
      i = i << 1 ;
   }
   return true ;
}

void ExpressionList::OutputComputable(ContextInfo *info,unsigned wantargs) 
{
   int i = 1 ;
   int j = 0 ;
   BOOST_FOREACH(Expression *e,vExpressions) {
      if(i&wantargs) {
         if(j++)
            *info << "," ;
         e->OutputComputable(info) ;
      }
      i = i << 1 ;
   }
}

