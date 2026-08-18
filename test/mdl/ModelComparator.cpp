#include "ModelComparator.h"

#include <cmath>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "../../src/Function/Function.h"
#include "../../src/Mdl/MDLGenerator.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Equation.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/ExpressionList.h"
#include "../../src/Symbol/LeftHandSide.h"
#include "../../src/Symbol/SymbolList.h"
#include "../../src/Symbol/UnitExpression.h"
#include "../../src/Vensim/VensimView.h"

namespace {

// A symbol name reduced to its logical identity for comparison. Vensim quotes
// are escaping, not part of the name: `"a b"` and `a b` denote the same
// variable, and the writer legitimately changes a name's quoting across a round
// trip (it must quote a name like `TIME STEP$` whose trailing `$` cannot survive
// bare, even though the reader stored that name unquoted from a CLEARN macro
// body). Stripping a single surrounding quote pair from both sides before
// comparing -- the same strip FormatMDLIdent itself performs -- lets the
// comparison see through that benign requoting without masking a real rename
// (the interiors must still match exactly, including any backslash escapes).
// Vensim's identifier equivalence (case-fold, '_' and whitespace runs are one
// space), the same canon MDLGenerator::IsControlVar compares under.
bool IsSimulationClock(const std::string &name) {
  std::string *canon = SymbolNameSpace::ToLowerSpace(name);
  bool is_clock = *canon == "time";
  delete canon;
  return is_clock;
}

std::string NormalizeName(const std::string &name) {
  if (name.size() >= 2 && name.front() == '"' && name.back() == '"')
    return name.substr(1, name.size() - 2);
  return name;
}

// Index a list of user-facing variables by name. Variables that the reader
// marked Unwanted (synthesized helpers that should not round-trip) and array
// elements (which are not standalone definitions) are excluded so the
// comparison reflects what a writer would actually emit. Keying by name makes
// the result independent of GetVariables' hash-table iteration order.
//
// `drop_controls` excludes the four sim-control variables: in the MAIN model
// they are compared as sim specs (CompareSimSpecs), not as ordinary variables,
// because the writer always materializes the four of them into its .Control
// group, so a model that originally left them implicit gains explicit ones
// across the round trip; comparing them here would flag that expected
// materialization as a spurious "variable only in b". A MACRO namespace has no
// .Control group and the writer never materializes controls into it, so when
// comparing a macro body the controls (if a body var happened to share such a
// name) are ordinary variables and must NOT be dropped.
//
// The simulation clock is dropped unconditionally. Both readers register a
// "Time" Variable to resolve references to it (never a definition -- it holds
// no equation), and neither writer emits one; XMILEGenerator::generateSimSpecs
// additionally marks it Unwanted on the model it prints, so a printed model and
// its re-parse disagree on that flag alone. Filtering by Unwanted above would
// then report the re-parse's clock as a variable "only in b".
std::map<std::string, Variable *> IndexVariables(const std::vector<Variable *> &vars, bool drop_controls) {
  std::map<std::string, Variable *> by_name;
  for (Variable *var : vars) {
    if (var->Unwanted())
      continue;
    if (var->VariableType() == XMILE_Type_ARRAY_ELM)
      continue;
    if (drop_controls && MDLGenerator::IsControlVar(var->GetName()))
      continue;
    if (IsSimulationClock(var->GetName()))
      continue;
    by_name[NormalizeName(var->GetName())] = var;
  }
  return by_name;
}

// The units text a writer would emit: the parsed UnitExpression if present,
// otherwise the raw units string (mirrors XMILEGenerator's units logic).
std::string UnitsText(Variable *var) {
  UnitExpression *un = var->Units();
  if (un)
    return un->GetEquationString();
  return var->GetUnitsString();
}

// The ordered element names of an array/subrange definition, expanded the same
// way XMILEGenerator expands dimension definitions.
std::vector<std::string> DimensionElements(Variable *var) {
  std::vector<std::string> names;
  Equation *eq = var->GetEquation(0);
  if (!eq)
    return names;
  Expression *exp = eq->GetExpression();
  if (!exp || exp->GetType() != EXPTYPE_Symlist)
    return names;
  SymbolList *symlist = static_cast<ExpressionSymbolList *>(exp)->SymList();
  if (!symlist)
    return names;
  int n = symlist->Length();
  for (int i = 0; i < n; i++) {
    const SymbolList::SymbolListEntry &elm = (*symlist)[i];
    if (elm.eType == SymbolList::EntryType_SYMBOL) {
      std::vector<Symbol *> expanded;
      Equation::GetSubscriptElements(expanded, elm.u.pSymbol);
      for (Symbol *s : expanded)
        names.push_back(s->GetName());
    }
  }
  return names;
}

bool ValuesDiffer(double x, double y) {
  // Sim-spec constants are parsed from text; an exact match is expected, but a
  // tiny epsilon guards against representation noise in derived values.
  return std::fabs(x - y) > 1e-9;
}

// Follow ExpressionParen -> child until a non-paren node. Identical to the
// writer's walker Unwrap (src/Mdl/MDLGenerator.cpp): the writer derives all
// parentheses from operator precedence and drops the parser's paren nodes, so a
// structural compare must do the same or `(a)` would spuriously differ from `a`.
// A paren node is an ExpressionOperator2 with an empty operator and a "(" before.
Expression *Unwrap(Expression *e) {
  while (e && e->GetType() == EXPTYPE_Operator && e->GetBefore() && std::string(e->GetBefore()) == "(") {
    e = e->GetArg(0);
  }
  return e;
}

// Two doubles are equal within a relative tolerance, with Vensim's missing-data
// sentinel (-1e38, rendered as :NA:) matched exactly so a real value can never
// be mistaken for :NA: or vice versa.
bool NumbersEqual(double x, double y) {
  bool x_na = (x == -1e38);
  bool y_na = (y == -1e38);
  if (x_na || y_na)
    return x_na && y_na;
  double scale = std::max(1.0, std::max(std::fabs(x), std::fabs(y)));
  return std::fabs(x - y) <= 1e-12 * scale;
}

// Equal-length, element-wise tolerant comparison of two value vectors (used for
// lookup/table x and y points and for arrayed constant data).
bool ValueVectorsEqual(const std::vector<double> &a, const std::vector<double> &b) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); i++) {
    if (!NumbersEqual(a[i], b[i]))
      return false;
  }
  return true;
}

