#include "Model.h"

#include <algorithm>
#include <unordered_set>
#include <vector>

#include "Mdl/MDLGenerator.h"
#include "Symbol/Equation.h"
#include "Symbol/ExpressionList.h"
#include "Symbol/LeftHandSide.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolList.h"
#include "Vensim/VensimView.h"
#include "XMUtil.h"
#include "Xmile/XMILEGenerator.h"
#include "Xmile/XmileReader.h"

Model::Model(void) {
  dLevel = dRate = dAux = NULL;
  bAsSectors = false;
  iIntegrationType = Integration_Type_EULER;
  bLetterPolarity = false;
  bFromXmile = false;
  _initial_time = -1;
  _final_time = 200;
  _dt = 1;
  bFromDyanmo = false;
}

Model::~Model(void) {
  // allocation is no longer clean ClearCompEquations() ;
}

bool Model::ParseXMILE(const std::string &filename, const char *contents, size_t len, std::vector<std::string> &errs) {
  // Recorded before the parse rather than after a successful one: the flag
  // describes where this Model's contents are coming from, and a half-populated
  // Model from a failed parse came from XMILE just as much as a complete one.
  bFromXmile = true;
  XmileReader reader{this};
  return reader.ProcessFile(filename, contents, len, errs);
}

void Model::RunPostParsePipeline() {
  // VensimParse confirms allocations after each equation; XMILE parsing does
  // not. Calling ConfirmAllAllocations here is idempotent for Vensim (the
  // unconfirmed set is already empty) and required for XMILE so that objects
  // created during parsing are not treated as abandoned allocations by any
  // later DeleteAllUnconfirmedAllocations call on the exception path.
  mSymbolNameSpace.ConfirmAllAllocations();
  MarkVariableTypes(nullptr);
  // AdjustGroupNames mutates ModelGroup::sName to disambiguate collisions; the
  // writer must emit the post-adjustment names so re-parsing reproduces the
  // model faithfully.
  AdjustGroupNames();
  for (MacroFunction *mf : mMacroFunctions)
    MarkVariableTypes(mf->NameSpace());
  // Ghosts that are never defined elsewhere have their first appearance
  // promoted to owner so the model graph is consistent.
  CheckGhostOwners();
  // Element/family ownership is now established, so bare `*` wildcards from the
  // XMILE reader can be bound to their concrete dimensions. Any that cannot be
  // bound are recorded in vUnresolvedWildcards for the writers to reject.
  vUnresolvedWildcards.clear();
  ResolveWildcardSubscripts(nullptr);
  for (MacroFunction *mf : mMacroFunctions)
    ResolveWildcardSubscripts(mf->NameSpace());
}

void Model::ResolveWildcardSubscripts(SymbolNameSpace *ns) {
  for (Variable *v : GetVariables(ns)) {
    for (Equation *eq : v->GetAllEquations())
      ResolveWildcardsInExpr(eq->GetExpression());
    for (Equation *eq : v->GetAllInitEquations())
      ResolveWildcardsInExpr(eq->GetExpression());
  }
}

void Model::ResolveWildcardsInExpr(Expression *e) {
  if (!e)
    return;
  switch (e->GetType()) {
  case EXPTYPE_Variable:
    ResolveWildcardsInVarRef(static_cast<ExpressionVariable *>(e));
    break;
  case EXPTYPE_Operator:
    // Covers +, -, *, /, ^, parens and unary minus (all ExpressionOperator2).
    ResolveWildcardsInExpr(e->GetArg(0));
    ResolveWildcardsInExpr(e->GetArg(1));
    break;
  case EXPTYPE_Logical: {
    ExpressionLogical *lg = static_cast<ExpressionLogical *>(e);
    ResolveWildcardsInExpr(lg->GetLeft());
    ResolveWildcardsInExpr(lg->GetRight());
    break;
  }
  case EXPTYPE_Function:
  case EXPTYPE_FunctionMemory: {
    ExpressionList *args = static_cast<ExpressionFunction *>(e)->GetArgs();
    if (args) {
      for (int i = 0; i < args->Length(); i++)
        ResolveWildcardsInExpr(args->GetExp(i));
    }
    break;
  }
  case EXPTYPE_Lookup: {
    ExpressionLookup *lk = static_cast<ExpressionLookup *>(e);
    ResolveWildcardsInExpr(lk->GetInput());
    ResolveWildcardsInExpr(lk->GetLookupVariable());
    break;
  }
  default:
    // Number, literal, table, symlist: no variable-reference subscripts.
    break;
  }
}

