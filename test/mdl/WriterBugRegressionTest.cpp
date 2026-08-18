// Focused regression guards for the writer/comparator bugs found by the Phase 7
// corpus breadth check (CorpusRoundTripTest). Each test reproduces one bug with
// a minimal in-memory .mdl so the specific failure is pinned independently of
// the on-disk corpus fixtures.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/UnitExpression.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

using roundtrip::ExpectCleanRoundTrip;

// Parse the .mdl text and emit it once via the writer, returning the raw bytes.
// Unlike RoundTripDiffs this performs no comparison and no normalization, so a
// caller can inspect the literal byte stream the writer produces.
std::string EmitOnce(const std::string &mdl) {
  Model *m = roundtrip::ParseVensim(mdl);
  if (!m)
    return std::string();
  std::vector<std::string> errs;
  std::string out = m->PrintMDL(errs);
  delete m;
  return out;
}

// The named variable of a model's main namespace, or nullptr.
Variable *FindVar(Model *m, const std::string &name) {
  for (Variable *v : m->GetVariables(nullptr)) {
    if (v->GetName() == name)
      return v;
  }
  return nullptr;
}

// How many of a model's main-namespace variables carry "net flow" in their name.
// Both the modeler's own variables and Variable::MarkStockFlows' artifacts land
// here, which is the point: the inlining contract is that re-parsing the writer's
// output re-synthesizes exactly the SAME net flows, never an extra "_1" copy.
int CountNetFlowVars(Model *m) {
  int n = 0;
  for (Variable *v : m->GetVariables(nullptr)) {
    if (v->GetName().find("net flow") != std::string::npos)
      n++;
  }
  return n;
}

size_t CountCRs(const std::string &s) {
  size_t n = 0;
  for (char c : s)
    if (c == '\r')
      n++;
  return n;
}

// A re-parsable sketch frame with a stock, its flow valve/aux, and two
// connectors. The connector records (lines beginning "1,") carry no width/height
// fields; VensimConnectorElement leaves those members default-initialized. Before
// the fix they held indeterminate stack values, so a connector's width/height
// differed nondeterministically between the original parse and the re-parse and
// the comparator flagged "geometry differs". The default-zero initialization
// makes them deterministically 0 on both sides.
const char *kConnectorSketch =
    "{UTF-8}\r\n"
    "Stock = INTEG(Flow, 0)\r\n\t~~|\r\n"
    "Flow = 1\r\n\t~~|\r\n"
    "FINAL TIME = 10 ~~|\r\n"
    "INITIAL TIME = 0 ~~|\r\n"
    "TIME STEP = 1 ~~|\r\n"
    "SAVEPER = 1 ~~|\r\n"
    "\\\\\\---///\r\n"
    "V300  Do not put anything below this section - it will be ignored\r\n"
    "*View 1\r\n"
    "$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0\r\n"
    "10,1,Stock,300,150,40,20,3,3,0,0,0,0,0,0\r\n"
    "12,2,48,150,150,10,8,0,3,0,0,-1,0,0,0\r\n"
    "1,4,6,1,4,0,0,22,0,0,0,-1--1--1,,1|(250,150)|\r\n"
    "1,5,6,2,100,0,0,22,0,0,0,-1--1--1,,1|(200,150)|\r\n"
    "11,6,0,220,150,6,8,34,3,0,0,1,0,0,0\r\n"
    "10,7,Flow,220,166,19,8,40,3,0,0,-1,0,0,0\r\n"
    "///---\\\\\\\r\n"
    ":L<%^E!@\r\n"
    "9:Current\r\n";

// A stock whose sole inflow is a modeler-authored flow that happens to carry the
// exact name Variable::MarkStockFlows mints when it has to synthesize one. The
// flow has its own units, documentation, group and sketch element -- all of which
// a writer that classified it by NAME threw away along with the equation.
const char *kUserNetFlowMdl =
    R"MDL({UTF-8}
********************************************************
	.Physics
