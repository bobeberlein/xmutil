#ifndef _XMUTIL_SYMLIST_H
#define _XMUTIL_SYMLIST_H

#include <vector>

#include "Symbol.h"
#include "SymbolTableBase.h"

class SymbolList : public SymbolTableBase {
public:
  enum EntryType { EntryType_SYMBOL, EntryType_BANG_SYMBOL, EntryType_LIST };
  class SymbolListEntry {
  public:
    SymbolListEntry(Symbol *s, bool bang) {
      u.pSymbol = s;
      eType = bang ? EntryType_BANG_SYMBOL : EntryType_SYMBOL;
    }
    SymbolListEntry(SymbolList *s) {
      u.pSymbolList = s;
      eType = EntryType_LIST;
    }
    union {
      Symbol *pSymbol;
      SymbolList *pSymbolList;
    } u;
    void SetOwner(Variable *var);
    EntryType eType;
  };
  SymbolList(SymbolNameSpace *sns, Symbol *first, bool bang);
  SymbolList(SymbolNameSpace *sns, SymbolList *first);
  ~SymbolList(void);
  SymbolList *Append(Symbol *last, bool bang) {
    vSymbols.push_back(SymbolListEntry(last, bang));
    return this;
  }
  SymbolList *Append(SymbolList *next) {
    vSymbols.push_back(SymbolListEntry(next));
    return this;
  }
  // Deep-copy producing a brand-new SymbolList in the same namespace. The XMILE
  // reader needs this because one synthesized INTEG expression for an arrayed
  // stock holds several ExpressionVariable children -- each inflow / outflow
  // reference plus the LHS -- and every one of those owns its SymbolList
  // outright (deletes it in its dtor). Sharing a single pointer between two
  // owners would double-free at namespace teardown. The Vensim grammar avoids
  // the problem by re-running its SymList builder per call site; the DOM-walker
  // has nowhere to re-run, so it Clones. EntryType_SYMBOL / _BANG_SYMBOL share
  // the underlying Variable* (those live in the namespace, not in any list);
  // EntryType_LIST entries are recursively cloned so each leaf list has a
  // unique owner. pMapRange is shallow-copied for the same Variable*-ownership
  // reason. Returns NULL for an empty source list -- "no subscripts" is what a
  // null SymbolList* already means to every consumer, and there is no valid
  // one-entry list that expresses it.
  SymbolList *Clone();
  int Length(void) {
    return vSymbols.size();
  }
  const SymbolListEntry &operator[](int pos) const {
    return vSymbols[pos];
  }
  bool IsMapList() {
    return pMapRange != NULL;
  }
  // Bind a bare `*` wildcard entry -- a BANG_SYMBOL whose symbol is still null,
  // as produced by the XMILE grammar's `[*]` -- to a concrete dimension once
  // the referenced variable's shape is known (XmileReader wildcard resolution).
  // No-op unless the entry at pos is a null-symbol BANG_SYMBOL.
  void BindWildcard(int pos, Symbol *dim) {
    if (pos < 0 || pos >= static_cast<int>(vSymbols.size()))
      return;
    if (vSymbols[pos].eType == EntryType_BANG_SYMBOL && vSymbols[pos].u.pSymbol == NULL)
      vSymbols[pos].u.pSymbol = dim;
  }
  Symbol *MapRange() {
    return pMapRange;
  }
  void SetMapRange(Symbol *range) {
    assert(!pMapRange);
    pMapRange = range;
  }
  void SetOwner(Variable *var);
  virtual void OutputComputable(ContextInfo *info);

private:
  std::vector<SymbolListEntry> vSymbols;
  Symbol *pMapRange;
};

#endif
