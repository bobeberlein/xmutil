#include "XmileParseFunctions.h"

#include <string>
#include <vector>

#include "../Function/Function.h"
#include "../Symbol/Expression.h"
#include "../Symbol/ExpressionList.h"
#include "../Symbol/Parse.h"
#include "../Symbol/Symbol.h"
#include "../Symbol/SymbolList.h"
#include "../Symbol/SymbolNameSpace.h"
#include "../Symbol/Variable.h"
#include "../XMUtil.h"
// Imported for the VPTT_* operator codes ExpressionLogical expects on its
// `oper` parameter. The XMILE grammar emits its own XPTT_* token codes but
// reuses the Vensim codes inside ExpressionLogical so downstream walkers
// (writers, comparator) see bit-for-bit equivalent trees regardless of which
// front-end produced them. VYacc.tab.hpp declares `extern YYSTYPE vpyylval`
// at file scope (outside any ifdef), so we must define YYSTYPE before the
// include even though we only consume the enum constants.
#define YYSTYPE ParseUnion
#include "../Vensim/VYacc.tab.hpp"
#include "XmileEqLex.h"
#include "XmileFunctions.h"
#include "XmileReader.h"

namespace {

// The mathematical constant, at full double precision, for lowering XMILE's
// pi() / bare-pi to a numeric literal (Vensim has no PI builtin).
constexpr double kPi = 3.141592653589793;

SymbolNameSpace *Sns() {
  return XPObject ? XPObject->GetSymbolNameSpace() : nullptr;
}

// Push an error to the active reader's errs vector, defensively no-op when
// the reader isn't set up. Real parser invocations always set _currentErrs;
// this guard is a safety net for unexpected call sites during refactors.
void PushErr(const std::string &msg) {
  if (XPObject && XPObject->CurrentErrs())
    XPObject->CurrentErrs()->push_back(msg);
}

// Used whenever a reduction can't produce a meaningful Expression (e.g.
// unknown function name). Returning a 0-literal lets the parse continue so
// the user sees every collected error rather than just the first one.
Expression *ZeroPlaceholder() {
  return new ExpressionNumber(Sns(), 0.0);
}

// Dispatch a Function call to the same wrapper class the Vensim front-end
// uses: ExpressionFunction for memoryless, time-independent functions;
// ExpressionFunctionMemory for everything with internal state or a
// time-varying evaluation order. All four grammar shims that build a
// Function-call node go through here so the invariant is maintained in
// one place.
Expression *WrapFunction(SymbolNameSpace *sns, Function *f, ExpressionList *args) {
  if (f->IsMemoryless() && !f->IsTimeDependent())
    return new ExpressionFunction(sns, f, args);
  return new ExpressionFunctionMemory(sns, f, args);
}

// Resolve a Vensim-canonical function name to its registered Function*, or NULL
// if the name is absent or shadowed by a non-Function symbol. Used by the PULSE
// structural translation, which targets PULSE / PULSE TRAIN by their exact
// registered names rather than through the XMILE un-rename table.
Function *FindVensimFunction(SymbolNameSpace *sns, const char *name) {
  Symbol *sym = sns ? sns->Find(name) : nullptr;
  if (!sym || sym->isType() != Symtype_Function)
    return nullptr;
  return static_cast<Function *>(sym);
}

// A fresh ExpressionVariable referencing the control Variable that the XMILE
// bare keyword maps to ("dt" -> TIME STEP, "final_time" -> FINAL TIME). Each
// call yields an independent node: sharing one ExpressionVariable across two
// parents would double-free at namespace teardown.
Expression *ControlRef(const char *xmileKeyword) {
  SymbolNameSpace *sns = Sns();
  Variable *v = xmile::LookupBareKeyword(sns, xmileKeyword);
  if (!v)
    return nullptr;
  return new ExpressionVariable(sns, v, nullptr);
}

// Translate an XMILE pulse(volume, first[, interval]) call into its Vensim
// equivalent. XMILE PULSE is an *area* impulse: it injects `volume` units over
// one time step -- an instantaneous height of volume/DT sustained for one DT,
// optionally repeating every `interval`. Vensim PULSE(start, width) is a
// *unit-height* rectangle, so the name-only map (pulse -> PULSE) that the
// generic path would apply silently rescales the model by a factor of DT. The
// structural forms are:
//   pulse(v, first)           -> (v / TIME STEP) * PULSE(first, TIME STEP)
//   pulse(v, first, interval) -> (v / TIME STEP) *
//                                PULSE TRAIN(first, TIME STEP, interval, FINAL TIME)
// An interval literal <= 0 degrades to the single-pulse form (XMILE treats a
// non-positive interval as "do not repeat"). Returns NULL when a required
// function or control keyword is not registered, so the caller falls through to
// the generic path (which reports the missing registration).
//
// `args` is abandoned after its children are re-homed into the new tree, the
// same way the LOOKUP special-case in xpyy_call re-homes its operands: the empty
// container is an unconfirmed allocation that is never re-entered.
Expression *BuildXmilePulse(ExpressionList *args) {
  SymbolNameSpace *sns = Sns();
  const int n = args->Length();
  Expression *volume = args->GetExp(0);
  Expression *first = args->GetExp(1);
  Expression *interval = (n >= 3) ? args->GetExp(2) : nullptr;

  bool repeating = (interval != nullptr);
  if (interval && interval->GetType() == EXPTYPE_Number && static_cast<ExpressionNumber *>(interval)->GetValue() <= 0.0)
    repeating = false;

  Expression *scale = ControlRef("dt");  // denominator of the volume/DT height
  Expression *width = ControlRef("dt");  // the one-DT pulse width
  Function *pulseFn = repeating ? FindVensimFunction(sns, "PULSE TRAIN") : FindVensimFunction(sns, "PULSE");
  if (!scale || !width || !pulseFn)
    return nullptr;

  ExpressionList *callArgs = new ExpressionList(sns);
  callArgs->Append(first);
  callArgs->Append(width);
  if (repeating) {
    Expression *finalTime = ControlRef("final_time");
    if (!finalTime)
      return nullptr;
    callArgs->Append(interval);
    callArgs->Append(finalTime);
  }

  Expression *height = new ExpressionDivide(sns, volume, scale);
  return new ExpressionMultiply(sns, height, WrapFunction(sns, pulseFn, callArgs));
}

// For the builtins whose final argument XMILE lets a model omit, synthesize the
// missing trailing argument so the stored AST carries the full Vensim signature
// and both writers emit valid Vensim (a reduced-arity call would otherwise
// serialize to `.mdl` Vensim rejects on load). Runs after ReorderArgs, so the
// args are already in Vensim order, and only fires when the call is exactly one
// argument short of the Vensim arity -- the widened accept range (SetArgRange)
// is what lets such a call reach here.
//   delay(x, d)    -> DELAY FIXED(x, d, x)      initial defaults to the input
//   trend(x, t)    -> TREND(x, t, 0)
//   ramp(s, start) -> RAMP(s, start, FINAL TIME)
//   uniform(a, b)  -> RANDOM UNIFORM(a, b, 0)
void PadOptionalArgs(Function *f, ExpressionList *args) {
  if (!f || !args || args->Length() != f->NumberArgs() - 1)
    return;
  SymbolNameSpace *sns = Sns();
  const std::string &name = f->GetName();
  if (name == "DELAY FIXED") {
    // The initial value defaults to the input expression; it must be a fresh
    // deep copy, not the same node, or namespace teardown would double-free it.
    if (Expression *initial = args->GetExp(0) ? args->GetExp(0)->Clone(sns) : nullptr)
      args->Append(initial);
  } else if (name == "TREND" || name == "RANDOM UNIFORM") {
    args->Append(new ExpressionNumber(sns, 0.0));
  } else if (name == "RAMP") {
    if (Expression *finalTime = ControlRef("final_time"))
      args->Append(finalTime);
  }
}

}  // namespace