********************************************************~
		Physical flows
	|

S= INTEG (
	S net flow,
		0)
	~	widget
	~	the accumulating stock
	|

S net flow=
	5
	~	widget/Month
	~	the user authored this flow
	|

********************************************************
	.Control
********************************************************~
		Simulation Control Parameters
	|

INITIAL TIME  = 0
	~	Month
	~
	|

FINAL TIME  = 10
	~	Month
	~
	|

TIME STEP  = 1
	~	Month
	~
	|

SAVEPER  = TIME STEP
	~	Month
	~
	|

\\\---///
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Helvetica|10|B|0-0-0|0-0-0|-1--1--1|-1--1--1|96,96,100,0
10,1,S,300,150,40,20,3,3,0,0,0,0,0,0
10,2,S net flow,220,166,19,8,40,3,0,0,-1,0,0,0
///---\\\
:L<%^E!@
15:0,0,0,0,0,0
)MDL";

}  // namespace

// A modeler may legally name a flow "<stock> net flow" -- the very shape
// Variable::MarkStockFlows mints for a stock whose net flow does not decompose
// into a clean +/- of named flows. Classifying by NAME deleted that user
// variable outright: its expression was inlined into the stock's INTEG and its
// standalone entry suppressed, so its units, its documentation and its group
// membership vanished and the emitted sketch referenced a variable the file no
// longer defined. Provenance recorded at the point of synthesis is the only
// thing that distinguishes the two.
TEST(WriterBug_user_authored_net_flow_is_not_deleted) {
  std::string emitted = EmitOnce(kUserNetFlowMdl);
  CHECK(!emitted.empty());

  // The flow keeps a standalone entry carrying its own units and documentation.
  CHECK(emitted.find("S net flow = 5") != std::string::npos);
  CHECK(emitted.find("widget/Month") != std::string::npos);
  CHECK(emitted.find("the user authored this flow") != std::string::npos);
  // ... and the stock still REFERENCES it instead of swallowing its expression.
  CHECK(emitted.find("S = INTEG(S net flow, 0)") != std::string::npos);
  CHECK(emitted.find("S = INTEG(5, 0)") == std::string::npos);

  ExpectCleanRoundTrip(kUserNetFlowMdl);
}

// The same defect seen from the model side: after the round trip the flow must
// still be a variable with its units, its documentation, its group and its place
// among the stock's inflows. Asserted against the RE-PARSED model so it covers
// the whole emit/re-read path, and preceded by the same assertions on the source
// model so a fixture that never had these facts could not make the test pass.
TEST(WriterBug_user_authored_net_flow_keeps_units_doc_and_group) {
  Model *m0 = roundtrip::ParseVensim(kUserNetFlowMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  auto checkFlow = [](Model *m) {
    Variable *flow = FindVar(m, "S net flow");
    CHECK(flow != nullptr);
    if (!flow)
      return;
    CHECK(flow->Units() != nullptr);
    if (flow->Units())
      CHECK_EQ_STR(flow->Units()->GetEquationString(), "widget/Month");
    CHECK(flow->Comment().find("the user authored this flow") != std::string::npos);
    CHECK(flow->GetGroup() != nullptr);
    // "Physics", not ".Physics": the banner lexer drops a leading '.' from a
    // group name (VensimLex.cpp:172-177), so that is the stored spelling on both
    // sides of the trip.
    if (flow->GetGroup())
      CHECK_EQ_STR(flow->GetGroup()->sName, "Physics");

    Variable *stock = FindVar(m, "S");
    CHECK(stock != nullptr);
    if (stock) {
      CHECK(stock->VariableType() == XMILE_Type_STOCK);
      CHECK(stock->Inflows().size() == 1);
      CHECK(stock->Inflows().size() == 1 && stock->Inflows()[0] == flow);
    }
  };

  // Non-vacuity: the source fixture genuinely carries every fact asserted below.
  checkFlow(m0);

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());
  delete m0;

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  checkFlow(m1);
  delete m1;
}

