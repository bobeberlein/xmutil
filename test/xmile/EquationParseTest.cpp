// Structural Expression-tree tests for the XMILE equation grammar driven
// through XmileReader::ParseEquation. These tests do not depend on
// Expression::OutputComputable (which requires a fully-built ContextInfo) --
// they assert the shape of the resulting tree via dynamic_cast and the
// public Expression accessors.
//
// Phase 2 acceptance criteria coverage: AC2.1 (arithmetic shapes + precedence),
// AC2.2 (XMILE function un-rename), AC2.3 (bare keywords as Variable*),
// AC2.4 (if/then/else), AC2.5 (subscripts), AC2.6 (keyword vs C-style operator
// equivalence), AC2.7 (// safediv -> ZIDZ; safediv(...,...,...) -> XIDZ),
// AC2.8 (unknown function reports an error), AC2.9 (postfix ' is rejected).

#include <string>
#include <vector>

#include "../../src/Function/Function.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/ExpressionList.h"
#include "../../src/Symbol/Parse.h"
#include "../../src/Symbol/SymbolList.h"
#include "../../src/Symbol/Variable.h"
// VYacc.tab.hpp declares `extern YYSTYPE vpyylval` at file scope, so YYSTYPE
// must be visible before the include even though we only consume the VPTT_*
// enum constants. The XMILE driver uses the same ParseUnion alias.
#define YYSTYPE ParseUnion
#include "../../src/Vensim/VYacc.tab.hpp"
#include "../../src/Xmile/XmileReader.h"
#include "../TestHarness.h"

namespace {

// Returns the int oper code (e.g. '<', VPTT_le, VPTT_and) stored on an
// ExpressionLogical node, or -1 if the cast fails. The tests use this to
// assert that keyword (`and`) and C-style (`&&`) spellings produce the same
// Vensim VPTT_* code, so downstream consumers see a single canonical encoding.
int LogicalOp(Expression *e) {
  ExpressionLogical *l = dynamic_cast<ExpressionLogical *>(e);
  return l ? l->LogicalOperator() : -1;
}

// Each test stands up its own Model and XmileReader. The reader's ctor calls
// RegisterXmutilFunctions on the namespace (INTEG, IF THEN ELSE, SMOOTH, ZIDZ,
// XIDZ, ...), so no transient VensimParse is needed to seed the Function table.

}  // namespace

TEST(XmileParse_addition_builds_ExpressionAdd) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("1 + 2", errs);
  CHECK(e != nullptr);
  CHECK(errs.empty());
  CHECK(dynamic_cast<ExpressionAdd *>(e) != nullptr);
  delete e;
}

TEST(XmileParse_subtraction_builds_ExpressionSubtract) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("3 - 4", errs);
  CHECK(e != nullptr);
  CHECK(dynamic_cast<ExpressionSubtract *>(e) != nullptr);
  delete e;
}

TEST(XmileParse_multiplication_division_power) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *mul = reader.ParseEquation("2 * 3", errs);
  Expression *div = reader.ParseEquation("6 / 2", errs);
  Expression *pow = reader.ParseEquation("2 ^ 3", errs);
  CHECK(dynamic_cast<ExpressionMultiply *>(mul) != nullptr);
  CHECK(dynamic_cast<ExpressionDivide *>(div) != nullptr);
  CHECK(dynamic_cast<ExpressionPower *>(pow) != nullptr);
  delete mul;
  delete div;
  delete pow;
}

TEST(XmileParse_precedence_a_plus_b_times_c) {
  // The XMILE grammar follows the simlin precedence lattice (multiplicative
  // binds tighter than additive), so `a + b * c` parses as
  // Add(a, Multiply(b, c)). The right operand of the Add (GetArg(1)) must be
  // the Multiply node.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("a + b * c", errs);
  ExpressionAdd *add = dynamic_cast<ExpressionAdd *>(e);
  CHECK(add != nullptr);
  if (add) {
    CHECK(dynamic_cast<ExpressionMultiply *>(add->GetArg(1)) != nullptr);
  }
  delete e;
}