void xpyy_set_result(Expression *e) {
  if (XPObject)
    XPObject->SetLastParsedExpr(e);
}

void xpyyerror(char const *s) {
  if (!XPObject)
    return;
  std::string msg = s ? s : "(null bison error)";
  XmileEqLex *lex = XPObject->CurrentLex();
  if (lex)
    msg += " near \"" + lex->Snippet() + "\"";
  PushErr(msg);
}

Expression *xpyy_num(double n) {
  return new ExpressionNumber(Sns(), n);
}

Expression *xpyy_resolve_symbol(const char *name, SymbolList *subs) {
  if (!XPObject || !name) {
    delete subs;
    return ZeroPlaceholder();
  }
  const std::string s(name);
  // Bare pi (no parens, no subscripts) is the XMILE mathematical constant, like
  // pi(); lower it to a numeric literal before it could be materialized as a
  // phantom variable. Unless this document declares a variable of its own by
  // that name, in which case the declaration is the more specific claim and
  // the reference belongs to it -- see XmileReader::DeclaresPi.
  if (!subs && StringMatch(s, "pi") && !XPObject->DeclaresPi())
    return new ExpressionNumber(Sns(), kPi);
  Variable *var = xmile::LookupBareKeyword(Sns(), s);
  if (!var)
    var = XPObject->InsertVariable(s);
  if (!var) {
    PushErr("unresolvable identifier '" + s + "'");
    // The placeholder owns no subscript list; without this delete the list
    // would sit in the allocation table unowned, get marked GoodAlloc by
    // ConfirmAllAllocations, and never be reachable again.
    delete subs;
    return ZeroPlaceholder();
  }
  return new ExpressionVariable(Sns(), var, subs);
}