bool TablesEqual(ExpressionTable *a, ExpressionTable *b) {
  if (!a || !b)
    return a == b;
  return ValueVectorsEqual(*a->GetXVals(), *b->GetXVals()) && ValueVectorsEqual(*a->GetYVals(), *b->GetYVals());
}

// Compare two subscript symbol lists used in a variable reference: same length,
// same element names in order, and the same "bang" status per entry (a bang
// subscript `Dim!` is a distinct vector-iteration marker, so dropping it is a
// real difference the comparator must catch). Nested LIST entries recurse.
bool SymbolListsEqual(SymbolList *a, SymbolList *b) {
  if (!a || !b)
    return a == b;
  if (a->Length() != b->Length())
    return false;
  for (int i = 0; i < a->Length(); i++) {
    const SymbolList::SymbolListEntry &ea = (*a)[i];
    const SymbolList::SymbolListEntry &eb = (*b)[i];
    if (ea.eType != eb.eType)
      return false;
    if (ea.eType == SymbolList::EntryType_LIST) {
      if (!SymbolListsEqual(ea.u.pSymbolList, eb.u.pSymbolList))
        return false;
    } else if (NormalizeName(ea.u.pSymbol->GetName()) != NormalizeName(eb.u.pSymbol->GetName())) {
      return false;
    }
  }
  return true;
}

