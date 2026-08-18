#include <string>

#include "../TestHarness.h"
#include "RoundTrip.h"

using roundtrip::ExpectCleanRoundTrip;

// AC2.1 / AC3.2: an aux defined by an arithmetic expression, with units and a
// comment, round-trips with its RHS, units, and comment preserved.
TEST(EquationRoundTrip_aux_with_units_and_comment) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 2\r\n\t~~|\r\n"
      "b = 3\r\n\t~~|\r\n"
      "c = a * b\r\n"
      "\t~\twidgets\r\n"
      "\t~\ta comment\r\n"
      "\t|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.2: a stock with an explicit inflow and outflow emits as INTEG(net, init)
// and round-trips, including the flow structure and initial value.
TEST(EquationRoundTrip_stock_with_flows) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "s = INTEG(inflow - outflow, 100)\r\n\t~~|\r\n"
      "inflow = 5\r\n\t~~|\r\n"
      "outflow = 2\r\n\t~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.2 / AC3.1: an apply-to-all arrayed stock -- one INTEG equation spanning
// the whole dimension whose net flow carries the dimension subscript
// (inflow[Dim] - outflow[Dim]). Regression guard for the lossy net-flow emit
// that dropped the flow subscripts (writing INTEG(inflow-outflow, 10)).
TEST(EquationRoundTrip_arrayed_stock_apply_to_all) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: d1, d2, d3 ~~|\r\n"
      "inflow[Dim] = 5 ~~|\r\n"
      "outflow[Dim] = 2 ~~|\r\n"
      "s[Dim] = INTEG(inflow[Dim] - outflow[Dim], 10) ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.2 / AC3.1: a per-element arrayed stock -- one INTEG equation per element,
// each with its OWN net-flow subscript and its own initial value. Regression
// guard for reusing a single bare net-flow string across every per-element
// equation (which produced a duplicated synthetic "s net flow" on re-parse).
TEST(EquationRoundTrip_arrayed_stock_per_element) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: a, b ~~|\r\n"
      "inflow[Dim] = 5 ~~|\r\n"
      "s[a] = INTEG(inflow[a], 10) ~~|\r\n"
      "s[b] = INTEG(inflow[b], 20) ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.2 / AC3.1: an apply-to-all arrayed stock whose net flow is not a clean
// +/- of flows, so MarkVariableTypes synthesizes a "s net flow" variable. The
// writer inlines that synthetic flow's original expression (a[Dim] * 2) back
// into the INTEG, so re-parsing re-synthesizes the identical "s net flow".
TEST(EquationRoundTrip_arrayed_stock_synthetic_net_flow) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: d1, d2, d3 ~~|\r\n"
      "a[Dim] = 4 ~~|\r\n"
      "s[Dim] = INTEG(a[Dim] * 2, 7) ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.3: a standalone graphical function emits in Vensim "name(<body>)" syntax
// and round-trips its data points.
TEST(EquationRoundTrip_standalone_graphical_function) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "f([(0,0)-(10,10)],(0,0),(5,3),(10,10))\r\n\t~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.3: an embedded WITH LOOKUP applies an inline table to an input expression
// and round-trips both the input and the table.
TEST(EquationRoundTrip_embedded_with_lookup) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "x = 5\r\n\t~~|\r\n"
      "y = WITH LOOKUP(x, ([(0,0)-(10,10)],(0,0),(10,10)))\r\n\t~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.4: a named lookup applied to an input -- the "table(input)" call form --
// round-trips the lookup target and the input.
TEST(EquationRoundTrip_lookup_call) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "x = 5\r\n\t~~|\r\n"
      "g([(0,0)-(10,10)],(0,0),(5,8),(10,10))\r\n\t~~|\r\n"
      "z = g(x)\r\n\t~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.4 / AC3.2: a dimension definition plus an apply-to-all subscripted
// variable (one equation spanning the whole dimension) round-trips its elements
// and its arrayed RHS.
TEST(EquationRoundTrip_dimension_apply_to_all) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: a, b, c ~~|\r\n"
      "v[Dim] = 1, 2, 3 ~~|\r\n"
      "w[Dim] = v[Dim] * 2 ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.4: a dimension with per-element equations (one equation per element)
// round-trips each element's own RHS.
TEST(EquationRoundTrip_dimension_per_element) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: a, b, c ~~|\r\n"
      "p[a] = 10 ~~|\r\n"
      "p[b] = 20 ~~|\r\n"
      "p[c] = 30 ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.6: an equation whose rendered text exceeds Vensim's ~80-char line budget
// wraps with "\" continuations and re-parses to the same expression. The long
// sum below renders well past 80 columns, forcing at least one continuation.
TEST(EquationRoundTrip_long_wrapped_equation) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "alpha = 1 ~~|\r\n"
      "beta = 2 ~~|\r\n"
      "gamma = 3 ~~|\r\n"
      "delta = 4 ~~|\r\n"
      "epsilon = 5 ~~|\r\n"
      "zeta = 6 ~~|\r\n"
      "total = alpha + beta + gamma + delta + epsilon + zeta + alpha + beta + gamma + delta ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.1: builtins (IF THEN ELSE, PULSE) emit verbatim and round-trip without any
