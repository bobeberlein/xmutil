#include <string>

#include "../../src/Model.h"
#include "../../src/Symbol/Equation.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/UnitExpression.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "ModelComparator.h"
#include "RoundTrip.h"

namespace {

// The units text a variable carries, mirroring the writer's units logic
// (XMILEGenerator / ModelComparator::UnitsText): the parsed UnitExpression's
// equation string when present, otherwise the raw units string.
std::string UnitsTextOf(Variable *var) {
  UnitExpression *un = var->Units();
  if (un)
    return un->GetEquationString();
  return var->GetUnitsString();
}

// A variable's comment with its leading separator whitespace removed, matching
// the normalization the writer applies (MDLGenerator::UnitsCommentTrailer): the
// Vensim lexer captures the whitespace between the second '~' and the comment
// text as part of the stored comment, and the writer drops that leading
// whitespace on emit so it does not accrete across cycles. Comparing the
// CONTENT (not the separator) is what makes a round-trip assertion meaningful.
std::string CommentContent(Variable *var) {
  std::string comment = var->Comment();
  size_t start = comment.find_first_not_of(" \t\r\n");
  if (start == std::string::npos)
    return std::string();
  return comment.substr(start);
}

// Look up a variable by name in the model's main namespace (mirrors
// Model::GetConstanValue's lookup). Returns nullptr if the name is unbound or
// resolves to a non-variable symbol. The model exposes no by-name variable
// accessor, so the SAVEPER equation check below reaches through the namespace.
Variable *FindVariable(Model *m, const char *name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (s && s->isType() == Symtype_Variable)
    return static_cast<Variable *>(s);
  return nullptr;
}

// The minimal sketch frame the writer emits (MDLGenerator::GenerateSketch). A
// fixture that wants a re-parsable settings section must reproduce it verbatim,
// because the Vensim parser only reads the trailing :L block after the
// \\\---/// ... ///---\\\ sketch frame (VensimParse.cpp:277-358). Defined once
// here so every fixture's settings section is reached the same way the writer's
// output is.
const char *kSketchFrame =
    "\\\\\\---///\r\n"
    "V300  Do not put anything below this section - it will be ignored\r\n"
    "*View 1\r\n"
    "$192-192-192,0,Helvetica|10|B|0-0-0|0-0-0|-1--1--1|-1--1--1|96,96,100,0\r\n"
    "///---\\\\\\\r\n";

// The :L settings marker line. The literal DEL byte (0x7F) after ":L" is what
// VensimParse.cpp:325 scans for; without it the 15:/22: lines are never read.
const char *kSettingsMarker = ":L\x7F<%^E!@\r\n";

using roundtrip::ExpectCleanRoundTrip;

// A complete .mdl: the four control equations, the sketch frame, the :L marker,
// the given 15: integration line, and any extra settings lines (e.g. 22:). The
// control equations use SAVEPER = TIME STEP so the SAVEPER fixture exercises the
// non-constant (variable-reference) case that GetConstanValue cannot see.
std::string ControlModel(const char *integrationLine, const std::string &extraSettings = "") {
  return std::string("{UTF-8}\r\n") +
         "INITIAL TIME = 0\r\n\t~~|\r\n"
         "FINAL TIME = 100\r\n\t~~|\r\n"
         "TIME STEP = 0.25\r\n\t~~|\r\n"
         "SAVEPER = TIME STEP\r\n\t~~|\r\n" +
         kSketchFrame + kSettingsMarker + extraSettings + integrationLine;
}

}  // namespace