// Structural, paren-insensitive equality of two expression ASTs, using the
// Phase 2 getters rather than re-rendering through the writer. Re-rendering and
// string-comparing would let an emit bug that drops information pass vacuously,
// because the same walker drives both emit and the comparison; a getter-based
// compare is independent of the writer. Mirrors the walker's node dispatch.
bool ExpressionsEqual(Expression *a, Expression *b) {
  a = Unwrap(a);
  b = Unwrap(b);
  if (!a || !b)
    return a == b;
  if (a->GetType() != b->GetType())
    return false;

  switch (a->GetType()) {
  case EXPTYPE_Number:
    return NumbersEqual(static_cast<ExpressionNumber *>(a)->GetValue(), static_cast<ExpressionNumber *>(b)->GetValue());

  case EXPTYPE_Literal:
    return static_cast<ExpressionLiteral *>(a)->GetValue() == static_cast<ExpressionLiteral *>(b)->GetValue();

  case EXPTYPE_Variable: {
    ExpressionVariable *va = static_cast<ExpressionVariable *>(a);
    ExpressionVariable *vb = static_cast<ExpressionVariable *>(b);
    if (NormalizeName(va->GetVariable()->GetName()) != NormalizeName(vb->GetVariable()->GetName()))
      return false;
    return SymbolListsEqual(va->GetSubs(), vb->GetSubs());
  }

  case EXPTYPE_Operator: {
    // GetOperator() is non-empty for binary arithmetic and empty for unary
    // minus / paren; GetBefore() carries the unary "-" sign. Comparing both
    // distinguishes `a - b` (operator "-") from `-(a) ... ` etc., and a + from
    // a -. Operands live in GetArg(0)/GetArg(1); a unary node has only arg 0.
    const char *oa = a->GetOperator();
    const char *ob = b->GetOperator();
    if (std::string(oa ? oa : "") != std::string(ob ? ob : ""))
      return false;
    const char *ba = a->GetBefore();
    const char *bb = b->GetBefore();
    if (std::string(ba ? ba : "") != std::string(bb ? bb : ""))
      return false;
    return ExpressionsEqual(a->GetArg(0), b->GetArg(0)) && ExpressionsEqual(a->GetArg(1), b->GetArg(1));
  }

  case EXPTYPE_Function:
  case EXPTYPE_FunctionMemory: {
    ExpressionFunction *fa = static_cast<ExpressionFunction *>(a);
    ExpressionFunction *fb = static_cast<ExpressionFunction *>(b);
    Function *funa = fa->GetFunction();
    Function *funb = fb->GetFunction();
    ExpressionList *aa = fa->GetArgs();
    ExpressionList *ab = fb->GetArgs();
    int na = aa ? aa->Length() : 0;
    int nb = ab ? ab->Length() : 0;

    // INTEG (Vensim) and INTEGRATE (Dynamo) are the SAME integrator operation in
    // two formalisms: Vensim stores the stock as INTEG(net flow, init) while a
    // Dynamo level becomes a one-argument INTEGRATE(net flow) plus a separate `N`
    // init equation. When comparing a Dynamo model against the Vensim re-parse of
    // its emitted .mdl, the two stock nodes therefore differ in function name and
    // argument count even though they denote the identical stock. Treat any two
    // integrator nodes as comparable on their net flow (arg 0) alone; the initial
    // value lives in different places on the two sides and is compared explicitly
    // at the variable level (CompareStockInit), so this is not a hole -- a stock
    // whose init or net flow was mis-emitted is still caught.
    if (funa && funb && funa->IsIntegrator() && funb->IsIntegrator()) {
      Expression *neta = na > 0 ? aa->GetExp(0) : nullptr;
      Expression *netb = nb > 0 ? ab->GetExp(0) : nullptr;
      return ExpressionsEqual(neta, netb);
    }

    if (funa->GetName() != funb->GetName())
      return false;
    if (na != nb)
      return false;
    for (int i = 0; i < na; i++) {
      if (!ExpressionsEqual(aa->GetExp(i), ab->GetExp(i)))
        return false;
    }
    return true;
  }

  case EXPTYPE_Logical: {
    ExpressionLogical *la = static_cast<ExpressionLogical *>(a);
    ExpressionLogical *lb = static_cast<ExpressionLogical *>(b);
    if (la->LogicalOperator() != lb->LogicalOperator())
      return false;
    // GetLeft() is NULL for unary :NOT: (operand lives in GetRight()); the
    // recursion's NULL-safe base case handles that without a special case.
    return ExpressionsEqual(la->GetLeft(), lb->GetLeft()) && ExpressionsEqual(la->GetRight(), lb->GetRight());
  }

  case EXPTYPE_Lookup: {
    ExpressionLookup *la = static_cast<ExpressionLookup *>(a);
    ExpressionLookup *lb = static_cast<ExpressionLookup *>(b);
    // Same form: an inline-table WITH LOOKUP (GetTable() set) is structurally
    // distinct from a named-lookup call (GetLookupVariable() set).
    bool a_table = la->GetTable() != nullptr;
    bool b_table = lb->GetTable() != nullptr;
    if (a_table != b_table)
      return false;
    if (!ExpressionsEqual(la->GetInput(), lb->GetInput()))
      return false;
    if (a_table)
      return TablesEqual(la->GetTable(), lb->GetTable());
    // Named-lookup call: compare the lookup variable's name (it carries no
    // subscripts in this form, mirroring the writer's RenderTableLike).
    return NormalizeName(la->GetLookupVariable()->GetVariable()->GetName()) ==
           NormalizeName(lb->GetLookupVariable()->GetVariable()->GetName());
  }

  case EXPTYPE_Table:
    return TablesEqual(static_cast<ExpressionTable *>(a), static_cast<ExpressionTable *>(b));

  case EXPTYPE_NumberTable:
    return ValueVectorsEqual(static_cast<ExpressionNumberTable *>(a)->GetVals(),
                             static_cast<ExpressionNumberTable *>(b)->GetVals());

  case EXPTYPE_Symlist:
    return SymbolListsEqual(static_cast<ExpressionSymbolList *>(a)->SymList(),
                            static_cast<ExpressionSymbolList *>(b)->SymList());

  default:
    // The switch above covers every concrete EXPTYPE: Number, Literal, Variable,
    // Operator (which also carries parenthesized nodes -- GetBefore() == "("),
    // Function, FunctionMemory, Logical, Lookup, Table, NumberTable, and Symlist.
    // The only remaining enumerator is EXPTYPE_None, which is the abstract base
    // Expression::GetType() default; no instantiable node returns it (the base
    // class is abstract via its pure-virtual destructor), so this case is
    // unreachable today. Return false rather than true so that a node kind added
    // in the future without a matching case here is caught as a round-trip diff
    // instead of being silently accepted as equal.
    return false;
  }
}

// The ordered subscript element-names of an equation's left-hand side. This is
// the signature that distinguishes the equations of an arrayed variable from
// each other: a scalar has the empty signature, an apply-to-all has its
// dimension name, and a per-element equation has its element name(s). Matching
// equations between the two models by this signature lets the comparison line
// up the right pair before comparing right-hand sides.
std::vector<std::string> LhsSignature(Equation *eq) {
  std::vector<std::string> sig;
  SymbolList *subs = eq->GetLeft()->GetSubs();
  if (!subs)
    return sig;
  for (int i = 0; i < subs->Length(); i++) {
    const SymbolList::SymbolListEntry &e = (*subs)[i];
    if (e.eType == SymbolList::EntryType_LIST)
      sig.push_back("<list>");  // nested LHS lists are rare; mark them distinctly
    else
      sig.push_back(NormalizeName(e.u.pSymbol->GetName()));
  }
  return sig;
}

// Compare a variable's equation set across the two models: every equation in a
// must have a same-signature counterpart in b with an equivalent RHS, and vice
// versa. Diffs are reported against `name`.
void CompareEquations(const std::string &name, Variable *va, Variable *vb, std::vector<std::string> &diffs) {
  std::vector<Equation *> ea = va->GetAllEquations();
  std::vector<Equation *> eb = vb->GetAllEquations();
  if (ea.size() != eb.size()) {
    diffs.push_back("equation count differs for " + name);
    return;  // a count mismatch makes per-signature matching ambiguous
  }

  // Track which b-equations have been consumed so a duplicate signature in a
  // cannot match the same b-equation twice. Signatures are immutable during
  // the match, so compute each b-signature once rather than once per (a, b)
  // pair -- arrayed variables can carry one equation per subscript element.
  std::vector<bool> matched(eb.size(), false);
  std::vector<std::vector<std::string>> sig_b(eb.size());
  for (size_t j = 0; j < eb.size(); j++)
    sig_b[j] = LhsSignature(eb[j]);
  for (Equation *qa : ea) {
    std::vector<std::string> sig_a = LhsSignature(qa);
    bool found = false;
    for (size_t j = 0; j < eb.size(); j++) {
      if (matched[j])
        continue;
      if (sig_b[j] != sig_a)
        continue;
      matched[j] = true;
      found = true;
      if (!ExpressionsEqual(qa->GetExpression(), eb[j]->GetExpression()))
        diffs.push_back("equation RHS differs for " + name);
      break;
    }
    if (!found)
      diffs.push_back("equation with matching subscripts missing for " + name);
  }
}

