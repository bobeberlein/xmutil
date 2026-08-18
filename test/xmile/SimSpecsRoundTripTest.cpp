// Sim-spec fidelity tests for the XMILE reader/writer pair: the save interval
// (SAVEPER) and the document-level time_units. Both were silently rewritten by
// a normalization pass -- the reader never looked at the isee:save_interval
// attribute the writer emits, and it stored time_units only as a raw string
// while the writer reads it back as a parsed UnitExpression -- so each test
// here asserts the ACTUAL value that survives, not merely that the document
// re-parses. Each fixture is a hand-written envelope so the wire shape under
// test is visible at the call site.
//
// The precedence between a control variable's own <units> and the document-wide
// time_units gets its own block of tests below. It is easy to write a reader
// that looks correct on a single well-ordered document and is wrong on any
// other: the envelope's children have no required order, and the answer must
// not depend on which of <sim_specs> / <model> the walker reaches first.
//
// The last block covers what happens when <variables> declares one of the four
// control names outright -- an ordinary <aux name="TIME_STEP">, which Stella
// and SDEverywhere exports really do emit. The invariant every one of those
// tests turns on is that a control Variable ends the parse with EXACTLY ONE
// equation, whatever shape and document position the declaration took:
// MDLGenerator::GenerateVariableEntry emits one .Control entry per stored
// equation, so a second equation is a duplicate definition Vensim rejects.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/UnitExpression.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// One <model> element: the <variables> body plus any extra <model> children
// (a <views> section) that a fixture needs.
std::string ModelBlock(const std::string &variables, const std::string &modelExtra) {
  return "<model><variables>" + variables + "</variables>" + modelExtra + "</model>";
}

// A minimal document holding one <sim_specs> block and one <model>, in that
// order -- what every writer emits.
std::string Document(const std::string &simSpecs, const std::string &variables,
                     const std::string &modelExtra = std::string()) {
  return std::string(
             "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
             "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\"\n"
             "       xmlns:isee=\"http://iseesystems.com/XMILE\">\n"
             "  <header><name>sim-specs</name></header>\n"
             "  ") +
         simSpecs + "\n  " + ModelBlock(variables, modelExtra) + "\n</xmile>\n";
}

// The same content with <sim_specs> AFTER <model>. XMILE fixes no order among
// the envelope's children, and a reader that settles time-unit precedence while
// walking <sim_specs> answers differently for the two orders; both must agree.
std::string DocumentSimSpecsLast(const std::string &simSpecs, const std::string &variables,
                                 const std::string &modelExtra = std::string()) {
  return std::string(
             "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
             "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\"\n"
             "       xmlns:isee=\"http://iseesystems.com/XMILE\">\n"
             "  <header><name>sim-specs</name></header>\n"
             "  ") +
         ModelBlock(variables, modelExtra) + "\n  " + simSpecs + "\n</xmile>\n";
}

// Wrap one <sim_specs> block in a minimal, otherwise-constant document.
std::string Envelope(const std::string &simSpecs) {
  return Document(simSpecs, "<aux name=\"x\"><eqn>1</eqn></aux>");
}

// The XMILE text one normalization pass produces. Empty on failure.
std::string RegenXmile(const std::string &xmileText) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmileText, errs);
  if (!m)
    return std::string();
  errs.clear();
  double xscale = 1.0;
  double yscale = 1.0;
  std::string regen = m->PrintXMILE(/*isCompact=*/false, errs, xscale, yscale);
  delete m;
  if (!errs.empty())
    return std::string();
  return regen;
}

// Parse -> PrintXMILE -> parse: the model the next tool in the pipeline sees
// after one normalization pass. Caller owns the result; null on any failure.
Model *NormalizeOnce(const std::string &xmileText) {
  std::string regen = RegenXmile(xmileText);
  if (regen.empty())
    return nullptr;
  std::vector<std::string> errs;
  return xmileroundtrip::ParseXMILE(regen, errs);
}

// Parse -> PrintMDL -> VensimParse: the model a downstream Vensim tool sees.
// Caller owns the result; null on any failure.
Model *ThroughMdl(const std::string &xmileText) {
  std::vector<std::string> errs;
  Model *m0 = xmileroundtrip::ParseXMILE(xmileText, errs);
  if (!m0)
    return nullptr;
  errs.clear();
  std::string mdl = m0->PrintMDL(errs);
  delete m0;
  if (!errs.empty() || mdl.empty())
    return nullptr;
  Model *m1 = new Model();
  VensimParse vp{m1};
  if (!vp.ProcessFile("<test>", mdl.c_str(), mdl.size())) {
    delete m1;
    return nullptr;
  }
  m1->RunPostParsePipeline();
  return m1;
}

// The control-variable values as every writer reads them back: off the
// variable's own constant equation, with a sentinel that no fixture uses so a
// missing variable cannot masquerade as a match.
double ControlValue(Model *m, const char *name) {
  return m->GetConstanValue(name, -12345.0);
}