// RED-driver / non-vacuity guard for the unit-equivalence comparison: two models
// that differ ONLY in their 22: unit-equivalence payloads must be reported as
// different by the comparator. Without the UnitEquivs comparison this passes
// vacuously (the comparator would ignore the difference), so a writer that
// dropped or reordered the 22: lines would round-trip green undetected.
TEST(Comparator_distinguishes_unit_equivs) {
  const std::string base = std::string("{UTF-8}\r\n") + "a = 1\r\n\t~~|\r\n" + kSketchFrame + kSettingsMarker;

  const std::string withHour = base + "22:Hour,Hours\r\n";
  const std::string withDollar = base + "22:$,Dollar,Dollars\r\n";
  const std::string withNone = base;

  Model *mHour = roundtrip::ParseVensim(withHour);
  Model *mDollar = roundtrip::ParseVensim(withDollar);
  Model *mNone = roundtrip::ParseVensim(withNone);
  CHECK(mHour != nullptr);
  CHECK(mDollar != nullptr);
  CHECK(mNone != nullptr);

  // Confirm the fixtures genuinely parsed the 22: lines (non-vacuous): the
  // comparison only means something if the payloads actually landed in
  // UnitEquivs().
  CHECK(mHour->UnitEquivs().size() == 1);
  CHECK(mDollar->UnitEquivs().size() == 1);
  CHECK(mNone->UnitEquivs().empty());

  // A present-vs-absent equivalence and two different equivalences must both be
  // reported as differences.
  CHECK(!ModelComparator::Compare(mHour, mNone).empty());
  CHECK(!ModelComparator::Compare(mHour, mDollar).empty());
  // Identical equivalences compare clean.
  CHECK(ModelComparator::Compare(mHour, mHour).empty());

  delete mHour;
  delete mDollar;
  delete mNone;
}

// RED-driver / non-vacuity guard for the group comparison: two models that
// differ ONLY in their group structure (the name of the banner that owns a
// variable, or the membership of a banner) must be reported as different by the
// comparator. Without CompareGroups this passes vacuously, so a writer that
// dropped or renamed a group would round-trip green undetected. The two models
// are otherwise identical (same variables, same equations), so any reported diff
// can only come from CompareGroups.
TEST(Comparator_distinguishes_groups) {
  // Build a banner section with a given group name owning `x`, plus a second
  // banner owning `y`. Same variables/equations across all fixtures; only the
  // grouping changes.
  auto banner = [](const char *name) {
    return std::string("********************************************************\r\n") + "\t" + name +
           "\r\n********************************************************~\r\n\t\t|\r\n";
  };
  auto model = [&](const char *first_group, const char *second_group) {
    return std::string("{UTF-8}\r\n") + banner(first_group) + "x = 1\r\n\t~~|\r\n" + banner(second_group) +
           "y = 2\r\n\t~~|\r\n" + kSketchFrame + kSettingsMarker + "15:0,0,0,0,0,0\r\n";
  };

  // Baseline: x in "Inputs", y in "Outputs".
  Model *mBase = roundtrip::ParseVensim(model("Inputs", "Outputs"));
  // Same variables, but x's owning banner is renamed "Sources" (a group-name
  // difference).
  Model *mRenamed = roundtrip::ParseVensim(model("Sources", "Outputs"));
  // Same two group names, but both x and y live under "Inputs" and "Outputs" is
  // empty -- a membership difference (y moved from Outputs to Inputs).
  Model *mRegrouped =
      roundtrip::ParseVensim(std::string("{UTF-8}\r\n") + banner("Inputs") + "x = 1\r\n\t~~|\r\ny = 2\r\n\t~~|\r\n" +
                             banner("Outputs") + kSketchFrame + kSettingsMarker + "15:0,0,0,0,0,0\r\n");
  CHECK(mBase != nullptr);
  CHECK(mRenamed != nullptr);
  CHECK(mRegrouped != nullptr);

  // Non-vacuity: confirm the fixtures genuinely differ in their grouping before
  // asserting the comparator notices. mBase: x->Inputs, y->Outputs.
  auto group_of = [](Model *m, const char *var) -> std::string {
    for (ModelGroup *g : m->Groups()) {
      for (Variable *v : g->vVariables) {
        if (v && v->GetName() == var)
          return g->sName;
      }
    }
    return std::string();
  };
  CHECK(group_of(mBase, "x") == "Inputs");
  CHECK(group_of(mRenamed, "x") == "Sources");
  CHECK(group_of(mRegrouped, "y") == "Inputs");

  // A renamed group and a moved member must both be reported as differences;
  // they can only originate in CompareGroups because everything else matches.
  CHECK(!ModelComparator::Compare(mBase, mRenamed).empty());
  CHECK(!ModelComparator::Compare(mBase, mRegrouped).empty());
  // A model compared to itself is clean.
  CHECK(ModelComparator::Compare(mBase, mBase).empty());

  delete mBase;
  delete mRenamed;
  delete mRegrouped;
}

