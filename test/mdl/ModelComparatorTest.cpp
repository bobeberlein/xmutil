#include <string>

#include "../../src/Model.h"
#include "../TestHarness.h"
#include "ModelComparator.h"
#include "RoundTrip.h"

namespace {

// Build a one-equation model whose single aux `c` has the given RHS, plus the
// referenced operands as constants so the model parses cleanly. The comparator
// keys variables by name, so two such models differ ONLY in c's RHS expression
// -- which is exactly what the ExpressionsEqual guard tests need to probe.
std::string OneAux(const std::string &rhs) {
  return "{UTF-8}\r\n"
         "a = 1\r\n\t~~|\r\n"
         "b = 2\r\n\t~~|\r\n"
         "x = 3\r\n\t~~|\r\n"
         "c = " +
         rhs + "\r\n\t~~|\r\n";
}

// True iff the comparator reports `c`'s RHS expressions as equivalent across two
// models that differ only in c's RHS. Drives ExpressionsEqual through the public
// Compare API (an emit-independent structural check, per the Phase 3 design).
bool RhsEquivalent(const std::string &rhsA, const std::string &rhsB) {
  Model *a = roundtrip::ParseVensim(OneAux(rhsA));
  Model *b = roundtrip::ParseVensim(OneAux(rhsB));
  if (!a || !b) {
    delete a;
    delete b;
    return false;
  }
  std::vector<std::string> diffs = ModelComparator::Compare(a, b);
  bool equal = diffs.empty();
  delete a;
  delete b;
  return equal;
}

}  // namespace

// AC3.5 guard: the structural comparator must distinguish a different operator.
// `a + b` and `a - b` share operands and structure but differ in the operator,
// so the comparator must report them as NON-equivalent -- otherwise a writer bug
// that swapped + for - would pass a round-trip test vacuously.
TEST(Comparator_distinguishes_operator) {
  CHECK(!RhsEquivalent("a + b", "a - b"));
  CHECK(RhsEquivalent("a + b", "a + b"));
}

// AC3.5 guard: a different builtin function name is a real difference. MAX and
// MIN parse as EXPTYPE_Function with the same arity and arguments, so only the
// function-name branch of ExpressionsEqual can tell them apart.
TEST(Comparator_distinguishes_function_name) {
  CHECK(!RhsEquivalent("MAX(a, b)", "MIN(a, b)"));
  CHECK(RhsEquivalent("MAX(a, b)", "MAX(a, b)"));
}

// AC3.5 guard: same function, different argument is a real difference (exercises
// the recursive per-argument compare inside the function branch).
TEST(Comparator_distinguishes_function_argument) {
  CHECK(!RhsEquivalent("MAX(a, b)", "MAX(a, x)"));
}

// AC3.5 guard: a reference to a different named lookup is a real difference. An
// undefined `name(input)` parses as a named-lookup call (EXPTYPE_Lookup), so
// `f(x)` vs `g(x)` exercises the lookup branch's lookup-variable-name compare.
TEST(Comparator_distinguishes_lookup_target) {
  CHECK(!RhsEquivalent("f(x)", "g(x)"));
  CHECK(RhsEquivalent("f(x)", "f(x)"));
}

// AC3.5 guard: parentheses that do not change structure are invisible to the
// comparator. `(a)` and `a` are the same expression, so they compare equivalent
// (the comparator unwraps ExpressionParen exactly like the writer's walker).
TEST(Comparator_paren_insensitive_for_redundant_parens) {
  CHECK(RhsEquivalent("(a)", "a"));
  CHECK(RhsEquivalent("(a + b)", "a + b"));
  CHECK(RhsEquivalent("a * (b + c)", "a * (b + c)"));
}

// AC3.5 guard: parentheses that DO change structure are real differences.
// `(a + b) * c` (a product of a sum and c) and `a + b * c` (a sum of a and a
// product) have different ASTs, so the comparator must report them NON-equal.
// This is the critical "paren-insensitive but structure-sensitive" property.
TEST(Comparator_distinguishes_meaningful_grouping) {
  CHECK(!RhsEquivalent("(a + b) * c", "a + b * x"));
  // Same operands, only the grouping differs.
  CHECK(!RhsEquivalent("(a + b) * x", "a + b * x"));
}

// The comparator must see through differing numeric formatting that denotes the
// same value, and must reject a genuinely different constant.
TEST(Comparator_number_tolerance_and_difference) {
  CHECK(RhsEquivalent("1", "1.0"));
  CHECK(!RhsEquivalent("1", "2"));
}