// The units text a writer would emit for a control variable: the parsed
// UnitExpression, which is what XMILEGenerator's time_units and
// MDLGenerator's units trailer both read (Model::GetUnits). "" when the
// variable carries no parsed units at all.
std::string ControlUnits(Model *m, const char *name) {
  UnitExpression *ue = m->GetUnits(name);
  return ue ? ue->GetEquationString() : std::string();
}

// The OTHER half of a Variable's units: the raw <units> text, which both
// writers fall back to when no UnitExpression parsed. A Variable whose two
// halves disagree has a silently dead raw string, so the tests below assert
// they match rather than only checking the half the writers happen to read.
std::string ControlUnitsRaw(Model *m, const char *name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (!s || s->isType() != Symtype_Variable)
    return std::string();
  return static_cast<Variable *>(s)->GetUnitsString();
}

bool Contains(const std::string &haystack, const char *needle) {
  return haystack.find(needle) != std::string::npos;
}

// Non-overlapping occurrences of needle in haystack.
int CountOccurrences(const std::string &haystack, const std::string &needle) {
  if (needle.empty())
    return 0;
  int n = 0;
  for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
    ++n;
  return n;
}

// '_' folded to ' ', so an emitted entry can be counted without knowing which
// spelling of a control name the source document used. The reader stamps the
// declared spelling onto the Variable (EnsureCanonicalName) and both Vensim and
// this codebase read `TIME_STEP` and `TIME STEP` as one identifier, so the
// count -- not the spelling -- is what the tests below are about.
std::string FoldUnderscores(const std::string &text) {
  std::string out(text);
  for (char &c : out) {
    if (c == '_')
      c = ' ';
  }
  return out;
}

// The .mdl text one conversion produces. Empty on any failure.
std::string MdlText(const std::string &xmileText) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmileText, errs);
  if (!m)
    return std::string();
  errs.clear();
  std::string mdl = m->PrintMDL(errs);
  delete m;
  if (!errs.empty())
    return std::string();
  return mdl;
}

// How many equations a Variable carries. This is the quantity the duplicate-
// control defect is measured in: GenerateVariableEntry emits one .Control entry
// per stored equation, so two equations is two definitions of the same name.
size_t EquationCount(Model *m, const char *name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (!s || s->isType() != Symtype_Variable)
    return 0;
  return static_cast<Variable *>(s)->GetAllEquations().size();
}

std::string ControlComment(Model *m, const char *name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (!s || s->isType() != Symtype_Variable)
    return std::string();
  return static_cast<Variable *>(s)->Comment();
}

// True if any diagnostic is the reader saying it overrode a control-variable
// declaration. Every such message names the offender as a "control variable",
// so its absence is what "the reader took this quietly" means.
bool ReportedControlOverride(const std::vector<std::string> &errs) {
  for (const std::string &e : errs) {
    if (e.find("control variable") != std::string::npos)
      return true;
  }
  return false;
}

const char *kSaveStepElement =
    "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0.25</dt>"
    "<save_step>1</save_step></sim_specs>";

const char *kSaveIntervalAttribute =
    "<sim_specs method=\"euler\" isee:save_interval=\"1\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt></sim_specs>";

// Contradictory input no writer produces; pins down the documented precedence.
const char *kBothSaveSpellings =
    "<sim_specs method=\"euler\" isee:save_interval=\"4\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt><save_step>1</save_step></sim_specs>";

const char *kSaveStepEqualsDt =
    "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0.25</dt>"
    "<save_step>0.25</save_step></sim_specs>";

const char *kSaveStepBelowDt =
    "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0.25</dt>"
    "<save_step>0.125</save_step></sim_specs>";

// Small enough that a fixed-six-decimal encoding ("%.6f") would write it as
// "0.000000" and lose it entirely.
const char *kTinySaveStep =
    "<sim_specs method=\"euler\"><start>0</start><stop>0.01</stop><dt>0.0000001</dt>"
    "<save_step>0.0000005</save_step></sim_specs>";

const char *kMalformedSaveInterval =
    "<sim_specs method=\"euler\" isee:save_interval=\"soon\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt></sim_specs>";

const char *kZeroSaveInterval =
    "<sim_specs method=\"euler\" isee:save_interval=\"0\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt></sim_specs>";

const char *kTimeUnitsDay =
    "<sim_specs method=\"euler\" time_units=\"Day\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt></sim_specs>";

const char *kTimeUnitsCompound =
    "<sim_specs method=\"euler\" time_units=\"Widgets/Day\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt></sim_specs>";

const char *kNoTimeUnits = "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0.25</dt></sim_specs>";

// Exercises both fixes at once on the shape the MDL writer has to render.
const char *kTimeUnitsAndSaveStep =
    "<sim_specs method=\"euler\" time_units=\"Day\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt><save_step>1</save_step></sim_specs>";