// The "_<n>" spelling exists precisely BECAUSE a variable of the un-suffixed name
// already existed when MarkStockFlows needed a fresh one, so it is just as
// available to a modeler -- and the name-based test matched it too.
TEST(WriterBug_user_authored_net_flow_numeric_suffix_is_not_deleted) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "S = INTEG(S net flow_1, 0)\r\n\t~\twidget\r\n\t~\t\r\n\t|\r\n"
      "S net flow_1 = 5\r\n\t~\twidget/Month\r\n\t~\tauthored, not synthesized\r\n\t|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = TIME STEP\r\n\t~~|\r\n";
  std::string emitted = EmitOnce(mdl);
  CHECK(!emitted.empty());
  CHECK(emitted.find("S net flow_1 = 5") != std::string::npos);
  CHECK(emitted.find("authored, not synthesized") != std::string::npos);
  CHECK(emitted.find("S = INTEG(S net flow_1, 0)") != std::string::npos);
  ExpectCleanRoundTrip(mdl);
}

// Guard against over-correcting: a stock whose net flow is NOT a +/- of named
// flows (here `a * b`) really does get a synthesized carrier variable, and that
// one must still be inlined back into the INTEG and suppressed. Emitting the
// reference instead would leak a reader artifact into the output.
TEST(WriterBug_synthesized_net_flow_is_still_inlined) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 3\r\n\t~~|\r\n"
      "b = 4\r\n\t~~|\r\n"
      "S = INTEG(a * b, 0)\r\n\t~~|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = TIME STEP\r\n\t~~|\r\n";

  // Non-vacuity: the reader must actually have synthesized a carrier here, or
  // "it is still suppressed" would be asserting nothing.
  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;
  CHECK(CountNetFlowVars(m0) == 1);
  delete m0;

  std::string emitted = EmitOnce(mdl);
  CHECK(!emitted.empty());
  CHECK(emitted.find("S = INTEG(a * b, 0)") != std::string::npos);
  CHECK(emitted.find("net flow") == std::string::npos);
  ExpectCleanRoundTrip(mdl);
}

// The arrayed case the inlining exists for. Two per-element stock equations with
// DIFFERENT flows do not reduce to one shared flow list, so MarkStockFlows
// synthesizes a single carrier holding both elements' expressions. Inlining each
// element's expression back into its own INTEG is what makes the re-parse
// synthesize exactly ONE carrier again; emitting the carrier by reference instead
// would make the re-parse mint a duplicate "S net flow_1".
TEST(WriterBug_synthesized_net_flow_arrayed_stock_not_duplicated) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: A, B\r\n\t~~|\r\n"
      "in one[Dim] = 1\r\n\t~~|\r\n"
      "in two[Dim] = 2\r\n\t~~|\r\n"
      "S[A] = INTEG(in one[A], 0)\r\n\t~~|\r\n"
      "S[B] = INTEG(in two[B], 0)\r\n\t~~|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = TIME STEP\r\n\t~~|\r\n";

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;
  // Non-vacuity: exactly one carrier was synthesized for the source model.
  CHECK(CountNetFlowVars(m0) == 1);

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());
  delete m0;

  // The carrier is inlined per element and never named in the output.
  CHECK(regen.find("S[A] = INTEG(in one[A], 0)") != std::string::npos);
  CHECK(regen.find("S[B] = INTEG(in two[B], 0)") != std::string::npos);
  CHECK(regen.find("net flow") == std::string::npos);

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  // ONE carrier again -- not a second "S net flow_1" stacked on top of it.
  CHECK(CountNetFlowVars(m1) == 1);
  Variable *carrier = FindVar(m1, "S net flow");
  CHECK(carrier != nullptr);
  CHECK(FindVar(m1, "S net flow_1") == nullptr);
  delete m1;

  ExpectCleanRoundTrip(mdl);
}