void Model::ResolveWildcardsInVarRef(ExpressionVariable *ev) {
  if (!ev)
    return;
  SymbolList *subs = ev->GetSubs();
  if (!subs)
    return;
  Variable *target = ev->GetVariable();
  for (int i = 0; i < subs->Length(); i++) {
    const SymbolList::SymbolListEntry &entry = (*subs)[i];
    // Only a bare `*` needs binding: `*:Dim` and named subscripts already carry
    // a concrete symbol.
    if (entry.eType != SymbolList::EntryType_BANG_SYMBOL || entry.u.pSymbol != NULL)
      continue;
    if (Symbol *fam = target ? FamilyAtPosition(target, i) : nullptr) {
      subs->BindWildcard(i, fam);
    } else {
      // The reference uses `[*]` but the target is not an arrayed variable at
      // this position (non-arrayed, or has no equation to read dimensions
      // from). There is no dimension to bind, so this wildcard cannot be
      // rendered as valid output; record it so the writer fails cleanly.
      std::string name = target ? target->GetName() : std::string("<unknown>");
      vUnresolvedWildcards.push_back("cannot resolve '*' wildcard at subscript position " + std::to_string(i + 1) +
                                     " of reference to '" + name + "': the target is not an arrayed variable");
    }
  }
}

Symbol *Model::FamilyAtPosition(Variable *target, int pos) {
  std::vector<Equation *> eqs = target->GetAllEquations();
  if (eqs.empty())
    return nullptr;
  // Every equation of an arrayed variable shares the same dimensionality, so
  // the first equation's LHS subscript list gives the family (apply-to-all,
  // `x[Dim]`) or an element (per-element, `x[Elem]`) at each position. Walk an
  // element up to its owning dimension family -- the form a Vensim bang names.
  LeftHandSide *lhs = eqs[0]->GetLeft();
  SymbolList *lhsSubs = lhs ? lhs->GetSubs() : nullptr;
  if (!lhsSubs || pos < 0 || pos >= lhsSubs->Length())
    return nullptr;
  const SymbolList::SymbolListEntry &e = (*lhsSubs)[pos];
  if (e.eType != SymbolList::EntryType_SYMBOL)
    return nullptr;
  Symbol *s = e.u.pSymbol;
  // An apply-to-all declaration (`x[Dim]`) already names the dimension family at
  // this position; return it verbatim. Walking Owner() here would be wrong: when
  // two dimensions share the same elements (a reordered alias, e.g. DimX over the
  // same members as DimA), the family's owner can point at the alias, and which
  // one wins depends on dimension declaration order -- unstable across a round
  // trip. Only a per-element declaration (`x[Elem]`) needs the element walked up
  // to its owning family.
  if (s && s->isType() == Symtype_Variable && static_cast<Variable *>(s)->VariableType() == XMILE_Type_ARRAY)
    return s;
  while (s && s->Owner() != s)
    s = s->Owner();
  return s;
}