// A control variable that spells out its own <units>, contradicting the
// document-level time_units. Not hypothetical: the vendored corpus has four
// xmutil-generated fixtures in this exact shape (test-models/tests/lookups/
// test_lookups.xmile and siblings), where the per-variable <units> carries the
// model's real time unit and time_units carries a stale writer default. The dt
// here matches the aux's own equation so the fixture says nothing about which
// EQUATION wins -- these tests are about units only.
const char *kControlVarWithOwnUnits =
    "<aux name=\"TIME STEP\"><eqn>0.25</eqn><units>Year</units></aux>"
    "<aux name=\"x\"><eqn>1</eqn></aux>";

// A control Variable can also be materialized by a group's <var> membership
// list before <sim_specs> is ever reached; the time unit still has to find it.
const char *kControlGroupView = "<views><group name=\"controls\"><var>INITIAL TIME</var></group></views>";

const char *kPlainVars = "<aux name=\"x\"><eqn>1</eqn></aux>";

// Only a positive SIMULATION PAUSE makes the writer take the branch that
// divides the run length by SAVEPER to derive isee:sim_duration.
const char *kSimulationPauseVars =
    "<aux name=\"SIMULATION PAUSE\"><eqn>1</eqn></aux><aux name=\"x\"><eqn>1</eqn></aux>";

const char *kNanSaveInterval =
    "<sim_specs method=\"euler\" isee:save_interval=\"nan\"><start>0</start><stop>10</stop>"
    "<dt>0.25</dt></sim_specs>";

const char *kNanSaveStepElement =
    "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0.25</dt>"
    "<save_step>nan</save_step></sim_specs>";

// SAVEPER defaults to dt, so this is how a non-finite save interval reaches the
// writer without ever being spelled out as one.
const char *kNanDt = "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>nan</dt></sim_specs>";

const char *kZeroDt = "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0</dt></sim_specs>";

const char *kEmptySaveStep =
    "<sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>0.25</dt><save_step/></sim_specs>";

// ---------------------------------------------------------------------------
// Documents that declare a control name in <variables>.
// ---------------------------------------------------------------------------

// The underbar spelling SDEverywhere and PySD exports use. It is a DIFFERENT
// string from the "TIME STEP" <sim_specs> writes into the namespace, and the
// same symbol -- the namespace folds '_' and ' ' -- which is exactly why a
// literal name compare misses it.
const char *kTimeStepAuxAgreeing = "<aux name=\"TIME_STEP\"><eqn>0.25</eqn></aux><aux name=\"x\"><eqn>1</eqn></aux>";

const char *kTimeStepAuxDisagreeing = "<aux name=\"TIME_STEP\"><eqn>0.5</eqn></aux><aux name=\"x\"><eqn>1</eqn></aux>";

// All four at once, each restating what <sim_specs> already says. SAVEPER is
// the odd one out: kNoTimeUnits spells out no save interval at all, so the
// declaration is the only source there is for it.
const char *kControlAuxesAgreeing =
    "<aux name=\"INITIAL_TIME\"><eqn>0</eqn></aux>"
    "<aux name=\"FINAL_TIME\"><eqn>10</eqn></aux>"
    "<aux name=\"TIME_STEP\"><eqn>0.25</eqn></aux>"
    "<aux name=\"SAVEPER\"><eqn>0.25</eqn></aux>"
    "<aux name=\"x\"><eqn>1</eqn></aux>";

const char *kControlAuxWithUnitsAndDoc =
    "<aux name=\"TIME_STEP\"><eqn>0.25</eqn><units>Year</units><doc>the simulation step</doc></aux>"
    "<aux name=\"x\"><eqn>1</eqn></aux>";

// A control name declared as a stock: the reader would otherwise synthesize an
// INTEG for it, and .Control would carry `TIME STEP = INTEG(rate, 0)`.
const char *kControlStock =
    "<stock name=\"TIME_STEP\"><eqn>0</eqn><inflow>rate</inflow></stock>"
    "<flow name=\"rate\"><eqn>1</eqn></flow>"
    "<aux name=\"x\"><eqn>1</eqn></aux>";

// A control name declared as a standalone graphical function.
const char *kControlGf = "<gf name=\"SAVEPER\"><xpts>0,1</xpts><ypts>0,1</ypts></gf><aux name=\"x\"><eqn>1</eqn></aux>";

// A control name used as a subscript family. Sits in the <model> extra slot
// because that is where a <dimensions> sibling of <variables> goes.
const char *kControlDimensions =
    "<dimensions><dim name=\"TIME STEP\"><elem name=\"a\"/><elem name=\"b\"/></dim></dimensions>";

// <sim_specs> that never states dt. The reader's 1.0 stand-in for the missing
// child is an invented default, not something the document claims, so a
// declaration that does state the value is the only real source there is.
const char *kSimSpecsWithoutDt = "<sim_specs method=\"euler\"><start>0</start><stop>10</stop></sim_specs>";

