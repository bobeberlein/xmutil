// XMILE -> XMILE normalization must reach a FIXPOINT: converting a document
// this writer produced has to reproduce it byte for byte.
//
// The structural round-trip tests (RoundTripDiffs) cannot see the two defects
// pinned here, because both preserve model structure exactly while changing the
// text on every pass:
//
//   * The serializer wrapped a construct that already parenthesizes itself
//     ("( IF c THEN a ELSE b )"), so each conversion read the writer's own
//     parens back as a grouping node and emitted one more layer -- paren depth
//     grew linearly, forever.
//   * <variables> came out in SymbolNameSpace hash-bucket order, which is a
//     function of insertion history; each pass re-inserted the symbols in the
//     previous document's order and so permuted the next one.
//
// The tests below therefore assert on the emitted TEXT, and the guard tests at
// the bottom assert that the paren fix drops only parens that were redundant
// -- a normalizer that reached a fixpoint by discarding grouping would be worse
// than one that never converged. The last two guards cover the case that makes
// "is this text already one paren group?" harder than counting: a QUOTED name
// may carry a paren of its own, which is a name character to the lexer and not
// grouping at all.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "../mdl/ModelComparator.h"
#include "RoundTrip.h"

namespace {

std::string Wrap(const std::string &variables) {
  return "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
         "  <header><name>fixpoint</name></header>\n"
         "  <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
         "  <model><variables>\n" +
         variables +
         "  </variables></model>\n"
         "</xmile>\n";
}

size_t CountChar(const std::string &s, char c) {
  size_t n = 0;
  for (char ch : s)
    if (ch == c)
      n++;
  return n;
}

// Normalize `text` `passes` times, collecting each emission. Returns false if
// any pass fails to parse or print, having FIRST recorded that failure as a
// failed check: every call site bails out on false, and the harness reports a
// test that ran no checks as a pass -- so leaving the report to the caller
// turned an unconvertible model into a green test that verified nothing.
bool Iterate(const std::string &text, int passes, std::vector<std::string> &out) {
  std::string current = text;
  for (int i = 0; i < passes; i++) {
    std::vector<std::string> errs;
    current = xmileroundtrip::NormalizeXMILE(current, errs);
    const bool normalized = !current.empty();
    if (!normalized)
      printf("  pass %d failed: %s\n", i + 1, errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    CHECK(normalized);
    if (!normalized)
      return false;
    out.push_back(current);
  }
  return true;
}

// Parse Vensim .mdl text through the reader and the shared post-parse
// pipeline, as the conversion entry points do. Returns nullptr on failure.
// Only the .mdl reader stores a quoted identifier with its quotes (the XMILE
// reader strips them), so a name whose quoting is load-bearing has to enter
// the model from this side.
Model *ParseMDL(const std::string &text) {
  Model *m = new Model();
  VensimParse vp{m};
  if (!vp.ProcessFile("<test>", text.c_str(), text.size())) {
    delete m;
    return nullptr;
  }
  m->RunPostParsePipeline();
  return m;
}

std::string EmitXMILE(Model *m) {
  std::vector<std::string> errs;
  return m->PrintXMILE(/*isCompact=*/false, errs, /*xscale=*/1.0, /*yscale=*/1.0);
}

std::string EmitMDL(Model *m) {
  std::vector<std::string> errs;
  return m->PrintMDL(errs);
}

// The <eqn> body of the first variable named `name` in an emitted document.
std::string EqnOf(const std::string &doc, const std::string &name) {
  const size_t at = doc.find("name=\"" + name + "\"");
  if (at == std::string::npos)
    return "<no such variable: " + name + ">";
  const size_t open = doc.find("<eqn>", at);
  if (open == std::string::npos)
    return "<no eqn for: " + name + ">";
  const size_t close = doc.find("</eqn>", open);
  if (close == std::string::npos)
    return "<unterminated eqn for: " + name + ">";
  return doc.substr(open + 5, close - open - 5);
}

}  // namespace

// Defect 1. An if/then/else parenthesizes its own output, so the paren the
// writer emits is read back as a grouping node and re-wrapped next pass. Five
// passes make unbounded growth unmistakable: before the fix the count went
// 2, 3, 4, 5, 6 on this one equation.
TEST(Fixpoint_if_then_else_does_not_accumulate_parens) {
  const std::string model = Wrap(
      "    <aux name=\"cond\"><eqn>1</eqn></aux>\n"
      "    <aux name=\"picked\"><eqn>IF cond > 0 THEN 1 ELSE 0</eqn></aux>\n");

  std::vector<std::string> passes;
  if (!Iterate(model, 5, passes))
    return;

  const size_t first = CountChar(passes[0], '(');
  for (size_t i = 1; i < passes.size(); i++) {
    const size_t n = CountChar(passes[i], '(');
    if (n != first)
      printf("  pass %zu has %zu '(' vs %zu in pass 1; eqn: %s\n", i + 1, n, first, EqnOf(passes[i], "picked").c_str());
    CHECK(n == first);
  }
  // The equation is exactly one if/then/else, so the writer's own wrapper is
  // the only paren pair it may ever carry. EqnOf returns the raw XML text, in
  // which tinyxml2 has entity-escaped the comparison operator.
  CHECK_EQ_STR(EqnOf(passes[0], "picked"), "( IF cond &gt; 0 THEN 1 ELSE 0 )");
  for (size_t i = 1; i < passes.size(); i++)
    CHECK_EQ_STR(passes[i], passes[0]);
}

// The unary NOT's operand sits in the right slot with an empty left slot, and
// its operator text used to carry a leading separator regardless -- doubling
// against the space the enclosing "( IF " had already written.
TEST(Fixpoint_unary_not_is_spelled_with_single_spaces) {
  const std::string model = Wrap(
      "    <aux name=\"flag\"><eqn>0</eqn></aux>\n"
      "    <aux name=\"inverted\"><eqn>IF NOT flag THEN 1 ELSE 0</eqn></aux>\n");

  std::vector<std::string> passes;
  if (!Iterate(model, 3, passes))
    return;
  CHECK_EQ_STR(EqnOf(passes[0], "inverted"), "( IF not flag THEN 1 ELSE 0 )");
  for (size_t i = 1; i < passes.size(); i++)
    CHECK_EQ_STR(passes[i], passes[0]);
}

// Defect 2. Variables declared out of alphabetical order must come back in one
// canonical order, not in whatever order the namespace hash happened to hold
// them -- an order that changes with insertion history, so it oscillated from
// pass to pass.
TEST(Fixpoint_variable_emission_order_is_canonical) {
  const std::string model = Wrap(
      "    <aux name=\"zebra\"><eqn>1</eqn></aux>\n"
      "    <aux name=\"apple\"><eqn>2</eqn></aux>\n"
      "    <aux name=\"mango\"><eqn>3</eqn></aux>\n"
      "    <aux name=\"banana\"><eqn>4</eqn></aux>\n"
      "    <aux name=\"cherry\"><eqn>5</eqn></aux>\n");

  std::vector<std::string> passes;
  if (!Iterate(model, 5, passes))
    return;

  const char *const expected[] = {"apple", "banana", "cherry", "mango", "zebra"};
  size_t at = 0;
  for (const char *name : expected) {
    const size_t found = passes[0].find(std::string("name=\"") + name + "\"");
    if (found == std::string::npos || found < at)
      printf("  '%s' is not in canonical position in the emitted document\n", name);
    CHECK(found != std::string::npos && found >= at);
    if (found != std::string::npos)
      at = found;
  }
  for (size_t i = 1; i < passes.size(); i++)
    CHECK_EQ_STR(passes[i], passes[0]);
}

// Guard: the paren suppression must only drop parens that were redundant. A
// grouping that changes the association of the surrounding operators has to
// survive, because this serializer emits binary operators with no parentheses
// of their own -- losing the node would silently reassociate the expression.
TEST(Fixpoint_grouping_parens_survive_normalization) {
  const std::string model = Wrap(
      "    <aux name=\"a\"><eqn>1</eqn></aux>\n"
      "    <aux name=\"b\"><eqn>2</eqn></aux>\n"
      "    <aux name=\"c\"><eqn>3</eqn></aux>\n"
      "    <aux name=\"grouped\"><eqn>(a + b) * c</eqn></aux>\n"
      "    <aux name=\"nested\"><eqn>a / (b - (c + 1))</eqn></aux>\n");

  std::vector<std::string> passes;
  if (!Iterate(model, 3, passes))
    return;
  CHECK_EQ_STR(EqnOf(passes[0], "grouped"), "(a+b)*c");
  CHECK_EQ_STR(EqnOf(passes[0], "nested"), "a/(b-(c+1))");
  for (size_t i = 1; i < passes.size(); i++)
    CHECK_EQ_STR(passes[i], passes[0]);
}

// Guard: a paren pair that wraps something already delimited collapses to one
// pair and then stops. Redundant nesting in a hand-written source is where the
// suppression is allowed to change the text -- once.
TEST(Fixpoint_redundant_nested_parens_collapse_once) {
  const std::string model = Wrap(
      "    <aux name=\"a\"><eqn>1</eqn></aux>\n"
      "    <aux name=\"b\"><eqn>2</eqn></aux>\n"
      "    <aux name=\"doubled\"><eqn>((a + b))</eqn></aux>\n");

  std::vector<std::string> passes;
  if (!Iterate(model, 3, passes))
    return;
  CHECK_EQ_STR(EqnOf(passes[0], "doubled"), "(a+b)");
  for (size_t i = 1; i < passes.size(); i++)
    CHECK_EQ_STR(passes[i], passes[0]);
}

// The .mdl body shared by the two quoted-name guards below. `"x("`, `"y)"` and
// `"it's("` are ONE name each: the paren is a character of the name, and the
// lexer that reads this text back never sees it as grouping. The controls
// and the (empty) sketch frame are spelled out because the .mdl writer always
// materializes both, so a model that left them implicit would not compare equal
// to its own round trip for reasons that have nothing to do with parentheses.
const char *const kQuotedParenNameMdl =
    R"MDL({UTF-8}
a = 3 ~~|
b = 5 ~~|
"x(" = 7 ~~|
"y)" = 11 ~~|
"it's(" = 13 ~~|
k = 2^((a+"x(")*(b+"y)")) ~~|
m = ( IF THEN ELSE("x(">1,1,0) ) ~~|
t = ( IF THEN ELSE("it's(">1,1,0) ) ~~|
INITIAL TIME = 0 ~~|
FINAL TIME = 100 ~~|
TIME STEP = 1 ~~|
SAVEPER = 1 ~~|
\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
///---\\\
)MDL";