Equation *Model::AddUnnamedVariable(ExpressionFunctionMemory *e) {
  ExpressionFunction *e2;
  std::string s;
  assert(s.empty());
  Variable *var = new Variable(&mSymbolNameSpace, s);
  ExpressionVariable *ev = new ExpressionVariable(&mSymbolNameSpace, var, NULL);
  LeftHandSide *lhs = new LeftHandSide(&mSymbolNameSpace, ev, NULL, NULL, 0);
  e2 = new ExpressionFunction(&mSymbolNameSpace, e->GetFunction(), e->GetArgs());
  Equation *eq = new Equation(&mSymbolNameSpace, lhs, e2, '=');
  // log("Adding in a placeholder from function %s\n",e->GetFunction()->GetName().c_str()) ;
  var->AddEq(eq);
  vUnamedVars.push_back(var);
  return eq;
}

void Model::ClearCompEquations(void) {
  vActiveComps.clear();
  vInitialComps.clear();
  vUnchangingComps.clear();
  vInitialTimeComps.clear();

  SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
  for (const SymbolNameSpace::iterator &it : *ht) {
    SNSitToSymbol(it)->SetupState(NULL);
    SNSitToSymbol(it)->CheckPlaceholderVars(NULL);
  }
  for (Variable *v : vUnamedVars) {
    v->GetEquation(0)->GetExpression()->RemoveFunctionArgs();  // these are allocated in the real variable's equation
    v->SetupState(NULL);
    delete v;
  }
  if (dLevel) {
    delete dLevel;
    dLevel = NULL;
  }
  if (dRate) {
    delete dRate;
    dRate = NULL;
  }
  if (dAux) {
    delete dAux;
    dAux = NULL;
  }
}
typedef struct {
  Variable *v;
  int count;
} SubInfoWCount;
bool Model::OrganizeSubscripts(void) {
  std::vector<SubInfoWCount> sublist;
  std::vector<Variable *> subelm;
  SubInfoWCount siwc;
  try {
    SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
    for (const SymbolNameSpace::iterator &it : *ht) {
      siwc.v = static_cast<Variable *> SNSitToSymbol(it);
      siwc.count = siwc.v->SubscriptCountVars(subelm);
      if (siwc.count > 0) {
        sublist.push_back(siwc);
      }
    }
    mSymbolNameSpace.ConfirmAllAllocations();
  } catch (...) {
    mSymbolNameSpace.DeleteAllUnconfirmedAllocations();
    return false;
  }
  return true;
}

bool Model::ValidatePlaceholderVars(void) {
  try {
    SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
    for (const SymbolNameSpace::iterator &it : *ht) {
      // log("Checking placeholders out %s\n",SNSitToSymbol(it)->GetName().c_str()) ;
      SNSitToSymbol(it)->CheckPlaceholderVars(this);
    }
    mSymbolNameSpace.ConfirmAllAllocations();
  } catch (...) {
    mSymbolNameSpace.DeleteAllUnconfirmedAllocations();
    return false;
  }
  return true;
}

bool Model::SetupVariableStates(int pass /* 0 just assign, 1 determine sizes, 2 pass pointers for computation*/) {
  ContextInfo info(NULL);
  info.pSymbolNameSpace = &mSymbolNameSpace;
  SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
  info.iComputeType = pass;  // flag to skip empty or count sizes
  try {
    if (pass == 2) {
      if (iNLevel) {
        dLevel = new double[iNLevel];
        dRate = new double[iNLevel];
      } else
        dLevel = dRate = NULL;
      if (iNAux)
        dAux = new double[iNAux];
      else
        dAux = NULL;
      info.pBaseAux = info.pCurAux = dAux;
      info.pBaseRate = info.pCurRate = dRate;
      info.pBaseLevel = info.pCurLevel = dLevel;
    } else {
      info.pBaseAux = info.pCurAux = NULL;
      info.pBaseRate = info.pCurRate = NULL;
      info.pBaseLevel = info.pCurLevel = NULL;
      info.iComputeType = 0;
    }
    for (const SymbolNameSpace::iterator &it : *ht) {
      SNSitToSymbol(it)->SetupState(&info);
    }
    // placeholder vars also need state set up
    for (Variable *v : vUnamedVars) {
      v->SetupState(&info);
    }
    mSymbolNameSpace.ConfirmAllAllocations();
    if (pass == 1) {
      iNLevel = (info.pCurLevel - info.pBaseLevel);
      iNAux = (info.pCurAux - info.pBaseAux);
    }
  } catch (...) {
    // set all states to null - they will be deleted
    for (const SymbolNameSpace::iterator &it : *ht) {
      SNSitToSymbol(it)->SetupState(NULL);  // clear if setup
    }
    mSymbolNameSpace.DeleteAllUnconfirmedAllocations();
    if (dLevel) {
      delete dLevel;
      dLevel = NULL;
    }
    if (dRate) {
      delete dRate;
      dRate = NULL;
    }
    if (dAux) {
      delete dAux;
      dAux = NULL;
    }
    return false;
  }
  return true;
}