// True if any diagnostic mentions both the source spelling and the reason.
bool Reported(const std::vector<std::string> &errs, const char *spelling, const char *reason) {
  for (const std::string &e : errs) {
    if (e.find(spelling) != std::string::npos && e.find(reason) != std::string::npos)
      return true;
  }
  return false;
}

}  // namespace

TEST(SimSpecs_save_step_element_is_read) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kSaveStepElement), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 1.0);
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  delete m;
}

TEST(SimSpecs_save_step_survives_round_trip) {
  Model *m = NormalizeOnce(Envelope(kSaveStepElement));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 1.0);
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  delete m;
}

TEST(SimSpecs_isee_save_interval_attribute_is_read) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kSaveIntervalAttribute), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 1.0);
  delete m;
}

TEST(SimSpecs_isee_save_interval_survives_round_trip) {
  Model *m = NormalizeOnce(Envelope(kSaveIntervalAttribute));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 1.0);
  delete m;
}

TEST(SimSpecs_save_step_element_wins_over_isee_attribute) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kBothSaveSpellings), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 1.0);
  delete m;
}

TEST(SimSpecs_contradictory_save_spellings_do_not_oscillate) {
  // One parse cannot distinguish "the element won" from "the two spellings take
  // turns": the writer re-emits SAVEPER as the attribute only, so a reader that
  // preferred the attribute reads 4 on pass 1 of THIS document and 4 forever
  // after, while a reader that mixed the two would drift. Iterate until the text
  // is a fixpoint, checking the value on every pass.
  std::string text = Envelope(kBothSaveSpellings);
  std::string previous;
  for (int pass = 1; pass <= 4; pass++) {
    text = RegenXmile(text);
    CHECK(!text.empty());
    if (text.empty())
      return;
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(text, errs);
    CHECK(m != nullptr);
    if (!m)
      return;
    CHECK(ControlValue(m, "SAVEPER") == 1.0);
    CHECK(ControlValue(m, "TIME STEP") == 0.25);
    delete m;
    // Pass 1 drops the losing spelling, so the text can only settle from 2 on.
    if (pass > 1)
      CHECK(text == previous);
    previous = text;
  }
}

TEST(SimSpecs_save_interval_equal_to_dt_round_trips) {
  // SAVEPER == dt is the reader's default when no save interval is present, so
  // the writer legitimately omits the attribute; the value must still survive.
  std::string regen = RegenXmile(Envelope(kSaveStepEqualsDt));
  CHECK(!regen.empty());
  CHECK(!Contains(regen, "isee:save_interval"));
  Model *m = NormalizeOnce(Envelope(kSaveStepEqualsDt));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 0.25);
  delete m;
}

TEST(SimSpecs_save_interval_below_dt_round_trips) {
  Model *m = NormalizeOnce(Envelope(kSaveStepBelowDt));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 0.125);
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  delete m;
}

TEST(SimSpecs_tiny_save_interval_round_trips) {
  Model *m = NormalizeOnce(Envelope(kTinySaveStep));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 0.0000005);
  delete m;
}

TEST(SimSpecs_malformed_save_interval_falls_back_to_dt) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kMalformedSaveInterval), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 0.25);
  bool reported = false;
  for (const std::string &e : errs) {
    if (e.find("isee:save_interval") != std::string::npos)
      reported = true;
  }
  CHECK(reported);
  delete m;
}

TEST(SimSpecs_nonpositive_save_interval_falls_back_to_dt) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kZeroSaveInterval), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 0.25);
  bool reported = false;
  for (const std::string &e : errs) {
    if (e.find("isee:save_interval") != std::string::npos)
      reported = true;
  }
  CHECK(reported);
  delete m;
}

TEST(SimSpecs_time_units_survive_round_trip) {
  std::string regen = RegenXmile(Envelope(kTimeUnitsDay));
  CHECK(!regen.empty());
  CHECK(Contains(regen, "time_units=\"Day\""));
  Model *m = NormalizeOnce(Envelope(kTimeUnitsDay));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Day");
  delete m;
}

TEST(SimSpecs_compound_time_units_survive_round_trip) {
  std::string regen = RegenXmile(Envelope(kTimeUnitsCompound));
  CHECK(!regen.empty());
  CHECK(Contains(regen, "time_units=\"Widgets/Day\""));
  Model *m = NormalizeOnce(Envelope(kTimeUnitsCompound));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Widgets/Day");
  delete m;
}

TEST(SimSpecs_time_units_attach_to_every_control_variable) {
  // A Vensim .Control group carries the time unit on all four control
  // variables, and the writer's read-back chain (TIME STEP -> FINAL TIME ->
  // INITIAL TIME) expects to find it on any of them.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kTimeUnitsDay), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "FINAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "SAVEPER"), "Day");
  delete m;
}