// Guard: a grouping whose child renders with an unmatched paren INSIDE a
// quoted name must survive. One name contributes a '(' and the other a ')',
// so a scan that counted every paren as grouping found the depth back at zero
// on the last character of `(a+"x(")*(b+"y)")` and dropped the enclosing pair
// -- silently rewriting 2^((a+X)*(b+Y)) as (2^(a+X))*(b+Y), a different tree
// with a different value.
TEST(Fixpoint_paren_in_quoted_name_does_not_drop_grouping) {
  Model *m0 = ParseMDL(kQuotedParenNameMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  const std::string doc = EmitXMILE(m0);
  CHECK(!doc.empty());
  CHECK_EQ_STR(EqnOf(doc, "k"), "2^((a+\"x(\")*(b+\"y)\"))");

  // Structural, not textual: read the emission back and render it as .mdl,
  // whose writer discards every parser paren node and re-derives parentheses
  // from operator precedence. That canonical form states the association
  // outright, so it catches a reassociation even if the XMILE text is spelled
  // differently.
  std::vector<std::string> errs;
  Model *m1 = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }
  const std::string canonical = EmitMDL(m1);
  const bool grouping_survived = canonical.find("k = 2 ^ ((a + \"x(\") * (b + \"y)\"))") != std::string::npos;
  if (!grouping_survived)
    printf("  canonical .mdl lost the grouping; emitted XMILE was: %s\n", EqnOf(doc, "k").c_str());
  CHECK(grouping_survived);

  const std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  for (const std::string &d : diffs)
    printf("  model diff after the XMILE hop: %s\n", d.c_str());
  CHECK(diffs.empty());

  delete m1;
  delete m0;
}