/* start anywhere - we just use the iterator order on the hash table -
   and get every variable computed - this needs to be done for both
   active and initial value (potentially reinitial as well but that is
   left out for now).
   */
bool Model::OrderEquations(ContextInfo *info, bool tonly) {
  bool haserr = false;
  try {
    SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
    if (tonly) {
      Variable *v;
      v = static_cast<Variable *>(mSymbolNameSpace.Find("INITIAL TIME"));
      // note CheckComputed called with false for first otherwise these won't be
      // initialized
      if (!v || !v->CheckComputed(info, false))
        haserr = true;
      v = static_cast<Variable *>(mSymbolNameSpace.Find("TIME STEP"));
      if (!v || !v->CheckComputed(info, false))
        haserr = true;
    } else {
      for (const SymbolNameSpace::iterator &it : *ht) {
        // log("Looping to: %s\n",SNSitToSymbol(it)->GetName().c_str()) ;
        if (!SNSitToSymbol(it)->CheckComputed(info, true))
          haserr = true;  // continue looking for simultaneous even when false
      }
      for (Variable *v : vUnamedVars) {
        if (!v->CheckComputed(info, true))
          haserr = true;
      }
    }

    mSymbolNameSpace.ConfirmAllAllocations();
  } catch (...) {
    mSymbolNameSpace.DeleteAllUnconfirmedAllocations();
    return false;
  }
  return !haserr;
}

bool Model::AnalyzeEquations(void) {
  ContextInfo info(NULL);

  ClearCompEquations();  // will also reset comp flag and delete placeholder vars
  /* ValidatePlaceholderVars will create the variables required for functions that either
     have memory are are time dependent (and therefore not subject to change within
     a DT as used in all but Euler integration */
  if (!ValidatePlaceholderVars())
    return false;
  /* SetupVariableStates will create states based on the variable equation types
     including subscript states needed to organize subscripts */
  if (!SetupVariableStates(0))
    return false;
  if (!OrganizeSubscripts())
    return false;
  if (!SetupVariableStates(1)) /* get size count */
    return false;
  if (!SetupVariableStates(2)) /* allocate and assign computed value locations */
    return false;
  // three passes on ordering equations first for initial
  // then for active - but putting out only those equations
  // that are not clearly unchanging - then again for
  // unchanging which are equations not put out in step
  // 2 then during execution 1 and 3 are computed once
  // and 2 is cycled through for integration
  //

  // before the passes initialize time, then dt
  log("\nInitial time \n");
  info.iComputeType = CF_initial;
  info.pEquations = &vInitialTimeComps;
  if (!OrderEquations(&info, true))
    return false;

  log("\nInitial equations \n");
  info.iComputeType = CF_initial;
  info.pEquations = &vInitialComps;
  if (!OrderEquations(&info, false))
    return false;
  log("\n\nActive equations \n");
  info.iComputeType = CF_active;
  info.pEquations = &vActiveComps;
  if (!OrderEquations(&info, false))
    return false;
  log("\n\nUnchanging equations \n");
  info.iComputeType = CF_unchanging;
  info.pEquations = &vUnchangingComps;
  if (!OrderEquations(&info, false))
    return false;
  log("\n\nRate equations \n");
  info.iComputeType = CF_rate;
  info.pEquations = &vRateComps;
  if (!OrderEquations(&info, false))
    return false;
  return true;
}