TEST(SimSpecs_missing_time_units_keeps_writer_default) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kNoTimeUnits), errs);
  CHECK(m != nullptr);
  if (m) {
    CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "");
    delete m;
  }
  std::string regen = RegenXmile(Envelope(kNoTimeUnits));
  CHECK(!regen.empty());
  CHECK(Contains(regen, "time_units=\"Months\""));
}

TEST(SimSpecs_xmile_to_mdl_carries_time_units_and_save_interval) {
  Model *m = ThroughMdl(Envelope(kTimeUnitsAndSaveStep));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 1.0);
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "FINAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "SAVEPER"), "Day");
  delete m;
}

TEST(SimSpecs_xmile_to_mdl_structural_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(Envelope(kTimeUnitsAndSaveStep));
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

// ---------------------------------------------------------------------------
// Precedence: a control variable's own <units> beats the document-wide
// time_units, which is only the default for the controls that stayed silent.
// ---------------------------------------------------------------------------

TEST(SimSpecs_control_variable_units_beat_time_units) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Document(kTimeUnitsDay, kControlVarWithOwnUnits), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Year");
  // The three that spelled out nothing still take the document-wide default.
  CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "FINAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "SAVEPER"), "Day");
  delete m;
}

TEST(SimSpecs_control_variable_units_halves_agree) {
  // Units live on a Variable in two halves -- the raw <units> text and the
  // parsed UnitExpression -- and every writer reads the parsed half first. When
  // the halves name different sources the raw one is silently dead text, which
  // is exactly how the per-variable spelling used to disappear: raw "Year",
  // parsed "Day", both writers emitting "Day".
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Document(kTimeUnitsDay, kControlVarWithOwnUnits), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnitsRaw(m, "TIME STEP"), ControlUnits(m, "TIME STEP"));
  CHECK_EQ_STR(ControlUnitsRaw(m, "TIME STEP"), "Year");
  CHECK_EQ_STR(ControlUnitsRaw(m, "INITIAL TIME"), ControlUnits(m, "INITIAL TIME"));
  CHECK_EQ_STR(ControlUnitsRaw(m, "INITIAL TIME"), "Day");
  delete m;
}

TEST(SimSpecs_control_variable_units_beat_time_units_through_xmile) {
  // XMILE has one time-unit slot per document and the writer marks the control
  // variables unwanted, so the winning unit can only come back as time_units.
  std::string regen = RegenXmile(Document(kTimeUnitsDay, kControlVarWithOwnUnits));
  CHECK(!regen.empty());
  CHECK(Contains(regen, "time_units=\"Year\""));
  CHECK(!Contains(regen, "time_units=\"Day\""));
  Model *m = NormalizeOnce(Document(kTimeUnitsDay, kControlVarWithOwnUnits));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Year");
  delete m;
}

TEST(SimSpecs_control_variable_units_beat_time_units_through_mdl) {
  // The .mdl writer emits a units trailer per control variable, so both the
  // winning per-variable unit and the default survive independently here.
  Model *m = ThroughMdl(Document(kTimeUnitsDay, kControlVarWithOwnUnits));
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Year");
  CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "FINAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnits(m, "SAVEPER"), "Day");
  delete m;
}

TEST(SimSpecs_control_variable_units_win_with_sim_specs_last) {
  // Same document, <sim_specs> after <model>. A reader that decides precedence
  // during the <sim_specs> walk sees no per-variable units at all in the
  // writer-emitted order and every per-variable unit in this one, so the two
  // orders are the test that the decision is genuinely deferred.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(DocumentSimSpecsLast(kTimeUnitsDay, kControlVarWithOwnUnits), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Year");
  CHECK_EQ_STR(ControlUnitsRaw(m, "TIME STEP"), "Year");
  CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
  CHECK_EQ_STR(ControlUnitsRaw(m, "INITIAL TIME"), "Day");
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  delete m;
}

TEST(SimSpecs_time_units_reach_a_group_precreated_control_variable) {
  // <views><group><var>INITIAL TIME</var></group> materializes the control
  // Variable as a placeholder. With <model> first it exists before <sim_specs>
  // runs; either way the deferred time unit has to land on it.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(DocumentSimSpecsLast(kTimeUnitsDay, kPlainVars, kControlGroupView), errs);
  CHECK(m != nullptr);
  if (m) {
    CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
    CHECK_EQ_STR(ControlUnitsRaw(m, "INITIAL TIME"), "Day");
    CHECK(ControlValue(m, "INITIAL TIME") == 0.0);
    delete m;
  }
  errs.clear();
  Model *m2 = xmileroundtrip::ParseXMILE(Document(kTimeUnitsDay, kPlainVars, kControlGroupView), errs);
  CHECK(m2 != nullptr);
  if (m2) {
    CHECK_EQ_STR(ControlUnits(m2, "INITIAL TIME"), "Day");
    CHECK_EQ_STR(ControlUnitsRaw(m2, "INITIAL TIME"), "Day");
    delete m2;
  }
}