// Provenance has to travel with the variable into a MACRO body too:
// MarkVariableTypes runs MarkStockFlows per macro namespace, and the writer's
// macro loop applies the same suppression. Here the body holds BOTH a genuinely
// synthesized carrier (for `lvl`, whose net flow is `a * 2`) and a modeler's own
// "<stock> net flow" (for `keep`), so a name-based test would suppress the wrong
// one in exactly the namespace the main-model loop never visits.
TEST(WriterBug_user_authored_net_flow_in_macro_body_is_not_deleted) {
  const std::string mdl =
      "{UTF-8}\r\n"
      ":MACRO: SYNTH MACRO(in)\r\n"
      "a = in * 2\r\n\t~~|\r\n"
      "lvl = INTEG(a * 2, 0)\r\n\t~~|\r\n"
      "keep net flow = 7\r\n\t~\twidget/Month\r\n\t~\tauthored inside the macro\r\n\t|\r\n"
      "keep = INTEG(keep net flow, 0)\r\n\t~~|\r\n"
      ":END OF MACRO:\r\n"
      "out = SYNTH MACRO(5)\r\n\t~~|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = TIME STEP\r\n\t~~|\r\n";

  std::string emitted = EmitOnce(mdl);
  CHECK(!emitted.empty());
  // The modeler's macro-body flow survives with its units and documentation...
  CHECK(emitted.find("keep net flow = 7") != std::string::npos);
  CHECK(emitted.find("authored inside the macro") != std::string::npos);
  CHECK(emitted.find("keep = INTEG(keep net flow, 0)") != std::string::npos);
  // ... while the synthesized carrier for `lvl` is still inlined and suppressed.
  CHECK(emitted.find("lvl = INTEG(a * 2, 0)") != std::string::npos);
  CHECK(emitted.find("lvl net flow") == std::string::npos);

  ExpectCleanRoundTrip(mdl);
}

// Connector elements have no width/height in the .mdl format; the base element
// fields must be deterministic (0) so a connector's geometry compares equal
// across a round trip instead of comparing indeterminate memory.
TEST(WriterBug_connector_geometry_is_deterministic) {
  ExpectCleanRoundTrip(kConnectorSketch);
}

// Control variables written with underscores (FINAL_TIME -- the SDEverywhere/PySD
// spelling) are the same control variable as the space form. IsControlVar must
// recognize them under Vensim's identifier equivalence, or the writer emits each
// once as a regular variable AND once in the .Control group, producing a
// duplicate definition (and a stray "Control" group) on re-parse.
TEST(WriterBug_underscore_control_vars_not_duplicated) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "x = RAMP(1, INITIAL_TIME, FINAL_TIME) ~~|\r\n"
      "FINAL_TIME = 5 ~~|\r\n"
      "INITIAL_TIME = 0 ~~|\r\n"
      "TIME_STEP = 1 ~~|\r\n"
      "SAVEPER = TIME_STEP ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// A dimension/subrange definition can carry units and a comment just like any
// other variable; the writer must emit them rather than the empty "~~|"
// shorthand, or a dimension's units and documentation are silently dropped.
TEST(WriterBug_dimension_units_and_comment_preserved) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Region: R1, R2, R3\r\n"
      "\t~\tdmnl\r\n"
      "\t~\ta small dimension with one constant per element\r\n"
      "\t|\r\n"
      "value[Region] = 1, 2, 3 ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// Variables that belong to no group must be emitted before any group banner.
// Vensim assigns a variable to the most recent banner above it, so emitting
// ungrouped variables (e.g. top-level dimension definitions) after the banners
// absorbed them into the last user group on re-parse.
TEST(WriterBug_ungrouped_vars_precede_group_banners) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "Dim: a, b ~~|\r\n"
      "********************************************************\r\n"
      "\t.Matrix\r\n"
      "********************************************************~\r\n"
      "\t\tMatrix Parameters\r\n"
      "\t|\r\n"
      "M[Dim] = 1, 2 ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// A name with multibyte UTF-8 characters is a legal bare identifier; the writer