bool Model::Simulate(void) {
  ContextInfo info(NULL);
  try {
    double t, s, e, dt;
    int i, n;
    Variable *time = static_cast<Variable *>(mSymbolNameSpace.Find("Time"));
    Variable *start = static_cast<Variable *>(mSymbolNameSpace.Find("INITIAL TIME"));
    Variable *end = static_cast<Variable *>(mSymbolNameSpace.Find("FINAL TIME"));
    Variable *step = static_cast<Variable *>(mSymbolNameSpace.Find("TIME STEP"));
    n = iNLevel;

    info.iComputeType = CF_initial;
    for (Equation *e : vInitialTimeComps) {
      e->Execute(&info);
      // log("%s = %g\n",e->GetVariable()->GetName().c_str(),e->GetVariable()->Eval(&info)) ;
    }
    if (start)
      s = start->Eval(&info);
    else
      s = 0;
    if (step)
      dt = step->Eval(&info);
    else
      dt = 1;
    info.dTime = s;
    info.dDT = dt;
    for (Equation *e : vInitialComps) {
      e->Execute(&info);
      // log("%s = %g\n",e->GetVariable()->GetName().c_str(),e->GetVariable()->Eval(&info)) ;
    }
    // now the active equations
    info.iComputeType = CF_active;
    // first the unchanging variables
    // log("\n Unchanging\n") ;
    for (Equation *e : vUnchangingComps) {
      e->Execute(&info);
      // log("%s = %g\n",e->GetVariable()->GetName().c_str(),e->GetVariable()->Eval(&info)) ;
    }

    // now over time
    if (end)
      e = end->Eval(&info);
    else
      e = 100;
    log("Time");
    for (Equation *e : vActiveComps) {
      log("\t%s", e->GetVariable()->GetName().c_str());
    };
    for (Equation *e : vRateComps) {
      log("\t%s", e->GetVariable()->GetName().c_str());
    }
    log("\n");

    for (t = s; t <= e; t += dt) {
      info.dTime = t;
      // log("\n\nAt time %g\n",t) ;
      if (time)
        time->SetActiveValue(0, t);
      log("%g", t);
      for (Equation *e : vActiveComps) {
        e->Execute(&info);
        log("\t%g", e->GetVariable()->Eval(&info));
      }
      for (Equation *e : vRateComps) {
        e->Execute(&info);
        log("\t%g", e->GetVariable()->Eval(&info));
      }
      log("\n");
      if (step)
        info.dDT = dt = step->Eval(&info);
      // update states
      for (i = 0; i < n; i++) {
        dLevel[i] += dt * this->dRate[i];
      }
      if (end)
        e = end->Eval(&info);
    }
  } catch (...) {
    log("Error of some sort");
    return false;
  }
  return true;
}

bool Model::OutputComputable(bool wantshort) {
  ContextInfo info(NULL);
  try {
    if (wantshort)
      GenerateShortNames();
    else
      GenerateCanonicalNames();
    info.iComputeType = CF_initial;
    log("------------- initial time -----------------\n");
    for (Equation *e : vInitialTimeComps) {
      e->OutputComputable(&info);
    }
    log("------------- initialization -----------------\n");
    for (Equation *e : vInitialComps) {
      e->OutputComputable(&info);
    }
    info.iComputeType = CF_active;
    log("------------- Unchanging -----------------\n");
    info.iComputeType = CF_active;
    for (Equation *e : vUnchangingComps) {
      e->OutputComputable(&info);
    }
    log("------------- active -----------------\n");
    for (Equation *e : vActiveComps) {
      e->OutputComputable(&info);
    }
    info.iComputeType = CF_rate;
    log("------------- rates -----------------\n");
    for (Equation *e : vRateComps) {
      e->OutputComputable(&info);
    }
  } catch (...) {
    log("Error of some sort");
    return false;
  }
  return true;
}