// Names of a stock's inflow and outflow variables. The stock's net flow is
// reconstructed by the writer from these lists, so equal SETS of flow names mean
// the INTEG structure round-trips. A set (not a vector) is used because the
// writer's ordering is not part of the model's meaning.
std::set<std::string> FlowNames(const std::vector<Variable *> &flows) {
  std::set<std::string> names;
  for (Variable *f : flows) {
    if (f)
      names.insert(NormalizeName(f->GetName()));
  }
  return names;
}

// The initial-value expression of a stock's i-th equation, sourced from whichever
// representation the reader produced:
//   - Vensim: INTEG(net flow, init) carries the init as the integrator's second
//     argument, and the stock has no separate init equations.
//   - Dynamo: a level becomes a one-argument INTEGRATE(net flow) plus a separate
//     `N` init equation; the i-th init equation supplies the i-th stock entry's
//     initial value (mirroring XMILEGenerator and MDLGenerator::EmitStockEntry).
// Returns nullptr when no initial value is present (a stock with no init at all).
// Because the integrator-node comparison (ExpressionsEqual) deliberately ignores
// arg 1, this is where a mis-emitted or dropped initial value is caught -- for
// BOTH formalisms, so the explicit check is no weaker than the old arg-1 compare.
Expression *StockInitExpression(Variable *stock, size_t eq_index) {
  std::vector<Equation *> eqs = stock->GetAllEquations();
  if (eq_index < eqs.size()) {
    Expression *rhs = eqs[eq_index]->GetExpression();
    if (rhs && (rhs->GetType() == EXPTYPE_Function || rhs->GetType() == EXPTYPE_FunctionMemory)) {
      ExpressionFunction *fn = static_cast<ExpressionFunction *>(rhs);
      ExpressionList *args = fn->GetArgs();
      if (fn->GetFunction() && fn->GetFunction()->IsIntegrator() && args && args->Length() >= 2)
        return args->GetExp(1);
    }
  }
  std::vector<Equation *> initEqs = stock->GetAllInitEquations();
  if (eq_index < initEqs.size())
    return initEqs[eq_index]->GetExpression();
  return nullptr;
}

// Compare the initial values of two stocks across their (possibly different)
// representations. Every stock equation's init must match its counterpart; a
// count mismatch is itself a difference. Reported against `name`.
void CompareStockInit(const std::string &name, Variable *va, Variable *vb, std::vector<std::string> &diffs) {
  size_t na = va->GetAllEquations().size();
  size_t nb = vb->GetAllEquations().size();
  if (na != nb)
    return;  // an equation-count mismatch is already reported by CompareEquations
  for (size_t i = 0; i < na; i++) {
    Expression *ia = StockInitExpression(va, i);
    Expression *ib = StockInitExpression(vb, i);
    if (!ExpressionsEqual(ia, ib)) {
      diffs.push_back("stock initial value differs for " + name);
      return;
    }
  }
}

// The subscript-mapping clause of a dimension definition, if present. A mapping
// (`Dim: ... -> MappedDim`) lives on the dimension equation's ExpressionSymbolList
// via Map(); a regression that dropped it would otherwise still re-parse to the
// same element SET and pass unnoticed, so it is compared structurally here.
SymbolList *DimensionMap(Variable *var) {
  Equation *eq = var->GetEquation(0);
  if (!eq)
    return nullptr;
  Expression *exp = eq->GetExpression();
  if (!exp || exp->GetType() != EXPTYPE_Symlist)
    return nullptr;
  return static_cast<ExpressionSymbolList *>(exp)->Map();
}

// Compare the mapping clauses of two dimension definitions: both absent, both
// present with equal element lists, and (for the "(Range: ...)" subdimension
// form) the same MapRange symbol.
bool DimensionMapsEqual(Variable *va, Variable *vb) {
  SymbolList *ma = DimensionMap(va);
  SymbolList *mb = DimensionMap(vb);
  if (!ma || !mb)
    return ma == mb;
  if (ma->IsMapList() != mb->IsMapList())
    return false;
  if (ma->IsMapList()) {
    Symbol *ra = ma->MapRange();
    Symbol *rb = mb->MapRange();
    if ((ra == nullptr) != (rb == nullptr))
      return false;
    if (ra && NormalizeName(ra->GetName()) != NormalizeName(rb->GetName()))
      return false;
  }
  return SymbolListsEqual(ma, mb);
}