TEST(XmileParse_function_smth1_resolves_to_SMOOTH) {
  // The xmutil SMOOTH function descends from DFunction (a delay-tagged
  // Function), not from FunctionMemoryBase, so Function::IsMemoryless stays
  // true and xpyy_call wraps it in a plain ExpressionFunction rather than
  // ExpressionFunctionMemory. That matches what VensimParse::FunctionExpression
  // produces for a Vensim-side SMOOTH call too, which is the invariant this
  // test really cares about. AC2.2 is satisfied by the SMOOTH name
  // resolution; the wrapper-type symmetry with Vensim is the secondary check.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("smth1(input, 5)", errs);
  ExpressionFunction *fn = dynamic_cast<ExpressionFunction *>(e);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetFunction()->GetName(), "SMOOTH");
  delete e;
}

TEST(XmileParse_function_integ_uses_ExpressionFunctionMemory) {
  // INTEG is registered through FSubclassMemory (FunctionMemoryBase), so
  // IsMemoryless returns false and xpyy_call yields ExpressionFunctionMemory.
  // This pins the memory-vs-memoryless dispatch the way VensimParse does it,
  // so a downstream walker that branches on EXPTYPE_FunctionMemory sees the
  // same node kind regardless of which front-end built the tree.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("integ(inflow, 0)", errs);
  ExpressionFunctionMemory *fn = dynamic_cast<ExpressionFunctionMemory *>(e);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetFunction()->GetName(), "INTEG");
  delete e;
}

TEST(XmileParse_if_then_else_builds_IF_THEN_ELSE) {
  // IF THEN ELSE is memoryless (FunctionIfThenElse inherits the default
  // IsMemoryless == true), so the production builds an ExpressionFunction.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("if x > 0 then 1 else -1", errs);
  ExpressionFunction *fn = dynamic_cast<ExpressionFunction *>(e);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetFunction()->GetName(), "IF THEN ELSE");
  delete e;
}

TEST(XmileParse_bare_time_resolves_to_Time_Variable) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("time", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var)
    CHECK_EQ_STR(var->GetVariable()->GetName(), "Time");
  delete e;
}

TEST(XmileParse_bare_dt_resolves_to_TIME_STEP_Variable) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("dt", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var)
    CHECK_EQ_STR(var->GetVariable()->GetName(), "TIME STEP");
  delete e;
}

TEST(XmileParse_bare_initial_time_resolves_to_INITIAL_TIME_Variable) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("initial_time", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var)
    CHECK_EQ_STR(var->GetVariable()->GetName(), "INITIAL TIME");
  delete e;
}

TEST(XmileParse_bare_final_time_resolves_to_FINAL_TIME_Variable) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("final_time", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var)
    CHECK_EQ_STR(var->GetVariable()->GetName(), "FINAL TIME");
  delete e;
}

TEST(XmileParse_subscripted_variable_x_a_b) {
  // For `x[a, b]` the grammar folds two singleton sub_term lists into one flat
  // SymbolList -- one entry per subscript position, each EntryType_SYMBOL. This
  // matches the shape VensimParse::SymList produces and the shape both writers
  // (MDLGenerator::RenderSubscripts, SymbolList::OutputComputable) consume; the
  // former list-of-lists shape rendered as empty `[]` in XMILE and crashed the
  // MDL writer.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("x[a, b]", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var) {
    SymbolList *subs = var->GetSubs();
    CHECK(subs != nullptr);
    if (subs) {
      CHECK(subs->Length() == 2);
      if (subs->Length() == 2) {
        CHECK((*subs)[0].eType == SymbolList::EntryType_SYMBOL);
        CHECK((*subs)[1].eType == SymbolList::EntryType_SYMBOL);
        CHECK_EQ_STR((*subs)[0].u.pSymbol->GetName(), "a");
        CHECK_EQ_STR((*subs)[1].u.pSymbol->GetName(), "b");
      }
    }
  }
  delete e;
}