// Guard, same predicate and the opposite failure: a single unmatched paren in
// a quoted name made every rendering look unbalanced, so the redundant layer
// the suppression exists to remove came back on every pass. `m` is a source
// paren wrapped around IF THEN ELSE, which parenthesizes its own output -- it
// must collapse to one pair and stay there.
TEST(Fixpoint_paren_in_quoted_name_does_not_regrow_parens) {
  Model *m0 = ParseMDL(kQuotedParenNameMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  const std::string first = EmitXMILE(m0);
  CHECK(!first.empty());
  CHECK_EQ_STR(EqnOf(first, "m"), "( IF \"x(\" &gt; 1 THEN 1 ELSE 0 )");
  // `t` pins the ORDER of the two rules the predicate applies: a quoted span is
  // skipped before the apostrophe bail-out, so `"it's("` -- an apostrophe and
  // an unmatched paren, both inside the quotes -- still collapses. A blunt
  // "any quote or apostrophe means give up" rule would answer "keep" here and
  // leave this construct growing a layer per conversion, which is the very
  // defect the suppression exists to fix.
  CHECK_EQ_STR(EqnOf(first, "t"), "( IF \"it's(\" &gt; 1 THEN 1 ELSE 0 )");

  // The cycle that carries this model is XMILE -> .mdl -> XMILE, not
  // XMILE -> XMILE: the XMILE reader drops the quotes around an identifier and
  // the XMILE writer does not put them back, so `x(` would be re-read as a
  // call on the next pass. The .mdl writer does re-quote it, so a turn through
  // Vensim is what closes the loop -- and a paren layer that grew per
  // conversion would still show up as a difference between the two emissions.
  std::vector<std::string> errs;
  Model *m1 = xmileroundtrip::ParseXMILE(first, errs);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }
  const std::string viaMdl = EmitMDL(m1);
  Model *m2 = ParseMDL(viaMdl);
  CHECK(m2 != nullptr);
  if (m2) {
    CHECK_EQ_STR(EmitXMILE(m2), first);
    delete m2;
  }
  delete m1;
  delete m0;
}