// A comment reduced to its comparable content. The writer applies two benign,
// content-preserving normalizations that a byte-for-byte compare would misread
// as a difference, so both sides are normalized the same way before comparing:
//
//   1. Leading separator whitespace: the Vensim lexer (VensimLex::GetComment)
//      captures the whitespace between the second '~' and the comment text as
//      part of the stored comment; the writer (MDLGenerator::UnitsCommentTrailer)
//      drops that leading whitespace and re-supplies a fixed "\t" separator, so a
//      round-tripped comment loses the original leading run.
//   2. Line endings: MDLGenerator::Print converts the whole document to CRLF, so
//      a multi-line comment whose internal newlines were LF in the source gains a
//      '\r' before each '\n' on re-parse. Stripping every '\r' compares the
//      comment modulo line-ending convention, which is not model content.
//
// Both normalizations preserve the comment's text exactly; only the writer's own
// formatting (separator, line endings) is canonicalized away. This mirrors the
// CommentContent helper the round-trip tests use.
std::string NormalizeComment(const std::string &comment) {
  std::string out;
  out.reserve(comment.size());
  for (char c : comment) {
    if (c != '\r')
      out += c;
  }
  size_t start = out.find_first_not_of(" \t\n");
  if (start == std::string::npos)
    return std::string();
  return out.substr(start);
}

// Compare two pre-indexed variable sets: presence on both sides, then per
// matched variable the type, units, comment, subscript count, dimension
// elements/mapping (for arrays), stock flow linkage, and the stored equation
// RHS ASTs. `context` prefixes every diff so a macro-body diff is
// distinguishable from a main-model diff. Factored out of CompareVariables so
// the identical logic drives both the main model and each macro's local
// namespace (the body of a :MACRO: block), rather than being duplicated.
void CompareVariableSets(const std::map<std::string, Variable *> &av, const std::map<std::string, Variable *> &bv,
                         const std::string &context, std::vector<std::string> &diffs) {
  for (const auto &kv : av) {
    if (bv.find(kv.first) == bv.end())
      diffs.push_back(context + "variable only in a: " + kv.first);
  }
  for (const auto &kv : bv) {
    if (av.find(kv.first) == av.end())
      diffs.push_back(context + "variable only in b: " + kv.first);
  }

  for (const auto &kv : av) {
    auto it = bv.find(kv.first);
    if (it == bv.end())
      continue;
    Variable *va = kv.second;
    Variable *vb = it->second;
    // Prefix every diff with the comparison context (empty for the main model,
    // e.g. "macro EXPRESSION MACRO: " for a macro body) so the source of a diff
    // is unambiguous and so a macro-body variable can never silently alias a
    // main-model variable of the same name.
    const std::string name = context + kv.first;

    if (va->VariableType() != vb->VariableType())
      diffs.push_back("variable type differs for " + name);

    if (UnitsText(va) != UnitsText(vb))
      diffs.push_back("units differ for " + name);

    if (NormalizeComment(va->Comment()) != NormalizeComment(vb->Comment()))
      diffs.push_back("comment differs for " + name);

    std::vector<Variable *> ea, eb;
    if (va->SubscriptCountVars(ea) != vb->SubscriptCountVars(eb))
      diffs.push_back("subscript count differs for " + name);

    if (va->VariableType() == XMILE_Type_ARRAY && vb->VariableType() == XMILE_Type_ARRAY) {
      if (DimensionElements(va) != DimensionElements(vb))
        diffs.push_back("dimension elements differ for " + name);
      // The element SET above does not cover the `-> MappedDim` mapping clause;
      // compare it explicitly so a dropped mapping is caught structurally rather
      // than passing because the expanded elements happen to re-parse the same.
      if (!DimensionMapsEqual(va, vb))
        diffs.push_back("dimension mapping differs for " + name);
    }

    // Stock comparison is INTENTIONALLY three complementary checks, none of
    // which alone is sufficient:
    //
    //   1. The INTEG NET FLOW (arg 0) is compared structurally by
    //      CompareEquations below (it runs for stocks too -- a stock is not an
    //      ARRAY). For integrator nodes ExpressionsEqual compares arg 0, the
    //      net-flow sub-expression (including the per-operand subscripts that an
    //      arrayed stock carries, e.g. `inflow[Dim] - outflow[Dim]`), and treats
    //      INTEG (Vensim) and INTEGRATE (Dynamo) as the same operation so a
    //      Dynamo->Vensim conversion does not flag a spurious node difference.
    //      This is the check that catches a writer that drops a flow's subscript
    //      or rewrites the net flow incorrectly.
    //
    //   2. The INITIAL VALUE is compared by CompareStockInit, which sources the
    //      init from whichever representation each side uses (INTEG arg 1 for
    //      Vensim, the separate `N` init equation for Dynamo). Because check (1)
    //      deliberately ignores the integrator's arg 1, this is where a dropped
    //      or mis-emitted initial value is caught -- for both formalisms, so it
    //      is no weaker than comparing arg 1 directly used to be.
    //
    //   3. The Inflows()/Outflows() NAME SETS are compared here. This is a
    //      complementary, deliberately redundant safety check: it is set-based
    //      and subscript-insensitive (flow names are `inflow`/`outflow`
    //      regardless of subscript), so it CANNOT catch a dropped subscript --
    //      that is check (1)'s job -- but it independently verifies the
    //      reader's stock/flow linkage (which flows MarkStockFlows attached to
    //      the stock) without going through the stored RHS, guarding against a
    //      regression where the RHS and the flow lists drift apart.
    //
    // Keeping all three is what makes the contract sound: (1) is the subscript-
    // aware net-flow truth, (2) is the formalism-independent init check, (3) is
    // the writer-independent linkage cross-check.
    if (va->VariableType() == XMILE_Type_STOCK && vb->VariableType() == XMILE_Type_STOCK) {
      if (FlowNames(va->Inflows()) != FlowNames(vb->Inflows()))
        diffs.push_back("stock inflows differ for " + name);
      if (FlowNames(va->Outflows()) != FlowNames(vb->Outflows()))
        diffs.push_back("stock outflows differ for " + name);
      CompareStockInit(name, va, vb, diffs);
    }

    // Structural, paren-insensitive comparison of the stored equation RHSs --
    // run for every non-ARRAY variable, INCLUDING stocks (whose INTEG RHS, per
    // the note above, is the subscript-aware truth). Dimension definitions carry
    // an ExpressionSymbolList that is already covered by the element/mapping
    // checks above, so skip the RHS compare for arrays to avoid a redundant (and
    // order-sensitive) Symlist comparison.
    if (va->VariableType() != XMILE_Type_ARRAY || vb->VariableType() != XMILE_Type_ARRAY)
      CompareEquations(name, va, vb, diffs);
  }
}