bool Model::MarkVariableTypes(SymbolNameSpace *ns) {
  try {
    SymbolNameSpace::HashTable *ht;
    if (ns)
      ht = ns->GetHashTable();
    else {
      ht = mSymbolNameSpace.GetHashTable();
      ns = &mSymbolNameSpace;
    }
    std::vector<Variable *> vars;
    for (const SymbolNameSpace::iterator &it : *ht) {
      Symbol *sym = SNSitToSymbol(it);

      if (sym->isType() == Symtype_Variable)
        vars.push_back(static_cast<Variable *>(sym));
    }
    //
    for (Variable *var : vars) {
      var->PurgeAFOEq();
      var->MarkTypes(ns);  // may change number of entries so can't be in above loop
    }
    // repeat this for flows after all stocks marked
    for (Variable *var : vars) {
      var->MarkStockFlows(ns);  // may change number of entries so can't be in above loop
    }
    // don't do this - we have broken the allocation setup mSymbolNameSpace.ConfirmAllAllocations();
  } catch (...) {
    ns->DeleteAllUnconfirmedAllocations();
    return false;
  }

#ifdef dodump
  // - dump eveything - mostly just to see how the translation is going
  ContextInfo info;

  SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
  for (const SymbolNameSpace::iterator &it : *ht) {
    Symbol *sym = SNSitToSymbol(it);

    if (sym->isType() == Symtype_Variable) {
      Variable *var = static_cast<Variable *>(sym);
      VariableContent *content = var->Content();
      if (content) {  // array elements don't have
        for (Equation *eq : content->GetAllEquations()) {
          eq->OutputComputable(&info);
        }
      }
    }
  }
#endif

  return true;
}

void Model::AdjustGroupNames() {
  // every group name must be unique and can't be in the global name space
  int n = vGroups.size();
  for (int i = 0; i < n; i++) {
    std::string name = vGroups[i]->sName;
    // test it
    bool good;
    do {
      good = mSymbolNameSpace.Find(name) == NULL;
      if (good) {
        std::string *lname = SymbolNameSpace::ToLowerSpace(name);
        for (int j = 0; good && j < i; j++) {
          std::string *tlname = SymbolNameSpace::ToLowerSpace(vGroups[j]->sName);
          if (*tlname == *lname)
            good = false;
          delete tlname;
        }
        delete lname;
      }
      if (!good)
        name += " 1";
    } while (!good);
    vGroups[i]->sName = name;
  }
}

void Model::CheckGhostOwners() {
  // now everything is defined (and only defined once) - we need to make sure there are no missing connectors
  for (View *view : vViews) {
    view->CheckGhostOwners();
  }
}

