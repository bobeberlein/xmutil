#include "SymbolList.h"

#include "../XMUtil.h"
#include "Equation.h"
#include "Variable.h"

void SymbolList::SymbolListEntry::SetOwner(Variable *var) {
  if (eType == EntryType_LIST)
    this->u.pSymbolList->SetOwner(var);
  else
    this->u.pSymbol->SetOwner(var);
}

SymbolList::SymbolList(SymbolNameSpace *sns, Symbol *first, bool bang) : SymbolTableBase(sns) {
  vSymbols.push_back(SymbolListEntry(first, bang));
  pMapRange = NULL;
}

SymbolList::SymbolList(SymbolNameSpace *sns, SymbolList *first) : SymbolTableBase(sns) {
  vSymbols.push_back(SymbolListEntry(first));
  pMapRange = NULL;
}

SymbolList::~SymbolList(void) {
  // do nothing symbols in one hash table or another
}

SymbolList *SymbolList::Clone() {
  // Clone runs during parsing, before SymbolNameSpace::ConfirmAllAllocations
  // nulls out pSymbolNameSpace, so GetSymbolNameSpace() still returns the
  // owning namespace. The new list registers itself there via the standard
  // SymbolTableBase ctor path -- it lives or dies with the parse the same way
  // every other freshly-allocated SymbolList does.
  SymbolNameSpace *sns = GetSymbolNameSpace();
  SymbolList *copy = nullptr;
  for (const SymbolListEntry &entry : vSymbols) {
    if (entry.eType == EntryType_LIST) {
      SymbolList *nested = entry.u.pSymbolList ? entry.u.pSymbolList->Clone() : nullptr;
      if (!copy)
        copy = new SymbolList(sns, nested);
      else
        copy->Append(nested);
    } else {
      bool bang = (entry.eType == EntryType_BANG_SYMBOL);
      if (!copy)
        copy = new SymbolList(sns, entry.u.pSymbol, bang);
      else
        copy->Append(entry.u.pSymbol, bang);
    }
  }
  // An empty source list has no subscripts to copy, so the copy is "no
  // subscript list" -- which is what a null SymbolList* already means to every
  // consumer (BuildNetFlowSubscripted passes one for a scalar stock).
  // Fabricating a one-entry list holding a NULL Symbol instead, which is what
  // this used to do, handed the caller an object that OutputComputable's
  // EntryType_SYMBOL branch and SetOwner both dereference unconditionally --
  // the null guard a few lines below covers only the BANG_SYMBOL spelling.
  // Neither ctor allows an empty list today, so this is a contract rather than
  // a bug fix; the point is that it fails closed if that ever changes.
  if (!copy)
    return nullptr;
  if (pMapRange)
    copy->SetMapRange(pMapRange);
  return copy;
}

// set the owner of array - only if we are bigger than other owners
void SymbolList::SetOwner(Variable *var) {
  if (vSymbols.empty())
    return;
  std::vector<Symbol *> expanded;
  for (size_t i = 0; i < vSymbols.size(); i++) {
    if (vSymbols[i].eType == EntryType_SYMBOL) {
      Equation::GetSubscriptElements(expanded, vSymbols[i].u.pSymbol);
    }
  }
  var->SetNelm(expanded.size());
  // the two that follow might end up doing the same thing depending on the content of the defining list
  for (size_t i = 0; i < vSymbols.size(); i++) {
    if (vSymbols[i].eType == EntryType_SYMBOL) {
      vSymbols[i].u.pSymbol->SetOwner(var);
    }
  }
  for (Symbol *s : expanded) {
    s->SetOwner(var);
  }
  if (expanded[0]->Owner() != var)
    var->SetOwner(expanded[0]->Owner());
}

void SymbolList::OutputComputable(ContextInfo *info) {
  if (vSymbols.empty())
    return;
  info->SetInSubList(true);
  *info << "[";
  for (size_t i = 0; i < vSymbols.size(); i++) {
    if (i)
      *info << ", ";
    if (vSymbols[i].eType == EntryType_SYMBOL) {
      // try to find the symbol in the lhs generic list - if there substitue specific otherwise
      // use the original symbol
      Symbol *s = info->GetLHSSpecific(vSymbols[i].u.pSymbol);
      *info << SpaceToUnderBar(s->GetName());
      if (i == vSymbols.size() - 1 && info->WantFinalStar())
        *info << ".*";
    } else if (vSymbols[i].eType == EntryType_BANG_SYMBOL) {
      Symbol *s = vSymbols[i].u.pSymbol;
      if (!s) {
        // Unbound `*` wildcard: XmileReader resolution binds these to a concrete
        // dimension before output, but guard against a null deref regardless.
        *info << "*";
      } else if (s->Owner() != s) {
        // Subrange bang: emit `*:Sub`, the inverse of the reader's
        // `sub_term: '*' ':' symbol` production (XmileEqYacc.y). The older
        // `Sub.*` spelling was not accepted by the equation grammar, so an
        // XMILE->XMILE round trip of a subrange wildcard failed to re-parse.
        *info << "*:" << SpaceToUnderBar(s->GetName());

        //// if this is a contiguous subrange we can use a:b notation - otherwise can't do it
        // std::vector<Symbol*> elms;
        // Equation::GetSubscriptElements(elms, s);
        // if(elms.size() == 1)
        //	*info << SpaceToUnderBar(elms[0]->GetName());
        // else
        //{
        //	std::vector<Symbol*> pelms;
        //	Symbol* owner = s->Owner();
        //	while (owner != owner->Owner())
        //		owner = owner->Owner(); // this is a bug elsewhere that does not properly reassign owners
        //	Equation::GetSubscriptElements(pelms, owner);
        //	int n = elms.size();
        //	int m = pelms.size();
        //	int i = 0;
        //	int j = 0;
        //	for (; j < m; j++)
        //	{
        //		if (pelms[j] == elms[i])
        //			break;
        //	}
        //	for (i = 1; i < n; i++)
        //	{
        //		j++; // this is next
        //		if (j >= m || pelms[j] != elms[i])
        //			break;
        //	}
        //	if (i == n)
        //	{
        //		*info << SpaceToUnderBar(elms.front()->GetName()) << ":" <<
        // SpaceToUnderBar(elms.back()->GetName());
        //	}
        //	else
        //		*info << "*" << SpaceToUnderBar(s->GetName());
        // }
      } else
        *info << "*";  // normally this is all
    }
  }
  *info << "]";
  info->SetInSubList(false);
}