// Compare the two models' main-namespace variables. The four sim-control
// variables are dropped (compared as sim specs instead, see IndexVariables) and
// diffs carry no context prefix.
void CompareVariables(Model *a, Model *b, std::vector<std::string> &diffs) {
  std::map<std::string, Variable *> av = IndexVariables(a->GetVariables(nullptr), /*drop_controls=*/true);
  std::map<std::string, Variable *> bv = IndexVariables(b->GetVariables(nullptr), /*drop_controls=*/true);
  CompareVariableSets(av, bv, "", diffs);
}

// Render a macro's formal-parameter list to a comparable signature: one entry
// per arg, rendered through the SAME writer the emitter uses so a parameter that
// the writer would mangle (or reorder) is caught. Formal params parse as
// ExpressionVariable, so this is normally the bare identifier list.
std::vector<std::string> MacroArgSignature(MacroFunction *mf) {
  std::vector<std::string> sig;
  ExpressionList *args = mf->Args();
  int n = args ? args->Length() : 0;
  // A const MDLGenerator is not needed; RenderExpression is a non-static member
  // but does not mutate the model, so a throwaway generator renders each arg.
  MDLGenerator gen(nullptr);
  for (int i = 0; i < n; i++)
    sig.push_back(gen.RenderExpression(args->GetExp(i)));
  return sig;
}

// Compare the two models' Vensim macros. Macros are matched by name; a macro
// present on only one side is reported. For each matched pair the formal
// parameter list (count + each rendered arg) and the body -- the macro's local
// namespace -- are compared, the body reusing the exact same variable comparison
// (CompareVariableSets) the main model uses. Without this, a round trip that
// dropped a macro entirely, renamed it, changed its parameters, or corrupted its
// body would pass VACUOUSLY, since nothing else in Compare looks at macros.
void CompareMacros(Model *a, Model *b, std::vector<std::string> &diffs) {
  std::map<std::string, MacroFunction *> am, bm;
  for (MacroFunction *mf : a->MacroFunctions())
    am[mf->GetName()] = mf;
  for (MacroFunction *mf : b->MacroFunctions())
    bm[mf->GetName()] = mf;

  for (const auto &kv : am) {
    if (bm.find(kv.first) == bm.end())
      diffs.push_back("macro only in a: " + kv.first);
  }
  for (const auto &kv : bm) {
    if (am.find(kv.first) == am.end())
      diffs.push_back("macro only in b: " + kv.first);
  }

  for (const auto &kv : am) {
    auto it = bm.find(kv.first);
    if (it == bm.end())
      continue;
    MacroFunction *ma = kv.second;
    MacroFunction *mb = it->second;
    const std::string &name = kv.first;

    std::vector<std::string> sig_a = MacroArgSignature(ma);
    std::vector<std::string> sig_b = MacroArgSignature(mb);
    if (sig_a.size() != sig_b.size())
      diffs.push_back("macro arg count differs for " + name);
    else if (sig_a != sig_b)
      diffs.push_back("macro args differ for " + name);

    // The body is the macro's local namespace. Controls are NOT dropped here: a
    // macro has no .Control group, so a body var is an ordinary variable even if
    // it happened to share a control name (see IndexVariables).
    std::map<std::string, Variable *> bav = IndexVariables(a->GetVariables(ma->NameSpace()), /*drop_controls=*/false);
    std::map<std::string, Variable *> bbv = IndexVariables(b->GetVariables(mb->NameSpace()), /*drop_controls=*/false);
    CompareVariableSets(bav, bbv, "macro " + name + ": ", diffs);
  }
}

// The effective value a model uses for a control variable: its constant
// equation when present, else the model's own default. Comparing effective
// values (rather than a NaN sentinel for "absent") means a model that left a
// control implicit and the round-tripped model that materialized it at the same
// default value compare equal -- while SAVEPER = TIME STEP, which is non-constant
// in both, falls back to the same dt default on each side and also compares equal.
double EffectiveControlValue(Model *m, const std::string &name) {
  double fallback = m->dt();  // SAVEPER and TIME STEP default to dt
  if (name == "INITIAL TIME")
    fallback = m->initial_time();
  else if (name == "FINAL TIME")
    fallback = m->final_time();
  return m->GetConstanValue(name.c_str(), fallback);
}

// How many equations a control variable carries. A control is definitionally a
// single value, so anything above one is malformed -- and it is invisible to
// EffectiveControlValue, which reads only the first.
size_t ControlEquationCount(Model *m, const std::string &name) {
  Symbol *sym = m->GetNameSpace()->Find(name);
  if (!sym || sym->isType() != Symtype_Variable)
    return 0;
  return static_cast<Variable *>(sym)->GetAllEquations().size();
}