void Model::AttachStragglers() {
  SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
  std::vector<Variable *> vars;
  for (const SymbolNameSpace::iterator &it : *ht) {
    Symbol *sym = SNSitToSymbol(it);

    if (sym->isType() == Symtype_Variable)
      vars.push_back(static_cast<Variable *>(sym));
  }
  // first try - anything that is not defined somewhere see if a ghost appears somewhere
  // and change that to the definition
  for (Variable *var : vars) {
    if (!var->GetView()) {
      for (View *view : vViews) {
        if (view->UpgradeGhost(var))
          break;
      }
    }
  }
  // now try undefined flows - attach to associated stocks
  for (Variable *var : vars) {
    if (!var->GetView() && var->VariableType() == XMILE_Type_FLOW) {
      // we don't know flows uses so just look at all stocks
      Variable *upstream = NULL;
      Variable *downstream = NULL;
      for (Variable *stock : vars) {
        if (stock->VariableType() == XMILE_Type_STOCK) {
          for (Variable *in : stock->Inflows()) {
            if (in == var) {
              downstream = stock;
              break;
            }
          }
          for (Variable *out : stock->Outflows()) {
            if (out == var) {
              upstream = stock;
              break;
            }
          }
          if (upstream && downstream)
            break;
        }
      }
      if (upstream && upstream->GetView()) {
        upstream->GetView()->AddFlowDefinition(var, upstream, downstream);
      } else if (downstream && downstream->GetView()) {
        downstream->GetView()->AddFlowDefinition(var, upstream, downstream);
      }
    }
  }
  // next pass look for inputs - if there are any put the var next to them
  // next pass look for otuputs - if there are any put the var next to them
  // finally dump everything remaining at 40,40 on the first view
  if (!vViews.empty()) {
    View *dump_view = vViews[0];
    for (Variable *var : vars) {
      if (!var->GetView() && var->VariableType() != XMILE_Type_ARRAY && var->VariableType() != XMILE_Type_ARRAY_ELM &&
          var->VariableType() != XMILE_Type_UNKNOWN)
        dump_view->AddVarDefinition(var, 200, 200);
    }
  }

  // now everything is defined (and only defined once) - we need to make sure there are no missing connectors
  for (View *view : vViews) {
    view->CheckLinksIn();
  }
}

void Model::LocalizeCrossViewFlows() {
  // Snapshot first: PreventFlowGhost registers new symbols, and GetVariables
  // walks the namespace's hash table.
  std::vector<Variable *> vars = GetVariables(nullptr);
  std::vector<Variable *> displaced;
  for (Variable *var : vars)
    var->LocalizeCrossViewFlows(&mSymbolNameSpace, displaced);
  if (displaced.empty())
    return;
  // A displaced flow may still be a same-view flow of some other stock, in
  // which case FLOW is its right type; only one no stock lists any more gets
  // its pre-promotion type back. Decided after every stock has been processed
  // so the answer does not depend on stock order.
  std::unordered_set<Variable *> still_listed;
  for (Variable *var : vars) {
    if (var->VariableType() != XMILE_Type_STOCK)
      continue;
    still_listed.insert(var->Inflows().begin(), var->Inflows().end());
    still_listed.insert(var->Outflows().begin(), var->Outflows().end());
  }
  for (Variable *var : displaced) {
    if (!still_listed.count(var))
      var->UndoFlowPromotion();
  }
}

void Model::MakeViewNamesUnique() {
  std::vector<View *> &views = Views();
  std::set<std::string> names;
  for (View *gview : views) {
    VensimView *view = static_cast<VensimView *>(gview);
    std::string name;
    // get rid of . and - in the name
    for (char c : view->Title()) {
      if (c == '.' || c == '-' || c == '+' || c == ',' || c == '/' || c == '*' || c == '^')
        c = ' ';
      if (c != ' ' || (!name.empty() && name.back() != ' '))
        name.push_back(c);
    }
    if (name.empty())
      name = "Module ";
    while (GetNameSpace()->Find(name) || names.find(name) != names.end()) {
      name += "1";  // not very original
    }
    names.insert(name);
    view->SetTitle(name);
  }
}

bool Model::RenameVariable(Variable *v, const std::string &newname) {
  assert(!newname.empty());
  if (mSymbolNameSpace.Find(newname)) {
    if (!newname.compare(v->GetName()))
      return true;  // nothing to do
    return false;
  }
  if (!mSymbolNameSpace.Remove(v))
    return false;
  v->SetName(newname);
  mSymbolNameSpace.Insert(v);
  return true;
}

void Model::GenerateCanonicalNames(void) {
  assert(0);
}

