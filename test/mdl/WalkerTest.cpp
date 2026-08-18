#include <string>

#include "../../src/Mdl/MDLGenerator.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Equation.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// Wrap a bare RHS expression in a minimal but complete Vensim equation so the
// parser accepts it, parse it, and render the parsed RHS of the named variable
// back to Vensim text via the walker. The `~~|` trailer is the Vensim shorthand
// for "no units, no comment".
std::string RenderRHS(const std::string &rhs, const std::string &varName = "tmp") {
  std::string text = "{UTF-8}\r\n" + varName + " = " + rhs + "\r\n\t~~|\r\n";
  Model *m = roundtrip::ParseVensim(text);
  if (!m)
    return "<parse-failed>";
  std::string out = "<not-found>";
  for (Variable *v : m->GetVariables(nullptr)) {
    if (v->GetName() == varName) {
      Equation *eq = v->GetEquation(0);
      if (!eq) {
        out = "<no-equation>";
        break;
      }
      MDLGenerator g(m);
      out = g.RenderExpression(eq->GetExpression());
      break;
    }
  }
  delete m;
  return out;
}

}  // namespace

// AC2.1: arithmetic precedence and associativity drive parenthesization, with
// paren nodes unwrapped and re-derived purely from precedence (matching simlin).
TEST(Walker_precedence_basic) {
  CHECK_EQ_STR(RenderRHS("a + b * c"), "a + b * c");
  CHECK_EQ_STR(RenderRHS("(a + b) * c"), "(a + b) * c");
  CHECK_EQ_STR(RenderRHS("a * b + c"), "a * b + c");
  CHECK_EQ_STR(RenderRHS("a * (b + c)"), "a * (b + c)");
}

TEST(Walker_non_associative_subtraction_division) {
  // Right child of - / keeps its parens at equal precedence; left child does not.
  CHECK_EQ_STR(RenderRHS("a - (b - c)"), "a - (b - c)");
  CHECK_EQ_STR(RenderRHS("(a - b) - c"), "a - b - c");
  CHECK_EQ_STR(RenderRHS("a / (b / c)"), "a / (b / c)");
  CHECK_EQ_STR(RenderRHS("(a / b) / c"), "a / b / c");
}

// The same equal-precedence rule applies to + and *, which ARE associative in
// value: `a + b + c` re-parses as `(a + b) + c`, so dropping the parens from
// `a + (b + c)` hands back a different tree than the modeler wrote. The writer
// promises a structurally equivalent re-parse (ModelComparator compares ASTs),
// and over floating point the regrouping is not even value-preserving.
TEST(Walker_equal_precedence_right_operand_keeps_parens) {
  CHECK_EQ_STR(RenderRHS("a + (b + c)"), "a + (b + c)");
  CHECK_EQ_STR(RenderRHS("a + (b - c)"), "a + (b - c)");
  CHECK_EQ_STR(RenderRHS("(a + b) + c"), "a + b + c");
  CHECK_EQ_STR(RenderRHS("a * (b * c)"), "a * (b * c)");
  CHECK_EQ_STR(RenderRHS("a * (b / c)"), "a * (b / c)");
  CHECK_EQ_STR(RenderRHS("(a * b) * c"), "a * b * c");
}

TEST(Walker_power_grouping) {
  // ^ has the highest arithmetic precedence; an explicit grouping is preserved.
  CHECK_EQ_STR(RenderRHS("2 ^ 3 ^ 2"), "2 ^ 3 ^ 2");
  CHECK_EQ_STR(RenderRHS("(2 ^ 3) ^ 2"), "(2 ^ 3) ^ 2");
  CHECK_EQ_STR(RenderRHS("a * b ^ c"), "a * b ^ c");
}

TEST(Walker_unary_minus) {
  CHECK_EQ_STR(RenderRHS("-a"), "-a");
  CHECK_EQ_STR(RenderRHS("-(a + b)"), "-(a + b)");
  // In the xmutil grammar unary minus binds looser than *, so `-a * b` parses
  // as -(a * b); the walker emits the grouping that reflects (and re-parses to)
  // that tree.
  CHECK_EQ_STR(RenderRHS("-a * b"), "-(a * b)");
  // A unary RIGHT operand of a tighter parent is parenthesized too. `a * -b`
  // re-parses correctly only when the `*` node ends the expression; the walker
  // decides the parens from the parent alone and cannot see what the
  // grandparent chain will append, so `a * (-b)` is the grouping it can
  // guarantee. Emitting it bare produced `a * -1 < b` for `(a * -1) < b`, which
  // re-parses as a * (-(1 < b)) -- see Walker_unary_right_operand_of_tighter_operator.
  CHECK_EQ_STR(RenderRHS("a * -b"), "a * (-b)");
}