TEST(XmileParse_range_subscript_preserves_dimension_names) {
  // `a[Dim1:Dim2]` -- MaybeColonNa peeks up to three characters past the ':'
  // before deciding it is not an :NA: literal. With the old single-slot
  // pushback two of those characters were silently dropped, mangling "Dim2"
  // into "D2".
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("a[Dim1:Dim2]", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var) {
    SymbolList *subs = var->GetSubs();
    CHECK(subs != nullptr && subs->Length() == 1);
    if (subs != nullptr && subs->Length() == 1 && (*subs)[0].eType == SymbolList::EntryType_LIST) {
      SymbolList *range = (*subs)[0].u.pSymbolList;
      CHECK(range->Length() == 1);
      if (range->Length() == 1)
        CHECK_EQ_STR((*range)[0].u.pSymbol->GetName(), "Dim1");
      CHECK(range->MapRange() != nullptr);
      if (range->MapRange() != nullptr)
        CHECK_EQ_STR(range->MapRange()->GetName(), "Dim2");
    }
  }
  delete e;
}

TEST(XmileParse_star_bounded_subscript_preserves_dimension_name) {
  // `a[*:Dimension]` exercises the same not-:NA: pushback path.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("a[*:Dimension]", errs);
  ExpressionVariable *var = dynamic_cast<ExpressionVariable *>(e);
  CHECK(var != nullptr);
  if (var) {
    SymbolList *subs = var->GetSubs();
    CHECK(subs != nullptr && subs->Length() == 1);
    // `*:Dimension` folds to a single flat BANG_SYMBOL entry naming the bound
    // dimension (the bang marks the summation axis; the concrete name means no
    // post-parse wildcard resolution is needed for this form).
    if (subs != nullptr && subs->Length() == 1) {
      CHECK((*subs)[0].eType == SymbolList::EntryType_BANG_SYMBOL);
      CHECK((*subs)[0].u.pSymbol != nullptr);
      if ((*subs)[0].u.pSymbol != nullptr)
        CHECK_EQ_STR((*subs)[0].u.pSymbol->GetName(), "Dimension");
    }
  }
  delete e;
}

TEST(XmileParse_keyword_and_matches_C_style_amp_amp) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e1 = reader.ParseEquation("a and b", errs);
  Expression *e2 = reader.ParseEquation("a && b", errs);
  CHECK(LogicalOp(e1) == LogicalOp(e2));
  CHECK(LogicalOp(e1) == VPTT_and);
  delete e1;
  delete e2;
}

TEST(XmileParse_keyword_or_matches_C_style_pipe_pipe) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e1 = reader.ParseEquation("a or b", errs);
  Expression *e2 = reader.ParseEquation("a || b", errs);
  CHECK(LogicalOp(e1) == LogicalOp(e2));
  CHECK(LogicalOp(e1) == VPTT_or);
  delete e1;
  delete e2;
}

TEST(XmileParse_keyword_not_matches_bang) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e1 = reader.ParseEquation("not a", errs);
  Expression *e2 = reader.ParseEquation("!a", errs);
  CHECK(LogicalOp(e1) == LogicalOp(e2));
  CHECK(LogicalOp(e1) == VPTT_not);
  delete e1;
  delete e2;
}

TEST(XmileParse_equality_eq_matches_double_eq) {
  // `=` and `==` are token-level synonyms in the lexer; both reduce to an
  // ExpressionLogical with oper == '='.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e1 = reader.ParseEquation("a = b", errs);
  Expression *e2 = reader.ParseEquation("a == b", errs);
  CHECK(LogicalOp(e1) == LogicalOp(e2));
  CHECK(LogicalOp(e1) == '=');
  delete e1;
  delete e2;
}

TEST(XmileParse_inequality_angle_matches_bang_eq) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e1 = reader.ParseEquation("a <> b", errs);
  Expression *e2 = reader.ParseEquation("a != b", errs);
  CHECK(LogicalOp(e1) == LogicalOp(e2));
  CHECK(LogicalOp(e1) == VPTT_ne);
  delete e1;
  delete e2;
}