// must not quote it (the lexer accepts high bytes as identifier characters), or
// the name comes back quoted and the comparator sees a different variable.
TEST(WriterBug_unicode_name_round_trips_unquoted) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "caf\xC3\xA9 level = 2 ~~|\r\n"
      "consumption = caf\xC3\xA9 level / 2 ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// A name whose stored spelling is escaped-and-quoted (interior \" and a literal
// backslash) must come back byte-identical: the writer emits the already-escaped
// core verbatim rather than re-escaping it. The comparator additionally sees
// through benign requoting (a name the reader stored bare that the writer must
// quote), so an unquoted-vs-quoted spelling of the same logical name compares
// equal.
TEST(WriterBug_escaped_quoted_name_round_trips) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "\"weird \\\"quoted\\\" name\" = 3 ~~|\r\n"
      "user = \"weird \\\"quoted\\\" name\" * 2 ~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// A real Vensim .mdl is CRLF, so a multi-line comment stores its internal line
// break as "\r\n" in the lexer's captured comment text. The writer assembles
// the body with bare '\n' and converts the whole document to CRLF in one pass;
// if it does not first drop the comment's pre-existing '\r', that internal break
// becomes '\r' + "\r\n" == "\r\r\n" -- a stray bare CR mid-comment, and the CR
// count grows on every re-emit (the writer is not idempotent on its own output).
// This is checked at the byte level on purpose: ModelComparator::NormalizeComment
// strips every '\r' before comparing, so it cannot see this defect.
TEST(WriterBug_multiline_comment_no_double_cr) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "x = 1\r\n"
      "\t~\tdmnl\r\n"
      "\t~\tfirst line of comment\r\n"
      "second line of comment\r\n"
      "\t|\r\n";
  std::string emitted = EmitOnce(mdl);
  CHECK(!emitted.empty());
  CHECK(emitted.find("\r\r") == std::string::npos);
}

// The writer must be a fixpoint on its own output: emit, re-parse, emit again,
// and the two emissions must be byte-identical. Because every emit converts '\n'
// to CRLF, any residual '\r' the writer fails to canonicalize accretes across
// passes; a stable CR count across the second emit is the operational proof of
// idempotence. Byte-level, not comparator-based, for the same reason as above.
TEST(WriterBug_multiline_comment_emit_is_fixpoint) {
  // The model declares the four control variables explicitly so that both emit
  // passes route them through the same path; otherwise the first pass synthesizes
  // them with the "~~|" shorthand and the second (re-parsed, now real) pass emits
  // the expanded trailer, an unrelated control-var formatting difference that is
  // not what this test is guarding. With control vars fixed, the multi-line
  // comment is the only content whose CRLF handling could drift across passes.
  const std::string mdl =
      "{UTF-8}\r\n"
      "x = 1\r\n"
      "\t~\tdmnl\r\n"
      "\t~\tfirst line of comment\r\n"
      "second line of comment\r\n"
      "\t|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = 1\r\n\t~~|\r\n";
  std::string first = EmitOnce(mdl);
  CHECK(!first.empty());
  std::string second = EmitOnce(first);
  CHECK(!second.empty());
  CHECK_EQ_STR(first, second);
  CHECK(CountCRs(first) == CountCRs(second));
}

// A call with no arguments must keep its parentheses. The Vensim grammar's only
// zero-argument production is `VPTT_function '(' ')'` (VYacc.y) -- there is no
// bare-name form for a function token -- so emitting `RANDOM 0 1` without them
// produced a .mdl that failed to re-parse, and because a syntax error only
// discards the offending equation the whole variable silently vanished while
// the conversion still reported success.
TEST(WriterBug_zero_argument_call_keeps_its_parentheses) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "noise = RANDOM 0 1()\r\n\t~~|\r\n";
  std::string out = EmitOnce(mdl);
  CHECK(out.find("RANDOM 0 1()") != std::string::npos);
  ExpectCleanRoundTrip(mdl);
}