// XMILE-style rewrite.
TEST(EquationRoundTrip_builtins) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "x = 1 ~~|\r\n"
      "z = IF THEN ELSE(x > 0, PULSE(1, 2), 0) ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC3.1 (corpus preview): a complete equation-only model from the corpus
// round-trips. This is the verbatim content of models/delay/delay.mdl from the
// SDEverywhere corpus (inlined rather than read from disk so the test does not
// depend on the process working directory). delay.mdl exercises dimensions, a subrange (SubA), arrayed
// constant data, scalar/arrayed mixes, arithmetic, and the DELAY1/DELAY3/DELAY1I
// /DELAY3I memory builtins, which the walker emits verbatim and the comparator
// matches structurally. It contains no :EXCEPT: clause, so it is within v1 scope.
TEST(EquationRoundTrip_corpus_delay_model) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "DimA: A1, A2, A3 ~~|\r\n"
      "SubA: A2, A3 ~~|\r\n"
      "\r\n"
      "input = STEP(10, 0) - STEP(10, 4) ~~|\r\n"
      "delay = 5 ~~|\r\n"
      "init 1 = 0 ~~|\r\n"
      "\r\n"
      "input a[DimA]= 10, 20, 30 ~~|\r\n"
      "delay a[DimA] = 1, 2, 3 ~~|\r\n"
      "init a[DimA] = 0 ~~|\r\n"
      "\r\n"
      "input 2[SubA]= 20, 30 ~~|\r\n"
      "delay 2 = 5 ~~|\r\n"
      "init 2[SubA] = 0 ~~|\r\n"
      "\r\n"
      "k = 42 ~~|\r\n"
      "\r\n"
      "d1 = DELAY1(input, delay) ~~|\r\n"
      "d2[DimA] = DELAY1I(input a[DimA], delay, init 1) ~~|\r\n"
      "d3[DimA] = DELAY1I(input, delay a[DimA], init 1) ~~|\r\n"
      "d4[DimA] = DELAY1I(input, delay, init a[DimA]) ~~|\r\n"
      "d5[DimA] = DELAY1I(input a[DimA], delay a[DimA], init a[DimA]) ~~|\r\n"
      "d6[SubA] = DELAY1I(input 2[SubA], delay 2, init 2[SubA]) ~~|\r\n"
      "\r\n"
      "d7 = DELAY3(input, delay) ~~|\r\n"
      "d8[DimA] = DELAY3(input, delay a[DimA]) ~~|\r\n"
      "d9[SubA] = DELAY3I(input 2[SubA], delay 2, init 2[SubA]) ~~|\r\n"
      "\r\n"
      "d10 = k * DELAY3(input, delay) ~~|\r\n"
      "d11[DimA] = k * DELAY3(input, delay a[DimA]) ~~|\r\n"
      "d12[SubA] = k * DELAY3I(input 2[SubA], delay 2, init 2[SubA]) ~~|\r\n"
      "\r\n"
      "INITIAL TIME = 0 ~~|\r\n"
      "FINAL TIME = 10 ~~|\r\n"
      "TIME STEP = 1 ~~|\r\n"
      "SAVEPER = TIME STEP ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC2.1: the walker drops the parser's paren nodes and re-derives every
// parenthesis from grammar precedence, so an equal-precedence operand is where
// that derivation can silently hand back a DIFFERENT tree. Every binary level in
// VYacc.y is `%left` except ^, so a right operand at the parent's own level must
// keep its parens (and under ^ it is the left operand instead).
//
// Regression guard for the equal-precedence rule having been keyed off the
// arithmetic operator spelling (Expression::GetOperator), which an
// ExpressionLogical does not have: every comparison, :AND: and :OR: parent fell
// straight through it, so `a < (b < c)` was emitted as `a < b < c` and re-parsed
// as `(a < b) < c` -- a different expression with a different value. The
// comparator is structural, so each line below fails if its grouping is lost.
TEST(EquationRoundTrip_equal_precedence_grouping) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 1 ~~|\r\n"
      "b = 2 ~~|\r\n"
      "c = 3 ~~|\r\n"
      "cmp lt right = a < (b < c) ~~|\r\n"
      "cmp lt left = (a < b) < c ~~|\r\n"
      "cmp ge right = a > (b >= c) ~~|\r\n"
      "cmp eq right = a = (b = c) ~~|\r\n"
      "cmp ne right = a <> (b <> c) ~~|\r\n"
      "cmp le left = (a <= b) <> c ~~|\r\n"
      "logical or right = a :OR: (b :OR: c) ~~|\r\n"
      "logical or left = (a :OR: b) :OR: c ~~|\r\n"
      "logical and right = a :AND: (b :AND: c) ~~|\r\n"
      "logical and left = (a :AND: b) :AND: c ~~|\r\n"
      "mixed and of cmp = (a < b) :AND: c ~~|\r\n"
      "mixed cmp of and = a < (b :AND: c) ~~|\r\n"
      "mixed cmp of or = (a :OR: b) < c ~~|\r\n"
      "mixed add of cmp = (a + b) < c ~~|\r\n"
      "logical not of cmp = :NOT: (a < b) ~~|\r\n"
      "add right = a + (b - c) ~~|\r\n"
      "add left = (a + b) - c ~~|\r\n"
      "mul right = a * (b / c) ~~|\r\n"
      "mul left = (a * b) / c ~~|\r\n"
      "sub right = a - (b - c) ~~|\r\n"
      "div right = a / (b / c) ~~|\r\n"
      "pow left = (a ^ b) ^ c ~~|\r\n"
      "pow right = a ^ (b ^ c) ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// Unary minus is the LOWEST-precedence production in the grammar (VYacc.y:76 --