TEST(XmileParse_mod_keyword_and_percent_map_to_MODULO) {
  // Both `mod` (keyword) and `%` (C-style) lex to XPTT_mod, which the grammar
  // funnels through xpyy_function("MODULO", l, r). MODULO is memoryless, so
  // we get a plain ExpressionFunction.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e1 = reader.ParseEquation("a mod b", errs);
  Expression *e2 = reader.ParseEquation("a % b", errs);
  ExpressionFunction *fn1 = dynamic_cast<ExpressionFunction *>(e1);
  ExpressionFunction *fn2 = dynamic_cast<ExpressionFunction *>(e2);
  CHECK(fn1 != nullptr);
  CHECK(fn2 != nullptr);
  if (fn1)
    CHECK_EQ_STR(fn1->GetFunction()->GetName(), "MODULO");
  if (fn2)
    CHECK_EQ_STR(fn2->GetFunction()->GetName(), "MODULO");
  delete e1;
  delete e2;
}

TEST(XmileParse_safediv_two_args_uses_ZIDZ) {
  // The `//` operator lowers via xpyy_safediv to a ZIDZ call.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("a // b", errs);
  ExpressionFunction *fn = dynamic_cast<ExpressionFunction *>(e);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetFunction()->GetName(), "ZIDZ");
  delete e;
}

TEST(XmileParse_safediv_three_args_uses_XIDZ) {
  // The 3-arg call form picks XIDZ (the variant with an explicit fallback
  // value) per the AC2.7 arg-count dispatch.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("safediv(a, b, 0)", errs);
  ExpressionFunction *fn = dynamic_cast<ExpressionFunction *>(e);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetFunction()->GetName(), "XIDZ");
  delete e;
}

// Return the PULSE / PULSE TRAIN function-call node buried inside the
// (volume / TIME STEP) * <pulse> tree the pulse translation builds, or NULL if
// the shape is not the expected Multiply(Divide, FunctionCall).
ExpressionFunction *PulseCallInside(Expression *e) {
  ExpressionMultiply *mul = dynamic_cast<ExpressionMultiply *>(e);
  if (!mul)
    return nullptr;
  if (!dynamic_cast<ExpressionDivide *>(mul->GetArg(0)))
    return nullptr;
  return dynamic_cast<ExpressionFunction *>(mul->GetArg(1));
}

TEST(XmileParse_pulse_two_arg_translates_to_scaled_PULSE) {
  // XMILE pulse(volume, first) is an area impulse -> (volume / TIME STEP) *
  // PULSE(first, TIME STEP). A name-only map to Vensim PULSE(volume, first)
  // would silently rescale the model, so the reader translates structurally.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("pulse(10, 3)", errs);
  CHECK(errs.empty());
  ExpressionMultiply *mul = dynamic_cast<ExpressionMultiply *>(e);
  CHECK(mul != nullptr);
  if (mul) {
    // numerator is the volume literal, denominator is TIME STEP
    ExpressionDivide *div = dynamic_cast<ExpressionDivide *>(mul->GetArg(0));
    CHECK(div != nullptr);
    if (div) {
      ExpressionNumber *vol = dynamic_cast<ExpressionNumber *>(div->GetArg(0));
      CHECK(vol != nullptr && vol->GetValue() == 10.0);
      ExpressionVariable *dt = dynamic_cast<ExpressionVariable *>(div->GetArg(1));
      CHECK(dt != nullptr);
      if (dt)
        CHECK_EQ_STR(dt->GetVariable()->GetName(), "TIME STEP");
    }
    ExpressionFunction *pulse = dynamic_cast<ExpressionFunction *>(mul->GetArg(1));
    CHECK(pulse != nullptr);
    if (pulse)
      CHECK_EQ_STR(pulse->GetFunction()->GetName(), "PULSE");
  }
  delete e;
}