Expression *xpyy_binop(int op, Expression *l, Expression *r) {
  SymbolNameSpace *sns = Sns();
  switch (op) {
  case '+':
    return new ExpressionAdd(sns, l, r);
  case '-':
    return new ExpressionSubtract(sns, l, r);
  case '*':
    return new ExpressionMultiply(sns, l, r);
  case '/':
    return new ExpressionDivide(sns, l, r);
  case '^':
    return new ExpressionPower(sns, l, r);
  }
  PushErr("internal: unknown binary operator");
  return ZeroPlaceholder();
}

Expression *xpyy_paren(Expression *inner) {
  if (!inner)
    return ZeroPlaceholder();
  return new ExpressionParen(Sns(), inner, nullptr);
}

Expression *xpyy_unary(int op, Expression *e) {
  SymbolNameSpace *sns = Sns();
  if (op == '-') {
    // Fold the sign into a literal number when we can -- this is the same
    // shortcut VensimParse::OperatorExpression takes (VensimParse.cpp ~582),
    // so the resulting tree matches a literal "-1" parsed from MDL.
    if (e && e->GetType() == EXPTYPE_Number) {
      e->FlipSign();
      return e;
    }
    return new ExpressionUnaryMinus(sns, e, nullptr);
  }
  PushErr("internal: unknown unary operator");
  return ZeroPlaceholder();
}

Expression *xpyy_logical(int op, Expression *l, Expression *r) {
  return new ExpressionLogical(Sns(), l, r, op);
}

Expression *xpyy_logical_le(Expression *l, Expression *r) {
  return new ExpressionLogical(Sns(), l, r, VPTT_le);
}

Expression *xpyy_logical_ge(Expression *l, Expression *r) {
  return new ExpressionLogical(Sns(), l, r, VPTT_ge);
}

Expression *xpyy_logical_ne(Expression *l, Expression *r) {
  return new ExpressionLogical(Sns(), l, r, VPTT_ne);
}

Expression *xpyy_logical_and(Expression *l, Expression *r) {
  return new ExpressionLogical(Sns(), l, r, VPTT_and);
}

Expression *xpyy_logical_or(Expression *l, Expression *r) {
  return new ExpressionLogical(Sns(), l, r, VPTT_or);
}

Expression *xpyy_logical_unary(Expression *operand) {
  // Match the Vensim grammar's NOT shape: the operand lives in the right
  // slot and the left slot is NULL (VensimParse.cpp ~604).
  return new ExpressionLogical(Sns(), nullptr, operand, VPTT_not);
}

Expression *xpyy_safediv(Expression *l, Expression *r) {
  SymbolNameSpace *sns = Sns();
  ExpressionList *args = new ExpressionList(sns);
  args->Append(l);
  args->Append(r);
  Function *zidz = xmile::LookupFunction(sns, "safediv", 2);
  if (!zidz) {
    PushErr("ZIDZ function not registered (cannot lower '//' to a call)");
    return ZeroPlaceholder();
  }
  return WrapFunction(sns, zidz, args);
}

Expression *xpyy_function(const char *vensimName, Expression *a, Expression *b) {
  SymbolNameSpace *sns = Sns();
  ExpressionList *args = new ExpressionList(sns);
  args->Append(a);
  args->Append(b);
  Symbol *sym = sns ? sns->Find(vensimName ? vensimName : "") : nullptr;
  if (!sym || sym->isType() != Symtype_Function) {
    PushErr(std::string("function '") + (vensimName ? vensimName : "") + "' not registered");
    return ZeroPlaceholder();
  }
  Function *f = static_cast<Function *>(sym);
  return WrapFunction(sns, f, args);
}