// ---------------------------------------------------------------------------
// Non-finite values: NaN is false on every comparison, so a sign test alone
// admits it on the way in and a difference test alone emits it on the way out.
// ---------------------------------------------------------------------------

TEST(SimSpecs_nan_save_interval_falls_back_to_dt) {
  // tinyxml2 converts through sscanf("%lf"), which glibc fills with a NaN for
  // "nan", and `v <= 0.0` is false for a NaN -- so the value used to be stored
  // undiagnosed and SAVEPER became NaN.
  const char *fixtures[] = {kNanSaveInterval, kNanSaveStepElement};
  const char *spellings[] = {"isee:save_interval", "<save_step>"};
  for (int i = 0; i < 2; i++) {
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(Envelope(fixtures[i]), errs);
    CHECK(m != nullptr);
    if (!m)
      continue;
    CHECK(ControlValue(m, "SAVEPER") == 0.25);
    CHECK(Reported(errs, spellings[i], "finite"));
    delete m;
  }
}

TEST(SimSpecs_writer_never_emits_a_nan_save_interval) {
  // SAVEPER defaults to dt, so a non-finite dt arrives at the writer as a
  // non-finite SAVEPER without ever passing the reader's save-interval guard.
  // NaN compares unequal to everything, dt included, so the difference test that
  // decides whether to emit the attribute fires -- and would write a literal
  // isee:save_interval="nan" that this very reader rejects on re-import.
  std::string regen = RegenXmile(Envelope(kNanDt));
  CHECK(!regen.empty());
  CHECK(!Contains(regen, "save_interval"));
}

TEST(SimSpecs_nonfinite_dt_is_diagnosed) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kNanDt), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(Reported(errs, "<dt>", "finite positive"));
  delete m;
}

TEST(SimSpecs_zero_dt_is_diagnosed) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kZeroDt), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(Reported(errs, "<dt>", "finite positive"));
  delete m;
}

TEST(SimSpecs_zero_save_interval_does_not_poison_sim_duration) {
  // isee:sim_duration divides the run length by SAVEPER, and SAVEPER reaches the
  // writer as dt whenever no interval is spelled out -- so the reader's guard on
  // explicit intervals cannot protect this quotient; the writer has to.
  std::string regen = RegenXmile(Document(kZeroDt, kSimulationPauseVars));
  CHECK(!regen.empty());
  CHECK(Contains(regen, "isee:sim_duration=\"0\""));
  CHECK(!Contains(regen, "isee:sim_duration=\"inf\""));
  CHECK(!Contains(regen, "isee:sim_duration=\"nan\""));
}

TEST(SimSpecs_empty_save_step_is_not_reported_as_malformed) {
  // <save_step/> expressed no interval at all. The dt fallback is right; calling
  // it "not a number" sends the reader hunting for a typo that isn't there.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Envelope(kEmptySaveStep), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(ControlValue(m, "SAVEPER") == 0.25);
  CHECK(!Reported(errs, "<save_step>", "is not a number"));
  CHECK(Reported(errs, "<save_step>", "is empty"));
  delete m;
}

// ---------------------------------------------------------------------------
// <variables> declaring a control name. <sim_specs> is the normative source for
// a control's VALUE, so a declaration contributes no equation of its own -- but
// it must not be able to add a second one either, whichever shape it takes and
// whichever side of <model> the <sim_specs> element sits on.
// ---------------------------------------------------------------------------

TEST(SimSpecs_control_declared_as_aux_keeps_one_equation) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(Document(kNoTimeUnits, kTimeStepAuxAgreeing), errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(EquationCount(m, "TIME STEP") == 1);
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  // Restating what <sim_specs> already says is not an error; it is the shape a
  // Vensim .Control group takes when a converter writes the controls out as
  // ordinary variables, and it says nothing the reader has to warn about.
  CHECK(!ReportedControlOverride(errs));
  delete m;
}

TEST(SimSpecs_control_declared_as_aux_emits_one_mdl_entry) {
  const std::string doc = Document(kNoTimeUnits, kTimeStepAuxAgreeing);
  std::string mdl = FoldUnderscores(MdlText(doc));
  CHECK(!mdl.empty());
  CHECK(CountOccurrences(mdl, "TIME STEP = ") == 1);
  // ...and the emission is something the Vensim reader accepts: a duplicate
  // definition is not merely ugly, it is a model Vensim rejects.
  Model *m = ThroughMdl(doc);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(EquationCount(m, "TIME STEP") == 1);
  CHECK(ControlValue(m, "TIME STEP") == 0.25);
  delete m;
}