TEST(XmileParse_pulse_three_arg_translates_to_scaled_PULSE_TRAIN) {
  // pulse(volume, first, interval) -> (volume / TIME STEP) *
  // PULSE TRAIN(first, TIME STEP, interval, FINAL TIME).
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("pulse(2, 7, 5)", errs);
  CHECK(errs.empty());
  ExpressionFunction *pulse = PulseCallInside(e);
  CHECK(pulse != nullptr);
  if (pulse)
    CHECK_EQ_STR(pulse->GetFunction()->GetName(), "PULSE TRAIN");
  delete e;
}

TEST(XmileParse_pulse_nonpositive_interval_degrades_to_single_pulse) {
  // An interval literal <= 0 means "do not repeat": the three-arg form degrades
  // to the single-pulse PULSE, not PULSE TRAIN.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("pulse(2, 7, 0)", errs);
  CHECK(errs.empty());
  ExpressionFunction *pulse = PulseCallInside(e);
  CHECK(pulse != nullptr);
  if (pulse)
    CHECK_EQ_STR(pulse->GetFunction()->GetName(), "PULSE");
  delete e;
}

TEST(XmileParse_NaN_literal_maps_to_minus_1e38) {
  // The lexer emits XPTT_number with the simlin sentinel value -1e38 for the
  // :NA: literal; the parser builds a plain ExpressionNumber from it.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation(":NA:", errs);
  ExpressionNumber *n = dynamic_cast<ExpressionNumber *>(e);
  CHECK(n != nullptr);
  if (n)
    CHECK(n->GetValue() == -1e38);
  delete e;
}

TEST(XmileParse_unknown_function_errors) {
  // xpyy_call returns a 0-numeric placeholder when the un-rename + fallback
  // both miss, AND pushes "unknown function 'X'" onto the errs sink. The
  // parse itself succeeds (so e != nullptr) but errs is non-empty -- this is
  // the design that lets the parser collect every problem in one pass.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("completely_unknown_function(1, 2)", errs);
  CHECK(!errs.empty());
  delete e;
}

// pi() and the bare keyword pi both lower to a full-precision numeric literal
// (Vensim has no PI builtin). The tree is a plain ExpressionNumber, not a call
// or a phantom variable reference.
TEST(XmileParse_pi_call_maps_to_numeric_literal) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("pi()", errs);
  CHECK(errs.empty());
  ExpressionNumber *n = dynamic_cast<ExpressionNumber *>(e);
  CHECK(n != nullptr);
  if (n)
    CHECK(n->GetValue() == 3.141592653589793);
  delete e;
}

TEST(XmileParse_bare_pi_maps_to_numeric_literal) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("PI", errs);
  CHECK(errs.empty());
  ExpressionNumber *n = dynamic_cast<ExpressionNumber *>(e);
  CHECK(n != nullptr);
  if (n)
    CHECK(n->GetValue() == 3.141592653589793);
  delete e;
}

// A fixed-arity builtin called with too many or too few arguments is now a hard
// error naming the function, the accepted count, and the got-count. The parse
// still returns a placeholder Expression (so the reader collects every problem),
// but the unprefixed diagnostic makes ProcessFile fail the whole conversion.
TEST(XmileParse_too_many_args_is_fatal_arity_error) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("sin(1, 2)", errs);
  bool named = false;
  for (const std::string &s : errs)
    if (s.find("SIN") != std::string::npos && s.find("got 2") != std::string::npos &&
        s.find("warning:") == std::string::npos)
      named = true;
  CHECK(named);
  delete e;
}

TEST(XmileParse_too_few_args_is_fatal_arity_error) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("sin()", errs);
  bool named = false;
  for (const std::string &s : errs)
    if (s.find("SIN") != std::string::npos && s.find("got 0") != std::string::npos &&
        s.find("warning:") == std::string::npos)
      named = true;
  CHECK(named);
  delete e;
}