Expression *xpyy_if(Expression *c, Expression *t, Expression *e) {
  SymbolNameSpace *sns = Sns();
  ExpressionList *args = new ExpressionList(sns);
  args->Append(c);
  args->Append(t);
  args->Append(e);
  Symbol *sym = sns ? sns->Find("IF THEN ELSE") : nullptr;
  if (!sym || sym->isType() != Symtype_Function) {
    PushErr("IF THEN ELSE function not registered");
    return ZeroPlaceholder();
  }
  Function *f = static_cast<Function *>(sym);
  return WrapFunction(sns, f, args);
}

Expression *xpyy_call(const char *xmileName, ExpressionList *args) {
  SymbolNameSpace *sns = Sns();
  const std::string s = xmileName ? xmileName : "";
  const int argCount = args ? args->Length() : 0;
  // The explicit "LOOKUP(table, input)" spelling is a named graphical-function
  // application, identical in meaning to the direct "table(input)" form some
  // producers emit. Lower both to an ExpressionLookup so a model round-trips to
  // one canonical form regardless of which spelling it was written in.
  if (StringMatch(s, "lookup") && argCount == 2) {
    if (ExpressionVariable *ev = dynamic_cast<ExpressionVariable *>(args->GetExp(0)))
      return new ExpressionLookup(sns, ev, args->GetExp(1));
  }
  // XMILE PULSE is an area impulse, not the unit-height rectangle Vensim PULSE
  // computes; translate it structurally so the converted model keeps its
  // magnitude. See BuildXmilePulse. On a missing-registration NULL we fall
  // through to the generic path, which reports the specific failure.
  if (StringMatch(s, "pulse") && (argCount == 2 || argCount == 3)) {
    if (Expression *p = BuildXmilePulse(args))
      return p;
  }
  // XMILE defines pi() (and the bare keyword pi) as the mathematical constant.
  // Vensim has no PI builtin, so lower it to a full-precision numeric literal at
  // read time; both writers format the double round-trip-exactly. Bare pi is
  // handled on the identifier path in xpyy_resolve_symbol, and both consult
  // DeclaresPi so a document that declares its own `pi` gets one answer rather
  // than the constant here and the variable there.
  if (StringMatch(s, "pi") && argCount == 0 && XPObject && !XPObject->DeclaresPi())
    return new ExpressionNumber(sns, kPi);
  Function *f = xmile::LookupFunction(sns, s, argCount);
  if (!f) {
    // Not a known function. A single-argument call whose callee names a model
    // variable is a graphical-function (lookup) application: `table(x)`
    // evaluates the lookup `table` at `x`. This mirrors the Vensim grammar's
    // `var '(' exprlist ')'` -> ExpressionLookup rule (VensimParse::
    // LookupExpression), which likewise only treats the one-argument form as a
    // lookup. The callee is resolved (or forward-declared) through the same
    // InsertVariable path an ordinary variable reference uses.
    if (XPObject && argCount == 1) {
      Variable *callee = xmile::LookupBareKeyword(sns, s);
      if (!callee)
        callee = XPObject->InsertVariable(s);
      if (callee) {
        ExpressionVariable *ev = new ExpressionVariable(sns, callee, nullptr);
        return new ExpressionLookup(sns, ev, args->GetExp(0));
      }
    }
    PushErr("unknown function '" + s + "'");
    return ZeroPlaceholder();
  }
  xmile::ReorderArgs(f->GetName(), args);
  // Synthesize any omitted optional trailing arg so the stored AST is full-arity
  // (delay/trend/ramp/uniform); this may grow args, so the range check below
  // uses the post-padding count.
  PadOptionalArgs(f, args);
  const int finalCount = args ? args->Length() : 0;
  // A negative NumberArgs() marks a variadic function; those bypass the count
  // check. Otherwise the accepted range is [MinNumberArgs, MaxNumberArgs] --
  // usually the single value NumberArgs(), but wider for builtins whose trailing
  // args XMILE may omit (initial value, random seed; see SetArgRange). A count
  // outside the range is a hard error: it would build a call Vensim rejects, so
  // failing the parse (unprefixed, per the eqn-diagnostics classification) is
  // better than a silently malformed conversion. The check runs AFTER
  // ReorderArgs, so the count is the Vensim-signature count, not the raw XMILE
  // order.
  const int lo = f->MinNumberArgs();
  const int hi = f->MaxNumberArgs();
  if (f->NumberArgs() >= 0 && (finalCount < lo || finalCount > hi)) {
    const std::string range = (lo == hi) ? std::to_string(lo) : (std::to_string(lo) + " to " + std::to_string(hi));
    PushErr("function '" + f->GetName() + "' expects " + range + " arg(s), got " + std::to_string(finalCount));
    return ZeroPlaceholder();
  }
  return WrapFunction(sns, f, args);
}