// Guard, the apostrophe half of the same predicate. An apostrophe reaches the
// rendered text as a bare-name character (`don't`) and as a Vensim
// string-literal delimiter (`'f.xlsx'`), and those two readings disagree about
// whether a following paren is grouping -- so the predicate used to give up on
// the character outright. Giving up means "keep the paren", which on a
// construct that already parenthesizes itself is the unbounded growth the
// suppression exists to stop.
//
// A LONE apostrophe cannot be a literal delimiter -- there is nothing to close
// it -- so that reading is not ambiguous at all and the scan settles it. The
// cycle runs through .mdl for the same reason the quoted-paren guards do: the
// XMILE writer does not re-quote a name, and `don't` bare is a postfix
// apostrophe the XMILE equation grammar rejects.
const char *const kApostropheNameMdl =
    R"MDL({UTF-8}
a = 3 ~~|
"don't" = 7 ~~|
z = ( IF THEN ELSE("don't">1,1,0) ) ~~|
INITIAL TIME = 0 ~~|
FINAL TIME = 100 ~~|
TIME STEP = 1 ~~|
SAVEPER = 1 ~~|
\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
///---\\\
)MDL";

TEST(Fixpoint_apostrophe_in_a_name_does_not_regrow_parens) {
  Model *m0 = ParseMDL(kApostropheNameMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  const std::string first = EmitXMILE(m0);
  delete m0;
  CHECK(!first.empty());
  CHECK_EQ_STR(EqnOf(first, "z"), "( IF \"don't\" &gt; 1 THEN 1 ELSE 0 )");

  std::vector<std::string> errs;
  Model *m1 = xmileroundtrip::ParseXMILE(first, errs);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  const std::string viaMdl = EmitMDL(m1);
  delete m1;
  Model *m2 = ParseMDL(viaMdl);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  CHECK_EQ_STR(EmitXMILE(m2), first);
  delete m2;
}

// The other reading: a PAIR of apostrophes really can delimit a literal, and a
// literal may carry a paren that is not grouping. Both readings of
// `GET DIRECT DATA('f(x).xlsx',...)` agree that the rendering is one enclosing
// group -- the literal's parens are balanced -- so the enclosing pair the
// function writes for itself must not be doubled.
TEST(Fixpoint_apostrophe_delimited_literal_does_not_regrow_parens) {
  const char *const kLiteralMdl =
      R"MDL({UTF-8}
a = 3 ~~|
w = ( IF THEN ELSE(a>1,GET DIRECT DATA('f(x).xlsx','A','b','c'),0) ) ~~|
INITIAL TIME = 0 ~~|
FINAL TIME = 100 ~~|
TIME STEP = 1 ~~|
SAVEPER = 1 ~~|
\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
///---\\\
)MDL";

  Model *m0 = ParseMDL(kLiteralMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;
  const std::string eqn = EqnOf(EmitXMILE(m0), "w");
  delete m0;
  // One enclosing pair from the function's own rendering, not two.
  CHECK(eqn.compare(0, 2, "((") != 0);
  if (eqn.compare(0, 2, "((") == 0)
    printf("  emitted eqn: %s\n", eqn.c_str());
}