// AC2.1: a unary operand of a *tighter* binary operator must be parenthesized.
// In the xmutil grammar unary minus is the lowest precedence (level 1, same as
// binary +/-) and :NOT: is level 6, so a left unary child of a tighter parent
// would otherwise re-bind the wrong way: e.g. emitting `-a ^ b` re-parses as
// -(a ^ b), which is numerically different ((-2)^2 = 4 vs -(2^2) = -4).
TEST(Walker_unary_operand_of_tighter_operator) {
  CHECK_EQ_STR(RenderRHS("(-a) ^ b"), "(-a) ^ b");
  CHECK_EQ_STR(RenderRHS("(-a) * b"), "(-a) * b");
  CHECK_EQ_STR(RenderRHS("(-a) / b"), "(-a) / b");
  CHECK_EQ_STR(RenderRHS("(:NOT: a) ^ b"), "(:NOT: a) ^ b");
  // At equal precedence a unary minus child of binary +/- re-parses correctly
  // without parens, so it must NOT be over-parenthesized.
  CHECK_EQ_STR(RenderRHS("-a + b"), "-a + b");
  CHECK_EQ_STR(RenderRHS("a + -b"), "a + -b");
}

// A unary RIGHT operand of a tighter binary parent needs parens for exactly the
// same reason the left one does. A unary production reaches rightward over
// everything that binds tighter than its own level, and as a right operand it is
// LAST in the parent's rendered text -- so what follows it comes from the
// grandparent chain. Leaving it bare is only safe when the parent happens to end
// the whole expression, which the walker cannot know locally, so the parent's
// precedence is used as the (provable) worst case on both sides.
//
// Regression guard: the walker used to exempt every right operand outright, so
// `(a * -1) < b` was emitted as `a * -1 < b` and re-parsed as a * (-(1 < b)) --
// unary minus is the grammar's LOWEST level (VYacc.y:76), so it swallowed the
// following comparison.
TEST(Walker_unary_right_operand_of_tighter_operator) {
  CHECK_EQ_STR(RenderRHS("a * (-b)"), "a * (-b)");
  CHECK_EQ_STR(RenderRHS("a / (-b)"), "a / (-b)");
  CHECK_EQ_STR(RenderRHS("a ^ (-b)"), "a ^ (-b)");
  CHECK_EQ_STR(RenderRHS("a :AND: (-b)"), "a :AND: (-b)");
  CHECK_EQ_STR(RenderRHS("a < (-b)"), "a < (-b)");
  CHECK_EQ_STR(RenderRHS("a ^ (:NOT: b)"), "a ^ (:NOT: b)");
  // The unary is at or above the parent's level here, so the parser reduces it
  // before whatever follows and the parens must NOT accrete.
  CHECK_EQ_STR(RenderRHS("a + (-b)"), "a + -b");
  CHECK_EQ_STR(RenderRHS("a - (-b)"), "a - -b");
  CHECK_EQ_STR(RenderRHS("a * (:NOT: b)"), "a * :NOT: b");
  CHECK_EQ_STR(RenderRHS("a :OR: (:NOT: b)"), "a :OR: :NOT: b");
  // The defect in situ: the `*` node is not the whole expression, so the bare
  // form let the unary minus reach past the `*` and capture the comparison.
  CHECK_EQ_STR(RenderRHS("(a * -1) < b"), "a * (-1) < b");
  CHECK_EQ_STR(RenderRHS("(a + -1) < b"), "(a + -1) < b");
  // The contrasting tree: here the unary minus really does own the comparison,
  // and that grouping has its own (different) rendering.
  CHECK_EQ_STR(RenderRHS("a * (-1 < b)"), "a * (-(1 < b))");
}