TEST(SimSpecs_all_four_controls_declared_as_auxes_keep_one_equation_each) {
  const char *kNames[] = {"INITIAL TIME", "FINAL TIME", "TIME STEP", "SAVEPER"};
  const double kValues[] = {0.0, 10.0, 0.25, 0.25};
  const std::string doc = Document(kNoTimeUnits, kControlAuxesAgreeing);

  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m != nullptr);
  if (m) {
    for (int i = 0; i < 4; i++) {
      CHECK(EquationCount(m, kNames[i]) == 1);
      CHECK(ControlValue(m, kNames[i]) == kValues[i]);
    }
    delete m;
  }

  std::string mdl = FoldUnderscores(MdlText(doc));
  CHECK(!mdl.empty());
  for (int i = 0; i < 4; i++)
    CHECK(CountOccurrences(mdl, std::string(kNames[i]) + " = ") == 1);

  Model *m1 = ThroughMdl(doc);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  for (int i = 0; i < 4; i++) {
    CHECK(EquationCount(m1, kNames[i]) == 1);
    CHECK(ControlValue(m1, kNames[i]) == kValues[i]);
  }
  delete m1;
}

TEST(SimSpecs_control_declared_as_aux_is_order_independent) {
  // XMILE fixes no order among the envelope's children. With <sim_specs> first
  // the control already carries its equation when the declaration arrives; with
  // <model> first the declaration arrives at an empty Variable. Both orders have
  // to land on the same single equation holding the same <sim_specs> value.
  const std::string docs[] = {Document(kNoTimeUnits, kControlAuxesAgreeing),
                              DocumentSimSpecsLast(kNoTimeUnits, kControlAuxesAgreeing)};
  for (const std::string &doc : docs) {
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(doc, errs);
    CHECK(m != nullptr);
    if (!m)
      continue;
    CHECK(EquationCount(m, "TIME STEP") == 1);
    CHECK(ControlValue(m, "TIME STEP") == 0.25);
    CHECK(EquationCount(m, "FINAL TIME") == 1);
    CHECK(ControlValue(m, "FINAL TIME") == 10.0);
    delete m;
  }
}

TEST(SimSpecs_control_declared_as_aux_disagreeing_loses_and_is_reported) {
  // A document that says dt is 0.25 in one place and 0.5 in another is wrong
  // about itself. <sim_specs> wins -- it is where every XMILE tool reads the
  // timestep, and it is what the engine's own _dt field already holds -- and the
  // contradiction is surfaced rather than silently resolved.
  const std::string docs[] = {Document(kNoTimeUnits, kTimeStepAuxDisagreeing),
                              DocumentSimSpecsLast(kNoTimeUnits, kTimeStepAuxDisagreeing)};
  for (const std::string &doc : docs) {
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(doc, errs);
    CHECK(m != nullptr);
    if (!m)
      continue;
    CHECK(EquationCount(m, "TIME STEP") == 1);
    CHECK(ControlValue(m, "TIME STEP") == 0.25);
    // The stored equation and the engine field are two halves of the same
    // answer: GetConstanValue reads the equation and falls back to the field, so
    // letting them disagree is a split brain no consumer can detect.
    CHECK(m->dt() == 0.25);
    CHECK(Reported(errs, "TIME STEP", "0.5"));
    delete m;
  }
}

TEST(SimSpecs_control_declared_as_aux_keeps_its_units_and_doc) {
  // The declaration loses its equation, not its documentation: <units> is still
  // the more specific claim than the document-wide time_units, and <doc> is the
  // only place the comment can come from.
  const std::string doc = Document(kTimeUnitsDay, kControlAuxWithUnitsAndDoc);
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m != nullptr);
  if (m) {
    CHECK(EquationCount(m, "TIME STEP") == 1);
    CHECK_EQ_STR(ControlUnits(m, "TIME STEP"), "Year");
    CHECK_EQ_STR(ControlUnitsRaw(m, "TIME STEP"), "Year");
    CHECK_EQ_STR(ControlComment(m, "TIME STEP"), "the simulation step");
    CHECK_EQ_STR(ControlUnits(m, "INITIAL TIME"), "Day");
    delete m;
  }
  Model *m1 = ThroughMdl(doc);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  CHECK(EquationCount(m1, "TIME STEP") == 1);
  CHECK_EQ_STR(ControlUnits(m1, "TIME STEP"), "Year");
  // Substring, not equality: the .mdl comment trailer is tab-indented and the
  // Vensim reader hands the indent back as part of the comment. That is a
  // pre-existing property of the .mdl comment format, not of this precedence.
  CHECK(Contains(ControlComment(m1, "TIME STEP"), "the simulation step"));
  delete m1;
}