void CompareSimSpecs(Model *a, Model *b, std::vector<std::string> &diffs) {
  const char *kControls[] = {"INITIAL TIME", "FINAL TIME", "TIME STEP", "SAVEPER"};
  for (const char *name : kControls) {
    if (ValuesDiffer(EffectiveControlValue(a, name), EffectiveControlValue(b, name)))
      diffs.push_back(std::string("sim spec differs for ") + name);
    // CompareVariableSets checks equation counts, but it drops the controls (the
    // writer relocates them into .Control), so a duplicate control equation is
    // seen by nothing else. That gap let an XMILE declaring a control name as an
    // ordinary <aux> accumulate a second equation and emit the definition twice
    // -- invalid Vensim -- while the corpus stayed green, because both sides
    // agreed on the first equation. Flag the malformed state itself rather than
    // only an a-vs-b mismatch: the round trip can reproduce it faithfully on
    // both sides, which is exactly how it went unnoticed.
    //
    // Only a count ABOVE one is reported. A plain count MISMATCH is expected and
    // legitimate: a source model may leave a control implicit where the emitted
    // .mdl always materializes all four, which is precisely the case
    // EffectiveControlValue exists to smooth over.
    if (ControlEquationCount(a, name) > 1 || ControlEquationCount(b, name) > 1)
      diffs.push_back(std::string("control variable has multiple equations: ") + name);
  }
  if (a->IntegrationType() != b->IntegrationType())
    diffs.push_back("integration type differs");
}

// The non-control member names of a group. Control variables are excluded
// because the writer moves them out of any source group and into a dedicated
// .Control group, so their group membership is not preserved across a round trip
// (the sim specs themselves are compared by CompareSimSpecs instead).
std::set<std::string> MemberNames(ModelGroup *group) {
  std::set<std::string> names;
  for (Variable *v : group->vVariables) {
    if (v && !MDLGenerator::IsControlVar(v->GetName()))
      names.insert(NormalizeName(v->GetName()));
  }
  return names;
}

void CompareGroups(Model *a, Model *b, std::vector<std::string> &diffs) {
  // Index groups by sName, keeping only those with at least one non-control
  // member. A group whose entire membership is control vars (the writer's
  // .Control group, or a source banner that only held control vars) carries no
  // comparable content -- its members are compared as sim specs -- and the
  // writer always re-creates a .Control group regardless of the source's
  // grouping, so comparing such groups would spuriously flag a round trip.
  std::map<std::string, ModelGroup *> ag, bg;
  for (ModelGroup *g : a->Groups()) {
    if (!MemberNames(g).empty())
      ag[g->sName] = g;
  }
  for (ModelGroup *g : b->Groups()) {
    if (!MemberNames(g).empty())
      bg[g->sName] = g;
  }

  for (const auto &kv : ag) {
    if (bg.find(kv.first) == bg.end())
      diffs.push_back("group only in a: " + kv.first);
  }
  for (const auto &kv : bg) {
    if (ag.find(kv.first) == ag.end())
      diffs.push_back("group only in b: " + kv.first);
  }
  for (const auto &kv : ag) {
    auto it = bg.find(kv.first);
    if (it == bg.end())
      continue;
    if (MemberNames(kv.second) != MemberNames(it->second))
      diffs.push_back("group members differ for " + kv.first);
  }
}

// A length-prefixed encoding of the whole declaration, so name/eqn/alias
// boundaries stay distinguishable no matter which characters the fields carry
// (any plain separator could itself occur inside a unit name).
std::string UnitEquivStructure(const UnitEquiv &u) {
  auto part = [](const std::string &s) { return std::to_string(s.size()) + ":" + s; };
  std::string key = part(u.name) + part(u.eqn);
  for (const std::string &alias : u.aliases)
    key += part(alias);
  return key;
}

// Compare the models' unit-equivalence declarations. A multiset is used because
// the equivalences are an unordered collection -- two models that declare the
// same equivalences in a different order are equivalent -- but a duplicate
// declaration is a distinct fact, so cardinality must match (a plain set would
// silently accept losing one of two identical lines).
//
// Two levels, because the two formats do not carry the same information. The
// flattened .mdl payload ("$,Dollar,Dollars,$s") is the part every direction
// must preserve, so it is always compared. The full structure -- which field is
// the derived-unit equation and which are aliases -- only survives between two
// XMILE documents; Vensim's 22: line has no equation concept, so demanding it
// across an XMILE -> MDL conversion would flag an unavoidable property of the
// target format rather than a writer defect.
void CompareUnitEquivs(Model *a, Model *b, std::vector<std::string> &diffs) {
  std::multiset<std::string> ea, eb;
  for (const UnitEquiv &u : a->UnitEquivs())
    ea.insert(u.MdlPayload());
  for (const UnitEquiv &u : b->UnitEquivs())
    eb.insert(u.MdlPayload());
  if (ea != eb) {
    diffs.push_back("unit equivalences differ");
    return;
  }
  if (!a->FromXmile() || !b->FromXmile())
    return;

  std::multiset<std::string> sa, sb;
  for (const UnitEquiv &u : a->UnitEquivs())
    sa.insert(UnitEquivStructure(u));
  for (const UnitEquiv &u : b->UnitEquivs())
    sb.insert(UnitEquivStructure(u));
  if (sa != sb)
    diffs.push_back("unit equivalence structure differs (eqn vs alias)");
}