void Model::GenerateShortNames(void) {
  int i = 0;
  SymbolNameSpace::HashTable *ht = mSymbolNameSpace.GetHashTable();
  for (const SymbolNameSpace::iterator &it : *ht) {
    Variable *v = static_cast<Variable *> SNSitToSymbol(it);
    if (v->isType() == Symtype_Variable) {
      std::string s = "v" + std::to_string(i);
      i++;
      v->SetAlternateName(s);
      // v->SetAlternateName(v->GetName()) ;
    }
  }
  for (Variable *v : vUnamedVars) {
    std::string s = "v" + std::to_string(i);
    i++;
    v->SetAlternateName(s);
  }
}

double Model::GetConstanValue(const char *str, double val) {
  Symbol *s = mSymbolNameSpace.Find(str);
  if (s && s->isType() == Symtype_Variable) {
    Variable *v = static_cast<Variable *>(s);
    Equation *eq = v->GetEquation(0);
    if (eq) {
      Expression *exp = eq->GetExpression();
      if (exp && exp->GetType() == EXPTYPE_Number)
        val = exp->Eval(NULL);
    }
  }
  return val;
}

UnitExpression *Model::GetUnits(const char *str) {
  Symbol *s = mSymbolNameSpace.Find(str);
  if (s && s->isType() == Symtype_Variable) {
    Variable *v = static_cast<Variable *>(s);
    return v->Units();
  }
  return NULL;
}

void Model::SetUnwanted(const char *str, const char *defname) {
  Symbol *s = mSymbolNameSpace.Find(str);
  if (s && s->isType() == Symtype_Variable) {
    Variable *v = static_cast<Variable *>(s);
    v->SetUnwanted(true);
    v->SetAlternateName(defname);
  }
}

std::vector<Variable *> Model::GetVariables(SymbolNameSpace *ns) {
  std::vector<Variable *> vars;
  SymbolNameSpace::HashTable *ht;
  if (ns)
    ht = ns->GetHashTable();
  else
    ht = mSymbolNameSpace.GetHashTable();
  for (auto it = ht->begin(); it != ht->end(); it++) {
    Symbol *s = it->second;
    if (s->isType() == Symtype_Variable)
      vars.push_back(static_cast<Variable *>(s));
  }
  // Bucket order records the namespace's insertion history, not anything about
  // the model, and it reaches emitted output: the XMILE writer emits <variables>
  // in this order, and the passes that assign views and groups walk it too. That
  // made XMILE -> XMILE non-idempotent -- each conversion re-inserted the symbols
  // in the previous document's order, which permuted the next one, forever. Sort
  // here rather than at each writer's call site so no future caller can
  // reintroduce the same defect -- and so exactly one place owns the invariant:
  // the .mdl writer's own by-name sorts over this result were the same compare
  // applied twice, with two comments each claiming to be why the output is
  // stable, and they are gone. Name order is total: names are unique within a
  // namespace (see SymbolNameLess), and the empty name
  // Model::AddUnnamedVariable uses never enters the hash table at all --
  // Symbol::Symbol skips Insert for it.
  std::sort(vars.begin(), vars.end(), SymbolNameLess());
  return vars;
}

std::string Model::PrintXMILE(bool isCompact, std::vector<std::string> &errs, double xscale, double yscale) {
  if (!vUnresolvedWildcards.empty()) {
    errs.insert(errs.end(), vUnresolvedWildcards.begin(), vUnresolvedWildcards.end());
    return "";
  }
  XMILEGenerator generator(this, xscale, yscale, bFromDyanmo);
  // See bFromXmile (Model.h): the module decomposition emits sibling <model>
  // elements, which this project's own XMILE reader rejects, so an XMILE-sourced
  // model takes the single-<model> sector path whatever the caller asked for.
  return generator.Print(isCompact, errs, bAsSectors || bFromXmile);
}

std::string Model::PrintMDL(std::vector<std::string> &errs) {
  if (!vUnresolvedWildcards.empty()) {
    errs.insert(errs.end(), vUnresolvedWildcards.begin(), vUnresolvedWildcards.end());
    return "";
  }
  MDLGenerator generator(this);
  return generator.Print();
}
