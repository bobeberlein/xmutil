#include "../../src/Model.h"
#include "../TestHarness.h"
#include "ModelComparator.h"
#include "RoundTrip.h"

// A tiny but realistic Vensim model: a stock fed by a flow, a constant aux that
// drives the flow, and the four .Control sim-spec variables. Used as the shared
// fixture for the comparator and round-trip tests.
static const char *kFixture =
    "{UTF-8}\r\n"
    "Stock = INTEG( Flow, 10)\r\n"
    "\t~\twidgets\r\n"
    "\t~\tA stock.\r\n"
    "\t|\r\n"
    "\r\n"
    "Flow = Rate\r\n"
    "\t~\twidgets/Month\r\n"
    "\t~\tThe inflow.\r\n"
    "\t|\r\n"
    "\r\n"
    "Rate = 5\r\n"
    "\t~\twidgets/Month\r\n"
    "\t~\tConstant rate.\r\n"
    "\t|\r\n"
    "\r\n"
    "INITIAL TIME = 0\r\n"
    "\t~\tMonth\r\n"
    "\t~\t\r\n"
    "\t|\r\n"
    "\r\n"
    "FINAL TIME = 100\r\n"
    "\t~\tMonth\r\n"
    "\t~\t\r\n"
    "\t|\r\n"
    "\r\n"
    "TIME STEP = 1\r\n"
    "\t~\tMonth\r\n"
    "\t~\t\r\n"
    "\t|\r\n"
    "\r\n"
    "SAVEPER = 1\r\n"
    "\t~\tMonth\r\n"
    "\t~\t\r\n"
    "\t|\r\n";

TEST(MDLGenerator_emits_utf8_shell) {
  Model m;
  std::vector<std::string> errs;
  std::string out = m.PrintMDL(errs);
  CHECK(errs.empty());
  CHECK(out.rfind("{UTF-8}", 0) == 0);  // starts with {UTF-8}
  // The shell re-parses without error.
  Model *reparsed = roundtrip::ParseVensim(out);
  CHECK(reparsed != nullptr);
  delete reparsed;
}

TEST(ModelComparator_equivalent_when_same_source_parsed_twice) {
  Model *a = roundtrip::ParseVensim(kFixture);
  Model *b = roundtrip::ParseVensim(kFixture);
  CHECK(a != nullptr);
  CHECK(b != nullptr);
  std::vector<std::string> diffs = ModelComparator::Compare(a, b);
  // Parsing identical source twice must compare as equivalent. Print any diffs
  // to make a failure self-explanatory.
  for (const std::string &d : diffs)
    printf("  unexpected diff: %s\n", d.c_str());
  CHECK(diffs.empty());
  delete a;
  delete b;
}

TEST(ModelComparator_detects_renamed_variable) {
  // A deliberately mutated copy of the fixture: the constant aux Rate becomes
  // RateX. The comparator must report a non-empty diff so round-trip tests
  // cannot pass vacuously.
  std::string mutated = kFixture;
  size_t pos = mutated.find("Rate = 5");
  CHECK(pos != std::string::npos);
  mutated.replace(pos, std::string("Rate").size(), "RateX");
  // Flow references Rate; rename that occurrence too so the mutated model still
  // parses cleanly. The point is the variable set differs (Rate vs RateX).
  size_t fpos = mutated.find("Flow = Rate");
  CHECK(fpos != std::string::npos);
  mutated.replace(fpos + std::string("Flow = ").size(), std::string("Rate").size(), "RateX");

  Model *a = roundtrip::ParseVensim(kFixture);
  Model *b = roundtrip::ParseVensim(mutated);
  CHECK(a != nullptr);
  CHECK(b != nullptr);
  std::vector<std::string> diffs = ModelComparator::Compare(a, b);
  CHECK(!diffs.empty());
  delete a;
  delete b;
}

// AC1.1 / AC1.3: the parse -> PrintMDL -> re-parse pipeline is wired end to end.
// In Phase 1 the writer emits only the {UTF-8} shell, so the regenerated model
// is empty; we assert the whole sequence succeeds and produces a model rather
// than asserting content equality (a full content round-trip lands in Phase 3).
TEST(MDLGenerator_shell_round_trip) {
  Model *m0 = roundtrip::ParseVensim(kFixture);
  CHECK(m0 != nullptr);

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());
  CHECK(regen.rfind("{UTF-8}", 0) == 0);

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);

  delete m0;
  delete m1;
}

// AC3.5 guard: a model that differs from the fixture in a single sim-spec
// constant (FINAL TIME) must produce a non-empty diff. This proves the
// comparator is not vacuous, so later round-trip tests cannot pass trivially.
// (An equation-internal constant such as a stock's initial value is compared
// only from Phase 3 onward, once the expression-AST getters exist; a sim-spec
// constant is the Phase-1 analogue of "change a constant".)
TEST(ModelComparator_detects_mutation) {
  std::string mutated = kFixture;
  size_t pos = mutated.find("FINAL TIME = 100");
  CHECK(pos != std::string::npos);
  mutated.replace(pos, std::string("FINAL TIME = 100").size(), "FINAL TIME = 200");

  Model *a = roundtrip::ParseVensim(kFixture);
  Model *b = roundtrip::ParseVensim(mutated);
  CHECK(a != nullptr);
  CHECK(b != nullptr);
  std::vector<std::string> diffs = ModelComparator::Compare(a, b);
  CHECK(!diffs.empty());
  delete a;
  delete b;
}