TEST(SimSpecs_control_declared_as_stock_is_not_integrated) {
  const std::string doc = Document(kNoTimeUnits, kControlStock);
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m != nullptr);
  if (m) {
    CHECK(EquationCount(m, "TIME STEP") == 1);
    CHECK(ControlValue(m, "TIME STEP") == 0.25);
    CHECK(Reported(errs, "TIME STEP", "control variable"));
    delete m;
  }
  std::string mdl = FoldUnderscores(MdlText(doc));
  CHECK(!mdl.empty());
  CHECK(!Contains(mdl, "TIME STEP = INTEG"));
  CHECK(CountOccurrences(mdl, "TIME STEP = ") == 1);
  Model *m1 = ThroughMdl(doc);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  CHECK(EquationCount(m1, "TIME STEP") == 1);
  CHECK(ControlValue(m1, "TIME STEP") == 0.25);
  delete m1;
}

TEST(SimSpecs_control_declared_as_gf_is_not_a_lookup) {
  const std::string doc = Document(kNoTimeUnits, kControlGf);
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m != nullptr);
  if (m) {
    CHECK(EquationCount(m, "SAVEPER") == 1);
    CHECK(ControlValue(m, "SAVEPER") == 0.25);
    CHECK(Reported(errs, "SAVEPER", "control variable"));
    delete m;
  }
  std::string mdl = FoldUnderscores(MdlText(doc));
  CHECK(!mdl.empty());
  CHECK(CountOccurrences(mdl, "SAVEPER = ") == 1);
  CHECK(!Contains(mdl, "SAVEPER("));
  Model *m1 = ThroughMdl(doc);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  CHECK(EquationCount(m1, "SAVEPER") == 1);
  delete m1;
}

TEST(SimSpecs_control_name_used_as_a_dimension_does_not_duplicate) {
  const std::string doc = Document(kNoTimeUnits, kPlainVars, kControlDimensions);
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m != nullptr);
  if (m) {
    CHECK(EquationCount(m, "TIME STEP") == 1);
    CHECK(ControlValue(m, "TIME STEP") == 0.25);
    CHECK(Reported(errs, "TIME STEP", "control variable"));
    delete m;
  }
  std::string mdl = FoldUnderscores(MdlText(doc));
  CHECK(!mdl.empty());
  CHECK(CountOccurrences(mdl, "TIME STEP = ") == 1);
  CHECK(!Contains(mdl, "TIME STEP: "));
  Model *m1 = ThroughMdl(doc);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  CHECK(EquationCount(m1, "TIME STEP") == 1);
  delete m1;
}

TEST(SimSpecs_control_declared_as_aux_supplies_a_value_sim_specs_omits) {
  // <sim_specs> without a <dt> child says nothing about the timestep; the 1.0
  // the reader falls back to is its own invention. An <aux> that spells the
  // value out is then the only source in the document, so it stands -- and the
  // engine field follows it, as does the SAVEPER that defaults to it.
  const std::string docs[] = {Document(kSimSpecsWithoutDt, kTimeStepAuxAgreeing),
                              DocumentSimSpecsLast(kSimSpecsWithoutDt, kTimeStepAuxAgreeing)};
  for (const std::string &doc : docs) {
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(doc, errs);
    CHECK(m != nullptr);
    if (!m)
      continue;
    CHECK(EquationCount(m, "TIME STEP") == 1);
    CHECK(ControlValue(m, "TIME STEP") == 0.25);
    CHECK(m->dt() == 0.25);
    CHECK(ControlValue(m, "SAVEPER") == 0.25);
    CHECK(!ReportedControlOverride(errs));
    delete m;
  }
  // The value the declaration supplied is what the XMILE writer emits as dt --
  // it has nowhere else to put it, since the control variables are unwanted on
  // the way out.
  std::string regen = RegenXmile(Document(kSimSpecsWithoutDt, kTimeStepAuxAgreeing));
  CHECK(!regen.empty());
  CHECK(Contains(regen, "<dt>0.25</dt>"));
}

TEST(SimSpecs_control_declared_as_aux_does_not_reach_the_xmile_output) {
  // XMILE keeps the sim specs in <sim_specs>, and the writer marks the control
  // variables unwanted, so a control declaration must not come back as an <aux>
  // -- that would be the same duplicate, one format over.
  const std::string doc = Document(kNoTimeUnits, kControlAuxesAgreeing);
  std::vector<std::string> errs;
  Model *m0 = xmileroundtrip::ParseXMILE(doc, errs);
  CHECK(m0 != nullptr);
  if (m0) {
    CHECK(EquationCount(m0, "SAVEPER") == 1);
    delete m0;
  }
  std::string regen = FoldUnderscores(RegenXmile(doc));
  CHECK(!regen.empty());
  CHECK(CountOccurrences(regen, "<aux name=\"TIME STEP\"") == 0);
  CHECK(CountOccurrences(regen, "<aux name=\"SAVEPER\"") == 0);
  CHECK(Contains(regen, "<dt>0.25</dt>"));

  Model *m1 = NormalizeOnce(doc);
  CHECK(m1 != nullptr);
  if (m1) {
    CHECK(EquationCount(m1, "TIME STEP") == 1);
    CHECK(ControlValue(m1, "TIME STEP") == 0.25);
    delete m1;
  }
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(doc);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}