// The parser folds `'-' <number>` into a single negative ExpressionNumber
// (VensimParse::OperatorExpression), so a negative literal is structurally a
// leaf -- neither a unary nor a binary node -- yet FormatMDLNumber renders the
// sign straight back out and the parser reads that sign as the `'-' exp`
// production again. It therefore has to be grouped exactly like a unary minus.
//
// Regression guard: emitted bare, `(-2) ^ 2` became `-2 ^ 2`, which re-parses as
// -(2 ^ 2) = -4 instead of 4 -- a silently WRONG value, not just a re-grouping.
TEST(Walker_negative_literal_groups_like_unary_minus) {
  CHECK_EQ_STR(RenderRHS("(-2) ^ 2"), "(-2) ^ 2");
  CHECK_EQ_STR(RenderRHS("(-2) * a"), "(-2) * a");
  CHECK_EQ_STR(RenderRHS("(-2) / a"), "(-2) / a");
  CHECK_EQ_STR(RenderRHS("(-1) < b"), "(-1) < b");
  CHECK_EQ_STR(RenderRHS("(-2) :AND: b"), "(-2) :AND: b");
  CHECK_EQ_STR(RenderRHS("a * (-2)"), "a * (-2)");
  CHECK_EQ_STR(RenderRHS("a ^ (-2)"), "a ^ (-2)");
  CHECK_EQ_STR(RenderRHS("((-2) ^ 2) ^ 3"), "((-2) ^ 2) ^ 3");
  // At the unary's own level (binary + and -) the sign re-attaches correctly, so
  // the negative literal stays bare.
  CHECK_EQ_STR(RenderRHS("(-2) + a"), "-2 + a");
  CHECK_EQ_STR(RenderRHS("a + (-2)"), "a + -2");
  CHECK_EQ_STR(RenderRHS("a - (-2)"), "a - -2");
  // :NA: is stored as the -1e38 sentinel but renders as ":NA:", which carries no
  // sign for a following operator to re-attach to -- so it must NOT be treated
  // as a leading minus and must not gain parens.
  CHECK_EQ_STR(RenderRHS(":NA: * a"), ":NA: * a");
  CHECK_EQ_STR(RenderRHS("a ^ :NA:"), "a ^ :NA:");
}

// MINOR: nested unary minus must not emit the fragile `--a`; the inner unary
// operand is parenthesized so the output is `-(-a)`. With a negative literal the
// parens are load-bearing rather than cosmetic: `--2` re-parses as the single
// number 2, because the `'-' <number>` fold fires again and absorbs the outer
// negation, turning -(-2) into a plain 2.
TEST(Walker_nested_unary_minus) {
  CHECK_EQ_STR(RenderRHS("- -a"), "-(-a)");
  CHECK_EQ_STR(RenderRHS("- (-2)"), "-(-2)");
  CHECK_EQ_STR(RenderRHS(":NOT: (-2)"), ":NOT: (-2)");
}

// AC2.1: builtins emit verbatim with their Vensim names and no XMILE expansion
// (no PULSE->IF rewrite, no underscored names, no ComputableName()).
TEST(Walker_builtins_emit_verbatim) {
  CHECK_EQ_STR(RenderRHS("IF THEN ELSE(x > 0, 1, 0)"), "IF THEN ELSE(x > 0, 1, 0)");
  CHECK_EQ_STR(RenderRHS("PULSE(5, 2)"), "PULSE(5, 2)");
  CHECK_EQ_STR(RenderRHS("DELAY FIXED(x, 3, 0)"), "DELAY FIXED(x, 3, 0)");
  CHECK_EQ_STR(RenderRHS("MAX(a, b)"), "MAX(a, b)");
  CHECK_EQ_STR(RenderRHS("MIN(a, b)"), "MIN(a, b)");
}

// AC2.1: logical and comparison operators map to their Vensim spellings.
TEST(Walker_logical_and_comparison) {
  CHECK_EQ_STR(RenderRHS("x > 0 :AND: y < 1"), "x > 0 :AND: y < 1");
  CHECK_EQ_STR(RenderRHS("x > 0 :OR: y < 1"), "x > 0 :OR: y < 1");
  CHECK_EQ_STR(RenderRHS("x <> y"), "x <> y");
  CHECK_EQ_STR(RenderRHS("a <= b"), "a <= b");
  CHECK_EQ_STR(RenderRHS("a >= b"), "a >= b");
  CHECK_EQ_STR(RenderRHS("a = b"), "a = b");
}

TEST(Walker_logical_not) {
  CHECK_EQ_STR(RenderRHS(":NOT: done"), ":NOT: done");
}