// AC3.3: a model with explicit INITIAL TIME/FINAL TIME/TIME STEP/SAVEPER and a
// 15: line selecting RK4 round-trips. The fixture is verified non-vacuous up
// front: M0 must actually parse to IntegrationType()==RK4 (the 4th field of
// "15:0,0,0,1,0,0" maps to RK4), otherwise "the integration type round-trips"
// would be asserting a property the input never had. The four sim-spec values
// are also confirmed on M0 so the round-trip assertion below is meaningful.
TEST(ControlRoundTrip_integration_rk4) {
  const std::string mdl = ControlModel("15:0,0,0,1,0,0\r\n");

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  CHECK(m0->IntegrationType() == Integration_Type_RK4);
  CHECK(m0->GetConstanValue("INITIAL TIME", -999) == 0);
  CHECK(m0->GetConstanValue("FINAL TIME", -999) == 100);
  CHECK(m0->GetConstanValue("TIME STEP", -999) == 0.25);
  delete m0;

  ExpectCleanRoundTrip(mdl);
}

// AC3.3: same model but a 15: line selecting RK2 (4th field 3). Non-vacuity:
// M0 must parse to IntegrationType()==RK2 before the round trip is asserted.
TEST(ControlRoundTrip_integration_rk2) {
  const std::string mdl = ControlModel("15:0,0,0,3,0,0\r\n");

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  CHECK(m0->IntegrationType() == Integration_Type_RK2);
  delete m0;

  ExpectCleanRoundTrip(mdl);
}

// AC3.3: a 15: line selecting Euler (4th field 0). Non-vacuity: M0 must parse to
// IntegrationType()==EULER. This is the writer's default code, so this fixture
// guards against an inverted map that would have, say, emitted RK4's code for an
// Euler model and still round-tripped clean.
TEST(ControlRoundTrip_integration_euler) {
  const std::string mdl = ControlModel("15:0,0,0,0,0,0\r\n");

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  CHECK(m0->IntegrationType() == Integration_Type_EULER);
  delete m0;

  ExpectCleanRoundTrip(mdl);
}

// AC3.3: SAVEPER = TIME STEP must round-trip as the EQUATION (a reference to the
// TIME STEP variable), not collapse to a literal number. RoundTripDiffs alone
// cannot catch a regression here: the comparator's sim-spec check reads SAVEPER
// via GetConstanValue, which falls back to dt() on BOTH sides for a non-constant
// SAVEPER, so writing "SAVEPER = 0.25" instead of "SAVEPER = TIME STEP" would
// still compare equal. This focused check inspects the regenerated model's
// SAVEPER expression directly: it must be a variable reference to TIME STEP.
TEST(ControlRoundTrip_saveper_equals_time_step) {
  const std::string mdl = ControlModel("15:0,0,0,0,0,0\r\n");

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);

  // Non-vacuity: confirm the INPUT's SAVEPER is itself a variable reference, so
  // the round-trip check below is testing preservation, not a coincidence.
  Variable *saveper0 = FindVariable(m0, "SAVEPER");
  Equation *eq0 = saveper0 ? saveper0->GetEquation(0) : nullptr;
  CHECK(eq0 != nullptr);
  if (eq0)
    CHECK(eq0->GetExpression()->GetType() == EXPTYPE_Variable);

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());
  delete m0;

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  Variable *saveper = m1 ? FindVariable(m1, "SAVEPER") : nullptr;
  CHECK(saveper != nullptr);
  if (saveper) {
    Equation *eq1 = saveper->GetEquation(0);
    CHECK(eq1 != nullptr);
    if (eq1) {
      Expression *exp = eq1->GetExpression();
      // The decisive check: SAVEPER's RHS is still a variable reference (to
      // TIME STEP), not an ExpressionNumber.
      CHECK(exp->GetType() == EXPTYPE_Variable);
      if (exp->GetType() == EXPTYPE_Variable)
        CHECK(static_cast<ExpressionVariable *>(exp)->GetVariable()->GetName() == "TIME STEP");
    }
  }
  delete m1;
}

