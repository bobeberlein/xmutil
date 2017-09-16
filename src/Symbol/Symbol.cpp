#include "Symbol.h"
#include "SymbolNameSpace.h"
#include "../XMUtil.h"
#include "Variable.h"

// Abstract Symbol. Parent to Variable, Function, Token...
// Has a changeable name and returns a type
// does not manage self needs to be part of a collection


Symbol::Symbol(SymbolNameSpace *sns,const std::string &name) 
   : SymbolTableBase(sns)
{
   sName = name ;
   pOwner = NULL;
   // insert into the name space sns if it has a name - empty get special treatment
   if(!name.empty())
      sns->Insert(this) ;
}

Symbol::~Symbol(void)
{
   if(!HasGoodAlloc() && !sName.empty()) { // remove from the lookup table
      GetSymbolNameSpace()->Remove(this) ;
   }
}

const std::string &Symbol::GetName(void)
{
   return sName ;
}

void Symbol::SetOwner(Symbol* var) 
{ 
	if (!pOwner || static_cast<Variable*>(pOwner)->Nelm() < static_cast<Variable*>(var)->Nelm())
	{
		if (pOwner)
			pOwner->SetOwner(var);
		pOwner = var;
	}
}


SYMTYPE Symbol::isType(void)
{
   return Symtype_None ;
}