// The comparison operators are `%left` in the grammar (VYacc.y:78) but are not
// associative in value, so an equal-precedence RIGHT operand must keep its
// parentheses -- exactly the rule - and / follow. Emitting `a < b < c` for
// `a < (b < c)` re-parses as `(a < b) < c`, a different expression with a
// different value. The LEFT operand is the grouping `%left` already produces,
// so it must NOT gain parens.
TEST(Walker_comparison_grouping) {
  CHECK_EQ_STR(RenderRHS("a < (b < c)"), "a < (b < c)");
  CHECK_EQ_STR(RenderRHS("(a < b) < c"), "a < b < c");
  CHECK_EQ_STR(RenderRHS("a > (b >= c)"), "a > (b >= c)");
  CHECK_EQ_STR(RenderRHS("(a > b) >= c"), "a > b >= c");
  CHECK_EQ_STR(RenderRHS("a = (b = c)"), "a = (b = c)");
  CHECK_EQ_STR(RenderRHS("(a = b) = c"), "a = b = c");
  CHECK_EQ_STR(RenderRHS("a <> (b <> c)"), "a <> (b <> c)");
  CHECK_EQ_STR(RenderRHS("(a <> b) <> c"), "a <> b <> c");
  CHECK_EQ_STR(RenderRHS("a <= (b <> c)"), "a <= (b <> c)");
}

// :AND: and :OR: are genuinely associative in value, but each sits alone on its
// own `%left` level, so an equal-precedence right operand still re-parses into a
// re-grouped tree without parens. The parens are emitted for the structural
// guarantee, not for the arithmetic.
TEST(Walker_logical_grouping) {
  CHECK_EQ_STR(RenderRHS("a :OR: (b :OR: c)"), "a :OR: (b :OR: c)");
  CHECK_EQ_STR(RenderRHS("(a :OR: b) :OR: c"), "a :OR: b :OR: c");
  CHECK_EQ_STR(RenderRHS("a :AND: (b :AND: c)"), "a :AND: (b :AND: c)");
  CHECK_EQ_STR(RenderRHS("(a :AND: b) :AND: c"), "a :AND: b :AND: c");
}

// Across levels the plain precedence comparison decides, and in this grammar the
// levels run (low to high) + - < :OR: < comparison < :AND: < * /. A tighter
// child needs no parens; a looser child under a tighter parent does. Note the
// unusual consequence on the last two lines: a comparison binds TIGHTER than +,
// so `a + b < c` is `a + (b < c)` and it is the arithmetic grouping that has to
// be spelled out.
TEST(Walker_logical_cross_level_precedence) {
  CHECK_EQ_STR(RenderRHS("a < (b :AND: c)"), "a < b :AND: c");
  CHECK_EQ_STR(RenderRHS("(a < b) :AND: c"), "(a < b) :AND: c");
  CHECK_EQ_STR(RenderRHS("a :OR: (b < c)"), "a :OR: b < c");
  CHECK_EQ_STR(RenderRHS("(a :OR: b) < c"), "(a :OR: b) < c");
  CHECK_EQ_STR(RenderRHS("a :OR: (b :AND: c)"), "a :OR: b :AND: c");
  CHECK_EQ_STR(RenderRHS("(a :OR: b) :AND: c"), "(a :OR: b) :AND: c");
  CHECK_EQ_STR(RenderRHS("a + (b < c)"), "a + b < c");
  CHECK_EQ_STR(RenderRHS("(a + b) < c"), "(a + b) < c");
}

// AC2.1: a subscripted variable reference round-trips with its subscript.
TEST(Walker_subscripted_reference) {
  const std::string text =
      "{UTF-8}\r\n"
      "Region: r1, r2 ~~|\r\n"
      "flow[Region] = 5\r\n\t~~|\r\n"
      "y[Region] = flow[Region]\r\n\t~~|\r\n";
  Model *m = roundtrip::ParseVensim(text);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::string out = "<not-found>";
  for (Variable *v : m->GetVariables(nullptr)) {
    if (v->GetName() == "y") {
      MDLGenerator g(m);
      out = g.RenderExpression(v->GetEquation(0)->GetExpression());
      break;
    }
  }
  CHECK_EQ_STR(out, "flow[Region]");
  delete m;
}

// AC2.5: numbers format per Task 4 (whole vs fractional), and :NA: renders as
// the sentinel rather than -1e38.
TEST(Walker_numbers_and_na) {
  CHECK_EQ_STR(RenderRHS("3"), "3");
  CHECK_EQ_STR(RenderRHS("0.5"), "0.5");
  CHECK_EQ_STR(RenderRHS("a + 2.5"), "a + 2.5");
  CHECK_EQ_STR(RenderRHS(":NA:"), ":NA:");
}