// AC3.3: a control variable's UNITS and COMMENT must round-trip. The comparator
// deliberately excludes control vars from CompareVariables and compares them only
// via CompareSimSpecs (numeric value + integration type), so a regression that
// dropped TIME STEP's units or comment would round-trip green through
// ExpectCleanRoundTrip undetected. This focused check bypasses the sim-spec-only
// path: it re-parses the regenerated .mdl and asserts the control Variable still
// carries the same Units() text and Comment() as the input.
TEST(ControlRoundTrip_control_units_and_comment) {
  // TIME STEP carries explicit units ("Year") and a comment; the other three
  // control vars are present so the settings round-trip the usual way. The "~~|"
  // for those three keeps their units/comment empty.
  const std::string mdl = std::string("{UTF-8}\r\n") +
                          "INITIAL TIME = 0\r\n\t~~|\r\n"
                          "FINAL TIME = 100\r\n\t~~|\r\n"
                          "TIME STEP = 0.25\r\n\t~Year~the simulation time step|\r\n"
                          "SAVEPER = TIME STEP\r\n\t~~|\r\n" +
                          kSketchFrame + kSettingsMarker + "15:0,0,0,0,0,0\r\n";

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);

  // Non-vacuity: the input's TIME STEP must actually carry the units and comment,
  // otherwise the round-trip assertion below would test preservation of nothing.
  Variable *ts0 = FindVariable(m0, "TIME STEP");
  CHECK(ts0 != nullptr);
  if (ts0) {
    CHECK(UnitsTextOf(ts0) == "Year");
    CHECK(CommentContent(ts0) == "the simulation time step");
  }

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());
  delete m0;

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  Variable *ts1 = m1 ? FindVariable(m1, "TIME STEP") : nullptr;
  CHECK(ts1 != nullptr);
  if (ts1) {
    // The decisive checks: the regenerated control var retains both fields. The
    // comparison strips the comment's leading separator whitespace, because the
    // lexer re-captures the "\t" the writer emits between the second '~' and the
    // text (VensimLex::GetComment) -- the writer itself strips that same leading
    // whitespace on emit so it never accretes (MDLGenerator::UnitsCommentTrailer),
    // so the CONTENT, not the separator, is what must round-trip.
    CHECK(UnitsTextOf(ts1) == "Year");
    CHECK(CommentContent(ts1) == "the simulation time step");
  }
  delete m1;
}

// AC3.3: two *** group banners, each owning a couple of variables, round-trip
// with their names and memberships intact. Non-vacuity is checked up front: M0
// must actually have two non-control groups with the expected members, otherwise
// "groups round-trip" would assert nothing.
TEST(ControlRoundTrip_groups) {
  const std::string mdl = std::string("{UTF-8}\r\n") +
                          "********************************************************\r\n"
                          "\tInputs\r\n"
                          "********************************************************~\r\n"
                          "\t\t|\r\n"
                          "alpha = 1\r\n\t~~|\r\n"
                          "beta = 2\r\n\t~~|\r\n"
                          "********************************************************\r\n"
                          "\tOutputs\r\n"
                          "********************************************************~\r\n"
                          "\t\t|\r\n"
                          "gamma = alpha + beta\r\n\t~~|\r\n"
                          "delta = alpha * beta\r\n\t~~|\r\n" +
                          kSketchFrame + kSettingsMarker + "15:0,0,0,0,0,0\r\n";

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);

  // Non-vacuity: confirm M0 has exactly the two non-control banners, each owning
  // its two variables. (Member lookup is by name; the parser assigns each
  // variable to the most-recent banner -- VensimParse::AddFullEq.)
  int inputs_members = -1, outputs_members = -1;
  for (ModelGroup *g : m0->Groups()) {
    if (g->sName == "Inputs")
      inputs_members = static_cast<int>(g->vVariables.size());
    else if (g->sName == "Outputs")
      outputs_members = static_cast<int>(g->vVariables.size());
  }
  CHECK(inputs_members == 2);
  CHECK(outputs_members == 2);
  delete m0;

  ExpectCleanRoundTrip(mdl);
}

// AC3.3: a model whose settings declare two unit equivalences round-trips them.
// Non-vacuity: M0 must actually carry both 22: payloads before the round trip is
// asserted (so a fixture whose settings section silently failed to parse cannot
// pass vacuously).
TEST(ControlRoundTrip_unit_equivs) {
  const std::string mdl = ControlModel("15:0,0,0,0,0,0\r\n", "22:Hour,Hours\r\n22:$,Dollar,Dollars\r\n");

  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  CHECK(m0->UnitEquivs().size() == 2);
  delete m0;

  ExpectCleanRoundTrip(mdl);
}