ExpressionList *xpyy_arglist(ExpressionList *prev, Expression *e) {
  if (!prev)
    prev = new ExpressionList(Sns());
  return prev->Append(e);
}

// Fold one sub_term's singleton SymbolList into the flat accumulator. Each
// sub_term reduction yields a one-entry SymbolList already carrying the correct
// entry kind (SYMBOL for a named dimension/element, BANG_SYMBOL for `*` and
// `*:Dim`); we hoist that single entry into `acc` so the finished reference
// subscript list is flat -- one entry per subscript position -- exactly the
// shape VensimParse::SymList produces and the shape both writers
// (MDLGenerator::RenderSubscripts and SymbolList::OutputComputable) consume.
// The historical list-of-lists shape rendered as empty `[]` in the XMILE
// writer (no EntryType_LIST branch) and crashed the MDL writer on `*`.
static void FoldSubTerm(SymbolList *acc, SymbolList *term) {
  if (!acc || !term || term->Length() != 1)
    return;
  const SymbolList::SymbolListEntry &e = (*term)[0];
  if (term->IsMapList())
    // A range subscript (lo:hi) carries a whole-list map range that a flat
    // symbol cannot represent; keep it as a nested LIST entry so no data is
    // lost. No corpus reference uses this form, so the flat path above covers
    // every exercised case.
    acc->Append(term);
  else
    acc->Append(e.u.pSymbol, e.eType == SymbolList::EntryType_BANG_SYMBOL);
}

SymbolList *xpyy_sub_init(SymbolList *first) {
  // The first sub_term's singleton doubles as the flat accumulator, except for
  // a range head (map range set), which must stay wrapped so later appends do
  // not attach to a list that already owns a whole-list map range.
  if (first && first->IsMapList())
    return new SymbolList(Sns(), first);
  return first;
}

SymbolList *xpyy_sub_append(SymbolList *list, SymbolList *next) {
  if (!list)
    return xpyy_sub_init(next);
  FoldSubTerm(list, next);
  return list;
}

SymbolList *xpyy_sub_name(const char *name) {
  if (!XPObject || !name)
    return nullptr;
  Variable *v = XPObject->InsertVariable(name);
  return new SymbolList(Sns(), v, false);
}

SymbolList *xpyy_sub_star(const char *bound) {
  // Bare `*` becomes a single bang-symbol entry whose owner gets sorted out
  // by later phases; `* : Dim` binds the wildcard to the named dimension.
  // The dimension symbol is created if absent so forward references resolve.
  if (!bound)
    return new SymbolList(Sns(), static_cast<Symbol *>(nullptr), true);
  if (!XPObject)
    return nullptr;
  Variable *dim = XPObject->InsertVariable(bound);
  return new SymbolList(Sns(), dim, true);
}

SymbolList *xpyy_sub_range(const char *lo, const char *hi) {
  // For `lo : hi` (range subscript), record the low endpoint as the list
  // head and the high endpoint as the map range. The range is carried through
  // unexpanded; nothing downstream currently expands it into an explicit
  // element list.
  if (!XPObject)
    return nullptr;
  Variable *vLo = XPObject->InsertVariable(lo ? lo : "");
  SymbolList *sl = new SymbolList(Sns(), vLo, false);
  if (hi) {
    Variable *vHi = XPObject->InsertVariable(hi);
    sl->SetMapRange(vHi);
  }
  return sl;
}

SymbolList *xpyy_sub_index(int idx) {
  // `@N` indexers reference the Nth element of the enclosing dimension. The
  // SymbolList model has no first-class integer slot, so encode the index as
  // a synthesized variable name `@N`, carried through unresolved.
  //
  // Collision risk: XMILE quoted identifiers (e.g. "\"@5\"") could produce a
  // variable named "@5" through the normal variable path, which would be
  // indistinguishable from a synthetic "@5" produced here. In practice XMILE
  // models do not use identifiers starting with `@`, but a subscript-binding
  // pass that consumes these entries should reserve a name shape that no
  // legal quoted identifier can produce (e.g. a non-printable prefix or a
  // namespace-local slot) to make the encoding collision-proof.
  if (!XPObject)
    return nullptr;
  const std::string name = "@" + std::to_string(idx);
  Variable *v = XPObject->InsertVariable(name);
  return new SymbolList(Sns(), v, false);
}

int xpyylex(void) {
  if (!XPObject || !XPObject->CurrentLex())
    return 0;
  return XPObject->CurrentLex()->yylex();
}