// `'-' exp` takes its precedence from the '-' it shares with binary +/-), so an
// emitted unary reaches rightward over every operator that binds tighter than
// level 1 unless it is parenthesized. The walker used to exempt a unary RIGHT
// operand entirely, on the theory that it "extends to the end of the
// sub-expression" -- true only when the parent ends the WHOLE expression.
//
// `unary right of tighter` below is the defect in situ: it was emitted as
// `a * -1 < b`, which re-parses as a * (-(1 < b)). Every line is compared
// structurally (ModelComparator walks the ASTs), so a re-grouped re-parse fails
// even where the value happens to survive.
TEST(EquationRoundTrip_unary_operand_grouping) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 1 ~~|\r\n"
      "b = 2 ~~|\r\n"
      "unary right of tighter = (a * -1) < b ~~|\r\n"
      "unary right of equal = (a + -1) < b ~~|\r\n"
      "unary owns comparison = a * (-1 < b) ~~|\r\n"
      "unary right of mul = a * (-b) ~~|\r\n"
      "unary right of div = a / (-b) ~~|\r\n"
      "unary right of pow = a ^ (-b) ~~|\r\n"
      "unary right of and = a :AND: (-b) ~~|\r\n"
      "unary right of cmp = a < (-b) ~~|\r\n"
      "not right of pow = a ^ (:NOT: b) ~~|\r\n"
      "unary right of add = a + (-b) ~~|\r\n"
      "unary right of sub = a - (-b) ~~|\r\n"
      "not right of mul = a * (:NOT: b) ~~|\r\n"
      "unary left of pow = (-a) ^ b ~~|\r\n"
      "unary left of add = (-a) + b ~~|\r\n"
      "nested unary = - -a ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// The parser folds `'-' <number>` into a single negative ExpressionNumber, so a
// negative literal is a leaf that nonetheless RENDERS with a leading sign -- and
// on re-import that sign is the `'-' exp` production again. Emitted bare it
// re-binds exactly like a unary minus: `(-2) ^ 2` became `-2 ^ 2`, which
// re-parses as -(2 ^ 2), swapping 4 for -4. The `-2 ^ 2 ^ 3` line pins the
// nested case, and the `:NA:` lines pin the sentinel that is stored as -1e38 but
// renders with no sign at all.
TEST(EquationRoundTrip_negative_literal_grouping) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 1 ~~|\r\n"
      "b = 2 ~~|\r\n"
      "neg pow = (-2) ^ 2 ~~|\r\n"
      "neg pow nested = ((-2) ^ 2) ^ 3 ~~|\r\n"
      "neg mul = (-2) * a ~~|\r\n"
      "neg div = (-2) / a ~~|\r\n"
      "neg cmp = (-1) < b ~~|\r\n"
      "neg and = (-2) :AND: b ~~|\r\n"
      "neg or = (-2) :OR: b ~~|\r\n"
      "neg right of mul = a * (-2) ~~|\r\n"
      "neg right of pow = a ^ (-2) ~~|\r\n"
      "neg add = (-2) + a ~~|\r\n"
      "neg right of add = a + (-2) ~~|\r\n"
      "neg right of sub = a - (-2) ~~|\r\n"
      "neg under unary = - (-2) ~~|\r\n"
      "na mul = :NA: * a ~~|\r\n"
      "na pow = a ^ :NA: ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// A user flow named literally "s net flow" is structurally indistinguishable
// from the variable MarkStockFlows synthesizes for the same stock (same name,
// same FLOW type, the stock's sole inflow). The writer inlines its expression
// into the INTEG (s = INTEG(5, 0)); re-parsing re-synthesizes the identical
// "s net flow", so the collapse is a stable, structurally-equivalent
// normalization. Guards against the inline+suppress logic introducing a spurious
// diff (or an unstable, non-converging output) on this name collision.
TEST(EquationRoundTrip_stock_with_user_named_net_flow) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "s net flow = 5 ~~|\r\n"
      "s = INTEG(s net flow, 0) ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}