// Task 6 / AC2.1, AC2.5: render -> parse -> render idempotence. The walker is a
// normalizer: once an expression is rendered to text, re-parsing and
// re-rendering must reproduce that text exactly. This is a strong property --
// it catches precedence, associativity, and quoting bugs that a single render
// against a hand-written golden could miss, because a wrong-but-self-consistent
// golden would still pass the one-shot test. Idempotence has no such blind spot:
// the second render is checked against the walker's own first output.
TEST(Walker_render_parse_render_idempotent) {
  const char *equations[] = {
      // Representative set from the golden tests above.
      "a + b * c",
      "(a + b) * c",
      "a - (b - c)",
      "a / (b / c)",
      "2 ^ 3 ^ 2",
      "(2 ^ 3) ^ 2",
      "-a * b",
      "a * -b",
      "IF THEN ELSE(x > 0, 1, 0)",
      "PULSE(5, 2)",
      "DELAY FIXED(x, 3, 0)",
      "MAX(a, b)",
      "x > 0 :AND: y < 1",
      "x > 0 :OR: y < 1",
      ":NOT: done",
      "x <> y",
      "a <= b",
      "3",
      "0.5",
      ":NA:",
      // Deeper nesting that the single-render goldens do not exercise.
      "a + b + c + d",
      "a - b + c - d",
      "(a - b) * (c - d)",
      "a / b / c / d",
      "a ^ b ^ c ^ d",
      "MAX(MIN(a, b), c + d)",
      "IF THEN ELSE(a > b :AND: c < d, e * f, g - h)",
      "PULSE(a + b, c / d) * 2",
      "-(a + b) * -(c - d)",
      ":NOT: (a > b) :OR: c = d",
      "((a + b) * c) ^ (d - e)",
      "a * b + c * d - e / f",
      // Unary operands of tighter operators: the parens are load-bearing and
      // must survive a render -> parse -> render round trip unchanged.
      "(-a) ^ b",
      "(-a) * b",
      "(-a) / b",
      "(:NOT: a) ^ b",
      "- -a",
      // Unary and negative-literal operands on the RIGHT of a tighter operator,
      // where the parens are equally load-bearing.
      "a * (-b)",
      "a ^ (:NOT: b)",
      "(a * -1) < b",
      "a * (-1 < b)",
      "(-2) ^ 2",
      "((-2) ^ 2) ^ 3",
      "(-2) :AND: b",
      "a ^ (-2)",
      "- (-2)",
      ":NA: * a",
      // Equal-precedence right operands: the parens are load-bearing for every
      // left-associative level, comparisons and logicals included.
      "a < (b < c)",
      "a = (b <> c)",
      "a + (b + c)",
      "a * (b / c)",
      "a :OR: (b :OR: c)",
      "a :AND: (b :AND: c)",
      // Mixed levels, where the parens must NOT accrete on re-render.
      "(a < b) :AND: c",
      "(a :OR: b) < c",
      "(a + b) < c",
      "a < b :AND: c",
  };
  for (const char *eqn : equations) {
    std::string s1 = RenderRHS(eqn);
    // A parse/lookup failure would silently make the assertion vacuous; guard it.
    CHECK(s1.rfind("<", 0) != 0);
    std::string s2 = RenderRHS(s1);
    CHECK_EQ_STR(s2, s1);
  }
}

// A reference to a quoted identifier must survive the render -> parse -> render
// round trip without the quotes accreting (a double-quoting regression in
// FormatMDLIdent would make the text grow on every pass -- the kind of genuine
// instability idempotence is meant to catch).
TEST(Walker_quoted_ident_reference_idempotent) {
  const std::string text =
      "{UTF-8}\r\n"
      "\"M&Ms\" = 3\r\n\t~~|\r\n"
      "uses = \"M&Ms\" * 2\r\n\t~~|\r\n";
  Model *m = roundtrip::ParseVensim(text);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::string s1 = "<not-found>";
  for (Variable *v : m->GetVariables(nullptr)) {
    if (v->GetName() == "uses") {
      MDLGenerator g(m);
      s1 = g.RenderExpression(v->GetEquation(0)->GetExpression());
      break;
    }
  }
  delete m;
  CHECK_EQ_STR(s1, "\"M&Ms\" * 2");
  // Re-render the rendered text and confirm it is unchanged.
  std::string s2 = RenderRHS(s1);
  CHECK_EQ_STR(s2, s1);
}