std::string ElementVarName(VensimViewElement *e) {
  Variable *v = e->GetVariable();
  return v ? NormalizeName(v->GetName()) : std::string();
}

// Return the effective length of a view's element vector: the index past the
// last non-NULL slot. The Vensim sketch parser grows the vector in +25 chunks
// and never trims, so the raw size carries a pad that depends on the order
// records were processed; the XMILE reader grows tightly. Comparing raw sizes
// would flag every Vensim-vs-XMILE round trip as "element count differs"
// merely because of differing trailing-NULL padding. Compare effective lengths
// instead so equivalent content compares equal regardless of padding.
static size_t EffectiveElementLen(const VensimViewElements &v) {
  for (size_t i = v.size(); i > 0; --i) {
    if (v[i - 1])
      return i;
  }
  return 0;
}

void CompareViewElements(int view_index, VensimView *va, VensimView *vb, std::vector<std::string> &diffs) {
  VensimViewElements &ea = va->Elements();
  VensimViewElements &eb = vb->Elements();
  std::ostringstream prefix;
  prefix << "view " << view_index;

  size_t la = EffectiveElementLen(ea);
  size_t lb = EffectiveElementLen(eb);
  if (la != lb) {
    diffs.push_back(prefix.str() + " element count differs");
    return;  // positional comparison below is only meaningful at equal length
  }

  for (size_t i = 0; i < la; i++) {
    VensimViewElement *xa = ea[i];
    VensimViewElement *xb = eb[i];
    if ((xa == nullptr) != (xb == nullptr)) {
      diffs.push_back(prefix.str() + " element nullness differs");
      continue;
    }
    if (!xa || !xb)
      continue;

    std::ostringstream ep;
    ep << prefix.str() << " element " << i;

    if (xa->Type() != xb->Type()) {
      diffs.push_back(ep.str() + " type differs");
      continue;  // remaining field comparisons assume matching element kinds
    }

    if (xa->X() != xb->X() || xa->Y() != xb->Y() || xa->Width() != xb->Width() || xa->Height() != xb->Height())
      diffs.push_back(ep.str() + " geometry differs");

    if (xa->Type() == VensimViewElement::ElementTypeCONNECTOR) {
      VensimConnectorElement *ca = static_cast<VensimConnectorElement *>(xa);
      VensimConnectorElement *cb = static_cast<VensimConnectorElement *>(xb);
      if (ca->From() != cb->From() || ca->To() != cb->To())
        diffs.push_back(ep.str() + " connector endpoints differ");
      if (ca->Polarity() != cb->Polarity())
        diffs.push_back(ep.str() + " connector polarity differs");
    }

    if (xa->Type() == VensimViewElement::ElementTypeVARIABLE) {
      if (ElementVarName(xa) != ElementVarName(xb))
        diffs.push_back(ep.str() + " variable reference differs");
      VensimVariableElement *vea = static_cast<VensimVariableElement *>(xa);
      VensimVariableElement *veb = static_cast<VensimVariableElement *>(xb);
      if (vea->Attached() != veb->Attached())
        diffs.push_back(ep.str() + " variable attached state differs");
      // Ghost (alias) state is part of the element's meaning: a ghost box is a
      // secondary reference to a variable defined elsewhere, not a primary
      // definition. The writer derives it from the record's bit0 (inverted), so a
      // dropped/inverted bit would silently turn an alias into a duplicate
      // definition; compare it explicitly. Ghost(nullptr, false) is the pure accessor
      // (passing a non-null set would mutate ghost-owner bookkeeping).
      if (vea->Ghost(nullptr, false) != veb->Ghost(nullptr, false))
        diffs.push_back(ep.str() + " variable ghost state differs");
    }
  }
}

// The element-bearing views of a model. Zero-element views are dropped because
// the writer always emits a minimal empty *View frame (so the settings section
// re-parses), so a model that had no sketch gains one empty view across the
// round trip -- "0 views" and "1 empty view" carry the same (no) geometry and
// must compare equal. All View instances the readers produce are VensimView,
// so the cast matches the rest of the codebase (Model.cpp, XMILEGenerator.cpp).
std::vector<VensimView *> NonEmptyViews(Model *m) {
  std::vector<VensimView *> views;
  for (View *v : m->Views()) {
    VensimView *vv = static_cast<VensimView *>(v);
    if (!vv->Elements().empty())
      views.push_back(vv);
  }
  return views;
}

void CompareViews(Model *a, Model *b, std::vector<std::string> &diffs) {
  std::vector<VensimView *> av = NonEmptyViews(a);
  std::vector<VensimView *> bv = NonEmptyViews(b);
  if (av.size() != bv.size()) {
    diffs.push_back("view count differs");
    return;
  }
  for (size_t i = 0; i < av.size(); i++)
    CompareViewElements(static_cast<int>(i), av[i], bv[i], diffs);
}

}  // namespace

std::vector<std::string> ModelComparator::Compare(Model *a, Model *b) {
  std::vector<std::string> diffs;
  if (!a || !b) {
    diffs.push_back("null model passed to ModelComparator::Compare");
    return diffs;
  }
  CompareVariables(a, b, diffs);
  CompareMacros(a, b, diffs);
  CompareSimSpecs(a, b, diffs);
  CompareGroups(a, b, diffs);
  CompareUnitEquivs(a, b, diffs);
  CompareViews(a, b, diffs);
  return diffs;
}