// Within a widened range (DELAY FIXED's optional initial value) the parse is
// clean -- no arity diagnostic at all.
TEST(XmileParse_delay_two_args_within_range_is_clean) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("delay(inflow, 3)", errs);
  CHECK(e != nullptr);
  bool arity = false;
  for (const std::string &s : errs)
    if (s.find("expects") != std::string::npos)
      arity = true;
  CHECK(!arity);
  delete e;
}

// A RANDOM-family call parses cleanly both with and without its optional seed.
TEST(XmileParse_random_uniform_optional_seed_both_clean) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *without = reader.ParseEquation("uniform(0, 1)", errs);
  Expression *with = reader.ParseEquation("uniform(0, 1, 42)", errs);
  CHECK(without != nullptr);
  CHECK(with != nullptr);
  bool arity = false;
  for (const std::string &s : errs)
    if (s.find("expects") != std::string::npos)
      arity = true;
  CHECK(!arity);
  delete without;
  delete with;
}

// A 2-argument delay pads to a full-arity DELAY FIXED whose synthesized initial
// value is a DEEP COPY of the input expression (not a shared node). The stored
// AST therefore carries three arguments and emits valid Vensim.
TEST(XmileParse_delay_two_args_pads_initial_with_input_clone) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("delay(x + 1, 3)", errs);
  CHECK(errs.empty());
  ExpressionFunction *fn = dynamic_cast<ExpressionFunction *>(e);
  CHECK(fn != nullptr);
  if (fn) {
    ExpressionList *args = fn->GetArgs();
    CHECK(args != nullptr);
    if (args) {
      CHECK(args->Length() == 3);
      // The synthesized initial (arg 2) is a distinct node from the input
      // (arg 0) -- a clone, not the same pointer -- so teardown cannot double
      // free it.
      CHECK(args->GetExp(0) != args->GetExp(2));
      // Both are the same structural shape (an add expression).
      CHECK(dynamic_cast<ExpressionAdd *>(args->GetExp(2)) != nullptr);
    }
  }
  delete e;
}

// A 2-argument trend/ramp/uniform pads its trailing arg too (0 / FINAL TIME / 0).
TEST(XmileParse_trend_ramp_uniform_pad_to_full_arity) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *tr = reader.ParseEquation("trend(x, 3)", errs);
  Expression *rm = reader.ParseEquation("ramp(2, 1)", errs);
  Expression *un = reader.ParseEquation("uniform(0, 1)", errs);
  CHECK(errs.empty());
  for (Expression *e : {tr, rm, un}) {
    ExpressionFunction *fn = dynamic_cast<ExpressionFunction *>(e);
    CHECK(fn != nullptr);
    if (fn && fn->GetArgs())
      CHECK(fn->GetArgs()->Length() == 3);
  }
  delete tr;
  delete rm;
  delete un;
}

// The DELAY N / SMOOTH N reorder swaps the trailing (order, initial) pair; the
// arity check runs on the post-reorder count, so a full four-argument call is
// accepted with no diagnostic (it must not be misread as out-of-range).
TEST(XmileParse_delay_n_full_arity_reorder_is_clean) {
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *dn = reader.ParseEquation("delayn(inflow, 3, 2, 0)", errs);
  Expression *sn = reader.ParseEquation("smthn(inflow, 3, 2, 0)", errs);
  CHECK(dn != nullptr);
  CHECK(sn != nullptr);
  bool arity = false;
  for (const std::string &s : errs)
    if (s.find("expects") != std::string::npos)
      arity = true;
  CHECK(!arity);
  delete dn;
  delete sn;
}

TEST(XmileParse_postfix_apostrophe_errors) {
  // The grammar's `expr XPTT_apostrophe` rule calls xpyyerror + YYABORT.
  // ParseEquation returns nullptr on a non-zero parse return, and errs gets
  // the descriptive "postfix ' (transpose) is not supported" message.
  Model m;
  XmileReader reader(&m);
  std::vector<std::string> errs;
  Expression *e = reader.ParseEquation("a'", errs);
  CHECK(e == nullptr);
  CHECK(!errs.empty());
  delete e;
}
